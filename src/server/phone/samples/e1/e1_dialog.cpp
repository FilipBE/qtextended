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

#include "e1_dialog.h"
#include "e1_bar.h"

#include <QLayout>
#include <QPushButton>
#include <QPainter>
#include <QApplication>
#include <QResizeEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QLabel>

E1Dialog::E1Dialog( QWidget* parent, E1Dialog::Type t )
    : QDialog( parent, Qt::Dialog | Qt::FramelessWindowHint ), m_contentsWidget(0)
{
    setObjectName("__nomove");

    setContentsMargins( 4, 6, 4, 2 ); // left top right bottom, for frame
//    setFixedSize( 200, 220 );
//    move( (240/2)-(width()/2), 45 ); // XXX hardcoded 240 == screen width 45 pixels == header height

    m_layout = new QVBoxLayout( this );
    m_layout->setSpacing( 0 );
    m_layout->setMargin( 0 );

    // contents widget added to layout by setContentsWidget()

    m_layout->addSpacing( 3 ); // 3 pixel frame divider between contents widget and button

    m_bar = new E1Bar( this );
    m_bar->setFixedHeight( 32 );
    m_layout->addWidget( m_bar );
    switch( t ) {
        case E1Dialog::Return:
        {
            E1Button* button = new E1Button;
            button->setPixmap( QPixmap(":image/samples/e1_back" ) );
            button->setMargin( 0 );
            button->setFlag( E1BarItem::Expanding );
            button->setFlag( E1BarItem::Clickable );
            connect( button, SIGNAL(clicked()), this, SLOT(reject()) );
            m_bar->addItem( button );
            break;
        }
        case E1Dialog::NewMessage:
        {
            E1Button* button = new E1Button;
            button->setText( "Cancel" );
            button->setMargin( 0 );
            button->setFlag( E1BarItem::Expanding );
            button->setFlag( E1BarItem::Clickable );
            connect( button, SIGNAL(clicked()), this, SLOT(reject()) );
            m_bar->addItem( button );
            m_bar->addSeparator();
            button = new E1Button;
            button->setText( "Read" );
            button->setMargin( 0 );
            button->setFlag( E1BarItem::Expanding );
            button->setFlag( E1BarItem::Clickable );
            connect( button, SIGNAL(clicked()), this, SLOT(accept()) );
            m_bar->addItem( button );

            QWidget* parent = new QWidget( this );
            QHBoxLayout* layout = new QHBoxLayout( parent );
            QLabel* label;
            label = new QLabel( parent );
            label->setPixmap( QPixmap(":image/addressbook/generic-contact") );
            layout->addWidget( label );
             label = new QLabel( parent );
             label->setWordWrap( true );
            label->setText( "<p>New Incoming message. Read now?</p>" );
            layout->addWidget( label );
            setContentsWidget( parent );
            setFixedWidth( QApplication::desktop()->width()/5*4 );
        }
        default:
            break;
    }

}

E1Bar *E1Dialog::bar() const
{
    return m_bar;
}

void E1Dialog::setContentsWidget( QWidget* contentsWidget )
{
    Q_ASSERT( contentsWidget != 0 ); // must be given
    if( m_contentsWidget )
        delete m_contentsWidget;
    m_contentsWidget = contentsWidget;
    m_layout->insertWidget( 0, m_contentsWidget );
}

QColor E1Dialog::highlightColor() const
{

    QColor color(palette().highlight().color());

    QColor rv( color.red() + ((255 - color.red()) * 60) / 100,
              color.green() + ((255 - color.green()) * 60) / 100,
              color.blue() + ((255 - color.blue()) * 60) / 100);
    return rv;
}

void E1Dialog::showEvent(QShowEvent *e)
{
    move((qApp->desktop()->width() - width()) / 2,
         (qApp->desktop()->height() - height()) / 2);
    QDialog::showEvent(e);
}

