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

#ifndef CALLMONITOR_H
#define CALLMONITOR_H

#include <QObject>

namespace mediaserver
{

class CallMonitorPrivate;

class CallMonitor : public QObject
{
    Q_OBJECT

public:
    CallMonitor(QObject* parent = 0);
    ~CallMonitor();

signals:
    void callActivityChanged(bool active);

private:
    CallMonitorPrivate* d;
};

}   // ns mediaserver

#endif
