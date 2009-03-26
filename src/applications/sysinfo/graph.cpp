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

#include <QPainter>
#include <QPixmap>
#include <QBrush>
#include "graph.h"

void GraphData::clear()
{
    names.clear();
    values.resize(0);
}

void GraphData::addItem( const QString &name, int value )
{
    names.append( name );
    values.resize( values.size() + 1 );
    values[values.size()-1] = value;
}

Graph::Graph(QWidget *parent, Qt::WFlags f )
    : QFrame( parent, f )
{
}

void Graph::paintEvent(QPaintEvent *pe) {
    QFrame::paintEvent(pe);
    QPainter p(this);
    drawContents( &p );
}

BarGraph::BarGraph(QWidget *parent, Qt::WFlags f )
    : Graph( parent, f )
{
    setMinimumHeight( 10 );
    setMaximumHeight( 45 );
}

void BarGraph::drawContents( QPainter *p )
{
    int h = contentsRect().height();
    int y = contentsRect().top();

    int total = 0;
    for ( int i = 0; i < data->count(); i++ )
        total += data->value(i);

    int pos = 0;
    p->setPen(Qt::NoPen);
    QRect r = contentsRect();
    for ( int i = 0; i < data->count(); i++ ) {
        int len;
        if ( i == data->count() - 1 || !total )
            len = r.width() - pos;
        else
            len = (uint)data->value(i) * r.width() / total;
        QColor col;
        col.setHsv( i * 360 / data->count(), 255, 255 );
        if ( layoutDirection() == Qt::LeftToRight )
            drawSegment( p, QRect(r.x() + pos, y, len, h), col );
        else
            drawSegment( p, QRect(r.width()+ r.x()-pos-len ,y,len,h), col );
        pos += len;
    }
}

void BarGraph::drawSegment( QPainter *p, const QRect &r, const QColor &c )
{
    if ( QPixmap::defaultDepth() > 8 ) {
        QColor topgrad = c.light(180);

        QLinearGradient grad (QPointF(r.x()+r.width()/2, r.y()),
                QPointF(r.x()+r.width()/2, r.y()+r.height()));
        grad.setColorAt(0,topgrad);
        grad.setColorAt(0.5,c);
        grad.setColorAt(1,topgrad);
        QBrush topBrush(grad);
        p->setBrush(topBrush);
        p->drawRect(r.x(), r.y(), r.width(), r.height());
   } else {
        p->fillRect( r.x(), r.top(), r.width(), r.height(), c );
        p->setPen(Qt::SolidLine);
    }
}


GraphLegend::GraphLegend( QWidget *parent, Qt::WFlags f )
    : QFrame( parent, f )
{
    horz = false;
}

void GraphLegend::setOrientation(Qt::Orientation o)
{
    horz = o == Qt::Horizontal;
}

QSize GraphLegend::sizeHint() const
{
    // For some reason, the painted text is bigger than the normal font
    // so we need to add some extra pixels or the text will get clipped
    int th = fontMetrics().height() + 4;
    int maxw = 0;
    for ( int i = 0; i < data->count(); i++ ) {
        int w = fontMetrics().width( data->name(i) );
        if ( w > maxw )
            maxw = w;
    }
    if ( 0 && horz ) {
        return QSize( maxw * data->count(), th );
    } else {
        return QSize( maxw, th * data->count() );
    }
}

void GraphLegend::setData( const GraphData *p )
{
    data = p;
    int th = fontMetrics().height();
    setMinimumHeight( th * ( horz ? 1 : data->count() ) );
    updateGeometry();
}

void GraphLegend::paintEvent( QPaintEvent *)
{
    QPainter p(this);

    int total = 0;
    for ( int i = 0; i < data->count(); i++ )
        total += data->value(i);

    int tw = width()/data->count()-1;
    int th = height()/(horz ? 1 : data->count());
    if ( th > p.fontMetrics().height() )
        th = p.fontMetrics().height();
    int x = 0;
    int y = 0;
    for ( int i = 0; i < data->count(); i++ ) {
        QColor col;
        col.setHsv( i * 360 / data->count(), 255, 255 );

        p.setBrush( col );
        if ( layoutDirection() == Qt::LeftToRight ) {
            p.drawRect( x+1, y+1, th - 2, th - 2 );
            p.drawText( x+th + 1, y + p.fontMetrics().ascent()+1, data->name(i) );
        } else {
            p.drawRect( width()-(th-2)-1, y+1, th-2, th-2 );
            p.drawText( x+1, y + 1,
                    width()-(th-2)-3, p.fontMetrics().ascent()+1,
                    Qt::AlignLeft, data->name(i) );
        }
        if ( horz ) {
            x += tw;
        } else {
            y += th;
        }
    }
}
