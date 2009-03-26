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

#include "qabstractdialerscreen.h"

/*!
  \class QAbstractDialerScreen
    \inpublicgroup QtTelephonyModule
  \brief The QAbstractDialerScreen class allows developers to replace the "dialer screen" portion of the Phone UI.

  The QAbstractDialerScreen interface is part of the 
  \l {QtopiaServerApplication#qt-extended-server-widgets}{server widgets framework} and allows developers 
  to replace the standard touchscreen or quick dialers in the Qt Extended phone UI.  
  A small tutorial on how to develop new server widgets using one of the abstract widgets as base can
  be found in QAbstractServerInterface class documentation.
  
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
  \ingroup QtopiaServer::PhoneUI::TTSmartPhone
 */

/*!
  \fn QAbstractDialerScreen::QAbstractDialerScreen(QWidget *parent = 0, Qt::WFlags flags = 0)

  Constructs a new QAbstractDialerScreen instance, with the specified \a parent
  and widget \a flags.
 */

/*!
  \fn QString QAbstractDialerScreen::digits() const

  Returns the currently displayed number from the dialer screen.
 */

/*!
  \fn void QAbstractDialerScreen::reset()

  Resets any displayed number and return to the screens initial view.  This is
  always called before displaying the screen to the user.
 */

/*!
    \fn void QAbstractDialerScreen::appendDigits(const QString &digits)

    Appends \a digits to the current number.
 */

/*!
  \fn void QAbstractDialerScreen::setDigits(const QString &digits)

  Sets the current number to  \a digits.
 */

/*!
    \fn void QAbstractDialerScreen::doOffHook()

    Implements the behavior when the phone handset is taken off its hook.

    \sa DialProxy::onHookGesture()
*/

/*!
    \fn void QAbstractDialerScreen::doOnHook()

    Implements the behavior when the phone handset is put on its hook.

    \sa DialProxy::offHookGesture()
*/

/*!
  \fn void QAbstractDialerScreen::requestDial(const QString &number, const QUniqueId &contact)

  Emitted whenever the user has selected a \a number to dial from the dial
  screen.  If valid, \a contact may be set to the unique id of the contact to
  which the number refers.  The dialer is generally expected to QWidget::close()
  after emitting this signal.
 */

/*!
    \fn void QAbstractDialerScreen::speedDial(const QString &number)

    Emitted whenever the user speed dials a number from the dial screen. A speed dial is
    usually characterized by a press and hold action on a keypad number. Qt Extended will resolve the \a number into a speed dial action and perform it
    appropriately.
*/
