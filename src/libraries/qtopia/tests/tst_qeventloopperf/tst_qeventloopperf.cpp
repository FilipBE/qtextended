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

#include <QTest>
#include <QKeyEvent>
#include <QTimer>
#include <QLineEdit>
#include <QApplication>
#include <QtopiaApplication>
#include <QSocketNotifier>

#include <unistd.h>
#include <errno.h>
#include <shared/util.h>
#include <shared/qtopiaunittest.h>
#include <shared/perftest.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <time.h>

#ifdef USE_BENCHLIB
# include <qbenchmark.h>
# undef PERF_LOG2
# define PERF_LOG2(...)
#endif

class tst_QEventLoopPerf : public QObject
{
    Q_OBJECT

public:
    virtual bool event(QEvent*);

public slots:
    void onSocketNotifierActivated(int);

private slots:
    void postEvent();
    void socketNotifier();

    void postEvent_QKeyEvent_QLineEdit();
#if 0
    void timestamp();
#endif

private:
    qint64      m_totalTicks;
    qint64      m_totalNanoseconds;
    QEventLoop* m_loop;
};

//Leave the next line for Test Plan doc generation
//QTEST_MAIN(tst_QEventLoopPerf)

class tst_QtopiaEventLoopPerf : public tst_QEventLoopPerf
{
    Q_OBJECT
};

struct Timestamp
{
    time_t seconds;
    qint64 nanoseconds;

    clock_t userticks;
    clock_t systemticks;

    qint64 nanosecsTo(Timestamp const&);
    qint64 userticksTo(Timestamp const&);
    qint64 systemticksTo(Timestamp const&);
    qint64 ticksTo(Timestamp const&);

    static qint64 ticksPerSecond();
    static Timestamp now();
};

struct QTimestampEvent : public QEvent
{
    QTimestampEvent();

    Timestamp timestamp;

    static const int Type = QEvent::User + 1;
};

int run_qtopia_test(int argc, char** argv)
{
    tst_QtopiaEventLoopPerf test;
    QtopiaApplication app(argc, argv);
    return QTest::qExec(&test, argc, argv);
}

int run_qt_test(int argc, char** argv)
{
    tst_QEventLoopPerf test;
    QApplication app(argc, argv);
    return QTest::qExec(&test, argc, argv);
}

int main(int argc, char** argv)
{
    // For valgrind purposes, allow either test to be run explicitly
    if (qgetenv("ONLY_TEST") == "QTOPIA") {
        return run_qtopia_test(argc, argv);
    } else if (qgetenv("ONLY_TEST") == "QT") {
        return run_qt_test(argc, argv);
    }

    // Run the test once with QApplication, once with QtopiaApplication
    int child = fork();
    if (-1 == child)
        qFatal("fork() failed: %s", strerror(errno));

    if (0 == child) {
        return run_qtopia_test(argc, argv);
    } else {
        waitpid(child,0,0);
    }

    return run_qt_test(argc, argv);

}

bool tst_QEventLoopPerf::event(QEvent* e)
{
    Q_ASSERT(e->type() == QTimestampEvent::Type);

    Timestamp now = Timestamp::now();
    m_totalTicks       += static_cast<QTimestampEvent*>(e)->timestamp.userticksTo(now);
    m_totalNanoseconds += static_cast<QTimestampEvent*>(e)->timestamp.nanosecsTo(now);
    if (m_loop) m_loop->exit();
    return true;
}

