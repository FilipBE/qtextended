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

#include <QKeyEvent>

#include "keyfilter.h"

namespace mediaplayer
{

/*!
    \class mediaplayer::KeyFilter
    \internal
*/

KeyFilter::KeyFilter( QObject* subject, QObject* target, QObject* parent )
    : QObject( parent ), m_target( target )
{
    subject->installEventFilter( this );
}

void KeyFilter::addKey( int key )
{
    m_keys.insert( key );
}

bool KeyFilter::eventFilter( QObject*, QEvent* e )
{
    // Guard against recursion
    static QEvent* d = 0;

    if( e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease ) {
        QKeyEvent *ke = (QKeyEvent*)e;
        if( d != e && !ke->isAutoRepeat() && m_keys.contains( ke->key() ) ) {
            QKeyEvent event = QKeyEvent( e->type(), ke->key(), Qt::NoModifier );
            QCoreApplication::sendEvent( m_target, d = &event );
            d = 0;
            return true;
        }
    }

    return false;
}

} // ns mediaplayer
