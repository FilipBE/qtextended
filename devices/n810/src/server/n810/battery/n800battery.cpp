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

#include "n800battery.h"
#include "qtopiaserverapplication.h"

#include <QPowerSourceProvider>
#include <QTimer>
#include <QFileMonitor>

#include <math.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


#include <QtDBus/QDBusConnection>


#include <QDebug>

N800Battery::N800Battery(QObject *parent)
: QObject(parent), ac(0), battery(0)
{
qWarning()<<"N800Battery::N800Battery";

    QtopiaServerApplication::taskValueSpaceSetAttribute("N800Battery",
                                                        "APMAvailable", false);


    ac = new QPowerSourceProvider(QPowerSource::Wall, "PrimaryAC", this);
    battery = new QPowerSourceProvider(QPowerSource::Battery, "N800Battery", this);
    QDBusConnection  dbc = QDBusConnection::systemBus();

    if (!dbc.isConnected()) {
        qWarning() << "Unable to connect to D-BUS:" << dbc.lastError();
        return;
    }

    dbc.connect(QString(), "/com/nokia/bme/signal",
                                "com.nokia.bme.signal", "charger_connected",
                this, SIGNAL(chargerConnected()));

    dbc.connect(QString(), "/com/nokia/bme/signal",
                                "com.nokia.bme.signal", "charger_disconnected",
                this, SIGNAL(chargerDisconnected()));

    dbc.connect(QString(), "/com/nokia/bme/signal",
                                "com.nokia.bme.signal", "battery_full",
                this, SIGNAL(batteryFull()));

    dbc.connect(QString(), "/com/nokia/bme/signal",
                                "com.nokia.bme.signal", "charger_charging_off",
                this, SIGNAL(chargerOff()));

    dbc.connect(QString(), "/com/nokia/bme/signal",
                                "com.nokia.bme.signal", "charger_charging_on",
                this, SIGNAL(chargerOn()));

    dbc.connect(QString(), "/com/nokia/bme/signal",
                                "com.nokia.bme.signal", "battery_time_left",
                this, SIGNAL(batteryTimeLeft()));

    dbc.connect(QString(), "/com/nokia/bme/signal",
                                "com.nokia.bme.signal", "battery_state_changed",
                this, SIGNAL(batteryStateChanged(uint)));

//battery_empty


    QDBusMessage msg = QDBusMessage::createSignal( "/com/nokia/bme/request",
                                                    "com.nokia.bme.request",
                                                    "status_info_req");
    dbc.send(msg);




//updateStatus();
}

void N800Battery::batteryPropertyChanged(QString str1,  QString str2)
{
    qWarning() << "batteryPropertyChanged()" << str1 << str2;
    updateStatus();
}

void N800Battery::updateStatus()
{
    QDBusConnection dbc = QDBusConnection::systemBus();
    QDBusMessage timeLeftMessage = QDBusMessage::createSignal( "/com/nokia/bme/request",
                                                                "com.nokia.bme.request",
                                                                "timeleft_info_req");
    dbc.send(timeLeftMessage);

}

/*! \internal */
void N800Battery::timerEvent(QTimerEvent *)
{
    updateStatus();
}

/*! \internal */
bool N800Battery::APMEnabled() const
{
    return false;
}



int N800Battery::getBatteryLevel()
{

    return 0;
}

bool N800Battery::batteryIsFull()
{
    if (percentCharge == 100)
        return true;
    else
        return false;
}


void N800Battery::chargerConnected()
{
    ac->setAvailability(QPowerSource::Available);
    battery->setAvailability(QPowerSource::NotAvailable);
    updateStatus();

}


void N800Battery::chargerDiconnected()
{
    ac->setAvailability(QPowerSource::NotAvailable);
    battery->setAvailability(QPowerSource::Available);

    updateStatus();

}


void N800Battery::batteryFull()
{
    battery->setCharge( 100);
    battery->setCharging( false);
    percentCharge = 100;
    updateStatus();

}

void N800Battery::chargerOff()
{
//give some level indication until we get a state change notification.
    battery->setCharge( percentCharge);

    battery->setCharging( false);
    updateStatus();

}

void N800Battery::chargerOn()
{
    battery->setCharge( 5);
    battery->setCharging( true);
    updateStatus();

}


void N800Battery::batteryTimeLeft(uint /*t1*/,  uint t2)
{
//t1 idle minutes
//t2 use minutes

    battery->setTimeRemaining( t2);

}

void N800Battery::batteryStateChanged(uint level)
{

    qWarning()<<"battery state changed" << level << level * 25;
    percentCharge = level * 25;

    if (level > 0 )
        battery->setCharge( percentCharge);
    else
        battery->setCharge(1);
}


QTOPIA_TASK(N800Battery, N800Battery);


/*

gconftool-2 --set /system/osso/dsm/display/display_on_with_charger --type bool true
    display_blank_timeout
    display_dim_timeout
    display_brightness
    max_display_brightness_levels
    possible_display_dim_timeouts
    possible_display_blank_timeouts
*/
