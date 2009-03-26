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

#ifndef FOLDERVIEW_H
#define FOLDERVIEW_H

#include "foldermodel.h"
#include "folderdelegate.h"

#include <QSet>
#include <QSmoothList>


class FolderView : public QSmoothList
{
    Q_OBJECT

public:
    FolderView(QWidget *parent);
    virtual ~FolderView();

    virtual FolderModel *model() const = 0;

    QMailMessageSet *currentItem() const;
    bool setCurrentItem(QMailMessageSet *item);

    QMailAccountId currentAccountId() const;
    bool setCurrentAccountId(const QMailAccountId& id);

    QMailFolderId currentFolderId() const;
    bool setCurrentFolderId(const QMailFolderId& id);

    bool ignoreMailStoreUpdates() const;
    void setIgnoreMailStoreUpdates(bool ignore);

signals:
    void selected(QMailMessageSet *);
    void activated(QMailMessageSet *);
    void backPressed();

protected slots:
    virtual void itemSelected(const QModelIndex &index);
    virtual void itemActivated(const QModelIndex &index);
    virtual void itemExpanded(const QModelIndex &index);
    virtual void itemCollapsed(const QModelIndex &index);
    virtual void currentChanged(const QModelIndex &currentIndex, const QModelIndex &previousIndex);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void showEvent(QShowEvent *e);
    virtual void hideEvent(QHideEvent *e);

    virtual void modelReset();

private:
    template<typename SetType>
    bool expandSet(SetType &ids, FolderModel *model);

    template<typename SetType>
    void removeNonexistent(SetType &ids, FolderModel *model);

    bool expandAccounts(QSet<QMailAccountId> &accountIds, FolderModel *model);
    bool expandFolders(QSet<QMailFolderId> &folderIds, FolderModel *model);

    QSet<QMailAccountId> expandedAccounts;
    QSet<QMailFolderId> expandedFolders;
};

#endif
