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

#include "appointmentdetails.h"
#include "../todolist/reminderpicker.h"

#include <qtopianamespace.h>
#include <qtimestring.h>
#include <qappointment.h>
#include <QAppointmentModel>
#include <QPimContext>
#include <QDL>
#include <QDLBrowserClient>
#include <QStyle>
#include <qglobal.h>
#include <QKeyEvent>

AppointmentDetails::AppointmentDetails( QWidget *parent )
:   QDLBrowserClient( parent, "editnote" ),
    previousDetails( 0 ), mIconsLoaded(false)
{
    setFrameStyle(NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void AppointmentDetails::init( const QOccurrence &occurrence )
{
    QPalette p = palette();
    p.setBrush( QPalette::Base, qApp->palette(this).brush( QPalette::Base ) );
    setPalette( p );
    mOccurrence = occurrence;
    if (!mIconsLoaded) {
        /* precache some icons, scaled nicely - just using these as img src=... gives poor results */
        QIcon audible(":icon/audible");
        QIcon repeat(":icon/repeat");
        QIcon silent(":icon/silent");
        QIcon exception(":icon/repeatException");
        QIcon timezone(":icon/globe");
        QIcon readonly(":icon/readonly");
        int iconMetric = style()->pixelMetric(QStyle::PM_SmallIconSize);

        QTextDocument* doc = document();
        doc->addResource(QTextDocument::ImageResource, QUrl("audibleicon"), audible.pixmap(iconMetric));
        doc->addResource(QTextDocument::ImageResource, QUrl("repeaticon"), repeat.pixmap(iconMetric));
        doc->addResource(QTextDocument::ImageResource, QUrl("silenticon"), silent.pixmap(iconMetric));
        doc->addResource(QTextDocument::ImageResource, QUrl("exceptionicon"), exception.pixmap(iconMetric));
        doc->addResource(QTextDocument::ImageResource, QUrl("timezoneicon"), timezone.pixmap(iconMetric));
        doc->addResource(QTextDocument::ImageResource, QUrl("readonlyicon"), readonly.pixmap(iconMetric));
        mIconsLoaded = true;
    }
    setHtml( createPreviewText( mOccurrence ) );
    verifyLinks();
}

void AppointmentDetails::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
        case Qt::Key_Space:
        case Qt::Key_Return:
        case Qt::Key_Back:
            emit done();
            break;
        default:
            QTextBrowser::keyPressEvent( e );
            break;
    }
}

QString AppointmentDetails::formatDate(const QDate& date, const QDate& today)
{
    /* We want either "Mon, 26 Feb 2008" or "Mon, 26 Feb" if the year is the current year */
    if (date.year() == today.year())
        return tr("%1, %2", "[Mon], [26 Feb]").arg(QTimeString::localDayOfWeek(date), QTimeString::localMD(date, QTimeString::Short));
    else
        return tr("%1, %2", "[Mon], [26 Feb 2007]").arg(QTimeString::localDayOfWeek(date), QTimeString::localYMD(date, QTimeString::Medium));
}

/* Basically.. we try to format things usefully:

Six cases:
1 all day event (today)
2 all day event (not on today)
3 timed event today
4 timed event split over today and other days
5 timed event on other day
6 timed event split over other days

1: Today (all day)

2a: Monday 26th February 2007 (all day)
2b: Mon 26 Feb, 2007 (all day)

3: Today, 11:00am - 12:00pm

4: Today, 11:00pm - Mon 26 Feb, 2007, 3:00am
4: Sun Feb 25 2007, 11:00pm - Today, 11:00am

5: Mon 26 Feb 2007, 2:00pm - 3:00pm

6: Mon 26 Feb 2007, 11:00am - Tue 27 Feb 2007, 11:00pm

We can drop the year if it is the current year.

In the presence of timezones, we do:

3: Today, 11:00am - 12:00pm (3:00pm - 4:00pm Oslo)
4: Today, 11:00pm (3:00am Oslo) - Mon 26 Feb, 2007, 3:00am (6:00pm Oslo)
*/

