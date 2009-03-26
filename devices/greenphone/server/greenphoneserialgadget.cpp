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

#include "greenphoneserialgadget.h"

#include <QUsbManager>

#include <QDebug>
#include <QFile>
#include <QSettings>
#include <QProcess>
#include <QDir>
#include <qtopialog.h>

#define GADGET_NAME "GreenphoneSerial"

GreenphoneSerialGadgetProvider::GreenphoneSerialGadgetProvider(const QString &group, QObject *parent)
    : QUsbSerialGadget(group, parent, Server), m_manager(0)
{
    QSettings settings("Trolltech", "Usb");

    QString function = settings.value("PeripheralController/Path").toString();

    setValue("gadget", QByteArray(GADGET_NAME));

    QFile file(function);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        initFromConfig();
        setValue("active", false);
        return;
    }

    foreach (QByteArray line, file.readAll().split('\n')) {
        if (line.startsWith("Function: ")) {
            initFromConfig();
            if (line.mid(10) == "function prototype")
                setValue("active", true);
            else
                setValue("active", false);
        }
    }
}

GreenphoneSerialGadgetProvider::~GreenphoneSerialGadgetProvider()
{
    delete m_manager;
}

void GreenphoneSerialGadgetProvider::setVendorId(const quint16 id)
{
    setValue("vendorId", id);
}

void GreenphoneSerialGadgetProvider::setProductId(const quint16 id)
{
    setValue("productId", id);
}

void GreenphoneSerialGadgetProvider::setVendor(const QByteArray &)
{
}

void GreenphoneSerialGadgetProvider::setProduct(const QByteArray &)
{
}

void GreenphoneSerialGadgetProvider::saveConfig()
{
    QSettings settings("Trolltech", "Usb");
    settings.beginGroup(GADGET_NAME);

    QVariant v = value("productId");
    if (v.isValid())
        settings.setValue("ProductId", v.toUInt());

    v = value("vendorId");
    if (v.isValid())
        settings.setValue("VendorId", v.toUInt());

    settings.endGroup();
}

void GreenphoneSerialGadgetProvider::activate()
{
    if (!value("active", false).toBool()) {
        if (!m_manager)
            m_manager = new QUsbManager;

        if (m_manager->canActivate(GADGET_NAME)) {
            loadModule();
        } else {
            connect(m_manager, SIGNAL(deactivateCompleted()), this, SLOT(loadModule()));
            connect(m_manager, SIGNAL(deactivateAborted()), this, SLOT(abort()));
            m_manager->deactivateGadgets();
        }
    }
}

void GreenphoneSerialGadgetProvider::loadModule()
{
    QStringList args;

    args << "serial_fd";

    QVariant v = value("productId");
    if (v.isValid())
        args << "product_id=0x" + QString::number(v.toInt(), 16);

    v = value("vendorId");
    if (v.isValid())
        args << "vendor_id=0x" + QString::number(v.toInt(), 16);

    qLog(USB) << "loading module" << args;
    if (QProcess::execute("/sbin/insmod", args) != 0) {
        abort();
        return;
    }

    args.clear();

    args << "bvd_bi";

    qLog(USB) << "loading module" << args;
    if (QProcess::execute("/sbin/insmod", args) != 0) {
        qLog(USB) << "unloading module serial_fd";
        QProcess::execute("/sbin/rmmod serial_fd");
        abort();
    } else {
        setValue("tty", "/dev/ttyUSB0");
        setValue("active", true);
        qLog(USB) << "serial gadget activated";
        emit activated();
    }
}

void GreenphoneSerialGadgetProvider::abort()
{
    removeValue("tty");
    qLog(USB) << "serial gadget activate failed";
    emit activateFailed();
}


void GreenphoneSerialGadgetProvider::deactivate()
{
    if (value("active", false).toBool()) {
        // We need to remove bvd_bi before serial_fd, however removing bvd_bi
        // always succeeds even if we cannot remove serial_fd because a process
        // still has the serial port open.  Check /proc/modules and fail if the
        // use count for serial_fd != 0
        QFile modules("/proc/modules");
        if (!modules.open(QIODevice::ReadOnly | QIODevice::Text)) {
            emit deactivateFailed();
            return;
        }

        foreach (QByteArray line, modules.readAll().split('\n')) {
            if (line.startsWith("serial_fd")) {
                QList<QByteArray> fields = line.split(' ');
                fields.removeAll(QByteArray());

                // module not in use, unload
                if (fields.at(2) == "0")
                    break;

                qLog(USB) << tty() << "is in use";
                emit deactivateFailed();
                return;
            }
        }

        qLog(USB) << "unloading modules bvd_bi serial_fd";
        if (QProcess::execute("/sbin/rmmod bvd_bi serial_fd") == 0) {
            setValue("active", false);
            removeValue("tty");
            qLog(USB) << "serial gadget deactivated";
            emit deactivated();
        } else {
            qLog(USB) << "serial gadget deactivate failed";
            emit deactivateFailed();
        }
    }
}

void GreenphoneSerialGadgetProvider::setCdcAcm(bool)
{
}

void GreenphoneSerialGadgetProvider::initFromConfig()
{
    QSettings settings("Trolltech", "Usb");

    settings.beginGroup(GADGET_NAME);

    QVariant v = settings.value("VendorId");
    if (v.isValid())
        setVendorId(v.toUInt());
    else
        removeValue("vendorId");

    v = settings.value("ProductId");
    if (v.isValid())
        setProductId(v.toUInt());
    else
        removeValue("productId");

    settings.endGroup();

    setValue("active", false);
}

