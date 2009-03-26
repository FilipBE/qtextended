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

#include "timeupdateservice.h"
#include <QtopiaApplication>
#include <QValueSpace>
#include <QtopiaIpcEnvelope>
#include <QtopiaService>
#include <QTimeZone>
#include "qtopiaserverapplication.h"

#include <sys/time.h>


TimeUpdater::TimeUpdater(int timeoffset, const QTimeZone& tz, bool chtime, bool chzone)
    : offset(timeoffset), zone(tz), updatetime(chtime), updatezone(chzone)
{
    prompt = 0;
}

void TimeUpdater::userCommit(int r)
{
    timer.stop();
    if (r == QMessageBox::Yes)
        commitUpdate();
}

void TimeUpdater::ask()
{
    delete prompt;
    if (!updatetime && !updatezone) {
        prompt = 0;
        return;
    }
    prompt = new QMessageBox(QMessageBox::Question, tr("Time Changed"), QString(), QMessageBox::Yes | QMessageBox::No);
    connect(prompt, SIGNAL(finished(int)), this, SLOT(userCommit(int)));
    prompt->setWindowModality(Qt::WindowModal);
    if (updatetime)
        timer.start(1000,this);
    else
        timer.stop();
    timerEvent(0);
    QtopiaApplication::showDialog(prompt);
}
void TimeUpdater::timerEvent(QTimerEvent*)
{
    if (!updatetime) {
        if (!updatezone)
            return;
        prompt->setText(tr("<qt>The network time zone is %1. Set this new time zone?</qt>").arg(zone.name()));
    } else {
        QDateTime lt = zone.fromTime_t(time(0)+offset);
        QDateTime now = QDateTime::currentDateTime();
        QString localtime;
        if ( now.date() == lt.date() )
            localtime = QTimeString::localHMS(lt.time(),QTimeString::Long);
        else
            localtime = QTimeString::localYMDHMS(lt,QTimeString::Long);
        if (updatezone)
            localtime += " (" + zone.name() + ")";

        // XXX DEBUG
        //localtime += " " + QString::number(offset) + "s";

        prompt->setText(tr("<qt>The network time is %1. Set this new time?</qt>").arg(localtime));
    }
}

void TimeUpdater::commitUpdate()
{
    emit changeSystemTime(updatetime ? time(0)+offset : 0, zone.id());
}

/*!
    \service TimeUpdateService TimeUpdate
    \inpublicgroup QtBaseModule
    \brief The TimeUpdateService class provides the TimeUpdate service.

    The \i TimeUpdate service monitors time and timezone data from sources
    such as the modem and updates the system time and timezone accordingly.

    The following settings control the automatic update:

    Trolltech/locale/Location/TimezoneAuto

    Trolltech/locale/Location/TimeAuto

    Trolltech/locale/Location/TimezoneAutoPrompt

    Trolltech/locale/Location/TimeAutoPrompt
*/

/*! \internal */
TimeUpdateService::TimeUpdateService()
    : QtopiaAbstractService("TimeUpdate", 0),
      lasttz(-1),externalTimeTimeStamp(0), externalTime(0), externalDstOffset(0),
      updater(0)
{
    vsObject = new QValueSpaceObject("/System/Time/ExternalTimeSource", this);
    publishAll();

#ifdef __UCLIBC__
    timeZoneAlarm();
#endif
}


/*! \internal */
void TimeUpdateService::changeSystemTime(uint newutc, QString newtz)
{
    if (newutc) {
        if ( externalTimeTimeStamp ) {
            int deltatime = newutc - ::time(0);
            externalTimeTimeStamp += deltatime;
            vsObject->setAttribute(externalSourceId+"/LastUpdate",externalTimeTimeStamp);
        }
        struct timeval myTv;
        ::gettimeofday( &myTv, 0 );
        uint oldutc = myTv.tv_sec;
        myTv.tv_sec = newutc;
        // Leave usec as it is to avoid timestamp skew
        ::settimeofday( &myTv, 0 );
        Qtopia::writeHWClock();

        // Advertise the change (old utc and new utc)
        QtopiaIpcEnvelope( "QPE/System", "timeChange(uint,uint)" )
            << oldutc
            << newutc
        ;
    }

    // This should be the only place in Qtopia that does this...
    QTimeZone::setSystemTimeZone(newtz);

#ifdef __UCLIBC__
    Qtopia::deleteAlarm(QDateTime(), "TimeUpdate", "timeZoneAlarm()");

    QRegExp expr("[^,]*,(\\d+)/(\\d+:\\d\\d:\\d\\d),(\\d+)/(\\d+:\\d\\d:\\d\\d)");
    // we need to use getenv here to get the uClibc TZ environment variable
    // QTimeZone::current() returns the corresponding zoneinfo string.
    if (expr.indexIn(getenv("TZ")) > -1) {
        int dstStartDay = expr.cap(1).toInt() + 1;
        QTime dstStartTime = QTime::fromString(expr.cap(2), "h:mm:ss");
        int dstEndDay = expr.cap(3).toInt() + 1;
        QTime dstEndTime = QTime::fromString(expr.cap(4), "h:mm:ss");

        QDateTime now = QDateTime::currentDateTime();

        // find next DST end time
        QDateTime dstEnd = QDateTime(QDate(now.date().year(), 1, 1), dstEndTime);
        if (dstStartDay > dstEndDay && now.date().dayOfYear() > dstEndDay)
            dstEnd = dstEnd.addYears(1);
        dstEnd = dstEnd.addDays(dstEndDay - 1);

        Qtopia::addAlarm(dstEnd, "TimeUpdate", "timeZoneAlarm()");
    }
#endif

    // set the time (from system) and zone (given to their TZ) for everyone else...
    QtopiaIpcEnvelope setTimeZone( "QPE/System", "timeChange(QString)" );
    setTimeZone << newtz;
}

