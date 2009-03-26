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

#include <QBluetoothLocalDevice>
#include <QBluetoothLocalDeviceManager>
#include <QBluetoothAddress>

#include <QVBoxLayout>
#include <QTabWidget>
#include <QListWidget>

#include "devinfo.h"

DevInfo::DevInfo(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    QTabWidget *tab = new QTabWidget(this);

    QBluetoothLocalDeviceManager mgr;

    // Obtain list of all devices on the system
    QStringList devices = mgr.devices();

    foreach (QString deviceName, devices) {

        QBluetoothLocalDevice dev(deviceName);

        if (!dev.isValid()) {
            continue;
        }

        QListWidget *list = new QListWidget;

        list->addItem(QString("[%1]").arg(tr("General")));
        list->addItem(tr("Name:"));
        list->addItem(dev.deviceName());
        list->addItem(tr("Address:"));
        list->addItem(dev.address().toString());
        list->addItem(tr("Manufacturer:"));
        list->addItem(dev.manufacturer());
        list->addItem(tr("Version:"));
        list->addItem(dev.version());
        list->addItem(tr("Company:"));
        list->addItem(dev.company());
        list->addItem(tr("Revision:"));
        list->addItem(dev.revision());

        list->addItem("");

        list->addItem(QString("[%1]").arg(tr("State")));
        if (dev.connectable()) {
            list->addItem(tr("Connectable:"));
            list->addItem(tr("true"));
        } else {
            list->addItem(tr("Connectable:"));
            list->addItem(tr("false"));
        }

        list->addItem(tr("Discoverable:"));
        if (dev.discoverable()) {
            list->addItem(tr("true"));
            list->addItem(tr("Timeout:"));
            list->addItem(QString::number(dev.discoverableTimeout() / 60));
        } else {
            list->addItem(tr("false"));
        }

        list->addItem("");

        list->addItem(QString("[%1]").arg(tr("Connections")));
        QList<QBluetoothAddress> connections = dev.connections();
        foreach (QBluetoothAddress addr, connections) {
            list->addItem(addr.toString());
        }

        list->addItem("");

        list->addItem(QString("[%1]").arg(tr("Paired Devices")));
        QList<QBluetoothAddress> paired = dev.pairedDevices();
        foreach (QBluetoothAddress addr, paired) {
            list->addItem(addr.toString());
        }

        QWidget *base = new QWidget;
        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addWidget(list);
        base->setLayout(mainLayout);
        tab->addTab(base, deviceName);
    }

    setCentralWidget(tab);
    setWindowTitle(tr("Local Device Info"));
}

DevInfo::~DevInfo()
{

}
