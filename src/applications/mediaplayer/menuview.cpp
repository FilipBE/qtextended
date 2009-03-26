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

#include "menuview.h"
#include "keyhold.h"

class StyleDelegate : public QStyle
{
public:
    StyleDelegate( QStyle* delegate )
        : m_delegate( delegate )
    { }

    void drawComplexControl( ComplexControl c, const QStyleOptionComplex* so, QPainter* p, const QWidget* w ) const
        { m_delegate->drawComplexControl( c, so, p, w ); }
    void drawControl( ControlElement ce, const QStyleOption* so, QPainter* p, const QWidget* w ) const
        { m_delegate->drawControl( ce, so, p, w ); }
    void drawItemPixmap( QPainter* p, const QRect& r, int a, const QPixmap& pix ) const
        { m_delegate->drawItemPixmap( p, r, a, pix ); }
    void drawItemText ( QPainter* p, const QRect& r, int a, const QPalette& pal, bool e, const QString& t, QPalette::ColorRole tr ) const
        { m_delegate->drawItemText( p, r, a, pal, e, t, tr ); }
    void drawPrimitive( PrimitiveElement pe, const QStyleOption* so, QPainter* p, const QWidget* w ) const
        { m_delegate->drawPrimitive( pe, so, p, w ); }
    QPixmap generatedIconPixmap( QIcon::Mode im, const QPixmap& p, const QStyleOption* so ) const
        { return m_delegate->generatedIconPixmap( im, p, so ); }
    SubControl hitTestComplexControl( ComplexControl cc, const QStyleOptionComplex* so, const QPoint& p, const QWidget* w ) const
        { return m_delegate->hitTestComplexControl( cc, so, p, w ); }
    QRect itemPixmapRect( const QRect& r, int a, const QPixmap& p ) const
        { return m_delegate->itemPixmapRect( r, a, p ); }
    QRect itemTextRect( const QFontMetrics& fm, const QRect& r, int a, bool e, const QString& t ) const
        { return m_delegate->itemTextRect( fm, r, a, e, t ); }
    int pixelMetric( PixelMetric pm, const QStyleOption* so, const QWidget* w ) const
        { return m_delegate->pixelMetric( pm, so, w ); }
    void polish( QWidget* w )
        { m_delegate->polish( w ); }
    void polish( QApplication* a )
        { m_delegate->polish( a ); }
    void polish( QPalette& p )
        { m_delegate->polish( p ); }
    QSize sizeFromContents( ContentsType ct, const QStyleOption* so, const QSize& s, const QWidget* w = 0 ) const
        { return m_delegate->sizeFromContents( ct, so, s, w ); }
    QPalette standardPalette () const
        { return m_delegate->standardPalette(); }
    QPixmap standardPixmap( StandardPixmap sp, const QStyleOption *o, const QWidget* w ) const
        { return m_delegate->standardPixmap( sp, o, w ); }
    int styleHint( StyleHint h, const QStyleOption* so, const QWidget* w, QStyleHintReturn* r ) const
        { return m_delegate->styleHint( h, so, w, r ); }
    QRect subControlRect( ComplexControl cc, const QStyleOptionComplex* so, SubControl s, const QWidget* w ) const
        { return m_delegate->subControlRect( cc, so, s, w ); }
    QRect subElementRect( SubElement se, const QStyleOption* so, const QWidget* w ) const
        { return m_delegate->subElementRect( se, so, w ); }
    void unpolish ( QWidget* w )
        { m_delegate->unpolish( w ); }
    void unpolish ( QApplication* a )
        { m_delegate->unpolish( a ); }

private:
    QStyle *m_delegate;
};

class MenuViewStyle : public StyleDelegate
{
public:
    MenuViewStyle( QStyle* delegate )
        : StyleDelegate( delegate )
    { }

    void drawPrimitive( PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget ) const;
};

void MenuViewStyle::drawPrimitive( PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget ) const
{
    if( element != QStyle::PE_FrameFocusRect ) {
        StyleDelegate::drawPrimitive( element, option, painter, widget );
    }
}

static const int KEY_SELECT_HOLD = Qt::Key_unknown + Qt::Key_Select;

/*!
    \class MenuView
    \inpublicgroup QtMediaModule
    \internal
*/

/*!
    \fn MenuView::MenuView( QWidget* parent )
    \internal
*/
MenuView::MenuView( QWidget* parent )
    : QListView( parent ), m_eventcache( QEvent::None, QPoint(), Qt::NoButton, Qt::NoButton, Qt::NoModifier )
{
    static const int HOLD_THRESHOLD = 1000;

    new KeyHold( Qt::Key_Select, KEY_SELECT_HOLD, HOLD_THRESHOLD, this, this );
    connect( this, SIGNAL(activated(QModelIndex)),
        this, SIGNAL(selected(QModelIndex)) );

    // Customize list view appearance
    setFrameStyle( QFrame::NoFrame );
    // TODO!!! This has been commented out as a hack, and needs to be investigated further
    // for Qtopia 4.3. See Task 139260
    //setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    m_style = new MenuViewStyle( style() );
    setStyle( m_style );

    m_holdtimer = new QTimer( this );
    connect( m_holdtimer, SIGNAL(timeout()),
        this, SLOT(emitHeld()) );
    m_holdtimer->setInterval( HOLD_THRESHOLD );
    m_holdtimer->setSingleShot( true );
}

/*!
    \fn MenuView::~MenuView()
    \internal
*/
MenuView::~MenuView()
{
    delete m_style;
}

/*!
    \fn void MenuView::emitHeld()
    \internal
*/
void MenuView::emitHeld()
{
    QModelIndex current = currentIndex();
    if( current.isValid() ) {
        emit held( current );
    }
}

/*!
    \fn void MenuView::keyPressEvent( QKeyEvent* e )
    \internal
*/
void MenuView::keyPressEvent( QKeyEvent* e )
{
    switch( e->key() )
    {
    case KEY_SELECT_HOLD:
        e->accept();
        emit( held( currentIndex() ) );
        break;
    default:
        QListView::keyPressEvent( e );
    }
}

/*!
    \fn void MenuView::mousePressEvent( QMouseEvent* e )
    \internal
*/
void MenuView::mousePressEvent( QMouseEvent* e )
{
    m_eventcache = *e;

    QModelIndex current = indexAt( e->pos() );
    if( current.isValid() ) {
        setCurrentIndex( current );
    }

    m_holdtimer->start();
}

/*!
    \fn void MenuView::mouseReleaseEvent( QMouseEvent* e )
    \internal
*/
void MenuView::mouseReleaseEvent( QMouseEvent* e )
{
    if( m_holdtimer->isActive() ) {
        QListView::mousePressEvent( &m_eventcache );
        QListView::mouseReleaseEvent( e );
        m_holdtimer->stop();
    }
}

/*!
    \fn void MenuStack::push( MenuModel* model )
    \internal
*/
void MenuStack::push( MenuModel* model )
{
    m_stack.push( model );

    m_view->setModel( model );
}

/*!
    \fn MenuModel* MenuStack::pop()
    \internal
*/
MenuModel* MenuStack::pop()
{
    MenuModel* model = m_stack.pop();

    m_view->setModel( m_stack.top() );

    return model;
}
