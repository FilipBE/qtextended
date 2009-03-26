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

#include "qappointment.h"
#include <qtopiaapplication.h>
#include <qtopianamespace.h>

#include "vobject_p.h"
#include "qappointmentsqlio_p.h"

#include <qbuffer.h>
#include <qtextcodec.h>
#include <QFile>

#include <QDebug>

#include <time.h>

#include <stdio.h>

class QAppointmentData : public QSharedData
{
public:
    QString mDescription;
    QString mLocation;
    QDateTime mStart;
    QDateTime mEnd;
    QString mNotes;
    QTimeZone mTimeZone;

    int mAlarmDelay;
    QAppointment::AlarmFlags mAlarmSound;

    int mType;
    int mFrequency;
    QDate mRepeatUntil;
    bool mShowOnNearest;
    QAppointment::WeekFlags weekMask;
    bool mAllDay;

    QUniqueId mExceptionParent;
    QDate mExceptionDate;

    QList<QAppointment::Exception> exceptions;

    QUniqueId mUid;
    QList<QString> mCategories;
    QMap<QString, QString> customMap;

    bool isException(const QDate date) const {
        foreach(QAppointment::Exception e, exceptions) {
            if (e.date == date)
                return true;
        }
        return false;
    }
};

static QDateTime trimSeconds( const QDateTime &dt )
{
    QDateTime r = dt;
    r.setTime( QTime(dt.time().hour(), dt.time().minute()) );
    return r;
}

/*!
  \class QAppointment
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QAppointment class holds the data of a calendar appointment.

  This data includes descriptive data of the appointment and scheduling information.
  Appointments can also repeat and have exceptions to their repeat rules. 
  Repetition of appointments is defined by the appointment's repeatRule(),
  frequency(), weekFlags() and start().  The repeat rule defines what
  type of repetition the appointment has and how to interpret the
  other aspect of repetition information.  For instance Daily indicates
  that the appointment repeats every N days, where N is provided by the
  frequency() of the appointment.  Whereas Weekly would indicate every
  N weeks, where N is the frequency() and the weekFlags() function
  indicates on which days of the week the appointment occurs.

  The start date is used to determine the Instance of the occurrence.  For
  example if the start date of the appointment is the 16th of April,
  then for the purposes of the repeat MonthlyDate rule, the Instance is
  16.  This differs from some other systems that explicitly store the
  Instance information separate from the start information.  In Qt Extended the start of an appointment must always be a valid occurrence for the
  appointments repetition behavior.

  \sa {Pim Library}
*/

/*!
  \enum QAppointment::RepeatRule

  This enum type defines how an appointment repeats.

  \value NoRepeat
    appointment occurs only once
  \value Daily
    appointment occurs every N days
  \value Weekly
    appointment occurs every N weeks
  \value MonthlyDate
    appointment occurs on the Xth day of the month every N months
  \value MonthlyDay
    appointment occurs on the Xth week of the month every N months
  \value MonthlyEndDay
    appointment occurs on the Xth last week of the month every N months
  \value Yearly
    appointment occurs every N years

  \sa frequency(), weekOffset(), showOnNearest(), repeatOnWeekDay()
*/

/*!
  \enum QAppointment::AlarmFlag

  This enum type defines the sound that is made when an appointment alarm occurs
  The currently defined types are:

  \value NoAlarm
    The appointment has no alarm set
  \value Visible
    The appointment has dialog popup alarm set
  \value Audible
    The appointment has an audible alarm set.

  \sa setAlarm(), alarm()
*/

/*!
  \enum QAppointment::WeekFlag

  The WeekFlags only apply to appointments that have a RepeatRule of Weekly,
  MonthlyDay or MonthlyEndDay.

  \value OccurMonday
    The appointment occurs on each Monday
  \value OccurTuesday
    The appointment occurs on each Tuesday
  \value OccurWednesday
    The appointment occurs on each Wednesday
  \value OccurThursday
    The appointment occurs on each Thursday
  \value OccurFriday
    The appointment occurs on each Friday
  \value OccurSaturday
    The appointment occurs on each Saturday
  \value OccurSunday
    The appointment occurs on each Sunday
*/

/*!
  \overload
*/
QUniqueId &QAppointment::uidRef() { return d->mUid; }

/*!
  \overload
*/
const QUniqueId &QAppointment::uidRef() const { return d->mUid; }

/*!
  \overload
*/
QList<QString> &QAppointment::categoriesRef() { return d->mCategories; }
/*!
  \overload
*/
const QList<QString> &QAppointment::categoriesRef() const { return d->mCategories; }

/*!
  \overload
*/
QMap<QString, QString> &QAppointment::customFieldsRef() { return d->customMap; }
/*!
  \overload
*/
const QMap<QString, QString> &QAppointment::customFieldsRef() const { return d->customMap; }

/*!
  Returns the description of the appointment.

  \sa setDescription()
*/
QString QAppointment::description() const { return d->mDescription; }

/*!
  Returns the location of the appointment.

  \sa setLocation()
*/
QString QAppointment::location() const { return d->mLocation; }

/*!
  Returns the notes of the appointment.

  \sa setNotes()
*/
QString QAppointment::notes() const { return d->mNotes; }

/*!
  \fn bool QAppointment::hasAlarm() const
  Returns true if there is an alarm set for the appointment.  Otherwise
 returns false.

 \sa setAlarm()
*/

/*!
  Returns the number of minutes before the appointment to activate the alarm
  for the appointment.

 \sa setAlarm()
*/
int QAppointment::alarmDelay() const { return d->mAlarmDelay; } // in minutes.

/*!
  Returns the type of alarm to sound.

  \sa setAlarm(), AlarmFlags
*/
QAppointment::AlarmFlags QAppointment::alarm() const { return d->mAlarmSound; }

/*!
  Returns the RepeatRule of the appointment.

  \sa setRepeatRule(), RepeatRule
*/
QAppointment::RepeatRule QAppointment::repeatRule() const { return (RepeatRule)d->mType; }

/*!
  \fn bool QAppointment::hasRepeat() const
  Returns false if the appointment has repeat type NoRepeat.  Otherwise returns true.

  \sa setRepeatRule(), RepeatRule
*/

/*!
  Returns how often the appointment repeats.

  \sa setFrequency()
*/
int QAppointment::frequency() const { return d->mFrequency; }

/*!
  Returns true if the repeating appointment occurs on the nearest previous date
  in cases where the date it would normally occur on is invalid.


  As an example, an appointment that normally repeats on the 31st of each month
  and returns true for this function will occur on the 30th in June
  as the 31st of June is not a valid date.

  \sa setShowOnNearest()
*/
bool QAppointment::showOnNearest() const { return d->mShowOnNearest; }

/*!
  Returns the flags representing the days of the week the appointment occurs
  on when set as a Weekly recurring event.
*/
QAppointment::WeekFlags QAppointment::weekFlags() const { return d->weekMask; }

/*!
  Sets the days of the week the appointment occurs on to the specified \a days.
*/
void QAppointment::setWeekFlags(WeekFlags days) { d->weekMask = days; }

/*!
  Returns true if the appointment is an all day appointment.  Otherwise returns false.

  \sa setAllDay()
*/
bool QAppointment::isAllDay() const { return d->mAllDay; }

/*!
  Returns when the first occurrence of the appointment starts.

  \sa startInCurrentTZ(), setStart()
*/
QDateTime QAppointment::start() const
{
    if (d->mAllDay)
        return QDateTime(d->mStart.date(), QTime(0,0));
    return d->mStart;
}

/*!
  Returns when the first occurrence of the appointment ends.

  \sa endInCurrentTZ(), setEnd()
*/
QDateTime QAppointment::end() const
{
    if (d->mAllDay)
        return QDateTime(d->mEnd.date(), QTime(23,59));
    return d->mEnd;
}

/*!
  Constructs a new QAppointment.
  The start time is set to the current time, and the end time is
  five minutes after the current time.
*/
QAppointment::QAppointment() : QPimRecord()
{
    QDateTime start = QDateTime::currentDateTime();
    QDateTime end = start.addSecs(5 * 60);
    init(start, end);
}

/*!
  Constructs a new QAppointment as a copy of \a appointment.
*/
QAppointment::QAppointment(const QAppointment &appointment) : QPimRecord(appointment)
{
    d = appointment.d;
}

