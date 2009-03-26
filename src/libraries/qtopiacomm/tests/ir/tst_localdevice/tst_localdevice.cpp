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

#include <QObject>
#include <QTest>
#include <QDebug>
#include <QEventLoop>
#include <QStringList>
#include <QList>

#include <qirnamespace.h>
#include <qirlocaldevice.h>
#include <qirremotedevice.h>
#include <qirremotedevicewatcher.h>

//TESTED_CLASS=QIrLocalDevice
//TESTED_FILES=src/libraries/qtopiacomm/ir/qirlocaldevice.cpp

//
// The test case
//
class tst_LocalDevice: public QObject
{
    Q_OBJECT
public:
    tst_LocalDevice();
    virtual ~tst_LocalDevice();

public slots:
    void started();
    void deviceFound(const QIrRemoteDevice &remote);
    void devicesFound(const QList<QIrRemoteDevice> &devices);
    void finished();
    void watcherAwoke();

private slots:
    void init();
    void initTestCase();
    void cleanupTestCase();
    void testDeviceList();
    void testBringUpDown();
    void testQueryRemote();

private:
    QList<QIrRemoteDevice> m_devices;
};

tst_LocalDevice::tst_LocalDevice() : QObject()
{

}

tst_LocalDevice::~tst_LocalDevice()
{

}

void tst_LocalDevice::initTestCase()
{
}

void tst_LocalDevice::cleanupTestCase()
{
}

void tst_LocalDevice::init()
{
}

void tst_LocalDevice::started()
{
    qDebug() << "Discovery started...";
}

void tst_LocalDevice::finished()
{
    qDebug() << "Discovery finished";
}

void tst_LocalDevice::watcherAwoke()
{
    qDebug() << "Watcher has awakened...";
}

void tst_LocalDevice::deviceFound(const QIrRemoteDevice &dev)
{
    qDebug() << "Found a new remote device:";
    qDebug() << "Address:" << dev.address();
    qDebug() << "Name: " << dev.name();
    qDebug() << "Device Classes: ";
    qDebug() << "Plug and Play: " << (dev.deviceClasses() & QIr::PlugNPlay);
    qDebug() << "PDA: " << (dev.deviceClasses() & QIr::PDA);
    qDebug() << "Computer: " << (dev.deviceClasses() & QIr::Computer);
    qDebug() << "Printer: " << (dev.deviceClasses() & QIr::Printer);
    qDebug() << "Modem: " << (dev.deviceClasses() & QIr::Modem);
    qDebug() << "Fax: " << (dev.deviceClasses() & QIr::Fax);
    qDebug() << "LAN: " << (dev.deviceClasses() & QIr::LAN);
    qDebug() << "Telephony: " << (dev.deviceClasses() & QIr::Telephony);
    qDebug() << "FileServer: " << (dev.deviceClasses() & QIr::FileServer);
    qDebug() << "Communications: " << (dev.deviceClasses() & QIr::Communications);
    qDebug() << "Message: " << (dev.deviceClasses() & QIr::Message);
    qDebug() << "HTTP: " << (dev.deviceClasses() & QIr::HTTP);
    qDebug() << "OBEX: " << (dev.deviceClasses() & QIr::OBEX);
    qDebug() << "";
}

void tst_LocalDevice::devicesFound(const QList<QIrRemoteDevice> &devices)
{
    m_devices = devices;
}

void tst_LocalDevice::testDeviceList()
{
    QStringList devices = QIrLocalDevice::devices();
    qDebug() << "Got a list of devices" << devices;
}

void tst_LocalDevice::testBringUpDown()
{
    QStringList devices = QIrLocalDevice::devices();
    if (devices.size() == 0) {
        qDebug() << "NO IR DEVICES FOUND";
        return;
    }

    QIrLocalDevice dev(devices[0]);
    QCOMPARE(dev.deviceName(), devices[0]);

    QVERIFY(dev.isUp());

    bool ret = dev.bringDown();
    QVERIFY(ret);
    QVERIFY(!dev.isUp());

    ret = dev.bringUp();
    QVERIFY(ret);
    QVERIFY(dev.isUp());
}

void tst_LocalDevice::testQueryRemote()
{
    QStringList devices = QIrLocalDevice::devices();
    if (devices.size() == 0) {
        qDebug() << "NO IR DEVICES FOUND";
        return;
    }

    QIrLocalDevice dev(devices[0]);
    QVERIFY(dev.isUp());

    connect(&dev, SIGNAL(discoveryStarted()),
             this, SLOT(started()));
    connect(&dev, SIGNAL(remoteDeviceFound(QIrRemoteDevice)),
             this, SLOT(deviceFound(QIrRemoteDevice)));
    connect(&dev, SIGNAL(remoteDevicesFound(QList<QIrRemoteDevice>)),
             this, SLOT(devicesFound(QList<QIrRemoteDevice>)));
    connect(&dev, SIGNAL(discoveryCompleted()),
             this, SLOT(finished()));

    QIrRemoteDeviceWatcher watcher;

    connect(&watcher, SIGNAL(deviceFound()),
             this, SLOT(watcherAwoke()));

    bool w = watcher.watch(30000);
    QVERIFY(w);

    bool ret = dev.discoverRemoteDevices();
    QVERIFY(ret);

    foreach (QIrRemoteDevice remote, m_devices) {
        QVariant ret =
                dev.queryRemoteAttribute(remote, "OBEX", "IrDA:TinyTP:LsapSel");
        qDebug() << "Attribute valid: " << ret.isValid();

        if (ret.isValid()) {
            switch (ret.type()) {
                case QVariant::String:
                    qDebug() << "Attribute (String): " << ret.value<QString>();
                    break;
                case QVariant::UInt:
                    qDebug() << "Attribute (Integer): " << ret.value<uint>();
                    break;
                case QVariant::ByteArray:
                    qDebug() << "Attribute (OctetSeq): ";
                    break;
                default:
                    break;
            };
        }
    }
}

    QTEST_MAIN( tst_LocalDevice )

#include "tst_localdevice.moc"
