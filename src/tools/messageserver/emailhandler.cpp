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


#include "emailhandler.h"
#ifndef QTOPIA_NO_MMS
#include "mmsclient.h"
#endif
#include <private/accountconfiguration_p.h>
#include <private/longstream_p.h>

#include <QFileInfo>
#include <QAbstractSocket>
#include <QApplication>
#include <QDSActionRequest>
#include <QMessageBox>
#include <QPair>
#include <QString>
#include <QString>
#include <QMailAddress>
#include <QMailFolder>
#include <QMailMessageServer>
#include <QMailAccountListModel>
#include <QtopiaIpcEnvelope>
#include <qtopialog.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

// Uncomment this definition to report signals emitted by messaging client connections:
//#define DEBUG_CLIENT_SIGNALS

#ifdef DEBUG_CLIENT_SIGNALS
#include <QSignalIntercepter>

class ClientSignalReporter : public QSignalIntercepter
{
    Q_OBJECT

    void activated(const QList<QVariant> &args) 
    { 
        int c = args.count();
        if (c > 1)
            qLog(Messaging) << "signal:" << static_cast<Client*>(sender()) << "-" << signal() << '(' << args[0] << ',' << args[1] << ')';
        else if (c > 0)
            qLog(Messaging) << "signal:" << static_cast<Client*>(sender()) << "-" << signal() << '(' << args[0] << ')';
        else
            qLog(Messaging) << "signal:" << static_cast<Client*>(sender()) << "-" << signal() << "()";
    }

public:
    ClientSignalReporter(Client *client, const QByteArray &signal) 
        : QSignalIntercepter(client, signal, client) {}
};

#define CLIENT_CONNECT(a,b,c,d) (void)new ClientSignalReporter((a),(b)); connect((a),(b),(c),(d))
#else
#define CLIENT_CONNECT(a,b,c,d) connect(a,b,c,d)
#endif


static EmailHandler::ErrorMap socketErrorInit()
{
    static const EmailHandler::ErrorEntry map[] = 
    {
        { QAbstractSocket::ConnectionRefusedError, QT_TRANSLATE_NOOP( "EmailHandler",  "Connection refused" ) },
        { QAbstractSocket::RemoteHostClosedError, QT_TRANSLATE_NOOP( "EmailHandler",  "Remote host closed the connection" ) },
        { QAbstractSocket::HostNotFoundError, QT_TRANSLATE_NOOP( "EmailHandler",  "Host not found" ) },
        { QAbstractSocket::SocketAccessError, QT_TRANSLATE_NOOP( "EmailHandler",  "Permission denied" ) },
        { QAbstractSocket::SocketResourceError, QT_TRANSLATE_NOOP( "EmailHandler",  "Insufficient resources" ) },
        { QAbstractSocket::SocketTimeoutError, QT_TRANSLATE_NOOP( "EmailHandler",  "Operation timed out" ) },
        { QAbstractSocket::DatagramTooLargeError, QT_TRANSLATE_NOOP( "EmailHandler",  "Datagram too large" ) },
        { QAbstractSocket::NetworkError, QT_TRANSLATE_NOOP( "EmailHandler",  "Network error" ) },
        { QAbstractSocket::AddressInUseError, QT_TRANSLATE_NOOP( "EmailHandler",  "Address in use" ) },
        { QAbstractSocket::SocketAddressNotAvailableError, QT_TRANSLATE_NOOP( "EmailHandler",  "Address not available" ) },
        { QAbstractSocket::UnsupportedSocketOperationError, QT_TRANSLATE_NOOP( "EmailHandler",  "Unsupported operation" ) },
        { QAbstractSocket::UnknownSocketError, QT_TRANSLATE_NOOP( "EmailHandler",  "Unknown error" ) },
    };

    return qMakePair( static_cast<const EmailHandler::ErrorEntry*>(map), ARRAY_SIZE(map) );
}

