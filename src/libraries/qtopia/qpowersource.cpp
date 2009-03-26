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

#include "qpowersource.h"
#include <QSettings>
#include <QHardwareManager>

static const char* const QPOWERSOURCE_NAME = "QPowerSource";
static const char* const QPOWERSOURCE_TYPE = "Type";
static const char* const QPOWERSOURCE_AVAILABILITY = "Availability";
static const char* const QPOWERSOURCE_CHARGE = "Charge";
static const char* const QPOWERSOURCE_CHARGING = "Charging";
static const char* const QPOWERSOURCE_CAPACITY = "Capacity";
static const char* const QPOWERSOURCE_TIMEREMAINING = "TimeRemaining";

/*!
  \class QPowerSource
    \inpublicgroup QtBaseModule

  \brief The QPowerSource class provides access to information about power sources on the device.

  Power sources are components, either external or internal, such as batteries
  or wall sockets that provide power to a Qt Extended device.  The QPowerSource API
  allows applications to query the status of such components.

  To access a list of QPowerSource instances, applications can use the 
  QHardwareManager class.  For example, to print out the charge percentage
  of each power source:

  \code
  QHardwareManager powerSources("QPowerSource");
  QStringList sources = powerSources.providers();

  foreach(QString source, sources) {
      QPowerSource powerSource(source);
      qWarning() << source << ":" << powerSource.charge() << "%";
      }
  \endcode

  If an application or system component wants to provide information about a new
  power source, it should use the QPowerSourceProvider API.

  \sa QPowerSourceProvider, QPowerStatus

  \ingroup hardware
 */

/*!
  \enum QPowerSource::Type

  Describes the type of the power source.

  \value Wall This is a wall power source.  Wall power sources are generally
  considered to provide infinite power, but are "unreliable" in that they may
  be disconnected at any time.
  \value Battery This is a battery power source.  Battery power sources provide
  a finite amount of power, but are considered "reliable".
  \value Virtual This is a virtual power source.  Virtual power sources do not
  correspond directly to a physical entity, but might, for example, be a virtual
  battery that consists of a weighted combination of other physical batteries.
*/

/*!
  \enum QPowerSource::Availability

  Represents whether the power source is available.

  \value Available The power source is available.
  \value NotAvailable The power source is not available.
  \value Failed The power source has failed.
 */

/*!
  \fn void QPowerSource::availabilityChanged( QPowerSource::Availability availability )

  This signal is emitted whenever the availability of the power source changes. The
  new value is passed along via \a availability.

  \sa QPowerSource::availability(), QPowerSourceProvider::setAvailability()
  */

/*!
  \fn void QPowerSource::chargingChanged( bool isCharging )

  This signal is emitted whenever the power source changes the charging state. The
  new state is represented by \a isCharging.

  \sa QPowerSource::charging(), QPowerSourceProvider::setCharging()
*/

/*!
  \fn void QPowerSource::chargeChanged(int charge)

  This signal is emitted whenever the power source \a charge changes.

  \sa QPowerSource::charge(), QPowerSourceProvider::setCharge()
*/

/*!
  \fn void QPowerSource::capacityChanged( int capacity )

  This signal is emitted whenever the power source \a capacity changes.

  \sa QPowerSource::capacity(), QPowerSourceProvider::setCapacity()
*/

/*!
  \fn void QPowerSource::timeRemainingChanged( int minutes )

  This signal is emitted whenever the time remaining, in \a minutes, changes .

  \sa QPowerSource::timeRemaining(), QPowerSourceProvider::setTimeRemaining()
*/

/*!
  Construct a new power source object for \a id with the specified \a parent.

  If \a id is empty, this class will use the default provider that supports
  the power source interface.
 */
QPowerSource::QPowerSource(const QString &id, QObject *parent)
: QHardwareInterface(QPOWERSOURCE_NAME, id, parent, Client)
{
    proxyAll( staticMetaObject );
}

/*! \internal */
QPowerSource::QPowerSource(const QString &id, QObject *parent, 
                           QAbstractIpcInterface::Mode mode)
: QHardwareInterface(QPOWERSOURCE_NAME, id, parent, mode)
{
    proxyAll( staticMetaObject );
}

/*!
  Destroys the QPowerSource instance.
 */
QPowerSource::~QPowerSource()
{
}

/*!
  Returns the power source type.
 */
QPowerSource::Type QPowerSource::type() const
{
    QString str = value(QPOWERSOURCE_TYPE).toString();
    if(str == QLatin1String("Wall"))
        return Wall;
    else if(str == QLatin1String("Battery"))
        return Battery;
    else 
        return Virtual;
}

/*!
  Returns the power source availability.
 */
