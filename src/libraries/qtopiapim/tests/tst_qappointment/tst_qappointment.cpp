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

#include <QAppointment>
#include <QDateTime>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QtopiaApplication>



//TESTED_CLASS=QAppointment,QPimRecord
//TESTED_FILES=src/libraries/qtopiapim/qappointment.h,src/libraries/qtopiapim/qpimrecord.h

class tst_QAppointment : public QObject
{
Q_OBJECT
private:
    void setTimeZone(const QString &city);
    QDateTime trimSeconds(const QDateTime &date);

private slots:
    void ctor();

    void startEndCtor();
    void startEndCtor_data();

    void setAlarm();
    void setAlarm_data();

    void timeZone();

    void repeatDaily();
    void repeatDaily_data();

    void repeatWeekly();
    void repeatWeekly_data();

    void repeatMonthly();
    void repeatMonthly_data();

    void repeatYearly();
    void repeatYearly_data();

    void showOnNearest();

    void setGet();

    void readVCal_data();
    void readVCal();

    void transferAsVCal();
};

QTEST_APP_MAIN( tst_QAppointment, QtopiaApplication )
#include "tst_qappointment.moc"

/*?
    Tests that appointments set to repeat daily repeat at
    the correct intervals.  This test:
      * Constructs a QAppointment using given test start and end dates.
      * Sets the appointment to repeat daily forever.
      * Tests that the appointment occurs daily for the next 1000 days.
*/
void tst_QAppointment::repeatDaily()
{
    QFETCH(QDateTime, start);
    QFETCH(QDateTime, end);

    QAppointment app(start, end);

    app.setRepeatRule(QAppointment::Daily);
    QCOMPARE( app.repeatRule(), QAppointment::Daily );

    app.setRepeatForever();
    QVERIFY( app.repeatForever() );

    for (int i = 0; i < 1000; ++i) {
        QDateTime expectedNextStart = trimSeconds(start.addDays(i));
        QDateTime expectedNextEnd = trimSeconds(end.addDays(i));
        QDate from = end.date().addDays(i);
        QCOMPARE( app.nextOccurrence(from).start(),
                  expectedNextStart);
        QCOMPARE( app.nextOccurrence(from).end(),
                  expectedNextEnd);
    }
}

/*?
    Data for the repeatDaily() testcase.
*/
void tst_QAppointment::repeatDaily_data()
{
    QTest::addColumn<QDateTime>("start");
    QTest::addColumn<QDateTime>("end");

    /* Test a 5 minute long appointment starting now - a typical testcase.*/
    QTest::newRow("about 5 minutes long")
        << QDateTime::currentDateTime()
        << QDateTime::currentDateTime().addSecs(5*60);

    /* Test an appointment exactly one day long. */
    QTest::newRow("1 day long")
        << QDateTime(QDate(2005, 10, 5))
        << QDateTime(QDate(2005, 10, 5), QTime(11, 59, 59));

    /* Test an appointment exactly two days long. */
    QTest::newRow("2 days long")
        << QDateTime(QDate(2001, 1, 4))
        << QDateTime(QDate(2001, 1, 5), QTime(11, 59, 59));

    /* Test an appointment two days and one second long;
     * the end of a date falling just across the boundary of a day might
     * confuse the class. */
    QTest::newRow("2 days and a bit long")
        << QDateTime(QDate(2001, 1, 4))
        << QDateTime(QDate(2001, 1, 5));

    /* Test an appointment exactly one month long. */
    QTest::newRow("1 month long")
        << QDateTime(QDate(2005, 10, 5))
        << QDateTime(QDate(2005, 11, 4), QTime(11, 59, 59));

    /* Test an appointment exactly two months long. */
    QTest::newRow("2 months long")
        << QDateTime(QDate(2001, 1, 4))
        << QDateTime(QDate(2001, 3, 3), QTime(11, 59, 59));

    /* Test an appointment two months and one second long;
     * the end of a date falling just across the boundary of a month might
     * confuse the class. */
    QTest::newRow("2 months and a bit long")
        << QDateTime(QDate(2001, 1, 4))
        << QDateTime(QDate(2001, 3, 4));


    /* Test an appointment exactly one year long. */
    QTest::newRow("1 year long")
        << QDateTime(QDate(2005, 10, 5))
        << QDateTime(QDate(2006, 10, 4), QTime(11, 59, 59));

    /* Test an appointment exactly two years long. */
    QTest::newRow("2 years long")
        << QDateTime(QDate(2001, 1, 4))
        << QDateTime(QDate(2003, 1, 3), QTime(11, 59, 59));

    /* Test an appointment two years and one second long;
     * the end of a date falling just across the boundary of a year might
     * confuse the class. */
    QTest::newRow("2 years and a bit long")
        << QDateTime(QDate(2001, 1, 4))
        << QDateTime(QDate(2003, 1, 4));

    /* Test an appointment where one of the days on which it should be
     * repeated is the 29th of February.  Since this is an unusual date,
     * the class might get confused.
     */
    QTest::newRow("repeats over 29 Feb")
        << QDateTime(QDate(2000, 2, 4))
        << QDateTime(QDate(2000, 2, 6));

    /* Test an appointment where one of the days on which the first occurrence
     * takes place is the 29th of February.  Since this is an unusual date,
     * the class might get confused.
     */
    QTest::newRow("takes place over 29 Feb")
        << QDateTime(QDate(2000, 2, 27))
        << QDateTime(QDate(2000, 3, 6));
}

