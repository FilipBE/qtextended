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

#include "packagescanner.h"
#include "localsocketlistener.h"
#include "deviceupdaterdialog.h"
#include "deviceupdater.h"

#include <QApplication>
#include <QTimer>

#include <qdebug.h>

const char *callName = "device_updater";

static void usage( QString msg )
{
    if ( !msg.isEmpty() )
        qWarning() << "Error:" << msg;
    qWarning() << "Usage:" << callName << "[--list|-l] [--nogui|-n] [package_name]";
    qWarning() << "        simple Qtopia device update tool";
    qWarning() << "  -l|--list - list all packages found";
    qWarning() << "  -n|--nogui - no GUI, just command line";
}

int main(int argc, char **argv)
{
    int result = 1;
    callName = argv[0];

    PackageScanner *ps = new PackageScanner(".");
    if ( argc > 1 && !ps->findPackageByName( argv[1] ))
        usage( QString( "Package %1 not found" ).arg( argv[1] ));

    LocalSocketListener *lls = new LocalSocketListener();
    if ( !lls->listen() ) // app is already running, just signal that instance
    {
        LocalSocket ls;
        if ( argc > 1 && ps->findPackageByName( argv[1] ))
            ls.sendRequest( QString( "SendPackage %1" ).arg( argv[1] ));
        else
            ls.sendRequest( "RaiseWindow" );
        result = 0;
    }
    else
    {
        QApplication app(argc, argv);
        lls->setupNotifier();
        DeviceUpdaterDialog *du = new DeviceUpdaterDialog();
        du->updater()->connectScanner( ps );
        du->updater()->connectLocalSocket( lls );
        if ( argc > 1 && ps->findPackageByName( argv[1] ))
            du->updater()->sendPackage( argv[1] );
        du->show();
        result = app.exec();
        delete du;
    }
    delete ps;
    delete lls;
    return result;
}
