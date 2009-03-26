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

#ifndef LOWMEMORYTASK_H
#define LOWMEMORYTASK_H

#include "memorymonitor.h"
#include "applicationlauncher.h"

struct LowMemoryTaskPrivate;
class LowMemoryTask : public QObject
{
    Q_OBJECT

  public:
    LowMemoryTask();
    virtual ~LowMemoryTask();

  private:
    void handleCriticalMemory();
    void handleVeryLowMemory();
    void handleLowMemory();
    bool kill(const QString& proc);
    bool quit(const QString& proc);
    int quitIfInvisible(const QMap<QString,int>& procs);
    QString selectProcess();

  private slots:
    void avoidOutOfMemory(MemoryMonitor::MemState newState);
    void showWarningBox( const QString& title, const QString& text );

  private:
    MemoryMonitor::MemState     m_state;
    MemoryMonitor::MemState     m_prevState;
    LowMemoryTaskPrivate* d;
};

QTOPIA_TASK_INTERFACE(LowMemoryTask);

#endif
