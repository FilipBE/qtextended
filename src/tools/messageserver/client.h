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

#ifndef Client_H
#define Client_H

#include <QObject>
#include <QMailMessageServer>
#include <qtopiaglobal.h>
#include <QMap>
#include <QMailAccountId>

class QMailMessageId;
class QMailMessage;

//map server uid to local id 
typedef QMap<QString,QMailMessageId> MessageMap;
typedef QMap<QMailFolderId, MessageMap> SelectionMap;

class Client : public QObject
{
    Q_OBJECT

public:
    enum DefunctReason { Removed = 0, FolderRemoved };

    Client(QObject* parent);
    virtual ~Client();

    virtual void setAccount(const QMailAccountId &account);
    virtual QMailAccountId account() const;

    virtual void foldersOnly(bool folders);
    virtual void headersOnly(bool headers, int limit);
    virtual void newConnection();
    virtual void closeConnection();
    virtual void setSelectedMails(const SelectionMap& data);
    virtual bool hasDeleteImmediately() const;
    virtual void deleteImmediately(const QString& serverUid);
    virtual void checkForNewMessages();
    virtual void resetNewMailCount();
    virtual void cancelTransfer();

signals:
    void errorOccurred(int, QString &);
    void partialRetrievalCompleted();
    void retrievalCompleted();
    void nonexistentMessage(const QString&, Client::DefunctReason);
    void deviceReady();

    void updateStatus(QMailMessageServer::Operation, const QString &);
    void newMessage(QMailMessage&, bool);
    void allMessagesReceived();
    void messageTransmitted(const QMailMessageId&);
    void newMailDiscovered(const QMailAccountId&);
};

class TransmittingClient : public Client
{
    Q_OBJECT

public:
    TransmittingClient(QObject *parent);

    virtual int addMail(const QMailMessage &);
};

#endif
