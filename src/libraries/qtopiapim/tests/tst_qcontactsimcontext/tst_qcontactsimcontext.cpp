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

#include "qfuturesignal.h"
#include <QtopiaApplication>
#include <QValueSpace>
#include <QTest>

#include <private/qsimsync_p.h>
#include <private/qsimcontext_p.h>

#include "../../qpimsqlio_p.h"

#ifndef ql1
#define ql1 QLatin1String
#endif

//TESTED_CLASS=QContactSimContext
//TESTED_FILES=src/libraries/qtopiapim/qsimcontext_p.h,src/libraries/qtopiapim/qsimcontext.cpp

/*
    This class is a unit test for the QContactSimContext class.
*/
class tst_QContactSimContext : public QObject
{
    Q_OBJECT

    public:
        tst_QContactSimContext();
        virtual ~tst_QContactSimContext();

        // change the below to 'private slots:' to enable tests
    private:
        void initTestCase();

        void currentCardSQLTable();
        void simCardIdMapSQLTable();

        void flicker();

        void initialIndexes();

        void addSimContact();
        void addSimContactFull();
        void updateSimContact();
        void updateSimContactFull(); // and keep same uid
        void removeSimContact();

        void moveSimContact();

        /*
            untestable?
            4.3 to 4.4 migration
            change sim card use cases... e.g.
            new phone, existing sim
            lend phone to other sim owner
            lend sim to other phone owner
            Name limits, e.g. if too long a name, shouldn't dispear
            on sim removal, on re-insert data should remain consistent

            Field collision from sim card... e.g. exact duplicate data
        */
};

// Uncomment this if you are sure a QtopiaApplication is really required.
//QTEST_APP_MAIN( tst_QContactSimContext, QtopiaApplication )
QTEST_MAIN( tst_QContactSimContext )
#include "tst_qcontactsimcontext.moc"


tst_QContactSimContext::tst_QContactSimContext()
{
}

tst_QContactSimContext::~tst_QContactSimContext()
{
}

void tst_QContactSimContext::initTestCase()
{
    //QValueSpace::initValuespaceManager();
}

/*?
    This function tests that the functions that query the state of the
    SM card indexes, e.g firstIndex(), usedIndexes(), return the
    expected values after syncing the sim card.
*/
void tst_QContactSimContext::initialIndexes()
{
    // TODO Currently based of default phonesim setup.  Should
    // use data for various cases.

    QContactModel m;
    if (m.loadingSim())
    {
        // ensure sync finishes without error.
        QFutureSignal fs(&m, SIGNAL(simLoaded(const QPimSource &)));
        QVERIFY(fs.wait(3000));
    }


    QVERIFY(m.loadingSim());
    QCOMPARE(m.firstSimIndex(), 1);
    QCOMPARE(m.lastSimIndex(), 150);
    QCOMPARE(m.simFreeIndexCount(), 110);
    QCOMPARE(m.simUsedIndexCount(), 40);
    QCOMPARE(m.simLabelLimit(), 16);
    QCOMPARE(m.simNumberLimit(), 32);
}


/*?
    This function tests that the functions that query the state of the
    SQL table storing the information on the current sim card
*/
void tst_QContactSimContext::currentCardSQLTable()
{
    QContactModel m;
    if (m.loadingSim())
    {
        // ensure sync finishes without error.
        QFutureSignal fs(&m, SIGNAL(simLoaded(const QPimSource &)));
        QVERIFY(fs.wait(3000));
    }

    QSqlQuery q(QPimSqlIO::database());
    q.prepare("SELECT firstindex, lastindex, labellimit, numberlimit, loaded FROM currentsimcard WHERE cardid = :c AND storage = :s");
    q.bindValue(":c", 246813579);
    q.bindValue(":s", "SM");

    QVERIFY(q.exec() && q.next());

    QCOMPARE(q.value(0).toInt(), 1);
    QCOMPARE(q.value(1).toInt(), 150);
    QCOMPARE(q.value(2).toInt(), 16);
    QCOMPARE(q.value(3).toInt(), 32);
    QCOMPARE(q.value(4).toBool(), true);

    q.bindValue(":c", 246813579);
    q.bindValue(":s", "SN");

    QVERIFY(q.exec());
    QVERIFY(q.next());
    QCOMPARE(q.value(0).toInt(), 1);
    QCOMPARE(q.value(1).toInt(), 50);
    QCOMPARE(q.value(2).toInt(), 16);
    QCOMPARE(q.value(3).toInt(), 32);
    QCOMPARE(q.value(4).toBool(), true);
}


