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

#include "../qwhereaboutssubclasstest.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QMetaType>
#include <QTimer>
#include <QSignalSpy>
#include <qtest.h>

#include <shared/util.h>    // QTRY_VERIFY
#include <qtopiaipcmarshal.h>

#include <QWhereabouts>
#include <QWhereaboutsUpdate>
#include <QWhereaboutsFactory>

Q_DECLARE_METATYPE(QWhereaboutsUpdate)
Q_DECLARE_USER_METATYPE_ENUM(QWhereabouts::State)
Q_IMPLEMENT_USER_METATYPE_ENUM(QWhereabouts::State)
Q_DECLARE_METATYPE(QList<QWhereabouts::State>)

static const QByteArray Q_GPSD_WATCHER_MODE_ON("w+\n");
static const QByteArray Q_GPSD_WATCHER_MODE_OFF("w-\n");
static const QByteArray Q_GPSD_GET_FIX("o\n");
static const QByteArray Q_GPSD_GET_STATUS("x\n");


class GpsdTestHelper: public QWhereaboutsTestHelper
{
    Q_OBJECT
public:
    GpsdTestHelper(QObject *parent = 0)
        : QWhereaboutsTestHelper(20, parent),
          m_server(new QTcpServer(this)),
          m_updateImmediately(false)
    {
        bool b = m_server->listen(QHostAddress::Any, 2947);
        Q_ASSERT(b);
        connect(m_server, SIGNAL(newConnection()), SLOT(newConnection()));
        m_whereabouts = 0;
        m_sock = 0;
    }

    ~GpsdTestHelper() {}

    virtual bool feedBeforeRequestUpdate() const
    {
        // requestUpdate() requests the current position data from GPSd, so
        // GPSd must have already received that data before requestUpdate()
        // is called
        return true;
    }

    virtual void initTest()
    {
        m_updateImmediately = false;
        if (m_whereabouts)
            delete m_whereabouts;
        m_whereabouts = QWhereaboutsFactory::create("gpsd");
        Q_ASSERT(m_whereabouts != 0);
    }

    virtual void cleanupTest()
    {
    }

    virtual void feedTestUpdate(const QDateTime &dt)
    {
        QByteArray dateStr = QString("%1.%2").arg(dt.toTime_t()).arg(dt.time().msec()).toLatin1();
        QByteArray bytes("GPSD,O=RMC " + dateStr + " ? -27.579742 153.100097 ? ? ? ? ? ? ? ? ? 3\n");
        m_updates << bytes;

        if (m_updateImmediately)
            m_sock->write(bytes);
    }

    virtual QWhereabouts *whereabouts() const
    {
        return m_whereabouts;
    }

    virtual void flush()
    {
        m_sock->flush();
        m_updates.clear();
    }

private slots:
    void newConnection()
    {
        if (m_sock)
            delete m_sock;
        m_sock = m_server->nextPendingConnection();
        connect(m_sock, SIGNAL(readyRead()), SLOT(readyRead()));
    }

    void readyRead()
    {
        while (m_sock->bytesAvailable()) {
            QByteArray cmd = m_sock->readLine();

            if (cmd == Q_GPSD_WATCHER_MODE_ON || cmd == Q_GPSD_WATCHER_MODE_OFF) {
                m_updateImmediately = (cmd == Q_GPSD_WATCHER_MODE_ON);

            } else if (cmd == Q_GPSD_GET_FIX) {
                // client has sent a GPSd request for an update
                // send most recent update, then clear the cache
                if (m_updates.count() > 0)
                    m_sock->write(m_updates.last());
                m_updates.clear();
            }
        }
    }

private:
    QWhereabouts *m_whereabouts;
    QList<QByteArray> m_updates;
    QTcpServer *m_server;
    QTcpSocket *m_sock;
    bool m_updateImmediately;
};


/*
    These tests feed GPS data (in the format expected from GPSd) to the
    default GPSd port, and checks that GpsdWhereabouts is able to parse
    this data correctly.
*/

