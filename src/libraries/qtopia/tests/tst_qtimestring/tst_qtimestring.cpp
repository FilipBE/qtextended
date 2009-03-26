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
#include <QtopiaApplication>
#include <qtopianamespace.h>
#include <QTimeString>
#include <QLocale>
#include <QSettings>
#include <QtopiaIpcEnvelope>

// Need this to use the type with QFETCH
Q_DECLARE_METATYPE(QTimeString::Length);



//TESTED_CLASS=QTimeString
//TESTED_FILES=src/libraries/qtopia/qtimestring.h

/*
    The tst_QTimeString class provides unit tests for the QTimeString class.
*/
class tst_QTimeString : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void nameOfWeekDay_data();
    void nameOfWeekDay();
    void nameOfMonth_data();
    void nameOfMonth();
    void numberDateString_data();
    void numberDateString();

    void currentFormat();
    void formatOptions();

    void localH_data();
    void localH();
    void localHM_data();
    void localHM();
    void localHMS_data();
    void localHMS();
    void localHMDayOfWeek_data();
    void localHMDayOfWeek();
    void localHMSDayOfWeek_data();
    void localHMSDayOfWeek();
    void localMD_data();
    void localMD();
    void localYMD_data();
    void localYMD();
    void localYMDHMS_data();
    void localYMDHMS();
    void localDayOfWeek_data();
    void localDayOfWeek();

    void currentAMPM();

private:
    void clearSettings();
    void setDateFormat(const QString &format);
    void setAMPM(bool useAMPM);
};

QTEST_APP_MAIN(tst_QTimeString, QtopiaApplication)
#include "tst_qtimestring.moc"


/*?
    Initialize the unit test. This gets called before any of the testcases are executed.
    Calls clearSettings() to clear date/time settings.
*/
void tst_QTimeString::initTestCase()
{
    clearSettings();
}

/*?
    Cleanup the unit test. This gets called after all the testcases have executed.
    Calls clearSettings() to clear date/time settings.
*/
void tst_QTimeString::cleanupTestCase()
{
    clearSettings();
}

/*?
    Cleanup the testcase. This gets called after each testcase.
    Calls clearSettings() to clear date/time settings.
*/
void tst_QTimeString::cleanup()
{
    clearSettings();
}


/*?
    Helper function which clears any saved date/time settings.
    Specifically, removes the "Date" and "Time" groups
    from Trolltech.qpe settings and forces QTimeString to reread config files.
*/
void tst_QTimeString::clearSettings()
{
    {
        QSettings cfg( "Trolltech", "qpe" );
        cfg.remove( "Date" );
        cfg.remove( "Time" );
    }

    // Send a setDateFormat() msg so QTimeString knows to reinit
    {
        QtopiaIpcEnvelope setDateFormat( "QPE/System", "setDateFormat()" );
    }

    // Msg wont get sent until next event loop
    qApp->processEvents();
}

/*?
    Helper function which sets the date format and notifies QPE.
    Sets Date/DateFormat key in Trolltech.qpe to the specified format
    and forces QTimeString to reload config files.
*/
void tst_QTimeString::setDateFormat(const QString &format)
{
    {
        QSettings config( "Trolltech", "qpe" );
        config.setValue( "Date/DateFormat", format );
    }

    // Send a setDateFormat() msg so QTimeString knows to reinit
    {
        QtopiaIpcEnvelope setDateFormat( "QPE/System", "setDateFormat()" );
    }

    // Msg wont get sent until next event loop
    qApp->processEvents();
}

/*?
    Helper function which sets the status of the QTimeString AMPM flag.
    Sets Time/AMPM key in Trolltech.qpe to the specified format
    and forces QTimeString to reload config files.
*/
void tst_QTimeString::setAMPM(bool useAMPM)
{
    {
        QSettings config( "Trolltech", "qpe" );
        config.setValue( "Time/AMPM", useAMPM );
    }

    // Send a clockChange(bool) msg so QTimeString knows to reinit
    {
        QtopiaIpcEnvelope setClock( "QPE/System", "clockChange(bool)" );
        setClock << useAMPM;
    }

    // Msg wont get sent until next event loop
    qApp->processEvents();
}


/*?
    Data for nameOfWeekDay test.
*/
void tst_QTimeString::nameOfWeekDay_data()
{
    QTest::addColumn<int>("day");
    QTest::addColumn<QString>("expectedLong");
    QTest::addColumn<QString>("expectedMed");
    QTest::addColumn<QString>("expectedShort");

    // Check valid days
    QTest::newRow("Day 1") << 1 << "Monday" << "Mon" << "M";
    QTest::newRow("Day 2") << 2 << "Tuesday" << "Tue" << "T";
    QTest::newRow("Day 3") << 3 << "Wednesday" << "Wed" << "W";
    QTest::newRow("Day 4") << 4 << "Thursday" << "Thu" << "T";
    QTest::newRow("Day 5") << 5 << "Friday" << "Fri" << "F";
    QTest::newRow("Day 6") << 6 << "Saturday" << "Sat" << "S";
    QTest::newRow("Day 7") << 7 << "Sunday" << "Sun" << "S";

    // Check out of bound values - these get treated as 1 with a warning msg
    QTest::newRow("Out of bounds - Day 0") << 0 << "Monday" << "Mon" << "M";
    QTest::newRow("Out of bounds - Day 8") << 8 << "Monday" << "Mon" << "M";
    QTest::newRow("Out of bounds - Day -1") << -1 << "Monday" << "Mon" << "M";
}

/*?
    Test for the QTimeString::nameOfWeekDay function.
    This test:
        * calls QTimeString::nameOfWeekDay for a specific day once for each of Long, Medium
          and Short, and checks the resulting names against expected values.
        * Checks that QTimeString::nameOfWeekDay without a specified length returns the same
          as specifying Medium length.
*/
void tst_QTimeString::nameOfWeekDay()
{
    QFETCH(int, day);

    // If the day is out of bounds then we should get a warning for each call we make
    if (day < 1 || day > 7) {
        QTest::ignoreMessage(QtWarningMsg, "QTimeString::nameOfWeekDay: not a valid day");
        QTest::ignoreMessage(QtWarningMsg, "QTimeString::nameOfWeekDay: not a valid day");
        QTest::ignoreMessage(QtWarningMsg, "QTimeString::nameOfWeekDay: not a valid day");
        QTest::ignoreMessage(QtWarningMsg, "QTimeString::nameOfWeekDay: not a valid day");
    }

    // Check the week day against each format length
    QTEST(QTimeString::nameOfWeekDay(day, QTimeString::Long), "expectedLong");
    QTEST(QTimeString::nameOfWeekDay(day, QTimeString::Medium), "expectedMed");
    QTEST(QTimeString::nameOfWeekDay(day, QTimeString::Short), "expectedShort");

    // Default format length should be medium
    QTEST(QTimeString::nameOfWeekDay(day), "expectedMed");
}


