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

#define protected public
#define private public

#include <QtopiaApplication>

#include "../qtopiabluetoothunittest.h"

#include <qtopiacomm/private/qbluetoothnamespace_p.h>
#include <qtopiacomm/qbluetoothlocaldevice.h>
#include <qtopiacomm/qbluetoothremotedevice.h>
#include <qtopiacomm/qbluetoothaddress.h>

#include <QSignalSpy>
#include <shared/util.h>

#undef protected
#undef private

namespace QTest {
    template<>
    char *toString( const QList<QVariant> &value ) {
        QByteArray b = "(";
        QByteArray sep;
        foreach (QVariant v, value) {
            b.append(sep);
            b.append(v.typeName());
            b.append(":'");
            b.append(v.toString().toLatin1());
            b.append("'");
            sep = ",";
        }
        b.append(")");
        return qstrdup(b.constData());
    }
    template<>
    char *toString( QVariant const& v ) {
        QByteArray b = v.typeName();
        b.append(":");
        b.append(v.canConvert<QString>() ? ("'" + v.toString() + "'") : "can't display as string");
        return qstrdup(b.constData());
    }
    template<>
    bool qCompare( QVariantList const &a, QVariantList const &e,
                   char const *actual, char const *expected, char const *file, int line)
    {
        if (!qCompare(a.count(), e.count(),
                    qPrintable(QString("%1.count()").arg(actual)),
                    qPrintable(QString("%1.count()").arg(expected)),
                    file, line)) return false;

        for (int i = 0; i < a.count(); ++i) {
            QVariant av = a[i];
            QVariant ev = e[i];
            if (av == ev) continue;
            if (!qCompare(av.userType(), ev.userType(),
                    qPrintable(QString("%1[%2].userType()").arg(actual).arg(i)),
                    qPrintable(QString("%1[%2].userType()").arg(expected).arg(i)),
                    file, line)) return false;

            /* Try custom bluetooth types */
#define HANDLE_TYPE(T) \
    else if (av.userType() == qMetaTypeId<T>()) { \
        if (!qCompare(av.value<T>(), ev.value<T>(), \
                qPrintable(QString("%1[%2].value<%3>").arg(actual).arg(i).arg(#T)), \
                qPrintable(QString("%1[%2].value<%3>").arg(expected).arg(i).arg(#T)), \
                file, line)) return false; \
    }

            if (false) {}
            HANDLE_TYPE(QBluetoothAddress)
            HANDLE_TYPE(QBluetooth::DeviceMajor)
            HANDLE_TYPE(QBluetooth::ServiceClasses)
            HANDLE_TYPE(QBluetoothLocalDevice::State)
            HANDLE_TYPE(uchar)
            else
                if (!qCompare(av, ev,
                       qPrintable(QString("%1[%2]").arg(actual).arg(i)), \
                       qPrintable(QString("%1[%2]").arg(expected).arg(i)), \
                       file, line)) return false;
        }

        return true;

    }

}

//TESTED_CLASS=QBluetoothLocalDevice
//TESTED_FILES=src/libraries/qtopiacomm/bluetooth/qbluetoothlocaldevice.cpp

/*
    This class primarily tests that QBluetoothLocalDevice correctly interacts with
    the underlying BlueZ D-Bus service.
*/
class tst_QBluetoothLocalDevice : public QtopiaBluetoothUnitTest
{
    Q_OBJECT

public:
    tst_QBluetoothLocalDevice();
    virtual ~tst_QBluetoothLocalDevice();


private slots:
    void disconnectRemoteDeviceRequested();
    void disconnectRemoteDeviceRequested_data();

    void remoteClassUpdated();
    void remoteClassUpdated_data();

    void remoteNameFailed();
    void remoteNameFailed_data();

    void remoteNameUpdated();
    void remoteNameUpdated_data();

    void pairingRemoved();
    void pairingRemoved_data();

    void pairingCreated();
    void pairingCreated_data();

    void remoteAliasRemoved();
    void remoteAliasRemoved_data();

    void remoteAliasChanged();
    void remoteAliasChanged_data();

    void discoveryCompleted();
    void discoveryCompleted_data();

    void discoveryStarted();
    void discoveryStarted_data();

    void remoteDeviceDisappeared();
    void remoteDeviceDisappeared_data();

    void remoteDeviceDisconnected();
    void remoteDeviceDisconnected_data();

    void remoteDeviceConnected();
    void remoteDeviceConnected_data();

    void discoverableTimeoutChanged();
    void discoverableTimeoutChanged_data();

    void stateChanged();
    void stateChanged_data();

    void nameChanged();
    void nameChanged_data();

    void knownDevices_qdatetime();
    void knownDevices_qdatetime_data();

    void updateRemoteDevice();
    void updateRemoteDevice_data();

    void isPaired();
    void isPaired_data();

    void pairedDevices();
    void pairedDevices_data();

    void lastUsed();
    void lastUsed_data();

    void lastSeen();
    void lastSeen_data();

    void remoteAlias();
    void remoteAlias_data();

    void knownDevices();
    void knownDevices_data();

    void disconnectRemoteDevice();
    void disconnectRemoteDevice_data();

    void pinCodeLength();
    void pinCodeLength_data();

    void isPeriodicDiscoveryEnabled();
    void isPeriodicDiscoveryEnabled_data();

    void setPeriodicDiscoveryEnabled();
    void setPeriodicDiscoveryEnabled_data();

    void setRemoteAlias();
    void setRemoteAlias_data();

    void removeRemoteAlias();
    void removeRemoteAlias_data();

    void isConnected();
    void isConnected_data();

    void discoverRemoteDevices();

    void connections();
    void connections_data();

    void isUp();
    void isUp_data();

    void connectable();
    void connectable_data();

    void discoverable();
    void discoverable_data();

    void setDiscoverableTimeout();
    void setDiscoverableTimeout_data();

    void discoverableTimeout();
    void discoverableTimeout_data();

    void setName();
    void setName_data();

    void name();
    void name_data();

    void company();
    void company_data();

    void revision();
    void revision_data();

    void version();
    void version_data();

    void manufacturer();
    void manufacturer_data();

    void address();
    void address_data();

    void constructor_qbluetoothaddress();
    void constructor_qbluetoothaddress_data();

    void constructor_qstring();
    void constructor_qstring_data();

    void constructor();
    void constructor_data();

    virtual void initTestCase();
    virtual void cleanupTestCase();
    virtual void init();
    virtual void cleanup();

private:
    void strings_data(char const *name);
    void bools_data(char const *name);
    void addressList_data(char const *name);
};

QTEST_APP_MAIN( tst_QBluetoothLocalDevice, QtopiaApplication )
#include "tst_qbluetoothlocaldevice.moc"

Q_DECLARE_METATYPE(QBluetoothLocalDevice::State);
Q_DECLARE_METATYPE(QBluetooth::ServiceClasses);
Q_DECLARE_METATYPE(QBluetooth::DeviceMajor);

/*?
    Expands to a test of a function in QBluetoothLocalDevice which acts solely as
    a wrapper for a BlueZ accessor function.
    @a T is the type of variable returned by the function,
    @a A is the name of the variable in test data and the BlueZ Adapter stub, and
    @a B is the name of the getter function in QBluetoothLocalDevice.
*/
#define TEST_BLUEZ_WRAPPER_FUNCTION(T,A,B) {\
    QFETCH(T, A);\
\
    /* Create the virtual BlueZ adapter */ \
    MAKEADAPTER("mydev");\
    bluez->adapterMap["dev"] = "mydev";\
\
    /* Set the BlueZ adapter stub's member to the test data */ \
    bluezAdapter->A = A;\
\
    /* Construct device and call the function; we expect it to \
       return what's in the stub */ \
    QBluetoothLocalDevice dev( "dev" );\
    T actual_ ## A = dev.B();\
\
    /* Clean up after ourselves */ \
    DESTROYADAPTER("mydev");\
    bluez->adapterMap.remove("dev");\
\
    /* Make sure we had the right device */ \
    QCOMPARE( dev.deviceName(), QString("mydev") );\
\
    /* Make sure we got back what we set in the stub */ \
    QCOMPARE( actual_ ## A, A );\
}

/*?
    Expands to a test of a function in QBluetoothLocalDevice which acts solely as
    a wrapper for a BlueZ mutator function.
    @a T is the type of variable set by the function,
    @a A is the name of the variable in test data and the BlueZ Adapter stub, and
    @a B is the name of the setter function in QBluetoothLocalDevice.
*/
#define TEST_BLUEZ_SETWRAPPER_FUNCTION(T,A,B) {\
    QFETCH(T, A);\
\
    /* Create the virtual BlueZ adapter */ \
    MAKEADAPTER("mydev");\
    bluez->adapterMap["dev"] = "mydev";\
\
    /* Set the BlueZ adapter stub's member to default-constructed value */ \
    bluezAdapter->A = T();\
\
    /* Construct device and call the function; we expect it to \
       return what's in the stub */ \
    QBluetoothLocalDevice dev( "dev" );\
    dev.B(A);\
\
    /* Make sure we had the right device */ \
    QCOMPARE( dev.deviceName(), QString("mydev") );\
\
    /* Make sure the appropriate member of the stub was set to the right value */ \
    QTRY_COMPARE( bluezAdapter->A, A );\
\
    /* Clean up after ourselves */ \
    DESTROYADAPTER("mydev");\
    bluez->adapterMap.remove("dev");\
\
}

/*?
    Expands to a test of a signal in QBluetoothLocalDevice which acts solely as
    a wrapper around a BlueZ signal.
    @a SIG is the signal in the QBluetoothLocalDevice class,
    @a GETDATA is a QList<QVariant> of arguments expected to be received in the
    QBluetoothLocalDevice signal.
    Rest of args are the combined signal and data to emit.
*/
#define TEST_BLUEZ_WRAPPER_SIGNAL(SIG,GETDATA,...) {\
    /* Create the virtual BlueZ adapter */ \
    MAKEADAPTER("mydev");\
    bluez->adapterMap["dev"] = "mydev";\
\
    /* Construct device, send BlueZ signal, make sure expected signal was \
       emitted from QBluetoothLocalDevice */ \
    QBluetoothLocalDevice dev( "dev" );\
    { \
        QSignalSpy spy(&dev,SIG);\
        dev.connectNotify(SIG); \
        bluezAdapter->emit ## __VA_ARGS__;\
        QTRY_COMPARE( spy.count(), 1 ); \
        QCOMPARE( spy.takeAt(0), (GETDATA) );\
    } \
\
    /* Clean up */ \
    DESTROYADAPTER("mydev");\
    bluez->adapterMap.remove("dev");\
\
    /* Make sure we had the right device */ \
    QCOMPARE( dev.deviceName(), QString("mydev") );\
}


tst_QBluetoothLocalDevice::tst_QBluetoothLocalDevice()
{
}

tst_QBluetoothLocalDevice::~tst_QBluetoothLocalDevice()
{
}

void tst_QBluetoothLocalDevice::initTestCase()
{
    qRegisterMetaType<QBluetooth::DeviceMajor>();
}

void tst_QBluetoothLocalDevice::cleanupTestCase()
{
}

void tst_QBluetoothLocalDevice::init()
{
}

/*?
    Cleanup after each test case.
    Stops intercepting D-Bus calls to BlueZ and destroys
    the most common virtual adapter.
*/
void tst_QBluetoothLocalDevice::cleanup()
{
    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");
}

/*?
    Test that the default constructor uses the BlueZ default adapter.
*/
void tst_QBluetoothLocalDevice::constructor()
{
    QFETCH(QString, device);

    bluez->defaultAdapter = device;

    MAKEADAPTER(device + "_dev");
    bluez->adapterMap[device] = device + "_dev";

    QBluetoothLocalDevice dev;
    QString name = dev.deviceName();

    DESTROYADAPTER(device + "_dev");
    bluez->adapterMap.remove(device);

    QCOMPARE( name, device + "_dev" );
    QCOMPARE( bluez->slotSpy, "BluezStub::FindAdapter( \"" + device + "\" )" );
}

void tst_QBluetoothLocalDevice::constructor_data()
{
    validDevice_data();
}

/*?
    Test that the QString constructor uses the device returned by
    the BlueZ FindAdapter method.
*/
void tst_QBluetoothLocalDevice::constructor_qstring()
{
    QFETCH(QString, device);

    MAKEADAPTER(device + "_dev");
    bluez->adapterMap[device] = device + "_dev";

    QBluetoothLocalDevice dev(device);
    QString name = dev.deviceName();

    DESTROYADAPTER(device + "_dev");
    bluez->adapterMap.remove(device);

    QCOMPARE( name, device + "_dev" );
    QCOMPARE( bluez->slotSpy, "BluezStub::FindAdapter( \"" + device + "\" )" );
}

void tst_QBluetoothLocalDevice::constructor_qstring_data()
{
    validDevice_data();
}

/*?
    Test that the QBluetoothAddress constructor uses the device returned by
    the BlueZ FindAdapter method.
*/
void tst_QBluetoothLocalDevice::constructor_qbluetoothaddress()
{
    QFETCH(QString, addressStr);
    QBluetoothAddress addr(addressStr);

    MAKEADAPTER("mydev");
    bluez->adapterMap[addressStr] = "mydev";

    QBluetoothLocalDevice dev( addr );
    QString name = dev.deviceName();

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove(addressStr);

    QCOMPARE( name, QString("mydev") );
    QCOMPARE( bluez->slotSpy, "BluezStub::FindAdapter( \"" + addressStr + "\" )" );
}

void tst_QBluetoothLocalDevice::constructor_qbluetoothaddress_data()
{
    validAddress_data();
}

/*?
    Test that the address() getter returns the address reported by BlueZ.
*/
void tst_QBluetoothLocalDevice::address()
{
    QFETCH(QString, addressStr);

    MAKEADAPTER("mydev");
    bluez->adapterMap["dev"] = "mydev";
    bluezAdapter->address = addressStr;

    QBluetoothLocalDevice dev( "dev" );
    QString name = dev.deviceName();

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");

    QCOMPARE( name, QString("mydev") );
    QCOMPARE( dev.address(), QBluetoothAddress(addressStr) );
}

void tst_QBluetoothLocalDevice::address_data()
{
    validAddress_data();
}

/*?
    Test the manufacturer() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::manufacturer()
{
    TEST_BLUEZ_WRAPPER_FUNCTION(QString, manufacturer, manufacturer);
}

void tst_QBluetoothLocalDevice::manufacturer_data()
{
    validManufacturer_data();
}

/*?
    Test the version() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::version()
{
    TEST_BLUEZ_WRAPPER_FUNCTION(QString, version, version);
}

void tst_QBluetoothLocalDevice::version_data()
{
    strings_data("version");
}

/*?
    Test the revision() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::revision()
{
    TEST_BLUEZ_WRAPPER_FUNCTION(QString, revision, revision);
}

void tst_QBluetoothLocalDevice::revision_data()
{
    strings_data("revision");
}

/*?
    Test the company() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::company()
{
    TEST_BLUEZ_WRAPPER_FUNCTION(QString, company, company);
}

void tst_QBluetoothLocalDevice::company_data()
{
    strings_data("company");
}

/*?
    Test the name() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::name()
{
    TEST_BLUEZ_WRAPPER_FUNCTION(QString, name, name);
}

void tst_QBluetoothLocalDevice::name_data()
{
    strings_data("name");
}

/*?
    Test the setName() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::setName()
{
    TEST_BLUEZ_SETWRAPPER_FUNCTION(QString, name, setName);
}

void tst_QBluetoothLocalDevice::setName_data()
{
    strings_data("name");
}

/*?
    Test the discoverableTimeout() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::discoverableTimeout()
{
    TEST_BLUEZ_WRAPPER_FUNCTION(uint, discoverableTimeout, discoverableTimeout);
}

void tst_QBluetoothLocalDevice::discoverableTimeout_data()
{
    QTest::addColumn<uint>("discoverableTimeout");
    
    QTest::newRow("zero")  << 0u;
    QTest::newRow("one")   << 1u;
    QTest::newRow("two")   << 2u;
    QTest::newRow("three") << 3u;

    QTest::newRow("magnitude 1") << 23u;
    QTest::newRow("magnitude 2") << 790u;
    QTest::newRow("magnitude 3") << 6002u;
    QTest::newRow("magnitude 4") << 23041u;
    QTest::newRow("magnitude 5") << 723041u;
    QTest::newRow("magnitude 6") << 7230410u;
    QTest::newRow("magnitude 7") << 27230410u;
    QTest::newRow("magnitude 8") << 127230410u;
    QTest::newRow("magnitude 9") << 3127230410u;

    QTest::newRow("near signed upper limit 1") << (1u << 31) - 3u;
    QTest::newRow("near signed upper limit 2") << (1u << 31) - 2u;

    QTest::newRow("signed upper limit") << (1u << 31) - 1u;

    QTest::newRow("near upper limit 1") << 0xFFFFFFFFu - 2u;
    QTest::newRow("near upper limit 2") << 0xFFFFFFFFu - 1u;

    QTest::newRow("upper limit") << 0xFFFFFFFFu;
}

/*?
    Test the setDiscoverable() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::setDiscoverableTimeout()
{
    TEST_BLUEZ_SETWRAPPER_FUNCTION(uint, discoverableTimeout, setDiscoverable);
}

void tst_QBluetoothLocalDevice::setDiscoverableTimeout_data()
{
    discoverableTimeout_data();
}

/*?
    Test the discoverable() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::discoverable()
{
    TEST_BLUEZ_WRAPPER_FUNCTION(bool, discoverable, discoverable);
}

void tst_QBluetoothLocalDevice::discoverable_data()
{
    bools_data("discoverable");
}

/*?
    Test the connectable() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::connectable()
{
    TEST_BLUEZ_WRAPPER_FUNCTION(bool, connectable, connectable);
}

void tst_QBluetoothLocalDevice::connectable_data()
{
    bools_data("connectable");
}

/*?
    Test the isUp() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::isUp()
{
    TEST_BLUEZ_WRAPPER_FUNCTION(bool, connectable, isUp);
}

void tst_QBluetoothLocalDevice::isUp_data()
{
    connectable_data();
}

/*?
    Test data for any general string-based test.
*/
void tst_QBluetoothLocalDevice::strings_data(char const *name)
{
    QTest::addColumn<QString>(name);
    
    QTest::newRow("empty") << "";
    QTest::newRow("null") << QString();
    QTest::newRow("simple") << "foo";
    QTest::newRow("some odd characters") << "lxc0ph423{_]>Z<!@&^%XZ:~``*/*-/???????????";
    {
        QString longStr;
        for (int i = 0; i < 1000; ++i) longStr += QString(" a long string %1 ").arg(i);
        QTest::newRow("rather long") << longStr;
    }
}

/*?
    Test data for any general bool-based test.
*/
void tst_QBluetoothLocalDevice::bools_data(char const *name)
{
    QTest::addColumn<bool>(name);
    
    QTest::newRow("false 1") << false;
    QTest::newRow("true 1")  << true;
    QTest::newRow("false 2") << false;
    QTest::newRow("true 2")  << true;
}

/*?
    Test the connections() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::connections()
{
    QFETCH(QStringList, connections);

    MAKEADAPTER("mydev");
    bluez->adapterMap["dev"] = "mydev";
    bluezAdapter->connections = connections;

    QBluetoothLocalDevice dev( "dev" );
    QList<QBluetoothAddress> actual_connections = dev.connections();

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");

    QCOMPARE( dev.deviceName(), QString("mydev") );

    foreach(QBluetoothAddress addr, actual_connections) {
         QVERIFY( connections.contains(addr.toString(), Qt::CaseInsensitive) );
    }
}

void tst_QBluetoothLocalDevice::connections_data()
{
    addressList_data("connections");
}

/*?
    Test data for any test needing a list of valid Bluetooth addresses.
*/
void tst_QBluetoothLocalDevice::addressList_data(char const *name)
{
    QTest::addColumn<QStringList>(name);

    QStringList all;

    QTest::newRow("none") << all;

    all << "01:23:45:67:89:AB";
    QTest::newRow("simple") << all;

    all << "45:67:89:Ab:cd:eF";
    QTest::newRow("mixed case") << all;

    all << "98:76:54:32:10:98";
    QTest::newRow("< A") << all;

    all << "FE:DC:BA:AB:CD:EF";
    QTest::newRow("> 9") << all;

    all << "00:00:00:00:00:00";
    QTest::newRow("any") << all;

    all << "FF:FF:FF:FF:FF:FF";
    QTest::newRow("all") << all;

    all << "00:00:00:FF:FF:FF";
    QTest::newRow("local") << all;
}

/*?
    Test that discoverRemoteDevices() calls the correct BlueZ method.
*/
void tst_QBluetoothLocalDevice::discoverRemoteDevices()
{
    bluezAdapter->slotSpy = "";
    MAKEADAPTER("mydev");
    bluez->adapterMap["dev"] = "mydev";

    QBluetoothLocalDevice dev( "dev" );
    dev.discoverRemoteDevices();

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");

    QCOMPARE( dev.deviceName(), QString("mydev") );
    QCOMPARE( bluezAdapter->slotSpy, QString("BluezAdapterStub::DiscoverDevices()") );
}

/*?
    Test the isConnected() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::isConnected()
{
    QFETCH(QString, address);
    QFETCH(bool, connected);

    MAKEADAPTER("mydev");
    bluez->adapterMap["dev"] = "mydev";
    bluezAdapter->connected.clear();
    bluezAdapter->connected[address] = connected;

    QBluetoothLocalDevice dev( "dev" );
    bool actual_connected = dev.isConnected(QBluetoothAddress(address));

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");

    QCOMPARE( dev.deviceName(), QString("mydev") );
    QCOMPARE( actual_connected, connected );
}

void tst_QBluetoothLocalDevice::isConnected_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<bool>("connected");

    QStringList all;
    all << "01:23:45:67:89:AB";
    all << "45:67:89:Ab:cd:eF";
    all << "98:76:54:32:10:98";
    all << "FE:DC:BA:AB:CD:EF";
    all << "00:00:00:00:00:00";
    all << "FF:FF:FF:FF:FF:FF";
    all << "00:00:00:FF:FF:FF";

    foreach(QString str, all) {
        QTest::newRow(qPrintable(str + " true 1"))  << str << true;
        QTest::newRow(qPrintable(str + " false 1")) << str << false;
        QTest::newRow(qPrintable(str + " true 2"))  << str << true;
        QTest::newRow(qPrintable(str + " false 2")) << str << false;
    }
}

/*?
    Test the removeRemoteAlias() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::removeRemoteAlias()
{
    QFETCH(QString, addressStr);
    bluezAdapter->slotSpy = "";
    MAKEADAPTER("mydev");
    bluez->adapterMap["dev"] = "mydev";

    QBluetoothLocalDevice dev( "dev" );
    dev.removeRemoteAlias(QBluetoothAddress(addressStr));

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");

    QCOMPARE( dev.deviceName(), QString("mydev") );
    QCOMPARE( bluezAdapter->slotSpy, QString("BluezAdapterStub::ClearRemoteAlias( \"" + addressStr + "\" )") );
}

void tst_QBluetoothLocalDevice::removeRemoteAlias_data()
{
    validAddress_data();
}

/*?
    Test the setRemoteAlias() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::setRemoteAlias()
{
    QFETCH(QString, address);
    QFETCH(QString, alias);

    MAKEADAPTER("mydev");
    bluez->adapterMap["dev"] = "mydev";
    bluezAdapter->remoteAlias.clear();

    QBluetoothLocalDevice dev( "dev" );
    dev.setRemoteAlias(QBluetoothAddress(address), alias);

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");

    QCOMPARE( dev.deviceName(), QString("mydev") );
    QCOMPARE( bluezAdapter->remoteAlias[address], alias );
}

void tst_QBluetoothLocalDevice::setRemoteAlias_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<QString>("alias");

    QStringList all;
    all << "01:23:45:67:89:AB";
    all << "45:67:89:Ab:cd:eF";
    all << "98:76:54:32:10:98";
    all << "FE:DC:BA:AB:CD:EF";
    all << "00:00:00:00:00:00";
    all << "FF:FF:FF:FF:FF:FF";
    all << "00:00:00:FF:FF:FF";

    foreach(QString str, all) {
        QTest::newRow(qPrintable(str + " empty"))               << str << "";
        QTest::newRow(qPrintable(str + " null"))                << str << QString();
        QTest::newRow(qPrintable(str + " simple"))              << str << "foo";
        QTest::newRow(qPrintable(str + " some odd characters")) << str << "lxc0ph423{_]>Z<!@&^%XZ:~``*/*-/???????????";
        {
            QString longStr;
            for (int i = 0; i < 1000; ++i) longStr += QString(" a long string %1 ").arg(i);
            QTest::newRow(qPrintable(str + " rather long")) << str << longStr;
        }
    }
}

