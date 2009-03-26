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

#include "virtualkeyboardservice.h"
#include <qtopiaipcenvelope.h>
#include "qtopiainputevents.h"
#include "qtopiaserverapplication.h"

/*!
  \service VirtualKeyboardService VirtualKeyboard
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::Task
  \brief The VirtualKeyboardService class provides a service to simulate keypresses on a device.

  The \c {VirtualKeyboard} service provided by the VirtualKeyboardService class
  allows applications to cause the equivalent of physical keypresses on a
  device.

  The VirtualKeyboardService class provides the \c {VirtualKeyboard} task.
  It is part of the Qt Extended server and cannot be used by other Qt Extended applications.
 */

/*!
  \internal
  Construct a new VirtualKeyboardService instance with the provided \a parent.
 */
VirtualKeyboardService::VirtualKeyboardService( QObject *parent )
    : QtopiaAbstractService( "VirtualKeyboard", parent )
{
    publishAll();
}

/*!
  \internal
  Destroys the VirtualKeyboardService instance.
 */
VirtualKeyboardService::~VirtualKeyboardService()
{
}

/*!
  Simulate the press and release of \a key.  This is equivalent to calling the
  QtopiaInputEvents::processKeyEvent() method for a press and then a release.

  This slot corresponds to the QCop service message \c{VirtualKeyboard::keyPress(int)}.
 */
void VirtualKeyboardService::keyPress( int key )
{
    QtopiaInputEvents::processKeyEvent(QChar(key).unicode(), key, 0, true, false);
    QtopiaInputEvents::processKeyEvent(QChar(key).unicode(), key, 0, false, false);
}

/*!
  Simulate the press and release of \a keySequence.  This is equivalent to calling the
  QtopiaInputEvents::processKeyEvent() method for a press and then a release.

  This slot corresponds to the QCop service message \c{VirtualKeyboard::keyPress(QString)}.

  \sa QKeySequence
 */
void VirtualKeyboardService::keyPress( QString keySequence )
{
    QKeySequence ks(keySequence);
    for (uint i = 0; i < ks.count(); ++i) {
        QtopiaInputEvents::processKeyEvent(0xffff, ks[i], 0, true, false);
        QtopiaInputEvents::processKeyEvent(0xffff, ks[i], 0, false, false);
    }
}

/*!
    Send a key event consisting of \a unicode, \a keycode,
    \a modifiers, \a isPress, and \a autoRepeat.

    This slot corresponds to the QCop service message
    \c{VirtualKeyboard::sendKeyEvent(int,int,int,bool,bool)}.
*/
void VirtualKeyboardService::sendKeyEvent
    ( int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat )
{
    QtopiaInputEvents::sendKeyEvent
        ( unicode, keycode, (Qt::KeyboardModifiers)modifiers,
          isPress, autoRepeat );
}

/*!
    Process a key event consisting of \a unicode, \a keycode,
    \a modifiers, \a isPress, and \a autoRepeat.

    This slot corresponds to the QCop service message
    \c{VirtualKeyboard::processKeyEvent(int,int,int,bool,bool)}.
*/
void VirtualKeyboardService::processKeyEvent
    ( int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat )
{
    QtopiaInputEvents::processKeyEvent
        ( unicode, keycode, (Qt::KeyboardModifiers)modifiers,
          isPress, autoRepeat );
}

QTOPIA_TASK(VirtualKeyboard, VirtualKeyboardService);
