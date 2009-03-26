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

#include <qbluetoothaddress.h>
#include <qbluetoothremotedevice.h>
#include "qbluetoothnamespace_p.h"

#include <QStringList>

class QBluetoothRemoteDevicePrivate
{
public:
    QBluetoothAddress m_addr;
    QString m_name;
    QString m_version;
    QString m_revision;
    QString m_manufacturer;
    QString m_company;
    QBluetooth::DeviceMajor m_dev_major;
    quint8 m_dev_minor;
    QBluetooth::ServiceClasses m_service_classes;
    int m_rssi;
};

/*!
    \class QBluetoothRemoteDevice
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothRemoteDevice class represents a remote bluetooth device.

    This class holds various device attributes of the remote device.  These
    attributes are device address, device class, manufacturer, etc.  The most
    common source of all remote device information is the discovery process.
    This process must be initiated by using the
    QBluetoothLocalDevice::discoverDevices() method and hooking onto the
    appropriate signals.

    Please note that depending on the source that provided the
    QBluetoothRemoteDevice instance, some or all of the information might
    not be available.  In general, address, device major, device minor
    and service class attributes should always be available.  Other
    attributes might require a connection be established to the
    remote device, and thus might not be available.
    In this case, default constructed values will be
    returned.

    \ingroup qtopiabluetooth
    \sa QBluetoothAddress, QBluetoothLocalDevice
 */

/*!
    Constructs a new QBluetoothRemoteDevice object.  The attributes are given
    default values.  The \a address parameter defines the address of the remote
    device.
*/
QBluetoothRemoteDevice::QBluetoothRemoteDevice(const QBluetoothAddress &address)
{
    m_data = new QBluetoothRemoteDevicePrivate();
    m_data->m_addr = address;
    m_data->m_version = "1.0";
    m_data->m_dev_major = QBluetooth::Uncategorized;
    m_data->m_dev_minor = 0;
    m_data->m_rssi = 0;
}

/*!
    Constructs a new QBluetoothRemoteDevice object.
    \list
        \o \a address defines the address of the remote device.
        \o \a name defines the name of the remote device.
        \o \a version defines the Bluetooth protocol version this device supports.
        \o \a revision defines the manufacturer specific revision of the device
        \o \a manufacturer defines the manufacturer string
        \o \a company defines the company producing the device
        \o \a rssi defines the Receiver Signal Strength (Indicator)
        \o \a devMajor defines the major device number
        \o \a devMinor defines the minor device number
        \o \a serviceClasses defines the bitmap of all device classes associated with this device.
    \endlist
*/
QBluetoothRemoteDevice::QBluetoothRemoteDevice(const QBluetoothAddress &address,
                                               const QString &name,
                                               const QString &version,
                                               const QString &revision,
                                               const QString &manufacturer,
                                               const QString &company,
                                               int rssi,
                                               QBluetooth::DeviceMajor devMajor,
                                               quint8 devMinor,
                                               QBluetooth::ServiceClasses serviceClasses)
{
    m_data = new QBluetoothRemoteDevicePrivate();
    m_data->m_addr = address;
    m_data->m_name = name;
    m_data->m_version = version;
    m_data->m_revision = revision;
    m_data->m_manufacturer = manufacturer;
    m_data->m_company = company;
    m_data->m_dev_major = devMajor;
    m_data->m_dev_minor = devMinor;
    m_data->m_service_classes = serviceClasses;
    m_data->m_rssi = rssi;
}

/*!
    Constructs a new QBluetoothRemoteDevice object from \a other.
*/
QBluetoothRemoteDevice::QBluetoothRemoteDevice(const QBluetoothRemoteDevice &other)
{
    m_data = new QBluetoothRemoteDevicePrivate();
    operator=(other);
}

/*!
    Deconstructs a QBluetoothRemoteDevice object.
*/
QBluetoothRemoteDevice::~QBluetoothRemoteDevice()
{
    if (m_data) {
        delete m_data;
        m_data = 0;
    }
}