static EmailHandler::ErrorMap mailErrorInit()
{
    static const EmailHandler::ErrorEntry map[] = 
    {
        { QMailMessageServer::ErrUnknownResponse, "" },
        { QMailMessageServer::ErrLoginFailed, QT_TRANSLATE_NOOP( "EmailHandler", "Login failed. Check user name and password") },
        { QMailMessageServer::ErrCancel, QT_TRANSLATE_NOOP( "EmailHandler", "Operation cancelled.") },
        { QMailMessageServer::ErrFileSystemFull, QT_TRANSLATE_NOOP( "EmailHandler", "Mail check failed.") },
        { QMailMessageServer::ErrNonexistentMessage, QT_TRANSLATE_NOOP( "EmailHandler", "Message deleted from server.") },
        { QMailMessageServer::ErrEnqueueFailed, QT_TRANSLATE_NOOP( "EmailHandler", "Unable to queue message for transmission.") },
        { QMailMessageServer::ErrNoConnection, QT_TRANSLATE_NOOP( "EmailHandler", "Cannot determine the connection to transmit message on.") },
        { QMailMessageServer::ErrConnectionInUse, QT_TRANSLATE_NOOP( "EmailHandler", "Outgoing connection already in use by another operation.") },
        { QMailMessageServer::ErrConnectionNotReady, QT_TRANSLATE_NOOP( "EmailHandler", "Outgoing connection is not ready to transmit message.") },
        { QMailMessageServer::ErrInvalidAddress, QT_TRANSLATE_NOOP( "EmailHandler", "Message recipient addresses are not correctly formatted.") },
        { QMailMessageServer::ErrConfiguration, QT_TRANSLATE_NOOP( "EmailHandler", "Unable to use account due to invalid configuration.") },
        { QMailMessageServer::ErrInvalidAddress, QT_TRANSLATE_NOOP( "EmailHandler", "Unable to transmit message with invalid address parameters.") },
    };

    return qMakePair( static_cast<const EmailHandler::ErrorEntry*>(map), ARRAY_SIZE(map) );
}


EmailHandler::EmailHandler(QObject* parent)
    : QObject(parent)
{
    LongStream::cleanupTempFiles();

    totalSendSize = 0;
    totalRetrievalSize = 0;
    mRetrievalInProgress = false;
    mCountEmailMessages = true;
    
    smtpClient = new SmtpClient(this);
    connectClient(smtpClient, Sending, QMailMessage::Email, SLOT(socketError(int,QString&)));

#ifndef QTOPIA_NO_SMS
    smsClient = new SmsClient(this);
    connectClient(smsClient, Sending|Receiving|AsyncArrival, QMailMessage::Sms, SLOT(genericError(int,QString&)));
    connect(smsClient, SIGNAL(simReady(bool)), this, SIGNAL(simReady(bool)));
#endif

#ifndef QTOPIA_NO_MMS
    mmsClient = new MmsClient(this);
    connectClient(mmsClient, Sending|Receiving, QMailMessage::Mms, SLOT(genericError(int,QString&)));
#endif

#ifndef QTOPIA_NO_COLLECTIVE
    collectiveClient = new CollectiveClient(this);
    connectClient(collectiveClient, Sending|Receiving|AsyncArrival, QMailMessage::Instant, SLOT(mailError(int,QString&)));
#endif

    systemClient = new SystemClient(this);
    connectClient(systemClient, Receiving|AsyncArrival, QMailMessage::System, QString());

    readAccounts();

    connect(QMailStore::instance(),SIGNAL(accountsRemoved(QMailAccountIdList)),
            this, SLOT(readAccounts()));
    connect(QMailStore::instance(),SIGNAL(messageRemovalRecordsAdded(QMailAccountIdList)),
            this, SLOT(deletedMessages(QMailAccountIdList)));
}

EmailHandler::~EmailHandler()
{
    while (!connectionList.isEmpty())
        delete connectionList.takeFirst().first;
}

