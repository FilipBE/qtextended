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
#include "qcolorselector.h"
#include <qpainter.h>
#include <qmenu.h>
#include <qpushbutton.h>
#include <qstyle.h>
#include <qlayout.h>
#include <qevent.h>
#include <qframe.h>
#include <qtopiaapplication.h>

#include <QStyleOptionButton>
#include <QDesktopWidget>


static const int DefaultBoxHeight = 22;

class QColorSelectorPrivate
{
public:
    QRgb palette[256];
    QColor col,defCol;
    int highlighted;
    bool pressed;
};

class QColorSelectorDialogPrivate
{
public:
    QColorSelector *picker;
};

/*!
    \class QColorSelectorDialog
    \inpublicgroup QtBaseModule


    \brief The QColorSelectorDialog class allows users to select a color.

    The user may select from a number of pre-defined colors displayed in
    a grid.  A default color is displayed at the bottom of the grid.

    The easiest way to create a QColorSelectorDialog is to use the static
    function getColor().

    \code
      QColor color = QColorSelectorDialog::getColor(Qt::red);
    \endcode

    \ingroup dialogs
*/

/*!
    \fn void QColorSelectorDialog::selected(const QColor &color)

    When the selected color changes, this signal is emitted with the
    \a color parameter containing the new color.
*/

/*!
    Constructs a color selector dialog with the given \a parent and \a flags
    that initially has the specifed \a color selected.
*/
QColorSelectorDialog::QColorSelectorDialog( const QColor &color, QWidget *parent,
                                        Qt::WindowFlags flags )
    : QDialog( parent, flags )
{
    init();
    setDefaultColor( color );
}

/*!
    Constructs a color selector dialog with the given \a parent and \a flags.
*/
QColorSelectorDialog::QColorSelectorDialog( QWidget *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags )
{
    init();
}

/*!
    Destroys a QColorSelectorDialog
    */
QColorSelectorDialog::~QColorSelectorDialog()
{
    delete d;
}

/*!
    Returns the currently highlighted color.

    \sa setColor()
*/
QColor QColorSelectorDialog::color() const
{
    return d->picker->color();
}

/*!
    Sets the default \a color.

    \sa setColor(), defaultColor()
*/
void QColorSelectorDialog::setDefaultColor( const QColor &color )
{
    d->picker->setDefaultColor( color );

}

/*!
    Returns the default color.

    \sa setDefaultColor(), color()
*/
const QColor &QColorSelectorDialog::defaultColor() const
{
    return d->picker->defaultColor();
}

/*!
    This is a convenience static function that returns a color selected
    by the user.  The default color is specified by \a color.

    The function creates a modal file dialog with the given \a parent widget.
*/
QColor QColorSelectorDialog::getColor( const QColor &color, QWidget *parent ) // static
{
    QColor fetchedColor;
    QColorSelectorDialog *dialog = new QColorSelectorDialog( color, parent );
    dialog->setModal(true);
    dialog->setWindowTitle( tr("Select color") );
    if( QtopiaApplication::execDialog( dialog ) == QDialog::Accepted )
        fetchedColor = dialog->color();
    delete dialog;
    return fetchedColor;
}

/*!
    Sets the currently highlighted color to \a color.

    \sa color()
*/
void QColorSelectorDialog::setColor( const QColor &color )
{
    d->picker->setColor( color );
}

void QColorSelectorDialog::colorSelected( const QColor &color )
{
    if( isModal() )
        accept();
    emit selected( color );
}

void QColorSelectorDialog::init()
{
    d = new QColorSelectorDialogPrivate;
    d->picker = new QColorSelector( this );
    connect( d->picker, SIGNAL(selected(QColor)),
                                     this, SLOT(colorSelected(QColor)) );
    QVBoxLayout *l = new QVBoxLayout( this );
    l->addWidget( d->picker );
    QtopiaApplication::setMenuLike( this, true );
}

