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

include( "notes.js" );
include("content.js");

//TESTED_COMPONENT=PIM: Text Notes (18708)

testcase = {

    initTestCase: function()
    {
        waitForQtopiaStart();
        if ( runsOnDevice() ) setVisibleResponseTime( 10000 );
        Notes.initNotes();
    },

    cleanupTestCase: function()
    {
    },

    init: function()
    {
        gotoHome();
        startApplication( "Notes" );
    },

    cleanup: function()
    {
        Notes.deleteNotes();
    },

/*
    \req QTOPIA-1732
    \groups
*/
    browsing_notes_data:
    {
        browsenotes:["Text", "browsenotetest"]
    },

    browsing_notes: function(text, name)
    {
        Notes.createNote( name, text );

        // This wait can be replaced with waitFor() when the expected failure is removed.
        wait(2000);
        var list = getList();
        if (!list.contains(name)) {
            expectFail("Bug 230627: renaming a note sometimes fails to update notes list");
        }
        verify(list.contains(name));
    },

/*
    \req QTOPIA-1732
    \groups
*/
    browsing_notes_by_category_data:
    {
        browsenotes:["Text", "browsecategory", "Personal"]
    },

    browsing_notes_by_category: function(text, name, category)
    {
        Notes.createNote( name, text, category );
    },

/*
    \req QTOPIA-1733
    \groups Acceptance
*/
    creating_a_new_text_file_data:
    {
        notestest:["Text"]
    },

    creating_a_new_text_file: function(text)
    {
        Notes.deleteExistingNote(text);
        select( "New" );
        enter( text );
        waitForTitle( "Properties" );
        compare( getText("Name"), text );
        select( "Back", softMenu() );
        waitForTitle( "Notes" );
        select( text );
        compare( getText(), text );
        select( "Cancel", optionsMenu() );
    },

/*
    \req QTOPIA-1734
    \groups
*/
    editing_a_text_file_data:{
        editnotestest:["The quick brown",
                        "The quick brown fox jumped over the lazy dogs back",
                        4, "left shoulder",
                        "The quick brown fox jumped over the lazy dogs left shoulder"]
    },

    editing_a_text_file: function(title, text, remove, newText, editedText)
    {
        Notes.deleteExistingNote(title);
        // create text
        select("New");
        compare(getText(), "");
        enter( text, undefined, NoCommit );
        select( "Accept", softMenu() );

        waitForTitle( "Properties" );
        waitFor() { return ( getList(softMenu()).contains("Back") ); }
        select( "Back", softMenu() );
        waitForTitle( "Notes" );
        select(title); //"The quick brown");

        // Move cursor to end of text
        mousePress(120, 160);
        wait(1000);
        for( var i=0; i<remove; i++ )
        {
            select("Delete", softMenu());
        }
        enter( newText , undefined,  NoCommit );
        select( "Accept", softMenu() );
        select(title);
        compare(getText(), editedText);
        select("Accept", softMenu());

        waitFor() { return ( getList(softMenu()).contains("Back") ); }
        select( "Back", softMenu() );
    },

/*
    \req QTOPIA-1738
    \groups
*/
    searching_text: function()
    {
        Notes.deleteExistingNote("The dog sat on the");
        // Preconditions - a text document with repeated words
        select("New");

        compare(getText(), "");
        enter( "The dog sat on the mat, ", "", NoCommit );
        enter( "the cat sat on the rug, ", "", NoCommit );
        enter( "the flea sat on the dog ", "", NoCommit );

        select( "Accept", softMenu() );
        waitForTitle( "Properties" );
        waitFor() { return ( getList(softMenu()).contains("Back") ); }
        select( "Back", softMenu() );
        waitForTitle( "Notes" );
        select("The dog sat on the");

        // Find non-existant word
        select( "Find", optionsMenu() );

        notFound = "z4ga7hs"
        enter( notFound, "", NoCommit );

        verifyImage( "No_Text_Found", "", "Text find screen - No Text Found" );
        for( var i=0; i<notFound.length; i++ )
        {
            select("Delete", softMenu());
        }

        select("Find", optionsMenu());
        waitFor() { return ( getList(softMenu()).contains("Accept") ); }
        select("Accept", softMenu());

        select("The dog sat on the");

        // Find existing word
        select( "Find", optionsMenu() );

        enter( "the", undefined, NoCommit );
        verifyImage( "Text_Found_Pixmap1", "", "Search screens (1st 'the')" );
        select("Next", softMenu());
        verifyImage( "Text_Found_Pixmap2", "", "Search screens (2nd 'the')" );
        select("Next", softMenu());
        verifyImage( "Text_Found_Pixmap3", "", "Search screens (3rd 'the')" );
        select("Next", softMenu());
        verifyImage( "Text_Found_Pixmap4", "", "Search screens (4th 'the')" );
        select("Next", softMenu());
        verifyImage( "Text_Found_Pixmap5", "", "Search screens (5th 'the')" );
        select("Next", softMenu());
        verifyImage( "Text_Found_Pixmap6", "", "Search screens (6th 'the')" );
        select("Next", softMenu());
        verifyImage( "No_More_Text_Found_Pixmap1", "", "Search screens (reached end)" );
    },

/*
    \req QTOPIA-1740
    \groups
*/
    wrapping_lines: function()
    {
        select("New");
        enter( "It must in all truth be confessed, things as yet had gone on well, and I should have acted in bad taste to have complained.", "", NoCommit );

        lineWrap = getProperty(focusWidget(), "lineWrapMode");
        if (lineWrap) {
            // switch off line wrap
            select("Settings/Wrap Lines", optionsMenu());
        }

        verifyImage( "Word_Wrap_Off_LHS", "", "Screenshots of LHS of text with word wrap off" );
        keyClick( Qt.Key_Right);
        verifyImage( "Word_Wrap_Off_RHS", "", "Screenshots of RHS of text with word wrap off" );

        // switch on line wrap
        select("Settings/Wrap Lines", optionsMenu());
        verifyImage( "Word_Wrap_On", "", "Screenshots of text with word wrap on" );

        enter( " rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr", "", NoCommit );
        verifyImage( "Word_Wrap_rrrrr", "", "Screenshots of text with word wrap on and line of 'rrrr'" );
        keyClick( Qt.Key_Up );
        enter( " ", "", NoCommit );
        verifyImage( "Word_Wrap_On_With_CR", "", "Screenshots of text with word wrap on and line of 'rrrr' and a CR in middle" );

        select("Accept", softMenu());
        waitFor() { return ( getList(softMenu()).contains("Back") ); }
        select( "Back", softMenu() );
        waitForTitle("Notes");
        waitFor() { return ( getList(softMenu()).contains("Back") ); }
        select( "Back", softMenu() );
    },

/*
    \req QTOPIA-1739
    \groups
*/
    changing_the_font_size: function()
    {
        Notes.deleteExistingNote("Space the final");
        select( "New" );
        enter( "Space the final frontier" );

        waitForTitle( "Properties" );
        waitFor() { return ( getList(softMenu()).contains("Back") ); }
        select( "Back", softMenu() );
        waitForTitle( "Notes" );

        select("Space the final");


        var zoomLevelAbsolute = 0;
        var zoomLevel = 0;
        var failure = null;

        if (runsOnDevice()) {
            expectFail("BUG 188762: Automated snapshot file naming is wrong");
        }

        // If we fail, make sure we return back to the original zoom level.
        try {
            // Zoom all the way out...
            opt = getList(optionsMenu());
            while (opt.contains("Settings/Zoom Out")) {
                --zoomLevel;
                select("Settings/Zoom Out", optionsMenu());
                opt = getList(optionsMenu());
            }

            verifyImage( "Zoom_Level_" + zoomLevelAbsolute, "",  "Zoom Level " + zoomLevelAbsolute );

            // Zoom in...
            while (opt.contains("Settings/Zoom In")) {
                select("Settings/Zoom In", optionsMenu());
                ++zoomLevelAbsolute;
                ++zoomLevel;
                verifyImage( "Zoom_Level_" + zoomLevelAbsolute, "",  "Zoom Level " + zoomLevelAbsolute );
                opt = getList(optionsMenu());
            }
        } catch (e) {
            failure = e;
        }

        // Zoom back out to the original level...
        while (zoomLevel != 0)
        {
            if (zoomLevel > 0) {
                select("Settings/Zoom Out", optionsMenu());
                --zoomLevel;
            } else {
                select("Settings/Zoom In", optionsMenu());
                ++zoomLevel;
            }
        }

        // Rethrow the failure, if any.
        if (failure != null) throw failure;

        waitFor() { return ( getList(softMenu()).contains("Accept") ); }
        select("Accept", softMenu());
    },

/*
    \req QTOPIA-1733
    \groups
*/
    document_format_consistency: function()
    {
        Notes.deleteExistingNote("The dog sat on the");
        // Preconditions - a text document with repeated words
        select("New");

        compare(getText(), "");
        enter( "The dog sat on the mat, ", "", NoCommit );
        enter( "the cat sat on the rug, ", "", NoCommit );
        enter( "the flea sat on the dog ", "", NoCommit );

        // Need to hide the cursor otherwise the images may not match...
        cursorWidth = getProperty(focusWidget(), "cursorWidth");
        setProperty(focusWidget(), "cursorWidth", 0);

        // Take a snapshot of the original document when first created
        verifyImage( "Document_Layout", "", "Snapshot of document immediatly after creation" );

        select( "Accept", softMenu() );
        waitForTitle( "Properties" );
        select( "Back", softMenu() );
        waitForTitle( "Notes" );
        select("The dog sat on the");

        setProperty(focusWidget(), "cursorWidth", 0);

        // Compare document to first snapshot to confirm layout is the same
        verifyImage( "Document_Layout", "", "Document format when re-opened" );

        // Restore the cursor
        setProperty(focusWidget(), "cursorWidth", cursorWidth);

        select( "Accept", softMenu() );
    },

/*
    \req QTOPIA-1735
    \groups
*/
    discard_changes: function()
    {
        Notes.deleteExistingNote( "Discard test" );

        select("New");

        compare(getText(), "");
        enter( "Discard test text", "", NoCommit );

        select( "Cancel", optionsMenu() );
        waitForTitle( "Notes" );
        verify(!getList().contains("Discard test"));

        select("New");

        compare(getText(), "");
        enter( "Discard test text" );

        waitForTitle( "Properties" );
        select( "Back", softMenu() );
        waitForTitle( "Notes" );
        verify(getList().contains("Discard test"));
    },

/*
    \req QTOPIA-1736
    \groups Acceptance
*/
    delete_note: function()
    {
        Notes.deleteExistingNote("Delete me");

        select("New");

        compare(getText(), "");
        enter( "Delete me please" );

        waitForTitle( "Properties" );
        select( "Back", softMenu() );
        waitForTitle( "Notes" );

        verify(getList().contains("Delete me"));
        Notes.deleteExistingNote("Delete me");
        verify(!getList().contains("Delete me"));
    },

/*
    \req QTOPIA-1737
    \groups
*/
    edit_properties: function()
    {
        var name1 = "Properties Test";
        var name2 = "Test Properties";
        Notes.deleteExistingNote( name1 );
        Notes.deleteExistingNote( name2 );

        select("New");

        compare(getText(), "");
        enter( "Properties Test one two three four five" );

        waitForTitle( "Properties" );
        enter( name1, "Name" );
        select( "Back", softMenu() );
        waitForTitle( "Notes" );
        verify( getList().contains( name1 ) );

        // Highlight by selecting the note then selecting Accept
        select( name1 );
        waitFor() { return( getList(softMenu()).contains("Accept") ); }
        select( "Accept", softMenu() );
        compare( getSelectedText(), name1 );

        // Rename
        select( "Properties...", optionsMenu() );
        waitForTitle( "Properties" );
        enter( name2, "Name" );
        select( "Back", softMenu() );
        waitForTitle( "Notes" );

        // This wait can be replaced with waitFor() when the expected failure is removed.
        wait(2000);
        var list = getList();
        if (!list.contains(name2)) {
            expectFail("Bug 230627: renaming a note sometimes fails to update notes list");
        }
        verify(list.contains(name2));

        // Highlight by selecting the note then selecting Accept
        select( name2 );
        waitFor() { return( getList(softMenu()).contains("Accept") ); }
        select( "Accept", softMenu() );
        compare( getSelectedText(), name2 );

        // Category
        select( "Properties...", optionsMenu() );
        waitForTitle( "Properties" );
        select( "Category" );
        waitForTitle( "Select Category" );
        select( "Personal" );
        select( "Back", softMenu() );
        waitForTitle( "Properties" );
        select( "Back", softMenu() );
        waitForTitle( "Notes" );

        // Check that it shows up if Personal category selected
        select( "View Category...", optionsMenu() );
        waitForTitle( "View Category" );
        select( "Personal" );
        select( "Back", softMenu() );
        waitForTitle( "Notes" );
        verify( getList().contains( name2 ) );

        // Check that it doesn't show up with Unfiled selected
        select( "View Category...", optionsMenu() );
        waitForTitle( "View Category" );
        select( "All" ); // unselects others
        select( "Unfiled" );
        select( "Back", softMenu() );
        waitForTitle( "Notes" );
        verify( !getList().contains( name2 ) );

        // Check that it's displayed with All selected
        select( "View Category...", optionsMenu() );
        waitForTitle( "View Category" );
        select( "All" );
        select( "Back", softMenu() );
        waitForTitle( "Notes" );
        verify( getList().contains( name2 ) );

        // TODO: Change storage location (on phone only)
    },

/*
    \req QTOPIA-7527
    \groups
*/
    file_size_limit: function()
    {
        // For this test we will need to return to the home screen first before copying and rescanning
        gotoHome();

        putFile( baseDataPath() + "/file_size_limit/262144.txt", documentsPath() + "text/plain/262144.txt" );
        putFile( baseDataPath() + "/file_size_limit/262145.txt", documentsPath() + "text/plain/262145.txt" );

        // rescan the documents
        Content.rescan();

        // now reopen Notes
        startApplication( "Notes" );

        // this one should open no problem
        select( "262144" );
        waitFor() { return( getList(softMenu()).contains("Accept") ); }
        select( "Accept", softMenu() );

        // this one is too big
        expectMessageBox( "File Too Large", "This file is too large for Notes to open.", "OK" ) {
            select( "262145" );
        }
    },

/*
    \req
    \groups Performance
*/
    perf_startup: function()
    {
        Performance.testStartupTime("Applications/Notes");
    }

} // end of test

