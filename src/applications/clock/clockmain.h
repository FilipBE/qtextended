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

#ifndef CLOCKMAIN_H
#define CLOCKMAIN_H

#include <qwidget.h>
#include <qtopiaabstractservice.h>

class Clock;
class StopWatch;
class Alarm;
class QTabWidget;
class QMenu;
class QAction;

class ClockMain : public QWidget
{
    Q_OBJECT
public:
    ClockMain( QWidget *parent=0, Qt::WFlags fl=0 );
    ~ClockMain();

private slots:
    void appMessage(const QString& msg, const QByteArray& data);

public slots:
    void showClock();
    void editAlarm();
    void setDailyEnabled( bool enable );
    void setTime();

private:
    QTabWidget *tabWidget;

    Clock *clock;
    StopWatch *stopWatch;
    Alarm *alarm;
    QMenu *contextMenu;

    int clockIndex;
    int stopwatchIndex;
    int alarmIndex;
};

class ClockService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class ClockMain;
private:
    ClockService( ClockMain *parent )
        : QtopiaAbstractService( "Clock", parent )
        { this->parent = parent; publishAll(); }

public:
    ~ClockService();

public slots:
    void showClock();

private:
    ClockMain *parent;
};

class AlarmService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class ClockMain;
private:
    AlarmService( ClockMain *parent )
        : QtopiaAbstractService( "Alarm", parent )
        { this->parent = parent; publishAll(); }

public:
    ~AlarmService();

public slots:
    void editAlarm();
    void setDailyEnabled( int flag );

private:
    ClockMain *parent;
};

#endif
