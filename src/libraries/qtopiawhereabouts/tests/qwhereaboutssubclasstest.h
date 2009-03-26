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

#ifndef QWHEREABOUTSTEST_H
#define QWHEREABOUTSTEST_H

#include <QObject>
#include <QList>
#include <QDateTime>
#include <QListIterator>

class QWhereabouts;
class QTimer;
class QBasicTimer;
class QTimerEvent;

class QWhereaboutsTestHelper : public QObject
{
    Q_OBJECT
public:
    QWhereaboutsTestHelper(int updateIntervalErrorMargin, QObject *parent = 0);
    ~QWhereaboutsTestHelper();

    virtual void initTest() {}
    virtual void cleanupTest() {}
    virtual void flush() {}

    virtual QWhereabouts *whereabouts() const = 0;

    // calls feedTestUpdate() for each update
    virtual void feedTimedUpdates(const QList<int> &intervals,
                                  const QList<QDateTime> &dateTimes = QList<QDateTime>());
    void feedTimedUpdates(int interval, int count);

    virtual bool feedBeforeRequestUpdate() const = 0;
    virtual inline bool isRealTime() const { return true; }
    inline int intervalErrorMargin() const { return m_intervalErrorMargin; }

protected:
    virtual void feedTestUpdate(const QDateTime &dt) = 0;

    virtual void timerEvent(QTimerEvent *);

private slots:
    void nextUpdate();

private:
    int m_intervalErrorMargin;
    QList<int> m_updateIntervals;
    QList<QDateTime> m_updateTimes;
    QBasicTimer *m_timer;
};


class QWhereaboutsSubclassTest : public QObject
{
    Q_OBJECT
public:
    QWhereaboutsSubclassTest(QWhereaboutsTestHelper *helper, QObject *parent = 0);

private slots:
    void init();
    void cleanup();

    void start_zeroInterval();

    void start_nonZeroInterval_data();
    void start_nonZeroInterval();

    void start_delayed();

    void start_restart();

    void stop_data();
    void stop();

    void stop_restart();

    void requestUpdate_A();
    void requestUpdate_B();
    void requestUpdate_C();

private:
    QWhereaboutsTestHelper *m_helper;
};


#endif
