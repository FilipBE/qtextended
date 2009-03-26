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

#ifndef QTOPIABLUETOOTHUNITTEST_H
#define QTOPIABLUETOOTHUNITTEST_H

#include <qtopialog.h>
#include <QDBusAbstractAdaptor>
#include <QDBusInterface>
#include <QStringList>
#include <QMap>
#include <QProcess>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <sys/types.h>
#include <signal.h>

#define STUBLOG(A) {\
    if (log) qDebug(A);\
    slotSpy = A;\
}

/*?
    Reimplementation of a subset of the BlueZ manager DBus interface.
    Use the public variables in this class to test that functions are
    being called correctly.
*/
class BluezStub : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Manager")

public:
    BluezStub(QObject *parent=0)
     : QDBusAbstractAdaptor(parent), log(false) {}

    QStringList adapters;
    QString defaultAdapter;
    QMap<QString,QString> adapterMap;

    inline void emitAdapterAdded(QString const& s)          { AdapterAdded(s); }
    inline void emitAdapterRemoved(QString const& s)        { AdapterRemoved(s); }
    inline void emitDefaultAdapterChanged(QString const& s) { DefaultAdapterChanged(s); }

    /*? Set to the signature of the last called function. */
    QString slotSpy;

    /*? If set to true, outputs a line every time a function in this class
        is called (over D-Bus or otherwise). */
    bool log;

public slots:
    inline QStringList ListAdapters() {
        STUBLOG("BluezStub::ListAdapters()");
        QStringList bluezAdapters;
        foreach(QString str, adapters) {
            bluezAdapters << "/org/bluez/" + str;
        }
        return bluezAdapters;
    }

    inline QString DefaultAdapter() {
        STUBLOG("BluezStub::DefaultAdapter()");
        return "/org/bluez/" + defaultAdapter;
    }

    inline QString FindAdapter(QString const& dev) {
        STUBLOG(qPrintable("BluezStub::FindAdapter( \"" + dev + "\" )"));
        return "/org/bluez/" + adapterMap[dev];
    }

signals:
    void AdapterAdded(QString const& device);
    void AdapterRemoved(QString const& device);
    void DefaultAdapterChanged(QString const& device);

    friend class QtopiaBluetoothUnitTest;
};

/*?
    Reimplementation of a subset of the BlueZ adapter DBus interface.
    Use the public variables in this class to test that functions are
    being called correctly.
*/
class BluezAdapterStub : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Adapter")

public:
    BluezAdapterStub(QObject *parent=0)
     : QDBusAbstractAdaptor(parent), log(false) {}
    QString address;
    QString manufacturer;
    QString version;
    QString revision;
    QString company;
    QString name;
    QString mode;
    QStringList connections;
    QStringList remoteDevices;
    QStringList bondings;
    uint discoverableTimeout;
    bool discoverable;
    bool connectable;
    bool periodicDiscovery;
    QMap<QString, bool> connected;
    QMap<QString, bool> hasBonding;
    QMap<QString, QString> remoteAlias;
    QMap<QString, QStringList> recentRemoteDevices;
    QMap<QString, uchar> pincodeLength;
    QMap<QString, QString> lastSeen;
    QMap<QString, QString> lastUsed;
    QMap<QString, QString> remoteVersion;
    QMap<QString, QString> remoteRevision;
    QMap<QString, QString> remoteManufacturer;
    QMap<QString, QString> remoteCompany;
    QMap<QString, QString> remoteName;
    QMap<QString, uint> remoteClass;

    /*? Set to the signature of the last called function. */
    QString slotSpy;

    /*? If set to true, outputs a line every time a function in this class
        is called (over D-Bus or otherwise). */
    bool log;

