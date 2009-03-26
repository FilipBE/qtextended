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

include("telephony.js");
include("contacts.js");
include("accounts.js");
include("messages.js");
include("content.js");

//TESTED_COMPONENT=PIM: Contacts (18584)

testcase = {

    userAccountCreated: false,

    initTestCase: function()
    {
        // strict mode must be disabled until bug 208103 is fixed,
        // as it causes incorrect failures.
        //strict();
        addExpectedMessageBox( "Missed Call", "Do you wish to view missed calls?" );
        waitForQtopiaStart();
        setSetting( "$HOME/Settings/Trolltech/qpe.conf", "CallControl", "MissedCalls", "0" );
        var tstModem;
        if (!runsOnDevice()) {
            deletePath( "$HOME/Applications" );
            tstModem = testModem();
            verify( tstModem != null );
        }
        waitForQtopiaStart();
        deletePath( documentsPath() + "nice_guy.jpg" );
        putFile( baseDataPath() + "nice_guy.jpg", documentsPath() + "nice_guy.jpg" );
        putFile( baseDataPath() + "tiger.jpg", documentsPath() + "tiger.jpg" );
        Content.rescan();
        clearExpectedMessageBoxes();
    },

    cleanupTestCase: function()
    {
        if (!runsOnDevice()) {
            deletePath( "$HOME/Applications" );
        }
    },

    init: function()
    {
    },

    cleanup: function()
    {
        select( callHangup() );
        Contacts.cleanupCreatedContacts();
        gotoHome();
    },

/*
    \req QTOPIA-358
    \groups Acceptance
*/
    calling_a_contact_data: {
        callcontact_using_call_button:     [ Accounts.getAccount(6), "Call", false],
        // There is no call key in mouse preferred mode, so this test is invalid in that case.
        callcontact_using_call_accept_key: [ Accounts.getAccount(6), callAccept(), true]
    },

    calling_a_contact: function(contactdata, button, skipInMousePreferred)
    {
        if (mousePreferred() && skipInMousePreferred) {
            skip("Test does not make sense in mouse preferred mode");
        }
        // Preconditions: Contact information with a telephone number specified; network is registered.
        Contacts.createContact(contactdata);
        select( contactdata.name );
        waitForTitle( "Contact Details" );
        select( "Overview", tabBar() );
        select( button );
        Telephony.waitForCallState( contactdata.name, "Connected" );
    },

/*
    \req QTOPIA-358
    \groups Acceptance
*/
    calling_a_voip_contact: function()
    {
        skip( "Test undefined" );
    },

/*
    \req QTOPIA-358
    \groups
*/
    calling_a_contact_with_avatar_data: {
        callcontact_using_call_button:     [ Accounts.getAccount(6), "Call", "tiger", false],
        // There is no call key in mouse preferred mode, so this test is invalid in that case.
        callcontact_using_call_accept_key: [ Accounts.getAccount(6), callAccept(), "tiger", true]
    },

    calling_a_contact_with_avatar: function(contactdata, button, imagefile, skipInMousePreferred )
    {
        if (mousePreferred() && skipInMousePreferred) {
            skip("Test does not make sense in mouse preferred mode");
        }
        contactdata.imagename = imagefile;
        Contacts.createContact(contactdata);
        select( contactdata.name );
        waitForTitle( "Contact Details" );
        select( "Overview", tabBar() );
        select( button );
        Telephony.waitForCallState( contactdata.name, "Connected" );
        prompt( "Verify that the avatar is shown on the call item" );
    },

/*
    \req QTOPIA-358
    \groups
*/
    incoming_call_from_contact_with_avatar_data: {
        incomingCallWithAvatar: [ Accounts.getAccount(6), "tiger", false]
    },

    incoming_call_from_contact_with_avatar: function(contactdata, imagefile, skipInMousePreferred )
    {
        if (mousePreferred() && skipInMousePreferred) {
            skip("Test does not make sense in mouse preferred mode");
        }
        contactdata.imagename = imagefile;
        Contacts.createContact(contactdata);
        Telephony.incomingCall(contactdata.businessmobile);
        Telephony.waitForCallState( contactdata.name, "Incoming" );
        select( "Answer", softMenu() );
        prompt( "Verify that the avatar is shown on the call item" );
    },

/*
    \req QTOPIA-355
    \groups Acceptance
*/
    creating_a_contact_data:
    {
        "simple": [{ name: "Alan" }],
        "detailed": [{ name: "John Doe",
            email: Accounts.blackHoleEmailAddress(),
            homevoip: "johndoe@voip.example.net",
            company: "Funny Eyes",
            companypronunciation: "funny-eyes",
            title: "Worker",
            homephone: "0712345",
            homemobile: "0412345",
            homefax: "0312345",
            businessphone: "0754321",
            businessmobile: "0454321",
            businessfax: "0354321",
            pager: "555123",
            businessstreet: "Bright st",
            businesscity: "Cairns",
            businessstate: "Queensland",
            businesszip: "4051",
            businesscountry: "Australia",
            businessurl: "http://funnyeyes.example.com",
            department: "QA",
            office: "24",
            profession: "Test Engineer",
            manager: "Mr B",
            assistant: "none",
            homestreet: "Some street",
            homecity: "Some town",
            homestate: "Some state",
            homezip: "4000",
            homecountry: "Some country",
            homeurl: "Some website",
            spouse: "Jane Doe",
            children: "Jimmy Doe",
            gender: "Male",
            anniversaryChecked: true,
            anniversary: new QDate(2004, 9, 18),
            birthdayChecked:  true,
            birthday:  new QDate(2007, 3, 21)
        }]
    },

    creating_a_contact: function(contactdata)
    {
        Contacts.createContact(contactdata);
    },

/*
    \req QTOPIA-359
    \groups Acceptance
*/
    smsing_a_contact_data: {
        "standardSms": [ Accounts.getAccount(6) ]
    },

    smsing_a_contact: function(contactdata)
    {
        Contacts.createContact(contactdata);

        select( contactdata.name );
        waitForTitle( "Contact Details" );

        var multiple_phone_numbers = getList().contains("Text...");
        if (multiple_phone_numbers) {
            select( "Text..." );
            waitForTitle( "Phone Number" );
            select( contactdata.businessmobile );
        } else {
            select("Overview", tabBar());
            select( "Text" );
        }
        waitForTitle( "Create Message" );
        enter( "This is an SMS test" );

        waitForTitle( "Message details" );
        compare( getSelectedText("To"), contactdata.businessmobile );

        //FIXME: could insert a specific Finxi-based test here.
        if (mousePreferred())
            skip("Sending message via touchscreen is theme-dependent");

        expectMessageBox("Sending","Sending: Message", "OK") {
            select( "Send", softMenu() );
        }

        //TODO: Verify that the receiving side is actually receiving the SMS
    },

/*
    \req QTOPIA-357
    \groups Acceptance
*/
    editing_a_contact_data: {
        "editingAContact": [ { name: "Frank Grimes",
            homephone: "12345",
            homemobile: "67890"
            },
            { name: "Frank Grimes",
            homephone: "5432199801",
            homemobile: "0909876891",
            homestreet: "New street"
            }
]
    },

    editing_a_contact: function(contactdata, newcontactdata)
    {
        Contacts.createContact(contactdata);
        Contacts.editCurrentContact(newcontactdata);
        Contacts.verifyCurrentContact(newcontactdata);
    },

/*
    \req QTOPIA-360
    \groups
*/
    emailing_a_contact_data: {
        emailAContact: [ { name: "Email Test", email: Accounts.blackHoleEmailAddress() },
        "email contact test", "Email Test <"+Accounts.blackHoleEmailAddress()+">"
        ]
    },

    emailing_a_contact: function(contactdata, text, displayedAddress)
    {
        if( !testcase.userAccountCreated ){
            Messages.createEmailAccount( Accounts.getAccount(1) );
            testcase.userAccountCreated = true;
        }
        Contacts.createContact(contactdata);
        waitForTitle( "Contacts" );
        select(contactdata.name);
        waitForTitle( "Contact Details" );
        select( "Overview", tabBar() );
        select( "Email" );
        waitForTitle( "Create Email" );
        enter( text );

        /* FIXME: could insert a specific Finxi-based test here. */
        if (mousePreferred())
            skip("Sending message via touchscreen is theme-dependent");

        waitForTitle( "Email details" );
        compare( getText("To"), displayedAddress  );
        expectMessageBox( "Sending", "Sending: Email", "" ) {
            select( "Send", softMenu() );
        }

        //FIXME: we should be able to wait and verify that the messagebox has been closed
    },

/*
    \req QTOPIA-363
    \groups
*/
    sending_a_contacts_details_via_email_data:
    {
        sendContactViaEmail: [ { name: "jim", email: Accounts.blackHoleEmailAddress() },
            "email test", Accounts.getAccount(2)
        ]
    },

    sending_a_contacts_details_via_email: function(contactdata, text, recipient)
    {
        /* FIXME: could insert a specific Finxi-based test here. */
        if (mousePreferred())
            skip("Sending message via touchscreen is theme-dependent");

        if( !testcase.userAccountCreated ){
            Messages.createEmailAccount( Accounts.getAccount(1) );
            testcase.userAccountCreated = true;
        }
        Contacts.gotoHome();
        Contacts.createContact(contactdata);
        waitForTitle( "Contacts" );
        select( contactdata.name );

        select( "Send...", optionsMenu() );
        waitForTitle( "Send via..." );
        select( "Email VCard" );
        waitForTitle( "Email details" );

        expectMessageBox ("Sending","Sending: Email", "OK") {
            enter( recipient.address, "To", NoCommit );
            select( "Send", softMenu() );
        }

        waitForTitle( "Contact Details" );
        waitFor( 10000 ){
            return Messages.getNewMessageCount(recipient) > 0;
        }
        verify( Messages.getNewTestMessage(recipient).contains("Content-Type: text/x-vcard; name=jim.vcf") );
    },

/*
    \req QTOPIA-356
    \groups Acceptance
*/
    deleting_a_contact_data:
    {
        "deletecontact1": [ { name: "delete test" } ]
    },

    deleting_a_contact: function(contactdata)
    {
        Contacts.createContact(contactdata);
        select(contactdata.name);
        waitForTitle( "Contact Details" );
        expectMessageBox( "Contacts", "Are you sure you want to delete: "+contactdata.name, "No") {
            select("Delete contact",optionsMenu());
        }
        waitForTitle( "Contact Details" );
        select( "Back", softMenu() );
        waitForTitle( "Contacts" );
        var list = getList();
        verify( list.contains(contactdata.name) );

        select(contactdata.name)
        expectMessageBox( "Contacts", "Are you sure you want to delete: "+contactdata.name, "Yes") {
            select("Delete contact",optionsMenu());
        }
        waitForTitle( "Contacts" );
        list = getList();
        verify( !list.contains(contactdata.name) );
    },


/*
    \req QTOPIA-370
    \groups
*/
    adding_contacts_via_vcard_data:
    {
        // Data expects a VCard filename and contact name in the vcard
        "vcardcontact1": [{
            VCardName: "Pamela-Brown-vcard",
            VCardContact: "Miss Pamela Brown"
        }]
    },

    adding_contacts_via_vcard: function(testdata)
    {
        // copy a reference file into the device home dir
        putFile( baseDataPath() + "Mr-Tony-Robert-Windsor-vcard.vcf", documentsPath() + "Mr-Tony-Robert-Windsor-vcard.vcf");
        putFile( baseDataPath() + "Pamela-Brown-vcard.vcf", documentsPath() + "Pamela-Brown-vcard.vcf");
        putFile( baseDataPath() + "Paul-Johnson-vcard.vcf", documentsPath() + "Paul-Johnson-vcard.vcf");
        Content.rescan();

        gotoHome();
        select( "Documents", launcherMenu() );
        waitForTitle("Documents");
        expectMessageBox( "Contacts", "1 new Contact(s): " + testdata.VCardContact, "Cancel" ) {
            select(testdata.VCardName);
        }
        startApplication("Contacts");
        verify( !getList().contains( testdata.VCardContact ) );

        gotoHome();
        select( "Documents", launcherMenu() );
        waitForTitle("Documents");
        expectMessageBox( "Contacts", "1 new Contact(s): " + testdata.VCardContact, "OK" ) {
            select(testdata.VCardName);
        }
        waitForTitle("Documents");

        startApplication("Contacts");
        verify( getList().containsOne( testdata.VCardContact ) );

        gotoHome();
        select( "Documents", launcherMenu() );
        waitForTitle("Documents");
        expectMessageBox( "Contacts", "already in your addressbook", "OK" ) {
            select(testdata.VCardName);
        }
        waitForTitle("Documents");
        startApplication("Contacts");
        verify( getList().containsOne( testdata.VCardContact ) );
    },

/*
    \req QTOPIA-362
    \groups
*/
    sending_a_contacts_details_via_sms_data:
    {
        // Expected data: Name and number of contact, number of target contact
        "contact1": [ { name: "jim",
            homephone: "123456"
        }, Accounts.getAccount(6)
        ]
    },

    sending_a_contacts_details_via_sms: function(contactdata, target)
    {
        Contacts.createContact(contactdata);
        select(contactdata.name);

        select("Send...", optionsMenu());
        select("SMS VCard");
        waitForTitle( "Message details" );

        /* FIXME: could insert a specific Finxi-based test here. */
        if (mousePreferred())
            skip("Sending message via touchscreen is theme-dependent");
        enter( target.businessmobile, "To", NoCommit );

        expectMessageBox("Sending","Sending: Message", "") {
            select("Send",softMenu());
        }

        waitForTitle( "Contact Details" );

        // fixme: replace this with real code when we are able to communicate with two devices at the same time.
        prompt(
            "Verify that the SMS with contact details is:\n"+
            "* received on the phone with number '" + target.businessmobile + "'\n"+
            "* and can be added to the target device addressbook."
        );
    },

/*
    \req QTOPIA-364
    \groups
*/
    associating_photo_to_a_contact_data:
    {
        "cameracontact1": [ { name: "camera contact" } ]
    },

    associating_photo_to_a_contact: function(contactdata)
    {
        prompt( "Precondition: A camera is available" );

        Contacts.createContact( contactdata );

        select( contactdata.name );
        select( "Overview", tabBar() );
        select( "Edit" );
        waitForTitle( "Edit Contact" );
        select( "Media", tabBar() );
        select( "Photo" );

        waitForTitle( "Contact Portrait" );
        select("Take Photo");

        waitForTitle( "Camera" );
        wait( 5000 );
        select( "Select", softMenu() );

        waitForTitle( "Contact Portrait", 10000 );
        prompt( "Verify that the photo taken is now displayed in the 'Contact Photo' dialog" );

        select( "Back", softMenu() );
        waitForTitle( "Edit Contact" );
        select( "Back", softMenu() );
        waitForTitle( "Contact Details" );
        select( "Back", softMenu() );
        waitForTitle( "Contacts" );

        prompt( "Verify that the image selected is viewable from the contacts list and in the details view." );
    },

/*
    \req QTOPIA-364
    \groups
*/
    associating_image_to_a_contact_data:
    {
        nice_guy_test: [ { name: "Nice Guy", imagename: "nice_guy", imagefilename: "nice_guy.jpg" } ]
    },

    associating_image_to_a_contact: function(contactdata)
    {
        Contacts.createContact( contactdata );
        select( contactdata.name );

        waitForTitle( "Contact Details" );
        select( "Overview", tabBar() );
        select("Edit",softMenu());
        select( "Media", tabBar() );
        verifyImage( "associating_photograph_file", "", "Verify that the image of "+contactdata.name+" is displayed in the 'Contact Photo' dialog." );
    },

/*
    \req QTOPIA-364
    \groups
*/
    removing_an_image_from_a_contact_data:
    {
        no_restart_test: [ { name: "Nice Guy", imagename: "nice_guy", imagefilename: "nice_guy.jpg" }, false],
        restart_test: [ { name: "Nice Guy", imagename: "nice_guy", imagefilename: "nice_guy.jpg" }, true]
    },

    removing_an_image_from_a_contact: function(contactdata, restartContacts)
    {
        Contacts.createContact( contactdata );
        if (restartContacts) {
            // let's see if there is a difference when we close and restart contacts
            gotoHome();
            startApplication( "Contacts" );
        }
        select( contactdata.name );
        waitForTitle( "Contact Details" );
        select( "Overview", tabBar() );
        select( "Edit", softMenu() );
        // now try to remove the image
        waitForTitle("Edit Contact");
        select( "Media", tabBar() );
        select( "Select", softMenu() );
        waitForTitle("Contact Portrait");
        select("Remove");
        select( "Back", softMenu() );
        waitForTitle( "Edit Contact" );
        verifyImage( "removed_image", "", "Verify that there is NO image associated to the contact." );
    },

/*
    \req QTOPIA-354
    \groups
*/
    filtering_contacts_by_name_data:
    {
        differentNames: [
            [{name: "xavier zephyr"},
                {name: "grant howard"},
                {name: "ian bronson"}]
        ]
    },
    filtering_contacts_by_name: function(nameslist)
    {
        if (mousePreferred())
            skip("Test only works on keypad device");

        for( var i = 0; i < nameslist.length; ++i ){
            Contacts.createContact(nameslist[i]);
            waitForTitle( "Contacts" );
            verify( getList().contains(nameslist[i].name) );
        }

        for( var i = 0; i < nameslist.length; ++i ){
            var inName = nameslist[i].name;
            // Create a temporary array
            var outNameList = [];
            for( var n = 0; n < nameslist.length; ++n ){
                if( nameslist[n].name != inName ){ outNameList.push(nameslist[n].name); }
            }

            enter( inName.left(3) );
            wait( 1000 );
            var contactslist = getList();
            verify( contactslist.contains(inName) );

            for( var j = 0; j < outNameList.length; ++j ){
                verify( !contactslist.contains(outNameList[j]) );
            }
            select( "Delete", softMenu() );select( "Delete", softMenu() );select( "Delete", softMenu() );
            wait( 1000 );
        }
    },

/*
    \req QTOPIA-353
    \groups
*/
    creating_a_contact_using_extended_characters_data:
    {
        "extendedcontact1": [{ name: "¥£" }]
    },

    creating_a_contact_using_extended_characters: function(contactdata)
    {
        //expectFail("BUG-213147");
        Contacts.createContact(contactdata);
        waitForTitle( "Contacts" );
        verify( getList().contains(contactdata.name) );
        select( contactdata.name );
        waitForTitle( "Contact Details" );
        verifyImage( "extended_name_details", "", "Verify that the name '" + contactdata.name + "' is displayed correctly." );
    },

/*
    \req QTOPIA-354
    \groups
*/
    viewing_contacts_by_category_data:
    {
        "categorycontact1": [ { name: "category test",
            homephone: "12345"
        }, "Business"
        ]
    },
    viewing_contacts_by_category: function(contactdata, category)
    {
        Contacts.createContact(contactdata);
        Contacts.gotoHome();
        select( contactdata.name );
        waitForTitle( "Contact Details" );
        select( "Overview", tabBar() );
        select("Edit");
        waitForTitle( "Edit Contact" );
        select( "Media", tabBar()  );
        select("Groups");
        waitForTitle( "Groups" );
        select(category);
        select( "Back", softMenu() );

        waitForTitle( "Edit Contact" );

//        expectFail( "BUG-198264" );
        select("Groups");
        select( "Back", softMenu() );

        compare( getSelectedText(), category );
        select( "Back", softMenu() );
        waitForTitle( "Contact Details" );

        select( "Back", softMenu() );
        waitForTitle( "Contacts" );
        select("Show Group...", optionsMenu());
        waitForTitle( "Contact Groups" );
        select("Business");
        waitForTitle( "Contacts - Business" );
        verify( getList().contains(contactdata.name) );

        select("Show Group...", optionsMenu());
        waitForTitle( "Contact Groups" );
        select("Personal");
        waitForTitle( "Contacts - Personal" );
        verify( !getList().contains(contactdata.name) );
    },

/*
    \req QTOPIA-355
    \groups
*/
    adding_an_empty_contact: function()
    {
        startApplication( "Contacts" );
        waitFor( 2000 ){
            return !getLabels().contains("Loading SIM...");
        }

        wait(1000);
        var pre_list = getList();
        select("New contact",optionsMenu());
        waitForTitle( "New Contact" );
        select( "Back", softMenu() );
        waitForTitle( "Contacts" );
        compare( getList(), pre_list );
    },

/*
    \req QTOPIA-353
    \groups Acceptance
*/
    associating_a_contact_to_a_new_category_data:
    {
        "category1": [{ name: "category contact",
            homephone: "54321"
        }, "temp"
        ]
    },

    associating_a_contact_to_a_new_category: function(contactdata, category)
    {
        Contacts.createContact(contactdata);
        select(contactdata.name);
        select( "Overview", tabBar() );
        select("Edit");
        waitForTitle( "Edit Contact" );
        select( "Media", tabBar()  );
        select( "Groups" );
        waitForTitle( "Groups" );
        select( "New group", optionsMenu());
        waitForTitle( "Group Name:" );
        enter( category );
        select( "Back", softMenu() );
        waitForTitle( "Groups" );
        verify( getList().contains(category) );
        select( category );
        select( "Back", softMenu() );
        waitForTitle( "Edit Contact" );

        if (mousePreferred())
            expectFail("Groups list (item view with check boxes) is not correctly handled");

        compare( getSelectedText(), category );
        select( "Back", softMenu() );

        Contacts.gotoHome();
        select("Show Group...",optionsMenu());
        select(category);
        compare( getList(), [ contactdata.name ] );
    },

/*
    \req QTOPIA-353
    \groups
*/
    trying_to_duplicate_an_existing_category_data:
    {
        "categoryDuplication": [{ name: "category contact",
            homephone: "54321"
        }, "temp"
        ]
    },

    trying_to_duplicate_an_existing_category: function(contactdata, category)
    {
        Contacts.createContact(contactdata);
        select(contactdata.name);
        select( "Overview", tabBar() );
        select("Edit");
        waitForTitle( "Edit Contact" );
        select( "Media", tabBar()  );
        select( "Groups" );
        waitForTitle( "Groups" );
        var list = getList();
        while( list.length > 2) {
            select( list[2] );
            select( "Delete", optionsMenu() );
            list = getList();
        }

        select( "New group", optionsMenu());
        waitForTitle( "Group Name:" );
        enter( category );
        select( "Back", softMenu() );
        waitForTitle( "Groups" );
        verify( getList().containsOne(category) );

        // try to add the same group again
        select( "New group", optionsMenu());
        waitForTitle( "Group Name:" );
        enter( category );
        select( "Back", softMenu() );
        waitForTitle( "Groups" );

        verify( getList().containsOne(category) );
    },

/*
    \req QTOPIA-353
    \groups
*/
    setting_multiple_email_addresses_data:
    {
        fourEmailAccounts: [
            "jim",
            [ "blackhole@email123.example.com",
            "notworky1@@email1234.example.com",
            "notworky2@@email1235.example.com",
            "notworky3@@email1235.example.com" ]
        ]
    },

    setting_multiple_email_addresses: function(name, emailList)
    {
        Contacts.gotoHome();
        select("New contact", optionsMenu());
        waitForTitle( "New Contact" );
        enter(name, "Name");
        enter("", "Emails");
        select("Details", optionsMenu());

        waitForTitle( "Email List" );

        for( var i = 0; i < emailList.length; ++i ){
            select("New",optionsMenu());
            enter(emailList[i], "qpe/addressbook:Email Address:");
        }

        select( "Back", softMenu() );

        var list = getText("Emails").split(",");
        for( var i = 0; i < emailList.length; ++i ){
            verify(list.contains(emailList[i]));
        }
    },

/*
    \req QTOPIA-353
    \groups
*/
    detailed_contact_names_data:
    {
        "drNickRivieraSr": [{
            Title: "Dr",
            Firstname: "nick",
            FNPron: "nik",
            Middlename: "james",
            Lastname: "riviera",
            LNPron: "riv-ee-eyr-uh",
            Suffix: "Sr",
            Nickname: "dr nick"
        }]
    },

    detailed_contact_names: function(testdata)
    {
        var expectedName = testdata.Title;
        expectedName += " " + testdata.Firstname + " \"" + testdata.Nickname + "\" " + testdata.Middlename + " ";
        expectedName += testdata.Lastname + " " + testdata.Suffix;

        startApplication( "Contacts" );
        select("New contact",optionsMenu());
        waitForTitle( "New Contact" );

//BUG_START bug #206088
        select( "Details", optionsMenu() );
        waitForTitle( "Edit Name" );
        select( testdata.Title, signature("First Name", -1) ); // Title field - is this broken?
        enter( testdata.Firstname, "First Name" );
        enter( testdata.FNPron, signature("First Name",+3) );
//        enter( testdata.FNPron, "Pronunciation" );
        enter( testdata.Middlename, "Middle Name" );
        enter( testdata.Lastname, "Last Name" );
//        enter( testdata.LNPron, "Pronunciation" );
        enter( testdata.LNPron, signature("Last Name",+1) );
//        select( testdata.Suffix, signature("Suffix", -1) );
        select( testdata.Suffix, signature("Nickname", -1) );  // Suffix field - is this broken?
        enter( testdata.Nickname, "Nickname" );
        select( "Back", softMenu() );
        waitForTitle( "New Contact" );
//BUG_END bug #206088

        compare( getSelectedText("Name"), expectedName );
        select( "Back", softMenu() );
        select(testdata.Title + " " + testdata.Firstname + " " + testdata.Lastname);
        waitForTitle( "Contact Details" );
    },

/*
    \req QTOPIA-361
    \groups
*/
    beaming_and_receiving_a_single_contact: function()
    {
        prompt(
            "* Ensure you have a receiving Qtopia device.\n" +
            "* Ensure a contact exists.\n" +
            "* Select the 'Beaming' application and select the 'Receiver on' option on the receiving device.\n" +
            "* Select a contact from the contacts list.\n" +
            "* While viewing the contacts details select 'Send...' from the context menu.\n" +
            "* Verify that a 'Send via...' dialog is displayed.\n" +
            "* Select 'Infrared' from the list displayed.\n" +
            "* Wait for the beamed entry to be received.\n" +
            "* Verify that a confirmation dialog is be displayed listing three options. 'Open', 'Store', and 'Cancel'.\n" +
            "* Select 'Store'.\n" +
            "* Verify that the sent contact is stored and displayed in the contacts list.\n"
        );
    },

/*
    \req QTOPIA-370
    \groups
*/
    setting_a_business_card_data:
    {
        "setABusinessCard": [{ name: "card contact" }]
    },

    setting_a_business_card: function(contactdata)
    {
        Contacts.createContact(contactdata);
        select(contactdata.name);
        waitForTitle( "Contact Details" );
        expectMessageBox( "Contacts", "Set \"" + contactdata.name + "\" as your Business Card?", "No") {
            select("Set as My Card",optionsMenu());
        }
        waitForTitle( "Contact Details" );
        select( "Back", softMenu() );
        waitForTitle( "Contacts" );
        enter( contactdata.name.left(3) );
        wait( 1000 ); // Wait for filter animation to stop
        verifyImage( "card_not_set", "", "Verify that the card icon is not shown next to the contact." );
        select(contactdata.name);
        waitForTitle( "Contact Details" );
        expectMessageBox( "Contacts", "Set \"" + contactdata.name + "\" as your Business Card?", "Yes") {
            select("Set as My Card",optionsMenu());
        }
        waitForTitle( "Contact Details" );
        select( "Back", softMenu() );
        waitForTitle( "Contacts" );
        verifyImage( "icon_visible", "", "Verify that an icon to indicate a business card is now displayed beside the contact when viewing the contacts list." );
        select("Show My Card",optionsMenu());
        verifyImage( "card_contact_shown", "", "Verify that the contact set as the card is shown." );
    },

/*
    \req QTOPIA-370
    \groups
*/
    removing_a_business_card_data:
    {
        "removeCard": [{ name: "uncard contact" }]
    },

    removing_a_business_card: function(contactdata)
    {
        Contacts.createContact(contactdata);
        select(contactdata.name);
        waitForTitle( "Contact Details" );
        expectMessageBox( "Contacts", "Set \"" + contactdata.name + "\" as your Business Card?", "Yes") {
            select("Set as My Card",optionsMenu());
        }
        waitForTitle( "Contact Details" );
        select( "Back", softMenu() );
        waitForTitle( "Contacts" );
        enter( contactdata.name.left(3) );
        wait( 1000 );  // Wait for filter animation to stop
        verifyImage( "icon_visible", "", "Verify that an icon to indicate a business card is now displayed beside the contact when viewing the contacts list." );
        select("Show My Card",optionsMenu());
        verifyImage( "card_contact_shown", "", "Verify that the contact set as the card is shown." );
        select("Remove as My Card",optionsMenu());
        select( "Back", softMenu() );
        waitForTitle( "Contacts" );
        verifyImage( "card_not_set", "", "Verify that the contact is unset as the business card." );
    },

/*
    \req QTOPIA-361
    \groups
*/
    beaming_and_receiving_multiple_contacts: function()
    {
        prompt(
            "Preconditions\n 1. Another (receiving) Qtopia device with IR enabled is present.\n" +
            "* Multiple contacts exists in the contacts list.\n" +
            "* Select 'Send All...' from the context menu.\n" +
            "* Verify that the 'Send via...' dialog is displayed.\n" +
            "* Select 'Infrared' from the list.\n" +
            "* Verify that the dialog is closed and the task bar updates displaying that status of the beam.\n" +
            "* Wait for the beamed entries on the receiving device.\n" +
            "* Verify that after the data transmission a confirmatory dialog is displayed listing three options, 'Open', 'Store', and 'Cancel'.\n" +
            "* Select 'Cancel' from the dialog.\n" +
            "* Verify that the contact data is not saved or added to the contacts list.\n" +
            "* Beam the contacts again.\n" +
            "* On the receiving device select 'Store' from the presented dialog.\n" +
            "* Verify that the contacts list is stored on the device (located in the 'Documents' launcher).\n" +
            "* Finally beam the contacts again.\n" +
            "* On the receiving device select 'Open' from the presented dialog.\n" +
            "* Verify that the new contacts have been added to the Contacts list.\n"
        );
    },

/*
    \req QTOPIA-361
    \groups
*/
    beaming_and_receiving_contacts_from_other_non_qtopia_devices: function()
    {
        prompt(
            "* Turn the receiver on via the 'Beaming' application.\n" +
            "* Select a contact on the other device and send it to the Qtopia device.\n" +
            "* Wait for the beamed entry.\n" +
            "* After the data transmission verify that a confirmation is displayed on the Qtopia device showing options to 'Open', 'Store', or 'Cancel' the received contact.\n" +
            "* Select 'Open' and verify that the contact is added to the contact list.\n" +
            "* Verify that the information sent is identical to the information received.\n" +
            "* Select a contact from the contacts list.\n" +
            "* Select the 'Send...' option from the context menu.\n" +
            "* Select 'Infrared' from the list presented in the 'Send via...' dialog.\n" +
            "* Verify that the contact is sent.\n" +
            "* Verify that the receiving device receives the contact and the contact information is correct.\n"
        );
    },

/*
    \req QTOPIA-365
    \groups
*/
    creating_and_using_a_data_link_data:
    {
        "qdlink1": [{ name: "test" }, "link contact"]
    },

    creating_and_using_a_data_link: function(contactdata,linkname)
    {
        // Preconditions: Two contacts exist
        Contacts.createContact(contactdata);
        Contacts.createContact({ name: linkname });

        Contacts.gotoHome();
        select(contactdata.name);
        waitForTitle( "Contact Details" );
        select("Overview", tabBar());
        select("Edit");
        waitForTitle( "Edit Contact" );

        // 3. Engage the 'Notes' text-area then select 'Insert Link' from the context menu.
        select( "Media", tabBar() );
        enter( "text", "Notes");
        select("Insert Link",optionsMenu());

        // 4. Verify a dialog titled 'Select Source' is displayed listing the 'Calendar', 'Contacts' and 'Tasks' applications.
        waitForTitle( "Select Source" );

        // 5. Select 'Contacts' from the list and verify that a list of current contacts is listed.
        select("Contacts");
        waitForTitle( "Select Contact" );

        // 6. Select a contact from the list.
        select(linkname);

        // 7. Verify that the 'Edit Contact' dialog is returned with the selected contact entered in the 'Notes', followed by a superscript number.
        waitForTitle( "Edit Contact" );
        var text = getText();
        verify( text.contains(linkname), "Text: " + text);

        // 8. Select 'Done' to create the contact.
        select( "Back", softMenu() );
        select( "Back", softMenu() );
        waitForTitle( "Contacts" );

        // 9. With the contact now created, select it to view the contacts details.
        select(contactdata.name);
        waitForTitle( "Contact Details" );
        select("Details", tabBar());

        // 10. Verify that the QDL hyperlink is displayed.
        text = getText();
        verify( text.contains(linkname), "Text: " + text);

        // 11. Select the link.
        // Test framework doesn't support this yet.
        prompt("Select the embedded '" + linkname + "' link in the contact details.");

        // 12. Verify the details for the selected contact are displayed.
        verifyImage( "link_contact_shown", "", "Verify the details for the '" + linkname + "' contact are displayed." );
        // TODO: Verify text browser
    },

/*
    \req QTOPIA-365
    \groups
*/
    updating_qdl_links_for_removed_contacts_data:
    {
        "removeQdl1": [{
            name: "primary contact",
            linkname: "link contact"
        }]
    },

    updating_qdl_links_for_removed_contacts: function(testdata)
    {
        startApplication( "Contacts" );
        select("New contact",optionsMenu());
        waitForTitle( "New Contact" );
        enter( testdata.linkname);
        select( "Back", softMenu() );
        waitForTitle( "Contacts" );

        select("New contact",optionsMenu());
        waitForTitle( "New Contact" );
        enter( testdata.name);
        select( "Media", tabBar() );

        enter( "", "Notes" );
        select("Insert Link",optionsMenu());
        waitForTitle( "Select Source" );

        select("Contacts");
        waitForTitle( "Select Contact" );
        select("link contact");
        waitForTitle( "New Contact" );
        select( "Back", softMenu() );
        waitForTitle( "Contacts" );
        select(testdata.name);
        waitForTitle( "Contact Details" );
        select( "Details", tabBar() );
        keyClick( Qt.Key_Down );
        select( "Select", softMenu() );
        //select( testdata.linkname );
        verifyImage( "qdl_link_works", "", "Verify that the QDL link worked correctly - 'link contact' should be displayed." );

        select( "Back", softMenu() );
        select( "Back", softMenu() );
        waitForTitle( "Contacts" );
        select( testdata.linkname );
        waitForTitle( "Contact Details" );
        expectMessageBox( "Contacts", "Are you sure you want to delete", "Yes" ) {
            select("Delete contact",optionsMenu());
        }
        waitForTitle( "Contacts" );
        select( testdata.name );
        waitForTitle( "Contact Details" );
        select( "Details", tabBar() );
        //select( testdata.linkname );
        verifyImage( "qdl_link_broken", "", "Verify that the QDL link is displayed as a 'broken link'" );
        keyClick( Qt.Key_Down );
        select( "Select", softMenu() );
        verifyImage( "qdl_link_disabled", "", "Verify that the QDL link does not activate when selected." );
    },

/*
    \req QTOPIA-7915
    \groups
*/
    presence_status_of_a_contact_data:
    {
        gtalkcontactpresence:[ Accounts.getAccount(3) ]
    },

    presence_status_of_a_contact: function(gtalksettings)
    {
        if( !isBuildOption("QTOPIA_HOMEUI") )
            skip( "Home test" );

        prompt(
            "* Preconditions: two separate Qtopia devices are available, two GMail accounts are available\n"+
            "* Login on a secondary machine using the secondary account\n"+
            "* Set the status to Available\n"+
            "* Login to the Qtopia device with the primary account\n"+
            "* Start the Contacts application\n"+
            "* Create a new contact with the Buddy field set to the address of the secondary login\n"+
            "* Verify the presence indicator of the contact is shown as 'online'"
        );
    },

/*
    \req QTOPIA-7915
    \groups
*/
    presence_status_of_contact_is_updated_data:
    {
        gtalkcontactpresenceupdated:[ Accounts.getAccount(3) ]
    },

    presence_status_of_contact_is_updated: function(gtalksettings)
    {
        prompt(
            "* Preconditions: two separate Qtopia devices are available, two GMail accounts are available\n"+
            "* Login on a secondary machine using the secondary account\n"+
            "* Set the status to Available\n"+
            "* Login to the Qtopia device with the primary account\n"+
            "* Start the Contacts application\n"+
            "* Create a new contact with the Buddy field set to the address of the secondary login\n"+
            "* Verify the contact list is shown and the presence indicator of the contact is shown as 'online'\n"+
            "* Change the status of the secondary device to Away\n"+
            "* Verify the presence indicator of the contact is shown as 'away'\n"+
            "* Select the contact\n"+
            "* Verify the contact details view is shown and the presence indicator of the contact is shown as 'away'\n"+
            "* Change the status of the secondary device to Online\n"+
            "* Verify the presence indicator of the contact is shown as 'Online'"
        );
    },

/*
    \req
*/
    perf_startup: function()
    {
        Performance.testStartupTime("Applications/Contacts");
    },

/*
    Bug section
*/

/*
    \req BUG-173657
*/
    deleteAvailableAfterContactCreated_data: {
        "bug173657": [{ name: "contact", homephone: "1234567890" }]
    },

    deleteAvailableAfterContactCreated: function(contactdata)
    {
        Contacts.createContact(contactdata);
        gotoHome();
        startApplication( "Contacts" );
        var nm = contactdata.name;
        select( nm );
        expectMessageBox( "Contacts", "Are you sure", "Yes" ) {
            select( "Delete contact", optionsMenu() );
        }
    }
} // end of testcases
