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


#ifndef MMSCLIENT_H
#define MMSCLIENT_H

#include <quuid.h>
#include <qlist.h>
#include <qobject.h>
#include "qmailaccount.h"
#include "client.h"
#include <QMailMessage>

#include <QTimer>
#include <QtopiaNetworkInterface>

class QMailAccount;
class MMSMessage;
class MmsComms;
class QHttp;
class QHttpResponseHeader;
class QHttpRequestHeader;
class QWspPart;
class QNetworkDevice;
class QDSActionRequest;

class MmsClient: public TransmittingClient
{
    Q_OBJECT

public:
    MmsClient(QObject* parent);
    ~MmsClient();

    virtual void setAccount(const QMailAccountId &accountId);
    virtual QMailAccountId account() const;

    virtual void newConnection();
    virtual bool hasDeleteImmediately() const;
    virtual void closeConnection();
    virtual void setSelectedMails(const SelectionMap& data);
    virtual void checkForNewMessages();
    virtual void cancelTransfer();

    virtual int addMail(const QMailMessage &);

    void sendNotifyResp(const QString& serverUid, const QString &status);
    void pushMmsMessage(const QDSActionRequest& request);
    void closeWhenDone();

signals:
    void sendProgress(const QMailMessageId&, uint);
    void messageProcessed(const QMailMessageId&);

public slots:
    void errorHandling(int, QString msg);

private slots:
    void notificationInd(const MMSMessage &msg);
    void deliveryInd(const MMSMessage &msg);
    void sendConf(const MMSMessage &msg);
    void retrieveConf(const MMSMessage &msg, int);
    void statusChange(const QString &status);
    void commsError(int code, const QString &msg);
    void transferSize(int);
    void transfersComplete();
    void transferNextMessage();
    void raiseFailure();
    void networkStatusChange(QtopiaNetworkInterface::Status, bool);
    void networkDormant();

private:
    void sendMessage(MMSMessage&);
    void sendAcknowledge(const MMSMessage &);
    void sendNextMessage();
    void getNextMessage();
    QString networkConfig() const;
    QString mmsInterfaceName() const;
    bool raiseNetwork();

    static QMailMessage toMailMessage(const MMSMessage &mms, int size);
    static MMSMessage toMms(const QMailMessage &mail);

    static void addField(MMSMessage &mms, const QMailMessage& mail, const QString &field);
    static QString encodeRecipient(const QString &r);
    static QString decodeRecipient(const QString &r);

private:
    QMailAccountId accountId;
    MmsComms *comms;
    MessageMap selectionMap;
    MessageMap::ConstIterator selectionItr;
    QList<MMSMessage> outgoing;
    int messagesSent;
    bool quitRecv;
    bool networkReference;
    QNetworkDevice* networkDevice;
    QtopiaNetworkInterface::Status networkStatus;
    QTimer raiseTimer;
    QTimer inactivityTimer;
    QMap<QString, QMailMessageId> sentMessages;
    uint messageLength;
    uint sentLength;
    QMap<QString, QString> locationForTransaction;

    static int txnId;
};

#endif
