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

#include "taskmanagerservice.h"

/*!
    \service TaskManagerService TaskManager
    \inpublicgroup QtBaseModule
    \brief The TaskManagerService class provides the Qt Extended TaskManager service.

    The \i TaskManager service enables applications to cause the home
    screen to switch between tasks or show a list of all running tasks.
*/

/*!
    \internal
    \fn TaskManagerService::TaskManagerService(QObject *parent)
*/

/*!
    \internal
 */
TaskManagerService::~TaskManagerService()
{
}

/*!
    \fn void TaskManagerService::multitask()
    Switch to the next task.

    This slot corresponds to the QCop service message
    \c{TaskManager::multitask()}.
*/

/*!
    \fn void TaskManagerService::showRunningTasks()
    Show a list of all running tasks.

    This slot corresponds to the QCop service message
    \c{TaskManager::showRunningTasks()}.
*/

