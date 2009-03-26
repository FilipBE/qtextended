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

#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>

#include <QtopiaApplication>
#include <QDrmContentPlugin>
#include <custom.h>

#include "domainmanager.h"
#include "sessionmanager.h"
#include "qtopiamediaprovider.h"
#include "qsoundprovider.h"
#include "mediavolumecontrol.h"
#include "qaudiointerfaceserver.h"

#ifndef QTOPIA_NO_MEDIAPOWERCONTROL
#include "mediapowercontrol.h"
#endif

#ifndef MEDIASERVER_PRIORITY
#define MEDIASERVER_PRIORITY 0
#endif

#ifdef SINGLE_EXEC
QTOPIA_ADD_APPLICATION(QTOPIA_TARGET,mediaserver)
#define MAIN_FUNC main_mediaserver
#else
#define MAIN_FUNC main
#endif

using namespace mediaserver;


QSXE_APP_KEY
int MAIN_FUNC(int argc, char** argv)
{
    int which = PRIO_PROCESS;
    id_t pid;
    int ret;

    pid = getpid();
    ret = getpriority(which, pid);
    setpriority(which, pid, ret - MEDIASERVER_PRIORITY );

    QSXE_SET_APP_KEY(argv[0])

    QtopiaApplication   app(argc, argv);

    // Set the preferred document system connection type
    QTOPIA_SET_DOCUMENT_SYSTEM_CONNECTION();

#ifndef QT_NO_SXE
    app.setSxeAuthorizerRole( QtopiaApplication::SxeAuthorizerServerAndClient );
#endif

    DomainManager::instance();  // force construct of DomainManager

    QtopiaMediaProvider     qmp(SessionManager::instance());
    QSoundProvider          qsp(SessionManager::instance());
    //MediaVolumeControl      mvc(SessionManager::instance());
    QAudioInterfaceServer   qas(SessionManager::instance());

#ifndef QTOPIA_NO_MEDIAPOWERCONTROL
    MediaPowerControl       mpc;

    QObject::connect(SessionManager::instance(), SIGNAL(activeSessionCountChanged(int)),
                     &mpc, SLOT(activeSessionCount(int)));
#endif

    app.registerRunningTask("SessionManager", SessionManager::instance());

    QDrmContentPlugin::initialize();

    return app.exec();
}

