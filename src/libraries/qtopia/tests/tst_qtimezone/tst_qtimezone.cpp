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

#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QTimeZone>
#include <QtopiaApplication>

Q_DECLARE_METATYPE(QDateTime);



//TESTED_CLASS=QTimeZone
//TESTED_FILES=src/libraries/qtopia/qtimezone.h

/*
    The tst_QTimeZone class provides unit tests for the QTimeZone class.
*/
class tst_QTimeZone : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void toUtc();
    void toUtc_data();

    void fromUtc();
    void fromUtc_data();

    void current();
    void current_data();

    void defaultCurrent();

    void convert();
    void convert_data();

    void toCurrent();
    void toCurrent_data();

    void fromCurrent();
    void fromCurrent_data();

    void fromTime_t();
    void fromTime_t_data();

    void toTime_t();
    void toTime_t_data();

    void isValid();
    void isValid_data();

    void utcDateTime_data();
    void utcDateTime();

    void defaultCtor();

    void copyCtor();

    void utc();
    void utc_data();

    void abbrev();
    void abbrev_data();

    void locationInformation();
    void locationInformation_data();

    void findFromMinutesEast();
    void findFromMinutesEast_data();

private:
    void setTimeZone(const QString &tzid);
    QString zonePath() const;

    QString oldTz;
};

QTEST_APP_MAIN(tst_QTimeZone, QtopiaApplication)
#include "tst_qtimezone.moc"

/*?
    Initialisation before every test function.
    Stores the current value of the TZ variable.
*/
void tst_QTimeZone::init()
{
    oldTz = getenv("TZ");
}

/*?
    Cleanup after every test function.

    Restores the TZ variable to its value when init()
    was last called, and calls QTimeZone::current() to
    force QTimeZone to see the change.

    Thanks to init() and cleanup(), tests can safely change
    the system timezone and have it changed back when they
    are done.
*/
void tst_QTimeZone::cleanup()
{
    setenv("TZ", oldTz.toLatin1().constData(), 1);
    QTimeZone::current();
}

/*?
    Data for the utc() test function.
*/
void tst_QTimeZone::utc_data()
{
    /* through the year to catch daylight savings cases as well */
    QTest::addColumn<QDateTime>("date");

    QTest::newRow("Date 1")
        << QDateTime( QDate(2003, 1, 1), QTime(12, 0, 0) );

    QTest::newRow("Date 2")
        << QDateTime( QDate(2003, 2, 6), QTime(3, 0, 0) );

    QTest::newRow("Date 4")
        << QDateTime( QDate(2003, 4, 6), QTime(4, 0, 0) );

    QTest::newRow("Date 5")
        << QDateTime( QDate(2003, 5, 1), QTime(4, 10, 0) );

    QTest::newRow("Date 6")
        << QDateTime( QDate(2003, 7, 1), QTime(17, 20, 0) );

    QTest::newRow("Date 7")
        << QDateTime( QDate(2003, 9, 1), QTime(21, 5, 0) );

    QTest::newRow("Date 8")
        << QDateTime( QDate(2003, 10, 1), QTime(23, 50, 0) );

    QTest::newRow("Date 9")
        << QDateTime( QDate(2003, 11, 1), QTime(0, 15, 0) );
}

