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

#ifndef EMAILHANDLER_H
#define EMAILHANDLER_H

#include "qmailaccount.h"
#include "qmailstore.h"
#include "client.h"
#include "smtpclient.h"
#ifndef QTOPIA_NO_SMS
#include "smsclient.h"
#endif
#include "popclient.h"
#include "imapclient.h"
#include "collectiveclient.h"
#include "systemclient.h"

#include <QObject>
#include <QString>
#include <QList>
#include <QMailMessageServer>
#include <QMap>
#include <QSet>
#include <QStringList>

class MmsClient;
class QDSActionRequest;

class EmailHandler : public QObject
{
    Q_OBJECT

public:
    struct ErrorEntry { int code; const char* text; };
    typedef QPair<const ErrorEntry*, size_t> ErrorMap;
    typedef QList<ErrorMap> ErrorSet;

    EmailHandler(QObject* parent);
    ~EmailHandler();

    enum TransmissionType {
        Unknown = 0,
        Receiving = 0x01,
        Sending = 0x02,
        AsyncArrival = 0x04,
        AsyncDeletion = 0x08,
        SyncRetrieval = 0x10
    };

    bool retrievalInProgress();

public slots:
    void send(const QMailMessageIdList& mailList);
    void retrieve(const QMailAccountId& accountId, bool foldersOnly, bool countEmailMessages = false);
    void completeRetrieval(const QMailMessageIdList& mailList);
    void cancelTransfer();
    void acknowledgeNewMessages(const QMailMessageTypeList& types);

    void synchroniseClients();

signals:
    void simReady(bool);
    void statusChanged(QMailMessageServer::Operation, const QString&, const QString&);
    void errorOccurred(const QMailAccountId&, const QString&, int);
    void newCountChanged();
    void newCountDetermined();
    void partialMessageRetrieved(QMailMessageMetaData&);
    void partialRetrievalCompleted();
    void retrievalTotal(uint);
    void retrievalProgress(uint);
    void messageRetrieved(QMailMessage&);

    void retrievalCompleted();
    void sendTotal(uint);
    void sendProgress(uint);
    void messageSent(const QMailMessageId&);
    void sendCompleted();
    void newMailDiscovered(const QMailAccountId&);

public slots:
    void readAccounts();
    void clientStatus(QMailMessageServer::Operation, const QString&);
    void sendProgress(const QMailMessageId&, uint percentage);
    void retrievalProgress(const QString&, uint length);
    void messageProcessed(const QMailMessageId&);
    void messageProcessed(const QString&);
    void deletedMessages(const QMailAccountIdList&);
    void mmsMessage(const QDSActionRequest& request);

private slots:
    void messagesReceived();
    void newClientMessage(QMailMessage&, bool);
    void nonexistentMessage(const QString&, Client::DefunctReason);

    void genericError(int, QString&);
    void socketError(int, QString&);
    void mailError(int, QString&);

    void clientError(int, QString&);

    void reportFailure(const QMailAccountId &accountId, const QString& message, const int code, const ErrorSet& errorSet = ErrorSet());
    void partialRetrievalComplete();
    void retrievalComplete();

    void fetchTotal(uint);
    void fetchProgress(uint);

private:
    void connectClient(Client *client, int type, QMailMessage::MessageType messageType, QString sigName);
    void setMailAccount(const QMailAccountId &accountId);
    void transmissionFailed(const Client* client);
    void clientSynchronised(const Client* client);

    typedef QPair<Client*, QMailAccountId> ConnectionDetails;

    const QList<ConnectionDetails>& connections() const;
    ConnectionDetails* connectionForClient(Client* client);

    QList<Client*> clients() const;

    Client* clientForType(QMailMessage::MessageType type) const;

    Client* clientForAccount(const QMailAccountId &accountId, bool retrieving);
    QMailAccountId accountForClient(const Client *client) const;

    void setClientAccount(Client* client, const QMailAccountId &accountId);

    bool appendErrorText(QString& message, const int code, const ErrorMap& map);
    bool appendErrorText(QString& message, const int code, const ErrorSet& mapList);

    QMailAccountId mailAccountId;

    SmtpClient *smtpClient;
#ifndef QTOPIA_NO_SMS
    SmsClient *smsClient;
#endif
#ifndef QTOPIA_NO_MMS
    MmsClient *mmsClient;
#endif
#ifndef QTOPIA_NO_COLLECTIVE
    CollectiveClient *collectiveClient;
#endif
    SystemClient *systemClient;

    QSet<const Client*> unsynchronised;

    // SendMap maps id -> (units) to be sent
    typedef QMap<QMailMessageId, uint> SendMap;
    SendMap sendSize;
    uint totalSendSize;

    // RetrievalMap maps uid -> ((units, bytes) to be retrieved, percentage retrieved)
    typedef QMap<QString, QPair< QPair<uint, uint>, uint> > RetrievalMap;
    RetrievalMap retrievalSize;
    uint totalRetrievalSize;

    QList<ConnectionDetails> connectionList;

    QMap<Client*, QMailMessage::MessageType> clientMessageTypes;

    QMap<QMailMessageId, Client*> clientForOutgoing;

    bool mRetrievalInProgress;
    bool mCountEmailMessages;
};

#endif