/*!
  Constructs a new QAppointment starting at \a start and running until \a end.
  If \a end is less than five minutes after \a start, the appointment will
  be created with an end time of five minutes after the \a start value.
*/
QAppointment::QAppointment(const QDateTime &start, const QDateTime &end)
{
    // smallest appointment 5 minutes
    if (end < start.addSecs(5 * 60))
        init(start, start.addSecs(5 * 60));
    else
        init(start, end);
}

/*!
  Sets the appointment to be a copy of \a other.
*/
QAppointment &QAppointment::operator=( const QAppointment &other )
{
    d = other.d;
    return *this;
}

void QAppointment::init(const QDateTime &s, const QDateTime &e)
{
    d = new QAppointmentData();
    d->mStart = trimSeconds(s);
    d->mEnd = trimSeconds(e);
    d->weekMask = 0;

    d->mAlarmDelay = 0;
    d->mAlarmSound = NoAlarm;

    d->mRepeatUntil = QDate();
    d->mType = NoRepeat;
    d->mFrequency = 1;
    d->mShowOnNearest = false;
    d->mAllDay = false;
}

/*!
  Destroys the appointment.
*/
QAppointment::~QAppointment()
{
}

/*!
  Sets the description of the appointment to \a text.

  \sa description()
*/
void QAppointment::setDescription( const QString &text )
{
    d->mDescription = text;
}

/*!
  Set the location of the appointment to \a text.

  \sa location()
*/
void QAppointment::setLocation( const QString &text )
{
    d->mLocation = text;
}

/*!
  Sets the start time of the appointment to \a time.
  This will change the end time of the appointment to maintain the same duration.

  \sa start(), startInCurrentTZ()
*/
void QAppointment::setStart( const QDateTime &time )
{
    // get existing, dependent state.
    int duration = d->mStart.secsTo(d->mEnd);

    // move the start, (end moves with it)
    d->mStart = trimSeconds(time);
    d->mEnd = trimSeconds(d->mStart.addSecs(duration));

    if (!d->mRepeatUntil.isNull() && d->mRepeatUntil < d->mEnd.date())
        d->mRepeatUntil = d->mEnd.date();
}

/*!
  Sets the end time of the appointment to \a time.
  This will also change the duration of the appointment. \a time must be at least
  5 minutes after the start time of the appointment.

  \sa end(), endInCurrentTZ()
*/
void QAppointment::setEnd( const QDateTime &time )
{
    if (time < d->mStart.addSecs(60 * 5))
        return; // can't set end before start.

    d->mEnd = trimSeconds(time);

    // make sure repeatForever and repeatUntil are still valid.
    if (!d->mRepeatUntil.isNull() && d->mRepeatUntil < d->mEnd.date())
        d->mRepeatUntil = d->mEnd.date();
}

/*!
  Sets the notes of the appointment to \a text.

  \sa notes()
*/
void QAppointment::setNotes( const QString &text )
{
    d->mNotes = text;
}

/*!
  Sets the time zone of the appointment to \a zone.
  This will affect when the appointment occurs in UTC.  All day appointments cannot have
  a time zone set.

  Setting the time zone to an invalid QTimeZone will cause the appointment to
  have no associated time zone.

  \sa timeZone(), isAllDay()
*/
void QAppointment::setTimeZone( const QTimeZone &zone )
{
    d->mTimeZone = zone;
}

/*!
  Sets the alarm \a action to precede the start of the appointment by the specified \a minutes.

  \sa clearAlarm(), hasAlarm(), alarm(), alarmDelay()
*/
void QAppointment::setAlarm( int minutes, AlarmFlags action )
{
    d->mAlarmDelay = minutes;
    d->mAlarmSound = action;
}

/*!
  Clears any alarms set for the appointment.

  \sa setAlarm()
*/
void QAppointment::clearAlarm()
{
    d->mAlarmSound = NoAlarm;
}

/*!
  Sets the repeat type of the appointment to \a t.

  \sa repeatRule(), hasRepeat(), RepeatRule
*/
void QAppointment::setRepeatRule( RepeatRule t )
{
    d->mType = t;
}

/*!
  Sets the repeat \a frequency of the appointment.  If the specified \a frequency is less than 1
  will set the repeat frequency of the appointment to 1.

  \sa frequency()
*/
void QAppointment::setFrequency( int frequency )
{
    d->mFrequency = frequency > 0 ? frequency : 1;
}

/*!
  Sets the appointment to repeat until the specified \a date.  If the \a date is before
  the end of the first appointment will set the appointment to occur once.

  \sa repeatUntil(), repeatForever()
*/
void QAppointment::setRepeatUntil( const QDate &date )
{
    d->mRepeatUntil = ( date.isNull() || date > d->mEnd.date() ) ? date : d->mEnd.date(); }

/*!
  Sets the appointment to an all day appointment if \a enable is true.
  All day appointments have a no set time-zone.

  \sa isAllDay(), setTimeZone()
*/
void QAppointment::setAllDay( bool enable )
{
    d->mAllDay = enable;
}

/*!
  Returns the time zone of the appointment or an invalid QTimeZone if the appointment has
  no time zone.  All day appointments always have no time zone.

  \sa setTimeZone(), isAllDay()
*/
QTimeZone QAppointment::timeZone() const
{
    return d->mAllDay ? QTimeZone() : d->mTimeZone;
}

/*!
  Returns the date the appointment will repeat until

  \sa repeatForever(), setRepeatUntil()
*/
QDate QAppointment::repeatUntil() const
{
    if (!hasRepeat())
        return d->mEnd.date();
    return d->mRepeatUntil;
}

/*!
  Returns true if the appointment will repeat forever.  Otherwise returns false.

  \sa repeatUntil(), setRepeatForever()
*/
bool QAppointment::repeatForever( ) const
{
    if (!hasRepeat())
        return false;
    return d->mRepeatUntil.isNull();
}

/*!
  Returns the instance of the week the appointment will occur for the repeat types
  MonthlyDay and MonthlyEndDay.  The week offset is determined by the starting
  date of the appointment.

  if (weekOffset() == 1) appointment occurs in first week of the month.
  if (weekOffset() == 3) appointment occurs in third week of the month.
  if (weekOffset() == -1) appointment occurs in last week of the month.

  Returns 0 if there is no week offset for the appointment.
*/
int QAppointment::weekOffset( ) const
{
    if (d->mType == MonthlyDay) {
        if (d->mStart.date().day() == 1)
            return 1;
        return (d->mStart.date().day() - 1) / 7 + 1;
    } else if (d->mType == MonthlyEndDay) {
        if (d->mStart.date().day() == d->mStart.date().daysInMonth())
            return -1;
        return -((d->mStart.date().daysInMonth() - d->mStart.date().day()) / 7 + 1);
    }
    return 0;
}

/*!
  Sets the appointment to repeat forever.

  \sa repeatForever()
*/
void QAppointment::setRepeatForever( )
{
    d->mRepeatUntil = QDate();
}

/*!
  If \a nearest is true sets the repeating appointment to occur on the nearest previous
  date if the day it would would normally occur on does not exist.

  As an example, for an appointment repeats on the 31st of each month passing true to this
  function will cause the appointment to occur on the 30th in June
  as the 31st of June is not a valid date.

  \sa showOnNearest()
*/
void QAppointment::setShowOnNearest( bool nearest)
{
    d->mShowOnNearest = nearest;
}

/*!
  Returns the WeekFlag associated with \a day as a Qt::DayOfWeek.
*/
QAppointment::WeekFlag QAppointment::fromDateDay(int day)
{
    switch(day) {
        default:
        case Qt::Sunday:
            return OccurSunday;
        case Qt::Monday:
            return OccurMonday;
        case Qt::Tuesday:
            return OccurTuesday;
        case Qt::Wednesday:
            return OccurWednesday;
        case Qt::Thursday:
            return OccurThursday;
        case Qt::Friday:
            return OccurFriday;
        case Qt::Saturday:
            return OccurSaturday;
    }
}

/*!
  Returns the Qt::DayOfWeek associated with \a day.
*/
int QAppointment::toDateDay(WeekFlag day)
{
    switch(day) {
        default:
        case OccurSunday:
            return Qt::Sunday;
        case OccurMonday:
            return Qt::Monday;
        case OccurTuesday:
            return Qt::Tuesday;
        case OccurWednesday:
            return Qt::Wednesday;
        case OccurThursday:
            return Qt::Thursday;
        case OccurFriday:
            return Qt::Friday;
        case OccurSaturday:
            return Qt::Saturday;
    }
}

