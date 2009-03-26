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

#ifndef MAILMESSAGECLIENT_H
#define MAILMESSAGECLIENT_H

#include "qmailmessageserver.h"

#include <QObject>
#include <QtopiaIpcAdaptor>

// The back-end corresponding to the front-end in QMailMessageServer
class MailMessageClient : public QObject
{
    Q_OBJECT

    friend class MessageServer;

public:
    MailMessageClient(QObject* parent);
    ~MailMessageClient();

private:
    // Disallow copying
    MailMessageClient(const MailMessageClient&);
    void operator=(const MailMessageClient&);

signals:
    void send(const QMailMessageIdList& mailList);

    void retrieve(const QMailAccountId& account, bool foldersOnly);
    void completeRetrieval(const QMailMessageIdList& mailList);

    void cancelTransfer();

    void searchMessages(const QMailMessageKey& filter, const QString& bodyText);

    void cancelSearch();

    void acknowledgeNewMessages(const QMailMessageTypeList&);

    void statusChanged(QMailMessageServer::Operation, const QString&, const QString&);
    void errorOccurred(const QMailAccountId&, const QString&, int);

    void newCountChanged(const QMailMessageCountMap&);

    void partialMessageRetrieved(const QMailMessageMetaData&);
    void partialRetrievalCompleted();

    void retrievalTotal(uint);
    void retrievalProgress(uint);

    void messageRetrieved(const QMailMessageMetaData&);
    void retrievalCompleted();

    void sendTotal(uint);
    void sendProgress(uint);

    void messageSent(const QMailMessageId&);
    void sendCompleted();

    void searchTotal(uint);
    void searchProgress(uint);

    void matchingMessageIds(const QMailMessageIdList&);
    void searchCompleted();

private:
    QtopiaIpcAdaptor* adaptor;
};

#endif
