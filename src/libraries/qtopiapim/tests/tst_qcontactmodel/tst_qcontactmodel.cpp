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

#include <QContactModel>
#include <QContact>

#include <QtopiaApplication>
#include <QtopiaSql>
#include <QTimeZone>
#include <QSqlQuery>
#include <QDebug>
#include <QAppointmentModel>
#include <QTest>
#include <shared/qtopiaunittest.h>

#include <QCollectivePresence>
#include <QCollectivePresenceInfo>
#include <QFieldDefinition>
#include <QValueSpace>

#include "../../qpimsqlio_p.h"
#include "../../qcontactsqlio_p.h"

// Enable this define to do some observational performance testing
// (theres no threshold for a pass test)
// #define QCONTACTMODEL_PERFTEST

#ifdef QCONTACTMODEL_PERFTEST
#include <math.h>
#endif

//TESTED_CLASS=QContactModel
//TESTED_FILES=src/libraries/qtopiapim/qcontactmodel.h

/*
    The tst_QContactModel class provides unit tests for QContactModel.
*/
class tst_QContactModel : public QObject
{
Q_OBJECT
protected slots:

    void cleanup();
    void initTestCase()
    {
        if (qApp->type() == QApplication::GuiServer)
            QValueSpace::initValuespaceManager();

        /* changes under this key will cause any QContactSqlIOs to reset their cache */
        const QString valueSpaceKey(QLatin1String("PIM/Contacts/Presence"));
        mVSObject = new QValueSpaceObject(valueSpaceKey);
        mVSValue = 1;
    }



private slots:
    void changeLog();
    void matchEmailAddress();
    void addNotify();
    void addContact();
    void checkDependentEvents();
    void contactField();
    void containsChat();
    void matchChat();
    void match();
    void matchPhoneNumber();
    void sort();
    void label();
    void presence();
#ifdef QCONTACTMODEL_PERFTEST
    void perf();
    void presPerf();
    void labelPerf();
#endif

    private:
        void pushPresence(const QString&, QList<QCollectivePresenceInfo>);
        QValueSpaceObject *mVSObject;
        int mVSValue;
};

QTEST_APP_MAIN( tst_QContactModel, QtopiaApplication )
#include "tst_qcontactmodel.moc"

/*?
    Test notification.
    When changes are made to one model changes should be reflected in other models both within the same process and
    in other processes.  This is an asynchronous update, however within process it should occur within two loops of
    the event loop and less than 100ms between processes.

    NOTE: need to test both update/remove/addContact and setData/removeRow style functions.
    the data and setData functions allow partial access to contacts and hence may go via different paths
    than updateContact
*/
void tst_QContactModel::addNotify()
{
    QContactModel *source = new QContactModel;
    QContactModel *destination = new QContactModel;

    /* ensure initial state */
    QVERIFY(source->count() == 0);
    QVERIFY(destination->count() == 0);

    QContact contact;
    contact.setFirstName("Abby");
    contact.setLastName("Contact");
    contact.setHomePhone("2229");

    contact.setUid(source->addContact(contact));

    QVERIFY(source->count() == 1);
    QContact stored = source->contact(0);
    QVERIFY(source->contact(0) == contact);

    // caching and bunching of updates can mean have to go through event loop twice.
    qApp->processEvents();
    qApp->processEvents();

    // and now the update...
    QVERIFY(destination->count() == 1);
    QVERIFY(destination->contact(0) == contact);
}

namespace QTest {
    /* Allow QDSData to be converted to string for use with QCOMPARE. */
    template<> inline char *toString(const QSet<QUniqueId> &c) {
        QStringList list;
        foreach(QUniqueId id, c) {
            list.append(id.toLocalContextString());
        }
        return qstrdup(list.join(", ").toLatin1().constData());
    }
}

/*?
  Test changelog.
  Tests that the changelog remains up to date and returns correct information for removed, added, and modified queries.
*/

void tst_QContactModel::changeLog()
{
    QContactModel *source = new QContactModel;

    QContact abby;
    abby.setFirstName("Abby");
    abby.setLastName("Contact");
    abby.setHomePhone("2229");

    QContact bill;
    bill.setFirstName("Bill");
    bill.setLastName("Contact");
    bill.setHomePhone("3456");

    QContact charlie;
    charlie.setFirstName("Charlie");
    charlie.setLastName("Contact");
    charlie.setHomePhone("8970");

    QContact david;
    david.setFirstName("David");
    david.setLastName("Contact");
    david.setHomePhone("8739");

    QContact edward;
    edward.setFirstName("Edward");
    edward.setLastName("Contact");
    edward.setHomePhone("8940");

    QDateTime start = QTimeZone::current().toUtc(QDateTime::currentDateTime());

    abby.setUid(source->addContact(abby));
    QVERIFY(!abby.uid().isNull());

    bill.setUid(source->addContact(charlie));
    QVERIFY(!bill.uid().isNull());

    QTest::qSleep(1100);
    QDateTime firsttwo = QTimeZone::current().toUtc(QDateTime::currentDateTime());
    QVERIFY(start < firsttwo);

    charlie.setUid(source->addContact(charlie));
    QVERIFY(!charlie.uid().isNull());

    david.setUid(source->addContact(david));
    QVERIFY(!david.uid().isNull());

    QTest::qSleep(1100);
    QDateTime secondtwo = QTimeZone::current().toUtc(QDateTime::currentDateTime());
    QVERIFY(firsttwo < secondtwo);

    // mod and remove

    QVERIFY(source->removeContact(bill));
    charlie.setLastName("NotContact");
    QVERIFY(source->updateContact(charlie));

    QTest::qSleep(1100);
    QDateTime modandremove = QTimeZone::current().toUtc(QDateTime::currentDateTime());
    QVERIFY(secondtwo < modandremove);

    // ok, now query for changes.

    QSet<QUniqueId> startMod, startRemoved, startAdded;
    QSet<QUniqueId> firstMod, firstRemoved, firstAdded;
    QSet<QUniqueId> secondMod, secondRemoved, secondAdded;
    QSet<QUniqueId> lastMod, lastRemoved, lastAdded;

    startAdded.insert(abby.uid());
    startAdded.insert(charlie.uid());
    startAdded.insert(david.uid());

    firstAdded.insert(charlie.uid());
    firstAdded.insert(david.uid());
    firstRemoved.insert(bill.uid());

    secondMod.insert(charlie.uid());
    secondRemoved.insert(bill.uid());

    QCOMPARE(QSet<QUniqueId>::fromList(source->modified(start)), startMod);
    QCOMPARE(QSet<QUniqueId>::fromList(source->added(start)), startAdded);
    QCOMPARE(QSet<QUniqueId>::fromList(source->removed(start)), startRemoved);

    QCOMPARE(QSet<QUniqueId>::fromList(source->modified(firsttwo)), firstMod);
    QCOMPARE(QSet<QUniqueId>::fromList(source->added(firsttwo)), firstAdded);
    QCOMPARE(QSet<QUniqueId>::fromList(source->removed(firsttwo)), firstRemoved);

    QCOMPARE(QSet<QUniqueId>::fromList(source->modified(secondtwo)), secondMod);
    QCOMPARE(QSet<QUniqueId>::fromList(source->added(secondtwo)), secondAdded);
    QCOMPARE(QSet<QUniqueId>::fromList(source->removed(secondtwo)), secondRemoved);

    QCOMPARE(QSet<QUniqueId>::fromList(source->modified(modandremove)), lastMod);
    QCOMPARE(QSet<QUniqueId>::fromList(source->added(modandremove)), lastAdded);
    QCOMPARE(QSet<QUniqueId>::fromList(source->removed(modandremove)), lastRemoved);

    // Now that we have a verified change history, lets apply a sync diff.
    // first one we will abort, and then one we will keep.

    // sync change will be to remove charlie, add edward, and modify abby.
    abby.setLastName("NotContact");
    QTest::qSleep(1100);
    QDateTime syncTime = QTimeZone::current().toUtc(QDateTime::currentDateTime());
    QTest::qSleep(1100);
    QDateTime postSync = QTimeZone::current().toUtc(QDateTime::currentDateTime());

    QVERIFY(source->startSyncTransaction(syncTime));
    QVERIFY(source->removeContact(charlie));
    edward.setUid(source->addContact(edward));
    QVERIFY(!edward.uid().isNull());
    QVERIFY(source->updateContact(abby));

    QVERIFY(source->abortSyncTransaction());

    // ok, change logs should be the same as they were before.
    QCOMPARE(QSet<QUniqueId>::fromList(source->modified(start)), startMod);
    QCOMPARE(QSet<QUniqueId>::fromList(source->added(start)), startAdded);
    QCOMPARE(QSet<QUniqueId>::fromList(source->removed(start)), startRemoved);

    QCOMPARE(QSet<QUniqueId>::fromList(source->modified(firsttwo)), firstMod);
    QCOMPARE(QSet<QUniqueId>::fromList(source->added(firsttwo)), firstAdded);
    QCOMPARE(QSet<QUniqueId>::fromList(source->removed(firsttwo)), firstRemoved);

    QCOMPARE(QSet<QUniqueId>::fromList(source->modified(secondtwo)), secondMod);
    QCOMPARE(QSet<QUniqueId>::fromList(source->added(secondtwo)), secondAdded);
    QCOMPARE(QSet<QUniqueId>::fromList(source->removed(secondtwo)), secondRemoved);

    QCOMPARE(QSet<QUniqueId>::fromList(source->modified(modandremove)), lastMod);
    QCOMPARE(QSet<QUniqueId>::fromList(source->added(modandremove)), lastAdded);
    QCOMPARE(QSet<QUniqueId>::fromList(source->removed(modandremove)), lastRemoved);

    // now to test a successful sync
    QVERIFY(source->startSyncTransaction(syncTime));
    QVERIFY(source->removeContact(charlie));
    edward.setUid(source->addContact(edward));
    QVERIFY(!edward.uid().isNull());
    QVERIFY(source->updateContact(abby));

    QVERIFY(source->commitSyncTransaction());

    QSet<QUniqueId> syncMod, syncRemoved, syncAdded;

    syncMod.insert(abby.uid());
    syncRemoved.insert(charlie.uid());
    syncAdded.insert(edward.uid());

    startAdded.insert(edward.uid());
    startAdded.remove(charlie.uid());

    firstAdded.insert(edward.uid());
    firstAdded.remove(charlie.uid());
    firstMod.insert(abby.uid());

    secondAdded.insert(edward.uid());
    secondRemoved.insert(charlie.uid());
    secondMod.insert(abby.uid());
    secondMod.remove(charlie.uid());

    QCOMPARE(QSet<QUniqueId>::fromList(source->modified(syncTime)), syncMod);
    QCOMPARE(QSet<QUniqueId>::fromList(source->added(syncTime)), syncAdded);
    QCOMPARE(QSet<QUniqueId>::fromList(source->removed(syncTime)), syncRemoved);

    QCOMPARE(QSet<QUniqueId>::fromList(source->modified(start)), startMod);
    QCOMPARE(QSet<QUniqueId>::fromList(source->added(start)), startAdded);
    QCOMPARE(QSet<QUniqueId>::fromList(source->removed(start)), startRemoved);

    QCOMPARE(QSet<QUniqueId>::fromList(source->modified(firsttwo)), firstMod);
    QCOMPARE(QSet<QUniqueId>::fromList(source->added(firsttwo)), firstAdded);
    QCOMPARE(QSet<QUniqueId>::fromList(source->removed(firsttwo)), firstRemoved);

    QCOMPARE(QSet<QUniqueId>::fromList(source->modified(secondtwo)), secondMod);
    QCOMPARE(QSet<QUniqueId>::fromList(source->added(secondtwo)), secondAdded);
    QCOMPARE(QSet<QUniqueId>::fromList(source->removed(secondtwo)), secondRemoved);

    QCOMPARE(QSet<QUniqueId>::fromList(source->modified(postSync)), lastMod);
    QCOMPARE(QSet<QUniqueId>::fromList(source->added(postSync)), lastAdded);
    QCOMPARE(QSet<QUniqueId>::fromList(source->removed(postSync)), lastRemoved);
}