/*?
    This function tests that the functions that query the state of the
    SQL table storing information on the mapping of sim entries to
    contact record ids.

    More specifically, it ensures that for each entry in the phone book entries,
    there exists a mapped index.
*/
void tst_QContactSimContext::simCardIdMapSQLTable()
{
    QContactModel m;
    if (m.loadingSim())
    {
        // ensure sync finishes without error.
        QFutureSignal fs(&m, SIGNAL(simLoaded(const QPimSource &)));
        QVERIFY(fs.wait(3000));
    }

    QSqlQuery q(QPimSqlIO::database());
    q.prepare("SELECT cardindex FROM simcardidmap WHERE cardid = :c AND storage = :s");
    q.bindValue(":c", 246813579);
    q.bindValue(":s", "SM");

    QVERIFY(q.exec());

    QSet<int> sqlIndexes;
    while (q.next())
        sqlIndexes.insert(q.value(0).toInt());

    QSet<int> phoneBookIndexes;
    // unfortunately pre-popluated.  But then its not the phone book we
    // are testing in this test.  Should match troll.xml in the short term,
    // later this list instead should drive how the phonesim is set up.
    int i;
    for (i = 1; i < 18; ++i) phoneBookIndexes.insert(i);
    for (i = 19; i < 41; ++i) phoneBookIndexes.insert(i);
    phoneBookIndexes.insert(101);

    QCOMPARE(sqlIndexes, phoneBookIndexes);
}

/*?
    Tests that the model only resets the required number of times.  Not at all
    if the sim has loaded when the contact model is created,
    just once if it loads afterwards, and no more than twice on a sim remove/insert.
*/
void tst_QContactSimContext::flicker()
{
    // this is slightly different from normal checks.  This time checking model reset, not sim loaded.
    QContactModel m;
    QFutureSignal fs(&m, SIGNAL(modelReset()));

    if (m.loadingSim())
        QVERIFY(fs.wait(3000));
    else {
        QTest::qWait(3000);
        QCOMPARE(fs.resultCount(), 0);
    }

    // TODO need tests also for when sim is removed or inserted etc.
}

/*?
    Tests that adding a sim contact results in correct data both in the SQL
    tables and in the phone book entries.
*/
void tst_QContactSimContext::addSimContact()
{
    QContactModel m;
    if (m.loadingSim())
    {
        // ensure sync finishes without error.
        QFutureSignal fs(&m, SIGNAL(simLoaded(const QPimSource &)));
        QVERIFY(fs.wait(3000));
    }
    QContact contact;
    contact.setFirstName("Abe");
    contact.setFirstName("Contact");
    contact.setBusinessPhone("9684");
    contact.setHomePhone("4663");
    contact.setUid(m.addContact(contact, m.simSource()));

    // check report of successful add
    QVERIFY(!contact.uid().isNull());

    // check simcardidmap
    QSqlQuery q(QPimSqlIO::database());
    q.prepare("SELECT sqlid FROM simcardidmap WHERE cardid = :c AND storage = :s AND cardindex = :i");
    q.bindValue(":c", 246813579);
    q.bindValue(":s", "SM");
    q.bindValue(":i", 18);

    QVERIFY(q.exec() && q.next());
    QCOMPARE(q.value(0).toUInt(), contact.uid().toUInt());

    q.bindValue(":i", 41);
    QVERIFY(q.exec() && q.next());
    QCOMPARE(q.value(0).toUInt(), contact.uid().toUInt());


    // will like be a delay before the sim is updated.  still.
    // SOMEHOW VERIFY two new entries, 18 and 41, are in the sim.
}

/*?
    Tests that adding a sim contact if the sim is already full returns false
    and does not modify either of the sim entries or SQL tables.
*/
void tst_QContactSimContext::addSimContactFull()
{
    QContactModel m;
    if (m.loadingSim())
    {
        // ensure sync finishes without error.
        QFutureSignal fs(&m, SIGNAL(simLoaded(const QPimSource &)));
        QVERIFY(fs.wait(3000));
    }
    QContact contact;
    contact.setFirstName("Abe");
    contact.setFirstName("Contact");
    contact.setBusinessPhone("9684");
    contact.setHomePhone("4663");
    contact.setUid(m.addContact(contact, m.simSource()));

    // check report of fail add
    QEXPECT_FAIL("", "Need to be able to initialize phonesim correctly", Continue);
    QVERIFY(contact.uid().isNull());

    // check simcardidmap
    QSqlQuery q(QPimSqlIO::database());
    q.prepare("SELECT sqlid FROM simcardidmap WHERE cardid = :c AND storage = :s AND cardindex = :i");
    q.bindValue(":c", 246813579);
    q.bindValue(":s", "SM");
    q.bindValue(":i", 18);

    QEXPECT_FAIL("", "Need to be able to initialize phonesim correctly", Continue);
    QVERIFY(q.exec() && !q.next());

    q.bindValue(":i", 41);
    QEXPECT_FAIL("", "Need to be able to initialize phonesim correctly", Continue);
    QVERIFY(q.exec() && !q.next());

    // will like be a delay before the sim is updated.
    // SOMEHOW VERIFY two new entries, 18 and 41, are not in the sim
}

