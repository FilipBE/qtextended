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


#ifndef QTOPIA_NO_SMS
#include "smsclient.h"
#include "smsdecoder.h"

#include <private/accountconfiguration_p.h>

#include <qphonenumber.h>
#include <qsmsreader.h>
#include <qsmsmessage.h>
#include <qsmssender.h>
#include <qmailaddress.h>
#include <qmailmessage.h>
#include <qmailtimestamp.h>
#include <QMailStore>
#include <QMailAccount>
#include <qregexp.h>
#include <qtopialog.h>
#include <QSettings>


static const int retrievalTimeoutPeriod = 2000;

QRegExp* SmsClient::sSmsAddress = 0;
QRegExp* SmsClient::sValidSmsAddress = 0;


SmsClient::SmsClient(QObject* parent) 
    : TransmittingClient(parent),
      req(0)
{
    smsFetching = false;
    smsSending = false;
    success = false;
    haveSimIdentity = false;
    sawNewMessage = false;
    smsCheckRequired = false;

    // TODO: this matches everything - what's the point?
    if (!sSmsAddress)
      sSmsAddress = new QRegExp( "^.*$");
    if (!sValidSmsAddress)
      sValidSmsAddress = new QRegExp( "^[\\+\\*#\\d\\-\\(\\)\\s]*$");

    sender = new QSMSSender( QString(), this );
    connect( sender, SIGNAL(finished(QString,QTelephony::Result)),
             this, SLOT(finished(QString,QTelephony::Result)) );

    simInfo = new QSimInfo( QString(), this );

    req = new QSMSReader( QString(), this );
    connect( req, SIGNAL(messageCount(int)),
             this, SLOT(messageCount(int)) );
    connect( req, SIGNAL(fetched(QString,QSMSMessage)),
             this, SLOT(fetched(QString,QSMSMessage)) );
    connect( req, SIGNAL(readyChanged()),
             this, SLOT(simReadyChanged()) );

    connect( &retrievalTimer, SIGNAL(timeout()), 
             this, SLOT(retrievalTimeout()) );
}

SmsClient::~SmsClient()
{
}

void SmsClient::setAccount(const QMailAccountId &id)
{
    accountId = id;
    _config = AccountConfiguration(accountId);
}

QMailAccountId SmsClient::account() const
{
    return accountId;
}

void SmsClient::newConnection()
{
    // XXX need better way to handle this.
    QSettings c("PhoneProfile"); // no tr
    c.beginGroup("Profiles"); // no tr
    bool planeMode = c.value("PlaneMode", false).toBool();
    smsSending = true;
    if (planeMode) {
        errorHandling(0, tr("Messages cannot be sent in Airplane Mode."));
        return;
    }

    QList<RawSms>::iterator rawMsg;
    for ( rawMsg = smsList.begin(); rawMsg != smsList.end(); rawMsg++) {
        QSMSMessage msg;

        //check for vcard over sms
        if (rawMsg->mimetype == QLatin1String("text/x-vCard")) {
            QString vcardData = rawMsg->body;
            //restore CR's stripped by composer
            vcardData.replace("\n","\r\n");
            msg.setApplicationData(vcardData.toLatin1());
            msg.setDestinationPort(9204);
        } else {
            msg.setText( rawMsg->body );
        }

        msg.setRecipient( rawMsg->number );

        QString smsKey = sender->send( msg );
        sentMessages.insert( smsKey, *rawMsg );
    }

    success = true;
    smsSending = false;
    smsList.clear();
}

int SmsClient::addMail(const QMailMessage& mail)
{
    QList<QMailAddress> smsRecipients = separateSmsAddresses(mail.recipients());
    Q_ASSERT(smsRecipients.count() > 0);

    QString smsBody = formatOutgoing(mail.subject(),mail.body().data());

    foreach (const QMailAddress& recipient, smsRecipients) {
        if (smsAddress(recipient)) {
            if (!validSmsAddress(recipient)) {
                return QMailMessageServer::ErrInvalidAddress;
            } else {
                // Extract the phone number from the e-mail address.
                RawSms msg;
                msg.msgId = mail.id();
                msg.number = QPhoneNumber::resolveLetters( recipient.address() );
                msg.body = smsBody;
                if (mail.contentType().content().toLower() == "text/x-vcard")
                    msg.mimetype = QLatin1String("text/x-vCard");
                else
                    msg.mimetype = QLatin1String("text/plain");

                smsList.append( msg );
            }
        }
    }

    return QMailMessageServer::ErrNoError;
}

