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

#include <QMetaType>
#include <QTimer>
#include <QSignalSpy>
#include <qtest.h>
#include <QBuffer>
#include <QDebug>
#include <QTcpSocket>
#include <QTcpServer>

#include <shared/util.h>    // QTRY_VERIFY
#include <qtopiaipcmarshal.h>
#include <shared/qtopiaunittest.h>  // qLog(Autotest)

#include <QWhereabouts>
#include <QWhereaboutsUpdate>
#include <QWhereaboutsFactory>
#include <QNmeaWhereabouts>

Q_DECLARE_METATYPE(QPointer<QBuffer>)
Q_DECLARE_METATYPE(QWhereaboutsUpdate)
Q_DECLARE_USER_METATYPE_ENUM(QNmeaWhereabouts::UpdateMode)
Q_DECLARE_USER_METATYPE_ENUM(QWhereabouts::State)
Q_IMPLEMENT_USER_METATYPE_ENUM(QWhereabouts::State)
Q_DECLARE_METATYPE(QList<QWhereabouts::State>)


/*
    This file does not test NMEA sentence parsing - that is done
    in tst_qwhereaboutsupdate when testing fromNmea().
*/

// assumes sentence starts with '$' and ends with '*'
QByteArray tst_qnmeawhereabouts_addChecksum(const QByteArray &sentence)
{
    // XOR byte value of all characters between '$' and '*'
    int result = 0;
    for (int i=1; i<sentence.length()-1; i++)
        result ^= sentence[i];

    QString sum;
    sum.sprintf("%02x", result);
    return sentence + sum.toAscii();
}

static QByteArray tst_qnmeawhereabouts_createGgaSentence(const QTime &time,
                QWhereaboutsUpdate::PositionFixStatus status = QWhereaboutsUpdate::FixAcquired)
{
    QString fix;    // empty string == unknown
    if (status == QWhereaboutsUpdate::FixAcquired)
        fix = "1";
    else if (status == QWhereaboutsUpdate::FixNotAcquired)
        fix = "0";

    QString nmea = QString("$GPGGA,%1,2734.76859,S,15305.99361,E,%2,04,3.5,49.4,M,39.2,M,,*")
            .arg(time.toString("hhmmss.zzz")).arg(fix);
    return tst_qnmeawhereabouts_addChecksum(nmea.toAscii()) + "\r\n";
}

static QByteArray tst_qnmeawhereabouts_createRmcSentence(const QDateTime &dt,
                QWhereaboutsUpdate::PositionFixStatus status = QWhereaboutsUpdate::FixAcquired)
{
    QString fix;    // empty string == unknown
    if (status == QWhereaboutsUpdate::FixAcquired)
        fix = "A";
    else if (status == QWhereaboutsUpdate::FixNotAcquired)
        fix = "V";

    QString time = dt.toString("hhmmss.zzz");
    QString date = dt.toString("ddMMyy");
    QString nmea = QString("$GPRMC,%1,%2,2730.83609,S,15301.87844,E,0.7,9.0,%3,11.2,W,A*")
            .arg(time).arg(fix).arg(date);
    return tst_qnmeawhereabouts_addChecksum(nmea.toAscii()) + "\r\n";
}


class NmeaRealTimeTestHelper : public QWhereaboutsTestHelper
{
    Q_OBJECT
public:
    NmeaRealTimeTestHelper(QObject *parent = 0)
        : QWhereaboutsTestHelper(20, parent)
    {
        m_sock = 0;
        m_whereabouts = 0;

        m_server = new QTcpServer(this);
        m_server->listen();
        connect(m_server, SIGNAL(newConnection()), SLOT(newConnection()));
    }

    ~NmeaRealTimeTestHelper() {}

    virtual bool feedBeforeRequestUpdate() const
    {
        // requestUpdate() will only emit an update for fresh data, so data
        // must be provided after requestUpdate() is called
        return false;
    }

    virtual void initTest()
    {
        QTcpSocket *sock = new QTcpSocket(this);
        sock->connectToHost(m_server->serverAddress(), m_server->serverPort());

        bool b = m_server->waitForNewConnection();
        Q_ASSERT(b);

        m_whereabouts = new QNmeaWhereabouts(QNmeaWhereabouts::RealTimeMode, this);
        m_whereabouts->setSourceDevice(sock);
    }

    virtual void cleanupTest()
    {
        delete m_whereabouts->sourceDevice();
        delete m_sock;
        delete m_whereabouts;
    }

