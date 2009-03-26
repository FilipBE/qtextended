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

#include "e1_dialer.h"
#include "e1_callhistory.h"
#include "e1_bar.h"
#include "e1_phonebrowser.h"
#include "e1_popup.h"


#include <themedview.h>
#include "dialercontrol.h"

#include <QWindowsStyle>
#include <QUniqueId>
#include <QLabel>
#include <QPainter>
#include <QLineEdit>
#include <QBoxLayout>
#include <QEvent>

// define E1DialerButton
E1DialerButton::E1DialerButton( QWidget* parent, Qt::WindowFlags f )
    : QWidget(parent, f), m_state(E1DialerButton::Up), m_bgImage(":image/samples/buttonalpha"), m_backgroundEnabled(true)
{

    QColor upButtonColor = palette().color( QPalette::Active, QPalette::Highlight );
    m_colorForState[E1DialerButton::Up] = upButtonColor;
    m_colorForState[E1DialerButton::Down] = QColor( (upButtonColor.red() * 60) / 100,
                             (upButtonColor.green() * 60) / 100,
                              (upButtonColor.blue() * 60) / 100 );
    generateBgPixmap( E1DialerButton::Up );
    generateBgPixmap( E1DialerButton::Down );
    setFixedSize( m_bgImage.width(), m_bgImage.height() );
}

void E1DialerButton::setBackgroundEnabled( bool e )
{
    if( m_backgroundEnabled != e ) {
        m_backgroundEnabled = e;
        if( m_backgroundEnabled )
            setFixedSize( m_bgImage.width(), m_bgImage.height() );
        else if( !m_fgPixmap.isNull() )
            setFixedSize( m_fgPixmap.width(), m_fgPixmap.height() );
        if( isVisible() )
            update();
    }
}

void E1DialerButton::setFgPixmap( const QPixmap& pix )
{
    m_fgPixmap = pix;
    if( !m_backgroundEnabled )
        setFixedSize( m_fgPixmap.width(), m_fgPixmap.height() );
    //resize( m_fgPixmap.width(), m_fgPixmap.height() );
    if( isVisible() )
        update();
}

void E1DialerButton::generateBgPixmap( const E1DialerButton::State& st )
{
    QPixmap pm;
    if( !m_bgImage.isNull() && m_colorForState[st].isValid() ) {
        QImage tmpImg = m_bgImage.copy();
        ThemePixmapItem::colorizeImage(tmpImg, m_colorForState[st], 255, true);
        pm = pm.fromImage(tmpImg);
        m_bgForState[st] = pm;
    } else if( !m_bgImage.isNull() ) {
        pm = pm.fromImage( m_bgImage );
        m_bgForState[st] = pm;
    }
}

void E1DialerButton::setText( const QString& text )
{
    m_text = text;
}

/*
void E1DialerButton::setColor( const E1DialerButton::State& st, const QColor& color )
{
    if( m_colorForState[st] != color ) {
        m_colorForState[st] = color;
        generateBgPixmap( st );
        if( isVisible() )
            update();
    }
}
*/

void E1DialerButton::paintEvent( QPaintEvent* )
{
    QPainter p( this );
    if( m_backgroundEnabled )
        p.drawPixmap( 0, 0, width(), height(), m_bgForState[m_state] );
    p.drawPixmap( (width()/2)-(m_fgPixmap.width()/2), (height()/2)-(m_fgPixmap.height()/2), m_fgPixmap.width(), m_fgPixmap.height(), m_fgPixmap );
}

void E1DialerButton::mousePressEvent( QMouseEvent* e )
{
    m_state = E1DialerButton::Down;
    if( isVisible() )
        update();
    QWidget::mousePressEvent( e );
}

void E1DialerButton::mouseReleaseEvent( QMouseEvent* e )
{
    m_state = E1DialerButton::Up;
    if( isVisible() )
        update();
    QWidget::mouseReleaseEvent( e );
    emit clicked( m_text );
}

void E1DialerButton::resizeEvent( QResizeEvent* e )
{
    QWidget::resizeEvent( e );
}

void E1DialerButton::moveEvent( QMoveEvent* e )
{
    QWidget::moveEvent( e );
}