void EmailHandler::readAccounts()
{
    // Find the existing set of accounts
    QSet<QString> existing;
    foreach (const QMailAccountId &id, QMailStore::instance()->queryAccounts()) {
        QMailAccount account(id);
        foreach (const QString &source, account.messageSources())
            existing.insert(source.toLower());
    }

    // See if we need to create any missing account types
    // Note: the order of account creation will yield default account list model ordering
    QList<QMailAccount> newAccounts;
#ifndef QTOPIA_NO_SMS
    if (!existing.contains("sms")) {
        qLog(Messaging) << "Creating SMS account";
        QMailAccount smsAccount = QMailAccount::accountFromSource("sms");
        smsAccount.setAccountName(tr("SMS"));
        newAccounts.append(smsAccount);
    }
#endif
#ifndef QTOPIA_NO_MMS
    if (!existing.contains("mms")) {
        qLog(Messaging) << "Creating MMS account";
        QMailAccount mmsAccount = QMailAccount::accountFromSource("mms");
        mmsAccount.setAccountName(tr("MMS"));
        newAccounts.append(mmsAccount);
    }
#endif
#ifndef QTOPIA_NO_COLLECTIVE
    if (!existing.contains("jabber")) {
        qLog(Messaging) << "Creating Collective account";
        QMailAccount collectiveAccount = QMailAccount::accountFromSource("jabber");
        collectiveAccount.setAccountName(tr("Collective"));
        newAccounts.append(collectiveAccount);
    }
#endif
    if (!existing.contains("qtopia-system")) {
        qLog(Messaging) << "Creating System account";
        QMailAccount systemAccount = QMailAccount::accountFromSource("qtopia-system");
        systemAccount.setAccountName(tr("System"));
        newAccounts.append(systemAccount);
    }

    if (!newAccounts.isEmpty()) {
        // Create the missing accounts
        for (QList<QMailAccount>::iterator it = newAccounts.begin(), end = newAccounts.end(); it != end; ++it)
            QMailStore::instance()->addAccount(&(*it));
    }

    // All necessary accounts now exist - set the client associations
    foreach (const QMailAccountId &id, QMailStore::instance()->queryAccounts()) {
        QMailAccount account(id);

#ifndef QTOPIA_NO_SMS
        if (account.messageType() == QMailMessage::Sms) {
            setClientAccount(smsClient, account.id());
        }
#endif
#ifndef QTOPIA_NO_MMS
        if (account.messageType() == QMailMessage::Mms)  {
            setClientAccount(mmsClient, account.id());
        }
#endif
#ifndef QTOPIA_NO_COLLECTIVE
        if (account.messageType() == QMailMessage::Instant)  {
            setClientAccount(collectiveClient, account.id());
        }
#endif
    }
}

void EmailHandler::synchroniseClients()
{
#ifndef QTOPIA_NO_SMS
    unsynchronised.insert(smsClient);
#endif
#ifndef QTOPIA_NO_MMS
    unsynchronised.insert(mmsClient);
#endif
    
    foreach (const ConnectionDetails& connection, connections())
        if ((qobject_cast<ImapClient*>(connection.first))
            || (qobject_cast<PopClient*>(connection.first)))
            unsynchronised.insert(connection.first);
    
    unsynchronised.insert(systemClient);

#ifndef QTOPIA_NO_SMS
    smsClient->checkForNewMessages();
#endif
#ifndef QTOPIA_NO_MMS
    mmsClient->checkForNewMessages();
#endif
    foreach (const ConnectionDetails& connection, connections())
        if ((qobject_cast<ImapClient*>(connection.first))
            || (qobject_cast<PopClient*>(connection.first)))
            connection.first->checkForNewMessages();
    systemClient->checkForNewMessages();
}

void EmailHandler::connectClient(Client *client, int type, QMailMessage::MessageType messageType, QString slotName)
{
    CLIENT_CONNECT(client, SIGNAL(errorOccurred(int,QString&)), 
                   this, SLOT(clientError(int,QString&)));
    if (!slotName.isEmpty()) {
        connect(client, SIGNAL(errorOccurred(int,QString&)), slotName.toAscii());
    }

    CLIENT_CONNECT(client, SIGNAL(updateStatus(QMailMessageServer::Operation,QString)), 
                   this, SLOT(clientStatus(QMailMessageServer::Operation,QString)));

    if (type & Receiving)
    {
        CLIENT_CONNECT(client, SIGNAL(allMessagesReceived()), 
                       this, SLOT(messagesReceived()));
        if (type & AsyncDeletion) {
            CLIENT_CONNECT(client, SIGNAL(nonexistentMessage(QString,Client::DefunctReason)), 
                           this, SLOT(nonexistentMessage(QString,Client::DefunctReason)));
        }
        if (type & SyncRetrieval) {
            CLIENT_CONNECT(client, SIGNAL(fetchTotal(uint)), 
                           this, SLOT(fetchTotal(uint)));
            CLIENT_CONNECT(client, SIGNAL(fetchProgress(uint)), 
                           this, SLOT(fetchProgress(uint)));
            CLIENT_CONNECT(client, SIGNAL(retrievalProgress(QString,uint)), 
                           this, SLOT(retrievalProgress(QString,uint)));
            CLIENT_CONNECT(client, SIGNAL(messageProcessed(QString)), 
                           this, SLOT(messageProcessed(QString)));
        }

        CLIENT_CONNECT(client, SIGNAL(newMessage(QMailMessage&,bool)), 
                       this, SLOT(newClientMessage(QMailMessage&,bool)));
        CLIENT_CONNECT(client, SIGNAL(newMailDiscovered(QMailAccountId)), 
                       this, SIGNAL(newMailDiscovered(QMailAccountId)));
    }
    if (type & Sending)
    {
        CLIENT_CONNECT(client, SIGNAL(messageTransmitted(QMailMessageId)), 
                       this, SIGNAL(messageSent(QMailMessageId)));
        CLIENT_CONNECT(client, SIGNAL(sendProgress(QMailMessageId,uint)), 
                       this, SLOT(sendProgress(QMailMessageId,uint)));
        CLIENT_CONNECT(client, SIGNAL(messageProcessed(QMailMessageId)), 
                       this, SLOT(messageProcessed(QMailMessageId)));
    }

    CLIENT_CONNECT(client, SIGNAL(partialRetrievalCompleted()), 
                   this, SLOT(partialRetrievalComplete()));
    CLIENT_CONNECT(client, SIGNAL(retrievalCompleted()), 
                   this, SLOT(retrievalComplete()));

    clientMessageTypes[client] = messageType;
}

