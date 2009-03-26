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

#include "clock.h"
#include "stopwatch.h"
#include "alarm.h"
#include "clockmain.h"

#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#include <qsettings.h>
#include <qtimestring.h>
#include <QtopiaServiceRequest>
#include <qsoftmenubar.h>
#include <qtabwidget.h>
#include <qmessagebox.h>
#include <qmenu.h>

ClockMain::ClockMain(QWidget *parent, Qt::WFlags f)
    : QWidget(parent, f)
    , clockIndex(-1)
    , stopwatchIndex(-1)
    , alarmIndex(-1)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    tabWidget = new QTabWidget(this);
    layout->addWidget(tabWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    clock = new Clock(tabWidget);
    clock->setFocusPolicy(Qt::TabFocus);    //allows left/right to switch to other tabs
    stopWatch = new StopWatch(tabWidget);
    alarm = new Alarm(tabWidget);

    // Add each widget to the tabbed widget, and save the index for each so that
    // we can refer to it later.
    clockIndex = tabWidget->addTab(clock, tr("Clock"));
    alarmIndex = tabWidget->addTab(alarm, tr("Alarm"));
    stopwatchIndex = tabWidget->addTab(stopWatch, tr("Stopwatch"));

    contextMenu = QSoftMenuBar::menuFor(this);
    QAction *action = contextMenu->addAction(tr("Set Time..."));
    connect(action, SIGNAL(triggered()), this, SLOT(setTime()));


    connect( qApp, SIGNAL(appMessage(QString,QByteArray)),
            this, SLOT(appMessage(QString,QByteArray)) );
    new ClockService(this);
    new AlarmService(this);

    setWindowTitle(tr("Clock"));
}

ClockMain::~ClockMain()
{
}

void ClockMain::showClock()
{
    tabWidget->setCurrentIndex(clockIndex);
    QtopiaApplication::instance()->showMainWidget();
}

void ClockMain::editAlarm()
{
    tabWidget->setCurrentIndex(alarmIndex);
    QtopiaApplication::instance()->showMainWidget();
}

void ClockMain::setDailyEnabled( bool enable )
{
    alarm->setDailyEnabled( enable );
}

void ClockMain::appMessage( const QString &msg, const QByteArray &data )
{
    if ( msg == "alarm(QDateTime,int)" ) {
        QDataStream ds(data);
        QDateTime when;
        int t;
        ds >> when >> t;
        alarm->triggerAlarm(when, t);
    }
}

void ClockMain::setTime()
{
    QtopiaServiceRequest req("Time", "editTime()");
    req.send();
}

/*!
    \service ClockService Clock
    \inpublicgroup QtEssentialsModule
    \brief The ClockService class provides the Clock service.

    The \i Clock service enables applications to pop up the clock on
    the user's display by sending it a \c{showClock()} message.

    Client applications can request the \i Clock service with the
    following code:

    \code
    QtopiaServiceRequest req( "Clock", "showClock()" );
    req.send();
    \endcode

    \sa AlarmService
*/

/*!
    \internal
*/
ClockService::~ClockService()
{
}

/*!checkboxes[i]->isChecked()
    Instruct the \i Clock service to pop up the clock on the
    user's display.

    This slot corresponds to the QCop service message \c{Clock::showClock()}.
*/
void ClockService::showClock()
{
    parent->showClock();
}

/*!
    \service AlarmService Alarm
    \inpublicgroup QtEssentialsModule
    \brief The AlarmService class provides the Alarm service.

    The \i Alarm service enables applications to pop up a dialog on
    the user's display to edit the alarm time.

    Client applications can request the \i Alarm service with the
    following code:

    \code
    QtopiaServiceRequest req( "Alarm", "editAlarm()" );
    req.send();
    \endcode

    \sa ClockService
*/

/*!
    \internal
*/
AlarmService::~AlarmService()
{
}

/*!
    Instruct the \i Alarm service to pop up a dialog box that permits
    editing of the alarm.

    This slot corresponds to the QCop service message \c{Alarm::editAlarm()}.
*/
void AlarmService::editAlarm()
{
    parent->editAlarm();
}

/*!
    Instruct the \i Alarm service to enable or disable the daily alarm
    according to \a flag.

    This slot corresponds to the QCop service message
    \c{Alarm::setDailyEnabled(int)}.
*/
void AlarmService::setDailyEnabled( int flag )
{
    parent->setDailyEnabled( flag != 0 );
}
