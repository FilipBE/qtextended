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

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QtopiaApplication>
#include <QtopiaNetwork>
#include <QtopiaNetworkInterface>
#include <QValueSpaceObject>
#include <QtopiaChannel>
#include <QSignalSpy>

#include <shared/qtopiaunittest.h>
#include <shared/util.h>

/*
    Struct containing all information needed to create a network
    configuration file.
*/
struct ConfFileInfo
{
    QString filename;
    QString type;
    QString name;
    QString serialType;
    QString serialDevice;
    QString serialGprs;
    QString deviceType;
    QString bluetoothProfile;
    QString visibility;
    uint expectedType;
};

// The number of different types of network configurations.
static const int NUMDIALUP = 3;
static const int NUMPCMCIA = 6;
static const int NUMLAN = 4;
#ifdef QTOPIA_BLUETOOTH
static const int NUMBLUETOOTH = 3;
#else
static const int NUMBLUETOOTH = 0;
#endif

/*
    Below are many different network configurations.
    These were chosen to ensure every possible code path
    in QtopiaNetwork::toType is executed at least once.
*/
static const ConfFileInfo TEST_DATA[] = {
    // Dialup1 - 
    {
        "dialup1.conf",
        "dialup",
        "Dialup1",
        "external",
        "",
        "",
        "",
        "",
        "",
        QtopiaNetwork::PCMCIA | QtopiaNetwork::Dialup
    },

    // Dialup2
    {
        "dialup2.conf",
        "dialup",
        "Dialup2",
        "external",
        "some_modem_name",
        "",
        "",
        "",
        "",
        QtopiaNetwork::NamedModem | QtopiaNetwork::Dialup
    },

    // Dialup3
    {
        "dialup3.conf",
        "dialup",
        "Dialup3",
        "",
        "",
        "",
        "",
        "",
        "",
#ifdef QTOPIA_CELL
        QtopiaNetwork::PhoneModem | QtopiaNetwork::Dialup
#else
        QtopiaNetwork::Dialup
#endif
    },

    // gprs1
    {
        "gprs1.conf",
        "dialup",
        "gprs1",
        "external",
        "",
        "y",
        "",
        "",
        "",
        QtopiaNetwork::PCMCIA | QtopiaNetwork::GPRS
    },

    // gprs2
    {
        "gprs2.conf",
        "dialup",
        "gprs2",
        "external",
        "some_modem_name",
        "y",
        "",
        "",
        "",
        QtopiaNetwork::NamedModem | QtopiaNetwork::GPRS
    },

    // gprs3
    {
        "gprs3.conf",
        "dialup",
        "gprs3",
        "",
        "",
        "y",
        "",
        "",
        "",
#ifdef QTOPIA_CELL
        QtopiaNetwork::PhoneModem | QtopiaNetwork::GPRS
#else
        QtopiaNetwork::GPRS
#endif
    },

    // gprs4
    {
        "gprs4.conf",
        "dialup",
        "gprs4",
        "external",
        "some_modem_name",
        "y",
        "",
        "",
        "hidden",
        QtopiaNetwork::NamedModem | QtopiaNetwork::GPRS
        | QtopiaNetwork::Hidden
    },

    // Pcmcialan1
    {
        "pcmcialan1.conf",
        "pcmcialan",
        "Pcmcialan1",
        "",
        "",
        "",
        "",
        "",
        "",
        QtopiaNetwork::PCMCIA | QtopiaNetwork::LAN
    },

    // Pcmcialan2
    {
        "pcmcialan2.conf",
        "pcmciawlan",
        "Pcmcialan2",
        "",
        "",
        "",
        "wlan",
        "",
        "",
        QtopiaNetwork::PCMCIA | QtopiaNetwork::WirelessLAN
    },

    // Pcmcialan3
    {
        "pcmcialan3.conf",
        "pcmcialan",
        "Pcmcialan3",
        "",
        "",
        "",
        "eth",
        "",
        "",
        QtopiaNetwork::PCMCIA | QtopiaNetwork::LAN
    },

    // Pcmcialan4
    {
        "pcmcialan4.conf",
        "pcmcialan",
        "Pcmcialan4",
        "",
        "",
        "",
        "eth",
        "",
        "hidden",
        QtopiaNetwork::PCMCIA | QtopiaNetwork::LAN | QtopiaNetwork::Hidden
    },

    // Lan1
    {
        "lan1.conf",
        "lan",
        "Lan1",
        "",
        "",
        "",
        "",
        "",
        "",
        QtopiaNetwork::LAN
    },

    // Lan2
    {
        "lan2.conf",
        "lan",
        "Lan2",
        "",
        "",
        "",
        "wlan",
        "",
        "",
        QtopiaNetwork::LAN
    },

    // Lan3
    {
        "lan3.conf",
        "lan",
        "Lan3",
        "",
        "",
        "",
        "eth",
        "",
        "",
        QtopiaNetwork::LAN
    },

    // Lan4
    {
        "lan4.conf",
        "lan",
        "Lan4",
        "",
        "",
        "",
        "eth",
        "",
        "hidden",
        QtopiaNetwork::LAN | QtopiaNetwork::Hidden
#ifndef QTOPIA_BLUETOOTH
    }
#else
    },

    // Bluetooth1
    {
        "bluetooth1.conf",
        "bluetooth",
        "Bluetooth1",
        "",
        "",
        "",
        "eth",
        "",
        "",
        QtopiaNetwork::Bluetooth
    },

    // Bluetooth2
    {
        "bluetooth2.conf",
        "bluetooth",
        "Bluetooth2",
        "",
        "",
        "",
        "",
        "DUN",
        "",
        QtopiaNetwork::Bluetooth | QtopiaNetwork::BluetoothDUN
    },

    // Bluetooth3
    {
        "bluetooth3.conf",
        "bluetooth",
        "Bluetooth3",
        "",
        "",
        "",
        "",
        "DUN",
        "hidden",
        QtopiaNetwork::Bluetooth | QtopiaNetwork::BluetoothDUN
         | QtopiaNetwork::Hidden
    }
