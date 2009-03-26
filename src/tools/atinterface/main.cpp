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

#include <qtopiaapplication.h>
#include "atinterface.h"

#ifdef SINGLE_EXEC
QTOPIA_ADD_APPLICATION(QTOPIA_TARGET,atinterface)
#define MAIN_FUNC main_atinterface
#else
#define MAIN_FUNC main
#endif

QSXE_APP_KEY

int MAIN_FUNC(int argc, char **argv)
{
    QSXE_SET_APP_KEY(argv[0])
    QtopiaApplication a(argc, argv );
    bool testMode = false;
    QString startupOptions;
    if ( argc > 1 && QString(argv[1]) == "--test" ) {
        testMode = true;
        if ( argc > 2 )
            startupOptions = argv[2];
    }
    AtInterface iface( testMode, startupOptions, &a );
    return a.exec();
}
