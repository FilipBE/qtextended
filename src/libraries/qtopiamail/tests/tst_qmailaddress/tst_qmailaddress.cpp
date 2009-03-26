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

// To detail string differences on comparison failure:
#include "shared/string_difference.h"

#include <QtopiaApplication>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QMailAddress>

/* 
Note: Any email addresses appearing in this test data must be example addresses,
as defined by RFC 2606.  Therefore, they should use one of the following domains:
    *.example.{com|org|net}
    *.test
    *.example
*/

//TESTED_CLASS=QMailAddress
//TESTED_FILES=src/libraries/qtopiamail/qmailmessage.cpp

/*
    This class primarily tests that QMailAddress correctly handles e-mail addresses.
*/
class tst_QMailAddress : public QObject
{
    Q_OBJECT

public:
    tst_QMailAddress();
    virtual ~tst_QMailAddress();

private slots:
    void constructor1_data();
    void constructor1();
    void constructor2_data();
    void constructor2();
    void name();
    void address();
    void isGroup();
    void groupMembers_data();
    void groupMembers();
    void notGroup();
    void toString();
    void isPhoneNumber_data();
    void isPhoneNumber();
    void isEmailAddress_data();
    void isEmailAddress();
    void toStringList_data();
    void toStringList();
    void fromStringList1_data();
    void fromStringList1();
    void fromStringList2();
    void removeComments_data();
    void removeComments();
    void removeWhitespace_data();
    void removeWhitespace();
};

QTEST_APP_MAIN( tst_QMailAddress, QtopiaApplication )
#include "tst_qmailaddress.moc"


tst_QMailAddress::tst_QMailAddress()
{
}

tst_QMailAddress::~tst_QMailAddress()
{
}

