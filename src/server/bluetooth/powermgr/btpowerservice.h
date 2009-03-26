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

#ifndef BTPOWERSERVICE_H
#define BTPOWERSERVICE_H

#include "qabstractdevicemanager.h"
#include <qbluetoothlocaldevice.h>


class BtPowerServicePrivate;
class BtPowerService : public QAbstractCommDeviceManager
{
    Q_OBJECT

public:
    BtPowerService(const QByteArray &serverPath,
                    const QByteArray &devId,
                    QObject *parent = 0);
    ~BtPowerService();

    virtual void bringUp();
    virtual void bringDown();
    virtual bool isUp() const;
    virtual bool shouldBringDown(QUnixSocket *) const;

private slots:
    void stateChanged(QBluetoothLocalDevice::State state);
    void error(QBluetoothLocalDevice::Error error, const QString& msg);
    void planeModeChanged(bool enabled);

private:
    BtPowerServicePrivate *m_data;
};

class QValueSpaceItem;
class BtPowerServiceTask : public QObject
{
    Q_OBJECT

public:
    BtPowerServiceTask(QObject *parent = 0);
    ~BtPowerServiceTask();

private slots:
    void delayedServiceStart();
    void deviceAdded(const QString &devName);
    void deviceRemoved(const QString &devName);
    void defaultDeviceChanged(const QString &devName);
    void startService();

private:
    BtPowerService *m_btPower;
    QValueSpaceItem *serverWidgetVsi;
};

#endif