// define E1Dialer
E1Dialer::E1Dialer( E1Button *b, QWidget* parent, Qt::WFlags f )
    : QWidget( parent, f | Qt::FramelessWindowHint ), m_textButton(b), m_activeCallCount(0)
{
    QVBoxLayout* vlayout = new QVBoxLayout( this );
    vlayout->setSpacing( 7 );
    vlayout->setMargin( 7 );

    // dialer interface

    // callhistory, input, backspace
    QHBoxLayout* hlayout1 = new QHBoxLayout();
    vlayout->addLayout( hlayout1 );
    hlayout1->setSpacing( 10 );
    hlayout1->setMargin( 0 );

    E1Bar * historyBar = new E1Bar(this);
    QPalette pal = palette();
    pal.setColor(QPalette::Highlight, QColor(255, 126, 0));
    historyBar->setPalette(pal);
    historyBar->setFixedSize(30, 20);
    historyBar->setBorder(E1Bar::ButtonBorder);
    E1Button *historyButton = new E1Button;
    historyButton->setPixmap(QPixmap(":image/samples/e1_callhistory"));
    historyBar->addItem(historyButton);
    connect(historyButton, SIGNAL(clicked()), this, SLOT(historyClicked()));
    hlayout1->addWidget( historyBar );

    m_input = new QLineEdit( this );
    m_input->setStyle( new QWindowsStyle );
    m_input->setFixedSize( 152, 24 );
    hlayout1->addWidget( m_input );
    connect( m_input, SIGNAL(textChanged(QString)), this, SLOT(numberChanged(QString)) );

    E1Bar * backspaceBar = new E1Bar(this);
    backspaceBar->setPalette(pal);
    backspaceBar->setFixedSize(30, 20);
    backspaceBar->setBorder(E1Bar::ButtonBorder);
    E1Button *backspaceButton = new E1Button;
    backspaceButton->setPixmap(QPixmap(":image/samples/e1_erase"));
    backspaceBar->addItem(backspaceButton);
    connect(backspaceButton, SIGNAL(clicked()), this, SLOT(eraseClicked()));
    hlayout1->addWidget( backspaceBar );

    QHBoxLayout* sideLayout = new QHBoxLayout;
    sideLayout->setMargin( 0 );
    sideLayout->setSpacing( 0 );
    vlayout->addLayout( sideLayout );

    QVBoxLayout* buttonLayout = new QVBoxLayout;
    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(15);
    sideLayout->addLayout( buttonLayout );

    m_callscreenBar = new E1Bar(this);
    m_callscreenBar->setVisible( false );
    m_callscreenBar->setBorder(E1Bar::ButtonBorder);
    E1Button *callscreen = new E1Button;
    callscreen->setFlag(E1Button::Expanding);
    callscreen->setPixmap(QPixmap(":image/samples/e1_dialer"));
    m_callscreenBar->addItem(callscreen);
    m_callscreenBar->setFixedSize(32, 96);
    sideLayout->addWidget(m_callscreenBar);
    QObject::connect(callscreen, SIGNAL(clicked()), this, SIGNAL(toCallScreen()));
    connect(DialerControl::instance(), SIGNAL(activeCount(int)), this, SLOT(activeCallCount(int)));

    // 1, 2, 3
    QHBoxLayout* hlayout2 = new QHBoxLayout();
    buttonLayout->addLayout( hlayout2 );
    hlayout2->setSpacing( 10 );
    hlayout2->setMargin( 0 );

    E1DialerButton* button = new E1DialerButton( this );
    button->setText( "1" );
    button->setFgPixmap( QPixmap(":image/samples/1" ) );
    connect( button, SIGNAL(clicked(QString)), this, SLOT(buttonClicked(QString)) );
    hlayout2->addWidget( button );
    hlayout2->addSpacing( 7 );

    button = new E1DialerButton( this );
    button->setText( "2" );
    button->setFgPixmap( QPixmap(":image/samples/2" ) );
    connect( button, SIGNAL(clicked(QString)), this, SLOT(buttonClicked(QString)) );
    hlayout2->addWidget( button );
    hlayout2->addSpacing( 7 );

    button = new E1DialerButton( this );
    button->setText( "3" );
    button->setFgPixmap( QPixmap(":image/samples/3" ) );
    connect( button, SIGNAL(clicked(QString)), this, SLOT(buttonClicked(QString)) );
    hlayout2->addWidget( button );

    // 4, 5, 6
    QHBoxLayout* hlayout3 = new QHBoxLayout();
    buttonLayout->addLayout( hlayout3 );
    hlayout3->setSpacing( 10 );
    hlayout3->setMargin( 0 );

    button = new E1DialerButton( this );
    button->setText( "4" );
    button->setFgPixmap( QPixmap(":image/samples/4" ) );
    connect( button, SIGNAL(clicked(QString)), this, SLOT(buttonClicked(QString)) );
    hlayout3->addWidget( button );
    hlayout3->addSpacing( 7 );

    button = new E1DialerButton( this );
    button->setText( "5" );
    button->setFgPixmap( QPixmap(":image/samples/5" ) );
    connect( button, SIGNAL(clicked(QString)), this, SLOT(buttonClicked(QString)) );
    hlayout3->addWidget( button );
    hlayout3->addSpacing( 7 );

    button = new E1DialerButton( this );
    button->setText( "6" );
    button->setFgPixmap( QPixmap(":image/samples/6" ) );
    connect( button, SIGNAL(clicked(QString)), this, SLOT(buttonClicked(QString)) );
    hlayout3->addWidget( button );

    // 7, 8, 9
    QHBoxLayout* hlayout4 = new QHBoxLayout();
    buttonLayout->addLayout( hlayout4 );
    hlayout4->setSpacing( 10 );
    hlayout4->setMargin( 0 );

    button = new E1DialerButton( this );
    button->setText( "7" );
    button->setFgPixmap( QPixmap(":image/samples/7" ) );
    connect( button, SIGNAL(clicked(QString)), this, SLOT(buttonClicked(QString)) );
    hlayout4->addWidget( button );
    hlayout4->addSpacing( 7 );

    button = new E1DialerButton( this );
    button->setText( "8" );
    button->setFgPixmap( QPixmap(":image/samples/8" ) );
    connect( button, SIGNAL(clicked(QString)), this, SLOT(buttonClicked(QString)) );
    hlayout4->addWidget( button );
    hlayout4->addSpacing( 7 );

    button = new E1DialerButton( this );
    button->setText( "9" );
    button->setFgPixmap( QPixmap(":image/samples/9" ) );
    connect( button, SIGNAL(clicked(QString)), this, SLOT(buttonClicked(QString)) );
    hlayout4->addWidget( button );

    // *, 0, hash
    QHBoxLayout* hlayout5 = new QHBoxLayout();
    buttonLayout->addLayout( hlayout5 );
    hlayout5->setSpacing( 10 );
    hlayout5->setMargin( 0 );

    button = new E1DialerButton( this );
    button->setText( "*" );
    button->setFgPixmap( QPixmap(":image/samples/star" ) );
    connect( button, SIGNAL(clicked(QString)), this, SLOT(buttonClicked(QString)) );
    hlayout5->addWidget( button );
    hlayout5->addSpacing( 7 );

    button = new E1DialerButton( this );
    button->setText( "0" );
    button->setFgPixmap( QPixmap(":image/samples/0" ) );
    connect( button, SIGNAL(clicked(QString)), this, SLOT(buttonClicked(QString)) );
    hlayout5->addWidget( button );
    hlayout5->addSpacing( 7 );

    button = new E1DialerButton( this );
    button->setText( "#" );
    button->setFgPixmap( QPixmap(":image/samples/hash" ) );
    connect( button, SIGNAL(clicked(QString)), this, SLOT(buttonClicked(QString)) );
    hlayout5->addWidget( button );

    connect( m_textButton, SIGNAL(clicked()), this, SLOT(textButtonClicked()) );
}

