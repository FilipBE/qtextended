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

#ifndef MESSAGESTORE_H
#define MESSAGESTORE_H

#include "messagestore.h"
#include "messagefolder.h"

#include <QIcon>
#include <QList>
#include <QMailMessage>
#include <QMailMessageKey>
#include <QMailFolderKey>
#include <QString>

class QMailAccount;

class MessageStore : public QObject
{
    Q_OBJECT

public:
    MessageStore(QObject *parent = 0);
    ~MessageStore();

    void openMailboxes();

    QMailFolderIdList standardFolders() const;

    MessageFolder* mailbox(QMailFolder::StandardFolder folder) const;
    MessageFolder* mailbox(const QMailFolderId& mailFolderId) const;

    MessageFolder* serverMailbox() const;

    MessageFolder* owner(const QMailMessageId& id) const;

    static QMailMessageIdList messages(QMailMessage::MessageType type = QMailMessage::AnyType, 
                                       const MessageFolder::SortOrder& order = MessageFolder::Submission);

    static QMailMessageIdList messages(quint64 status, 
                                       bool contains,
                                       QMailMessage::MessageType type = QMailMessage::AnyType,
                                       const MessageFolder::SortOrder& order = MessageFolder::Submission);

    static QMailMessageIdList messagesFromAccount(const QMailAccount& account, 
                                                  QMailMessage::MessageType type = QMailMessage::AnyType,
                                                  const MessageFolder::SortOrder& order = MessageFolder::Submission);

    static QMailFolderIdList foldersFromAccount(const QMailAccount& account);

    static QMailMessageIdList messages(QMailMessageKey queryKey, const MessageFolder::SortOrder& order);
    static QMailFolderIdList folders(QMailFolderKey queryKey);

    static uint messageCount( MessageFolder::MailType status, QMailMessage::MessageType type = QMailMessage::AnyType );
    static uint messageCount( MessageFolder::MailType status, QMailMessage::MessageType type, const QMailAccount& account );

    static uint messageCount( QMailMessageKey queryKey );

    static QMailMessageKey statusFilterKey(MessageFolder::MailType status);
    static QMailMessageKey statusFilterKey(quint64 status, bool contains);
    static QMailMessageKey messageFilterKey(QMailMessage::MessageType type, 
                                            const QMailAccountId& accountId = QMailAccountId(), 
                                            const QMailFolderId& mailboxId = QMailFolderId(), 
                                            bool subfolders = false);

signals:
    void externalEdit(const QString &);
    void contentModified(const MessageFolder* folder);
    void stringStatus(QString &);

protected slots:
    void folderContentModified();

private:
    QList<MessageFolder*> _mailboxes;
};

#endif