/*?
  Should test that the utcDateTime functions reflect the utc() timezone.

  Also should ensure each part of the api returns a sensible value
  for the UTC timezone.  UTC is NOT London, nor any other physical location.
*/
void tst_QTimeZone::utc()
{
    QString testTimeZone;
    QTimeZone current, tz;
    QDateTime utcDate, c1, c2;

    // preferably should test with timezone both with, and without
    // daylight saving.
    testTimeZone = "Australia/Sydney";

    setTimeZone(testTimeZone);
    current = QTimeZone::current();
    QVERIFY(current.isValid());
    QCOMPARE(current.id(), testTimeZone);
    

    QFETCH(QDateTime, date);

    tz = QTimeZone::utc();
    QVERIFY(tz.isValid());

    // We are in UTC, so tz.fromUtc() should return an equivalent time
    utcDate = tz.fromUtc(date);
    QCOMPARE(utcDate, date);

    // and to current should work as well
    c1 = current.fromUtc(date);
    c2 = tz.convert(date, current);
    QCOMPARE(c2, c2);

    c1 = current.toUtc(date);
    c2 = current.convert(date, tz);
    QCOMPARE(c2, c2);

    // second run, with a TZ without DST
    testTimeZone = "Australia/Brisbane";

    setTimeZone(testTimeZone);
    current = QTimeZone::current();
    QVERIFY(current.isValid());
    QCOMPARE(current.id(), testTimeZone);
    
    c1 = current.fromUtc(date);
    c2 = tz.convert(date, current);
    QCOMPARE(c2, c2);

    c1 = current.toUtc(date);
    c2 = current.convert(date, tz);
    QCOMPARE(c2, c2);

    // now check functions don't crash and return sensible answers.

    QCOMPARE(tz.latitude(), 0);
    QCOMPARE(tz.longitude(), 0);
    QCOMPARE(tz.id(), QString("UTC"));

    QVERIFY(tz.standardAbbreviation().isNull());
    QVERIFY(tz.dstAbbreviation().isNull());
    QVERIFY(tz.description().isNull());
    QVERIFY(tz.area().isNull());
    QVERIFY(tz.city().isNull());
    QVERIFY(tz.countryCode().isNull());

    QCOMPARE(tz.distance(current), 0);
}

/*?
    Test function for the QTimeZone::standardAbbreviation()
    and QTimeZone::dstAbbreviation() functions.
    Constructs a timezone for the given location and compares
    the standard and daylight savings abbreviations with
    expected results.
*/
void tst_QTimeZone::abbrev()
{
    QFETCH(QString, tz);
    QFETCH(QString, standardAbbrev);
    QFETCH(QString, dstAbbrev);

    
    

    QTimeZone zone(tz.toLatin1().constData());
    QCOMPARE(zone.standardAbbreviation(), standardAbbrev);
    QCOMPARE(zone.dstAbbreviation(), dstAbbrev);
}

/*?
    Data for abbrev() test function.
*/
void tst_QTimeZone::abbrev_data()
{
    // timezone name (for QTimeZone(char *) ctor)
    QTest::addColumn<QString>("tz");
    // what standardAbbreviation() should return
    QTest::addColumn<QString>("standardAbbrev");
    // what dstAbbreviation() should return
    QTest::addColumn<QString>("dstAbbrev");

    QTest::newRow("standard and dst New York")   // TZ with daylight savings
        << "America/New_York"
        << "EST"
        << "EDT";

    QTest::newRow("standard and dst London")   // TZ with daylight savings
        << "Europe/London"
        << "GMT"
        << "BST";

    /*
     * Must check: for a timezone which does not observe DST, should
     * dstAbbreviation() return empty string or same as
     * standardAbbreviation() ?
     */
    /*
    QTest::newRow("standard, no dst")   // TZ without daylight savings
        << "Australia/Brisbane"
        << "EST"
        << "EST";
     */
}

/*?
    Helper function; returns the path containing zoneinfo database
    (usually /usr/share/zoneinfo).
    Uses the same algorithm as qtimezone.cpp.
*/
QString tst_QTimeZone::zonePath() const
{
        QString ret;
#if defined(QTOPIA_ZONEINFO_PATH)
        ret = QTOPIA_ZONEINFO_PATH;
#elif defined(Q_OS_WIN32)
        ret = Qtopia::qtopiaDir() + "etc\\zoneinfo\\";
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
        ret = "/usr/share/zoneinfo/";
#else
        ret = Qtopia::qtopiaDir() + "etc/zoneinfo/";
#endif
        return ret;
}

/*?
    Helper function; sets the current timezone for Qt Extended applications to
    that of a specific location.
*/
void tst_QTimeZone::setTimeZone(const QString &tzid)
{
    QSettings config("Trolltech", "locale");
    config.beginGroup("Location");
    config.setValue("Timezone", tzid);
    config.endGroup();

    QTimeZone temp(tzid.toLatin1().constData());
    setenv("TZ", tzid.toLatin1().constData(), true);

    QTimeZone::current();
}