#ifdef __UCLIBC__
/*!
    \internal

    Periodically updates the daylight savings start and end times on systems that require it.
*/
void TimeUpdateService::timeZoneAlarm()
{
    changeSystemTime(0, QTimeZone::current().id());
}
#endif

/*!
  Records an external time update from a source identified by \a sourceid. The recorded time is
  \a time (a UNIX time_t measuring UTC seconds since the start of 1970), in the given \a time_zone
  (the number of minutes east of GMT), with \a dstoffset minutes of daylight savings adjustment
  already applied to the \a time_zone.

  This time is recorded and may be used immediately or in the future upon user request.

  The \a sourceid is currently not used, so only one source should be enabled.
*/
void TimeUpdateService::storeExternalSource(QString sourceid,uint time,int time_zone,int dstoffset)
{
    // Allowing multiple sources would require changing the UI
    // to allow the user to select a source or sources to use,
    // possibly prioritized, and would require this class to
    // distinguish between different sources according to the user's wishes.
    //
    externalSourceId = sourceid; // not really used, just one supported

    vsObject->setAttribute(sourceid,"1");

    externalTimeTimeStamp = ::time(0);
    vsObject->setAttribute(sourceid+"/LastUpdate",externalTimeTimeStamp);

    if ( time ) {
        externalTime = time;
        externalDstOffset = dstoffset;
        vsObject->setAttribute(sourceid+"/Time",externalTime);
        vsObject->setAttribute(sourceid+"/DstOffset",externalDstOffset);
    }

    if ( time_zone != lasttz ) {
        lasttz = time_zone;
        externalTimeZone = QTimeZone::findFromMinutesEast(QDateTime::currentDateTime(),time_zone,externalDstOffset!=0).id();
        vsObject->setAttribute(sourceid+"/TimeZone",externalTimeZone);
    }

    updateFromExternalSources();
}


/*! \internal */
void TimeUpdateService::updateFromExternalSources()
{
    QSettings lconfig("Trolltech","locale");

    bool autotz = lconfig.value("Location/TimezoneAuto").toBool();
    bool autotm = lconfig.value("Location/TimeAuto").toBool();

    if ( !autotz && !autotm )
        return;

    bool ask_tz = lconfig.value("Location/TimezoneAutoPrompt").toBool();
    bool ask_time = lconfig.value("Location/TimeAutoPrompt").toBool();

    updateFromExternalSources(autotz, autotm, ask_tz, ask_time);
}

/*!
  Analyzes the most recent external time information and updates the
  time zone if \a autotz, and the time if \a autotm, prompting
  respectively if \a ask_tz and \a ask_time. Only a single prompt
  is ever used, if at all.

  To avoid annoying the user, changes of less than 60 seconds are
  applied without prompting if \a autotm is true.
*/
void TimeUpdateService::updateFromExternalSources(bool autotz, bool autotm, bool ask_tz, bool ask_time)
{
    // Somewhat arbitrary.
    //
    // Too low annoys the user.
    // Too high may cause confusion (eg. files on system dated after now).
    const int maxtimejitter = 60;

    bool tzchanged = autotz && externalTimeZone != QTimeZone::current().id();
    bool tchanged = autotm;

    if (!tzchanged && !tchanged || externalTimeZone.isEmpty())
        return;

    QTimeZone zone(externalTimeZone.toLatin1());

    delete updater;
    updater = new TimeUpdater(externalTime-externalTimeTimeStamp, zone, tchanged, tzchanged);
    connect(updater,SIGNAL(changeSystemTime(uint,QString)),
               this,SLOT(changeSystemTime(uint,QString)));

    if ( ask_time && abs(externalTime-externalTimeTimeStamp) < maxtimejitter )
        ask_time = false;

    if ( ask_tz && tzchanged || ask_time && tchanged )
        updater->ask();
    else
        updater->commitUpdate();
}

QTOPIA_TASK(TimeUpdateService, TimeUpdateService);