/*?
    Cleanup after each test function.
    Removes all appointments from a QContactModel.
*/
void tst_QContactModel::cleanup()
{
    QString noclean = qgetenv("QTEST_NO_CLEANUP");
    if (noclean.isEmpty()) {
        QContactModel *model = new QContactModel;
        int count = model->count();
        while(count--) {
            QUniqueId id = model->id(0);
            QVERIFY(model->removeContact(id));
        }
        // need to reset changelog as well
        QSqlQuery q;
        q.prepare("DELETE FROM changelog");
        q.exec();
    }
}

#define SET_ALL(C,S) \
    { \
        QContactAddress _h = { "h1" S, "h2" S, "h3" S, "h4" S, "h5" S }; \
        C.setAddress(QContact::Home, _h); \
    } \
    { \
        QContactAddress _b = { "b1" S, "b2" S, "b3" S, "b4" S, "b5" S }; \
        C.setAddress(QContact::Business, _b); \
    } \
    { \
        QContactAddress _o = { "o1" S, "o2" S, "o3" S, "o4" S, "o5" S }; \
        C.setAddress(QContact::Other, _o); \
    } \
    C.setAnniversary(QDate(1900,1,1).addDays(QString(S).size())); \
    C.setAssistant("assistant " S); \
    C.setBirthday(QDate(1900,1,1).addDays(QString(S).size()*2));  \
    C.setChildren("children " S); \
    C.setCompany("company " S); \
    C.setCompanyPronunciation("company pronunciation " S); \
    C.setDepartment("department " S); \
    C.setEmailList(QStringList() << "email" S << "email" S "2" << "email" S "3"); \
    C.setFirstName("firstname " S); \
    C.setFirstNamePronunciation("firstname pro " S); \
    C.setGender((QContact::GenderType)(QString(S).size() % 3)); \
    C.setJobTitle("job " S); \
    C.setLastName("lastname " S); \
    C.setLastNamePronunciation("lastname pro " S); \
    C.setManager("manager " S); \
    C.setMiddleName("middlename " S); \
    C.setNameTitle("nametitle " S); \
    C.setNickname("nick " S); \
    C.setNotes("notes " S); \
    C.setOffice("office " S); \
    C.setPhoneNumber(QContact::HomePhone, "homephone " S); \
    C.setPhoneNumber(QContact::HomeMobile, "homemobile " S); \
    C.setPhoneNumber(QContact::HomeFax, "homefax " S); \
    C.setPhoneNumber(QContact::HomePager, "homepager " S); \
    C.setPhoneNumber(QContact::BusinessPhone, "businessphone " S); \
    C.setPhoneNumber(QContact::BusinessMobile, "businessmobile " S); \
    C.setPhoneNumber(QContact::BusinessFax, "businessfax " S); \
    C.setPhoneNumber(QContact::BusinessPager, "businesspager " S); \
    C.setPhoneNumber(QContact::OtherPhone, "otherphone " S); \
    C.setPhoneNumber(QContact::Mobile, "mobile " S); \
    C.setPhoneNumber(QContact::Fax, "fax " S); \
    C.setPhoneNumber(QContact::Pager, "pager " S); \
    C.setPortraitFile("portrait " S); \
    C.setProfession("profession " S); \
    C.setSpouse("spouse " S); \
    C.setSuffix("suffix " S);


/* Verify the contents of QContact C are equal to those set by SET_ALL(). */
#define VERIFY_ALL(C,S) \
    { \
        QContactAddress _h = { "h1" S, "h2" S, "h3" S, "h4" S, "h5" S }; \
        QCOMPARE(C.address(QContact::Home), _h); \
    } \
    { \
        QContactAddress _b = { "b1" S, "b2" S, "b3" S, "b4" S, "b5" S }; \
        QCOMPARE(C.address(QContact::Business), _b); \
    } \
    { \
        QContactAddress _o = { "o1" S, "o2" S, "o3" S, "o4" S, "o5" S }; \
        QCOMPARE(C.address(QContact::Other), _o); \
    } \
    QCOMPARE(C.anniversary(),QDate(1900,1,1).addDays(QString(S).size())); \
    QCOMPARE(C.assistant(), QString("assistant " S)); \
    QCOMPARE(C.birthday(), QDate(1900,1,1).addDays(QString(S).size()*2));  \
    QCOMPARE(C.children(), QString("children " S)); \
    QCOMPARE(C.company(), QString("company " S)); \
    QCOMPARE(C.companyPronunciation(), QString("company pronunciation " S)); \
    QCOMPARE(C.department(), QString("department " S)); \
    QCOMPARE(C.emailList(), QStringList() << "email" S << "email" S "2" << "email" S "3"); \
    QCOMPARE(C.firstName(), QString("firstname " S)); \
    QCOMPARE(C.firstNamePronunciation(), QString("firstname pro " S)); \
    QCOMPARE(C.gender(), (QContact::GenderType)(QString(S).size() % 3)); \
    QCOMPARE(C.jobTitle(), QString("job " S)); \
    QCOMPARE(C.lastName(), QString("lastname " S)); \
    QCOMPARE(C.lastNamePronunciation(), QString("lastname pro " S)); \
    QCOMPARE(C.manager(), QString("manager " S)); \
    QCOMPARE(C.middleName(), QString("middlename " S)); \
    QCOMPARE(C.nameTitle(), QString("nametitle " S)); \
    QCOMPARE(C.nickname(), QString("nick " S)); \
    QCOMPARE(C.notes(), QString("notes " S)); \
    QCOMPARE(C.office(), QString("office " S)); \
    QCOMPARE(C.phoneNumber(QContact::HomePhone), QString("homephone " S)); \
    QCOMPARE(C.phoneNumber(QContact::HomeMobile), QString("homemobile " S)); \
    QCOMPARE(C.phoneNumber(QContact::HomeFax), QString("homefax " S)); \
    QCOMPARE(C.phoneNumber(QContact::HomePager), QString("homepager " S)); \
    QCOMPARE(C.phoneNumber(QContact::BusinessPhone), QString("businessphone " S)); \
    QCOMPARE(C.phoneNumber(QContact::BusinessMobile), QString("businessmobile " S)); \
    QCOMPARE(C.phoneNumber(QContact::BusinessFax), QString("businessfax " S)); \
    QCOMPARE(C.phoneNumber(QContact::BusinessPager), QString("businesspager " S)); \
    QCOMPARE(C.phoneNumber(QContact::OtherPhone), QString("otherphone " S)); \
    QCOMPARE(C.phoneNumber(QContact::Mobile), QString("mobile " S)); \
    QCOMPARE(C.phoneNumber(QContact::Fax), QString("fax " S)); \
    QCOMPARE(C.phoneNumber(QContact::Pager), QString("pager " S)); \
    QCOMPARE(C.portraitFile(), QString("portrait " S)); \
    QCOMPARE(C.profession(), QString("profession " S)); \
    QCOMPARE(C.spouse(), QString("spouse " S)); \
    QCOMPARE(C.suffix(), QString("suffix " S));

/*?
  Add a contact to the database, retrieve it again,
  and make sure the retrieved copy is the same as
  the original contact.
*/
void tst_QContactModel::addContact()
{
    QContactModel model;

    QContact bob;
    QList<QString> cats;
    cats << "Business";
    SET_ALL(bob, "bob");
    bob.setCustomField("custom1", "custom1value");
    bob.setCustomField("custom2", "custom2value");
    bob.setCategories(cats);
    QUniqueId id = model.addContact(bob);
    bob.setUid(id);

    QContact bobstwin = model.contact(id);

    VERIFY_ALL(bobstwin, "bob");
    QCOMPARE(bob, bobstwin);
    QCOMPARE(bobstwin.customField("custom1"), QString("custom1value"));
    QCOMPARE(bobstwin.customField("custom2"), QString("custom2value"));
    QCOMPARE(bobstwin.categories(), cats);
}


/*?
  Add a contact with a birthday and an anniversary to the database,
  and make sure that a corresponding recurring appointment has been
  created.
*/

void tst_QContactModel::checkDependentEvents()
{
    QContactModel model;

    QContact alice;
    alice.setFirstName("Alice");
    alice.setAnniversary(QDate(1918, 4, 11));

    QContact bob;
    bob.setFirstName("Bob");
    bob.setBirthday(QDate(1960,3,3));

    alice.setUid(model.addContact(alice));
    bob.setUid(model.addContact(bob));

    QAppointmentModel amodel;
    QAppointment anniv = amodel.appointment(alice.dependentChildrenOfType("anniversary").value(0));
    QAppointment bday = amodel.appointment(bob.dependentChildrenOfType("birthday").value(0));

    QVERIFY2(!anniv.uid().isNull(), "Dependent anniversary does not exist");
    QVERIFY2(!bday.uid().isNull(), "Dependent birthday does not exist");

    QCOMPARE(anniv.repeatRule(), QAppointment::Yearly);
    QCOMPARE(bday.repeatRule(), QAppointment::Yearly);

    QCOMPARE(anniv.start().date(), alice.anniversary());
    QCOMPARE(bday.start().date(), bob.birthday());

    // These may change..
    QCOMPARE(anniv.description(), QString("Alice"));
    QCOMPARE(bday.description(), QString("Bob"));
}