/*?
    Test the setPeriodicDiscoveryEnabled() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::setPeriodicDiscoveryEnabled()
{
    QFETCH(bool, enabled);
    QFETCH(QString, slot);

    MAKEADAPTER("mydev");
    bluez->adapterMap["dev"] = "mydev";

    QBluetoothLocalDevice dev( "dev" );
    dev.setPeriodicDiscoveryEnabled(enabled);

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");

    QCOMPARE( dev.deviceName(), QString("mydev") );
    QCOMPARE( bluezAdapter->slotSpy, slot );
}

void tst_QBluetoothLocalDevice::setPeriodicDiscoveryEnabled_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<QString>("slot");
    
    QTest::newRow("false 1") << false << "BluezAdapterStub::StopPeriodicDiscovery()";
    QTest::newRow("true 1")  << true  << "BluezAdapterStub::StartPeriodicDiscovery()";
    QTest::newRow("false 2") << false << "BluezAdapterStub::StopPeriodicDiscovery()";
    QTest::newRow("true 2")  << true  << "BluezAdapterStub::StartPeriodicDiscovery()";
}

/*?
    Test the isPeriodicDiscoveryEnabled() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::isPeriodicDiscoveryEnabled()
{
    TEST_BLUEZ_WRAPPER_FUNCTION(bool, periodicDiscovery, isPeriodicDiscoveryEnabled);
}

void tst_QBluetoothLocalDevice::isPeriodicDiscoveryEnabled_data()
{
    bools_data("periodicDiscovery");
}

/*?
    Test the pinCodeLength() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::pinCodeLength()
{
    QFETCH(QString, address);
    QFETCH(uchar, length);

    MAKEADAPTER("mydev");
    bluez->adapterMap["dev"] = "mydev";
    bluezAdapter->pincodeLength.clear();
    bluezAdapter->pincodeLength[address] = length;

    QBluetoothLocalDevice dev( "dev" );
    uchar actual_length = dev.pinCodeLength(QBluetoothAddress(address));

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");

    QCOMPARE( dev.deviceName(), QString("mydev") );
    QCOMPARE( actual_length, length );
}

void tst_QBluetoothLocalDevice::pinCodeLength_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<uchar>("length");

    QStringList all;
    all << "01:23:45:67:89:AB";
    all << "45:67:89:Ab:cd:eF";
    all << "98:76:54:32:10:98";
    all << "FE:DC:BA:AB:CD:EF";
    all << "00:00:00:00:00:00";
    all << "FF:FF:FF:FF:FF:FF";
    all << "00:00:00:FF:FF:FF";

    foreach(QString str, all) {
        QTest::newRow(qPrintable(str + " zero"))             << str << (uchar)0u;
        QTest::newRow(qPrintable(str + " one"))              << str << (uchar)1u;
        QTest::newRow(qPrintable(str + " two"))              << str << (uchar)2u;

        QTest::newRow(qPrintable(str + " magnitude 1"))      << str << (uchar)72u;
        QTest::newRow(qPrintable(str + " magnitude 2"))      << str << (uchar)109u;

        QTest::newRow(qPrintable(str + " signed near upper limit 1"))      << str << (uchar)((1u << 7) - 3u);
        QTest::newRow(qPrintable(str + " signed near upper limit 2"))      << str << (uchar)((1u << 7) - 2u);

        QTest::newRow(qPrintable(str + " signed upper limit"))      << str << (uchar)((1u << 7) - 1u);

        QTest::newRow(qPrintable(str + " near upper limit 1"))      << str << (uchar)253u;
        QTest::newRow(qPrintable(str + " near upper limit 2"))      << str << (uchar)254u;

        QTest::newRow(qPrintable(str + " upper limit"))      << str << (uchar)255u;
    }
}

/*?
    Test the disconnectRemoteDevice() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::disconnectRemoteDevice()
{
    QFETCH(QString, addressStr);
    bluezAdapter->slotSpy = "";
    MAKEADAPTER("mydev");
    bluez->adapterMap["dev"] = "mydev";

    QBluetoothLocalDevice dev( "dev" );
    dev.disconnectRemoteDevice(QBluetoothAddress(addressStr));

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");

    QCOMPARE( dev.deviceName(), QString("mydev") );
    QCOMPARE( bluezAdapter->slotSpy, QString("BluezAdapterStub::DisconnectRemoteDevice( \"" + addressStr + "\" )") );
}

void tst_QBluetoothLocalDevice::disconnectRemoteDevice_data()
{
    validAddress_data();
}

/*?
    Test the knownDevices() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::knownDevices()
{
    QFETCH(QStringList, devices);

    MAKEADAPTER("mydev");
    bluez->adapterMap["dev"] = "mydev";
    bluezAdapter->remoteDevices = devices;

    QBluetoothLocalDevice dev( "dev" );
    QList<QBluetoothAddress> actual_devices = dev.knownDevices();

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");

    QCOMPARE( dev.deviceName(), QString("mydev") );

    foreach(QBluetoothAddress addr, actual_devices) {
         QVERIFY( devices.contains(addr.toString(), Qt::CaseInsensitive) );
    }
}

void tst_QBluetoothLocalDevice::knownDevices_data()
{
    addressList_data("devices");
}

/*?
    Test the remoteAlias() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::remoteAlias()
{
    QFETCH(QString, address);
    QFETCH(QString, alias);

    MAKEADAPTER("mydev");
    bluez->adapterMap["dev"] = "mydev";
    bluezAdapter->remoteAlias.clear();
    bluezAdapter->remoteAlias[address] = alias;

    QBluetoothLocalDevice dev( "dev" );
    QString actual_alias = dev.remoteAlias(QBluetoothAddress(address));

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");

    QCOMPARE( dev.deviceName(), QString("mydev") );
    QCOMPARE( actual_alias, alias );
}

void tst_QBluetoothLocalDevice::remoteAlias_data()
{
    setRemoteAlias_data();
}

/*?
    Test the lastSeen() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::lastSeen()
{
    QFETCH(QString, address);
    QFETCH(QString, time);

    QDateTime expected_lastSeen = QDateTime::fromString(time, Qt::ISODate);
    expected_lastSeen.setTimeSpec(Qt::UTC);
    expected_lastSeen = expected_lastSeen.toLocalTime();

    MAKEADAPTER("mydev");
    bluez->adapterMap["dev"] = "mydev";
    bluezAdapter->lastSeen.clear();
    bluezAdapter->lastSeen[address] = time;

    QBluetoothLocalDevice dev( "dev" );
    QDateTime actual_lastSeen = dev.lastSeen(QBluetoothAddress(address));

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");

    QCOMPARE( dev.deviceName(), QString("mydev") );
    QCOMPARE( actual_lastSeen, expected_lastSeen );
}

void tst_QBluetoothLocalDevice::lastSeen_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<QString>("time");

    QStringList all;
    all << "01:23:45:67:89:AB";
    all << "45:67:89:Ab:cd:eF";
    all << "98:76:54:32:10:98";
    all << "FE:DC:BA:AB:CD:EF";
    all << "00:00:00:00:00:00";
    all << "FF:FF:FF:FF:FF:FF";
    all << "00:00:00:FF:FF:FF";

    foreach(QString str, all) {
        QTest::newRow(qPrintable(str + " without minutes/seconds"))  << str << "2002-10-01";
        QTest::newRow(qPrintable(str + " with minutes/seconds"))     << str << "2002-10-01 13:02:03";

        QTest::newRow(qPrintable(str + " millenium boundary 1"))     << str << "1999-12-31 23:59:59";
        QTest::newRow(qPrintable(str + " millenium boundary 2"))     << str << "2000-01-01 00:00:00";
        QTest::newRow(qPrintable(str + " millenium boundary 3"))     << str << "2000-01-01 00:00:01";

        // Year 2038 problem, 03:14:07 UTC on Tuesday, January 19, 2038
        QTest::newRow(qPrintable(str + " year 2038 1"))     << str << "2038-01-19 03:14:06";
        QTest::newRow(qPrintable(str + " year 2038 2"))     << str << "2038-01-19 03:14:07";
        QTest::newRow(qPrintable(str + " year 2038 3"))     << str << "2038-01-19 03:14:08";

        QTest::newRow(qPrintable(str + " example from BlueZ docs"))  << str << "2006-02-08 12:00:00 GMT";
    }

}

/*?
    Test the lastUsed() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::lastUsed()
{
    QFETCH(QString, address);
    QFETCH(QString, time);

    QDateTime expected_lastUsed = QDateTime::fromString(time, Qt::ISODate);
    expected_lastUsed.setTimeSpec(Qt::UTC);
    expected_lastUsed = expected_lastUsed.toLocalTime();

    MAKEADAPTER("mydev");
    bluez->adapterMap["dev"] = "mydev";
    bluezAdapter->lastUsed.clear();
    bluezAdapter->lastUsed[address] = time;

    QBluetoothLocalDevice dev( "dev" );
    QDateTime actual_lastUsed = dev.lastUsed(QBluetoothAddress(address));

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");

    QCOMPARE( dev.deviceName(), QString("mydev") );
    QCOMPARE( actual_lastUsed, expected_lastUsed );
}

void tst_QBluetoothLocalDevice::lastUsed_data()
{
    lastSeen_data();
}

/*?
    Test the pairedDevices() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::pairedDevices()
{
    QFETCH(QStringList, bondings);

    MAKEADAPTER("mydev");
    bluez->adapterMap["dev"] = "mydev";
    bluezAdapter->bondings = bondings;

    QBluetoothLocalDevice dev( "dev" );
    QList<QBluetoothAddress> actual_bondings = dev.pairedDevices();

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");

    QCOMPARE( dev.deviceName(), QString("mydev") );

    foreach(QBluetoothAddress addr, actual_bondings) {
         QVERIFY( bondings.contains(addr.toString(), Qt::CaseInsensitive) );
    }
}

void tst_QBluetoothLocalDevice::pairedDevices_data()
{
    addressList_data("bondings");
}

/*?
    Test the isPaired() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::isPaired()
{
    QFETCH(QString, address);
    QFETCH(bool, hasBonding);

    MAKEADAPTER("mydev");
    bluez->adapterMap["dev"] = "mydev";
    bluezAdapter->hasBonding.clear();
    bluezAdapter->hasBonding[address] = hasBonding;

    QBluetoothLocalDevice dev( "dev" );
    bool actual_hasBonding = dev.isPaired(QBluetoothAddress(address));

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");

    QCOMPARE( dev.deviceName(), QString("mydev") );
    QCOMPARE( actual_hasBonding, hasBonding );
}

void tst_QBluetoothLocalDevice::isPaired_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<bool>("hasBonding");

    QStringList all;
    all << "01:23:45:67:89:AB";
    all << "45:67:89:Ab:cd:eF";
    all << "98:76:54:32:10:98";
    all << "FE:DC:BA:AB:CD:EF";
    all << "00:00:00:00:00:00";
    all << "FF:FF:FF:FF:FF:FF";
    all << "00:00:00:FF:FF:FF";

    foreach(QString str, all) {
        QTest::newRow(qPrintable(str + " true 1"))  << str << true;
        QTest::newRow(qPrintable(str + " false 1")) << str << false;
        QTest::newRow(qPrintable(str + " true 2"))  << str << true;
        QTest::newRow(qPrintable(str + " false 2")) << str << false;
    }
}

/*?
    Test that the updateRemoteDevice() function correctly updates all
    attributes of the given device according to the data returned by
    BlueZ.
*/
void tst_QBluetoothLocalDevice::updateRemoteDevice()
{
    QFETCH(QString, address);
    QFETCH(QString, version);
    QFETCH(QString, revision);
    QFETCH(QString, manufacturer);
    QFETCH(QString, company);
    QFETCH(QString, name);
    QFETCH(uint, klass);

    QBluetoothAddress baddr(address);
    QBluetoothRemoteDevice remote(baddr);

    MAKEADAPTER("mydev");
    bluez->adapterMap["dev"] = "mydev";
    bluezAdapter->remoteVersion.clear();
    bluezAdapter->remoteVersion[address] = version;
    bluezAdapter->remoteRevision.clear();
    bluezAdapter->remoteRevision[address] = revision;
    bluezAdapter->remoteManufacturer.clear();
    bluezAdapter->remoteManufacturer[address] = manufacturer;
    bluezAdapter->remoteCompany.clear();
    bluezAdapter->remoteCompany[address] = company;
    bluezAdapter->remoteName.clear();
    bluezAdapter->remoteName[address] = name;
    bluezAdapter->remoteClass.clear();
    bluezAdapter->remoteClass[address] = klass;

    QBluetoothLocalDevice dev( "dev" );
    bool ret = dev.updateRemoteDevice(remote);

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");

    QCOMPARE( dev.deviceName(), QString("mydev") );
    QVERIFY( ret );
    QCOMPARE( remote.version(), version );
    QCOMPARE( remote.revision(), revision );
    QCOMPARE( remote.manufacturer(), manufacturer );
    QCOMPARE( remote.company(), company );
    QCOMPARE( remote.name(), name );

    quint8 major = (klass >> 8) & 0x1F;
    quint8 minor = (klass >> 2) & 0x3F;
    quint8 service = (klass >> 16) & 0xFF;

    QCOMPARE( remote.deviceMinor(), minor );
    QCOMPARE( remote.deviceMajor(), major_to_device_major(major) );
    QCOMPARE( (int)remote.serviceClasses(), (int)service );
}