/*?
    Tests that appointments set to repeat weekly repeat as
    expected.
*/
void tst_QAppointment::repeatWeekly()
{
    QFETCH(QDateTime, start);
    QFETCH(QDateTime, end);

    QAppointment app(start, end);

    app.setRepeatRule(QAppointment::Daily);
    QCOMPARE( app.repeatRule(), QAppointment::Daily );

    app.setRepeatForever();
    QVERIFY( app.repeatForever() );

    for (int i = Qt::Monday; i <= Qt::Sunday; ++i) {
        QVERIFY( !app.repeatOnWeekDay(i) );
        app.setRepeatOnWeekDay(i, true);
        QVERIFY( app.repeatOnWeekDay(i) );
        app.setRepeatOnWeekDay(i, false);
        QVERIFY( !app.repeatOnWeekDay(i) );
    }

    app.setRepeatRule(QAppointment::Weekly);
    QCOMPARE( app.repeatRule(), QAppointment::Weekly );

    for (int i = Qt::Monday; i <= Qt::Sunday; ++i) {
        QVERIFY( !app.repeatOnWeekDay(i) );
        app.setRepeatOnWeekDay(i, true);
        QVERIFY( app.repeatOnWeekDay(i) );
        app.setRepeatOnWeekDay(i, false);
        QVERIFY( !app.repeatOnWeekDay(i) );
    }

    {
        // test next occurrence
        // always repeats on the day of the first occurrence,
        // regardless of setRepeatOnWeekDay
        QDate tomorrow = end.addDays(1).date();
        QDate expected = start.addDays(7).date();
        QCOMPARE( app.nextOccurrence(tomorrow).date(), expected);
        app.setFrequency(2);
        expected = expected.addDays(7);
        QCOMPARE( app.nextOccurrence(tomorrow).date(), expected);
        app.setFrequency(1);
    }

}

/*?
    Data for the repeatWeekly() testcase.
    Uses same data as repeatDaily().
*/
void tst_QAppointment::repeatWeekly_data()
{
    repeatDaily_data();
}

/*?
    Tests that appointments set to repeat monthly repeat as
    expected.  This test:
      * Constructs a QAppointment using given test start and end dates.
      * Sets the appointment to repeat monthly forever.
      * Tests that the appointment repeats in one month's time, and
        again in three months' time if setFrequency(3) is called.
*/
void tst_QAppointment::repeatMonthly()
{
    QFETCH(QDateTime, start);
    QFETCH(QDateTime, end);

    QAppointment app(start, end);

    app.setRepeatRule(QAppointment::MonthlyDate);
    QCOMPARE( app.repeatRule(), QAppointment::MonthlyDate );

    app.setRepeatForever();
    QVERIFY( app.repeatForever() );
    
    app.setShowOnNearest(true);

    {
        QDate from = end.addDays(1).date();
        QDate expected = start.addMonths(1).date();
        QCOMPARE( app.nextOccurrence(from).date(), expected);
        app.setFrequency(3);
        expected = start.date().addMonths(3);
        QCOMPARE( app.nextOccurrence(from).date(), expected);
        app.setFrequency(1);
    }

/*
    for (int i = 1; i < 1000; ++i) {
        QDateTime expectedNextStart = trimSeconds(start.addMonths(i));
        QDateTime expectedNextEnd = trimSeconds(end.addMonths(i));
        QDate from = end.date().addMonths(i);
        QCOMPARE( app.nextOccurrence(from).start(),
                  expectedNextStart);
        QCOMPARE( app.nextOccurrence(from).end(),
                  expectedNextEnd);
    }
*/
}

