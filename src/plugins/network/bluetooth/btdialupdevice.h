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

#ifndef BTDIALUPDEVICE_H
#define BTDIALUPDEVICE_H

#include <QByteArray>
#include <QObject>
#include <QStringList>

#include <QBluetoothAddress>
#include <QBluetoothLocalDevice>
#include <QBluetoothSdpQuery>
#include <QBluetoothRfcommSerialPort>

class QBluetoothLocalDeviceManager;
class QBluetoothRfcommSocket;

class BluetoothDialupDevice : public QObject
{
    Q_OBJECT
public:
    BluetoothDialupDevice( QObject* parent = 0 );
    ~BluetoothDialupDevice();

    QString name() const;
    bool isAvailable( const QString& devName );
    void connectToDUNService( const QBluetoothAddress& remote );
    void releaseDUNConnection();
    bool hasActiveConnection();
    QByteArray rfcommDevice() const;

signals:
    void deviceStateChanged();
    void connectionEstablished();

private slots:
    void devAdded( const QString& devName );
    void devRemoved( const QString& devName );
    void deviceStateChanged( QBluetoothLocalDevice::State state );
    void searchComplete( const QBluetoothSdpQueryResult& result );
    void serialPortConnected( const QString& boundDevice );
    void serialPortError(QBluetoothRfcommSerialPort::Error error);

private:
    void reconnectDevice();

private:
    QStringList knownDevices;
    QBluetoothLocalDeviceManager* btManager;
    QBluetoothLocalDevice* btDevice;
    QBluetoothAddress remoteAddress;
    QBluetoothRfcommSerialPort* serialPort;
    QBluetoothRfcommSocket *socket;
    QString btDeviceName;
    QBluetoothSdpQuery m_sdap;
};

#endif
