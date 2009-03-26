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

#ifndef COLLECTIVECLIENT_H
#define COLLECTIVECLIENT_H

#include "client.h"

#include <QList>
#include <QPair>
#include <QTimer>

class QCollectiveSimpleMessage;
class QCollectiveMessenger;
class QCommServiceManager;
class QMailAccount;
class QMailMessage;
class QNetworkRegistration;

class CollectiveClient : public TransmittingClient
{
    Q_OBJECT

public:
    CollectiveClient(QObject *parent);
    ~CollectiveClient();

    virtual void setAccount(const QMailAccountId &accountId);
    virtual QMailAccountId account() const;

    virtual void newConnection();
    virtual bool hasDeleteImmediately() const;
    virtual void checkForNewMessages();
    virtual void cancelTransfer();

    virtual int addMail(const QMailMessage &mail);

signals:
    void sendProgress(const QMailMessageId &, uint);
    void messageProcessed(const QMailMessageId &);

public slots:
    void errorHandling(int, QString msg);

private slots:
    void servicesChanged();
    void registrationStateChanged();
    void messengerDisconnected();
    void incomingMessages(const QList<QCollectiveSimpleMessage> &messages);
    void registrationTimeout();

private:
    void registerRegistration();
    void registerMessenger();
    void updateAccountSettings();
    void transmitMessages();

    QCommServiceManager *m_manager;
    QNetworkRegistration *m_registration;
    QCollectiveMessenger *m_messenger;
    QMailAccountId m_accountId;
    QString m_fromAddress;
    bool m_registered;
    QTimer registrationTimer;

    typedef QPair<QCollectiveSimpleMessage, QPair<QStringList, QMailMessageId> > MessageData;
    QList<MessageData> m_messages;
};

#endif
