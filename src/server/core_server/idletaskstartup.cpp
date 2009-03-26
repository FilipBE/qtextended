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

#include "idletaskstartup.h"

#include <QAbstractEventDispatcher>

#ifdef Q_WS_QWS
#include <QWSEvent>
#endif

#ifdef Q_WS_X11
#include "windowmanagement.h"
#include <X11/Xlib.h>
#endif

struct IdleTaskStartupPrivate
{
    static bool eventFilter(void*);

    static void installEventFilter(QTimer*);
    static void removeEventFilter (QTimer*);

    static bool                                     s_installed;
    static QAbstractEventDispatcher::EventFilter    s_oldEventFilter;
    static QList<QTimer*>                           s_idleTimers;
};

/*!
    \class IdleTaskStartup
    \inpublicgroup QtBaseModule
    \ingroup QtopiaServer
    \ingroup QtopiaServer::Task

    \brief  The IdleTaskStartup class provides an interface for launching server
            tasks while the system is idle.

    IdleTaskStartup takes a list of server tasks to be started.  The tasks will
    be launched either when the system is idle or when a specified maximum timeout
    is reached.

    The system is considered idle when no user-generated events (such as keyboard/mouse
    input events) have occurred for a period of time and there are no active phone calls.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended
    applications.
*/

/*!
    \fn IdleTaskStartup::finished()

    This signal is emitted after this IdleTaskStartup has launched all its specified tasks.
*/

/*!
    Constructs the IdleTaskStartup.

    After start() is called, \a tasks will be launched through the interface provided by \a qst.
    The system is considered idle if no user-generated events have occurred within \a idleTimeout
    milliseconds.

    After \a maximumTimeout milliseconds have elapsed, all remaining tasks will be launched.
    If many tasks have not been started by this time, the device may temporarily become
    unresponsive.
*/
IdleTaskStartup::IdleTaskStartup(QtopiaServerTasksPrivate* qst,
        QList<QtopiaServerTasksPrivate::Task*> const& tasks, int idleTimeout, int maximumTimeout)
    :     m_qst(qst)
        , m_tasks(tasks)
        , m_idleTimer()
        , m_maximumTimer()
        , m_activeCalls("Communications/Calls/ActiveCalls")
        , m_modemStatus("Telephony/Status/ModemStatus")
{
    m_idleTimer.setInterval(idleTimeout);
    m_maximumTimer.setInterval(maximumTimeout);

    m_idleTimer.setObjectName("IdleTaskStartup_idleTimer");
    m_idleTimer.setSingleShot(true);
    if (!connect(&m_idleTimer, SIGNAL(timeout()), this, SLOT(onIdleTimeout())))
        Q_ASSERT(0);

    m_maximumTimer.setObjectName("IdleTaskStartup_maximumTimer");
    m_maximumTimer.setSingleShot(true);
    if (!connect(&m_maximumTimer, SIGNAL(timeout()), this, SLOT(onMaximumTimeout())))
        Q_ASSERT(0);

    IdleTaskStartupPrivate::installEventFilter(&m_idleTimer);
}

/*!
    Destroys the IdleTaskStartup.
*/
IdleTaskStartup::~IdleTaskStartup()
{
    IdleTaskStartupPrivate::removeEventFilter(&m_idleTimer);
}

/*!
    Start launching tasks when idle.
*/
void IdleTaskStartup::start()
{
    qLog(QtopiaServer) << "IdleTaskStartup will start" << m_tasks.count() << "tasks;"
        << "system is idle after" << m_idleTimer.interval() << "ms inactivity;"
        << "tasks will be started after" << m_maximumTimer.interval() << "ms at the latest.";

    if (!m_tasks.count()) {
        emit finished();
        return;
    }

    m_idleTimer.start();
    m_maximumTimer.start();
}

/*!
    Called when the system appears to be idle.
*/
void IdleTaskStartup::onIdleTimeout()
{
    if (!m_tasks.count()) return;

    // If we aren't idle, check again after the idle timeout.
    if (!idle()) {
        m_idleTimer.start();
        return;
    }

    startNextTask();

    /*
        If there are more tasks to start, give the system a chance to process events,
        then check if we are still idle.
    */
    if (m_tasks.count()) {
        QTimer::singleShot(0, this, SLOT(onIdleTimeout()));
    }
}

