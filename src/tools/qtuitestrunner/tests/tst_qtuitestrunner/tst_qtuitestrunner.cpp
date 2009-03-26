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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QtCore/QtCore>
#include <QtTest/QtTest>
#include <QtTest/private/qtestresult_p.h>

//TESTED_COMPONENT=QA: Testing Framework (18707)

/*
    Tested Class: QTest
    Tested File:
*/
class tst_qtuitestrunner : public QObject
{
    Q_OBJECT

public:
    tst_qtuitestrunner();
    virtual ~tst_qtuitestrunner();

private slots:
    virtual void initTestCase();
    virtual void cleanupTestCase();
    virtual void init();
    virtual void cleanup();

    void skipTest();
    void skipTest_data();

    void skipSingleTest();
    void skipSingleTest_data();

    void skipAllTest();
    void skipAllTest_data();

    void failTest();
    void failTest_data();

    void verifyTest();
    void verifyTest_data();

    void expectFailTest();
    void expectFailTest_data();

    void unexpectedPassTest();
    void unexpectedPassTest_data();

    void stringCompareTest();
    void stringCompareTest_data();

    void boolCompareTest();
    void boolCompareTest_data();

    void intCompareTest();
    void intCompareTest_data();

    void uniqueDataTest();
    void uniqueDataTest_data();

    void nothingIsTestedTest();

    void passTest_data();
    void passTest();

private:
    int func_count;
    QString cur_function;
};

QTEST_MAIN(tst_qtuitestrunner)
#include "tst_qtuitestrunner.moc"

tst_qtuitestrunner::tst_qtuitestrunner()
{
}

tst_qtuitestrunner::~tst_qtuitestrunner()
{
}

void tst_qtuitestrunner::initTestCase()
{
    func_count = 0;
    cur_function = "";
//    QFAIL( "Intentionally entering a fail in the initTestCase phase, in which case NO testfunction should be executed." );
    if (qgetenv("QTUITEST_SELFTEST").isEmpty())
        QSKIP( "Skipping all tests.  This test intentionally fails and therefore "
               "shouldn't be run as part of an automated test system.  If you want "
               "to run this test manually, set the QTUITEST_SELFTEST environment "
               "variable.", SkipAll );
}

void tst_qtuitestrunner::cleanupTestCase()
{
//    QFAIL( "Intentionally entering a fail in the cleanup phase" );
}

void tst_qtuitestrunner::init()
{
    if (cur_function != QTestResult::currentTestFunction())
        func_count = 0;
    func_count++;
}

void tst_qtuitestrunner::cleanup()
{
}

void tst_qtuitestrunner::skipTest_data()
{
    QTest::addColumn<bool>("skipSingle");

    QTest::newRow("skip_single") << bool(true);
    QTest::newRow("skip_all") << bool(false);
    QTest::newRow("skip_notexecuted") << bool(true);
}

/*!
    \usecase Any tests following a SkipAll should not be executed and the test as a whole should not result in a PASS
*/
void tst_qtuitestrunner::skipTest()
{
    QFETCH( bool, skipSingle );
    if (QTestResult::currentDataTag() == "skip_notexecuted") QFAIL( "This test should not be executed" );

    if (skipSingle) {
        QSKIP( "This is a skip single test and the next dataset should be executed as well.", SkipSingle );
    } else {
        QSKIP( "This is a skip all test and the next dataset should not be executed anymore.", SkipAll );
    }

    QFAIL( "You should not see this message" );
    // Totals (5): 2 pass, 0 fail, 3 skip
}

/*!
    \usecase A single test using a SkipSingle should not result in a PASS
*/
void tst_qtuitestrunner::skipSingleTest_data()
{
}

void tst_qtuitestrunner::skipSingleTest()
{
    QSKIP( "This is a skip single test and should not result in a PASS", SkipSingle );
    QFAIL( "This message should not be visible" );
    // Totals (6): 2 pass, 0 fail, 4 skip
}

/*!
    \usecase A single test using a SkipAll should not result in a PASS
*/
void tst_qtuitestrunner::skipAllTest_data()
{
    QTest::addColumn<bool>("skipAll");

    QTest::newRow("skip1") << bool(true);
    QTest::newRow("skip2") << bool(false);
    QTest::newRow("skip3") << bool(false);
}

void tst_qtuitestrunner::skipAllTest()
{
    QFETCH( bool, skipAll );
    QSKIP( "This is a skip all test and should not result in a PASS", SkipAll );
    QFAIL( "This message should not be visible" );
    // Totals (9): 2 pass, 0 fail, 7 skip
}

/*!
    \usecase A test that fails should give a failure message and should abort immediately
*/
void tst_qtuitestrunner::failTest_data()
{
}

void tst_qtuitestrunner::failTest()
{
    QFAIL( "This test is intended to FAIL" );
    qDebug( "You should NOT see this text" );
    // Totals (10): 2 pass, 1 fail, 7 skip
}

