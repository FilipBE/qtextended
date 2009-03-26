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

#ifndef MEMORYMONITOR_H
#define MEMORYMONITOR_H

#include <QObject>
#include "qtopiaserverapplication.h"

class MemoryMonitor : public QObject
{
    Q_OBJECT

  public:
    enum MemState {
        MemUnknown      = 0,
        MemCritical     = 1,
        MemVeryLow      = 2,
        MemLow          = 3,
        MemNormal       = 4
    };

    virtual MemState memoryState() const = 0;
    virtual unsigned int timeInState() const = 0;

  signals:
    void memoryStateChanged(MemoryMonitor::MemState newState);
};

QTOPIA_TASK_INTERFACE(MemoryMonitor);

#endif
