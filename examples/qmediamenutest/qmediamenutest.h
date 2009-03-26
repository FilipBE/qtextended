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

#ifndef QMEDIAMENUTEST_H
#define QMEDIAMENUTEST_H

#include <QWidget>
#include <QAbstractListModel>

#include <QMediaList>
#include <QMediaMenu>

class QMediaMenuItemPrivate;

class CustomMenuItem : public QAbstractMediaMenuItem
{
    Q_OBJECT
public:
    CustomMenuItem(QIcon* icon, QString text, QMediaList* data);

    bool  execute();
    QSize size();
    void  paint(QPainter* painter, const QStyleOptionViewItem& option);
};

class QMediaMenuTest : public QWidget
{
    Q_OBJECT
public:
    QMediaMenuTest( QWidget *parent = 0, Qt::WindowFlags flags = 0 );

    QMediaMenu*  mainmenu;
    QMediaList*  medialist;

protected:
    void keyPressEvent( QKeyEvent *event );
};

#endif
