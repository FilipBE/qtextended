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

#include "qtconcurrentthreadengine.h"

#ifndef QT_NO_CONCURRENT

QT_BEGIN_NAMESPACE

namespace QtConcurrent {

ThreadEngineBase::ThreadEngineBase()
:futureInterface(0), threadPool(QThreadPool::globalInstance())
{
    setAutoDelete(false);
}

ThreadEngineBase::~ThreadEngineBase() {}

void ThreadEngineBase::startSingleThreaded()
{
    start();
    while (threadFunction() != ThreadFinished)
        ;
    finish();
}

void ThreadEngineBase::startBlocking()
{
    start();
    semaphore.acquire();
    startThreads();

    bool throttled = false;
#ifndef QT_NO_EXCEPTIONS
    try {
#endif
        while (threadFunction() == ThrottleThread) {
            if (threadThrottleExit()) {
                throttled = true;
                break;
            }
        }
#ifndef QT_NO_EXCEPTIONS
    } catch (QtConcurrent::Exception &e) {
        handleException(e);
    } catch (...) {
        handleException(QtConcurrent::UnhandledException());
    }
#endif

    if (throttled == false) {
        semaphore.release();
    }

    semaphore.wait();
    finish();
    exceptionStore.throwPossibleException();
}

void ThreadEngineBase::startThread()
{
    startThreadInternal();
}

bool ThreadEngineBase::isCanceled()
{
    if (futureInterface)
        return futureInterface->isCanceled();
    else
        return false;
}

bool ThreadEngineBase::isProgressReportingEnabled()
{
    // If we don't have a QFuture, there is no-one to report the progress to.
    return (futureInterface != 0);
}

void ThreadEngineBase::setProgressValue(int progress)
{
    if (futureInterface)
        futureInterface->setProgressValue(progress);
}

void ThreadEngineBase::setProgressRange(int minimum, int maximum)
{
    if (futureInterface)
        futureInterface->setProgressRange(minimum, maximum);
}

bool ThreadEngineBase::startThreadInternal()
{
    if (this->isCanceled())
        return false;

    semaphore.acquire();
    if (!threadPool->tryStart(this)) {
        semaphore.release();
        return false;
    }
    return true;
}

void ThreadEngineBase::startThreads()
{
    while (shouldStartThread() && startThreadInternal())
        ;
}

void ThreadEngineBase::threadExit()
{
    const bool asynchronous = futureInterface != 0;
    const int lastThread = (semaphore.release() == 0);

    if (lastThread && asynchronous)
        this->asynchronousFinish();
}

// Called by a worker thread that wants to be throttled. If the current number
// of running threads is larger than one the thread is allowed to exit and
// this function returns one.
bool ThreadEngineBase::threadThrottleExit()
{
    return semaphore.releaseUnlessLast();
}

void ThreadEngineBase::run() // implements QRunnable.
{
    if (this->isCanceled()) {
        threadExit();
        return;
    }

    startThreads();

#ifndef QT_NO_EXCEPTIONS
    try {
#endif
        while (threadFunction() == ThrottleThread) {
            // threadFunction returning ThrottleThread means it that the user
            // struct wants to be throttled by making a worker thread exit.
            // Respect that request unless this is the only worker thread left
            // running, in which case it has to keep going.
            if (threadThrottleExit())
                return;
        }

#ifndef QT_NO_EXCEPTIONS
    } catch (QtConcurrent::Exception &e) {
        handleException(e);
    } catch (...) {
        handleException(QtConcurrent::UnhandledException());
    }
#endif
    threadExit();
}

#ifndef QT_NO_EXCEPTIONS

void ThreadEngineBase::handleException(const QtConcurrent::Exception &exception)
{
    if (futureInterface)
        futureInterface->reportException(exception);
    else
        exceptionStore.setException(exception);
}
#endif


} // namepsace QtConcurrent

QT_END_NAMESPACE

#endif // QT_NO_CONCURRENT