QPowerSource::Availability QPowerSource::availability() const
{
    QString str = value(QPOWERSOURCE_AVAILABILITY).toString();
    if(str == QLatin1String("Available"))
        return Available;
    else if(str == QLatin1String("NotAvailable"))
        return NotAvailable;
    else
        return Failed;
}

/*!
  Returns true if the power source is charging, false if not.
 */
bool QPowerSource::charging() const
{
    return value(QPOWERSOURCE_CHARGING, false).toBool();
}

/*!
  Returns the current charge of the source in percentage, or -1 if the charge
  is unavailable or invalid.
 */
int QPowerSource::charge() const
{
    return value(QPOWERSOURCE_CHARGE, -1).toInt();
}

/*!
  Returns the capacity, in milliamp-hours of the power source of -1 if the 
  capacity is unavailable or invalid.
 */
int QPowerSource::capacity() const
{
    return value(QPOWERSOURCE_CAPACITY, -1).toInt();
}

/*!
  Returns the time remaining, in minutes, of the power source, or -1 if the 
  time remaining is unavailable or invalid.
 */
int QPowerSource::timeRemaining() const
{
    return value(QPOWERSOURCE_TIMEREMAINING, -1).toInt();
}

/*!
  \class QPowerSourceProvider
    \inpublicgroup QtBaseModule

  \brief The QPowerSourceProvider class provides an interface for power sources to integrate into Qtopia.

  Power sources are components, either external or internal, such as batteries
  or wall sockets that provide power to a Qt Extended device.  The 
  QPowerSourceProvider API allows the addition of information about new power
  sources into Qtopia.

  By deriving from the QPowerSourceProvider (or just creating an instance), an
  application or system component can provide information about a power source
  that can then be accessed through the QPowerSource API.  For example,

  \code
  QPowerSourceProvider *battery = new QPowerSourceProvider(QPowerSource::Battery, "My Pretend Battery");
  battery->setAvailability(QPowerSource::NotAvailable);
  battery->setCharging(true);
  \endcode

  \sa QPowerSource

  \ingroup hardware
 */

struct QPowerSourceProviderPrivate
{
    QPowerSourceProviderPrivate()
        : availability(QPowerSource::NotAvailable),
        charging(false), charge(-1), capacity(-1), timeRemaining(-1) {}

    QPowerSource::Availability availability;
    bool charging;
    int charge;
    int capacity;
    int timeRemaining;
};


/*!
  Create a new QPowerSourceProvider of the given \a type and with the specified
  \a id and \a parent.
  */
QPowerSourceProvider::QPowerSourceProvider(Type type, const QString &id, QObject *parent)
: QPowerSource(id, parent, Server), d(0)
{
    d = new QPowerSourceProviderPrivate;

    QString str;
    switch(type) {
        case Wall:
            str = QLatin1String("Wall");
            break;
        case Battery:
            str = QLatin1String("Battery");
            break;
        default:
            str = QLatin1String("Virtual");
            break;
    }
    setValue(QPOWERSOURCE_TYPE, str);
    setValue( QPOWERSOURCE_AVAILABILITY, QLatin1String("NotAvailable") );
}

/*!
  Destroy the QPowerSourceProvider instance and remove information about the
  power source from Qtopia.
 */
QPowerSourceProvider::~QPowerSourceProvider()
{
    delete d;
    d = 0;
}

/*!
  Set the \a availability of the power source.
 */
void QPowerSourceProvider::setAvailability(QPowerSource::Availability availability)
{
    if(availability == d->availability) return;

    QString str;
    switch(availability) {
        case Available:
            str = QLatin1String("Available");
            break;
        case NotAvailable:
            str = QLatin1String("NotAvailable");
            break;
        default:
        case Failed:
            str = QLatin1String("Failed");
            break;
    }

    d->availability = availability;
    setValue(QPOWERSOURCE_AVAILABILITY, str);
    emit availabilityChanged(availability);
}

/*!
  Set the \a charging status of the power source.
 */
void QPowerSourceProvider::setCharging(bool charging)
{

    if(charging == d->charging) return;

    d->charging = charging;
    setValue(QPOWERSOURCE_CHARGING, charging);
    emit chargingChanged(charging);
}

/*!
  Set the current \a charge of the power source.
 */
void QPowerSourceProvider::setCharge(int charge)
{
    if(charge == d->charge) return;
    
    Q_ASSERT(charge >= -1 && charge <= 100);
    d->charge = charge;
    setValue(QPOWERSOURCE_CHARGE, charge);
    emit chargeChanged(charge);
}

/*!
  Set the \a capacity, in milliamp-hourse, of the power source.
 */
void QPowerSourceProvider::setCapacity(int capacity)
{
    if(capacity == d->capacity) return;
    
    d->capacity = capacity;
    setValue(QPOWERSOURCE_CAPACITY, capacity);
    emit capacityChanged(capacity);
}

/*!
  Set the time \a remaining, in minutes, for the power source.
 */
