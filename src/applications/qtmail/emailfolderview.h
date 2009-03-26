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

#ifndef EMAILFOLDERVIEW_H
#define EMAILFOLDERVIEW_H

#include "emailfoldermodel.h"
#include "folderdelegate.h"
#include "folderview.h"

#include <QtopiaItemDelegate>


class EmailFolderView : public FolderView
{
    Q_OBJECT

public:
    EmailFolderView(QWidget *parent);

    virtual EmailFolderModel *model() const;
    void setModel(EmailFolderModel *model);

private:
    virtual void setModel(QAbstractItemModel *model);

    EmailFolderModel *mModel;
};


class EmailFolderDelegate : public FolderDelegate
{
    Q_OBJECT

public:
    using QtopiaItemDelegate::drawDecoration;

    EmailFolderDelegate(EmailFolderView *parent = 0);

    virtual void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const;
    virtual void drawDecoration(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QVariant &decoration) const;

private:
    virtual void init(const QStyleOptionViewItem &option, const QModelIndex &index);

    bool _unsynchronized;
};

#endif