QString AppointmentDetails::formatDateTimes(const QOccurrence& ev, const QDate& today)
{
    QString text;

    QDateTime start = ev.startInCurrentTZ();
    QDateTime end = ev.endInCurrentTZ();

    QString startdate(start.date() == today ? tr("Today") : formatDate(start.date(), today));
    QString enddate(end.date() == today ? tr("Today") : formatDate(end.date(), today));

    QString timeZoneText = ev.timeZone().city();
    if (timeZoneText.isEmpty())
        timeZoneText = ev.timeZone().name();
    if (timeZoneText.isEmpty())
        timeZoneText = ev.timeZone().id();

    if (ev.appointment().isAllDay()) {
        // Cases 1 and 2
        text = tr("%1 (all day)", "[Today] (all day) / [Mon Sep 27 2007] (all day)").arg(startdate);
    } else {
        if (start.date() == end.date()) {
            // Cases 3 and 5
            if ( ev.timeZone().isValid() && ev.timeZone() != QTimeZone::current() )
                text = tr("%1, %2 to %3 (%4 to %5 %6 time)", "[Mon Feb 26 2007], [3:00pm] - [6:00pm] ([6:00am] - [9:00am] [Oslo] time)")
                    .arg(startdate,
                    QTimeString::localHM(start.time()),
                    QTimeString::localHM(end.time()))
                    .arg(QTimeString::localHM(ev.start().time()),
                    QTimeString::localHM(ev.end().time()),
                    timeZoneText); // XXX qt/4.3 has up to 9 args.
            else
                text = tr("%1, %2 to %3", "[Mon Feb 26 2007], [3:00pm] to [6:00pm]")
                    .arg(startdate,
                    QTimeString::localHM(start.time()),
                    QTimeString::localHM(end.time()));
        } else {
            // Cases 4 and 6
            if ( ev.timeZone().isValid() && ev.timeZone() != QTimeZone::current() )
                text = tr("%1, %2 (%3 %4 time) to %5, %6  (%7 %4 time)", "[Mon Feb 26 2007], [3:00pm] ([6:00 am] [Oslo] time) to [Tue Feb 27 2007], [6:00pm] ([9:00am] [Oslo] time)")
                    .arg(startdate,
                    QTimeString::localHM(start.time()),
                    QTimeString::localHM(ev.start().time()))
                    .arg(timeZoneText,
                    enddate,
                    QTimeString::localHM(end.time()),
                    QTimeString::localHM(ev.end().time()));
            else
                text = tr("%1, %2 to %3, %4", "[Mon Feb 26 2007], [3:00pm] to [Tue Feb 27 2007], [6:00pm]")
                    .arg(startdate,
                    QTimeString::localHM(start.time()),
                    enddate,
                    QTimeString::localHM(end.time()));
        }
    }
    return text;
}