/*?
    Test data for nameOfMonth test.
*/
void tst_QTimeString::nameOfMonth_data()
{
    QTest::addColumn<int>("month");
    QTest::addColumn<QString>("expectedLong");
    QTest::addColumn<QString>("expectedMed");
    QTest::addColumn<QString>("expectedShort");

    // Check valid months
    QTest::newRow("Month 1") << 1 << "January" << "Jan" << "Jan";
    QTest::newRow("Month 2") << 2 << "February" << "Feb" << "Feb";
    QTest::newRow("Month 3") << 3 << "March" << "Mar" << "Mar";
    QTest::newRow("Month 4") << 4 << "April" << "Apr" << "Apr";
    QTest::newRow("Month 5") << 5 << "May" << "May" << "May";
    QTest::newRow("Month 6") << 6 << "June" << "Jun" << "Jun";
    QTest::newRow("Month 7") << 7 << "July" << "Jul" << "Jul";
    QTest::newRow("Month 8") << 8 << "August" << "Aug" << "Aug";
    QTest::newRow("Month 9") << 9 << "September" << "Sep" << "Sep";
    QTest::newRow("Month 10") << 10 << "October" << "Oct" << "Oct";
    QTest::newRow("Month 11") << 11 << "November" << "Nov" << "Nov";
    QTest::newRow("Month 12") << 12 << "December" << "Dec" << "Dec";

    // Check out of bound values - these get treated as 1 with a warning msg
    QTest::newRow("Out of bounds - Month 0") << 0 << "January" << "Jan" << "Jan";
    QTest::newRow("Out of bounds - Month 13") << 13 << "January" << "Jan" << "Jan";
    QTest::newRow("Out of bounds - Month -1") << -1 << "January" << "Jan" << "Jan";
}

/*?
    Test for QTimeString::nameOfMonth function.
    This test:
        * calls QTimeString::nameOfMonth for a specific month once for each of Long, Medium
          and Short, and checks the resulting names against expected values.
        * Checks that QTimeString::nameOfMonth without a specified length returns the same
          as specifying Medium length.
*/
void tst_QTimeString::nameOfMonth()
{
    QFETCH(int, month);

    // If the month is out of bounds then we should get a warning for each call we make
    if (month < 1 || month > 12) {
        QTest::ignoreMessage(QtWarningMsg, "QTimeString::nameOfMonth: not a valid month");
        QTest::ignoreMessage(QtWarningMsg, "QTimeString::nameOfMonth: not a valid month");
        QTest::ignoreMessage(QtWarningMsg, "QTimeString::nameOfMonth: not a valid month");
        QTest::ignoreMessage(QtWarningMsg, "QTimeString::nameOfMonth: not a valid month");
    }

    QTEST(QTimeString::nameOfMonth(month, QTimeString::Long), "expectedLong");
    QTEST(QTimeString::nameOfMonth(month, QTimeString::Medium), "expectedMed");
    QTEST(QTimeString::nameOfMonth(month, QTimeString::Short), "expectedShort");

    // Default format length should be medium
    QTEST(QTimeString::nameOfMonth(month), "expectedMed");
}


/*?
    Test data for numberDateString test.
*/
void tst_QTimeString::numberDateString_data()
{
    QTest::addColumn<QDate>("date");
    QTest::addColumn<QString>("format");
    QTest::addColumn<QString>("expectedLong");
    QTest::addColumn<QString>("expectedMed");
    QTest::addColumn<QString>("expectedShort");

    // Tests where day and month will be zero padded
    QTest::newRow("zero padding 1")
        << QDate(2000,  1,  1)
        << "%D/%M/%Y"
        << "01/01/2000"
        << "01/01/00"
        << "1/1/00";
    QTest::newRow("zero padding 2")
        << QDate(2001,  2,  2)
        << "%D/%M/%Y"
        << "02/02/2001"
        << "02/02/01"
        << "2/2/01";

    // Tests where day only will be zero padded
    QTest::newRow("day zero padding 1")
        << QDate(1998,  11,  5)
        << "%D/%M/%Y"
        << "05/11/1998"
        << "05/11/98"
        << "5/11/98";
    QTest::newRow("day zero padding 2")
        << QDate(2001,  12,  9)
        << "%D/%M/%Y"
        << "09/12/2001"
        << "09/12/01"
        << "9/12/01";


    // Tests where month only will be zero padded
    QTest::newRow("month zero padding 1")
        << QDate(1998,  5,  11)
        << "%D/%M/%Y"
        << "11/05/1998"
        << "11/05/98"
        << "11/5/98";
    QTest::newRow("month zero padding 2")
        << QDate(2001,  9,  26)
        << "%D/%M/%Y"
        << "26/09/2001"
        << "26/09/01"
        << "26/9/01";

    // Tests where no part of the date will be zero padded
    QTest::newRow("no zero padding 1")
        << QDate(2010, 10, 10)
        << "%D/%M/%Y"
        << "10/10/2010"
        << "10/10/10"
        << "10/10/10";
    QTest::newRow("no zero padding 2")
        << QDate(1988, 12, 30)
        << "%D/%M/%Y"
        << "30/12/1988"
        << "30/12/88"
        << "30/12/88";

    // Test 20xx and 19xx 2 digit abbrev
    QTest::newRow("20xx")
        << QDate(2021, 1, 1)
        << "%D/%M/%Y"
        << "01/01/2021"
        << "01/01/21"
        << "1/1/21";
    QTest::newRow("19xx")
        << QDate(1921, 1, 1)
        << "%D/%M/%Y"
        << "01/01/1921"
        << "01/01/21"
        << "1/1/21";

    // Test 2 digit abbrev of extreme years
    QTest::newRow("year 4145")
        << QDate(4145, 1, 1)
        << "%D/%M/%Y"
        << "01/01/4145"
        << "01/01/45"
        << "1/1/45";
    QTest::newRow("year 123")
        << QDate(123, 1, 1)
        << "%D/%M/%Y"
        << "01/01/0123"
        << "01/01/23"
        << "1/1/23";
}

/*?
    Test for QTimeString::numberDateString.
    This test:
        * calls QTimeString::numberDateString for a specific date and date format once for each
          of Long, Medium and Short, and checks the resulting strings against expected values.
        * Checks that QTimeString::numberDateString without a specified length returns the same
          as specifying Medium length.
*/
void tst_QTimeString::numberDateString()
{
    QFETCH(QDate, date);
    QFETCH(QString, format);

    // numberDateString()'s formatting is dependent on the current date format
    // So set the format to match what the data function expects
    setDateFormat(format);
    QCOMPARE( QTimeString::currentFormat(), format.remove('%') );

    QTEST(QTimeString::numberDateString(date, QTimeString::Long), "expectedLong");
    QTEST(QTimeString::numberDateString(date, QTimeString::Medium), "expectedMed");
    QTEST(QTimeString::numberDateString(date, QTimeString::Short), "expectedShort");

    // Default format length should be medium
    QTEST(QTimeString::numberDateString(date), "expectedMed");
}


