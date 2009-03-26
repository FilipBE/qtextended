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

#ifndef FOLDERDELEGATE_H
#define FOLDERDELEGATE_H

#include "foldermodel.h"

#include <QtopiaItemDelegate>


class QAbstractItemView;
class QScrollBar;

class FolderDelegate : public QtopiaItemDelegate
{
    Q_OBJECT

public:
    using QtopiaItemDelegate::drawDecoration;

    FolderDelegate(QAbstractItemView *parent = 0);
    FolderDelegate(QWidget *parent = 0);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const;
    virtual void drawDecoration(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QVariant &decoration) const;

    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

protected:
    virtual void init(const QStyleOptionViewItem &option, const QModelIndex &index);

    QWidget *_parent;
    QScrollBar *_scrollBar;
    QString _statusText;
};

#endif