//    QMailAddress::QMailAddress(const QString &s);
void tst_QMailAddress::constructor1_data()
{
    QTest::addColumn<bool>("valid_email_address");
    QTest::addColumn<QString>("from");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("address");
    QTest::addColumn<QString>("to_string");

    QTest::newRow("No angle brackets") 
        << true
        << "wizard@oz.test" 
        << "wizard@oz.test" 
        << "wizard@oz.test" 
        << "wizard@oz.test";

    QTest::newRow("preceding comment") 
        << true
        << "(Wizard of Oz) wizard@oz.test" 
        << "(Wizard of Oz) wizard@oz.test" 
        << "(Wizard of Oz) wizard@oz.test" 
        << "(Wizard of Oz) wizard@oz.test";

    QTest::newRow("trailing comment") 
        << true
        << "wizard@oz.test (Wizard of Oz)"
        << "wizard@oz.test (Wizard of Oz)"
        << "wizard@oz.test (Wizard of Oz)"
        << "wizard@oz.test (Wizard of Oz)";

    QTest::newRow("intervening comment") 
        << true
        << "wizard(Wizard of Oz)@oz.test"
        << "wizard(Wizard of Oz)@oz.test"
        << "wizard(Wizard of Oz)@oz.test"
        << "wizard(Wizard of Oz)@oz.test";

    QTest::newRow("No address part") 
        << false
        << "Wizard Of Oz" 
        << "Wizard Of Oz" 
        << "Wizard Of Oz" 
        << "Wizard Of Oz";

    QTest::newRow("Left angle bracket") 
        << true
        << "<wizard@oz.test" 
        << "wizard@oz.test" 
        << "wizard@oz.test" 
        << "wizard@oz.test";

    QTest::newRow("Right angle bracket") 
        << true
        << "wizard@oz.test>" 
        << "wizard@oz.test" 
        << "wizard@oz.test" 
        << "wizard@oz.test";

    QTest::newRow("Two angle brackets") 
        << true
        << "Wizard Of Oz <wizard@oz.test>" 
        << "Wizard Of Oz"
        << "wizard@oz.test" 
        << "Wizard Of Oz <wizard@oz.test>";

    QTest::newRow("Upper-case address characters") 
        << true
        << "Wizard Of Oz <WiZarD@Oz.tEsT>" 
        << "Wizard Of Oz"
        << "WiZarD@Oz.tEsT" 
        << "Wizard Of Oz <WiZarD@Oz.tEsT>";

    QTest::newRow("Quoted name") 
        << true
        << "\"Wizard Of Oz\" <wizard@oz.test>" 
        << "Wizard Of Oz"
        << "wizard@oz.test" 
        << "\"Wizard Of Oz\" <wizard@oz.test>";

    QTest::newRow("Trailing garbage") 
        << true
        << "Wizard Of Oz <wizard@oz.test> crap" 
        << "Wizard Of Oz"
        << "wizard@oz.test" 
        << "Wizard Of Oz <wizard@oz.test>";

    QTest::newRow("Trailing type specifier") 
        << true
        << "Wizard Of Oz <wizard@oz.test>/TYPE=unknown" 
        << "Wizard Of Oz"
        << "wizard@oz.test" 
        << "Wizard Of Oz <wizard@oz.test> /TYPE=unknown";

    QTest::newRow("With whitespace") 
        << true
        << "  \t \"Wizard Of Oz\"\t\t\n \r <wizard@oz.test>  \r\r \t"
        << "Wizard Of Oz"
        << "wizard@oz.test" 
        << "\"Wizard Of Oz\" <wizard@oz.test>";

    QTest::newRow("'(' needs quoting") 
        << true
        << "\"Wizard (Of Oz\" <wizard@oz.test>"
        << "Wizard (Of Oz"
        << "wizard@oz.test"
        << "\"Wizard (Of Oz\" <wizard@oz.test>";

    QTest::newRow("')' needs quoting") 
        << true
        << "\"Wizard) Of Oz\" <wizard@oz.test>"
        << "Wizard) Of Oz"
        << "wizard@oz.test"
        << "\"Wizard) Of Oz\" <wizard@oz.test>";

    QTest::newRow("Comments don't need quoting") 
        << true
        << "Wizard (Of Oz) <wizard@oz.test>"
        << "Wizard (Of Oz)"
        << "wizard@oz.test"
        << "Wizard (Of Oz) <wizard@oz.test>";

    QTest::newRow("Nested Comments don't need quoting") 
        << true
        << "Wizard ((Of) Oz) <wizard@oz.test>"
        << "Wizard ((Of) Oz)"
        << "wizard@oz.test"
        << "Wizard ((Of) Oz) <wizard@oz.test>";

    QTest::newRow("Mismatched Comments need quoting") 
        << true
        << "\"Wizard ((Of Oz)\" <wizard@oz.test>"
        << "Wizard ((Of Oz)"
        << "wizard@oz.test"
        << "\"Wizard ((Of Oz)\" <wizard@oz.test>";

    QTest::newRow("Non-Comments need quoting") 
        << true
        << "\"Wizard )Of Oz(\" <wizard@oz.test>"
        << "Wizard )Of Oz("
        << "wizard@oz.test"
        << "\"Wizard )Of Oz(\" <wizard@oz.test>";

    QTest::newRow("'<' and '>' need quoting") 
        << true
        << "\"Wizard <Of Oz>\" <wizard@oz.test>"
        << "Wizard <Of Oz>"
        << "wizard@oz.test"
        << "\"Wizard <Of Oz>\" <wizard@oz.test>";

    QTest::newRow("'[' and ']' need quoting") 
        << true
        << "\"Wizard [Of Oz]\" <wizard@oz.test>"
        << "Wizard [Of Oz]"
        << "wizard@oz.test"
        << "\"Wizard [Of Oz]\" <wizard@oz.test>";

    QTest::newRow("'@' needs quoting") 
        << true
        << "\"Wizard at SETI@Home\" <wizard@seti.test>"
        << "Wizard at SETI@Home"
        << "wizard@seti.test"
        << "\"Wizard at SETI@Home\" <wizard@seti.test>";

    QTest::newRow("';' and ':' need quoting") 
        << true
        << "\"Wizard;Wizard of Oz:\" <wizard@oz.test>"
        << "Wizard;Wizard of Oz:"
        << "wizard@oz.test"
        << "\"Wizard;Wizard of Oz:\" <wizard@oz.test>";

    QTest::newRow("',' needs quoting") 
        << true
        << "\"Wizard, Of Oz\" <wizard@oz.test>"
        << "Wizard, Of Oz"
        << "wizard@oz.test"
        << "\"Wizard, Of Oz\" <wizard@oz.test>";

    QTest::newRow("'.' needs quoting") 
        << true
        << "\"O. Wizard\" <wizard@oz.test>"
        << "O. Wizard"
        << "wizard@oz.test"
        << "\"O. Wizard\" <wizard@oz.test>";

    /* Honestly, I don't know what to do about this...
    QTest::newRow("'\\' needs quoting") 
        << true
        << "\"Wizard\\Oz\" <wizard@oz.test>"
        << "Wizard\\Oz"
        << "wizard@oz.test"
        << "\"Wizard\\Oz\" <wizard@oz.test>";
    */
}

void tst_QMailAddress::constructor1()
{
    QFETCH( QString, from ); 

    QMailAddress addr(from);
    QTEST( addr.isEmailAddress(), "valid_email_address" );
    QTEST( addr.name(), "name" );
    QTEST( addr.address(), "address" );
    QTEST( addr.toString(), "to_string" );
}