#endif
};



//TESTED_CLASS=QtopiaNetwork
//TESTED_FILES=src/libraries/qtopia/qtopianetwork.h

/*
    The tst_QtopiaNetwork class provides unit tests for the QtopiaNetwork class.
*/
class tst_QtopiaNetwork: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void cleanup();

    void loadPlugin();
    void loadPlugin_data();

    void availableNetworkConfigs();

    void settingsDir();

    void toType();
    void toType_data();

    void online();

    void ipcMessages();

private:
    void createTestFiles(const QString &dest);
    void deleteTestFiles(const QString &dest);

    QString testDataPath;
};

QTEST_APP_MAIN(tst_QtopiaNetwork, QtopiaApplication)
#include "tst_qtopianetwork.moc"

/*?
    Test that IPC messages are sent at the correct times.
    This test ensures that Qt Extended IPC messages are sent over the QPE/Network
    channel with the correct signature and parameters when the following
    QtopiaNetwork function calls are made:
        * startInterface()
        * stopInterface()
        * privilegedInterfaceStop()
        * shutdown()
        * setDefaultGateway()
        * unsetDefaultGateway()
        * lockdown()
        * extendInterfaceLifetime()
*/
void tst_QtopiaNetwork::ipcMessages()
{
    QtopiaChannel ch("QPE/Network");
    QSignalSpy spy(&ch, SIGNAL(received(QString,QByteArray)));

    QtopiaNetwork::startInterface("foo", "bar");
    QTRY_COMPARE( spy.count(), 1 );
    QCOMPARE( qPrintable(spy.takeFirst().at(0).toString()), "startInterface(QString,QString,QVariant)" );

    QtopiaNetwork::stopInterface("baz", true);
    QTRY_COMPARE( spy.count(), 1 );
    QCOMPARE( qPrintable(spy.takeFirst().at(0).toString()), "stopInterface(QString,QString,bool)" );

    QtopiaNetwork::privilegedInterfaceStop("ahoy");
    QTRY_COMPARE( spy.count(), 1 );
    QCOMPARE( qPrintable(spy.takeFirst().at(0).toString()), "privilegedInterfaceStop(QString)" );

    QtopiaNetwork::shutdown();
    QTRY_COMPARE( spy.count(), 1 );
    QCOMPARE( qPrintable(spy.takeFirst().at(0).toString()), "shutdownNetwork()" );

    QtopiaNetwork::setDefaultGateway("frank");
    QTRY_COMPARE( spy.count(), 1 );
    QCOMPARE( qPrintable(spy.takeFirst().at(0).toString()), "setDefaultGateway(QString,bool)" );

    QtopiaNetwork::unsetDefaultGateway("jill");
    QTRY_COMPARE( spy.count(), 1 );
    QCOMPARE( qPrintable(spy.takeFirst().at(0).toString()), "setDefaultGateway(QString,bool)" );

    QtopiaNetwork::lockdown(true);
    QTRY_COMPARE( spy.count(), 1 );
    QCOMPARE( qPrintable(spy.takeFirst().at(0).toString()), "setLockMode(bool)" );

    QtopiaNetwork::lockdown(false);
    QTRY_COMPARE( spy.count(), 1 );
    QCOMPARE( qPrintable(spy.takeFirst().at(0).toString()), "setLockMode(bool)" );

    QtopiaNetwork::extendInterfaceLifetime("my_iface", true);
    QTRY_COMPARE( spy.count(), 1 );
    QCOMPARE( qPrintable(spy.takeFirst().at(0).toString()), "setExtendedInterfaceLifetime(QString,bool)" );
}

