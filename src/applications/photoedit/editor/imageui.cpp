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

#include "imageui.h"

#include "imageprocessor.h"

#include <qpainter.h>
#include <qbrush.h>
#include <qcolor.h>

ImageUI::ImageUI( ImageProcessor* ip, QWidget* parent, Qt::WFlags f )
    : QWidget( parent, f ), image_processor( ip ), enabled( true )
{
    // When image changed in image processor update view
    connect( image_processor, SIGNAL(changed()),
        this, SLOT(updateViewport()) );

    setAttribute( Qt::WA_OpaquePaintEvent );
}

QRect ImageUI::viewport( const QRect& rect ) const
{
    QRect area( rect );
    // Displace area by position of viewport
    area.translate( _viewport.x(), _viewport.y() );
    return area;
}

void ImageUI::reset()
{
    _viewport.moveCenter( _space.center() );
    viewport_center = image_processor->unmap( _viewport.center() );
    emit changed();
    update();
}

QRegion ImageUI::region() const
{
    QRect region( preview.rect() );
    region.moveCenter(rect().center());
    return region;
}

void ImageUI::moveViewportBy( int dx, int dy )
{
    _viewport.translate( dx, dy );
    viewport_center = image_processor->unmap( _viewport.center() );

    // Reload preview from image processor
    update();
}

void ImageUI::updateViewport()
{
    // Update space dimensions
    _space = QRect( QPoint( 0, 0 ), image_processor->size() );

    // Initialize viewport
    _viewport.moveCenter( image_processor->map( viewport_center ) );
    if( _viewport.width() > _space.width() )
        _viewport.moveCenter(
            QPoint( _space.center().x(), _viewport.center().y() ) );
    if( _viewport.height() > _space.height() )
        _viewport.moveCenter(
            QPoint( _viewport.center().x(), _space.center().y() ) );
    containViewport();

    // Emit changed signal
    emit changed();

    // Reload preview from image processor
    update();
}

void ImageUI::paintEvent( QPaintEvent* )
{
#define BACKGROUND_COLOR Qt::lightGray
#define PATTERN Qt::BDiagPattern
#define PATTERN_COLOR QColor( Qt::lightGray ).light( 110 )

    QPainter painter( this );

    // Draw background onto widget buffer
    painter.fillRect( rect(), BACKGROUND_COLOR );
    painter.fillRect( rect(), QBrush( PATTERN_COLOR, PATTERN ) );

    if( enabled ) {
        preview = image_processor->preview( _viewport );
        if ( !(preview.isNull()) ) {
            // Update image position
            // Draw preview on widget buffer
            QRect previewRect(QPoint(0, 0), preview.size());
            previewRect.moveCenter(rect().center());

            painter.drawPixmap(previewRect, preview);
        }
    }
}

void ImageUI::resizeEvent( QResizeEvent* )
{
    // Resize viewport while maintaining center and contain viewport
    QPoint viewport_center( _viewport.center() );
    _viewport.setWidth( width() );
    _viewport.setHeight( height() );
    _viewport.moveCenter( viewport_center );
    containViewport();

    emit changed();

    // Reload preview from image processor
    update();
}

void ImageUI::containViewport()
{
    // If viewport is thinner than space, contain horizontally
    if( _viewport.width() <= _space.width() ) {
        // If viewport beyond the left bound, move within the left
        if( _viewport.left() < _space.left() )
            _viewport.translate( _space.left() - _viewport.left(), 0 );
        // Otherwise, if viewport beyond the right bound, move within the right
        else if( _viewport.right() > _space.right() )
            _viewport.translate( _space.right() - _viewport.right(), 0 );
    }

    // If viewport is shorter than space, contain vertically
    if( _viewport.height() <= _space.height() ) {
        // If viewport beyond the upper bound, move within the upper
        if( _viewport.top() < _space.top() )
            _viewport.translate( 0, _space.top() - _viewport.top() );
        // Otherwise, if viewport beyond the lower bound, move within the lower
        else if( _viewport.bottom() > _space.bottom() )
            _viewport.translate( 0, _space.bottom() - _viewport.bottom() );
    }
}
