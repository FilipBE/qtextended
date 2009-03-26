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

//TESTED_COMPONENT=Messaging: Email (18599)

include( "messages.js" );
include( "accounts.js" );
include( "userinterface.js" );

testcase = {

    initTestCase: function()
    {
        Messages.verifySetup();
        waitForQtopiaStart();
        putFile( baseDataPath() + "cows.png", "$HOME/Documents/cows.png" );
        putFile( baseDataPath() + "tiger.jpg", "$HOME/Documents/tiger.jpg" );
        ipcSend("QPE/DocAPI", "scanPath(QString,int)", "all", 1);
        startApplication( "Messages" );
        Messages.cleanupAccounts();
        Messages.expungeMessages(Accounts.getAccount(1));
        Messages.expungeMessages(Accounts.getAccount(2));
        Messages.createEmailAccount( Accounts.getAccount(1) );
    },

    cleanupTestCase: function()
    {
        waitExpectedMessageBox(5000);
    },

    init: function()
    {
        setSetting( "$HOME/Settings/Trolltech/qtmail.conf", "restart", "lastDraftId", "" );
        while( currentApplication == "simapp") select( "Back", softMenu());
        if (currentApplication() != "qtmail") startApplication( "Messages" );
    },

    cleanup: function()
    {
        if( currentTitle() == "Message details" ){
            select( "Cancel", optionsMenu() );
        }
        gotoHome();
        Messages.expungeMessages(Accounts.getAccount(1));
        Messages.expungeMessages(Accounts.getAccount(2));

    },

/*
    \req QTOPIA-503
    \groups Acceptance
*/
    sending_an_email_message_data:
    {
        sendingAnEmail:[Accounts.getAccount(2), "Test email text", "Test email" ],
        sendingAnEmailWithNoSubject:[Accounts.getAccount(2), "Test email text", undefined ]

    },

    sending_an_email_message: function(targetAccount, test_text, test_subject)
    {
        Messages.sendMessage(  {type: "email",
                                to:   targetAccount.address,
                                text: test_text,
                                subject: test_subject},
                                false );
        var postedMessage = Messages.getNewTestMessage( targetAccount );
        if( test_subject ){
            verify( postedMessage.contains("Subject: "+test_subject) );
        }
        verify( postedMessage.contains(test_text) );
    },

/*
    \req QTOPIA-504
    \groups
*/
    deleting_a_message_from_the_server: function()
    {
        prompt(
            "Preconditions 1. An IMAP/POP account is available.\n"+
            "* The device is able to connect to the IMAP/POP account.\n"+
            "* An external email client is able to contact the IMAP/POP account.\n"+
            "* Select the 'Account settings...' option from the context menu.\n"+
            "* Verify that an 'Account settings' dialog is presented.\n"+
            "* Select 'Add Account' from the context menu.\n"+
            "* Verify that a 'Create New Account' dialog is presented.\n"+
            "* Select 'IMAP' from the 'Type' drop-down list and enter in the appropriate details to connect to the IMAP account.\n"+
            "* Enable the 'Delete mail' option then select the 'Done' context menu option to create the account.\n"+
            "* Verify that the created account is now listed in the 'Account settings' dialog then press 'Done' to return to the 'Messages' dialog.\n"+
            "* Select 'Get all mail' from the context menu.\n"+
            "* Verify that the messages are downloaded onto the device.\n"+
            "* View a folder containing an email.\n"+
            "* Select an email for deletion.\n"+
            "* Verify that the 'Read Mail' dialog is displayed showing the contents of the selected message.\n"+
            "* Select 'Move to Trash' from the context menu.\n"+
            "* Verify that the display returns to the list of remaining emails and the deleted message is moved to the 'Trash' folder.\n"+
            "* Return to the folder containing the list of emails and select but don't open another email.\n"+
            "* Select 'Move to Trash' from the context menu.\n"+
            "* Verify that the display returns to the list of remaining emails and the deleted message is moved to the 'Trash' folder.\n"+
            "* Return to the email folder and select 'Empty trash' from the context menu.\n"+
            "* Select 'Yes' from the presented confirmation dialog.\n"+
            "* Verify that all messages found in the 'Trash' folder are removed from the device.\n"+
            "* Select 'Get all mail' again to update the IMAP account.\n"+
            "* Connect to the IMAP account using another email client.\n"+
            "* Verify that the deleted message is not listed in the IMAP account.\n"+
            "* Repeat the test using the POP mail account.\n"
        );
    },

/*
    \req QTOPIA-841
    \groups Acceptance
*/
    receiving_an_email_message_data:
    {
        receivedEmail:[ Accounts.getAccount(1), "Receive test", "This is a test message", Accounts.getAccount(2) ]
    },

    receiving_an_email_message: function( userAccount, subject, message, senderAccount)
    {
        initMsgCount = Messages.getNewMessageCount( userAccount );
        Messages.sendTestMessage( userAccount.address, subject, message, senderAccount.accountname, senderAccount.address );
        waitFor() {
            wait( 500 );
            return (Messages.getNewMessageCount(userAccount) > initMsgCount);
        }

        Messages.gotoEmailHomeScreen();
        addExpectedMessageBox( "New message", "arrived. Do you wish to view", "No" );
        select( "Get all mail", optionsMenu() );
        clearExpectedMessageBoxes();
        Messages.waitRetrievingMail();
        select( userAccount.accountname );
        waitForTitle( userAccount.accountname );
        //expectFail("BUG-222441");
        verify( getList().contains(senderAccount.accountname) );
        select( senderAccount.accountname );
        waitForTitle( senderAccount.accountname );
    },


/*
    \req QTOPIA-505
    \groups Acceptance
*/
    sending_an_email_attachment_data:
    {
        singleAttachment: [Accounts.getAccount(2), "Test email text", [{displayname: "cows",
                                                                        filename:    "cows.png",
                                                                        contenttype: "image/png"}]],

        multipleAttachments: [Accounts.getAccount(2), "Test email text", [{displayname:  "cows",
                                                                            filename:    "cows.png",
                                                                            contenttype: "image/png"},
                                                                            {displayname:  "tiger",
                                                                            filename:    "tiger.jpg",
                                                                            contenttype: "image/jpeg"} ]]
    },

    sending_an_email_attachment: function(targetAccount, bodytext, attached)
    {
        Messages.sendMessage(  {type: "email",
                                to:   targetAccount.address,
                                text: bodytext,
                                attachments: attached},
                                false );
        var inbox = Messages.getNewTestMessage( targetAccount );
        for( var i = 0; i < attached.length; ++i ){
            verify( inbox.contains("Content-Type: "+attached[i].contenttype+"; name="+attached[i].displayname) );
            verify( inbox.contains("Content-Disposition: attachment; filename="+attached[i].filename) );
        }
    },

/*
    \req QTOPIA-8219
    \groups
*/
    replying_to_an_email_data:
    {
        replyToEmail:[ "Reply to me", "Please respond", "tester", Accounts.getAccount(1), Accounts.getAccount(2), "This is the response"]
    },

    replying_to_an_email: function( subject, message, remoteuser, userAccount, senderAccount, reply)
    {
        initMsgCount = Messages.getNewMessageCount( userAccount );
        Messages.sendTestMessage( userAccount.address, subject, message, senderAccount.accountname, senderAccount.address );
        waitFor() {
            wait( 500 );
            return (Messages.getNewMessageCount(userAccount) > initMsgCount);
        }

        Messages.gotoEmailHomeScreen();

        addExpectedMessageBox("New message", "arrived. Do you wish to view", "No");
        select( "Get all mail", optionsMenu() );
        clearExpectedMessageBoxes();
        Messages.waitRetrievingMail();
        select( userAccount.accountname );
        waitForTitle( userAccount.accountname );
        //expectFail("BUG-222441");
        verify( getList().contains(senderAccount.accountname) );
        select( senderAccount.accountname );
        waitForTitle( senderAccount.accountname );
        select( "Reply", optionsMenu() );
        enter( reply, "", NoCommit );
        select( "Next", softMenu() );
        waitForTitle( "Email details" );
        compare( getText("To"), senderAccount.address );
        initMsgCount = Messages.getNewMessageCount( senderAccount );
        expectMessageBox( "Sending", "Sending: Email", "" ) {
            select( "Send", softMenu() );
        }
        // Wait for message to arrive on server...
        waitFor() {
            wait(500);
            return (Messages.getNewMessageCount(senderAccount) > initMsgCount);
        }
        var postedMessage = Messages.getNewTestMessage( senderAccount );
        verify( postedMessage.contains(reply) );
    },

/*
    \req QTOPIA-8219
    \groups
*/
    replying_to_multiple_recipients_data:
    {
        replyToEmail:[ "Reply", "Please respond", Accounts.getAccount(1), Accounts.getAccount(2), Accounts.blackHoleEmailAddress()]
    },

    replying_to_multiple_recipients: function( subject, message, userAccount, senderAccount, secondRecipient)
    {
        var recipients = userAccount.address + "," + secondRecipient;
        initMsgCount = Messages.getNewMessageCount( userAccount );
        Messages.sendTestMessage( recipients, subject, message, senderAccount.accountname, senderAccount.address );
        waitFor() {
            wait(500);
            return (Messages.getNewMessageCount(userAccount) > initMsgCount);
        }

        Messages.gotoEmailHomeScreen();
        addExpectedMessageBox("New message", "arrived. Do you wish to view", "No");
        select( "Get all mail", optionsMenu() );
        clearExpectedMessageBoxes();
        Messages.waitRetrievingMail();
        select( userAccount.accountname );
        waitForTitle( userAccount.accountname );
        //expectFail("BUG-222441");
        verify( getList().contains(senderAccount.accountname) );
        select( senderAccount.accountname );
        waitForTitle( senderAccount.accountname );
        select( "Reply all", optionsMenu() );
        waitForTitle( "Reply to all" );
        select( "Next", softMenu() );
        waitForTitle( "Email details" );
        compare( getText("To"), senderAccount.address );
        compare( getText("CC"), secondRecipient );
        select( "Cancel", optionsMenu() );
    },


/*
    \req QTOPIA-781
    \groups
*/
    message_status: function()
    {
        prompt(
            "Preconditions 1. A number of emails have already been retrieved on the device.\n"+
            "* A outgoing email account has been specified.\n"+
            "* Select the folder for the email account, this will be a sub-folder in the 'email' folder.\n"+
            "* Verify that a number of existing emails are listed.\n"+
            "* Verify that a unread message is displayed with an 'Unread' flag (a blue ball icon is displayed in the flag field).\n"+
            "* Select the message to view its contents (and mark it as read).\n"+
            "* Select the 'Done' option to return to the messages list.\n"+
            "* Verify that the read messages flag has changed to the 'Read' flag (a green arrow pointing right).\n"+
            "* Compose a new E-mail message.\n"+
            "* Enter the text 'nothing' into the body, then select 'Done' without entering any email addresses to store the message in the drafts folder.\n"+
            "* Verify that the message is placed in the 'Drafts' folder with the draft-with-no-address flag displayed (a yellow triangle).\n"+
            "* Edit the message in the 'Drafts' folder and specify the 'To' and 'Subject' fields, then save the message to drafts.\n"+
            "* Verify that the message is placed in the 'Drafts' folder with the draft flag displayed (a green arrow pointing up).\n"+
            "* Edit the message again and this time select 'Send'.\n"+
            "* Verify that the message is placed in the 'Outbox'.\n"+
            "* After the message has been sent verify that the message is in the 'Outbox' and now has the flag set to 'Sent' (a green arrow pointing right similar to the 'Read' flag).\n"
        );
    },

/*
    \req QTOPIA-506
    \groups
*/
    sending_bcc_messages_data:
    {
        sendAMessageToBCC: [Accounts.getAccount(1), "BCC", "Should not see BCC", Accounts.getAccount(2)]
    },

    sending_bcc_messages: function(userAccount, subject, text, recipientAccount)
    {
        Messages.createEmailAccount( recipientAccount );
        var messageObject = {
            type: "email",
            to: userAccount.address,
            bcc: recipientAccount.address,
            text: text,
            subject: subject };

        Messages.sendMessage( messageObject, false );
        Messages.gotoEmailHomeScreen();
        addExpectedMessageBox("New message", "arrived. Do you wish to view", "No");
        select( "Get all mail", optionsMenu() );
        clearExpectedMessageBoxes();
        Messages.waitRetrievingMail();
        select( recipientAccount.accountname );
        waitForTitle( recipientAccount.accountname );
        //expectFail("BUG-222441");
        select( "Select", softMenu() );
        verify( !getText().contains(recipientAccount.address) );
    },

/*
    \req QTOPIA-781
    \groups
*/
    viewing_messages_in_text_mode_data:
    {
        viewEmailInTextMode:[ Accounts.getAccount(1), "Text test", "This is a text test message", Accounts.getAccount(2) ]
    },

    viewing_messages_in_text_mode: function( userAccount, subject, message, senderAccount )
    {
        initMsgCount = Messages.getNewMessageCount( userAccount );
        Messages.sendTestMessage( userAccount.address, subject, message.text, senderAccount.accountname, senderAccount.address );
        waitFor() {
            wait( 500 );
            return (Messages.getNewMessageCount(userAccount) > initMsgCount);
        }

        Messages.gotoEmailHomeScreen();
        addExpectedMessageBox("New message", "arrived. Do you wish to view", "No");
        select( "Get all mail", optionsMenu() );
        clearExpectedMessageBoxes();
        Messages.waitRetrievingMail();
        select( userAccount.accountname );
        waitForTitle( userAccount.accountname );
        verify( getList().contains(senderAccount.accountname) );
        select( senderAccount.accountname );
        waitForTitle( senderAccount.accountname );
        select( "Plain text", optionsMenu() );
        var plainTxt = getText();
        verify( plainTxt.contains("Subject:") );
    },

/*
    \req QTOPIA-505
    \groups Acceptance
*/
    receiving_a_message_with_an_attachment: function()
    {
        Messages.gotoMessagesHomeScreen();
        prompt(
            "* Send an email with an attachment to the configured email address\n"+
            "* Select 'Get all mail' from the context menu.\n"+
            "* Verify that messages stored on the mail server are downloaded onto the device.\n"+
            "* Select the 'Inbox' to view the newly received messages list.\n"+
            "* Select the message containing the attachments.\n"+
            "* Verify that the message displays the messages body, attachment hyperlinks and displays any inline attachments such as images.\n"+
            "* Select an attachment hyperlink.\n"+
            "* Verify that the 'Attachments' dialog is presented displaying all the attachments in the message.\n"+
            "* Check the binary file from the list then select 'Done'.\n"+
            "* Close the 'Messages' application and verify that the messages binary attachment is now listed in the 'Documents' launcher.\n"
        );
    },

/*
    \req QTOPIA-781
    \groups
*/
    forwarding_an_email_message_data:
    {
        forwardStandardEmail:[Accounts.getAccount(1), Accounts.getAccount(2), {text: "Forward this"}]
    },

    forwarding_an_email_message: function(sender, recipient, message)
    {
        Messages.createEmailAccount( recipient );
        initMsgCount = Messages.getNewMessageCount( sender );
        Messages.sendTestMessage( sender.address, "", message.text, "Messenger", Accounts.blackHoleEmailAddress() );
        waitFor() {
            wait( 500 );
            return (Messages.getNewMessageCount(sender) > initMsgCount);
        }

        Messages.gotoEmailHomeScreen();
        addExpectedMessageBox("New message", "arrived. Do you wish to view", "No");
        select( "Get all mail", optionsMenu() );
        clearExpectedMessageBoxes();
        Messages.waitRetrievingMail();
        select( sender.accountname );
        select( "Messenger" );
        verify( getText().contains(message.text) );
        select( "Forward", optionsMenu() );
        waitForTitle( "Forward Email" );
        select( "Next", softMenu() );
        //expectFail("BUG-225066");
        compare( getText("To"), "" );
        enter( recipient.address, "To", NoCommit );
        expectMessageBox( "Sending", "Sending: Email", "" ) {
            select( "Send", softMenu() );
        }
        Messages.gotoEmailHomeScreen();
        select( "Get all mail", optionsMenu() );
        select( recipient.accountname );
        select( sender.alias );
        verify( getText().contains(message.text) );
    },

/*
    \req QTOPIA-782
    \groups
*/
    discarding_a_composed_message: function()
    {
        prompt(
            "* Select 'New Message' from the Messages menu.\n"+
            "* Verify that the 'Select Type' dialog is presented.\n"+
            "* Select the 'Email' type from the dialog and verify that the 'Create Email' dialog is presented.\n"+
            "* Begin composing the message by entering the text 'this is a test'.\n"+
            "* Select 'Cancel' from the context menu.\n"+
            "* Verify that the dialog is closed and the folder listing is presented leaving no sign of the half composed message.\n"+
            "* Again select 'New' from the context menu.\n"+
            "* Select 'Email' from the type dialog.\n"+
            "* Compose the message in the body of the email the select 'Next' to view the 'Email Details' dialog.\n"+
            "* Enter text into the 'To' and 'Subject' fields then select 'Cancel' from the context menu.\n"+
            "* Verify that the message is discarded.\n"+
            "* Repeat the tests using the 'Multimedia Message' and 'Text Message' message types.\n"
        );
    },

/*
    \req QTOPIA-775
    \groups
*/
    specifying_a_default_mail_server: function()
    {
        prompt(
            "Preconditions 1. A number of SMTP accounts are accessible for use.\n"+
            "* An external email client is available to view the sent message.\n"+
            "* Select 'Account settings...' from the context menu to view the 'Account settings' dialog.\n"+
            "* Select 'Add Account' to create a new email account.\n"+
            "* Select the 'Outgoing' tab and enter the appropriate outgoing mail account details.\n"+
            "* Select 'Done' to create the account.\n"+
            "* Repeat the previous steps, this time creating an outgoing account with a different server, name and email address.\n"+
            "* Enable the 'Default mail server' option then select 'Done' to create the account.\n"+
            "* Return to the folder listing and select 'New' from the context menu.\n"+
            "* Select 'Email' from the 'Select Type' dialog.\n"+
            "* Compose the body of the message then select 'Next'.\n"+
            "* Verify that the 'Email Details' dialog is displayed with the 'From' field displaying the email address of the specified default mail server.\n"+
            "* Select the other outgoing mail server from the 'From' drop-down list.\n"+
            "* Enter an email address in the 'To' field which is able to verify that the sent message is using the selected outgoing mail server, then select 'Send'.\n"+
            "* Verify that the message is sent.\n"+
            "* Using an external mail client retrieve the sent message.\n"+
            "* Verify that the 'From' field uses the selected mail server/sender details.\n"+
            "* Repeat the test using the default mail server for sending the message.\n"
        );
    },

/*
    \req QTOPIA-504
    \groups
*/
    skipping_mail_larger_than_a_specified_size: function()
    {
        prompt(
            "Preconditions 1. An incoming mail account is accessible which contains a number of messages with differing sizes.\n"+
            "* Select 'Account settings' from the context menu to view the 'Account settings' dialog.\n"+
            "* Select 'Add Account...' from the context menu.\n"+
            "* Verify that the 'Create New Account' dialog is displayed.\n"+
            "* Create a new account using valid account details and enable the 'Skip larger' option, leaving the default value at '5K' then select 'Done'.\n"+
            "* Return to the folder listing and select 'Get all mail' from the context menu.\n"+
            "* Verify that the emails stored on the specified incoming mail server are downloaded onto the device.\n"+
            "* Once the messages have been received, select the inbox to view the message list.\n"+
            "* Select a message that is less that 5K in size.\n"+
            "* Verify that the message contents is displayed.\n"+
            "* Return to the message list and select a message that is greater than 5K.\n"+
            "* Verify that the messages contents state 'Awaiting download, Size of mail: XX K'.\n"+
            "* Select 'Get this mail' from the context menu.\n"+
            "* Verify that the entire message is now downloaded onto the device.\n"
        );
    },

/*
    \req QTOPIA-507
    \groups
*/
    Encrypted_and_Authenticated_Connections_IMAP_data:
    {
    },

    Encrypted_and_Authenticated_Connections_IMAP: function()
    {
        prompt(
            "Preconditions 1. For this test it is required that you have network access.\n"+
            "* An IMAP account is available with existing emails stored on it.\n"+
            "* Another email client is able to verify the contents of the IMAP account.\n"+
            "* Select 'Account settings...' from the context menu.\n"+
            "* Verify that the 'Account settings' dialog is presented.\n"+
            "* Select 'Add Account' from the context menu.\n"+
            "* View the 'Incoming' tab and select 'IMAP' from the 'Type' drop-down list.\n"+
            "* Enter your username, password and server address.\n"+
            "* Verify that the port is set as required for the account.\n"+
            "* Verify that the 'Delete mail' option is checked.\n"+
            "* Verify that the 'Skip larger' option is unchecked.\n"+
            "* Verify that the 'Encryption' is set to SSL.\n"+
            "* View the 'Outgoing' tab and enter the account name 'MyAccount'\n"+
            "* Enter your username, password and server address.\n"+
            "* Verify that the port is set as required for the account.\n"+
            "* Verify that the 'Encryption' is set to TLS.\n"+
            "* Select 'Done' to close the 'Create New account' dialog.\n"+
            "* Verify that the account 'MyAccount' is listed in the 'Account settings' dialog.\n"+
            "* Verify that a folder with the name 'MyAccount' is shown as a sub-folder of the Email folder.\n"+
            "* Select 'Get all mail' from the context menu.\n"+
            "* Verify that the device reports its status in the status bar with information such as: DNS lookup Retrieving folders Retrieving messages.\n"+
            "* Select the 'Inbox' folder and verify that the same messages are available from the 'Messages' application as to another e-mail clients on a desktop machine connected to the same account.\n"
        );
    },

/*
    \req QTOPIA-507
    \groups
*/
    Encrypted_and_Authenticated_Connections_POP_data:
    {
    },

    Encrypted_and_Authenticated_Connections_POP: function()
    {
        prompt(
            "Preconditions 1. For this test it is required that you have network access.\n"+
            "* An POP account is available with existing emails stored on it.\n"+
            "* Another email client is able to verify the contents of the POP account.\n"+
            "* Select 'Account settings...' from the context menu.\n"+
            "* Verify that the 'Account settings' dialog is presented.\n"+
            "* Select 'Add Account' from the context menu.\n"+
            "* View the 'Incoming' tab and select 'POP' from the 'Type' drop-down list.\n"+
            "* Enter your username, password and server address.\n"+
            "* Verify that the port is set as required for the account.\n"+
            "* Verify that the 'Delete mail' option is checked.\n"+
            "* Verify that the 'Skip larger' option is unchecked.\n"+
            "* Verify that the 'Encryption' is set to SSL.\n"+
            "* View the 'Outgoing' tab and enter the account name 'MyAccount'\n"+
            "* Enter your username, password and server address.\n"+
            "* Verify that the port is set as required for the account.\n"+
            "* Verify that the 'Encryption' is set to TLS.\n"+
            "* Select 'Done' to close the 'Create New account' dialog.\n"+
            "* Verify that the account 'MyAccount' is listed in the 'Account settings' dialog.\n"+
            "* Verify that a folder with the name 'MyAccount' is shown as a sub-folder of the Email folder.\n"+
            "* Select 'Get all mail' from the context menu.\n"+
            "* Verify that the device reports its status in the status bar with information such as: DNS lookup Retrieving folders Retrieving messages.\n"+
            "* Select the 'Inbox' folder and verify that the same messages are available from the 'Messages' application as to another e-mail clients on a desktop machine connected to the same account.\n"
        );
    },

/*
    \req QTOPIA-8351
*/
    interval_checking_data:
    {
    },

    interval_checking: function()
    {
        skip("Test not implemented");
    },

/*
    \req BUG-212343
*/
    interval_checking_over_multiple_accounts_data:
    {

    },

    interval_checking_over_multiple_accounts: function()
    {
        skip("Test not implemented");
    },

/*
    \req BUG-228480
*/
    large_mail_download_data:
    {
        download50emails: [Accounts.getAccount(1), Accounts.getAccount(2), 50],
        download500emails: [Accounts.getAccount(1), Accounts.getAccount(2), 500]
    },

    large_mail_download: function(userAccount, senderAccount, numberofemails)
    {
        var subject;
        var message;
        print( "Sending "+numberofemails+" emails..." );
        for( var i=0; i<numberofemails; ++i ){
            subject = "Message "+i;
            message = "Message "+i+" of "+numberofemails;
            Messages.sendTestMessage( userAccount.address, subject, message, senderAccount.accountname, senderAccount.address );
        }
        print( "Sent "+i+" emails... Waiting for server." );
        waitFor( 600000, 60, "Only received "+ Messages.getNewMessageCount( userAccount ) + "emails" ){
            return Messages.getNewMessageCount( userAccount ) >= numberofemails;
        }
        print( "Server has "+i+" new emails." );
        Messages.gotoEmailHomeScreen();
        addExpectedMessageBox( "New message", "arrived. Do you wish to view", "No" );
        select( "Get all mail", optionsMenu() );
        Messages.waitRetrievingMail();
        waitExpectedMessageBox( 5000 );
        select( userAccount.accountname );
        waitForTitle( userAccount.accountname );
        var messagelist = getList();
        verify( messagelist.contains(senderAccount.accountname) );
        verify( messagelist.length >= numberofemails );
        select( senderAccount.accountname );
        waitForTitle( senderAccount.accountname );


    }

} // end of testcase