/*?
    Test function for the QtopiaNetwork::online() function.
    This test:
        * Verifies that QtopiaNetwork::online() initially returns false.
        * Verifies that QtopiaNetwork::online() returns the correct value
          when an interface exists with each possible QtopiaNetworkInterface::Status.
*/
void tst_QtopiaNetwork::online()
{
    QVERIFY( !QtopiaNetwork::online() );

    /* If 'bad' is 'down', whether or not we are online solely depends on other interfaces. */
    QValueSpaceObject *bad = new QValueSpaceObject( "/Network/Interfaces/my_bad_iface" );
    bad->setAttribute("State", QtopiaNetworkInterface::Down);

    QValueSpaceObject netSpace( "/Network/Interfaces/my_nice_iface" );

    netSpace.setAttribute("State", QtopiaNetworkInterface::Unknown);
    QVERIFY( !QtopiaNetwork::online() );

    netSpace.setAttribute("State", QtopiaNetworkInterface::Down);
    QVERIFY( !QtopiaNetwork::online() );

    netSpace.setAttribute("State", QtopiaNetworkInterface::Up);
    QVERIFY( QtopiaNetwork::online() );

    netSpace.setAttribute("State", QtopiaNetworkInterface::Pending);
    QVERIFY( QtopiaNetwork::online() );

    netSpace.setAttribute("State", QtopiaNetworkInterface::Demand);
    QVERIFY( QtopiaNetwork::online() );

    netSpace.setAttribute("State", QtopiaNetworkInterface::Unavailable);
    QVERIFY( !QtopiaNetwork::online() );

    /* If 'bad' is 'up', we should always be online regardless of other interfaces. */
    delete bad;
    bad = new QValueSpaceObject( "/Network/Interfaces/my_bad_iface2" );
    bad->setAttribute("State", QtopiaNetworkInterface::Up);

    netSpace.setAttribute("State", QtopiaNetworkInterface::Unknown);
    QVERIFY( QtopiaNetwork::online() );

    netSpace.setAttribute("State", QtopiaNetworkInterface::Down);
    QVERIFY( QtopiaNetwork::online() );

    netSpace.setAttribute("State", QtopiaNetworkInterface::Up);
    QVERIFY( QtopiaNetwork::online() );

    netSpace.setAttribute("State", QtopiaNetworkInterface::Pending);
    QVERIFY( QtopiaNetwork::online() );

    netSpace.setAttribute("State", QtopiaNetworkInterface::Demand);
    QVERIFY( QtopiaNetwork::online() );

    netSpace.setAttribute("State", QtopiaNetworkInterface::Unavailable);
    QVERIFY( QtopiaNetwork::online() );

    delete bad;
}

/*?
    Test function for QtopiaNetwork::toType.
    This test:
        * Calls QtopiaNetwork::toType for each network configuration in
          \a TEST_DATA and verifies that it returns the expected network
          type, which is also stored in \a TEST_DATA.
*/
void tst_QtopiaNetwork::toType()
{
    QFETCH( QString, filename );
    QFETCH( uint, expectedType );
    QCOMPARE( (uint)QtopiaNetwork::toType( filename ), expectedType );
}

/*?
    Data for the toType() test function.
    This function loads all data from the \a TEST_DATA array previously
    declared.
*/
void tst_QtopiaNetwork::toType_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<uint>("expectedType");

    for (int i = 0; i < (int)(sizeof(TEST_DATA) / sizeof(ConfFileInfo)); ++i){
        QTest::newRow(QString("test data %1").arg(i).toLatin1())
            << testDataPath + TEST_DATA[i].filename
            << TEST_DATA[i].expectedType;
    }
}

/*?
    Cleanup after each test.
    Calls QValueSpaceObject::sync() so network options and changes to network
    interfaces take effect.
*/
void tst_QtopiaNetwork::cleanup()
{
    QValueSpaceObject::sync();
}

