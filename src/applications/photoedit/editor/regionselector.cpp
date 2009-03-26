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

#include "regionselector.h"

#include "imageui.h"

#include <qsoftmenubar.h>
#include <qtopianamespace.h>

#include <qpainter.h>
#include <qbrush.h>
#include <qregion.h>

#include <QMouseEvent>
#include <QKeyEvent>

RegionSelector::RegionSelector( ImageUI* iui, Qt::WFlags f )
    : QWidget( iui, f ), image_ui( iui ), enabled( false )
{
    if( Qtopia::mousePreferred() )
        current_state = MARK;
    else
        current_state = MOVE;

    previous_state = current_state;

    // Update display when image ui updated
    connect( image_ui, SIGNAL(updated()), this, SLOT(update()) );
}

QRect RegionSelector::region() const
{
        if( !_region.isNull() &&
            _region.left() != _region.right() &&
            _region.top() != _region.bottom() &&
            !image_ui->viewport().intersect( _region ).isEmpty() )
            return _region;
        return QRect();
}

void RegionSelector::setEnabled( bool b )
{
    enabled = b;

    // If selection enabled, add labels to context bar
    // Otherwise, remove labels from context bar
    if( enabled ) {
        if( Qtopia::mousePreferred() ) {
            // Disable context menu
            QSoftMenuBar::setLabel( this, QSoftMenuBar::menuKey(), QSoftMenuBar::NoLabel );
        } else {
            setStateLabel();
            QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::Select );
        }
        QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Cancel );
    } else {
        QSoftMenuBar::clearLabel( this, QSoftMenuBar::menuKey() );
        if( !Qtopia::mousePreferred() ) QSoftMenuBar::clearLabel( this, Qt::Key_Select );
        QSoftMenuBar::clearLabel( this, Qt::Key_Back );
    }
}

void RegionSelector::reset()
{
#define DEFAULT_WIDTH 100
#define DEFAULT_HEIGHT 100

    // Reset region
    if( Qtopia::mousePreferred() ) {
        region_start = QPoint();
        _region = QRect();
        current_state = MARK;
    } else {
        // Set default region
        _region = QRect( 0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT );
        _region.moveCenter( rect().center() + image_ui->viewport().topLeft() );
        // Set current state to move
        current_state = MOVE;
        // If enabled toggle state label
        if( enabled ) setStateLabel();
    }
}

void RegionSelector::paintEvent( QPaintEvent* )
{
static const QPixmap top_left( ":image/photoedit/top_left" );
static const QPixmap top_right( ":image/photoedit/top_right" );
static const QPixmap bottom_left( ":image/photoedit/bottom_left" );
static const QPixmap bottom_right( ":image/photoedit/bottom_right" );
static const QPixmap crosshair( ":image/photoedit/crosshair" );

#define CROSSHAIR_CENTER 7
#define CORNER_WIDTH 7
#define CORNER_HEIGHT 7
#define INSET 1

#define PAINTER_COLOR Qt::white
#define SPACE_FILL_PATTERN Qt::Dense6Pattern
#define PEN_WIDTH 1

    QPainter painter( this );
    painter.setPen( PAINTER_COLOR );

    // If selection enabled, draw selected region onto widget
    // Otherwise, draw image ui onto widget
    if( enabled ) {
        // painter.setRasterOp( Qt::XorROP );
        QRect viewport = image_ui->viewport();

        if (viewport.width()  < width() ) viewport.moveLeft((width() - viewport.width() ) / 2);
        if (viewport.height() < height()) viewport.moveTop((height() - viewport.height()) / 2);

        QRect translatedRegion = _region.translated(-viewport.topLeft());

        // Draw box around current selection region
        painter.drawRect( translatedRegion.adjusted( 0, 0, -PEN_WIDTH, -PEN_WIDTH ) );

        if( !Qtopia::mousePreferred() ) {
            QPoint center( translatedRegion.center() );
            switch( current_state ) {
            // If current state is move, draw crosshair in center of region
            case MOVE:
                painter.drawPixmap( center.x() - CROSSHAIR_CENTER, center.y() - CROSSHAIR_CENTER, crosshair );
                break;
            // If current state is size, draw corners around inner edge of region
            case SIZE:
                painter.drawPixmap( translatedRegion.left() + INSET, translatedRegion.top() + INSET, top_left );
                painter.drawPixmap( translatedRegion.right() - CORNER_WIDTH, translatedRegion.top() + INSET, top_right );
                painter.drawPixmap( translatedRegion.left() + INSET, translatedRegion.bottom() - CORNER_HEIGHT, bottom_left );
                painter.drawPixmap( translatedRegion.right() - CORNER_WIDTH, translatedRegion.bottom() - CORNER_HEIGHT, bottom_right );
                break;
            default:
                // Ignore
                break;
            }
        }

        QRegion region = image_ui->region().subtract( translatedRegion );
        if( !region.isEmpty() ) {
            painter.setClipRegion( region );
            painter.setClipping( true );

            // Gray out area surrounding current selection region
            painter.fillRect( rect(), QBrush( PAINTER_COLOR, SPACE_FILL_PATTERN ) );
        }
    }
}

