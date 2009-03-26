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

#ifndef SYSTEMSUSPEND_H
#define SYSTEMSUSPEND_H

#include <QObject>
#include <QtopiaAbstractService>
#include "qtopiaserverapplication.h"

class SystemSuspendHandler : public QObject
{
Q_OBJECT
public:
    SystemSuspendHandler(QObject *parent = 0)
        : QObject(parent) {}

    virtual bool canSuspend() const = 0;
    virtual bool suspend() = 0;
    virtual bool wake() = 0;

signals:
    void operationCompleted();
};
QTOPIA_TASK_INTERFACE(SystemSuspendHandler);

class SystemSuspend : public QObject
{
Q_OBJECT
public:
    SystemSuspend(QObject *parent = 0) : QObject(parent) {}

public slots:
    virtual bool suspendSystem() = 0;

signals:
    void systemSuspending();
    void systemWaking();
    void systemActive();
    void systemSuspendCanceled();
};
QTOPIA_TASK_INTERFACE(SystemSuspend);

class SuspendService : public QtopiaAbstractService
{
Q_OBJECT
public:
    SuspendService(QObject *parent);

signals:
    void doSuspend();

public slots:
    void suspend();
};

#endif
