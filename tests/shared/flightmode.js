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
include("messages.js");

FlightMode = {

/*
    \usecase setFlightMode() sets Airplane Safe Mode in Qt Extended on system under and checks feed back.
    argument: state - true is on, false is off.
*/
    setFlightMode: function( state )
    {
        switch( state ) {
            case true:  backgroundAndGotoHome();
                        select( "Profile...", optionsMenu() );
                        waitForTitle( "Profiles", 10000 );
                        select( "Airplane Safe Mode", optionsMenu() );  // FIXME: work around until task 207405 is implemented
                        waitForTitle( "Profiles", 10000 );
                        select( "Back", softMenu() );
                        waitForCurrentApplication("qpe", 10000);
                        verify( FlightMode.getFlightMode() );
                        return true;
                        break;
            case false: backgroundAndGotoHome();
                        verify( FlightMode.getFlightMode() );
                        select( "Profile...", optionsMenu() );
                        waitForTitle( "Profiles", 10000 );
                        var labels = getLabels();
                        if( labels.contains( "Airplane" ) && labels.contains( "Airplane Safe Mode" ) ) {   // take care of the case we might be Airplane Profile - which 'locks out' the menu item
                            expectMessageBox( "Airplane Mode", "<qt>Do you wish to stay in Airplane Safe Mode?</qt>", "No" ) {
                                select( "General" );
                            };
                        }
                        else {
                            select( "Airplane Safe Mode", optionsMenu() );  // FIXME: work around until task 207405 is implemented
                            waitForTitle( "Profiles", 10000 );
                            select( "Back", softMenu() );
                        }
                        waitForCurrentApplication("qpe", 10000);
                        verify( !FlightMode.getFlightMode() );
                        return false;
                        break;
            default:    fail( "setFlightMode() requires either: 'true' or 'false' to be passed to it." );
                        break;
        };
    },

/*
    \usecase getFlightMode() returns true when the value ModemStatus in valuespace indicate modem is in flightmode
*/
    getFlightMode: function()
    {
        // the following two lines give the required, correct, result - whilst: return waitForValueSpace( "Telephony/Status/ModemStatus", "AerialOff", 2000 );
        // appears not to do so.    FIXME: task 208706
        waitForValueSpace( "Telephony/Status/ModemStatus", "", 2000 );
        return (getValueSpace("Telephony/Status/ModemStatus") == "AerialOff");
        // return waitForValueSpace( "Telephony/Status/ModemStatus", "AerialOff", 2000 );   // FIXME: left for demonstration of task 208706
    },

/*
    \usecase setAirplaneProfile() sets/unsets the Airplane Profile - true sets Airplane profile - false sets General profile
*/
    setAirplaneProfile: function( state )
    {
        select( "Profile...", optionsMenu() );
        waitForTitle( "Profiles", 10000 );
        switch( state ) {
            case true:  select( "Airplane" );
                        waitForCurrentApplication("qpe", 10000);
                        verify( FlightMode.getFlightMode() );
                        break;
            case false: expectMessageBox("Airplane Mode","Do you wish to stay in Airplane Safe Mode?","No") {
                            select( "General" );
                        };
                        waitForCurrentApplication("qpe", 10000);
                        verify( !FlightMode.getFlightMode() );
                        break;
            default:    fail( "setAirPlaneProfile() requires either: 'true' or 'false' to be passed to it." );
                        break;
        };
    },

/*
    \usecase checkExpectation() verifies values we use for verification change when we expect
*/
    checkExpectations: function()
    {
        // check things we expect to happen occur - most important check valuespace changes, then the other things reflect it.
        select( "Profile...", optionsMenu() );
        waitForTitle( "Profiles", 10000 );
        select( "Airplane Safe Mode", optionsMenu() );  // FIXME: work around until task 207405 is implemented
        verify( waitForValueSpace( "Telephony/Status/ModemStatus", "AerialOff", 10000 ) );    // might have to wait a little on actual devices
        compare(getSetting( "$HOME/Settings/Trolltech/PhoneProfile.conf","Profiles","PlaneMode"), "true");
        waitForTitle( "Profiles", 10000 );             // need to be back at homescreen to dial
        select( "Back", softMenu() );
        waitForCurrentApplication("qpe", 10000);
        waitFor(10000, 20, "Airplane safe mode message did not appear.") {
            return getText().contains("Airplane safe mode");    // checks we are in Airplane safe mode & also detects we are back to homescreen
        };
        expectMessageBox("No GSM/VoIP Network","No phone call is possible.","Back") {
            Telephony.dial( "1194" );
        };
        // now do the checks for not being in flightmode
        select( "Profile...", optionsMenu() );
        waitForTitle( "Profiles", 10000 );
        select( "Airplane Safe Mode", optionsMenu() );  // FIXME: work around until task 207405 is implemented
        verify( waitForValueSpace( "Telephony/Status/ModemStatus", "Ready", 10000 ) );      // might have to wait a little on actual devices
        compare(getSetting( "$HOME/Settings/Trolltech/PhoneProfile.conf","Profiles","PlaneMode"), "false");
        waitForTitle( "Profiles", 10000 );             // need to be back at homescreen to dial
        select( "Back", softMenu() );
        waitForCurrentApplication("qpe", 10000);
        waitFor(10000, 20, "Airplane safe mode message did appear.") {
                return !getText().contains("Airplane safe mode");    // checks we are in Airplane safe mode & also detects we are back to homescreen
        };
        Telephony.dial( "1194" );
        Telephony.waitForCallState( "1", "Connected");
        select( "End", optionsMenu() );
        waitFor(10000, 20 ) {
            return !currentTitle().contains("Calls");
        };
    },

/*
    \usecase attempts to unset Airplane Safe Mode by item in Options menu whilst in the Airplane Profile
*/
    attemptUnsetByOptionUnderProfile: function()
    {
        select( "Profile...", optionsMenu() );  // attempt to unselect Airplane Safe Mode from Options menu
        waitForTitle( "Profiles", 10000 );
        var labels = getLabels();
        if( labels.contains( "Airplane" ) && labels.contains( "Airplane Safe Mode" ) ) {   // take care of the case we might be Airplane Profile - which 'locks out' the menu item
            expectMessageBox("Airplane Mode","Cannot turn off Airplane Safe Mode when the profile <b>Airplane</b> is active.","OK") {
                select( "Airplane Safe Mode", optionsMenu() );
            };
            waitForTitle( "Profiles", 10000 );
            select( "Back", softMenu() );
            waitForCurrentApplication("qpe", 10000);
            verify( FlightMode.getFlightMode() );
        } else {
            fail( "Error: Labels checked appear to indicate we are not in the required mode for this function." );
        }
    },

/*
    \usecase wrapper for Messages.createEmailAccount() ensures we play nice by backgrounding apps, and once finished exiting Messages entirely.
*/
    createEmailAccount: function(o)
    {
        backgroundAndGotoHome();
        Messages.createEmailAccount(o);
        select( "Back", softMenu() );
        waitForTitle( "Messages", 10000 );
        select( "Back", softMenu() );
        waitForCurrentApplication("qpe", 10000);
    },

/*
    \usecase wrapper for Messages.removeEmailAccount() - which was implemented emulating the behaviour of Messages.createEmailAccount()
*/
    removeEmailAccount: function(o)
    {
        backgroundAndGotoHome();
        Messages.removeEmailAccount(o);
        waitForTitle( "Messages", 10000 );
        select( "Back", softMenu() );
        waitForCurrentApplication("qpe", 10000);
    }

};

