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

#ifndef BLUETOOTHSERIALPORTSERVICE_H
#define BLUETOOTHSERIALPORTSERVICE_H

#include <qbluetoothabstractservice.h>
#include <qbluetoothnamespace.h>

class QBluetoothSerialPortServicePrivate;

class QBluetoothSerialPortService : public QBluetoothAbstractService
{
    Q_OBJECT
public:
    QBluetoothSerialPortService( const QString& serviceID,
                                 const QString& serviceName,
                                 const QBluetoothSdpRecord &record,
                                 QObject* parent = 0 );
    ~QBluetoothSerialPortService();

    void start();
    void stop();
    void setSecurityOptions( QBluetooth::SecurityOptions options );

protected slots:
    void newConnection();

private slots:
    void initiateModemEmulator();
    void emulatorStateChanged();

private:
    QBluetoothSerialPortServicePrivate* d;
};

class BtSerialServiceTask : public QObject
{
    Q_OBJECT
public:
    BtSerialServiceTask( QObject* parent = 0 );
    ~BtSerialServiceTask();

private:
    QBluetoothSerialPortService* provider;
};

#endif