/*!
    \class QColorSelector
    \inpublicgroup QtBaseModule
    \brief The QColorSelector class allows users to select a color.

    The user may select from a number of pre-defined colors displayed in
    a grid.  A default color is displayed at the bottom of the grid.

    \internal
*/
QColorSelector::QColorSelector( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    d = new QColorSelectorPrivate;
    d->highlighted = -1;
    d->pressed = false;
    int idx = 0;
    for( int ir = 0x0; ir <= 0xff; ir+=0x55 ) {
        for( int ig = 0x0; ig <= 0xff; ig+=0x55 ) {
            for( int ib = 0x0; ib <= 0xff; ib+=0x55 ) {
                d->palette[idx]=qRgb( ir, ig, ib );
                idx++;
            }
        }
    }

    bool changed = true;
    while ( changed ) {
        changed = false;
        int i = 0;
        QColor col( d->palette[i] );
        while ( i < idx-1 ) {
            QColor ncol( d->palette[i+1] );
            int h1, s1, v1;
            int h2, s2, v2;
            col.getHsv( &h1, &s1, &v1 );
            ncol.getHsv( &h2, &s2, &v2 );
            if ( h1*255+v1 > h2*255+v2 ) {
                QRgb tmp = d->palette[i];
                d->palette[i] = d->palette[i+1];
                d->palette[i+1] = tmp;
                changed = true;
            }
            col = ncol;
            i++;
        }
    }
    setMinimumSize( 65, 65 );
    setFocus();
}

QColorSelector::~QColorSelector()
{
    delete d;
}

QSize QColorSelector::sizeHint() const
{
    int s = 12*8+1;
    s = qMax(s, minimumWidth());
    return QSize( s, s + (d->defCol.isValid() ? ::DefaultBoxHeight : 0) );
}

void QColorSelector::setDefaultColor( const QColor &c )
{
    d->defCol = c;
    if ( !d->col.isValid() && d->defCol.isValid() )
        d->highlighted = -1;
    update();
}

const QColor &QColorSelector::defaultColor() const
{
    return d->defCol;
}

QColor QColorSelector::color() const
{
    return d->col;
}

void QColorSelector::setColor( const QColor &c )
{
    d->col = c;
    if ( QColor(d->defCol) == c ) {
        d->highlighted = -1;
        update();
        return;
    }

    int r = c.red();
    int g = c.green();
    int b = c.blue();
    int rd = (QColor(d->palette[0]).red() - r);
    int gd = (QColor(d->palette[0]).green() - g);
    int bd = (QColor(d->palette[0]).blue() - b);
    int bestCol = 0;
    int bestDiff = rd*rd + gd*gd + bd*bd;
    for ( int i = 0; i < 8*8; i++ ) {
        if ( QColor(d->palette[i]) == c ) {
            d->highlighted = i;
            update();
            return;
        } else {
            rd = (QColor(d->palette[i]).red() - r);
            gd = (QColor(d->palette[i]).green() - g);
            bd = (QColor(d->palette[i]).blue() - b);
            int diff = rd*rd + gd*gd + bd*bd;
            if ( diff < bestDiff ) {
                bestDiff = diff;
                bestCol = i;
            }
        }
    }

    // We don't have the exact color specified, so we'll pick the closest.
    // Not really optimal, but better than settling for the default.
    d->highlighted = bestCol;
    update();
}

void QColorSelector::paintEvent( QPaintEvent * )
{
    QPainter p( this );

    int defHeight = (d->defCol.isValid() ? ::DefaultBoxHeight : 0);
    int gridHeight = height()-defHeight;
    int cw = (width()-1)/8;
    int ch = (gridHeight-1)/8;

    // Center the grid in the available space
    const int xOffset = (width() - (8 * cw)) / 2;
    const int yOffset = (gridHeight - (8 * ch)) / 2;

    int idx = 0;
    for ( int y = 0; y < 8; y++ ) {
        for ( int x = 0; x < 8; x++ ) {
            p.fillRect( x*cw+1+xOffset, y*ch+1+yOffset, cw-1, ch-1, QColor(d->palette[idx]) );
            if ( idx == d->highlighted ) {
                p.drawRect( x*cw+xOffset, y*ch+yOffset, cw, ch );
            }

            idx++;
        }
    }
    if ( d->defCol.isValid() ) {
        p.fillRect( 1+xOffset, height()-defHeight, 8*cw-1, defHeight-2, d->defCol );
        if ( d->highlighted < 0 )
            p.drawRect( 0+xOffset, height()-defHeight-1, 8*cw, defHeight-1);
        if ( qGray(d->defCol.rgb()) < 128 )
            p.setPen( Qt::white );
        p.drawText( 0, 8*ch+1+yOffset, width(), defHeight, Qt::AlignCenter, tr("Default") );
    }
}

void QColorSelector::mousePressEvent( QMouseEvent *me )
{
    d->pressed = true;
    mouseMoveEvent( me );
}