/*!
    Called when the maximum timeout passed to start() occurs.
    This function will block while starting any remaining tasks.
*/
void IdleTaskStartup::onMaximumTimeout()
{
    if (!m_tasks.count()) return;

    /*
        If we are currently on a call, starting up the remaining tasks could be undesirable...
        Try again in a little while.
    */
    if (m_activeCalls.value().toInt()) {
        QTimer::singleShot(m_idleTimer.interval(), this, SLOT(onMaximumTimeout()));
        return;
    }

    qLog(QtopiaServer) << "IdleTaskStartup: maximum timeout reached, all remaining tasks will "
        "be launched now.";
    while (m_tasks.count())
        startNextTask();
}

/*!
    Returns true if the system currently appears to be idle.
*/
bool IdleTaskStartup::idle() const
{
    if (m_idleTimer.isActive() || (m_activeCalls.value().toInt() != 0))
        return false;

    /*
        For these modem states, it's best to consider us not idle.
        Otherwise, by starting up background tasks, we are likely to delay the user
        being able to make a call.
    */
    static QStringList const BusyModemStatus = QStringList()
        << "Initializing"
        << "Initializing2"
    ;
    QString modemStatus = m_modemStatus.value().toString();
    if (BusyModemStatus.contains(modemStatus))
        return false;

#ifdef Q_WS_X11
    /*
        FIXME: on X11, we consider the system not idle if any app other than qpe has focus.
        We do this because eventFilter stops getting key/mouse events as soon as qpe loses
        focus.  This fails to start tasks in the case where the user starts an app soon
        after boot and doesn't interact with it.
    */
    if (WindowManagement::activeAppName() != QLatin1String("qpe"))
        return false;
#endif

    return true;
}

/*!
    Starts the next task.
    If the last task is started, finished() is emitted.
*/
void IdleTaskStartup::startNextTask()
{
    if (m_tasks.count() == 0) return;

    // Find the first task which hasn't been started already
    QtopiaServerTasksPrivate::Task* task = 0;
    do {
        task = m_tasks.takeFirst();
        // launchOrder > 0 indicates task has been started
        if (task && task->launchOrder > 0) task = 0;
    } while (m_tasks.count() && !task);

    // Did we find a task?
    if (task) {
        qLog(QtopiaServer) << "(idle) Starting task" << task->name.toByteArray();
        m_qst->startTask(task, false);
    }

    // Did we start the last task?
    if (m_tasks.count() == 0) {
        emit finished();
    }
}

/*************************************************************************************************/

bool                                    IdleTaskStartupPrivate::s_installed(false);
QAbstractEventDispatcher::EventFilter   IdleTaskStartupPrivate::s_oldEventFilter(0);
QList<QTimer*>                          IdleTaskStartupPrivate::s_idleTimers;

bool IdleTaskStartupPrivate::eventFilter(void* raw)
{
    bool isUserInput = false;

#ifdef Q_WS_QWS
    QWSEvent* event = reinterpret_cast<QWSEvent*>(raw);

    switch (event->type) {
        case QWSEvent::Mouse:
        case QWSEvent::Key:
            isUserInput = true;
        default:
            break;
    }
#endif


#ifdef Q_WS_X11
    XEvent* event = reinterpret_cast<XEvent*>(raw);
    switch (event->type) {
        case ButtonPress:
        case ButtonRelease:
        case MotionNotify:
        case KeyPress:
        case KeyRelease:
        case EnterNotify:
        case LeaveNotify:
            isUserInput = true;
        default:
            break;
    }
#endif

    if (isUserInput) {
        foreach (QTimer* timer, s_idleTimers) {
            timer->start();
        }
    }

    return false;
}

void IdleTaskStartupPrivate::installEventFilter(QTimer* timer)
{
    if (!s_installed) {
        s_installed = true;
        s_oldEventFilter = QAbstractEventDispatcher::instance()->setEventFilter(eventFilter);
    }

    s_idleTimers << timer;
}

void IdleTaskStartupPrivate::removeEventFilter(QTimer* timer)
{
    s_idleTimers.removeAll(timer);
    if (!s_idleTimers.count()) {
        QAbstractEventDispatcher::instance()->setEventFilter(s_oldEventFilter);
        s_installed = false;
    }
}

