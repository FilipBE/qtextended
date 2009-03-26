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

#include "e3_today.h"
#include <QTask>
#include <QTaskModel>
#include <QAppointment>
#include <QAppointmentModel>
#include <QtopiaApplication>
#include <QDate>
#include <QDateTime>

E3Today::E3Today(QObject *parent)
: QObject(parent), m_status(NoAppointments), m_tasks(0), m_appointments(0),
  blockReset(false)     
{
    m_tasks = new QTaskModel(this);
    m_tasks->setFilterCompleted(true);

    QObject::connect(m_tasks, SIGNAL(dataChanged(QModelIndex,QModelIndex)), 
                     this, SIGNAL(todayChanged()));
    QObject::connect(m_tasks, SIGNAL(rowsInserted(QModelIndex,int,int)), 
                     this, SIGNAL(todayChanged()));
    QObject::connect(m_tasks, SIGNAL(rowsRemoved(QModelIndex,int,int)), 
                     this, SIGNAL(todayChanged()));
    QObject::connect(m_tasks, SIGNAL(layoutChanged()),
                     this, SIGNAL(todayChanged()));

    m_appointments = new QAppointmentModel(this);
    QObject::connect(m_appointments, SIGNAL(dataChanged(QModelIndex,QModelIndex)), 
                     this, SLOT(resetAppointments()));
    QObject::connect(m_appointments, SIGNAL(rowsInserted(QModelIndex,int,int)), 
                     this, SLOT(resetAppointments()));
    QObject::connect(m_appointments, SIGNAL(rowsRemoved(QModelIndex,int,int)), 
                     this, SLOT(resetAppointments()));
    QObject::connect(m_appointments, SIGNAL(layoutChanged()),
                     this, SLOT(resetAppointments()));
    QObject::connect(QtopiaApplication::instance(), SIGNAL(timeChanged()),
                     this, SLOT(resetAppointments()));

    QObject::connect(&m_timer, SIGNAL(timeout()), 
                     this, SLOT(resetAppointments()));
    m_timer.setSingleShot(true);

    resetAppointments();
}

void E3Today::forceUpdate()
{
    resetAppointments();
}

E3Today::DayStatus E3Today::dayStatus() const
{
    return m_status;
}

QAppointment E3Today::nextAppointment() const
{
    return m_nextAppointment;
}

int E3Today::tasks() const
{
    return m_tasks->rowCount();
}

QTask E3Today::task(int ii) const
{
    return m_tasks->task(ii);
}

void E3Today::resetAppointments()
{
    QDate today = QDate::currentDate();
    QDateTime todayTime = QDateTime::currentDateTime();
    blockReset = true;
    m_appointments->setRange(QDateTime(today), QDateTime());
    blockReset = false;

    // Determine states and next appointment
    bool wasToday = false;
    QOccurrence nextOccurrence;
    for(int ii = 0; !nextOccurrence.isValid() && ii < m_appointments->count(); ++ii) {

        QAppointment appointment = m_appointments->appointment(ii);
        QOccurrence o = appointment.nextOccurrence(today);

        if(o.start() > todayTime) {
            if(!nextOccurrence.isValid())
                nextOccurrence = o;
        }

        if(o.date() == today)
            wasToday = true;
    }

    m_nextAppointment = nextOccurrence.appointment();
    if(nextOccurrence.date() == today) {
        m_status = MoreAppointments;
    } else if(wasToday) {
        m_status = NoMoreAppointments;
    } else {
        m_status = NoAppointments;
    }

    m_timer.stop();
    if(nextOccurrence.isValid()) {
        int secs = todayTime.secsTo(nextOccurrence.start()) + 1;
        int msecs = secs * 1000;
        if(msecs < secs)
            msecs = secs;
        m_timer.start(msecs);
    }

    emit todayChanged();
}