/*!
    \usecase boolean statements can be verified and result in a FAIL if evaluated to false
*/
void tst_qtuitestrunner::verifyTest_data()
{
}

void tst_qtuitestrunner::verifyTest()
{
    QVERIFY2( false, "This test is intended to fail" );
    // Totals (11): 2 pass, 2 fail, 7 skip
}

/*!
    \usecase A test can be expected to fail, in which case the test should result in an XFAIL
*/
void tst_qtuitestrunner::expectFailTest_data()
{
}

void tst_qtuitestrunner::expectFailTest()
{
    QEXPECT_FAIL( "", "Bug #00001", Abort );
    bool statement = false;
    QVERIFY2( statement, "This test is expected to fail" );
    QFAIL( "You should NOT see this line" );
    // Totals (12): 2 pass, 3 fail, 7 skip
}

/*!
    \usecase A test can be marked as 'expected to fail' but can unexpectedly pass
*/
void tst_qtuitestrunner::unexpectedPassTest_data()
{
}

void tst_qtuitestrunner::unexpectedPassTest()
{
    QEXPECT_FAIL( "", "Bug #00002", Abort );
    bool statement = true;
    QVERIFY2( statement, "This should pass unexpectedly" );
    // Totals (13): 2 pass, 4 fail, 7 skip
}

/*!
    \usecase Two string values can be QCOMPAREd with each other. If the values are different a meaningfull failure message wil be given.
*/
void tst_qtuitestrunner::stringCompareTest_data()
{
    QTest::addColumn<QString>("actual");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<bool>("shouldFail");

    QTest::newRow("passTest") << QString("foobar") << QString("foobar") << bool(false);
    QTest::newRow("failTest") << QString("foobar") << QString("barfoo") << bool(true);
}

void tst_qtuitestrunner::stringCompareTest()
{
    QFETCH( QString, actual );
    QFETCH( QString, expected );
    QFETCH( bool, shouldFail );

    QCOMPARE( actual, expected );
    // Totals (15): 3 pass, 5 fail, 7 skip
}

/*!
    \usecase Two bool values can be QCOMPAREd with each other. If the values are different a meaningfull failure message wil be given.
*/
void tst_qtuitestrunner::boolCompareTest_data()
{
    QTest::addColumn<bool>("actual");
    QTest::addColumn<bool>("expected");
    QTest::addColumn<bool>("shouldFail");
    QTest::newRow("bothTruePassTest") << bool(true) << bool(true) << bool(false);
    QTest::newRow("failTest") << bool(true) << bool(false) << bool(true);
    QTest::newRow("bothFalsePassTest") << bool(false) << bool(false) << bool(false);
}

void tst_qtuitestrunner::boolCompareTest()
{
    QFETCH( bool, actual );
    QFETCH( bool, expected );
    QFETCH( bool, shouldFail );
    QCOMPARE( actual, expected );
    // Totals (18): 5 pass, 6 fail, 7 skip
}

/*!
    \usecase Two int values can be QCOMPAREd with each other. If the values are different a meaningfull failure message wil be given.
*/
void tst_qtuitestrunner::intCompareTest_data()
{
    QTest::addColumn<int>("actual");
    QTest::addColumn<int>("expected");
    QTest::addColumn<bool>("shouldFail");
    QTest::newRow("equalTest") << 1 << 1 << bool(false);
    QTest::newRow("unEqualTest") << 2 << 3 << bool(true);
}

void tst_qtuitestrunner::intCompareTest()
{
    QFETCH( int, actual );
    QFETCH( int, expected );
    QFETCH( bool, shouldFail );
    QCOMPARE( actual, expected );
    // Totals (20): 7 pass, 7 fail, 7 skip
}

/*!
    \usecase datatags must be unique
*/
void tst_qtuitestrunner::uniqueDataTest_data()
{
    QTest::addColumn<int>("dummy");
    QTest::newRow("tagtest") << 1;
    QTest::newRow("tagtest") << 2;
}

void tst_qtuitestrunner::uniqueDataTest()
{
    QVERIFY(true);
    if (func_count > 1)
        QFAIL( "We should never get here, because the data creation is invalid" );
    // Totals (21): 7 pass, 8 fail, 7 skip
}

void tst_qtuitestrunner::nothingIsTestedTest()
{
    // Since nothing is done, this test should fail with a 'Nothing is tested' message
    // Totals (22): 7 pass, 9 fail, 7 skip
}

void tst_qtuitestrunner::passTest_data()
{
    QTest::addColumn<int>("dummy");
    QTest::newRow("test1") << 1;
    QTest::newRow("test2") << 2;
}
void tst_qtuitestrunner::passTest()
{
    QVERIFY( true );
    // Totals (24): 8 pass, 9 fail, 7 skip
}