//    QMailAddress::QMailAddress(const QString &name, const QString &addr);
void tst_QMailAddress::constructor2_data()
{
    QTest::addColumn<QString>("arg1");
    QTest::addColumn<QString>("arg2");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("address");
    QTest::addColumn<QString>("to_string");

    QTest::newRow("No name") 
        << QString()
        << "wizard@oz.test"
        << QString()
        << "wizard@oz.test"
        << "wizard@oz.test";

    QTest::newRow("Empty name") 
        << ""
        << "wizard@oz.test"
        << QString()
        << "wizard@oz.test"
        << "wizard@oz.test";

    QTest::newRow("No address") 
        << "Wizard Of Oz"
        << QString()
        << "Wizard Of Oz"
        << QString()
        << "Wizard Of Oz";

    QTest::newRow("Empty address") 
        << "Wizard Of Oz"
        << ""
        << "Wizard Of Oz"
        << QString()
        << "Wizard Of Oz";

    QTest::newRow("Name and address") 
        << "Wizard Of Oz" 
        << "wizard@oz.test"
        << "Wizard Of Oz" 
        << "wizard@oz.test"
        << "Wizard Of Oz <wizard@oz.test>";

    QTest::newRow("Upper-case address characters")
        << "Wizard Of Oz" 
        << "WIzARd@oZ.TesT"
        << "Wizard Of Oz" 
        << "WIzARd@oZ.TesT"
        << "Wizard Of Oz <WIzARd@oZ.TesT>";

    QTest::newRow("Quoted name") 
        << "\"Wizard Of Oz\"" 
        << "wizard@oz.test"
        << "Wizard Of Oz" 
        << "wizard@oz.test"
        << "\"Wizard Of Oz\" <wizard@oz.test>";

    QTest::newRow("With trailing type specifier") 
        << "Wizard Of Oz" 
        << "wizard@oz.test /TYPE=email"
        << "Wizard Of Oz" 
        << "wizard@oz.test"
        << "Wizard Of Oz <wizard@oz.test> /TYPE=email";

    QTest::newRow("'(' needs quoting") 
        << "Wizard (Of Oz"
        << "<wizard@oz.test>"
        << "Wizard (Of Oz"
        << "wizard@oz.test"
        << "\"Wizard (Of Oz\" <wizard@oz.test>";

    QTest::newRow("')' needs quoting") 
        << "Wizard) Of Oz"
        << "<wizard@oz.test>"
        << "Wizard) Of Oz"
        << "wizard@oz.test"
        << "\"Wizard) Of Oz\" <wizard@oz.test>";

    QTest::newRow("Comments don't need quoting") 
        << "Wizard (Of Oz)"
        << "wizard@oz.test"
        << "Wizard (Of Oz)"
        << "wizard@oz.test"
        << "Wizard (Of Oz) <wizard@oz.test>";

    QTest::newRow("Nested Comments don't need quoting") 
        << "Wizard ((Of) Oz)"
        << "wizard@oz.test"
        << "Wizard ((Of) Oz)"
        << "wizard@oz.test"
        << "Wizard ((Of) Oz) <wizard@oz.test>";

    QTest::newRow("Mismatched Comments need quoting") 
        << "Wizard ((Of Oz)"
        << "wizard@oz.test"
        << "Wizard ((Of Oz)"
        << "wizard@oz.test"
        << "\"Wizard ((Of Oz)\" <wizard@oz.test>";

    QTest::newRow("Non-Comments need quoting") 
        << "Wizard )Of Oz("
        << "wizard@oz.test"
        << "Wizard )Of Oz("
        << "wizard@oz.test"
        << "\"Wizard )Of Oz(\" <wizard@oz.test>";

    QTest::newRow("'<' and '>' need quoting") 
        << "Wizard <Of Oz>"
        << "wizard@oz.test"
        << "Wizard <Of Oz>"
        << "wizard@oz.test"
        << "\"Wizard <Of Oz>\" <wizard@oz.test>";

    QTest::newRow("'[' and ']' need quoting") 
        << "Wizard [Of Oz]"
        << "wizard@oz.test"
        << "Wizard [Of Oz]"
        << "wizard@oz.test"
        << "\"Wizard [Of Oz]\" <wizard@oz.test>";

    QTest::newRow("'@' needs quoting") 
        << "Wizard at SETI@Home"
        << "wizard@seti.test"
        << "Wizard at SETI@Home"
        << "wizard@seti.test"
        << "\"Wizard at SETI@Home\" <wizard@seti.test>";

    QTest::newRow("':' and ';' need quoting") 
        << "Wizard:Wizard of Oz;"
        << "wizard@oz.test"
        << "Wizard:Wizard of Oz;"
        << "wizard@oz.test"
        << "\"Wizard:Wizard of Oz;\" <wizard@oz.test>";

    QTest::newRow("',' needs quoting") 
        << "Wizard, Of Oz"
        << "wizard@oz.test"
        << "Wizard, Of Oz"
        << "wizard@oz.test"
        << "\"Wizard, Of Oz\" <wizard@oz.test>";

    QTest::newRow("'.' needs quoting") 
        << "O. Wizard"
        << "wizard@oz.test"
        << "O. Wizard"
        << "wizard@oz.test"
        << "\"O. Wizard\" <wizard@oz.test>";

    /* Honestly, I don't know what to do about this...
    QTest::newRow("'\\' needs quoting") 
        << "Wizard\\Oz"
        << "wizard@oz.test"
        << "Wizard\\Oz"
        << "wizard@oz.test"
        << "\"Wizard\\Oz\" <wizard@oz.test>";
    */
}

