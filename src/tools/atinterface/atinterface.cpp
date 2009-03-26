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

#include "atinterface.h"
#include "atsessionmanager.h"
#include "atfrontend.h"
#include "atcommands.h"
#include <qapplication.h>
#include <qtopialog.h>

AtInterface::AtInterface
        ( bool testMode, const QString& startupOptions, QObject *parent )
    : QObject( parent )
{
    qLog(ModemEmulator) << "atinterface starting up";

    manager = new AtSessionManager( this );
    connect( manager, SIGNAL(newSession(AtFrontEnd*)),
             this, SLOT(newSession(AtFrontEnd*)) );

    // Bind to port 12350 if we are simply testing the AT interface.
    if ( testMode ) {
        if ( !manager->addTcpPort( 12350, true, startupOptions ) ) {
            qWarning( "Could not bind to AT interface test port" );
            QApplication::quit();
            return;
        }
    }
}

AtInterface::~AtInterface()
{
    qLog(ModemEmulator) << "atinterface shutting down";
}

void AtInterface::newSession( AtFrontEnd *session )
{
    // Wrap the session in an AtCommands object.  It will be
    // automatically cleaned up when the session is destroyed.
    new AtCommands( session, manager );
}