/*?
  Make sure that using QContactModel::contactField(task, field) returns the
  same as using QContactModel::data(index(row, field))
*/
void tst_QContactModel::contactField()
{
    QContactModel model;

    QList<QContactModel::Field> fields;

    QContact alice;

    SET_ALL(alice, "alice");
    alice.setCategories(QStringList() << "Business"); // no tr

    /* uncomment fields as data for field included */

    fields << QContactModel::Label;
    fields << QContactModel::NameTitle;
    fields << QContactModel::FirstName;
    fields << QContactModel::MiddleName;
    fields << QContactModel::LastName;
    fields << QContactModel::Suffix;
    fields << QContactModel::JobTitle;
    fields << QContactModel::Department;
    fields << QContactModel::Company;
    fields << QContactModel::DefaultPhone;
    fields << QContactModel::BusinessPhone;
    fields << QContactModel::BusinessFax;
    fields << QContactModel::BusinessMobile;
    fields << QContactModel::DefaultEmail;
    fields << QContactModel::Emails;
    fields << QContactModel::OtherPhone;
    fields << QContactModel::OtherFax;
    fields << QContactModel::OtherMobile;
    fields << QContactModel::OtherPager;
    fields << QContactModel::HomePhone;
    fields << QContactModel::HomeFax;
    fields << QContactModel::HomeMobile;
    fields << QContactModel::HomePager;
    fields << QContactModel::BusinessStreet;
    fields << QContactModel::BusinessCity;
    fields << QContactModel::BusinessState;
    fields << QContactModel::BusinessZip;
    fields << QContactModel::BusinessCountry;
    fields << QContactModel::BusinessPager;
    fields << QContactModel::BusinessWebPage;
    fields << QContactModel::Office;
    fields << QContactModel::Profession;
    fields << QContactModel::Assistant;
    fields << QContactModel::Manager;
    fields << QContactModel::HomeStreet;
    fields << QContactModel::HomeCity;
    fields << QContactModel::HomeState;
    fields << QContactModel::HomeZip;
    fields << QContactModel::HomeCountry;
    fields << QContactModel::HomeWebPage;
    fields << QContactModel::Spouse;
    fields << QContactModel::Gender;
    fields << QContactModel::Birthday;
    fields << QContactModel::Anniversary;
    fields << QContactModel::Nickname;
    fields << QContactModel::Children;
    fields << QContactModel::Portrait;
    fields << QContactModel::Notes;
    fields << QContactModel::LastNamePronunciation;
    fields << QContactModel::FirstNamePronunciation;
    fields << QContactModel::CompanyPronunciation;
    fields << QContactModel::Identifier;
    fields << QContactModel::Categories;
    fields << QContactModel::PresenceStatus;
    fields << QContactModel::PresenceStatusString;
    fields << QContactModel::PresenceMessage;
    fields << QContactModel::PresenceDisplayName;
    fields << QContactModel::PresenceCapabilities;
    fields << QContactModel::PresenceAvatar;
    fields << QContactModel::PresenceUpdateTime;

    QUniqueId id = model.addContact(alice);
    alice.setUid(id);
    QModelIndex index = model.index(id);

    QCollectivePresenceInfo p1("homephone alice");
    p1.setPresence("Online", QCollectivePresenceInfo::Online);
    p1.setMessage("Available");
    p1.setAvatar("Alice's Avatar");
    p1.setLastUpdateTime(QDateTime::currentDateTime());
    p1.setDisplayName("AliceCool73");
    p1.setCapabilities(QStringList() << "voice" << "email" << "video");

    foreach(QContactModel::Field f, fields) {
        QVariant contactField = QContactModel::contactField(alice, f);
        QVariant modelData = model.data(model.index(index.row(), f), Qt::DisplayRole);

        /* Certain fields cannot be compared as QVariants */

        switch(f) {
            case QContactModel::PresenceStatusString:
            case QContactModel::PresenceDisplayName:
            case QContactModel::PresenceMessage:
            case QContactModel::PresenceAvatar:
                {
                    QPresenceStringMap p1 = contactField.value<QPresenceStringMap>();
                    QPresenceStringMap p2 = modelData.value<QPresenceStringMap>();
                    if (p1 != p2) {
                        QString msg;
                        QDebug msgd(&msg);
                        msgd << "field " << QContactModel::fieldLabel(f) << "didn't match: contactField() " << p1 << "!= model.data()" << p2 << ":\n";
                        for(int i=0; i < qMax(p1.count(), p2.count()); i++) {
                            msgd << " " << i << ": " << p1.values().value(i) << p2.values().value(i) << "\n";
                        }
                        QByteArray ba = msg.toLatin1();
                        QFAIL(ba.constData());
                    }
                }
                break;

            case QContactModel::PresenceUpdateTime:
                {
                    QPresenceDateTimeMap p1 = contactField.value<QPresenceDateTimeMap>();
                    QPresenceDateTimeMap p2 = modelData.value<QPresenceDateTimeMap>();
                    if (p1 != p2) {
                        QString msg;
                        QDebug msgd(&msg);
                        msgd << "field " << QContactModel::fieldLabel(f) << "didn't match: contactField() " << p1 << "!= model.data()" << p2 << ":\n";
                        for(int i=0; i < qMax(p1.count(), p2.count()); i++) {
                            msgd << " " << i << ": " << p1.values().value(i) << p2.values().value(i) << "\n";
                        }
                        QByteArray ba = msg.toLatin1();
                        QFAIL(ba.constData());
                    }
                }
                break;

            case QContactModel::PresenceCapabilities:
                {
                    QPresenceCapabilityMap p1 = contactField.value<QPresenceCapabilityMap>();
                    QPresenceCapabilityMap p2 = modelData.value<QPresenceCapabilityMap>();
                    if (p1 != p2) {
                        QString msg;
                        QDebug msgd(&msg);
                        msgd << "field " << QContactModel::fieldLabel(f) << "didn't match: contactField() " << p1 << "!= model.data()" << p2 << ":\n";
                        for(int i=0; i < qMax(p1.count(), p2.count()); i++) {
                            msgd << " " << i << ": " << p1.values().value(i) << p2.values().value(i) << "\n";
                        }
                        QByteArray ba = msg.toLatin1();
                        QFAIL(ba.constData());
                    }
                }
                break;

            case QContactModel::PresenceStatus:
                {
                    QPresenceTypeMap p1 = contactField.value<QPresenceTypeMap>();
                    QPresenceTypeMap p2 = modelData.value<QPresenceTypeMap>();
                    if (p1 != p2) {
                        QString msg;
                        QDebug msgd(&msg);
                        msgd << "field " << QContactModel::fieldLabel(f) << "didn't match: contactField() " << p1 << "!= model.data()" << p2 << ":\n";
                        for(int i=0; i < qMax(p1.count(), p2.count()); i++) {
                            msgd << " " << i << ": " << QString::number(p1.values().value(i)) << QString::number(p2.values().value(i)) << "\n";
                        }
                        QByteArray ba = msg.toLatin1();
                        QFAIL(ba.constData());
                    }
                }
                break;

            default:
                if (contactField != modelData) {
                    QString msg;
                    QDebug msgd(&msg);
                    msgd << "field " << QContactModel::fieldLabel(f) << "didn't match: contactField() " << contactField << "!= model.data()" << modelData;
                    QByteArray ba = msg.toLatin1();
                    QFAIL(ba.constData());
                }
                break;
        }
    }
}


void tst_QContactModel::matchPhoneNumber()
{
    QContactModel model;

    QSet<QPimSource> sources = model.availableSources();
    QPimSource simSource = model.simSource();
    QSet<QPimSource> nonSimSources = sources;
    nonSimSources -= simSource;

    // Stuff some stuff in
    QContact a;
    a.setFirstName("Aaron");
    a.setLastName("Aaronson");
    a.setCompany("Aardvark Inc.");
    a.setHomePhone("1111");
    a.setBusinessPhone("2222");
    a.setJobTitle("The Boss");
    a.insertEmail("theboss@aardvark.com");

    QContact b;
    b.setFirstName("Burt");
    b.setLastName("Bacharach");
    b.setCompany("Aardvark Inc.");
    b.setBusinessPhone("2223");
    b.setJobTitle("His Burtness");
    b.insertEmail("burt@bacharach.com"); // default email
    b.insertEmail("burt@aardvark.com");

    QContact c;
    c.setFirstName("Carol-Ann");
    c.setLastName("Vanderbilt");
    c.setHomePhone("3333");
    c.setHomeStreet("56 Mansion Way");

    QContact e;
    e.setFirstName("Bruce");
    e.setLastName("Bruceson");
    e.insertEmail("bruce@bruce.au");
    e.setHomePhone("3334");

    QContact m;
    m.setFirstName("David");
    m.setMiddleName("Fingers");
    m.setLastName("Hurt");
    m.setJobTitle("office lackey");
    m.setBusinessPhone("1222");
    m.insertEmail("sql\%inje*tion@aardvark.com*:bind"); // silly test

    QContact i;
    i.setFirstName("International");
    i.setLastName("Man of Mystery");
    i.setHomePhone("+61701234567");

    QContact j;
    j.setFirstName("Long");
    j.setLastName("Country Code");
    j.setBusinessPhone("0712345678");

    a.setUid(model.addContact(a));
    b.setUid(model.addContact(b));
    c.setUid(model.addContact(c));
    e.setUid(model.addContact(e));
    m.setUid(model.addContact(m));
    i.setUid(model.addContact(i));
    j.setUid(model.addContact(j));

    QContact none;
    QContact matched;

    // Now do some testing on the unfiltered model
    QCOMPARE(model.matchPhoneNumber("1111"), a);
    QCOMPARE(model.matchPhoneNumber("2222"), a);
    QCOMPARE(model.matchPhoneNumber("2223"), b);
    QCOMPARE(model.matchPhoneNumber("3333"), c);
    QCOMPARE(model.matchPhoneNumber("3334"), e);
    QCOMPARE(model.matchPhoneNumber("1222"), m);

    // International prefix stripping
    QCOMPARE(model.matchPhoneNumber("+61701234567"), i);
    QCOMPARE(model.matchPhoneNumber("0701234567"), i);
    QCOMPARE(model.matchPhoneNumber("0061701234567"), i);
    QCOMPARE(model.matchPhoneNumber("12345678"), j);
    QCOMPARE(model.matchPhoneNumber("0712345678"), j);
    QCOMPARE(model.matchPhoneNumber("+880712345678"), j);

    // Now set a text filter
    model.setFilter("David");
    QCOMPARE(model.matchPhoneNumber("1111"), none);
    QCOMPARE(model.matchPhoneNumber("2222"), none);
    QCOMPARE(model.matchPhoneNumber("2223"), none);
    QCOMPARE(model.matchPhoneNumber("3333"), none);
    QCOMPARE(model.matchPhoneNumber("3334"), none);
    QCOMPARE(model.matchPhoneNumber("1222"), m);

    // Now set a contains filter
    model.setFilter(QString(), QContactModel::ContainsEmail);
    QCOMPARE(model.matchPhoneNumber("1111"), a);
    QCOMPARE(model.matchPhoneNumber("2222"), a);
    QCOMPARE(model.matchPhoneNumber("2223"), b);
    QCOMPARE(model.matchPhoneNumber("3333"), none);
    QCOMPARE(model.matchPhoneNumber("3334"), e);
    QCOMPARE(model.matchPhoneNumber("1222"), m);

    // XXX Need to test sources, but difficult with current API
}


