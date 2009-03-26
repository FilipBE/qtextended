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
#include "sysmessages.h"

#ifdef SINGLE_EXEC
QTOPIA_ADD_APPLICATION(QTOPIA_TARGET,sysmessages)
#define MAIN_FUNC main_sysmessages
#else
#define MAIN_FUNC main
#endif

QSXE_APP_KEY
int MAIN_FUNC( int argc, char **argv )
{
    QSXE_SET_APP_KEY(argv[0]);
    QtopiaApplication app(argc, argv);
    new SysMessagesService( &app );
    return app.exec();
}

