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

#include "messagefolder.h"
#include "messagestore.h"

#include <QMailAccount>
#include <QMailStore>

#include "qtopialog.h"


MessageFolder::MessageFolder(const QMailFolder::StandardFolder& folder, QObject *parent)
    : QObject(parent),
    mFolder(folder),
    mParentFolderKey(QMailMessageKey::ParentFolderId,mFolder.id())
{
}

MessageFolder::MessageFolder(const QMailFolderId& id, QObject *parent)
    : QObject(parent),
    mFolder(id),
    mParentFolderKey(QMailMessageKey::ParentFolderId,mFolder.id())
{
}

MessageFolder::~MessageFolder()
{
}

QString MessageFolder::mailbox() const
{
    return mFolder.name();
}

QMailFolder MessageFolder::mailFolder() const
{
    return mFolder;
}

QMailFolderId MessageFolder::id() const
{
    return mFolder.id();
}

bool MessageFolder::insertMessage(QMailMessage &message)
{
    message.setParentFolderId(mFolder.id());

    if (message.id().isValid()) {
        return QMailStore::instance()->updateMessage(&message);
    } else {
        return QMailStore::instance()->addMessage(&message);
    }
}

bool MessageFolder::insertMessage(QMailMessageMetaData &message)
{
    message.setParentFolderId(mFolder.id());

    if (message.id().isValid()) {
        return QMailStore::instance()->updateMessage(&message);
    } else {
        return QMailStore::instance()->addMessage(&message);
    }
}

bool MessageFolder::moveMessage(const QMailMessageId& id)
{
    QMailMessageMetaData metaData(id);

    // Message is already in the correct folder; report success
    if (metaData.parentFolderId() == mFolder.id())
        return true;

    metaData.setParentFolderId(mFolder.id());
    return QMailStore::instance()->updateMessage(&metaData);
}

bool MessageFolder::moveMessages(const QMailMessageIdList& ids)
{
    bool rv(true);

    foreach (const QMailMessageId &id, ids)
        rv &= moveMessage(id);

    return rv;
}

bool MessageFolder::copyMessage(const QMailMessageId& id)
{
    QMailMessage message(id);

    message.setId(QMailMessageId()); //reset id
    message.setParentFolderId(mFolder.id());
    return QMailStore::instance()->addMessage(&message);
}

bool MessageFolder::copyMessages(const QMailMessageIdList& ids)
{
    bool rv(true);

    foreach (const QMailMessageId &id, ids)
        rv &= copyMessage(id);

    return rv;
}

bool MessageFolder::deleteMessage(const QMailMessageId &id)
{
    return QMailStore::instance()->removeMessage(id, QMailStore::CreateRemovalRecord);
}

bool MessageFolder::deleteMessages(const QMailMessageIdList& ids)
{
    QMailMessageKey key(ids);
    return QMailStore::instance()->removeMessages(key, QMailStore::CreateRemovalRecord);
}

bool MessageFolder::contains(const QMailMessageId& id) const
{
    return ( messageCount(QMailMessageKey(QMailMessageKey::Id, id)) > 0 );
}

QMailMessageIdList MessageFolder::messages(QMailMessage::MessageType messageType, const SortOrder& order ) const
{
    return messages(MessageStore::messageFilterKey(messageType), order);
}

QMailMessageIdList MessageFolder::messages(quint64 status, 
                                           bool contains,
                                           QMailMessage::MessageType messageType, 
                                           const SortOrder& order) const
{
    return messages(MessageStore::statusFilterKey(status, contains) & MessageStore::messageFilterKey(messageType), order);
}

QMailMessageIdList MessageFolder::messagesFromAccount(const QMailAccount& account,
                                                      QMailMessage::MessageType messageType,
                                                      const SortOrder& order) const
{
    return messages(MessageStore::messageFilterKey(messageType, account.id()), order);
}

QMailMessageIdList MessageFolder::messages(QMailMessageKey queryKey, const SortOrder& order) const
{
    return MessageStore::messages(queryKey & mParentFolderKey, order);
}

uint MessageFolder::messageCount( MailType status, QMailMessage::MessageType type ) const
{
    return messageCount(MessageStore::statusFilterKey(status) & MessageStore::messageFilterKey(type));
}

uint MessageFolder::messageCount( MailType status, QMailMessage::MessageType type, const QMailAccount& account ) const
{
    return messageCount(MessageStore::statusFilterKey(status) & MessageStore::messageFilterKey(type, account.id()));
}

uint MessageFolder::messageCount( QMailMessageKey queryKey ) const
{
    return MessageStore::messageCount(queryKey & mParentFolderKey);
}

void MessageFolder::externalChange()
{
    emit externalEdit( mailbox() );
}

void MessageFolder::folderContentsModified(const QMailFolderIdList& ids)
{
    foreach (const QMailFolderId& id, ids)
        if (id == mFolder.id()) {
            emit contentModified();
            return;
        }
}

void MessageFolder::openMailbox()
{
    if (QMailStore* store = QMailStore::instance()) {
        if (!mFolder.id().isValid()) {
            QMailFolderKey key(QMailFolderKey::Name,mailbox());
            key &= QMailFolderKey(QMailFolderKey::ParentId,QMailFolderId());
            
            QMailFolderIdList folderIdList = QMailStore::instance()->queryFolders(key);
            if(folderIdList.isEmpty()) {
                // create folder
                QMailFolder newFolder(mailbox());
                if(!QMailStore::instance()->addFolder(&newFolder))
                    qWarning() << "Failed to add folder " << mailbox();
                mFolder = newFolder;
            } else { 
                // load folder
                QMailFolderId folderId = folderIdList.first();
                mFolder = QMailFolder(folderId);
            }

            //set the folder key   
            mParentFolderKey = QMailMessageKey(QMailMessageKey::ParentFolderId,mFolder.id());
        }

        // notify when our content is reported to have changed
        connect(store, SIGNAL(folderContentsModified(QMailFolderIdList)), this, SLOT(folderContentsModified(QMailFolderIdList)));
    } 
}

