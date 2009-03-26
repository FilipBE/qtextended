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

#include <QTask>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QtopiaApplication>


//TESTED_CLASS=QTask
//TESTED_FILES=src/libraries/qtopiapim/qtask.h,src/libraries/qtopiapim/qtask.cpp

/*
    This class is a unit test for the QTask class.
*/
class tst_QTask : public QObject
{
    Q_OBJECT

public:
    tst_QTask();
    virtual ~tst_QTask();

private slots:
    void testSetStatus();
    void testSetPercent();
    void testSetStartedDate();
    void testSetCompletedDate();
    void testProgressEquality();
};

QTEST_APP_MAIN( tst_QTask, QtopiaApplication )
#include "tst_qtask.moc"


tst_QTask::tst_QTask()
{
}

tst_QTask::~tst_QTask()
{
}

/*?
  Try to test various ways of modifying the percentage
  completed of a task to make sure they result in a
  consistent state.  This includes checking the status,
  percent completed, start date, and end date.
*/
void tst_QTask::testSetPercent()
{
    QTask t;

    // Initial condition
    t.setStatus(QTask::NotStarted);
    QCOMPARE(t.status(), QTask::NotStarted);
    QCOMPARE(t.startedDate(), QDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 0U);

    // Go to 1 percent
    t.setPercentCompleted(1);
    QCOMPARE(t.status(), QTask::InProgress);
    QCOMPARE(t.startedDate(), QDate::currentDate()); // Don't run this at midnight.
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 1U);

    // Back to 0
    t.setPercentCompleted(0);
    QCOMPARE(t.status(), QTask::InProgress);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 0U);

    // Reset to initial
    t.setStatus(QTask::NotStarted);
    QCOMPARE(t.status(), QTask::NotStarted);
    QCOMPARE(t.startedDate(), QDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 0U);

    // Go to 100 percent
    t.setPercentCompleted(100);
    QCOMPARE(t.status(), QTask::Completed);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate::currentDate());
    QCOMPARE(t.percentCompleted(), 100U);

    // Back to 99 percent
    t.setPercentCompleted(99);
    QCOMPARE(t.status(), QTask::InProgress);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 99U);
}

/*?
  Try to test various ways of modifying the start date
  of a task to make sure they result in a
  consistent state.  This includes checking the status,
  percent completed, start date, and end date.
*/
void tst_QTask::testSetStartedDate()
{
    QTask t;

    // Initial condition
    t.setStatus(QTask::NotStarted);
    QCOMPARE(t.status(), QTask::NotStarted);
    QCOMPARE(t.startedDate(), QDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 0U);

    // Add a start date
    t.setStartedDate(QDate::currentDate());
    QCOMPARE(t.status(), QTask::InProgress);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 0U);

    // Back to the starting condition
    t.setStartedDate(QDate());
    QCOMPARE(t.status(), QTask::NotStarted);
    QCOMPARE(t.startedDate(), QDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 0U);


    // Set a completed date...
    t.setCompletedDate(QDate::currentDate());
    QCOMPARE(t.status(), QTask::Completed);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate::currentDate());
    QCOMPARE(t.percentCompleted(), 100U);

    // And now clear the started date
    t.setStartedDate(QDate());
    QCOMPARE(t.status(), QTask::NotStarted);
    QCOMPARE(t.startedDate(), QDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 0U);
}

/*?
  Try to test various ways of modifying the start date
  of a task to make sure they result in a
  consistent state.  This includes checking the status,
  percent completed, start date, and end date.
*/
void tst_QTask::testSetCompletedDate()
{
    QTask t;

    // Initial condition
    t.setStatus(QTask::NotStarted);
    QCOMPARE(t.status(), QTask::NotStarted);
    QCOMPARE(t.startedDate(), QDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 0U);

    // Add a completed date
    t.setCompletedDate(QDate::currentDate());
    QCOMPARE(t.status(), QTask::Completed);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate::currentDate());
    QCOMPARE(t.percentCompleted(), 100U);

    // Clear the completed date
    t.setCompletedDate(QDate());
    QCOMPARE(t.status(), QTask::InProgress);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 99U);

    // Add a completed date again
    t.setCompletedDate(QDate::currentDate());
    QCOMPARE(t.status(), QTask::Completed);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate::currentDate());
    QCOMPARE(t.percentCompleted(), 100U);

}

