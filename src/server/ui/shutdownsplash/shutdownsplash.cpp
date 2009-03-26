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

#include "shutdownsplash.h"
#include <QWaitWidget>

/*!
  \class ShutdownSplashScreen
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::GeneralUI
  \brief The ShutdownSplashScreen class displays a static splash screen while the system is restarting.

  The ShutdownSplashScreen provides a Qt Extended Server Task.  Qt Extended Server Tasks 
  are documented in full in the QtopiaServerApplication class documentation.

  \table
  \row \o Task Name \o ShutdownSplashScreen
  \row \o Interfaces \o SystemShutdownHandler
  \row \o Services \o None
  \endtable

  The ShutdownSplashScreen class displays a full screen image,
  while the system is restarting.  It uses the
  SystemShutdownHandler hook to ensure that it is run during restart.  For best
  results, the \c {ShutdownSplashScreen} task should be given highest priority
  for the SystemShutdownHandler to ensure that it appears as soon as possible.

  \i {Note:} The shutdown splash screen only appears on system restart and NOT
  on system shutdown.
  
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
 */

/*! \internal */
bool ShutdownSplashScreen::systemRestart()
{
    QWaitWidget* w = new QWaitWidget( 0 );
    w->setText(tr("Please wait..."));
    w->setWindowFlags(Qt::WindowStaysOnTopHint);
    w->setWindowState(Qt::WindowFullScreen);
    w->setAttribute( Qt::WA_DeleteOnClose );
    w->setCancelEnabled( false );
    w->showFullScreen();
    return true;
}

QTOPIA_TASK(ShutdownSplashScreen, ShutdownSplashScreen);
QTOPIA_TASK_PROVIDES(ShutdownSplashScreen, SystemShutdownHandler);
