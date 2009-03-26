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

#include <qd_common.h>
#include <qdplugin.h>
#include <private/qdplugin_p.h>
#define private public
#include "qsyncprotocol.h"
#undef private
#include "fakeplugins.h"

#define NEW_SYNC()\
do {\
    if (protocol) delete protocol;\
    protocol = new QSyncProtocol;\
    syncTime = syncTime.addSecs(300);\
    client->nextSync = syncTime;\
    server->nextSync = syncTime;\
    client->clearSyncData();\
    server->clearSyncData();\
} while (0)

class syncmerge: public QObject
{
    Q_OBJECT
    QSyncProtocol *protocol;
    FakeClient *client;
    FakeServer *server;
    QDateTime syncTime;
private slots:
    void initTestCase()
    {
        QD_COMMON_INIT_TEST_CASE_BODY(
                false, // TRACE/LOG enabled
                false  // wait for debugger
                )

        client = new FakeClient();
        server = new FakeServer();
        protocol = 0;
        syncTime = QDateTime::fromString("2001.01.01 10:00:00", "yyyy.MM.dd hh:mm:ss");
        syncTime.setTimeSpec(Qt::UTC);
    }

    // Add a record on the client, see it on the server
    // The server will map the id
    void test1_1()
    {
        NEW_SYNC();
        QByteArray added = "<Record><Identifier>client.1</Identifier></Record>";
        QByteArray expected = "<Record><Identifier localIdentifier=\"false\">client.1</Identifier></Record>";
        client->do_add.append(added);
        server->expect_add.append(expected);
        server->expect_map.append("server.1 client.1");

        QVERIFY(performSync());
    }

    // Edit the record on the server, see it edited on the client
    // Note that the mapping is used to translate the id
    void test1_2()
    {
        NEW_SYNC();
        QByteArray record = "<Record><Identifier>server.1</Identifier></Record>";
        server->do_edit.append(record);
        QByteArray expected = "<Record><Identifier>client.1</Identifier></Record>";
        client->expect_edit.append(expected);

        QVERIFY(performSync());
    }

    // Reproduce something a customer has seen...
    // Add a record on the server, see it added on the client
    void test2_1()
    {
        NEW_SYNC();
        server->reference_schema = "<Contact><Identifier/><NameTitle/><FirstName pronunciation=\"\"/><MiddleName/><LastName pronunciation=\"\"/><Suffix/><Company pronunciation=\"\"/><BusinessWebpage/><JobTitle/><Department/><Office/><Profession/><Assistant/><Manager/><Spouse/><Nickname/><Children/><Birthday/><Anniversary/><Notes/><Gender/><Addresses/><PhoneNumbers/><EmailAddresses/><Categories/></Contact>";
        QByteArray record = "<Contact><Identifier>server.2</Identifier><NameTitle></NameTitle><FirstName pronunciation=\"\">Foo</FirstName><MiddleName>Bar</MiddleName><LastName pronunciation=\"\">Baz</LastName><Suffix></Suffix><Company pronunciation=\"\"></Company><BusinessWebpage></BusinessWebpage><JobTitle></JobTitle><Department></Department><Office></Office><Profession></Profession><Assistant></Assistant><Manager></Manager><Spouse></Spouse><Nickname></Nickname><Children></Children><Birthday></Birthday><Anniversary></Anniversary><Notes></Notes><Gender>UnspecifiedGender</Gender><Addresses maxItems=\"2\"><Address type=\"Home\"><Street></Street><City></City><State></State><Zip></Zip><Country></Country></Address><Address type=\"Business\"><Street></Street><City></City><State></State><Zip></Zip><Country></Country></Address></Addresses><PhoneNumbers maxItems=\"7\"><Number type=\"HomePhone\"></Number><Number type=\"HomeMobile\"></Number><Number type=\"HomeFax\"></Number><Number type=\"BusinessPhone\"></Number><Number type=\"BusinessMobile\"></Number><Number type=\"BusinessFax\"></Number><Number type=\"BusinessPager\"></Number></PhoneNumbers><EmailAddresses maxItems=\"3\"><Email></Email><Email></Email><Email></Email></EmailAddresses><Categories></Categories><CustomFields></CustomFields></Contact>";
        // Sorting affects the order of items
        QByteArray expect = "<Contact><Identifier localIdentifier=\"false\">server.2</Identifier><NameTitle></NameTitle><FirstName pronunciation=\"\">Foo</FirstName><MiddleName>Bar</MiddleName><LastName pronunciation=\"\">Baz</LastName><Suffix></Suffix><Company pronunciation=\"\"></Company><BusinessWebpage></BusinessWebpage><JobTitle></JobTitle><Department></Department><Office></Office><Profession></Profession><Assistant></Assistant><Manager></Manager><Spouse></Spouse><Nickname></Nickname><Children></Children><Birthday></Birthday><Anniversary></Anniversary><Notes></Notes><Gender>UnspecifiedGender</Gender><Addresses maxItems=\"2\"><Address type=\"Business\"><Street></Street><City></City><State></State><Zip></Zip><Country></Country></Address><Address type=\"Home\"><Street></Street><City></City><State></State><Zip></Zip><Country></Country></Address></Addresses><PhoneNumbers maxItems=\"7\"><Number type=\"BusinessFax\"></Number><Number type=\"BusinessMobile\"></Number><Number type=\"BusinessPager\"></Number><Number type=\"BusinessPhone\"></Number><Number type=\"HomeFax\"></Number><Number type=\"HomeMobile\"></Number><Number type=\"HomePhone\"></Number></PhoneNumbers><EmailAddresses maxItems=\"3\"><Email></Email><Email></Email><Email></Email></EmailAddresses><Categories></Categories></Contact>";
        server->do_add.append(record);
        client->expect_add.append(expect);
        client->expect_map.append("server.2 client.2");

        QVERIFY(performSync());
    }