/*!
  If the appointment is set to repeat on the given
  \a day of the week, then returns true.  Otherwise returns false.

  \sa setRepeatOnWeekDay()
*/
bool QAppointment::repeatOnWeekDay(int day) const
{
    return weekFlags() & fromDateDay(day);
}

/*!
  Sets the appointment to repeat on the specified \a day of the week if \a enable is true.
  Otherwise sets the appointment not to repeat on the specified \a day of the week.

  Event will always repeat on the day of the week that it started on.

  \sa repeatOnWeekDay()
*/
void QAppointment::setRepeatOnWeekDay(int day, bool enable)
{
    WeekFlag f = fromDateDay(day);
    if (((d->weekMask & f) == 0) != (enable == 0))
        d->weekMask ^= f;
}

/*!
  Returns the list of exceptions to the appointment's repeat rule.
*/
QList<QAppointment::Exception> QAppointment::exceptions() const
{
    return d->exceptions;
}

/*!
  Sets the exceptions for the appointment to the specified \a list.

Note: setting exceptions and updating appointment in the QAppointmentModel will
NOT store the exception.  Use the functions
QAppointmentModel::removeOccurrence(), QAppointmentModel::replaceOccurrence()
and QAppointmentModel::replaceRemaining() instead.

*/
void QAppointment::setExceptions(const QList<Exception> &list)
{
    d->exceptions = list;
}

/*!
  Removes all exceptions from the appointment.

Note: clearing exceptions and updating appointment in the QAppointmentModel will
NOT store the exception changes.  Use the functions
QAppointmentModel::removeOccurrence(), QAppointmentModel::replaceOccurrence()
and QAppointmentModel::replaceRemaining() instead.
 */
void QAppointment::clearExceptions()
{
    d->exceptions.clear();
}

// helper functions
int monthsTo(const QDate &from, const QDate &to)
{
    int result = 12 * (to.year() - from.year());
    result += (to.month() - from.month());

    return result;
}

// weeks is the number of times this dayOfWeek of the
// week appears in the month for this date.
int weeksForDayInMonth(int dayOfWeek, const QDate &date)
{
    QDate result;
    result.setYMD(date.year(), date.month(), 1);
    int dayOfMonth = result.dayOfWeek();

    int fromStart = dayOfWeek - dayOfMonth;
    if (fromStart < 0)
        fromStart += 7;

    return (result.daysInMonth() - fromStart - 1) / 7 + 1;
}

/*!
  Returns true if the appointment can occur.  Otherwise returns false.
*/
bool QAppointment::isValid() const
{
    return nextOccurrence(start().date()).isValid();
}

/*!
  Returns the first date on or after \a from that the appointment will next occur.

  If the appointment does not occur on or after \a from then a null occurrence is returned.
*/
QOccurrence QAppointment::nextOccurrence( const QDate &from) const
{
    int duration = d->mStart.daysTo(d->mEnd); // normally 0, can be 1.
    QDate date = p_nextOccurrence(from);
    while(date.isValid() && d->isException(date))
        date = p_nextOccurrence(date.addDays(duration+1));

    if (date.isValid())
        return QOccurrence(date, *this);
    return QOccurrence();
}

/*!
  Returns the first occurrence for the appointment.

  If there are no valid occurrence for the appointment will
  return a null occurrence.
*/
QOccurrence QAppointment::firstOccurrence() const
{
    return nextOccurrence(start().date().addDays(-1));
}

/*!
  \internal
  Returns true if more than one week day is set in weekFlags()
*/
bool QAppointment::p_weekFlagsActive() const
{
    switch(weekFlags())
    {
        case OccurMonday:
        case OccurTuesday:
        case OccurWednesday:
        case OccurThursday:
        case OccurFriday:
        case OccurSaturday:
        case OccurSunday:
        case 0:
            return false;
        default:
            return true;
    }
}

/*!
  \internal
  Code that handles the algorithms for repeat rules.  Exceptions excluded.
*/
QDate QAppointment::p_nextOccurrence(const QDate &from) const
{
    QDate result = d->mStart.date();
    // from should be for the start of the possible appointment.

    QDate after = from.addDays(d->mEnd.daysTo(d->mStart));

    if (result >= after)
        return result;

    if (!repeatForever() && repeatUntil() < after)
        return QDate();

    switch(d->mType) {
        default:
            return QDate();
        case NoRepeat:
            return QDate();
        case Daily:
            {
                int daysBetween = d->mStart.date().daysTo(after);
                int outBy = daysBetween % d->mFrequency;
                if (outBy) {
                    // this is in the wrong direction;
                    outBy = d->mFrequency - outBy;
                }
                result = after.addDays(outBy);
            }
            break;
        case Weekly:
            {
                // first do a quick check to see if it is possible that there
                // is an overlap.
                int diff = result.daysTo(after);
                int mod = diff % (d->mFrequency * 7);

                // if diff is < 7, it may be not the day of week of the start
                // day.  Check.  Don't look more than 6 days after the start
                // day.
                if (mod < 7) {
                    // go after % to 6.  if day of week match, to a normal but
                    // start with that day.
                    for (int i = mod; i < 7; i++) {
                        if (repeatOnWeekDay(result.addDays(i).dayOfWeek())) {
                            // move result forward to that week day
                            result = result.addDays(i);
                            break;
                        }
                    }
                }

                // Now treat as a regular, daily, Freq*7 appointment.

                // Remember, new result may now be after after.  Check
                int daysBetween = result.daysTo(after);
                if (daysBetween > 0) {
                    int outBy = daysBetween % (d->mFrequency * 7);
                    if (outBy) {
                        outBy = (d->mFrequency * 7) - outBy;
                    }
                    result = after.addDays(outBy);
                }
            }
            break;
        case MonthlyDate:
            {
                int monthsBetween = monthsTo(d->mStart.date(), after);

                // check to see if will be in after month.
                if (d->mStart.date().day() < after.day()) {
                    // wont be in after month, move to the next month.
                    monthsBetween++;
                    after = after.addMonths(1);
                }

                int outBy = monthsBetween % d->mFrequency;
                if (outBy) {
                    outBy = d->mFrequency - outBy;
                }
                result = after.addMonths(outBy);

                if (d->mShowOnNearest) {
                    if (d->mStart.date().day() < result.daysInMonth())
                        result.setYMD(result.year(), result.month(), d->mStart.date().day());
                    else
                        result.setYMD(result.year(), result.month(), result.daysInMonth());
                } else {
                    // can't show on nearest, when is the next valid date.
                    while (!QDate::isValid(
                                result.year(), result.month(), d->mStart.date().day())
                            ) {
                        result = result.addMonths(d->mFrequency);
                    }
                    result.setYMD(result.year(), result.month(), d->mStart.date().day());
                }
            }
            break;
        case MonthlyDay:
        case MonthlyEndDay:
#if 0
            // complied out for the moment until the gui can catch up
            if (p_weekFlagsActive()) {
                QDate candidate(after.year(), after.month(), 1);
                int monthsBetween = monthsTo(d->mStart.date(), candidate);
                int outBy = monthsBetween % d->mFrequency;
                if (outBy) {
                    outBy = d->mFrequency - outBy;
                }
                result = candidate.addMonths(outBy);
                // with weekflags we are referring to the last matching day
                // in month, not a week offset.
                // for instance last workday of the month.
                for (int attempt = 0; attempt < 2; ++attempt) {
                    int direction;
                    if (d->mType == MonthlyEndDay) {
                        direction = -1;
                        result.setDate(result.year(), result.month(), result.daysInMonth());
                    } else {
                        direction = 1;
                        result.setDate(result.year(), result.month(), 1);
                    }
                    while(!repeatOnWeekDay(result.dayOfWeek()))
                        result = result.addDays(direction);
                    if (result > after) {
                        break;
                    } else {
                        result = result.addMonths(d->mFrequency);
                    }
                }
                // chance is that result will still be before direction

            }
            else
#endif
            {
                // + for MonthlyDay, - for MonthlyEndDay.
                // otherwise these are basically the same.
                int mWeekOffset = weekOffset();

                int monthsBetween = monthsTo(d->mStart.date(), after);
                int outBy = monthsBetween % d->mFrequency;
                if (outBy) {
                    outBy = d->mFrequency - outBy;
                }
                result = after.addMonths(outBy);

                // this is tricky.  Need to move by d->mFreq months till we
                // get a good one.
                bool foundDate = false;
                while(!foundDate) {
                    int day;
                    int weeks = weeksForDayInMonth(d->mStart.date().dayOfWeek(), result);
                    // get to first day for that day of week.
                    int weekShift = d->mStart.date().dayOfWeek()
                        - QDate(result.year(), result.month(), 1).dayOfWeek();
                    if (weekShift < 0)
                        weekShift += 7;

                    if (mWeekOffset > 0) {
                        day = (mWeekOffset - 1) * 7 + 1 + weekShift;
                    } else {
                        day = (weeks + mWeekOffset) * 7 + 1 + weekShift;
                    }

                    if (d->mShowOnNearest) {
                        if (day > result.daysInMonth())
                            day -= 7;
                        if (day < 1)
                            day += 7;
                    } else {
                        if (day > result.daysInMonth() || day < 1) {
                            result = result.addMonths(d->mFrequency);
                            continue;
                        }
                    }

                    result.setYMD(result.year(), result.month(), day);
                    if (result < after)
                        result = result.addMonths(d->mFrequency);
                    else
                        foundDate = true; // success.
                }
            }
            break;
        case Yearly:
            {
                int yearsBetween = after.year() - d->mStart.date().year();
                int outBy = yearsBetween % d->mFrequency;
                if (outBy) {
                    outBy = d->mFrequency - outBy;
                }

                result = d->mStart.date().addYears(yearsBetween + outBy);

                if (result < after)
                    result = result.addYears(d->mFrequency);

                // at least after, may not be valid though.
                if (!d->mShowOnNearest) {
                    while (!QDate::isValid(result.year(), result.month(), d->mStart.date().day()))
                        result = result.addYears(d->mFrequency);
                    result.setYMD(result.year(), result.month(), d->mStart.date().day());
                }
            }
            break;
    }
    if (repeatForever() || result <= repeatUntil() )
        return result;
    return QDate();
}