void tst_QMailAddress::constructor2()
{
    QFETCH( QString, arg1 );
    QFETCH( QString, arg2 );

    QMailAddress addr(arg1, arg2);
    QTEST( addr.name(), "name" );
    QTEST( addr.address(), "address" );
    QTEST( addr.toString(), "to_string" );
}

void tst_QMailAddress::name()
{
    // Tested-by: constructor1, constructor2
}

void tst_QMailAddress::address()
{
    // Tested-by: constructor1, constructor2
}

void tst_QMailAddress::isGroup()
{
    // Tested-by: groupMembers
}

void tst_QMailAddress::groupMembers_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("input_name");
    QTest::addColumn<QString>("input_address");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("address");
    QTest::addColumn<QString>("to_string");
    QTest::addColumn<bool>("is_group");
    QTest::addColumn<QStringList>("member_names");
    QTest::addColumn<QStringList>("member_addresses");

    QTest::newRow("Empty")
        << QString()
        << QString()
        << QString()
        << QString()
        << QString()
        << QString()
        << false
        << QStringList()
        << QStringList();

    QTest::newRow("Non-group ctor1")
        << QString("Wizard Of Oz <wizard@oz.test>")
        << QString()
        << QString()
        << QString("Wizard Of Oz")
        << QString("wizard@oz.test")
        << QString("Wizard Of Oz <wizard@oz.test>")
        << false
        << QStringList()
        << QStringList();

    QTest::newRow("Non-group ctor2")
        << QString()
        << QString("Wizard Of Oz")
        << QString("wizard@oz.test")
        << QString("Wizard Of Oz")
        << QString("wizard@oz.test")
        << QString("Wizard Of Oz <wizard@oz.test>")
        << false
        << QStringList()
        << QStringList();

    QTest::newRow("Non-group with comma in name ctor1")
        << QString("\"Wizard, Of Oz\" <wizard@oz.test>")
        << QString()
        << QString()
        << QString("Wizard, Of Oz")
        << QString("wizard@oz.test")
        << QString("\"Wizard, Of Oz\" <wizard@oz.test>")
        << false
        << QStringList()
        << QStringList();

    QTest::newRow("Non-group with comma in name ctor2")
        << QString()
        << QString("\"Wizard, Of Oz\"")
        << QString("wizard@oz.test")
        << QString("Wizard, Of Oz")
        << QString("wizard@oz.test")
        << QString("\"Wizard, Of Oz\" <wizard@oz.test>")
        << false
        << QStringList()
        << QStringList();

    QTest::newRow("Non-group with comma in address ctor1")
        << QString("Wizard Of Oz <wizard(is,allowed here?)@oz.test>")
        << QString()
        << QString()
        << QString("Wizard Of Oz")
        << QString("wizard(is,allowed here?)@oz.test")
        << QString("Wizard Of Oz <wizard(is,allowed here?)@oz.test>")
        << false
        << QStringList()
        << QStringList();

    QTest::newRow("Non-group with comma in address ctor2")
        << QString()
        << QString("Wizard Of Oz")
        << QString("wizard(is,allowed here?)@oz.test")
        << QString("Wizard Of Oz")
        << QString("wizard(is,allowed here?)@oz.test")
        << QString("Wizard Of Oz <wizard(is,allowed here?)@oz.test>")
        << false
        << QStringList()
        << QStringList();

    QTest::newRow("Group-of-zero ctor1")
        << QString("Wizards:;")
        << QString()
        << QString()
        << QString("Wizards")
        << QString()
        << QString("Wizards: ;")
        << true
        << QStringList()
        << QStringList();

    /* It is not possible to create a group of zero via ctor2
    QTest::newRow("Group-of-zero ctor2")
        << ...
    */

    QTest::newRow("Group-of-one ctor1")
        << QString("Wizards: Wizard Of Oz <wizard@oz.test>;")
        << QString()
        << QString()
        << QString("Wizards")
        << QString("Wizard Of Oz <wizard@oz.test>")
        << QString("Wizards: Wizard Of Oz <wizard@oz.test>;")
        << true
        << ( QStringList() << "Wizard Of Oz" )
        << ( QStringList() << "wizard@oz.test" );

    /* It is not possible to create a group of one via ctor2
    QTest::newRow("Group-of-one ctor2")
        << ...
    */

    QTest::newRow("Group ctor1")
        << "Wizard Group:Wizard Of Oz <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;" 
        << QString()
        << QString()
        << "Wizard Group"
        << "Wizard Of Oz <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>" 
        << "Wizard Group: Wizard Of Oz <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;"
        << true
        << ( QStringList() << "Wizard Of Oz" << "Rincewind" ) 
        << ( QStringList() << "wizard@oz.test" << "wizzard@uu.edu.example" );

    QTest::newRow("Group ctor2")
        << QString()
        << "Wizard Group"
        << "Wizard Of Oz <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>" 
        << "Wizard Group"
        << "Wizard Of Oz <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>" 
        << "Wizard Group: Wizard Of Oz <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;"
        << true
        << ( QStringList() << "Wizard Of Oz" << "Rincewind" ) 
        << ( QStringList() << "wizard@oz.test" << "wizzard@uu.edu.example" );

    QTest::newRow("Group with quoted member names ctor1")
        << "Wizard Group: \"Wizard Of Oz\" <wizard@oz.test>, \"Rincewind\" <wizzard@uu.edu.example>;" 
        << QString()
        << QString()
        << "Wizard Group"
        << "\"Wizard Of Oz\" <wizard@oz.test>, \"Rincewind\" <wizzard@uu.edu.example>" 
        << "Wizard Group: \"Wizard Of Oz\" <wizard@oz.test>, \"Rincewind\" <wizzard@uu.edu.example>;"
        << true
        << ( QStringList() << "Wizard Of Oz" << "Rincewind" ) 
        << ( QStringList() << "wizard@oz.test" << "wizzard@uu.edu.example" );

    QTest::newRow("Group with quoted member names ctor2")
        << QString()
        << "Wizard Group"
        << "\"Wizard Of Oz\" <wizard@oz.test>, \"Rincewind\" <wizzard@uu.edu.example>" 
        << "Wizard Group"
        << "\"Wizard Of Oz\" <wizard@oz.test>, \"Rincewind\" <wizzard@uu.edu.example>" 
        << "Wizard Group: \"Wizard Of Oz\" <wizard@oz.test>, \"Rincewind\" <wizzard@uu.edu.example>;"
        << true
        << ( QStringList() << "Wizard Of Oz" << "Rincewind" ) 
        << ( QStringList() << "wizard@oz.test" << "wizzard@uu.edu.example" );
}

