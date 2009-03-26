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

//TESTED_COMPONENT=QA: Testing Framework (18707)

testcase = {
    /* Test modem */
    tm: undefined,

    initTestCase: function() {
        waitForQtopiaStart();
        //setDemoMode(true);
    },

    init: function() {
        gotoHome();
    },

    cleanup: function() {
        // If we have a test modem, get rid of it.
        // Catch the exception which is thrown if tm is already destroyed or never
        // existed.
        try {
            this.tm.destroy();
        } catch(e) {
        }
        this.tm = undefined;
        setSetting( "Trolltech", "qpe", "CallControl", "MissedCalls", 0);
    },

/*
    \req QTOPIA-78

    \groups
    enter() at homescreen
*/
    enter_homescreen: function(number) {
        gotoHome();

        enter(number);
        var list = getList(optionsMenu());
        verify(list.contains("Save to Contacts"), "options menu contains: " + list);
        compare(getText(), number);

        /* FIXME due to bug 187475, gotoHome() doesn't work right and can cause
            * further tests to fail.  Must manually gotoHome().
            */
        for (var i = 0;
                i < 2000 && !getList(softMenu()).contains("Delete") && !getList(softMenu()).contains("Cancel");
                i += 100, wait(100))
        {}
        while (getList(softMenu()).contains("Delete")) {
            select("Delete", softMenu());
        }
        while (getList(softMenu()).contains("Cancel")) {
            select("Cancel", softMenu());
        }
    },

    enter_homescreen_data: {
        simple1: "12345",
        simple2: "54321"
        //, symbols: "123+45#67*89"
    },

/*
    \req QTOPIA-78
    enter() with text fields.
*/
    enter_text: function(text) {
        startApplication("testapp1");

        enter(text, "LineEdit1");
        compare( getText("Status"), "LineEdit1 text changed" );
        compare( getText("LineEdit1"), text );

        enter("clear", "LineEdit2");
        compare( getText("Status"), "LineEdit2 text changed" );
        compare( getText("LineEdit2"), "clear" );

        /* test that we can erase the whole string when the cursor isn't at the end by hitting the home key to get to the front */
        if (mousePreferred()) {
            mouseClick("LineEdit1"); keyClick(Qt.Key_Home); mouseClick("LineEdit2");
        }
        enter("overwrite", "LineEdit1");
        compare( getText("LineEdit1"), "overwrite" );
    },

/*
    \req QTOPIA-78

    \groups
*/
    enter_text_data: {
        mixed_case:   [ "MiXeD CASe" ],
        with_numbers: [ "1 2 3 4 25 312" ],
        in_dict:      [ "hello kitty" ],
        not_in_dict:  [ "jtnerlnjas mtrnen" ],
        long_str:     [ "this is a really long string to test we can overwrite properly" ],

        /* Test for bug: "Jimmy" sometimes gives "Jinny" in predictive
            * keyboard. */
        jimmy:        [ "Jimmy jimmy" ]
    },

    enter_QTextEdit: function(text) {
        startApplication("testapp3");

        select("Text", tabBar());

        enter(text, "Text");
        compare( getText("Text"), text );

        enter("overwrite", "Text");
        compare( getText("Text"), "overwrite" );
    },

/*
    \req QTOPIA-78
    Test Commit/NoCommit with line edit fields
*/
    enter_text_nocommit: function() {
        startApplication("testapp1");

        enter("hello", "LineEdit1", NoCommit);
        wait(500); // softmenu update is asynchronous
        verify( getList(softMenu()).contains("Accept") );
        compare( getText("LineEdit1"), "hello" );

        enter(" world", "LineEdit1", Commit);
        wait(500);
        verify( !getList(softMenu()).contains("Accept") );
        compare( getText("LineEdit1"), "hello world" );

        enter("goodbye", "LineEdit1");
        wait(500);
        verify( !getList(softMenu()).contains("Accept") );
        compare( getText("LineEdit1"), "goodbye" );
    },

/*
    \req QTOPIA-78
    Test Commit/NoCommit with line textedit fields
*/
    enter_QTextEdit_nocommit: function() {
        startApplication("testapp3");

        select("Text", tabBar());

        var text1 = "This is some text";
        var text2 = ", and this is some more";

        enter( text1, "Text", NoCommit );
        wait(500);
        verify( getList(softMenu()).contains("Accept") );
        compare( getText("Text"), text1 );

        enter( text2, "Text", Commit );
        wait(500);
        verify( !getList(softMenu()).contains("Accept") );
        compare( getText("Text"), text1+text2 );

        enter("overwrite", "Text");
        wait(500);
        verify( !getList(softMenu()).contains("Accept") );
        compare( getText("Text"), "overwrite" );
    },
/*
    \req QTOPIA-78

    \groups
    enter() with QTimeEdit fields.
*/
    enter_QTextEdit_data: {
        single_line: [ "This is a single line" ],
        //multi_lines: [ "This is\nmultiple lines of\n\ntext" ]
        alotta_text: [ "This is a lot of text to ensure we get a scroll bar happening yes this is a lot of text to ensure we get a scroll bar happening yes" ]
    },

    enter_QTimeEdit: function(time, mode) {
        startApplication("testapp1");

        select("Awesome Tab", tabBar());

        enter(time, "Time", mode);
        wait(500);

        if ( mode == NoCommit ) {
            verify( getList(softMenu()).contains("Accept") );
        } else {
            verify( !getList(softMenu()).contains("Accept") );
        }

        compare( getText("Time"), time.toString("hh:mm:ss") );
    },

/*
    \req QTOPIA-78

    \groups
*/
    enter_QTimeEdit_data: {
        nocommit: [ new QTime(16, 55, 00), NoCommit ],
        simple:  [ new QTime(14, 10, 38), Commit ]
    },

    /* enter() with QDateEdit fields. */
    enter_QDateEdit: function(date, mode) {
        startApplication("testapp1");

        select("Awesome Tab", tabBar());

        enter(date, "Date", mode);
        wait(500);

        if ( mode == NoCommit ) {
            verify( getList(softMenu()).contains("Accept") );
        } else {
            verify( !getList(softMenu()).contains("Accept") );
        }

        compare( getText("Date"), date.toString("dd/MM/yyyy") );
    },

/*
    \req QTOPIA-78

    \groups
*/
    enter_QDateEdit_data: {
        simple:       [ new QDate(2000, 6,  15), Commit ],
        december:     [ new QDate(2000, 12, 15), Commit ],
        end_of_month: [ new QDate(2002, 6, 30), Commit ],
        nocommit:     [ new QDate(2004, 10, 11), NoCommit ]
    },


    /* Explicit mouse clicks on widgets */
    mouseClick: function() {
        startApplication("testapp1");

	/* Because mouseClick(), unlike select(), does not / cannot verify that
	 * the expected widget receives the click, it is possible that (due to
	 * focus being grabbed etc), it can take more than one try for the
	 * button to actually get clicked.
	 */
	for (var i = 0; i < 2 && getText("Status") != "Button1 clicked"; ++i) {
	        mouseClick("Button1");
	        wait(1500); // BUG 194361
	}
        compare( getText("Status"), "Button1 clicked" );

	for (var i = 0; i < 2 && getText("Status") != "Button2 clicked"; ++i) {
	        mouseClick("Button2");
	        wait(1500); // BUG 194361
	}
        compare( getText("Status"), "Button2 clicked" );
    },

/*
    \req QTOPIA-78

    \groups
    selecting buttons
*/
    select_button: function() {
        startApplication("testapp1");
        select("Button1");
        compare( getText("Status"), "Button1 clicked" );
        select("Button2");
        compare( getText("Status"), "Button2 clicked" );
    },


    select_close: function() {
        startApplication("testapp1");
        select("Close");
    },
/*
    \req QTOPIA-78

    \groups
    select() things from the optionMenu()
*/
    select_optionsMenu: function() {
        startApplication("testapp1");

        select( "Button1" );
        compare( getText("Status"), "Button1 clicked" );

        select( "Action 1", optionsMenu() );
        compare( getText("Status"), "Action 1 activated" );

        select( "Action 2", optionsMenu() );
        compare( getText("Status"), "Action 2 activated" );

        select( "Submenu/Subaction 1", optionsMenu() );
        compare( getText("Status"), "Subaction 1 activated" );

        select( "Submenu/Subaction 2", optionsMenu() );
        compare( getText("Status"), "Subaction 2 activated" );

        select( "Last!", optionsMenu() );
        compare( getText("Status"), "Last activated" );
    },

/*
    \req QTOPIA-78

    \groups
    select() things from the tabBar()
*/
    select_tabBar: function() {
        startApplication("testapp1");

        compare( getSelectedText(tabBar()), "First Tab" );

        select( "Cow Tab", tabBar() );

        compare( getSelectedText(tabBar()), "Cow Tab" );

        select( "First Tab", tabBar() );

        compare( getSelectedText(tabBar()), "First Tab" );
    },

/*
    \req QTOPIA-78

    \groups
    select() things from a combobox
*/
    select_combobox: function() {
        startApplication("testapp1");

        select( "Cow Tab", tabBar() );

        select( "Woof", "Cow Goes?" );
        compare( getText("CowStatus"), "Cow Goes? changed" );
        compare( getSelectedText("Cow Goes?"), "Woof" );

        /* To clear 'Status' */
        select( "CowButton0" );
        compare( getText("CowStatus"), "CowButton0 clicked" );

        select( "Moo", "Cow Goes?" );
        compare( getText("CowStatus"), "Cow Goes? changed" );
        compare( getSelectedText("Cow Goes?"), "Moo" );
    },

/*
    \req QTOPIA-78

    \groups
    select() things from the launcher menu
*/
    select_launcherMenu: function() {
        if (mousePreferred()) {
            setSetting( "Trolltech", "qpe", "Appearance", "DecorationTheme", "finxi/decorationrc" );
            setSetting( "Trolltech", "qpe", "Appearance", "Style", "QThumbStyle" );
            setSetting( "Trolltech", "qpe", "Appearance", "Theme", "finxi.conf" );
            restartQtopia();
        }

        gotoHome();
        select( "Clock", launcherMenu() );
        waitForCurrentApplication( "Clock" );

        gotoHome();
        select( "Calendar", launcherMenu() );
        waitForCurrentApplication( "Calendar" );

        gotoHome();
        select( "Settings/Language", launcherMenu() );
        waitForCurrentApplication( "Language" );
    },

/*
    \req QTOPIA-78

    \groups
    Implicitly scroll vertically in a full-screen scroll area
*/
    scroll_scrollarea_vertical: function() {
        startApplication("testapp1");

        select( "Cow Tab", tabBar() );

        select( "CowButton0" );
        compare( getText("CowStatus"), "CowButton0 clicked" );

        /* Scroll down to the bottom */
        select( "CowButton15" );
        compare( getText("CowStatus"), "CowButton15 clicked" );

        /* Scroll up to the top */
        select( "CowButton0" );
        compare( getText("CowStatus"), "CowButton0 clicked" );
    },

/*
    \req QTOPIA-78

    \groups
    Scroll vertically in a combobox.
*/
    scroll_combobox: function() {
        startApplication("testapp1");

        select( "Cow Tab", tabBar() );

        select( "Woof", "Cow Goes?" );
        compare( getText("CowStatus"), "Cow Goes? changed" );
        compare( getSelectedText("Cow Goes?"), "Woof" );

        /* This item is the last one in the combobox */
        select( "Choo choo!", "Cow Goes?" );
        compare( getText("CowStatus"), "Cow Goes? changed" );
        compare( getSelectedText("Cow Goes?"), "Choo choo!" );
    },

/*
    \req QTOPIA-78

    \groups
    Toggle a checkbox on or off
*/
    setChecked_checkbox: function() {
        startApplication("testapp1");

        select( "Awesome Tab", tabBar() );

        select( "Clear" );
        compare( getText("Status"), "'Clear' clicked" );

        setChecked( true, "Checkbox" );
        compare( getText("Status"), "'Checkbox' clicked" );
        verify( isChecked("Checkbox") );

        select( "Clear" );
        compare( getText("Status"), "'Clear' clicked" );

        setChecked( false, "Checkbox" );
        compare( getText("Status"), "'Checkbox' clicked" );
        verify( !isChecked("Checkbox") );

        select( "Clear" );
        compare( getText("Status"), "'Clear' clicked" );

        setChecked( true, "Checkbox" );
        compare( getText("Status"), "'Checkbox' clicked" );
        verify( isChecked("Checkbox") );
    },

/*
    \req QTOPIA-78

    \groups
    select() with callAccept() and callHangup()
*/
    select_calls: function() {
        if (this.tm == undefined) {
            this.tm = testModem();
            verify( this.tm != undefined );
            //setDemoMode(true);
        }
        this.tm.startIncomingCall("1234");

        /* Wait for the incoming call to pop up the "Calls" screen */
        // FIXME: need a less fragile way to determine we are on the calls screen.
        waitFor() { return getList(optionsMenu()).contains("Answer"); }

        var list = getList();
        verify(list.join(",").contains("1234"), "List is: " + list);

        /* Select "hangup"; we should be asked to view missed calls */
        expectMessageBox("Missed Call", "Do you wish to view missed calls?", "Yes") {
            select( callHangup() );
        }
        /* We should no longer have the option to answer. */
        verify( !getList(optionsMenu()).contains("Answer") );
        /* FIXME: QSystemTestModem should know when the call is hung up. */

        /* Back out of "missed calls" */
        select( "Back", softMenu() );

        /* Wait for phonesim to definitely realise call is gone */
        wait(3000);
        this.tm.startIncomingCall("1234");
        wait(3000);
        waitFor() { return getList(optionsMenu()).contains("Answer"); }

        select( callAccept() );
        wait(3000);

        /* Should no longer have the option to answer... */
        var list = getList(optionsMenu());
        verify( !list.contains("Answer"), "Options menu unexpectedly contains 'Answer': " + list );

        /* ...but we should have the option to end. */
        verify( list.contains("End"), "Options menu doesn't contain 'End': " + list );
        select( callHangup() );
        wait(3000);

        /* We ended it, so we should no longer have the option to end. */
        list = getList(optionsMenu());
        verify( !list.contains("End"), "Options menu unexpectedly contains 'End': " + list );
    },

/*
    select() an item that results in application closing
*/
    select_close: function() {
        startApplication("testapp1");
        expectFail("232557");
        select("Close");
    }

} // end of test

