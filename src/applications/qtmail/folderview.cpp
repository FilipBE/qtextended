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

#include "folderview.h"

#include <QKeyEvent>


FolderView::FolderView(QWidget *parent)
    : QSmoothList(parent)
{
    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(itemActivated(QModelIndex)));
    connect(this, SIGNAL(expanded(QModelIndex)), this, SLOT(itemExpanded(QModelIndex)));
    connect(this, SIGNAL(collapsed(QModelIndex)), this, SLOT(itemCollapsed(QModelIndex)));
    connect(this, SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(currentChanged(QModelIndex, QModelIndex)));
}

FolderView::~FolderView()
{
}

QMailMessageSet *FolderView::currentItem() const
{
    if (FolderModel *folderModel = model())
        return folderModel->itemFromIndex(currentIndex());

    return 0;
}

bool FolderView::setCurrentItem(QMailMessageSet *item)
{
    if (FolderModel *folderModel = model()) {
        QModelIndex index(folderModel->indexFromItem(item));
        if (index.isValid()) {
            setCurrentIndex(index);
            return true;
        }
    }

    return false;
}

QMailAccountId FolderView::currentAccountId() const
{
    if (FolderModel *folderModel = model())
        return folderModel->accountIdFromIndex(currentIndex());

    return QMailAccountId();
}

bool FolderView::setCurrentAccountId(const QMailAccountId& id)
{
    if (FolderModel *folderModel = model()) {
        QModelIndex index(folderModel->indexFromAccountId(id));
        if (index.isValid()) {
            setCurrentIndex(index);
            return true;
        }
    }

    return false;
}

QMailFolderId FolderView::currentFolderId() const
{
    if (FolderModel *folderModel = model())
        return folderModel->folderIdFromIndex(currentIndex());

    return QMailFolderId();
}

bool FolderView::setCurrentFolderId(const QMailFolderId& id)
{
    if (FolderModel *folderModel = model()) {
        QModelIndex index(folderModel->indexFromFolderId(id));
        if (index.isValid()) {
            setCurrentIndex(index);
            return true;
        }
    }

    return false;
}

bool FolderView::ignoreMailStoreUpdates() const
{
    if (FolderModel *folderModel = model())
        return folderModel->ignoreMailStoreUpdates();

    return false;
}

void FolderView::setIgnoreMailStoreUpdates(bool ignore)
{
    if (FolderModel *folderModel = model())
        folderModel->setIgnoreMailStoreUpdates(ignore);
}

void FolderView::itemActivated(const QModelIndex &index)
{
    if (FolderModel *folderModel = model())
        if (QMailMessageSet *item = folderModel->itemFromIndex(index))
            emit activated(item);
}

void FolderView::itemSelected(const QModelIndex &index)
{
    if (FolderModel *folderModel = model())
        if (QMailMessageSet *item = folderModel->itemFromIndex(index))
            emit selected(item);
}

void FolderView::itemExpanded(const QModelIndex &index)
{
    if (FolderModel *folderModel = model()) {
        QMailFolderId folderId = folderModel->folderIdFromIndex(index);
        if (folderId.isValid()) {
            expandedFolders.insert(folderId);
        } else {
            QMailAccountId accountId = folderModel->accountIdFromIndex(index);
            if (accountId.isValid()) {
                expandedAccounts.insert(accountId);
            }
        }
    }
}

void FolderView::itemCollapsed(const QModelIndex &index)
{
    if (FolderModel *folderModel = model()) {
        QMailFolderId folderId = folderModel->folderIdFromIndex(index);
        if (folderId.isValid()) {
            expandedFolders.remove(folderId);
        } else {
            QMailAccountId accountId = folderModel->accountIdFromIndex(index);
            if (accountId.isValid()) {
                expandedAccounts.remove(accountId);
            }
        }
    }
}

void FolderView::currentChanged(const QModelIndex &currentIndex, const QModelIndex &previousIndex)
{
    itemSelected(currentIndex);

    Q_UNUSED(previousIndex)
}

namespace {

template<typename IdType>
QModelIndex itemIndex(const IdType &id, FolderModel *folderModel);

template<>
QModelIndex itemIndex<QMailAccountId>(const QMailAccountId &id, FolderModel *folderModel)
{
    return folderModel->indexFromAccountId(id);
}

template<>
QModelIndex itemIndex<QMailFolderId>(const QMailFolderId &id, FolderModel *folderModel)
{
    return folderModel->indexFromFolderId(id);
}

}

template<typename SetType>
bool FolderView::expandSet(SetType &ids, FolderModel *model)
{
    int originalCount = ids.count();
    int count = originalCount;
    int oldCount = count + 1;

    while (count && (count < oldCount)) {
        oldCount = count;

        typename SetType::iterator it = ids.begin();
        while (it != ids.end()) {
            QModelIndex index(itemIndex(*it, model));
            if (index.isValid()) {
                if (!isExpanded(index))
                    expand(index);

                if (isExpanded(index)) {
                    // We no longer need to expand this folder
                    it = ids.erase(it);
                    --count;
                } else {
                    ++it;
                }
            } else {
                ++it;
            }
        }
    }

    return (count != originalCount);
}

template<typename SetType>
void FolderView::removeNonexistent(SetType &ids, FolderModel *model)
{
    typename SetType::iterator it = ids.begin();
    while (it != ids.end()) {
        QModelIndex index(itemIndex(*it, model));
        if (!index.isValid()) {
            it = ids.erase(it);
        } else {
            ++it;
        }
    }
}

bool FolderView::expandFolders(QSet<QMailFolderId> &folderIds, FolderModel *model)
{
    return expandSet(folderIds, model);
}

bool FolderView::expandAccounts(QSet<QMailAccountId> &accountIds, FolderModel *model)
{
    return expandSet(accountIds, model);
}

void FolderView::modelReset()
{
    QSmoothList::modelReset();

    if (FolderModel *folderModel = model()) {
        // Remove any items that are no longer in the model
        removeNonexistent(expandedAccounts, folderModel);
        removeNonexistent(expandedFolders, folderModel);

        // Ensure all the expanded items are re-expanded
        QSet<QMailAccountId> accountIds(expandedAccounts);
        QSet<QMailFolderId> folderIds(expandedFolders);

        bool itemsExpanded(false);
        do {
            itemsExpanded = false;

            // We need to repeat this process, because many items cannot be expanded until
            // their parent item is expanded...
            itemsExpanded |= expandAccounts(accountIds, folderModel);
            itemsExpanded |= expandFolders(folderIds, folderModel);
        } while (itemsExpanded);

        // Any remainining IDs must not be accessible in the model any longer
        foreach (const QMailAccountId &accountId, accountIds)
            expandedAccounts.remove(accountId);

        foreach (const QMailFolderId &folderId, folderIds)
            expandedFolders.remove(folderId);
    }
}

void FolderView::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) 
    {
    case Qt::Key_Select:
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:

        e->accept();
        itemActivated(currentIndex());
        break;

    case Qt::Key_Back:
        e->accept();
        emit backPressed();
        break;

    default:  
        QSmoothList::keyPressEvent(e);
    }
}

void FolderView::showEvent(QShowEvent *e)
{
    setIgnoreMailStoreUpdates(false);
    QSmoothList::showEvent(e);
}

void FolderView::hideEvent(QHideEvent *e)
{
    setIgnoreMailStoreUpdates(true);
    QSmoothList::hideEvent(e);
}

