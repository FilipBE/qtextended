/***************************************************************************r
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

#include "messageserver.h"

#include "emailhandler.h"
#include "mailmessageclient.h"
#include "messagearrivalservice.h"

#include <private/accountconfiguration_p.h>

#include <QMailFolder>
#include <QMailMessage>
#include <QMailMessageMetaData>
#include <QMailStore>

#include <QDataStream>
#include <QDSAction>
#include <QDSActionRequest>
#include <QDSData>
#include <QMimeType>
#ifndef QTOPIA_NO_SMS
#include <QSMSMessage>
#endif
#include <qtopialog.h>
#include <QtopiaApplication>
#include <QtopiaIpcEnvelope>
#include <QtopiaServiceRequest>
#include <qnetworkstate.h>


static QMailFolderId initFolderId()
{
    static const QString folderName("MessageServer");

    QMailFolderKey key(QMailFolderKey::Name, folderName);
    key &= QMailFolderKey(QMailFolderKey::ParentId, QMailFolderId());

    QMailFolderIdList folderIdList = QMailStore::instance()->queryFolders(key);
    if (folderIdList.isEmpty()) {
        QMailFolder serverFolder(folderName);
        if (!QMailStore::instance()->addFolder(&serverFolder)) {
            qLog(Messaging) << "Unable to create folder for server message storage!";
            qApp->quit();
        }
        return serverFolder.id();
    } else {
        return folderIdList.first();
    }
}

static QMailFolderId serverFolderId()
{
    static const QMailFolderId folderId(initFolderId());
    return folderId;
}


MessageServer::MessageSearch::MessageSearch(const QMailMessageIdList &ids, const QString &text)
    : _ids(ids), 
      _text(text),
      _active(false),
      _total(ids.count()),
      _progress(0)
{
}

const QString &MessageServer::MessageSearch::bodyText() const
{
    return _text;
}

bool MessageServer::MessageSearch::pending() const
{
    return !_active;
}

void MessageServer::MessageSearch::inProgress()
{
    _active = true;
}

bool MessageServer::MessageSearch::isEmpty() const
{
    return _ids.isEmpty();
}

uint MessageServer::MessageSearch::total() const
{
    return _total;
}

uint MessageServer::MessageSearch::progress() const
{
    return _progress;
}

QMailMessageIdList MessageServer::MessageSearch::takeBatch()
{
    // TODO: this should be smarter
    static const int BatchSize = 10;

    QMailMessageIdList result;
    if (_ids.count() <= BatchSize) {
        result = _ids;
        _ids.clear();
    } else {
        result = _ids.mid(0, BatchSize);
        _ids = _ids.mid(BatchSize, -1);
    }

    _progress += result.count();
    return result;
}


MessageServer::MessageServer(QObject *parent)
    : QObject(parent),
      handler(new EmailHandler(this)),
      client(new MailMessageClient(this)),
      unacknowledgedFlashSms(0),
      intervalCheckRetrievalInProgress(false),
      roamingMode("/Telephony/Status/Roaming"),
      messageCountUpdate("QPE/Messages/MessageCountUpdated"),
      telephonyValueSpace("/Telephony/Status"),
      newMessageTotal(0)
{
    qLog(Messaging) << "MessageServer ctor begin";

    QtopiaApplication::loadTranslations("libqtopiamail");

    QMailMessageCountMap::iterator it = messageCounts.begin(), end = messageCounts.end();
    for ( ; it != end; ++it)
        it.value() = 0;

    QMailStore *store = QMailStore::instance();
    if (!store->initialized()) {
        // We can't do anything without the mail store...
        QtopiaServiceRequest req("SystemMessages", "showDialog(QString,QString)");
        req << tr("Messaging DB Invalid");
        req << tr("Messaging cannot operate due to database incompatibilty!");
        req.send();

        // Do not close, however, or QPE will start another instance.
    } else {
        // Ensure we have a folder to use
        QMailFolderId folderId(serverFolderId());

        connect(store, SIGNAL(accountsAdded(QMailAccountIdList)),
                this, SLOT(initIntervalChecking()));
        connect(store, SIGNAL(accountsRemoved(QMailAccountIdList)),
                this, SLOT(initIntervalChecking()));
        connect(store, SIGNAL(accountsUpdated(QMailAccountIdList)),
                this, SLOT(initIntervalChecking()));

        connect(store, SIGNAL(messagesUpdated(QMailMessageIdList)),
                this, SLOT(messagesUpdated(QMailMessageIdList)));
        connect(store, SIGNAL(messagesRemoved(QMailMessageIdList)),
                this, SLOT(messagesRemoved(QMailMessageIdList)));
        
        // Propagate email handler signals to the client
        connect(handler, SIGNAL(statusChanged(QMailMessageServer::Operation,QString,QString)),
                this, SLOT(statusChanged(QMailMessageServer::Operation,QString,QString)));
        connect(handler, SIGNAL(retrievalTotal(uint)),
                this, SLOT(retrievalTotal(uint)));
        connect(handler, SIGNAL(retrievalProgress(uint)),
                this, SLOT(retrievalProgress(uint)));
        connect(handler, SIGNAL(sendTotal(uint)),
                client, SIGNAL(sendTotal(uint)));
        connect(handler, SIGNAL(sendProgress(uint)),
                client, SIGNAL(sendProgress(uint)));
        connect(handler, SIGNAL(sendCompleted()), 
                this, SLOT(sendCompleted()));
        connect(handler, SIGNAL(messageSent(QMailMessageId)), 
                this, SLOT(messageSent(QMailMessageId)));
        connect(handler, SIGNAL(errorOccurred(QMailAccountId,QString,int)),
                this, SLOT(errorOccurred(QMailAccountId,QString,int)));
        connect(handler, SIGNAL(newCountChanged()),
                this, SLOT(reportNewCounts()));
        connect(handler, SIGNAL(newCountDetermined()),
                this, SLOT(reportNewCounts()));
        connect(handler, SIGNAL(partialMessageRetrieved(QMailMessageMetaData&)),
                this, SLOT(partialMessageRetrieved(QMailMessageMetaData&)));
        connect(handler, SIGNAL(messageRetrieved(QMailMessage&)),
                this, SLOT(messageRetrieved(QMailMessage&)));
        connect(handler, SIGNAL(partialRetrievalCompleted()),
                this, SLOT(partialRetrievalCompleted()));
        connect(handler, SIGNAL(retrievalCompleted()),
                this, SLOT(retrievalCompleted()));
        connect(handler, SIGNAL(newMailDiscovered(QMailAccountId)),
                this, SLOT(newMailDiscovered(QMailAccountId)));
        connect(handler, SIGNAL(simReady(bool)),
                this, SLOT(simReady(bool)));

        // The email handler should handle the email client signals
        connect(client, SIGNAL(send(QMailMessageIdList)),
                handler, SLOT(send(QMailMessageIdList)));
        connect(client, SIGNAL(retrieve(QMailAccountId, bool)),
                handler, SLOT(retrieve(QMailAccountId, bool)));
        connect(client, SIGNAL(completeRetrieval(QMailMessageIdList)),
                handler, SLOT(completeRetrieval(QMailMessageIdList)));
        connect(client, SIGNAL(cancelTransfer()),
                handler, SLOT(cancelTransfer()));
        connect(client, SIGNAL(cancelSearch()),
                this, SLOT(cancelSearch()));
        connect(client, SIGNAL(acknowledgeNewMessages(QMailMessageTypeList)),
                this, SLOT(acknowledgeNewMessages(QMailMessageTypeList)));
        connect(client, SIGNAL(searchMessages(QMailMessageKey, QString)),
                this, SLOT(searchMessages(QMailMessageKey, QString)));

#ifndef QTOPIA_NO_SMS
#ifndef QTOPIA_NO_MMS
        // Hook up the QCop service handlers
        QtopiaAbstractService* svc = new MessageArrivalService( this );
        connect(svc, SIGNAL(mmsMessage(QDSActionRequest)), 
                this, SLOT(mmsMessage(QDSActionRequest)));
        connect(svc, SIGNAL(smsFlash(QDSActionRequest)), 
                this, SLOT(smsFlash(QDSActionRequest)));
#endif
#endif

        netState = new QNetworkState(this);
        connect(netState, SIGNAL(connected()),
                this, SLOT(initIntervalChecking()));

        QtopiaIpcAdaptor::connect(this, SIGNAL(messageCountUpdated()),
                                  &messageCountUpdate, MESSAGE(changeValue()));

        initIntervalChecking();
    }
}

MessageServer::~MessageServer()
{
    delete netState;
    foreach(QTimer *timer, intervalCheckMap.keys())
        delete timer;
}

void MessageServer::initIntervalChecking()
{
    QMailAccountIdList accountIds = QMailStore::instance()->queryAccounts();
    bool pushAccountsExist = false;

    //Remove deleted accounts
    foreach(QTimer *timer, intervalCheckMap.keys())
        if (!accountIds.contains(intervalCheckMap[timer])) {
            intervalCheckMap.remove(timer);
            delete timer;
        }

    //Insert/update new/modified accounts
    foreach(const QMailAccountId &id, accountIds) {
        AccountConfiguration config(id);
        int checkInterval = config.checkInterval()*1000*60;

        QTimer *timer = intervalCheckMap.key(id);
        if (config.pushEnabled()) {
            if (!intervalAccountIds.contains(id)) {
                intervalAccountIds.insert(id);
                pushAccountsExist = true;
            }
        }
        if (checkInterval <= 0) {
            if (timer)
                intervalCheckMap.remove(timer);
            continue;
        }
        if (!timer) {
            timer = new QTimer;
            connect(timer, SIGNAL(timeout()), this, SLOT(intervalCheckTimeout()));
            intervalCheckMap.insert(timer, config.id());
            timer->start(checkInterval);
        } else if (timer->interval() != checkInterval) {
            timer->setInterval(checkInterval);
            timer->start();
        }
    }
    if (pushAccountsExist) //kick off mail checking for push accounts
        QTimer::singleShot(0, this, SLOT(intervalCheckTimeout()));
}

void MessageServer::newMailDiscovered(const QMailAccountId &accountId)
{
    if (!intervalAccountIds.contains(accountId))
        intervalAccountIds.insert(accountId);
    intervalCheckTimeout();
}

void MessageServer::intervalCheckTimeout()
{
    if (sender()) {
        if (QTimer *timer = qobject_cast<QTimer*>(sender())) {
            QMailAccountId id = intervalCheckMap[timer];
            if (id.isValid()) {
                if (!intervalAccountIds.contains(id)) {
                    intervalAccountIds.insert(id);
                } else {
                    // Something is wrong, if we haven't completed our previous check!
                    qLog(Messaging) << "Account" << id << "already waiting for interval check!";
                }
            }
        }
    }

    if (!intervalAccountIds.isEmpty()) {
        if (!handler->retrievalInProgress()) {
            QMailAccountId accountId = *intervalAccountIds.begin();
            intervalAccountIds.remove(accountId);

            AccountConfiguration config(accountId);
            if (roamingMode.value().toBool() && !config.intervalCheckRoamingEnabled())
                return;

            connect(handler, SIGNAL(errorOccurred(QMailAccountId,QString,int)),
                    this, SLOT(intervalCheckingErrorOccurred(QMailAccountId,QString,int)));
            connect(handler, SIGNAL(retrievalCompleted()),
                    this, SLOT(intervalCheckingRetrievalComplete()));

            qLog(Messaging) << "Performing interval check for account " 
                            << accountId 
                            << " name "
                            << QMailAccount(accountId).displayName();
            intervalCheckRetrievalInProgress = true;
            handler->retrieve(accountId, false, true);
        }
    }
}

void MessageServer::intervalCheckingErrorOccurred(const QMailAccountId &id, const QString &txt, int code)
{
    disconnect(handler, SIGNAL(errorOccurred(QMailAccountId,QString,int)),
            this, SLOT(intervalCheckingErrorOccurred(QMailAccountId,QString,int)));
    disconnect(handler, SIGNAL(retrievalCompleted()),
            this, SLOT(intervalCheckingRetrievalComplete()));

    intervalCheckRetrievalInProgress = false;
    qLog(Messaging) << "Error performing interval check for account" << id << "-" << txt << ":" << code;
}

void MessageServer::intervalCheckingRetrievalComplete()
{
    if (!intervalAccountIds.isEmpty()) {
        QMailAccountId accountId = *intervalAccountIds.begin();
        intervalAccountIds.remove(accountId);

        qLog(Messaging) << "Performing interval check for account:" << accountId;
        handler->retrieve(accountId, false, true);
    } else {
        disconnect(handler, SIGNAL(errorOccurred(QMailAccountId,QString,int)),
                   this, SLOT(intervalCheckingErrorOccurred(QMailAccountId,QString,int)));
        disconnect(handler, SIGNAL(retrievalCompleted()),
                   this, SLOT(intervalCheckingRetrievalComplete()));

        intervalCheckRetrievalInProgress = false;

        // Emit any updates arising from this check
        handler->synchroniseClients();
    }
}

void MessageServer::partialMessageRetrieved(QMailMessageMetaData& message)
{
    message.setStatus(QMailMessage::Incoming, true);
    message.setStatus(QMailMessage::New, true);
    if (!message.parentFolderId().isValid())
        message.setParentFolderId(serverFolderId());

    // Set the content type for this message, where possible
    classifier.classifyMessage(message);

    // Store this message to the mail store
    QMailStore::instance()->addMessage(&message);

    bool complete(false);

    if (message.messageType() == QMailMessage::Mms) {
        if (message.parentAccountId().isValid()) {
            // If the user has configured automatic download, we should get this message immediately
            AccountConfiguration config(message.parentAccountId());
            if (config.isAutoDownload())
                complete = true;
        }
    } else if (message.messageType() == QMailMessage::Email) {
        // Automatically download voicemail messages
        if (message.content() == QMailMessage::VoicemailContent ||
            message.content() == QMailMessage::VideomailContent) {
            complete = true;
        }
    }

    if (intervalCheckRetrievalInProgress) {
        if (message.parentAccountId().isValid()) {
            AccountConfiguration config(message.parentAccountId());
            uint maxSize = static_cast<uint> ( config.maxMailSize() * 1024 );
            if ((config.maxMailSize() <= 0) || (message.size() <= maxSize))
                complete = true;
        }
    }

    if (complete) {
        autoCompleteIds.append(message.id());
    } else if (!intervalCheckRetrievalInProgress) {
        emit client->partialMessageRetrieved(QMailStore::instance()->messageMetaData(message.id()));
    }
}

void MessageServer::partialRetrievalCompleted()
{
    if (!autoCompleteIds.isEmpty() || intervalCheckRetrievalInProgress) {
        // Do not report this event until we have retrieved the auto-completed messages
        handler->completeRetrieval(autoCompleteIds);
    } else {
        // Ensure the client receives any resulting events before the completion notification
        QMailStore::instance()->flushIpcNotifications();

        emit client->partialRetrievalCompleted();
    }
}

void MessageServer::messageRetrieved(QMailMessage& message)
{
    if (!message.id().isValid()) {
        message.setStatus(QMailMessage::Incoming, true);
        message.setStatus(QMailMessage::New, true);
        if (!message.parentFolderId().isValid())
            message.setParentFolderId(serverFolderId());
    } else {
        QMailMessageMetaData existing(message.id());
        message.setStatus(QMailMessage::Incoming, (existing.status() & QMailMessage::Incoming));
        message.setStatus(QMailMessage::New, (existing.status() & QMailMessage::New));
        message.setContent(existing.content());
        message.setParentFolderId(existing.parentFolderId());

        if (message.messageType() == QMailMessage::Mms) {
            QString mmsType = message.headerFieldText("X-Mms-Message-Type");
            if (mmsType.contains("m-send-req")) {
                // The date of this MMS message is already recorded from the notification 
                message.setDate(existing.date());
            }
        }
    }
    
    // Set the content type for this message, where possible
    classifier.classifyMessage(message);

    if (message.id().isValid()) {
        // Update this message in the mail store
        QMailStore::instance()->updateMessage(&message);
    } else {
        // Store this message to the mail store
        QMailStore::instance()->addMessage(&message);
    }

    if (!autoCompleteIds.contains(message.id())) {
        // Send only the header information to the client
        emit client->messageRetrieved(QMailStore::instance()->messageMetaData(message.id()));
    }
}

void MessageServer::retrievalCompleted()
{
    if (!autoCompleteIds.isEmpty()) {
        // We have only completed the partial retrieval stage now
        QMailStore::instance()->flushIpcNotifications();
        emit client->partialRetrievalCompleted();

        foreach (const QMailMessageId &id, autoCompleteIds) {
            emit client->messageRetrieved(QMailStore::instance()->messageMetaData(id));
        }

        autoCompleteIds.clear();
    } else {
        // Ensure the client receives any resulting events before the completion notification
        QMailStore::instance()->flushIpcNotifications();

        emit client->retrievalCompleted();
    }
}

static QMap<QMailMessage::MessageType, QString> typeSignatureInit()
{
    QMap<QMailMessage::MessageType, QString> map;

    map.insert(QMailMessage::Sms, "newSmsCount(int)");
    map.insert(QMailMessage::Mms, "newMmsCount(int)");
    map.insert(QMailMessage::Email, "newEmailCount(int)");
    map.insert(QMailMessage::Instant, "newInstantCount(int)");
    map.insert(QMailMessage::System, "newSystemCount(int)");

    return map;
}

void MessageServer::reportNewCount(QMailMessage::MessageType type, int count)
{
    static QMap<QMailMessage::MessageType, QString> typeSignature(typeSignatureInit());

    if (typeSignature.contains(type)) {
        QtopiaIpcEnvelope e("QPE/MessageControl", typeSignature[type]);
        e << static_cast<int>(count);
    }
}

void MessageServer::acknowledgeNewMessages(const QMailMessageTypeList& types)
{
    if (unacknowledgedFlashSms && types.contains(QMailMessage::Sms))
        unacknowledgedFlashSms = 0;

    handler->acknowledgeNewMessages(types);

    foreach (QMailMessage::MessageType type, types) {
        // No messages of this type are new any longer
        QMailMessageKey newMessages(QMailMessageKey::Type, type);
        newMessages &= QMailMessageKey(QMailMessageKey::Status, QMailMessage::New, QMailDataComparator::Includes);
        QMailStore::instance()->updateMessagesMetaData(newMessages, QMailMessage::New, false);

        if (messageCounts[type] != 0) {
            newMessageTotal -= messageCounts[type];

            messageCounts[type] = 0;
            reportNewCount(type, 0);
        }
    }
}

void MessageServer::mmsMessage(const QDSActionRequest& request)
{
    // Don't request synchronisation; the handler knows that the MMS client is now un-synchronised
    handler->mmsMessage(request);
}

#ifndef QTOPIA_NO_SMS
static QMailAccountId getSmsAccountId()
{
    QMailAccountKey key(QMailAccountKey::MessageType, QMailMessage::Sms);
    QMailAccountIdList ids = QMailStore::instance()->queryAccounts(key);
    if (!ids.isEmpty())
        return ids.first();

    return QMailAccountId();
}
#endif

void MessageServer::smsFlash(const QDSActionRequest& request)
{
#ifndef QTOPIA_NO_SMS
    // Extract the SMS message from the request payload.
    QDataStream stream(request.requestData().data());
    QSMSMessage sms;
    stream >> sms;

    // Convert to a QMailMessage
    QMailMessage message(SmsClient::toMailMessage(sms, "flash", QString()));

    static QMailAccountId smsAccountId(getSmsAccountId());
    message.setParentAccountId(smsAccountId);

    // Process via the usual mechanism
    messageRetrieved(message);
    ++unacknowledgedFlashSms;

    handler->synchroniseClients();
#else
    Q_UNUSED(request)
#endif
}

static QMap<QMailMessage::MessageType, QString> typeServiceInit()
{
    QMap<QMailMessage::MessageType, QString> map;

    map.insert(QMailMessage::Sms, "NewSmsArrival");
    map.insert(QMailMessage::Mms, "NewMmsArrival");
    map.insert(QMailMessage::Email, "NewEmailArrival");
    map.insert(QMailMessage::Instant, "NewInstantMessageArrival");
    map.insert(QMailMessage::System, "NewSystemMessageArrival");

    return map;
}

static QString serviceForType(QMailMessage::MessageType type)
{
    static QMap<QMailMessage::MessageType, QString> typeService(typeServiceInit());
    return typeService[type];
}

int MessageServer::newMessageCount(QMailMessage::MessageType type) const
{
    QMailMessageKey newMessageKey(QMailMessageKey::Status, QMailMessage::New, QMailDataComparator::Includes);
    if (type != QMailMessage::AnyType) {
        newMessageKey &= QMailMessageKey(QMailMessageKey::Type, type);
    }

    return QMailStore::instance()->countMessages(newMessageKey);
}

void MessageServer::reportNewCounts()
{
    static QMap<QMailMessage::MessageType, QString> typeSignature(typeSignatureInit());

    QMailMessageCountMap newCounts;
    foreach (QMailMessage::MessageType type, typeSignature.keys()) {
        newCounts[type] = newMessageCount(type);
    }
    
    newMessageTotal = newMessageCount(QMailMessage::AnyType);

    if (newMessageTotal) {
        // Inform QPE of changes to the new message counts
        foreach (QMailMessage::MessageType type, typeSignature.keys()) {
            if ((newCounts[type] > 0) && (newCounts[type] != messageCounts[type]))
                reportNewCount(type, newCounts[type]);
        }

        // Request handling of the new message events
        QMailMessageCountMap::const_iterator it = newCounts.begin(), end = newCounts.end();
        for ( ; it != end; ++it) {
            QMailMessage::MessageType type(it.key());
            if (it.value() != messageCounts[type]) {
                // This type's count has changed since last reported
                if (QDSAction *action = new QDSAction("arrived", serviceForType(type))) {
                    actionType[action->id()] = type;

                    connect(action, SIGNAL(response(QUniqueId, QDSData)), this, SLOT(actionResponse(QUniqueId, QDSData)));
                    connect(action, SIGNAL(error(QUniqueId, QString)), this, SLOT(actionError(QUniqueId, QString)));

                    QByteArray data;
                    {
                        QDataStream insertor(&data, QIODevice::WriteOnly);
                        insertor << it.value();
                    }

                    // Ensure the client receives any generated events before the arrival notification
                    QMailStore::instance()->flushIpcNotifications();

                    if (!action->invoke(QDSData(data, QMimeType())))
                        qWarning() << "Unable to invoke service:" << serviceForType(type);
                }
            }
        }

#ifdef QTOPIA_HOMEUI
        // For Home edition, propagate the updated count to be displayed (it is
        // always visible). Otherwise, wait til the client has had a chance to 
        // acknowledge or suppress the event.
        emit messageCountUpdated();
#endif
    }

    messageCounts = newCounts;
}

void MessageServer::actionResponse(const QUniqueId &uid, const QDSData &responseData)
{
    if (QDSAction *action = static_cast<QDSAction*>(sender())) {
        const QByteArray &data(responseData.data());
        bool handled(false);
        {
            QDataStream extractor(data);
            extractor >> handled;
        }

        if (handled) {
            acknowledgeNewMessages(QMailMessageTypeList() << actionType[uid]);
        }

        actionType.remove(uid);
        action->deleteLater();

        if (actionType.isEmpty()) {
            // All outstanding handler events have been processed
            emit messageCountUpdated();
        }
    }
}

void MessageServer::actionError(const QUniqueId &uid, const QString &message)
{
    if (QDSAction *action = static_cast<QDSAction*>(sender())) {
        qWarning() << "Unable to complete service:" << serviceForType(actionType[uid]) << "-" << message;
        actionType.remove(uid);
        action->deleteLater();
    }

    if (actionType.isEmpty()) {
        // No outstanding handler events remain
        emit messageCountUpdated();
    }
}

void MessageServer::sendCompleted()
{
    // Ensure the client receives any resulting events before the completion notification
    QMailStore::instance()->flushIpcNotifications();

    emit client->sendCompleted();
}

void MessageServer::messageSent(const QMailMessageId &id)
{
    // Mark this message as having been sent
    QMailMessageMetaData message(id);
    message.setStatus(QMailMessage::Sent, true);
    QMailStore::instance()->updateMessage(&message);

    emit client->messageSent(id);
}

void MessageServer::errorOccurred(const QMailAccountId &id, const QString &txt, int code)
{
    if (!autoCompleteIds.isEmpty())
        autoCompleteIds.clear();

    emit client->errorOccurred(id, txt, code);
}

void MessageServer::statusChanged(QMailMessageServer::Operation op, const QString &accountName, const QString &status)
{
    if (!intervalCheckRetrievalInProgress) {
        emit client->statusChanged(op, accountName, status);
    }
}

void MessageServer::retrievalTotal(uint n)
{
    if (!intervalCheckRetrievalInProgress) {
        emit client->retrievalTotal(n);
    }
}

void MessageServer::retrievalProgress(uint n)
{
    if (!intervalCheckRetrievalInProgress) {
        emit client->retrievalProgress(n);
    }
}

void MessageServer::searchMessages(const QMailMessageKey& filter, const QString& bodyText)
{
    // Find the messages that match the filter criteria
    QMailMessageIdList searchIds = QMailStore::instance()->queryMessages(filter);

    // Schedule this search
    searches.append(MessageSearch(searchIds, bodyText));
    QTimer::singleShot(0, this, SLOT(continueSearch()));
}

static bool messageBodyContainsText(const QMailMessage &message, const QString& text)
{
    // Search only messages or message parts that are of type 'text/*'
    if (message.hasBody()) {
        if (message.contentType().type().toLower() == "text") {
            if (message.body().data().contains(text, Qt::CaseInsensitive))
                return true;
        }
    } else if (message.multipartType() != QMailMessage::MultipartNone) {
        // We could do a recursive search for text elements, but since we can't currently
        // display generic multipart messages anyway, let's just search the top level
        for (uint i = 0; i < message.partCount(); ++i) {
            const QMailMessagePart &part = message.partAt(i);

            if (part.contentType().type().toLower() == "text") {
                if (part.body().data().contains(text, Qt::CaseInsensitive))
                    return true;
            }
        }
    }

    return false;
}

void MessageServer::continueSearch()
{
    if (!searches.isEmpty()) {
        MessageSearch &currentSearch(searches.first());

        if (currentSearch.pending()) {
            matchingIds.clear();
            currentSearch.inProgress();

            if (!currentSearch.isEmpty()) {
                emit client->searchTotal(currentSearch.total());
            }
        }

        if (!currentSearch.isEmpty()) {
            foreach (const QMailMessageId &id, currentSearch.takeBatch()) {
                QMailMessage message(id);
                if (messageBodyContainsText(message, currentSearch.bodyText()))
                    matchingIds.append(id);
            }

            emit client->searchProgress(currentSearch.progress());
        }

        if (currentSearch.isEmpty()) {
            // Nothing more to search - we're finished
            emit client->matchingMessageIds(matchingIds);
            emit client->searchCompleted();

            searches.takeFirst();
            if (!searches.isEmpty())
                QTimer::singleShot(0, this, SLOT(continueSearch()));
        } else {
            QTimer::singleShot(0, this, SLOT(continueSearch()));
        }
    }
}

void MessageServer::cancelSearch()
{
    searches.clear();
    emit client->matchingMessageIds(matchingIds);
    emit client->searchCompleted();
}

void MessageServer::messagesUpdated(const QMailMessageIdList &)
{
    // Don't bother checking if we're responsible for the update
    if (QMailStore::instance()->asynchronousEmission()) {
        updateNewMessageCounts();
    }
}

void MessageServer::messagesRemoved(const QMailMessageIdList &)
{
    updateNewMessageCounts();
}

void MessageServer::updateNewMessageCounts()
{
    int newTotal = newMessageCount(QMailMessage::AnyType);
    if (newTotal != newMessageTotal) {
        // The number of messages marked as new has changed, but not via a message arrival event
        static QMap<QMailMessage::MessageType, QString> typeSignature(typeSignatureInit());

        // Update the individual counts
        foreach (QMailMessage::MessageType type, typeSignature.keys()) {
            int count(newMessageCount(type));
            if (count != messageCounts[type]) {
                messageCounts[type] = count;
                reportNewCount(type, count);
            }
        }

        emit messageCountUpdated();
    }
}

void MessageServer::simReady(bool ready)
{
    telephonyValueSpace.setAttribute("SMSReady", ready);
}

