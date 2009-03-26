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

include( "userinterface.js" );

Contacts = {
    created_contacts: [],

    gotoHome: function()
    {
        var count = 10;
        addExpectedMessageBox( "Empty account name", "Do you want to continue and discard any changes?", "Yes" );
        while (currentTitle() != "Contacts" && getList(softMenu()).contains("Back") && count-- > 0) {
            select( "Back", softMenu() );
        }
        clearExpectedMessageBoxes();
        if (currentTitle() != "Contacts") {
            gotoHome();
            startApplication("Contacts");
        }
    },

    cleanupGroups: function()
    {
        // cleanup groups from a previous testcase
        select( "Show Groups...",optionsMenu() );
        waitForTitle( "Contact Groups" );
        var firstGroup = getSelectedText();
        keyClick( Qt.Key_Down );
        while( getSelectedText() != firstGroup ){
            if( getSelectedText() == "Personal" || getSelectedText() == "Business" ){
                keyClick( Qt.Key_Down );
            }else{
                select( "Delete", optionsMenu() );
                keyClick( Qt.Key_Down );
                keyClick( Qt.Key_Up );
            }
        }
    },

    cleanupCreatedContacts: function()
    {
        Contacts.gotoHome();
        // cleanup contacts from a previous testcase
        for(i = 0; i < Contacts.created_contacts.length; i++){
            var list = getList();
            var listlen = list.length;
            while (list.contains(Contacts.created_contacts[i])) {
                select( Contacts.created_contacts[i] );
                if (getList(optionsMenu()).contains("Delete contact")) {
                    expectMessageBox( "Contacts", "Are you sure ", "Yes" ) {
                        select( "Delete contact", optionsMenu() );
                    }
                }
                list = getList();
                if (list.length == listlen && list.contains(Contacts.created_contacts[i])) {
                    expectFail("Bug 230628: Deleting a contact sometimes silently fails");
                    fail("Contact " + Contacts.created_contacts[i] + " wasn't removed from "
                        +"contacts list after being deleted!");
                }
                listlen = list.length;
            }
        }
        Contacts.created_contacts = [];
    },

    createContact: function(o)
    {
        if (currentTitle() != "Contacts") {
            startApplication( "Contacts" );
        }
        select( "New contact",optionsMenu() ) ;
        waitForTitle( "New Contact" );
        Contacts.created_contacts.push(o.name);
        Contacts.editCurrentContact(o);

        waitForTitle("Contacts");
        verify( getList().contains(o.name) );
        select(o.name);

        Contacts.verifyCurrentContact(o);
        select( "Back", softMenu() );
        waitForTitle( "Contacts" );
    },

    verifyCurrentContact: function(o)
    {
        waitForTitle( "Contact Details" );
        select( "Details", tabBar()  );

        text = getText();
        for( att in o ){
            if( att == "gender") {
                /* Bug 186890 */
            } else if ((att == "birthday" && !o.birthdayChecked) ||
                    (att == "anniversary" && !o.anniversaryChecked)) {
                // skip test
            } else if ( att != "anniversaryChecked" &&
                    att != "birthdayChecked" &&
                    att != "imagename" &&
                    att != "imagefilename"  &&
                    att != "group") {
                verify( text.contains( o[att] ), "Contact details do not contain '" + o[att] + "' information for attribute '" + att + "'\nContact details:\n" + text );
            }
        }
    },

    editCurrentContact: function(o)
    {
        if (currentTitle() == "Contacts" ) {
            select(o.name);
        }

        if (currentTitle() == "Contact Details" ) {
            select( "Overview", tabBar() );
            select( "Edit" ); // selects a button
            waitForTitle( "Edit Contact" );
        } else {
            waitForTitle( "New Contact" );
        }

        select( "Contact", tabBar() );

        var oldname = getText("Name");
        enter( o.name, "Name" ) ;
        for (var i = 0; i < Contacts.created_contacts.length; i++){
            if (Contacts.created_contacts[i] == oldname)
                Contacts.created_contacts[i] = o.name;
        }
        enter( o.email, "Emails" );

        if (o.company != undefined || o.title != undefined) {
            setChecked( true, "Business contact" );
            enter( o.company, "Company" ) ;
            enter( o.title, "Title" ) ;
        } else {
            setChecked( false, "Business contact" );
        }

/*
        Contacts.enterPhoneNumber( o.homephone, "Home Phone" );
        Contacts.enterPhoneNumber( o.homemobile, "Home Mobile" );
        Contacts.enterPhoneNumber( o.homefax, "Home Fax" );
        Contacts.enterPhoneNumber( o.businessphone, "Business Phone" );
        Contacts.enterPhoneNumber( o.businessmobile, "Business Mobile" );
        Contacts.enterPhoneNumber( o.businessfax, "Business Fax" );
        Contacts.enterPhoneNumber( o.pager, "Business Pager" );
*/
        if ( anyDefined(
                    o.businessphone,
                    o.businessmobile,
                    o.businessfax,
                    o.businessvoip,
                    o.pager,
                    o.companypronunciation,
                    o.businessstreet,
                    o.businesscity,
                    o.businessstate,
                    o.businesszip,
                    o.businesscountry,
                    o.businessurl,
                    o.department,
                    o.office,
                    o.profession,
                    o.manager,
                    o.assistant) )
            select( "Business", tabBar() );

        enter( o.businessphone, "Phone" );
        enter( o.businessmobile, "Mobile" );
        enter( o.businessfax, "Fax" );
        enter( o.pager, "Pager" );
        if( o.businessvoip && isBuildOption("QTOPIA_VOIP") ){
            enter( o.businessvoip, "VOIP" ) ;
        }
        enter( o.companypronunciation, "Pronunciation" ) ;
        enter( o.businessstreet, "Address" ) ;
        enter( o.businesscity, "City" ) ;
        enter( o.businessstate, "State" ) ;
        enter( o.businesszip, "Zip" ) ;
        enter( o.businesscountry, "Country" ) ;
        enter( o.businessurl, "URL" ) ;
        enter( o.department, "Department" ) ;
        enter( o.office, "Office" ) ;
        enter( o.profession, "Profession") ;
        enter( o.manager, "Manager") ;
        enter( o.assistant, "Assistant" ) ;

        if ( anyDefined(
                    o.homephone,
                    o.homemobile,
                    o.homefax,
                    o.homevoip,
                    o.gtalk,
                    o.homestreet,
                    o.homecity,
                    o.homestate,
                    o.homezip,
                    o.homecountry,
                    o.homeurl,
                    o.spouse,
                    o.children ) )
            select( "Personal", tabBar()  );

        enter( o.homephone, "Phone" );
        enter( o.homemobile, "Mobile" );
        enter( o.homefax, "Fax" );
        if( o.homevoip && isBuildOption("QTOPIA_VOIP") ){
            enter( o.homevoip, "VOIP" ) ;
        }
        if( o.gtalk ){
            enter( o.gtalk, "Google Talk" ) ;
        }
        enter( o.homestreet, "Street" ) ;
        enter( o.homecity, "City" ) ;
        enter( o.homestate, "State" ) ;
        enter( o.homezip, "Zip" ) ;
        enter( o.homecountry, "Country" ) ;
        enter( o.homeurl, "URL" ) ;
        enter( o.spouse, "Spouse" ) ;
        enter( o.children, "Children" ) ;
        select( o.gender, "Gender" );

        if ( o.anniversaryChecked != undefined ){
            if( o.anniversaryChecked ){
                verify( o.anniversary != undefined );
                setChecked( true, "Anniversary" );
                enter( o.anniversary, signature("Anniversary",1) );
                compare( getSelectedText(signature("Anniversary",1)), o.anniversary.toString("dd/MM/yyyy") );
            }else{
                setChecked( false, "Anniversary" );
            }
        }
        if ( o.birthdayChecked != undefined ){
            if( o.birthdayChecked ){
                verify( o.birthday != undefined );
                setChecked( true, "Birthday" );
                enter( o.birthday, signature("Birthday",1) );
                compare( getSelectedText(signature("Birthday",1)), o.birthday.toString("dd/MM/yyyy") );
            }else{
                setChecked( false, "Birthday" );
            }
        }

        if (o.imagename != undefined) {
            select( "Media", tabBar() );
            select( "Photo" );
            waitForTitle( "Contact Portrait" );
            select("Pictures");
            waitForTitle( "Select Image" );

            // see bug #197903 for test "removing_an_image_from_a_contact"
            select(o.imagename);
            select( "Back", softMenu() );
        }

        if(o.group != undefined) {
            select( "Media", tabBar() );
            select( "Groups" );
            waitForTitle( "Groups" );
            select( o.group );
            select( "Select", softMenu() );
            select( "Back", softMenu() );
        }

        select( "Back", softMenu() );
    },

    enterPhoneNumber: function(number, type)
    {
        if (number == undefined) return;

        var list = getLabels();
        if (!list.contains(type)) {
            // there should always be at least 1 phoneline field
            verify(list.length > 0);
            // enter the phone number in the last entry in the list (which is presumably an empty line)
            enter( number, list[list.length-1] );
            select( "Type", optionsMenu() );
            select( type );
        } else {
            enter( number, type );
        }
    }

};