/*?
    Test for QTimeString::currentFormat() and related functionality.
    This test:
        * Tests that the default date format returned a null string.
        * Tests that the default date format used is equivalent to the sort version of QLocale::dateFormat.
        * Sets a date format and tests that it is returned correctly.
        * Tests that the new date format is being used correctly.
*/
void tst_QTimeString::currentFormat()
{
    // The default format should be a null string, implying that QLocale::dateFormat is used.
    QCOMPARE( QTimeString::currentFormat(), QString() );

    // Verify that it's using QLocale to calculate the format.
    // NOTE: This code should match what the QLocale format is converted into by QTimeString::numberDateString.
    // The short form is used, and day/month/year are set to d/M/yy. QLocale is only used for the actual layout.
    QLocale loc;
    QString format = loc.dateFormat( QLocale::ShortFormat );
    format.replace( QRegExp("y+"), "yy" );
    format.replace( QRegExp("M+"), "M" );
    format.replace( QRegExp("d+"), "d" );
    QDate date(1999, 1, 2);
    QCOMPARE( QTimeString::numberDateString(date, QTimeString::Short), date.toString(format) );

    // Now change the format, and check that currentFormat matches the new format (%'s get stripped out)
    setDateFormat("%D/%M/%Y");
    QCOMPARE( QTimeString::currentFormat(), QString("D/M/Y") );

    // QTimeString::numberDateString should also now use it
    QCOMPARE( QTimeString::numberDateString(date, QTimeString::Short), QString("2/1/99") );
}


/*?
    Tests QTimeString::formatOptions().
    Since it (for now) just returns a hardcoded list of options, we'll just check that the returned list isn't empty.
*/
void tst_QTimeString::formatOptions()
{
    QVERIFY( !QTimeString::formatOptions().empty() );
}


/*?
    Test data for localH test.
*/
void tst_QTimeString::localH_data()
{
    QTest::addColumn<int>("hour");
    QTest::addColumn<QString>("expected12");
    QTest::addColumn<QString>("expected24");

    // Check all valid values
    QTest::newRow("12hr: 0") << 0 << "12am" << "00";
    for (int i = 1; i < 10; ++i) {
        QTest::newRow(QString("12hr: %1").arg(i).toLatin1())
            << i
            << QString("%1am").arg(i)
            << QString("0%1").arg(i);
    }
    QTest::newRow("12hr: 10") << 10 << "10am" << "10";
    QTest::newRow("12hr: 11") << 11 << "11am" << "11";
    QTest::newRow("12hr: 12") << 12 << "12pm" << "12";
    for (int i = 13; i < 24; ++i) {
        QTest::newRow(QString("12hr: %1").arg(i).toLatin1())
            << i
            << QString("%1pm").arg(i-12)
            << QString("%1").arg(i);
    }

    // Check out of bound values - these are treated as hour 0
    QTest::newRow("12hr: Out of bounds: -1") << -1 << "12am" << "00";
    QTest::newRow("12hr: Out of bounds: 24") << 24 << "12am" << "00";
    QTest::newRow("12hr: Out of bounds: 25") << 25 << "12am" << "00";
}

/*?
    Test for QTimeString::localH.
    This test calls QTimeString::localH for a specified hour in both 12 hour
    and 24 hour mode and compares the result with an expected value.
*/
void tst_QTimeString::localH()
{
    QFETCH(int, hour);

    // localH()'s formatting is dependent on the current ampm state.
    // First test 24 hour.
    setAMPM(false);
    QTEST(QTimeString::localH(hour), "expected24");

    // Set to true and test the 12hr formats
    setAMPM(true);
    QTEST(QTimeString::localH(hour), "expected12");
}


/*?
    Test data for QTimeString::localHM.
*/
void tst_QTimeString::localHM_data()
{
    QTest::addColumn<QTime>("time");
    QTest::addColumn<QString>("expectedLong12");
    QTest::addColumn<QString>("expectedMed12");
    QTest::addColumn<QString>("expectedShort12");
    QTest::addColumn<QString>("expectedLong24");
    QTest::addColumn<QString>("expectedMed24");
    QTest::addColumn<QString>("expectedShort24");

    QTest::newRow("12:00 AM") << QTime( 0,  0) << "12:00 AM" << "12:00 AM" << "12:00" << "00:00" << "0:00"  << "0:00";
    QTest::newRow("12:59 PM") << QTime(12, 59) << "12:59 PM" << "12:59 PM" << "12:59" << "12:59" << "12:59" << "12:59";
    QTest::newRow("01:01 AM") << QTime( 1,  1) << "01:01 AM" << "1:01 AM"  << "1:01"  << "01:01" << "1:01"  << "1:01";
    QTest::newRow("01:30 PM") << QTime(13, 30) << "01:30 PM" << "1:30 PM"  << "1:30"  << "13:30" << "13:30" << "13:30";
    QTest::newRow("08:08 AM") << QTime( 8,  8) << "08:08 AM" << "8:08 AM"  << "8:08"  << "08:08" << "8:08"  << "8:08";
    QTest::newRow("08:49 PM") << QTime(20, 49) << "08:49 PM" << "8:49 PM"  << "8:49"  << "20:49" << "20:49" << "20:49";
    QTest::newRow("11:11 AM") << QTime(11, 11) << "11:11 AM" << "11:11 AM" << "11:11" << "11:11" << "11:11" << "11:11";
    QTest::newRow("11:09 PM") << QTime(23,  9) << "11:09 PM" << "11:09 PM" << "11:09" << "23:09" << "23:09" << "23:09";
}

/*?
    Test for QTimeString::localHM (Hour and Minute formatting).
    This test calls QTimeString::localHM on a specified time, once with each of Long, Medium
    and Short, and both 12 hour time and 24 hour time, and compares the result with an expected
    value.
*/
void tst_QTimeString::localHM()
{
    QFETCH(QTime, time);

    // localHM()'s formatting is dependent on the current ampm state
    setAMPM(false);
    QTEST(QTimeString::localHM(time, QTimeString::Long), "expectedLong24");
    QTEST(QTimeString::localHM(time, QTimeString::Medium), "expectedMed24");
    QTEST(QTimeString::localHM(time, QTimeString::Short), "expectedShort24");

    // Default format length should be medium
    QTEST(QTimeString::localHM(time), "expectedMed24");

    // Set to true and test the 12hr formats
    setAMPM(true);
    QTEST(QTimeString::localHM(time, QTimeString::Long), "expectedLong12");
    QTEST(QTimeString::localHM(time, QTimeString::Medium), "expectedMed12");
    QTEST(QTimeString::localHM(time, QTimeString::Short), "expectedShort12");
    QTEST(QTimeString::localHM(time), "expectedMed12");
}


