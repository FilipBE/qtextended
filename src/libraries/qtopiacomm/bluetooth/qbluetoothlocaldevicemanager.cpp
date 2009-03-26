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

#include <qbluetoothlocaldevicemanager.h>

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDebug>


class QBluetoothLocalDeviceManager_Private : public QObject
{
    Q_OBJECT
public:
    QBluetoothLocalDeviceManager_Private(QBluetoothLocalDeviceManager *parent);

public slots:
    void deviceAdded(const QString &device);
    void deviceRemoved(const QString &device);
    void defaultDeviceChanged(const QString &device);

public:
    QBluetoothLocalDeviceManager *m_parent;
    QDBusInterface *m_iface;
};

QBluetoothLocalDeviceManager_Private::QBluetoothLocalDeviceManager_Private(
        QBluetoothLocalDeviceManager *parent) : QObject(parent), m_parent(parent), m_iface(0)
{
    QDBusConnection dbc =
#ifdef QTOPIA_TEST
        QDBusConnection::sessionBus();
#else
        QDBusConnection::systemBus();
#endif

    if (!dbc.isConnected()) {
        qWarning() << "Unable to connect to D-BUS:" << dbc.lastError();
        return;
    }

    m_iface = new QDBusInterface("org.bluez", "/org/bluez",
                                 "org.bluez.Manager", dbc);
    if (!m_iface->isValid()) {
        qWarning() << "Could not find org.bluez interface";
        return;
    }

    dbc.connect("org.bluez", "/org/bluez", "org.bluez.Manager", "AdapterAdded",
                this, SIGNAL(deviceAdded(QString)));
    dbc.connect("org.bluez", "/org/bluez", "org.bluez.Manager", "AdapterRemoved",
                this, SIGNAL(deviceRemoved(QString)));
    dbc.connect("org.bluez", "/org/bluez", "org.bluez.Manager", "DefaultAdapterChanged",
                this, SIGNAL(defaultDeviceChanged(QString)));
}

void QBluetoothLocalDeviceManager_Private::deviceAdded(const QString &device)
{
    emit m_parent->deviceAdded(device.mid(11));
}

void QBluetoothLocalDeviceManager_Private::deviceRemoved(const QString &device)
{
    emit m_parent->deviceRemoved(device.mid(11));
}

void QBluetoothLocalDeviceManager_Private::defaultDeviceChanged(const QString &device)
{
    emit m_parent->defaultDeviceChanged(device.mid(11));
}

/*!
    \class QBluetoothLocalDeviceManager
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothLocalDeviceManager class provides access to local Bluetooth devices.

    The purpose of the QBluetoothLocalDeviceManager is to enumerate all
    local Bluetooth adapters on the system.  This class can also notify
    the user when a Bluetooth adapter has been added or removed.  The
    values returned by devices() can be used to construct
    QBluetoothLocalDevice objects that refer to the particular
    local device.

    On Linux, each Bluetooth device on the system has a Bluetooth device
    name and an address associated with it.  The Bluetooth device name
    is of the form hciXX where XX is a number between 0 and 31.

    Use deviceRemoved() and deviceAdded() signals to subscribe to device
    removed and device added events, respectively.  Use defaultDeviceChanged()
    signal to get notification of the default device change. 

    \sa QBluetoothLocalDevice

    \ingroup qtopiabluetooth
*/

/*!
    Constructs the device manager for local Bluetooth devices.  The
    \a parent parameter specifies the parent.
  */
QBluetoothLocalDeviceManager::QBluetoothLocalDeviceManager(QObject *parent) :
    QObject(parent), m_data(0)
{
    m_data = new QBluetoothLocalDeviceManager_Private(this);
}

/*!
    Destroys the device manager.
*/
QBluetoothLocalDeviceManager::~QBluetoothLocalDeviceManager()
{
    if (m_data)
        delete m_data;
}

/*!
    Returns a list of all Bluetooth devices found on the system.  The list
    typically consists of hci0, hci1, ..., hciN, depending on the number of
    devices.

    \sa defaultDevice()
 */
QStringList QBluetoothLocalDeviceManager::devices()
{
    QStringList ret;

    if (!m_data->m_iface || !m_data->m_iface->isValid())
        return ret;

    QDBusReply<QStringList> reply = m_data->m_iface->call("ListAdapters");

    if (!reply.isValid()) {
        return ret;
    }

    foreach (QString device, reply.value()) {
        ret.push_back(device.mid(11));
    }

    return ret;
}

/*!
    Returns a device name of the default Bluetooth adapter.  This is
    typically 'hci0'.  Returns a null string if no device is installed.

    The QBluetoothLocalDevice default constructor can be used to
    construct an instance of the default local device.

    \sa QBluetoothLocalDevice::QBluetoothLocalDevice(), devices()
    \sa defaultDeviceChanged()
 */
QString QBluetoothLocalDeviceManager::defaultDevice()
{
    if (!m_data->m_iface || !m_data->m_iface->isValid())
        return QString();

    QDBusReply<QString> reply = m_data->m_iface->call("DefaultAdapter");

    if (!reply.isValid()) {
        return QString();
    }

    return reply.value().mid(11);
}

/*!
    \fn void QBluetoothLocalDeviceManager::deviceAdded(const QString &device)

    This signal is emitted whenever a new device has been added to the system.
    The \a device parameter contains the representation of the device which can be
    passed to the QBluetoothLocalDevice constructor.

    \sa QBluetoothLocalDevice, devices()
 */

/*!
    \fn void QBluetoothLocalDeviceManager::deviceRemoved(const QString &device)

    This signal is emitted whenever a device has been removed from the system.
    The \a device parameter contains the representation of the device which
    was removed.

    \sa QBluetoothLocalDevice, devices()
 */

/*!
    \fn void QBluetoothLocalDeviceManager::defaultDeviceChanged(const QString &device)

    This signal is emitted whenever the default device has changed.  The \a device 
    parameter contains the representation of the new default device.

    \sa QBluetoothLocalDevice, defaultDevice()
*/

#include "qbluetoothlocaldevicemanager.moc"