void tst_QContactModel::match()
{
    QContactModel model;

    // Stuff some stuff in
    QContact a;
    a.setFirstName("Aaron");
    a.setLastName("Aaronson");
    a.setCompany("Aardvark Inc.");
    a.setHomePhone("1111");
    a.setBusinessPhone("2222");
    a.setJobTitle("The Boss");
    a.insertEmail("theboss@aardvark.com");

    QContact b;
    b.setFirstName("Burt");
    b.setLastName("Bacharach");
    b.setCompany("Aardvark Inc.");
    b.setBusinessPhone("2223");
    b.setJobTitle("His Burtness");
    b.insertEmail("burt@bacharach.com"); // default email
    b.insertEmail("burt@aardvark.com");

    QContact c;
    c.setFirstName("Carol-Ann");
    c.setLastName("Vanderbilt");
    c.setHomeStreet("56 Mansion Way");

    QContact d;
    d.setFirstName("David");
    d.setLastName("White");
    d.setBusinessStreet("123 Mansion Way");
    d.insertEmail("david@mansionindustries.com");

    QContact m;
    m.setFirstName("Ma");
    m.setMiddleName("Fingers");
    m.setLastName("Hurt");
    m.setJobTitle("office lackey");
    m.insertEmail("sql\%inje*tion@aardvark.com*:bind"); // silly test

    a.setUid(model.addContact(a));
    b.setUid(model.addContact(b));
    c.setUid(model.addContact(c));
    d.setUid(model.addContact(d));
    m.setUid(model.addContact(m));

    // Now try a few queries

    // Upper case A on firstname
    QVariant v = QVariant(QString("A"));
    QModelIndexList mil = model.match(QContactModel::FirstName, v, Qt::MatchCaseSensitive | Qt::MatchStartsWith, 0, -1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), a);

    mil = model.match(QContactModel::FirstName, v, Qt::MatchCaseSensitive | Qt::MatchContains, 0, -1);
    QCOMPARE(mil.count(), 2);
    QCOMPARE(model.contact(mil.value(0)), a);
    QCOMPARE(model.contact(mil.value(1)), c);

    mil = model.match(QContactModel::FirstName, v, Qt::MatchCaseSensitive | Qt::MatchExactly, 0, -1);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::FirstName, v, Qt::MatchCaseSensitive | Qt::MatchEndsWith, 0, -1);
    QCOMPARE(mil.count(), 0);

    // Lower case a on firstname
    v = QVariant(QString("a"));
    mil = model.match(QContactModel::FirstName, v, Qt::MatchCaseSensitive | Qt::MatchStartsWith, 0, -1);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::FirstName, v, Qt::MatchCaseSensitive | Qt::MatchContains, 0, -1);
    QCOMPARE(mil.count(), 4);
    QCOMPARE(model.contact(mil.value(0)), a);
    QCOMPARE(model.contact(mil.value(1)), c);
    QCOMPARE(model.contact(mil.value(2)), d);
    QCOMPARE(model.contact(mil.value(3)), m);

    mil = model.match(QContactModel::FirstName, v, Qt::MatchCaseSensitive | Qt::MatchExactly, 0, -1);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::FirstName, v, Qt::MatchCaseSensitive | Qt::MatchEndsWith, 0, -1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), m);

    // Case insensitive on firstname
    mil = model.match(QContactModel::FirstName, v, Qt::MatchStartsWith, 0, -1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), a);

    mil = model.match(QContactModel::FirstName, v, Qt::MatchContains, 0, -1);
    QCOMPARE(mil.count(), 4);
    QCOMPARE(model.contact(mil.value(0)), a);
    QCOMPARE(model.contact(mil.value(1)), c);
    QCOMPARE(model.contact(mil.value(2)), d);
    QCOMPARE(model.contact(mil.value(3)), m);

    mil = model.match(QContactModel::FirstName, v, Qt::MatchExactly, 0, -1);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::FirstName, v, Qt::MatchEndsWith, 0, -1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), m);

    // Now some Company matches (assume case sensitivity is ok after the above tests)
    v = QVariant(QString("Aardvark"));
    mil = model.match(QContactModel::Company, v, Qt::MatchExactly, 0, -1);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::Company, v, Qt::MatchContains, 0, -1);
    QCOMPARE(mil.count(), 2);
    QCOMPARE(model.contact(mil.value(0)), a);
    QCOMPARE(model.contact(mil.value(1)), b);

    mil = model.match(QContactModel::Company, v, Qt::MatchStartsWith, 0, -1);
    QCOMPARE(mil.count(), 2);
    QCOMPARE(model.contact(mil.value(0)), a);
    QCOMPARE(model.contact(mil.value(1)), b);

    mil = model.match(QContactModel::Company, v, Qt::MatchEndsWith, 0, -1);
    QCOMPARE(mil.count(), 0);

    // Email matches!
    v = QVariant(QString("@aardvark.com"));
    mil = model.match(QContactModel::Emails, v, Qt::MatchExactly, 0, -1);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::Emails, v, Qt::MatchContains, 0, -1);
    QCOMPARE(mil.count(), 3);
    QCOMPARE(model.contact(mil.value(0)), a);
    QCOMPARE(model.contact(mil.value(1)), b);
    QCOMPARE(model.contact(mil.value(2)), m);

    mil = model.match(QContactModel::Emails, v, Qt::MatchStartsWith, 0, -1);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::Emails, v, Qt::MatchEndsWith, 0, -1);
    QCOMPARE(mil.count(), 2);
    QCOMPARE(model.contact(mil.value(0)), a);
    QCOMPARE(model.contact(mil.value(1)), b);

    // Default email
    mil = model.match(QContactModel::DefaultEmail, v, Qt::MatchExactly, 0, -1);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::DefaultEmail, v, Qt::MatchContains, 0, -1);
    QCOMPARE(mil.count(), 2);
    QCOMPARE(model.contact(mil.value(0)), a);
    QCOMPARE(model.contact(mil.value(1)), m);

    mil = model.match(QContactModel::DefaultEmail, v, Qt::MatchStartsWith, 0, -1);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::DefaultEmail, v, Qt::MatchEndsWith, 0, -1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), a);

    // SQL escaping
    v = QVariant(QString("%"));
    mil = model.match(QContactModel::Emails, v, Qt::MatchExactly, 0, -1);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::Emails, v, Qt::MatchContains, 0, -1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), m);

    mil = model.match(QContactModel::Emails, v, Qt::MatchStartsWith, 0, -1);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::Emails, v, Qt::MatchEndsWith, 0, -1);
    QCOMPARE(mil.count(), 0);

    v = QVariant(QString("*"));
    mil = model.match(QContactModel::Emails, v, Qt::MatchExactly, 0, -1);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::Emails, v, Qt::MatchContains, 0, -1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), m);

    mil = model.match(QContactModel::Emails, v, Qt::MatchStartsWith, 0, -1);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::Emails, v, Qt::MatchEndsWith, 0, -1);
    QCOMPARE(mil.count(), 0);

    // wildcard matching [on '*']
    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 0, -1);
    QCOMPARE(mil.count(), 4);
    QCOMPARE(model.contact(mil.value(0)), a);
    QCOMPARE(model.contact(mil.value(1)), b);
    QCOMPARE(model.contact(mil.value(2)), d);
    QCOMPARE(model.contact(mil.value(3)), m);

    v = QVariant(QString("*aardvark"));
    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 0, -1);
    QCOMPARE(mil.count(), 0);

    v = QVariant(QString("*aardvark*"));
    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 0, -1);
    QCOMPARE(mil.count(), 3);
    QCOMPARE(model.contact(mil.value(0)), a);
    QCOMPARE(model.contact(mil.value(1)), b);
    QCOMPARE(model.contact(mil.value(2)), m);

    // Addresses
    v = QVariant(QString("Mansion Way"));
    mil = model.match(QContactModel::HomeStreet, v, Qt::MatchExactly, 0, -1);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::HomeStreet, v, Qt::MatchContains, 0, -1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), c);

    mil = model.match(QContactModel::HomeStreet, v, Qt::MatchStartsWith, 0, -1);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::HomeStreet, v, Qt::MatchEndsWith, 0, -1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), c);

    mil = model.match(QContactModel::BusinessStreet, v, Qt::MatchExactly, 0, -1);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::BusinessStreet, v, Qt::MatchContains, 0, -1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), d);

    mil = model.match(QContactModel::BusinessStreet, v, Qt::MatchStartsWith, 0, -1);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::BusinessStreet, v, Qt::MatchEndsWith, 0, -1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), d);

    // Label matching
    v = QVariant(QString("Aaron Aaronson"));
    mil = model.match(QContactModel::Label, v, Qt::MatchExactly, 0, -1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), a);

    mil = model.match(QContactModel::Label, v, Qt::MatchContains, 0, -1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), a);

    mil = model.match(QContactModel::Label, v, Qt::MatchStartsWith, 0, -1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), a);

    mil = model.match(QContactModel::Label, v, Qt::MatchEndsWith, 0, -1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), a);

    // Double check that the start/hit parameters work
    v = QVariant(QString("*aardvark*"));
    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 1, -1);
    QCOMPARE(mil.count(), 2);
    QCOMPARE(model.contact(mil.value(0)), b);
    QCOMPARE(model.contact(mil.value(1)), m);

    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 1, 2);
    QCOMPARE(mil.count(), 2);
    QCOMPARE(model.contact(mil.value(0)), b);
    QCOMPARE(model.contact(mil.value(1)), m);

    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 1, 3);
    QCOMPARE(mil.count(), 2);
    QCOMPARE(model.contact(mil.value(0)), b);
    QCOMPARE(model.contact(mil.value(1)), m);

    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 1, 1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), b);

    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 2, -1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), m);

    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 2, 1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(model.contact(mil.value(0)), m);

    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, -1, 0);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, -1, 1);
    QCOMPARE(mil.count(), 1);

    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 0, 0);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 1, 0);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 2, 0);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 3, 0);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 4, 0);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 4, 1);
    QCOMPARE(mil.count(), 1);

    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 5, 0);
    QCOMPARE(mil.count(), 0);

    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 5, 1);
    QCOMPARE(mil.count(), 0);

    // Now... set the sort order to something else
    v = QVariant(QString("*aardvark*"));
    QList<QContactModel::SortField> list;
    list << qMakePair(QContactModel::JobTitle, Qt::AscendingOrder);
    model.sort(list);
    mil = model.match(QContactModel::Emails, v, Qt::MatchWildcard, 0, -1);
    QCOMPARE(mil.count(), 3);

    QCOMPARE(model.contact(mil.value(0)), b);
    QCOMPARE(model.contact(mil.value(1)), m);
    QCOMPARE(model.contact(mil.value(2)), a);

    /* Business phone */
    // Reset to sorting on label
    list.clear();
    list << qMakePair(QContactModel::Label, Qt::AscendingOrder);
    model.sort(list);
    mil = model.match(QContactModel::BusinessPhone, QString("222"), Qt::MatchStartsWith, 0, -1);
    QCOMPARE(mil.count(), 2);

    QCOMPARE(model.contact(mil.value(0)), a);
    QCOMPARE(model.contact(mil.value(1)), b);
}

