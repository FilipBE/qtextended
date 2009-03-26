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
#include "qdthread.h"

#include <trace.h>
QD_LOG_OPTION(QDThread)

/*!
  \class QDThread
  \brief The QDThread class extends QThread to include basic synchronization with the calling thread.

  QThread does not provide synchronization with the calling thread. The problem is that some objects
  need to be created on the new thread (due to Qt's threading rules) but need access to information
  that is not available until the thread is actually running. Another example is needing to do some
  on-thread setup before the calling thread continues.

  QDThread allows this by providing 2 methods to start the thread. init() brings the thread up and
  start() allows it to continue running. The t_init() function is called on-thread before init() returns.
  The t_run() function is called on the thread after start() is called. The t_quit() function is called
  after t_run() returns.
*/

/*!
  Construct a QDThread instance with \a parent.
*/
QDThread::QDThread( QObject *parent )
    : QThread( parent ), inited( false )
{
}

/*!
  Destructor.
*/
QDThread::~QDThread()
{
}

/*!
  Bring up the thread. This causes t_init() to be called on the new thread.
  The init() function will not return until t_init() has completed.
*/
void QDThread::init()
{
    TRACE(QDThread) << "QDThread::init";
    inited = true;
    waitForStartMutex.lock(); // Don't let t_run() get called until start() is called
    QThread::start();
    LOG() << "waiting for t_init()";
    waitForRunSemaphore.acquire(); // block until t_init() returns
}

/*!
  Start the thread. This causes t_run() to be called on the new thread.
  If the thread has not been created init() will be called to bring it up.
*/
void QDThread::start()
{
    TRACE(QDThread) << "QDThread::start";
    if ( !inited )
        init(); // does not return until t_init() has finished
    waitForStartMutex.unlock();
}

/*!
  \internal
*/
void QDThread::run()
{
    TRACE(QDThread) << "QDThread::run";
    t_init();
    LOG() << "let the main thread continue";
    waitForRunSemaphore.release(); // allow init() to exit
    LOG() << "wait for start()";
    waitForStartMutex.lock(); // this blocks until the mutex is unlocked
    LOG() << "waitForStartMutex unlocked";
    t_run();
    t_quit();
}

/*!
  On-thread initialization. Objects that need to live in the new thread should
  be constructed here.
*/
void QDThread::t_init()
{
}

/*!
  This is the main run method. The thread is stopped when this method finishes.
  The default implementation calls QThread::exec().
*/
void QDThread::t_run()
{
    TRACE(QDThread) << "QDThread::exec";
    exec();
}

/*!
  On-thread cleanup. Objects created in t_init() should be destroyed here.
  This is called immediatley after t_run() returns.
*/
void QDThread::t_quit()
{
}