// In pimrecord.cpp
void qpe_startVObjectInput();
bool qpe_vobjectCompatibility(const char* misfeature);
void qpe_endVObjectInput();
void qpe_startVObjectOutput();
void qpe_setVObjectProperty(const QString&, const QString&, const char* type, QPimRecord*);
void qpe_endVObjectOutput(VObject *,const char* type,const QPimRecord*);
VObject *qpe_safeAddPropValue( VObject *o, const char *prop, const QString &value );
static inline VObject *safeAddPropValue( VObject *o, const char *prop, const QString &value )
{ return qpe_safeAddPropValue(o,prop,value); }
VObject *qpe_safeAddProp( VObject *o, const char *prop);
static inline VObject *safeAddProp( VObject *o, const char *prop)
{ return qpe_safeAddProp(o,prop); }

const char *dayToString(int d)
{
    switch (d) {
        case 1:
            return "MO ";
        case 2:
            return "TU ";
        case 3:
            return "WE ";
        case 4:
            return "TH ";
        case 5:
            return "FR ";
        case 6:
            return "SA ";
        case 7:
        default:
            return "SU ";
    }
}

static const char *vCalDateTimeFormat = "yyyyMMddThhmmss"; // Z only if giving it in utc.
static const char *vCalDateFormat = "yyyyMMdd";

static VObject *createVObject( const QAppointment &e )
{
    qpe_startVObjectOutput();

    VObject *vcal = newVObject( VCCalProp );
    safeAddPropValue( vcal, VCVersionProp, "1.0" );
    VObject *appointment = safeAddProp( vcal, VCEventProp );

    bool timeAsUTC = false;
    QString start, end;
    QTimeZone tz = e.timeZone();
    if ( !e.isAllDay() ) {
        // don't give UTC times if we don't have a timezone
        timeAsUTC = tz.isValid();
        if (timeAsUTC) {
            start = tz.toUtc(e.start()).toString(vCalDateTimeFormat)+"Z";
            end = tz.toUtc(e.end()).toString(vCalDateTimeFormat)+"Z";
        } else {
            start = e.start().toString(vCalDateTimeFormat);
            end = e.end().toString(vCalDateTimeFormat);
        }
    } else {
        start = e.start().date().toString(vCalDateFormat);
        end = e.end().date().toString(vCalDateFormat);
    }
    safeAddPropValue( appointment, VCDTstartProp, start );
    safeAddPropValue( appointment, VCDTendProp, end );

    // vCal spec: VCSummaryProp is required
    // Palm m100:     Yes (but accepts VCDescriptionProp VCAttachProp)
    // SL5500:        No
    // Ericsson T39m: Yes
    if ( qpe_vobjectCompatibility("Palm-Event-DN") ) {
        safeAddPropValue( appointment, VCDescriptionProp, e.description() );
        safeAddPropValue( appointment, VCAttachProp, e.notes() );
    } else {
        safeAddPropValue( appointment, VCSummaryProp, e.description() );
        safeAddPropValue( appointment, VCDescriptionProp, e.notes() );
    }

    safeAddPropValue( appointment, VCLocationProp, e.location() );

    if ( e.hasAlarm() ) {
        QDateTime dt = e.start();
        dt = dt.addSecs( -e.alarmDelay()*60 );
        VObject *alarm = safeAddProp( appointment, VCDAlarmProp );
        QString dtString;
        if (timeAsUTC)
            dtString = tz.toUtc(dt).toString(vCalDateTimeFormat)+"Z";
        else
            dtString = dt.toString(vCalDateTimeFormat);
        safeAddPropValue( alarm, VCRunTimeProp, dtString );
        if (e.alarm() != QAppointment::Visible)  {
            VObject *aalarm = safeAddProp( appointment, VCAAlarmProp );
            safeAddPropValue( aalarm, VCRunTimeProp, dtString );
        }
    }

    if (timeAsUTC)
        safeAddPropValue( appointment, "X-Qtopia-TIMEZONE", e.timeZone().id() );

    if (e.hasRepeat()) {
        // minimal data.  if its optional and we want the default, stay quiet.
        QString repeat_format;
        switch (e.repeatRule())
        {
            default:
            case QAppointment::NoRepeat:
                break;
            case QAppointment::Daily:
                repeat_format = "D%1 %2";
                break;
            case QAppointment::Weekly:
                repeat_format = "W%1 ";
                {
                    for (int i = 1; i < 8; i++) {
                        if (e.repeatOnWeekDay(i)) {
                            repeat_format += dayToString(i);
                        }
                    }
                }
                repeat_format += "%2";
                break;
            case QAppointment::MonthlyDate:
                repeat_format = "MD%1 %2";
                break;
            case QAppointment::MonthlyDay:
                /* vCal can't handle the type of week masked monthly
                   rules Outlook and Qt Extended support.
                   Do best match instead.
                */
                repeat_format = "MP%3 %1 %2%4";
                repeat_format = repeat_format.arg(e.weekOffset());
                repeat_format = repeat_format.arg(dayToString(e.start().date().dayOfWeek()));
                // other stuff is default.
                break;
            case QAppointment::MonthlyEndDay:
                repeat_format = "MP%3 %1- %2%4";
                repeat_format = repeat_format.arg(-e.weekOffset());
                repeat_format = repeat_format.arg(dayToString(e.start().date().dayOfWeek()));
                break;
            case QAppointment::Yearly:
                repeat_format = "YM%1 %2";
                break;
        }

        repeat_format = repeat_format.arg(e.frequency())
            .arg( e.repeatForever() ? "#0" :
                    (const char *)e.repeatUntil().toString(vCalDateFormat).toLatin1());

        safeAddPropValue( appointment, VCRRuleProp, repeat_format );
    }

    qpe_endVObjectOutput(appointment,"Calendar",&e); // No tr

    return vcal;
}

