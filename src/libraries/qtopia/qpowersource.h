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

#ifndef QPOWERSOURCE_H
#define QPOWERSOURCE_H

#include <QHardwareInterface>
#include <qtopiaipcmarshal.h>

class QTOPIA_EXPORT QPowerSource : public QHardwareInterface
{
Q_OBJECT
public:
    enum Type { Wall, Battery, Virtual };
    enum Availability { Available, NotAvailable, Failed };

    explicit QPowerSource(const QString &id = QString(), QObject *parent = 0);
    virtual ~QPowerSource();


    Type type() const;
    Availability availability() const;
    bool charging() const;
    int charge() const;
    int capacity() const;
    int timeRemaining() const;

signals:
    void availabilityChanged(QPowerSource::Availability);
    void chargingChanged(bool);
    void chargeChanged(int);
    void capacityChanged(int);
    void timeRemainingChanged(int);

private:
    friend class QPowerSourceProvider;
    QPowerSource(const QString &id, QObject *parent, 
                 QAbstractIpcInterface::Mode mode);
};
Q_DECLARE_USER_METATYPE_ENUM(QPowerSource::Availability);

class QPowerSourceProviderPrivate;
class QTOPIA_EXPORT QPowerSourceProvider : public QPowerSource
{
Q_OBJECT
public:
    explicit QPowerSourceProvider(Type type, const QString &id, QObject *parent = 0);
    virtual ~QPowerSourceProvider();

public slots:
    void setAvailability(QPowerSource::Availability availability);
    void setCharging(bool charging);
    void setCharge(int);
    void setCapacity(int);
    void setTimeRemaining(int);

private:
    QPowerSourceProviderPrivate *d;
};

class QPowerStatusPrivate;
class QTOPIA_EXPORT QPowerStatus : public QObject
{
Q_OBJECT
public:
    QPowerStatus(QObject * = 0);
    virtual ~QPowerStatus();

    enum WallStatus { Available, NotAvailable, NoWallSource };
    enum BatteryStatus { Normal, Low, VeryLow, Critical, NoBattery };

    WallStatus wallStatus() const;
    bool batteryCharging() const;
    BatteryStatus batteryStatus() const;

    QPowerSource *wallSource() const;
    QPowerSource *batterySource() const;

signals:
    void wallStatusChanged(QPowerStatus::WallStatus);
    void batteryStatusChanged(QPowerStatus::BatteryStatus);
    void batteryChargingChanged(bool);

private slots:
    void wallAvailabilityChanged();
    void batteryChargeChanged();

private:
    QPowerStatusPrivate *d;
};

#endif
