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
#include <QContact>



//TESTED_CLASS=QContact
//TESTED_FILES=src/libraries/qtopiapim/qcontact.h,src/libraries/qtopiapim/qcontact.cpp

/*
    This class is a unit test for the QContact class.
*/
class tst_QContact : public QObject
{
    Q_OBJECT

private slots:
    void match();

    void copy();

    void addressSetGet();

};

QTEST_APP_MAIN( tst_QContact, QtopiaApplication )
#include "tst_qcontact.moc"

/* Set the contents of QContact C to unique strings based on S. */
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
    C.setAnniversary(QDate(100,1,1).addDays(QString(S).size())); \
    C.setAssistant("assistant " S); \
    C.setBirthday(QDate(100,1,1).addDays(QString(S).size()*2));  \
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
    QCOMPARE(C.anniversary(),QDate(100,1,1).addDays(QString(S).size())); \
    QCOMPARE(C.assistant(), QString("assistant " S)); \
    QCOMPARE(C.birthday(), QDate(100,1,1).addDays(QString(S).size()*2));  \
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
    Ensure copy constructor and assignment operator work correctly and
    copy all data.  Also ensure the == operator works as expected.
*/
void tst_QContact::copy()
{

    /* Ensure == works correctly */
    QContact c1, c2, c3;
    SET_ALL(c1, "frank");
    SET_ALL(c2, "frank");
    QVERIFY(c1 == c2);
    SET_ALL(c3, "jones");
    QVERIFY(c1 != c3);

    VERIFY_ALL(c1, "frank");
    VERIFY_ALL(c2, "frank");
    VERIFY_ALL(c3, "jones");

    QContact copy1(c1);
    QContact copy2(c2);
    QContact copy3(c3);
    QContact assn1 = c1;
    QContact assn2 = c2;
    QContact assn3 = c3;

    QVERIFY(c1 == copy1);
    QVERIFY(c1 == assn1);
    QVERIFY(c2 == copy2);
    QVERIFY(c2 == assn2);
    QVERIFY(c3 == copy3);
    QVERIFY(c3 == assn3);

    VERIFY_ALL(copy1, "frank");
    VERIFY_ALL(copy2, "frank");
    VERIFY_ALL(copy3, "jones");
    VERIFY_ALL(assn1, "frank");
    VERIFY_ALL(assn2, "frank");
    VERIFY_ALL(assn3, "jones");

    c1.setFirstName("not frank");
    QVERIFY(c1 != copy1);
    QVERIFY(c1 != copy2);
    QVERIFY(c1 != copy3);
}

