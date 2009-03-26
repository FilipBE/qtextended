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

#include "pressholdgate.h"
#include <QDeviceButtonManager>
#include <QtopiaApplication>
#include "qtopiainputevents.h"

/*!
  \class PressHoldGate
    \inpublicgroup QtBaseModule
  \brief The PressHoldGate class encapsulates the processing of a key with press and press-and-hold actions.
  \ingroup QtopiaServer
  \internal
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
 */

/*! \internal */
void PressHoldGate::timerEvent(QTimerEvent* e)
{
    if ( e->timerId() == held_tid ) {
        killTimer(held_tid);
        // button held
        if ( held_key ) {
            emit activate(held_key, true, true);
            held_key = 0;
        }
        held_tid = 0;
    }

    QObject::timerEvent(e);
}

bool PressHoldGate::filterKey(int keycode, bool pressed, bool pressable, bool holdable, bool releasable)
{
    bool filterout=true;
    if ( held_tid ) {
        killTimer(held_tid);
        held_tid = 0;
    }
    if ( !holdable ) {
        if ( !pressable ) {
            filterout=false;
        } else {
            emit activate(keycode, false, pressed);
        }
    } else if ( pressed ) {
        if ( held_key ) {
            // If QWSServer::sendKeyEvent is changed to QWSServer::processKeyEvent,
            // this would happen.
            filterout=false;
        } else {
            held_key = keycode;
            if ( holdable ) {
                held_tid = startTimer(500);
            }
        }
    } else if ( held_key ) {
        if ( pressable ) {
            held_key = 0;
            emit activate(keycode, false, true);
        } else {
            if ( hardfilter ) // send the press now...
                QtopiaInputEvents::sendKeyEvent(0, keycode, 0, true, false);
            held_key = 0;
            filterout=false;
        }
    }
    else if (releasable && !pressed)
    {
        emit activate(keycode, false, false);
    }

    return filterout;
}

/*!
 \internal
*/
bool PressHoldGate::filterDeviceButton(int keycode, bool press, bool autorepeat)
{
    const QDeviceButton* button =
        QDeviceButtonManager::instance().buttonForKeycode(keycode,context);

    if (button) {
        bool pressable=!button->pressedAction().isNull();
        bool holdable=!button->heldAction().isNull();
        bool releasable = !button->releasedAction().isNull();
        if ( autorepeat || filterKey(keycode,press,pressable,holdable, releasable) )
            return true;
    }

    return false;
}