/*?
    Tests that updating a sim contact results in the correct data both in
    the SQL tables and the phone book entries.
*/
void tst_QContactSimContext::updateSimContact()
{
    QContactModel m;
    if (m.loadingSim())
    {
        // ensure sync finishes without error.
        QFutureSignal fs(&m, SIGNAL(simLoaded(const QPimSource &)));
        QVERIFY(fs.wait(3000));
    }

    // use good ol' chuck.  We know where is in the sim index so....
    QSqlQuery q(QPimSqlIO::database());
    q.prepare("SELECT sqlid FROM simcardidmap WHERE cardid = :c AND storage = :s AND cardindex = :i");
    q.bindValue(":c", 246813579);
    q.bindValue(":s", "SM");
    q.bindValue(":i", 4);

    QVERIFY(q.exec() && q.next());
    QUniqueId id = QUniqueId::fromUInt(q.value(0).toUInt());

    q.clear(); // may be required to stop db from blocking when running
    // transaction

    QContact contact = m.contact(id);
    QVERIFY(m.isSimCardContact(id));
    QCOMPARE(contact.firstName(), ql1("Chuck"));
    QCOMPARE(contact.lastName(), ql1("Woodbury"));

    QCOMPARE(contact.businessPhone(), ql1("45575"));
    QCOMPARE(contact.homePhone(), ql1("57968"));

    QCOMPARE(contact.phoneNumbers().count(), 2);

    contact.setHomeMobile("118122");
    contact.setDefaultEmail("chuck@thewoods.com");

    QVERIFY(m.updateContact(contact));

    // check simcardidmap
    q.prepare("SELECT sqlid FROM simcardidmap WHERE cardid = :c AND storage = :s AND cardindex = :i");
    q.bindValue(":c", 246813579);
    q.bindValue(":s", "SM");
    q.bindValue(":i", 18);

    QEXPECT_FAIL("", "Need to be able to re-initialize phonesim correctly. Card index 18 taken up by previous test", Continue);
    QVERIFY(q.exec() && q.next());
    QEXPECT_FAIL("", "Need to be able to re-initialize phonesim correctly. Card index 18 taken up by previous test", Continue);
    QCOMPARE(q.value(0).toUInt(), contact.uid().toUInt());

    // TODO also need to check actual sim at command results etc.
}

