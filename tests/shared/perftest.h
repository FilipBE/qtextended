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

#ifndef PERFTEST_H
#define PERFTEST_H

#include <sys/time.h>

/*!
    \file perftest.h
    Macros to aid in writing performance tests.

    To write a performance unit test, #include "perftest.h" and use the macros
    defined within.

    Example:
    \code
        // Test how long it takes to construct an orphan QLabel and reparent it.
        void perf_QLabel::constructor_reparent()
        {
            QFETCH(QString, text);
            QWidget widget;

            QLabel *label = 0;

            // Begin the block of code to be tested, and run it 100 times.
            PERF_BEGIN_TASK(100) {
                // Construct a QLabel and reparent it.
                label = new QLabel(text);
                label->setParent(&widget);

                // Since we aren't testing the performance of the destructor,
                // delete the label inside of a skipped block.
                PERF_BEGIN_SKIP() {
                    delete label;
                } PERF_END_SKIP();

            // End the tested block of code.
            } PERF_END_TASK();

            // Output a log message, also showing the length of the text in the
            // QLabel, since it clearly may have an impact on performance.
            PERF_LOG(QString("string length %1").arg(text.size()));

            // If it takes more than 1000.0 microseconds on average to construct
            // and reparent a QLabel, consider it a failure.  Otherwise the
            // test succeeds.
            if (PERF_MICROSECONDS_PER_TASK() > 1000.0) {
                QFAIL("QLabel is too slow!");
            }
        }
    \endcode
*/

/*!
    \macro PERF_BEGIN_TASK(num)

    Begin a task specification for a performance task to be repeated \a num times.
    A "performance task" is a block of code which will be run many times with
    its execution time recorded.

    Each test function should contain at most one performance task.

    \sa PERF_END_TASK()
*/
#define PERF_BEGIN_TASK(num)\
    int _TIMES = (num);\
    struct timeval _before, _after, _beforeSkip, _afterSkip;\
    QVERIFY2( 0 == gettimeofday( &_before, 0 ), "Couldn't get time before beginning task!" );\
    qulonglong _sub_microseconds = 0;\
    for (int _i = 0; _i < _TIMES; ++_i)

/*!
    \macro PERF_END_TASK()

    End a task specification for a performance task.
    A "performance task" is a block of code which will be run many times with
    its execution time recorded.

    \sa PERF_BEGIN_TASK(num)
*/
#define PERF_END_TASK() \
    QVERIFY2( 0 == gettimeofday( &_after, 0 ), "Couldn't get time after task!" );\
    qulonglong _total_microseconds = _after.tv_usec + _after.tv_sec*1000000 - (_before.tv_usec + _before.tv_sec*1000000) - _sub_microseconds; \
    double _total_seconds = ((double)_total_microseconds)/1000000.0;

/*!
    \macro PERF_BEGIN_SKIP()

    Begin a block of code which should not be included in the execution time of
    the current performance task.

    \sa PERF_END_SKIP()
*/
#define PERF_BEGIN_SKIP() {\
    QVERIFY2( 0 == gettimeofday( &_beforeSkip, 0 ), "Couldn't get time before skipped section!" );

/*!
    \macro PERF_END_SKIP()

    End a block of code begun with PERF_BEGIN_SKIP().

    \sa PERF_BEGIN_SKIP()
*/
#define PERF_END_SKIP() \
    QVERIFY2( 0 == gettimeofday( &_afterSkip, 0 ), "Couldn't get time at end of skipped section!" );\
    _sub_microseconds += _afterSkip.tv_usec + _afterSkip.tv_sec*1000000 - (_beforeSkip.tv_usec + _beforeSkip.tv_sec*1000000);\
}

/*!
    \macro PERF_TOTAL_SECONDS()
    Returns the total amount of seconds required for the last performance task, as a double.
*/
#define PERF_TOTAL_SECONDS (_total_seconds)

/*!
    \macro PERF_TOTAL_MICROSECONDS()
    Returns the total amount of microseconds required for the last performance task, as an integer.
*/
#define PERF_TOTAL_MICROSECONDS (_total_microseconds)

/*!
    \macro PERF_SECONDS_PER_TASK()
    Returns the average amount of seconds required for each execution of the last performance task, as a double.
*/
#define PERF_SECONDS_PER_TASK (_total_seconds/((double)_TIMES))

/*!
    \macro PERF_MICROSECONDS_PER_TASK()
    Returns the average amount of microseconds required for each execution of the last performance task, as a double.
*/
#define PERF_MICROSECONDS_PER_TASK (((double)_total_microseconds)/((double)_TIMES))

/*!
    \macro PERF_TASKS()
    Returns the amount of times the last performance task was executed.
*/
#define PERF_TASKS (_TIMES)

/*!
    \macro PERF_LOG(str)

    Outputs a log message showing all information about the most recent test.
    \a str is a string to show at the beginning of the log message; it can be empty.
*/
#define PERF_LOG(str) {\
    QString __ = QString("%1").arg(str).remove(':').remove(',').trimmed();\
    if (__.isEmpty()) __ = "perftest";\
    qDebug("%s: %d tasks, %f seconds total, %f microseconds per task", qPrintable(__), PERF_TASKS, PERF_TOTAL_SECONDS, PERF_MICROSECONDS_PER_TASK );\
}

namespace Perftest
{
    template <typename T>
    inline void perflog(T const& value, QString const& id = QString(), QString const& app = QString(), QString const& desc = QString())
    {
        QString lId(id);
        if (lId.isEmpty()) {
            lId = QString("%1_%2")
                .arg(QTest::testObject()->metaObject()->className())
                .arg(QTest::currentTestFunction());
        }
        lId = lId.toLower();
        lId.replace(QRegExp("[^a-z0-9_]"),"_");
        QString logMsg = QString("QTOPIAPERF(%1%2) : %3%4")
            .arg(lId)
            .arg(app.isEmpty() ? QString() : (":" + app))
            .arg(value)
            .arg(desc.isEmpty() ? QString() : (" : " + desc));
        QDebug(QtDebugMsg) << qPrintable(logMsg);
    }
}

/*!
    \macro PERF_LOG2(value,id,app,desc)

    Outputs a log message with the given \a value for the performance metric
    identified by \a id.  \a app specifies the application this value applies to.
    \a desc is an additional descriptive message to append to the log.

    \a id, \a app and \a desc are optional.
    If \a id is omitted, the id will be based on the currently executing testfunction.
*/
#define PERF_LOG2(...) Perftest::perflog(__VA_ARGS__)

#endif

