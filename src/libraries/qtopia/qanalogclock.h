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
#ifndef QANALOGCLOCK_H
#define QANALOGCLOCK_H

#include <QTime>
#include <QFrame>
#include <QPixmap>

#include <qtopiaglobal.h>

class QAnalogClockPrivate;

class QTOPIA_EXPORT QAnalogClock : public QFrame
{
    Q_OBJECT
public:
    explicit QAnalogClock( QWidget *parent = 0 );
    ~QAnalogClock();

    void display( const QTime& time );
    void setFace( const QPixmap& face );

protected:
    void resizeEvent( QResizeEvent *event );
    void paintEvent( QPaintEvent *event );
    void drawContents( QPainter *p );
    void drawHand( QPainter *p, QPoint, QPoint );

private:
    QAnalogClockPrivate* d;
};

#endif
