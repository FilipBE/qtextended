/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include "smtpclient.h"

#include <QTextCodec>

#include <qmailaddress.h>
#include <qmailcodec.h>
#include <qtopiaapplication.h>

#include "mailtransport.h"
#include "emailhandler.h"

SmtpClient::SmtpClient(QObject* parent)
    : TransmittingClient(parent)
{
    sending = false;
    authenticating = false;
    transport = 0;
}

SmtpClient::~SmtpClient()
{
    delete transport;
}

void SmtpClient::setAccount(const QMailAccountId &id)
{
    accountId = id;
}

QMailAccountId SmtpClient::account() const
{
    return accountId;
}

void SmtpClient::newConnection()
{
    qLog(SMTP) << "newConnection" << flush;
    if (sending) {
        QString msg("Cannot send message; transport in use");
        errorHandling(QMailMessageServer::ErrConnectionInUse, msg);
        return;
    }

    if (!accountId.isValid()) {
        status = Done;
        QString msg("Cannot send message without account configuration");
        errorHandling(QMailMessageServer::ErrConfiguration, msg);
        return;
    }

    // Load the current configuration for this account
    _config = AccountConfiguration(accountId);

    if ( _config.smtpServer().isEmpty() ) {
        status = Done;
        QString msg("Cannot send message without SMTP server configuration");
        errorHandling(QMailMessageServer::ErrConfiguration, msg);
        return;
    }

    if (!transport) {
        // Set up the transport
        transport = new MailTransport("SMTP");

        connect(transport, SIGNAL(readyRead()),
                this, SLOT(readyRead()));
        connect(transport, SIGNAL(connected(AccountConfiguration::EncryptType)),
                this, SLOT(connected(AccountConfiguration::EncryptType)));
        connect(transport, SIGNAL(bytesWritten(qint64)),
                this, SLOT(sent(qint64)));
        connect(transport, SIGNAL(updateStatus(QString)),
                this, SLOT(transportStatus(QString)));
        connect(transport, SIGNAL(errorOccurred(int,QString)),
                this, SLOT(errorHandling(int,QString)));
    }

    // authenticate with POP first?
    bool authenticateFirst = ( !_config.mailServer().isEmpty() && 
                               (_config.smtpAuthentication() == AccountConfiguration::Auth_INCOMING) );
    doSend(authenticateFirst);
}

void SmtpClient::doSend(bool withAuthentication)
{
    status = Init;
    sending = true;
    authenticating = withAuthentication;

    if (authenticating)
    {
        qLog(SMTP) << "Open Authentication connection" << flush;
        transport->open(_config.mailServer(), _config.mailPort(), _config.mailEncryption());
    }
    else
    {

        qLog(SMTP) << "Open SMTP connection" << flush;
        transport->open(_config.smtpServer(), _config.smtpPort(), _config.smtpEncryption());
    }
}

#ifndef QT_NO_OPENSSL
QString SmtpClient::_toBase64(const QString& in) const
{
    // We really should QByteArray::toBase64 here, because we don't want embedded line breaks
    QMailBase64Codec codec(QMailBase64Codec::Text);
    QByteArray encoded = codec.encode(in, "ISO-8859-1");
    return QString::fromLatin1(encoded.constData(), encoded.length());
}
#endif

int SmtpClient::addMail(const QMailMessage& mail)
{
    if (mail.from().address().isEmpty()) {
        qLog(Messaging) << "Cannot send SMTP message with empty from address!";
        return QMailMessageServer::ErrInvalidAddress;
    }

    QStringList sendTo;
    foreach (const QMailAddress& address, mail.recipients()) {
        // Only send to mail addresses
        if (address.isEmailAddress())
            sendTo.append(address.address());
    }

    if (sendTo.isEmpty()) {
        qLog(Messaging) << "Cannot send SMTP message with empty recipient address!";
        return QMailMessageServer::ErrInvalidAddress;
    }

    RawEmail rawmail;
    rawmail.from = "<" + mail.from().address() + ">";
    rawmail.to = sendTo;
    rawmail.mail = mail;

    mailList.append(rawmail);
    return QMailMessageServer::ErrNoError;
}

