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

#include "qusbstoragegadget.h"
#include "qusbmanager.h"

#include <QDebug>
#include <QFile>
#include <QSettings>
#include <QProcess>
#include <QDir>

/*!
    \class QUsbStorageGadget
    \inpublicgroup QtBaseModule


    \brief The QUsbStorageGadget class is used to enable mass storage class of USB gadgets.

    The QUsbStorageGadget class is used to configure the USB gadget hardware
    as a USB mass storage device.

    The following code will export the block device, \c {/dev/sda}.

    \code
        QUsbStorageGadget *storage = QUsbStorageGadget;
        if (gadget->available()) {
            connect(gadget, SIGNAL(activated()), this, SLOT(storageActivated()));
            gadget->setBackingStore(QStringList() << "/dev/sda");
            gadget->activate();
        }
    \endcode

    \warning Filesystem drivers in most operating systems expect exclusive access to the
    underlying block device.  Any modification to the on disk data structures that do not
    go through the filesystem layer can result in data losses and system crashes.  Therefore
    it is important that the backing store block devices are unmounted prior to activating
    the USB storage gadget.

    \sa QUsbManager, UsbGadgetTask

    \ingroup hardware
*/

/*!
    Constructs a new QUsbStorageGadget object in \a group and attach it to
    \a parent.  If \a mode is \c Client, then the object is constructed in
    client mode and \a group may be empty to indicate that the default group
    should be used.
*/
QUsbStorageGadget::QUsbStorageGadget(const QString &group, QObject *parent,
                                     QAbstractIpcInterface::Mode mode)
    : QUsbGadget("QUsbStorageGadget", group, parent, mode)
{
    proxyAll(staticMetaObject);
}

/*!
    Returns the list of devices that will be used as the backing store.
*/
QStringList QUsbStorageGadget::backingStore() const
{
    return value("backingStore", QStringList()).toStringList();
}

/*!
    Sets the list of backing store devices to \a paths.
*/
void QUsbStorageGadget::setBackingStore(const QStringList &paths)
{
    invoke(SLOT(setBackingStore(QStringList)), qVariantFromValue(paths));
}

/*!
    Adds device \a path to the list of backing stores.
*/
void QUsbStorageGadget::addBackingStore(const QString &path)
{
    invoke(SLOT(addBackingStore(QString)), qVariantFromValue(path));
}

/*!
    Removes device \a path from the list of backing stores.
*/
void QUsbStorageGadget::removeBackingStore(const QString &path)
{
    invoke(SLOT(removeBackingStore(QString)), qVariantFromValue(path));
}


/*!
    \class QUsbStorageGadgetProvider
    \inpublicgroup QtBaseModule


    \brief The QUsbStorageGadgetProvider class provides a backend implementation for QUsbStorageGadget

    This class works with the standard Linux file-backed mass storage gadget module.

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
            \o BackingStore
            \o List of files and device to use as backing store.
    \endtable

    \sa QUsbManager, UsbGadgetTask

    \ingroup hardware
*/

class QUsbStorageGadgetProviderPrivate {
public:
    QUsbManager *manager;

    QUsbStorageGadgetProviderPrivate();
    ~QUsbStorageGadgetProviderPrivate();
};

QUsbStorageGadgetProviderPrivate::QUsbStorageGadgetProviderPrivate()
    : manager(0)
{
}

QUsbStorageGadgetProviderPrivate::~QUsbStorageGadgetProviderPrivate()
{
    delete manager;
}

#define GADGET_NAME "Storage"

/*!
    Constructs a new QUsbStorageGadgetProvider in \a group and attach it to \a parent.
*/
QUsbStorageGadgetProvider::QUsbStorageGadgetProvider(const QString &group, QObject *parent)
    : QUsbStorageGadget(group, parent, Server)
{
    d = new QUsbStorageGadgetProviderPrivate;

    QSettings settings("Trolltech", "Usb");

    QByteArray function = settings.value("PeripheralController/Path").toByteArray() +
                                         "/gadget/function";

    QFile file(function);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text) &&
        file.readLine().trimmed() == "File-backed Storage Gadget") {
        initFromSystem();
    } else {
        initFromConfig();
    }

    setValue("gadget", QByteArray(GADGET_NAME));
}

/*!
    Destructor.
*/
QUsbStorageGadgetProvider::~QUsbStorageGadgetProvider()
{
    delete d;
}

/*!
    \reimp
*/
void QUsbStorageGadgetProvider::setVendorId(const quint16 id)
{
    setValue("vendorId", id);
}

/*!
    \reimp
*/
void QUsbStorageGadgetProvider::setProductId(const quint16 id)
{
    setValue("productId", id);
}

/*!
    \reimp
*/
void QUsbStorageGadgetProvider::setVendor(const QByteArray &vendor)
{
    setValue("vendor", vendor);
}

/*!
    \reimp
*/
void QUsbStorageGadgetProvider::setProduct(const QByteArray &product)
{
    setValue("product", product);
}

/*!
    \reimp
*/
void QUsbStorageGadgetProvider::saveConfig()
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

    v = value("backingStore");
    if (v.isValid())
        settings.setValue("BackingStore", v.toStringList());

    settings.endGroup();
}

/*!
    \reimp
*/
void QUsbStorageGadgetProvider::activate()
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

void QUsbStorageGadgetProvider::loadModule()
{
    QStringList args;

    args << "g_file_storage";

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

    v = value("backingStore");
    if (v.isValid())
        args << "file=" + v.toString();

    if (QProcess::execute("/sbin/modprobe", args) == 0) {
        setValue("active", true);
        emit activated();
    } else {
        emit activateFailed();
    }
}

void QUsbStorageGadgetProvider::abort()
{
    emit activateFailed();
}

/*!
    \reimp
*/
void QUsbStorageGadgetProvider::deactivate()
{
    if (value("active", false).toBool()) {
        if (QProcess::execute("/sbin/rmmod g_file_storage") == 0) {
            setValue("active", false);
            emit deactivated();
        } else {
            emit deactivateFailed();
        }
    }
}

/*!
    \reimp
*/
void QUsbStorageGadgetProvider::setBackingStore(const QStringList &paths)
{
    setValue("backingStore", qVariantFromValue(paths));
}

/*!
    \reimp
*/
void QUsbStorageGadgetProvider::addBackingStore(const QString &path)
{
    QStringList bs = backingStore();
    if (!bs.contains(path)) {
        bs.append(path);
        setValue("backingStore", qVariantFromValue(bs));
    }
}

/*!
    \reimp
*/
void QUsbStorageGadgetProvider::removeBackingStore(const QString &path)
{
    QStringList bs = backingStore();
    bs.removeAll(path);
    setValue("backingStore", qVariantFromValue(bs));
}

/*!
    Initialize the gadget from settings in the configuration file.
*/
void QUsbStorageGadgetProvider::initFromConfig()
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

    v = settings.value("BackingStore");
    if (v.isValid())
        setBackingStore(v.toStringList());
    else
        removeValue("backingStore");

    settings.endGroup();

    setValue("active", false);
}

/*!
    Initialize the gadget from settings obtained from the system.  This
    function is called if the storage gadget is \l active() when it is
    constructed.
*/
void QUsbStorageGadgetProvider::initFromSystem()
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

    file.setFileName(paramPath + "file");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QStringList devices = QString(file.readLine().trimmed()).split(',');
        setBackingStore(devices);
        file.close();
    }

    setValue("active", true);
}