QString AppointmentDetails::createPreviewText( const QOccurrence &o )
{
    QAppointment ev = o.appointment();
    QString text;
    QString iconText;
    QDate today = QDate::currentDate();
    loadLinks( ev.customField( QDL::CLIENT_DATA_KEY ) );

    bool rtl = qApp->layoutDirection() == Qt::RightToLeft;

    QAppointmentModel am;
    QPimContext *ctx = am.context(ev.uid());
    if (ctx) {
        if (!ctx->editable(ev.uid()))
            iconText += QLatin1String("<img src=\"readonlyicon\">");
        iconText += QLatin1String("<img src=\"contexticon\">");
        int iconMetric = style()->pixelMetric(QStyle::PM_SmallIconSize);
        document()->addResource(QTextDocument::ImageResource, QUrl("contexticon"), ctx->icon().pixmap(iconMetric));
    }

    if ( ev.hasRepeat() )
        iconText += QLatin1String("<img src=\"repeaticon\">");
    else if ( ev.isException() )
        iconText += QLatin1String("<img src=\"exceptionicon\">");

    if ( ev.hasAlarm() ) {
        if (ev.alarm() == QAppointment::Audible)
            iconText += QLatin1String("<img src=\"audibleicon\">");
        else
            iconText += QLatin1String("<img src=\"silenticon\">");
    }

    if ( ev.timeZone().isValid() )
        iconText += QLatin1String("<img src=\"timezoneicon\">");

    if (!iconText.isEmpty())
        text += "<table style='float:"+ QLatin1String(rtl ? "left":"right") +"'><tr><td>" + iconText + "</td></tr></table>";

    text += "<b>";

    if ( !ev.description().isEmpty() )
        text += Qt::escape(ev.description());
    else
        text += tr("No description", "no description entered for appointment");

    text += "</b>";

    if ( !ev.location().isEmpty() )
        text += "<br><b>" + tr("Where:") + " </b>" + Qt::escape(ev.location());

    text += "<br><b>" + tr("When:")+ " </b>" + formatDateTimes(o, today);

    if ( ev.hasAlarm() ) {
        text += "<br><b>" + tr("Reminder:") + " </b>" + ReminderPicker::formatReminder(ev.isAllDay(), ev.alarm(), ev.alarmDelay());
    }
    if ( ev.hasRepeat() ) {
        QString word;
        if ( ev.repeatRule() == QAppointment::Daily )
            if ( ev.frequency() > 1 )
                word = tr("every %1 days", "eg. every 2 days", ev.frequency());
            else
                word = tr("every day");
        else if ( ev.repeatRule() == QAppointment::Weekly ) {
            int repDays = 0;
            int dayOfWeek;
            QString repStr;

            for (dayOfWeek = 1; dayOfWeek <= 7; dayOfWeek++) {
                if (ev.repeatOnWeekDay(dayOfWeek)) {
                    repDays++;
                }
            }

            if ( ev.frequency() > 1 )
                word = tr("every %2 weeks on %1", "eg. every 2 weeks on: Monday, Wednesday", ev.frequency());
            else
                word = tr("every week on %1", "e.g. every week on: Monday, Thursday");

            if (repDays > 0) {
                Qt::DayOfWeek startDay = Qtopia::weekStartsOnMonday() ? Qt::Monday : Qt::Sunday;
                dayOfWeek = startDay;
                do {
                    if (ev.repeatOnWeekDay(dayOfWeek)) {
                        if (!repStr.isEmpty())
                            repStr += tr(",", "list separator - e.g. Monday_, _Tuesday") + " ";
                        repStr += QTimeString::nameOfWeekDay(dayOfWeek, QTimeString::Long);
                    }
                    if (dayOfWeek == 7)
                        dayOfWeek = 1;
                    else
                        dayOfWeek++;
                } while (dayOfWeek != startDay);
            } else {
                repStr = QTimeString::nameOfWeekDay(ev.startInCurrentTZ().date().dayOfWeek(), QTimeString::Long);
            }

            word = word.arg(repStr);
        }
        else if ( ev.repeatRule() == QAppointment::MonthlyDate ||
                ev.repeatRule() == QAppointment::MonthlyDay ||
                ev.repeatRule() == QAppointment::MonthlyEndDay )
            /// XXX this could also get extra information
            if ( ev.frequency() > 1 )
                word = tr("every %1 months", "eg. every 2 months", ev.frequency());
            else
                word = tr("every month");
        else if ( ev.repeatRule() == QAppointment::Yearly )
            if ( ev.frequency() > 1 )
                word = tr("every %1 years", "eg. every 2 years", ev.frequency());
            else
                word = tr("every year");

        text += "<br><b>" + tr("Repeat:") + " </b>";
        if ( ev.frequency() > 1 )
            word = word.arg( ev.frequency() );

        QString endword;
        if ( ev.repeatForever() )
            endword = tr("forever");
        else
            endword = tr("ending on %1", "1=date").arg(formatDate(ev.repeatUntil(), today));

        text += tr("From %1, %2, %3", "1=date, 2=every x days/weeks/etc, 3=ending on date/forever")
            .arg(formatDate(ev.startInCurrentTZ().date(), today))
            .arg(word)
            .arg(endword);
    }

    if ( !ev.notes().isEmpty() ) {
        QString txt = ev.notes();
        text += "<br><b>" + tr("Notes:") + " </b>" + txt; // txt is already formatted html
    }

    return text;
}