/*?
    Data for the repeatMonthly() testcase.
    Uses same data as repeatDaily().
*/
void tst_QAppointment::repeatMonthly_data()
{
    repeatDaily_data();
}

/*?
    Tests that appointments set to repeat yearly repeat as
    expected.  This test:
      * Constructs a QAppointment using given test start and end dates.
      * Sets the appointment to repeat yearly forever.
      * Tests that the appointment repeats in one year's time, and
        again in three years' time if setFrequency(3) is called.
*/
void tst_QAppointment::repeatYearly()
{
    QFETCH(QDateTime, start);
    QFETCH(QDateTime, end);

    QAppointment app(start, end);

    app.setRepeatRule(QAppointment::Yearly);
    QCOMPARE( app.repeatRule(), QAppointment::Yearly );

    app.setRepeatForever();
    QVERIFY( app.repeatForever() );

    {
        // test next occurrence
        QDate from = end.addDays(1).date();
        QDate expected = start.addYears(1).date();
        QCOMPARE( app.nextOccurrence(from).date(), expected);
        app.setFrequency(3);
        expected = expected.addYears(2);
        QCOMPARE( app.nextOccurrence(from).date(), expected);
        app.setFrequency(1);
    }
}

/*?
    Data for the repeatYearly() testcase.
    Uses same data as repeatDaily().
*/
void tst_QAppointment::repeatYearly_data()
{
    repeatDaily_data();
}

/*?
    Test showOnNearest by constructing a yearly appointment taking
    place entirely on February 29 and testing whether or not it
    occurs on various leap and non-leap years.
*/
void tst_QAppointment::showOnNearest()
{
    QDateTime start = QDateTime( QDate(2000, 2, 29), QTime( 11, 30, 0) );
    QDateTime end = start.addSecs(5*60);
    QAppointment app(start, end);

    app.setRepeatRule(QAppointment::Yearly);
    app.setRepeatForever();
    QVERIFY( app.repeatForever() );

    {
        QDate from = end.addDays(1).date();
        app.setShowOnNearest(false);

        // without showOnNearest, next occurrence should be the next
        // leap year.
        QDate expected = start.addYears(4).date();
        QCOMPARE( app.nextOccurrence(from).date(), expected);

        app.setShowOnNearest(true);
        // with showOnNearest, next occurrence should be next year,
        // february 28.
        expected = expected.addDays(-1);
        expected = expected.addYears(-3);
        QCOMPARE( app.nextOccurrence(from).date(), expected);
    }
}

/*?
    Helper function for timeZone() testcase.
    Sets the current timezone for Qt Extended applications to 
    the given \a city.
*/
void tst_QAppointment::setTimeZone(const QString &city)
{
    QTimeZone::setApplicationTimeZone(city);
}