void tst_QBluetoothLocalDevice::updateRemoteDevice_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<QString>("version");
    QTest::addColumn<QString>("revision");
    QTest::addColumn<QString>("manufacturer");
    QTest::addColumn<QString>("company");
    QTest::addColumn<QString>("name");
    QTest::addColumn<uint>("klass");

    QStringList all;
    all << "01:23:45:67:89:AB";
    all << "45:67:89:Ab:cd:eF";
    all << "98:76:54:32:10:98";
    all << "FE:DC:BA:AB:CD:EF";
    all << "00:00:00:00:00:00";
    all << "FF:FF:FF:FF:FF:FF";
    all << "00:00:00:FF:FF:FF";

    foreach(QString str, all) {
        int i = 0;
        QTest::newRow(qPrintable(QString(str + " %1").arg(++i)))
            << str
            << "Bluetooth 2.0 + EDR"
            << "HCI 19.2"
            << "Yoyodyne Mobile Phones"
            << "Billy Corporation"
            << "00:11:22:33:44:55"
            << 0xABCDEF01u;

        QTest::newRow(qPrintable(QString(str + " %1").arg(++i)))
            << str
            << "an odd version"
            << "vendor specific string"
            << "48KL~2l_+VX?"
            << "'';[[]*&)_(!@#$%^~`<<,.>,/"
            << "Frank"
            << 0xFFFFFFFFu;

        QTest::newRow(qPrintable(QString(str + " %1").arg(++i)))
            << str
            << QString()
            << QString()
            << QString()
            << QString()
            << QString()
            << 0u;
    }
}

