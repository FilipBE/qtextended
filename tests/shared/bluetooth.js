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

Bluetooth = {

/*
    setBluetooth - handles setting the various valus of the bluetooth UI - can be called in two ways: predetermined arguments and named arguments
        predetermined arguments - setBluetooth( state, visiblity, timeout, name ) - with null used when an argument is not to change but a later one is
                                - setBluetooth( true, null, null, "foobar") - set bluetooth on + changing name to string supplied.
        named arguments - setBluetooth( { state : value, visiblity : value, timeout : value, name : value } ) - where only argument(s) to change are named and given value
                        - setBluetooth( { state : true, name : "foobar" } )

    state :         Bluetooth is enabled/disabled
    visibility :    Device is visible to others
    timeout :       Time that device is visible to others (where 0 is 'Off')
    name :          Human readable identifier as seen by other devices
*/
    setBluetooth: function( state, visiblity, timeout, name )
    {
        // argument handling starts
        // psudeo overloading - if detects presence of 'named arguments'
        if( arguments.length == 1 && typeof arguments[0] == 'object' && arguments[0] != null ) {
            visiblity = 'visiblity' in arguments[0]? arguments[0].visiblity : null;
            timeout = 'timeout' in arguments[0]? arguments[0].timeout : null;
            name = 'name' in arguments[0]? arguments[0].name : null;
            state = 'state' in arguments[0]? arguments[0].state : null; // as argument[0] and state are one and the same it must be last to get it's value
        }
        // check the type of the data, make null any that don't match
        state = typeof state == 'boolean'? state : null;
        visiblity = typeof visiblity == 'boolean'? visiblity : null;
        timeout = typeof timeout == 'number'? timeout : null;
        name = typeof name == 'string'? name : null;
        // argument handling finishs

        startApplication( "Bluetooth" );

        var enableState = isChecked( "Enable Bluetooth" );
        var visiblityState = isChecked( "Enable Bluetooth/Visible to others" );

        if ( visiblity != null || timeout != null || name != null ) // turn bluetooth on (if not already) as we need to do some changes
            setChecked( true, "Enable Bluetooth" );

        if ( isChecked( "Enable Bluetooth" ) ) {    // as the state of the check box is tied to the device it controls there maybe a delay for enabling dependent widgets
            waitFor() {
                return isEnabled( "Enable Bluetooth/Visible to others" );
            }
        }

        if ( timeout != null ) {
            setChecked( true, "Enable Bluetooth/Visible to others" );
            enter( timeout ,"qpe/Bluetooth:Enable Bluetooth/Timeout:");
        }

        switch( visiblity ) {
            case true:  enableState=true;   // visiblity is only on when bluetooth is on - therefore implying caller wants bluetooth on
            case false: visiblityState=visiblity;
                        break;
            default:    break;  // don't do anything if we get anything else
        }

        setChecked( visiblityState, "Enable Bluetooth/Visible to others" ); // set visiblity based on visiblityState

        if ( name != null )
            enter( name, "qpe/Bluetooth:Enable Bluetooth/Name:" );  // FIXME: task 209618

        switch( state ) {
            case true:  // this is correct - switch is being used so only true and false have the same thing code run, anything else will have no effect
            case false: enableState=state;
                        break;
            default:    break;  // do nothing
        }

        setChecked( enableState, "Enable Bluetooth" );    // set enable based on original setting or ar

        compare( isChecked( "Enable Bluetooth" ), enableState );    // check we are leaving in the state we entered or was changed by args

        if ( enableState )  // visiblity is only checked when bluetooth is enabled so don't check if we aren't on
            compare( isChecked( "Enable Bluetooth/Visible to others"), visiblityState );

        select( "Back", softMenu() );
        waitForTitle( "Settings" );
        select( "Back", softMenu() );
        waitForTitle( "Settings" );
        select( "Back", softMenu() );
    },

    devicePluggedIn: function()
    {
        var reply = runProcess( "hcitool", ["dev"], "" );
        return( reply.contains("hci0") );
    },

    deviceRecognised: function()
    {
        var bluetoothDevice;
        expectFail( "BUG-233897" );
        waitFor( 15000 ){
            bluetoothDevice = getValueSpace("/Hardware/Devices/hci0/Path").toString();
            return( bluetoothDevice.toString().contains("hci0") );
        }
    }

};

