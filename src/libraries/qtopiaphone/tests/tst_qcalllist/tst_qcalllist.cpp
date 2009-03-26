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

#include <QCallList>

#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QtopiaApplication>



//TESTED_CLASS=QCallList
//TESTED_FILES=src/libraries/qtopiaphone/qcalllist.h

/*
    The tst_QCallList class provides unit tests for the QCallList class.
*/
class tst_QCallList : public QObject
{
Q_OBJECT
private slots:
    void init();
    void initTestCase();

    void maximum();

    void listAddRemoveByType();

    void listAddRemoveByNumber();

    void listAddRemoveByPosition();
};

QTEST_APP_MAIN( tst_QCallList, QtopiaApplication )
#include "tst_qcalllist.moc"

/*?
    initTestCase() function called once before all tests.
    Seeds the random number generator for use with some tests.
*/
void tst_QCallList::initTestCase()
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime tomorrow = QDateTime(QDate::currentDate().addDays(1));
    qsrand(now.secsTo(tomorrow));

    
}

/*?
    init() function called before each testcase.
    All QCallList objects use the same underlying list.  This function clears
    all calls from that list and unsets any maximum amount of items.
*/
void tst_QCallList::init()
{
    QCallList list;
    list.setMaxItems(-1);
    list.clear();
}

/*?
    Testcase for QCallList's max items functionality.
    This test:
        * Constructs a QCallList with the maxItems constructor
          and checks that maxItems has been set correctly.
        * Checks that the list cannot contain more than maxItems items.
        * Changes maxItems several times and checks the above again.
        * Checks that the underlying QList is also constrained by
          maxItems.
*/
void tst_QCallList::maximum()
{
    QDateTime now = QDateTime::currentDateTime();
    QCallListItem item(QCallListItem::Dialed,
                       "foo",
                       now.addSecs(-120),
                       now.addSecs(-60),
                       QUniqueId(),
                       "VOIP");

    int count = 10;

    QCallList list(count);
    QCOMPARE(list.maxItems(), count);
    for ( int i = 0; i < 100; ++i ) {
        list.record(item, QCallList::AllowDuplicates);
    }
    QCOMPARE((int)list.count(), count);

    count = 50;
    list.setMaxItems(count);
    QCOMPARE(list.maxItems(), count);
    for ( int i = 0; i < 100; ++i ) {
        list.record(item, QCallList::AllowDuplicates);
    }
    QCOMPARE((int)list.count(), count);

    count = 5;
    list.setMaxItems(count);
    QCOMPARE(list.maxItems(), count);
    for ( int i = 0; i < 100; ++i ) {
        list.record(item, QCallList::AllowDuplicates);
    }
    QCOMPARE((int)list.count(), count);

    QList<QCallListItem> qlist = list.allCalls();
    QCOMPARE(qlist.count(), count);
}

/*?
    Testcase for remove by call type functionality of QCallList.
    This test:
        * Constructs a QCallList with the default constructor.
        * Adds many calls to the list, each with a different number,
          and with all CallTypes used at least once.
        * Checks that all QCallListItems are located in the list at
          the appropriate place.
        * Removes all dialed calls from the list and checks that they
          have been removed and that the remaining calls alternate between
          received and missed.
        * Removes all received calls from the list and checks that they
          have been removed and that the remaining calls are all missed.
        * Removes all missed calls from the list and checks that they
          have been removed.
*/
void tst_QCallList::listAddRemoveByType()
{
    QCallList list;
    QDateTime now = QDateTime::currentDateTime();

    // MAX must be divisible by 3 for a later portion of the test to
    // work correctly.
    const unsigned MAX = 99;

    for (int i = 0; i < (int)MAX; ++i) {
        QCallListItem item((QCallListItem::CallType)(i % 3),
                           QString("%1").arg(i),
                           now.addSecs(-10*MAX+i),
                           now.addSecs(-5*MAX+i),
                           QUniqueId(),
                           QString("service %1").arg(i));
        list.record(item);
    }

    QCOMPARE(list.count(), MAX);

    for (int i = 0; i < (int)MAX; ++i) {
        QCallListItem item((QCallListItem::CallType)(i % 3),
                           QString("%1").arg(i),
                           now.addSecs(-10*MAX+i),
                           now.addSecs(-5*MAX+i),
                           QUniqueId(),
                           QString("service %1").arg(i));
        QCallListItem lItem = list.at(MAX-1-i);
        QCOMPARE(lItem, item);
    }

    // Remove all dialed
    list.removeAll(QCallListItem::Dialed);
    QCOMPARE(list.count(), 2*MAX/3);

    // Test type and sequence of all remaining
    QCallListItem::CallType type = list.at(0).type();
    QVERIFY( type == QCallListItem::Missed
          || type == QCallListItem::Received);
    for (int i = 1; i < (int)list.count(); ++i) {
        if (type == QCallListItem::Missed) type = QCallListItem::Received;
        else                               type = QCallListItem::Missed;
        QCOMPARE( list.at(i).type(), type );
    }

    // Remove all received
    list.removeAll(QCallListItem::Received);
    QCOMPARE(list.count(), MAX/3);

    // Test type of all remaining
    for (int i = 0; i < (int)list.count(); ++i) {
        QCOMPARE(list.at(i).type(), QCallListItem::Missed);
    }

    // Check that all items in the list remain valid
    for (unsigned i = 0; i < list.count(); ++i) {
        QVERIFY(!list.at(i).isNull());
    }

    // This should remove only one call, since only one of the
    // first three items should have been QCallListItem::Missed.
    list.removeAll("0");
    QCOMPARE(list.count(), MAX/3);
    list.removeAll("1");
    QCOMPARE(list.count(), MAX/3);
    list.removeAll("2");
    QCOMPARE(list.count(), MAX/3 - 1);

    // Now remove all the rest.
    list.removeAll(QCallListItem::Missed);
    QCOMPARE(list.count(), (unsigned)0);
}

