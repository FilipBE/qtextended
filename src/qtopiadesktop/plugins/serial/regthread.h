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
#ifndef REGTHREAD_H
#define REGTHREAD_H

#include <QStringList>
#include <QMutex>
#include <QThread>
#include <registryhelper.h>

class RegThread : public QThread
{
    Q_OBJECT
public:
    RegThread();
    ~RegThread();
    void run();
    QStringList ports();
signals:
    void comPortsChanged();
public:
    HANDLE quit;
private:
    void getPorts();
    HKEY hKey;
    HANDLE handle;
    QStringList mPorts;
    QMutex portMutex;
};

#endif
