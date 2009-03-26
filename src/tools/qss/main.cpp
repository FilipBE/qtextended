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

#include <qsoundqss_qws.h>
#include <qtopiaapplication.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#ifdef SINGLE_EXEC
QTOPIA_ADD_APPLICATION(QTOPIA_TARGET,mediaserver)
#define MAIN_FUNC main_mediaserver
#else
#define MAIN_FUNC main
#endif

QSXE_APP_KEY

int MAIN_FUNC(int argc, char **argv)
{
    QSXE_SET_APP_KEY(argv[0])

    QApplication a(argc, argv);
    (void)new QWSSoundServer(0);

    //if (1) {
    // later, check if root first
        setpriority(PRIO_PROCESS, 0, -15);
    //}
    // hook up some quit mech.
    return a.exec();
}

