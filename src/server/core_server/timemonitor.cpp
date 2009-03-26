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

#include "timemonitor.h"
#include <QtopiaApplication>
#include <QtopiaIpcEnvelope>
#include <QtopiaService>
#include <QTimeZone>

#include "qtopiaserverapplication.h"

/*!
  \class TimeMonitorTask
    \inpublicgroup QtBaseModule
  \brief The TimeMonitorTask class is required for the TimeMonitor service to be supported.
  \ingroup QtopiaServer::Task
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
 */

/*! \internal */
TimeMonitorTask::TimeMonitorTask()
{
    QObject::connect(QtopiaApplication::instance(), SIGNAL(timeChanged()),
                     this, SLOT(pokeTimeMonitors()));
}

/*! \internal */
void TimeMonitorTask::pokeTimeMonitors()
{
    // inform all TimeMonitors
    QStringList tms = QtopiaService::channels("TimeMonitor");
    foreach (QString ch, tms) {
        QString t = QTimeZone::current().id();
        QtopiaIpcEnvelope e(ch, "TimeMonitor::timeChange(QString)");
        e << t;
    }
}

QTOPIA_TASK(TimeMonitor, TimeMonitorTask);
