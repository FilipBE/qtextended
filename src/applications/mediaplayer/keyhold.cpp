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

#include "keyhold.h"

#include <QtGui>

/*!
    \class KeyHold
    \inpublicgroup QtMediaModule
    \internal
*/

/*!
    \fn KeyHold::KeyHold( int key, int keyHold, int threshold, QObject* target, QObject* parent )
    \internal
*/
KeyHold::KeyHold( int key, int keyHold, int threshold, QObject* target, QObject* parent )
    : QObject( parent ), m_key( key ), m_keyHold( keyHold ), m_threshold( threshold ), m_target( target )
{
    m_target->installEventFilter( this );

    m_countdown = new QTimer( this );
    m_countdown->setSingleShot( true );
    connect( m_countdown, SIGNAL(timeout()), this, SLOT(generateKeyHoldPress()) );
}

// ### Installing ignores auto repeat

/*!
    \fn KeyHold::eventFilter( QObject* o, QEvent* e )
    \internal
*/
bool KeyHold::eventFilter( QObject*, QEvent* e )
{
static bool enabled = true;

    if( enabled ) {
        switch( e->type() )
        {
        case QEvent::KeyPress:
            {
            QKeyEvent *ke = (QKeyEvent*)e;
            if( ke->key() == m_key ) {
                if( !ke->isAutoRepeat() ) {
                    // Start hold countdown
                    m_countdown->start( m_threshold );
                }
                return true;
            }
            }
            break;
        case QEvent::KeyRelease:
            {
            QKeyEvent *ke = (QKeyEvent*)e;
            if( ke->key() == m_key ) {
                if( !ke->isAutoRepeat() ) {
                    // If countdown active, generate key press and key release
                    // Otherwise, generate key hold release
                    if( m_countdown->isActive() ) {
                        m_countdown->stop();
                        enabled = false;
                        QKeyEvent event = QKeyEvent( QEvent::KeyPress, m_key, Qt::NoModifier );
                        QCoreApplication::sendEvent( m_target, &event );
                        event = QKeyEvent( QEvent::KeyRelease, m_key, Qt::NoModifier );
                        QCoreApplication::sendEvent( m_target, &event );
                        enabled = true;
                    } else {
                        QKeyEvent event = QKeyEvent( QEvent::KeyRelease, m_keyHold, Qt::NoModifier );
                        QCoreApplication::sendEvent( m_target, &event );
                    }
                }
                return true;
            }
            }
            break;
        default:
            // Ignore
            break;
        }
    }

    return false;
}

/*!
    \fn KeyHold::generateKeyHoldPress()
    \internal
*/
void KeyHold::generateKeyHoldPress()
{
    QKeyEvent event = QKeyEvent( QEvent::KeyPress, m_keyHold, Qt::NoModifier );
    QCoreApplication::sendEvent( m_target, &event );
}