/*?
    Test the knownDevices() BlueZ wrapper function.
*/
void tst_QBluetoothLocalDevice::knownDevices_qdatetime()
{
    QFETCH(QString, timeStr);
    QFETCH(QStringList, devices);

    MAKEADAPTER("mydev");
    bluez->adapterMap["dev"] = "mydev";
    bluezAdapter->recentRemoteDevices[timeStr] = devices;

    QDateTime t = QDateTime::fromString(timeStr, "yyyy-MM-dd hh:mm:ss");
    t.setTimeSpec(Qt::UTC);

    QBluetoothLocalDevice dev( "dev" );
    QList<QBluetoothAddress> actual_devices = dev.knownDevices();

    DESTROYADAPTER("mydev");
    bluez->adapterMap.remove("dev");

    QCOMPARE( dev.deviceName(), QString("mydev") );

    foreach(QBluetoothAddress addr, actual_devices) {
         QVERIFY( devices.contains(addr.toString(), Qt::CaseInsensitive) );
    }
}

void tst_QBluetoothLocalDevice::knownDevices_qdatetime_data()
{
    QTest::addColumn<QStringList>("devices");
    QTest::addColumn<QString>("timeStr");

    QList<QStringList> allAll;

    {
        QStringList all;

        allAll << all;

        all << "01:23:45:67:89:AB";
        allAll << all;

        all << "45:67:89:Ab:cd:eF";
        allAll << all;

        all << "98:76:54:32:10:98";
        allAll << all;

        all << "FE:DC:BA:AB:CD:EF";
        allAll << all;

        all << "00:00:00:00:00:00";
        allAll << all;

        all << "FF:FF:FF:FF:FF:FF";
        allAll << all;

        all << "00:00:00:FF:FF:FF";
        allAll << all;
    }

    int i = 0;
    foreach(QStringList list, allAll) {
        QTest::newRow(qPrintable(QString("without minutes/seconds %1").arg(i)))  << list << "2002-10-01";
        QTest::newRow(qPrintable(QString("with minutes/seconds %1").arg(i)))     << list << "2002-10-01 13:02:03";

        QTest::newRow(qPrintable(QString("millenium boundary 1 %1").arg(i)))     << list << "1999-12-31 23:59:59";
        QTest::newRow(qPrintable(QString("millenium boundary 2 %1").arg(i)))     << list << "2000-01-01 00:00:00";
        QTest::newRow(qPrintable(QString("millenium boundary 3 %1").arg(i)))     << list << "2000-01-01 00:00:01";

        // Year 2038 problem, 03:14:07 UTC on Tuesday, January 19, 2038
        QTest::newRow(qPrintable(QString("year 2038 1 %1").arg(i)))     << list << "2038-01-19 03:14:06";
        QTest::newRow(qPrintable(QString("year 2038 2 %1").arg(i)))     << list << "2038-01-19 03:14:07";
        QTest::newRow(qPrintable(QString("year 2038 3 %1").arg(i)))     << list << "2038-01-19 03:14:08";

        QTest::newRow(qPrintable(QString("example from BlueZ docs %1").arg(i)))  << list << "2006-02-08 12:00:00 GMT";
        ++i;
    }
}

