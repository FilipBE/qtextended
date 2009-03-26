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

#ifndef E1_BATTERY_H
#define E1_BATTERY_H

#include <QObject>
#include <QValueSpaceItem>

class E1Battery : public QObject
{
Q_OBJECT
public:
    E1Battery(QObject *parent = 0);
    virtual ~E1Battery();

    int charge();

signals:
    void chargeChanged(int);

protected:
    virtual void timerEvent(QTimerEvent *);

private slots:
    void batteryChanged();

private:
    void startcharge();
    void stopcharge();
    int m_timerId;
    QValueSpaceItem m_battery;
    int m_charge;
};

#endif