/*?
    Testcase for remove by call number functionality of QCallList.
    This test:
        * Constructs a QCallList with the default constructor.
        * Adds many calls to the list, each with the number '0', '1' or '2'
          in equal frequency, and with all CallTypes used at least once.
        * Checks that all QCallListItems are located in the list at
          the appropriate place.
        * Removes all calls of number '0' and checks that a third of calls
          were removed and the remaining calls alternate between '1' and '2'.
        * Removes all calls of number '1' and checks that a third of calls
          were removed and the remaining calls are '2'.
        * Removes all calls of number '2' and checks that a third of calls
          were removed, and the list is now empty.
*/
void tst_QCallList::listAddRemoveByNumber()
{
    QCallList list;
    QDateTime now = QDateTime::currentDateTime();

    // MAX must be divisible by 3 for the test to work correctly.
    const unsigned MAX = 99;

     // Add MAX numbers, equal amounts '0', '1' and '2'.
    for (int i = 0; i < (int)MAX; ++i) {
        QCallListItem item((QCallListItem::CallType)(i % 3),
                           QString("%1").arg(i % 3),
                           now.addSecs(-10*MAX+i),
                           now.addSecs(-5*MAX+i),
                           QUniqueId(),
                           QString("service %1").arg(i));
        list.record(item);
    }

    // Check list is properly populated
    for (int i = 0; i < (int)MAX; ++i) {
        QCallListItem item((QCallListItem::CallType)(i % 3),
                           QString("%1").arg(i % 3),
                           now.addSecs(-10*MAX+i),
                           now.addSecs(-5*MAX+i),
                           QUniqueId(),
                           QString("service %1").arg(i));
        QCallListItem lItem = list.at(MAX-1-i);
        QCOMPARE(lItem, item);
    }

    // Now remove one third of all numbers at a time.
    QCOMPARE(list.count(), MAX);
    list.removeAll("0");
    // Check remaining items are in correct sequence
    QString num = list.at(0).number();
    QVERIFY2( num == "1" || num == "2", QString("num is %1").arg(num).toLatin1().constData() );
    for (unsigned i = 1; i < list.count(); ++i) {
        if ( num == "1" ) num = "2";
        else              num = "1";
        QCOMPARE( list.at(i).number(), num );
    }
    QCOMPARE(list.count(), 2*MAX/3);
    list.removeAll("1");
    for (unsigned i = 0; i < list.count(); ++i) {
        QCOMPARE(list.at(i).number(), QString("2") );
    }
    QCOMPARE(list.count(), MAX/3);
    list.removeAll("2");
    QCOMPARE(list.count(), (unsigned)0);
}

/*?
    Testcase for remove by position functionality of QCallList.
    This test:
        * Constructs a QCallList with the default constructor.
        * Adds many calls to the list, counting upwards in number, and
          with all CallTypes used at least once.
        * Checks that all QCallListItems are located in the list at
          the appropriate place.
        * Removes all calls in a random number and, after each remove,
          checks that numbers remain correctly ordered in the list.
        * Checks that the list is now empty.
*/
void tst_QCallList::listAddRemoveByPosition()
{
    QCallList list;
    QDateTime now = QDateTime::currentDateTime();

    const unsigned MAX = 99;

     // Add MAX numbers
    for (int i = 0; i < (int)MAX; ++i) {
        QCallListItem item((QCallListItem::CallType)(i % 3),
                           QString("%1").arg(i),
                           now.addSecs(-10*MAX+i),
                           now.addSecs(-5*MAX+i),
                           QUniqueId(),
                           QString("service %1").arg(i));
        list.record(item);
    }

    // Check list is properly populated
    for (int i = 0; i < (int)MAX; ++i) {
        QCallListItem item((QCallListItem::CallType)(i % 3),
                           QString("%1").arg(i),
                           now.addSecs(-10*MAX+i),
                           now.addSecs(-5*MAX+i),
                           QUniqueId(),
                           QString("service %1").arg(i));
        QCallListItem lItem = list.at(MAX-1-i);
        QCOMPARE(lItem, item);
    }

    // Now remove numbers in a random order and check integrity of list
    for (int i = 0; i < (int)MAX; ++i) {
        list.removeAt(qrand() % list.count());
        QCOMPARE(list.count(), MAX - 1 - i);
        int last = MAX + 1;
        for (unsigned j = 0; j < list.count(); ++j) {
            QVERIFY( list.at(j).number().toInt() < last );
            QVERIFY( list.at(j).number().toInt() >= 0 );
            last = list.at(j).number().toInt();
        }
    }

    QCOMPARE(list.count(), (unsigned)0);

}