void EmailHandler::send(const QMailMessageIdList& mailList)
{
    int errorCode = QMailMessageServer::ErrNoError;
    QMailAccountId errorAccountId;

    uint smtpCount = 0;
    uint smsCount = 0;
    uint mmsCount = 0;
    uint collectiveCount = 0;

    // We shouldn't have anything left in our send list...
    if (!sendSize.isEmpty()) {
        foreach (const QMailMessageId& id, sendSize.keys())
            qLog(Messaging) << "Message" << id << "still in send map...";

        sendSize.clear();
        clientForOutgoing.clear();
    }

    foreach (const QMailMessageId& messageId, mailList) {
        if (errorCode != QMailMessageServer::ErrNoError)
            break;

        // Get the complete message
        QMailMessage message(messageId);
        QMailAccountId accountId(message.parentAccountId());

#ifndef QTOPIA_NO_COLLECTIVE
        if (message.messageType() == QMailMessage::Instant) {
            QMailAccount account(accountId);
            if (account.messageSinks().contains("jabber", Qt::CaseInsensitive)) {
                if ((errorCode = collectiveClient->addMail(message)) == QMailMessageServer::ErrNoError) {
                    if (++collectiveCount == 1)
                        setClientAccount(collectiveClient, accountId);

                    sendSize.insert(messageId, message.indicativeSize());
                    clientForOutgoing[messageId] = collectiveClient;
                } else {
                    errorAccountId = accountId;
                }
            } else {
                qLog(Messaging) << "Unable to send using unknown IM account sinks:" << account.messageSinks();
            }
            continue;
        } 
#endif

        int mailRecipients = 0;
        int phoneRecipients = 0;

        foreach (const QMailAddress& address, message.recipients()){
            if (address.isEmailAddress())
                ++mailRecipients;
            else if (address.isPhoneNumber())
                ++phoneRecipients;
        }

#ifndef QTOPIA_NO_MMS
        //mms message
        if ((message.messageType() == QMailMessage::Mms) && ((mailRecipients + phoneRecipients) > 0)) {
            if ((errorCode = mmsClient->addMail(message)) == QMailMessageServer::ErrNoError) {
                if (++mmsCount == 1)
                    setClientAccount(mmsClient, accountId);

                sendSize.insert(messageId, message.indicativeSize());
                clientForOutgoing[messageId] = mmsClient;
            } else {
                errorAccountId = accountId;
            }
            continue;
        }
#endif

#ifndef QTOPIA_NO_SMS
        // sms message
        if (message.messageType() == QMailMessage::Sms && phoneRecipients > 0) {
            if ((errorCode = smsClient->addMail(message)) == QMailMessageServer::ErrNoError) {
                if (++smsCount == 1)
                    setClientAccount(smsClient, accountId);

                sendSize.insert(messageId, message.indicativeSize());
                clientForOutgoing[messageId] = smsClient;
            } else {
                errorAccountId = accountId;
            }
            // fall through to send to mail recipients
        }
#endif

        if (mailRecipients == 0)
            continue;

        if (errorCode != QMailMessageServer::ErrNoError)
            break;

        if (smtpCount == 0) {
            // We need the account already set to addMail correctly for SMTP
            setClientAccount(smtpClient, accountId);
        } else {
            // Ensure we are using the same account
            if (accountId != smtpClient->account()) {
                qLog(Messaging) << "Unable to send message on account" << accountId << "; already using SMTP account:" << smtpClient->account();
                break;
            }
        }

        if ((errorCode = smtpClient->addMail(message)) == QMailMessageServer::ErrNoError) {
            ++smtpCount;
            sendSize.insert(messageId, message.indicativeSize());
            clientForOutgoing[messageId] = smtpClient;
        } else {
            errorAccountId = accountId;
        }
    }

    if (errorCode == QMailMessageServer::ErrNoError) {
        uint totalQueued = (smtpCount + smsCount + mmsCount + collectiveCount);
        if (totalQueued != static_cast<uint>(mailList.count())) {
            errorCode = QMailMessageServer::ErrNoConnection;
        }
    }

    if (errorCode == QMailMessageServer::ErrNoError) {
        // Note: We currently assume that no more than 1 account of any one type
        // will participate in a single send operation...
        if (smtpCount) {
            smtpClient->newConnection();
        }
#ifndef QTOPIA_NO_SMS
        if (smsCount) {
            smsClient->newConnection();
        } else {
            smsClient->clearList();
        }
#endif
#ifndef QTOPIA_NO_MMS
        if (mmsCount) {
            mmsClient->newConnection();
        }
#endif
#ifndef QTOPIA_NO_COLLECTIVE
        if (collectiveCount) {
            collectiveClient->newConnection();
        }
#endif

        // Calculate the total indicative size of the messages we're sending
        uint totalSize = 0;
        foreach (uint size, sendSize.values())
            totalSize += size;

        totalSendSize = 0;
        emit sendTotal(totalSize);
    } else {
        sendSize.clear();
        clientForOutgoing.clear();

        static ErrorMap mailErrorMap(mailErrorInit());

        QString temp;
        if (appendErrorText(temp, errorCode, mailErrorMap)) {
            errorCode = QMailMessageServer::ErrEnqueueFailed;
        }

        reportFailure(errorAccountId, temp, errorCode);
    }
}