/*?
    Test data for localHMS.
*/
void tst_QTimeString::localHMS_data()
{
    QTest::addColumn<QTime>("time");
    QTest::addColumn<QString>("expectedLong12");
    QTest::addColumn<QString>("expectedMed12");
    QTest::addColumn<QString>("expectedShort12");
    QTest::addColumn<QString>("expectedLong24");
    QTest::addColumn<QString>("expectedMed24");
    QTest::addColumn<QString>("expectedShort24");

    QTest::newRow("12:00:00 AM") << QTime( 0,  0,  0) << "12:00:00 AM" << "12:00:00 AM" << "12:00:00" 
        << "00:00:00" << "0:00:00"  << "0:00:00";
    QTest::newRow("12:59:01 PM") << QTime(12, 59,  1) << "12:59:01 PM" << "12:59:01 PM" << "12:59:01" 
        << "12:59:01" << "12:59:01" << "12:59:01";
    QTest::newRow("01:01:10 AM") << QTime( 1,  1, 10) << "01:01:10 AM" << "1:01:10 AM"  << "1:01:10"
        << "01:01:10" << "1:01:10"  << "1:01:10";
    QTest::newRow("01:30:59 PM") << QTime(13, 30, 59) << "01:30:59 PM" << "1:30:59 PM"  << "1:30:59"
        << "13:30:59" << "13:30:59" << "13:30:59";
    QTest::newRow("08:08:11 AM") << QTime( 8,  8, 11) << "08:08:11 AM" << "8:08:11 AM"  << "8:08:11"
        << "08:08:11" << "8:08:11"  << "8:08:11";
    QTest::newRow("08:49:23 PM") << QTime(20, 49, 23) << "08:49:23 PM" << "8:49:23 PM"  << "8:49:23"
        << "20:49:23" << "20:49:23" << "20:49:23";
    QTest::newRow("11:11:09 AM") << QTime(11, 11,  9) << "11:11:09 AM" << "11:11:09 AM" << "11:11:09"
        << "11:11:09" << "11:11:09" << "11:11:09";
    QTest::newRow("11:09:30 PM") << QTime(23,  9, 30) << "11:09:30 PM" << "11:09:30 PM" << "11:09:30"
        << "23:09:30" << "23:09:30" << "23:09:30";
}

/*?
    Test for QTimeString::localHMS (Hour, Minute and Second formatting).
    This test calls QTimeString::localHMS on a specified time, once with each of Long, Medium
    and Short, and both 12 hour time and 24 hour time, and compares the result with an expected
    value.
*/
void tst_QTimeString::localHMS()
{
    QFETCH(QTime, time);

    // localHMS()'s formatting is dependent on the current ampm state
    setAMPM(false);
    QTEST(QTimeString::localHMS(time, QTimeString::Long), "expectedLong24");
    QTEST(QTimeString::localHMS(time, QTimeString::Medium), "expectedMed24");
    QTEST(QTimeString::localHMS(time, QTimeString::Short), "expectedShort24");

    // Default format length should be medium
    QTEST(QTimeString::localHMS(time), "expectedMed24");

    // Set to true and test the 12hr formats
    setAMPM(true);
    QTEST(QTimeString::localHMS(time, QTimeString::Long), "expectedLong12");
    QTEST(QTimeString::localHMS(time, QTimeString::Medium), "expectedMed12");
    QTEST(QTimeString::localHMS(time, QTimeString::Short), "expectedShort12");
    QTEST(QTimeString::localHMS(time), "expectedMed12");
}