public slots:
    inline QString GetAddress() {
        STUBLOG("BluezAdapterStub::GetAddress()");
        return address;
    }

    inline QString GetManufacturer() {
        STUBLOG("BluezAdapterStub::GetManufacturer()");
        return manufacturer;
    }

    inline QString GetVersion() {
        STUBLOG("BluezAdapterStub::GetVersion()");
        return version;
    }

    inline QString GetRevision() {
        STUBLOG("BluezAdapterStub::GetRevision()");
        return revision;
    }

    inline QString GetCompany() {
        STUBLOG("BluezAdapterStub::GetCompany()");
        return company;
    }

    inline QString GetName() {
        STUBLOG("BluezAdapterStub::GetName()");
        return name;
    }

    inline bool IsDiscoverable() {
        STUBLOG("BluezAdapterStub::IsDiscoverable()");
        return discoverable;
    }

    inline bool IsConnectable() {
        STUBLOG("BluezAdapterStub::IsConnectable()");
        return connectable;
    }

    inline bool IsPeriodicDiscovery() {
        STUBLOG("BluezAdapterStub::IsPeriodicDiscovery()");
        return periodicDiscovery;
    }

    inline bool IsConnected(QString const &addr) {
        STUBLOG(qPrintable("BluezAdapterStub::IsConnected( \"" + addr + "\" )"));
        return connected[addr];
    }

    inline uchar GetPinCodeLength(QString const &addr) {
        STUBLOG(qPrintable("BluezAdapterStub::GetPinCodeLength( \"" + addr + "\" )"));
        return pincodeLength[addr];
    }

    inline QString LastSeen(QString const &addr) {
        STUBLOG(qPrintable("BluezAdapterStub::LastSeen( \"" + addr + "\" )"));
        return lastSeen[addr];
    }

    inline QString LastUsed(QString const &addr) {
        STUBLOG(qPrintable("BluezAdapterStub::LastUsed( \"" + addr + "\" )"));
        return lastUsed[addr];
    }


    inline QStringList ListRecentRemoteDevices(QString const &d) {
        STUBLOG(qPrintable("BluezAdapterStub::ListRecentRemoteDevices( \"" + d + "\" )"));
        return recentRemoteDevices[d];
    }


    inline QStringList ListConnections() {
        STUBLOG("BluezAdapterStub::ListConnections()");
        return connections;
    }

    inline QStringList ListBondings() {
        STUBLOG("BluezAdapterStub::ListBondings()");
        return bondings;
    }

    inline QStringList ListRemoteDevices() {
        STUBLOG("BluezAdapterStub::ListRemoteDevices()");
        return remoteDevices;
    }

    inline QString GetRemoteAlias(QString const &add) {
        STUBLOG(qPrintable("BluezAdapterStub::GetRemoteAlias( \"" + add + "\" )"));
        return remoteAlias[add];
    }

    inline bool HasBonding(QString const &add) {
        STUBLOG(qPrintable("BluezAdapterStub::HasBonding( \"" + add + "\" )"));
        return hasBonding[add];
    }

    inline uint GetDiscoverableTimeout() {
        STUBLOG("BluezAdapterStub::GetDiscoverableTimeout()");
        return discoverableTimeout;
    }

    inline void SetName(QString const &n) {
        STUBLOG(qPrintable("BluezAdapterStub::SetName( \"" + n + "\" )"));
        name = n;
    }

    inline void SetMode(QString const &m) {
        STUBLOG(qPrintable("BluezAdapterStub::SetMode( \"" + m + "\" )"));
        mode = m;
    }

    inline void SetRemoteAlias(QString const &add, QString const &al) {
        STUBLOG(qPrintable("BluezAdapterStub::SetRemoteAlias( \"" + add + "\", \"" + al + "\" )"));
        remoteAlias[add] = al;
    }

    inline void SetDiscoverableTimeout(uint t) {
        STUBLOG(qPrintable(QString("BluezAdapterStub::SetDiscoverableTimeout( %1 )").arg(t)));
        discoverableTimeout = t;
    }

    inline void DiscoverDevices() {
        STUBLOG("BluezAdapterStub::DiscoverDevices()");
    }

    inline void StartPeriodicDiscovery() {
        STUBLOG("BluezAdapterStub::StartPeriodicDiscovery()");
    }

    inline void StopPeriodicDiscovery() {
        STUBLOG("BluezAdapterStub::StopPeriodicDiscovery()");
    }

    inline void ClearRemoteAlias(QString const &a) {
        STUBLOG(qPrintable("BluezAdapterStub::ClearRemoteAlias( \"" + a + "\" )"));
    }

    inline void DisconnectRemoteDevice(QString const &a) {
        STUBLOG(qPrintable("BluezAdapterStub::DisconnectRemoteDevice( \"" + a + "\" )"));
    }

    inline QString GetRemoteVersion(QString const &a) {
        STUBLOG(qPrintable("BluezAdapterStub::GetRemoteVersion( \"" + a + "\" )"));
        return remoteVersion[a];
    }

    inline QString GetRemoteRevision(QString const &a) {
        STUBLOG(qPrintable("BluezAdapterStub::GetRemoteRevision( \"" + a + "\" )"));
        return remoteRevision[a];
    }

    inline QString GetRemoteManufacturer(QString const &a) {
        STUBLOG(qPrintable("BluezAdapterStub::GetRemoteManufacturer( \"" + a + "\" )"));
        return remoteManufacturer[a];
    }

    inline QString GetRemoteCompany(QString const &a) {
        STUBLOG(qPrintable("BluezAdapterStub::GetRemoteCompany( \"" + a + "\" )"));
        return remoteCompany[a];
    }

    inline QString GetRemoteName(QString const &a) {
        STUBLOG(qPrintable("BluezAdapterStub::GetRemoteName( \"" + a + "\" )"));
        return remoteName[a];
    }

    inline uint GetRemoteClass(QString const &a) {
        STUBLOG(qPrintable("BluezAdapterStub::GetRemoteClass( \"" + a + "\" )"));
        return remoteClass[a];
    }