void EmailHandler::sendProgress(const QMailMessageId& id, uint percentage)
{
    SendMap::const_iterator it = sendSize.find(id);
    if (it != sendSize.end()) {
        if (percentage > 100)
            percentage = 100;

        // Update the progress figure to count the sent portion of this message
        uint partialSize = (*it) * percentage / 100;
        emit sendProgress(totalSendSize + partialSize);
    }
}

void EmailHandler::retrievalProgress(const QString& uid, uint bytesReceived)
{
    RetrievalMap::iterator it = retrievalSize.find(uid);
    if (it != retrievalSize.end()) {
        QPair< QPair<uint, uint>, uint> &values = it.value();

        // Calculate the percentage of the retrieval completed
        uint totalBytes = values.first.second;
        uint percentage = totalBytes ? qMin<uint>(bytesReceived * 100 / totalBytes, 100) : 100;

        if (percentage > values.second) {
            values.second = percentage;

            // Update the progress figure to count the sent portion of this message
            uint partialSize = values.first.first * percentage / 100;
            emit retrievalProgress(totalRetrievalSize + partialSize);
        }
    }
}

void EmailHandler::fetchTotal(uint total)
{
    emit retrievalTotal(total);
}

void EmailHandler::fetchProgress(uint progress)
{
    emit retrievalProgress(progress);
}

void EmailHandler::messageProcessed(const QMailMessageId& id)
{
    SendMap::iterator it = sendSize.find(id);
    if (it != sendSize.end()) {
        // Update the progress figure
        totalSendSize += *it;
        emit sendProgress(totalSendSize);

        sendSize.erase(it);
        clientForOutgoing.remove(id);

        if (sendSize.isEmpty()) {
            // We have attempted to send all the supplied messages
            emit sendCompleted();
        }
    }
}

void EmailHandler::messageProcessed(const QString& uid)
{
    RetrievalMap::iterator it = retrievalSize.find(uid);
    if (it != retrievalSize.end()) {
        // Update the progress figure
        totalRetrievalSize += it.value().first.first;
        emit retrievalProgress(totalRetrievalSize);

        retrievalSize.erase(it);
    }
}

EmailHandler::ConnectionDetails* EmailHandler::connectionForClient(Client* client)
{
    QList<ConnectionDetails>::iterator it = connectionList.begin(), end = connectionList.end();
    for ( ; it != end; ++it)
        if ((*it).first == client)
            return &(*it);

    return 0;
}

