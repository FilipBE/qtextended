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

#ifndef E3_CLOCK_H
#define E3_CLOCK_H

#include <QWidget>
#include <QTime>
#include <QBasicTimer>

class E3Clock : public QWidget
{
    Q_OBJECT
public:
    E3Clock(QWidget *parent=0, Qt::WFlags f=0);

    void showTime( const QTime& time );
    void showCurrentTime();

protected:
    void paintEvent( QPaintEvent *event );
    void drawContents( QPainter *p );
    void drawHand( QPainter *p, QPoint, QPoint, int width );
    void timerEvent(QTimerEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);

private:
    QPoint rotate( QPoint center, QPoint p, int angle );

    QTime currTime;
    QTime prevTime;
    bool isEvent;
    QRegion changed;
    bool drawSeconds;
    QBasicTimer timer;
    bool showCurrent;
};

#endif