    virtual void flush() { if (m_sock) m_sock->flush(); }
    virtual QWhereabouts *whereabouts() const { return m_whereabouts; }

protected:
    virtual void feedTestUpdate(const QDateTime &dt)
    {
        m_sock->write(tst_qnmeawhereabouts_createRmcSentence(dt));
        //m_sock->flush();
    }

private slots:
    void newConnection()
    {
        m_sock = m_server->nextPendingConnection();
        Q_ASSERT(m_sock != 0);
    }

private:
    QNmeaWhereabouts *m_whereabouts;
    QTcpServer *m_server;
    QTcpSocket *m_sock;
    QTime m_last;
};


class NmeaSimulationTestHelper : public QWhereaboutsTestHelper
{
    Q_OBJECT
public:
    NmeaSimulationTestHelper(QObject *parent = 0)
        : QWhereaboutsTestHelper(20, parent)
    {
        m_buffer = new QBuffer(this);
    }

    ~NmeaSimulationTestHelper() {}

    virtual bool feedBeforeRequestUpdate() const
    {
        // requestUpdate() will only emit an update for fresh data, so data
        // must be provided after requestUpdate() is called
        return false;
    }

    virtual void initTest()
    {
        m_buffer->setData(QByteArray());
        m_buffer->open(QIODevice::ReadWrite);

        m_whereabouts = new QNmeaWhereabouts(this);
        m_whereabouts->setUpdateMode(QNmeaWhereabouts::SimulationMode);
        m_whereabouts->setSourceDevice(m_buffer);
    }

    virtual void cleanupTest()
    {
        m_buffer->close();
        delete m_whereabouts;
    }

    // override this
    virtual void feedTimedUpdates(const QList<int> &intervals, const QList<QDateTime> &dateTimes)
    {
        int origPos = m_buffer->pos();

        if (dateTimes.isEmpty()) {
            QDateTime dt = QDateTime::currentDateTime();
            for (int i=0; i<intervals.count(); i++) {
                dt = dt.addMSecs(intervals[i]);
                m_buffer->write(tst_qnmeawhereabouts_createRmcSentence(dt));
            }
        } else {
            for (int i=0; i<dateTimes.count(); i++)
                m_buffer->write(tst_qnmeawhereabouts_createRmcSentence(dateTimes[i]));
        }

        m_buffer->seek(origPos);
    }

    virtual void flush() { m_buffer->buffer().clear(); }
    virtual QWhereabouts *whereabouts() const { return m_whereabouts; }

    // must ignore first interval when testing update intervals because
    // the first update is always emitted *immediately* if in SimulationMode
    virtual bool isRealTime() const { return false; }

protected:
    virtual void feedTestUpdate(const QDateTime &)
    {
        // feedTimedUpdates() is overriden so this shouldn't be called
        Q_ASSERT(false);
    }

    QBuffer *m_buffer;
    QNmeaWhereabouts *m_whereabouts;
};


class tst_QNmeaWhereabouts : public QObject
{
    Q_OBJECT

public:
    tst_QNmeaWhereabouts(QNmeaWhereabouts::UpdateMode mode, QObject *parent = 0)
        : QObject(parent)
    {
        m_updateMode = mode;
    }

private:
    QNmeaWhereabouts *m_whereabouts;
    QNmeaWhereabouts::UpdateMode m_updateMode;

protected slots:
    void updated(const QWhereaboutsUpdate &update)
    {
        Q_UNUSED(update);
        //qDebug() << "tst_QNmeaWhereabouts::updated()" << update;
    }

    void stateChanged(QWhereabouts::State state)
    {
        Q_UNUSED(state);
        //qDebug() << "tst_QNmeaWhereabouts::stateChanged()" << state;
    }

private slots:
    void initTestCase()
    {
    }

    void init()
    {
        m_whereabouts = new QNmeaWhereabouts(this);
        m_whereabouts->setUpdateMode(m_updateMode);
        connect(m_whereabouts, SIGNAL(updated(QWhereaboutsUpdate)),
                SLOT(updated(QWhereaboutsUpdate)));
        connect(m_whereabouts, SIGNAL(stateChanged(QWhereabouts::State)),
                SLOT(stateChanged(QWhereabouts::State)));
    }

    void cleanup()
    {
        delete m_whereabouts;
    }

