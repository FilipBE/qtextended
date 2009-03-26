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

#ifndef QFAVORITESERVICESLIST_H
#define QFAVORITESERVICESLIST_H

#include <QSmoothList>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QAbstractItemDelegate>

class QFavoriteServicesListPrivate;

class QFavoriteServicesList : public QSmoothList
{
    Q_OBJECT

    public:
        QFavoriteServicesList(QWidget* parent=0);
        ~QFavoriteServicesList();

        int rowCount();
        void setCurrentRow(int);
    signals:
        void selected(const QModelIndex &index);

    private slots:
        void select(const QModelIndex& index);
        void selectionChange();
        void selectionChanged();
        void selectionChanged(const QModelIndex &current, const QModelIndex& previous);
        void addService();
        void removeCurrentService();
        void moveCurrentService();
        void editCurrentSpeedDial();
        void removeCurrentSpeedDial();

    protected:
        void mousePressEvent(QMouseEvent *event);

    private:
        QFavoriteServicesListPrivate *d;
};

#endif
