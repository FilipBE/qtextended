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

#include <QtopiaApplication>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include "qtopiabase/qabstractipcinterface.h"

//TESTED_CLASS=QAbstractIPCInterface
//TESTED_FILES=src/libraries/qtopiabase/qabstractipcinterface.h

/*
    The tst_QAbstractIPCInterface class provides unit tests for the QAbstractIPCInterface class.
*/
class tst_QAbstractIPCInterface : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void tst_foo_data();
    void tst_foo();
};

QTEST_APP_MAIN(tst_QAbstractIPCInterface, QtopiaApplication)
#include "tst_qabstractipcinterface.moc"

/*?
    Initialisation prior to the first test.
*/
void tst_QAbstractIPCInterface::initTestCase()
{
}

/*?
    Cleanup after last test.
*/
void tst_QAbstractIPCInterface::cleanupTestCase()
{
}

/*?
    Initialisation before each test.
*/
void tst_QAbstractIPCInterface::init()
{
}

/*?
    Cleanup after each test.
*/
void tst_QAbstractIPCInterface::cleanup()
{
}

/*?
    Test data function for ....
*/
void tst_QAbstractIPCInterface::tst_foo_data()
{
    // Define the test elements we're going to use
    QTest::addColumn<QString>("data1");
    QTest::addColumn<int>("data2");
    QTest::addColumn<QString>("expected");

    // Define each set of values we're going to use
    QTest::newRow("data0") << "Attention!" << 0 << "This test is not implemented yet!";
}

/*?
    Test function for ....
*/
void tst_QAbstractIPCInterface::tst_foo()
{
    // Fetch test data
    QFETCH(QString, data1);
    QFETCH(int, data2);
    QFETCH(QString, expected);

    // Perform test actions

    // Verify test result
    QEXPECT_FAIL("", "This test isn't implemented", Abort);
    QCOMPARE(QString(data1), expected);
}
