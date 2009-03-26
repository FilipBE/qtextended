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

#include "navigator.h"

#include "imageui.h"

#include <qpainter.h>
#include <qbrush.h>
#include <qregion.h>

#include <QKeyEvent>
#include <QStyle>

Navigator::Navigator( ImageUI* iui, QWidget* parent, Qt::WFlags f )
    : QWidget( parent, f ), image_ui( iui ), valid( false )
{
    setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum ) );
    setFocusPolicy( Qt::StrongFocus );

    // When image ui has changed update viewport and space
    connect( image_ui, SIGNAL(changed()), this, SLOT(updateNavigator()) );

    // Update display when image ui updated
    connect( image_ui, SIGNAL(updated()), this, SLOT(update()) );

    // When navigator moved notify image ui of movement
    connect( this, SIGNAL(viewportMoved(int,int)),
        image_ui, SLOT(moveViewportBy(int,int)) );
}

void Navigator::updateNavigator()
{
    // Retrive and set space
    actual_space = QRect( QPoint( 0, 0 ), image_ui->space() );
    // Retrive and set viewport
    actual_viewport = image_ui->viewport();

    // Update display
    if ( !actual_viewport.contains( actual_space ) && actual_viewport != actual_space ) {
        calculateReduced();
        valid = true;
    } else {
        valid = false;
    }

    update();
}

void Navigator::keyPressEvent( QKeyEvent* e )
{
#define MOVE_STEP 1

    // Record direction and start viewport movement
    switch( e->key() ) {
    case Qt::Key_Right:
        moveViewportBy(int(MOVE_STEP / reduction_ratio), 0);
        break;
    case Qt::Key_Left:
        moveViewportBy(int(-MOVE_STEP / reduction_ratio), 0);
        break;
    case Qt::Key_Up:
        moveViewportBy(0, int(-MOVE_STEP / reduction_ratio));
        break;
    case Qt::Key_Down:
        moveViewportBy(0, int(MOVE_STEP / reduction_ratio));
        break;
    default:
        // Ignore
        e->ignore();
        break;
    }
}

void Navigator::mousePressEvent( QMouseEvent* e )
{
    // If stylus pressed
    if( e->button() == Qt::LeftButton ) {
        // Record position and allow viewport to be moved
        mouse_position = e->pos();
        moving_viewport = true;
    }
}

void Navigator::mouseReleaseEvent( QMouseEvent* /*e*/ )
{
    // Disallow viewport to be moved
    moving_viewport = false;
}

void Navigator::mouseMoveEvent( QMouseEvent* e )
{
    // If viewport can be moved and mouse is in space
    if( moving_viewport ) {
        // Calculate displacement and move viewport
        QPoint delta(mouse_position - e->pos());
        moveViewportBy( delta.x(), delta.y() );
        mouse_position = e->pos();
    }
}

void Navigator::paintEvent( QPaintEvent* )
{
#define PAINTER_COLOR Qt::white
#define SPACE_FILL_PATTERN Qt::Dense6Pattern
#define PEN_WIDTH 1

    if ( valid )  {
        QPainter painter( this );
        // Draw navigator onto widget
        painter.setPen( PAINTER_COLOR );
        // painter.setRasterOp( Qt::XorROP );

        // Draw reduced viewport
        painter.setBrush( QBrush() );
        painter.drawRect( reduced_viewport.adjusted( 0, 0, -PEN_WIDTH, -PEN_WIDTH ) );

        // Draw reduced space border
        painter.setClipRegion( QRegion( rect() ).subtract( reduced_viewport ) );
        painter.setClipping( true );
        painter.setBrush( QBrush( PAINTER_COLOR, SPACE_FILL_PATTERN ) );
        painter.drawRect( reduced_space.adjusted( 0, 0, -PEN_WIDTH, -PEN_WIDTH) );
    }
}

QSize Navigator::sizeHint() const
{
    return reduced_space.size();
}

void Navigator::calculateReduced()
{
#define PREFERRED_WIDTH 65
#define PREFERRED_HEIGHT 65
#define REDUCTION_RATIO( dw, dh, sw, sh ) \
    ( (dw)*(sh) > (dh)*(sw) ? (double)(dh)/(double)(sh) : \
    (double)(dw)/(double)(sw) )

    // If viewport is wider than space, reduce viewport to fit width
    // Otherwise if viewport is taller than space, reduce viewport to fit height
    if( actual_viewport.width() > actual_space.width() ) {
        actual_viewport.setLeft( actual_space.left() );
        actual_viewport.setRight( actual_space.right() );
    } else if( actual_viewport.height() > actual_space.height() ) {
        actual_viewport.setTop( actual_space.top() );
        actual_viewport.setBottom( actual_space.bottom() );
    }

    // Reduce viewport to fit within widget dimensions
    reduction_ratio = REDUCTION_RATIO( PREFERRED_WIDTH, PREFERRED_HEIGHT,
        actual_space.width(), actual_space.height() );
    // Reduce and center space
    reduced_space = QRect( actual_space.topLeft() * reduction_ratio,
        actual_space.bottomRight() * reduction_ratio );

    centered_origin.setX( width() - reduced_space.width() - style()->pixelMetric( QStyle::PM_LayoutRightMargin ) );
    centered_origin.setY( height() - reduced_space.height() - style()->pixelMetric( QStyle::PM_LayoutBottomMargin ) );
    reduced_space.translate( centered_origin.x(), centered_origin.y() );

    // Reduce and center viewport by same amount
    reduced_viewport = QRect( actual_viewport.topLeft() * reduction_ratio,
        actual_viewport.bottomRight() * reduction_ratio );
    reduced_viewport.translate( centered_origin.x(), centered_origin.y() );
}

void Navigator::moveViewportBy( int dx, int dy )
{
    // If viewport is wider than space, don't move horizontally
    // Otherwise, restrict dx to within actual space
    if( actual_viewport.width() >= actual_space.width() ) dx = 0;
    else {
        if( dx + actual_viewport.right() > actual_space.right() )
            dx = actual_space.right() - actual_viewport.right();
        else if( dx + actual_viewport.left() < actual_space.left() )
            dx = actual_space.left() - actual_viewport.left();
    }

    // If viewport is taller than space, don't move vertically
    // Otherwise, restrict dy to within actual space
    if( actual_viewport.height() >= actual_space.height() ) dy = 0;
    else {
        if( dy + actual_viewport.bottom() > actual_space.bottom() )
            dy = actual_space.bottom() - actual_viewport.bottom();
        else if( dy + actual_viewport.top() < actual_space.top() )
            dy = actual_space.top() - actual_viewport.top();
    }

    // If viewport needs to be move, move viewports
    if( dx | dy ) {
        actual_viewport.translate( dx, dy );

        // Calculate reduced viewport
        reduced_viewport = QRect( actual_viewport.topLeft() * reduction_ratio,
            actual_viewport.bottomRight() * reduction_ratio );
        reduced_viewport.translate( centered_origin.x(), centered_origin.y() );

        // Emit viewport moved signal
        emit viewportMoved( dx, dy );

        // Update display
        update();
    }
}