class tst_GpsdWhereabouts : public QObject
{
    Q_OBJECT

private:
    QTcpServer *m_server;
    QWhereabouts *m_whereabouts;
    QWhereaboutsUpdate m_lastUpdate;
    QWhereabouts::State m_lastState;

    void connectPluginClient()
    {
        m_whereabouts->startUpdates(0);
        bool b = m_server->waitForNewConnection();
        Q_ASSERT(b);
    }

    void compareUpdates(const QWhereaboutsUpdate &u1, const QWhereaboutsUpdate &u2)
    {
#define COMPARE_REAL(r1, r2) QCOMPARE(QString::number(r1), QString::number(r2));

        QCOMPARE(u1.dataValidityFlags(), u2.dataValidityFlags());
        QCOMPARE(u1.updateTime(), u2.updateTime());

        COMPARE_REAL(u1.groundSpeed(), u2.groundSpeed());
        COMPARE_REAL(u1.verticalSpeed(), u2.verticalSpeed());
        COMPARE_REAL(u1.course(), u2.course());
        COMPARE_REAL(u1.updateTimeAccuracy(), u2.updateTimeAccuracy());
        COMPARE_REAL(u1.horizontalAccuracy(), u2.horizontalAccuracy());
        COMPARE_REAL(u1.verticalSpeedAccuracy(), u2.verticalSpeedAccuracy());
        COMPARE_REAL(u1.courseAccuracy(), u2.courseAccuracy());

        QWhereaboutsCoordinate c1 = u1.coordinate();
        QWhereaboutsCoordinate c2 = u2.coordinate();
        COMPARE_REAL(c1.latitude(), c2.latitude());
        COMPARE_REAL(c1.longitude(), c2.longitude());
        COMPARE_REAL(c1.altitude(), c2.altitude());
    }

protected slots:
    void updated(const QWhereaboutsUpdate &update)
    {
        //qDebug() << "updated()" << update;
        m_lastUpdate = update;
    }

    void stateChanged(QWhereabouts::State state)
    {
        //qDebug() << ".........stateChanged" << state;
        m_lastState = state;
    }

private slots:
    void initTestCase()
    {
        m_server = new QTcpServer(this);
        bool b = m_server->listen(QHostAddress::Any, 2947);
        Q_ASSERT(b);
    }

    void cleanupTestCase()
    {
        m_server->close();
    }

    void init()
    {
        m_whereabouts = QWhereaboutsFactory::create("GPSd");
        Q_ASSERT(m_whereabouts != 0);
        connect(m_whereabouts, SIGNAL(updated(QWhereaboutsUpdate)),
                SLOT(updated(QWhereaboutsUpdate)));
        connect(m_whereabouts, SIGNAL(stateChanged(QWhereabouts::State)),
                SLOT(stateChanged(QWhereabouts::State)));
    }

    void cleanup()
    {
        delete m_whereabouts;
    }

    void connect_alternativePort()
    {
        QTcpServer *server = new QTcpServer;
        bool b = server->listen(QHostAddress::LocalHost, 8899);
        Q_ASSERT(b);

        QWhereabouts *w = QWhereaboutsFactory::create("GPSd", "127.0.0.1:8899");
        QSignalSpy spy(w, SIGNAL(stateChanged(QWhereabouts::State)));
        w->startUpdates();
        QTRY_VERIFY( spy.count() == 2 );

        QCOMPARE(spy[0][0].value<QWhereabouts::State>(), QWhereabouts::Initializing);
        QCOMPARE(spy[1][0].value<QWhereabouts::State>(), QWhereabouts::Available);

        delete w;
        delete server;
    }

    void connect_badAddress_data()
    {
        QTest::addColumn<QString>("source");
        QTest::newRow("bad host, good port") << QString("localhost:2947");
        QTest::newRow("good host, bad port") << QString("127.0.0.1:43355");
        QTest::newRow("badly formed address") << QString("127.0.0.1");
    }