void QPowerSourceProvider::setTimeRemaining(int remaining)
{
    if(remaining == d->timeRemaining) return;
    
    d->timeRemaining = remaining;
    setValue(QPOWERSOURCE_TIMEREMAINING, remaining);
    emit timeRemainingChanged(remaining);
}

struct QPowerStatusPrivate 
{
    QPowerStatusPrivate() : normalStatus(30), lowStatus(15), veryLowStatus(5), 
                            wallSource(0), batterySource(0) {}

    int normalStatus;
    int lowStatus;
    int veryLowStatus;
    QPowerSource *wallSource;
    QPowerSource *batterySource;

    QPowerStatus::WallStatus lastWall;
    QPowerStatus::BatteryStatus lastBattery;
};

/*!
  \class QPowerStatus
    \inpublicgroup QtBaseModule

  \brief The QPowerStatus class provides a simplified interface to the device's primary power sources.

  Applications may use the QPowerStatus class to obtain a simplified view of the
  system's power status.  A more complete view can be obtained by examining 
  the supported QPowerSource providers.

  Each device may contain two primary power sources - a primary wall power 
  source and a primary battery power source.  Both sources are optional, and
  a specific device may have one, both or neither.

  The primary power source sources are selected from the list of available 
  QPowerSource providers.  The selection may be configured in the 
  \c {Trolltech/HardwareAccessories} configuration file in the following way:

  \table
  \header \o Key \o Description
  \row \o PowerSources/PrimaryWallSource \o Name of the QPowerSource to use as the primary wall source.
  \row \o PowerSources/PrimaryBatterySource \o Name of the QPowerSource to use as the primary battery source.
  \endtable

  If the primary wall source is not configured, the \c {PrimaryAC} QPowerSource
  name is tried first, and, if not available, the first AC source is used.  If
  the primary wall source is configured, but the specified provider does not
  exist, then no QPowerSource is used.

  If the primary battery source is not configured, the \c {DefaultBattery} 
  QPowerSource is tried first, and, if not available, the first battery source
  is used.  If the primary battery source is configured, but the specified 
  provider does not exist, then no QPowerSource is used.

  The battery status level is determined by mapping a battery percentage to
  one the four defined levels.  The configuration for the various battery status
  levels are also defined in the \c {Trolltech/HardwareAccessories} 
  configuration file in the following way:

  \table
  \header \o Key \o Description
  \row \o PrimaryBattery/NormalStatus \o If the battery charge is above this 
  percentage level, it is considered Normal.  The default is 30.
  \row \o PrimaryBattery/LowStatus \o If the battery charge is above this 
  percentage level, but below the normal status level, it is considered Low.  
  The default is 15.
  \row \o PrimaryBattery/VeryLowStatus \o If the battery charge is above this 
  percentage level, but below the low status level, it is considered very Low.  
  The default is 5.
  \endtable

  If any one of these values is specified illegally (for example, the 
  NormalStatus is lower than the LowStatus), the default values are used.
  
  \ingroup hardware

  \sa QPowerSource
 */

/*!
  \enum QPowerStatus::WallStatus 

  Describes the status of the device's wall connection.

  \value Available The device is connected to a wall power supply.
  \value NotAvailable The device is not connected to a wall power supply, but 
         connection is supported.
  \value NoWallSource The device is not connected to a wall power supply, and no
         such connection is supported.
 */

/*!
  \enum QPowerStatus::BatteryStatus

  Describes the status of the device's battery connection.

  \value Normal The battery level is within normal operational range.
  \value Low The battery is low.  
  \value VeryLow The battery is very low.  
  \value Critical The battery is at a critical level.  The device should shut
  down to preserve data.
  \value NoBattery There is no battery in the device.
  */

/*!
  Construct a new QPowerStatus instance with the specified \a parent.
 */
