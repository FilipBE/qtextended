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

Telephony = {

/*
    Modem wait states
*/
    modemWaitPin: "WaitingSIMPin",
    modemWaitPuk: "WaitingSIMPuk",
    modemReady:   "Ready",
    simReady:     "Ready",
    simPin:       "SIM PIN",
    simPuk:       "SIM PUK",

/*
    Service codes
*/
    changePin:  "*04*",
    changePin2: "*042*",

/*
    Enter a **<sc>* code prefix
*/
    enterServiceCode: function( code )
    {
        keyClick( Qt.Key_Asterisk );
        wait(1500);
        compare( getText(), "*" );
        enter( code );
    },

/*
    Dial a number. If VoIP is active, select GSM
*/
    dial: function( number, calltype )
    {
        gotoHome();
        enter( number );
        waitForTitle( "Quick Dial" );
        compare( getText(), number );
        if (!mousePreferred()) {
            waitFor (1000) {
                return getText(softMenu()).contains("Dial");
            }
        }

        if( isBuildOption("QTOPIA_VOIP") && Telephony.isVoipActive() ){
            expectMessageBox( "", "Which type of call do you wish to make?", "" ){
                if (mousePreferred())
                    select( "Call" );
                else
                    select( "Dial", softMenu() );
            }
            select( calltype );
            select( "Select", softMenu() );
        }else{
            if (mousePreferred())
                select( "Call" );
            else
                select( "Dial", softMenu() );
        }
    },

/*
    Enter a number into the dialer, but do not dial
*/
    enter: function( number )
    {
        gotoHome();
        enter( number, "", NoCommit );
        waitForTitle( "Quick Dial", 5000 );
        compare( getText(), number );
    },

/*
    Create an incoming call, and wait for the interface to allow user action
*/
    incomingCall: function( number )
    {
        testModem().startIncomingCall( number );
        if( number == "" )
            number == "Unknown";
        waitForTitle( "Calls", 5000 );
        Telephony.waitForCallState("", "Incoming");
        waitFor (3000) {
            return getList(softMenu()).contains("Answer");
        }
    },

/*
    Answer the incoming call <number>
*/
    answerIncoming: function( number )
    {
        if( number == "" )
            number == "Unknown";
        verify( Telephony.waitForCallState(number, "Incoming") );
        wait( 1500 ); // FIXME: Bug-202559
        select( "Answer", softMenu() );
        verify( Telephony.waitForCallState(number, "Connected") );
    },

/*
    Wait up to 10 seconds for the <number> call status to indicate <state>
*/
    waitForCallState: function( number, state )
    {
        var i;
        var j;
        var list;
        var stateFound = false;
        waitForTitle("Calls");
        try {
            waitFor () {
                list = getList();
                // Find number,state in list
                for( j = 0; j < list.length; j+=2 ){
                    if( list[j].contains(number) && list[j+1].contains(state) ){
                        stateFound = true;
                        break;
                    }
                }
                return stateFound;
            }
        } catch(e) {
            print("Expected to find " + number + ":" + state + " in list.");
            print("List is: " + list);
            throw(e);
        }
        return stateFound;
    },

/*
    Returns a list of active connections.
*/
    activeConnections: function()
    {
        var j;
        var list = getList();
        var ret = [];

        for( j = 0; j < list.length; j+2 ){
            if( list[j] != "" && list[j] == "Connected" ){
                print( list[j] + ": " + list[j+1] );
                ret += list[j];
            }
        }
        return list;
    },

/*
    Perform a gsm key action, which consists of a sequence of keys and pressing SEND
*/
    gsmKey: function( expectedKey )
    {
        wait( 1500 ); // FIXME: Bug-202559
        enter( expectedKey );
        waitFor (2000) {
            return (getList(softMenu()).contains("Send"));
        }
        select( "Send", softMenu() );
    },

/*
    Wait <interval> milliseconds for the modem status to indicate <modemstate>
*/
    waitForModemState: function( modemstate, interval )
    {
        waitFor( interval ){
            return (getValueSpace("Telephony/Status/ModemStatus").toString() == modemstate);
        }
        return (getValueSpace("Telephony/Status/ModemStatus").toString() == modemstate);
    },

/*
    Wait <interval> milliseconds for the modem status to be un/registered (true/false)
*/
    waitForModemRegistration: function( modemstate, interval )
    {
        var mState;
        waitFor( interval ){
            mState = getValueSpace("Telephony/Status/NetworkRegistered").toString();
            if( mState == "0" )
                mState = "false";
            else if( mState == "1" )
                mState = "true";
            return mState == modemstate;
        }
        return mState == modemstate;
    },

/*
    Determine VoIP status by checking configuration
*/
    isVoipActive: function()
    {
        return getValueSpace("Telephony/Status/VoIP/Registered") == "true";
    },

/*
    Get the entry for an extended number
*/
    getExtendedNumber: function( number, extension )
    {
        var ret = number;
        if( extension != "" )
            ret += "p" + extension;
        return ret;
    },

/*
    Get the visible text for an extended number
*/
    getVisibleExtendedNumber: function( number, extension )
    {
        var ret = number;
        if( extension != "" )
            ret += "," + extension;
        return ret;
    },

/*
    Forcibly turn on/off fixed dialing
*/
    activateFixedDialing: function( activate )
    {
        if( testModem().getVariable("FD") != activate ){
            testModem().setVariable( "FD", activate, true );
            restartQtopia();
        }
    }
};

