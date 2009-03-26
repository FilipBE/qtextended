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

#include "btdialupdevice.h"

#include <QDebug>
#include <QTimer>

#include <qtopialog.h>

#include <qbluetoothlocaldevicemanager.h>
#include <qbluetoothrfcommserialport.h>
#include <qbluetoothrfcommsocket.h>

#include <QBluetoothRemoteDevice>
#include <qbluetoothsdprecord.h>

#include <unistd.h>

BluetoothDialupDevice::BluetoothDialupDevice( QObject* parent )
    : QObject( parent ), btDevice( 0 ), serialPort( 0 ), socket( 0 )
{
    btManager = new QBluetoothLocalDeviceManager( this );
    connect( btManager, SIGNAL(deviceAdded(QString)),
                this, SLOT(devAdded(QString)) );
    connect( btManager, SIGNAL(deviceRemoved(QString)),
                this, SLOT(devRemoved(QString)) );
    knownDevices = btManager->devices();

    reconnectDevice();

    connect( &m_sdap, SIGNAL(searchComplete(QBluetoothSdpQueryResult)),
            this, SLOT(searchComplete(QBluetoothSdpQueryResult)) );
    remoteAddress = QBluetoothAddress::invalid;
}

BluetoothDialupDevice::~BluetoothDialupDevice()
{
}

QString BluetoothDialupDevice::name() const
{
    return btDeviceName;
}

QByteArray BluetoothDialupDevice::rfcommDevice() const
{
    if ( serialPort )
        return serialPort->device().toAscii();
    else
        return QByteArray();
}

bool BluetoothDialupDevice::isAvailable( const QString& devName )
{
    if ( devName.isEmpty() )
        return false;

    if ( btDeviceName == devName
            && btDevice->isValid()
            && btDevice->isUp()
            && knownDevices.contains( devName ) ) {
            return true;
    }

    return false;
}

void BluetoothDialupDevice::devAdded( const QString& devName )
{
    bool wasEmpty = knownDevices.isEmpty();
    if ( !knownDevices.contains( devName ) )
        knownDevices.append( devName );

    if ( wasEmpty ) {
        reconnectDevice();
    }
    if ( knownDevices.count() == 1 && wasEmpty )
        emit deviceStateChanged();
}

void BluetoothDialupDevice::devRemoved( const QString& devName )
{
    if ( knownDevices.isEmpty() )
        return;
    if ( knownDevices.contains( devName ) ) {
        knownDevices.removeAll( devName );
    }

   if ( btDeviceName == devName ) {
        delete btDevice;
        btDevice = 0;
        if ( knownDevices.isEmpty() ) {
            btDeviceName = "";
        } else {
            reconnectDevice();
        }
        emit deviceStateChanged();
    } else {
        //do nothing
    }
}

void BluetoothDialupDevice::deviceStateChanged( QBluetoothLocalDevice::State /*state*/ )
{
    emit deviceStateChanged();
}

void BluetoothDialupDevice::reconnectDevice()
{
    if ( btDevice ) {
        delete btDevice;
        btDevice = 0;
    }
    if ( serialPort ) {
        delete serialPort;
        serialPort = 0;
    }
    remoteAddress = QBluetoothAddress::invalid;

    //this is bad but hcid forces us to do this.
    //deviceAdded() signal is triggered but the bluetooth device still doesn't exist
    //as a consequence defaultDevice would return an empty string.
    int i = 0;
    while ( i < 30 && btDeviceName.isEmpty() ) {
        usleep( 100 );
        btDeviceName = btManager->defaultDevice();
        i++;
    }

    btDevice = new QBluetoothLocalDevice( btDeviceName, this );
    if ( btDevice->isValid() ) {
        connect( btDevice, SIGNAL(stateChanged(QBluetoothLocalDevice::State)),
            this, SLOT(deviceStateChanged(QBluetoothLocalDevice::State)) );
    } else {
        delete btDevice;
        btDevice = 0;
        btDeviceName = "";
    }
}

void BluetoothDialupDevice::connectToDUNService( const QBluetoothAddress& remote )
{
    if ( !btDevice ) {
        qLog(Network) << "Cannot search for Dialup Service due to missing Bluetooth device";
        return;
    }
    qLog(Network) << "Searching for Dialup Networking Profile";
    remoteAddress = remote;
    m_sdap.searchServices( remote, *btDevice, QBluetooth::DialupNetworkingProfile  );
}

void BluetoothDialupDevice::releaseDUNConnection()
{
    if ( serialPort ) {
        serialPort->disconnect();
        serialPort->deleteLater();
        serialPort = 0;
        remoteAddress = QBluetoothAddress::invalid;
    }
}

bool BluetoothDialupDevice::hasActiveConnection()
{
    if ( serialPort
            && !serialPort->device().isEmpty()
            && remoteAddress.isValid() )
        return true;
    return false;
}

void BluetoothDialupDevice::searchComplete( const QBluetoothSdpQueryResult& result )
{
    qLog(Network) << "Search for remote Bluetooth Dialup Networking Service complete";
    foreach( QBluetoothSdpRecord service, result.services() ) {
        if ( service.isInstance( QBluetooth::DialupNetworkingProfile ) ) {
            int channel = QBluetoothSdpRecord::rfcommChannel( service );
            if ( serialPort )
                delete serialPort;

            serialPort = new QBluetoothRfcommSerialPort( this );
            QObject::connect(serialPort, SIGNAL(connected(QString)), this, SLOT(serialPortConnected(QString)));
            QObject::connect(serialPort, SIGNAL(error(QBluetoothRfcommSerialPort::Error)),
                             this, SLOT(serialPortError(QBluetoothRfcommSerialPort::Error)));
            serialPort->connect(btDevice->address(), remoteAddress, channel);
            return;
        }
    }
    //the service doesn't exist
    //cancel connect process
    qLog(Network) << "Target device doesn't provide Dialup Networking Profile";
    serialPortError( QBluetoothRfcommSerialPort::ConnectionFailed );
}

void BluetoothDialupDevice::serialPortConnected( const QString& dev )
{
    qLog(Network) << "Serial device for DUN created: " << dev;
    emit connectionEstablished();
}

void BluetoothDialupDevice::serialPortError(QBluetoothRfcommSerialPort::Error error)
{
    if ((error == QBluetoothRfcommSerialPort::ConnectionFailed) ||
        (error == QBluetoothRfcommSerialPort::ConnectionCancelled) ||
        (error == QBluetoothRfcommSerialPort::CreationError))
    {
        qLog(Network) << "Cannot create serial device for DUN";
        if ( serialPort )
            delete serialPort;
        serialPort = 0;
        remoteAddress = QBluetoothAddress::invalid;

        emit connectionEstablished();
    }
}
