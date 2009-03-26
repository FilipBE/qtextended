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
#include "zoomslider.h"
#include <QPainter>
#include <QPaintEvent>
#include <QApplication>
#include <QDebug>
#include <QStyle>

ZoomSlider::ZoomSlider(QWidget *parent)
    : QAbstractSlider(parent)
    , m_opacity(100)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    //setFixedWidth( 24 );
    //setMinimumHeight( width()*5 );
}

ZoomSlider::~ZoomSlider()
{
}

QSize ZoomSlider::sizeHint() const
{
    int margin = style()->pixelMetric( QStyle::PM_ButtonMargin, 0, this );
    int iconSize = style()->pixelMetric( QStyle::PM_ButtonIconSize, 0, this );
    int w = margin*2 + iconSize;

    return QSize( w, w*5 ).expandedTo(QApplication::globalStrut());
}

void ZoomSlider::paintEvent( QPaintEvent *e )
{
    int w = width();
    int h = height();

    int margin = w/6;

    QPainter p(this);
    p.setClipRegion( e->region() );
    p.setRenderHint( QPainter::Antialiasing );
    p.setOpacity(qreal(m_opacity) / 100.0);

    p.save();
    p.setBrush( QColor(0,0,0,128) );
    p.setPen( QColor(0,0,0,128) );
    p.drawRect( 2, w/2, w-4, h-w );
    p.restore();

    p.setBrush( value() != maximum() ? Qt::white : Qt::lightGray );
    p.drawEllipse( 2, 2, w-4, w-4 );
    p.setBrush( Qt::darkGray );
    p.drawRect( w/2-2, margin, 4,  w-margin*2 );
    p.drawRect( margin, w/2-2, w-margin*2, 4 );

    p.setBrush( value() != minimum() ? Qt::white : Qt::lightGray );
    p.drawEllipse( 2, h-w-2, w-4, w-4 );
    p.setBrush( Qt::darkGray );
    p.drawRect( margin, h-w+w/2-5, w-margin*2, 4 );

    p.setBrush( Qt::white );
    p.setPen( QColor(0,0,0,64) );

    int ticksCount = ( maximum() - minimum() + 1 ) / singleStep();
    if ( ticksCount > 10 )
        ticksCount = ( maximum() - minimum() + 1 ) / pageStep();

    int currentTick = ( value()-minimum() ) * ticksCount / ( maximum() - minimum() + 1 );

    for ( int i=0; i<ticksCount; i++ ) {
        int y = w + (ticksCount-i)*(h-w*2)/(ticksCount+1);
        if ( i==currentTick )
            p.drawRect( margin*2, y-2, w-margin*4, 4 );
        else
            p.drawRect( margin*2, y-1, w-margin*4, 2 );
    }
}


void ZoomSlider::mousePressEvent( QMouseEvent *e )
{
    e->accept();
}

void ZoomSlider::mouseMoveEvent( QMouseEvent *e )
{
    e->accept();
}

void ZoomSlider::mouseReleaseEvent( QMouseEvent *e )
{
    int w = width();
    int h = height();
    int y = e->pos().y();

    if ( QRect(0,0,w,h).contains( e->pos() ) ) {
        if ( y < w ) {
            setValue( value() + singleStep() );
        } else if ( y > h-w ) {
            setValue( value() - singleStep() );
        } else {
            int ticksCount = ( maximum() - minimum() + 1 );
            int tick = ticksCount - (y-w)*(ticksCount+1)/(h-w*2);
            setValue( tick );
        }
        e->accept();
    } else
        QAbstractSlider::mouseReleaseEvent(e);
}

