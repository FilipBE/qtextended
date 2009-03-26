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

#ifndef QAPPOINTMENTVIEW_H
#define QAPPOINTMENTVIEW_H

#include <QStyleOptionViewItem>
#include <QAbstractItemDelegate>

class QFont;
class QTOPIAPIM_EXPORT QAppointmentDelegate : public QAbstractItemDelegate
{
public:
    QAppointmentDelegate( QObject * parent = 0 );
    virtual ~QAppointmentDelegate();

    virtual void paint(QPainter *painter, const QStyleOptionViewItem & option,
            const QModelIndex & index ) const;

    virtual QSize sizeHint(const QStyleOptionViewItem & option,
            const QModelIndex &index) const;

    virtual QFont mainFont(const QStyleOptionViewItem &) const;
    virtual QFont secondaryFont(const QStyleOptionViewItem &) const;
private:
    QFont differentFont(const QFont& start, int step) const;

    int iconSize;
};

#endif
