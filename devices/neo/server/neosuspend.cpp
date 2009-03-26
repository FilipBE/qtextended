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

#include <QtopiaIpcAdaptor>
#include <QtopiaIpcEnvelope>
#include <QtopiaServiceRequest>
#include <QPowerSource>
#include <QProcess>
#include <stdio.h>
#include <stdlib.h>
#include <QDesktopWidget>
#include <QTimer>


#include "systemsuspend.h"

#include <qvaluespace.h>

#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif

class NeoSuspend : public SystemSuspendHandler
{
public:

  NeoSuspend();
    virtual bool canSuspend() const;
    virtual bool suspend();
    virtual bool wake();
private slots:
};

QTOPIA_DEMAND_TASK(NeoSuspend, NeoSuspend);
QTOPIA_TASK_PROVIDES(NeoSuspend, SystemSuspendHandler);

NeoSuspend::NeoSuspend()
{
}

bool NeoSuspend::canSuspend() const
{
/*    QPowerSource src( QLatin1String("DefaultBattery") );
    return !src.charging();
*/
    return true;
}

bool NeoSuspend::suspend()
{
    qLog(PowerManagement)<< __PRETTY_FUNCTION__;

      QProcess apm;
      apm.start("apm", QStringList() << "-s");
      apm.waitForFinished(-1);
/*      QFile powerStateFile("/sys/power/state");
    if( !powerStateFile.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)) {
        qWarning()<<"File not opened";
    } else {
        QTextStream out(&powerStateFile);
        out << "mem";
        powerStateFile.close();
    }
*/
    return true;
}

bool NeoSuspend::wake()
{
#ifdef Q_WS_QWS
    QWSServer::instance()->refresh();
#endif
    QtopiaIpcEnvelope("QPE/Card", "mtabChanged()" ); // might have changed while asleep

   //screenSaverActivate uses QTimer which depends on hwclock update
    //when the device wakes up. The clock update is another event in the
    //Qt event loop (see qeventdispatcher_unix.cpp. We have to call
    //processEvents to give Qt a chance to sync its internal timer with
    //the hw clock
#ifdef Q_WS_QWS
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    QWSServer::screenSaverActivate( false );
    {
        QtopiaIpcEnvelope("QPE/Card", "mtabChanged()" ); // might have changed while asleep
        QtopiaServiceRequest e("QtopiaPowerManager", "setBacklight(int)");
        e << -3; // Force on
        e.send();
        QtopiaIpcEnvelope("QPE/NetworkState", "updateNetwork()"); //might have changed
    }
#endif
    return true;
}
