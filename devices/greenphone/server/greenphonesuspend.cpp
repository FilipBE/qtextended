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
#include <QPowerStatus>

#include "systemsuspend.h"
#include "custom.h"

#include <qvaluespace.h>

#include "../include/ipmc.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define _LCDCTRL_IOCTL_BRIGHTNESS 4

class GreenphoneSuspend : public SystemSuspendHandler
{
public:
    GreenphoneSuspend();
    virtual bool canSuspend() const;
    virtual bool suspend();
    virtual bool wake();
};

QTOPIA_DEMAND_TASK(GreenphoneSuspend, GreenphoneSuspend);
QTOPIA_TASK_PROVIDES(GreenphoneSuspend, SystemSuspendHandler);

GreenphoneSuspend::GreenphoneSuspend()
{
}

bool GreenphoneSuspend::canSuspend() const
{
    QPowerSource src( QLatin1String("DefaultBattery") );
    return !src.charging();
}

bool GreenphoneSuspend::suspend()
{
    QSettings config("Trolltech","qpe");
    QPowerStatus powerstatus;
    if (powerstatus.wallStatus() == QPowerStatus::Available)
        config.beginGroup( "ExternalPower" );
    else
        config.beginGroup( "BatteryPower" );

    int bright = config.value("Brightness", 255).toInt();
    bright = bright * qpe_sysBrightnessSteps() / 255;

    if (bright > 0) {
        int lcdFd = ::open("/dev/lcdctrl", O_RDWR);
        if(lcdFd >= 0) {
            ::ioctl(lcdFd, 4, bright-1);
            ::close(lcdFd);
        }
    }

    QProcess apm;
    apm.start("apm", QStringList() << "--suspend");
    apm.waitForFinished(-1);
    return true;
}

bool GreenphoneSuspend::wake()
{
    QtopiaIpcEnvelope("QPE/Card", "mtabChanged()" ); // might have changed while asleep
    return true;
}