static void parseRrule( QAppointment &e, const QString &v)
{
    QString value = v.simplified();
    enum state {
        type,
        interval,
        occurrencelist,
        weekdaylist,
        daynumberlist,
        monthlist,
        daylist, // not used, we don't do this type.
        duration,
    };
    state st = type; // state;
    int i = 0; // index;
    int acc = 0; // for building ints.

    for (i = 0; i < (int)value.length(); i++) {
        switch (st) {
            case type: // repeat class/type
                // work out the basic rule type.
                if (value[i] == QChar('D')) {
                    e.setRepeatRule(QAppointment::Daily);
                } else if (value[i] == QChar('W')) {
                    e.setRepeatRule(QAppointment::Weekly);
                } else if (value[i] == QChar('M')) {
                    i++;
                    if (i >= (int)value.length())
                        break;

                    // may need to change from MonthlyDay to MonthlyEndDay
                    // later.
                    if (value[i] == QChar('P'))
                        e.setRepeatRule(QAppointment::MonthlyDay);
                    else
                        e.setRepeatRule(QAppointment::MonthlyDate);
                } else if (value[i] == QChar('Y')) {
                    i++;
                    if (i >= (int)value.length() ||
                            value[i] != QChar('M')) {
                        // force exit from lup
                        i = (int)value.length();
                        break;;  // only know Yearly Month.
                    }
                    e.setRepeatRule(QAppointment::Yearly);
                }
                st = interval; // frequency;
                break;
            case interval: // repeat frequency
                if (value[i].isSpace()) {
                    // finished frequency;
                    e.setFrequency(acc);
                    if (e.repeatRule() == QAppointment::Daily)
                        st = duration; // duration;
                    else if (e.repeatRule() == QAppointment::Weekly)
                        st = weekdaylist;
                    else if (e.repeatRule() == QAppointment::MonthlyDay)
                        st = occurrencelist;
                    else if (e.repeatRule() == QAppointment::MonthlyDate)
                        st = daylist;
                    else
                        st = monthlist;
                    acc = 0;
                } else if (value[i].isDigit()) {
                    if (acc)
                        acc *=10;
                    acc += value[i].digitValue();
                } else {
                    // fail.  parse error.
                    qWarning("failed to parse RRULE: non-digit in frequency");
                    return;
                }
                break;
            case occurrencelist:
                // this could actually be duration.
                // read next two to check, i+1 should be either
                // a + or a -;
                if (i+1 < (int)value.length() &&
                        value[i].isDigit()) {
                    // what the digit is won't help, we always work
                    // of the start date.
                    if (value[i+1] == QChar('+')) {
                        e.setRepeatRule(QAppointment::MonthlyDay);
                        i += 2; // get past inevitable ' '
                        st = weekdaylist;
                        break;
                    } else if (value[i+1] == QChar(' ')) {
                        e.setRepeatRule(QAppointment::MonthlyDay);
                        i += 1; // get past ' '
                        st = weekdaylist;
                        break;
                    } else if (value[i+1] == QChar('-')) {
                        e.setRepeatRule(QAppointment::MonthlyEndDay);
                        st = weekdaylist;
                        i += 2; // get past inevitable ' '
                        break;;
                    }
                }
                // not an occurrence list, but still monthly day.
                i--;
                st = weekdaylist;;
                break;
            case weekdaylist:
                // end on digit or #.  otherwise try it as a day.
                if (value[i] == QChar('#') || value[i].isDigit()) {
                    st = duration;
                    i--;
                    break;
                }
                // read the next 2/3 (if third is space)
                if (i+1 >= (int)value.length()) {
                    i = (int)value.length();
                    break;  // only know Yearly Month.
                }
                if (value[i] == QChar('M'))
                    e.setRepeatOnWeekDay(1, true);
                else if (value[i] == QChar('T') &&
                        value[i+1] == QChar('U'))
                    e.setRepeatOnWeekDay(2, true);
                else if (value[i] == QChar('W'))
                    e.setRepeatOnWeekDay(3, true);
                else if (value[i] == QChar('T') &&
                        value[i+1] == QChar('H'))
                    e.setRepeatOnWeekDay(4, true);
                else if (value[i] == QChar('F'))
                    e.setRepeatOnWeekDay(5, true);
                else if (value[i] == QChar('S') &&
                        value[i+1] == QChar('A'))
                    e.setRepeatOnWeekDay(6, true);
                else if (value[i] == QChar('S') &&
                        value[i+1] == QChar('U'))
                    e.setRepeatOnWeekDay(7, true);

                // no to the inc.
                i += 2;  // safe, as either while will cut out, or i+1 would be a ' '
                break;
                // can't use either of these of these.
            case daylist:
                // FALL THROUGH
            case daynumberlist:
                // FALL THROUGH
            case monthlist:
                // find the optional duration.
                // find a # or more than 3 digits from end.
                // note we will be at the start of the string here.
                {
                    int space = value.lastIndexOf(QChar(' '));
                    if (space < i-1) {
                        i = (int)value.length();
                        break;
                    }
                    if (space + 4 < (int)value.length()
                            || value[i] == QChar('#')) {
                        i = space;
                        st = duration;
                    } else {
                        i = (int)value.length();
                        break;
                    }
                }
                break;
            case duration: // repeat duration
                // duration.
                // expect either a # or a datetime.
                // if # just finish of the number now.
                if (value[i] == QChar('#')) {
                    i++;
                    acc = 0;
                    while (i < (int)value.length()) {
                        if (value[i].isDigit()) {
                            acc *= 10;
                            acc += value[i].digitValue();
                        } else {
                            qWarning("failed to parse RRULE: non-digit in duration count");
                        }
                        i++;
                    }
                    // if 0, repeat forever.  if anything else will need
                    if (acc == 0)
                        e.setRepeatForever();
                    i = (int)value.length();
                    // XXX Could add code to work out the count -> date.
                } else {
                    // from hear till the end is an ISO value, and we want it.

                    e.setRepeatUntil(QDate::fromString(value.mid(i), vCalDateFormat));
                    i = (int)value.length();
                }
        }
    }

    // out without errors.
    if (!(e.frequency() % 12) && e.repeatRule() == QAppointment::MonthlyDate)
    {
        e.setRepeatRule(QAppointment::Yearly);
        e.setFrequency(e.frequency() / 12);
    }
}

