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

#include <qirremotedevice.h>

/*!
    \class QIrRemoteDevice

    \brief The QIrRemoteDevice class represents a remote infrared device.

    QIrRemoteDevice holds information about a remote Infrared device.
    Only basic information is provided, namely the name of the remote
    device, its major device classes and the negotiated address.

    \ingroup qtopiair
    \sa QIrLocalDevice
 */

/*!
    Constructs a QIrRemoteDevice with \a name as the name,
    \a devClasses as the major device classes supported by by this
    device and address given by \a addr.

    \sa name(), deviceClasses(), address()
*/
QIrRemoteDevice::QIrRemoteDevice(const QString &name,
                                 QIr::DeviceClasses &devClasses,
                                 uint addr) :
                                 m_name(name),
                                 m_dev_class(devClasses),
                                 m_addr(addr)
{

}

/*!
    Copy constructor. Constructs the current object from the attributes of \a dev.
*/
QIrRemoteDevice::QIrRemoteDevice(const QIrRemoteDevice &dev)
{
    operator=(dev);
}

/*!
    Destroys the device object.
*/
QIrRemoteDevice::~QIrRemoteDevice()
{

}

/*!
    Assignment operator.  Assigns the contents of \a other to the contents
    of the current object.
*/
QIrRemoteDevice &QIrRemoteDevice::operator=(const QIrRemoteDevice &other)
{
    if (this == &other)
        return *this;

    m_name = other.m_name;
    m_addr = other.m_addr;
    m_dev_class = other.m_dev_class;

    return *this;
}

/*!
    Comparison operator.  Compares the contents of \a other to the contents
    of the current object.  Returns true if the contents are equal.
*/
bool QIrRemoteDevice::operator==(const QIrRemoteDevice &other) const
{
    if (this == &other)
        return true;

    if ( (m_name == other.m_name) &&
         (m_addr == other.m_addr) &&
         (m_dev_class == other.m_dev_class) ) {
        return true;
    }

    return false;
}

/*!
    Returns the address of the remote device.

    \warning This is a 32 bit integer, and due to the nature of the
    IrDA protocol this number should be considered highly dynamic.  To
    refresh the number, a new discovery must be performed. It is not
    recommended to try and reuse the address for extended periods
    of time.

    \sa QIrLocalDevice, name()
*/
uint QIrRemoteDevice::address() const
{
    return m_addr;
}

/*!
    Returns the major device classes supported by the remote device.

    \sa name()
*/
QIr::DeviceClasses QIrRemoteDevice::deviceClasses() const
{
    return m_dev_class;
}

/*!
    Returns the name of the remote device.

    \sa address(), deviceClasses()
*/
QString QIrRemoteDevice::name() const
{
    return m_name;
}
