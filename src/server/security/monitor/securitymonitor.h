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

#ifndef SECURITYMONITOR_H
#define SECURITYMONITOR_H

#include "qtopiaserverapplication.h"

#ifndef QT_NO_SXE
#include <QProcess>
#endif

class SecurityMonitor;
class SecurityMonitorTask : public SystemShutdownHandler
{
Q_OBJECT
public:
    SecurityMonitorTask();

    virtual bool systemRestart();
    virtual bool systemShutdown();

private slots:
    void init();
    void finished();

#ifndef QT_NO_SXE
    void sxeMonitorProcessError(QProcess::ProcessError);
    void sxeMonitorProcessExited(int);
    void startNewSxeMonitor();
#endif

private:
    void doShutdown();

#ifndef QT_NO_SXE
    bool isShutdown;
    static QString sxemonitorExecutable();
    QProcess *m_sxeMonitorProcess;
#endif

};

#endif