    // Edit (without actually changing anything) on the server
    // This is what Outlook does because it doesn't have a change journal
    // Edit (for real) on the client (add an email address)
    // This should produce an edit for the server (which it does)
    // This should not produce an edit for the client (but it does)
    void test2_2()
    {
        NEW_SYNC();
        server->reference_schema = "<Contact><Identifier/><NameTitle/><FirstName pronunciation=\"\"/><MiddleName/><LastName pronunciation=\"\"/><Suffix/><Company pronunciation=\"\"/><BusinessWebpage/><JobTitle/><Department/><Office/><Profession/><Assistant/><Manager/><Spouse/><Nickname/><Children/><Birthday/><Anniversary/><Notes/><Gender/><Addresses/><PhoneNumbers/><EmailAddresses/><Categories/></Contact>";
        QByteArray serverChange = "<Contact><Identifier>server.2</Identifier><NameTitle></NameTitle><FirstName pronunciation=\"\">Foo</FirstName><MiddleName>Bar</MiddleName><LastName pronunciation=\"\">Baz</LastName><Suffix></Suffix><Company pronunciation=\"\"></Company><BusinessWebpage></BusinessWebpage><JobTitle></JobTitle><Department></Department><Office></Office><Profession></Profession><Assistant></Assistant><Manager></Manager><Spouse></Spouse><Nickname></Nickname><Children></Children><Birthday></Birthday><Anniversary></Anniversary><Notes></Notes><Gender>UnspecifiedGender</Gender><Addresses maxItems=\"2\"><Address type=\"Home\"><Street></Street><City></City><State></State><Zip></Zip><Country></Country></Address><Address type=\"Business\"><Street></Street><City></City><State></State><Zip></Zip><Country></Country></Address></Addresses><PhoneNumbers maxItems=\"7\"><Number type=\"HomePhone\"></Number><Number type=\"HomeMobile\"></Number><Number type=\"HomeFax\"></Number><Number type=\"BusinessPhone\"></Number><Number type=\"BusinessMobile\"></Number><Number type=\"BusinessFax\"></Number><Number type=\"BusinessPager\"></Number></PhoneNumbers><EmailAddresses maxItems=\"3\"><Email></Email><Email></Email><Email></Email></EmailAddresses><Categories></Categories></Contact>";
        QByteArray clientChange = "<Contact><Identifier>client.2</Identifier><NameTitle></NameTitle><FirstName pronunciation=\"\">Foo</FirstName><MiddleName>Bar</MiddleName><LastName pronunciation=\"\">Baz</LastName><Suffix></Suffix><Company pronunciation=\"\"></Company><BusinessWebpage></BusinessWebpage><JobTitle></JobTitle><Department></Department><Office></Office><Profession></Profession><Assistant></Assistant><Manager></Manager><HomeWebpage></HomeWebpage><Spouse></Spouse><Nickname></Nickname><Children></Children><Birthday></Birthday><Anniversary></Anniversary><Portrait></Portrait><Notes></Notes><Gender></Gender><Addresses></Addresses><PhoneNumbers></PhoneNumbers><EmailAddresses default=\"foo@bar.com\"><Email>foo@bar.com</Email></EmailAddresses><Categories></Categories><CustomFields></CustomFields></Contact>";
        QByteArray expected = "<Contact><Identifier>server.2</Identifier><NameTitle></NameTitle><FirstName pronunciation=\"\">Foo</FirstName><MiddleName>Bar</MiddleName><LastName pronunciation=\"\">Baz</LastName><Suffix></Suffix><Company pronunciation=\"\"></Company><BusinessWebpage></BusinessWebpage><JobTitle></JobTitle><Department></Department><Office></Office><Profession></Profession><Assistant></Assistant><Manager></Manager><Spouse></Spouse><Nickname></Nickname><Children></Children><Birthday></Birthday><Anniversary></Anniversary><Notes></Notes><Gender></Gender><Addresses></Addresses><PhoneNumbers></PhoneNumbers><EmailAddresses default=\"foo@bar.com\"><Email>foo@bar.com</Email></EmailAddresses><Categories></Categories></Contact>";
        server->do_edit.append(serverChange);
        client->do_edit.append(clientChange);
        server->expect_edit.append(expected);

        QVERIFY(performSync());
    }