void RegionSelector::keyPressEvent( QKeyEvent* e )
{
#define STEP 4

    if( enabled && !Qtopia::mousePreferred() ) {
        if( e->key() == QSoftMenuBar::menuKey() ) {
            // Toggle current state
            switch( current_state ) {
            case MOVE:
                current_state = SIZE;
                setStateLabel();
                update();
                break;
            case SIZE:
                current_state = MOVE;
                setStateLabel();
                update();
                break;
            default:
                // Ignore
                break;
            }
        } else if( e->key() == Qt::Key_Select ) {
            emit selected();
        } else {
            switch( current_state ) {
            // Move region
            case MOVE:
                switch( e->key() ) {
                case Qt::Key_Left:
                    moveBy( -STEP, 0 );
                    update();
                    break;
                case Qt::Key_Right:
                    moveBy( STEP, 0 );
                    update();
                    break;
                case Qt::Key_Up:
                    moveBy( 0, -STEP );
                    update();
                    break;
                case Qt::Key_Down:
                    moveBy( 0, STEP );
                    update();
                    break;
                default:
                    // Ignore
                    e->ignore();
                    break;
                }
                break;
            // Size region
            case SIZE:
                switch( e->key() ) {
                case Qt::Key_Left:
                    sizeBy( -STEP, 0 );
                    update();
                    break;
                case Qt::Key_Right:
                    sizeBy( STEP, 0 );
                    update();
                    break;
                case Qt::Key_Up:
                    sizeBy( 0, STEP );
                    update();
                    break;
                case Qt::Key_Down:
                    sizeBy( 0, -STEP );
                    update();
                    break;
                default:
                    // Ignore
                    e->ignore();
                    break;
                }
                break;
            default:
                // Ignore
                break;
            }
        }
    } else {
        if( e->key() == Qt::Key_Back ) emit pressed();
        e->ignore();
    }
}

void RegionSelector::mousePressEvent( QMouseEvent* e )
{
#define LAG 15

    switch( e->button() ) {
    // If stylus has been pressed
    case Qt::LeftButton:
        // If selection enabled update press position with stylus position
        // Otherwise, emit pressed
        if( enabled ) {
            region_start = e->pos();
            lag_area = QRect( region_start - QPoint( LAG, LAG ),
                region_start + QPoint( LAG, LAG ) );
        } else emit pressed();
        break;
    default:
        // Ignore
        break;
    }
}

void RegionSelector::mouseReleaseEvent( QMouseEvent* e )
{
    if( enabled ) {
        switch( current_state ) {
        // If region is being marked
        case MARK:
            // If stylus released within the selected region, emit selected
            // Otherwise, emit canceled signal
            if( _region.contains( e->pos() ) ) emit selected();
            else emit canceled();
            break;
        // If region is moving, change to mark
        case MOVING:
            current_state = previous_state;
            update();
            break;
        default:
            // Ignore
            break;
        }
    }
}

void RegionSelector::mouseMoveEvent( QMouseEvent* e )
{
    if( enabled ) {
        if ( current_state == MOVING ) {
            // Update region
            int x = e->pos().x(), y = e->pos().y();

            // Contain region within widget
            if( x < rect().left() ) x = rect().left();
            if( x > rect().right() ) x = rect().right();
            if( y < rect().top() ) y = rect().top();
            if( y > rect().bottom() ) y = rect().bottom();

            QRect viewport = image_ui->viewport();

            if (viewport.width()  < width() ) viewport.moveLeft((width()  - viewport.width() ) / 2);
            if (viewport.height() < height()) viewport.moveTop ((height() - viewport.height()) / 2);

            // Update region end with current stylus position
            _region = QRect(region_start, QPoint( x, y )).normalized().translated(viewport.topLeft());
            // Update display
            update();
        } else if ( !lag_area.contains( e->pos() ) ) {
            previous_state = current_state;
            current_state = MOVING;
        }
    }
}

void RegionSelector::setStateLabel()
{
    switch( current_state ) {
    case MOVE:
        QSoftMenuBar::setLabel( this, QSoftMenuBar::menuKey(), "photoedit/resize", tr( "Size" ) );
        break;
    case SIZE:
        QSoftMenuBar::setLabel( this, QSoftMenuBar::menuKey(), "photoedit/move", tr( "Move" ) );
        break;
    default:
        // Ignore
        break;
    }
}

void RegionSelector::moveBy( int dx, int dy )
{
    QRect viewport(QPoint(0, 0), size());
    viewport.moveCenter(image_ui->viewport().center());

    // Contain region within widget
    if (_region.left()   + dx < viewport.left()  ) dx = viewport.left()   - _region.left();
    if (_region.right()  + dx > viewport.right() ) dx = viewport.right()  - _region.right();
    if (_region.top()    + dy < viewport.top()   ) dy = viewport.top()    - _region.top();
    if (_region.bottom() + dy > viewport.bottom()) dy = viewport.bottom() - _region.bottom();

    _region.translate(dx, dy);
}

void RegionSelector::sizeBy( int dw, int dh )
{
#define MIN_WIDTH 20
#define MIN_HEIGHT 20

    _region.setLeft( _region.left() - dw );
    _region.setRight( _region.right() + dw );
    _region.setTop( _region.top() - dh );
    _region.setBottom( _region.bottom() + dh );

    // Limit to minimum
    if( _region.width() < MIN_WIDTH ) {
        _region.setLeft( _region.left() + dw );
        _region.setRight( _region.right() - dw );
    }
    if( _region.height() < MIN_HEIGHT ) {
        _region.setTop( _region.top() + dh );
        _region.setBottom( _region.bottom() - dh );
    }

    QRect viewport(QPoint(0, 0), size());
    viewport.moveCenter(image_ui->viewport().center());

    // Contain region within widget
    _region = _region.intersected(viewport);
}