/*?
    Test function for copy constructor.
    This test:
        * Verifies that a copy of an invalid QTimeZone is itself invalid.
        * Verifies that calling setId(foo) on a QTimeZone gives the same
          result as creating a copy of a QTimeZone(foo) for both invalid
          and valid timezones.
*/
void tst_QTimeZone::copyCtor()
{
    // Test that a copy of an invalid timezone is itself invalid.
    {
        QTimeZone tz;
        QTimeZone other(tz);
        QVERIFY(tz == other);
        QVERIFY(!other.isValid());
    }

    // Test that using a copy constructor has the same effect as setId for
    // invalid locations
    {
        QTimeZone tz("Invalid/Location");
        QTimeZone copy(tz);
        QTimeZone set;
        set.setId("Invalid/Location");
        QVERIFY(tz == copy);
        QVERIFY(tz == set);
        // if == defined correctly, should be redundant, but to be safe...
        QVERIFY(copy == set);
    }

    // Test that using a copy constructor has the same effect as setId for
    // valid locations
    {
        QTimeZone tz("Australia/Brisbane");
        QTimeZone copy(tz);
        QTimeZone set;
        set.setId("Australia/Brisbane");
        QVERIFY(tz == copy);
        QVERIFY(tz == set);
        // if == defined correctly, should be redundant, but to be safe...
        QVERIFY(copy == set);
    }
}

/*?
    Data for toUtc() test function.
*/
void tst_QTimeZone::toUtc_data()
{
    // define the test elements we're going to use
    QTest::addColumn<QDateTime>("localTime");
    QTest::addColumn<QDateTime>("utcTime");
    QTest::addColumn<QString>("tzid");
    QTest::addColumn<bool>("reversable");

    // create a first testdata instance and fill it with data
    QTest::newRow("NYC Winter")
        << QDateTime( QDate(2003, 1, 1), QTime(12, 0, 0) )
        << QDateTime( QDate(2003, 1, 1), QTime(17, 0, 0) )
        << "America/New_York"
        << true;

    QTest::newRow("NYC Winter - GMT offset")
        << QDateTime( QDate(2003, 1, 1), QTime(12, 0, 0) )
        << QDateTime( QDate(2003, 1, 1), QTime(17, 0, 0) )
        << "Etc/GMT+5"
        << true;

    // DST = Sun Apr 6 02:00:00 2003 UTC
    QTest::newRow("1 Sec Before DST")   // offset = 5
        << QDateTime( QDate(2003, 4, 6), QTime(1, 59, 59) )
        << QDateTime( QDate(2003, 4, 6), QTime(6, 59, 59) )
        << "America/New_York"
        << true;

    QTest::newRow("Skipped Hour")       // offset = 4
        << QDateTime( QDate(2003, 4, 6), QTime(2, 0, 0) )
        << QDateTime( QDate(2003, 4, 6), QTime(7, 0, 0) )
        << "America/New_York"
        << false;

    QTest::newRow("DST Start")          // offset = 4
        << QDateTime( QDate(2003, 4, 6), QTime(3, 0, 0) )
        << QDateTime( QDate(2003, 4, 6), QTime(7, 0, 0) )
        << "America/New_York"
        << true;

    // STD = Sun Oct 26 06:00:00 2003
    QTest::newRow( "1 Sec Before STD" ) // offset = 4 (DST)
        << QDateTime( QDate(2003, 10, 26), QTime(0, 59, 59) )
        << QDateTime( QDate(2003, 10, 26), QTime(4, 59, 59) )
        << "America/New_York"
        << true;

    QTest::newRow("STD Start")          // offset = 5 (STD)
        << QDateTime( QDate(2003, 10, 26), QTime(1, 0, 0) )
        << QDateTime( QDate(2003, 10, 26), QTime(6, 0, 0) )
        << "America/New_York"
        << true;

    QTest::newRow("Early Date")
        << QDateTime( QDate(1890, 5, 6), QTime(8, 0, 1) )
        << QDateTime( QDate(1890, 5, 6), QTime(12, 0, 1) )
        << "America/New_York"
        << true;

    QTest::newRow("Brisbane Winter")
        << QDateTime( QDate(2003, 6, 1), QTime(22, 0, 0) )
        << QDateTime( QDate(2003, 6, 1), QTime(12, 0, 0) )
        << "Australia/Brisbane"
        << true;
}

