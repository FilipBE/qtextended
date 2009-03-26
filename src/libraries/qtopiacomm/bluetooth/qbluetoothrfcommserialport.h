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

#ifndef QBLUETOOTHRFCOMMSERIALPORT_H
#define QBLUETOOTHRFCOMMSERIALPORT_H

#include <QObject>
#include <QList>

class QString;

#include <qbluetoothglobal.h>
#include <qbluetoothaddress.h>
#include <qbluetoothrfcommsocket.h>

class QBluetoothRfcommSerialPortPrivate;

class QBLUETOOTH_EXPORT QBluetoothRfcommSerialPort : public QObject
{
    Q_OBJECT
    friend class QBluetoothRfcommSerialPortPrivate;

public:
    enum Flag { KeepAlive = 0x01 };
    Q_DECLARE_FLAGS(Flags, Flag)

    enum Error {
        NoError,
        SocketNotConnected,
        ConnectionFailed,
        ConnectionCancelled,
        CreationError
    };

    explicit QBluetoothRfcommSerialPort(QObject* parent = 0);
    explicit QBluetoothRfcommSerialPort(QBluetoothRfcommSocket *socket,
                                        QBluetoothRfcommSerialPort::Flags flags = 0,
                                        QObject *parent = 0);
    ~QBluetoothRfcommSerialPort();

    bool connect(const QBluetoothAddress &local,
                 const QBluetoothAddress &remote,
                 int channel);
    bool disconnect();

    QString device() const;
    int id() const;
    QBluetoothRfcommSerialPort::Flags flags() const;

    QBluetoothRfcommSerialPort::Error error() const;
    QString errorString() const;

    QBluetoothAddress remoteAddress() const;
    int remoteChannel() const;
    QBluetoothAddress localAddress() const;

    static QList<int> listDevices();
    static QList<int> listDevices(const QBluetoothLocalDevice &device);
    static bool releaseDevice(int id);

signals:
    void connected(const QString &boundDevice);
    void error(QBluetoothRfcommSerialPort::Error err);
    void disconnected();

protected:
    void setError(QBluetoothRfcommSerialPort::Error err);

private:
    QBluetoothRfcommSerialPortPrivate* d;
};

#endif