void SmtpClient::connected(AccountConfiguration::EncryptType encryptType)
{
    if (_config.smtpEncryption() == encryptType) {
        qLog(SMTP) << "Connected" << flush;
        emit updateStatus(QMailMessageServer::None, tr("Connected"));
    }

#ifndef QT_NO_OPENSSL
    if (status == TLS)
    {
        transport->stream() << "EHLO qtopia-messageserver\r\n" << flush;
        if ((_config.smtpAuthentication() == AccountConfiguration::Auth_LOGIN) ||
            (_config.smtpAuthentication() == AccountConfiguration::Auth_PLAIN))
            status = Auth;
        else
            status = From;
    }
#endif
}

void SmtpClient::errorHandling(int errorCode, QString msg)
{
    if ( sending ) {
    transport->close();
    qLog(SMTP) << "Closed connection" << flush;
    
    if (authenticating) {
            // authentication failure; pop may be down, but try sending anyway?
            qLog(SMTP) << "Mail server authentication unavailable";
            if (errorCode != QMailMessageServer::ErrCancel) {
                doSend();
                return;
            }
        }

        sendingId = QMailMessageId();
        sending = false;
    }

    if (msg.isEmpty())
        msg = tr("Error occurred");

    mailList.clear();

    emit updateStatus(QMailMessageServer::None, msg);
    emit errorOccurred(errorCode, msg);
}

void SmtpClient::sent(qint64 size)
{
    if (sendingId.isValid() && messageLength) {
        sentLength += size;
        uint percentage = qMin<uint>(sentLength * 100 / messageLength, 100);
        emit sendProgress(sendingId, percentage);
    }
}

void SmtpClient::readyRead()
{
    // Are we processing the authentication stage?
    if (authenticating) {
        authenticate();
    } else {
        incomingData();
    }
}

void SmtpClient::authenticate()
{
    qLog(SMTP) << "Authenticate begin" << flush;
    QTextStream& stream = transport->stream();
    QString response = transport->readLine();
    qLog(SMTP) << "Authenticate response " << response.left(response.length() - 2) << flush;

    QMailAccount account(accountId);
    if ( account.messageSources().contains("imap4", Qt::CaseInsensitive) ) {
        if (status == Init ) {
            qLog(SMTP) << "Authenticating IMAP Init" << flush;
            status = Done;
            qLog(SMTP) << "sent:" << "A01 LOGIN " + _config.mailUserName() + " <password hidden>" << flush;
            stream << "A01 LOGIN " + _config.mailUserName() + " " + _config.mailPassword() + "\r\n" << flush;
            qLog(SMTP) << "Authenticate end" << flush;
            return;
        } else if ( status == Done ) {
            qLog(SMTP) << "Authenticating IMAP Done" << flush;
            QString rsp = response.mid(3, response.indexOf(" ") ).trimmed();
            if ( rsp.toUpper() != "OK") {
                qLog(SMTP) << "Authentication failed:" << rsp << flush;
                errorHandling(QMailMessageServer::ErrLoginFailed, "");
                qLog(SMTP) << "Authenticate end" << flush;
                return;
            }

            status = Quit;
            qLog(SMTP) << "sent:" << "A02 LOGOUT" << flush;
            stream << "A02 LOGOUT\r\n" << flush;
            qLog(SMTP) << "Authenticate end" << flush;
            return;
        } else if ( status == Quit ) {
            qLog(SMTP) << "Authenticating IMAP Quit" << flush;
            transport->close();
            qLog(SMTP) << "Authenticate end" << flush;

            // Begin the data connection
            doSend();
            return;
        }
    }

    if (status == Init) {
        status = Pass;
        qLog(SMTP) << "Authenticating POP sent:" << "USER " << _config.mailUserName() << flush;
        stream << "USER " << _config.mailUserName() << "\r\n" <<flush;
        return;
    } else if (status == Pass) {
        if (response[0] != '+') {
            errorHandling(QMailMessageServer::ErrLoginFailed, "");
            return;
        }

        status = Done;
        qLog(SMTP) << "Authenticating POP sent:" << "PASS <password hidden>" << flush;
        stream << "PASS " << _config.mailPassword() << "\r\n" << flush;
        return;
    } else if (status == Done) {
        if (response[0] != '+') {
            errorHandling(QMailMessageServer::ErrLoginFailed, "");
            return;
        }

        status = Quit;
        qLog(SMTP) << "Authenticating POP sent:" << "QUIT" << flush;
        stream << "QUIT\r\n" << flush;
        return;
    } else if ( status == Quit ) {
        transport->close();
        qLog(SMTP) << "Closed Authentication connection" << flush;

        // Begin the data connection
        doSend();
    }
}