/*?
    Test function for QTimeZone::toUtc function.
    Constructs a QTimeZone for the specified zone,  converts a local
    time to UTC time using toUtc, and verifies that the generated
    time equals some expected time.
*/
void tst_QTimeZone::toUtc()
{
    QFETCH(QString, tzid);
    QFETCH(QDateTime, localTime);
    QFETCH(QDateTime, utcTime);
    QFETCH(bool, reversable);

    QTimeZone tz(tzid.toLatin1().constData());
    QDateTime utcCompare = tz.toUtc(localTime);
    QCOMPARE(utcCompare, utcTime);

    if ( reversable ) {
        utcCompare = tz.fromUtc(utcCompare);
        QCOMPARE(utcCompare, localTime);
    }
}

/*?
    Data for fromUtc() test function.
*/
void tst_QTimeZone::fromUtc_data()
{
    // Define the test elements we're going to use
    QTest::addColumn<QDateTime>("localTime");
    QTest::addColumn<QDateTime>("utcTime");
    QTest::addColumn<QString>("tzid");

    // Create a first testdata instance and fill it with data
    QTest::newRow("NYC Winter")
        << QDateTime( QDate(2003, 1, 1), QTime(12, 0, 0) )
        << QDateTime( QDate(2003, 1, 1), QTime(17, 0, 0) )
        << "America/New_York";

    // DST = Sun Apr 6 02:00:00 2003 UTC
    QTest::newRow("1 Sec Before DST")   // offset = 5
        << QDateTime( QDate(2003, 4, 6), QTime(1, 59, 59) )
        << QDateTime( QDate(2003, 4, 6), QTime(6, 59, 59) )
        << "America/New_York";

    QTest::newRow("After Dst")          // offset = 4
        << QDateTime( QDate(2003, 4, 6), QTime(3, 0, 0) )
        << QDateTime( QDate(2003, 4, 6), QTime(7, 0, 0) )
        << "America/New_York";

    // STD = Sun Oct 26 06:00:00 2003 UTC
    QTest::newRow("1 Sec Before STD")   // offset = 4 (DST)
        << QDateTime( QDate(2003, 10, 26), QTime(0, 59, 59) )
        << QDateTime( QDate(2003, 10, 26), QTime(4, 59, 59) )
        << "America/New_York";

    QTest::newRow("STD Start")          // offset = 5 (STD)
        << QDateTime( QDate(2003, 10, 26), QTime(1, 0, 0) )
        << QDateTime( QDate(2003, 10, 26), QTime(6, 0, 0) )
        << "America/New_York";

    QTest::newRow("Early Date")
        << QDateTime( QDate(1890, 5, 6), QTime(8, 0, 1) )
        << QDateTime( QDate(1890, 5, 6), QTime(12, 0, 1) )
        << "America/New_York";

    // STD = Sun Oct 26 06:00:00 2003
    QTest::newRow("Brisbane Winter")
        << QDateTime( QDate(2003, 6, 1), QTime(22, 0, 0) )
        << QDateTime( QDate(2003, 6, 1), QTime(12, 0, 0) )
        << "Australia/Brisbane";
}

/*?
    Test function for QTimeZone::fromUtc function.
    Constructs a QTimeZone for the specified zone,  converts a UTC
    time to local time using fromUtc, and verifies that the generated
    time equals some expected time.
*/
void tst_QTimeZone::fromUtc()
{
    QFETCH(QString, tzid);
    QFETCH(QDateTime, localTime);
    QFETCH(QDateTime, utcTime);

    QTimeZone tz(tzid.toLatin1().constData());
    QDateTime localCompare = tz.fromUtc(utcTime);
    QCOMPARE(localCompare, localTime);
}

/*?
    Data for current() test function.
*/
void tst_QTimeZone::current_data()
{
    toUtc_data();
}

