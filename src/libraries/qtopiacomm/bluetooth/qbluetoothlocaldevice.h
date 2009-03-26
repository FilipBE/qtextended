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

#ifndef QBLUETOOTHLOCALDEVICE_H
#define QBLUETOOTHLOCALDEVICE_H

#include <qbluetoothglobal.h>
#include <qbluetoothnamespace.h>

#include <qobject.h>
#include <qglobal.h>
#include <QString>

class QBluetoothRemoteDevice;
template <class T> class QList;
class QBluetoothLocalDevice_Private;
class QDateTime;
class QBluetoothAddress;

template <typename T> class QBLUETOOTH_EXPORT QBluetoothReply {
public:
    QBluetoothReply(const T &value) : m_isError(false), m_value(value) {}
    QBluetoothReply() : m_isError(true) {}

    bool isError() const { return m_isError; }
    const T &value() const { return m_value; }

    operator const T& () const { return m_value; }

private:
    bool m_isError;
    T m_value;
};

class QBLUETOOTH_EXPORT QBluetoothLocalDevice : public QObject
{
    Q_OBJECT
    friend class QBluetoothLocalDevice_Private;

public:

    enum State {
        Off,
        Connectable,
        Discoverable
    };

    enum Error {
        NoError = 0,
        InvalidArguments,
        NotAuthorized,
        OutOfMemory,
        NoSuchAdaptor,
        UnknownAddress,
        ConnectionAttemptFailed,
        NotConnected,
        AlreadyExists,
        DoesNotExist,
        InProgress,
        AuthenticationFailed,
        AuthenticationTimeout,
        AuthenticationRejected,
        AuthenticationCancelled,
        UnknownError
    };

    explicit QBluetoothLocalDevice(QObject *parent = 0);
    explicit QBluetoothLocalDevice(const QBluetoothAddress &addr, QObject* parent=0 );
    explicit QBluetoothLocalDevice(const QString &devName, QObject* parent = 0 );

    ~QBluetoothLocalDevice();

    bool isValid() const;
    QBluetoothLocalDevice::Error error() const;
    QString errorString() const;

    QString deviceName() const;
    QBluetoothAddress address() const;

    QBluetoothReply<QString> manufacturer() const;
    QBluetoothReply<QString> version() const;
    QBluetoothReply<QString> revision() const;
    QBluetoothReply<QString> company() const;

    QBluetoothReply<QString> name() const;
    bool setName(const QString &name);

    bool setDiscoverable(uint timeout = 0);
    QBluetoothReply<uint> discoverableTimeout() const;
    QBluetoothReply<bool> discoverable() const;
    bool setConnectable();
    QBluetoothReply<bool> connectable() const;
    bool turnOff();
    QBluetoothReply<bool> isUp();

    QBluetoothReply<bool> isConnected(const QBluetoothAddress &addr) const;
    QBluetoothReply<QList<QBluetoothAddress> > connections() const;

    QBluetoothReply<QDateTime> lastSeen(const QBluetoothAddress &addr) const;
    QBluetoothReply<QDateTime> lastUsed(const QBluetoothAddress &addr) const;
    bool updateRemoteDevice(QBluetoothRemoteDevice &device) const;

    bool requestPairing(const QBluetoothAddress &addr);
    bool removePairing(const QBluetoothAddress &addr);
    QBluetoothReply<QList<QBluetoothAddress> > pairedDevices() const;
    QBluetoothReply<bool> isPaired(const QBluetoothAddress &addr) const;
    bool cancelPairing(const QBluetoothAddress &addr);

    QBluetoothReply<QString> remoteAlias(const QBluetoothAddress &addr) const;
    bool setRemoteAlias(const QBluetoothAddress &addr, const QString &alias);
    bool removeRemoteAlias(const QBluetoothAddress &addr);

    QBluetoothReply<QList<QBluetoothAddress> > knownDevices() const;
    QBluetoothReply<QList<QBluetoothAddress> > knownDevices(const QDateTime &time) const;

    bool setPeriodicDiscoveryEnabled(bool enabled);
    QBluetoothReply<bool> isPeriodicDiscoveryEnabled() const;

    QBluetoothReply<uchar> pinCodeLength(const QBluetoothAddress &addr) const;

    bool disconnectRemoteDevice(const QBluetoothAddress &addr);

public slots:
    bool discoverRemoteDevices();
    bool cancelDiscovery();

signals:
    void nameChanged(const QString &name);
    void stateChanged(QBluetoothLocalDevice::State state);
    void error(QBluetoothLocalDevice::Error error, const QString &msg);

    void remoteDeviceConnected(const QBluetoothAddress &addr);
    void remoteDeviceDisconnected(const QBluetoothAddress &addr);

    void discoveryStarted();
    void remoteDeviceFound(const QBluetoothRemoteDevice &device);
    void remoteDeviceDisappeared(const QBluetoothAddress &addr);
    void remoteDevicesFound(const QList<QBluetoothRemoteDevice> &devices);
    void discoveryCancelled();
    void discoveryCompleted();

    void pairingCreated(const QBluetoothAddress &addr);
    void pairingRemoved(const QBluetoothAddress &addr);
    void pairingFailed(const QBluetoothAddress &addr);

    void remoteAliasChanged(const QBluetoothAddress &addr, const QString &alias);
    void remoteAliasRemoved(const QBluetoothAddress &addr);

    void remoteNameUpdated(const QBluetoothAddress &addr, const QString &name);
    void remoteNameFailed(const QBluetoothAddress &addr);
    void remoteClassUpdated(const QBluetoothAddress &addr,
                            QBluetooth::DeviceMajor major, quint8 minor,
                            QBluetooth::ServiceClasses serviceClasses);

    void discoverableTimeoutChanged(uint timeout);

    void remoteDeviceDisconnectRequested(const QBluetoothAddress &addr);

private:
    virtual void connectNotify(const char *signal);

    Q_DISABLE_COPY(QBluetoothLocalDevice)
    QBluetoothLocalDevice_Private *m_data;
};

#endif
