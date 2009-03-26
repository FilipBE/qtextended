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

#include "emailfolderview.h"

#include <QPainter>


EmailFolderView::EmailFolderView(QWidget *parent)
    : FolderView(parent),
      mModel(0)
{
    setItemDelegate(new EmailFolderDelegate(this));
}

EmailFolderModel *EmailFolderView::model() const
{
    return mModel;
}

void EmailFolderView::setModel(EmailFolderModel *model)
{
    mModel = model;
    FolderView::setModel(model);

    if (!mModel->isEmpty()) {
        setCurrentIndex(mModel->index(0, 0, QModelIndex()));
    }

    // Expand the inbox to show accounts
    QModelIndex inboxIndex(mModel->indexFromFolderId(QMailFolderId(QMailFolder::InboxFolder)));
    expand(inboxIndex);
}

void EmailFolderView::setModel(QAbstractItemModel *)
{
    qWarning() << "EmailFolderView requires a model of type: EmailFolderModel!";
}


EmailFolderDelegate::EmailFolderDelegate(EmailFolderView *parent)
    : FolderDelegate(parent),
      _unsynchronized(false)
{
}

void EmailFolderDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &originalRect, const QString &text) const
{
    if (_unsynchronized) {
        painter->save();
        painter->setOpacity(0.5);
    }

    FolderDelegate::drawDisplay(painter, option, originalRect, text);

    if (_unsynchronized)
        painter->restore();
}

void EmailFolderDelegate::drawDecoration(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QVariant &decoration) const
{
    if (_unsynchronized) {
        painter->save();
        painter->setOpacity(0.5);
    }

    FolderDelegate::drawDecoration(painter, option, rect, decoration);

    if (_unsynchronized)
        painter->restore();
}

void EmailFolderDelegate::init(const QStyleOptionViewItem &option, const QModelIndex &index)
{
    FolderDelegate::init(option, index);

    // Don't show the excess indicators if this item is expanded
    if (static_cast<EmailFolderView*>(_parent)->isExpanded(index)) {
        // Don't show the indicators for hidden counts when they're not hidden
        _statusText.remove(FolderModel::excessIndicator());

        // Don't show an empty unread count
        if (_statusText.startsWith("0/"))
            _statusText.remove(0, 2);

        // Don't show a zero count
        if (_statusText == "0")
            _statusText = QString();
    }

    _unsynchronized = !index.data(EmailFolderModel::FolderSynchronizationEnabledRole).value<bool>();
}