void EmailHandler::setClientAccount(Client* client, const QMailAccountId &accountId)
{
    if (ConnectionDetails* details = connectionForClient(client)) {
        details->second = accountId;
    } else {
        connectionList.append(qMakePair(client, accountId));
    }

    client->setAccount(accountId);
}

Client* EmailHandler::clientForType(QMailMessage::MessageType type) const
{
#ifndef QTOPIA_NO_SMS
    if (type == QMailMessage::Sms) return smsClient;
#endif
#ifndef QTOPIA_NO_MMS
    if (type == QMailMessage::Mms) return mmsClient;
#endif
#ifndef QTOPIA_NO_COLLECTIVE
    if (type == QMailMessage::Instant) return collectiveClient;
#endif

    return 0;
}

void EmailHandler::setMailAccount(const QMailAccountId &accountId)
{
    mailAccountId = accountId;

    QMailAccount account(mailAccountId);
    if (account.messageType() == QMailMessage::Email) {
        Client *client = clientForAccount(mailAccountId, true);
        if (!client) {
            if (account.messageSources().contains("imap4", Qt::CaseInsensitive)) {
                client = new ImapClient(this);
            } else if (account.messageSources().contains("pop3", Qt::CaseInsensitive)) {
                client = new PopClient(this);
            } else {
                return;
            }
            
            connectClient(client, Receiving|SyncRetrieval|AsyncDeletion, QMailMessage::Email, SLOT(mailError(int,QString&)));
        }
        setClientAccount(client, mailAccountId);
    } else {
        setClientAccount(clientForType(account.messageType()), mailAccountId);
    }
}

bool EmailHandler::retrievalInProgress()
{
    return mRetrievalInProgress;
}

void EmailHandler::retrieve(const QMailAccountId& accountId, bool foldersOnly, bool countEmailMessages)
{
    if (mRetrievalInProgress) {
        qLog(Messaging) << "Unable to initiate retrieval while operation is in progress";
        return;
    }

    mCountEmailMessages = countEmailMessages;
    setMailAccount(accountId);

    Client *client = clientForAccount(accountId, true);
    if (client) {
        client->foldersOnly(foldersOnly);
        client->headersOnly(true, 2000);        //less than 2000, download all

        mRetrievalInProgress = true;
        client->newConnection();
    } else {
        qLog(Messaging) << "Unable to retrieve messages for unknown account:" << accountId;
    }
}

void EmailHandler::partialRetrievalComplete()
{
    emit partialRetrievalCompleted();
}

void EmailHandler::retrievalComplete()
{
    mRetrievalInProgress = false;
    emit retrievalCompleted();
}

void EmailHandler::clientError(int, QString&)
{
    mRetrievalInProgress = false;
}

void EmailHandler::completeRetrieval(const QMailMessageIdList& mailList)
{
    // We shouldn't have anything left in our retrieval list...
    if (!retrievalSize.isEmpty()) {
        foreach (const QString& uid, retrievalSize.keys())
            qLog(Messaging) << "Message" << uid << "still in retrieve map...";

        retrievalSize.clear();
    }

    if (mailList.isEmpty()) {
        mRetrievalInProgress = false;

        // There are no message bodies to be retrieved
        if (QMailAccount(mailAccountId).messageType() == QMailMessage::Email) {
            // The mail client will not have closed its connection - do that now
            if (Client *client = clientForAccount(mailAccountId, true))
                client->closeConnection();
        }
        return;
    }

    QMailMessageMetaData first(mailList.first());
    QMailAccountId accountId = first.parentAccountId();

    setMailAccount(accountId);

    if (Client *client = clientForAccount(mailAccountId, true)) {
        mRetrievalInProgress = true;
        client->newConnection();

        uint totalSize = 0;
        uint totalBytes = 0;

        SelectionMap selectionMap;

        QStringList uidList;
        foreach (const QMailMessageId& id, mailList) {
            QMailMessageMetaData message(id);
            if (message.parentAccountId() == accountId) {
                // Find the indicative size of these messages
                QString uid = message.serverUid();
                uint size = message.indicativeSize();
                uint bytes = message.size();

                // Also record the bytes to be received for this message

                selectionMap[message.parentFolderId()].insert(uid,id);

                retrievalSize.insert(uid, qMakePair(qMakePair(size, bytes), 0u));
                totalSize += size;
                totalBytes += bytes;
            } else {
                qLog(Messaging) << "Ignoring retrieval for non-matching account:" << message.fromAccount();
            }
        }

        // Ensure that we have space to retrieve these messages into
        // Conservative estimate: we need twice the reported size plus 10 KB
        if (!LongStream::freeSpace( "", totalBytes * 2 + 1024*10 )) {
            mRetrievalInProgress = false;
            retrievalSize.clear();
            reportFailure(mailAccountId, QString(), QMailMessageServer::ErrFileSystemFull);
        } else {
            // Emit the total size we will receive
            totalRetrievalSize = 0;
            emit retrievalTotal(totalSize);

            client->foldersOnly(false);
            client->headersOnly(false, 0);
            client->setSelectedMails(selectionMap);
        }
    } else {
        qLog(Messaging) << "Unable to complete message retrieval for unknown account:" << accountId;
    }
}