/*!
    Assigns the contents of \a other to the current object.
*/
QBluetoothRemoteDevice &QBluetoothRemoteDevice::operator=(const QBluetoothRemoteDevice &other)
{
    if (this == &other)
        return *this;

    m_data->m_addr = other.m_data->m_addr;
    m_data->m_name = other.m_data->m_name;
    m_data->m_version = other.m_data->m_version;
    m_data->m_revision = other.m_data->m_revision;
    m_data->m_manufacturer = other.m_data->m_manufacturer;
    m_data->m_company = other.m_data->m_company;
    m_data->m_dev_major = other.m_data->m_dev_major;
    m_data->m_dev_minor = other.m_data->m_dev_minor;
    m_data->m_service_classes = other.m_data->m_service_classes;

    return *this;
}

/*!
    Compares this remote device against the remote device given by \a other.
    Two remote devices are considered to be the same if they have the same
    Bluetooth address.

    Returns true if the devices are the same.
*/
bool QBluetoothRemoteDevice::operator==(const QBluetoothRemoteDevice &other) const
{
    return m_data->m_addr == other.m_data->m_addr;
}

/*!
    \fn bool QBluetoothRemoteDevice::operator!=(const QBluetoothRemoteDevice &other) const;

    Compares this remote device against the remote device given by \a other.
    Two remote devices are considered not equal if the addresses are not equal.

    Returns false if the devices are equal, and true otherwise.
*/

/*!
    Returns the human readable name of the remote device.
    If the name is unknown, an empty string is returned.
    For display purposes, it is best to display the address
    of the device in this case.

    Each local Bluetooth adapter might have an alias associated
    with the remote device.

    \sa setName(), QBluetoothLocalDevice::remoteAlias()
*/
QString QBluetoothRemoteDevice::name() const
{
    return m_data->m_name;
}

/*!
    Sets the name attribute of the remote device to \a name.

    In general, you should not need to use this function.

    \sa name()
*/
void QBluetoothRemoteDevice::setName(const QString &name)
{
    m_data->m_name = name;
}

/*!
    Returns the Bluetooth protocol version the remote device supports.
    This can be for instance, 1.0, 1.1, 1.2, etc.

    \sa setVersion()
*/
QString QBluetoothRemoteDevice::version() const
{
    return m_data->m_version;
}

/*!
    Sets the version attribute of the remote device to \a version.

    In general, you should not need to use this function.

    \sa version()
*/
void QBluetoothRemoteDevice::setVersion(const QString &version)
{
    m_data->m_version = version;
}

/*!
    Returns the revision of the remote bluetooth device.
    This is generally manufacturer specific information.
    If the revision is unknown, a null string is returned.

    \sa setRevision()
*/
QString QBluetoothRemoteDevice::revision() const
{
    return m_data->m_revision;
}

/*!
    Sets the revision attribute of the remote device to \a revision.
    Clients should not need to use this function.

    \sa revision()
*/
void QBluetoothRemoteDevice::setRevision(const QString &revision)
{
    m_data->m_revision = revision;
}

/*!
    Returns the manufacturer of the remote bluetooth device.
    If the manufacturer is unknown, returns a null string.

    \sa setManufacturer()
 */
QString QBluetoothRemoteDevice::manufacturer() const
{
    return m_data->m_manufacturer;
}

/*!
    Sets the manufacturer attribute of the remote device
    to \a manufacturer.

    In general, you should not need to use this function.

    \sa manufacturer()
*/
void QBluetoothRemoteDevice::setManufacturer(const QString &manufacturer)
{
    m_data->m_manufacturer = manufacturer;
}

