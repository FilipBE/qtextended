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

TaskManager = {

    deleteFavorites: function()
    {
        gotoHome();
        waitFor( 5000 ){
            return( getText(softMenu()).contains("Menu") );
        }
        select( "Settings/Task Manager", launcherMenu() );
        //startApplication( "Task Manager", 5000, NoFlag );
        waitForTitle( "Task Manager" );
        select( "Favorites", tabBar() );
        var favlist = getList();
        var retries = favlist.length;
        while( favlist.length > 0 && retries >= 0 ){
            select( "Delete", optionsMenu() );
            --retries;
            favlist = getList();
        }
    },

    replaceDefault: function()
    {
        TaskManager.deleteFavorites();
        startApplication( "Speed Dial" );
        select( "Add", optionsMenu() );
        waitForTitle( "Select Service", 10000 );
        select( "Call Voicemail" );
        waitForTitle( "Set Speed Dial" );
        select( "Select", softMenu() );
        waitForTitle( "Speed Dial" );
    }
}
