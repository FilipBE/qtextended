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

/*!
  \class QAbstractTaskManager
    \inpublicgroup QtBaseModule
  \brief The QAbstractTaskManager class allows developers to replace the Qt Extended server
  TaskManager.
    \ingroup QtopiaServer::PhoneUI::TTSmartPhone

  It is the task managers responsibility to present a list of all running applications to the user.
  In addition it should provide an implementation of the TaskManagerService. Upon activation
  of the various TaskManagerSerivce slots the similar named signals provided by this interface will be emitted.

  The QAbstractTaskManager interface is part of the
  \l {QtopiaServerApplication#qt-extended-server-widgets}{server widgets framework} and allows developers 
  to replace the standard home screen in the Qt Extended server UI.
  A small tutorial on how to develop new server widgets using one of the abstract widgets as base can
  be found in QAbstractServerInterface class documentation.

  The QAbstractTaskManager interface is marked as singleton interface. For more details 
  about the concept of singleton server widgets refer to the \l {QtopiaServerApplication#singleton-pattern}{server widget documentation}.
  
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  \sa TaskManagerService
  */

/*!
  \fn QAbstractTaskManager::QAbstractTaskManager(QWidget *parent, Qt::WFlags flags)

  Constrcuts a new QAbstractTaskManager instance, with the specified \a parent
  and widget \a flags.
  */

/*!
  \fn void QAbstractTaskManager::showRunningTasks()

  This signal is emitted whenever the TaskManagerService::showRunningTasks() is activated.
  This may happens as a result of a hardware key press by the user.
  Usually the owner of the QAbstractTaskManager object raises QAbstractTaskManager widget
  as a result of this signal.

  \sa TaskManagerService::showRunningTasks()
*/

/*!
  \fn void QAbstractTaskManager::multitaskRequested()

  This signal is emitted whenever the user requests multitasking functionality via e.g. 
  a hardware button. The specified reaction to this signal is not determined and must be
  provided by the object connected to the task manager. 
  
  The Qt Extended default implementation cycles through the list of open applications whenever this
  signal is emitted.

  \sa TaskManagerService::multitask()
  */
