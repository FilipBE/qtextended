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

#ifndef BLUETOOTHSERVICEMANAGER_H
#define BLUETOOTHSERVICEMANAGER_H

#include <qbluetoothnamespace.h>
#include <QObject>

class QValueSpaceObject;
class QSettings;
class ServiceMessenger;
class ServiceUserMessenger;
class BluetoothServiceSettings;

class BluetoothServiceManager : public QObject
{
    Q_OBJECT

public:
    BluetoothServiceManager(QObject *parent = 0);
    ~BluetoothServiceManager();

    void registerService(const QString &name, const QString &displayName);

    void startService(const QString &name);
    void serviceStarted(const QString &name, bool error, const QString &errorDesc);
    void serviceStopped(const QString &name);

    void stopService(const QString &name);
    void setServiceSecurity(const QString &name, QBluetooth::SecurityOptions options);

    BluetoothServiceSettings *m_settings;
    ServiceMessenger *m_serviceIpc;
    ServiceUserMessenger *m_serviceUserIpc;
};

#endif