    void no_mode_set()
    {
        QNmeaWhereabouts *w = new QNmeaWhereabouts;
        QCOMPARE(w->updateMode(), QNmeaWhereabouts::InvalidMode);

        QSignalSpy spy(w, SIGNAL(stateChanged(QWhereabouts::State)));
        QBuffer buffer;
        w->setSourceDevice(&buffer);
        w->startUpdates();

        // should return to NotAvailable state
        QTRY_VERIFY(spy.count() == 2);
        QCOMPARE(spy[0][0].value<QWhereabouts::State>(), QWhereabouts::Initializing);
        QCOMPARE(spy[1][0].value<QWhereabouts::State>(), QWhereabouts::NotAvailable);

        delete w;
    }

    void stateChanged_basic_data()
    {
        QTest::addColumn< QPointer<QBuffer> >("device");
        QTest::addColumn< QList<QWhereabouts::State> >("expectedStates");

        QPointer<QBuffer> buf0;
        QTest::newRow("invalid device")
                << buf0
                << (QList<QWhereabouts::State>() << QWhereabouts::Initializing << QWhereabouts::NotAvailable);

        QPointer<QBuffer> buf1 = new QBuffer;
        buf1->open(QIODevice::ReadOnly);
        QTest::newRow("valid opened device")
                << buf1
                << (QList<QWhereabouts::State>() << QWhereabouts::Initializing << QWhereabouts::Available);
    }

    void stateChanged_basic()
    {
        QFETCH(QPointer<QBuffer>, device);
        QFETCH(QList<QWhereabouts::State>, expectedStates);

        m_whereabouts->setSourceDevice(device);

        QSignalSpy spy(m_whereabouts, SIGNAL(stateChanged(QWhereabouts::State)));
        m_whereabouts->startUpdates();
        QTRY_VERIFY(spy.count() == expectedStates.count());

        for (int i=0; i<expectedStates.count(); i++) {
            QWhereabouts::State actualState = spy[i][0].value<QWhereabouts::State>();
            QCOMPARE(actualState, expectedStates[i]);
        }

        // should not get any more stateChanged() signals
        spy.clear();
        QTest::qWait(100);
        QCOMPARE(spy.count(), 0);
    }

    void stateChanged_initialization_data()
    {
        QTest::addColumn<QByteArray>("bytes");
        QTest::addColumn<bool>("callStart");
        QTest::addColumn< QList<QWhereabouts::State> >("expectedStates");

        QByteArray sentence =
                tst_qnmeawhereabouts_createRmcSentence(QDateTime::currentDateTime());

        QTest::newRow("device has sentence with fix, startUpdates()")
                << sentence
                << true
                << (QList<QWhereabouts::State>()
                        << QWhereabouts::Initializing
                        << QWhereabouts::Available
                        << QWhereabouts::PositionFixAcquired);

        QTest::newRow("device has sentence with fix, requestUpdate()")
                << sentence
                << false
                << (QList<QWhereabouts::State>()
                        << QWhereabouts::Initializing
                        << QWhereabouts::Available
                        << QWhereabouts::PositionFixAcquired);
    }

    void stateChanged_initialization()
    {
        QFETCH(QByteArray, bytes);
        QFETCH(bool, callStart);
        QFETCH(QList<QWhereabouts::State>, expectedStates);

        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);
        m_whereabouts->setSourceDevice(&buffer);

        QSignalSpy spy(m_whereabouts, SIGNAL(stateChanged(QWhereabouts::State)));
        if (callStart)
            m_whereabouts->startUpdates();
        else
            m_whereabouts->requestUpdate();

        int pos = buffer.pos();
        buffer.write(bytes);
        buffer.seek(pos);