    // Actually edit the record on the server
    void test2_3()
    {
        NEW_SYNC();
        server->reference_schema = "<Contact><Identifier/><NameTitle/><FirstName pronunciation=\"\"/><MiddleName/><LastName pronunciation=\"\"/><Suffix/><Company pronunciation=\"\"/><BusinessWebpage/><JobTitle/><Department/><Office/><Profession/><Assistant/><Manager/><Spouse/><Nickname/><Children/><Birthday/><Anniversary/><Notes/><Gender/><Addresses/><PhoneNumbers/><EmailAddresses/><Categories/></Contact>";
        QByteArray serverChange = "<Contact><Identifier>server.2</Identifier><NameTitle></NameTitle><FirstName pronunciation=\"\">Foo</FirstName><MiddleName>Bar</MiddleName><LastName pronunciation=\"\">Baz</LastName><Suffix></Suffix><Company pronunciation=\"\"></Company><BusinessWebpage></BusinessWebpage><JobTitle></JobTitle><Department></Department><Office></Office><Profession></Profession><Assistant></Assistant><Manager></Manager><Spouse></Spouse><Nickname></Nickname><Children></Children><Birthday></Birthday><Anniversary></Anniversary><Notes></Notes><Gender>UnspecifiedGender</Gender><Addresses maxItems=\"2\"><Address type=\"Home\"><Street>745 street name</Street><City>city name</City><State>state</State><Zip>11111</Zip><Country>Austrealia</Country></Address><Address type=\"Business\"><Street></Street><City></City><State></State><Zip></Zip><Country></Country></Address></Addresses><PhoneNumbers maxItems=\"7\"><Number type=\"HomePhone\">555-5555</Number><Number type=\"HomeMobile\"></Number><Number type=\"HomeFax\"></Number><Number type=\"BusinessPhone\"></Number><Number type=\"BusinessMobile\"></Number><Number type=\"BusinessFax\"></Number><Number type=\"BusinessPager\"></Number></PhoneNumbers><EmailAddresses maxItems=\"3\"><Email>foo@bar.com</Email><Email></Email><Email></Email></EmailAddresses><Categories></Categories></Contact>";
        // Sorting changes the order of items
        QByteArray expect = "<Contact><Identifier>client.2</Identifier><NameTitle></NameTitle><FirstName pronunciation=\"\">Foo</FirstName><MiddleName>Bar</MiddleName><LastName pronunciation=\"\">Baz</LastName><Suffix></Suffix><Company pronunciation=\"\"></Company><BusinessWebpage></BusinessWebpage><JobTitle></JobTitle><Department></Department><Office></Office><Profession></Profession><Assistant></Assistant><Manager></Manager><Spouse></Spouse><Nickname></Nickname><Children></Children><Birthday></Birthday><Anniversary></Anniversary><Notes></Notes><Gender>UnspecifiedGender</Gender><Addresses maxItems=\"2\"><Address type=\"Home\"><Street>745 street name</Street><City>city name</City><State>state</State><Zip>11111</Zip><Country>Austrealia</Country></Address><Address type=\"Business\"><Street></Street><City></City><State></State><Zip></Zip><Country></Country></Address></Addresses><PhoneNumbers maxItems=\"7\"><Number type=\"HomePhone\">555-5555</Number><Number type=\"BusinessFax\"></Number><Number type=\"BusinessMobile\"></Number><Number type=\"BusinessPager\"></Number><Number type=\"BusinessPhone\"></Number><Number type=\"HomeFax\"></Number><Number type=\"HomeMobile\"></Number></PhoneNumbers><EmailAddresses maxItems=\"3\"><Email>foo@bar.com</Email><Email></Email><Email></Email></EmailAddresses><Categories></Categories></Contact>";
        server->do_edit.append(serverChange);
        client->expect_edit.append(expect);

        QVERIFY(performSync());
    }

