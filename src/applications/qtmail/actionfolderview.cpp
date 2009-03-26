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

#include "actionfolderview.h"
#include <QtopiaItemDelegate>

ActionFolderView::ActionFolderView(QWidget *parent)
    : FolderView(parent),
      mModel(0),
      mSize(0)
{
    setItemDelegate(new ActionFolderDelegate(this));
}

ActionFolderModel *ActionFolderView::model() const
{
    return mModel;
}

void ActionFolderView::setModel(ActionFolderModel *model)
{
    mModel = model;
    FolderView::setModel(mModel);

    if (!mModel->isEmpty()) {
        setCurrentIndex(mModel->index(0, 0, QModelIndex()));
    }
}

void ActionFolderView::showEvent(QShowEvent* e)
{
    if (!mSize) {
        // Ensure all initial items can fit on screen
        int maxItemHeight = height() / mModel->rowCount(QModelIndex());
        int pixmapMargin = style()->pixelMetric(QStyle::PM_FocusFrameHMargin);

        mSize = qMin(style()->pixelMetric(QStyle::PM_ListViewIconSize), maxItemHeight) - pixmapMargin;
        setIconSize(QSize(mSize, mSize));
    }

    FolderView::showEvent(e);
}

void ActionFolderView::itemActivated(const QModelIndex &index)
{
    if (QMailMessageSet *item = mModel->itemFromIndex(index)) {
        if (qobject_cast<ActionFolderMessageSet*>(item)) {
            emit folderActivated(item);
        } else if (qobject_cast<ComposeActionMessageSet*>(item)) {
            emit composeActionActivated(item);
        } else if (qobject_cast<EmailActionMessageSet*>(item)) {
            emit emailActionActivated(item);
        }
    }
}

void ActionFolderView::setModel(QAbstractItemModel *)
{
    qWarning() << "ActionFolderView requires a model of type: ActionFolderModel!";
}


ActionFolderDelegate::ActionFolderDelegate(ActionFolderView *parent)
    : FolderDelegate(parent)
{
}