/*?
    Test function for QTimeZone::current() function.
    This test:
        * Sets the current timezone to a specified zone.
        * Fetches a QTimeZone using QTimeZone::current().
        * Converts a local time in the current timezone to UTC and
          checks against an expected time.
        * Checks that the timezone ID of the current timezone is the
          expected value.
*/
void tst_QTimeZone::current()
{
    QFETCH(QString, tzid);
    QFETCH(QDateTime, localTime);
    QFETCH(QDateTime, utcTime);
    QFETCH(bool, reversable);
    QDateTime utcCompare;

    setTimeZone(tzid);

    QTimeZone tz = QTimeZone::current();
    QCOMPARE(tz.id(), tzid);

    // check both directions of convert (unless cannot - i.e. times in the Skipped Hour)
    utcCompare = tz.toUtc(localTime);
    QCOMPARE(utcCompare, utcTime);

    if ( reversable ) {
        utcCompare = tz.fromUtc(utcTime);
        QCOMPARE(utcCompare, localTime);
    }
}

/*?
    Test function for the default value of QTimeZone::current().
    This function tests that, when no TZ variable is set, QTimeZone::current()
    returns a default timezone defined in Qt Extended settings files.
*/
void tst_QTimeZone::defaultCurrent()
{
    QFile::remove(Qtopia::homePath() + "/Settings/Trolltech/locale.conf");

    // If the conf file is deleted, settings should fallback on the default values
    QSettings config("Trolltech", "locale");
    config.beginGroup("Location");
    QString currentLoc = config.value("Timezone"
            ,"America/New_York" // FIXME default hardcoded to same as in qtimezone.cpp
    ).toString();
    config.endGroup();

    // QTimeZone does not update unless TZ has been changed, so to trigger
    // the default timezone we need to set it to something and then back
    // to nothing.
    setenv("TZ", "not a real timezone", true);
    tzset();
    QTimeZone::current();
    setenv("TZ", "", true);
    tzset();

    QTimeZone tz = QTimeZone::current();
    QCOMPARE(tz.id(), currentLoc);
}

/*?
    Data for convert() test function.
*/
void tst_QTimeZone::convert_data()
{
    // Define the test elements we're going to use
    QTest::addColumn<QDateTime>("dt1");
    QTest::addColumn<QString>("dt1TzId");
    QTest::addColumn<QDateTime>("dt2");
    QTest::addColumn<QString>("dt2TzId");

    // Used http://www.timeanddate.com/worldclock/meetingtime.html to
    // help generate the test data
    QTest::newRow("Brisbane and New York - N. Hemi Winter")
        << QDateTime( QDate(2003, 1, 24), QTime(8, 0, 0) )
        << "Australia/Brisbane"
        << QDateTime( QDate(2003, 1, 23), QTime(17, 0, 0) )
        << "America/New_York";

    QTest::newRow("Brisbane and New York - N. Hemi Summer")
        << QDateTime( QDate(2003, 7, 24), QTime(8, 0, 0) )
        << "Australia/Brisbane"
        << QDateTime( QDate(2003, 7, 23), QTime(18, 0, 0) )
        << "America/New_York";

    QTest::newRow("Sydney and Newfoundland - N. Hemi Winter")
        << QDateTime( QDate(2003, 1, 24), QTime(21, 0, 0) )
        << "Australia/Sydney"
        << QDateTime( QDate(2003, 1, 24), QTime(6, 30, 0) )
        << "America/St_Johns";

    QTest::newRow("Sydney and Newfoundland - N. Hemi Summer")
        << QDateTime( QDate(2003, 7, 24), QTime(21, 0, 0) )
        << "Australia/Sydney"
        << QDateTime( QDate(2003, 7, 24), QTime(8, 30, 0) )
        << "America/St_Johns";
}

/*?
    Test function for QTimeZone::convert function.
    This test:
        * Constructs two QTimeZones for specified zones.
        * Verifies the QTimeZone objects were successfully created
          and are valid.
        * Converts a time from each timezone to the other and
          verifies the converted time matches some expected
          value.
*/
void tst_QTimeZone::convert()
{
    QFETCH(QDateTime, dt1);
    QFETCH(QString, dt1TzId);
    QFETCH(QDateTime, dt2);
    QFETCH(QString, dt2TzId);

    QTimeZone dt1Tz(dt1TzId.toLatin1().constData());
    QTimeZone dt2Tz(dt2TzId.toLatin1().constData());

    QVERIFY(dt1Tz.isValid());
    QVERIFY(dt2Tz.isValid());

    QDateTime convert1To2 = dt2Tz.convert(dt1, dt1Tz);
    QCOMPARE(convert1To2, dt2);

    QDateTime convert2To1 = dt1Tz.convert(dt2, dt2Tz);
    QCOMPARE(convert2To1, dt1);
}