/*?
    Test the nameChanged() BlueZ wrapper signal.
*/
void tst_QBluetoothLocalDevice::nameChanged()
{
    QFETCH(QString, name);
    QList<QVariant> data;
    data << name;

    TEST_BLUEZ_WRAPPER_SIGNAL(SIGNAL(nameChanged(QString)),data,NameChanged(name));
}

void tst_QBluetoothLocalDevice::nameChanged_data()
{
    strings_data("name");
}

/*?
    Test the stateChanged() BlueZ wrapper signal.
*/
void tst_QBluetoothLocalDevice::stateChanged()
{
    QFETCH(QString, stateStr);
    QFETCH(QBluetoothLocalDevice::State, state);
    QList<QVariant> getData;
    getData << QVariant::fromValue(state);

    TEST_BLUEZ_WRAPPER_SIGNAL(SIGNAL(stateChanged(QBluetoothLocalDevice::State)),getData,ModeChanged(stateStr));
}

void tst_QBluetoothLocalDevice::stateChanged_data()
{
    QTest::addColumn<QString>("stateStr");
    QTest::addColumn<QBluetoothLocalDevice::State>("state");

    QTest::newRow("off") << "off" << QBluetoothLocalDevice::Off;
    QTest::newRow("connectable") << "connectable" << QBluetoothLocalDevice::Connectable;
    QTest::newRow("discoverable") << "discoverable" << QBluetoothLocalDevice::Discoverable;
}