/*?
    Test data for localHMDayOfWeek.
*/
void tst_QTimeString::localHMDayOfWeek_data()
{
    QTest::addColumn<QDateTime>("dateTime");
    QTest::addColumn<QString>("expectedLong12");
    QTest::addColumn<QString>("expectedMed12");
    QTest::addColumn<QString>("expectedShort12");
    QTest::addColumn<QString>("expectedLong24");
    QTest::addColumn<QString>("expectedMed24");
    QTest::addColumn<QString>("expectedShort24");

    // Check every day of the week
    QTest::newRow("Mon") << QDateTime(QDate(2003,11,17),QTime( 8,56,0)) 
        << QString("Monday 08:56 AM") << QString("Mon 8:56 AM") << QString("M 8:56")
        << QString("Monday 08:56") << QString("Mon 8:56") << QString("M 8:56");
    QTest::newRow("Tue") << QDateTime(QDate(2003,11,18),QTime(10,50,1)) 
        << QString("Tuesday 10:50 AM") << QString("Tue 10:50 AM") << QString("T 10:50")
        << QString("Tuesday 10:50") << QString("Tue 10:50") << QString("T 10:50");
    QTest::newRow("Wed") << QDateTime(QDate(2003,11,19),QTime(13,14,2)) 
        << QString("Wednesday 01:14 PM") << QString("Wed 1:14 PM") << QString("W 1:14")
        << QString("Wednesday 13:14") << QString("Wed 13:14") << QString("W 13:14");
    QTest::newRow("Thu") << QDateTime(QDate(2003,11,20),QTime( 9,40,3)) 
        << QString("Thursday 09:40 AM") << QString("Thu 9:40 AM") << QString("T 9:40")
        << QString("Thursday 09:40") << QString("Thu 9:40") << QString("T 9:40");
    QTest::newRow("Fri") << QDateTime(QDate(2003,12,26),QTime( 3,40,4)) 
        << QString("Friday 03:40 AM") << QString("Fri 3:40 AM") << QString("F 3:40")
        << QString("Friday 03:40") << QString("Fri 3:40") << QString("F 3:40");
    QTest::newRow("Sat") << QDateTime(QDate(2003,12,27),QTime(19,40,5)) 
        << QString("Saturday 07:40 PM") << QString("Sat 7:40 PM") << QString("S 7:40")
        << QString("Saturday 19:40") << QString("Sat 19:40") << QString("S 19:40");
    QTest::newRow("Sun") << QDateTime(QDate(2003,12,28),QTime(16,40,6)) 
        << QString("Sunday 04:40 PM") << QString("Sun 4:40 PM") << QString("S 4:40")
        << QString("Sunday 16:40") << QString("Sun 16:40") << QString("S 16:40");

    // Check millenium boundary
    QTest::newRow("") << QDateTime(QDate(1999,12,31),QTime(18,34,7)) 
        << QString("Friday 06:34 PM") << QString("Fri 6:34 PM") << QString("F 6:34")
        << QString("Friday 18:34") << QString("Fri 18:34") << QString("F 18:34");
    QTest::newRow("") << QDateTime(QDate(1999,12,31),QTime(23,59,59)) 
        << QString("Friday 11:59 PM") << QString("Fri 11:59 PM") << QString("F 11:59")
        << QString("Friday 23:59") << QString("Fri 23:59") << QString("F 23:59");
    QTest::newRow("") << QDateTime(QDate(2000,1,1),QTime(0,0,0)) 
        << QString("Saturday 12:00 AM") << QString("Sat 12:00 AM") << QString("S 12:00")
        << QString("Saturday 00:00") << QString("Sat 0:00") << QString("S 0:00");
    QTest::newRow("") << QDateTime(QDate(2000,1,1),QTime(23,59,59)) 
        << QString("Saturday 11:59 PM") << QString("Sat 11:59 PM") << QString("S 11:59")
        << QString("Saturday 23:59") << QString("Sat 23:59") << QString("S 23:59");

    // Check dates around the end of February for many years.
    QTest::newRow("") << QDateTime(QDate(1999,2,28),QTime(0,0,0)) 
        << QString("Sunday 12:00 AM") << QString("Sun 12:00 AM") << QString("S 12:00")
        << QString("Sunday 00:00") << QString("Sun 0:00") << QString("S 0:00");
    QTest::newRow("") << QDateTime(QDate(1999,3,1),QTime(0,0,0)) 
        << QString("Monday 12:00 AM") << QString("Mon 12:00 AM") << QString("M 12:00")
        << QString("Monday 00:00") << QString("Mon 0:00") << QString("M 0:00");

    // Leap year
    QTest::newRow("") << QDateTime(QDate(2000,2,28),QTime(0,0,0)) 
        << QString("Monday 12:00 AM") << QString("Mon 12:00 AM") << QString("M 12:00")
        << QString("Monday 00:00") << QString("Mon 0:00") << QString("M 0:00");
    QTest::newRow("") << QDateTime(QDate(2000,2,29),QTime(0,0,0)) 
        << QString("Tuesday 12:00 AM") << QString("Tue 12:00 AM") << QString("T 12:00")
        << QString("Tuesday 00:00") << QString("Tue 0:00") << QString("T 0:00");
    QTest::newRow("") << QDateTime(QDate(2000,3,1),QTime(0,0,0)) 
        << QString("Wednesday 12:00 AM") << QString("Wed 12:00 AM") << QString("W 12:00")
        << QString("Wednesday 00:00") << QString("Wed 0:00") << QString("W 0:00");

    QTest::newRow("") << QDateTime(QDate(2001,2,28),QTime(0,0,0)) 
        << QString("Wednesday 12:00 AM") << QString("Wed 12:00 AM") << QString("W 12:00")
        << QString("Wednesday 00:00") << QString("Wed 0:00") << QString("W 0:00");
    QTest::newRow("") << QDateTime(QDate(2001,3,1),QTime(0,0,0)) 
        << QString("Thursday 12:00 AM") << QString("Thu 12:00 AM") << QString("T 12:00")
        << QString("Thursday 00:00") << QString("Thu 0:00") << QString("T 0:00");

    QTest::newRow("") << QDateTime(QDate(2002,2,28),QTime(0,0,0)) 
        << QString("Thursday 12:00 AM") << QString("Thu 12:00 AM") << QString("T 12:00")
        << QString("Thursday 00:00") << QString("Thu 0:00") << QString("T 0:00");
    QTest::newRow("") << QDateTime(QDate(2002,3,1),QTime(0,0,0)) 
        << QString("Friday 12:00 AM") << QString("Fri 12:00 AM") << QString("F 12:00")
        << QString("Friday 00:00") << QString("Fri 0:00") << QString("F 0:00");

    QTest::newRow("") << QDateTime(QDate(2003,2,28),QTime(0,0,0)) 
        << QString("Friday 12:00 AM") << QString("Fri 12:00 AM") << QString("F 12:00")
        << QString("Friday 00:00") << QString("Fri 0:00") << QString("F 0:00");
    QTest::newRow("") << QDateTime(QDate(2003,3,1),QTime(0,0,0)) 
        << QString("Saturday 12:00 AM") << QString("Sat 12:00 AM") << QString("S 12:00")
        << QString("Saturday 00:00") << QString("Sat 0:00") << QString("S 0:00");

    // and the next leap year...
    QTest::newRow("") << QDateTime(QDate(2004,2,28),QTime(0,0,0)) 
        << QString("Saturday 12:00 AM") << QString("Sat 12:00 AM") << QString("S 12:00")
        << QString("Saturday 00:00") << QString("Sat 0:00") << QString("S 0:00");
    QTest::newRow("") << QDateTime(QDate(2004,2,29),QTime(0,0,0)) 
        << QString("Sunday 12:00 AM") << QString("Sun 12:00 AM") << QString("S 12:00")
        << QString("Sunday 00:00") << QString("Sun 0:00") << QString("S 0:00");
    QTest::newRow("") << QDateTime(QDate(2004,3,1),QTime(0,0,0)) 
        << QString("Monday 12:00 AM") << QString("Mon 12:00 AM") << QString("M 12:00")
        << QString("Monday 00:00") << QString("Mon 0:00") << QString("M 0:00");
}

/*?
    Test for QTimeString::localHMDayOfWeek (Hour, Minute and Day Of Week formatting).
    This test calls QTimeString::localHMDayOfWeek on a specified date and time, once with each
    of Long, Medium and Short, and both 12 hour time and 24 hour time, and compares the result
    with an expected value.    
*/
void tst_QTimeString::localHMDayOfWeek()
{
    QFETCH(QDateTime, dateTime);

    // localHMDayOfWeek()'s formatting is dependent on the current ampm state.
    setAMPM(false);
    QTEST(QTimeString::localHMDayOfWeek(dateTime, QTimeString::Long), "expectedLong24");
    QTEST(QTimeString::localHMDayOfWeek(dateTime, QTimeString::Medium), "expectedMed24");
    QTEST(QTimeString::localHMDayOfWeek(dateTime, QTimeString::Short), "expectedShort24");

    // Default format length should be medium
    QTEST(QTimeString::localHMDayOfWeek(dateTime), "expectedMed24");

    // Set to true and test the 12hr formats
    setAMPM(true);
    QTEST(QTimeString::localHMDayOfWeek(dateTime, QTimeString::Long), "expectedLong12");
    QTEST(QTimeString::localHMDayOfWeek(dateTime, QTimeString::Medium), "expectedMed12");
    QTEST(QTimeString::localHMDayOfWeek(dateTime, QTimeString::Short), "expectedShort12");
    QTEST(QTimeString::localHMDayOfWeek(dateTime), "expectedMed12");
}