/*?
    Data for toCurrent() test function.
*/
void tst_QTimeZone::toCurrent_data()
{
    QTest::addColumn<QDateTime>("brisbaneTime");
    QTest::addColumn<QDateTime>("tzTime");
    QTest::addColumn<QString>("tzId");

    // Used http://www.timeanddate.com/worldclock/meetingtime.html to
    // help me generate the test data
    QTest::newRow("Brisbane and New York - N. Hemi Winter")
        << QDateTime( QDate(2003, 1, 24), QTime(8, 0, 0) )
        << QDateTime( QDate(2003, 1, 23), QTime(17, 0, 0) )
        << "America/New_York";

    QTest::newRow("Brisbane and New York - N. Hemi Summer")
        << QDateTime( QDate(2003, 7, 24), QTime(8, 0, 0) )
        << QDateTime( QDate(2003, 7, 23), QTime(18, 0, 0) )
        << "America/New_York";
}

/*?
    Test case for QTimeZone::toCurrent function.
    Sets system timezone to Brisbane and verifies that times can be converted
    into Brisbane time correctly from various timezones.
*/
void tst_QTimeZone::toCurrent()
{
    // Ensure that the current timezone is brisbane
    setTimeZone("Australia/Brisbane");

    QFETCH(QDateTime, brisbaneTime);
    QFETCH(QDateTime, tzTime);
    QFETCH(QString, tzId);

    QTimeZone tz(tzId.toLatin1().constData());
    QVERIFY(tz.isValid());

    QDateTime currentTime = tz.toCurrent(tzTime);
    QCOMPARE(currentTime, brisbaneTime);
}

/*?
    Test data for fromCurrent() test function.
    Uses the same data as toCurrent().
*/
void tst_QTimeZone::fromCurrent_data()
{
    toCurrent_data();
}

/*?
    Test case for QTimeZone::fromCurrent function.
    Sets system timezone to Brisbane and verifies that times can be converted
    from Brisbane time correctly into various timezones.
*/
void tst_QTimeZone::fromCurrent()
{
    // Ensure that the current timezone is Brisbane
    setTimeZone("Australia/Brisbane");

    QFETCH(QDateTime, brisbaneTime);
    QFETCH(QDateTime, tzTime);
    QFETCH(QString, tzId);

    QTimeZone tz(tzId.toLatin1().constData());
    QVERIFY(tz.isValid());

    QDateTime compareTime = tz.fromCurrent(brisbaneTime);
    QCOMPARE(compareTime, tzTime);
}

/*?
    Data for fromTime_t test.
*/
void tst_QTimeZone::fromTime_t_data()
{
    // Define the test elements we're going to use
    QTest::addColumn<QDateTime>("localTime");
    QTest::addColumn<uint>("timet");
    QTest::addColumn<QString>("tzid");

    QTest::newRow( "1 second after epoch, NYC" )   // UTC-5
        << QDateTime( QDate(1969, 12, 31), QTime(19, 0, 1) )
        << (uint) 1
        << "America/New_York";

    QTest::newRow( "epoch, NYC" )   // UTC-5
        << QDateTime( QDate(1969, 12, 31), QTime(19, 0, 0) )
        << (uint) 0
        << "America/New_York";

    QTest::newRow( "1 second before epoch, NYC" )   // UTC-5
        << QDateTime( QDate(1969, 12, 31), QTime(19, 0, 0) )
        << (uint) 0
        << "America/New_York";

    QTest::newRow( "10 seconds after epoch, UTC" )   // UTC
        << QDateTime( QDate(1970, 1, 1), QTime(0, 0, 10) )
        << (uint) 10
        << "Africa/Abidjan";
}

/*?
    Test function for QTimeZone::fromTime_t.
    Constructs a QTimeZone for specified zone, generates a QDateTime using
    fromTime_t with a specified value for seconds-from-epoch, compares
    with an expected QDateTime.
*/
void tst_QTimeZone::fromTime_t()
{
    QFETCH(QString, tzid);
    QFETCH(QDateTime, localTime);
    QFETCH(uint, timet);

    QTimeZone tz(tzid.toLatin1().constData());
    QDateTime localCompare = tz.fromTime_t(timet);
    QCOMPARE(localCompare, localTime);
}