void tst_QContactModel::sort()
{
    QContactModel model;

    QContact a;
    a.setFirstName("Abigail");
    a.setLastName("Zanderhurst");

    QContact b;
    b.setFirstName("Barry");
    b.setLastName("Yowington");

    QContact c;
    c.setFirstName("Clarice");
    c.setLastName("Xercopopoulos");

    QContact d;
    d.setFirstName("David");
    d.setLastName("Wiltons");

    QContact e;
    e.setFirstName("Emily");
    e.setLastName("Wiltons");

    // Single named people
    QContact p;
    p.setFirstName("Dynamo"); // the supervillain/freedom fighter

    QContact y;
    y.setLastName("Yellow");
    y.setNameTitle("Mr."); // not used, but more polite

    a.setUid(model.addContact(a));
    b.setUid(model.addContact(b));
    c.setUid(model.addContact(c));
    d.setUid(model.addContact(d));
    e.setUid(model.addContact(e));
    p.setUid(model.addContact(p));
    y.setUid(model.addContact(y));

    QCOMPARE(model.rowCount(), 7);

    /* Explicitly set the label */
    QContact::setLabelFormat("firstname _ lastname | firstname | lastname");

    /* and the sort order */
    model.sort((int)QContactModel::FirstName);

    /* by first name should go yabcdpe */

    int row = 0;
    QCOMPARE(model.contact(row++), y);
    QCOMPARE(model.contact(row++), a);
    QCOMPARE(model.contact(row++), b);
    QCOMPARE(model.contact(row++), c);
    QCOMPARE(model.contact(row++), d);
    QCOMPARE(model.contact(row++), p);
    QCOMPARE(model.contact(row++), e);

    row = 0;
    QCOMPARE(model.contact(row++).label(), QString("Yellow"));
    QCOMPARE(model.contact(row++).label(), QString("Abigail Zanderhurst"));
    QCOMPARE(model.contact(row++).label(), QString("Barry Yowington"));
    QCOMPARE(model.contact(row++).label(), QString("Clarice Xercopopoulos"));
    QCOMPARE(model.contact(row++).label(), QString("David Wiltons"));
    QCOMPARE(model.contact(row++).label(), QString("Dynamo"));
    QCOMPARE(model.contact(row++).label(), QString("Emily Wiltons"));

    /* reversed firstname should be the opposite */
    model.sort((int)QContactModel::FirstName, Qt::DescendingOrder);
    row = 0;
    QCOMPARE(model.contact(row++), e);
    QCOMPARE(model.contact(row++), p);
    QCOMPARE(model.contact(row++), d);
    QCOMPARE(model.contact(row++), c);
    QCOMPARE(model.contact(row++), b);
    QCOMPARE(model.contact(row++), a);
    QCOMPARE(model.contact(row++), y);

    row = 0;
    QCOMPARE(model.contact(row++).label(), QString("Emily Wiltons"));
    QCOMPARE(model.contact(row++).label(), QString("Dynamo"));
    QCOMPARE(model.contact(row++).label(), QString("David Wiltons"));
    QCOMPARE(model.contact(row++).label(), QString("Clarice Xercopopoulos"));
    QCOMPARE(model.contact(row++).label(), QString("Barry Yowington"));
    QCOMPARE(model.contact(row++).label(), QString("Abigail Zanderhurst"));
    QCOMPARE(model.contact(row++).label(), QString("Yellow"));

    /* just by lastname should go p{de,ed}cyba - either d or e could be second, and Dynamo should be first (consider NULL first) */
    model.sort((int)QContactModel::LastName);

    row = 0;
    QCOMPARE(model.contact(row++), p);

    QContact c1 = model.contact(row++);
    QContact c2 = model.contact(row++);
    QVERIFY((c1 == d && c2 == e) || (c1 == e && c2 == d));

    QCOMPARE(model.contact(row++), c);
    QCOMPARE(model.contact(row++), y);
    QCOMPARE(model.contact(row++), b);
    QCOMPARE(model.contact(row++), a);

    /* but the label should be unaffected */
    QString l1 = c1.label();
    QString l2 = c2.label();

    row = 0;
    QCOMPARE(model.contact(row++).label(), QString("Dynamo"));
    QVERIFY((l1 == "David Wiltons" && l2 == "Emily Wiltons") || (l1 == "Emily Wiltons" && l2 == "David Wiltons"));
    row += 2;
    QCOMPARE(model.contact(row++).label(), QString("Clarice Xercopopoulos"));
    QCOMPARE(model.contact(row++).label(), QString("Yellow"));
    QCOMPARE(model.contact(row++).label(), QString("Barry Yowington"));
    QCOMPARE(model.contact(row++).label(), QString("Abigail Zanderhurst"));

    /* Ditto for reversed lastname */
    model.sort((int)QContactModel::LastName, Qt::DescendingOrder);

    row = 0;
    QCOMPARE(model.contact(row++), a);
    QCOMPARE(model.contact(row++), b);
    QCOMPARE(model.contact(row++), y);
    QCOMPARE(model.contact(row++), c);
    c1 = model.contact(row++);
    c2 = model.contact(row++);
    QVERIFY((c1 == d && c2 == e) || (c1 == e && c2 == d));
    QCOMPARE(model.contact(row++), p);

    /* but the label should be unaffected */
    l1 = c1.label();
    l2 = c2.label();

    row = 0;
    QCOMPARE(model.contact(row++).label(), QString("Abigail Zanderhurst"));
    QCOMPARE(model.contact(row++).label(), QString("Barry Yowington"));
    QCOMPARE(model.contact(row++).label(), QString("Yellow"));
    QCOMPARE(model.contact(row++).label(), QString("Clarice Xercopopoulos"));
    QVERIFY((l1 == "David Wiltons" && l2 == "Emily Wiltons") || (l1 == "Emily Wiltons" && l2 == "David Wiltons"));

    row += 2;
    QCOMPARE(model.contact(row++).label(), QString("Dynamo"));

    /* Sorting by last, first should resolve the ambiguity and give pdecyba */
    QList<QContactModel::SortField> list;
    list << qMakePair(QContactModel::LastName, Qt::AscendingOrder) << qMakePair(QContactModel::FirstName, Qt::AscendingOrder);
    model.sort(list);

    row = 0;
    QCOMPARE(model.contact(row++), p);
    QCOMPARE(model.contact(row++), d);
    QCOMPARE(model.contact(row++), e);
    QCOMPARE(model.contact(row++), c);
    QCOMPARE(model.contact(row++), y);
    QCOMPARE(model.contact(row++), b);
    QCOMPARE(model.contact(row++), a);

    row = 0;
    QCOMPARE(model.contact(row++).label(), QString("Dynamo"));
    QCOMPARE(model.contact(row++).label(), QString("David Wiltons"));
    QCOMPARE(model.contact(row++).label(), QString("Emily Wiltons"));
    QCOMPARE(model.contact(row++).label(), QString("Clarice Xercopopoulos"));
    QCOMPARE(model.contact(row++).label(), QString("Yellow"));
    QCOMPARE(model.contact(row++).label(), QString("Barry Yowington"));
    QCOMPARE(model.contact(row++).label(), QString("Abigail Zanderhurst"));

    /* and check that last asc, first desc also works (and gives pedcyba) */
    list.clear();
    list << qMakePair(QContactModel::LastName, Qt::AscendingOrder) << qMakePair(QContactModel::FirstName, Qt::DescendingOrder);
    model.sort(list);

    row = 0;
    QCOMPARE(model.contact(row++), p);
    QCOMPARE(model.contact(row++), e);
    QCOMPARE(model.contact(row++), d);
    QCOMPARE(model.contact(row++), c);
    QCOMPARE(model.contact(row++), y);
    QCOMPARE(model.contact(row++), b);
    QCOMPARE(model.contact(row++), a);

    row = 0;
    QCOMPARE(model.contact(row++).label(), QString("Dynamo"));
    QCOMPARE(model.contact(row++).label(), QString("Emily Wiltons"));
    QCOMPARE(model.contact(row++).label(), QString("David Wiltons"));
    QCOMPARE(model.contact(row++).label(), QString("Clarice Xercopopoulos"));
    QCOMPARE(model.contact(row++).label(), QString("Yellow"));
    QCOMPARE(model.contact(row++).label(), QString("Barry Yowington"));
    QCOMPARE(model.contact(row++).label(), QString("Abigail Zanderhurst"));

    /* Changing the label format should not change that */
    QContact::setLabelFormat("lastname , _ firstname | lastname | firstname");

    row = 0;
    QCOMPARE(model.rowCount(), 7);
    QCOMPARE(model.contact(row++), p);
    QCOMPARE(model.contact(row++), e);
    QCOMPARE(model.contact(row++), d);
    QCOMPARE(model.contact(row++), c);
    QCOMPARE(model.contact(row++), y);
    QCOMPARE(model.contact(row++), b);
    QCOMPARE(model.contact(row++), a);

    row = 0;
    QCOMPARE(model.contact(row++).label(), QString("Dynamo"));
    QCOMPARE(model.contact(row++).label(), QString("Wiltons, Emily"));
    QCOMPARE(model.contact(row++).label(), QString("Wiltons, David"));
    QCOMPARE(model.contact(row++).label(), QString("Xercopopoulos, Clarice"));
    QCOMPARE(model.contact(row++).label(), QString("Yellow"));
    QCOMPARE(model.contact(row++).label(), QString("Yowington, Barry"));
    QCOMPARE(model.contact(row++).label(), QString("Zanderhurst, Abigail"));

    /* if we sort by the label format.. the sort order should change when we change the format */
    /* for lastname, _ firstname, the order should be pdecyba */
    model.sort(QContactModel::Label);

    row = 0;
    QCOMPARE(model.contact(row++), p);
    QCOMPARE(model.contact(row++), d);
    QCOMPARE(model.contact(row++), e);
    QCOMPARE(model.contact(row++), c);
    QCOMPARE(model.contact(row++), y);
    QCOMPARE(model.contact(row++), b);
    QCOMPARE(model.contact(row++), a);

    row = 0;
    QCOMPARE(model.contact(row++).label(), QString("Dynamo"));
    QCOMPARE(model.contact(row++).label(), QString("Wiltons, David"));
    QCOMPARE(model.contact(row++).label(), QString("Wiltons, Emily"));
    QCOMPARE(model.contact(row++).label(), QString("Xercopopoulos, Clarice"));
    QCOMPARE(model.contact(row++).label(), QString("Yellow"));
    QCOMPARE(model.contact(row++).label(), QString("Yowington, Barry"));
    QCOMPARE(model.contact(row++).label(), QString("Zanderhurst, Abigail"));

    /* for firstname lastname, it should be abcdpey */
    QContact::setLabelFormat("firstname _ lastname | firstname | lastname");

    row = 0;
    QCOMPARE(model.contact(row++), a);
    QCOMPARE(model.contact(row++), b);
    QCOMPARE(model.contact(row++), c);
    QCOMPARE(model.contact(row++), d);
    QCOMPARE(model.contact(row++), p);
    QCOMPARE(model.contact(row++), e);
    QCOMPARE(model.contact(row++), y);

    row = 0;
    QCOMPARE(model.contact(row++).label(), QString("Abigail Zanderhurst"));
    QCOMPARE(model.contact(row++).label(), QString("Barry Yowington"));
    QCOMPARE(model.contact(row++).label(), QString("Clarice Xercopopoulos"));
    QCOMPARE(model.contact(row++).label(), QString("David Wiltons"));
    QCOMPARE(model.contact(row++).label(), QString("Dynamo"));
    QCOMPARE(model.contact(row++).label(), QString("Emily Wiltons"));
    QCOMPARE(model.contact(row++).label(), QString("Yellow"));

    // Now reverse the label - should be abycedp
    QContact::setLabelFormat("lastname , _ firstname | lastname | firstname");
    model.sort(QContactModel::Label, Qt::DescendingOrder);

    row = 0;
    QCOMPARE(model.contact(row++), a);
    QCOMPARE(model.contact(row++), b);
    QCOMPARE(model.contact(row++), y);
    QCOMPARE(model.contact(row++), c);
    QCOMPARE(model.contact(row++), e);
    QCOMPARE(model.contact(row++), d);
    QCOMPARE(model.contact(row++), p);

    row = 0;
    QCOMPARE(model.contact(row++).label(), QString("Zanderhurst, Abigail"));
    QCOMPARE(model.contact(row++).label(), QString("Yowington, Barry"));
    QCOMPARE(model.contact(row++).label(), QString("Yellow"));
    QCOMPARE(model.contact(row++).label(), QString("Xercopopoulos, Clarice"));
    QCOMPARE(model.contact(row++).label(), QString("Wiltons, Emily"));
    QCOMPARE(model.contact(row++).label(), QString("Wiltons, David"));
    QCOMPARE(model.contact(row++).label(), QString("Dynamo"));

    /* for firstname lastname, it should be yepdcba */
    QContact::setLabelFormat("firstname _ lastname | firstname | lastname");

    row = 0;
    QCOMPARE(model.contact(row++), y);
    QCOMPARE(model.contact(row++), e);
    QCOMPARE(model.contact(row++), p);
    QCOMPARE(model.contact(row++), d);
    QCOMPARE(model.contact(row++), c);
    QCOMPARE(model.contact(row++), b);
    QCOMPARE(model.contact(row++), a);

    row = 0;
    QCOMPARE(model.contact(row++).label(), QString("Yellow"));
    QCOMPARE(model.contact(row++).label(), QString("Emily Wiltons"));
    QCOMPARE(model.contact(row++).label(), QString("Dynamo"));
    QCOMPARE(model.contact(row++).label(), QString("David Wiltons"));
    QCOMPARE(model.contact(row++).label(), QString("Clarice Xercopopoulos"));
    QCOMPARE(model.contact(row++).label(), QString("Barry Yowington"));
    QCOMPARE(model.contact(row++).label(), QString("Abigail Zanderhurst"));
}