void SmsClient::clearList()
{
    smsList.clear();
}

bool SmsClient::smsAddress( const QMailAddress& fromAddress )
{
    return sSmsAddress->indexIn( fromAddress.address() ) != -1;
}

bool SmsClient::validSmsAddress( const QMailAddress& fromAddress )
{
    return sValidSmsAddress->indexIn( fromAddress.address() ) != -1;
}

QList<QMailAddress> SmsClient::separateSmsAddresses( const QList<QMailAddress> &addresses )
{
    QList<QMailAddress> validSms;
    foreach (const QMailAddress& address, addresses)
        if (validSmsAddress( address ) )
            validSms.append( address );

    return validSms;
}

void SmsClient::errorHandling(int id, QString msg)
{
    if (smsSending)
        emit errorOccurred(id, msg);
}

void SmsClient::finished( const QString& id, QTelephony::Result result )
{
    QMap<QString, RawSms>::iterator it = sentMessages.find(id);
    if (it != sentMessages.end()) {
        QMailMessageId msgId(it->msgId);
        sentMessages.erase(it);

        if ( result == QTelephony::OK )
            emit messageTransmitted(msgId);
        else
            success = false;

        emit messageProcessed(msgId);
    } else {
        qWarning() << "SMS: Cannot process unknown message:" << id;
    }
}

void SmsClient::messageCount( int reported )
{
    if ( reported > 0 && !smsFetching) {
        smsFetching = true;

        // Get the SIM identity, for use in creating unique identifiers.
        // After we get the identity, we will start fetching the messages.
        haveSimIdentity = false;
        simIdentityChanged();

    } else if ( reported > 0 && smsFetching ) {

        // A new message arrived in the queue while we were fetching
        // the existing messages.  By setting "sawNewMessage" to true,
        // we can force another check to happen at the end of the fetch.
        sawNewMessage = true;

    } else if ( reported == 0 ) {

        // We're not waiting for new messages any longer
        retrievalTimer.stop();

        emit allMessagesReceived();
    }
}

// Extract the subject line and body from an SMS message.
// According to GSM 03.40, subjects can be encoded in one
// of two ways:
//
//          (subject)message
//          ##subject#message
//
// If the message doesn't have one of the formats above,
// then we extract the first few words from the message
// body and use that as the subject.
static void extractSubjectAndBody( const QString& str, QString& subject,
                                   QString& body )
{
    int index;

    // See if we have one of the standard GSM 03.40 forms.
    if ( str.startsWith( "(" ) ) {
        index = str.indexOf(')');
        if ( index != -1 ) {
            subject = str.mid( 1, index - 1 );
            body = str.mid( index + 1 );
            return;
        }
    } else if ( str.startsWith( "##" ) ) {
        index = str.indexOf( '#', 2 );
        if ( index != -1 ) {
            subject = str.mid( 2, index - 2 );
            body = str.mid( index + 1 );
            return;
        }
    }

    // Extract the first few words from the body to use as the subject.
    int posn = 0;
    int lastSpace = 0;
    while ( posn < str.length() && posn < 30 ) {
        if ( str[posn] == ' ' || str[posn] == '\t' ) {
            lastSpace = posn;
        } else if ( str[posn] == '\r' || str[posn] == '\n' ) {
            // Only return the first line if there is an EOL present.
            subject = str.left( posn );
            body = str;
            return;
        }
        ++posn;
    }
    if ( posn < 30 )
        lastSpace = posn;
    subject = str.left( lastSpace );
    body = str;
}

