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

#include "qusbethernetgadget.h"
#include "qusbmanager.h"

#include <QDebug>
#include <QFile>
#include <QSettings>
#include <QProcess>
#include <QDir>

/*!
    \class QUsbEthernetGadget
    \inpublicgroup QtBaseModule


    \brief The QUsbEthernetGadget class is used to enable ethernet communication class of USB gadgets.

    The QUsbEthernetGadget class is used to configure the USB gadget hardware
    as an ethernet communication class device.

    \code
        QUsbEthernetGadget *gadget = new QUsbEthernetGadget;
        if (gadget->available()) {
            connect(gadget, SIGNAL(activated()), this, SLOT(ethernetActivated()));
            gadget->activate();
        }
    \endcode

    \sa QUsbManager, UsbGadgetTask

    \ingroup hardware
*/

/*!
    Constructs a new QUsbEthernetGadget object in \a group and attach it to
    \a parent.  If \a mode is \c Client, then the object is constructed in
    client mode and \a group may be empty to indicate that the default group
    should be used.
*/
QUsbEthernetGadget::QUsbEthernetGadget(const QString &group, QObject *parent,
                                       QAbstractIpcInterface::Mode mode)
    : QUsbGadget("QUsbEthernetGadget", group, parent, mode)
{
    proxyAll(staticMetaObject);
}

/*!
    Returns the remote MAC address.
*/
QByteArray QUsbEthernetGadget::remoteMac() const
{
    return value("remoteMac", QByteArray()).toByteArray();
}

/*!
    Returns the local MAC address.
*/
QByteArray QUsbEthernetGadget::localMac() const
{
    return value("localMac", QByteArray()).toByteArray();
}

/*!
    Returns the name of the network interface.  A null bytearray is returned if
    the gadget is not \l active().
*/
QByteArray QUsbEthernetGadget::interface() const
{
    return value("interface", QByteArray()).toByteArray();
}

/*!
    Sets the remote MAC address to \a mac.
*/
void QUsbEthernetGadget::setRemoteMac(const QByteArray &mac)
{
    invoke(SLOT(setRemoteMac(QByteArray)), qVariantFromValue(mac));
}

/*!
    Sets the local MAC address to \a mac.
*/
void QUsbEthernetGadget::setLocalMac(const QByteArray &mac)
{
    invoke(SLOT(setLocalMac(QByteArray)), qVariantFromValue(mac));
}


/*!
    \class QUsbEthernetGadgetProvider
    \inpublicgroup QtBaseModule


    \brief The QUsbEthernetGadgetProvider class provides a backend implementation for QUsbEthernetGadget

    This class works with the standard Linux ethernet gadget module.

    This class requires \c {/sbin/modprobe} and \c {/sbin/rmmod} system utilities.

    The following configuration keys are supported
    \table
        \header
            \o Key
            \o Description
        \row
            \o Product
            \o USB Product string
        \row
            \o ProductId
            \o USB Product id
        \row
            \o Vendor
            \o USB Vendor string
        \row
            \o VendorId
            \o USB Vendor id
        \row
            \o RemoteMac
            \o MAC address of the remote side of the Ethernet interface
        \row
            \o LocalMac
            \o MAC address of the local side of the Ethernet interface
    \endtable

    \sa QUsbManager, UsbGadgetTask

    \ingroup hardware
*/

class QUsbEthernetGadgetProviderPrivate
{
public:
    QUsbManager *manager;

    QUsbEthernetGadgetProviderPrivate();
    ~QUsbEthernetGadgetProviderPrivate();
};

QUsbEthernetGadgetProviderPrivate::QUsbEthernetGadgetProviderPrivate()
    : manager(0)
{
}

QUsbEthernetGadgetProviderPrivate::~QUsbEthernetGadgetProviderPrivate()
{
    delete manager;
}

#define GADGET_NAME "Ethernet"

/*!
    Constructs a new QUsbEthernetGadgetProvider in \a group and attach it to
    \a parent.
*/
QUsbEthernetGadgetProvider::QUsbEthernetGadgetProvider(const QString &group, QObject *parent)
    : QUsbEthernetGadget(group, parent, Server)
{
    d = new QUsbEthernetGadgetProviderPrivate;

    QSettings settings("Trolltech", "Usb");

    QString function = settings.value("PeripheralController/Path").toString() +
                       "/gadget/function";

    QFile file(function);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text) &&
        file.readLine().trimmed() == "Ethernet Gadget") {
            initFromSystem();
    } else {
        initFromConfig();
    }

    setValue("gadget", QByteArray(GADGET_NAME));
}

/*!
    Destructor.
*/
QUsbEthernetGadgetProvider::~QUsbEthernetGadgetProvider()
{
    delete d;
}

/*!
    \reimp
*/
void QUsbEthernetGadgetProvider::setVendorId(const quint16 id)
{
    setValue("vendorId", id);
}

/*!
    \reimp
*/
void QUsbEthernetGadgetProvider::setProductId(const quint16 id)
{
    setValue("productId", id);
}

/*!
    \reimp
*/
void QUsbEthernetGadgetProvider::setVendor(const QByteArray &vendor)
{
    setValue("vendor", vendor);
}

/*!
    \reimp
*/
void QUsbEthernetGadgetProvider::setProduct(const QByteArray &product)
{
    setValue("product", product);
}