/*?
    Data for toTime_t test function.
*/
void tst_QTimeZone::toTime_t_data()
{
    fromTime_t_data();
}

/*?
    Test function for QTimeZone::toTime_t.
    Constructs a QTimeZone for specified zone, generates a seconds-from-epoch
    value using toTime_t with a specified QDateTime, and compares with an
    expected seconds-from-epoch value.
*/
void tst_QTimeZone::toTime_t()
{
    QFETCH(QString, tzid);
    QFETCH(QDateTime, localTime);
    QFETCH(uint, timet);

    QTimeZone tz(tzid.toLatin1().constData());
    uint timetCompare = tz.toTime_t(localTime);
    QCOMPARE(timetCompare, timet);
}

/*?
    Data for isValid test function.
*/
void tst_QTimeZone::isValid_data()
{
    QTest::addColumn<QString>("tzid");
    QTest::addColumn<bool>("valid");

    // Test every single timezone ID
    QStringList validIds = QTimeZone::ids();
    for ( int i = 0; i < validIds.size(); ++i ) {
        QTest::newRow(QString("Valid (%1)").arg(validIds[i]).toLatin1().constData())
            << validIds[i]
            << true;
    }
    QTest::newRow("Null") << QString() << false;
    QTest::newRow("Empty") << "" << false;
    QTest::newRow("Invalid 1") << "America/" << false;
    QTest::newRow("Invalid 2") << "Fake/Country" << false;
}

/*?
    Test function for QTimeZone::isValid() function.
    Constructs a QTimeZone for a specified zone and compares its isValid() return value
    to an expected value.
*/
void tst_QTimeZone::isValid()
{
    QFETCH(QString, tzid);
    QFETCH(bool, valid);

    // We should get a warning message if a non empty but invalid string was passed
    if ( !tzid.isEmpty() && !tzid.isNull() && valid == false ) {
        // Ignore additional messages:
        // If we were invalid because we have a country but not a city (e.g. 'Country/'): 
        if ( tzid.count('/') == 1 && tzid.indexOf('/') == tzid.size() - 1 ) {
                QTest::ignoreMessage(QtWarningMsg, "invalid data size = 0");
        }
        // Otherwise, we were invalid because file couldn't be found in /usr/share/zoneinfo
        else {
                QString msg = QString("Unable to open '%1%2'").arg(zonePath()).arg(tzid);
                QTest::ignoreMessage(QtWarningMsg, msg.toLatin1().constData());
        }
        QString msg = QString("QTimeZone::data Can't create a valid data object for '%1'").arg(tzid);
        QTest::ignoreMessage(QtWarningMsg, msg.toLatin1().constData());
    }

    QTimeZone tz(tzid.toLatin1().constData());

    QCOMPARE(tz.isValid(), valid);
}

/*?
    Data for utcDateTime() test function.
*/
void tst_QTimeZone::utcDateTime_data()
{
    QTest::addColumn<QString>("tzId");
    QTest::addColumn<int>("hourOffset");

    QTest::newRow("Brisbane: 10 hour offset") << "Australia/Brisbane" << 10;
    QTest::newRow("UTC: 0 hour offset") << "Africa/Abidjan" << 0;
}

/*?
    Test function for QTimeZone::utcDateTime().
    Retrieves current time in UTC, converts to local time in a specified timezone,
    and verifies that the difference in hours matches an expected value.
*/
void tst_QTimeZone::utcDateTime()
{
    QFETCH(QString, tzId);
    QFETCH(int, hourOffset);

    QDateTime utc = QTimeZone::utcDateTime();

    QTimeZone tz(tzId.toLatin1().constData());
    QDateTime local = tz.fromUtc(utc);

    QCOMPARE( utc.secsTo(local) / 3600, hourOffset );
}

