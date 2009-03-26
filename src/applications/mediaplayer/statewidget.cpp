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

#include "statewidget.h"
#include "keyhold.h"

#include <media.h>

#include <QSoftMenuBar>

static const int KEY_SELECT_HOLD = Qt::Key_unknown + Qt::Key_Select;

StateWidget::StateWidget( PlayerControl* control, QWidget* parent )
    : QWidget( parent ), m_control( control )
{
    static const int HOLD_THRESHOLD = 500;

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin( 0 );

    m_label = new QMediaStateLabel;
    layout->addWidget( m_label );

    setLayout( layout );

    connect( control, SIGNAL(stateChanged(PlayerControl::State)),
        this, SLOT(setState(PlayerControl::State)) );

    m_holdtimer = new QTimer( this );
    connect( m_holdtimer, SIGNAL(timeout()),
        this, SLOT(setStopped()) );
    m_holdtimer->setInterval( HOLD_THRESHOLD );
    m_holdtimer->setSingleShot( true );

    new KeyHold( Qt::Key_Select, KEY_SELECT_HOLD, HOLD_THRESHOLD, this, this );
}

void StateWidget::setState( PlayerControl::State state )
{
    switch( state )
    {
    case PlayerControl::Playing:
        m_label->setState( QtopiaMedia::Playing );
        QSoftMenuBar::setLabel( this, Qt::Key_Select, QLatin1String( ":icon/pause" ), tr( "Pause" ) );
        emit playing();
        break;
    case PlayerControl::Paused:
        m_label->setState( QtopiaMedia::Paused );
        QSoftMenuBar::setLabel( this, Qt::Key_Select, QLatin1String( ":icon/play" ), tr( "Play" ) );
        emit paused();
        break;
    case PlayerControl::Stopped:
        m_label->setState( QtopiaMedia::Stopped );
        QSoftMenuBar::setLabel( this, Qt::Key_Select, QLatin1String( ":icon/play" ), tr( "Play" ) );
        emit stopped();
        break;
    }
}

void StateWidget::setStopped()
{
    m_control->setState( PlayerControl::Stopped );
}

void StateWidget::keyPressEvent( QKeyEvent* e )
{
    switch( e->key() )
    {
    case Qt::Key_Select:
    case Qt::Key_MediaPlay:
        e->accept();
        togglePlaying();
        break;
    case KEY_SELECT_HOLD:
    case Qt::Key_MediaStop:
        e->accept();
        setStopped();
        break;
    default:
        QWidget::keyPressEvent(e);
        break;
    }
}

void StateWidget::mousePressEvent( QMouseEvent* )
{
    m_holdtimer->start();
}

void StateWidget::mouseReleaseEvent( QMouseEvent* )
{
    if( m_holdtimer->isActive() ) {
        togglePlaying();
        m_holdtimer->stop();
    }
}

void StateWidget::togglePlaying()
{
    switch( m_control->state() )
    {
    case PlayerControl::Playing:
        m_control->setState( PlayerControl::Paused );
        break;
    default:
        m_control->setState( PlayerControl::Playing );
        break;
    }
}
