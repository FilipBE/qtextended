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

#include <QBluetoothAuthorizationAgent>
#include <QBluetoothAbstractService>
#include <QBluetoothAddress>
#include <QBluetoothSdpQuery>
#include <QBluetoothSdpRecord>
#include <QBluetoothRfcommSocket>
#include <QProcess>
#include <QTest>
#include <QSignalSpy>
#include <QStringList>
#include <shared/util.h>
#include <shared/qtopiaunittest.h>
#include <QDebug>
#include <QFile>
#include <QDir>
#include "qsdpxmlgenerator_p.h"

Q_DECLARE_METATYPE(QBluetoothAbstractService::Error)

QBluetoothAddress REMOTE_DEVICE(qgetenv("TEST_BLUETOOTH_DEVICE"));

class TestAuthorizationAgent : public QBluetoothAuthorizationAgent
{
    Q_OBJECT

public:
    explicit TestAuthorizationAgent(QObject *parent = 0);
    ~TestAuthorizationAgent();

protected:
    virtual bool authorize(const QString &localDevice,
                            const QBluetoothAddress &addr,
                            const QString &service,
                            const QString &uuid);
    virtual void cancel(const QString &localDevice,
                         const QBluetoothAddress &addr,
                         const QString &service,
                         const QString &uuid);
    virtual void release();

};

TestAuthorizationAgent::TestAuthorizationAgent(QObject *parent)
    : QBluetoothAuthorizationAgent("TestAgent", parent)
{
}

TestAuthorizationAgent::~TestAuthorizationAgent()
{
    
}

bool TestAuthorizationAgent::authorize(const QString &localDevice,
                                       const QBluetoothAddress &addr,
                                       const QString &service,
                                       const QString &uuid)
{
    qDebug() << "Authorize called..." << localDevice << addr.toString() << service << uuid;

    if (uuid == "sdfl")
        return true;

    return false;
}

void TestAuthorizationAgent::cancel(const QString &localDevice,
                                    const QBluetoothAddress &addr,
                                    const QString &service,
                                    const QString &uuid)
{
    qDebug() << "cancel called" << localDevice << addr.toString() << service << uuid;
}

void TestAuthorizationAgent::release()
{
    qDebug() << "release called";
}

class TestService : public QBluetoothAbstractService
{
    Q_OBJECT

public:
    explicit TestService(QObject *parent = 0);
    ~TestService();

    bool startAuth(const QString &uuid);
    void start();
    void stop();
    void setSecurityOptions(QBluetooth::SecurityOptions options);

    using QBluetoothAbstractService::registerRecord;
    using QBluetoothAbstractService::updateRecord;
    using QBluetoothAbstractService::unregisterRecord;
    using QBluetoothAbstractService::error;
    using QBluetoothAbstractService::errorString;
};

TestService::TestService(QObject *parent)
    : QBluetoothAbstractService("TestService", "TestDisplayService", "This is a testing service", parent)
{

}

TestService::~TestService()
{

}

bool TestService::startAuth(const QString &uuid)
{
    return requestAuthorization(REMOTE_DEVICE, uuid);
}

void TestService::start()
{
    emit started(false, "");
}

void TestService::stop()
{
    emit stopped();
}

void TestService::setSecurityOptions(QBluetooth::SecurityOptions options)
{
}

class tst_QBluetoothAuthorization : public QObject
{
    Q_OBJECT

private:
    TestService *service;
    TestAuthorizationAgent *agent;
    quint32 handle;

private slots:

    void initTestCase()
    {
        if (!REMOTE_DEVICE.isValid())
            QSKIP("This test requires a remote bluetooth device set up for testing, "
                  "with its address set in the TEST_BLUETOOTH_DEVICE environment variable.",
                  SkipAll);
    }

    void init()
    {
        service = new TestService();
        qRegisterMetaType<QBluetoothAbstractService::Error>("QBluetoothAbstractService::Error");
        agent = new TestAuthorizationAgent();
    }

    void cleanup()
    {
        delete service;
        delete agent;
    }

    void testAttributes()
    {
        QCOMPARE(service->name(), QString("TestService"));
        QCOMPARE(service->displayName(), QString("TestDisplayService"));
        QCOMPARE(service->description(), QString("This is a testing service"));
    }