void tst_QMailAddress::groupMembers()
{
    QFETCH( QString, input );
    QFETCH( QString, input_name );
    QFETCH( QString, input_address );

    QMailAddress addr;
    if ( !input.isEmpty() )
        addr = QMailAddress( input );
    else
        addr = QMailAddress( input_name, input_address );

    QTEST( addr.name(), "name" );
    QTEST( addr.address(), "address" );
    QTEST( addr.toString(), "to_string" );
    QTEST( addr.isGroup(), "is_group" );

    QStringList names, addresses;
    foreach (const QMailAddress& member, addr.groupMembers()) {
        names.append(member.name());
        addresses.append(member.address());
    }

    QTEST( names, "member_names" );
    QTEST( addresses, "member_addresses" );
}

void tst_QMailAddress::notGroup()
{
    QString input("\"Wizard: Oz\" <wizard@oz.test>,Rincewind <wizzard@uu.edu.example>");

    QList<QMailAddress> list(QMailAddress::fromStringList(input));
    QCOMPARE( list.count(), 2 );

    QCOMPARE( list.at(0).isGroup(), false );
    QCOMPARE( list.at(0).name(), QString("Wizard: Oz") );
    QCOMPARE( list.at(0).address(), QString("wizard@oz.test") );

    QCOMPARE( list.at(1).isGroup(), false );
    QCOMPARE( list.at(1).name(), QString("Rincewind") );
    QCOMPARE( list.at(1).address(), QString("wizzard@uu.edu.example") );
}

void tst_QMailAddress::toString()
{
    // Tested-by: constructor1, constructor2
}

