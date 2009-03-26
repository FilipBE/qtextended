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

#include "qwhereaboutssubclasstest.h"

#include <QMetaType>
#include <QTimer>
#include <QBasicTimer>
#include <QSignalSpy>
#include <QDateTime>
#include <qtest.h>
#include <QDebug>
#include <QList>

#include <shared/util.h>    // QTRY_VERIFY
#include <qtopiaipcmarshal.h>
#include <qtopialog.h>

#include <QWhereabouts>
#include <QWhereaboutsUpdate>
#include <QWhereaboutsFactory>

Q_DECLARE_METATYPE(QWhereaboutsUpdate);
Q_DECLARE_METATYPE(QWhereabouts::State);
Q_DECLARE_METATYPE(QList<int>);
Q_DECLARE_METATYPE(QList<QDateTime>);

// need to call qRegisterMetaType() within a function
static int registerMetaType() { qRegisterMetaType<QWhereaboutsUpdate>("QWhereaboutsUpdate"); return 0; }
static int dummy = registerMetaType();



/*
    Records the intervals between each update emitted by a QWhereabouts instance.
*/
class QUpdateIntervalLogger : public QObject
{
    Q_OBJECT
public:
    QUpdateIntervalLogger(QWhereabouts *whereabouts, QObject *parent = 0)
        : QObject(parent),
          m_whereabouts(whereabouts)
    {
    }

    ~QUpdateIntervalLogger() {}

    int receivedUpdatesCount() const { return m_updateCount; }

    void compareIntervals(int expectedInterval, int errorMarginMsecs, bool compareFirstInterval)
    {
        // same as other compareIntervals() but all intervals are the same
        QList<int> intervals;
        for (int i=0; i<receivedUpdatesCount(); i++)
            intervals << expectedInterval;

        compareIntervals(intervals, errorMarginMsecs, compareFirstInterval);
    }

    void compareIntervals(QList<int> expectedIntervals, int errorMarginMsecs, bool compareFirstInterval)
    {
        QList<int> intervals = listUpdateIntervals();
        //qDebug() << ".......intervals" << intervals << expectedIntervals << compareFirstInterval;
        Q_ASSERT(intervals.size() == expectedIntervals.size());

        for (int i=0; i<intervals.count(); i++) {
            if (i==0 && !compareFirstInterval)
                continue;
            //qDebug() << "Actual vs expected time:"
            //        << qAbs(expectedIntervals[i] - intervals[i]) << "ms";
            QString debug;
            QTextStream stream(&debug);
            stream << intervals[i] << " not within " << errorMarginMsecs << "ms"
                    << " of " <<  expectedIntervals[i];
            QVERIFY2( (intervals[i] >= expectedIntervals[i] - errorMarginMsecs)
                    && (intervals[i] <= expectedIntervals[i] + errorMarginMsecs),
                    debug.toLatin1().constData());
        }
    }

public slots:
    void startLogging()
    {
        connect(m_whereabouts, SIGNAL(updated(QWhereaboutsUpdate)),
                SLOT(updated(QWhereaboutsUpdate)));

        // insert starting timestamp to measure first interval
        m_updateCount = 0;
        m_timestamps << QDateTime::currentDateTime();
    }

private slots:
    void updated(const QWhereaboutsUpdate &update)
    {
        Q_UNUSED(update);
        m_timestamps << QDateTime::currentDateTime();
        m_updateCount++;
        //qDebug() << "> update" << update.updateTime().toString("hh:mm:ss:zzz") << "(count)" << m_updateCount;
    }

private:
    QList<int> listUpdateIntervals() const
    {
        QList<int> intervals;
        for (int i=0; i<m_timestamps.count(); i++) {
            if (i+1 == m_timestamps.count())
                break;
            intervals << m_timestamps[i].time().msecsTo(m_timestamps[i+1].time());
        }
        return intervals;
    }

    QWhereabouts *m_whereabouts;
    QList<QDateTime> m_timestamps;
    int m_updateCount;
    QTime m_last;
};


//=========================================================

QWhereaboutsTestHelper::QWhereaboutsTestHelper(int updateIntervalErrorMargin, QObject *parent)
    : QObject(parent), m_intervalErrorMargin(updateIntervalErrorMargin), m_timer(new QBasicTimer)
{
}

QWhereaboutsTestHelper::~QWhereaboutsTestHelper()
{
    delete m_timer;
}

void QWhereaboutsTestHelper::feedTimedUpdates(const QList<int> &intervals, const QList<QDateTime> &dateTimes)
{
    if (intervals.count() == 0)
        return;

    m_updateIntervals = intervals;
    m_updateTimes = dateTimes;

    if (m_updateTimes.isEmpty()) {
        QDateTime dt = QDateTime::currentDateTime();
        for (int i=0; i<m_updateIntervals.count(); i++) {
            dt = dt.addMSecs(m_updateIntervals[i]);
            m_updateTimes << dt;
        }
    }

    Q_ASSERT(m_updateTimes.count() == m_updateIntervals.count());
    m_timer->start(m_updateIntervals.first(), this);
}