void E1Dialer::activeCallCount( int c )
{
    if( m_activeCallCount != c ) {
        if( m_activeCallCount == 0 || c == 0 )
            m_input->setText( QString() );// active calls exist changed, clear
                                        // if no active calls, person dials number, callscreen shows, back to dialer, no number dispayed
                                       // if on a call, go to dialer, dial dtmf, go to callscreen, end call, back to dialer, no number displayed
        m_activeCallCount = c;
        m_callscreenBar->setVisible( c != 0 );
        if( m_activeCallCount != 0 ) {
            if(isVisible())
                m_textButton->setText(" ");
        } else {
            if( m_input->text().isEmpty() )
                numberChanged( QString() ); // manually force and update if text doesn't change
        }
    }
}

// Insert menu slots
void E1Dialer::addPlus() { }
void E1Dialer::addPause() { }
void E1Dialer::addWait() { }

void E1Dialer::numberChanged( const QString& txt )
{
    if(!isVisible())
        return;
    if( m_activeCallCount == 0 ) {
        if( txt.isEmpty() ) {
            m_textButton->setText("Redial");
        } else {
            m_textButton->setText("Send");
        }
    } else {
        m_textButton->setText(" ");
    }
}

void E1Dialer::textButtonClicked()
{
    if(!isVisible())
        return;

    if( m_activeCallCount != 0 )
        return;
    if( m_input->text().isEmpty() ) {
        // open up the call history and dial the last number called
        QCallList& historyListAccess = DialerControl::instance()->callList();
        QList<QCallListItem> historyList = historyListAccess.allCalls();
        QString lastNumber = historyList.first().number().trimmed();
        if(!lastNumber.isEmpty())
            m_input->setText( historyList.first().number() );
    } else {
        emit sendNumber(m_input->text());
    }
}

void E1Dialer::historyClicked()
{
    buttonClicked("callhistory");
}

void E1Dialer::eraseClicked()
{
    buttonClicked("backspace");
}

void E1Dialer::buttonClicked( const QString& txt )
{
    if( txt == "callhistory" )
        selectCallHistory();
    else if( txt == "backspace" )
        m_input->backspace();
    else {
        if( m_activeCallCount != 0 ) {
            Q_ASSERT(DialerControl::instance()->activeCalls().count() > 0);
            DialerControl::instance()->activeCalls().first().tone( txt );

        }
        m_input->setText( m_input->text()+txt );
    }
}

void E1Dialer::selectCallHistory()
{
    E1CallHistory* history = new E1CallHistory( this );
    history->exec();
    delete history;
}

void E1Dialer::resizeEvent( QResizeEvent* e )
{
    QWidget::resizeEvent( e );
}

void E1Dialer::setActive()
{
    if(isVisible())
        numberChanged(m_input->text());
}

void E1Dialer::setNumber(const QString &num)
{
    m_input->setText(num);
}