/*?
    Test the discoverableTimeoutChanged() BlueZ wrapper signal.
*/
void tst_QBluetoothLocalDevice::discoverableTimeoutChanged()
{
    QFETCH(uint, discoverableTimeout);
    QList<QVariant> data;
    data << discoverableTimeout;

    TEST_BLUEZ_WRAPPER_SIGNAL(SIGNAL(discoverableTimeoutChanged(uint)),data,DiscoverableTimeoutChanged(discoverableTimeout));
}

void tst_QBluetoothLocalDevice::discoverableTimeoutChanged_data()
{
    discoverableTimeout_data();
}

/*?
    Test the remoteDeviceConnected() BlueZ wrapper signal.
*/
void tst_QBluetoothLocalDevice::remoteDeviceConnected()
{
    QFETCH(QString, addressStr);
    QList<QVariant> sendData, getData;
    sendData << addressStr;
    getData << QVariant::fromValue(QBluetoothAddress(addressStr));

    TEST_BLUEZ_WRAPPER_SIGNAL(SIGNAL(remoteDeviceConnected(QBluetoothAddress)),getData,RemoteDeviceConnected(addressStr));
}

void tst_QBluetoothLocalDevice::remoteDeviceConnected_data()
{
    validAddress_data();
}

/*?
    Test the remoteDeviceDisconnected() BlueZ wrapper signal.
*/
void tst_QBluetoothLocalDevice::remoteDeviceDisconnected()
{
    QFETCH(QString, addressStr);
    QList<QVariant> sendData, getData;
    sendData << addressStr;
    getData << QVariant::fromValue(QBluetoothAddress(addressStr));

    TEST_BLUEZ_WRAPPER_SIGNAL(SIGNAL(remoteDeviceDisconnected(QBluetoothAddress)),getData,RemoteDeviceDisconnected(addressStr));
}