    // Bogus edit on the server
    // Delete on the client
    // Note that this tests the conflict remove/edit where the server does and edit and the client does a remove.
    void test3_1()
    {
        NEW_SYNC();
        server->reference_schema = "<Contact><Identifier/><NameTitle/><FirstName pronunciation=\"\"/><MiddleName/><LastName pronunciation=\"\"/><Suffix/><Company pronunciation=\"\"/><BusinessWebpage/><JobTitle/><Department/><Office/><Profession/><Assistant/><Manager/><Spouse/><Nickname/><Children/><Birthday/><Anniversary/><Notes/><Gender/><Addresses/><PhoneNumbers/><EmailAddresses/><Categories/></Contact>";
        QByteArray serverChange = "<Contact><Identifier>server.2</Identifier><NameTitle></NameTitle><FirstName pronunciation=\"\">Foo</FirstName><MiddleName>Bar</MiddleName><LastName pronunciation=\"\">Baz</LastName><Suffix></Suffix><Company pronunciation=\"\"></Company><BusinessWebpage></BusinessWebpage><JobTitle></JobTitle><Department></Department><Office></Office><Profession></Profession><Assistant></Assistant><Manager></Manager><Spouse></Spouse><Nickname></Nickname><Children></Children><Birthday></Birthday><Anniversary></Anniversary><Notes></Notes><Gender>UnspecifiedGender</Gender><Addresses maxItems=\"2\"><Address type=\"Home\"><Street>745 street name</Street><City>city name</City><State>state</State><Zip>11111</Zip><Country>Austrealia</Country></Address><Address type=\"Business\"><Street></Street><City></City><State></State><Zip></Zip><Country></Country></Address></Addresses><PhoneNumbers maxItems=\"7\"><Number type=\"HomePhone\">555-5555</Number><Number type=\"HomeMobile\"></Number><Number type=\"HomeFax\"></Number><Number type=\"BusinessPhone\"></Number><Number type=\"BusinessMobile\"></Number><Number type=\"BusinessFax\"></Number><Number type=\"BusinessPager\"></Number></PhoneNumbers><EmailAddresses maxItems=\"3\"><Email>foo@bar.com</Email><Email></Email><Email></Email></EmailAddresses><Categories></Categories></Contact>";
        server->do_edit.append(serverChange);
        client->do_delete.append("client.2");
        server->expect_delete.append("server.2");

        QVERIFY(performSync());
    }

    // Conflict add/add
    // Client wins (edit on the server)
    // Note that this doesn't happen in the real world because Outlook and Qtopia create different XML for the same record.
    void test4_1()
    {
        NEW_SYNC();
        QByteArray clientEdit = "<Record><Identifier>client.4</Identifier> <f1>foo</f1><f2>foo</f2></Record>";
        QByteArray serverEdit = "<Record><Identifier>server.4</Identifier><f1>foo</f1><f2>foo</f2></Record>";
        QByteArray serverExpected = "<Record><Identifier>server.4</Identifier><f1>foo</f1><f2>foo</f2></Record>";
        server->do_add.append(serverEdit);
        client->do_add.append(clientEdit);
        server->expect_edit.append(serverExpected);

        QVERIFY(performSync());
    }

    // Conflict edit/edit
    // Client wins (edit on the server)
    void test4_2()
    {
        NEW_SYNC();
        QByteArray clientEdit = "<Record><Identifier>client.4</Identifier><f1>foo</f1><f2>client</f2></Record>";
        QByteArray serverEdit = "<Record><Identifier>server.4</Identifier><f1>client</f1><f2>foo</f2></Record>";
        QByteArray sExpected = "<Record><Identifier>server.4</Identifier><f1>foo</f1><f2>client</f2></Record>";
        server->do_edit.append(serverEdit);
        client->do_edit.append(clientEdit);
        server->expect_edit.append(sExpected);

        QVERIFY(performSync());
    }