static QAppointment parseVObject( VObject *obj )
{
    QAppointment e;

    bool haveAlarm = false;
    bool haveStart = false;
    bool haveEnd = false;
    QDateTime alarmTime;
    QDateTime startTime;
    QAppointment::AlarmFlags soundType = QAppointment::Visible;

    VObjectIterator it;
    initPropIterator( &it, obj );
    QString summary, description, attach; // vCal properties, not Qtopias
    int subSection = 0;
    while( moreIteration( &it ) ) {
        VObject *o = nextVObject( &it );
        QString name = vObjectName( o );

        // check this key/value for a CHARSET field.
        VObjectIterator tnit;
        initPropIterator( &tnit, o );
        QTextCodec *tc = 0;
        while( moreIteration( &tnit ) ) {
            VObject *otc = nextVObject( &tnit );
            if ( qstrcmp(vObjectName(otc), VCCharSetProp ) == 0) {
                tc = QTextCodec::codecForName(vObjectStringZValue(otc));
                break;
            }
        }
        QString value;
        if (tc)
            value = tc->toUnicode( vObjectStringZValue( o ) );
        else
            value = vObjectStringZValue( o );

        // iCals can have other parts (e.g. TIMEZONE descriptor, shield form them
        if ( name == "BEGIN" ) {
            subSection++;
        } else if (name == "END") {
            subSection--;
        }
        if (subSection != 0)
            continue;

        // XXX We may need to modify this by timezone later if there
        // is a timezone.  if you have a receive appointment bug, check
        // this first.
        if ( name == VCDTstartProp ) {
            // check string-length.  if no time, its an allday.
            //
            if (value.startsWith("TZID")) {
                // iCal style timezone included dtstart.
                int sepIndex = value.indexOf(':');
                e.setTimeZone( QTimeZone(value.left(sepIndex).toLocal8Bit()) );
                value = value.mid(sepIndex+1);
            }
            if (value.endsWith("Z")) {
                e.setTimeZone( QTimeZone::utc() );
                // time is given in utc, not local TODO, timezone implications.
                value = value.left(value.length()-1);
            }
            if (value.length() == 8) {
                e.setAllDay(true);
                startTime = QDateTime(QDate::fromString(value, vCalDateFormat));
            } else {
                startTime = QDateTime::fromString(value, vCalDateTimeFormat);
            }
            e.setStart( startTime );
            haveStart = true;
        }
        else if ( name == VCDTendProp ) {
            if (value.startsWith("TZID")) {
                // iCal style timezone included dtstart.
                int sepIndex = value.indexOf(':');
                value = value.mid(sepIndex+1);
                // can't store end timezone in qappointment
            }
            if (value.endsWith("Z")) {
                e.setTimeZone( QTimeZone::utc() );
                // time is given in utc, not local TODO, timezone implications.
                value = value.left(value.length()-1);
            }
            // check string-length.  if no time, its an allday.
            if (value.length() == 8) {
                e.setEnd( QDateTime(QDate::fromString(value, vCalDateFormat)));
            } else {
                QDateTime endTime = QDateTime::fromString(value, vCalDateTimeFormat);
                e.setEnd (endTime);
            }
            haveEnd = true;
        }
        else if (name == "DURATION") {
            // iCal format allows for duration instead of end time
            /*
               dur-value  = (["+"] / "-") "P" (dur-date / dur-time / dur-week)

               dur-date   = dur-day [dur-time]
               dur-time   = "T" (dur-hour / dur-minute / dur-second)
               dur-week   = 1*DIGIT "W"
               dur-hour   = 1*DIGIT "H" [dur-minute]
               dur-minute = 1*DIGIT "M" [dur-second]
               dur-second = 1*DIGIT "S"
               dur-day    = 1*DIGIT "D"
             */
            bool reverseDuration = false;
            if (value.startsWith('+')) {
                value = value.mid(2); // skip '+P'
            } else if (value.startsWith('-')) {
                reverseDuration = true;
                value = value.mid(2); // skip '-P'
            } else {
                value = value.mid(1); // skip 'P'
            }
            int iVal = 0;
            int days = 0;
            int seconds = 0;
            for (int i =0; i < value.length(); ++i) {
                switch(value[i].toAscii()) {
                    default:
                        if (value[i].isDigit()) {
                            iVal *= 10;
                            iVal += value[i].digitValue();
                        } else {
                            qWarning("parse error in DURATION at %d in %s",
                                    i, value.toLocal8Bit().constData());
                        }
                        break;
                    case 'D':
                        // digits precede.
                        days = iVal;
                        iVal = 0;
                        break;
                    case 'W':
                        days = iVal*7;
                        iVal = 0;
                        break;
                    case 'T':
                        break; // just ignore it.
                    case 'H':
                        seconds += iVal*60*60;
                        iVal = 0;
                        break;
                    case 'M':
                        seconds = iVal*60;
                        iVal = 0;
                        break;
                    case 'S':
                        seconds = iVal;
                        iVal = 0;
                        break;
                }
            }

            // assume start time already set.
            QDateTime end = e.start();
            end = end.addDays(days);
            end = end.addSecs(seconds);
            e.setEnd(end);
        }
        // X-Qtopia-NOTES is for 1.5.0 compatibility
        else if ( name == "X-Qtopia-NOTES" || name == VCAttachProp) {
            attach = value;
        }
        else if ( name == VCSummaryProp ) {
            summary = value;
        }
        else if ( name == VCDescriptionProp ) {
            description = value;
        }
        else if ( name == VCLocationProp ) {
            e.setLocation( value );
        }
        else if ( name == VCAAlarmProp || name == VCDAlarmProp ) {
            haveAlarm = true;
            VObjectIterator nit;
            initPropIterator( &nit, o );
            while( moreIteration( &nit ) ) {
                VObject *o = nextVObject( &nit );
                QString subname = vObjectName( o );
                QString subvalue = vObjectStringZValue( o );
                if ( subname == VCRunTimeProp ) {
                    // utc time
                    if (subvalue.endsWith("Z")) {
                        subvalue = subvalue.mid(0, subvalue.length()-1);
                    }
                    alarmTime = QDateTime::fromString( subvalue, vCalDateTimeFormat );
                }
            }

            if ( name == VCAAlarmProp )
                soundType = QAppointment::Audible;
        }
        // We don't use VCTimeZoneProp as that has the form +05:30.
        // We are a bit better at timezones than this so we need the actual name
        // We _may_ want to use VCGeoLocationProp ( lang long )
        else if ( name == "X-Qtopia-TIMEZONE") {
            QTimeZone tz = QTimeZone(value.toLocal8Bit());
            if (tz.isValid()) {
                // will also shift e.end.
                QDateTime start = tz.fromUtc(e.start());
                QDateTime end = tz.fromUtc(e.end());
                e.setStart(start);
                e.setEnd(end);
                e.setTimeZone(tz);
            }
        }
        else if ( name == VCRRuleProp) {
            // iCal RRULE differs from vCal RRULE
            // vobject doesn't sub-object it for us for whatever reason.
            if (value.startsWith("FREQ")) {
                QStringList parts = value.split(";");
                // is iCal type
                QAppointment::RepeatRule rType = QAppointment::NoRepeat;
                QAppointment::RepeatRule rModifier = QAppointment::MonthlyDate;
                QDate rUntil;
                int rFreq = 1;
                int rCount = 0;
                QAppointment::WeekFlags rWeekMask = 0;
                foreach(QString part, parts) {
                    int epos = part.indexOf('=');
                    QString subname = part.left(epos);
                    QString subvalue = part.mid(epos+1);
                // type
                    if (subname == "FREQ") {
                        if (subvalue == "DAILY") {
                            rType = QAppointment::Daily;
                        } else if (subvalue == "WEEKLY") {
                            rType = QAppointment::Weekly;
                        } else if (subvalue == "MONTHLY") {
                            rType = QAppointment::MonthlyDate;
                        } else if (subvalue == "YEARLY") {
                            rType = QAppointment::Yearly;
                        }
                // type modifiers
                    } else if (subname == "BYDAY") { // don't do by second,minute/hour
                        // comma separated list of MO,TU,WE,TH,FR,SA,SU
                        // each may be preceded by +/- integer.  we can't
                        // handle that (repeat structure won't represent it)
                        // so we just use it to select between the three
                        // type of month repeats we do handle.
                        // first check if preceded by (+n/-n);
                        if (subvalue[0] == '-') {
                            rModifier = QAppointment::MonthlyEndDay;
                        } else {
                            rModifier = QAppointment::MonthlyDay;
                        }
                        QStringList days = subvalue.split(',');
                        foreach(QString day, days) {
                            if (day == "MO") {
                                rWeekMask |= QAppointment::OccurMonday;
                            } else if (day == "TU") {
                                rWeekMask |= QAppointment::OccurTuesday;
                            } else if (day == "WE") {
                                rWeekMask |= QAppointment::OccurWednesday;
                            } else if (day == "TH") {
                                rWeekMask |= QAppointment::OccurThursday;
                            } else if (day == "FR") {
                                rWeekMask |= QAppointment::OccurFriday;
                            } else if (day == "SA") {
                                rWeekMask |= QAppointment::OccurSaturday;
                            } else if (day == "SU") {
                                rWeekMask |= QAppointment::OccurSunday;
                            }
                        }
                    } else if (subname == "UNTIL") {
                        rUntil = QDateTime(QDate::fromString(value, vCalDateFormat)).date();
                    } else if (subname == "COUNT") {
                        rCount = subvalue.toInt();
                    } else if (subname == "INTERVAL") {
                        rFreq = subvalue.toInt();
                    }
                }
                // store repeat information.
                switch (rType) {
                    case QAppointment::MonthlyDate:
                        e.setRepeatRule(rModifier);
                    default:
                        e.setRepeatRule(rType);
                }
                e.setFrequency(rFreq);

                if (rType == QAppointment::Weekly)
                    e.setWeekFlags(rWeekMask);

                if (rUntil.isValid()) {
                    e.setRepeatUntil(rUntil);
                } else if (rCount > 0 && rCount < 100) {
                    // limited to 100 for to avoid possible excessive processing.
                    QOccurrence o = e.firstOccurrence();
                    for (int i = 0; i < rCount; ++i)
                        o = o.nextOccurrence();
                    e.setRepeatUntil(o.date());
                }
            } else {
                parseRrule(e, value);
            }

        } else {
            qpe_setVObjectProperty(name,value,"Calendar",&e); // No tr
        }
    }

    // Find best mapping from (Summary,Description,Attach) to our (Description,Notes)
    // Similar code in task.cpp
    if ( !summary.isEmpty() && !description.isEmpty() && summary != description ) {
        e.setDescription( summary );
        e.setNotes( description );
        // all 3 - drop attach
    } else if ( !summary.isEmpty() ) {
        e.setDescription( summary );
        e.setNotes( attach );
    } else {
        e.setDescription( description );
        e.setNotes( attach );
    }

    if ( !haveStart && !haveEnd )
        e.setStart( QDateTime::currentDateTime() );

    if ( !haveEnd ) {
        QDateTime end = e.start();
        end.setTime(QTime(32, 59, 59));
        e.setEnd( end );
    }

    if ( haveAlarm ) {
        int minutes = alarmTime.secsTo( startTime ) / 60;
        e.setAlarm( minutes, soundType );
    }
    return e;
}