void QWhereaboutsTestHelper::feedTimedUpdates(int interval, int count)
{
    QList<int> intervals;
    for (int i=0; i<count; i++)
        intervals << interval;
    feedTimedUpdates(intervals);
}

void QWhereaboutsTestHelper::timerEvent(QTimerEvent *)
{
    nextUpdate();
}

void QWhereaboutsTestHelper::nextUpdate()
{
    if (m_updateIntervals.count() == 0)
        return;

    feedTestUpdate(m_updateTimes.takeFirst());
    m_updateIntervals.takeFirst();

    if (m_updateIntervals.count() > 0)
        m_timer->start(m_updateIntervals.first(), this);
}

//=========================================================

QWhereaboutsSubclassTest::QWhereaboutsSubclassTest(QWhereaboutsTestHelper *helper, QObject *parent)
    : QObject(parent),
      m_helper(helper)
{
}

void QWhereaboutsSubclassTest::init()
{
    m_helper->initTest();
}

void QWhereaboutsSubclassTest::cleanup()
{
    //m_helper->flush();
    m_helper->cleanupTest();
}

void QWhereaboutsSubclassTest::start_zeroInterval()
{
    // make some random time intervals - then test that updated() is emitted
    // as soon as each update is made available
    QList<int> testIntervals;
    testIntervals << 50 << 180 << 60 << 480 << 30 << 40;

    m_helper->feedTimedUpdates(testIntervals);

    QUpdateIntervalLogger *logger = new QUpdateIntervalLogger(m_helper->whereabouts());
    logger->startLogging();

    m_helper->whereabouts()->startUpdates();

    QTRY_VERIFY(logger->receivedUpdatesCount() == testIntervals.count());

    logger->compareIntervals(testIntervals, m_helper->intervalErrorMargin(), m_helper->isRealTime());

    delete logger;
}

void QWhereaboutsSubclassTest::start_nonZeroInterval_data()
{
    QTest::addColumn< QList<int> >("testIntervals");
    QTest::addColumn< QList<QDateTime> >("testDateTimes");
    QTest::addColumn<int>("updateInterval");
    QTest::addColumn< QList<QDateTime> >("expectedDateTimes");

    QDateTime dt = QDateTime::currentDateTime().toUTC();

    QTest::newRow("0 updates, expect 0 received")
            << QList<int>()
            << QList<QDateTime>()
            << 50
            << QList<QDateTime>();

    QTest::newRow("1 updates, expect 1 update received")
            << (QList<int>() << 10)         // 1 update after 10ms
            << (QList<QDateTime>() << dt)   // date/time of update
            << 50                           // read every 50ms
            << (QList<QDateTime>() << dt);       // expect update to be received


    QTest::newRow("2 updates, expect only last update received")
            << (QList<int>() << 10 << 10)
            << (QList<QDateTime>() << dt << dt.addMSecs(10))
            << 50
            << (QList<QDateTime>() << dt.addMSecs(10));

    // if the first interval falls after an update, the update should
    // not be received
    bool ignoreFirstInterval = (!m_helper->isRealTime());
    QTest::newRow("3 updates, expect only 2nd and 3rd updates to be received")
            << (QList<int>() << 100 << 100 << 100)     // 3 updates, 100 ms between each update
            << (QList<QDateTime>() << dt
                                   << dt.addMSecs(100)
                                   << dt.addMSecs(200))
            << (ignoreFirstInterval ? 150 : 250)     // ignore 1st 100ms as 1st update is emitted immediately
            << (QList<QDateTime>() << dt.addMSecs(100)
                                   << dt.addMSecs(200));   // expect 2nd and 3rd updates only
}

void QWhereaboutsSubclassTest::start_nonZeroInterval()
{
    QFETCH(QList<int>, testIntervals);
    QFETCH(QList<QDateTime>, testDateTimes);
    QFETCH(int, updateInterval);
    QFETCH(QList<QDateTime>, expectedDateTimes);

    QSignalSpy spy(m_helper->whereabouts(), SIGNAL(updated(QWhereaboutsUpdate)));
    m_helper->feedTimedUpdates(testIntervals, testDateTimes);

    QUpdateIntervalLogger *logger = new QUpdateIntervalLogger(m_helper->whereabouts());
    m_helper->whereabouts()->startUpdates(updateInterval);
    logger->startLogging();

    QTRY_VERIFY(logger->receivedUpdatesCount() == expectedDateTimes.count());

    logger->compareIntervals(updateInterval, m_helper->intervalErrorMargin(),
            m_helper->isRealTime());

    // check the correct updates were emitted
    QCOMPARE(spy.count(), expectedDateTimes.count());
    for (int i=0; i<expectedDateTimes.count(); i++) {
        QDateTime emittedTime = spy[i][0].value<QWhereaboutsUpdate>().updateDateTime();
        QCOMPARE(emittedTime, expectedDateTimes[i]);
    }

    delete logger;
}

