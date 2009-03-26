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

#ifdef QTOPIA_BLUETOOTH

#include <qwindowsystem_qws.h>

#include <QtopiaIpcAdaptor>
#include <QProcess>

#include "systemsuspend.h"

#include <qbluetoothlocaldevice.h>
#include <qvaluespace.h>
#include <qtopialog.h>

class BluetoothSuspend : public SystemSuspendHandler
{
public:
    BluetoothSuspend();
    ~BluetoothSuspend();

    virtual bool canSuspend() const;
    virtual bool suspend();
    virtual bool wake();

private:
    QProcess m_process;
};

QTOPIA_TASK(BluetoothSuspend, BluetoothSuspend)
QTOPIA_TASK_PROVIDES(BluetoothSuspend, SystemSuspendHandler)

BluetoothSuspend::BluetoothSuspend()
{
    QObject::connect(&m_process, SIGNAL(finished(int)), this, SIGNAL(operationCompleted()));
}

BluetoothSuspend::~BluetoothSuspend()
{
}

bool BluetoothSuspend::canSuspend() const
{
    QByteArray path("/Hardware/Devices/");
    QBluetoothLocalDevice dev;

    path.append(dev.deviceName());
    QValueSpaceItem activeSessions(path);

    bool ret = !activeSessions.value("ActiveSessions", false).toBool();

    qLog(Bluetooth) << "Bluetooth::canSuspend returning: " << ret;

    return ret;
}

bool BluetoothSuspend::suspend()
{
    qLog(Bluetooth) << "Bluetooth Suspend called";
    return true;
}

bool BluetoothSuspend::wake()
{
    qLog(Bluetooth) << "Wake called!!!";

    m_process.start("bluetooth-wakeup.sh");

    qLog(Bluetooth) << "Successfully woken up the device";

    return false;
}

#endif
