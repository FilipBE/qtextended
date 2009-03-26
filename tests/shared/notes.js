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

Notes = {
    createNote: function( name, text, category )
    {
        startApplication( "Notes" );
        select( "New" );
        enter( text, undefined, NoCommit );
        select( "Accept", softMenu() );
        waitForTitle( "Properties" );
        enter( name, "Name" );
        if( category != 0 && category != undefined && category != "Unfiled" ){
            select( "Category" );
            if( !getList().contains(category) )
                createNotesCategory(category);
            select( category );
            select( "Back", softMenu() );
        }
        select( "Back", softMenu() );
        waitForTitle("Notes");
    },

    createNotesCategory: function( name, text, category )
    {
        /*if( currentTitle() != "Select Category" ){
            startApplication( "Notes" );
            select( "Show Category", optionsMenu() )
        }*/
        select( "New Category", optionsMenu() );
    },

    initNotes: function()
    {
        gotoHome();
        startApplication( "Notes" );

        select( "View Category...", optionsMenu() );
        waitForTitle( "View Category" );
        if (getText().contains("All"))
            select("All") ;
        waitFor() { return ( getList(softMenu()).contains("Back") ); }
        select( "Back", softMenu() );

        initNotes = getText().split("\n");
    },

    deleteNotes: function()
    {
        gotoHome();
        startApplication( "Notes" );

        select( "View Category...", optionsMenu() );
        waitForTitle( "View Category" );
        if (getText().contains("All"))
            select("All") ;
        waitFor() { return ( getList(softMenu()).contains("Back") ); }
        select("Back", softMenu());

        lt = getText();
        toDelete = [];
        notes = lt.split("\n");
        var i=0;
        var j=0;
        while (i < notes.length) {
            if (notes[i] != initNotes[j]) {
                toDelete.push(notes[i]);
            } else {
                ++j;
            }
            ++i;
        }

        while (toDelete.length > 0) {
            noteToDelete = toDelete.shift();
            do {
                keyClick( Qt.Key_Up );
            } while (getSelectedText() != noteToDelete);

            expectMessageBox( "Delete", "<qt>Are you sure you want to delete: " + noteToDelete + "?</qt>", "Yes") {
                select( "Delete", optionsMenu() );
            }
        }

        waitFor() { return ( getList(softMenu()).contains("Back") ); }
        select( "Back", softMenu() );
    },

    deleteExistingNote: function( noteToDelete )
    {
        while ( getText().contains(noteToDelete) ) {
            do {
                keyClick( Qt.Key_Up );
            } while (getSelectedText() != noteToDelete);

            expectMessageBox( "Delete", "<qt>Are you sure you want to delete: " + noteToDelete + "?</qt>", "Yes") {
                select( "Delete", optionsMenu() );
            }
        }

        var i = 0;
        tempNotes = [];
        for (var i=0; i < initNotes.length; ++i) {
            if (initNotes[i] != noteToDelete)
                tempNotes.push(initNotes[i]);
        }

        initNotes = tempNotes;
    }
}
