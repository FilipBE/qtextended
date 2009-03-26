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

#include "greenphoneethernetgadget.h"

#include <QUsbManager>

#include <QDebug>
#include <QFile>
#include <QSettings>
#include <QProcess>
#include <QDir>
#include <QTimer>
#include <qtopialog.h>

#define GADGET_NAME "GreenphoneEthernet"

//! [1]
GreenphoneEthernetGadgetProvider::GreenphoneEthernetGadgetProvider(const QString &group, QObject *parent)
    : QUsbEthernetGadget(group, parent, Server), m_manager(0)
{
    QSettings settings("Trolltech", "Usb");

    QString function = settings.value("PeripheralController/Path").toString();

    setValue("gadget", QByteArray(GADGET_NAME));

    QProcess tat;
    tat.start("tat", QStringList() << "remotemac");
    tat.waitForFinished(-1);
    m_defaultRemoteMac = tat.readAllStandardOutput().trimmed();

    tat.start("tat", QStringList() << "localmac");
    tat.waitForFinished(-1);
    m_defaultLocalMac = tat.readAllStandardOutput().trimmed();

    QFile file(function);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        initFromConfig();
        setValue("active", false);
        return;
    }

    foreach (QByteArray line, file.readAll().split('\n')) {
        if (line.startsWith("Function: ")) {
            initFromConfig();
            if (line.mid(10) == "Generic Network")
                setValue("active", true);
            else
                setValue("active", false);
        }
    }
}
//! [1]

GreenphoneEthernetGadgetProvider::~GreenphoneEthernetGadgetProvider()
{
    delete m_manager;
}

//! [2]
void GreenphoneEthernetGadgetProvider::setVendorId(const quint16 id)
{
    setValue("vendorId", id);
}

void GreenphoneEthernetGadgetProvider::setProductId(const quint16 id)
{
    setValue("productId", id);
}
//! [2]

//! [3]
void GreenphoneEthernetGadgetProvider::setVendor(const QByteArray &)
{
}

void GreenphoneEthernetGadgetProvider::setProduct(const QByteArray &)
{
}
//! [3]

//! [4]
void GreenphoneEthernetGadgetProvider::saveConfig()
{
    QSettings settings("Trolltech", "Usb");
    settings.beginGroup(GADGET_NAME);

    QVariant v = value("productId");
    if (v.isValid())
        settings.setValue("ProductId", v.toUInt());

    v = value("vendorId");
    if (v.isValid())
        settings.setValue("VendorId", v.toUInt());

    v = value("localMac");
    if (v.isValid() && v.toByteArray() != m_defaultLocalMac)
        settings.setValue("LocalMac", v.toString());

    v = value("remoteMac");
    if (v.isValid() && v.toByteArray() != m_defaultRemoteMac)
        settings.setValue("RemoteMac", v.toString());

    settings.endGroup();
}
//! [4]

//! [5]
void GreenphoneEthernetGadgetProvider::activate()
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
//! [5]

//! [6]
void GreenphoneEthernetGadgetProvider::loadModule()
{
    QStringList args;

    args << "net_fd";

    QVariant v = value("productId");
    if (v.isValid())
        args << "product_id=0x" + QString::number(v.toInt(), 16);

    v = value("vendorId");
    if (v.isValid())
        args << "vendor_id=0x" + QString::number(v.toInt(), 16);

    v = value("localMac");
    if (v.isValid())
        args << "local_mac_address=" + v.toString().remove(':');

    v = value("remoteMac");
    if (v.isValid())
        args << "remote_mac_address=" + v.toString().remove(':');

    qLog(USB) << "loading module" << args;
    if (QProcess::execute("/sbin/insmod", args) != 0) {
        abort();
        return;
    }

    args.clear();

    args << "bvd_bi";

    qLog(USB) << "loading module" << args;
    if (QProcess::execute("/sbin/insmod", args) != 0) {
        qLog(USB) << "unloading module net_fd";
        QProcess::execute("/sbin/rmmod net_fd");
        abort();
    } else {
        setValue("interface", "eth0");
        setValue("active", true);
        qLog(USB) << "ethernet gadget activated";
        QTimer::singleShot(500, this, SIGNAL(activated()));
    }
}
//! [6]

//! [7]
void GreenphoneEthernetGadgetProvider::abort()
{
    removeValue("interface");
    qLog(USB) << "ethernet gadget activate failed";
    emit activateFailed();
}
//! [7]

//! [8]
void GreenphoneEthernetGadgetProvider::deactivate()
{
    if (value("active", false).toBool()) {
        qLog(USB) << "unloading modules bvd_bi net_fd";
        if (QProcess::execute("/sbin/rmmod bvd_bi net_fd") == 0) {
            setValue("active", false);
            removeValue("interface");
            qLog(USB) << "ethernet gadget deactivated";
            emit deactivated();
        } else {
            qLog(USB) << "ethernet gadget deactivate failed";
            emit deactivateFailed();
        }
    }
}
//! [8]

//! [9]
void GreenphoneEthernetGadgetProvider::setRemoteMac(const QByteArray &mac)
{
    setValue("remoteMac", mac);
}

void GreenphoneEthernetGadgetProvider::setLocalMac(const QByteArray &mac)
{
    setValue("localMac", mac);
}
//! [9]

//! [10]
void GreenphoneEthernetGadgetProvider::initFromConfig()
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

    v = settings.value("RemoteMac");
    if (v.isValid())
        setRemoteMac(v.toByteArray());
    else
        setRemoteMac(m_defaultRemoteMac);

    v = settings.value("LocalMac");
    if (v.isValid())
        setLocalMac(v.toByteArray());
    else
        setLocalMac(m_defaultLocalMac);

    settings.endGroup();
}
//! [10]

