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

Language = {
    ensureInputLanguages: function(languages)
    {
        if (languages.count == 0) return;
        var list = getDirectoryEntries( "$QPEDIR/i18n", QDir.NoDotAndDotDot | QDir.Dirs );
        for (var i = 0; i < languages.count; ++i) {
            if ( !list.contains(languages[i]) ) {
                fail("Necessary languages not installed");
                return;
            }
        }
    },

    selectLanguage: function( language )
    {
        // TODO: Only supports English / German currently
        startApplication( "language" );
        var availableLanguages = getList();
        verify( availableLanguages.contains(language) );

        addExpectedMessageBox( "Language Change", "");
        addExpectedMessageBox( "Sprachwechsel", "");
        select( language );
        waitExpectedMessageBox(1000, false);

        // This is going to result in the language application closing and
        // Qt Extended restarting...
        expectApplicationClose(true);
        if (currentTitle() == "Language Change") select ("Yes", softMenu() );
        else if (currentTitle() == "Sprachwechsel") select ("Ja", softMenu() );

        // Give Qt Extended a bit of time to close...
        if (runsOnDevice()) {
            wait(20000);
        } else {
            wait(5000);
        }
        expectApplicationClose(false);
        waitForQtopiaStart();
    },

    useForInput: function( languages )
    {
        if( getSetting("$HOME/Settings/Trolltech/locale.conf", "Language", "InputLanguages") != languages ){
            setSetting( "$HOME/Settings/Trolltech/locale.conf", "Language", "InputLanguages", languages );
            //restartQtopia();
        }

        // TODO: Do through UI
/*
    startApplication( language );
    UserInterface.highlight(language);
    select( "Use for input", optionsMenu() );
    select( "Back", softMenu() );
*/
    }
}
