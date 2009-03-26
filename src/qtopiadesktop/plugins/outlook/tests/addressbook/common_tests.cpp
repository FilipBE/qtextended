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

#define OUTLOOK_CAPS(x)\
    do {\
        QString data = expected;\
        QRegExp re( "([a-z]+)</" #x );\
        if ( re.indexIn( data ) != -1 ) {\
            QString orig = re.cap( 0 );\
            QString copy = orig;\
            copy[0] = copy[0].toUpper();\
            expected = data.replace( orig, copy ).toLocal8Bit();\
        }\
    } while ( 0 )

void cleanOutAddressbook1()
{
    QVERIFY(cleanOutTable());
    QVERIFY(checkForEmptyTable());
}

void addClientRecord1()
{
    QByteArray newRecord =
"<Contact>\n"
"    <Identifier localIdentifier=\"false\">Client Identifier</Identifier>\n"
"    <FirstName>John &amp; Jane</FirstName>\n"
"    <LastName pronunciation=\"d-oh\">Doe</LastName>\n"
"    <Gender>Male</Gender>\n"
"    <Addresses>\n"
"        <Address type=\"Home\">\n"
"            <Street>24 Elm</Street>\n"
"            <City>Wooddale</City>\n"
"            <Zip>1111</Zip>\n"
"        </Address>\n"
"    </Addresses>\n"
"    <PhoneNumbers default=\"588224\">\n"
"        <Number type=\"HomePhone\">334224</Number>\n"
"        <Number type=\"HomeMobile\">588224</Number>\n"
"        <Number type=\"HomeFax\">312381</Number>\n"
"    </PhoneNumbers>\n"
"    <EmailAddresses default=\"john.doe@home.net\">\n"
"        <Email>john.doe@home.net</Email>\n"
"        <Email>john.doe@work.com</Email>\n"
"    </EmailAddresses>\n"
"</Contact>\n";
    QVERIFY(addClientRecord( newRecord ));
}

void checkForAddedItem1()
{
    QByteArray expected =
"<Contact>\n"
"<Identifier>Client Identifier</Identifier>\n"
"<NameTitle></NameTitle>\n"
"<FirstName pronunciation=\"\">John &amp; Jane</FirstName>\n"
"<MiddleName></MiddleName>\n"
"<LastName pronunciation=\"d-oh\">Doe</LastName>\n"
"<Suffix></Suffix>\n"
"<Company pronunciation=\"\"></Company>\n"
"<BusinessWebpage></BusinessWebpage>\n"
"<JobTitle></JobTitle>\n"
"<Department></Department>\n"
"<Office></Office>\n"
"<Profession></Profession>\n"
"<Assistant></Assistant>\n"
"<Manager></Manager>\n"
#ifdef Q_WS_QWS // Qtopia supports HomeWebpage
"<HomeWebpage></HomeWebpage>\n"
#endif
"<Spouse></Spouse>\n"
"<Nickname></Nickname>\n"
"<Children></Children>\n"
"<Birthday></Birthday>\n"
"<Anniversary></Anniversary>\n"
#ifdef Q_WS_QWS // Qtopia supports Portrait
"<Portrait></Portrait>\n"
#endif
"<Notes></Notes>\n"
"<Gender>Male</Gender>\n"
#ifdef Q_WS_QWS // Qtopia supports unlimited addresses?
"<Addresses>\n"
#else // Outlook only supports 2 addresses (mapped to specific fields)
"<Addresses maxItems=\"2\">\n"
#endif
"<Address type=\"Home\">\n"
"<Street>24 Elm</Street>\n"
"<City>Wooddale</City>\n"
"<State></State>\n"
"<Zip>1111</Zip>\n"
"<Country></Country>\n"
"</Address>\n"
#ifdef Q_WS_QWS // Qtopia sends a list and ignores things not present
#else // Outlook always sends both "home" and "business" addresses
"<Address type=\"Business\">\n"
"<Street></Street>\n"
"<City></City>\n"
"<State></State>\n"
"<Zip></Zip>\n"
"<Country></Country>\n"
"</Address>\n"
#endif
"</Addresses>\n"
#ifdef Q_WS_QWS // Qtopia supports unlimited phone numbers? Stores a default value.
"<PhoneNumbers default=\"588224\">\n"
#else // Outlook only supports 7 phone numbers (mapped to specific fields). Doesn't store a default value.
"<PhoneNumbers maxItems=\"7\">\n"
#endif
"<Number type=\"HomePhone\">334224</Number>\n"
"<Number type=\"HomeMobile\">588224</Number>\n"
"<Number type=\"HomeFax\">312381</Number>\n"
#ifdef Q_WS_QWS // Qtopia only sends numbers that have values
#else // Outlook sends all numbers
"<Number type=\"BusinessPhone\"></Number>\n"
"<Number type=\"BusinessMobile\"></Number>\n"
"<Number type=\"BusinessFax\"></Number>\n"
"<Number type=\"BusinessPager\"></Number>\n"
#endif
"</PhoneNumbers>\n"
#ifdef Q_WS_QWS // Qtopia supports unlimited email addresses? Stores a default value.
"<EmailAddresses default=\"john.doe@home.net\">\n"
#else // Outlook only supports 3 email addresses (mapped to specific fields). Doesn't store a default value.
"<EmailAddresses maxItems=\"3\">\n"
#endif
"<Email>john.doe@home.net</Email>\n"
"<Email>john.doe@work.com</Email>\n"
#ifdef Q_WS_QWS // Qtopia only sends items that have values
#else // Outlook sends all email addresses
"<Email></Email>\n"
#endif
"</EmailAddresses>\n"
"<Categories>\n"
"</Categories>\n"
#ifdef Q_WS_QWS
"<CustomFields>\n"
"</CustomFields>\n"
#endif
"</Contact>\n";
    QVERIFY(checkForAddedItem( expected ));
}

void cleanOutAddressbook2()
{
    QVERIFY(cleanOutTable());
    QVERIFY(checkForEmptyTable());
}

void addClientRecord2()
{
    QByteArray newRecord =
"<Contact>\n"
"<Identifier localIdentifier=\"false\">Client Identifier 2</Identifier>\n"
"<NameTitle>title</NameTitle>\n"
"<FirstName pronunciation=\"fpron\">fname</FirstName>\n"
"<MiddleName>mid</MiddleName>\n"
"<LastName pronunciation=\"lpron\">lname</LastName>\n"
"<Suffix>suf</Suffix>\n"
"<Company pronunciation=\"comppron\">company</Company>\n"
"<BusinessWebpage>http://foo.com/</BusinessWebpage>\n"
"<JobTitle>jobtitle</JobTitle>\n"
"<Department>dept</Department>\n"
"<Office>office</Office>\n"
"<Profession>prof</Profession>\n"
"<Assistant>assistant</Assistant>\n"
"<Manager>manager</Manager>\n"
"<Spouse>spouse</Spouse>\n"
"<Nickname>nick</Nickname>\n"
"<Children>kids</Children>\n"
"<Birthday>2001-02-02</Birthday>\n"
"<Anniversary>2002-03-04</Anniversary>\n"
"<Notes>notes</Notes>\n"
"<Gender>UnspecifiedGender</Gender>\n"
"<Addresses>\n"
"<Address type=\"Home\">\n"
"<Street>street</Street>\n"
"<City>city</City>\n"
"<State>state</State>\n"
"<Zip>zip</Zip>\n"
"<Country>count</Country>\n"
"</Address>\n"
"<Address type=\"Business\">\n"
"<Street>bs</Street>\n"
"<City>bc</City>\n"
"<State>bst</State>\n"
"<Zip>bz</Zip>\n"
"<Country>bco</Country>\n"
"</Address>\n"
"</Addresses>\n"
"<PhoneNumbers>\n"
"<Number type=\"HomePhone\">1</Number>\n"
"<Number type=\"HomeMobile\">2</Number>\n"
"<Number type=\"HomeFax\">3</Number>\n"
"<Number type=\"BusinessPhone\">4</Number>\n"
"<Number type=\"BusinessMobile\">5</Number>\n"
"<Number type=\"BusinessFax\">6</Number>\n"
"<Number type=\"BusinessPager\">7</Number>\n"
"</PhoneNumbers>\n"
"<EmailAddresses>\n"
"<Email>1@home.net</Email>\n"
"<Email>2@work.com</Email>\n"
"<Email>3@bar.com</Email>\n"
"</EmailAddresses>\n"
"<Categories>\n"
"<Category>Foo</Category>\n"
"</Categories>\n"
"</Contact>\n";
#ifdef Q_WS_QWS // Qtopia can't add categories to an item in the unittest
                // unless the category already exists in the database.
    { QCategoryManager().add("Foo"); }
#endif
    QVERIFY(addClientRecord( newRecord ));
}

void checkForAddedItem2()
{
    QByteArray expected =
"<Contact>\n"
"<Identifier>Client Identifier 2</Identifier>\n"
"<NameTitle>title</NameTitle>\n"
"<FirstName pronunciation=\"fpron\">fname</FirstName>\n"
"<MiddleName>mid</MiddleName>\n"
"<LastName pronunciation=\"lpron\">lname</LastName>\n"
"<Suffix>suf</Suffix>\n"
"<Company pronunciation=\"comppron\">company</Company>\n"
"<BusinessWebpage>http://foo.com/</BusinessWebpage>\n"
"<JobTitle>jobtitle</JobTitle>\n"
"<Department>dept</Department>\n"
"<Office>office</Office>\n"
"<Profession>prof</Profession>\n"
"<Assistant>assistant</Assistant>\n"
"<Manager>manager</Manager>\n"
#ifdef Q_WS_QWS // Qtopia supports HomeWebpage
"<HomeWebpage></HomeWebpage>\n"
#endif
"<Spouse>spouse</Spouse>\n"
"<Nickname>nick</Nickname>\n"
"<Children>kids</Children>\n"
"<Birthday>2001-02-02</Birthday>\n"
"<Anniversary>2002-03-04</Anniversary>\n"
#ifdef Q_WS_QWS // Qtopia supports Portrait
"<Portrait></Portrait>\n"
#endif
"<Notes>notes</Notes>\n"
#ifdef Q_WS_QWS // Qtopia returns an empty string (means the same as UnspecifiedGender)
"<Gender></Gender>\n"
#else // Outlook returns UnspecifiedGender
"<Gender>UnspecifiedGender</Gender>\n"
#endif
#ifdef Q_WS_QWS // Qtopia supports unlimited addresses?
"<Addresses>\n"
#else // Outlook only supports 2 addresses (mapped to specific fields)
"<Addresses maxItems=\"2\">\n"
#endif
"<Address type=\"Home\">\n"
"<Street>street</Street>\n"
"<City>city</City>\n"
"<State>state</State>\n"
"<Zip>zip</Zip>\n"
"<Country>count</Country>\n"
"</Address>\n"
"<Address type=\"Business\">\n"
"<Street>bs</Street>\n"
"<City>bc</City>\n"
"<State>bst</State>\n"
"<Zip>bz</Zip>\n"
"<Country>bco</Country>\n"
"</Address>\n"
"</Addresses>\n"
#ifdef Q_WS_QWS // Qtopia supports unlimited phone numbers? Stores a default value.
"<PhoneNumbers default=\"1\">\n"
#else // Outlook only supports 7 phone numbers (mapped to specific fields). Doesn't store a default value.
"<PhoneNumbers maxItems=\"7\">\n"
#endif
#ifdef Q_WS_QWS // Qtopia returns the numbers in this order
"<Number type=\"HomePhone\">1</Number>\n"
"<Number type=\"BusinessPhone\">4</Number>\n"
"<Number type=\"HomeMobile\">2</Number>\n"
"<Number type=\"BusinessMobile\">5</Number>\n"
"<Number type=\"HomeFax\">3</Number>\n"
"<Number type=\"BusinessFax\">6</Number>\n"
"<Number type=\"BusinessPager\">7</Number>\n"
#else // Outlook returns the numbers in this order
"<Number type=\"HomePhone\">1</Number>\n"
"<Number type=\"HomeMobile\">2</Number>\n"
"<Number type=\"HomeFax\">3</Number>\n"
"<Number type=\"BusinessPhone\">4</Number>\n"
"<Number type=\"BusinessMobile\">5</Number>\n"
"<Number type=\"BusinessFax\">6</Number>\n"
"<Number type=\"BusinessPager\">7</Number>\n"
#endif
"</PhoneNumbers>\n"
#ifdef Q_WS_QWS // Qtopia supports unlimited email addresses? Stores a default value.
"<EmailAddresses default=\"1@home.net\">\n"
#else // Outlook only supports 3 email addresses (mapped to specific fields). Doesn't store a default value.
"<EmailAddresses maxItems=\"3\">\n"
#endif
"<Email>1@home.net</Email>\n"
"<Email>2@work.com</Email>\n"
"<Email>3@bar.com</Email>\n"
"</EmailAddresses>\n"
"<Categories>\n"
"<Category>Foo</Category>\n"
"</Categories>\n"
#ifdef Q_WS_QWS
"<CustomFields>\n"
"</CustomFields>\n"
#endif
"</Contact>\n";
#ifndef Q_WS_QWS // Outlook auto-captializes some fields
    OUTLOOK_CAPS(NameTitle);
    OUTLOOK_CAPS(FirstName);
    OUTLOOK_CAPS(MiddleName);
    OUTLOOK_CAPS(LastName);
    OUTLOOK_CAPS(Suffix);
    OUTLOOK_CAPS(Company);
    OUTLOOK_CAPS(JobTitle);
    OUTLOOK_CAPS(Assistant);
    OUTLOOK_CAPS(Manager);
    OUTLOOK_CAPS(Spouse);
    OUTLOOK_CAPS(Nickname);
    OUTLOOK_CAPS(Children);
#endif
    QVERIFY(checkForAddedItem( expected ));
}

#ifdef Q_WS_QWS
void cleanOutAddressbook3()
{
    QVERIFY(cleanOutTable());
    QVERIFY(checkForEmptyTable());
}

void addServerRecord3()
{
    QByteArray newRecord = "<Contact><Identifier localIdentifier=\"false\">server.2</Identifier><NameTitle></NameTitle><FirstName pronunciation=\"\">Foo</FirstName><MiddleName>Bar</MiddleName><LastName pronunciation=\"\">Baz</LastName><Suffix></Suffix><Company pronunciation=\"\"></Company><BusinessWebpage></BusinessWebpage><JobTitle></JobTitle><Department></Department><Office></Office><Profession></Profession><Assistant></Assistant><Manager></Manager><Spouse></Spouse><Nickname></Nickname><Children></Children><Birthday></Birthday><Anniversary></Anniversary><Notes></Notes><Gender>UnspecifiedGender</Gender><Addresses maxItems=\"2\"><Address type=\"Home\"><Street>745 street name</Street><City>city name</City><State>state</State><Zip>11111</Zip><Country>Austrealia</Country></Address><Address type=\"Business\"><Street></Street><City></City><State></State><Zip></Zip><Country></Country></Address></Addresses><PhoneNumbers maxItems=\"7\"><Number type=\"HomePhone\">555-5555</Number><Number type=\"HomeMobile\"></Number><Number type=\"HomeFax\"></Number><Number type=\"BusinessPhone\"></Number><Number type=\"BusinessMobile\"></Number><Number type=\"BusinessFax\"></Number><Number type=\"BusinessPager\"></Number></PhoneNumbers><EmailAddresses maxItems=\"3\"><Email>foo@bar.com</Email><Email></Email><Email></Email></EmailAddresses><Categories></Categories></Contact>";
    QVERIFY(addClientRecord(newRecord));
}

void checkForAddedItem3()
{
    QByteArray expected = "<Contact><Identifier>server.2</Identifier><NameTitle></NameTitle><FirstName pronunciation=\"\">Foo</FirstName><MiddleName>Bar</MiddleName><LastName pronunciation=\"\">Baz</LastName><Suffix></Suffix><Company pronunciation=\"\"></Company><BusinessWebpage></BusinessWebpage><JobTitle></JobTitle><Department></Department><Office></Office><Profession></Profession><Assistant></Assistant><Manager></Manager><HomeWebpage></HomeWebpage><Spouse></Spouse><Nickname></Nickname><Children></Children><Birthday></Birthday><Anniversary></Anniversary><Portrait></Portrait><Notes></Notes><Gender></Gender><Addresses><Address type=\"Home\"><Street>745 street name</Street><City>city name</City><State>state</State><Zip>11111</Zip><Country>Austrealia</Country></Address></Addresses><PhoneNumbers default=\"555-5555\"><Number type=\"HomePhone\">555-5555</Number></PhoneNumbers><EmailAddresses default=\"foo@bar.com\"><Email>foo@bar.com</Email></EmailAddresses><Categories></Categories><CustomFields></CustomFields></Contact>";
    QVERIFY(checkForAddedItem(expected));
}
#endif

void cleanOutAddressbook4()
{
    QVERIFY(cleanOutTable());
    QVERIFY(checkForEmptyTable());
}

void addClientRecord4()
{
    QByteArray newRecord =
"<Contact>\n"
"<Identifier localIdentifier=\"false\">Client Identifier 4</Identifier>\n"
"<NameTitle>title</NameTitle>\n"
"<FirstName pronunciation=\"fpron\">fname</FirstName>\n"
"<MiddleName>mid</MiddleName>\n"
"<LastName pronunciation=\"lpron\">lname</LastName>\n"
"<Suffix>suf</Suffix>\n"
"<Company pronunciation=\"comppron\"></Company>\n"
"<BusinessWebpage>http://foo.com/</BusinessWebpage>\n"
"<JobTitle>jobtitle</JobTitle>\n"
"<Department>dept</Department>\n"
"<Office>office</Office>\n"
"<Profession>prof</Profession>\n"
"<Assistant>assistant</Assistant>\n"
"<Manager>manager</Manager>\n"
"<Spouse>spouse</Spouse>\n"
"<Nickname>nick</Nickname>\n"
"<Children>kids</Children>\n"
"<Birthday>2001-02-02</Birthday>\n"
"<Anniversary>2002-03-04</Anniversary>\n"
"<Notes>notes</Notes>\n"
"<Gender>UnspecifiedGender</Gender>\n"
"<Addresses>\n"
"<Address type=\"Home\">\n"
"<Street>street</Street>\n"
"<City>city</City>\n"
"<State>state</State>\n"
"<Zip>zip</Zip>\n"
"<Country>count</Country>\n"
"</Address>\n"
"<Address type=\"Business\">\n"
"<Street>bs</Street>\n"
"<City>bc</City>\n"
"<State>bst</State>\n"
"<Zip>bz</Zip>\n"
"<Country>bco</Country>\n"
"</Address>\n"
"</Addresses>\n"
"<PhoneNumbers>\n"
"<Number type=\"HomePhone\">1</Number>\n"
"</PhoneNumbers>\n"
"<EmailAddresses>\n"
"<Email>1@home.net</Email>\n"
"</EmailAddresses>\n"
"<Categories>\n"
"</Categories>\n"
"</Contact>\n";
    QVERIFY(addClientRecord( newRecord ));
}

void checkForAddedItem4()
{
    QByteArray expected =
"<Contact>\n"
"<Identifier>Client Identifier 4</Identifier>\n"
"<NameTitle>title</NameTitle>\n"
"<FirstName pronunciation=\"fpron\">fname</FirstName>\n"
"<MiddleName>mid</MiddleName>\n"
"<LastName pronunciation=\"lpron\">lname</LastName>\n"
"<Suffix>suf</Suffix>\n"
"<Company pronunciation=\"comppron\"></Company>\n"
"<BusinessWebpage>http://foo.com/</BusinessWebpage>\n"
"<JobTitle>jobtitle</JobTitle>\n"
"<Department>dept</Department>\n"
"<Office>office</Office>\n"
"<Profession>prof</Profession>\n"
"<Assistant>assistant</Assistant>\n"
"<Manager>manager</Manager>\n"
#ifdef Q_WS_QWS // Qtopia supports HomeWebpage
"<HomeWebpage></HomeWebpage>\n"
#endif
"<Spouse>spouse</Spouse>\n"
"<Nickname>nick</Nickname>\n"
"<Children>kids</Children>\n"
"<Birthday>2001-02-02</Birthday>\n"
"<Anniversary>2002-03-04</Anniversary>\n"
#ifdef Q_WS_QWS // Qtopia supports Portrait
"<Portrait></Portrait>\n"
#endif
"<Notes>notes</Notes>\n"
#ifdef Q_WS_QWS // Qtopia returns an empty string (means the same as UnspecifiedGender)
"<Gender></Gender>\n"
#else // Outlook returns UnspecifiedGender
"<Gender>UnspecifiedGender</Gender>\n"
#endif
#ifdef Q_WS_QWS // Qtopia supports unlimited addresses?
"<Addresses>\n"
#else // Outlook only supports 2 addresses (mapped to specific fields)
"<Addresses maxItems=\"2\">\n"
#endif
"<Address type=\"Home\">\n"
"<Street>street</Street>\n"
"<City>city</City>\n"
"<State>state</State>\n"
"<Zip>zip</Zip>\n"
"<Country>count</Country>\n"
"</Address>\n"
"<Address type=\"Business\">\n"
"<Street>bs</Street>\n"
"<City>bc</City>\n"
"<State>bst</State>\n"
"<Zip>bz</Zip>\n"
"<Country>bco</Country>\n"
"</Address>\n"
"</Addresses>\n"
#ifdef Q_WS_QWS // Qtopia supports unlimited phone numbers? Stores a default value.
"<PhoneNumbers default=\"1\">\n"
#else // Outlook only supports 7 phone numbers (mapped to specific fields). Doesn't store a default value.
"<PhoneNumbers maxItems=\"7\">\n"
#endif
#ifdef Q_WS_QWS // Qtopia returns the numbers in this order
"<Number type=\"HomePhone\">1</Number>\n"
#else // Outlook returns the numbers in this order
"<Number type=\"HomePhone\">1</Number>\n"
"<Number type=\"HomeMobile\"></Number>\n"
"<Number type=\"HomeFax\"></Number>\n"
"<Number type=\"BusinessPhone\"></Number>\n"
"<Number type=\"BusinessMobile\"></Number>\n"
"<Number type=\"BusinessFax\"></Number>\n"
"<Number type=\"BusinessPager\"></Number>\n"
#endif
"</PhoneNumbers>\n"
#ifdef Q_WS_QWS // Qtopia supports unlimited email addresses? Stores a default value.
"<EmailAddresses default=\"1@home.net\">\n"
#else // Outlook only supports 3 email addresses (mapped to specific fields). Doesn't store a default value.
"<EmailAddresses maxItems=\"3\">\n"
#endif
"<Email>1@home.net</Email>\n"
#ifndef Q_WS_QWS
"<Email></Email>\n"
"<Email></Email>\n"
#endif
"</EmailAddresses>\n"
"<Categories>\n"
"</Categories>\n"
#ifdef Q_WS_QWS
"<CustomFields>\n"
"</CustomFields>\n"
#endif
"</Contact>\n";
#ifndef Q_WS_QWS // Outlook auto-captializes some fields
    OUTLOOK_CAPS(NameTitle);
    OUTLOOK_CAPS(FirstName);
    OUTLOOK_CAPS(MiddleName);
    OUTLOOK_CAPS(LastName);
    OUTLOOK_CAPS(Suffix);
    OUTLOOK_CAPS(Company);
    OUTLOOK_CAPS(JobTitle);
    OUTLOOK_CAPS(Assistant);
    OUTLOOK_CAPS(Manager);
    OUTLOOK_CAPS(Spouse);
    OUTLOOK_CAPS(Nickname);
    OUTLOOK_CAPS(Children);
#endif
    QVERIFY(checkForAddedItem( expected ));
}

void editClientRecord4()
{
    QByteArray newRecord =
"<Contact>\n"
"<Identifier>Client Identifier 4</Identifier>\n"
"<NameTitle>title</NameTitle>\n"
"<FirstName pronunciation=\"fpron\">fname</FirstName>\n"
"<MiddleName>mid</MiddleName>\n"
"<LastName pronunciation=\"lpron\">lname</LastName>\n"
"<Suffix>suf</Suffix>\n"
"<Company pronunciation=\"comppron\"></Company>\n"
"<BusinessWebpage>http://foo.com/</BusinessWebpage>\n"
"<JobTitle>jobtitle</JobTitle>\n"
"<Department>dept</Department>\n"
"<Office>office</Office>\n"
"<Profession>prof</Profession>\n"
"<Assistant>assistant</Assistant>\n"
"<Manager>manager</Manager>\n"
"<Spouse>spouse</Spouse>\n"
"<Nickname>nick</Nickname>\n"
"<Children>kids</Children>\n"
"<Birthday>2001-02-02</Birthday>\n"
"<Anniversary>2002-03-04</Anniversary>\n"
"<Notes>notes</Notes>\n"
"<Gender>UnspecifiedGender</Gender>\n"
"<Addresses>\n"
"<Address type=\"Home\">\n"
"<Street>street</Street>\n"
"<City>city</City>\n"
"<State>state</State>\n"
"<Zip>zip</Zip>\n"
"<Country>count</Country>\n"
"</Address>\n"
"<Address type=\"Business\">\n"
"<Street>bs</Street>\n"
"<City>bc</City>\n"
"<State>bst</State>\n"
"<Zip>bz</Zip>\n"
"<Country>bco</Country>\n"
"</Address>\n"
"</Addresses>\n"
#ifdef Q_WS_QWS
"<PhoneNumbers maxItems=\"7\">\n"
#else
"<PhoneNumbers>\n"
#endif
"<Number type=\"HomeMobile\">1</Number>\n"
"</PhoneNumbers>\n"
"<EmailAddresses>\n"
"<Email>1@home.net</Email>\n"
"</EmailAddresses>\n"
"</Contact>\n";
    QString serverId;
    foreach ( QString id, idMap.keys() ) {
        if ( idMap[id] == "Client Identifier 4" ) {
            serverId = id;
            break;
        }
    }
    QVERIFY(!serverId.isEmpty());
    newRecord.replace("Client Identifier 4", serverId.toLocal8Bit());
    QVERIFY(editClientRecord( newRecord ));
}

void checkForEditedItem4()
{
    QByteArray expected =
"<Contact>\n"
"<Identifier>Client Identifier 4</Identifier>\n"
"<NameTitle>title</NameTitle>\n"
"<FirstName pronunciation=\"fpron\">fname</FirstName>\n"
"<MiddleName>mid</MiddleName>\n"
"<LastName pronunciation=\"lpron\">lname</LastName>\n"
"<Suffix>suf</Suffix>\n"
"<Company pronunciation=\"comppron\"></Company>\n"
"<BusinessWebpage>http://foo.com/</BusinessWebpage>\n"
"<JobTitle>jobtitle</JobTitle>\n"
"<Department>dept</Department>\n"
"<Office>office</Office>\n"
"<Profession>prof</Profession>\n"
"<Assistant>assistant</Assistant>\n"
"<Manager>manager</Manager>\n"
#ifdef Q_WS_QWS // Qtopia supports HomeWebpage
"<HomeWebpage></HomeWebpage>\n"
#endif
"<Spouse>spouse</Spouse>\n"
"<Nickname>nick</Nickname>\n"
"<Children>kids</Children>\n"
"<Birthday>2001-02-02</Birthday>\n"
"<Anniversary>2002-03-04</Anniversary>\n"
#ifdef Q_WS_QWS // Qtopia supports Portrait
"<Portrait></Portrait>\n"
#endif
"<Notes>notes</Notes>\n"
#ifdef Q_WS_QWS // Qtopia returns an empty string (means the same as UnspecifiedGender)
"<Gender></Gender>\n"
#else // Outlook returns UnspecifiedGender
"<Gender>UnspecifiedGender</Gender>\n"
#endif
#ifdef Q_WS_QWS // Qtopia supports unlimited addresses?
"<Addresses>\n"
#else // Outlook only supports 2 addresses (mapped to specific fields)
"<Addresses maxItems=\"2\">\n"
#endif
"<Address type=\"Home\">\n"
"<Street>street</Street>\n"
"<City>city</City>\n"
"<State>state</State>\n"
"<Zip>zip</Zip>\n"
"<Country>count</Country>\n"
"</Address>\n"
"<Address type=\"Business\">\n"
"<Street>bs</Street>\n"
"<City>bc</City>\n"
"<State>bst</State>\n"
"<Zip>bz</Zip>\n"
"<Country>bco</Country>\n"
"</Address>\n"
"</Addresses>\n"
#ifdef Q_WS_QWS // Qtopia supports unlimited phone numbers? Stores a default value.
"<PhoneNumbers default=\"1\">\n"
#else // Outlook only supports 7 phone numbers (mapped to specific fields). Doesn't store a default value.
"<PhoneNumbers maxItems=\"7\">\n"
#endif
#ifdef Q_WS_QWS // Qtopia returns the numbers in this order
"<Number type=\"HomeMobile\">1</Number>\n"
#else // Outlook returns the numbers in this order
"<Number type=\"HomePhone\"></Number>\n"
"<Number type=\"HomeMobile\">1</Number>\n"
"<Number type=\"HomeFax\"></Number>\n"
"<Number type=\"BusinessPhone\"></Number>\n"
"<Number type=\"BusinessMobile\"></Number>\n"
"<Number type=\"BusinessFax\"></Number>\n"
"<Number type=\"BusinessPager\"></Number>\n"
#endif
"</PhoneNumbers>\n"
#ifdef Q_WS_QWS // Qtopia supports unlimited email addresses? Stores a default value.
"<EmailAddresses default=\"1@home.net\">\n"
#else // Outlook only supports 3 email addresses (mapped to specific fields). Doesn't store a default value.
"<EmailAddresses maxItems=\"3\">\n"
#endif
"<Email>1@home.net</Email>\n"
#ifndef Q_WS_QWS
"<Email></Email>\n"
"<Email></Email>\n"
#endif
"</EmailAddresses>\n"
"<Categories>\n"
"</Categories>\n"
#ifdef Q_WS_QWS
"<CustomFields>\n"
"</CustomFields>\n"
#endif
"</Contact>\n";
#ifndef Q_WS_QWS // Outlook auto-captializes some fields
    OUTLOOK_CAPS(NameTitle);
    OUTLOOK_CAPS(FirstName);
    OUTLOOK_CAPS(MiddleName);
    OUTLOOK_CAPS(LastName);
    OUTLOOK_CAPS(Suffix);
    OUTLOOK_CAPS(Company);
    OUTLOOK_CAPS(JobTitle);
    OUTLOOK_CAPS(Assistant);
    OUTLOOK_CAPS(Manager);
    OUTLOOK_CAPS(Spouse);
    OUTLOOK_CAPS(Nickname);
    OUTLOOK_CAPS(Children);
#endif
    QVERIFY(checkForAddedItem( expected ));
}