    void connect_badAddress()
    {
        // if GPSd port doesn't exist, should get Initializing, NotAvailable states

        QFETCH(QString, source);

        QWhereabouts *w = QWhereaboutsFactory::create("GPSd", source);
        QSignalSpy spy(w, SIGNAL(stateChanged(QWhereabouts::State)));
        w->startUpdates();
        QTRY_VERIFY( spy.count() == 2 );

        QCOMPARE(spy[0][0].value<QWhereabouts::State>(), QWhereabouts::Initializing);
        QCOMPARE(spy[1][0].value<QWhereabouts::State>(), QWhereabouts::NotAvailable);

        delete w;
    }

    void connect_correctAddress()
    {
        QVERIFY(m_whereabouts->lastUpdate().isNull());
        QCOMPARE(m_whereabouts->state(), QWhereabouts::NotAvailable);

        QSignalSpy spyInit(m_whereabouts, SIGNAL(stateChanged(QWhereabouts::State)));
        connectPluginClient();      // wait for TCP connection
        QTRY_VERIFY( spyInit.count() == 1 );
        QCOMPARE(m_whereabouts->state(), QWhereabouts::Initializing);   // now connected

        QSignalSpy spyAvailable(m_whereabouts, SIGNAL(stateChanged(QWhereabouts::State)));
        QTcpSocket *s = m_server->nextPendingConnection();
        Q_ASSERT(s != 0);
        QTRY_VERIFY( spyAvailable.count() == 1 );
        QCOMPARE(m_whereabouts->state(), QWhereabouts::Available);   // now connected
    }

