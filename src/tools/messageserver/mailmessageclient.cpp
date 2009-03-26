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

#include "mailmessageclient.h"

MailMessageClient::MailMessageClient(QObject* parent)
    : QObject(parent),
      adaptor(new QtopiaIpcAdaptor("QPE/QMailMessageServer", this))
{
    QtopiaIpcAdaptor::connect(this, SIGNAL(statusChanged(QMailMessageServer::Operation,QString,QString)),
                              adaptor, MESSAGE(statusChanged(QMailMessageServer::Operation, QString, QString)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(retrievalTotal(uint)),
                              adaptor, MESSAGE(retrievalTotal(uint)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(retrievalProgress(uint)),
                              adaptor, MESSAGE(retrievalProgress(uint)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(sendTotal(uint)),
                              adaptor, MESSAGE(sendTotal(uint)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(sendProgress(uint)),
                              adaptor, MESSAGE(sendProgress(uint)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(sendCompleted()), 
                              adaptor, MESSAGE(sendCompleted()));
    QtopiaIpcAdaptor::connect(this, SIGNAL(messageSent(QMailMessageId)), 
                              adaptor, MESSAGE(messageSent(QMailMessageId)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(errorOccurred(QMailAccountId,QString,int)),
                              adaptor, MESSAGE(errorOccurred(QMailAccountId, QString, int)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(newCountChanged(QMailMessageCountMap)),
                              adaptor, MESSAGE(newCountChanged(QMailMessageCountMap)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(partialMessageRetrieved(QMailMessageMetaData)),
                              adaptor, MESSAGE(partialMessageRetrieved(QMailMessageMetaData)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(messageRetrieved(QMailMessageMetaData)),
                              adaptor, MESSAGE(messageRetrieved(QMailMessageMetaData)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(partialRetrievalCompleted()),
                              adaptor, MESSAGE(partialRetrievalCompleted()));
    QtopiaIpcAdaptor::connect(this, SIGNAL(retrievalCompleted()),
                              adaptor, MESSAGE(retrievalCompleted()));
    QtopiaIpcAdaptor::connect(this, SIGNAL(searchTotal(uint)),
                              adaptor, MESSAGE(searchTotal(uint)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(searchProgress(uint)),
                              adaptor, MESSAGE(searchProgress(uint)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(matchingMessageIds(QMailMessageIdList)), 
                              adaptor, MESSAGE(matchingMessageIds(QMailMessageIdList)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(searchCompleted()), 
                              adaptor, MESSAGE(searchCompleted()));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(cancelSearch()),
                              this, SIGNAL(cancelSearch()));

    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(send(QMailMessageIdList)),
                              this, SIGNAL(send(QMailMessageIdList)));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(retrieve(QMailAccountId, bool)),
                              this, SIGNAL(retrieve(QMailAccountId, bool)));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(completeRetrieval(QMailMessageIdList)),
                              this, SIGNAL(completeRetrieval(QMailMessageIdList)));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(cancelTransfer()),
                              this, SIGNAL(cancelTransfer()));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(acknowledgeNewMessages(QMailMessageTypeList)),
                              this, SIGNAL(acknowledgeNewMessages(QMailMessageTypeList)));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(searchMessages(QMailMessageKey, QString)),
                              this, SIGNAL(searchMessages(QMailMessageKey, QString)));
}

MailMessageClient::~MailMessageClient()
{
}