    // Conflict remove/edit
    // Remove wins
    // This tests the client doing an edit and the server doing a delete. Test 3_1 tests the other way around.
    void test4_3()
    {
        NEW_SYNC();
        QByteArray clientEdit = "<Record><Identifier>client.4</Identifier><f1>foo</f1><f2>client</f2></Record>";
        QByteArray serverDelete = "server.4";
        QByteArray cExpected = "client.4";
        client->do_edit.append(clientEdit);
        server->do_delete.append(serverDelete);
        client->expect_delete.append(cExpected);

        QVERIFY(performSync());
    }

    // Advance the time on the desktop by 1 day then do a sync.
    void test5_1()
    {
        NEW_SYNC();
        server->nextSync = server->nextSync.addDays(1);

        QVERIFY(performSync());
    }

    // Keeping the desktop advanced by a day, add a record on the client.
    void test5_2()
    {
        NEW_SYNC();
        server->nextSync = server->nextSync.addDays(1);

        QByteArray added = "<Record><Identifier>client.5</Identifier></Record>";
        QByteArray expected = "<Record><Identifier localIdentifier=\"false\">client.5</Identifier></Record>";
        client->do_add.append(added);
        server->expect_add.append(expected);
        server->expect_map.append("server.5 client.5");

        QVERIFY(performSync());
    }

    // Keeping the desktop advanced by a day, edit a record on the server.
    void test5_3()
    {
        NEW_SYNC();
        server->nextSync = server->nextSync.addDays(1);

        QByteArray record = "<Record><Identifier>server.5</Identifier></Record>";
        server->do_edit.append(record);
        QByteArray expected = "<Record><Identifier>client.5</Identifier></Record>";
        client->expect_edit.append(expected);

        QVERIFY(performSync());
    }

    void cleanupTestCase()
    {
        delete protocol;
        protocol = 0;
    }

private:
#undef QVERIFY
#define QVERIFY(x) if ( !(x) ) return false
#define SANITIZE(array) QString(array).replace("\n","").replace(QRegExp("> +<"), "><").trimmed().toLocal8Bit()

    bool performSync()
    {
        QSignalSpy spy(protocol, SIGNAL(syncComplete()));
        QSignalSpy spy2(protocol, SIGNAL(syncError(QString)));
        // We can't inject this so set it now (a special define means it won't get clobbered)
        protocol->m_serverNextSync = server->nextSync;
        protocol->startSync(client, server);
        // Async reply... just idle the event loop while we wait
        //qDebug() << "waiting for spy" << __LINE__;
        while ( spy.count() == 0 && spy2.count() == 0 )
            qApp->processEvents();

        if ( spy2.count() ) {
            qWarning() << "Sync Error!" << spy2.takeFirst().at(0);
            return false;
        }

        bool all_expected_found = true;
        bool all_accounted_for = true;
#define TEST(server,Add,QByteArray)\
        foreach ( const QByteArray &_expected, server->expect_##Add ) {\
            QByteArray expected = SANITIZE(_expected);\
            bool found = false;\
            foreach ( const QByteArray &_added, server->did_##Add ) {\
                QByteArray added = SANITIZE(_added);\
                if ( expected == added ) {\
                    server->did_##Add.removeAll(_added);\
                    found = true;\
                    break;\
                }\
            }\
            if ( !found ) {\
                qWarning() << "ERROR: Missing expected `" #Add "' on the " #server << endl << expected;\
                all_expected_found = false;\
            }\
        }\
        foreach ( const QByteArray &_added, server->did_##Add ) {\
            QByteArray added = SANITIZE(_added);\
            qWarning() << "ERROR: Unexpected `" #Add "' on the " #server << endl << added;\
            all_accounted_for = false;\
        }

        TEST(server,add,QByteArray);
        TEST(server,edit,QByteArray);
        TEST(server,delete,QString);
        TEST(server,map,QString);
        TEST(client,add,QByteArray);
        TEST(client,edit,QByteArray);
        TEST(client,delete,QString);
        TEST(client,map,QString);
        QVERIFY(all_expected_found && all_accounted_for);
        return true;
    }
};
QTEST_MAIN(syncmerge)

#include "main.moc"