void E1Dialog::paintEvent( QPaintEvent* e )
{
    Q_ASSERT(m_contentsWidget != 0);

    const QColor black(0,0,0),
                dark(65, 65, 65 ),
                mid(114, 106, 106),
                blue(highlightColor()),
                light(242, 240, 240)
                    ;

    QDialog::paintEvent( e );
    QPainter p( this );
    QRect r = rect();

    /* Top frame */

    // width(), height() is bottom right pixel, so width() is right most and height() is bottom most
    // top black line
    QPen pen( black );
    pen.setWidth( 1 );
    p.setPen( pen );
    p.drawLine( r.x()+1, r.y(), r.x()+r.width()-2, r.y() ); // black line

    // left black dot
    p.drawPoint( r.x()+1, r.y()+1 );
    // right black dot
    p.drawPoint( r.x()+r.width()-2, r.y()+1 );

    // left inside dark dot
    pen.setColor( dark );
    p.setPen( pen );
    p.drawPoint( r.x()+2, r.y()+1 );

    // left/right lighter dots
    pen.setColor( mid );
    p.setPen( pen );
    p.drawPoint( r.x()+3, r.y()+1 );
    p.drawPoint( r.x()+r.width()-3, r.y()+1 );

    // highlight
    pen.setColor( QColor(Qt::white) );
    p.setPen( pen );
    p.drawLine( r.x()+4, r.y()+1, r.x()+r.width()-4, r.y()+1 );

    // first dark horiz line
    pen.setColor( dark );
    p.setPen( pen );
    p.drawLine( r.x(), r.y()+2, r.x()+r.width()-1, r.y()+2 );

    // mid top horiz. background
    pen.setColor( mid );
    p.setPen( pen );
    p.drawLine( r.x(), r.y()+3, r.x()+r.width()-1, r.y()+3 );
    p.drawLine( r.x(), r.y()+4, r.x()+r.width()-1, r.y()+4 );

    // black horiz top line
    pen.setColor( black );
    p.setPen( pen );
    p.drawLine( r.x(), r.y()+5, r.x()+r.width()-1, r.y()+5 );

    // left corner and vertical dark line
    pen.setColor( dark );
    pen.setWidth( 1 );
    p.setPen( pen );
    p.drawLine( r.x(), r.y()+2, r.x()+4, r.y()+2 );
    p.drawLine( r.x(), r.y()+3, r.x()+4, r.y()+3 );
    p.drawLine( r.x(), r.y()+2, r.x(), r.y()+r.height()-1 );
    p.drawLine( r.x()+1, r.y()+2, r.x()+1, r.y()+r.height()-1 );

    // right side vertical dark line
    p.drawLine( r.x()+r.width()-2, r.y()+2, r.x()+r.width()-2, r.y()+r.height()-1 );
    p.drawLine( r.x()+r.width()-1, r.y()+2, r.x()+r.width()-1, r.y()+r.height()-1 );

    // left mid vertical line
    pen.setWidth( 1 );
    pen.setColor( mid );
    p.setPen( pen );
    p.drawLine( r.x()+2, r.y()+4, r.x()+2, r.y()+r.height()-1 );

    // left black vertical line
    pen.setColor( black );
    p.setPen( pen );
    p.drawLine( r.x()+3, r.y()+5, r.x()+3, r.y()+r.height()-1 );

    // right light line
    pen.setColor( light );
    p.setPen( pen );
    p.drawLine( r.x()+r.width()-3, r.y()+4, r.x()+r.width()-3, r.y()+r.height()-1 );

    // right blue line
    pen.setColor( blue );
    p.setPen( pen );
    p.drawLine( r.x()+r.width()-4, r.y()+5, r.x()+r.width()-4, r.y()+r.height()-2 );

    // split blue line
    p.drawLine( r.x()+3, m_bar->y()-3/*r.y()+r.height()-3*/, r.x()+r.width()-4, m_bar->y()-3 /*r.y()+r.height()-2*/ );

    // split light line
    pen.setColor( light );
    p.setPen( pen );
    p.drawLine( r.x()+2, m_bar->y()-2/*r.y()+r.height()-2*/, r.x()+r.width()-3, m_bar->y()-2/*r.y()+r.height()-2*/ );

    // split dark line
    pen.setColor( dark );
    p.setPen( pen );
    p.drawLine( r.x(), m_bar->y()-1/*r.y()+r.height()-1*/, r.x()+r.width()-1, m_bar->y()-1/*r.y()+r.height()-1*/ );


    p.drawLine( r.x(), r.y()+r.height()-2, r.x()+r.width()-2, r.y()+r.height()-2 );
    p.drawLine( r.x(), r.y()+r.height()-1, r.x()+r.width()-1, r.y()+r.height()-1 );

    /*
    pen.setWidth( 4 );
    p.setPen( pen );
    p.drawLine( r.topLeft(), r.bottomLeft() );
    p.drawLine( r.topRight(), r.bottomRight() );
    pen.setWidth( 2 );
    p.setPen( pen );
    p.drawLine( r.bottomLeft(), r.bottomRight() );
    */

    /*
    pen.setWidth( 3 );
    p.setPen( pen );

    QPoint divleft( 0, m_bar->y()-3 );
    QPoint divright( width()-1, divleft.y() );
    p.drawLine( divleft, divright );
    */
}

void E1Dialog::resizeEvent( QResizeEvent * e )
{
    QDialog::resizeEvent( e );
}

void E1Dialog::moveEvent( QMoveEvent* e )
{
    if( e->oldPos().x() != 0 && e->pos().x() == 0 ) // hack it up
    {
        e->accept();
        return;
    }
    QDialog::moveEvent( e );
}