    void readFixInfo_data()
    {
        QTest::addColumn<QByteArray>("gpsdInput");
        QTest::addColumn<QWhereaboutsUpdate>("expectedUpdate");

        QWhereaboutsUpdate u;
        QTest::newRow("no fix")
                << QByteArray("GPSD,O=?\n")
                << u;

        u = QWhereaboutsUpdate();
        u.setUpdateDateTime(QDateTime::fromTime_t(1203314493).toUTC()); // time is always present
        QTest::newRow("no fix (no lat-long)")
                << QByteArray("GPSD,O=GGA 1203314493.000 ? ? ? ? ? ? ? ? ? ? ? ? ?\n")
                << u;

        u = QWhereaboutsUpdate(QWhereaboutsCoordinate(-27.579893, 153.100063),
                               QDateTime::fromTime_t(1203314493).toUTC());
        QTest::newRow("basic fix")
                << QByteArray("GPSD,O=GGA 1203314493.000 ? -27.579893 153.100063 ? ? ? ? ? ? ? ? ? ? ?\n")
                << u;

        u = QWhereaboutsUpdate(QWhereaboutsCoordinate(-27.579893, 153.100063),
                               QDateTime::fromTime_t(1203314493).toUTC());
        u.setUpdateTimeAccuracy(0.005);
        QTest::newRow("fix with time error")
                << QByteArray("GPSD,O=GGA 1203314493.000 0.005 -27.579893 153.100063 ? ? ? ? ? ? ? ? ? ? ?\n")
                << u;

        u = QWhereaboutsUpdate(QWhereaboutsCoordinate(-27.579893, 153.100063, 35.7),    // + altitude
                               QDateTime::fromTime_t(1203314493).toUTC());
        QTest::newRow("fix with altitude")
                << QByteArray("GPSD,O=GGA 1203314493.000 ? -27.579893 153.100063 35.7 ? ? ? ? ? ? ? ? ? ?\n")
                << u;

        u = QWhereaboutsUpdate(QWhereaboutsCoordinate(-27.579893, 153.100063),
                               QDateTime::fromTime_t(1203314493).toUTC());
        u.setHorizontalAccuracy(32.0);
        QTest::newRow("fix with horiz accuracy")
                << QByteArray("GPSD,O=GGA 1203314493.000 ? -27.579893 153.100063 ? 32.0 ? ? ? ? ? ? ? ? ?\n")
                << u;

        u = QWhereaboutsUpdate(QWhereaboutsCoordinate(-27.579893, 153.100063),
                               QDateTime::fromTime_t(1203314493).toUTC());
        u.setVerticalAccuracy(15.8);
        QTest::newRow("fix with vertical accuracy")
                << QByteArray("GPSD,O=GGA 1203314493.000 ? -27.579893 153.100063 ? ? 15.8 ? ? ? ? ? ? ? ?\n")
                << u;

        u = QWhereaboutsUpdate(QWhereaboutsCoordinate(-27.579893, 153.100063),
                               QDateTime::fromTime_t(1203314493).toUTC());
        u.setCourse(290);
        QTest::newRow("fix with course")
                << QByteArray("GPSD,O=GGA 1203314493.000 ? -27.579893 153.100063 ? ? ? 290 ? ? ? ? ? ? ?\n")
                << u;

        u = QWhereaboutsUpdate(QWhereaboutsCoordinate(-27.579893, 153.100063),
                               QDateTime::fromTime_t(1203314493).toUTC());
        u.setGroundSpeed(1.2);
        QTest::newRow("fix with speed")
                << QByteArray("GPSD,O=GGA 1203314493.000 ? -27.579893 153.100063 ? ? ? ? 1.2 ? ? ? ? ? ?\n")
                << u;

        u = QWhereaboutsUpdate(QWhereaboutsCoordinate(-27.579893, 153.100063),
                               QDateTime::fromTime_t(1203314493).toUTC());
        u.setVerticalSpeed(0.2);
        QTest::newRow("fix with climb")
                << QByteArray("GPSD,O=GGA 1203314493.000 ? -27.579893 153.100063 ? ? ? ? ? 0.2 ? ? ? ? ?\n")
                << u;

        u = QWhereaboutsUpdate(QWhereaboutsCoordinate(-27.579893, 153.100063),
                               QDateTime::fromTime_t(1203314493).toUTC());
        u.setCourseAccuracy(3.5);
        QTest::newRow("fix with course accuracy")
                << QByteArray("GPSD,O=GGA 1203314493.000 ? -27.579893 153.100063 ? ? ? ? ? ? 3.5 ? ? ? ?\n")
                << u;

        u = QWhereaboutsUpdate(QWhereaboutsCoordinate(-27.579893, 153.100063),
                               QDateTime::fromTime_t(1203314493).toUTC());
        u.setGroundSpeedAccuracy(50.5);
        QTest::newRow("fix with speed accuracy")
                << QByteArray("GPSD,O=GGA 1203314493.000 ? -27.579893 153.100063 ? ? ? ? ? ? ? 50.5 ? ?\n")
                << u;

        u = QWhereaboutsUpdate(QWhereaboutsCoordinate(-27.579893, 153.100063),
                               QDateTime::fromTime_t(1203314493).toUTC());
        u.setVerticalSpeedAccuracy(30.1);
        QTest::newRow("fix with climb accuracy")
                << QByteArray("GPSD,O=GGA 1203314493.000 ? -27.579893 153.100063 ? ? ? ? ? ? ? ? 30.1 ?\n")
                << u;

        u = QWhereaboutsUpdate(QWhereaboutsCoordinate(-27.579742, 153.100097, 36.5),
                               QDateTime::fromTime_t(1203314342).toUTC());
        u.setUpdateTimeAccuracy(0.005);
        u.setHorizontalAccuracy(32.0);
        u.setVerticalAccuracy(22.4);
        u.setCourse(0);
        u.setGroundSpeed(0);
        u.setVerticalSpeed(0);
        u.setGroundSpeedAccuracy(63.2);
        QTest::newRow("complete sample fix")
                << QByteArray("GPSD,O=GGA 1203314342.000 0.005 -27.579742 153.100097 36.50 32.00 22.40 0.0000 0.000 0.000 ? 63.20 ? 3\n")
                << u;
    }

    void readFixInfo()
    {
        QFETCH(QByteArray, gpsdInput);
        QFETCH(QWhereaboutsUpdate, expectedUpdate);

        connectPluginClient();
        QTcpSocket *s = m_server->nextPendingConnection();
        Q_ASSERT(s != 0);

        QSignalSpy spy(m_whereabouts, SIGNAL(updated(QWhereaboutsUpdate)));
        s->write(gpsdInput);
        s->waitForBytesWritten();
        if (!expectedUpdate.isNull()) {     // is an update expected?
            QTRY_VERIFY( spy.count() == 1 );
        } else {
            // check the signal is not emitted
            QTest::qWait(100);
            QCOMPARE(spy.count(), 0);
        }

        if (!expectedUpdate.isNull())
            compareUpdates(m_lastUpdate, expectedUpdate);
    }