/*?
    Test timezone related functions of QAppointment.
    This test:
        * Checks that a QAppointment has, by default, an invalid timezone.
        * Checks that setting and getting a timezone on an appointment works
          correctly.
        * Checks that startInCurrentTZ and endInCurrentTZ return a start and
          end date appropriately modified for the current timezone.
*/
void tst_QAppointment::timeZone()
{
    QAppointment app;

    // if no timezone set, should be invalid
    QCOMPARE( app.timeZone(), QTimeZone() );

    QTimeZone tz("Australia/Brisbane");
    QVERIFY( tz.isValid() );
    app.setTimeZone(tz);
    QCOMPARE( app.timeZone(), tz );

    QDateTime start(QDate(2005, 10, 2), QTime(12, 30, 0));
    QDateTime end(QDate(2005, 10, 3), QTime(10, 15, 0));

    app.setStart(start);
    app.setEnd(end);

    {
        setTimeZone("Australia/Brisbane");

        QDateTime gotStart = app.start();
        QDateTime gotEnd = app.end();
        QDateTime gotStartTz = app.startInCurrentTZ();
        QDateTime gotEndTz = app.endInCurrentTZ();

        QCOMPARE( gotStart, start );
        QCOMPARE( gotStart, gotStartTz );
        QCOMPARE( gotEnd, end );
        QCOMPARE( gotEnd, gotEndTz );

        // Africa/Abidjan is UTC
        setTimeZone("Africa/Abidjan");

        gotStart = app.start();
        gotEnd = app.end();
        gotStartTz = app.startInCurrentTZ();
        gotEndTz = app.endInCurrentTZ();

        QCOMPARE( gotStart, start );
        QCOMPARE( gotEnd, end );

        // tz versions should be 10 hours less than others
        gotStart = gotStart.addSecs(-60*60*10);
        gotEnd = gotEnd.addSecs(-60*60*10);
        QCOMPARE(gotStartTz, gotStart);
        QCOMPARE(gotEndTz, gotEnd);
    }

/*
    {
        QDate date(2006, 10, 10);
        setTimeZone("America/Juneau");
        app.setRepeatUntil(date);
        app.setRepeatRule(QAppointment::Daily);
        QDate gotDate = app.repeatUntil();
        QCOMPARE( gotDate, date );
        gotDate = app.repeatUntilInCurrentTZ();
        date.addDays(1);
        QCOMPARE( gotDate, date );
    }
*/
}

/*?
    Test case for setting alarms on QAppointment objects.
    This test:
        * Tests that a QAppointment, by default, has no alarm.
        * Tests that alarm flags and delay can be set and retrieved.
        * Tests that an appointment has an alarm when alarm flags are
          not QAppointment::NoAlarm.
        * Tests that clearing an alarm returns the appointment to a state
          of having no alarm.
*/
void tst_QAppointment::setAlarm()
{
    QFETCH( QDateTime, start );
    QFETCH( QDateTime, end );
    QFETCH( int, minutes );
    QFETCH( int, flags );

    QAppointment app(start, end);
    QCOMPARE( app.alarm(), QAppointment::NoAlarm );
    QCOMPARE( app.alarmDelay(), 0 );
    QVERIFY( !app.hasAlarm() );

    app.setAlarm( minutes, QFlags<QAppointment::AlarmFlag>(flags) );
    QCOMPARE( (int)app.alarm(), flags );
    QCOMPARE( app.alarmDelay(), minutes );
    if ( flags != (int)QAppointment::NoAlarm) QVERIFY( app.hasAlarm() );

    app.clearAlarm();
    QCOMPARE( app.alarm(), QAppointment::NoAlarm );
    QCOMPARE( app.alarmDelay(), minutes );
    QVERIFY( !app.hasAlarm() );
}

/*?
    Data for setAlarm() testcase.
*/
void tst_QAppointment::setAlarm_data()
{
    QTest::addColumn<QDateTime>("start");
    QTest::addColumn<QDateTime>("end");
    QTest::addColumn<int>("minutes");
    QTest::addColumn<int>("flags");

    QTest::newRow("audible")
        << QDateTime( QDate( 2005, 10, 1 ) )
        << QDateTime( QDate( 2005, 12, 1 ) )
        << 10
        << (int)QAppointment::Audible;

    QTest::newRow("visible")
        << QDateTime( QDate( 2005, 10, 1 ) )
        << QDateTime( QDate( 2005, 12, 1 ) )
        << 10
        << (int)QAppointment::Visible;

    QTest::newRow("none")
        << QDateTime( QDate( 2005, 10, 1 ) )
        << QDateTime( QDate( 2005, 12, 1 ) )
        << 10
        << (int)QAppointment::NoAlarm;

    QTest::newRow("negative")
        << QDateTime( QDate( 2005, 10, 1 ) )
        << QDateTime( QDate( 2005, 12, 1 ) )
        << -10
        << (int)QAppointment::Audible;
}