void QWhereaboutsSubclassTest::start_delayed()
{
    // can't test for simulated data because test data will not be read until
    // startUpdates() is called
    if (!m_helper->isRealTime())
        return;

    // call startUpdates() after several updates have already arrived - updated()
    // should only be emitted with updates that arrive *after* startUpdates()
    // was called

    QList<int> testIntervals;
    testIntervals << 10 << 10 << 500 << 510;

    QList<QDateTime> testDateTimes;
    QDateTime dt = QDateTime::currentDateTime().toUTC();
    for (int i=0; i<testIntervals.count(); i++) {
        dt = dt.addMSecs(testIntervals[i]);
        testDateTimes << dt;
    }

    QSignalSpy spyUpdate(m_helper->whereabouts(), SIGNAL(updated(QWhereaboutsUpdate)));
    m_helper->feedTimedUpdates(testIntervals, testDateTimes);

    // call startUpdates() after first two update have passed
    QTimer::singleShot(300, m_helper->whereabouts(), SLOT(startUpdates()));

    // should only have received last two updates
    QTRY_VERIFY(spyUpdate.count() == 2);

    QList<QDateTime> expectedUpdateTimes = testDateTimes.mid(2);
    for (int i=0; i<spyUpdate.count(); i++) {
        QDateTime emittedTime = spyUpdate[i][0].value<QWhereaboutsUpdate>().updateDateTime();
        QDateTime expectedTime = expectedUpdateTimes[i];
        QCOMPARE(emittedTime, expectedTime);
    }

    spyUpdate.clear();
    QTest::qWait(100);
    QCOMPARE(spyUpdate.count(), 0);
}


void QWhereaboutsSubclassTest::start_restart()
{
    // send 30 updates, then
    // start the tester with interval==0 and check 10 updates are received,
    // then restart with interval > 0 then check,
    // then restart with interval==0 then check
    const int testDataCount = 10;
    const int testDataInterval = 50;
    QList<int> intervals_start;
    intervals_start << 0 << 50 << 0;

    for (int i=0; i<intervals_start.count(); i++) {

        QList<QDateTime> dateTimes;
        QList<int> dataIntervals;
        QDateTime dt = QDateTime::currentDateTime().toUTC();
        for (int j=0; j<testDataCount; j++) {
            dt = dt.addMSecs(testDataInterval);
            dateTimes << dt;

            dataIntervals << testDataInterval;
        }

        // only need to measure intervals if interval > 0
        QUpdateIntervalLogger *logger = 0;
        if (intervals_start[i] > 0) {
            logger = new QUpdateIntervalLogger(m_helper->whereabouts());
            logger->startLogging();
        }

        QSignalSpy spy(m_helper->whereabouts(), SIGNAL(updated(QWhereaboutsUpdate)));
        m_helper->whereabouts()->startUpdates(intervals_start[i]);

        m_helper->feedTimedUpdates(dataIntervals, dateTimes);

        if (intervals_start[i] == 0)
            QTRY_VERIFY(spy.count() == testDataCount);
        else
            logger->compareIntervals(50, 20, m_helper->isRealTime());
    }
}

void QWhereaboutsSubclassTest::stop_data()
{
    QTest::addColumn<int>("testDataCount");
    QTest::addColumn<int>("testDataInterval");
    QTest::addColumn<int>("updateInterval");
    QTest::addColumn<int>("updateLimit");

    QTest::newRow("stopUpdates() after 1 update")
            << 10 << 50
            << 50
            << 1;

    QTest::newRow("stopUpdates() after 1 update, zero interval")
            << 10 << 50
            << 0
            << 1;

    QTest::newRow("stopUpdates() after 2 updates")
            << 10 << 50
            << 50
            << 2;

    QTest::newRow("stopUpdates() after 2 update, zero interval")
            << 10 << 50
            << 0
            << 2;

    QTest::newRow("stopUpdates() after 10 updates")
            << 10 << 50
            << 50
            << 10;

    QTest::newRow("stopUpdates() after 10 updates, zero interval")
            << 10 << 50
            << 0
            << 10;
}