/*?
  Set various statuses and check the other fields match up
*/
void tst_QTask::testSetStatus()
{
    QTask t;

    // Initial condition
    t.setStatus(QTask::NotStarted);
    QCOMPARE(t.status(), QTask::NotStarted);
    QCOMPARE(t.startedDate(), QDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 0U);

    // Set in progress
    t.setStatus(QTask::InProgress);
    QCOMPARE(t.status(), QTask::InProgress);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 0U);

    // Set complete
    t.setStatus(QTask::Completed);
    QCOMPARE(t.status(), QTask::Completed);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate::currentDate());
    QCOMPARE(t.percentCompleted(), 100U);

    // Set in progress again
    t.setStatus(QTask::InProgress);
    QCOMPARE(t.status(), QTask::InProgress);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 99U);

    // Set complete again
    t.setStatus(QTask::Completed);
    QCOMPARE(t.status(), QTask::Completed);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate::currentDate());
    QCOMPARE(t.percentCompleted(), 100U);

    // Set waiting
    t.setStatus(QTask::Waiting);
    QCOMPARE(t.status(), QTask::Waiting);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 99U);

    // Set complete again again
    t.setStatus(QTask::Completed);
    QCOMPARE(t.status(), QTask::Completed);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate::currentDate());
    QCOMPARE(t.percentCompleted(), 100U);

    // Set deferred
    t.setStatus(QTask::Deferred);
    QCOMPARE(t.status(), QTask::Deferred);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 99U);

    // back to start
    t.setStatus(QTask::NotStarted);
    QCOMPARE(t.status(), QTask::NotStarted);
    QCOMPARE(t.startedDate(), QDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 0U);

    // Set waiting (again)
    t.setStatus(QTask::Waiting);
    QCOMPARE(t.status(), QTask::Waiting);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 0U);

    // back to start
    t.setStatus(QTask::NotStarted);
    QCOMPARE(t.status(), QTask::NotStarted);
    QCOMPARE(t.startedDate(), QDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 0U);

    // Set deferred again
    t.setStatus(QTask::Deferred);
    QCOMPARE(t.status(), QTask::Deferred);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 0U);

    // back to start
    t.setStatus(QTask::NotStarted);
    QCOMPARE(t.status(), QTask::NotStarted);
    QCOMPARE(t.startedDate(), QDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 0U);

    // Set deferred again again
    t.setStatus(QTask::Deferred);
    QCOMPARE(t.status(), QTask::Deferred);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 0U);

    // Set waiting again again
    t.setStatus(QTask::Waiting);
    QCOMPARE(t.status(), QTask::Waiting);
    QCOMPARE(t.startedDate(), QDate::currentDate());
    QCOMPARE(t.completedDate(), QDate());
    QCOMPARE(t.percentCompleted(), 0U);

}


/*?
  Create pairs of tasks and manipulate them
  to the same logical state (e.g. completed) via
  different methods, and make sure they compare
  equally
*/
void tst_QTask::testProgressEquality()
{
    QTask a, b;

    QCOMPARE(a,b);
    a.setStatus(QTask::NotStarted);

    QCOMPARE(a,b);
    b.setStatus(QTask::NotStarted);

    // no real changes yet
    QCOMPARE(a,b);

    // Percent and Status comparisons
    a.setPercentCompleted(0);
    b.setStatus(QTask::NotStarted);
    QCOMPARE(a,b);

    a.setPercentCompleted(100);
    b.setStatus(QTask::Completed);
    QCOMPARE(a,b);

    a.setPercentCompleted(99);
    b.setStatus(QTask::InProgress);
    QCOMPARE(a,b);

    a.setPercentCompleted(100);
    b.setStatus(QTask::Completed);
    QCOMPARE(a,b);

    // Percent and start date comparisons

    // back to start
    a.setStatus(QTask::NotStarted);
    b.setStatus(QTask::NotStarted);
    QCOMPARE(a,b);

    a.setPercentCompleted(1);
    a.setPercentCompleted(0);
    b.setStartedDate(QDate::currentDate()); // started date doesn't change %
    QCOMPARE(a,b);

    a.setStatus(QTask::NotStarted);
    b.setStartedDate(QDate());
    QCOMPARE(a,b);

    // Completion date comparisons
    // back to start
    a.setStatus(QTask::NotStarted);
    b.setStatus(QTask::NotStarted);
    QCOMPARE(a,b);

    a.setPercentCompleted(100);
    b.setCompletedDate(QDate::currentDate());
    QCOMPARE(a,b);

    a.setPercentCompleted(99);
    b.setCompletedDate(QDate());
    QCOMPARE(a,b);
}