/*!
  Writes the given list of \a appointments to the given \a device as vCalendars.

  Returns true on success.
  \sa readVCalendar()
*/
bool QAppointment::writeVCalendar( QIODevice *device, const QList<QAppointment> &appointments )
{
    foreach (QAppointment t, appointments)
        if (!writeVCalendar(device, t))
            return false;
    return true;
}

/*!
  Writes the given \a appointment to the given \a device as vCalendars.

  Returns true on success.
  \sa readVCalendar()
*/
bool QAppointment::writeVCalendar( QIODevice *device, const QAppointment &appointment )
{
    VObject *obj = createVObject(appointment);
    writeVObject( device, obj );
    cleanVObject( obj );
    cleanStrTbl();
    return true;
}

/*!
  Reads a list of vCalendars from the given \a device and returns the
  equivalent set of appointments.

  \sa writeVCalendar()
*/
QList<QAppointment> QAppointment::readVCalendar( QIODevice *device )
{
    QList<QAppointment> appointments;

    QBuffer *buffer = qobject_cast<QBuffer *>(device);
    QFile *file = qobject_cast<QFile *>(device);
    if (file) {
        int handle = file->handle();
        FILE *input = fdopen(handle, "r");
        if (input) {
            appointments = readVCalendarData( Parse_MIME_FromFile( input ) );
        }
    } else if (buffer) {
        appointments = readVCalendarData( Parse_MIME( (const char*)buffer->data(), buffer->data().count() ) );
    } else {
        const QByteArray bytes = device->readAll();
        appointments = readVCalendarData( Parse_MIME( (const char*)bytes, bytes.count() ) );
    }

    return appointments;
}

/*!
  \deprecated

   Write the list of \a appointments as vCalendar objects to the file
   specified by \a filename.

   \sa readVCalendar()
*/
void QAppointment::writeVCalendar( const QString &filename, const QList<QAppointment> &appointments)
{
    FILE *f = fopen(filename.toLocal8Bit(),"w");
    if ( !f ) {
        qWarning("Unable to open vcard write");
        return;
    }

    QList<QAppointment>::ConstIterator it;
    for( it = appointments.begin(); it != appointments.end(); ++it ) {
        VObject *obj = createVObject( *it );
        writeVObject( f, obj );
        cleanVObject( obj );
    }

    cleanStrTbl();
    fclose(f);
}

/*!
  \deprecated

   Writes the appointment as a vCalendar object to the file specified
   by \a filename.

   Returns true on success, false on fail.

   \sa readVCalendar()
*/
void QAppointment::writeVCalendar( const QString &filename ) const
{
    writeVCalendar(filename, *this);
}

/*!
   \deprecated

   Writes the appointment as a vCalendar object to the given \a file,
   which must be already open for writing.

   \sa readVCalendar()
*/
void QAppointment::writeVCalendar( QFile &file ) const
{
    QDataStream stream( &file );
    writeVCalendar( &stream );

}

/*!
   \deprecated

   Writes the appointment as a vCalendar object to the given \a stream,
   which must be writable.

   \sa readVCalendar()
*/
void QAppointment::writeVCalendar( QDataStream *stream ) const
{
    VObject *obj = createVObject(*this);
    writeVObject( stream->device(), obj );
    cleanVObject( obj );
    cleanStrTbl();
}

/*!
  \deprecated

   Write the \a appointment as a vCalendar to the file specified by \a filename.

   \sa readVCalendar()
*/
void QAppointment::writeVCalendar( const QString &filename, const QAppointment &appointment)
{
    FILE *f = fopen(filename.toLocal8Bit(),"w");
    if ( !f ) {
        qWarning("Unable to open vcard write");
        return;
    }

    VObject *obj = createVObject( appointment );
        writeVObject( f, obj );
        cleanVObject( obj );

	cleanStrTbl();
    fclose(f);
}

/*!
  \deprecated

  Reads the file specified by \a filename as a list of vCalendar objects
  and returns the list of near equivalent appointments.

  \sa writeVCalendar()
*/
QList<QAppointment> QAppointment::readVCalendar( const QString &filename )
{
    VObject *obj = Parse_MIME_FromFileName( (const char *)filename.toUtf8() );
    return readVCalendarData(obj);
}

/*!
  \deprecated

  Reads the \a data of \a len bytes as a list of vCalendar objects
  and returns the list of near equivalent appointments.

  \sa writeVCalendar()
*/
QList<QAppointment> QAppointment::readVCalendarData( const char *data, unsigned long len )
{
    VObject *obj = Parse_MIME( data, len );
    return readVCalendarData(obj);
}

/*!
  \deprecated

  Reads the given VCalendar data in \a vcal and returns the list of
  near equivalent appointments.

  \sa writeVCalendar()
*/
QList<QAppointment> QAppointment::readVCalendar( const QByteArray &vcal )
{
    return readVCalendarData( Parse_MIME( (const char*)vcal, vcal.count() ) );
}

/*!
  Reads the file specified by \a filename as a list of vCalendar objects
  and returns the list of near equivalent appointments.

  \sa writeVCalendar()
*/
QList<QAppointment> QAppointment::readVCalendarData( VObject *obj )
{
    QList<QAppointment> appointments;

    qpe_startVObjectInput();
    while ( obj ) {
        QString name = vObjectName( obj );
        if ( name == VCCalProp ) {
            VObjectIterator nit;
            initPropIterator( &nit, obj );
            while( moreIteration( &nit ) ) {
                VObject *o = nextVObject( &nit );
                QString name = vObjectName( o );
                if ( name == VCEventProp )
                    appointments.append( parseVObject( o ) );
            }
        } else if ( name == VCEventProp ) {
            // shouldn't happen, but just to be sure
            appointments.append( parseVObject( obj ) );
        }
        VObject *t = obj;
        obj = nextVObjectInList(obj);
        cleanVObject( t );
    }
    qpe_endVObjectInput();

    return appointments;
}

/*!
  Returns the start time of the appointment in the current system timezone.

  \sa start()
*/
QDateTime QAppointment::startInCurrentTZ() const
{
    if (!timeZone().isValid())
        return start();

    // if no zone given.. assume local
    return timeZone().toCurrent(start());
}

/*!
  Returns the end time of the appointment in the current system timezone.

  \sa end()
*/
QDateTime QAppointment::endInCurrentTZ() const
{
    if (!timeZone().isValid())
        return end();

    QDateTime nStart = timeZone().toCurrent(start());

    return end().addSecs(start().secsTo(nStart));
}

/*!
  Returns the date the appointment will repeat until in the current system timezone.

  \sa repeatUntil()
*/
QDate QAppointment::repeatUntilInCurrentTZ() const
{
    if (!timeZone().isValid())
        return repeatUntil();

    // duration should be the same... shift the start.. diff to end.
    // if no zone given.. assume local
    QDateTime nStart = timeZone().toCurrent(start());

    QDateTime rtDateTime = QDateTime(repeatUntil(), QTime(0,0,0));
    return rtDateTime.addSecs(start().secsTo(nStart)).date();
}

/*!
  \class QOccurrence
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QOccurrence class holds the data of an occurrence of a calendar appointment.

  This data includes descriptive data of the appointment and scheduling information.

  \sa QAppointment, {Pim Library}
*/


/*!
  \fn QAppointment::AlarmFlags QOccurrence::alarm() const

  Returns the type of alarm to sound.
*/

/*!
    \fn bool QAppointment::hasExceptions() const

    Returns if there are any exceptions to the appointments recurrence rule.
*/


/*!
  \fn int QOccurrence::alarmDelay() const

  Returns the number of minutes before the appointment to activate the alarm.
*/

/*!
  \fn QAppointment QOccurrence::appointment() const

  Returns the appointment associated with the occurrence.
*/

/*!
  \fn QList<QString> QOccurrence::categories() const

  Returns the set of categories the appointment belongs to.
*/

/*!
  \fn bool QOccurrence::conflicts(const QOccurrence &other) const

  Returns true if the occurrence overlaps with the \a other occurrence.
  Otherwise returns false.
*/

/*!
  \fn bool QOccurrence::conflicts(const QDateTime &from, const QDateTime &to) const

  Returns true if the occurrence overlaps with the time period starting at \a from
  and ending at \a to.
  Otherwise returns false.
*/

/*!
  \fn QString QOccurrence::description() const
  Returns the description of the appointment associated with the occurrence.
*/

/*!
  \fn QOccurrence::isValid() const

  Returns true if the occurrence contains valid data. Otherwise returns false.
*/

/*!
  \fn QString QOccurrence::location() const

  Returns the location of the appointment associated with the occurrence.
*/

