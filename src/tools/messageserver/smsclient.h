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

#ifndef SMSCLIENT_H
#define SMSCLIENT_H
#ifndef QTOPIA_NO_SMS

#include "client.h"
#include <private/accountconfiguration_p.h>

#include <qtelephonynamespace.h>
#include <qtopiaglobal.h>

#include <QDateTime>
#include <QList>
#include <QMailAddress>
#include <QMailMessageId>
#include <QMailMessage>
#include <QObject>
#include <QSimInfo>
#include <QString>
#include <QStringList>
#include <QTimer>


struct RawSms
{
    QString number;
    QString body;
    QString mimetype;
    QMailMessageId msgId;
};


class QSMSReader;
class QSMSSender;
class QSMSMessage;
class QRegExp;
class QMailAccount;

class SmsClient: public TransmittingClient
{
        Q_OBJECT

public:
        SmsClient(QObject* parent);
        ~SmsClient();

        virtual void setAccount(const QMailAccountId &account);
        virtual QMailAccountId account() const;

        virtual void newConnection();
        virtual bool hasDeleteImmediately() const;
        virtual void deleteImmediately(const QString& serverUid);
        virtual void resetNewMailCount();
        virtual void checkForNewMessages();
        virtual void cancelTransfer();

        virtual int addMail(const QMailMessage& mail);

        void clearList();
        // Determines if a string is in the form *
        bool smsAddress(const QMailAddress &);
        // Determines if a string is in the form "^\\+?\\d[\\d-]*$"
        bool validSmsAddress(const QMailAddress &);
        // Separates the sms (phone numbers) addresses from the passed address list
        // Returns the sms addressses and modifies the passed list
        QList<QMailAddress> separateSmsAddresses(const QList<QMailAddress> &);
        // Format an outgoing message
        QString formatOutgoing( const QString& subject, const QString &body );

        static QMailMessage toMailMessage(const QSMSMessage& msg, const QString& id, const QString& simId);

signals:
        void simReady(bool);
        void updateStatus(const QString &);
        void sendProgress(const QMailMessageId&, uint);    // Not implemented
        void messageProcessed(const QMailMessageId&);

public slots:
        void errorHandling(int, QString msg);

private slots:
        void finished(const QString&, QTelephony::Result);
        void messageCount( int );
        void fetched( const QString&, const QSMSMessage& );
        void simReadyChanged();
        void simIdentityChanged();
        void retrievalTimeout();

private:
        void allMessagesRetrieved();

        QList<RawSms> smsList;
        QMap<QString, RawSms> sentMessages;
        QSMSReader *req;
        QSMSSender *sender;
        bool smsFetching, smsSending;
        bool success;
        QString simIdentity;
        bool haveSimIdentity;
        bool sawNewMessage;
        bool smsCheckRequired;
        QStringList activeIds;
        QStringList retrievedIds;
        QList<QDateTime> timeStamps;
        QMailAccountId accountId;
        QSimInfo *simInfo;
        QList<QMailMessage> retrievedMessages;
        QTimer retrievalTimer;
        AccountConfiguration _config;
        static QRegExp *sSmsAddress, *sValidSmsAddress;
};

#endif // QTOPIA_NO_SMS
#endif
