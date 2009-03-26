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

#include "qusbserialgadget.h"
#include "qusbmanager.h"

#include <QDebug>
#include <QFile>
#include <QSettings>
#include <QProcess>
#include <QDir>
#include <QTimer>

/*!
    \class QUsbSerialGadget
    \inpublicgroup QtBaseModule


    \brief The QUsbSerialGadget class is used to enable serial communication class of USB gadgets.

    The QUsbSerialGadget class is used to configure the USB gadget hardware as
    a serial communication class device.

    \code
        QUsbSerialGadget *gadget = new QUsbSerialGadget;
        if (gadget->available()) {
            connect(gadget, SIGNAL(activated()), this, SLOT(serialActivated()));
            gadget->activate();
        }
    \endcode

    \sa QUsbManager, UsbGadgetTask

    \ingroup hardware
*/

/*!
    Constructs a new QUsbSerialGadget object in \a group and attach it to
    \a parent.  If \a mode is \c Client, then the object is constructed in
    client mode and \a group may be empty to indicate that the default group
    should be used.
*/
QUsbSerialGadget::QUsbSerialGadget(const QString &group, QObject *parent,
                                   QAbstractIpcInterface::Mode mode)
    : QUsbGadget("QUsbSerialGadget", group, parent, mode)
{
    proxyAll(staticMetaObject);
}

/*!
    Returns true if the USB gadget supports settings whether CDC ACM mode is used.
*/
bool QUsbSerialGadget::supportsCdcAcm() const
{
    return value("cdcAcm").isValid();
}

/*!
    Returns true if the USB gadget uses the CDC ACM protocol.
*/
bool QUsbSerialGadget::cdcAcm() const
{
    return value("cdcAcm", false).toBool();
}

/*!
    Returns the name of the tty device.
*/
QByteArray QUsbSerialGadget::tty() const
{
    return value("tty").toByteArray();
}

/*!
    Set the serial gadget to use the CDC ACM protocol if \a acmMode is true.
*/
void QUsbSerialGadget::setCdcAcm(bool acmMode)
{
    invoke(SLOT(setCdcAcm(bool)), qVariantFromValue(acmMode));
}


/*!
    \class QUsbSerialGadgetProvider
    \inpublicgroup QtBaseModule


    \brief The QUsbSerialGadgetProvider class provides a backend implementation for QUsbSerialGadget

    This class works with the standard Linux serial communications gadget module.

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
            \o CdcAcm
            \o True if the Serial gadget uses the CDC ACM protocol.
    \endtable

    \sa QUsbManager, UsbGadgetTask

    \ingroup hardware
*/

class QUsbSerialGadgetProviderPrivate
{
public:
    QUsbManager *manager;

    QUsbSerialGadgetProviderPrivate();
    ~QUsbSerialGadgetProviderPrivate();
};

QUsbSerialGadgetProviderPrivate::QUsbSerialGadgetProviderPrivate()
    : manager(0)
{
}

QUsbSerialGadgetProviderPrivate::~QUsbSerialGadgetProviderPrivate()
{
    delete manager;
}

#define GADGET_NAME "Serial"

/*!
    Constructs a new QUsbSerialGadgetProvider in \a group and attach it to
    \a parent.
*/
QUsbSerialGadgetProvider::QUsbSerialGadgetProvider(const QString &group, QObject *parent)
    : QUsbSerialGadget(group, parent, Server)
{
    d = new QUsbSerialGadgetProviderPrivate;

    QSettings settings("Trolltech", "Usb");

    QString function = settings.value("PeripheralController/Path").toString() +
                       "/gadget/function";

    QFile file(function);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text) &&
        file.readLine().trimmed() == "Gadget Serial") {
        initFromSystem();
    } else {
        initFromConfig();
    }

    setValue("gadget", QByteArray(GADGET_NAME));
}

/*!
    Destructor.
*/
QUsbSerialGadgetProvider::~QUsbSerialGadgetProvider()
{
    delete d;
}

/*!
    \reimp
*/
void QUsbSerialGadgetProvider::setVendorId(const quint16)
{
}

/*!
    \reimp
*/
void QUsbSerialGadgetProvider::setProductId(const quint16)
{
}

/*!
    \reimp
*/
void QUsbSerialGadgetProvider::setVendor(const QByteArray &)
{
}

/*!
    \reimp
*/
void QUsbSerialGadgetProvider::setProduct(const QByteArray &)
{
}

/*!
    \reimp
*/
void QUsbSerialGadgetProvider::saveConfig()
{
    QSettings settings("Trolltech", "Usb");
    settings.beginGroup(GADGET_NAME);

    QVariant v = value("cdcAcm");
    if (v.isValid())
        settings.setValue("CdcAcm", v.toBool());

    settings.endGroup();
}

/*!
    \reimp
*/
void QUsbSerialGadgetProvider::activate()
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

void QUsbSerialGadgetProvider::loadModule()
{
    QStringList args;

    args << "g_serial";

    QVariant v = value("cdcAcm");
    if (v.isValid())
        args << "use_acm=" + (v.toBool() ? '1' : '0');

    if (QProcess::execute("/sbin/modprobe", args) == 0) {
        setValue("tty", "/dev/ttygs0");
        setValue("active", true);

        // need to delay before accessing /dev/ttygs0
        QTimer::singleShot(500, this, SIGNAL(activated()));
    } else {
        abort();
    }
}

void QUsbSerialGadgetProvider::abort()
{
    removeValue("tty");
    emit activateFailed();
}

/*!
    \reimp
*/
void QUsbSerialGadgetProvider::deactivate()
{
    if (value("active", false).toBool()) {
        if (QProcess::execute("/sbin/rmmod g_serial") == 0) {
            setValue("active", false);
            removeValue("tty");
            emit deactivated();
        } else {
            emit deactivateFailed();
        }
    }
}

/*!
    \reimp
*/
void QUsbSerialGadgetProvider::setCdcAcm(bool acmMode)
{
    setValue("cdcAcm", acmMode);
}


/*!
    Initialize the gadget from settings in the configuration file.
*/
void QUsbSerialGadgetProvider::initFromConfig()
{
    QSettings settings("Trolltech", "Usb");

    settings.beginGroup(GADGET_NAME);

    QVariant v = settings.value("CdcAcm");
    if (v.isValid())
        setCdcAcm(v.toBool());
    else
        removeValue("cdcAcm");

    settings.endGroup();

    setValue("active", false);
}

/*!
    Initialize the gadget from settings obtained from the system.  This
    function is called if the serial gadget is \l active() when it is
    constructed.
*/
void QUsbSerialGadgetProvider::initFromSystem()
{
    QString paramPath = "/sys/modules/g_serial/parameters/";

    QFile file(paramPath + "use_acm");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (file.readLine().trimmed() == "1")
            setCdcAcm(true);
        else
            setCdcAcm(false);
        file.close();
    }

    setValue("active", true);
}

