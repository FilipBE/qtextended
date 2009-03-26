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

Calendar = {

    created_events: [],

    removeAllEvents: function()
    {
        if (Calendar.created_events.length == 0) return;

        gotoHome();
        wait(5000);
        if (mousePreferred()) {
            /* FIXME implement this by simulating user input.
                * It's difficult to do so, because the "View" option doesn't appear
                * in the context menu on a mousePreferred() device, and the class
                * displaying the events is a custom widget which doesn't inherit
                * from any standard item view. */
            deletePath("$HOME/Applications");
            restartQtopia();
            testcase.initTestCase();
            startApplication( "Calendar" );
        } else {
            startApplication( "Calendar" );

            if (getLabels().contains("Snooze")) {
                print( "WARNING: had to close a Reminder dialog" );
                select( "Back", softMenu() );
            }

            for (var i = 0; i < Calendar.created_events.length; i++) {
                var DT = Calendar.created_events[i];
                if (DT == undefined) { continue; }

                Calendar.enterDateKeyClicks( DT );

                if (getLabels().contains("Snooze")) {
                    print( "WARNING: had to close a Reminder dialog" );
                    select( "Back", softMenu() );
                }

                if ( getList(softMenu()).contains("Select") ) {
                    select( "Select", softMenu() );
                }

                waitFor() { return ( getList(softMenu()).contains("View") ); }

                select( "View", softMenu() );
                waitForTitle( "Event Details" );

                addExpectedMessageBox( "Delete Event", "This appointment is part of" );
                addExpectedMessageBox( "Calendar", "Are you sure you want to delete:" );

                select( "Delete event", optionsMenu() );
                waitExpectedMessageBox(4000, false);

                if (currentTitle() == "Delete Event") select( "OK", softMenu() );
                else if (currentTitle() == "Calendar") select( "Yes", softMenu() );
                else fail( "ERROR: Unexpected title: " + currentTitle() );

                Calendar.created_events[i] = undefined;
                clearExpectedMessageBoxes();

                waitForTitle( "Calendar" );
            }
        }
        Calendar.created_events = [];
    },

    enterNewEvent: function( event )
    {
        if ( event.StartDateTime != undefined ) {
            event.StartDate = event.StartDateTime.date();
            event.StartTime = event.StartDateTime.time();
        }

        if ( event.EndDateTime != undefined ) {
            event.EndDate = event.EndDateTime.date();
            event.EndTime = event.EndDateTime.time();
        }

        waitForTitle( "Calendar" );
        select( "New event", optionsMenu() );
        waitForTitle( "New Event" );
        enter( event.Description, "Desc." );

        if ( event.Location != undefined ) enter( event.Location, "Loc.");
        if ( event.StartDate != undefined ) enter( event.StartDate, "Start/Date" );

        if ( event.AllDayEvent != undefined && event.AllDayEvent == true ) {
            setChecked( true, "All day event" );
            verify( !isVisible( "Start/Time" ) );
        } else {
            if ( event.StartTime != undefined ) enter( event.StartTime, "Start/Time" );
        }

        if ( event.EndDate != undefined ) enter( event.EndDate, "End/Date" );
        if ( event.AllDayEvent == undefined && event.EndTime != undefined ) enter( event.EndTime, "End/Time" );
        if ( event.TimeZone != undefined ) select( event.TimeZone, "T.Z." );

        if ( event.Reminder != undefined ) {
            select( event.Reminder, "Reminder" );
            if ( event.BeforeTime != undefined ) {
                select( event.BeforeTime, signature("Reminder", 1) );
                //    FIXME: The next line shows what we would like to do instead of the previous line
                //    select( BeforeTime, "At the event time" );
            } else {
                if ( event.AllDayEvent == true )
                    select( "On the day", signature("At", 1) );
                else
                    select( "At the event time", signature("Reminder", 1) );
            }
        }

        if ( event.Recurrence != undefined ) {
            select( "Recurrence", tabBar() );
            select( event.Recurrence, "Repeat" );

            if ( event.Frequency != undefined ) {
                enter( event.Frequency, "Frequency" );
            }
            if ( event.RepeatUntil != undefined ) {
                setChecked( true, "Repeat until" );
                enter( event.RepeatUntil, "Repeat until/Until" );
            }

            if ( event.Recurrence == "Weekly" && event.DaysOfWeek != undefined ) {
                for (var i=0; i<event.DaysOfWeek.length; ++i) {
                    setChecked( true, "On these days/" + event.DaysOfWeek[i] );
                }
                // The day of the week when event starts should not be enabled
                verify( !isEnabled("On these days/" + event.StartDate.toString("dddd")) )
            }
        }

        waitFor() { return ( getList(softMenu()).contains("Back") ); }
        select( "Back", softMenu() );
        Calendar.created_events.push( event.StartDate );
        waitForTitle( "Calendar" );
    },

    updateEvent: function( event )
    {
        select( "Edit event", optionsMenu() );
        waitForTitle( "Edit Event" );

        if ( getText().contains("Earlier Appointments") ) {
            select( "Earlier Appointments" );
            select( "Next", softMenu() );
        }

        Calendar.updateCurrentEvent( event );
    },

    updateCurrentEvent: function( event )
    {
        waitForTitle( "Edit Event" );
        wait(1000);

        if ( event.Description != undefined ) enter( event.Description, "Desc." );
        if ( event.Location != undefined ) enter( event.Location, "Loc.");

        if ( event.AllDayEvent != undefined && event.AllDayEvent == true ) {
            setChecked( true, "All day event" );
            verify( !isVisible( "Start/Time" ) );
        } else if ( event.AllDayEvent == false ) {
            setChecked( false, "All day event" );
            verify( isVisible( "Start/Time" ) );
        }

        if ( event.TimeZone != undefined ) select( event.TimeZone, "T.Z." );

        if ( event.StartDate != undefined ) {
            verify( isEnabled( "Start/Date" ) );
            enter( event.StartDate.date(), "Start/Date" );
        }

        if ( event.StartTime != undefined ) {
            verify( isEnabled( "Start/Time" ) );
            enter( event.StartTime.time(), "Start/Time" );
        }

        if ( event.Reminder != undefined) {
            select( event.Reminder, "Reminder" );
            if ( event.BeforeTime != undefined) {
                select( event.BeforeTime, signature("Reminder", 1) );
            } else if ( event.AllDayEvent != undefined ) {
                if ( event.AllDayEvent == true )
                    select( "On the day", signature("At", 1) );
                else
                    select( "At the event time", signature("Reminder", 1) );
            }
        }

        if ( event.Recurrence != undefined ) {
            select( "Recurrence", tabBar() );
            select( event.Recurrence, "Repeat" );
        }

        if ( event.Frequency != undefined ) {
            select( "Recurrence", tabBar() );
            enter( event.Frequency, "Frequency" );
        }

        if ( event.RepeatUntil != undefined ) {
            select( "Recurrence", tabBar() );
            setChecked( true, "Repeat until" );
            enter( event.RepeatUntil, "Repeat until/Until" );
        }

        waitFor() { return ( getList(softMenu()).contains("Back") ); }
        select( "Back", softMenu() );
        waitForTitle( "Event Details" );
    },

    setDefaultSettings: function()
    {
        if (currentTitle().contains("Reminder")) {
            print( "WARNING: had to close a Reminder dialog" );
            select( "Back", softMenu() );
        }
        waitForTitle( "Calendar" );
        select( "Settings...", optionsMenu() );
        waitForTitle( "Settings" );
        enter( "7:00", "View/Day starts at" );
        waitFor() { return ( getList(softMenu()).contains("Back") ); }
        select( "Back", softMenu() );
    },

    setTimeZone: function(tz)
    {
        startApplication( "systemtime" );
        waitForTitle( "Date/Time" );
        select( "Time", tabBar() );
        select( "Off", "Automatic" );
        select( tz, "Time Zone" );
        waitFor() { return ( getList(softMenu()).contains("Back") ); }
        select( "Back", softMenu() );
        gotoHome();
    },

    enterKeyClicks : function(keyClicks)
    {
        // Sometimes the first keyclick gets ignored, so simulate pressing # (which does nothing)
        keyClick( Qt.Key_NumberSign );
        for (var i=0; i<keyClicks.length; i++) {
            wait(200);
            eval("keyClick(Qt.Key_" + keyClicks[i] + ")");
        }
    },

    enterDateKeyClicks : function(dt)
    {
        df = dateFormat();
        formatString = "MMddyyyy";

        if (df == "Y-M-D") {
            formatString = "yyyyMMdd";
        } else if (df == "D/M/Y" || df == "D.M.Y" ) {
            formatString = "ddMMyyyy";
        }

        Calendar.enterKeyClicks( dt.toString(formatString) );
        wait(5000);
    },

    setDateTimeSetting: function( dt )
    {
        setDateTime( dt );
        gotoHome();
    }

};