/*?
    Testcase for simple setting and getting of
    QAppointment attributes.
    This test sets and retrieves the following QAppointment
    attributes:
        * description
        * location
        * notes
        * allDay
        * frequency
        * showOnNearest
*/
void tst_QAppointment::setGet()
{
    QAppointment app;
    QString str;
    
    app.setDescription(str);
    app.setLocation(str);
    app.setNotes(str);
    QCOMPARE( app.description(), str );
    QCOMPARE( app.location(), str );
    QCOMPARE( app.notes(), str );

    str = "";
    app.setDescription(str);
    app.setLocation(str);
    app.setNotes(str);
    QCOMPARE( app.description(), str );
    QCOMPARE( app.location(), str );
    QCOMPARE( app.notes(), str );

    str = "Simple string";
    app.setDescription(str);
    app.setLocation(str);
    app.setNotes(str);
    QCOMPARE( app.description(), str );
    QCOMPARE( app.location(), str );
    QCOMPARE( app.notes(), str );

    app.setAllDay(true);
    QVERIFY(app.isAllDay());
    app.setAllDay(false);
    QVERIFY(!app.isAllDay());
    app.setAllDay(true);
    QVERIFY(app.isAllDay());

    app.setFrequency( 12 );
    QCOMPARE( app.frequency(), 12 );
    app.setFrequency( 1 );
    QCOMPARE( app.frequency(), 1 );
    app.setFrequency( 0 );
    QCOMPARE( app.frequency(), 1 );
    app.setFrequency( -1 );
    QCOMPARE( app.frequency(), 1 );

    app.setShowOnNearest(true);
    QVERIFY(app.showOnNearest());
    app.setShowOnNearest(false);
    QVERIFY(!app.showOnNearest());
    app.setShowOnNearest(true);
    QVERIFY(app.showOnNearest());
}

void tst_QAppointment::readVCal_data()
{
    QTest::addColumn<QByteArray>("bytes");
    QTest::addColumn<QString>("description");
    QTest::addColumn<QString>("localzone");
    QTest::addColumn<QDateTime>("utcStart");
    QTest::addColumn<QDateTime>("utcEnd");
    QTest::addColumn<QDateTime>("localStart");
    QTest::addColumn<QDateTime>("localEnd");

    QTest::newRow("utc vcal") <<
        QByteArray(
                "BEGIN:VCALENDAR\n"
                "VERSION:1.0\n"
                "BEGIN:VEVENT\n"
                "SUMMARY:b1\n"
                "DTSTART:20070201T110000Z\n"
                "DTEND:20070201T111500Z\n"
                "END:VEVENT\n"
                "END:VCALENDAR\n")
        << "b1"
        << "Australia/Brisbane" // +10
        << QDateTime(QDate(2007, 2, 1), QTime(11,0,0))
        << QDateTime(QDate(2007, 2, 1), QTime(11,15,0))
        << QDateTime(QDate(2007, 2, 1), QTime(21,0,0))
        << QDateTime(QDate(2007, 2, 1), QTime(21,15,0));

    // from the manual tests directory

    QTest::newRow("local vcal") <<
        QByteArray(
                "BEGIN:VCALENDAR\n"
                "VERSION:1.0\n"
                "BEGIN:VEVENT\n"
                "SUMMARY:b2\n"
                "DTSTART:20030114T080000\n"
                "DTEND:20030114T090000\n"
                "END:VEVENT\n"
                "END:VCALENDAR\n")
        << "b2"
        << "Australia/Brisbane" // +10
        << QDateTime(QDate(2003, 1, 14), QTime(8,0,0))
        << QDateTime(QDate(2003, 1, 14), QTime(9,0,0))
        << QDateTime(QDate(2003, 1, 14), QTime(8,0,0))
        << QDateTime(QDate(2003, 1, 14), QTime(9,0,0));

    // specific timezones are optional and are not handled by Qtopia, e.g. TZ:+10
    // since these dont' account for daylight saving times anyway.
    // later may support the iCal style time zone handling.
}