/*?
    Test data for localHMSDayOfWeek.
*/
void tst_QTimeString::localHMSDayOfWeek_data()
{
    QTest::addColumn<QDateTime>("dateTime");
    QTest::addColumn<QString>("expectedLong12");
    QTest::addColumn<QString>("expectedMed12");
    QTest::addColumn<QString>("expectedShort12");
    QTest::addColumn<QString>("expectedLong24");
    QTest::addColumn<QString>("expectedMed24");
    QTest::addColumn<QString>("expectedShort24");

    // Check every day of the week
    QTest::newRow("Mon") << QDateTime(QDate(2003,11,17),QTime( 8,56,0)) 
        << QString("Monday 08:56:00 AM") << QString("Mon 8:56:00 AM") << QString("M 8:56:00")
        << QString("Monday 08:56:00") << QString("Mon 8:56:00") << QString("M 8:56:00");
    QTest::newRow("Tue") << QDateTime(QDate(2003,11,18),QTime(10,50,1)) 
        << QString("Tuesday 10:50:01 AM") << QString("Tue 10:50:01 AM") << QString("T 10:50:01")
        << QString("Tuesday 10:50:01") << QString("Tue 10:50:01") << QString("T 10:50:01");
    QTest::newRow("Wed") << QDateTime(QDate(2003,11,19),QTime(13,14,2)) 
        << QString("Wednesday 01:14:02 PM") << QString("Wed 1:14:02 PM") << QString("W 1:14:02")
        << QString("Wednesday 13:14:02") << QString("Wed 13:14:02") << QString("W 13:14:02");
    QTest::newRow("Thu") << QDateTime(QDate(2003,11,20),QTime( 9,40,3)) 
        << QString("Thursday 09:40:03 AM") << QString("Thu 9:40:03 AM") << QString("T 9:40:03")
        << QString("Thursday 09:40:03") << QString("Thu 9:40:03") << QString("T 9:40:03");
    QTest::newRow("Fri") << QDateTime(QDate(2003,12,26),QTime( 3,40,4)) 
        << QString("Friday 03:40:04 AM") << QString("Fri 3:40:04 AM") << QString("F 3:40:04")
        << QString("Friday 03:40:04") << QString("Fri 3:40:04") << QString("F 3:40:04");
    QTest::newRow("Sat") << QDateTime(QDate(2003,12,27),QTime(19,40,5)) 
        << QString("Saturday 07:40:05 PM") << QString("Sat 7:40:05 PM") << QString("S 7:40:05")
        << QString("Saturday 19:40:05") << QString("Sat 19:40:05") << QString("S 19:40:05");
    QTest::newRow("Sun") << QDateTime(QDate(2003,12,28),QTime(16,40,6)) 
        << QString("Sunday 04:40:06 PM") << QString("Sun 4:40:06 PM") << QString("S 4:40:06")
        << QString("Sunday 16:40:06") << QString("Sun 16:40:06") << QString("S 16:40:06");

    // Check millenium boundaries
    QTest::newRow("") << QDateTime(QDate(1999,12,31),QTime(18,34,7)) 
        << QString("Friday 06:34:07 PM") << QString("Fri 6:34:07 PM") << QString("F 6:34:07")
        << QString("Friday 18:34:07") << QString("Fri 18:34:07") << QString("F 18:34:07");
    QTest::newRow("") << QDateTime(QDate(1999,12,31),QTime(23,59,59)) 
        << QString("Friday 11:59:59 PM") << QString("Fri 11:59:59 PM") << QString("F 11:59:59")
        << QString("Friday 23:59:59") << QString("Fri 23:59:59") << QString("F 23:59:59");
    QTest::newRow("") << QDateTime(QDate(2000,1,1),QTime(0,0,0)) 
        << QString("Saturday 12:00:00 AM") << QString("Sat 12:00:00 AM") << QString("S 12:00:00")
        << QString("Saturday 00:00:00") << QString("Sat 0:00:00") << QString("S 0:00:00");
    QTest::newRow("") << QDateTime(QDate(2000,1,1),QTime(23,59,59)) 
        << QString("Saturday 11:59:59 PM") << QString("Sat 11:59:59 PM") << QString("S 11:59:59")
        << QString("Saturday 23:59:59") << QString("Sat 23:59:59") << QString("S 23:59:59");

    // Check around the end of February every year for several years.
    QTest::newRow("") << QDateTime(QDate(1999,2,28),QTime(0,0,0)) 
        << QString("Sunday 12:00:00 AM") << QString("Sun 12:00:00 AM") << QString("S 12:00:00")
        << QString("Sunday 00:00:00") << QString("Sun 0:00:00") << QString("S 0:00:00");
    QTest::newRow("") << QDateTime(QDate(1999,3,1),QTime(0,0,0)) 
        << QString("Monday 12:00:00 AM") << QString("Mon 12:00:00 AM") << QString("M 12:00:00")
        << QString("Monday 00:00:00") << QString("Mon 0:00:00") << QString("M 0:00:00");

    // Leap year
    QTest::newRow("") << QDateTime(QDate(2000,2,28),QTime(0,0,0)) 
        << QString("Monday 12:00:00 AM") << QString("Mon 12:00:00 AM") << QString("M 12:00:00")
        << QString("Monday 00:00:00") << QString("Mon 0:00:00") << QString("M 0:00:00");
    QTest::newRow("") << QDateTime(QDate(2000,3,1),QTime(0,0,0)) 
        << QString("Wednesday 12:00:00 AM") << QString("Wed 12:00:00 AM") << QString("W 12:00:00")
        << QString("Wednesday 00:00:00") << QString("Wed 0:00:00") << QString("W 0:00:00");

    QTest::newRow("") << QDateTime(QDate(2001,2,28),QTime(0,0,0)) 
        << QString("Wednesday 12:00:00 AM") << QString("Wed 12:00:00 AM") << QString("W 12:00:00")
        << QString("Wednesday 00:00:00") << QString("Wed 0:00:00") << QString("W 0:00:00");
    QTest::newRow("") << QDateTime(QDate(2001,3,1),QTime(0,0,0)) 
        << QString("Thursday 12:00:00 AM") << QString("Thu 12:00:00 AM") << QString("T 12:00:00")
        << QString("Thursday 00:00:00") << QString("Thu 0:00:00") << QString("T 0:00:00");

    QTest::newRow("") << QDateTime(QDate(2002,2,28),QTime(0,0,0)) 
        << QString("Thursday 12:00:00 AM") << QString("Thu 12:00:00 AM") << QString("T 12:00:00")
        << QString("Thursday 00:00:00") << QString("Thu 0:00:00") << QString("T 0:00:00");
    QTest::newRow("") << QDateTime(QDate(2002,3,1),QTime(0,0,0)) 
        << QString("Friday 12:00:00 AM") << QString("Fri 12:00:00 AM") << QString("F 12:00:00")
        << QString("Friday 00:00:00") << QString("Fri 0:00:00") << QString("F 0:00:00");

    QTest::newRow("") << QDateTime(QDate(2003,2,28),QTime(0,0,0)) 
        << QString("Friday 12:00:00 AM") << QString("Fri 12:00:00 AM") << QString("F 12:00:00")
        << QString("Friday 00:00:00") << QString("Fri 0:00:00") << QString("F 0:00:00");
    QTest::newRow("") << QDateTime(QDate(2003,3,1),QTime(0,0,0)) 
        << QString("Saturday 12:00:00 AM") << QString("Sat 12:00:00 AM") << QString("S 12:00:00")
        << QString("Saturday 00:00:00") << QString("Sat 0:00:00") << QString("S 0:00:00");

    // and the next leap year...
    QTest::newRow("") << QDateTime(QDate(2004,2,28),QTime(0,0,0)) 
        << QString("Saturday 12:00:00 AM") << QString("Sat 12:00:00 AM") << QString("S 12:00:00")
        << QString("Saturday 00:00:00") << QString("Sat 0:00:00") << QString("S 0:00:00");
    QTest::newRow("") << QDateTime(QDate(2004,3,1),QTime(0,0,0)) 
        << QString("Monday 12:00:00 AM") << QString("Mon 12:00:00 AM") << QString("M 12:00:00")
        << QString("Monday 00:00:00") << QString("Mon 0:00:00") << QString("M 0:00:00");
}