void tst_QContactModel::label()
{
    QContactModel cm;

    QContact a;
    a.setFirstName("Adam");
    a.setLastName("Smith");

    QContact::setLabelFormat("firstname _ lastname");
    QCOMPARE(a.label(), QString("Adam Smith"));

    QContact::setLabelFormat("lastname , _ firstname");
    QCOMPARE(a.label(), QString("Smith, Adam"));

    /* now add it to the model */

    a.setUid(cm.addContact(a));

    QVERIFY(cm.count() == 1);

    QCOMPARE(cm.data(cm.index(0,0), QContactModel::LabelRole).toString(), QString("<b>Smith, Adam</b>"));
    QCOMPARE(cm.data(cm.index(0, QContactModel::Label), Qt::DisplayRole).toString(), QString("Smith, Adam"));
    QCOMPARE(cm.contact(0).label(), QString("Smith, Adam"));

    QContact::setLabelFormat("firstname _ lastname");

    QCOMPARE(cm.data(cm.index(0,0), QContactModel::LabelRole).toString(), QString("<b>Adam Smith</b>"));
    QCOMPARE(cm.data(cm.index(0, QContactModel::Label), Qt::DisplayRole).toString(), QString("Adam Smith"));
    QCOMPARE(cm.contact(0).label(), QString("Adam Smith"));
}

void tst_QContactModel::matchEmailAddress()
{
    // set up two misses and a hit, + search term.  Ensure right item is found each time.

    QString miss1_a("abe@contact.com");
    QString miss1_b("acontact@freemail.net");
    QString miss2_a("henry@hamstersRock.org");
    QString miss2_b("henry@hamster.com");
    QString hit_a("bob@village.net");
    QString hit_b("bbuilder@construction.com");

    QString hitString("bbuilder@construction.com");
    QString missAllString("mtwain@writers.org");

    QContactModel m;
    QContact c;
    c.setFirstName("Miss1");
    c.setEmailList(QStringList());
    c.insertEmail(miss1_a);
    c.insertEmail(miss1_b);

    m.addContact(c);

    c.setFirstName("Miss2");
    c.setEmailList(QStringList());
    c.insertEmail(miss2_a);
    c.insertEmail(miss2_b);

    m.addContact(c);

    c.setFirstName("Hit");
    c.setEmailList(QStringList());
    c.insertEmail(hit_a);
    c.insertEmail(hit_b);

    QUniqueId hitId = m.addContact(c);

    QCOMPARE(m.matchEmailAddress(hitString).uid().toLocalContextString(), hitId.toLocalContextString());
    QVERIFY(m.matchEmailAddress(missAllString).uid().isNull());
}

void tst_QContactModel::containsChat()
{
    /* Add a number of contacts and make sure we can only see the chat ones */
    QStringList fields = QContactFieldDefinition::fields("chat");
    QContactFieldDefinition def(fields.value(0));

    QContact chat1;
    chat1.setFirstName("Alice");
    def.setValue(chat1, "test:alice");

    QContact chat2;
    chat2.setFirstName("Bob");
    def.setValue(chat2, "test:bob");

    QContact nochat1;
    nochat1.setFirstName("Carol");
    nochat1.setHomeVOIP("test:carol");

    QContact nochat2;
    nochat2.setFirstName("David");
    nochat2.setHomeVOIP("test:david");

    QContact nochat3;
    nochat3.setFirstName("Edward");
    nochat3.setBusinessPhone("sip:12345");

    QContactModel cm;
    chat1.setUid(cm.addContact(chat1));
    chat2.setUid(cm.addContact(chat2));
    nochat1.setUid(cm.addContact(nochat1));
    nochat2.setUid(cm.addContact(nochat2));
    nochat3.setUid(cm.addContact(nochat3));

    QCOMPARE(cm.rowCount(), 5);
    QCOMPARE(cm.contact(0), chat1);
    QCOMPARE(cm.contact(1), chat2);
    QCOMPARE(cm.contact(2), nochat1);
    QCOMPARE(cm.contact(3), nochat2);
    QCOMPARE(cm.contact(4), nochat3);

    cm.setFilter(QString(), QContactModel::ContainsChat);

    QCOMPARE(cm.rowCount(), 2);
    QCOMPARE(cm.contact(0), chat1);
    QCOMPARE(cm.contact(1), chat2);

    cm.clearFilter();

    QCOMPARE(cm.rowCount(), 5);

    cm.setFilter(QString(), QContactModel::ContainsPhoneNumber);

    QCOMPARE(cm.rowCount(), 3);
    QCOMPARE(cm.contact(0), nochat1);
    QCOMPARE(cm.contact(1), nochat2);
    QCOMPARE(cm.contact(2), nochat3);
}

void tst_QContactModel::matchChat()
{
    QStringList fields = QContactFieldDefinition::fields("chat");
    QContactFieldDefinition def(fields.value(0));

    QContact chat1;
    chat1.setFirstName("Alice");
    def.setValue(chat1, "alice");

    QContact chat2;
    chat2.setFirstName("Bob");
    def.setValue(chat2, "bob");

    QContact nochat1;
    nochat1.setFirstName("Carol");
    nochat1.setHomeVOIP("test:carol");

    QContact nochat2;
    nochat2.setFirstName("David");
    nochat2.setHomeVOIP("test:david");

    QContact nochat3;
    nochat3.setFirstName("Edward");
    nochat3.setBusinessPhone("sip:12345");

    QContactModel cm;
    chat1.setUid(cm.addContact(chat1));
    chat2.setUid(cm.addContact(chat2));
    nochat1.setUid(cm.addContact(nochat1));
    nochat2.setUid(cm.addContact(nochat2));
    nochat3.setUid(cm.addContact(nochat3));

    QCOMPARE(cm.rowCount(), 5);
    QCOMPARE(cm.contact(0), chat1);
    QCOMPARE(cm.contact(1), chat2);
    QCOMPARE(cm.contact(2), nochat1);
    QCOMPARE(cm.contact(3), nochat2);
    QCOMPARE(cm.contact(4), nochat3);

    QCOMPARE(cm.matchChatAddress("alice"), chat1);
    QCOMPARE(cm.matchChatAddress("bob"), chat2);
    QCOMPARE(cm.matchChatAddress("test:carol"), QContact());
    QCOMPARE(cm.matchChatAddress("test:david"), QContact());
    QCOMPARE(cm.matchChatAddress("sip:12345"), QContact());
}

void tst_QContactModel::presence()
{
    QContactModel cm;

    /* Add several contacts */
    QContact a;
    a.setFirstName("Alice");

    QContact b;
    b.setFirstName("Bob");

    QContact c;
    c.setFirstName("Carol");

    QContact d;
    d.setFirstName("David");

    QContact e;
    e.setFirstName("Elizabeth");

    /* Now add some presence URIs */
    a.setHomeVOIP("test:alice");
    b.setHomeVOIP("test:bob");
    c.setHomeVOIP("test:carol");
    c.setBusinessVOIP("test:supportlist");
    d.setBusinessVOIP("test:supportlist");
    e.setHomePhone("12345");

    a.setUid(cm.addContact(a));
    b.setUid(cm.addContact(b));
    c.setUid(cm.addContact(c));
    d.setUid(cm.addContact(d));
    e.setUid(cm.addContact(e));

    cm.sort(QContactModel::Label);

    QCOMPARE(cm.count(), 5);

    // Make sure there is no presence information left over from contactField
    QSqlQuery q;
    q.prepare("DELETE FROM contactpresence");
    q.exec();

    // at this point, there should be no presence information
    // setting the filter to include status none should include everything
    cm.setPresenceFilter(QList<QCollectivePresenceInfo::PresenceType>() << QCollectivePresenceInfo::None);

    QCOMPARE(cm.count(), 5);

    // Set some presence information (no Bob)
    // This is normally done by a server task when the presence provider updates presence.
    // but do it manually for the purposes of this test
    QCollectivePresenceInfo pi_alice;
    QCollectivePresenceInfo pi_carol;
    QCollectivePresenceInfo pi_support;

    pi_alice.setUri("test:alice");
    pi_alice.setPresence("Online", QCollectivePresenceInfo::Online);
    pi_alice.setMessage("Available");
    pi_alice.setDisplayName("AliceCool73");
    pi_alice.setLastUpdateTime(QDateTime::currentDateTime());
    pi_alice.setCapabilities(QStringList() << "voice" << "email" << "video");

    pi_carol.setUri("test:carol");
    pi_carol.setPresence("Offline", QCollectivePresenceInfo::Offline);
    pi_carol.setDisplayName(QString());
    pi_carol.setCapabilities(QStringList());

    pi_support.setUri("test:supportlist");
    pi_support.setPresence("Online", QCollectivePresenceInfo::Online);
    pi_support.setDisplayName("ProductSupport");
    pi_support.setCapabilities(QStringList() << "email" << "im");

    pushPresence("test", QList<QCollectivePresenceInfo>() << pi_alice << pi_carol << pi_support);

    // Set the filter for online
    cm.setPresenceFilter(QList<QCollectivePresenceInfo::PresenceType>() << QCollectivePresenceInfo::Online);
    QCOMPARE(cm.count(), 3);
    QCOMPARE(cm.contact(0), a);
    QCOMPARE(cm.contact(1), c);
    QCOMPARE(cm.contact(2), d);

    // No 'away' contacts
    cm.setPresenceFilter(QList<QCollectivePresenceInfo::PresenceType>() << QCollectivePresenceInfo::Away);
    QCOMPARE(cm.count(), 0);

    // now clear it again, for testing matching
    cm.clearPresenceFilter();
    QCOMPARE(cm.count(), 5);

    // Now try some matching
    QModelIndexList mil = cm.match(QContactModel::PresenceStatus, QCollectivePresenceInfo::Online, Qt::MatchExactly, 0, -1);
    QCOMPARE(mil.count(), 3);
    QCOMPARE(cm.count(), 5);
    QCOMPARE(cm.contact(mil.at(0)), a);
    QCOMPARE(cm.contact(mil.at(1)), c);
    QCOMPARE(cm.contact(mil.at(2)), d);

    mil = cm.match(QContactModel::PresenceMessage, QString("Available"), Qt::MatchExactly, 0, -1);
    QCOMPARE(mil.count(), 1);
    QCOMPARE(cm.contact(mil.at(0)), a);

    // Now some modifications to presence state
    cm.setPresenceFilter(QList<QCollectivePresenceInfo::PresenceType>() << QCollectivePresenceInfo::Online);

    QCOMPARE(cm.count(), 3);
    QCOMPARE(cm.contact(0), a);
    QCOMPARE(cm.contact(1), c);
    QCOMPARE(cm.contact(2), d);

    pi_carol.setPresence("Online", QCollectivePresenceInfo::Online);
    pushPresence("test", QList<QCollectivePresenceInfo>() << pi_alice << pi_carol << pi_support);

    QCOMPARE(cm.count(), 3);
    QCOMPARE(cm.contact(0), a);
    QCOMPARE(cm.contact(1), c);
    QCOMPARE(cm.contact(2), d);

    pi_support.setPresence("Offline", QCollectivePresenceInfo::Offline);
    pushPresence("test", QList<QCollectivePresenceInfo>() << pi_alice << pi_carol << pi_support);

    QCOMPARE(cm.count(), 2);
    QCOMPARE(cm.contact(0), a);
    QCOMPARE(cm.contact(1), c);

    QContact f;
    f.setFirstName("Frederick");
    f.setBusinessVOIP("test:supportlist");
    f.setUid(cm.addContact(f));

    QCOMPARE(cm.count(), 2);
    QCOMPARE(cm.contact(0), a);
    QCOMPARE(cm.contact(1), c);

    pi_support.setPresence("Online", QCollectivePresenceInfo::Online);
    pushPresence("test", QList<QCollectivePresenceInfo>() << pi_alice << pi_carol << pi_support);

    QCOMPARE(cm.count(), 4);
    QCOMPARE(cm.contact(0), a);
    QCOMPARE(cm.contact(1), c);
    QCOMPARE(cm.contact(2), d);
    QCOMPARE(cm.contact(3), f);

    /* Try some cross model propagation */
    QContactModel cm2;
    QContact g;
    g.setFirstName("Gary");
    g.setBusinessVOIP("test:nonexistent");
    g.setUid(cm2.addContact(g));

    QCOMPARE(cm.count(), 4);
    QCOMPARE(cm.contact(0), a);
    QCOMPARE(cm.contact(1), c);
    QCOMPARE(cm.contact(2), d);
    QCOMPARE(cm.contact(3), f);

    QCollectivePresenceInfo pi_nonexistent;
    pi_nonexistent.setUri("test:nonexistent");
    pi_nonexistent.setPresence("Online", QCollectivePresenceInfo::Online);
    pushPresence("test", QList<QCollectivePresenceInfo>() << pi_alice << pi_carol << pi_support << pi_nonexistent);

    QCOMPARE(cm.count(), 5);
    QCOMPARE(cm.contact(0), a);
    QCOMPARE(cm.contact(1), c);
    QCOMPARE(cm.contact(2), d);
    QCOMPARE(cm.contact(3), f);
    QCOMPARE(cm.contact(4), g);
}