QMailMessage SmsClient::toMailMessage(const QSMSMessage& message, const QString& id, const QString& simId)
{
    QMailMessage mail;
    mail.setMessageType( QMailMessage::Sms );

    // Construct a full identity for the message.  This should be
    // unique enough to identify messages on different SIM's.
    QDateTime dt = message.timestamp();
    QString identity = QString("sms:%1:%2%3%4%5%6%7:>%8")
                               .arg( simId )
                               .arg( dt.date().year(), 4 )
                               .arg( dt.date().month(), 2 )
                               .arg( dt.date().day(), 2 )
                               .arg( dt.time().hour(), 2 )
                               .arg( dt.time().minute(), 2 )
                               .arg( dt.time().second(), 2 )
                               .arg( id );

    mail.setServerUid( identity );

    // If the sender is not set, but the recipient is, then this
    // is probably an outgoing message that was reflected back
    // by the phone simulator.
    if( !message.sender().isEmpty() )
        mail.setHeaderField( "From", message.sender() );
    else if( !message.recipient().isEmpty() )
        mail.setHeaderField( "From", message.recipient() );

    // Extract the subject and body.
    QString subject, body;
    extractSubjectAndBody( message.text(), subject, body );

    // Set the subject from the first few words of the text.
    mail.setSubject( subject );

    // Determine if the entire body is text, or if it contains attachments.
    bool hasAttachments = false;
    QList<QSMSMessagePart> parts = message.parts();
    for ( int part = 0; part < parts.count(); ++part ) {
        if ( !(parts[part].isText()) ) {
            hasAttachments = true;
            break;
        }
    }
    if( !hasAttachments ) {
        QMailMessageContentType type("text/plain; charset=UTF-8");
        mail.setBody( QMailMessageBody::fromData( body, type, QMailMessageBody::Base64 ) );
    } else {
        SMSDecoder::formatMessage( mail, message );
    }

    // Set the reception date.
    if (!dt.isValid())
        dt = QDateTime::currentDateTime();
    mail.setDate( QMailTimeStamp( dt ) );

    // Synthesize some other headers
    QString smsType;
    switch(message.messageType()) {
        case QSMSMessage::Normal:
            smsType = "normal"; break;
        case QSMSMessage::CellBroadCast:
            smsType = "broadcast"; break;
        case QSMSMessage::StatusReport:
            smsType = "status-report"; break;
        default:
            smsType = "unknown"; break;
    }
    mail.setHeaderField( "X-Sms-Type", smsType );

    int messageClass = message.messageClass();
    if (messageClass != -1)
        mail.setHeaderField( "X-Sms-Class", QString::number(messageClass) );

    mail.setStatus( QMailMessage::Downloaded, true);

    // Is this necessary?
    QByteArray mailStr = mail.toRfc2822();
    mail.setSize( mailStr.length() );

    return mail;
}

// Format an outgoing message using GSM 03.40 rules.
QString SmsClient::formatOutgoing( const QString& /*subject*/, const QString &body )
{
#if 0
    // If the subject is empty, then return the body as-is.
    if ( subject.isEmpty() || subject == tr("(no subject)") ) {
        return body;
    }

    // Concatenate the subject and body using GSM 03.40 encoding rules.
    return "##" + subject + "#" + body;
#endif

    // XXX - the subject formatting confuses some users, so it
    // has been disabled for the time being.
    return body;
}

bool SmsClient::hasDeleteImmediately() const
{
    return true;
}

void SmsClient::deleteImmediately(const QString& serverUid)
{
    // Bail out if the SIM does not actually contain this message.
    // This is probably because another SIM was inserted, or because
    // the message was deleted on another phone before the SIM was
    // re-inserted into this one.
    if ( !activeIds.contains( serverUid ) ) {
        return;
    }

    // Extract the qtopiaphone identity for the message.
    int posn = serverUid.lastIndexOf( QChar('>') );
    if ( posn < 0 )
        return;     // Shouldn't happen, but be careful anyway.
    QString id = serverUid.mid( posn + 1 );

    // Send the deletion request to the SIM.
    req->deleteMessage( id );

    // Remove the identifier from the active identifiers.
    while (activeIds.indexOf( serverUid ) != -1) {
        int idx = activeIds.indexOf( serverUid );
        activeIds.removeAt( idx );
        timeStamps.removeAt( idx );
    }
}

void SmsClient::resetNewMailCount()
{
    req->setUnreadCount(0);
}