/*?
    Testcase for reading vcalendar files.
    QAppointment attributes.
    This test reads a vcalendar file setting the
    attributes:
        * description
        * start
        * end
*/
void tst_QAppointment::readVCal()
{
    QFETCH(QByteArray, bytes);
    QFETCH(QString, description);
    QFETCH(QString, localzone);
    QFETCH(QDateTime, utcStart);
    QFETCH(QDateTime, utcEnd);
    QFETCH(QDateTime, localStart);
    QFETCH(QDateTime, localEnd);

    // setup
    setTimeZone(localzone);

    QList<QAppointment> list = QAppointment::readVCalendarData(bytes.constData(), bytes.size());

    QCOMPARE(list.count(), 1);
    QAppointment a = list[0];

    QCOMPARE(a.description(), description);
    QCOMPARE(a.start(), utcStart);
    QCOMPARE(a.end(), utcEnd);
    QCOMPARE(a.startInCurrentTZ(), localStart);
    QCOMPARE(a.endInCurrentTZ(), localEnd);
};

/*
    Test that records written as a vcalender file read back to an equivalent appointment.
*/
void tst_QAppointment::transferAsVCal()
{
    setTimeZone("Australia/Brisbane");

    QAppointment before;
    before.setDescription("Tokyo event");
    before.setTimeZone(QTimeZone("Asia/Tokyo"));
    before.setStart(QDateTime(QDate(2008, 10, 10), QTime(9, 0, 0)));
    before.setEnd(QDateTime(QDate(2008, 10, 10), QTime(10, 0, 0)));

    QByteArray bytes;
    {
        QDataStream ds(&bytes, QIODevice::WriteOnly);
        before.writeVCalendar(&ds);
    }
    QAppointment after;
    QList<QAppointment> list = QAppointment::readVCalendar(bytes);
    QCOMPARE(list.count(), 1);
    after = list[0];


    // early compares covered in latter ones, does both
    // to focus on main areas of concern.
    QCOMPARE(before.description(), after.description());
    QCOMPARE(before.timeZone(), after.timeZone());
    QCOMPARE(before.start(), after.start());
    QCOMPARE(before.startInCurrentTZ(), after.startInCurrentTZ());
    QCOMPARE(before, after);
}

/*?
    Helper function; return a date with seconds set to 0.
*/
QDateTime tst_QAppointment::trimSeconds( const QDateTime &dt )
{
    QDateTime r = dt;
    r.setTime( QTime(dt.time().hour(), dt.time().minute()) );
    return r;
}

/*?
    Data for the startEndCtor testcase.
*/
void tst_QAppointment::startEndCtor_data()
{
    QTest::addColumn<QDateTime>("start");
    QTest::addColumn<QDateTime>("end");

    QTest::newRow("simple")
        << QDateTime::currentDateTime()
        << QDateTime::currentDateTime().addSecs(1000);

    QTest::newRow("extreme")
        << QDateTime(QDate(-4711, 1, 1))
        << QDateTime(QDate(1000000, 12, 31));

    QTest::newRow("invalid")
        << QDateTime(QDate(5000, 6, 3))
        << QDateTime(QDate(2001, 11, 1));
}

/*?
    Test that the constructor with a start and end date works
    as expected.  This test:
        * Constructs an appointment using a specified start and end date.
        * Tests that the start and end date are correctly retrieved.
        * Tests that the start and end dates are correctly set in the case
          where an appointment is constructed with start > end.
*/
void tst_QAppointment::startEndCtor()
{
    QFETCH( QDateTime, start );
    QFETCH( QDateTime, end );

    QAppointment app(start, end);
    
    QCOMPARE( app.start(), trimSeconds(start) );
    if (start > end)
        QCOMPARE( app.end(), trimSeconds(start.addSecs(5*60)) );
    else
        QCOMPARE( app.end(), trimSeconds(end) );
}

/*?
    Testcase for QAppointment default constructor.  This test:
        * Constructs an appointment using the default constructor.
        * Ensures that the appointment is 5 minutes in length.
*/
void tst_QAppointment::ctor()
{
    QAppointment app;
    QDateTime start = app.start();
    QDateTime end = app.end();

    start = start.addSecs( 5*60 );
    QCOMPARE( start, end );
}