void tst_QContactModel::pushPresence(const QString& provider, QList<QCollectivePresenceInfo> presences)
{
    /* Copy of code in buddysyncer */
    QSqlDatabase db = QPimSqlIO::database();

    QSqlQuery deletePresence("DELETE FROM contactpresence WHERE uri=:u", db);
    QSqlQuery setPhonePresence("REPLACE INTO contactpresence(recid,uri,status,statusstring,message,displayname,updatetime,capabilities,avatar) SELECT contactphonenumbers.recid,:u,:t,:st,:m,:dn,:ut,:cap,:a FROM contactphonenumbers WHERE contactphonenumbers.phone_number=:ph", db);
    QSqlQuery stashPresence("REPLACE INTO contactpresence(recid,uri,status,statusstring,message,displayname,updatetime,capabilities,avatar) VALUES(0,:u,:t,:st,:m,:dn,:ut,:cap,:a)", db);
    QSqlQuery setCustomPresence("REPLACE INTO contactpresence(recid,uri,status,statusstring,message,displayname,updatetime,capabilities,avatar) SELECT contactcustom.recid,:u,:t,:st,:m,:dn,:ut,:cap,:a FROM contactcustom WHERE contactcustom.fieldname=:cn AND contactcustom.fieldvalue=:fv", db);

    /* Early out if no presences.. */
    if (presences.count() == 0)
        return;

    /* Start a transaction .. */
    bool commit = db.transaction();

    /* Pesky custom fields */
    QStringList fields = QContactFieldDefinition::fields("chat");
    QList<QContactFieldDefinition> customFields;

    foreach(QString field, fields) {
        QContactFieldDefinition def(field);
        if (def.provider() == provider) {
            QContactModel::Field f = QContactModel::identifierField(def.id());
            /* We assume that valid fields are handled by the contactphonenumber query... */
            if (f == QContactModel::Invalid)
                customFields.append(def);
        }
    }

    /* Now create them all */
    foreach (QCollectivePresenceInfo info, presences) {
        if (!info.isNull()) {
            QString uri = QCollective::encodeUri(provider,info.uri());

            deletePresence.bindValue(":u", uri);
            deletePresence.exec();

            /* If the type is "None", we don't store it to the db */
            if (info.presenceType() != QCollectivePresenceInfo::None) {
                setPhonePresence.bindValue(":u", uri);
                setPhonePresence.bindValue(":t", info.presenceType());
                setPhonePresence.bindValue(":st", info.presence());
                setPhonePresence.bindValue(":m", info.message());
                setPhonePresence.bindValue(":dn", info.displayName());
                setPhonePresence.bindValue(":ut", info.lastUpdateTime());
                setPhonePresence.bindValue(":cap", info.capabilities().join(QLatin1String(",")));
                setPhonePresence.bindValue(":a", info.avatar());
                setPhonePresence.bindValue(":ph", info.uri()); // Phones match on unencoded URI for SIP compatibility
                setPhonePresence.exec();

                foreach(QContactFieldDefinition def, customFields) {
                    /* Check that this field definition has the same provider */
                    if (def.provider() == provider) {
                        setCustomPresence.bindValue(":u", uri);
                        setCustomPresence.bindValue(":t", info.presenceType());
                        setCustomPresence.bindValue(":st", info.presence());
                        setCustomPresence.bindValue(":m", info.message());
                        setCustomPresence.bindValue(":dn", info.displayName());
                        setCustomPresence.bindValue(":ut", info.lastUpdateTime());
                        setCustomPresence.bindValue(":cap", info.capabilities().join(QLatin1String(",")));
                        setCustomPresence.bindValue(":a", info.avatar());
                        setCustomPresence.bindValue(":cn", def.id()); // XXX this is the custom field name
                        setCustomPresence.bindValue(":fv", info.uri());
                        setCustomPresence.exec();
                    }
                }

                stashPresence.bindValue(":u", uri);
                stashPresence.bindValue(":t", info.presenceType());
                stashPresence.bindValue(":st", info.presence());
                stashPresence.bindValue(":m", info.message());
                stashPresence.bindValue(":dn", info.displayName());
                stashPresence.bindValue(":ut", info.lastUpdateTime());
                stashPresence.bindValue(":cap", info.capabilities().join(QLatin1String(",")));
                stashPresence.bindValue(":a", info.avatar());
                stashPresence.exec();
            }
        }
    }

    if (commit) {
        db.commit();
    }

    mVSObject->setAttribute(QLatin1String("SerialNumber"), ++mVSValue);
    QValueSpaceObject::sync();
}

#ifdef QCONTACTMODEL_PERFTEST
const int OVERLOOPS = 3;
const int MAXLOOPS = 10;
const int MAXCOUNT = 1000;

class TimeReporter
{
    public:
        TimeReporter(QString s)
            : message(s)
        {
            start = QTime::currentTime();
        }

        ~TimeReporter()
        {
            unsigned long ms = start.msecsTo(QTime::currentTime());
            qDebug() << message << ":" << ms << "ms";
            mStats.insert(message, ms);
        }

        static void printStats()
        {
            QList<QString> keys = mStats.uniqueKeys();

            foreach (QString key, keys) {
                QList<unsigned long> values = mStats.values(key);

                unsigned long min = 500000000;
                unsigned long max = 0;
                unsigned long sum = 0;
                double ave = 0;
                double std = 0;

                foreach(unsigned long v, values) {
                    if (v < min)
                        min = v;
                    if (v > max)
                        max = v;
                    sum += v;
                }

                ave = sum / values.count();
                foreach(unsigned long v, values) {
                    std += ((double)v - ave) * ((double)v - ave);
                }

                std /= values.count();
                std = sqrt(std);

                qDebug() << QString("%1: %2 samples, %3/%4/%5ms(min/avg/max) [ave %6us/contact], stdev %7ms").arg(key).arg(values.count()).arg(min).arg(ave,0,'f',1).arg(max).arg((1000 * ave)/(MAXCOUNT * MAXLOOPS * OVERLOOPS), 0, 'f', 1).arg(std,0,'f',1);
            }
        }

        static void clearStats()
        {
            mStats.clear();
        }

    private:
        QString message;
        QTime start;
        static QMultiMap<QString, unsigned long> mStats;
};

QMultiMap<QString, unsigned long> TimeReporter::mStats;

QString randomString(int length)
{
    QString ret;

    length += (rand() % (length/2));
    while (length-- > 0)
        ret.append(QChar("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVQXYZ01234567890 -'"[rand() % 65]));

    return ret;
}

typedef struct {
    QString label;
    QString photo;
    QString number;
} simpleContact;

/*
    Copy of output from ContactSqlIO::sqlLabel() for a representative labelFormat,
    something like:

        title _ firstname _ lastname | firstname _ lastname | title _ firstname | firstname | title _ lastname | lastname | company

*/
static QString sqlLabel()
{
    return QString("(CASE WHEN title IS NOT NULL AND firstname IS NOT NULL AND lastname IS NOT NULL THEN title || ' ' || firstname || ' ' || lastname WHEN firstname IS NOT NULL AND lastname IS NOT NULL THEN firstname || ' ' || lastname WHEN title IS NOT NULL AND firstname IS NOT NULL THEN title || ' ' || firstname WHEN firstname IS NOT NULL THEN firstname WHEN title IS NOT NULL AND lastname IS NOT NULL THEN title || ' ' || lastname WHEN lastname IS NOT NULL THEN lastname WHEN company IS NOT NULL THEN company ELSE NULL END)");
}


void tst_QContactModel::perf()
{
    /* Create a lot of contacts */
    QContactModel cm;
    QList<QUniqueId> uidList;
    QVector<QUniqueId> uidVector;

    QSqlDatabase db = QPimSqlIO::database();

    /* Simple cache */
    QMap<QUniqueId, simpleContact> simpleContacts;

    qDebug() << "Creating" << MAXCOUNT << "contacts.";
    for (int i=0; i < MAXCOUNT; i++) {
        QContact c;
        SET_ALL(c, "Contact");
        // Generate some random names
        c.setFirstName(randomString(8));
        c.setLastName(randomString(12));
        c.setCompany(QString());
        c.setJobTitle(QString());
        c.setNotes(QString());
        QUniqueId id = cm.addContact(c);
        simpleContact sc;
        sc.label = c.label();
        sc.photo = c.portraitFile();
        sc.number = c.defaultPhoneNumber();
        simpleContacts.insert(id, sc);
    }

    QCOMPARE(cm.count(), MAXCOUNT);

    qDebug() << "Extracting UID in order";
    /* Set up our list and vector */
    for (int i=0; i < MAXCOUNT; i++) {
        QUniqueId id = cm.id(i);
        uidList.append(id);
        uidVector.append(id);
    }

    /* Now try to do some performance tests */
    TimeReporter::clearStats();
    for (int outerloop = 0; outerloop < OVERLOOPS; outerloop++) {
        qDebug() << "Starting outer loop " << (outerloop + 1) << " of " << OVERLOOPS;
        /* Loops of accessing each contact in a row */
        int loop;
        int idx;
        QContact c;
        QVariant v;

        {
            TimeReporter t(" 1. Plain loop");
            for (loop = 0; loop < MAXLOOPS; loop++) {
                for (idx = 0; idx < MAXCOUNT; idx++) {
                    c = cm.contact(idx);
                    v = c.label();
                }
            }
        }

        {
            TimeReporter t(" 2. List sequential");
            /* Try accessing our ids from the list */
            for (loop = 0; loop < MAXLOOPS; loop++) {
                for (idx = 0; idx < MAXCOUNT; idx++) {
                    c = cm.contact(uidList.at(idx));
                    v = c.label();
                }
            }
        }

        /* Try just the label */
        {
            TimeReporter t(" 4. Looped label access");
            for (loop = 0; loop < MAXLOOPS; loop++) {
                for (idx = 0; idx < MAXCOUNT; idx++) {
                    v = cm.data(cm.index(idx, 0), QContactModel::LabelRole);
                }
            }
        }

        /* Simple cache */
        {
            TimeReporter t(" 6. Simple cache structure using list");
            for (loop = 0; loop < MAXLOOPS; loop++) {
                for (idx = 0; idx < MAXCOUNT; idx++) {
                    v = simpleContacts[uidList.at(idx)].label;
                }
            }
        }
    }
    qDebug() << " ===== Final results =====";
    TimeReporter::printStats();
}