public:
    inline void emitNameChanged(QString const& a) { NameChanged(a); }
    inline void emitModeChanged(QString const& a) { ModeChanged(a); }
    inline void emitDiscoverableTimeoutChanged(uint a) { DiscoverableTimeoutChanged(a); }
    inline void emitRemoteDeviceConnected(QString const& a) { RemoteDeviceConnected(a); }
    inline void emitRemoteDeviceDisconnected(QString const& a) { RemoteDeviceDisconnected(a); }
    inline void emitRemoteDeviceDisappeared(QString const& a) { RemoteDeviceDisappeared(a); }
    inline void emitDiscoveryStarted() { DiscoveryStarted(); }
    inline void emitDiscoveryCompleted() { DiscoveryCompleted(); }
    inline void emitRemoteAliasChanged(QString const& a,QString const& b) { RemoteAliasChanged(a,b); }
    inline void emitRemoteAliasCleared(QString const& a) { RemoteAliasCleared(a); }
    inline void emitBondingCreated(QString const& a) { BondingCreated(a); }
    inline void emitBondingRemoved(QString const& a) { BondingRemoved(a); }
    inline void emitRemoteNameUpdated(QString const& a,QString const& b) { RemoteNameUpdated(a,b); }
    inline void emitRemoteNameFailed(QString const& a) { RemoteNameFailed(a); }
    inline void emitRemoteClassUpdated(QString const& a,uint b) { RemoteClassUpdated(a,b); }
    inline void emitRemoteDeviceDisconnectRequested(QString const& a) { RemoteDeviceDisconnectRequested(a); }

signals:
    void NameChanged(QString const&);
    void ModeChanged(QString const&);
    void DiscoverableTimeoutChanged(uint);
    void RemoteDeviceConnected(QString const&);
    void RemoteDeviceDisconnected(QString const&);
    void RemoteDeviceDisappeared(QString const&);
    void DiscoveryStarted();
    void DiscoveryCompleted();
    void RemoteAliasChanged(QString const&,QString const&);
    void RemoteAliasCleared(QString const&);
    void BondingCreated(QString const&);
    void BondingRemoved(QString const&);
    void RemoteNameUpdated(QString const&,QString const&);
    void RemoteNameFailed(QString const&);
    void RemoteClassUpdated(QString const&,uint);
    void RemoteDeviceDisconnectRequested(QString const&);
};

#undef STUBLOG

#ifdef QT_QWS_GREENPHONE

