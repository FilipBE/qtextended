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

#ifndef E3_TODAY_H
#define E3_TODAY_H

#include <QObject>
#include <QAppointment>
#include <QTimer>
class QTask;
class QTaskModel;
class QAppointmentModel;

class E3Today : public QObject
{
Q_OBJECT
public:
    E3Today(QObject *parent = 0);

    enum DayStatus { NoAppointments, NoMoreAppointments, MoreAppointments };
    DayStatus dayStatus() const;

    QAppointment nextAppointment() const;

    int tasks() const;
    QTask task(int ii) const;

    void forceUpdate();

signals:
    void todayChanged();

private slots:
    void resetAppointments();

private:
    DayStatus m_status;
    QTaskModel *m_tasks;
    QAppointmentModel *m_appointments;
    QAppointment m_nextAppointment;
    bool blockReset;
    QTimer m_timer;
};

#endif