    void testAuthorize()
    {
        QSignalSpy spy(service, SIGNAL(authorizationFailed()));
        QVERIFY(service->startAuth("sdfl"));
        QTRY_VERIFY( spy.count() == 1 );
        spy.takeAt(0);
        // We should get a not connected error as
        // no connection is established
        QCOMPARE(service->error(), QBluetoothAbstractService::NotAuthorized);

        QSignalSpy started(service, SIGNAL(started(bool,QString)));
        service->start();
        QTRY_VERIFY(started.count() == 1);
        started.takeAt(0);

        QVERIFY(service->startAuth("sdfl"));
        QTRY_VERIFY( spy.count() == 1 );
        spy.takeAt(0);
        // We should get a not connected error as
        // no connection is established

        QCOMPARE(service->error(), QBluetoothAbstractService::NotConnected);

        // Now try to establish a connection
        QBluetoothRfcommSocket sock;
        sock.connect(QBluetoothAddress::any, REMOTE_DEVICE, 14);

        QVERIFY(service->startAuth("sdfl"));
        QTRY_VERIFY( spy.count() == 1 );
        spy.takeAt(0);
        // We should get a does not exit error as
        // no agent is registered
        QCOMPARE(service->error(), QBluetoothAbstractService::DoesNotExist);

        QVERIFY(agent->registerAgent());

        // Now should be Rejected
        QVERIFY(service->startAuth("shouldberejected"));
        QTRY_VERIFY( spy.count() == 1 );
        spy.takeAt(0);
        QCOMPARE(service->error(), QBluetoothAbstractService::Rejected);

        // Now should be successful
        QSignalSpy spy2(service, SIGNAL(authorizationSucceeded()));
        QVERIFY(service->startAuth("sdfl"));
        QTRY_VERIFY(spy2.count() == 1);
        spy2.takeAt(0);

        QVERIFY(agent->unregisterAgent());
    }

    void testAddRecord()
    {
        QDir dataDir = QtopiaUnitTest::baseDataPath();
        QString orig = dataDir.filePath("orig.xml");
        QFile origFile(orig);
        QVERIFY(origFile.open(QIODevice::ReadOnly | QIODevice::Text));

        QBluetoothSdpRecord origRecord = QBluetoothSdpRecord::fromDevice(&origFile);
        QVERIFY(!origRecord.isNull());

        handle = service->registerRecord(orig);
        QVERIFY(handle != 0);

        QStringList args;
        args << "get" << "--xml" << QString::number(handle, 16);
        QProcess sdptool;
        sdptool.start("/usr/local/bin/sdptool", args);
        sdptool.waitForFinished();

        QByteArray output = sdptool.readAllStandardOutput();
        QBluetoothSdpRecord registeredRecord = QBluetoothSdpRecord::fromData(output);
        QVERIFY(!registeredRecord.isNull());

        // Need to remove the record handle attribute, as it won't be in
        // the original record
        registeredRecord.removeAttribute(0x0000);

        QCOMPARE(registeredRecord, origRecord);
    }

    void testUpdateRecord()
    {
        QDir dataDir = QtopiaUnitTest::baseDataPath();
        QString update = dataDir.filePath("update.xml");
        QFile updateFile(update);
        QVERIFY(updateFile.open(QIODevice::ReadOnly | QIODevice::Text));

        QBluetoothSdpRecord updateRecord = QBluetoothSdpRecord::fromDevice(&updateFile);
        QVERIFY(!updateRecord.isNull());

        QVERIFY(service->updateRecord(handle, update));

        QStringList args;
        args << "get" << "--xml" << QString::number(handle, 16);
        QProcess sdptool;
        sdptool.start("/usr/local/bin/sdptool", args);
        sdptool.waitForFinished();

        QByteArray output = sdptool.readAllStandardOutput();
        QBluetoothSdpRecord registeredRecord = QBluetoothSdpRecord::fromData(output);
        QVERIFY(!registeredRecord.isNull());

        // Need to remove the record handle attribute, as it won't be in
        // the original record
        registeredRecord.removeAttribute(0x0000);

        QCOMPARE(registeredRecord, updateRecord);
    }

    void testDeleteRecord()
    {
        QVERIFY(service->unregisterRecord(handle));
    }

    void testAddRecord2()
    {
        QDir dataDir = QtopiaUnitTest::baseDataPath();
        QString orig = dataDir.filePath("orig.xml");
        QFile origFile(orig);
        QVERIFY(origFile.open(QIODevice::ReadOnly | QIODevice::Text));

        QBluetoothSdpRecord origRecord = QBluetoothSdpRecord::fromDevice(&origFile);
        QVERIFY(!origRecord.isNull());

        handle = service->registerRecord(origRecord);
        QVERIFY(handle != 0);

        QStringList args;
        args << "get" << "--xml" << QString::number(handle, 16);
        QProcess sdptool;
        sdptool.start("/usr/local/bin/sdptool", args);
        sdptool.waitForFinished();

        QByteArray output = sdptool.readAllStandardOutput();
        QBluetoothSdpRecord registeredRecord = QBluetoothSdpRecord::fromData(output);
        QVERIFY(!registeredRecord.isNull());

        // Need to remove the record handle attribute, as it won't be in
        // the original record
        registeredRecord.removeAttribute(0x0000);

        QCOMPARE(registeredRecord, origRecord);

        QVERIFY(service->unregisterRecord(handle));
    }
};

QTEST_MAIN(tst_QBluetoothAuthorization)
#include "tst_qbluetoothauthorization.moc"