void QWhereaboutsSubclassTest::stop()
{
    QFETCH(int, testDataCount);
    QFETCH(int, testDataInterval);
    QFETCH(int, updateInterval);
    QFETCH(int, updateLimit);

    QSignalSpy spy(m_helper->whereabouts(), SIGNAL(updated(QWhereaboutsUpdate)));

    m_helper->whereabouts()->startUpdates(updateInterval);
    m_helper->feedTimedUpdates(testDataInterval, testDataCount);

    if (updateLimit == 0) {
        m_helper->whereabouts()->stopUpdates();
    } else {
        // when we have received a certain number of updates, call stopUpdates()
        QTRY_VERIFY(spy.count() == updateLimit); spy.clear();
        m_helper->whereabouts()->stopUpdates();
    }

    // check the updated() signal is not emitted again
    QTest::qWait(updateInterval * 2);
    QTRY_VERIFY(spy.count() == 0);
}

void QWhereaboutsSubclassTest::stop_restart()
{
    // receive 10 updates, then stopUpdates(), then startUpdates() again and receive
    // another 10 updates
    const int firstUpdateGroupCount = 10;
    const int ignoredUpdateCount = 10;
    const int secondUpdateCount = 10;
    const int dataInterval = 50;

    m_helper->feedTimedUpdates(dataInterval,
                firstUpdateGroupCount + ignoredUpdateCount + secondUpdateCount);

    QSignalSpy spy(m_helper->whereabouts(), SIGNAL(updated(QWhereaboutsUpdate)));
    m_helper->whereabouts()->startUpdates();

    QTRY_VERIFY(spy.count() == firstUpdateGroupCount); spy.clear();
    m_helper->whereabouts()->stopUpdates();

    QTest::qWait(dataInterval * ignoredUpdateCount);
    QCOMPARE(spy.count(), 0);
    m_helper->whereabouts()->startUpdates();

    QTRY_VERIFY(spy.count() == secondUpdateCount); spy.clear();
    m_helper->whereabouts()->stopUpdates();

    // check the updated() signal is not emitted again
    QTest::qWait(100);
    QTRY_VERIFY(spy.count() == 0);
}

void QWhereaboutsSubclassTest::requestUpdate_A()
{
    const int updateInterval = 10;
    QSignalSpy spy(m_helper->whereabouts(), SIGNAL(updated(QWhereaboutsUpdate)));

    // call requestUpdate() 10 times and check each update is received
    for (int i=0; i<10; i++) {
        if (m_helper->feedBeforeRequestUpdate()) {
            m_helper->feedTimedUpdates(updateInterval, 1);
            QTest::qWait(updateInterval * 2);
            m_helper->whereabouts()->requestUpdate();
        } else {
            m_helper->whereabouts()->requestUpdate();
            m_helper->feedTimedUpdates(updateInterval, 1);
        }
        QTRY_VERIFY(spy.count() == 1); spy.clear();
    }
}

void QWhereaboutsSubclassTest::requestUpdate_B()
{
    // Test requestUpdate() when used with startUpdates().
    // Call startUpdates() with 400 ms interval, then call requestUpdate() after
    // 100 ms and check that an update is received by the next available update
    // instead of having to wait for 400 ms, and check also that an update is
    // received at 400 ms as well.

    const int updateInterval = 400;
    const int requestUpdateDelay = 100;
    const int updateDataInterval = 50;

    QSignalSpy spy(m_helper->whereabouts(), SIGNAL(updated(QWhereaboutsUpdate)));
    QUpdateIntervalLogger *logger = new QUpdateIntervalLogger(m_helper->whereabouts());

    // feed at least enough updates to last until 400ms
    m_helper->feedTimedUpdates(updateDataInterval, 10);

    m_helper->whereabouts()->startUpdates(updateInterval);
    logger->startLogging();
    QTimer::singleShot(requestUpdateDelay, m_helper->whereabouts(), SLOT(requestUpdate()));

    // should receive updates for both the requestUpdate() and startUpdate() calls
    QTRY_VERIFY(spy.count() == 2);
    QList<int> expectedIntervals;
    expectedIntervals << 100 << 300;

    // add updateDataInterval to the error margin because the updates
    // don't arrive straight away
    logger->compareIntervals(expectedIntervals, m_helper->intervalErrorMargin() + updateDataInterval,
                          m_helper->isRealTime());

    delete logger;
}

void QWhereaboutsSubclassTest::requestUpdate_C()
{
    // stopUpdates() should not affect requestUpdate()

    QSignalSpy spy(m_helper->whereabouts(), SIGNAL(updated(QWhereaboutsUpdate)));

    m_helper->whereabouts()->startUpdates();
    m_helper->whereabouts()->stopUpdates();

    if (m_helper->feedBeforeRequestUpdate()) {
        m_helper->feedTimedUpdates(10, 1);
        QTest::qWait(20);
        m_helper->whereabouts()->requestUpdate();
    } else {
        m_helper->whereabouts()->requestUpdate();
        m_helper->feedTimedUpdates(10, 1);
    }

    QTRY_VERIFY(spy.count() == 1); spy.clear();
}


#include "qwhereaboutssubclasstest.moc"
