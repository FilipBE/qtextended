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

#include <QPhoneCallManager>

#include "callmonitor.h"

namespace mediaserver
{

class CallMonitorPrivate : public QObject
{
    Q_OBJECT

public:
    bool                active;
    QPhoneCallManager*  callManager;

signals:
    void callActivityChanged(bool active);

private slots:
    void checkCalls();
};

void CallMonitorPrivate::checkCalls()
{
    bool h = callManager->calls().count() > 0;

    if (active != h)
        emit callActivityChanged(active = h);
}

/*!
    \class mediaserver::CallMonitor
    \internal
*/
CallMonitor::CallMonitor(QObject* parent):
    QObject(parent),
    d(new CallMonitorPrivate)
{
    d->active = false;
    d->callManager = new QPhoneCallManager(this);
    connect(d->callManager, SIGNAL(statesChanged(QList<QPhoneCall>)), d, SLOT(checkCalls()));

    connect(d, SIGNAL(callActivityChanged(bool)), SIGNAL(callActivityChanged(bool)));
}

CallMonitor::~CallMonitor()
{
    delete d;
}

}   // ns mediaserver

#include "callmonitor.moc"