/*?
    Helper function; creates all test files described in the \a TEST_DATA array.
*/
void tst_QtopiaNetwork::createTestFiles(const QString &dest)
{
    QDir d;
    d.mkpath(dest);
    if ( QDir::setCurrent(dest) ) {
        for (uint i = 0; i < (sizeof(TEST_DATA) / sizeof(ConfFileInfo)); ++i) {
            QFile confFile(TEST_DATA[i].filename);
            if ( !confFile.open(QIODevice::WriteOnly | QIODevice::Text) )
                continue;
            QTextStream out(&confFile);
            out << "[Info]\nType = " << TEST_DATA[i].type  << "\n"
                << "Name[] = " << TEST_DATA[i].name << "\n";
                if (!TEST_DATA[i].visibility.isEmpty())
                out << "Visibility = " << TEST_DATA[i].visibility << "\n";
                out << "[Serial]\n";
                if (!TEST_DATA[i].serialType.isEmpty())
                out << "Type = " << TEST_DATA[i].serialType << "\n";
                if (!TEST_DATA[i].serialDevice.isEmpty())
                out << "SerialDevice = " << TEST_DATA[i].serialDevice
                    << "\n";
                if (!TEST_DATA[i].serialGprs.isEmpty())
                out << "GPRS = " << TEST_DATA[i].serialGprs << "\n";
                out << "[Properties]\n";
                if (!TEST_DATA[i].deviceType.isEmpty())
                out << "DeviceType = " << TEST_DATA[i].deviceType << "\n";
                out << "[Bluetooth]\n";
                if (!TEST_DATA[i].bluetoothProfile.isEmpty())
                out << "Profile = " << TEST_DATA[i].bluetoothProfile
                    << "\n";
            confFile.close();
        }
    }
}

/*?
    Helper function; deletes all test files described in the \a TEST_DATA array.
*/
void tst_QtopiaNetwork::deleteTestFiles(const QString &dest)
{
    // Remove the created files/directories
    if ( QDir::setCurrent(dest) ) {
        for (uint i = 0; i < (sizeof(TEST_DATA) / sizeof(ConfFileInfo)); ++i)
            QFile::remove(TEST_DATA[i].filename);
    }
}

/*?
    Initialisation before all tests.
    Creates network configuration files and initialises the value space manager.
*/
void tst_QtopiaNetwork::initTestCase()
{
    // Create the test files
    testDataPath = QDir::homePath() + "/autotestTmpData/";
    createTestFiles(testDataPath);
    QValueSpace::initValuespaceManager();
}

/*?
    Cleanup after all tests.
    Deletes all previously created network configuration files.
*/
void tst_QtopiaNetwork::cleanupTestCase()
{
    // Remove the created files/directories
    deleteTestFiles(QtopiaNetwork::settingsDir());
    deleteTestFiles(testDataPath);
    QDir d;
    d.rmdir(testDataPath);
}

/*?
    Data for the loadPlugin() test function.
*/
void tst_QtopiaNetwork::loadPlugin_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<bool>("exists");
    QTest::addColumn<uint>("expectedType");

    QTest::newRow("nonexistent empty")
        << ""
        << false
        << (uint)0;

    QTest::newRow("nonexistent null")
        << QString()
        << false
        << (uint)0;

    QTest::newRow("nonexistent fake, absolute")
        << "/some/nonexistent/file"
        << false
        << (uint)0;

    QTest::newRow("nonexistent fake, relative")
        << "another/nonexistent/file"
        << false
        << (uint)0;

    for (int i = 0; i < (int)(sizeof(TEST_DATA) / sizeof(ConfFileInfo)); ++i) {
        QTest::newRow(QString("test data %1").arg(i).toLatin1())
            << testDataPath + TEST_DATA[i].filename
            << true
            << TEST_DATA[i].expectedType;
    }
}

/*?
    Test function for QtopiaNetwork::loadPlugin.
    This test:
        * Attempts to load a plugin using a specified network configuration file.
        * Verifies that a plugin either was or was not loaded according to expected
          test results.
        * If a plugin was loaded, compares the plugin's type with the expected type.
*/
void tst_QtopiaNetwork::loadPlugin()
{
    QPointer<QtopiaNetworkInterface> iface1 = 0;
    QPointer<QtopiaNetworkInterface> iface2 = 0;

    QFETCH(QString, filename);
    QFETCH(bool, exists);
    QFETCH(uint, expectedType);

    QVERIFY( !exists || QFile::exists(filename) );

    iface1 = QtopiaNetwork::loadPlugin(filename);
    // If we don't expect it to exist...
    if (!exists || (expectedType & QtopiaNetwork::Hidden)) {
        QCOMPARE(iface1, QPointer<QtopiaNetworkInterface>(0));
        return;
    }
    else {
        QVERIFY(iface1 != 0);
    }

    iface2 = QtopiaNetwork::loadPlugin(filename);
    // should be the same
    QCOMPARE(iface1, iface2);

    QCOMPARE((uint)iface1->type(), expectedType);
}

