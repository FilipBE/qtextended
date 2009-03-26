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

#include <QCallListItem>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QtopiaApplication>


//TESTED_CLASS=QCallListItem
//TESTED_FILES=src/libraries/qtopiaphone/qcalllist.h

/*
    The tst_QCallListItem class provides unit tests for the QCallListItem class.
*/
class tst_QCallListItem : public QObject
{
Q_OBJECT
private slots:
    void ctor();
    void ctor_data();
};

QTEST_APP_MAIN( tst_QCallListItem, QtopiaApplication )
#include "tst_qcalllistitem.moc"

/*?
    Data for ctor() test function.
*/
void tst_QCallListItem::ctor_data()
{
    QTest::addColumn<QString>("number");
    QTest::addColumn<QUniqueId>("contact");
    QTest::addColumn<QDateTime>("start");
    QTest::addColumn<QDateTime>("end");
    QTest::addColumn<int>("type");

    // typical data
    QTest::newRow("simple")
        << "0"
        << QUniqueId()
        << QDateTime::currentDateTime()
        << QDateTime::currentDateTime().addSecs(60*5)
        << (int)QCallListItem::Dialed;

    // A null phone number
    QTest::newRow("null number")
        << QString()
        << QUniqueId()
        << QDateTime::currentDateTime()
        << QDateTime::currentDateTime().addSecs(60*5)
        << (int)QCallListItem::Dialed;

    // Dates the wrong way around (call ends before it begins).
    // Currently QCallListItem allows this with no complaint.
    QTest::newRow("bad dates")
        << "1234"
        << QUniqueId()
        << QDateTime::currentDateTime().addSecs(60*5)
        << QDateTime::currentDateTime()
        << (int)QCallListItem::Received;

    // Null date start.
    // Currently QCallListItem allows this with no complaint.
    QTest::newRow("null dates 1")
        << "1234"
        << QUniqueId()
        << QDateTime()
        << QDateTime::currentDateTime()
        << (int)QCallListItem::Received;

    // Null date end.
    // Currently QCallListItem allows this with no complaint.
    QTest::newRow("null dates 2")
        << "1234"
        << QUniqueId()
        << QDateTime::currentDateTime()
        << QDateTime()
        << (int)QCallListItem::Missed;

    // Non-null unique id, constructed with QByteArray
    QTest::newRow("unique id 1")
        << "1234"
        << QUniqueId(QByteArray("foobar"))
        << QDateTime::currentDateTime()
        << QDateTime::currentDateTime().addSecs(5*60)
        << (int)QCallListItem::Missed;

    // Non-null unique id, constructed with QString
    QTest::newRow("unique id 2")
        << "1234"
        << QUniqueId(QString("foobar"))
        << QDateTime::currentDateTime()
        << QDateTime::currentDateTime().addSecs(5*60)
        << (int)QCallListItem::Dialed;
}

/*?
    Test function for QCallListItem constructor.
    This test:
        * Constructs a QCallListItem using the full constructor and checks that all
          parameters are set to the correct values.
        * Constructs a QCallListItem using the copy constructor, creating a copy of the
          first item, and checks that all parameters of the copy are set to the correct
          values.
        * Constructs a QCallListItem using the empty constructor and verifies that
          the item is a null QCallListItem.
*/
void tst_QCallListItem::ctor()
{
    QFETCH(QString, number);
    QFETCH(QUniqueId, contact);
    QFETCH(QDateTime, start);
    QFETCH(QDateTime, end);
    QFETCH(int, type);

    QCallListItem ctorItem((QCallListItem::CallType)type, number, start, end, contact);

    QVERIFY( !ctorItem.isNull() );

    QCOMPARE(ctorItem.type(), (QCallListItem::CallType)type);
    QCOMPARE(ctorItem.number(), number);
    QCOMPARE(ctorItem.start(), start);
    QCOMPARE(ctorItem.end(), end);
    QCOMPARE(ctorItem.contact(), contact);

    QCallListItem copy(ctorItem);
    QCOMPARE(copy, ctorItem);
    QCOMPARE(copy.number(), number);
    QCOMPARE(copy.start(), start);
    QCOMPARE(copy.end(), end);
    QCOMPARE(copy.contact(), contact);

    QCallListItem nullItem;
    QVERIFY(nullItem.isNull());
}
