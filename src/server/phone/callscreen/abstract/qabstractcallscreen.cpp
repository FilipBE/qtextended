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

#include "qabstractcallscreen.h"

/*!
  \class QAbstractCallScreen
    \inpublicgroup QtTelephonyModule
  \brief The QAbstractCallScreen class allows developers to replace the
  call management screen.

  The QAbstractDialerScreen interface is part of the
  \l {QtopiaServerApplication#qt-extended-server-widgets}{server widgets framework}.
  It allows replacement of the standard call screen in the Qt Extended phone UI.

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
  \ingroup QtopiaServer::PhoneUI::TTSmartPhone
 */

/*!
  \fn QAbstractCallScreen::QAbstractCallScreen(QWidget *parent = 0, Qt::WFlags flags = 0)

  Constructs a new QAbstractDialerScreen instance, with the specified \a parent
  and widget \a flags.
 */

/*!
    \fn void QAbstractCallScreen::acceptIncoming()

    Emitted when the user accepts an incoming call.
 */

/*!
    \fn void QAbstractCallScreen::hangupCall()

    Emitted when the user hangs the call up.
*/

/*!
  Notifies the call screen that the call state has changed and the call
  display should be refreshed.
 */
void QAbstractCallScreen::stateChanged()
{
}