void tst_QMailAddress::isPhoneNumber_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<bool>("phoneNumber");

    QTest::newRow("simple") 
        << "32199000" << true;

    QTest::newRow("legal white space") 
        << "32 19 9000" << true;

    QTest::newRow("area code") 
        << "(07) 3219 9000" << true;

    QTest::newRow("country code") 
        << "+61 7 3219 9000" << true;

    QTest::newRow("mobile") 
        << "0404 444 444" << true;

    QTest::newRow("dashed") 
        << "1800-123-321" << true;

    QTest::newRow("accepted chars") 
        << "+01 2#3 45* 678X90" << true;

    QTest::newRow("extension 1") 
        << "(07) 3219 9000 (x100)" << true;

    QTest::newRow("extension 2") 
        << "(07) 3219 9000 (P100)" << true;

    QTest::newRow("extension 3") 
        << "(07) 3219 9000 w100" << true;

    QTest::newRow("extension 4") 
        << "(07) 3219 9000,100" << true;

    // Perhaps this should be parsed?
    QTest::newRow("alphanumeric") 
        << "1800-Reverse" << false;

    QTest::newRow("email") 
        << "1800@123321" << false;

    QTest::newRow("illegal white space 1") 
        << " 3219\t9000" << false;

    QTest::newRow("illegal white space 2") 
        << " 3219\n9000" << false;

    QTest::newRow("garbage 1") 
        << "[1800 123 321]" << false;

    QTest::newRow("garbage 2") 
        << "1800 123 321:" << false;

    QTest::newRow("garbage 3") 
        << "1800_123_321" << false;

    QTest::newRow("rejected chars 1") 
        << "A" << false;

    QTest::newRow("rejected chars 2") 
        << "@" << false;

    QTest::newRow("rejected chars 3") 
        << "&" << false;
}

void tst_QMailAddress::isPhoneNumber()
{
    QFETCH( QString, address );

    QMailAddress addr = QMailAddress(QString(), address);
    QTEST(addr.isPhoneNumber(), "phoneNumber");
}

void tst_QMailAddress::isEmailAddress_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<bool>("emailAddress");

    QTest::newRow("simple") 
        << "fred@example.net" << true;

    QTest::newRow("white space 1") 
        << " fred@example.net   " << true;

    QTest::newRow("white space 2") 
        << "\t\n  \tfred@example.net\r \n" << true;

    QTest::newRow("multi-part domain") 
        << "fred@mail-machine-1.example.net.au" << true;

    QTest::newRow("multi-part name") 
        << "fred.smith@example.net" << true;

    QTest::newRow("short") 
        << "x@y.zz" << true;

    QTest::newRow("missing mailbox name") 
        << "@example.net" << false;

    QTest::newRow("missing domain") 
        << "fred@" << false;

    QTest::newRow("single part domain") 
        << "fred@example" << false;
}

void tst_QMailAddress::isEmailAddress()
{
    QFETCH( QString, address );

    QMailAddress addr = QMailAddress(QString(), address);
    QTEST(addr.isEmailAddress(), "emailAddress");
}

void tst_QMailAddress::toStringList_data()
{
    QTest::addColumn<QList<QMailAddress> >("address_list");
    QTest::addColumn<QStringList>("string_list");

    QTest::newRow("Empty")
        << QList<QMailAddress>()
        << QStringList();

    QTest::newRow("Single plain address")
        << ( QList<QMailAddress>() 
                << QMailAddress("wizard@oz.test") )
        << ( QStringList() 
                << "wizard@oz.test" );

    QTest::newRow("Single named address")
        << ( QList<QMailAddress>() 
                << QMailAddress("\"Wizard, Of Oz\" <wizard@oz.test>") )
        << ( QStringList() 
                << "\"Wizard, Of Oz\" <wizard@oz.test>" );

    QTest::newRow("Single group address")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;") )
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;" );

    QTest::newRow("Multiple named addressses")
        << ( QList<QMailAddress>() 
                << QMailAddress("\"Wizard, Of Oz\" <wizard@oz.test>")
                << QMailAddress("Rincewind <wizzard@uu.edu.example>") )
        << ( QStringList() 
                << "\"Wizard, Of Oz\" <wizard@oz.test>"
                << "Rincewind <wizzard@uu.edu.example>" );

    QTest::newRow("Multiple group addresses")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;")
                << QMailAddress("Witch Group: Wicked Witch (East) <eastwitch@oz.test>, \"Wicked Witch, South\" <southwitch@oz.test>;") )
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;"
                << "Witch Group: Wicked Witch (East) <eastwitch@oz.test>, \"Wicked Witch, South\" <southwitch@oz.test>;" );

    QTest::newRow("Multiple mixed addresses")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;")
                << QMailAddress("Dorothy <dot2000@kansas.test>")
                << QMailAddress("Witch Group: Wicked Witch (East) <eastwitch@oz.test>, \"Wicked Witch, South\" <southwitch@oz.test>;") )
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;"
                << "Dorothy <dot2000@kansas.test>"
                << "Witch Group: Wicked Witch (East) <eastwitch@oz.test>, \"Wicked Witch, South\" <southwitch@oz.test>;" );
}