void tst_QContactModel::labelPerf()
{
    /* Create a lot of contacts */
    QContactModel cm;
    QList<QUniqueId> uidList;
    QVector<QUniqueId> uidVector;

    QSqlDatabase db = QPimSqlIO::database();

    /* Simple cache */
    QMap<QUniqueId, simpleContact> simpleContacts;

    qDebug() << "Creating" << MAXCOUNT << "contacts.";
    for (int i=0; i < MAXCOUNT; i++) {
        QContact c;
        SET_ALL(c, "Contact");
        // Generate some random names
        c.setFirstName(randomString(8));
        c.setLastName(randomString(12));
        c.setCompany(QString());
        c.setJobTitle(QString());
        c.setNotes(QString());
        QUniqueId id = cm.addContact(c);
        simpleContact sc;
        sc.label = c.label();
        sc.photo = c.portraitFile();
        sc.number = c.defaultPhoneNumber();
        simpleContacts.insert(id, sc);
    }

    QCOMPARE(cm.count(), MAXCOUNT);

    TimeReporter::clearStats();
    for (int outerloop = 0; outerloop < OVERLOOPS; outerloop++) {
        qDebug() << "Starting outer loop " << (outerloop + 1) << " of " << OVERLOOPS;
        /* Loops of accessing each contact in a row */
        int loop;
        /* Try various label update procedures */
        {
            TimeReporter t(" 7. Update label, no transaction");
            for (loop = 0; loop < MAXLOOPS; loop++) {
                QSqlQuery q(db);
                q.prepare("UPDATE contacts SET label=" + sqlLabel());
                if(!q.exec())
                    qFatal("Failed to update: %s", q.lastError().text().toLatin1().constData());
            }
        }
        {
            TimeReporter t(" 8. Update label, with transaction");
            for (loop = 0; loop < MAXLOOPS; loop++) {
                db.transaction();
                QSqlQuery q(db);
                q.prepare("UPDATE contacts SET label=" + sqlLabel());
                if(!q.exec())
                    qFatal("Failed to update: %s", q.lastError().text().toLatin1().constData());
                db.commit();
            }
        }
        {
            TimeReporter t(" 9. Drop index, update label, recreate index, transaction");
            for (loop = 0; loop < MAXLOOPS; loop++) {
                db.transaction();
                QSqlQuery q(db);
                q.prepare("DROP INDEX contactslabelindex");
                if(!q.exec())
                    qFatal("Failed to drop index: %s", q.lastError().text().toLatin1().constData());
                q.prepare("UPDATE contacts SET label=" + sqlLabel());
                if(!q.exec())
                    qFatal("Failed to update: %s", q.lastError().text().toLatin1().constData());
                q.prepare("CREATE INDEX contactslabelindex ON contacts(label)");
                if(!q.exec())
                    qFatal("Failed to crate index: %s", q.lastError().text().toLatin1().constData());
                db.commit();
            }
        }
    }
    qDebug() << " ===== Final results =====";
    TimeReporter::printStats();
}

void tst_QContactModel::presPerf()
{
    /* Create a lot of contacts */
    QContactModel cm;

    QSqlDatabase db = QPimSqlIO::database();

    // create the contactpresence tables (one combined, and one split)
    db.exec("CREATE TABLE 'contactpresence1'(uri NVARCHAR(255) COLLATE NOCASE, recid INTEGER NOT NULL, status INTEGER, message NVARCHAR(255), FOREIGN KEY(recid) REFERENCES contacts(recid), PRIMARY KEY(uri,recid))");

    // and the map table
    db.exec("CREATE TABLE 'contactpresence2'(uri NVARCHAR(255) COLLATE NOCASE, status INTEGER, message NVARCHAR(255), PRIMARY KEY(uri))");
    db.exec("CREATE TABLE 'contactpresencemap'(recid INTEGER NOT NULL, uri NVARCHAR(255) COLLATE NOCASE, FOREIGN KEY(recid) REFERENCES contacts(recid), FOREIGN KEY(uri) REFERENCES contactpresence2(uri))");

    // indices
    db.exec("CREATE INDEX cpuri ON contactpresence1(uri,recid)");
    db.exec("CREATE INDEX cprecid ON contactpresence1(recid,uri)");
    db.exec("CREATE INDEX cp2uri ON contactpresence2(uri)");
    db.exec("CREATE INDEX cpmidx ON contactpresencemap(recid,uri)");
    db.exec("CREATE INDEX cpmidx2 ON contactpresencemap(uri,recid)");
    qDebug() << "Creating" << MAXCOUNT << "contacts.";
    int nopresence = 0;
    int onepresence = 0;
    int twopresence = 0;
    int threepresence = 0;

    for (int i=0; i < MAXCOUNT; i++) {
        QContact c;
        // Generate some random names
        c.setFirstName(randomString(8));

        // 50% of contacts have one presence
        // 25% have two
        // 1/16th of our contacts have the same presence URI as another contact

        int numpresence = 0;
        QString uri = QString::number(i);
        if (i%2 == 1) {
            c.setHomePhone(uri);
            numpresence++;
        }
        if (i%4 == 3) {
            c.setBusinessPhone(uri + "bus");
            numpresence++;
        }
        if (i%16 == 15) {
            c.setHomeVOIP(QString("shareduri%1").arg(i/64)); // shared between 4 contacts
            numpresence++;
        }

        if (numpresence == 0 )
            nopresence++;
        else if (numpresence == 1)
            onepresence++;
        else if (numpresence == 2)
            twopresence++;
        else if (numpresence == 3)
            threepresence++;

        QString id = QString::number(cm.addContact(c).toUInt());

        // Now add presence information
        if (!c.homePhone().isEmpty()) {
            db.exec("INSERT INTO contactpresence1(recid,uri,status,message) VALUES(" + id +",'" + c.homePhone() + "',1,'Online and kicking')");
            db.exec("INSERT INTO contactpresence2(uri,status,message) VALUES('" + c.homePhone() + "',1, 'Online and kicking')");
            db.exec("INSERT INTO contactpresencemap(recid,uri) VALUES(" + id + ",'" + c.homePhone() + "')");
        }
        if (!c.businessPhone().isEmpty()) {
            db.exec("INSERT INTO contactpresence1(recid,uri,status,message) VALUES(" + id +",'" + c.businessPhone() + "',1,'Online and kicking')");
            db.exec("INSERT INTO contactpresence2(uri,status,message) VALUES('" + c.businessPhone() + "',1, 'Online and kicking')");
            db.exec("INSERT INTO contactpresencemap(recid,uri) VALUES(" + id + ",'" + c.businessPhone() + "')");
        }
        if (!c.homeVOIP().isEmpty()) {
            db.exec("INSERT INTO contactpresence1(recid,uri,status,message) VALUES(" + id +",'" + c.homeVOIP() + "',1,'Online and kicking')");
            // We should only add this once per URI.. but ignore the error message
            db.exec("INSERT INTO contactpresence2(uri,status,message) VALUES('" + c.homeVOIP() + "',1, 'Online and kicking')");
            db.exec("INSERT INTO contactpresencemap(recid,uri) VALUES(" + id + ",'" + c.homeVOIP() + "')");
        }
    }

    // Now test!
    TimeReporter::clearStats();
    for (int outerloop = 0; outerloop < OVERLOOPS; outerloop++) {
        qDebug() << "Starting outer loop " << (outerloop + 1) << " of " << OVERLOOPS;
        int loop;
        /* Try various selections */
        {
            TimeReporter t("1. Rows with both URI and recid - contacts with no presence information");
            for (loop = 0; loop < MAXLOOPS; loop++) {
                int count = 0;
                QSqlQuery q(db);
                q.prepare("SELECT contacts.recid,contacts.label FROM contacts LEFT JOIN contactpresence1 ON (contacts.recid=contactpresence1.recid) WHERE contactpresence1.recid IS NULL");
                q.exec();

                while (q.next()) {
                    ++count;
                }

                QCOMPARE(count, nopresence);
            }
        }
        {
            TimeReporter t("2. Map and presence table [subselect] - contacts with no presence information");
            for (loop = 0; loop < MAXLOOPS; loop++) {
                int count = 0;
                QSqlQuery q(db);
                q.prepare("SELECT contacts.label,contacts.recid FROM contacts LEFT JOIN (SELECT contactpresencemap.recid as t2 FROM contactpresencemap INNER JOIN contactpresence2 ON (contactpresencemap.uri=contactpresence2.uri)) ON contacts.recid=t2 WHERE t2 IS NULL");
                q.exec();

                while (q.next()) {
                    ++count;
                }

                QCOMPARE(count, nopresence);
            }
        }
        {
            TimeReporter t("2b. Map and presence table [two left joins]- contacts with no presence information");
            for (loop = 0; loop < MAXLOOPS; loop++) {
                int count = 0;
                QSqlQuery q(db);
                q.prepare("SELECT contacts.label,contacts.recid,contactpresencemap.uri FROM contacts LEFT JOIN contactpresencemap ON contacts.recid=contactpresencemap.recid LEFT JOIN contactpresence2 ON contactpresencemap.uri=contactpresence2.uri WHERE contactpresence2.uri IS NULL");
                q.exec();

                while (q.next()) {
                    ++count;
                }

                QCOMPARE(count, nopresence);
            }
        }
        {
            TimeReporter t("3. Just presence table (subselect)- contacts with no presence information");
            for (loop = 0; loop < MAXLOOPS; loop++) {
                int count = 0;
                QSqlQuery q(db);
                q.prepare("SELECT contacts.label,contacts.recid FROM contacts LEFT JOIN (SELECT contactphonenumbers.recid AS t2 FROM contactphonenumbers INNER JOIN contactpresence2 ON (contactphonenumbers.phone_number=contactpresence2.uri)) ON (contacts.recid=t2) WHERE t2 IS NULL");
                q.exec();
                while (q.next()) {
                    ++count;
                }

                QCOMPARE(count, nopresence);
            }
        }
        {
            TimeReporter t("3b. Just presence table (two left joins - with group by) - contacts with no presence information");
            for (loop = 0; loop < MAXLOOPS; loop++) {
                int count = 0;
                QSqlQuery q(db);
                q.prepare("SELECT contacts.label,contacts.recid FROM contacts LEFT JOIN contactphonenumbers ON (contactphonenumbers.recid=contacts.recid) LEFT JOIN contactpresence2 ON (contactphonenumbers.phone_number=contactpresence2.uri) GROUP BY contacts.recid HAVING contactpresence2.uri IS NULL");
                q.exec();
                while (q.next()) {
                    ++count;
                }

                QCOMPARE(count, nopresence);
            }
        }
        {
            TimeReporter t("3c. Just presence table (two left joins)- contacts with no presence information");
            for (loop = 0; loop < MAXLOOPS; loop++) {
                int count = 0;
                QSqlQuery q(db);
                q.prepare("SELECT contacts.label,contacts.recid,contactpresence2.uri FROM contacts LEFT JOIN contactphonenumbers ON contacts.recid=contactphonenumbers.recid LEFT JOIN contactpresence2 ON contactphonenumbers.phone_number=contactpresence2.uri WHERE contactpresence2.uri IS NULL");
                q.exec();
                while (q.next()) {
                    ++count;
                }

                QCOMPARE(count, nopresence);
            }
        }
    }
    qDebug() << " ===== Final results =====";
    TimeReporter::printStats();
}
#endif // QCONTACTMODEL_PERFTESTS

