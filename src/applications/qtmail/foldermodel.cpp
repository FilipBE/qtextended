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

#include "foldermodel.h"

#include <QMailAccount>
#include <QMailFolder>
#include <QMailStore>
#include <QApplication>
#include <QTimer>


using QMailDataComparator::Includes;
using QMailDataComparator::Excludes;


FolderModel::FolderModel(QObject *parent)
    : QMailMessageSetModel(parent)
{
}

FolderModel::~FolderModel()
{
}

QVariant FolderModel::data(QMailMessageSet *item, int role, int column) const
{
    if (item) {
        if (role == FolderIconRole) {
            return itemIcon(item);
        } else if (role == FolderStatusRole) {
            return itemStatus(item);
        } else if (role == FolderStatusDetailRole) {
            return itemStatusDetail(item);
        } else if (role == FolderIdRole) {
            return itemFolderId(item);
        }

        return QMailMessageSetModel::data(item, role, column);
    }

    return QVariant();
}

QString FolderModel::excessIndicator()
{
    return "*";
}

void FolderModel::appended(QMailMessageSet *item)
{
    QMailMessageSetModel::appended(item);

    // Determine an initial status for this item
    scheduleUpdate(item);
}

void FolderModel::updated(QMailMessageSet *item)
{
    QMailMessageSetModel::updated(item);

    // See if the status has changed for this item
    scheduleUpdate(item);
}

void FolderModel::removed(QMailMessageSet *item)
{
    QMailMessageSetModel::removed(item);

    updatedItems.removeAll(item);
}

static QMap<QMailFolderId, QIcon> iconMapInit()
{
    QMap<QMailFolderId, QIcon> map;

    map[QMailFolderId(QMailFolder::InboxFolder)] = QIcon(":icon/inbox");
    map[QMailFolderId(QMailFolder::SentFolder)] = QIcon(":icon/sent");
    map[QMailFolderId(QMailFolder::DraftsFolder)] = QIcon(":icon/drafts");
    map[QMailFolderId(QMailFolder::TrashFolder)] = QIcon(":icon/trash");
    map[QMailFolderId(QMailFolder::OutboxFolder)] = QIcon(":icon/outbox");

    return map;
}

static QIcon folderIcon(const QMailFolderId &id)
{
    const QMap<QMailFolderId, QIcon> iconMap(iconMapInit());

    QMap<QMailFolderId, QIcon>::const_iterator it = iconMap.find(id);
    if (it != iconMap.end())
        return it.value();

    return QIcon(":icon/folder");
}

QIcon FolderModel::itemIcon(QMailMessageSet *item) const
{
    if (QMailFolderMessageSet *folderItem = qobject_cast<QMailFolderMessageSet*>(item)) {
        return folderIcon(folderItem->folderId());
    } else if (qobject_cast<QMailAccountMessageSet*>(item)) {
        return QIcon(":icon/account");
    } else if (qobject_cast<QMailFilterMessageSet*>(item)) {
        return QIcon(":icon/find");
    }

    return QIcon();
}

QString FolderModel::itemStatus(QMailMessageSet *item) const
{
    QMap<QMailMessageSet*, StatusText>::const_iterator it = statusMap.find(item);
    if (it != statusMap.end())
        return it->first;

    return QString();
}

QString FolderModel::itemStatusDetail(QMailMessageSet *item) const
{
    QMap<QMailMessageSet*, StatusText>::const_iterator it = statusMap.find(item);
    if (it != statusMap.end())
        return it->second;

    return QString();
}

FolderModel::StatusText FolderModel::itemStatusText(QMailMessageSet *item) const
{
    if (QMailFolderMessageSet *folderItem = qobject_cast<QMailFolderMessageSet*>(item)) {
        return folderStatusText(folderItem);
    } else if (QMailAccountMessageSet *accountItem = qobject_cast<QMailAccountMessageSet*>(item)) {
        return accountStatusText(accountItem);
    } else if (QMailFilterMessageSet *filterItem = qobject_cast<QMailFilterMessageSet*>(item)) {
        return filterStatusText(filterItem);
    }

    return qMakePair(QString(), QString());
}

static QString formatCounts(int total, int unread, bool excessTotal = false, bool excessUnread = false)
{
    QString countStr;

    if (total || excessTotal || excessUnread) {
        if (unread || excessUnread) {
            QString unreadIndicator(excessUnread ? FolderModel::excessIndicator() : "");
            QString totalIndicator(excessTotal ? FolderModel::excessIndicator() : "");

            if (QApplication::isRightToLeft())
                countStr.append(QString("%1%2/%3%4").arg(total).arg(totalIndicator).arg(unread).arg(unreadIndicator));
            else
                countStr.append(QString("%1%2/%3%4").arg(unread).arg(unreadIndicator).arg(total).arg(totalIndicator));
        } else {
            countStr.append(QString("%1%2").arg(total).arg(excessTotal ? FolderModel::excessIndicator() : ""));
        }
    }

    return countStr;
}

QString FolderModel::describeFolderCount(int total, int subTotal, SubTotalType type)
{
    QString desc(QString::number(total));

    if (total && subTotal) {
        if (type == New) {
            desc += tr(" (%n new)", "%1 = number of new messages", subTotal);
        } else if (type == Unsent) {
            desc += tr(" (%n unsent)", "%1 = number of unsent messages", subTotal);
        } else if (type == Unread) {
            desc += tr(" (%n unread)", "%1 = number of unread messages", subTotal);
        }
    }

    return desc;
}

