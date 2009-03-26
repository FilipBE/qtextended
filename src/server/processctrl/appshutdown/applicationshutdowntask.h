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

#ifndef APPLICATIONSHUTDOWNTASK_H
#define APPLICATIONSHUTDOWNTASK_H

#include "qtopiaserverapplication.h"
#include <QTimer>

class ApplicationLauncher;
class ApplicationShutdownTask : public SystemShutdownHandler
{
Q_OBJECT
public:
    ApplicationShutdownTask();

    virtual bool systemRestart();
    virtual bool systemShutdown();

private slots:
    void terminated();
    void timeout();

private:
    bool doShutdown();
    bool allAppsQuit();
    void killAll();

    enum State { NoShutdown, WaitingForShutdown, KilledApps, Shutdown };
    State m_state;
    QTimer m_timer;
    ApplicationLauncher *m_launcher;
};

#endif
