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

#include "activitymonitor_p.h"

/*!
    \class ActivityMonitor
    \inpublicgroup QtMediaModule
    \internal
*/

/*!
    \fn void ActivityMonitor::update()
    \internal
*/
void ActivityMonitor::update()
{
    if( isActive() ) {
        m_updateCalled = true;
    } else {
        m_active = true;
        emit active();

        // Start polling for activity
        m_timer = startTimer( m_interval );
    }
}

/*!
    \fn void ActivityMonitor::timerEvent( QTimerEvent* e )
    \internal
*/
void ActivityMonitor::timerEvent( QTimerEvent* e )
{
    if( e->timerId() == m_timer ) {
        if( m_updateCalled ) {
            m_updateCalled = false;
        } else {
            // Stop polling for activity
            killTimer( m_timer );

            m_active = false;
            emit inactive();
        }
    }
}