void SmsClient::fetched( const QString& id, const QSMSMessage& message )
{
    if (!id.isEmpty()) {
        QMailMessage mail(toMailMessage(message, id, simIdentity));

        // Add it to the active list, so that we know what's on the
        // inserted SIM right now, as opposed to the cached copy in
        // the mailbox folder.
        const QString identity(mail.serverUid());
        activeIds += identity;
        timeStamps += message.timestamp();

        // If we already have this message in the mailbox, then ignore it.
        if ( accountId.isValid() ) {
            // TODO: Surely this logic belongs in the email handler, or message server?
            if (retrievedIds.isEmpty()) {
                retrievedIds = QMailAccount(accountId).serverUids();
            }
            if ( retrievedIds.contains( identity ) ) {
                if ( _config.canDeleteMail() )
                    deleteImmediately( id );
                req->nextMessage();
                return;
            }


            mail.setParentAccountId( accountId );
            mail.setFromMailbox( QString() );
        }

        retrievedIds += identity;
        retrievedMessages.append(mail);

        // If the "deleteMail" flag is set, then delete the message
        // from the SIM immediately, rather than waiting for later.
        if ( accountId.isValid() && _config.canDeleteMail() )
            deleteImmediately( QString::number(id.toULongLong()) );

        req->nextMessage();
    } else {
        smsFetching = false;
        if ( sawNewMessage ) {
            // Check again, just in case another new message arrived
            // while we were performing the fetch.
            req->check();
            return;
        }

        allMessagesRetrieved();
        retrievedIds.clear();

        // Make sure there are always 5 free slots on the SIM card
        // so there is enough space for the reception of new messages.
        if ( accountId.isValid()
             && !_config.canDeleteMail()
             && (req->totalMessages() - req->usedMessages()) < 5
             && req->totalMessages() >= 5
             && !timeStamps.isEmpty() ) {
            int toBeDeleted = 5 - (req->totalMessages() - req->usedMessages());
            while ( toBeDeleted-- > 0 && activeIds.size() > 0 ) {
                QDateTime dt = timeStamps[0];
                int index = 0;
                for (int i = 1; i < timeStamps.size(); ++i) {
                    if (timeStamps[i] < dt) {
                        dt = timeStamps[i];
                        index = i;
                    }
                }
                deleteImmediately( activeIds[index] );
                activeIds.removeAt( index );
                timeStamps.removeAt( index );
            }
        }
    }
}

static bool lessThanByTimestamp(const QMailMessage& lhs, const QMailMessage& rhs)
{
    return (lhs.date() < rhs.date());
}

void SmsClient::allMessagesRetrieved()
{
    retrievalTimer.stop();

    int newSmsCount = req->unreadCount();
    int retrievedCount = retrievedMessages.count();

    // Are these new SMS messages, or were they put on the SIM by a previous phone?
    if (retrievedCount > 0) {
        // Find how many of these messages have not been acknowledged
        if (retrievedCount > newSmsCount) {
            // We need to flag all except the \a newSmsCount most recent as read
            qSort(retrievedMessages.begin(), retrievedMessages.end(), lessThanByTimestamp);

            QList<QMailMessage>::iterator it = retrievedMessages.begin();
            for ( ; retrievedCount > newSmsCount; --retrievedCount, ++it)
                (*it).setStatus(QMailMessage::Read, true);
        }

        QList<QMailMessage>::iterator it = retrievedMessages.begin(), end = retrievedMessages.end();
        for ( ; it != end; ++it)
            emit newMessage((*it), false);

        retrievedMessages.clear();
    }

    emit allMessagesReceived();
}

void SmsClient::simReadyChanged()
{
    emit simReady(req->ready());

    if (req->ready() && smsCheckRequired) {
        // Force a message check if the sim has just become ready.
        smsCheckRequired = false;
        req->check();
    }
}

void SmsClient::simIdentityChanged()
{
    if ( smsFetching && !haveSimIdentity ) {
        haveSimIdentity = true;
        simIdentity = simInfo->identity();
        sawNewMessage = false;
        activeIds.clear();
        timeStamps.clear();
        req->firstMessage();
    }
}

void SmsClient::checkForNewMessages()
{
    retrievalTimer.start(retrievalTimeoutPeriod);

    if ( req->ready() ) {
        // Notify interested parties that the SIM is already ready
        simReadyChanged();

        req->check();
    } else {
        smsCheckRequired = true;
    }
}

void SmsClient::cancelTransfer()
{
    errorHandling(QMailMessageServer::ErrCancel, tr("Cancelled by user"));
}

void SmsClient::retrievalTimeout()
{
    // We weren't able to retrieve messages from the SIM in a reasonable period;
    // pretend we were successful to allow the server to proceed
    qLog(Messaging) << "Timed out waiting for SIM message retrieval";
    retrievalTimer.stop();

    emit allMessagesReceived();
}

#endif //QTOPIA_NO_SMS