QPowerStatus::QPowerStatus(QObject *parent)
: QObject(parent), d(0)
{
    d = new QPowerStatusPrivate;


    QSettings cfg("Trolltech", "HardwareAccessories");
    // Set status levels
    int normal;
    int low;
    int veryLow;
    cfg.beginGroup("PrimaryBattery");
    normal = cfg.value("NormalStatus", 30).toInt();
    low = cfg.value("LowStatus", 15).toInt();
    veryLow = cfg.value("VeryLowStatus", 5).toInt();
    cfg.endGroup();

    if(normal > low && low > veryLow) {
        d->normalStatus = normal;
        d->lowStatus = low;
        d->veryLowStatus = veryLow;
    } // else keep defaults

    // Locate the primary battery source and primary wall source
    QString wall;
    QString battery;
    cfg.beginGroup("PowerSources");
    wall = cfg.value("PrimaryWallSource").toString();
    battery = cfg.value("PrimaryBatterySource").toString();
    cfg.endGroup();

    QStringList accessories = 
        QHardwareManager::providers<QPowerSource>();

    if(!wall.isEmpty()) {
        if(accessories.contains(wall)) 
            d->wallSource = new QPowerSource(wall, this);
    } else if(accessories.contains(QLatin1String("PrimaryAC"))) {
        d->wallSource = new QPowerSource(QLatin1String("PrimaryAC"), this);
    } else {
        for(int ii = 0; !d->wallSource && ii < accessories.count(); ++ii) {
            d->wallSource = new QPowerSource(accessories.at(ii), this);
            if(d->wallSource->type() != QPowerSource::Wall) {
                delete d->wallSource;
                d->wallSource = 0;
            }
        }
    }
    if(d->wallSource) {
        QObject::connect(d->wallSource, 
                         SIGNAL(availabilityChanged(QPowerSource::Availability)),
                         this,
                         SLOT(wallAvailabilityChanged()));
    }

    if(!battery.isEmpty()) {
        if(accessories.contains(battery)) 
            d->batterySource = new QPowerSource(battery, this);
    } else if(accessories.contains(QLatin1String("DefaultBattery"))) {
        d->batterySource = new QPowerSource(QLatin1String("DefaultBattery"), 
                                            this);
    } else {
        for(int ii = 0; !d->batterySource && ii < accessories.count(); ++ii) {
            d->batterySource = new QPowerSource(accessories.at(ii), this);
            if(d->batterySource->type() != QPowerSource::Battery) {
                delete d->batterySource;
                d->batterySource = 0;
            }
        }
    }

    if(d->batterySource) {
        QObject::connect(d->batterySource,
                         SIGNAL(chargeChanged(int)), 
                         this,
                         SLOT(batteryChargeChanged()));
        QObject::connect(d->batterySource,
                         SIGNAL(chargingChanged(bool)),
                         this,
                         SIGNAL(batteryChargingChanged(bool)));
    }

    d->lastWall = wallStatus();
    d->lastBattery = batteryStatus();
}

/*!
  Destroy the QPowerStatus instance.
 */
QPowerStatus::~QPowerStatus()
{
    delete d;
    d = 0;
}

/*!
  \fn void QPowerStatus::wallStatusChanged(QPowerStatus::WallStatus status)

  This signal is emitted whenever the wall \a status changes.

  \sa QPowerStatus::wallStatus()
*/

/*!
  \fn void QPowerStatus::batteryStatusChanged(QPowerStatus::BatteryStatus status)

  This signal is emitted whenever the battery \a status changes.

  \sa QPowerStatus::batteryStatus()
*/

/*!
  \fn void QPowerStatus::batteryChargingChanged(bool charging)

  This signal is emitted when the batteries \a charging flag changes.

  \sa QPowerStatus::batteryCharging()
*/

/*!
  Return the status of the primary wall power source, or NoWallSource if there
  is no primary wall power source.
 */
QPowerStatus::WallStatus QPowerStatus::wallStatus() const
{
    if(!d->wallSource)
        return NoWallSource;
    else if( d->wallSource->availability() == QPowerSource::Available)
        return Available;
    else
        return NotAvailable;
}

/*!
  Returns true if the primary battery is charging, false if not.  If there is
  no primary battery, false is returned.
 */
bool QPowerStatus::batteryCharging() const
{
    return (d->batterySource)?(d->batterySource->charging()):false;
}

/*!
  Returns the status of the primary battery power source, or NoBattery if there
  is no primary battery source.
 */
QPowerStatus::BatteryStatus QPowerStatus::batteryStatus() const
{
    if(!d->batterySource)
        return NoBattery;

    int charge = d->batterySource->charge();
    if(-1 == charge || charge >= d->normalStatus)
        return Normal;
    else if(charge >= d->lowStatus)
        return Low;
    else if(charge >= d->veryLowStatus)
        return VeryLow;
    else
        return Critical;
}

/*!
  Returns a pointer to the QPowerSource instance representing the primary wall
  power source, or null if there is no primary wall power source.
  */
QPowerSource *QPowerStatus::wallSource() const
{
    return d->wallSource;
}

/*!
  Returns a pointer to the QPowerSource instance representing the primary 
  battery power source, or null if there is no primary battery power source.
  */
QPowerSource *QPowerStatus::batterySource() const
{
    return d->batterySource;
}

void QPowerStatus::wallAvailabilityChanged()
{
    if(wallStatus() != d->lastWall) {
        d->lastWall = wallStatus();
        emit wallStatusChanged(d->lastWall);
    }
}

void QPowerStatus::batteryChargeChanged()
{
    if(batteryStatus() != d->lastBattery) {
        d->lastBattery = batteryStatus();
        emit batteryStatusChanged(d->lastBattery);
    }
}

Q_IMPLEMENT_USER_METATYPE_ENUM(QPowerSource::Availability);
