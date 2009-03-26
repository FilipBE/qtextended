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

#include "qusbgadget.h"
#include "qusbmanager.h"

#include <QDebug>
#include <QFile>
#include <QSettings>
#include <QProcess>
#include <QDir>

/*!
    \class QUsbGadget
    \inpublicgroup QtBaseModule


    \brief The QUsbGadget class is the base class of all USB gadget classes.

    QUsbGadget and derived classes provides information on the supported USB
    device classes and an API for controlling the USB gadget hardware.  The
    USB gadget API is split into two sets of classes: the provider classes and
    the client classes.  Both sets are subclasses of QUsbGadget.

    The client classes provide an API for querying and controlling the USB
    gadget hardware.  Client classes are created with the
    \l QAbstractIpcInterface::Client mode.

    The provider classes implement the backend functionality that interfaces
    with the USB hardware.  Provider classes are created with the
    \l QAbstractIpcInterface::Server mode.  Qt Extended comes with an implementation
    for the standard Linux USB gadget stack.  Support for custom USB gadget
    drivers is achieved by creating addition subclasses of QUsbGadget.  Support
    for alternative USB gadget stacks is achieved by implementing alternative
    provider classes.

    \sa QUsbManager, UsbGadgetTask

    \ingroup hardware
*/

/*!
    Constructs a new QUsbGadget object for the interface called \a interfaceName, on
    \a group and attach it to \a parent.  If \a mode is \c Server then the object is
    constructed in server mode and register with \l QUsbManager.  If \a mode is
    \c Client, then the object is constructed in client mode and \a group may be empty
    to indicate that the default group should be used.
    o
*/
QUsbGadget::QUsbGadget(const QString &interfaceName, const QString &group,
                       QObject *parent, QAbstractIpcInterface::Mode mode)
    : QAbstractIpcInterface("/Hardware/UsbGadget", interfaceName, group, parent, mode)
{
    proxyAll(staticMetaObject);
}

/*!
    Returns true if the USB gadget supports setting and getting the vendor id.
*/
bool QUsbGadget::supportsVendorId() const
{
    return value("vendorId").isValid();
}

/*!
    Returns the vendor id.  Returns 0 if supportsVendorId() returns false.
*/
quint16 QUsbGadget::vendorId() const
{
    return value("vendorId", 0).toUInt();
}

/*!
    Returns true if the USB gadget supports setting and getting the product id.
*/
bool QUsbGadget::supportsProductId() const
{
    return value("productId").isValid();
}

/*!
    Returns the product id.  Returns 0 if supportsProductId() returns false.
*/
quint16 QUsbGadget::productId() const
{
    return value("productId", 0).toUInt();
}

/*!
    Returns true if the USB gadget supports setting and getting the vendor
    string.
*/
bool QUsbGadget::supportsVendor() const
{
    return value("vendor").isValid();
}

/*!
    Returns the vendor string.  Returns a null bytearray if supportsVendor()
    returns false.
*/
QByteArray QUsbGadget::vendor() const
{
    return value("vendor", QByteArray()).toByteArray();
}

/*!
    Returns true if the USB gadget supports setting and getting the product
    string.
*/
bool QUsbGadget::supportsProduct() const
{
    return value("product").isValid();
}

/*!
    Returns the product string.  Returns a null bytearray if supportsProduct()
    returns false.
*/
QByteArray QUsbGadget::product() const
{
    return value("product", QByteArray()).toByteArray();
}

/*!
    Returns true if the USB gadget is active.
*/
bool QUsbGadget::active() const
{
    return value("active", false).toBool();
}

/*!
    Returns the name of the gadget.  The value that is returned is the same
    as the \c {Usb.conf} settings group associated with this gadget.
*/
QByteArray QUsbGadget::gadget() const
{
    return value("gadget", QByteArray()).toByteArray();
}

/*!
    Sets the vendor id to \a id.
*/
void QUsbGadget::setVendorId(const quint16 id)
{
    invoke(SLOT(setVendorId(quint16)), qVariantFromValue(id));
}

/*!
    Sets the product id to \a id.
*/
void QUsbGadget::setProductId(const quint16 id)
{
    invoke(SLOT(setProductId(quint16)), qVariantFromValue(id));
}

/*!
    Sets the vendor string to \a vendor.
*/
void QUsbGadget::setVendor(const QByteArray &vendor)
{
    invoke(SLOT(setVendor(QByteArray)), qVariantFromValue(vendor));
}

/*!
    Sets the product string to \a product.
*/
void QUsbGadget::setProduct(const QByteArray &product)
{
    invoke(SLOT(setProduct(QByteArray)), qVariantFromValue(product));
}

/*!
    Saves the settings to the \c {Trolltech/Usb.conf} configuration file.
*/
void QUsbGadget::saveConfig()
{
    invoke(SLOT(saveDefault()));
}

/*!
    Activates the USB gadget.  Deactivating other gadgets if necessary.

    The \l activated() signal will be emitted to indicate that the USB gadget
    has been activated and is ready for use.  On failure the \l deactivated()
    signal is emitted.
*/
void QUsbGadget::activate()
{
    invoke(SLOT(activate()));
}

/*!
    Deactivates the USB gadget.

    The \l deactivated() signal will be emitted to indicate that the USB gadget
    has been deactivated.  On failure the \l activated() signal is emitted.
*/
void QUsbGadget::deactivate()
{
    invoke(SLOT(deactivate()));
}

/*!
    \fn void QUsbGadget::activated()

    Signal which is emitted when this USB gadget activates.
*/

/*!
    \fn void QUsbGadget::deactivated()

    Signal which is emitted when this USB gadget deactivates.
*/

/*!
    \fn void QUsbGadget::activateFailed()

    Signal which is emitted when the activation of this USB gadget fails.
*/

/*!
    \fn void QUsbGadget::deactivateFailed()

    Signal which is emitted when the deactivation of this USB gadget fails.
*/