/*!
    Returns the company of the remote bluetooth device.  This is
    generally the brand name of the company under which the device
    is sold.  If the company attribute is unknown, returns a null string.

    Note that the underlying HCI daemon requires the IEEE standard oui.txt
    file in order to read the company information correctly. This file
    can be downloaded from the IEEE site. The HCI daemon expects the file
    to be placed at \c /usr/share/misc/oui.txt.

    \sa setCompany()
 */
QString QBluetoothRemoteDevice::company() const
{
    return m_data->m_company;
}

/*!
    Sets the company attribute of the remote device to \a company.

    In general, you should not need to use this function.

    \sa company()
*/
void QBluetoothRemoteDevice::setCompany(const QString &company)
{
    m_data->m_company = company;
}

/*!
    Returns the RSSI (Receiver Signal Strength Indicator) of the remote
    device at the time of the last device discovery.
*/
int QBluetoothRemoteDevice::rssi() const
{
    return m_data->m_rssi;
}


/*!
    Sets the RSSI (Receiver Signal Strength Indicator) of the remote
    device \a rssi. This should be the RSSI of the device at the time
    of the last discovery.
*/
void QBluetoothRemoteDevice::setRssi(int rssi)
{
    m_data->m_rssi = rssi;
}

/*!
    Returns the major device number of the remote device.

    \sa deviceMajorAsString(), setDeviceMajor()
*/
QBluetooth::DeviceMajor QBluetoothRemoteDevice::deviceMajor() const
{
    return m_data->m_dev_major;
}

/*!
    Returns the major device number interpreted as a translated string.

    \sa deviceMajor(), setDeviceMajor()
*/
QString QBluetoothRemoteDevice::deviceMajorAsString() const
{
    return convertDeviceMajorToString(m_data->m_dev_major);
}

/*!
    Sets the major device attribute of the remote device
    to \a deviceMajor.

    In general, you should not need to use this function.

    \sa deviceMajor(), deviceMajorAsString()
*/
void QBluetoothRemoteDevice::setDeviceMajor(QBluetooth::DeviceMajor deviceMajor)
{
    m_data->m_dev_major = deviceMajor;
}

/*!
    Returns the minor device number of the remote device.

    \sa deviceMinorAsString(), setDeviceMinor()
*/
quint8 QBluetoothRemoteDevice::deviceMinor() const
{
    return m_data->m_dev_minor;
}

/*!
    Returns the minor device number of the remote device as a human
    readable string.

    \sa deviceMinor(), setDeviceMinor()
 */
QString QBluetoothRemoteDevice::deviceMinorAsString() const
{
    return convertDeviceMinorToString(m_data->m_dev_major,
                                      m_data->m_dev_minor);
}

/*!
    Sets the minor device attribute of the remote device to \a deviceMinor.

    \sa deviceMinor(), deviceMinorAsString()
*/
void QBluetoothRemoteDevice::setDeviceMinor(quint8 deviceMinor)
{
    m_data->m_dev_minor = deviceMinor;
}

/*!
    Returns the service classes for the remote device.

    \sa serviceClassesAsString(), setServiceClasses()
*/
QBluetooth::ServiceClasses QBluetoothRemoteDevice::serviceClasses() const
{
    return m_data->m_service_classes;
}

/*!
    Returns the service classes for the remote device as a list of
    translated strings.

    \sa serviceClasses(), setServiceClasses()
*/
QStringList QBluetoothRemoteDevice::serviceClassesAsString() const
{
    return convertServiceClassesToString(m_data->m_service_classes);
}

/*!
    Sets the service classes attribute of the remote device to \a serviceClasses.

    In general, you should not need to use this function.

    \sa serviceClasses(), serviceClassesAsString()
*/
void QBluetoothRemoteDevice::setServiceClasses(QBluetooth::ServiceClasses serviceClasses)
{
    m_data->m_service_classes = serviceClasses;
}

/*!
    Returns the Bluetooth Address of the remote device.
*/
QBluetoothAddress QBluetoothRemoteDevice::address() const
{
    return m_data->m_addr;
}
