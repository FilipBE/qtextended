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

#ifndef QBLUETOOTHREMOTEDEVICE_H
#define QBLUETOOTHREMOTEDEVICE_H

#include <qbluetoothglobal.h>
#include <qbluetoothnamespace.h>
#include <qbluetoothaddress.h>

#include <QString>
#include <qglobal.h>

class QBluetoothRemoteDevicePrivate;

class QBLUETOOTH_EXPORT QBluetoothRemoteDevice
{
public:
    explicit QBluetoothRemoteDevice(const QBluetoothAddress &address);
    QBluetoothRemoteDevice(const QBluetoothAddress &address,
                           const QString &name,
                           const QString &version,
                           const QString &revision,
                           const QString &manufacturer,
                           const QString &company,
                           int rssi,
                           QBluetooth::DeviceMajor devMajor,
                           quint8 devMinor,
                           QBluetooth::ServiceClasses serviceClasses);
    QBluetoothRemoteDevice(const QBluetoothRemoteDevice &other);

    ~QBluetoothRemoteDevice();

    QBluetoothRemoteDevice &operator=(const QBluetoothRemoteDevice &other);
    bool operator==(const QBluetoothRemoteDevice &other) const;
    bool operator!=(const QBluetoothRemoteDevice &other) const
    {
        return !operator==(other);
    }

    QString name() const;
    void setName(const QString &name);

    QString version() const;
    void setVersion(const QString &version);

    QString revision() const;
    void setRevision(const QString &revision);

    QString manufacturer() const;
    void setManufacturer(const QString &manufacturer);

    QString company() const;
    void setCompany(const QString &company);

    int rssi() const;
    void setRssi(int rssi);

    QBluetooth::DeviceMajor deviceMajor() const;
    QString deviceMajorAsString() const;
    void setDeviceMajor(QBluetooth::DeviceMajor deviceMajor);

    quint8 deviceMinor() const;
    QString deviceMinorAsString() const;
    void setDeviceMinor(quint8 deviceMinor);

    QBluetooth::ServiceClasses serviceClasses() const;
    QStringList serviceClassesAsString() const;
    void setServiceClasses(QBluetooth::ServiceClasses serviceClasses);

    QBluetoothAddress address() const;

private:
    QBluetoothRemoteDevice() {}
    QBluetoothRemoteDevicePrivate *m_data;
};

#endif
