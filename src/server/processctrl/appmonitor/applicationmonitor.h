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

#ifndef APPLICATIONMONITOR_H
#define APPLICATIONMONITOR_H

#include <QObject>
#include <QPair>
#include <QList>
#include <QStringList>
#include "qtopiaserverapplication.h"
#include "applicationlauncher.h"

class UIApplicationMonitorPrivate;
class UIApplicationMonitor : public QObject
{
Q_OBJECT
public:
    UIApplicationMonitor(QObject *parent = 0);
    virtual ~UIApplicationMonitor();

    enum ApplicationState { NotRunning = 0x0000,
                            Starting = 0x0001,
                            Running  = 0x0002,

                            StateMask = 0x00FF,

                            NotResponding = 0x0100,
                            Busy = 0x0200,
                            Active = 0x0400 };

    ApplicationState applicationState(const QString &) const;

    QStringList runningApplications() const;
    QStringList notRespondingApplications() const;
    QStringList busyApplications() const;

    static int notRespondingTimeout();
    static void setNotRespondingTimeout(int);

signals:
    void applicationStateChanged(const QString &,
                                 UIApplicationMonitor::ApplicationState);
    void busy();
    void notBusy();

private:
    UIApplicationMonitorPrivate *d;
};

#endif