/*!
    \reimp
*/
void QUsbEthernetGadgetProvider::saveConfig()
{
    QSettings settings("Trolltech", "Usb");
    settings.beginGroup(GADGET_NAME);

    QVariant v = value("product");
    if (v.isValid())
        settings.setValue("Product", v.toString());

    v = value("productId");
    if (v.isValid())
        settings.setValue("ProductId", v.toUInt());

    v = value("vendor");
    if (v.isValid())
        settings.setValue("Vendor", v.toString());

    v = value("vendorId");
    if (v.isValid())
        settings.setValue("VendorId", v.toUInt());

    v = value("localMac");
    if (v.isValid())
        settings.setValue("LocalMac", v.toString());

    v = value("remoteMac");
    if (v.isValid())
        settings.setValue("RemoteMac", v.toString());

    settings.endGroup();
}

/*!
    \reimp
*/
void QUsbEthernetGadgetProvider::activate()
{
    if (!value("active", false).toBool()) {
        if (!d->manager)
            d->manager = new QUsbManager;

        if (d->manager->canActivate(GADGET_NAME)) {
            loadModule();
        } else {
            connect(d->manager, SIGNAL(deactivateCompleted()), this, SLOT(loadModule()));
            connect(d->manager, SIGNAL(deactivateAborted()), this, SLOT(abort()));
            d->manager->deactivateGadgets();
        }
    }
}

/*!
    \internal
*/
void QUsbEthernetGadgetProvider::loadModule()
{
    QStringList args;

    args << "g_ether";

    QVariant v = value("product");
    if (v.isValid())
        args << "iProduct=\"" + v.toString() + '"';

    v = value("productId");
    if (v.isValid())
        args << "idProduct=0x" + QString::number(v.toInt(), 16);

    v = value("vendor");
    if (v.isValid())
        args << "iManufacturer=\"" + v.toString() + '"';

    v = value("vendorId");
    if (v.isValid())
        args << "idVendor=0x" + QString::number(v.toInt(), 16);

    v = value("localMac");
    if (v.isValid())
        args << "dev_addr=" + v.toString();

    v = value("remoteMac");
    if (v.isValid())
        args << "host_addr=" + v.toString();

    if (QProcess::execute("/sbin/modprobe", args) == 0) {
        setValue("interface", interfaceName());
        setValue("active", true);
        emit activated();
    } else {
        abort();
    }
}

/*!
    \internal
*/
void QUsbEthernetGadgetProvider::abort()
{
    removeValue("interface");
    emit activateFailed();
}

/*!
    \reimp
*/
void QUsbEthernetGadgetProvider::deactivate()
{
    if (value("active", false).toBool()) {
        if (QProcess::execute("/sbin/rmmod g_ether") == 0) {
            setValue("active", false);
            removeValue("interface");
            emit deactivated();
        } else {
            emit deactivateFailed();
        }
    }
}

/*!
    \reimp
*/
void QUsbEthernetGadgetProvider::setRemoteMac(const QByteArray &mac)
{
    setValue("remoteMac", mac);
}

/*!
    \reimp
*/
void QUsbEthernetGadgetProvider::setLocalMac(const QByteArray &mac)
{
    setValue("localMac", mac);
}

/*!
    Initialize the gadget from settings in the configuration file.
*/
void QUsbEthernetGadgetProvider::initFromConfig()
{
    QSettings settings("Trolltech", "Usb");

    settings.beginGroup(GADGET_NAME);

    QVariant v = settings.value("Vendor");
    if (v.isValid())
        setVendor(v.toByteArray());
    else
        removeValue("vendor");

    v = settings.value("VendorId");
    if (v.isValid())
        setVendorId(v.toUInt());
    else
        removeValue("vendorId");

    v = settings.value("Product");
    if (v.isValid())
        setProduct(v.toByteArray());
    else
        removeValue("product");

    v = settings.value("ProductId");
    if (v.isValid())
        setProductId(v.toUInt());
    else
        removeValue("productId");

    v = settings.value("RemoteMac");
    if (v.isValid())
        setRemoteMac(v.toByteArray());
    else
        removeValue("remoteMac");

    v = settings.value("LocalMac");
    if (v.isValid())
        setLocalMac(v.toByteArray());
    else
        removeValue("localMac");

    settings.endGroup();

    setValue("active", false);
}

/*!
    Initialize the gadget from settings obtained from the system.  This
    function is called if the ethernet gadget is \l active() when it is
    constructed.
*/
void QUsbEthernetGadgetProvider::initFromSystem()
{
    QSettings settings("Trolltech", "Usb");

    QString paramPath = settings.value("PeripheralController/Path").toString() +
                        "/gadget/driver/module/parameters/";

    QFile file(paramPath + "iManufacturer");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setVendor(file.readLine().trimmed());
        file.close();
    }

    file.setFileName(paramPath + "idVendor");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setVendorId(file.readLine().trimmed().toUInt());
        file.close();
    }

    file.setFileName(paramPath + "iProduct");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setProduct(file.readLine().trimmed());
        file.close();
    }

    file.setFileName(paramPath + "idProduct");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setProductId(file.readLine().trimmed().toUInt());
        file.close();
    }

    file.setFileName(paramPath + "host_addr");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setRemoteMac(file.readLine().trimmed());
        file.close();
    }

    file.setFileName(paramPath + "dev_addr");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setLocalMac(file.readLine().trimmed());
        file.close();
    }

    setValue("active", true);
}

/*!
    \internal
*/
QByteArray QUsbEthernetGadgetProvider::interfaceName() const
{
    QSettings settings("Trolltech", "Usb");
    QString gadgetPath = settings.value("PeripheralController/Path").toString() +
                         "/gadget";
    QDir gadget(gadgetPath, "net:*");
    QStringList entries = gadget.entryList();
    if (entries.count() == 1) {
        return QByteArray(entries.first().mid(4).toLocal8Bit().constData());
    } else {
        qWarning("More than 1 entry found when detecting usb interface name");
        return QByteArray();
    }
}