void QColorSelector::mouseMoveEvent( QMouseEvent *me )
{
    int defHeight = (d->defCol.isValid() ? ::DefaultBoxHeight : 0);
    int cw = (width()-1)/8;
    int ch = (height()-1-defHeight)/8;
    int row = (me->pos().y()-1)/ch;
    int col = (me->pos().x()-1)/cw;
    int oldIdx = d->highlighted;

    if ( col >= 0 && col < 8 ) {
        if ( row >=0 && row < 8 ) {
            d->highlighted = row*8+col;
            repaint( rectOfColor( oldIdx ) );
            repaint( rectOfColor( d->highlighted ) );
        }
        else if ( (defHeight != 0) && ((me->pos().x() - 1 - 8*ch) <= defHeight) ) {
            d->highlighted = -1;
            repaint( rectOfColor( oldIdx ) );
            repaint( rectOfColor( d->highlighted ) );
        }
    }
}

void QColorSelector::mouseReleaseEvent( QMouseEvent * )
{
    if ( !d->pressed )
        return;
    if ( d->highlighted >= 0 ) {
        d->col = QColor(d->palette[d->highlighted]);
        emit selected( d->col );
    } else {
        d->col = d->defCol;
        emit selected( d->col );
    }
}

void QColorSelector::keyPressEvent( QKeyEvent *e )
{
    const bool validDefault = d->defCol.isValid();

    int step = 0;

    switch ( e->key() ) {
        case Qt::Key_Left:
        {
            step = -1;
            break;
        }
        case Qt::Key_Right:
        {
            step = 1;
            break;
        }
        case Qt::Key_Up:
        {
            step = -8;
            break;
        }
        case Qt::Key_Down:
        {
            step = 8;
            break;
        }
        case Qt::Key_Select:
        case Qt::Key_Space:
        case Qt::Key_Return:
        {
            if ( d->highlighted >= 0 ) {
                d->col = QColor(d->palette[d->highlighted]);
                emit selected( d->col );
            } else {
                d->col = d->defCol;
                emit selected( d->col );
            }
            topLevelWidget()->hide();
            break;
        }
        case Qt::Key_Back:
        case Qt::Key_No:
        case Qt::Key_Escape:
        {
            topLevelWidget()->hide();
            break;
        }
        default:
            QWidget::keyPressEvent( e );
    }

    if (step != 0) {
        int oldIdx = d->highlighted;
        int newIdx = oldIdx + step;
        int magnitude = abs(step);

        int maximum = 63;
        int minimum = (validDefault ? (0 - magnitude) : 0);
        int wrapValue = (maximum - minimum) + 1;

        if (magnitude == 1) {
            if (newIdx < minimum)
                newIdx = (step < 0 ? maximum : 0);
            else if (newIdx > maximum)
                newIdx = minimum;
        }
        else {
            if (newIdx < minimum)
                newIdx += wrapValue;
            else if (newIdx > maximum)
                newIdx -= wrapValue;
        }

        d->highlighted = newIdx;
        repaint( rectOfColor( oldIdx ) );
        repaint( rectOfColor( newIdx ) );
    }
}

void QColorSelector::showEvent( QShowEvent *e )
{
    QWidget::showEvent(e);
    d->pressed = false;
}

QRect QColorSelector::rectOfColor( int idx ) const
{
    int gridHeight = height()-(d->defCol.isValid() ? ::DefaultBoxHeight : 0);
    int cw = (width()-1)/8;
    int ch = (gridHeight-1)/8;

    QRect r;
    if ( idx >= 0 ) {
        int xOffset = (width() - (8 * cw)) / 2;
        int yOffset = (gridHeight - (8 * ch)) / 2;

        int row = idx/8;
        int col = idx%8;
        r = QRect( col*cw+xOffset, row*ch+yOffset, cw+1, ch+1 );
    } else {
        r = QRect( 0, ch*8, width(), height()-ch*8 );
    }

    return r;
}

//===========================================================================

class QColorButtonPrivate
{
public:
    QFrame *popup;
    QColorSelector *picker;
    QColor col;
};

/*!
    \class QColorButton
    \inpublicgroup QtBaseModule
    \brief The QColorButton class allows users to select a color.


    The QColorButton class presents a push button with the current color
    displayed as the label.  Clicking the button pops up a color
    selection grid for the user to choose from a number of pre-defined
    colors.  A default color is displayed at the bottom of the grid.

    \ingroup advanced
*/

