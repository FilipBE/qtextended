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

#include "qanalogclock.h"
#include <qtopianamespace.h>

#include <QLayout>
#include <QPainter>
#include <QImage>

#include <QPaintEvent>

#include <math.h>

// Constants
const double deg2rad = 0.017453292519943295769; // pi/180

// ============================================================================
//
// Functions
//
// ============================================================================

/*!
    \internal
*/
static QRect expand(const QRect &r, int e=1)
{
    QRect er(r);
    er = er.adjusted(-e, -e, e, e);

    return er;
}

// ============================================================================
//
// QAnalogClockPrivate
//
// ============================================================================

class QAnalogClockPrivate
{
public:
    QAnalogClockPrivate()
    :   currTime(),
        prevTime(),
        isEvent( true ),
        face(),
        scaledFace(),
        changed()
    {};

    QPoint rotate( QPoint center, QPoint p, int angle );

    QTime currTime;
    QTime prevTime;
    bool isEvent;
    QPixmap face;
    QPixmap scaledFace;
    QRegion changed;
};

/*!
    \internal
*/
QPoint QAnalogClockPrivate::rotate( QPoint c, QPoint p, int a )
{
    double angle = deg2rad * ( - a + 180 );
    double nx = c.x() - ( p.x() - c.x() ) * cos( angle ) -
                ( p.y() - c.y() ) * sin( angle );
    double ny = c.y() - ( p.y() - c.y() ) * cos( angle ) +
                ( p.x() - c.x() ) * sin( angle );
    return QPoint( int(nx), int(ny) );
}

// ============================================================================
//
// QAnalogClock
//
// ============================================================================

/*!
    \class QAnalogClock
    \inpublicgroup QtBaseModule

    \brief The QAnalogClock widget displays an analog clock face.

    The QAnalogClock widget displays an analog clock face.

    The clock is not animated. You must call
    QAnalogClock::display() to display an appropriate time.

    This class exists to ensure consistent appearance of such a widget in Qtopia.

    \ingroup time
*/

/*!
    Creates an analog clock face with the given \a parent.
*/
QAnalogClock::QAnalogClock(QWidget *parent)
:   QFrame( parent ),
    d( 0 )
{
    d = new QAnalogClockPrivate();

    setMinimumSize(50,50);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
}

/*!
    Destroys the analog clock face.
*/
QAnalogClock::~QAnalogClock()
{
    delete d;
}

/*!
    Sets the background image for the clock \a face. The default image is null.
    Hourly tick marks and the clock hands are drawn over this image.
*/
void QAnalogClock::setFace( const QPixmap& face )
{
    d->scaledFace = QPixmap();
    d->face = face;
}

/*!
    \internal
*/
void QAnalogClock::resizeEvent( QResizeEvent *event )
{
    d->scaledFace = QPixmap();
    QFrame::resizeEvent(event);
}

/*!
    \internal
*/
void QAnalogClock::paintEvent( QPaintEvent *event )
{
    QPainter paint(this);
    if ( event->rect().intersects( contentsRect() ) ) {
        paint.setClipRegion( event->region().intersect( contentsRect() ) );
        drawContents( &paint );
    }
}

/*!
    \internal
*/
void QAnalogClock::drawContents( QPainter *pp )
{
    pp->setRenderHint(QPainter::Antialiasing);

    QRect r = contentsRect();
    int size = qMin( r.width(), r.height() );
    QPoint offs( (r.width()-size)/2, (r.height()-size)/2 );
    QRect pr(0, 0, size, size);

    if ( d->scaledFace.isNull() && !d->face.isNull() ) {
        d->scaledFace = d->face.scaled( size,
                                        size,
                                        Qt::IgnoreAspectRatio,
                                        Qt::SmoothTransformation );
    }

    pp->save();
    pp->translate( offs.x(), offs.y() );

    if ( !d->scaledFace.isNull() )
        pp->drawPixmap( 0, 0, d->scaledFace );

    d->isEvent = true;

    pp->setRenderHint( QPainter::Antialiasing );

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

    QColor color(palette().color(QPalette::Text));
    QTime time = d->currTime;

    if ( d->isEvent || d->prevTime.minute() != d->currTime.minute()
            || d->prevTime.hour() != d->currTime.hour()
            || qAbs( d->prevTime.secsTo( d->currTime )) > 1 ) {

        if ( !d->scaledFace.isNull() )
            pp->drawPixmap( 0, 0, d->scaledFace );

        // draw ticks
        if ( d->scaledFace.isNull() ) {
            pp->setPen( QPen( color, w_tick ) );
            for ( int i = 0; i < 12; i++ )
                pp->drawLine( d->rotate( center, l1, i * 30 ),
                              d->rotate( center, l2, i * 30 ) );
        }

        // draw hour pointer
        h1 = d->rotate( center, h1, 30 * ( time.hour() % 12 ) + time.minute() / 2 );
        h2 = d->rotate( center, h2, 30 * ( time.hour() % 12 ) + time.minute() / 2 );
        pp->setPen( color );
        pp->setBrush( color );
        drawHand( pp, h1, h2 );

        // draw minute pointer
        m1 = d->rotate( center, m1, time.minute() * 6 );
        m2 = d->rotate( center, m2, time.minute() * 6 );
        pp->setPen( color );
        pp->setBrush( color );
        drawHand( pp, m1, m2 );
    }

    pp->restore();

    d->prevTime = d->currTime;

    d->changed = QRegion();
    pp->setClipping(false);

    // draw second pointer
    s1 = d->rotate( center, s1, time.second() * 6 );
    s2 = d->rotate( center, s2, time.second() * 6 );
    QRect sr = QRect(s1, s2).normalized();
    pp->setPen( QPen( color, w_sec ) );
    pp->drawLine( s1+offs, s2+offs );

    // cap
    pp->setBrush(color);
    pp->drawEllipse( center.x()-w_hour/2+offs.x(), center.y()-w_hour/2+offs.y(), w_hour, w_hour );

    d->changed = expand(sr);
    d->isEvent = true;
}

// Dijkstra's bisection algorithm to find the square root as an integer.

/*!
    \internal
*/
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

/*!
    \internal
*/
void QAnalogClock::drawHand( QPainter *p, QPoint p1, QPoint p2 )
{
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
}

/*!
    Displays the \a time on the clock.
*/
void QAnalogClock::display( const QTime& time )
{
    d->currTime = time;
    if  ( isVisible() )
        d->isEvent = false; // forced to true before used anyway
    repaint();
}
