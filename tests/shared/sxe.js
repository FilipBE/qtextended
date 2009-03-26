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

SXE = {

    addServer: function( serverName, url )
    {
        startApplication( "Software Packages" );
        select( "Downloads", tabBar() );
        if( getList(optionsMenu()).contains("Connect/"+serverName) ){
            print( "Server already added" );
        }else{
            select( "Edit servers", optionsMenu() );
            waitForTitle( "Edit servers" );
            select( "New", optionsMenu() );
            enter( serverName, "packagemanager:Name:" );
            enter( url, "packagemanager:URL:" );
            select( "Accept", softMenu() );
            select( "Back", softMenu() );
        }
    },

    removeServer: function( serverName )
    {
        startApplication( "Software Packages" );
        select( "Downloads", tabBar() );
        select( "Edit servers", optionsMenu() );
        if( !getList().contains(serverName) ){
            select( serverName );
            select( "Remove", softMenu() );
        }
        select( "Back", softMenu() );
    },

    installPackage: function( packageName, serverName )
    {
        startApplication( "Software Packages" );
        if( getList().contains(serverName) ){
            print( "Package already installed" );
            return;
        }
        select( "Downloads", tabBar() );
        select( "Connect/"+serverName, optionsMenu() );
        waitForTitle( "Downloading" );
        waitForTitle( "Package Manager", 10000 );
        select( packageName );
        waitForTitle( packageName );
        select( "Install", optionsMenu() );
        expectMessageBox( "Successful Installation", "Package successfully installed", "OK" ){
            select( "Confirm", optionsMenu() ); // Task 206582 Inconsistent UI for Confirm/Cancel
            waitForTitle( "Downloading" );
            waitForTitle( "Successful Installation", 10000 );
        }
        select( "Back", softMenu() );
    }

};

