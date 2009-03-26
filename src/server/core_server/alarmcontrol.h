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

#ifndef ALARMCONTROL_H
#define ALARMCONTROL_H

#include <QObject>
#include <qvaluespace.h>
#include <QtopiaIpcAdaptor>
#include <QDateTime>

class AlarmControl : public QObject
{
Q_OBJECT
public:
    AlarmControl(QObject* parent = 0);
    bool alarmState() const;
    void alarmEnabled(bool);

signals:
    void alarmStateChanged(bool);

private:
    bool alarmOn;
    QValueSpaceObject alarmValueSpace;
};

class AlarmServerService : public QtopiaIpcAdaptor
{
    Q_OBJECT
public:
    ~AlarmServerService();

public slots:
    void addAlarm( QDateTime when, const QString& channel,
                   const QString& msg, int data );
    void deleteAlarm( QDateTime when, const QString& channel,
                      const QString& msg, int data );
    void dailyAlarmEnabled( bool flag );
private:
    AlarmServerService( QObject *parent = 0 );
    AlarmControl* ctrl;
    friend class AlarmControl;
    void initAlarmServer();
};

#endif