void tst_QMailAddress::toStringList()
{
    QFETCH( QList<QMailAddress>, address_list );
    QTEST( QMailAddress::toStringList(address_list), "string_list");
}

void tst_QMailAddress::fromStringList1_data()
{
    QTest::addColumn<QString>("string_list");
    QTest::addColumn<QList<QMailAddress> >("address_list");

    QTest::newRow("Empty")
        << QString()
        << QList<QMailAddress>();

    QTest::newRow("Single plain address")
        << "wizard@oz.test"
        << ( QList<QMailAddress>() 
                << QMailAddress("wizard@oz.test") );

    QTest::newRow("Single named address")
        << "\"Wizard, Of Oz\" <wizard@oz.test>"
        << ( QList<QMailAddress>() 
                << QMailAddress("\"Wizard, Of Oz\" <wizard@oz.test>") );

    QTest::newRow("Single group address")
        << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;"
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;") );

    QTest::newRow("Multiple plain addressses, comma-separated")
        << ( QStringList() 
                << "wizard@oz.test"
                << "wizzard@uu.edu.example" ).join(",")
        << ( QList<QMailAddress>() 
                << QMailAddress("wizard@oz.test")
                << QMailAddress("wizzard@uu.edu.example") );

    QTest::newRow("Multiple plain addressses, semicolon-separated")
        << ( QStringList() 
                << "wizard@oz.test"
                << "wizzard@uu.edu.example" ).join(";")
        << ( QList<QMailAddress>() 
                << QMailAddress("wizard@oz.test")
                << QMailAddress("wizzard@uu.edu.example") );

    QTest::newRow("Multiple plain addressses, whitespace-separated")
        << ( QStringList() 
                << "wizard@oz.test"
                << "wizzard@uu.edu.example" ).join(" ")
        << ( QList<QMailAddress>() 
                << QMailAddress("wizard@oz.test")
                << QMailAddress("wizzard@uu.edu.example") );

    QTest::newRow("Multiple named addressses, comma-separated")
        << ( QStringList() 
                << "\"Wizard, Of Oz\" <wizard@oz.test>"
                << "Rincewind <wizzard@uu.edu.example>" ).join(",")
        << ( QList<QMailAddress>() 
                << QMailAddress("\"Wizard, Of Oz\" <wizard@oz.test>")
                << QMailAddress("Rincewind <wizzard@uu.edu.example>") );

    QTest::newRow("Multiple named addressses, semicolon-separated")
        << ( QStringList() 
                << "\"Wizard, Of Oz\" <wizard@oz.test>"
                << "Rincewind <wizzard@uu.edu.example>" ).join(";")
        << ( QList<QMailAddress>() 
                << QMailAddress("\"Wizard, Of Oz\" <wizard@oz.test>")
                << QMailAddress("Rincewind <wizzard@uu.edu.example>") );

    QTest::newRow("Multiple named addressses, whitespace-separated")
        << ( QStringList() 
                << "\"Wizard, Of Oz\" <wizard@oz.test>"
                << "Rincewind <wizzard@uu.edu.example>" ).join(" ")
        << ( QList<QMailAddress>() 
                << QMailAddress("\"Wizard, Of Oz\" <wizard@oz.test>")
                << QMailAddress("Rincewind <wizzard@uu.edu.example>") );

    QTest::newRow("Multiple group addresses, comma-separated")
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;"
                << "Witch Group: Wicked Witch (East) <eastwitch@oz.test>, \"Wicked Witch, South\" <southwitch@oz.test>;" ).join(",")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;")
                << QMailAddress("Witch Group: Wicked Witch (East) <eastwitch@oz.test>, \"Wicked Witch, South\" <southwitch@oz.test>;") );

    QTest::newRow("Multiple group addresses, semicolon-separated")
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;"
                << "Witch Group: Wicked Witch (East) <eastwitch@oz.test>, \"Wicked Witch, South\" <southwitch@oz.test>;" ).join(";")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;")
                << QMailAddress("Witch Group: Wicked Witch (East) <eastwitch@oz.test>, \"Wicked Witch, South\" <southwitch@oz.test>;") );

    QTest::newRow("Multiple group addresses, whitespace-separated")
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;"
                << "Witch Group: Wicked Witch (East) <eastwitch@oz.test>, \"Wicked Witch, South\" <southwitch@oz.test>;" ).join(" ")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;")
                << QMailAddress("Witch Group: Wicked Witch (East) <eastwitch@oz.test>, \"Wicked Witch, South\" <southwitch@oz.test>;") );

    QTest::newRow("Multiple mixed addresses, comma-separated")
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;"
                << "gandalf@whitewizard.org"
                << "Dorothy <dot2000@kansas.test>"
                << "Witch Group: Wicked Witch (East) <eastwitch@oz.test>, \"Wicked Witch, South\" <southwitch@oz.test>;" ).join(",")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;")
                << QMailAddress("gandalf@whitewizard.org")
                << QMailAddress("Dorothy <dot2000@kansas.test>")
                << QMailAddress("Witch Group: Wicked Witch (East) <eastwitch@oz.test>, \"Wicked Witch, South\" <southwitch@oz.test>;") );

    QTest::newRow("Multiple mixed addresses, semicolon-separated")
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;"
                << "gandalf@whitewizard.org"
                << "Dorothy <dot2000@kansas.test>"
                << "Witch Group: Wicked Witch (East) <eastwitch@oz.test>, \"Wicked Witch, South\" <southwitch@oz.test>;" ).join(";")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;")
                << QMailAddress("gandalf@whitewizard.org")
                << QMailAddress("Dorothy <dot2000@kansas.test>")
                << QMailAddress("Witch Group: Wicked Witch (East) <eastwitch@oz.test>, \"Wicked Witch, South\" <southwitch@oz.test>;") );

    QTest::newRow("Multiple mixed addresses, whitespace-separated")
        << ( QStringList() 
                << "Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;"
                << "gandalf@whitewizard.org"
                << "Dorothy <dot2000@kansas.test>"
                << "Witch Group: Wicked Witch (East) <eastwitch@oz.test>, \"Wicked Witch, South\" <southwitch@oz.test>;" ).join(" ")
        << ( QList<QMailAddress>() 
                << QMailAddress("Wizard Group: \"Wizard, Of Oz\" <wizard@oz.test>, Rincewind <wizzard@uu.edu.example>;")
                << QMailAddress("gandalf@whitewizard.org")
                << QMailAddress("Dorothy <dot2000@kansas.test>")
                << QMailAddress("Witch Group: Wicked Witch (East) <eastwitch@oz.test>, \"Wicked Witch, South\" <southwitch@oz.test>;") );
}