/*?
    Tests that updating a sim contact when the new entry would not fit in the sim (phoneNumbers().count() > freeIndexCount()) returns false.

    Need another test to ensure can migrate the contact to the phone list
    while retaining the same unique id.
*/
void tst_QContactSimContext::updateSimContactFull()
{
    QContactModel m;
    if (m.loadingSim())
    {
        // ensure sync finishes without error.
        QFutureSignal fs(&m, SIGNAL(simLoaded(const QPimSource &)));
        QVERIFY(fs.wait(3000));
    }

    // use good ol' chuck.  We know where is in the sim index so....
    QSqlQuery q(QPimSqlIO::database());
    q.prepare("SELECT sqlid FROM simcardidmap WHERE cardid = :c AND storage = :s AND cardindex = :i");
    q.bindValue(":c", 246813579);
    q.bindValue(":s", "SM");
    q.bindValue(":i", 4);

    QVERIFY(q.exec() && q.next());
    QUniqueId id = QUniqueId::fromUInt(q.value(0).toUInt());

    q.clear(); // may be required to stop db from blocking when running
    // transaction

    QContact contact = m.contact(id);
    QVERIFY(m.isSimCardContact(id));
    QCOMPARE(contact.firstName(), ql1("Chuck"));
    QCOMPARE(contact.lastName(), ql1("Woodbury"));

    QCOMPARE(contact.businessPhone(), ql1("45575"));
    QCOMPARE(contact.homePhone(), ql1("57968"));

    QCOMPARE(contact.phoneNumbers().count(), 2);

    contact.setHomeMobile("118122");
    contact.setDefaultEmail("chuck@thewoods.com");

    QVERIFY(!m.updateContact(contact));

    // check simcardidmap
    q.prepare("SELECT sqlid FROM simcardidmap WHERE cardid = :c AND storage = :s AND cardindex = :i");
    q.bindValue(":c", 246813579);
    q.bindValue(":s", "SM");
    q.bindValue(":i", 18);

    QVERIFY(q.exec() && !q.next());

    // TODO also need to check actual sim at command results etc.
}
/*?
    Tests removing a sim contact
*/
void tst_QContactSimContext::removeSimContact()
{
    QContactModel m;
    if (m.loadingSim())
    {
        // ensure sync finishes without error.
        QFutureSignal fs(&m, SIGNAL(simLoaded(const QPimSource &)));
        QVERIFY(fs.wait(3000));
    }

    // use good ol' chuck.  We know where is in the sim index so....
    QSqlQuery q(QPimSqlIO::database());
    q.prepare("SELECT sqlid FROM simcardidmap WHERE cardid = :c AND storage = :s AND cardindex = :i");
    q.bindValue(":c", 246813579);
    q.bindValue(":s", "SM");
    q.bindValue(":i", 4);

    QVERIFY(q.exec() && q.next());
    QUniqueId id = QUniqueId::fromUInt(q.value(0).toUInt());

    q.clear(); // may be required to stop db from blocking when running
    // transaction

    QContact contact = m.contact(id);
    QCOMPARE(contact.firstName(), ql1("Chuck"));
    QCOMPARE(contact.lastName(), ql1("Woodbury"));

    QCOMPARE(contact.businessPhone(), ql1("45575"));
    QCOMPARE(contact.homePhone(), ql1("57968"));

    QCOMPARE(contact.phoneNumbers().count(), 2);

    QVERIFY(m.removeContact(contact));

    // check simcardidmap
    q.prepare("SELECT sqlid FROM simcardidmap WHERE cardid = :c AND storage = :s AND cardindex = :i");
    q.bindValue(":c", 246813579);
    q.bindValue(":s", "SM");
    q.bindValue(":i", 4);

    QVERIFY(q.exec() && !q.next());

    q.bindValue(":i", 5);

    QVERIFY(q.exec() && !q.next());

    // TODO, once again, need to verify SIM behavior
}

void tst_QContactSimContext::moveSimContact()
{
    QContactModel m;
    if (m.loadingSim())
    {
        // ensure sync finishes without error.
        QFutureSignal fs(&m, SIGNAL(simLoaded(const QPimSource &)));
        QVERIFY(fs.wait(3000));
    }

    // use good ol' chuck.  We know where is in the sim index so....
    QSqlQuery q(QPimSqlIO::database());
    q.prepare("SELECT sqlid FROM simcardidmap WHERE cardid = :c AND storage = :s AND cardindex = :i");
    q.bindValue(":c", 246813579);
    q.bindValue(":s", "SM");
    q.bindValue(":i", 4);

    QVERIFY(q.exec() && q.next());
    QUniqueId id = QUniqueId::fromUInt(q.value(0).toUInt());

    q.clear(); // may be required to stop db from blocking when running
    // transaction

    QContact contact = m.contact(id);
    QCOMPARE(contact.firstName(), ql1("Chuck"));
    QCOMPARE(contact.lastName(), ql1("Woodbury"));

    QCOMPARE(contact.businessPhone(), ql1("45575"));
    QCOMPARE(contact.homePhone(), ql1("57968"));

    QCOMPARE(contact.phoneNumbers().count(), 2);

    contact.setHomeMobile("118122");
    contact.setDefaultEmail("chuck@thewoods.com");

    QVERIFY(m.isSimCardContact(id));
    QVERIFY(m.exists(id));
    QVERIFY(m.removeContact(id));
    QVERIFY(!m.exists(id));

    // check simcardidmap
    q.prepare("SELECT sqlid FROM simcardidmap WHERE cardid = :c AND storage = :s AND cardindex = :i");
    q.bindValue(":c", 246813579);
    q.bindValue(":s", "SM");
    q.bindValue(":i", 4);

    QVERIFY(q.exec() && !q.next());

    q.bindValue(":i", 5);

    QVERIFY(q.exec() && !q.next());

    // and that it isn't on the SIM anymore
}
