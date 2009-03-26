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

#include "taskmanagerentry.h"
#include <qtopiaipcadaptor.h>


class TaskManagerEntryPrivate : public QtopiaIpcAdaptor
{
    Q_OBJECT
public:
    TaskManagerEntryPrivate(TaskManagerEntry *parent, const QString &description, const QString &iconPath);
    void show();
    void hide();

signals:
    void addDynamicLauncherItem(int id, const QString &description, const QString &iconPath);
    void removeDynamicLauncherItem(int id);

public slots:
    void runningApplicationsViewLoaded();
    void activatedLaunchItem(int id);

private:
    TaskManagerEntry *m_parent;
    int m_id;
    QString m_description;
    QString m_iconPath;

    bool m_runningAppsViewLoaded;
    bool m_showWhenViewLoaded;

    static QAtomicInt idCounter;
    static int nextId();
};

QAtomicInt TaskManagerEntryPrivate::idCounter(1);

/*
    This talks over IPC to the RunningAppsLauncherView class in
    src/server/ui/launcherviews/taskmanagerview/taskmanagerlauncherview.cpp.
*/

TaskManagerEntryPrivate::TaskManagerEntryPrivate(TaskManagerEntry *parent, const QString &description, const QString &iconPath)
    : QtopiaIpcAdaptor("QPE/RunningAppsLauncherViewService", parent),
      m_parent(parent),
      m_id(nextId()),
      m_description(description),
      m_iconPath(iconPath),
      m_runningAppsViewLoaded(false),
      m_showWhenViewLoaded(false)
{
    publishAll(SignalsAndSlots);
}

void TaskManagerEntryPrivate::show()
{
    m_showWhenViewLoaded = true;
    emit addDynamicLauncherItem(m_id, m_description, m_iconPath);
}

void TaskManagerEntryPrivate::hide()
{
    m_showWhenViewLoaded = false;
    emit removeDynamicLauncherItem(m_id);
}

void TaskManagerEntryPrivate::runningApplicationsViewLoaded()
{
    if (m_showWhenViewLoaded)
        show();
}

void TaskManagerEntryPrivate::activatedLaunchItem(int id)
{
    if (id == m_id)
        emit m_parent->activated();
}

/*
    Generates an ID for an item.
*/
int TaskManagerEntryPrivate::nextId()
{
    register int id;
    for (;;) {
        id = idCounter;
        if (idCounter.testAndSetOrdered(id, id + 1))
            break;
    }
    return id;
}


/*!
    \class TaskManagerEntry
    \inpublicgroup QtBaseModule
    \brief The TaskManagerEntry class is used to insert non-application items into the Running Applications/TaskManager window.
    \ingroup QtopiaServer::AppLaunch

    Normally, the TaskManager only shows Qt Extended applications.
    However, there may be times when you need to show other items in the
    TaskManager. For example, if you have a server task that displays an
    informational dialog about a long-running activity, you might want the dialog
    to be accessible via the TaskManager window so the user can return
    to the dialog if it is backgrounded. However, in this situation, the
    dialog will not automatically be displayed in the TaskManager
    because it is an ordinary dialog and not a Qt Extended application.

    In these cases, you can use the TaskManagerEntry class. For
    example:

    \code
    TaskManagerEntry *cpuMonitorItem =
            new TaskManagerEntry(tr("CPU Monitor"), "cpuMonitorIcon");
    connect(cpuMonitorItem, SIGNAL(activated()),
            this, SLOT(raiseCpuMonitorDialog()));
    cpuMonitorItem->show();
    \endcode

    The activated() signal is emitted when the user activates the item in the
    TaskManager window.
*/

/*!
  Constructs a new TaskManagerEntry. \a description represents the string under which 
  the entry should appear in the task task manager launcher view and \a iconPath describes the 
  icon to be used. \a parent is the standard QObject parent parameter.
*/
TaskManagerEntry::TaskManagerEntry(const QString &description, const QString &iconPath, QObject *parent)
    : QObject(parent),
      m_data(new TaskManagerEntryPrivate(this, description, iconPath))
{
}

/*!
    Shows this item in the Running Applications window. This is equivalent to registering the
    this new entry with the task manager.

    \sa hide()
*/
void TaskManagerEntry::show()
{
    m_data->show();
}

/*!
    Hides this item in the TaskManager. This is equivalent to deregistering the entry.

    \sa show()
*/
void TaskManagerEntry::hide()
{
    m_data->hide();
}

/*!
    \fn TaskManagerEntry::activated()

    Emitted when the user has activated this item from the TaskManager.
*/

#include "taskmanagerentry.moc"