/*?
    Test for QTimeString::localHMSDayOfWeek (Hour, Minute, Second and Day Of Week formatting).
    This test calls QTimeString::localHMSDayOfWeek on a specified date and time, once with each
    of Long, Medium and Short, and both 12 hour time and 24 hour time, and compares the result
    with an expected value.
*/
void tst_QTimeString::localHMSDayOfWeek()
{
    QFETCH(QDateTime, dateTime);

    // localHMSDayOfWeek()'s formatting is dependent on the current ampm state.
    setAMPM(false);
    QTEST(QTimeString::localHMSDayOfWeek(dateTime, QTimeString::Long), "expectedLong24");
    QTEST(QTimeString::localHMSDayOfWeek(dateTime, QTimeString::Medium), "expectedMed24");
    QTEST(QTimeString::localHMSDayOfWeek(dateTime, QTimeString::Short), "expectedShort24");

    // Default format length should be medium
    QTEST(QTimeString::localHMSDayOfWeek(dateTime), "expectedMed24");

    // Set to true and test the 12hr formats
    setAMPM(true);
    QTEST(QTimeString::localHMSDayOfWeek(dateTime, QTimeString::Long), "expectedLong12");
    QTEST(QTimeString::localHMSDayOfWeek(dateTime, QTimeString::Medium), "expectedMed12");
    QTEST(QTimeString::localHMSDayOfWeek(dateTime, QTimeString::Short), "expectedShort12");
    QTEST(QTimeString::localHMSDayOfWeek(dateTime), "expectedMed12");
}


/*?
    Test data for localMD.
*/
void tst_QTimeString::localMD_data()
{
    QTest::addColumn<QDate>("date");
    QTest::addColumn<QString>("expectedShort");
    QTest::addColumn<QString>("expectedMed");
    QTest::addColumn<QString>("expectedLong");

    QTest::newRow( "1 January" )
        << QDate(2003,1,1) 
        << QString("1 Jan")
        << QString("01 Jan")
        << QString("01 January");

    QTest::newRow( "4 February" )
        << QDate(2003,2,4)
        << QString("4 Feb")
        << QString("04 Feb")
        << QString("04 February");

    QTest::newRow( "7 March" )
        << QDate(2003,3,7)
        << QString("7 Mar")
        << QString("07 Mar")
        << QString("07 March");

    QTest::newRow( "10 April" )
        << QDate(2003,4,10)
        << QString("10 Apr")
        << QString("10 Apr")
        << QString("10 April");

    QTest::newRow( "13 May" )
        << QDate(2003,5,13)
        << QString("13 May")
        << QString("13 May")
        << QString("13 May");

    QTest::newRow( "16 June" )
        << QDate(2003,6,16)
        << QString("16 Jun")
        << QString("16 Jun")
        << QString("16 June");


    QTest::newRow( "19 July" )
        << QDate(2003,7,19)
        << QString("19 Jul")
        << QString("19 Jul")
        << QString("19 July");


    QTest::newRow( "22 August" )
        << QDate(2003,8,22)
        << QString("22 Aug")
        << QString("22 Aug")
        << QString("22 August");

    QTest::newRow( "25 September" )
        << QDate(2003,9,25)
        << QString("25 Sep")
        << QString("25 Sep")
        << QString("25 September");

    QTest::newRow( "28 October" )
        << QDate(2003,10,28)
        << QString("28 Oct")
        << QString("28 Oct")
        << QString("28 October");

    QTest::newRow( "1 November" )
        << QDate(2003,11,1)
        << QString("1 Nov")
        << QString("01 Nov")
        << QString("01 November");

    QTest::newRow( "4 December" )
        << QDate(2003,12,4)
        << QString("4 Dec")
        << QString("04 Dec")
        << QString("04 December");
}

/*?
    Test for QTimeString::localMD() (Month and Day formatting).
    This test calls QTimeString::localMD on a specified date, once with each
    of Long, Medium and Short, and compares the result
    with an expected value.
*/
void tst_QTimeString::localMD()
{
    QFETCH(QDate, date);

    QTEST(QTimeString::localMD(date, QTimeString::Short), "expectedShort");
    QTEST(QTimeString::localMD(date, QTimeString::Medium), "expectedMed");
    QTEST(QTimeString::localMD(date, QTimeString::Long), "expectedLong");

    // default format should be Medium
    QTEST(QTimeString::localMD(date), "expectedMed");
}


/*?
    Test data for localYMD.
*/
void tst_QTimeString::localYMD_data()
{
    QTest::addColumn<QDate>("date");
    QTest::addColumn<QString>("expectedShort");
    QTest::addColumn<QString>("expectedMed");
    QTest::addColumn<QString>("expectedLong");

    // localYMD simply uses date.toString(), so that is what we'll test.

    QLocale loc;
    QString longFmt = loc.dateFormat( QLocale::LongFormat );
    QString shortFmt = loc.dateFormat( QLocale::ShortFormat );
    // Note that, for localYMD, medium and short give same results

    // Test every day for 10 years.
    QDate date(1995, 11, 10);
    for (int i = 0; i < 3650; ++i) {
        QTest::newRow(QString("%1").arg(i).toLatin1())
            << date
            << date.toString(shortFmt)
            << date.toString(shortFmt)
            << date.toString(longFmt);
        date = date.addDays(1);
    }
}

/*?
    Test for QTimeString::localYMD() (Year, Month and Day formatting).
    This test calls QTimeString::localYMD on a specified date, once with each
    of Long, Medium and Short, and compares the result
    with an expected value.
*/
void tst_QTimeString::localYMD()
{
    QFETCH(QDate, date);
    QTEST( QTimeString::localYMD(date, QTimeString::Long), "expectedLong" );
    QTEST( QTimeString::localYMD(date, QTimeString::Medium), "expectedMed" );
    QTEST( QTimeString::localYMD(date, QTimeString::Short), "expectedShort" );

    // Default length should be medium
    QTEST( QTimeString::localYMD(date), "expectedMed" );
}