void tst_QBluetoothLocalDevice::remoteDeviceDisconnected_data()
{
    validAddress_data();
}

/*?
    Test the remoteDeviceDisappeared() BlueZ wrapper signal.
*/
void tst_QBluetoothLocalDevice::remoteDeviceDisappeared()
{
    QFETCH(QString, addressStr);
    QList<QVariant> sendData, getData;
    sendData << addressStr;
    getData << QVariant::fromValue(QBluetoothAddress(addressStr));

    TEST_BLUEZ_WRAPPER_SIGNAL(SIGNAL(remoteDeviceDisappeared(QBluetoothAddress)),getData,RemoteDeviceDisappeared(addressStr));
}

void tst_QBluetoothLocalDevice::remoteDeviceDisappeared_data()
{
    validAddress_data();
}

/*?
    Test the discoveryStarted() BlueZ wrapper signal.
*/
void tst_QBluetoothLocalDevice::discoveryStarted()
{
    QList<QVariant> data;

    TEST_BLUEZ_WRAPPER_SIGNAL(SIGNAL(discoveryStarted()),data,DiscoveryStarted());
}

void tst_QBluetoothLocalDevice::discoveryStarted_data()
{
}

/*?
    Test the discoveryCompleted() BlueZ wrapper signal.
*/
void tst_QBluetoothLocalDevice::discoveryCompleted()
{
    QList<QVariant> data;

    TEST_BLUEZ_WRAPPER_SIGNAL(SIGNAL(discoveryCompleted()),data,DiscoveryCompleted());
}

void tst_QBluetoothLocalDevice::discoveryCompleted_data()
{
}

/*?
    Test the remoteAliasChanged() BlueZ wrapper signal.
*/
void tst_QBluetoothLocalDevice::remoteAliasChanged()
{
    QFETCH(QString, address);
    QFETCH(QString, alias);
    QList<QVariant> data;
    QList<QVariant> sendData, getData;
    sendData << address << alias;
    getData << QVariant::fromValue(QBluetoothAddress(address)) << alias;

    TEST_BLUEZ_WRAPPER_SIGNAL(SIGNAL(remoteAliasChanged(QBluetoothAddress,QString)),getData,RemoteAliasChanged(address,alias));
}

void tst_QBluetoothLocalDevice::remoteAliasChanged_data()
{
    setRemoteAlias_data();
}

/*?
    Test the remoteAliasRemoved() BlueZ wrapper signal.
*/
void tst_QBluetoothLocalDevice::remoteAliasRemoved()
{
    QFETCH(QString, addressStr);
    QList<QVariant> sendData, getData;
    sendData << addressStr;
    getData << QVariant::fromValue(QBluetoothAddress(addressStr));

    TEST_BLUEZ_WRAPPER_SIGNAL(SIGNAL(remoteAliasRemoved(QBluetoothAddress)),getData,RemoteAliasCleared(addressStr));
}

