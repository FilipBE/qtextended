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

#ifndef PopClient_H
#define PopClient_H

#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qlist.h>

#include "client.h"
#include <private/accountconfiguration_p.h>

class LongStream;
class MailTransport;
class QMailAccount;

class PopClient: public Client
{
    Q_OBJECT

public:
    PopClient(QObject* parent);
    ~PopClient();

    virtual void setAccount(const QMailAccountId &accountId);
    virtual QMailAccountId account() const;

    virtual void newConnection();
    virtual void closeConnection();
    virtual void headersOnly(bool headers, int limit);
    virtual void setSelectedMails(const SelectionMap& data);
    virtual void checkForNewMessages();
    virtual void cancelTransfer();

signals:
    void retrievalProgress(const QString&, uint);
    void messageProcessed(const QString&);
    void fetchTotal(uint);
    void fetchProgress(uint);

public slots:
    void errorHandling(int, QString msg);

protected slots:
    void incomingData();
    void transportStatus(const QString& status);

private:
    int nextMsgServerPos();
    int msgPosFromUidl(QString uidl);
    int getSize(int pos);
    void uidlIntegrityCheck();
    void createMail();
    void sendCommand(const QString& cmd);
    QString readResponse();
    void retrieveOperationCompleted();

private:
    enum TransferStatus
    {
            Init, Pass, Stat, Mcnt, Read, List, Size, Retr, Acks,
            Quit, Done, Ignore, Dele, Rset, Uidl, Guidl, Exit
    };

    QMailAccountId accountId;
    AccountConfiguration _config;
    TransferStatus status;
    int messageCount, mailSize, headerLimit;
    int msgNum;
    bool receiving, preview, selected;
    bool awaitingData;
    QString message;
    MessageMap selectionMap;
    MessageMap::ConstIterator selectionItr;
    int listSize;

    QMap<QString, int> serverUidNumber;
    QMap<int, QString> serverUid;
    QMap<int, int> serverSize;

    QString msgUidl;
    QStringList uniqueUidlList;
    QStringList lastUidl;
    QStringList deleteList;
    LongStream *d;

    MailTransport *transport;

    QString retrieveUid;
};

#endif
