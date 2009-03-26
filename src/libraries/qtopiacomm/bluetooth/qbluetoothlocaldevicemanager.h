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

#ifndef QBLUETOOTHLOCALDEVICEMANAGER_H
#define QBLUETOOTHLOCALDEVICEMANAGER_H

#include <qobject.h>
#include <qglobal.h>
#include <QString>

#include <qbluetoothglobal.h>

class QBluetoothLocalDeviceManager_Private;

class QBLUETOOTH_EXPORT QBluetoothLocalDeviceManager : public QObject
{
    Q_OBJECT
    friend class QBluetoothLocalDeviceManager_Private;

public:
    explicit QBluetoothLocalDeviceManager(QObject *parent = 0);

    ~QBluetoothLocalDeviceManager();

    QStringList devices();
    QString defaultDevice();

signals:
    void deviceAdded(const QString &device);
    void deviceRemoved(const QString &device);
    void defaultDeviceChanged(const QString &device);

private:
    QBluetoothLocalDeviceManager_Private *m_data;
};

#endif