/*!
    \fn void QColorButton::selected(const QColor &color)

    When the selected color changes, this signal is emitted with the
    \a color parameter containing the new color.
*/

/*!
    Constructs a QColorButton with the given \a parent.
*/
QColorButton::QColorButton( QWidget *parent )
    : QPushButton( parent )
{
    init();
}

/*!
    Constructs a QColorButton with the given \a parent and the default
    color set to \a color.
*/
QColorButton::QColorButton( const QColor &color, QWidget *parent )
    : QPushButton(parent)
{
    init();
    d->col = color;
}

/*!
    Destroys the QColorButton.
*/
QColorButton::~QColorButton()
{
    delete d;
}

/*!
    Returns the currently selected color.

    \sa setColor()
*/
QColor QColorButton::color() const
{
    return d->col;
}

void QColorButton::init()
{
    d = new QColorButtonPrivate;

    d->popup = new QFrame( this, Qt::Popup );
    d->popup->setAttribute(Qt::WA_ShowModal, true);
    d->popup->setFrameStyle( QFrame::Box | QFrame::Plain );

    QVBoxLayout *l = new QVBoxLayout( d->popup );
    l->setMargin( 0 );

    d->picker = new QColorSelector( d->popup );
    l->addWidget( d->picker );

    connect( d->picker, SIGNAL(selected(QColor)),
            this, SIGNAL(selected(QColor)) );
    connect( d->picker, SIGNAL(selected(QColor)),
            this, SLOT(colorSelected(QColor)) );
    connect( this, SIGNAL(clicked()),
            this, SLOT(showSelector()));
}

void QColorButton::showSelector()
{
    const QRect available( QApplication::desktop()->availableGeometry() );

    QPoint global( mapToGlobal( QPoint( 0, height() ) ) );

    // Set the popup to be the same width as the button
    d->picker->setMinimumWidth( width() - 2 * d->popup->lineWidth() );

    QSize popupSize( d->popup->sizeHint() );
    if ( ( global.x() + popupSize.width() ) > available.width() )
        global.setX( global.x() + width() - popupSize.width() );
    if ( ( global.y() + popupSize.height() ) > available.height() )
        global.setY( global.y() - height() - popupSize.height() );

    d->popup->move( global );
    d->popup->show();
}

/*!
    Sets the currently selected \a color.

    \sa color()
*/
void QColorButton::setColor( const QColor &color )
{
    d->col = color;
    d->picker->setColor( color );
    update();
}

/*!
    Sets the default \a color.

    \sa defaultColor(), setColor()
*/
void QColorButton::setDefaultColor( const QColor &color )
{
    d->picker->setDefaultColor( color );
}


/*!
    Returns the default color.

    \sa setDefaultColor(), color()
*/
const QColor &QColorButton::defaultColor() const
{
     return d->picker->defaultColor();
}

void QColorButton::colorSelected( const QColor &c )
{
    d->popup->hide();
    d->col = c;
}

/*!
    \reimp
*/
void QColorButton::paintEvent( QPaintEvent *e )
{
    QPushButton::paintEvent( e );
    QPainter p( this );
    drawButtonLabel( &p );
}

/*!
    Paints the color label using painter \a painter.
*/
void QColorButton::drawButtonLabel( QPainter *painter )
{
    QStyleOptionButton sob;
    sob.init( this );
    QRect r = style()->subElementRect( QStyle::SE_PushButtonContents, &sob );
    if ( isDown() || isChecked() ){
        int sx = style()->pixelMetric( QStyle::PM_ButtonShiftHorizontal, &sob );
        int sy = style()->pixelMetric( QStyle::PM_ButtonShiftVertical, &sob );
        r.moveTo( r.topLeft() + QPoint( sx, sy ) );
    }
    int x, y, w, h;
    r.getRect( &x, &y, &w, &h );
    int dx = style()->pixelMetric( QStyle::PM_MenuButtonIndicator, &sob );
    QStyleOptionButton arrowStyle = sob;
    arrowStyle.rect.setLeft( x + (w - dx - 2) );
    arrowStyle.rect.setTop( y );
    style()->drawPrimitive( QStyle::PE_IndicatorArrowDown, &arrowStyle, painter );
    w -= dx;
    if ( d->col.isValid() )
        painter->fillRect( x+2, y+2, w-4, h-4, d->col );
    else if ( defaultColor().isValid() )
        painter->fillRect( x+2, y+2, w-4, h-4, defaultColor() );
}