/*!
  \fn QString QOccurrence::notes() const

  Returns the notes of the appointment associated with the occurrence.
*/

/*!
  \fn QTimeZone QOccurrence::timeZone() const

  Returns the notes of the time zone associated with the occurrence.
*/

/*!
  \fn QUniqueId QOccurrence::uid() const

  Returns the unique ID for the appointment associated with the occurrence.
*/

/*!
  Constructs an empty occurrence.
*/
QOccurrence::QOccurrence() {}

/*!
  Constructs a copy of \a occurrence;
*/
QOccurrence::QOccurrence(const QOccurrence &occurrence)
    : appointmentCache(occurrence.appointmentCache), mStart(occurrence.mStart)
{
}

/*!
  Constructs an occurrence of the \a appointment occurring on
  \a date.  Does not check whether this is a valid occurrence for the \a appointment.
*/
QOccurrence::QOccurrence(const QDate &date, const QAppointment &appointment ) : appointmentCache(appointment), mStart(date)
{
}

/*!
  Destroys the occurrence.
*/
QOccurrence::~QOccurrence() {}

/*!
  Sets the occurrence to be a copy of \a other.
*/
QOccurrence& QOccurrence::operator=(const QOccurrence &other)
{
    appointmentCache = other.appointmentCache;
    mStart = other.mStart;
    return *this;
}

/*!
  Returns true if this occurrence is equal to \a other.  Otherwise
  returns false.
*/
bool QOccurrence::operator==(const QOccurrence &other) const
{
    if (appointmentCache != other.appointmentCache)
        return false;
    if (mStart != other.mStart)
        return false;
    return true;
}

/*!
  \fn QDate QOccurrence::date() const

  Returns the date that the occurrence starts.

  \sa endDate(), start(), end()
*/

/*!
  Returns the date that the occurrence ends.
  \sa date(), start(), end()
*/
QDate QOccurrence::endDate() const
{
    return date().addDays(appointmentCache.start().date().daysTo(appointmentCache.end().date()));
}

/*!
  Returns date and time that the occurrence starts.
  \sa end(), date(), endDate()
*/
QDateTime QOccurrence::start() const
{
    return QDateTime(date(), appointmentCache.start().time());
}

/*!
  Returns date and time that the occurrence ends.
  \sa start(), date(), endDate()
*/
QDateTime QOccurrence::end() const
{
    return QDateTime(endDate(), appointmentCache.end().time());
}

// Timezone dependent functions...
/*!
  Returns the start date and time of the occurrence in the current time zone.
*/
QDateTime QOccurrence::startInCurrentTZ() const
{
    if (!appointmentCache.timeZone().isValid())
        return start();

    return appointmentCache.timeZone().toCurrent(start());
}

/*!
  Returns the end date and time of the occurrence in the current time zone.
*/
QDateTime QOccurrence::endInCurrentTZ() const
{
    if (!appointmentCache.timeZone().isValid())
        return end();


    QDateTime nStart = appointmentCache.timeZone().toCurrent(start());

    return end().addSecs(start().secsTo(nStart));
}

/*!
  Returns the date and time for the alarm of the occurrence in the current time zone.
*/
QDateTime QOccurrence::alarmInCurrentTZ() const
{
    if (mStart.isNull())
        return QDateTime();

    return startInCurrentTZ().addSecs(-60*appointmentCache.alarmDelay());
}

/*!
   \internal
   \fn void QAppointment::deserialize(Stream &s)
   Reads the appointment from the stream \a s.
*/
template <typename Stream> void QAppointment::deserialize(Stream &s)
{
    s >> d->mUid;
    s >> d->mCategories;
    s >> d->customMap;
    s >> d->mDescription;
    s >> d->mLocation;
    s >> d->mStart;
    d->mStart = trimSeconds(d->mStart);
    s >> d->mEnd;
    d->mEnd = trimSeconds(d->mEnd);
    s >> d->mNotes;
    s >> d->mTimeZone;
    uchar val;
    s >> d->mAlarmDelay;
    s >> val;
    d->mAlarmSound = (QAppointment::AlarmFlags)val;
    s >> val;
    d->mType = (QAppointment::RepeatRule)val;
    s >> d->mFrequency;
    s >> d->mRepeatUntil;
    s >> val;
    d->mShowOnNearest = val == 0 ? false : true;
    s >> val;
    d->weekMask = (QAppointment::WeekFlags)val;
    s >> val;
    d->mAllDay = val == 0 ? false : true;
}

/*!
   \internal
   \fn void QAppointment::serialize(Stream &s) const
   Writes the appointment to the stream \a s.
*/
template <typename Stream> void QAppointment::serialize(Stream &s) const
{
    s << d->mUid;
    s << d->mCategories;
    s << d->customMap;
    s << d->mDescription;
    s << d->mLocation;
    s << d->mStart;
    s << d->mEnd;
    s << d->mNotes;
    s << d->mTimeZone;
    s << d->mAlarmDelay;
    s << (uchar)d->mAlarmSound;
    s << (uchar)d->mType;
    s << d->mFrequency;
    s << d->mRepeatUntil;
    s << (d->mShowOnNearest ? (uchar)1 : (uchar)0);
    s << (uchar)d->weekMask;
    s << (d->mAllDay ? (uchar)1 : (uchar)0);
}

/*!
  Returns true if this appointment is equal to \a other.  Otherwise
  returns false.
*/
bool QAppointment::operator==( const QAppointment &other ) const
{
    if (!QPimRecord::operator==(other))
        return false;

    const QAppointment &oe = (const QAppointment&)(*this);
    const QAppointment &ne = (const QAppointment&)other;

    return (oe.timeZone() == ne.timeZone() &&
            oe.start() == ne.start() &&
            oe.end() == ne.end() &&
            oe.description() == ne.description() &&
            oe.location() == ne.location() &&
            oe.alarmDelay() == ne.alarmDelay() &&
            oe.hasAlarm() == ne.hasAlarm() &&
            oe.alarm() == ne.alarm() &&
            oe.isAllDay() == ne.isAllDay() &&
            oe.notes() == ne.notes() &&
            oe.hasRepeat() == ne.hasRepeat() &&
            oe.frequency() == ne.frequency() &&
            oe.repeatRule() == ne.repeatRule() &&
            oe.repeatForever() == ne.repeatForever() &&
            oe.repeatUntil() == ne.repeatUntil());
}

/*!
  Returns true if this appointment is not equal to \a other.  Otherwise
  returns false.
*/
bool QAppointment::operator!=( const QAppointment &other ) const
{
    return !((*this) == other );
}

/*!
  If the appointment has an occurrence following this occurrence, returns the following
  occurrence.  Otherwise returns a null occurrence.
*/
QOccurrence QOccurrence::nextOccurrence() const
{
    if (!isValid())
        return QOccurrence();
    // ignore TZ, since can base of non tz typing.
    int duration = appointmentCache.start().date().daysTo(appointmentCache.end().date()) + 1;
    QDate from = date().addDays(duration);
    return appointmentCache.nextOccurrence(from);
}

/*!
  Sets this appointment to be an exception to an appointment with the specified \a identifier.
 */
void QAppointment::setExceptionParent( const QUniqueId &identifier )
{
    d->mExceptionParent = identifier;
}

/*!
  Sets the appointment to be an exception to the recurring appointment with the specified \a identifier.  The appointment is specified to replace the occurrence that occurs on the given \a date.
*/
void QAppointment::setAsException(const QUniqueId &identifier, const QDate &date)
{
    d->mExceptionParent = identifier;
    d->mExceptionDate = date;
}

/*!
  Returns the identifier of the repeating appointment to which this appointment is an exception.
 */
QUniqueId QAppointment::exceptionParent() const
{
    return d->mExceptionParent;
}

/*!
  Returns the date of the repeating appointment to which this appointment is
  an exception.
*/
QDate QAppointment::exceptionDate() const
{
    return d->mExceptionDate;
}

/*!
  Returns true if the appointment is an exception of another repeating appointment.
 */
bool QAppointment::isException() const
{
    return !d->mExceptionParent.isNull();
}

Q_IMPLEMENT_USER_METATYPE(QAppointment)

/*!
  \class QAppointment::Exception
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
  \inpublicgroup QtPimModule

  \ingroup pim

  \brief The Exception class provides information about exceptions to appointment related repeat rules. 

  This structure has the following fields:

  QDate date;

  QUniqueId alternative;

  \sa {Pim Library}
*/

