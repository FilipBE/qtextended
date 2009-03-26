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



#ifndef SmtpClient_H
#define SmtpClient_H

#include "client.h"
#include <private/accountconfiguration_p.h>

#include <qstring.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qlist.h>
#include <QMailMessage>

class MailTransport;
class QMailAccount;

struct RawEmail
{
    QString from;
    QStringList to;
    QMailMessage mail;
};

class SmtpClient: public TransmittingClient
{
    Q_OBJECT

public:
    SmtpClient(QObject* parent);
    ~SmtpClient();

    virtual void setAccount(const QMailAccountId &accountId);
    virtual QMailAccountId account() const;

    virtual void newConnection();
    virtual void cancelTransfer();

    virtual int addMail(const QMailMessage& mail);

signals:
    void sendProgress(const QMailMessageId&, uint);
    void messageProcessed(const QMailMessageId&);

public slots:
    void errorHandling(int, QString msg);

protected slots:
    void connected(AccountConfiguration::EncryptType encryptType);
    void readyRead();
    void sent(qint64);
    void transportStatus(const QString& status);

private:
    void doSend(bool authenticating = false);
    void incomingData();
    void authenticate();
#ifndef QT_NO_OPENSSL
    QString _toBase64(const QString& in) const;
#endif

private:
    enum TransferStatus
    {
        Init,
#ifndef QT_NO_OPENSSL
        StartTLS, TLS, Auth, AuthUser, AuthPass,
#endif
        Login, Pass, Done, From, Recv, MRcv, Data, Body, Quit
    };

    QMailAccountId accountId;
    AccountConfiguration _config;
    TransferStatus status;
    QList<RawEmail> mailList;
    QList<RawEmail>::Iterator mailItr;
    QMailMessageId sendingId;
    uint messageLength;
    uint sentLength;
    bool sending, authenticating, success;
    QStringList::Iterator it;
    MailTransport *transport;
};

#endif