    void readGpsStatus_data()
    {
        QTest::addColumn<QByteArray>("gpsdInput");
        QTest::addColumn< QList<QWhereabouts::State> >("expectedStates");

        QTest::newRow("no fix")
                << QByteArray("GPSD,O=?\n")
                << QList<QWhereabouts::State>();

        QTest::newRow("fix unknown")
                << QByteArray("GPSD,O=RMC 1203314493.101 ? ? ? ? ? ? ? ? ? ? ? ? ?\n")
                << QList<QWhereabouts::State>();

        QTest::newRow("no fix (2)")
                << QByteArray("GPSD,O=RMC 1203314493.101 ? ? ? ? ? ? ? ? ? ? ? ? 1\n")
                << QList<QWhereabouts::State>();

        QTest::newRow("2D fix")
                << QByteArray("GPSD,O=RMC 1203314493.101 ? ? ? ? ? ? ? ? ? ? ? ? 2\n")
                << (QList<QWhereabouts::State>() << QWhereabouts::PositionFixAcquired);

        QTest::newRow("3D fix")
                << QByteArray("GPSD,O=RMC 1203314493.101 ? ? ? ? ? ? ? ? ? ? ? ? 3\n")
                << (QList<QWhereabouts::State>() << QWhereabouts::PositionFixAcquired);

        QTest::newRow("gps offline")
                << QByteArray("GPSD,X=0\n")
                << (QList<QWhereabouts::State>() << QWhereabouts::NotAvailable);

        QTest::newRow("no fix, then 3D fix, then no fix, then offline")
                << QByteArray("GPSD,O=?\n"
                              "GPSD,O=RMC 1203314493.101 ? ? ? ? ? ? ? ? ? ? ? ? 3\n"
                              "GPSD,O=RMC 1203314493.101 ? ? ? ? ? ? ? ? ? ? ? ? 1\n"
                              "GPSD,X=0\n")
                << (QList<QWhereabouts::State>()
                         << QWhereabouts::PositionFixAcquired
                         << QWhereabouts::Available
                         << QWhereabouts::NotAvailable );
    }

    void readGpsStatus()
    {
        QFETCH(QByteArray, gpsdInput);
        QFETCH(QList<QWhereabouts::State>, expectedStates);

        // wait for initial stateChanged() to be emitted (once connected)
        QSignalSpy spy(m_whereabouts, SIGNAL(stateChanged(QWhereabouts::State)));

        connectPluginClient();
        QTcpSocket *s = m_server->nextPendingConnection();
        Q_ASSERT(s != 0);

        // skip over Initializing and Available states, since client
        // connection should have been successful
        QTRY_VERIFY( spy.count() == 2 ); spy.takeLast(); spy.takeLast();

        s->write(gpsdInput);
        s->waitForBytesWritten();

        if (expectedStates.size() > 0) {
            // expect some state changes
            QTRY_VERIFY( spy.count() == expectedStates.size() );
            for (int i=0; i<expectedStates.size(); i++) {
                QWhereabouts::State actualState = spy[i][0].value<QWhereabouts::State>();
                QCOMPARE(actualState, expectedStates[i]);
            }
        } else {
            // expect no state changes, check the signal is not emitted
            QTest::qWait(100);
            QTRY_VERIFY( spy.count() == 0 );
        }
    }
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    int r;

    tst_GpsdWhereabouts gpsdTest;
    r = QTest::qExec(&gpsdTest, argc, argv);
    if (r < 0)
        return r;

    QWhereaboutsSubclassTest genericTest(new GpsdTestHelper);
    r = QTest::qExec(&genericTest, argc, argv);
    if (r < 0)
        return r;

    return 0;
}

//QTEST_MAIN(tst_GpsdWhereabouts)
#include "tst_gpsdwhereabouts.moc"