void SmtpClient::incomingData()
{
    QString response;
    QString line;

    if (!transport->canReadLine())
        return;

    do
    {
        line = transport->readLine();
        response += line;
    }
    while (transport->canReadLine() && line.length() != 0);
    qLog(SMTP) << "response:" << response.left(response.length() - 2) << flush;

    QTextStream& stream = transport->stream();

    switch (status) {
    case Init:  {
        if (response[0] == '2') {
            status = From;
            mailItr = mailList.begin();
#ifndef QT_NO_OPENSSL
            if ((_config.smtpAuthentication() == AccountConfiguration::Auth_LOGIN ||
                 _config.smtpAuthentication() == AccountConfiguration::Auth_PLAIN) ||
                _config.smtpEncryption() == AccountConfiguration::Encrypt_TLS)
            {
                qLog(SMTP) << "Init: sent:" << "EHLO qtopia-messageserver" << flush;
                stream << "EHLO qtopia-messageserver\r\n" << flush;
                if (_config.smtpEncryption() == AccountConfiguration::Encrypt_TLS)
                    status = StartTLS;
                else
                    status = Auth;
            }
            else
#endif
            {
                qLog(SMTP) << "Init: sent:" << "HELO qtopia-messageserver" << flush;
                stream << "HELO qtopia-messageserver\r\n" << flush;
            }
        } else  {
            errorHandling(QMailMessageServer::ErrUnknownResponse, response);
        } 
        break;
    }
#ifndef QT_NO_OPENSSL
    case StartTLS:
    {
        if (line[0] == '2') {
            qLog(SMTP) << "StartTLS: sent:" << "STARTTLS" << flush;
            stream << "STARTTLS\r\n" << flush;
            status = TLS;
        } else  {
            errorHandling(QMailMessageServer::ErrUnknownResponse,response);
        } 
        break;
    }
    case TLS:
    {
        if (line[0] == '2') {
            // Switch into encrypted mode
            transport->switchToEncrypted();
        } else  {
            errorHandling(QMailMessageServer::ErrUnknownResponse,response);
        }
        break;
    }

    case Auth:
    {
        if (line[0] == '2') {
            if (_config.smtpAuthentication() == AccountConfiguration::Auth_LOGIN)
        {
                qLog(SMTP) << "Auth: sent:" << "AUTH LOGIN " << flush;
                stream << "AUTH LOGIN \r\n" << flush;
                status = AuthUser;
            }
            else if (_config.smtpAuthentication() == AccountConfiguration::Auth_PLAIN)
            {
                QString temp = _config.smtpUsername() + '\0' + _config.smtpUsername() + '\0' + _config.smtpPassword();
                temp = _toBase64(temp);
                qLog(SMTP) << "Auth: sent:" << "AUTH PLAIN " << "<Base64 username/password hidden>" << flush;
                stream << "AUTH PLAIN " << temp << "\r\n" << flush;
                status = From;
            }
        } else  {
            errorHandling(QMailMessageServer::ErrUnknownResponse, response);
        }
        break;
    }

    case AuthUser:
    {
        if (line[0] == '3') {
            qLog(SMTP) << "AuthUser: sent:" << _toBase64(_config.smtpUsername()) << flush;
            stream << _toBase64(_config.smtpUsername()) << "\r\n" << flush;
            status = AuthPass;
        } else {
            errorHandling(QMailMessageServer::ErrUnknownResponse, response);
        }
        break;
    }
    case AuthPass:
    {
        if (line[0] == '3') {
            qLog(SMTP) << "AuthPass: sent:" << "<Base64 password hidden>" << flush;
            stream << _toBase64(_config.smtpPassword()) << "\r\n" << flush;
            status = From;
        } else {
            errorHandling(QMailMessageServer::ErrUnknownResponse, response);
        }
        break;
    }
#endif

    case Login:
    case Pass:
    case Done:  {
        // Supposed to be unused here - handled by authentication
        qWarning() << "incomingData - Unexpected status value: " << status;
        break;
    }

    case From:  {
        if (response[0] == '2') {
            if (sendingId.isValid())
                emit messageTransmitted(sendingId);

            qLog(SMTP) << "From: sent:" << "MAIL FROM: " << mailItr->from << flush;
            stream << "MAIL FROM: " << mailItr->from << "\r\n" << flush;
            status = Recv;
        } else {
            errorHandling(QMailMessageServer::ErrUnknownResponse, response);
        }

        if (sendingId.isValid()) {
            // The last send operation is complete
            emit messageProcessed(sendingId);
            sendingId = QMailMessageId();
        } 
        break;
    }
    case Recv:  {
        if (response[0] == '2') {
            it = mailItr->to.begin();
            if (it == mailItr->to.end())
                errorHandling(QMailMessageServer::ErrUnknownResponse, "no recipients");
            qLog(SMTP) << "Recv: sent:" << "RCPT TO: <" << *it << '>' << flush;
            stream << "RCPT TO: <" << *it << ">\r\n" << flush;
            status = MRcv;
        } else  {
            errorHandling(QMailMessageServer::ErrUnknownResponse, response);
        } 
        break;
    }
    case MRcv:  {
        if (response[0] == '2') {
            it++;
            if ( it != mailItr->to.end() ) {
                qLog(SMTP) << "MRcv: sent:" << "RCPT TO: <" << *it << '>' << flush;
                stream << "RCPT TO: <" << *it << ">\r\n" << flush;
                break;
            } else  {
                status = Data;
                // fall-through
            }
        } else {
            errorHandling(QMailMessageServer::ErrUnknownResponse, response);
            break;
        }
    }
    case Data:  {
        if (response[0] == '2') {
            qLog(SMTP) << "Data: sent:" << "DATA" << flush;
            stream << "DATA\r\n" << flush;
            status = Body;
            emit updateStatus(QMailMessageServer::Send, tr( "Sending: %1").arg(mailItr->mail.subject()) );
        } else {
            errorHandling(QMailMessageServer::ErrUnknownResponse, response);
        } 
        break;
    }
    case Body:  {
        if (response[0] == '3' || response[0] == '2') {
            QByteArray rfcData = mailItr->mail.toRfc2822(QMailMessage::TransmissionFormat);

            sendingId = mailItr->mail.id();
            messageLength = rfcData.length();
            sentLength = 0;

            qLog(SMTP) << "Body: sent:" << "<rfcData omitted for brevity>" << flush;
            stream << rfcData;
            qLog(SMTP) << "Body: sent:" << "." << flush;
            stream << "\r\n.\r\n" << flush;

            mailItr++;
            if (mailItr != mailList.end()) {
                status = From;
            } else {
                status = Quit;
            }
        } else {
            errorHandling(QMailMessageServer::ErrUnknownResponse, response);
        }
        break;
    }
    case Quit:  {
        if ( response[0] == '2' || response[0] == '3' ) {
            if (sendingId.isValid())
                emit messageTransmitted(sendingId);

            qLog(SMTP) << "Quit: sent:" << "QUIT" << flush;
            stream << "QUIT\r\n" << flush;

            sending = false;
            status = Done;
            transport->close();
            qLog(SMTP) << "Closed connection" << flush;

            int count = mailList.count();
            mailList.clear();

            emit updateStatus(QMailMessageServer::Send,  tr("Sent %n messages", "", count));
        } else {
            errorHandling(QMailMessageServer::ErrUnknownResponse, response);
        }

        if (sendingId.isValid()) {
            // The last send operation is complete
            emit messageProcessed(sendingId);
            sendingId = QMailMessageId();
        }
        break;
    }
    }
}

void SmtpClient::transportStatus(const QString& status)
{
    emit updateStatus(QMailMessageServer::None, status);
}

void SmtpClient::cancelTransfer()
{
    errorHandling(QMailMessageServer::ErrCancel, tr("Cancelled by user"));
}

