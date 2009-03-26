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

#ifndef QMAILMESSAGESERVER_H
#define QMAILMESSAGESERVER_H

#include <qtopiaglobal.h>

#include <QList>
#include <QMailMessage>
#include <QMailMessageKey>
#include <QMailMessageMetaData>
#include <QMap>
#include <QSharedDataPointer>
#include <QString>
#include <QStringList>
#include <QtopiaIpcAdaptor>

class QMailMessageServerPrivate;

typedef QMap<QMailMessage::MessageType, int> QMailMessageCountMap;

class QTOPIAMAIL_EXPORT QMailMessageServer : public QObject
{
    Q_OBJECT

private:
    enum { ErrorCodeBase = 256 };

public:
    enum Operation { None = 0, Send, Retrieve };

    enum ErrorCode {
        ErrNoError = 0,
        ErrUnknownResponse = ErrorCodeBase,
        ErrLoginFailed,
        ErrCancel,
        ErrFileSystemFull,
        ErrNonexistentMessage,
        ErrEnqueueFailed,
        ErrNoConnection,
        ErrConnectionInUse,
        ErrConnectionNotReady,
        ErrConfiguration,
        ErrInvalidAddress
    };

    QMailMessageServer(QObject* parent = 0);
    ~QMailMessageServer();

private:
    // Disallow copying
    QMailMessageServer(const QMailMessageServer&);
    void operator=(const QMailMessageServer&);

signals:
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

public slots:
    void send(const QMailMessageIdList& mailList);

    void retrieve(const QMailAccountId& account, bool foldersOnly);
    void completeRetrieval(const QMailMessageIdList& mailList);

    void cancelTransfer();

    void acknowledgeNewMessages(const QMailMessageTypeList& types);

    void deleteMessages(const QMailMessageIdList& mailList);

    void searchMessages(const QMailMessageKey& filter, const QString& bodyText);
    void cancelSearch();

private:
    QMailMessageServerPrivate* d;
};

Q_DECLARE_USER_METATYPE_ENUM(QMailMessageServer::Operation)
Q_DECLARE_USER_METATYPE_ENUM(QMailMessageServer::ErrorCode)

Q_DECLARE_METATYPE(QMailMessageCountMap)
Q_DECLARE_USER_METATYPE_TYPEDEF(QMailMessageCountMap, QMailMessageCountMap)

#endif