/*?
    Class to ensure DBUS session bus is available.
    Needed for Greenphone, where DBUS session bus normally isn't started,
    and autolaunch doesn't work.
 */
class DBusLauncher
{
public:
    DBusLauncher();
    ~DBusLauncher();
private:
    int m_pid;
};

#endif

/*?
    Base class for Bluetooth-specific unit tests.
    Compared to QObject, this class provides the following additional functionality:
    \list
        \o Automatically connects and registers BlueZ stubs for debugging purposes.
        \o Provides several _data() functions for common Bluetooth test data.
        \o Provides member functions to generate fake BlueZ D-Bus signals.
    \endlist
*/
class QtopiaBluetoothUnitTest : public QObject
{
    Q_OBJECT
public:
    QtopiaBluetoothUnitTest();

protected:
    /*?
        Test data consisting of human-readable device names, for use with any test function.
    */
    inline void deviceName_data()
    {
        QTest::addColumn<QString>("deviceName");

        QTest::newRow("simple") << "foobar";
        QTest::newRow("empty") << "";
        QTest::newRow("null") << QString();
        QTest::newRow("with slashes") << "some/device/with/slashes";
        QTest::newRow("with colons") << "some:device:with:colons";
        QTest::newRow("with spaces") << "some device with slashes";

        {
            QString longStr;
            for (int i = 0; i < 1000; ++i) longStr += QString(" a long string %1 ").arg(i);
            QTest::newRow("rather long") << longStr;
        }
    }

    /*?
        Test data consisting of valid device identifiers, for use with any test function.
    */
    inline void validDevice_data()
    {
        QTest::addColumn<QString>("device");

        for (int i = 0; i < 8; ++i) {
            QString dev = QString("hci%1").arg(i);
            QTest::newRow(qPrintable(dev)) << dev;
        }
    }

    /*?
        Test data consisting of valid bluetooth addresses, for use with any test function.
    */
    inline void validAddress_data()
    {
        QTest::addColumn<QString>("addressStr");

        QTest::newRow("simple") << "01:23:45:67:89:AB";
        QTest::newRow("mixed case") << "45:67:89:Ab:cd:eF";
        QTest::newRow("< A") << "98:76:54:32:10:98";
        QTest::newRow("> 9") << "FE:DC:BA:AB:CD:EF";
        QTest::newRow("any") << "00:00:00:00:00:00";
        QTest::newRow("all") << "FF:FF:FF:FF:FF:FF";
        QTest::newRow("local") << "00:00:00:FF:FF:FF";
    }

    /*?
        Test data consisting of valid bluetooth manufacturers, for use with any test function.
    */
    inline void validManufacturer_data()
    {
        QTest::addColumn<QString>("manufacturer");

        QTest::newRow("simple") << "foo manufacturer";
        QTest::newRow("mixed case") << "I am a Great Manufacturer, Buy My Products";
        QTest::newRow("with colons") << "colons:are:great";
        QTest::newRow("with slashes") << "slashes/are\\even/better";
        QTest::newRow("company ID") << "Company ID 1234";
        {
            QString longStr;
            for (int i = 0; i < 1000; ++i) longStr += QString(" a long string %1 ").arg(i);
            QTest::newRow("rather long") << longStr;
        }
    }

    /*? BlueZ manager stub. */
    BluezStub *bluez;
    /*? BlueZ adapter stub. */
    BluezAdapterStub *bluezAdapter;

private:
#ifdef QT_QWS_GREENPHONE
    static DBusLauncher launcher;
#endif
};

/*? Make a new virtual adapter named @a A */
#define MAKEADAPTER(A) {\
    if ( !QDBusConnection::sessionBus().registerObject(QString("/org/bluez/%1").arg(A), this) ) {\
        QFAIL(qPrintable(QString("Couldn't register adapter %1.").arg(A)));\
    }\
}

/*? Destroy the virtual adapter named @a A */
#define DESTROYADAPTER(A) {\
    QDBusConnection::sessionBus().unregisterObject(QString("/org/bluez/%1").arg(A));\
}

#endif