static QMailMessageKey unreadKey() 
{
    // Both 'read' and 'read-elsewhere' mean !unread
    return (QMailMessageKey(QMailMessageKey::Status, QMailMessage::Read, Excludes) &
            QMailMessageKey(QMailMessageKey::Status, QMailMessage::ReadElsewhere, Excludes));
}

FolderModel::StatusText FolderModel::folderStatusText(QMailFolderMessageSet *item) const
{
    static const QMailFolderId inboxFolderId(QMailFolder::InboxFolder);
    static const QMailFolderId trashFolderId(QMailFolder::TrashFolder);
    static const QMailFolderId draftsFolderId(QMailFolder::DraftsFolder);

    QString status, detail;

    if (QMailStore* store = QMailStore::instance()) {
        int inclusiveTotal = 0;
        int inclusiveUnreadTotal = 0;

        // Find the total and unread total for this folder
        QMailMessageKey itemKey = item->messageKey();
        int total = store->countMessages(itemKey);
        int unreadTotal = store->countMessages(itemKey & unreadKey());
        
        // Find the subtotal for this folder
        int subTotal = 0;
        SubTotalType type = Unread;

        QMailFolderId folderId(item->folderId());
        if ((folderId == inboxFolderId) || (folderId == trashFolderId)) {
            // For Inbox and Trash, report the 'new' count, or the 'unread' count
            subTotal = store->countMessages(itemKey & QMailMessageKey(QMailMessageKey::Status, QMailMessage::New, Includes));
            type = New;
        } else if (folderId == draftsFolderId) {
            // For Drafts, report the 'unsent' count, but not the 'unread' count
            subTotal = store->countMessages(itemKey & QMailMessageKey(QMailMessageKey::Status, QMailMessage::Sent, Excludes));
            type = Unsent;
            unreadTotal = 0;
        } else {
            // Determine whether there are messages lower in the hierarchy
            QMailMessageKey inclusiveKey = item->descendantsMessageKey();
            inclusiveTotal = total + store->countMessages(inclusiveKey);

            if (inclusiveTotal > total) {
                inclusiveUnreadTotal = unreadTotal + store->countMessages(inclusiveKey & unreadKey());
            }
        }

        if (subTotal) {
            detail = describeFolderCount(total, subTotal, type);
        } else {
            detail = describeFolderCount(total, unreadTotal, Unread);
        }

        status = formatCounts(total, unreadTotal, (inclusiveTotal > total), (inclusiveUnreadTotal > unreadTotal));
    }

    return qMakePair(status, detail);
}

FolderModel::StatusText FolderModel::accountStatusText(QMailAccountMessageSet *item) const
{
    QString status, detail;

    if (QMailStore* store = QMailStore::instance()) {
        QMailMessageKey itemKey = item->messageKey();
        int total = store->countMessages(itemKey);

        if (total) {
            // Find the unread total for this account
            int unreadTotal = store->countMessages(itemKey & unreadKey());
            
            // See if there are 'new' messages for this account
            int subTotal = store->countMessages(itemKey & QMailMessageKey(QMailMessageKey::Status, QMailMessage::New, Includes));
            if (subTotal) {
                detail = describeFolderCount(total, subTotal, New);
            } else {
                detail = formatCounts(total, unreadTotal, false, false);
            }

            status = formatCounts(total, unreadTotal, false, false);
        } else {
            detail = QString::number(0);
        }
    }

    return qMakePair(status, detail);
}

FolderModel::StatusText FolderModel::filterStatusText(QMailFilterMessageSet *item) const
{
    QString status, detail;

    if (QMailStore* store = QMailStore::instance()) {
        QMailMessageKey itemKey = item->messageKey();
        int total = store->countMessages(itemKey);

        if (total) {
            // Find the unread total for this set
            int unreadTotal = store->countMessages(itemKey & unreadKey());
            
            detail = describeFolderCount(total, unreadTotal);
            status = formatCounts(total, unreadTotal, false, false);
        } else {
            detail = QString::number(0);
        }
    }

    return qMakePair(status, detail);
}

void FolderModel::scheduleUpdate(QMailMessageSet *item)
{
    if (updatedItems.isEmpty()) {
        QTimer::singleShot(0, this, SLOT(processUpdatedItems()));
    } else if (updatedItems.contains(item)) {
        return;
    }

    updatedItems.append(item);
}

void FolderModel::processUpdatedItems()
{
    // Note: throughput can be increased at a cost to interactivity by increasing batchSize:
    const int batchSize = 1;

    // Only process a small number before returning to the event loop
    int count = 0;
    while (!updatedItems.isEmpty() && (count < batchSize)) {
        QMailMessageSet *item = updatedItems.takeFirst();

        FolderModel::StatusText text = itemStatusText(item);
        if (text != statusMap[item]) {
            statusMap[item] = text;
            emit dataChanged(item->modelIndex(), item->modelIndex());
        }

        ++count;
    }

    if (!updatedItems.isEmpty())
        QTimer::singleShot(0, this, SLOT(processUpdatedItems()));
}


