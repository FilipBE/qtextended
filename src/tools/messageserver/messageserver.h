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

#ifndef MESSAGESERVER_H
#define MESSAGESERVER_H

#include "messageclassifier.h"

#include <QUniqueId>
#include <QMailMessageServer>
#include <QObject>
#include <QSet>
#include <QValueSpaceItem>
#include <QValueSpaceObject>

class EmailHandler;
class MailMessageClient;
class QDSActionRequest;
class QDSData;
class QMailMessageMetaData;
class QNetworkState;

class MessageServer : public QObject
{
    Q_OBJECT

public:
    MessageServer(QObject *parent = 0);
    ~MessageServer();

signals:
    void messageCountUpdated();

private slots:
    void initIntervalChecking();
    void partialMessageRetrieved(QMailMessageMetaData&);
    void partialRetrievalCompleted();
    void retrievalTotal(uint);
    void retrievalProgress(uint);
    void messageRetrieved(QMailMessage&);
    void retrievalCompleted();
    void acknowledgeNewMessages(const QMailMessageTypeList&);
    void newMailDiscovered(const QMailAccountId &accountId);

    void mmsMessage(const QDSActionRequest&);
    void smsFlash(const QDSActionRequest&);

    void sendCompleted();
    void messageSent(const QMailMessageId &);

    void errorOccurred(const QMailAccountId &id, const QString &txt, int code);
    void statusChanged(QMailMessageServer::Operation, const QString&, const QString&);
    void intervalCheckTimeout();
    void intervalCheckingErrorOccurred(const QMailAccountId &id, const QString &txt, int code);
    void intervalCheckingRetrievalComplete();

    void searchMessages(const QMailMessageKey &filter, const QString &bodyText);
    void continueSearch();
    void cancelSearch();

    void actionResponse(const QUniqueId &uid, const QDSData &responseData);
    void actionError(const QUniqueId &uid, const QString &message);

    void messagesUpdated(const QMailMessageIdList &ids);
    void messagesRemoved(const QMailMessageIdList &ids);

    void reportNewCounts();

    void simReady(bool);

private:
    class MessageSearch
    {
    public:
        MessageSearch(const QMailMessageIdList &ids, const QString &text);

        const QString &bodyText() const;

        bool pending() const;
        void inProgress();

        bool isEmpty() const;

        uint total() const;
        uint progress() const;

        QMailMessageIdList takeBatch();

    private:
        QMailMessageIdList _ids;
        QString _text;
        bool _active;
        uint _total;
        uint _progress;
    };

    void reportNewCount(QMailMessage::MessageType type, int count);

    int newMessageCount(QMailMessage::MessageType type) const;

    void updateNewMessageCounts();

    EmailHandler *handler;
    MailMessageClient *client;
    uint unacknowledgedFlashSms;
    MessageClassifier classifier;
    QMailMessageIdList autoCompleteIds;
    QMailMessageCountMap messageCounts;
    QList<MessageSearch> searches;
    QMailMessageIdList matchingIds;

    //Interval checking variables
    bool intervalCheckRetrievalInProgress;
    QMap<QTimer*, QMailAccountId> intervalCheckMap;
    QSet<QMailAccountId> intervalAccountIds;
    QValueSpaceItem roamingMode;
    QNetworkState *netState;

    QtopiaIpcAdaptor messageCountUpdate;
    QMap<QUniqueId, QMailMessage::MessageType> actionType;

    QValueSpaceObject telephonyValueSpace;

    int newMessageTotal;
};

#endif