void EmailHandler::cancelTransfer()
{
    foreach (const ConnectionDetails& connection, connections())
        connection.first->cancelTransfer();
}

const QList<EmailHandler::ConnectionDetails>& EmailHandler::connections() const
{
    return connectionList;
}

Client* EmailHandler::clientForAccount(const QMailAccountId &accountId, bool retrieving)
{
    foreach (ConnectionDetails connection, connections())
        // Ensure we don't return the SMTP client for an account when retrieving
        if ((accountId == connection.second) && (retrieving == (connection.first != smtpClient)))
            return connection.first;

    return 0;
}

QMailAccountId EmailHandler::accountForClient(const Client *client) const
{
    foreach (ConnectionDetails connection, connections())
        if (connection.first == client) {
            if (connection.second != client->account())
                qWarning() << "Mismatched account ID for client:" << client;
            return connection.second;
        }

    qWarning() << "Unregistered client has account ID:" << client->account();
    return QMailAccountId();
}

void EmailHandler::newClientMessage(QMailMessage& message, bool partial)
{
    if (partial) {
        emit partialMessageRetrieved(message);
    } else {
        emit messageRetrieved(message);

        // Report that the this message has been processed
        messageProcessed(message.serverUid());
    }
}

void EmailHandler::nonexistentMessage(const QString& uid, Client::DefunctReason reason)
{
    if (const Client* client = static_cast<const Client*>(sender())) {
        QMailAccountId accountId = accountForClient(client);

        QMailMessageMetaData message(uid, accountId);
        if (message.id().isValid()) {
            if (reason == Client::FolderRemoved) {
                // No need to individually delete messages whose containing folder will be removed
            } else {
                if (!(message.status() & QMailMessage::Removed)) {
                    // Mark this message as deleted
                    message.setStatus(QMailMessage::Removed, true);
                    QMailStore::instance()->updateMessage(&message);
                }
            }
        } else {
            // We must have a deletion record for this message
            QMailStore::instance()->purgeMessageRemovalRecords(accountId, QStringList() << uid);
        }
    }

    messageProcessed(uid);
}

void EmailHandler::messagesReceived()
{
    if (Client* client = static_cast<Client*>(sender())) {
        clientSynchronised(client);
    }
}

void EmailHandler::clientSynchronised(const Client* client)
{
    if (unsynchronised.contains(client)) {
        unsynchronised.remove(client);

        if (unsynchronised.isEmpty()) {
            qLog(Messaging) << "Messaging clients synchronised";

            emit newCountDetermined();
        }
    } else {
        // Notify of this change, whether we're currently synchronising or not
        emit newCountChanged();
    }
}

void EmailHandler::mmsMessage(const QDSActionRequest& request)
{
#ifndef QTOPIA_NO_MMS
    unsynchronised.insert(mmsClient);

    mmsClient->pushMmsMessage(request);
#endif

    // Respond to the request
    QDSActionRequest(request).respond();
}

void EmailHandler::clientStatus(QMailMessageServer::Operation op, const QString& status)
{
    if (const Client* client = static_cast<const Client*>(sender())) {
        QMailAccountId accountId = accountForClient(client);
        emit statusChanged(op, QMailAccount(accountId).accountName(), status);
    }
}

QList<Client*> EmailHandler::clients() const
{
    return clientMessageTypes.keys();
}

void EmailHandler::acknowledgeNewMessages(const QMailMessageTypeList& types)
{
    QMap<Client*, QMailMessage::MessageType>::iterator it = clientMessageTypes.begin(), end = clientMessageTypes.end();
    for ( ; it != end; ++it) {
        if (types.contains(it.value())) {
            it.key()->resetNewMailCount();
        }
    }
}

