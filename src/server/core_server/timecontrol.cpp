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

#include "timecontrol.h"
#include "qtopiaserverapplication.h"
#include "systemsuspend.h"

/*!
  \class TimeControl 
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::Task
  \brief The TimeControl class updates the current time and date in the value space.

  The current system date and time is kept updated in the following value space
  keys:

  \table
  \header \o Key \o Description
  \row \o \c {/UI/DisplayTime/Time} \o The current time with minute resolution (eg "22:13").
  \row \o \c {/UI/DisplayTime/Date} \o The current date in expanded form (eg. "31 Aug 06").
  \row \o \c {/UI/DisplayTime/BriefDate} \o The current date in reduced form (eg. "31/08/06").
  \endtable

  While it is possible, and correct, for applications to source this
  information directly through the QTime and QDate APIs, these values can be
  used for displaying the time in title bars or other incidental locations.
  By doing so, these time or date displays will remain in sync with one
  another.

  Invoking this method will disable the creation and updating of the listed
  value space keys.
  
  The TimeControl class is a server task and is part of the Qt Extended server.
  It cannot be used by other Qt Extended applications.
*/

/*!
    Creates a TimeControl instance with the given \a parent.
  */
TimeControl::TimeControl(QObject *parent)
: QObject(parent), m_timer(0), timeValueSpace("/UI/DisplayTime")
{
    m_timer = new QtopiaTimer(this);
    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(doTimeTick()));
    doTimeTick();
    QObject::connect(qApp, SIGNAL(timeChanged()), this, SLOT(doTimeTick()));
    QObject::connect(qApp, SIGNAL(dateFormatChanged()), this, SLOT(dateFormatChanged()));
    QObject::connect(qApp, SIGNAL(clockChanged(bool)), this, SLOT(clockChanged(bool)));

    QObject::connect(qtopiaTask<SystemSuspend>(), SIGNAL(systemSuspending()),
                     this, SLOT(systemSuspending()));
    QObject::connect(qtopiaTask<SystemSuspend>(), SIGNAL(systemWaking()),
                     this, SLOT(systemWaking()));
}

void TimeControl::doTimeTick()
{
    clockChanged(QTimeString::currentAMPM());
    dateFormatChanged();

    m_timer->stop();
    QTime time = QTime::currentTime();
    m_timer->start((60 - time.second()) * 1000 + 500, 
                   QtopiaTimer::PauseWhenInactive);
}

void TimeControl::dateFormatChanged()
{
    QString ldate = QTimeString::localYMD(QDate::currentDate(), QTimeString::Short);
    QString sdate = QTimeString::numberDateString(QDate::currentDate());
    timeValueSpace.setAttribute("Date", ldate);
    timeValueSpace.setAttribute("BriefDate", sdate);
}

void TimeControl::clockChanged(bool)
{
    timeValueSpace.setAttribute("Time",
            QTimeString::localHM(QDateTime::currentDateTime().time(), QTimeString::Short));
}

void TimeControl::systemSuspending()
{
    timeValueSpace.setAttribute("Date", QString());
    timeValueSpace.setAttribute("BriefDate", QString());
    timeValueSpace.setAttribute("Time", QString());
}

void TimeControl::systemWaking()
{
    clockChanged(QTimeString::currentAMPM());
    dateFormatChanged();
}

QTOPIA_TASK( TimeControl, TimeControl )
