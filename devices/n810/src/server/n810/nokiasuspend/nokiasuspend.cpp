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

#include <qwindowsystem_qws.h>

#include <QtopiaIpcAdaptor>
#include <QtopiaIpcEnvelope>
#include <QtopiaServiceRequest>
#include <QPowerSource>
#include <QProcess>

#include "systemsuspend.h"

#include <qvaluespace.h>

class NokiaSuspend : public SystemSuspendHandler
{
public:
    NokiaSuspend();
    virtual bool canSuspend() const;
    virtual bool suspend();
    virtual bool wake();
};

QTOPIA_DEMAND_TASK(NokiaSuspend, NokiaSuspend);
QTOPIA_TASK_PROVIDES(NokiaSuspend, SystemSuspendHandler);

NokiaSuspend::NokiaSuspend()
{
}

bool NokiaSuspend::canSuspend() const
{
    QPowerSource src( QLatin1String("N800Battery") );
    return !src.charging();
}


//also has hybernate "mem"
bool NokiaSuspend::suspend()
{
    QFile powerStateFile("/sys/power/state");
    if( !powerStateFile.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)) {
        qWarning()<<"File not opened";
    } else {
        QTextStream out(&powerStateFile);
        out << "standby";
        powerStateFile.close();
    }

    return true;

}

bool NokiaSuspend::wake()
{
    QWSServer::screenSaverActivate( false );
    {
        QtopiaIpcEnvelope("QPE/Card", "mtabChanged()" ); // might have changed while asleep
        QtopiaServiceRequest e("QtopiaPowerManager", "setBacklight(int)");
        e << -3; // Force on
        e.send();
    }

    return true;
}

