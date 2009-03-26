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

#include "mailtransport.h"

#include <QtGlobal>

#ifndef QT_NO_OPENSSL
#include <QSslSocket>
#include <QSslError>
#else
#include <QTcpSocket>
#endif

#include <qtopialog.h>

#include "emailhandler.h"
#include <qtopiaapplication.h>

#ifndef QT_NO_OPENSSL
static QString sslCertsPath()
{
    static QString certsPath(Qtopia::qtopiaDir() + "/etc/ssl/certs");
    static bool firstCall = true;
    if (firstCall) {
        if (!QFile::exists(certsPath))
            qWarning() << "Cannot find SSL certificates" << certsPath << __FILE__ << __LINE__;
        firstCall = false;
    }
    return certsPath;
}
#endif


MailTransport::MailTransport(const char* name)
    : mName(name),
      mConnected(false),
      mInUse(false)
{
#ifndef QT_NO_OPENSSL
    if (QSslSocket::defaultCaCertificates().isEmpty())
    {
        QSslSocket::addDefaultCaCertificates(sslCertsPath());
    }

    encryption = AccountConfiguration::Encrypt_NONE;
#endif
    mSocket = 0;
    mStream = 0;
    connectToHostTimeOut = new QTimer(this);
    connect( connectToHostTimeOut, SIGNAL(timeout()),
	     this, SLOT(hostConnectionTimeOut()) );
#ifdef QT_NO_OPENSSL
    createSocket(AccountConfiguration::Encrypt_NONE);
#endif
}

MailTransport::~MailTransport()
{
    delete connectToHostTimeOut;
    delete mStream;
    delete mSocket;
}

void MailTransport::createSocket(AccountConfiguration::EncryptType encryptType)
{
#ifndef QT_NO_OPENSSL
    if (mSocket)
    {
        // Note: socket recycling doesn't seem to work in SSL mode...
        if (mSocket->mode() == QSslSocket::UnencryptedMode &&
            (encryptType == AccountConfiguration::Encrypt_NONE || encryptType == AccountConfiguration::Encrypt_TLS))
        {
            // The socket already exists in the correct mode
            return;
        }
        else
        {
            // We need to create a new socket in the correct mode
            delete mStream;
            mSocket->deleteLater();
        }
    }

    mSocket = new QSslSocket(this);
    encryption = encryptType;
    connect(mSocket, SIGNAL(encrypted()),
            this, SLOT(encryptionEstablished()));
    connect(mSocket, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(connectionFailed(QList<QSslError>)));
#else
    Q_UNUSED(encryptType);

    mSocket = new QTcpSocket(this);
#endif

    const int bufferLimit = 65536; // Limit memory used when downloading
    mSocket->setReadBufferSize( bufferLimit );
    mSocket->setObjectName(QString(mName) + "-socket");
    connect(mSocket, SIGNAL(connected()),
            this, SLOT(connectionEstablished()));
    connect(mSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(mSocket, SIGNAL(readyRead()),
            this, SIGNAL(readyRead()));
    connect(mSocket, SIGNAL(bytesWritten(qint64)),
            this, SIGNAL(bytesWritten(qint64)));

    mStream = new QTextStream(mSocket);
}

void MailTransport::open(const AccountConfiguration& config)
{
    open(config.mailServer(), config.mailPort(), config.mailEncryption());
}

void MailTransport::open(const QString& url, int port, AccountConfiguration::EncryptType encryptionType)
{
    if (mSocket && mSocket->isOpen())
    {
        qWarning() << "Failed to open connection - already open!";
        return;
    }
    
    mInUse = true;

    const int timeOut = 3 * 60 * 1000; // 3 minutes
    connectToHostTimeOut->start( timeOut );
    createSocket(encryptionType);
    emit updateStatus(tr("DNS lookup"));

#ifndef QT_NO_OPENSSL
    qLog(Messaging) << "Opening connection - " << url << ':' << port << (encryptionType == AccountConfiguration::Encrypt_SSL ? " SSL" : (encryptionType == AccountConfiguration::Encrypt_TLS ? " TLS" : ""));
    if (mailEncryption() == AccountConfiguration::Encrypt_SSL)
        mSocket->connectToHostEncrypted(url, port);
    else
#endif
        mSocket->connectToHost(url, port);
}

#ifndef QT_NO_OPENSSL
void MailTransport::switchToEncrypted()
{
    if (mSocket->mode() == QSslSocket::UnencryptedMode)
        mSocket->startClientEncryption();
}
#endif

void MailTransport::close()
{
    connectToHostTimeOut->stop();

    while (mSocket->bytesToWrite()) {
        // Flush any pending write data before closing
        mSocket->flush();
    }

    mConnected = false;
    mInUse = false;
    mSocket->close();
}

bool MailTransport::connected() const
{
    return mConnected;
}

bool MailTransport::inUse() const
{
    return mInUse;
}

QTextStream& MailTransport::stream()
{
    return *mStream;
}

bool MailTransport::canReadLine() const
{
    return mSocket->canReadLine();
}

QByteArray MailTransport::readLine(qint64 maxSize)
{
    return mSocket->readLine(maxSize);
}

void MailTransport::connectionEstablished()
{
    connectToHostTimeOut->stop();
    if (mailEncryption() == AccountConfiguration::Encrypt_NONE) {
        mConnected = true;
        emit updateStatus(tr("Connected"));
    }

    qLog(Messaging) << mName << ": connection established";
    emit connected(AccountConfiguration::Encrypt_NONE);
}

void MailTransport::hostConnectionTimeOut()
{
    connectToHostTimeOut->stop();
    errorHandling(QAbstractSocket::SocketTimeoutError, tr("Connection timed out"));
}

#ifndef QT_NO_OPENSSL
void MailTransport::encryptionEstablished()
{
    if (mailEncryption() != AccountConfiguration::Encrypt_NONE) {
        mConnected = true;
        emit updateStatus(tr("Connected"));
    }

    qLog(Messaging) << mName << ": Secure connection established";
    emit connected(mailEncryption());
}

void MailTransport::connectionFailed(const QList<QSslError>& errors)
{
    if (ignoreCertificateErrors(errors))
        mSocket->ignoreSslErrors();
    else
        errorHandling(QAbstractSocket::UnknownSocketError, "");
}

bool MailTransport::ignoreCertificateErrors(const QList<QSslError>& errors)
{
    bool failed = false;

    QString text;
    foreach (const QSslError& error, errors)
    {
        text += (text.isEmpty() ? "'" : ", '");
        text += error.errorString();
        text += "'";

        if (error.error() == QSslError::NoSslSupport)
            failed = true;
    }

    qWarning() << "Encrypted connect" << (failed ? "failed:" : "warnings:") << text;
    return false;
}
#endif

void MailTransport::errorHandling(int status, QString msg)
{
    connectToHostTimeOut->stop();
    mConnected = false;
    mInUse = false;
    mSocket->abort();
    emit updateStatus(tr("Error occurred"));
    emit errorOccurred(status, msg);
}

void MailTransport::socketError(QAbstractSocket::SocketError status)
{
    qWarning() << "socketError:" << static_cast<int>(status) << ':' << mSocket->errorString();
    errorHandling(static_cast<int>(status), tr("Socket error"));
}

AccountConfiguration::EncryptType MailTransport::mailEncryption() const
{
#ifndef QT_NO_OPENSSL
    return encryption;
#else
    return AccountConfiguration::Encrypt_NONE;
#endif
}

