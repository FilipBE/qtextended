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

#ifndef E2_TASKMANAGER_H
#define E2_TASKMANAGER_H

#include "taskmanagerservice.h"
#include "e2_frames.h"

#include "applicationmonitor.h"
#include <QTimer>
#include <QPair>

class E2TaskManagerService : public TaskManagerService
{
public:
    E2TaskManagerService(QObject* par);
    void multitask();
    void showRunningTasks();
};

class QTreeWidget;
class QTreeWidgetItem;
class E2Bar;
class E2MemoryBar;
class QContent;
class E2TaskManager : public E2PopupFrame
{
    Q_OBJECT
public:
    static E2TaskManager* instance();

protected:
    void showEvent(QShowEvent* e);

private slots:
    void switchToTask();
    void endTask();
    void memoryUpdate();
    void doUpdate();
private:
    QList<QPair<QTreeWidgetItem*,QContent*> > m_items;
    E2TaskManager(QWidget* par);
    int getMemoryPercentUsed() const;
    E2Bar* m_bar;
    QTreeWidget* m_taskList;
    QTimer* m_memoryTimer;
    E2MemoryBar* m_memoryBar;
    UIApplicationMonitor m_appMonitor;
};

#endif