void tst_QBluetoothLocalDevice::remoteAliasRemoved_data()
{
    validAddress_data();
}

/*?
    Test the pairingCreated() BlueZ wrapper signal.
*/
void tst_QBluetoothLocalDevice::pairingCreated()
{
    QFETCH(QString, addressStr);
    QList<QVariant> sendData, getData;
    sendData << addressStr;
    getData << QVariant::fromValue(QBluetoothAddress(addressStr));

    TEST_BLUEZ_WRAPPER_SIGNAL(SIGNAL(pairingCreated(QBluetoothAddress)),getData,BondingCreated(addressStr));
}

void tst_QBluetoothLocalDevice::pairingCreated_data()
{
    validAddress_data();
}

/*?
    Test the pairingRemoved() BlueZ wrapper signal.
*/
void tst_QBluetoothLocalDevice::pairingRemoved()
{
    QFETCH(QString, addressStr);
    QList<QVariant> sendData, getData;
    sendData << addressStr;
    getData << QVariant::fromValue(QBluetoothAddress(addressStr));

    TEST_BLUEZ_WRAPPER_SIGNAL(SIGNAL(pairingRemoved(QBluetoothAddress)),getData,BondingRemoved(addressStr));
}

void tst_QBluetoothLocalDevice::pairingRemoved_data()
{
    validAddress_data();
}

/*?
    Test the remoteNameUpdated() BlueZ wrapper signal.
*/
void tst_QBluetoothLocalDevice::remoteNameUpdated()
{
    QFETCH(QString, address);
    QFETCH(QString, alias);
    QList<QVariant> sendData, getData;
    sendData << address << alias;
    getData << QVariant::fromValue(QBluetoothAddress(address)) << alias;

    TEST_BLUEZ_WRAPPER_SIGNAL(SIGNAL(remoteNameUpdated(QBluetoothAddress,QString)),getData,RemoteNameUpdated(address,alias));
}

void tst_QBluetoothLocalDevice::remoteNameUpdated_data()
{
    setRemoteAlias_data();
}

/*?
    Test the remoteNameFailed() BlueZ wrapper signal.
*/
void tst_QBluetoothLocalDevice::remoteNameFailed()
{
    QFETCH(QString, addressStr);
    QList<QVariant> sendData, getData;
    sendData << addressStr;
    getData << QVariant::fromValue(QBluetoothAddress(addressStr));

    TEST_BLUEZ_WRAPPER_SIGNAL(SIGNAL(remoteNameFailed(QBluetoothAddress)),getData,RemoteNameFailed(addressStr));
}

void tst_QBluetoothLocalDevice::remoteNameFailed_data()
{
    validAddress_data();
}

/*?
    Test the remoteClassUpdated() BlueZ wrapper signal.
*/
void tst_QBluetoothLocalDevice::remoteClassUpdated()
{
    QFETCH(QString, address);
    QFETCH(uint, klass);

    quint8 major = (klass >> 8) & 0x1F;
    quint8 minor = (klass >> 2) & 0x3F;
    quint8 service = (klass >> 16) & 0xFF;

    QList<QVariant> sendData, getData;
    sendData << address << klass;
    getData
        << QVariant::fromValue(QBluetoothAddress(address))
        << QVariant::fromValue(major_to_device_major(major))
        << QVariant::fromValue((uchar)minor)
        << QVariant::fromValue(QBluetooth::ServiceClasses(service));

    TEST_BLUEZ_WRAPPER_SIGNAL(SIGNAL(remoteClassUpdated(QBluetoothAddress,QBluetooth::DeviceMajor,quint8,QBluetooth::ServiceClasses)),getData,RemoteClassUpdated(address,klass));
}

void tst_QBluetoothLocalDevice::remoteClassUpdated_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<uint>("klass");

    QStringList all;
    all << "01:23:45:67:89:AB";
    all << "45:67:89:Ab:cd:eF";
    all << "98:76:54:32:10:98";
    all << "FE:DC:BA:AB:CD:EF";
    all << "00:00:00:00:00:00";
    all << "FF:FF:FF:FF:FF:FF";
    all << "00:00:00:FF:FF:FF";

    foreach(QString str, all) {
        QTest::newRow(qPrintable(str + " " + "zero"))  << str << (uint)0u;
        QTest::newRow(qPrintable(str + " " + "one"))   << str << (uint)1u;
        QTest::newRow(qPrintable(str + " " + "two"))   << str << (uint)2u;
        QTest::newRow(qPrintable(str + " " + "three")) << str << (uint)3u;

        QTest::newRow(qPrintable(str + " " + "magnitude 1")) << str << (uint)23u;
        QTest::newRow(qPrintable(str + " " + "magnitude 2")) << str << (uint)790u;
        QTest::newRow(qPrintable(str + " " + "magnitude 3")) << str << (uint)6002u;
        QTest::newRow(qPrintable(str + " " + "magnitude 4")) << str << (uint)23041u;
        QTest::newRow(qPrintable(str + " " + "magnitude 5")) << str << (uint)723041u;
        QTest::newRow(qPrintable(str + " " + "magnitude 6")) << str << (uint)7230410u;
        QTest::newRow(qPrintable(str + " " + "magnitude 7")) << str << (uint)27230410u;
        QTest::newRow(qPrintable(str + " " + "magnitude 8")) << str << (uint)127230410u;
        QTest::newRow(qPrintable(str + " " + "magnitude 9")) << str << (uint)3127230410u;

        QTest::newRow(qPrintable(str + " " + "near signed upper limit 1")) << str << (uint)((1u << 31) - 3u);
        QTest::newRow(qPrintable(str + " " + "near signed upper limit 2")) << str << (uint)((1u << 31) - 2u);

        QTest::newRow(qPrintable(str + " " + "signed upper limit")) << str << (uint)((1u << 31) - 1u);

        QTest::newRow(qPrintable(str + " " + "near upper limit 1")) << str << (uint)(0xFFFFFFFFu - 2u);
        QTest::newRow(qPrintable(str + " " + "near upper limit 2")) << str << (uint)(0xFFFFFFFFu - 1u);

        QTest::newRow(qPrintable(str + " " + "upper limit")) << str << (uint)0xFFFFFFFFu;
    }
}

/*?
    Test the disconnectRemoteDeviceRequested() BlueZ wrapper signal.
*/
void tst_QBluetoothLocalDevice::disconnectRemoteDeviceRequested()
{
    QFETCH(QString, addressStr);
    QList<QVariant> sendData, getData;
    sendData << addressStr;
    getData << QVariant::fromValue(QBluetoothAddress(addressStr));

    TEST_BLUEZ_WRAPPER_SIGNAL(SIGNAL(remoteDeviceDisconnectRequested(QBluetoothAddress)),getData,RemoteDeviceDisconnectRequested(addressStr));
}

void tst_QBluetoothLocalDevice::disconnectRemoteDeviceRequested_data()
{
    validAddress_data();
}

