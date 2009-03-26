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

#include <QtopiaIpcEnvelope>
#include "applicationshutdowntask.h"
#include "applicationlauncher.h"


/*!
  \class ApplicationShutdownTask
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::Task
  \ingroup QtopiaServer::AppLaunch
  \brief The ApplicationShutdownTask class provides a way of terminating all running Qt Extended applications before
  the Qt Extended server itself terminates.

  This shutdown handler instance is responsible for the proper termination of all running Qt Extended applications.
  Usually this task is only performed when the user requests a reboot or restart of the entire
  Qt Extended system. 
*/

/*!
  Creates a ApplicationShutdownTask instance.
  */
ApplicationShutdownTask::ApplicationShutdownTask()
: SystemShutdownHandler(), m_state(NoShutdown), m_launcher(0)
{
    m_launcher = qtopiaTask<ApplicationLauncher>();
    if(!m_launcher) return;
    
    m_timer.setInterval(3000);
    m_timer.setSingleShot(true);
    QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    QObject::connect(m_launcher, SIGNAL(applicationTerminated(QString,ApplicationTypeLauncher::TerminationReason,bool)), this, SLOT(terminated()));
}

/*!
  \reimp
  */
bool ApplicationShutdownTask::systemRestart()
{
    return doShutdown();
}

/*!
  \reimp
  */
bool ApplicationShutdownTask::systemShutdown()
{
    return doShutdown();
}

/*!
  \internal
  */
bool ApplicationShutdownTask::doShutdown()
{
    if(!m_launcher || m_state != NoShutdown) return true;

    if(!allAppsQuit()) {
        QtopiaIpcEnvelope env("QPE/System", "quit()");
        m_timer.start();
        m_state = WaitingForShutdown;
        return false;
    } else {
        m_state = Shutdown;
        return true;
    }
}

/*!
  \internal
  */
void ApplicationShutdownTask::killAll()
{
    QStringList apps = m_launcher->applications();
    for(int ii = 0; ii < apps.count(); ++ii)
        m_launcher->kill(apps.at(ii));
}

/*!
  \internal
  */
bool ApplicationShutdownTask::allAppsQuit()
{
    QStringList apps = m_launcher->applications();
    return (apps.isEmpty() || (apps.count() == 1 && apps.first() == "qpe"));
}

/*!
  \internal
  */
void ApplicationShutdownTask::timeout()
{
    switch(m_state) {
        case NoShutdown:
        case Shutdown:
        default:
            Q_ASSERT(!"Unknown");
            break;
        case WaitingForShutdown:
            // Issue kill
            killAll();
            m_timer.start();
            m_state = KilledApps;
            break;
        case KilledApps:
            // Failed - proceed anyway
            m_state = Shutdown;
            qWarning("Applications failed to exit at system shutdown");
            emit proceed();
            break;
    }
}

/*!
  \internal
  */
void ApplicationShutdownTask::terminated()
{
    if(allAppsQuit()) {
        m_timer.stop();
        if(m_state != NoShutdown && m_state != Shutdown) {
            m_state = Shutdown;
            emit proceed();
        }
    }
}

QTOPIA_DEMAND_TASK(ApplicationShutdownTask, ApplicationShutdownTask);
QTOPIA_TASK_PROVIDES(ApplicationShutdownTask, SystemShutdownHandler);

#include "applicationshutdowntask.moc"