/*?
    Set and get addresses using several different
    methods and ensure the class consistently returns
    the correct addresses.
*/
void tst_QContact::addressSetGet()
{
    /* The tested contact. */
    QContact c;

    QContactAddress a;

    a.city = "Foo City";
    a.country = "Republic of Foo";
    a.state = "Fooland";
    a.street = "123 Foo St.";
    a.zip = "F00";

    // Create a new QContactAddress from an existing one.
#define NEW_CONTACT_ADDRESS(B,A) \
    QContactAddress B; \
    B.street = #B " " + A.street; \
    B.city = #B " " + A.city;     \
    B.state = #B " " + A.state;   \
    B.zip = #B " " + A.zip;       \
    B.country = #B " " + A.country;

    // Check that displayAddress() contains everything we expect.
#define CHECK_DISPLAY_ADDRESS(A,B) \
    QVERIFY( A.contains(B.street) );  \
    QVERIFY( A.contains(B.city) );    \
    QVERIFY( A.contains(B.state) );   \
    QVERIFY( A.contains(B.zip) );     \
    QVERIFY( A.contains(B.country) );

    /* Check that individual address getters return expected values,
     * comparing with a QContactAddress. */
#define CHECK_METHOD_CONTACT_ADDRESS(A,B,C) \
    QCOMPARE( A.B ## Street(), C.street ); \
    QCOMPARE( A.B ## City(), C.city );     \
    QCOMPARE( A.B ## State(), C.state );   \
    QCOMPARE( A.B ## Zip(), C.zip );       \
    QCOMPARE( A.B ## Country(), C.country );

    /* Set an address by a FullAddress using individual setter functions. */
#define SET_METHOD_CONTACT_ADDRESS(A,B,C) \
    A.set ## B ## City( C.city );       \
    A.set ## B ## Country( C.country ); \
    A.set ## B ## State( C.state );     \
    A.set ## B ## Street( C.street );   \
    A.set ## B ## Zip( C.zip );

    /* Make addresses for Home, Business, Other. */
    NEW_CONTACT_ADDRESS(home,a);
    NEW_CONTACT_ADDRESS(business,a);
    NEW_CONTACT_ADDRESS(other,a);
    NEW_CONTACT_ADDRESS(home2,a);
    NEW_CONTACT_ADDRESS(business2,a);

    for (int i = 0; i < 2; ++i) {
        /* Set each QContactAddress */
        c.setAddress( QContact::Home, home );
        c.setAddress( QContact::Business, business );
        c.setAddress( QContact::Other, other );

        /* Verify we get the same addresses we set */
        QCOMPARE(c.address(QContact::Home), home);
        QCOMPARE(c.address(QContact::Business), business);
        QCOMPARE(c.address(QContact::Other), other);

        /* Get all addresses in a map and verify they match expected values */
        QMap<QContact::Location, QContactAddress> addresses = c.addresses();
        QCOMPARE(addresses.count(), 3);
        QCOMPARE(addresses[QContact::Home], home);
        QCOMPARE(addresses[QContact::Business], business);
        QCOMPARE(addresses[QContact::Other], other);

        /* Check the displayAddress functions contain the expected information */
        CHECK_DISPLAY_ADDRESS( c.displayAddress(QContact::Home), home );
        CHECK_DISPLAY_ADDRESS( c.displayAddress(QContact::Business), business );
        CHECK_DISPLAY_ADDRESS( c.displayAddress(QContact::Other), other );
        CHECK_DISPLAY_ADDRESS( c.displayBusinessAddress(), business );
        CHECK_DISPLAY_ADDRESS( c.displayHomeAddress(), home );

        /* Check the home and business getters return correct information */
        CHECK_METHOD_CONTACT_ADDRESS( c, home, home );
        CHECK_METHOD_CONTACT_ADDRESS( c, business, business );

        /* Set the address using individual methods for home and business */
        SET_METHOD_CONTACT_ADDRESS( c, Home, home2 );
        SET_METHOD_CONTACT_ADDRESS( c, Business, business2 );

        /* Check the addresses using individual methods for home and business */
        CHECK_METHOD_CONTACT_ADDRESS( c, home, home2 );
        CHECK_METHOD_CONTACT_ADDRESS( c, business, business2 );

        /* Once again, check the addresses returned as a map.  This time we are
         * checking that the QContactAddresses returned contain the same data
         * as we set via individual methods. */
        addresses = c.addresses();
        QCOMPARE(addresses.count(), 3);
        QCOMPARE(addresses[QContact::Home], home2);
        QCOMPARE(addresses[QContact::Business], business2);
        QCOMPARE(addresses[QContact::Other], other);

        /* Finally, check that addresses can be unset. */
        /* Unset by setting entire address to empty at once. */
        QContactAddress emptyAddress;
        c.setAddress(QContact::Home, emptyAddress);
        addresses = c.addresses();
        QCOMPARE(addresses.count(), 2);
        QCOMPARE(addresses[QContact::Business], business2);
        QCOMPARE(addresses[QContact::Other], other);
        CHECK_METHOD_CONTACT_ADDRESS(c, home, emptyAddress);

        /* Unset by setting each individual address element to empty. */
        SET_METHOD_CONTACT_ADDRESS(c, Business, emptyAddress);
        addresses = c.addresses();
        QCOMPARE(addresses.count(), 1);
        QCOMPARE(addresses[QContact::Other], other);
        CHECK_METHOD_CONTACT_ADDRESS(c, home, emptyAddress);
        CHECK_METHOD_CONTACT_ADDRESS(c, business, emptyAddress);

        /* Unset last address using a QContactAddress. */
        c.setAddress(QContact::Other, emptyAddress);
        addresses = c.addresses();
        QCOMPARE(addresses.count(), 0);
        CHECK_METHOD_CONTACT_ADDRESS(c, home, emptyAddress);
        CHECK_METHOD_CONTACT_ADDRESS(c, business, emptyAddress);
    }

#undef NEW_CONTACT_ADDRESS
#undef CHECK_DISPLAY_ADDRESS
#undef CHECK_METHOD_CONTACT_ADDRESS
#undef SET_METHOD_CONTACT_ADDRESS
}


/*?
    Test that the QContact::match function behaves correctly.
*/
void tst_QContact::match()
{
    /* Add to a QStringList every string which should be expected to cause match()
     * to return true on a QContact modified via SET_ALL(). */
#define MAKE_MATCH_STRINGS(A,S) \
    A << "h1" S << "h2" S << "h3" S << "h4" S << "h5" S; \
    A << "b1" S << "b2" S << "b3" S << "b4" S << "b5" S; \
    A << "o1" S << "o2" S << "o3" S << "o4" S << "o5" S; \
    A << "assistant " S << "company " S << "children " S; \
    A << "company pronunciation " S << "department " S << "firstname " S; \
    A << "email" S << "email" S "2" << "email" S "3"; \
    A << "firstname pro " S << "job " S << "lastname " S; \
    A << "lastname pro " S << "job " S << "lastname " S; \
    A << "manager " S << "middlename " S << "nametitle " S; \
    A << "nick " S << "notes " S << "office " S; \
    A << "homephone " S << "homemobile " S << "homefax " S << "homepager " S; \
    A << "businessphone " S << "businessmobile " S << "businessfax " S << "businesspager " S; \
    A << "otherphone " S << "mobile " S << "fax " S << "pager " S; \
    A << "profession " S << "spouse " S << "suffix " S;

    /* Create two contacts with different strings. */
    QContact j, b;
    SET_ALL(j, "janine");
    SET_ALL(b, "billy");
    VERIFY_ALL(j, "janine");
    VERIFY_ALL(b, "billy");
    
    /* Enumerate all strings which should match for each contact. */
    QStringList jStrings, bStrings;
    MAKE_MATCH_STRINGS(jStrings, "janine");
    MAKE_MATCH_STRINGS(bStrings, "billy");

    /* Basic match checking.  Note these depend on exactly what SET_ALL() does.*/
    QVERIFY(j.match("janine"));
    QVERIFY(j.match("janine$"));
    QVERIFY(!j.match("^janine"));
    QVERIFY(b.match("billy"));
    QVERIFY(b.match("billy$"));
    QVERIFY(!b.match("^billy"));

    /* List of matches which will fail due to Bug 147432 */
    QStringList bug147432Strings;
    for (int i = 1; i <= 5; ++i) {
        bug147432Strings << QString("^h%1janine$").arg(i);
        bug147432Strings << QString("^b%1janine$").arg(i);
        bug147432Strings << QString("^o%1janine$").arg(i);
    }
    bug147432Strings << "^homephone janine$";
    bug147432Strings << "^homemobile janine$";
    bug147432Strings << "^homefax janine$";
    bug147432Strings << "^homepager janine$";
    bug147432Strings << "^businessphone janine$";
    bug147432Strings << "^businessmobile janine$";
    bug147432Strings << "^businessfax janine$";
    bug147432Strings << "^businesspager janine$";
    bug147432Strings << "^otherphone janine$";
    bug147432Strings << "^mobile janine$";
    bug147432Strings << "^fax janine$";
    bug147432Strings << "^pager janine$";

    /* Make sure j matches every string in jStrings and b does not match any
     * string in jStrings. */
    foreach (QString str, jStrings) {
        QString exS = QString("^%1$").arg(str);
        QRegExp ex(exS);

        QVERIFY2(j.match(exS), QString(exS).prepend("exS = ").toLatin1());
        QVERIFY2(!b.match(exS), QString(exS).prepend("exS = ").toLatin1());

        QVERIFY2(j.match(ex), QString(exS).prepend("ex = ").toLatin1());
        QVERIFY2(!b.match(ex), QString(exS).prepend("ex = ").toLatin1());
    }

    bug147432Strings.clear();
    for (int i = 1; i <= 5; ++i) {
        bug147432Strings << QString("^h%1billy$").arg(i);
        bug147432Strings << QString("^b%1billy$").arg(i);
        bug147432Strings << QString("^o%1billy$").arg(i);
    }
    bug147432Strings << "^homephone billy$";
    bug147432Strings << "^homemobile billy$";
    bug147432Strings << "^homefax billy$";
    bug147432Strings << "^homepager billy$";
    bug147432Strings << "^businessphone billy$";
    bug147432Strings << "^businessmobile billy$";
    bug147432Strings << "^businessfax billy$";
    bug147432Strings << "^businesspager billy$";
    bug147432Strings << "^otherphone billy$";
    bug147432Strings << "^mobile billy$";
    bug147432Strings << "^fax billy$";
    bug147432Strings << "^pager billy$";

    /* Make sure b matches every string in bStrings and j does not match any
     * string in bStrings. */
    foreach (QString str, bStrings) {
        QString exS = QString("^%1$").arg(str);
        QRegExp ex(exS);

        QVERIFY2(b.match(exS), QString(exS).prepend("exS = ").toLatin1());
        QVERIFY2(!j.match(exS), QString(exS).prepend("exS = ").toLatin1());

        QVERIFY2(b.match(ex), QString(exS).prepend("ex = ").toLatin1());
        QVERIFY2(!j.match(ex), QString(exS).prepend("ex = ").toLatin1());
    }

#undef MAKE_MATCH_STRINGS
}


