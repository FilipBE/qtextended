/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qwaitcondition.h"
#include "qnamespace.h"
#include "qmutex.h"
#include "qreadwritelock.h"
#include "qlist.h"
#include "qalgorithms.h"
#include "qt_windows.h"

#ifndef QT_NO_THREAD

#define Q_MUTEX_T void*
#include <private/qmutex_p.h>
#include <private/qreadwritelock_p.h>

QT_BEGIN_NAMESPACE

//***********************************************************************
// QWaitConditionPrivate
// **********************************************************************

class QWaitConditionEvent
{
public:
    inline QWaitConditionEvent() : priority(0), wokenUp(false)
    {
        QT_WA ({
            event = CreateEvent(NULL, TRUE, FALSE, NULL);
        }, {
            event = CreateEventA(NULL, TRUE, FALSE, NULL);
        });
    }
    inline ~QWaitConditionEvent() { CloseHandle(event); }
    int priority;
    bool wokenUp;
    HANDLE event;
};

typedef QList<QWaitConditionEvent *> EventQueue;

class QWaitConditionPrivate
{
public:
    QMutex mtx;
    EventQueue queue;
    EventQueue freeQueue;

    QWaitConditionEvent *pre();
    bool wait(QWaitConditionEvent *wce, unsigned long time);
    void post(QWaitConditionEvent *wce, bool ret);
};

QWaitConditionEvent *QWaitConditionPrivate::pre()
{
    mtx.lock();
    QWaitConditionEvent *wce =
        freeQueue.isEmpty() ? new QWaitConditionEvent : freeQueue.takeFirst();
    wce->priority = GetThreadPriority(GetCurrentThread());
    wce->wokenUp = false;

    // insert 'wce' into the queue (sorted by priority)
    int index = 0;
    for (; index < queue.size(); ++index) {
        QWaitConditionEvent *current = queue.at(index);
        if (current->priority < wce->priority)
            break;
    }
    queue.insert(index, wce);
    mtx.unlock();

    return wce;
}

bool QWaitConditionPrivate::wait(QWaitConditionEvent *wce, unsigned long time)
{
    // wait for the event
    bool ret = false;
    switch (WaitForSingleObject(wce->event, time)) {
    default: break;

    case WAIT_OBJECT_0:
        ret = true;
        break;
    }
    return ret;
}

void QWaitConditionPrivate::post(QWaitConditionEvent *wce, bool ret)
{
    mtx.lock();

    // remove 'wce' from the queue
    queue.removeAll(wce);
    ResetEvent(wce->event);
    freeQueue.append(wce);

    // wakeups delivered after the timeout should be forwarded to the next waiter
    if (!ret && wce->wokenUp && !queue.isEmpty()) {
        QWaitConditionEvent *other = queue.first();
        SetEvent(other->event);
        other->wokenUp = true;
    }

    mtx.unlock();
}

//***********************************************************************
// QWaitCondition implementation
//***********************************************************************

QWaitCondition::QWaitCondition()
{
    d = new QWaitConditionPrivate;
}

QWaitCondition::~QWaitCondition()
{
    if (!d->queue.isEmpty()) {
        qWarning("QWaitCondition: Destroyed while threads are still waiting");
        qDeleteAll(d->queue);
    }

    qDeleteAll(d->freeQueue);
    delete d;
}

bool QWaitCondition::wait(QMutex *mutex, unsigned long time)
{
    if (!mutex)
        return false;
    if (mutex->d->recursive) {
        qWarning("QWaitCondition::wait: Cannot wait on recursive mutexes");
        return false;
    }

    QWaitConditionEvent *wce = d->pre();
    mutex->unlock();

    bool returnValue = d->wait(wce, time);

    mutex->lock();
    d->post(wce, returnValue);

    return returnValue;
}

bool QWaitCondition::wait(QReadWriteLock *readWriteLock, unsigned long time)
{
    if (!readWriteLock || readWriteLock->d->accessCount == 0)
        return false;
    if (readWriteLock->d->accessCount < -1) {
        qWarning("QWaitCondition: cannot wait on QReadWriteLocks with recursive lockForWrite()");
        return false;
    }

    QWaitConditionEvent *wce = d->pre();
    int previousAccessCount = readWriteLock->d->accessCount;
    readWriteLock->unlock();

    bool returnValue = d->wait(wce, time);

    if (previousAccessCount < 0)
        readWriteLock->lockForWrite();
    else
        readWriteLock->lockForRead();
    d->post(wce, returnValue);

    return returnValue;
}

void QWaitCondition::wakeOne()
{
    // wake up the first waiting thread in the queue
    QMutexLocker locker(&d->mtx);
    for (int i = 0; i < d->queue.size(); ++i) {
        QWaitConditionEvent *current = d->queue.at(i);
        if (current->wokenUp)
            continue;
        SetEvent(current->event);
        current->wokenUp = true;
        break;
    }
}

void QWaitCondition::wakeAll()
{
    // wake up the all threads in the queue
    QMutexLocker locker(&d->mtx);
    for (int i = 0; i < d->queue.size(); ++i) {
        QWaitConditionEvent *current = d->queue.at(i);
        SetEvent(current->event);
        current->wokenUp = true;
    }
}

QT_END_NAMESPACE
#endif // QT_NO_THREAD
