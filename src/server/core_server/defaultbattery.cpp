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

#include "defaultbattery.h"
#include <QHardwareManager>
#include <QSettings>
#include "qtopiaserverapplication.h"

/*! 
   \class DefaultBattery
    \inpublicgroup QtBaseModule
   \ingroup QtopiaServer::Task
   \brief The DefaultBattery class provides a proxy for another system battery.

   To simplify the design of system themes and other mechanisms that source 
   information directly from the value space, the DefaultBattery class proxies
   the information of another, configurable system battery under the name
   "DefaultBattery".

   The battery to proxy can be manually configured by setting the 
   \c {PowerSources/PrimaryBatterySource} key in the \c {Trolltech/HardwareAccessories}
   configuration file.  If this setting is missing, the DefaultBattery class 
   will proxy the first available QPowerSource::Battery typed power source.

   The DefaultBattery class provides the \c {DefaultBattery} task.
   It is part of the Qt Extended server and cannot be used by other Qt Extended applications.
  */

/*!
  Constructs a new DefaultBattery instance with the given \a parent.
  */
DefaultBattery::DefaultBattery(QObject *parent)
: QPowerSourceProvider(Virtual, QLatin1String("DefaultBattery"), parent),
  m_powerSource(0), m_accessories(0)
{
    QSettings cfg(QLatin1String("Trolltech"), QLatin1String("HardwareAccessories"));
    m_primary = cfg.value(QLatin1String("PowerSources/PrimaryBatterySource")).toString();

    m_accessories = new QHardwareManager("QPowerSource", this);
    QObject::connect(m_accessories, SIGNAL(providerAdded(QString)),
                     this, SLOT(accessoryAdded(QString)));
    QStringList accs = m_accessories->providers();
    for(int ii = 0; m_accessories && ii < accs.count(); ++ii)
        accessoryAdded(accs.at(ii));

    syncPowerSource();
}

void DefaultBattery::syncPowerSource()
{
    if(m_powerSource) {
        setAvailability(m_powerSource->availability());
        setCharging(m_powerSource->charging());
        setCharge(m_powerSource->charge());
        setCapacity(m_powerSource->capacity());
        setTimeRemaining(m_powerSource->timeRemaining());
    } else {
        setAvailability(NotAvailable);
        setCharging(false);
        setCharge(-1);
        setCapacity(-1);
        setTimeRemaining(-1);
    }
}

void DefaultBattery::initPowerSource()
{
    QObject::connect(m_powerSource, SIGNAL(availabilityChanged(QPowerSource::Availability)), this, SLOT(pAvailabilityChanged(QPowerSource::Availability)));
    QObject::connect(m_powerSource, SIGNAL(chargingChanged(bool)), this, SLOT(pChargingChanged(bool)));
    QObject::connect(m_powerSource, SIGNAL(chargeChanged(int)), this, SLOT(pChargeChanged(int)));
    QObject::connect(m_powerSource, SIGNAL(capacityChanged(int)), this, SLOT(pCapacityChanged(int)));
    QObject::connect(m_powerSource, SIGNAL(timeRemainingChanged(int)), this, SLOT(pTimeRemainingChanged(int)));
}

void DefaultBattery::accessoryAdded(const QString &acc)
{
    QPowerSource *ps = new QPowerSource(acc);
    if(ps->type() == Battery) {
        if (m_primary.isEmpty() || m_primary == acc) {
            m_powerSource = ps;
            m_accessories->disconnect();
            m_accessories->deleteLater();
            m_accessories = 0;
            initPowerSource();
            syncPowerSource();
        } else {
            delete ps;
        }
    } else {
        delete ps;
    }
}

void DefaultBattery::pAvailabilityChanged(QPowerSource::Availability a)
{
    setAvailability(a);
}

void DefaultBattery::pChargingChanged(bool c)
{
    setCharging(c);
}

void DefaultBattery::pChargeChanged(int c)
{
    setCharge(c);
}

void DefaultBattery::pCapacityChanged(int c)
{
    setCapacity(c);
}

void DefaultBattery::pTimeRemainingChanged(int t)
{
    setTimeRemaining(t);
}

QTOPIA_TASK(DefaultBattery, DefaultBattery);

