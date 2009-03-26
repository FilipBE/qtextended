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

#include "e3_clock.h"
#include <QPainter>
#include <QImage>
#include <QPaintEvent>
#include <QDebug>

#include <math.h>
#include <limits.h>

const double deg2rad = 0.017453292519943295769; // pi/180

static QRect expand(const QRect &r, int e=1)
{
    QRect er(r);
    er = er.adjusted(-e, -e, e, e);

    return er;
}

E3Clock::E3Clock(QWidget *parent, Qt::WFlags f)
    : QWidget(parent, f), drawSeconds(false), showCurrent(false)
{
    setMinimumSize(50,50);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    showTime(QTime::currentTime());
}

void E3Clock::paintEvent( QPaintEvent *event )
{
    QPainter paint(this);
    if (event->rect().intersects(contentsRect())) {
        paint.setClipRegion(event->region().intersect(contentsRect()));
        drawContents(&paint);
    }
}

void E3Clock::drawContents( QPainter *pp )
{
    pp->setRenderHint(QPainter::Antialiasing);

    QRect r = contentsRect();
    int size = qMin(r.width(), r.height());
    QPoint offs((r.width()-size)/2, (r.height()-size)/2);
    QRect pr(0, 0, size, size);

    pp->save();
    pp->translate(offs.x(), offs.y());

    isEvent = true;

    QPoint center( size / 2, size / 2 );

    const int w_tick = pr.width()/300+1;
    const int w_sec = pr.width()/400+1;
    const int w_hour = pr.width()/80+1;

    QPoint l1( pr.x() + pr.width() / 2, pr.y() + 2 );
    QPoint l2( pr.x() + pr.width() / 2, pr.y() + 6 );

    QPoint h1( pr.x() + pr.width() / 2, pr.y() + pr.height() / 4 );
    QPoint h2( pr.x() + pr.width() / 2, pr.y() + pr.height() / 2 );

    QPoint m1( pr.x() + pr.width() / 2, pr.y() + pr.height() / 9 );
    QPoint m2( pr.x() + pr.width() / 2, pr.y() + pr.height() / 2 );

    QPoint s1( pr.x() + pr.width() / 2, pr.y() + 8 );
    QPoint s2( pr.x() + pr.width() / 2, pr.y() + pr.height() / 2 );

    QColor color(palette().color(QPalette::Foreground));
    QTime time = currTime;

    if ( isEvent || prevTime.minute() != currTime.minute()
            || prevTime.hour() != currTime.hour()
            || qAbs(prevTime.secsTo(currTime)) > 1 ) {

        // draw ticks
        /*
        if (facePm.isNull()) {
            pp->setPen( QPen( color, w_tick ) );
            for ( int i = 0; i < 12; i++ )
                pp->drawLine( rotate( center, l1, i * 30 ), rotate( center, l2, i * 30 ) );
        }
        */
        pp->setPen(QPen(color, w_tick));
        pp->drawLine(l1, l2);

        // draw hour pointer
        h1 = rotate( center, h1, 30 * ( time.hour() % 12 ) + time.minute() / 2 );
        h2 = rotate( center, h2, 30 * ( time.hour() % 12 ) + time.minute() / 2 );
        pp->setPen( color );
        pp->setBrush( color );
        drawHand( pp, h1, h2, 2 );

        // draw minute pointer
        m1 = rotate( center, m1, time.minute() * 6 );
        m2 = rotate( center, m2, time.minute() * 6 );
        pp->setPen( color );
        pp->setBrush( color );
        drawHand( pp, m1, m2, 1 );

        // draw border
        pp->setPen(QPen(palette().color(QPalette::Foreground), 2));
        pp->setBrush(QBrush());
        pp->drawRoundRect(pr, 2000/pr.width(), 2000/pr.height());
    }

    pp->restore();

    prevTime = currTime;

    changed = QRegion();
    pp->setClipping(false);

    // draw second pointer
    if (drawSeconds) {
        s1 = rotate( center, s1, time.second() * 6 );
        s2 = rotate( center, s2, time.second() * 6 );
        QRect sr = QRect(s1, s2).normalized();
        pp->setPen( QPen( color, w_sec ) );
        pp->drawLine( s1+offs, s2+offs );
        changed = expand(sr);
    }

    // cap
    pp->setBrush(color);
    pp->drawEllipse( center.x()-w_hour/2+offs.x(), center.y()-w_hour/2+offs.y(), w_hour, w_hour );

    isEvent = true;
}

// Dijkstra's bisection algorithm to find the square root as an integer.

static uint int_sqrt(uint n)
{
    if ( n >= UINT_MAX>>2 ) // n must be in the range 0...UINT_MAX/2-1
        return 2*int_sqrt( n/4 );
    uint h, p= 0, q= 1, r= n;
    while ( q <= n )
        q <<= 2;
    while ( q != 1 ) {
        q >>= 2;
        h= p + q;
        p >>= 1;
        if ( r >= h ) {
            p += q;
            r -= h;
        }
    }
    return p;
}

void E3Clock::drawHand( QPainter *p, QPoint p1, QPoint p2, int width )
{
/*
    int hw = 7;
    if ( contentsRect().height() < 100 )
        hw = 5;

    int dx = p2.x() - p1.x();
    int dy = p2.y() - p1.y();
    int w = dx*dx+dy*dy;
    int ix,iy;
    w = int_sqrt(w*256);
    iy = w ? (hw * dy * 16)/ w : dy ? 0 : hw;
    ix = w ? (hw * dx * 16)/ w : dx ? 0 : hw;

    // rounding dependent on sign
    int nix, niy;
    if ( ix < 0 ) {
        nix = ix/2;
        ix = (ix-1)/2;
    } else {
        nix = (ix+1)/2;
        ix = ix/2;
    }
    if ( iy < 0 ) {
        niy = iy/2;
        iy = (iy-1)/2;
    } else {
        niy = (iy+1)/2;
        iy = iy/2;
    }

    QPolygon pa(4);
    pa[0] = p1;
    pa[1] = QPoint( p2.x()+iy, p2.y()-nix );
    pa[2] = QPoint( p2.x()-niy, p2.y()+ix );
    pa[3] = p1;

    p->drawPolygon( pa );
*/
    QPen pen(p->pen());
    pen.setWidth(width);
    p->setPen(pen);
    p->drawLine(p1, p2);
}

void E3Clock::showTime( const QTime& t )
{
    currTime = t;
    if(isVisible()) isEvent = false;
    repaint();
    timer.stop();
    showCurrent = false;
}

void E3Clock::showCurrentTime()
{
    currTime = QTime::currentTime();
    if (isVisible()) isEvent = false;
    repaint();
    timer.start(drawSeconds ? 1000 : 60000, this);
    showCurrent = true;
}

void E3Clock::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == timer.timerId()) {
        currTime = QTime::currentTime();
        if (isVisible()) isEvent = false;
        repaint();
    }
}

void E3Clock::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    if (showCurrent && !timer.isActive())
        showCurrentTime();
}

void E3Clock::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);
    timer.stop();
}

QPoint E3Clock::rotate( QPoint c, QPoint p, int a )
{
    double angle = deg2rad * ( - a + 180 );
    double nx = c.x() - ( p.x() - c.x() ) * cos( angle ) -
                ( p.y() - c.y() ) * sin( angle );
    double ny = c.y() - ( p.y() - c.y() ) * cos( angle ) +
                ( p.x() - c.x() ) * sin( angle );
    return QPoint( int(nx), int(ny) );
}