        QTRY_VERIFY(spy.count() == expectedStates.count());
    }

    void testBufferedDataHandling_data()
    {
        QTest::addColumn<bool>("callStart");
        QTest::newRow("startUpdates()") << true;
        QTest::newRow("requestUpdate()") << false;
    }

    void testBufferedDataHandling()
    {
        // In SimulationMode, data stored in the QIODevice is read when
        // startUpdates() or requestUpdate() is called.
        // In RealTimeMode, all existing data in the QIODevice is ignored -
        // only new data will be read.

        QFETCH(bool, callStart);

        QDateTime dt = QDateTime::currentDateTime().toUTC();
        QBuffer buffer;
        buffer.setData(tst_qnmeawhereabouts_createRmcSentence(dt));
        m_whereabouts->setSourceDevice(&buffer);

        QSignalSpy spy(m_whereabouts, SIGNAL(updated(QWhereaboutsUpdate)));
        if (callStart)
            m_whereabouts->startUpdates();
        else
            m_whereabouts->requestUpdate();

        if (m_whereabouts->updateMode() == QNmeaWhereabouts::RealTimeMode) {
            QTest::qWait(100);
            QCOMPARE(spy.count(), 0);
        } else {
            QTRY_VERIFY(spy.count() == 1);
            QCOMPARE(spy[0][0].value<QWhereaboutsUpdate>().updateDateTime(), dt);
        }
    }

    void stateChanged_catchesReadyRead()
    {
        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);

        QSignalSpy spy(m_whereabouts, SIGNAL(stateChanged(QWhereabouts::State)));

        m_whereabouts->setSourceDevice(&buffer);
        m_whereabouts->startUpdates();

        // wait until state==Available
        QTRY_VERIFY(spy.count() == 2);
        spy.clear();

        QByteArray sentence = tst_qnmeawhereabouts_createRmcSentence(QDateTime::currentDateTime());
        int pos = buffer.pos();
        buffer.write(sentence);
        buffer.seek(pos);

        // should get the new data and return to PositionFixAcquired state
        QTRY_VERIFY(spy.count() == 1);
        QCOMPARE(spy[0][0].value<QWhereabouts::State>(), QWhereabouts::PositionFixAcquired);

        // should not get any more stateChanged() signals
        spy.clear();
        QTest::qWait(100);
        QCOMPARE(spy.count(), 0);
    }

    void stateChanged_closeDevice()
    {
        QBuffer buffer;
        buffer.open(QIODevice::ReadOnly);
        m_whereabouts->setSourceDevice(&buffer);
        m_whereabouts->startUpdates();

        QSignalSpy spy(m_whereabouts, SIGNAL(stateChanged(QWhereabouts::State)));
        buffer.close();     // should catch aboutToClose()
        QTRY_VERIFY(spy.count() == 1);

        QWhereabouts::State actualState = spy[0][0].value<QWhereabouts::State>();
        QCOMPARE(actualState, QWhereabouts::NotAvailable);

        // should not get any more stateChanged() signals
        spy.clear();
        QTest::qWait(100);
        QCOMPARE(spy.count(), 0);
    }

    void stateChanged_closeDeviceReadChannel()
    {
        QTcpServer *server = new QTcpServer;
        server->listen();

        QTcpSocket *client = new QTcpSocket;
        client->connectToHost(server->serverAddress(), server->serverPort());

        bool b = server->waitForNewConnection();
        Q_ASSERT(b);
        QTcpSocket *newConn = server->nextPendingConnection();
        Q_ASSERT(newConn != 0);

        m_whereabouts->setSourceDevice(client);
        m_whereabouts->startUpdates();

        QSignalSpy spy(m_whereabouts, SIGNAL(stateChanged(QWhereabouts::State)));
        newConn->close();     // other side closed conn, should catch readChannelFinished()
        QTRY_VERIFY(spy.count() == 1);

        QWhereabouts::State actualState = spy[0][0].value<QWhereabouts::State>();
        QCOMPARE(actualState, QWhereabouts::NotAvailable);

        // should not get any more stateChanged() signals
        spy.clear();
        QTest::qWait(100);
        QCOMPARE(spy.count(), 0);
    }

    void stateChanged_destroyDevice()
    {
        QBuffer *buffer = new QBuffer;
        m_whereabouts->setSourceDevice(buffer);
        m_whereabouts->startUpdates();

        QSignalSpy spy(m_whereabouts, SIGNAL(stateChanged(QWhereabouts::State)));
        delete buffer;     // should catch destroyed()
        QTRY_VERIFY(spy.count() == 1);

        QWhereabouts::State actualState = spy[0][0].value<QWhereabouts::State>();
        QCOMPARE(actualState, QWhereabouts::NotAvailable);

        // should not get any more stateChanged() signals
        QTest::qWait(100);
        QCOMPARE(spy.count(), 1);
    }

    void update_firstValidDateTime()
    {
        // updated() should not be emitted until a sentence with a valid
        // date & time is found.
        // In particular, if a valid GGA sentence is received before a RMC
        // sentence, the GGA sentence cannot be emitted because it does not
        // have a date (only time), so we have to wait for a valid RMC
        // sentence first.

        QDateTime dateTime1 = QDateTime::currentDateTime().toUTC();
        QDateTime dateTime2 = dateTime1.addSecs(1);
        QDateTime dateTime3 = dateTime1.addSecs(2);

        QList<QByteArray> bytes;
        bytes.append(tst_qnmeawhereabouts_createGgaSentence(dateTime1.time()));
        bytes.append(tst_qnmeawhereabouts_createRmcSentence(dateTime2));
        bytes.append(tst_qnmeawhereabouts_createGgaSentence(dateTime3.time()));

        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);

        QSignalSpy spy(m_whereabouts, SIGNAL(updated(QWhereaboutsUpdate)));
        m_whereabouts->setSourceDevice(&buffer);
        m_whereabouts->startUpdates();

        QList<QWhereaboutsUpdate> emittedUpdates;

        for (int i=0; i<bytes.size(); i++) {
            int pos = buffer.pos();
            buffer.write(bytes[i]);
            buffer.seek(pos);

            // shouldn't receive the first GGA sentence
            if (i > 0) {
                QTRY_VERIFY(spy.count() == 1);
                emittedUpdates.append(spy[0][0].value<QWhereaboutsUpdate>());
                spy.clear();
            }
        }

        // should only have received RMC sentence and the GGA sentence *after* it
        QCOMPARE(emittedUpdates.count(), 2);
        QCOMPARE(emittedUpdates[0].updateDateTime(), dateTime2);
        QCOMPARE(emittedUpdates[1].updateDateTime(), dateTime3);
    }

    void update_unknownFix()
    {
        // should emit update even if fix status is FixStatusUnknown, as long
        // as it's not now NotAcquired

        QDateTime dateTime1 = QDateTime::currentDateTime().toUTC();
        QDateTime dateTime2 = dateTime1.addSecs(1);

        QList<QByteArray> bytes;
        bytes += tst_qnmeawhereabouts_createRmcSentence(
                dateTime1, QWhereaboutsUpdate::FixAcquired);
        bytes += tst_qnmeawhereabouts_createRmcSentence(
                dateTime2, QWhereaboutsUpdate::FixStatusUnknown);

        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);

        QSignalSpy spy(m_whereabouts, SIGNAL(updated(QWhereaboutsUpdate)));

        m_whereabouts->setSourceDevice(&buffer);
        m_whereabouts->startUpdates();

        QList<QWhereaboutsUpdate> emittedUpdates;

        for (int i=0; i<bytes.size(); i++) {
            int pos = buffer.pos();
            buffer.write(bytes[i]);
            buffer.seek(pos);

            QTRY_VERIFY(spy.count() == 1);
            emittedUpdates.append(spy[0][0].value<QWhereaboutsUpdate>());
            spy.clear();
        }

        QCOMPARE(emittedUpdates.count(), 2);
        QCOMPARE(emittedUpdates[0].updateDateTime(), dateTime1);
        QCOMPARE(emittedUpdates[1].updateDateTime(), dateTime2);
    }
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    int r;

    qLog(Autotest) << "\n>>> tst_QNmeaWhereabouts: basic test, RealTimeMode:";
    tst_QNmeaWhereabouts basicRealTimeTest(QNmeaWhereabouts::RealTimeMode);
    r = QTest::qExec(&basicRealTimeTest, argc, argv);
    if (r < 0)
        return r;

    qLog(Autotest) << "\n>>> tst_QNmeaWhereabouts: basic test, SimulationMode:";
    tst_QNmeaWhereabouts basicSimTest(QNmeaWhereabouts::SimulationMode);
    r = QTest::qExec(&basicSimTest, argc, argv);
    if (r < 0)
        return r;

    qLog(Autotest) << "\n>>> tst_QNmeaWhereabouts: detailed test, RealTimeMode:";
    QWhereaboutsSubclassTest detailedRealTimeTest(new NmeaRealTimeTestHelper);
    r = QTest::qExec(&detailedRealTimeTest, argc, argv);
    if (r < 0)
        return r;

    qLog(Autotest) << "\n>>> tst_QNmeaWhereabouts: detailed test, SimulationMode:";
    QWhereaboutsSubclassTest detailedSimTest(new NmeaSimulationTestHelper);
    r = QTest::qExec(&detailedSimTest, argc, argv);
    if (r < 0)
        return r;

    return 0;
}

//QTEST_MAIN(tst_QNmeaWhereabouts)
#include "tst_qnmeawhereabouts.moc"