void tst_QEventLoopPerf::postEvent()
{
    int i;
    const int MAX = 100000;
    m_totalTicks = 0;
    m_totalNanoseconds = 0;
    QEventLoop loop;
    m_loop = &loop;
#ifdef USE_BENCHLIB
    QBENCHMARK {
#else
    for (i = 0; i < MAX; ++i) {
#endif
        QCoreApplication::instance()->postEvent(this, new QTimestampEvent);
        loop.exec();
    }

    qLog(Autotest) << QString().sprintf("To send %d events took %f seconds CPU time, %f seconds real time", MAX, qreal(m_totalTicks)/qreal(Timestamp::ticksPerSecond()), qreal(m_totalNanoseconds)/1000000000.);

    QString id = qobject_cast<tst_QtopiaEventLoopPerf*>(this) ? "qtopia" : "qt";
    PERF_LOG2(qreal(m_totalTicks)*1000./qreal(Timestamp::ticksPerSecond()), id + "_postevent_cpu");
    PERF_LOG2(qreal(m_totalNanoseconds)/qreal(1000000.), id + "_postevent_real");
}

void tst_QEventLoopPerf::postEvent_QKeyEvent_QLineEdit()
{
    QLineEdit le;

    const int MAX = 7500;

    QEventLoop loop;
    QObject::connect(&le, SIGNAL(textChanged(QString)), &loop, SLOT(quit()));

    Timestamp before = Timestamp::now();
    Qt::Key key1 = Qt::Key_A;
    Qt::Key key2 = Qt::Key_Backspace;
    QString text1 = QString::fromLatin1("A");
    QString text2 = QString();

#ifdef USE_BENCHLIB
    QBENCHMARK {
#else
    for (int i = 0; i < MAX; ++i) {
#endif
        QCoreApplication::instance()->postEvent(&le, new QKeyEvent(QEvent::KeyPress,   key1, Qt::KeyboardModifiers(0), text1));
        QCoreApplication::instance()->postEvent(&le, new QKeyEvent(QEvent::KeyRelease, key1, Qt::KeyboardModifiers(0), text1));
        loop.exec();
        QCoreApplication::instance()->postEvent(&le, new QKeyEvent(QEvent::KeyPress,   key2, Qt::KeyboardModifiers(0), text2));
        QCoreApplication::instance()->postEvent(&le, new QKeyEvent(QEvent::KeyRelease, key2, Qt::KeyboardModifiers(0), text2));
        loop.exec();
    }

    Timestamp now = Timestamp::now();

    qint64 totalTicks       = before.userticksTo(now);
    qint64 totalNanoseconds = before.nanosecsTo(now);

    QString id = qobject_cast<tst_QtopiaEventLoopPerf*>(this) ? "qtopia" : "qt";
    PERF_LOG2(qreal(totalTicks)*1000./qreal(Timestamp::ticksPerSecond()), id + "_postevent_qkeyevent_qlineedit_cpu");
    PERF_LOG2(qreal(totalNanoseconds)/qreal(1000000.), id + "_postevent_qkeyevent_qlineedit_real");
}

void tst_QEventLoopPerf::onSocketNotifierActivated(int fd)
{
    Timestamp now = Timestamp::now();

    Timestamp then;
    if (sizeof(Timestamp) != read(fd, &then, sizeof(Timestamp)))
        qFatal("read() failed: %s", strerror(errno));

    m_totalTicks       += then.userticksTo(now);
    m_totalNanoseconds += then.nanosecsTo(now);

    if (m_loop) m_loop->exit();
}


void tst_QEventLoopPerf::socketNotifier()
{
    int fds[2];
    if (0 != pipe(fds))
        qFatal("pipe() failed: %s", strerror(errno));

    QSocketNotifier notifier(fds[0], QSocketNotifier::Read);
    QObject::connect(   &notifier, SIGNAL(activated(int)),
                        this,      SLOT(onSocketNotifierActivated(int)));

    const int MAX = 100000;
    m_totalTicks = 0;
    m_totalNanoseconds = 0;
    QEventLoop loop;
    m_loop = &loop;
#ifdef USE_BENCHLIB
    QBENCHMARK {
#else
    for (int i = 0; i < MAX; ++i) {
#endif
        Timestamp now = Timestamp::now();
        if (sizeof(Timestamp) != write(fds[1], &now, sizeof(Timestamp)))
            qFatal("write() failed: %s", strerror(errno));
        loop.exec();
    }

    qLog(Autotest) << QString().sprintf("To do %d socket notifier wakeups took %f seconds CPU time, %f seconds real time", MAX, qreal(m_totalTicks)/qreal(Timestamp::ticksPerSecond()), qreal(m_totalNanoseconds)/1000000000.);

    QString id = qobject_cast<tst_QtopiaEventLoopPerf*>(this) ? "qtopia" : "qt";
    PERF_LOG2(qreal(m_totalTicks)*1000./qreal(Timestamp::ticksPerSecond()), id + "_socketnotifier_cpu");
    PERF_LOG2(qreal(m_totalNanoseconds)/qreal(1000000.), id + "_socketnotifier_real");
}

#if 0
void tst_QEventLoopPerf::timestamp()
{
    const int MAX = 10000000;
    Timestamp before = Timestamp::now();
    for (int i = 0; i < MAX; ++i) {}
    Timestamp after  = Timestamp::now();
    m_totalTicks       = before.ticksTo(after);
    m_totalNanoseconds = before.nanosecsTo(after);
    qLog(Autotest) << QString().sprintf("To do %d loops took %f seconds CPU time, %f seconds real time", MAX, qreal(m_totalTicks)/qreal(Timestamp::ticksPerSecond()), qreal(m_totalNanoseconds)/1000000000.);
}
#endif

/*********************************************************************************/

#include <time.h>

Timestamp Timestamp::now()
{
    Timestamp ret;

    static struct timespec spec;
    /* Note, CLOCK_MONOTONIC would be better, but it does not work on the
     * Greenphone and probably other devices.
     */
    if (0 != clock_gettime(CLOCK_REALTIME, &spec))
        qFatal("clock_gettime(CLOCK_REALTIME,...) failed: %s", strerror(errno));
    ret.seconds     = spec.tv_sec;
    ret.nanoseconds = spec.tv_nsec;

    static struct tms times_buf;
    if (-1 == times(&times_buf))
        qFatal("times() failed: %s", strerror(errno));
    ret.userticks   = times_buf.tms_utime;
    ret.systemticks = times_buf.tms_stime;

    return ret;
}

/*
    Returns nanoseconds in real time from this timestamp to \a t0.
*/
qint64 Timestamp::nanosecsTo(Timestamp const& t0)
{
    return ((qint64)difftime(t0.seconds, seconds))*Q_INT64_C(1000000000) + (t0.nanoseconds - nanoseconds);
}

/*
    Returns total clock ticks spent on behalf of this process
    between this timestamp and \a t0.
*/
qint64 Timestamp::ticksTo(Timestamp const& t0)
{
    return t0.userticks + t0.systemticks - userticks - systemticks;
}

qint64 Timestamp::userticksTo(Timestamp const& t0)
{
    return t0.userticks - userticks;
}

qint64 Timestamp::systemticksTo(Timestamp const& t0)
{
    return t0.systemticks - systemticks;
}

qint64 Timestamp::ticksPerSecond()
{
    static long ret = sysconf(_SC_CLK_TCK);
    return ret;
}

QTimestampEvent::QTimestampEvent()
    :   QEvent(QEvent::Type(Type)),
        timestamp(Timestamp::now())
{}



#include "tst_qeventloopperf.moc"

