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

#include "qabstractbrowserscreen.h"

/*!
  \class QAbstractBrowserScreen
    \inpublicgroup QtBaseModule
  \brief The QAbstractBrowserScreen class allows developers to replace the "application browser screen" portion of the Phone UI.

  The application browser is part of the \l {QtopiaServerApplication#qt-extended-server-widgets}{server widgets framework}
  and represents the portion of the phone UI that users navigate
  through to launch applications or view their documents.
  
  A small tutorial on how to develop new server widgets using one of the abstract widgets as base can
  be found in QAbstractServerInterface class documentation.

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
  \ingroup QtopiaServer::PhoneUI::TTSmartPhone
 */

/*!
  \fn QAbstractBrowserScreen::QAbstractBrowserScreen(QWidget *parent, Qt::WFlags flags)

  Constructs a new QAbstractBrowserScreen instance, with the specified \a parent
  and widget \a flags.
*/

/*!
  \fn QString QAbstractBrowserScreen::currentView() const

  Returns the name of the current view.
 */

/*!
  \fn bool QAbstractBrowserScreen::viewAvailable(const QString &view) const

  Returns true if the browser is capable of switching to \a view.
 */

/*!
  \fn void QAbstractBrowserScreen::resetToView(const QString &view)

  Displays the \a view immediately.  Any logical back or history handling
  should be reset to have \a view as a starting point.
 */

/*!
  \fn void QAbstractBrowserScreen::moveToView(const QString &view)

  Displays the \a view using any appropriate transition effects.  \a view
  should be added to the end of any back or history handing list.
 */

/*!
  \fn void QAbstractBrowserScreen::currentViewChanged(const QString &view)

  Emitted whenever the current view changes to \a view.  This may be caused
  by calls the resetToView() or moveToView(), or by user interaction with the
  browser.
 */

/*!
  \fn void QAbstractBrowserScreen::applicationLaunched(const QString &application)

  Emitted whenever the user launches an \a application through the browser.
 */
