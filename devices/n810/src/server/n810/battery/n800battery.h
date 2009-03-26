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

#ifndef N800BATTERY_H
#define N800BATTERY_H

#include <QObject>
#include <QSocketNotifier>
#include <QDBusConnection>
#include <QDBusInterface>
class QPowerSourceProvider;

class N800Battery : public QObject
{
Q_OBJECT
public:
    N800Battery(QObject *parent = 0);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    bool APMEnabled() const;
    int percentCharge;

    QPowerSourceProvider *ac;
    QPowerSourceProvider *battery;
//    QPowerSourceProvider *backup;

    QSocketNotifier *powerNotify;
//    int  kbdFDpower;
    int getBatteryLevel();
    bool batteryIsFull();

private Q_SLOTS:
    void updateStatus();
    void batteryPropertyChanged(QString, QString);

    void chargerDiconnected();
    void chargerConnected();
    void batteryFull();
    void chargerOff();
    void chargerOn();
    void batteryTimeLeft(uint, uint);
    void batteryStateChanged(uint);
};

#endif