void tst_QMailAddress::fromStringList1()
{
    QFETCH( QString, string_list );
    QTEST( QMailAddress::fromStringList(string_list), "address_list");
}

void tst_QMailAddress::fromStringList2()
{
    // Tested-by: fromStringList1
}

void tst_QMailAddress::removeComments_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");

    QTest::newRow("Empty")
        << QString()
        << QString();

    QTest::newRow("No comments")
        << "\"Wizard, Of Oz\" <wizard@oz.test>"
        << "\"Wizard, Of Oz\" <wizard@oz.test>";

    QTest::newRow("Leading comment")
        << "(comment) \"Wizard, Of Oz\" <wizard@oz.test>"
        << "\"Wizard, Of Oz\" <wizard@oz.test>";

    QTest::newRow("Trailing comment")
        << "\"Wizard, Of Oz\" <wizard@oz.test> (comment)"
        << "\"Wizard, Of Oz\" <wizard@oz.test>";

    QTest::newRow("Interspersed comments")
        << "\"Wizard, Of Oz\"(comment) <wizard@(comment)oz.test>"
        << "\"Wizard, Of Oz\" <wizard@oz.test>";

    QTest::newRow("Nested comments")
        << "\"Wizard, Of Oz\"(comment(comment)) <wizard((comment)comment)@oz.test>"
        << "\"Wizard, Of Oz\" <wizard@oz.test>";
}

void tst_QMailAddress::removeComments()
{
    QFETCH( QString, input );
    QTEST( QMailAddress::removeComments(input), "output" );
}

void tst_QMailAddress::removeWhitespace_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");

    QTest::newRow("Empty")
        << QString()
        << QString();

    QTest::newRow("No whitespace")
        << "wizard@oz.test"
        << "wizard@oz.test";

    QTest::newRow("Leading whitespace")
        << "\t\n wizard@oz.test"
        << "wizard@oz.test";

    QTest::newRow("Trailing whitespace")
        << "wizard@oz.test   \t  "
        << "wizard@oz.test";

    QTest::newRow("Interspersed whitespace")
        << "wizard @\n\toz . test"
        << "wizard@oz.test";

    QTest::newRow("Quoted whitespace")
        << "\"wizard \" @ \"oz.test\t\n\""
        << "\"wizard \"@\"oz.test\t\n\"";

    QTest::newRow("Comment whitespace")
        << "wizard(Of Oz) @ oz.test((\t))"
        << "wizard(Of Oz)@oz.test((\t))";
}

void tst_QMailAddress::removeWhitespace()
{
    QFETCH( QString, input );
    QTEST( QMailAddress::removeWhitespace(input), "output" );
}

