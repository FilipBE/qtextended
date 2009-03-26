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

#include "e1_battery.h"

E1Battery::E1Battery(QObject *parent)
: QObject(parent), m_timerId(0), m_battery("/Hardware/Accessories/QPowerSource/DefaultBattery")
{
    QObject::connect(&m_battery, SIGNAL(contentsChanged()), this, SLOT(batteryChanged()));
    batteryChanged();
}

E1Battery::~E1Battery()
{
}

int E1Battery::charge()
{
    return m_charge;
}

void E1Battery::batteryChanged()
{
    bool isCharging = m_battery.value("Charging").toBool();
    int charge = m_battery.value("Charge").toInt();

    if(isCharging) {
        startcharge();
    } else {
        stopcharge();
        m_charge = charge;
        emit chargeChanged(this->charge());
    }
}

void E1Battery::timerEvent(QTimerEvent *)
{
    m_charge = (m_charge + 20) % 100;
    emit chargeChanged(charge());
}

void E1Battery::startcharge()
{
    if(!m_timerId) {
        m_timerId = startTimer(500);
        m_charge = 0;
        emit chargeChanged(charge());
    }
}

void E1Battery::stopcharge()
{
    if(m_timerId) {
        killTimer(m_timerId);
        m_timerId = 0;
    }
}