void EmailHandler::deletedMessages(const QMailAccountIdList& ids)
{
    foreach (const QMailAccountId &accountId, ids) {
        // Delete from client storage, where appropriate
        if (Client *client = clientForAccount(accountId, true)) {
            if (client->hasDeleteImmediately()) {
                foreach (const QMailMessageRemovalRecord& removalRecord, QMailStore::instance()->messageRemovalRecords(accountId))
                    if (!removalRecord.serverUid().isEmpty())
                        client->deleteImmediately(removalRecord.serverUid());

                QMailStore::instance()->purgeMessageRemovalRecords(accountId);
            }
        }
    }
}

void EmailHandler::transmissionFailed(const Client* client)
{
    QMailMessageIdList failedMessages;

    // We have finished with all messages for this client
    QMap<QMailMessageId, Client*>::iterator it = clientForOutgoing.begin(), end = clientForOutgoing.end();
    for ( ; it != end; ++it)
        if (it.value() == client)
            failedMessages.append(it.key());
   
    foreach (const QMailMessageId& id, failedMessages)
        messageProcessed(id);
}

void EmailHandler::genericError(int code, QString& text)
{
    static ErrorMap errorMap(mailErrorInit());

    if (const Client* client = static_cast<const Client*>(sender())) {
        reportFailure(accountForClient(client), text, code, (ErrorSet() << errorMap));
        transmissionFailed(client);
    }
}

void EmailHandler::socketError(int code, QString& text)
{
    // Create a map of error text strings, once only
    static ErrorMap errorMap(socketErrorInit());

    if (const Client* client = static_cast<const Client*>(sender())) {
        reportFailure(accountForClient(client), text, code, (ErrorSet() << errorMap));
        transmissionFailed(client);
    }
}

void EmailHandler::mailError(int code, QString& text)
{
    // Create a map of error text strings, once only
    static ErrorMap mailErrorMap(mailErrorInit());
    static ErrorMap socketErrorMap(socketErrorInit());

    if (const Client* client = static_cast<const Client*>(sender())) {
        reportFailure(accountForClient(client), text, code, (ErrorSet() << mailErrorMap << socketErrorMap));
        transmissionFailed(client);
    }
}

void EmailHandler::reportFailure(const QMailAccountId &accountId, const QString& message, const int code, const ErrorSet& errorSet)
{
    QString temp(message);

    bool handledByErrorSet = appendErrorText(temp, code, errorSet);

    bool handledByHandler = true;
    if (code == QMailMessageServer::ErrFileSystemFull) {
        temp.append(' ').append(LongStream::errorMessage());
    } else if (code == QMailMessageServer::ErrEnqueueFailed) {
        temp.append("\n" + tr("Unable to send; message moved to Drafts folder"));
    } else if (code == QMailMessageServer::ErrUnknownResponse) {
        QString serverName;
        if (accountId.isValid()) {
            AccountConfiguration config(accountId);
            serverName = " " + config.mailServer();
        }
        temp.prepend(QString(tr("Unexpected response from server%1:", "%1: server address preceded by space") + "\n").arg(serverName));
    } else {
        handledByHandler = false;
    }

    if (!handledByErrorSet && !handledByHandler) {
        if (!temp.isEmpty())
            temp.append('\n');
        temp.append('<' + QString(tr("Error %1", "%1 contains numeric error code")).arg(code) + '>');
    }

    emit errorOccurred(accountId, temp, code);
}

bool EmailHandler::appendErrorText(QString& message, const int code, const ErrorMap& map)
{
    const ErrorEntry *it = map.first, *end = map.first + map.second; // ptr arithmetic!

    for ( ; it != end; ++it)
        if (it->code == code) {
            QString extra(tr(it->text));
            if (!extra.isEmpty()) {
                if (message.isEmpty()) {
                    message = extra;
                } else {
                    message.append("\n[").append(extra).append(']');
                }
            }
            return true;
        }

    return false;
}

bool EmailHandler::appendErrorText(QString& message, const int code, const ErrorSet& mapList)
{
    foreach (const ErrorMap& map, mapList)
        if (appendErrorText(message, code, map))
            return true;

    return false;
}

#ifdef DEBUG_CLIENT_SIGNALS
#include "emailhandler.moc"
#endif

