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

#include "sxemonitor.h"
#include "sxemonqlog.h"
#include <qtopiaapplication.h>

QSXE_APP_KEY
int main(int argc, char* argv[])
{
    QSXE_SET_APP_KEY(argv[0])

    QtopiaApplication* app = new QtopiaApplication(argc, argv);
    app->registerRunningTask("sxemonitor");
    SxeMonitor *monitor = new SxeMonitor();
    Q_UNUSED( monitor );
    
    int returnVal = app->exec();
    return returnVal;
}
