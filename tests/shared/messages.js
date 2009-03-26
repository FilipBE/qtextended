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

include("mail.js");
include( "userinterface.js" );
/*
    A message object is defined as having the parameters:
        type                    - message type currently: sms, mms, im or email
        to                      - numbers or email addresses (contact handling not implemented)
        cc                      - carbon copy recipients
        bcc                     - blind carbon copy recipients
        text                    - message body. In the case of an mms, this is an object containing slide information
        subject                 - text to be placed in the subject field (where appropriate)
        attachments             - email attachment/mms slide list
*/

/*
    An attachment is a three part object:
        displayname    - the user visible name
        filename       - filename
        contenttype    - object type, eg image/png

    A 'slide' is a three part MMS object:
        media   - image or video
        text    - body text
        audio   - audio file
*/

Messages = {
/*
    Clear all missed message counts
*/
    clearMissedMessages: function()
    {
        setSetting("$HOME/Settings/Trolltech/qpe.conf", "Messages", "MissedEmailMessages", "0" );
        setSetting("$HOME/Settings/Trolltech/qpe.conf", "Messages", "MissedInstantMessages", "0" );
        setSetting("$HOME/Settings/Trolltech/qpe.conf", "Messages", "MissedMMSMessages", "0" );
        setSetting("$HOME/Settings/Trolltech/qpe.conf", "Messages", "MissedSMSMessages", "0" );
        setSetting("$HOME/Settings/Trolltech/qpe.conf", "Messages", "MissedSystemMessages", "0" );
    },

/*
    Create a Messages email account based on supplied data
*/
    createEmailAccount: function(o)
    {
        Messages.gotoMessagesHomeScreen();
        var imapPullTitle = "";

        select("Account settings...",optionsMenu());
        waitForTitle( "Account settings", 10000 );
        if( getList().contains(o.accountname) ){
            imapPullTitle = "Account Modified";
            select( o.accountname );
            waitForTitle( "Edit Account", 10000 );
        }else{
            imapPullTitle = "Account Added";
            select("Add account...",optionsMenu());
            waitForTitle( "Create new account", 10000 );
        }

        enter( o.accountname, "Name");
        // Incoming Email
        if( anyDefined(o.mailtype, o.username, o.password, o.inserver, o.inport, o.inencryption, o.deletemail,
                        o.skiplarger, o.skipsize, o.interval, o.intervaltime, o.roaminginterval) ){
            select( o.mailtype, "Type" );
            enter( o.username, "Username");
            enter( o.password, "Password");
            enter( o.inserver, "Server");
            enter( o.inport, "Port" );
            select( o.inencryption, "Encryption" );
            if( anyDefined(o.deletemail) ){
                setChecked( o.deletemail, "Delete mail" );
            }
            if( anyDefined(o.skiplarger) ){
                setChecked( o.skiplarger, "Skip larger" );
            }
            if( o.skiplarger && o.skipsize ){
                enter( o.skipsize, signature("Skip larger", 1) );
            }
            setChecked( o.interval, "Interval" );
            if( o.interval && o.interval ){
                enter( o.intervaltime, signature("Interval", 1) );
            }
            if( o.interval ){
                setChecked( roaminginterval, "Roaming interval checking" );
            }
            if( o.push ){
                setChecked( o.push, "Push Enabled" );
            }
        }
        // Outgoing Email
        if( anyDefined(o.alias, o.address, o.outserver, o.outport, o.authentication,
                        o.authusername, o.authpassword) ){
            select( "Outgoing", tabBar() );
            enter( o.alias, "From");
            enter( o.address, "Email");
            enter( o.outserver, "Server");
            enter( o.outport, "Port" );
            select( o.authentication, "Authentication" );
            enter( o.authusername, "Username" );
            enter( o.authpassword, "Password" );
        }
        // IMAP accounts prompt for folder download
        if( o.mailtype == "IMAP" ){
            expectMessageBox( imapPullTitle, "Do you wish to retrieve", "Yes" ){
                select( "Back", softMenu() );
            }
        }else{
            select( "Back", softMenu() );
        }
    },

/*
    Remove a Messages email account based on supplied data
*/
    removeEmailAccount: function(o)
    {
        Messages.gotoMessagesHomeScreen();

        select("Account settings...",optionsMenu());
        waitForTitle( "Account settings", 10000 );
        if( getList().contains(o.name) ){
            select( o.name );                   // FIXME: work around until task 197129 is implemented
            waitForTitle( "Edit Account", 10000 );     // FIXME: work around until task 197129 is implemented
            select( "Cancel", optionsMenu() );  // FIXME: work around until task 197129 is implemented
            waitForTitle( "Account settings", 10000 ); // FIXME: work around until task 197129 is implemented
            expectMessageBox( "Email", "Delete account:", "Yes" ) {
                select( "Remove account...", optionsMenu() );
            };
            select( "Back", softMenu() );
        };
    },

/*
    Create a message of the type required & send it, taking into account flight_mode
*/
    sendMessage: function(msgObject, flight_mode )
    {
        msg_box_title = "Sending";
        msg_box_text = msg_box_title + ": ";
        msg_box_option = "OK";

        switch( msgObject.type ) {
            case "sms":
            title_name = "Message";
            msg_box_text = msg_box_text + title_name;
            break;
            case "mms":
            title_name = "Message";
            msg_box_text = msg_box_text + title_name;
            break;
            case "email":
            title_name = "Email";
            msg_box_text = msg_box_text + title_name;
            break;
            default:        fail( "Message 'type' needs to be selected." );
                            break;
        };

        switch( flight_mode ) {
            case true:  msg_box_title = "Airplane safe mode";
                        msg_box_text = "Saved message to Outbox. Message will be sent after exiting Airplane Safe mode.";
                        msg_box_option = "OK";
                        break;
            default:    break;
        };

        startApplication( "Messages" );

        Messages.composeMessage( msgObject );
        if( currentTestFunction() == "sending_bcc_messages" ){
            expectFail( "BUG-235609" );
        }
        expectMessageBox( msg_box_title, msg_box_text, msg_box_option ) {
            select( "Send" );
        };

        waitForTitle( "Messages", 10000 );
        select( "Back", softMenu() );
    },

/*
    Generic composer. Allows straightforward use of a compose action
*/
    composeMessage: function(msgObject)
    {
        select( "New message" );
        waitForTitle( "Select type" );
        switch( msgObject.type ){
            case "sms":
                Messages.composeSMS(msgObject);
                break;
            case "email":
                Messages.composeEmail(msgObject);
                break;
            case "im":
                Messages.composeIM(msgObject);
                break;
            case "mms":
                Messages.composeMMS(msgObject);
                break;
        }
    },
/*
    Compose SMS message.  Assumes view is the 'Select Type' dialog
*/
    composeSMS: function(msgObject)
    {
        select( "Text message" );
        waitForTitle( "Create Message" );
        if( msgObject.text ) enter( msgObject.text, "", NoCommit );
        select( "Next", softMenu() );
        waitForTitle( "Message details", 10000 );
        if( msgObject.to ) enter( msgObject.to, "To", NoCommit );
    },

/*
    Compose MMS, with attachments
*/
    composeMMS: function(msgObject)
    {
        select( "Multimedia message" );
        waitForTitle( "Create MMS" );

        var mediaSignature = signature(focusWidget());
        var textSignature = signature(focusWidget(),1);
        var audioSignature = signature(focusWidget(),2);
        if( msgObject.attachments ){
            for( var i = 0; i < msgObject.attachments.length; ++i ){
                if( msgObject.attachments[i].media ){
                    // TODO: select( "No media" );
                }
                if( msgObject.attachments[i].text ){
                    enter( msgObject.attachments[i].text, textSignature );
                }
                if( msgObject.attachments[i].audio ){
                    // TODO: select( "Audio" );
                }
            }
        }
        select( "Next", softMenu() );
        waitForTitle( "MMS details", 10000 );
        if( msgObject.to ) enter( msgObject.to, "To", NoCommit );
    },

/*
    Compose instant message
*/
    composeIM: function(msgObject)
    {
        select( "Instant message" );
        waitForTitle( "Create Message" );
        if( msgObject.text ) enter( msgObject.text, "", NoCommit );
        select( "Next", softMenu() );
        waitForTitle( "Message details", 10000 );
        if( msgObject.to ) enter( msgObject.to, "To", NoCommit );
    },

/*
    Compose email, with attachments
*/
    composeEmail: function(msgObject)
    {
        select( "Email" );
        waitForTitle( "Create Email" );
        enter( msgObject.text, "", NoCommit );
        // Enter attachments
        if( msgObject.attachments ){
            select( "Attachments...", optionsMenu() );
            waitForTitle( "Attachments" );
            for( var i = 0; i < msgObject.attachments.length; ++i ){
                select( "Add attachment", optionsMenu() );
                waitForTitle( "Add attachment" );
                select( msgObject.attachments[i].displayname );
                waitForTitle( "Attachments" );
            }
            select( "Back", softMenu() );
        }
        select( "Next", softMenu() );
        waitForTitle( "Email details", 10000 );
        enter( msgObject.to, "To", NoCommit );
        if( msgObject.cc ) enter( msgObject.cc, "CC", NoCommit );

        if( msgObject.bcc ) {
            //expectFail("BUG-224906");
            enter( msgObject.bcc, "BCC", NoCommit );
        }
        if( msgObject.subject ){
            //expectFail( "BUG-222854" ); // Crash on object being destroyed
            enter( msgObject.subject, "Subject", NoCommit );
        }
        if( msgObject.from ){
            select( msgObject.from, "From" );
        }

    },


/*
    Initiate an incoming short message
*/
    receiveSMS: function( type, sender, serviceCenter, message_text )
    {
        if( testModem() ){
            testModem().incomingSMS( type, sender, serviceCenter, message_text );
        }
    },

/*
    \usecase delete a message from Messages application
    arguments:  type            - message type currently: sms or email
                location        - where to find the message to be deleted
                identifier      - title of the message, eg. sender of the sms/email
                flight_mode     - if true, and location not set, then we will look in the outbox
*/
    removeMessage: function(type, location, identifier, flight_mode)
    {
        function moveToTrash()
        {
            select( location );
            waitForTitle( location, 10000 );
            UserInterface.highlight(identifier);
            expectMessageBox( "Moving", "Moving ", "OK" ) { // full message text is: "Moving 1 message to Trash" or plural
                select( "Move to Trash", optionsMenu() );
            };
        };

        Messages.gotoMessagesHomeScreen();
        switch( type ) {
            case "email":   title_name = "Email";
                select( title_name );
                waitForTitle( title_name, 10000 );
                moveToTrash();
            break;
            case "sms":     moveToTrash();
            break;
            default:        fail( "Message 'type' needs to be selected." );
            break;
        };
    },

/*
    Use the SIM toolkit to ask for an Account Balance send to us via an SMS.
*/
    requestBalanceViaSimToolkit: function()
    {
        startApplication( "SIM Applications", NoFlag );
        select( "My Account" );
        select( "Balance" );
        waitFor() { return getText( menuBarName() ) == "\n\nOK" ; }
        waitFor() { return getText( menuBarName() ) == "\nSelect\nBack"; }
        compare( getText( menuBarName() ), "\nSelect\nBack" );
    },

/*
    Use the SIM toolkit to ask for an Account Balance send to us via an SMS.
*/
    requestBalanceViaEmptySMS: function()
    {
        Messages.gotoMessagesHomeScreen();
        select( "New message" );
        select( "Text message" );

        if (operatorName == "vodafone AU") {
            enter( " " );
            enter( "1555" );
        } else if (operatorName == "Telstra Mobile") {
            enter( "test" );
            enter( "1767670001" );
        } else {
            fail( "Don't know how to send a Balance request for " + operatorName );
        }
    },

    gotoMessagesInbox: function()
    {
        Messages.gotoMessagesHomeScreen();
        select( "Inbox" );
    },

    gotoEmailHomeScreen: function()
    {
        Messages.gotoMessagesHomeScreen();
        select( "Email" );
        waitForTitle( "Email" );
    },

    gotoEmailInbox: function()
    {
        Messages.gotoEmailHomeScreen();
        select(getList()[0]);
    },

    gotoMessagesHomeScreen: function()
    {
        setSetting( "$HOME/Settings/Trolltech/qtmail.conf", "restart", "lastDraftId", "" );

        if (currentApplication() != "Messages"){
            //startApplication( "Messages" );
            gotoHome(); wait( 1000 ); select( "Messages", launcherMenu() );
            if(currentTitle() == "Incomplete message") select( "No", softMenu() );

        } else {
            while (currentTitle() != "Messages") {
                select( "Back", softMenu() );
            }
        }
/*
    // are we viewing the Inbox?
    if (focusWidget().indexOf( "messageView") >= 0) {
    select( "Back", softMenu());
    }

    verify( currentTitle() == "Messages");

    // make sure we're not in the email page
    list = getList();
    if (!list.contains( "Email" )) {
    select( "Back", softMenu());
    }

    verify( currentTitle() == "Messages");
*/
    },

    clearSendBox: function()
    {
        Messages.gotoMessagesHomeScreen();
        select( "Sent" );
        select( "Back", softMenu());
        var sent_count = messageCount();
        if (sent_count > 0) {
            select( "Sent" );
            if (sent_count > 1) {
                select( "Select all", optionsMenu() );
                select( "Delete messages", optionsMenu() );
            } else {
                select( "Delete message", optionsMenu() );
            }
        }
        Messages.gotoMessagesHomeScreen();
    },

    cleanupAccounts: function()
    {
        Messages.gotoMessagesHomeScreen();
        select( "Account settings...", optionsMenu() );
        waitForTitle( "Account settings" );
        var list;
        do {
            list = getList();
            if (list != undefined && list.length > 2) {
                if (list[2] != "MMS" && list[2] != "Collective") {
                    UserInterface.highlight( list[2] );
                    expectMessageBox(  "Email", "Delete account:", "Yes" ) {
                        select( "Remove account...", optionsMenu() );
                    }
                }
            }
            list = getList();
        } while (list.length > 2);
        Messages.gotoMessagesHomeScreen();
    },

    messageCount: function()
    {
        var ret = getSelectedText( signature(focusWidget(), 2) );
        var pos = ret.indexOf( " (" );
        if (pos > 0) {
            ret = ret.left(pos);
            return ret.toValue();
        }
        var pos = ret.indexOf( "(" );
        if (pos > 0) {
            ret = ret.left(pos);
            return ret.toValue();
        }
        if (ret != "")
            return ret.toValue();

        fail( "Couldn't 'read' the total number of messages available in the Inbox" );
        return undef;
    },

    newMessageCount: function()
    {
        var ret = getSelectedText( signature(focusWidget(), 2) );
        var pos = ret.indexOf( " new)" );
        if (pos > 0) {
            ret = ret.left(pos);
            pos = ret.indexOf( "(" );
            if (pos > 0) {
                ret = ret.slice( pos + 1 );
                return ret.toValue();
            }
        }
        fail( "Couldn't 'read' the total number of NEW messages available in the Inbox" );
        return -1;
    },

    waitRetrievingMail: function()
    {
        wait(100);
    //    waitProgressBar( [ "etrieving", "onnecting", "onnected", "ogging" ], false, 5000 );
        wait(5000);
    },

    deleteAllMail: function()
    {
        Messages.gotoEmailHomeScreen();
        var mailboxes = ["Inbox","Sent","Drafts","Outbox"];
        for( var i = 0; i < mailboxes.length; ++i ){
            select( mailboxes[i] );
            waitForTitle( mailboxes[i] );
            var optionslist = getList(optionsMenu());
            if( getList(optionsMenu()).contains("Select all") ){
                select( "Select all", optionsMenu() );
                ignoreMessageBoxes(true);
                select( "Move to Trash", optionsMenu() );
                wait(2000);
                ignoreMessageBoxes(false);
            }else if( optionslist.contains("Move to Trash") ){
                ignoreMessageBoxes(true);
                select( "Move to Trash", optionsMenu() );
                wait(2000);
                ignoreMessageBoxes(false);
            }
            select( "Back", softMenu() );
        }
        if( getList(optionsMenu()).contains("Empty trash") ){
            expectMessageBox("Empty trash", "Are you sure you want to delete:", "Yes") {
                select( "Empty trash", optionsMenu() );
            }
        }
    },

    deleteAllMessages: function()
    {
        Messages.gotoMessagesHomeScreen();
        var mailboxes = ["Inbox","Sent","Drafts","Outbox"];
        for( var i = 0; i < mailboxes.length; ++i ){
            select( mailboxes[i] );
            waitForTitle( mailboxes[i] );
            var optionslist = getList(optionsMenu());
            if( optionslist.contains("Select all") ){
                select( "Select all", optionsMenu() );
                ignoreMessageBoxes(true);
                select( "Move to Trash", optionsMenu() );
                wait(2000);
                ignoreMessageBoxes(false);
            }else if( optionslist.contains("Move to Trash") ){
                ignoreMessageBoxes(true);
                select( "Move to Trash", optionsMenu() );
                wait(2000);
                ignoreMessageBoxes(false);
            }
            select( "Back", softMenu() );
        }
        if( getList(optionsMenu()).contains("Empty trash") ){
            expectMessageBox("Empty trash", "Are you sure you want to delete:", "Yes") {
                select( "Empty trash", optionsMenu() );
            }
        }
    },

/*
    Send an test email:
        to      - recipient address
        subject - message subject line
        msg     - message body text
        frm     - sender name
        replyto - sender address
*/
    sendTestMessage: function( to, subject, msg, from, replyto, cc, bcc )
    {
        Mail.sendMessage( to, subject, msg, from, replyto );
    },

/*
    Get new emails from message account 'ma'
*/
    getNewTestMessage: function( ma )
    {
        port = "143";
        return Mail.getNewMessage( ma.inserver, ma.username, ma.password, port );
    },

    getNewMessageCount: function( ma )
    {
        port = "143";
        return Mail.getNewMessageCount( ma.inserver, ma.username, ma.password, port );
    },

/*
    Remove all emails from account 'ac'
*/
    expungeMessages: function( ac )
    {
        port = "143";
        reply = "";
        return Mail.expungeMessages( ac.inserver, ac.username, ac.password, port );
    },

/*
    Verify that the required setup looks OK
*/
    verifySetup: function()
    {
        if ( !Mail.verifySetup() ) {
            fail( "Required Perl modules for mail.js not found." );
        }
    }

};

