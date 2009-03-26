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

#include <QtopiaApplication>

#include "../qtopiabluetoothunittest.h"
#include <QSignalSpy>
#include <shared/util.h>

#include <qtopiacomm/qbluetoothlocaldevicemanager.h>


//TESTED_CLASS=QBluetoothLocalDeviceManager
//TESTED_FILES=src/libraries/qtopiacomm/bluetooth/qbluetoothlocaldevicemanager.h

/*
    Unit test for QBluetoothLocalDeviceManager
*/
class tst_QBluetoothLocalDeviceManager : public QtopiaBluetoothUnitTest
{
    Q_OBJECT

private slots:
    void defaultDevice();
    void defaultDevice_data();

    void devices();
    void devices_data();

    void defaultDeviceChanged();
    void defaultDeviceChanged_data();

    void deviceRemoved();
    void deviceRemoved_data();

    void deviceAdded();
    void deviceAdded_data();

    virtual void initTestCase();

};

QTEST_APP_MAIN( tst_QBluetoothLocalDeviceManager, QtopiaApplication )
#include "tst_qbluetoothlocaldevicemanager.moc"

/*?
    Initialisation before all test cases.
*/
void tst_QBluetoothLocalDeviceManager::initTestCase()
{
}

/*?
    Test the deviceAdded() signal.
    Ensures the deviceAdded() signal is emitted at the appropriate times by
    simulating the BlueZ DBus signals which are emitted when a local bluetooth device
    is added.
*/
void tst_QBluetoothLocalDeviceManager::deviceAdded()
{
    QFETCH(QString,device);
    QBluetoothLocalDeviceManager man;

    QSignalSpy spy(&man,SIGNAL(deviceAdded(QString)));
    bluez->emitAdapterAdded("/org/bluez/" + device);
    QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << device );
}

/*?
    Data for deviceAdded() test.
*/
void tst_QBluetoothLocalDeviceManager::deviceAdded_data()
{
    validDevice_data();
}

/*?
    Test the deviceRemoved() signal.
    Ensures the deviceRemoved() signal is emitted at the appropriate times by
    simulating the BlueZ DBus signals which are emitted when a local bluetooth device
    is removed.
*/
void tst_QBluetoothLocalDeviceManager::deviceRemoved()
{
    QFETCH(QString,device);
    QBluetoothLocalDeviceManager man;

    QSignalSpy spy(&man,SIGNAL(deviceRemoved(QString)));
    bluez->emitAdapterRemoved("/org/bluez/" + device);
    QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << device );
}

/*?
    Data for deviceRemoved() test.
*/
void tst_QBluetoothLocalDeviceManager::deviceRemoved_data()
{
    validDevice_data();
}

/*?
    Test the defaultDeviceChanged() signal.
    Ensures the defaultDeviceChanged() signal is emitted at the appropriate times by
    simulating the BlueZ DBus signals which are emitted when the default local bluetooth
    device changes.
*/
void tst_QBluetoothLocalDeviceManager::defaultDeviceChanged()
{
    QFETCH(QString,device);
    QBluetoothLocalDeviceManager man;

    QSignalSpy spy(&man,SIGNAL(defaultDeviceChanged(QString)));
    bluez->emitDefaultAdapterChanged("/org/bluez/" + device);
    QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << device );
}

/*?
    Data for defaultDeviceChanged() test.
*/
void tst_QBluetoothLocalDeviceManager::defaultDeviceChanged_data()
{
    validDevice_data();
}

/*?
    Test that the devices() method returns all the local devices BlueZ reports are available.
    This is done by reimplementing a portion of the BlueZ DBus interface.
*/
void tst_QBluetoothLocalDeviceManager::devices()
{
    QFETCH(QStringList, deviceNames);
    bluez->adapters = deviceNames;

    QBluetoothLocalDeviceManager man;

    QStringList dev = man.devices();

    QCOMPARE(dev, deviceNames);
}

/*?
    Data for the devices() test.
*/
void tst_QBluetoothLocalDeviceManager::devices_data()
{
    QTest::addColumn<QStringList>("deviceNames");

    QTest::newRow("simple hci") << (QStringList() << "hci0" << "hci1" << "hci2");
    QTest::newRow("simple non-hci") << (QStringList() << "foobar" << "blue" << "green");
//    QTest::newRow("with spaces") << (QStringList() << "foo bar" << "hi there" << "here are some spaces, and yet more spaces    ");
//    QTest::newRow("with colons") << (QStringList() << "foo:bar" << "hi:there" << "here:are:some:colons");
    QTest::newRow("only one") << (QStringList() << "jnrlheas");
    QTest::newRow("none") << QStringList();
//    QTest::newRow("with empty strings") << (QStringList() << "" << "" << "hi" << "");
//    QTest::newRow("with null strings") << (QStringList() << QString() << QString() << "hi" << QString());

    {
        QStringList manyList;
        for (int i = 0; i < 100; ++i) {
            QString longStr;
            for (int j = 0; j < 100; ++j) longStr += QString("a_long_string_%1_%2").arg(i).arg(j).toLatin1();
            manyList << longStr;
        }
        QTest::newRow("quite a few, with very long strings") << manyList;
    }
}

/*?
    Test that the defaultDevice() method returns the default local device reported by BlueZ.
    This is done by reimplementing a portion of the BlueZ DBus interface.
*/
void tst_QBluetoothLocalDeviceManager::defaultDevice()
{
    QFETCH(QString, device);
    bluez->defaultAdapter = device;

    QBluetoothLocalDeviceManager man;

    QString dev = man.defaultDevice();

    QCOMPARE(dev, device);
}

/*?
    Test data for defaultDevice().
*/
void tst_QBluetoothLocalDeviceManager::defaultDevice_data()
{
    validDevice_data();
}