/*?
    Data for localYMDHMS test.
*/
void tst_QTimeString::localYMDHMS_data()
{
    QTest::addColumn<QDateTime>("dateTime");

    // Check every day of the week
    QTest::newRow("Mon") << QDateTime(QDate(2003,11,17),QTime( 8,56,0));
    QTest::newRow("Tue") << QDateTime(QDate(2003,11,18),QTime(10,50,1));
    QTest::newRow("Wed") << QDateTime(QDate(2003,11,19),QTime(13,14,2));
    QTest::newRow("Thu") << QDateTime(QDate(2003,11,20),QTime( 9,40,3));
    QTest::newRow("Fri") << QDateTime(QDate(2003,12,26),QTime( 3,40,4));
    QTest::newRow("Sat") << QDateTime(QDate(2003,12,27),QTime(19,40,5));
    QTest::newRow("Sun") << QDateTime(QDate(2003,12,28),QTime(16,40,6));

    // Check millenium boundaries
    QTest::newRow("before 2000 1") << QDateTime(QDate(1999,12,31),QTime(18,34,7));
    QTest::newRow("before 2000 2") << QDateTime(QDate(1999,12,31),QTime(23,59,59));
    QTest::newRow("after 2000 1") << QDateTime(QDate(2000,1,1),QTime(0,0,0));
    QTest::newRow("after 2000 2") << QDateTime(QDate(2000,1,1),QTime(23,59,59));

    // Check around the end of February every year for several years.
    QTest::newRow("") << QDateTime(QDate(1999,2,28),QTime(0,0,0));
    QTest::newRow("") << QDateTime(QDate(1999,3,1),QTime(0,0,0));

    // Leap year
    QTest::newRow("") << QDateTime(QDate(2000,2,28),QTime(0,0,0));
    QTest::newRow("") << QDateTime(QDate(2000,3,1),QTime(0,0,0));

    QTest::newRow("") << QDateTime(QDate(2001,2,28),QTime(0,0,0));
    QTest::newRow("") << QDateTime(QDate(2001,3,1),QTime(0,0,0));

    QTest::newRow("") << QDateTime(QDate(2002,2,28),QTime(0,0,0));
    QTest::newRow("") << QDateTime(QDate(2002,3,1),QTime(0,0,0));

    QTest::newRow("") << QDateTime(QDate(2003,2,28),QTime(0,0,0));
    QTest::newRow("") << QDateTime(QDate(2003,3,1),QTime(0,0,0));

    // and the next leap year...
    QTest::newRow("") << QDateTime(QDate(2004,2,28),QTime(0,0,0));
    QTest::newRow("") << QDateTime(QDate(2004,3,1),QTime(0,0,0));
}

/*?
    Test for QTimeString::localYMDHMS() (Year, Month, Day, Hour, Minute and Second formatting).
    The QTimeString::localYMDHMS() function simply uses localYMD and localHMS, each of which are
    tested elsewhere.  Hence this test simply checks that localYMDHMS returns an equivalent string
    to one constructed from localYMD and localHMS.
*/
void tst_QTimeString::localYMDHMS()
{
    QFETCH(QDateTime, dateTime);
    QDate date = dateTime.date();
    QTime time = dateTime.time();

    QCOMPARE( QTimeString::localYMDHMS(dateTime, QTimeString::Long), 
        QTimeString::localYMD(date,QTimeString::Long) + " " + QTimeString::localHMS(time, QTimeString::Long) );
    QCOMPARE( QTimeString::localYMDHMS(dateTime, QTimeString::Medium), 
        QTimeString::localYMD(date,QTimeString::Medium) + " " + QTimeString::localHMS(time, QTimeString::Medium) );
    QCOMPARE( QTimeString::localYMDHMS(dateTime, QTimeString::Short), 
        QTimeString::localYMD(date,QTimeString::Short) + " " + QTimeString::localHMS(time, QTimeString::Short) );

    // Default format length should be medium
    QCOMPARE( QTimeString::localYMDHMS(dateTime), 
        QTimeString::localYMD(date,QTimeString::Medium) + " " + QTimeString::localHMS(time, QTimeString::Medium) );
}


/*?
    Data for localDayOfWeek.
*/
void tst_QTimeString::localDayOfWeek_data()
{
    QTest::addColumn<QDate>("date");
    QTest::addColumn<QString>("expectedLong");
    QTest::addColumn<QString>("expectedMed");
    QTest::addColumn<QString>("expectedShort");

    QTest::newRow("Day 1") << QDate(2006,7,17) << "Monday" << "Mon" << "M";
    QTest::newRow("Day 2") << QDate(2006,7,18) << "Tuesday" << "Tue" << "T";
    QTest::newRow("Day 3") << QDate(2006,7,19) << "Wednesday" << "Wed" << "W";
    QTest::newRow("Day 4") << QDate(2006,7,20) << "Thursday" << "Thu" << "T";
    QTest::newRow("Day 5") << QDate(2006,7,21) << "Friday" << "Fri" << "F";
    QTest::newRow("Day 6") << QDate(2006,7,22) << "Saturday" << "Sat" << "S";
    QTest::newRow("Day 7") << QDate(2006,7,23) << "Sunday" << "Sun" << "S";

    QTest::newRow("Day 1") << QDate(2006,7,31) << "Monday" << "Mon" << "M";
    QTest::newRow("Day 2") << QDate(2006,8,1) << "Tuesday" << "Tue" << "T";
}

/*?
    Test for QTimeString::localDayOfWeek().
    This test calls QTimeString::localDayOfWeek with a specified date, once for each of
    Long, Medium and Short, and compares the result against an expected value.
    It also checks that the default length is Medium.
*/
void tst_QTimeString::localDayOfWeek()
{
    QFETCH(QDate, date);
    QTEST( QTimeString::localDayOfWeek( date, QTimeString::Long ), "expectedLong" );
    QTEST( QTimeString::localDayOfWeek( date, QTimeString::Medium ), "expectedMed" );
    QTEST( QTimeString::localDayOfWeek( date, QTimeString::Short ), "expectedShort" );
    QTEST( QTimeString::localDayOfWeek( date ), "expectedMed" );
}

/*?
    Test for QTimeString::currentAMPM().
    This test toggles the use of 12 hour time several times and checks that
    the setting is correctly returned.
*/
void tst_QTimeString::currentAMPM()
{
    for (int i = 0; i < 10; ++i) {
        setAMPM(true);
        QVERIFY( QTimeString::currentAMPM() );
        setAMPM(false);
        QVERIFY( !QTimeString::currentAMPM() );
    }
}