/*?
    Data for locationInformation test function.
*/
void tst_QTimeZone::locationInformation_data()
{
    QTest::addColumn<QString>("tzId");
    QTest::addColumn<QString>("countryCode");
    QTest::addColumn<int>("lat");
    QTest::addColumn<int>("lon");
    QTest::addColumn<QString>("area");
    QTest::addColumn<QString>("city");
    QTest::addColumn<QString>("description");

    // AQ -690022+0393524 Antarctica/Syowa Syowa Station, E Ongul I
    QTest::newRow("deg, min, sec") << "Antarctica/Syowa"
        << "AQ"
        << (int) -248422 << (int) 142524
        << "Antarctica" << "Syowa"
        << "Syowa Station, E Ongul I";

    // AU -2728+15302	Australia/Brisbane	Queensland - most locations
    QTest::newRow("deg, min") << "Australia/Brisbane"
        << "AU"
        << (int) -98880 << (int) 550920
        << "Australia" << "Brisbane"
        << "Queensland - most locations";

    // US +470659-1011757 America/North_Dakota/Center	Central Time - North Dakota - Oliver County
    QTest::newRow("country/state with _")
        << "America/North_Dakota/Center"
        << "US"
        << (int) 169619 << (int) -364677
        << "America/North Dakota" << "Center"
        << "Central Time - North Dakota - Oliver County";
}

/*?
    Test function for QTimeZone location attributes.
    Constructs a timezone for a specified zone and verifies that all attributes match
    expected values.
*/
void tst_QTimeZone::locationInformation()
{
    QFETCH(QString, tzId);
    QFETCH(QString, countryCode);
    QFETCH(int, lat);
    QFETCH(int, lon);
    QFETCH(QString, area);
    QFETCH(QString, city);
    QFETCH(QString, description);

    QTimeZone tz(tzId.toLatin1().constData());
    QCOMPARE(tz.countryCode(), countryCode);
    QCOMPARE(tz.latitude(), lat);
    QCOMPARE(tz.longitude(), lon);
    QCOMPARE(tz.area(), area);
    QCOMPARE(tz.city(), city);
    QCOMPARE(tz.description(), description);
    QCOMPARE(tz.id(), tzId);
}

/*?
    Test conversion from a number of minutes east of GMT.
*/
void tst_QTimeZone::findFromMinutesEast()
{
    QFETCH(QDateTime, dt);
    QFETCH(int, mineast);
    QFETCH(bool, isdst);
    QFETCH(QString, tzId);
    QFETCH(QString, tzName);

    QTimeZone tz = QTimeZone::findFromMinutesEast(dt,mineast,isdst);
    QCOMPARE(tz.id(), tzId);
    QCOMPARE(tz.name(), tzName);
}

void tst_QTimeZone::findFromMinutesEast_data()
{
    QTest::addColumn<QDateTime>("dt");
    QTest::addColumn<int>("mineast");
    QTest::addColumn<bool>("isdst");
    QTest::addColumn<QString>("tzId");
    QTest::addColumn<QString>("tzName");

    QTest::newRow("Brisbane, GMT +10 hours")
        << QDateTime( QDate(2003, 1, 24), QTime(8, 0, 0) )
        << 60*10 << false
        << "Etc/GMT-10"
        << "GMT +10";

    QTest::newRow("Sydney, GMT +10 hours, Winter")
        << QDateTime( QDate(2003, 6, 24), QTime(8, 0, 0) )
        << 60*10 << false
        << "Etc/GMT-10"
        << "GMT +10";

    QTest::newRow("Sydney, GMT +9 hours, Summer")
        << QDateTime( QDate(2003, 1, 24), QTime(8, 0, 0) )
        << 60*9 << true
        << "Etc/GMT-9"
        << "GMT +9";

    // Non-whole-hour timezones...

    QTest::newRow("Adelaide, GMT +9:30 hours, Winter")
        << QDateTime( QDate(2003, 6, 24), QTime(8, 0, 0) )
        << 60*9+30 << false
        << "Australia/Adelaide"
        << "Adelaide";

    QTest::newRow("Adelaide, GMT +10:30 hours, Summer")
        << QDateTime( QDate(2003, 1, 24), QTime(8, 0, 0) )
        << 60*10+30<< true
        << "Australia/Adelaide"
        << "Adelaide";
}


/*?
    Test for default constructor.
    Tests that default QTimeZone constructor constructs an invalid timezone.
*/
void tst_QTimeZone::defaultCtor()
{
    QTimeZone tz;

    QVERIFY( !tz.isValid() );
}