/*?
    Test function for QtopiaNetwork::availableNetworkConfigs.
    This test:
        * Verifies that all network configurations in \a TEST_DATA have been
          created.
        * Verifies that the expected amount of network configurations of each type
          are returned by QtopiaNetwork::availableNetworkConfigs using an isolated
          testDataPath.
        * Deletes all test files and verifies that there are no available network
          configurations.
        * Creates \a TEST_DATA network configuration files in the default Qt Extended settings directory.
        * Verifies that the expected amount of network configurations are returned
          by QtopiaNetwork::availableNetworkConfigs.
        * Destroys all test files and recreates them in an isolated testDataPath
          for later test functions.
*/
void tst_QtopiaNetwork::availableNetworkConfigs()
{
    // Check that all the test files exist
    for (int i = 0; i < (int)(sizeof(TEST_DATA) / sizeof(ConfFileInfo)); ++i)
        QVERIFY( QFile::exists(testDataPath + TEST_DATA[i].filename) );

    QStringList list;

    // Query for PCMCIA
    list = QtopiaNetwork::availableNetworkConfigs(QtopiaNetwork::PCMCIA, testDataPath);
    QCOMPARE( list.count(), NUMPCMCIA );
    foreach (QString name, list) {
        name = testDataPath + name;
        for (int i = 0; i <(int)(sizeof(TEST_DATA)/sizeof(ConfFileInfo)); ++i){
            if (name == TEST_DATA[i].filename) {
                QVERIFY( TEST_DATA[i].expectedType & QtopiaNetwork::PCMCIA );
            }
        }
    }

    // Query for Dialup
    list = QtopiaNetwork::availableNetworkConfigs(QtopiaNetwork::Dialup, testDataPath);
    QCOMPARE( list.count(), NUMDIALUP );
    foreach (QString name, list) {
        name = testDataPath + name;
        for (int i = 0; i <(int)(sizeof(TEST_DATA)/sizeof(ConfFileInfo)); ++i){
            if (name == TEST_DATA[i].filename) {
                QVERIFY( TEST_DATA[i].expectedType & QtopiaNetwork::Dialup );
            }
        }
    }

    // Query for Bluetooth
    list = QtopiaNetwork::availableNetworkConfigs(QtopiaNetwork::Bluetooth, testDataPath);
    QCOMPARE( list.count(), NUMBLUETOOTH );
    foreach (QString name, list) {
        name = testDataPath + name;
        for (int i = 0; i <(int)(sizeof(TEST_DATA)/sizeof(ConfFileInfo)); ++i){
            if (name == TEST_DATA[i].filename) {
                QVERIFY( TEST_DATA[i].expectedType & QtopiaNetwork::Bluetooth );
            }
        }
    }

    // Query for all
    list = QtopiaNetwork::availableNetworkConfigs(QtopiaNetwork::Any, testDataPath);
    QCOMPARE( list.count(), (int)(sizeof(TEST_DATA)/sizeof(ConfFileInfo)) );

    // Kill the config files and check that no results are now returned
    deleteTestFiles(testDataPath);
    list = QtopiaNetwork::availableNetworkConfigs(QtopiaNetwork::Any, testDataPath);
    QCOMPARE( list.count(), 0 );

    // Default should be all config files in the settings dir which should be currently empty
    list = QtopiaNetwork::availableNetworkConfigs();
    QCOMPARE( list.count(), 0 );

    // Now create the config files in the settings dir
    createTestFiles(QtopiaNetwork::settingsDir());
    list = QtopiaNetwork::availableNetworkConfigs();
    QCOMPARE( list.count(), (int)(sizeof(TEST_DATA)/sizeof(ConfFileInfo)) );
    deleteTestFiles(QtopiaNetwork::settingsDir());

    createTestFiles(testDataPath);
}

/*?
    Test function for QtopiaNetwork::settingsDir() function.
    Verifies that QtopiaNetwork::settingsDir() returns a
    path of the form "<prefix>/Applications/Network/config".
*/
void tst_QtopiaNetwork::settingsDir()
{
    QString r = "/Applications/Network/config";
    QString e = QtopiaNetwork::settingsDir();
    QVERIFY(e.endsWith(r));
}
