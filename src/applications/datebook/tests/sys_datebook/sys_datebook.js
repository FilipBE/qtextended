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

include("calendar.js");

//TESTED_COMPONENT=PIM: Calendar (18573)

testcase = {

    created_events: [],

    initTestCase: function()
    {
        waitForQtopiaStart();

        // Ensure certain timezones are available for selection
        origZone0 = getSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone0");
        setSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone0", "America/New_York");
        origZone1 = getSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone1");
        setSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone1", "America/Los_Angeles");
        origZone2 = getSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone2");
        setSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone2", "Asia/Dubai");
        origZone3 = getSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone3");
        setSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone3", "Asia/Tokyo");
        origZone4 = getSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone4");
        setSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone4", "Australia/Brisbane");
        origZone5 = getSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone5");
        setSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone5", "Asia/Hong_Kong");

        // Week should start on Sunday
        monday = getSetting("$HOME/Settings/Trolltech/qpe.conf", "Time", "MONDAY");
        setSetting("$HOME/Settings/Trolltech/qpe.conf", "Time", "MONDAY", "false");

        // Set the current timezone to "Brisbane"
        Calendar.setTimeZone( "Brisbane" );
    },

    cleanupTestCase: function()
    {
        // FIXME: I am assuming that the mode WAS auto!
        setTimeSynchronization( AutoTimeSynchronization );

        // Restore original timezone selections
        setSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone0", origZone0);
        setSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone1", origZone1);
        setSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone2", origZone2);
        setSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone3", origZone3);
        setSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone4", origZone4);
        setSetting("$HOME/Settings/Trolltech/WorldTime.conf", "TimeZones", "Zone5", origZone5);

        // Restore week starts on setting
        setSetting("$HOME/Settings/Trolltech/qpe.conf", "Time", "MONDAY", monday);
    },

    init: function()
    {

/*
    FIXME: should not be necessary.  Bug 231167.
    There are some difficult-to-reproduce failures of this test currently.  One of these
    failures can cause all subsequent testfunctions to fail.
    To help diagnose the failure, try to make each testfunction execution independent by
    restarting Qtopia for each test.
*/
        deletePath("$HOME/Applications");
        restartQtopia();

        setTimeSynchronization( ManualTimeSynchronization );
        wait(4000);
        // BUG 187069
        startApplication( "Calendar" );

        var L = getList(optionsMenu());
        if (L.contains("Today") && !L.contains("Month")) {
            select( "Back", softMenu() );
            wait(500);
        }
        Calendar.setDefaultSettings();
    },

    cleanup: function()
    {
        Calendar.removeAllEvents();
        gotoHome();
        synchronizeDateTime();
    },

/*
    \req QTOPIA-244
    \groups Acceptance
*/
    creating_a_single_event_data:
    {
        // Test data expects a name, location, time (both to enter and expected display) and date (both entry and expected display),
        // eg for new event at some location at 11:15 on the 29th March 2007
        // enter [ new event, some location, 1115, 11:15:00, 03292007, Thu Mar 29 2007 ]
        "event1": [{
            Name: "new event",
            Location: "some location",
            StartDt: new QDateTime( new QDate( 2007, 3, 29), new QTime(11, 13, 0) )
        }]
    },

    creating_a_single_event: function(testdata)
    {
        Calendar.enterNewEvent({ Description: testdata.Name, Location: testdata.Location,
                        StartDateTime: testdata.StartDt, AllDayEvent: false });

        setDateTime( testdata.StartDt );
        select( "Today", optionsMenu() );

        verifyImage( "new_event_listed", "", "Verify that the new event is listed at: " + testdata.StartDt.toString() );
    },

/*
    \req QTOPIA-250
    \groups Acceptance
*/
    creating_an_event_with_an_audible_alarm_data:
    {
        "eventtime1": [new QDateTime( new QDate( 2007, 5, 27), new QTime(11, 20, 15) ), new QDateTime( new QDate( 2007, 5, 27), new QTime(11, 21, 0) )]
    },

    creating_an_event_with_an_audible_alarm: function(TestDt,EventDt)
    {
        setDateTime( TestDt );
        Calendar.enterNewEvent({ Description: "alarm event", StartDateTime: EventDt, Reminder: "Audible" });
        gotoHome();

//        showWaitMessage("Waiting for Alarm Event to take place...");
        waitForTitle( "Calendar", 50000 );
//        hideWaitMessage();

        // Select the alarm event from the list, to show details.
        select( "alarm event", signature("Snooze", -1) );

        // Verify that the events details are displayed correctly.
        waitForTitle( "Event Details" );
        verify( getText().contains("alarm event") );

        // FIXME: Now we actually have to verify that we can hear a sound!!!
    },

/*
    \req QTOPIA-345
*/
    snoozing_an_event_data:
    {
        snooze_default: [new QDateTime( new QDate( 2004, 2, 22), new QTime(11, 20, 0) ), QDateTime( new QDate( 2004, 2, 22), new QTime(11, 25, 0) ), undefined   ],
        snooze_5_mins:  [new QDateTime( new QDate( 2004, 2, 22), new QTime(11, 20, 0) ), QDateTime( new QDate( 2004, 2, 22), new QTime(11, 25, 0) ), "5 minutes" ],
        snooze_10_mins: [new QDateTime( new QDate( 2004, 2, 22), new QTime(11, 20, 0) ), QDateTime( new QDate( 2004, 2, 22), new QTime(11, 30, 0) ), "10 minutes"],
        snooze_15_mins: [new QDateTime( new QDate( 2004, 2, 28), new QTime(23, 50, 0) ), QDateTime( new QDate( 2004, 2, 29), new QTime(00, 05, 0) ), "15 minutes"],
        snooze_30_mins: [new QDateTime( new QDate( 2004, 2, 22), new QTime(11, 20, 0) ), QDateTime( new QDate( 2004, 2, 22), new QTime(11, 50, 0) ), "30 minutes"],
        snooze_1_hour:  [new QDateTime( new QDate( 2004, 2, 22), new QTime(11, 20, 0) ), QDateTime( new QDate( 2004, 2, 22), new QTime(12, 20, 0) ), "1 hour"    ],
        snooze_1_day:   [new QDateTime( new QDate( 2004, 2, 22), new QTime(11, 20, 0) ), QDateTime( new QDate( 2004, 2, 23), new QTime(11, 20, 0) ), "1 day"     ],
        snooze_1_week:  [new QDateTime( new QDate( 2004, 2, 22), new QTime(11, 20, 0) ), QDateTime( new QDate( 2004, 2, 29), new QTime(11, 20, 0) ), "1 week"    ],
        snooze_1_month: [new QDateTime( new QDate( 2004, 2, 22), new QTime(11, 20, 0) ), QDateTime( new QDate( 2004, 3, 22), new QTime(11, 20, 0) ), "1 month"   ],
        snooze_no_leap: [new QDateTime( new QDate( 2005, 2, 28), new QTime(23, 50, 0) ), QDateTime( new QDate( 2005, 3, 1),  new QTime(00, 05, 0) ), "15 minutes"]
    },

    snoozing_an_event: function( EventDt, SnoozeDt, SnoozeMode )
    {
        var offset = 60;
        if (runsOnDevice()) {
            // Greenphone shouldn't need this much additional time
            offset = 120;
        }

        Calendar.setDateTimeSetting( EventDt.addSecs(-offset) );
        startApplication("Calendar");
        waitForTitle( "Calendar" );
        Calendar.enterNewEvent({ Description: "snooze event test", StartDateTime: EventDt, Reminder: "Silent" });
        gotoHome();

        // This test can take a while. So user doesn't think it has locked up, print a message...
        print( "Waiting for alarm (expected in " + getDateTime().secsTo(EventDt) + " seconds)" );

        // Wait for alarm...
        waitForTitle( "Calendar", offset * 1100 );
        if (SnoozeMode != undefined) {
            select( SnoozeMode, "Snooze delay\\:" );
        }

        // Selecting "Snooze" causes the application to close, which would
        // result in a failure if QtUiTest was not expecting it...
        expectApplicationClose(true);
        select( "Snooze" );
        expectApplicationClose(false);

        // Jump forward in time and wait for the Snooze alarm...
        Calendar.setDateTimeSetting( SnoozeDt.addSecs(-offset) );
        print( "Waiting for snooze alarm (expected in " + getDateTime().secsTo(SnoozeDt) + " seconds)" );
        waitForTitle( "Calendar", offset * 1100 );
    },

/*
    \req QTOPIA-250
*/
    events_while_running_other_apps_data:
    {
        goto_home:        [new QDateTime( new QDate( 2007, 3, 29), new QTime(11, 15, 0) ), new QDateTime( new QDate( 2007, 3, 29), new QTime(11, 16, 0) ), "home"],
        stay_in_calendar: [new QDateTime( new QDate( 2007, 3, 29), new QTime(11, 15, 0) ), new QDateTime( new QDate( 2007, 3, 29), new QTime(11, 16, 0) ), "stay"],
        run_clock:        [new QDateTime( new QDate( 2007, 3, 29), new QTime(11, 15, 0) ), new QDateTime( new QDate( 2007, 3, 29), new QTime(11, 16, 0) ), "Clock"],
        run_contacts:     [new QDateTime( new QDate( 2007, 3, 29), new QTime(11, 15, 0) ), new QDateTime( new QDate( 2007, 3, 29), new QTime(11, 16, 0) ), "Contacts"],
        run_systemtime:   [new QDateTime( new QDate( 2007, 3, 29), new QTime(11, 15, 0) ), new QDateTime( new QDate( 2007, 3, 29), new QTime(11, 16, 0) ), "Settings/Date/Time"]
    },

    events_while_running_other_apps: function(TestDt, EventDt, Mode)
    {
        // This test times out in autotest runs
        //skip( "BUG-229518", SkipAll );

        setDateTime( TestDt );
        Calendar.enterNewEvent({ Description: "Silent alarm event", StartDateTime: EventDt, Reminder: "Silent" });

        if (Mode == "home") gotoHome();
        else if (Mode != "stay") {
            gotoHome();
            select( Mode, launcherMenu() );
        }

        // We assume that the reminder popping up means we'll have a field labelled "Snooze" on the
        // screen.
        verify(!getLabels().contains("Snooze"));

        // This test can take a while. So user doesn't think it has locked up, print a message...
        print( "Waiting for alarm (expected in " + getDateTime().secsTo(EventDt) + " seconds)" );
        waitFor(150000) { return getLabels().contains("Snooze"); }

        // Make sure we get a reminder at the correct date and time
        var acceptableDelta = 10;
        if (runsOnDevice()) {
            // Give Greenphone a bit more time
            acceptableDelta = 20;
        }

        verify (EventDt.secsTo(getDateTime()) < acceptableDelta);
    },

/*
    \req QTOPIA-250
*/
    setting_multiple_simultaneous_audible_events_data:
    {
        test1: [new QDateTime( new QDate( 2007, 3, 29), new QTime(11, 15, 0) ), new QDateTime( new QDate( 2007, 3, 29), new QTime(11, 16, 0) )]
    },

    setting_multiple_simultaneous_audible_events: function(TestDt, EventDt)
    {
        setDateTime( TestDt );

        Calendar.enterNewEvent({ Description: "alarm event one", StartDateTime: EventDt, Reminder: "Audible" });
        Calendar.enterNewEvent({ Description: "alarm event two", StartDateTime: EventDt, Reminder: "Audible" });

        gotoHome();
        waitForTitle(  "Calendar", 60000 );

        // Verify that the events details are displayed correctly.
        select( "alarm event one", signature("Snooze", -1) );
        waitForTitle( "Event Details" );
        verify( getText().contains("alarm event one") );

        select( "Back", softMenu() );
        // we should be back in reminders, so that I can take a look at the other event
//        expectFail( "BUG-187471" ); //View event doesn't return to Reminders dialog
        waitForTitle( "Calendar", 15000 );

        select( "alarm event two", signature("Snooze", -1) );
        waitForTitle( "Event Details" );
        verify( getText().contains("alarm event two") );

        prompt( "Did you hear a sound when the Reminder dialog was shown?" );

// FIXME: we need to check the date and time
//        compare( getDateTime(), testdata.EventDt );
    },

/*
    \req QTOPIA-250
*/
    sounding_alarms_before_an_events_start_time_data:
    {
        //                         BeforeTime,          ClockStart  (AlarmDt - 1 minute                                 AlarmDt (StartTime - BeforeTime)                            StartTime)
        alarm_5_minutes_before: [  "5 minutes before",  new QDateTime( new QDate( 2007, 3, 29), new QTime(11, 15, 0) ), QDateTime( new QDate( 2007, 3, 29), new QTime(11, 16, 0) ), QDateTime( new QDate( 2007, 3, 29), new QTime(11, 21, 0) ) ],
        alarm_15_minutes_before: [ "15 minutes before", new QDateTime( new QDate( 2007, 3, 29), new QTime(11, 5, 0) ),  QDateTime( new QDate( 2007, 3, 29), new QTime(11, 6, 0) ),  QDateTime( new QDate( 2007, 3, 29), new QTime(11, 21, 0) ) ],
        alarm_30_minutes_before: [ "30 minutes before", new QDateTime( new QDate( 2007, 3, 29), new QTime(10, 50, 0) ), QDateTime( new QDate( 2007, 3, 29), new QTime(10, 51, 0) ), QDateTime( new QDate( 2007, 3, 29), new QTime(11, 21, 0) ) ],
        alarm_1_hour_before: [     "1 hour before",     new QDateTime( new QDate( 2007, 3, 29), new QTime(10, 20, 0) ), QDateTime( new QDate( 2007, 3, 29), new QTime(10, 21, 0) ), QDateTime( new QDate( 2007, 3, 29), new QTime(11, 21, 0) ) ],
        alarm_2_hours_before: [    "2 hours before",    new QDateTime( new QDate( 2007, 3, 29), new QTime(9, 20, 0) ),  QDateTime( new QDate( 2007, 3, 29), new QTime(9, 21, 0) ),  QDateTime( new QDate( 2007, 3, 29), new QTime(11, 21, 0) ) ],
        alarm_1_day_before: [      "1 day before",      new QDateTime( new QDate( 2007, 3, 28), new QTime(11, 20, 0) ), QDateTime( new QDate( 2007, 3, 28), new QTime(11, 21, 0) ), QDateTime( new QDate( 2007, 3, 29), new QTime(11, 21, 0) ) ],
        alarm_2_days_before: [     "2 days before",     new QDateTime( new QDate( 2007, 3, 27), new QTime(11, 20, 0) ), QDateTime( new QDate( 2007, 3, 27), new QTime(11, 21, 0) ), QDateTime( new QDate( 2007, 3, 29), new QTime(11, 21, 0) ) ],
        alarm_3_days_before: [     "3 days before",     new QDateTime( new QDate( 2007, 3, 26), new QTime(11, 20, 0) ), QDateTime( new QDate( 2007, 3, 26), new QTime(11, 21, 0) ), QDateTime( new QDate( 2007, 3, 29), new QTime(11, 21, 0) ) ],
        alarm_1_week_before: [     "1 week before",     new QDateTime( new QDate( 2007, 3, 22), new QTime(11, 20, 0) ), QDateTime( new QDate( 2007, 3, 22), new QTime(11, 21, 0) ), QDateTime( new QDate( 2007, 3, 29), new QTime(11, 21, 0) ) ],
        alarm_2_weeks_before: [    "2 weeks before",    new QDateTime( new QDate( 2007, 3, 15), new QTime(11, 20, 0) ), QDateTime( new QDate( 2007, 3, 15), new QTime(11, 21, 0) ), QDateTime( new QDate( 2007, 3, 29), new QTime(11, 21, 0) ) ]
    },

    sounding_alarms_before_an_events_start_time: function(BeforeTime,ClockStart,AlarmDt,StartTime)
    {
        setDateTime( ClockStart );
        Calendar.enterNewEvent({ Description: "alarm event", StartDateTime: StartTime, Reminder: "Audible", BeforeTime: BeforeTime });
        gotoHome();
        waitForTitle( "Calendar", 70000 );

        compare( getDateTime().toString("hh:mm"), AlarmDt.toString("hh:mm") );

        prompt( "You should have heard a sound when the reminder dialog was shown. Is this correct?" );
        select( "Back", softMenu() );
    },

/*
    \req QTOPIA-250
*/
    configuring_the_alarm_settings_data:
    {
        //                  PresetChecked, PresetTime, ReminderText, BeforeText
        preset_15_minutes: [true,          "15",       "Audible",    "15 minutes before"],
        preset_30_minutes: [true,          "30",       "Audible",    "30 minutes before"],
        preset_false:      [false,         "",         "None",       ""]
    },

    configuring_the_alarm_settings: function(PresetChecked, PresetTime, ReminderText, BeforeText)
    {
        select( "Settings...", optionsMenu() );

        waitForTitle( "Settings" );
        select (ReminderText, "Preset/Reminder");
        // FIXME: once BUG 187273 is fixed this should be setChecked(PresetChecked,"Alarm/Preset");

        if (ReminderText != "None") {
            select(BeforeText, signature("Preset/Reminder", 1));
        }
        waitFor() { return ( getList(softMenu()).contains("Back") ); }
        select( "Back", softMenu() );

        select( "New event", optionsMenu() );
        compare( getSelectedText("Reminder"),ReminderText);

//        expectFail( "BUG-187275" ); //New Event doesn't use Preset values from 'Settings' dialog.\nActual: " + getSelectedText(signature("Reminder",1)) + "\nExpected: " + BeforeText

        if (BeforeText != "")
        {
            print("Selected text: " + getSelectedText(signature("Reminder",1)) + ", BeforeText: " + BeforeText);
            compare( getSelectedText(signature("Reminder",1)), BeforeText );
        }
        select( "Cancel", optionsMenu() );
    },

/*
    \req QTOPIA-7405
*/
    viewing_today_data: {
        date1: new QDate( 2007, 7, 1 )
    },

    viewing_today: function( Today )
    {
        setDateTime(Today);
        select( "Month", optionsMenu() );
        // select an arbitrary day other than today.
        select( "Select", softMenu() );
        Calendar.enterDateKeyClicks( Today );
        verifyImage( "today", "", "Verify that the date is shown as: Today (" + Today.toString("ddd") + ") " + Today.toString("dd/MM/yyyy") );

        var nextWeek = Today.addDays(7);
        Calendar.enterDateKeyClicks( nextWeek );
        verifyImage( "next_week", "", "Verify that the date is shown as: " + nextWeek.toString("dddd dd/MM/yyyy") );

        select( "Today", optionsMenu() );
        verify( compareImage( "today", "" ) );
    },

/*
    \req QTOPIA-243
*/
    displaying_multiple_events_data: {
    },

    displaying_multiple_events: function()
    {
        var Today = new QDateTime( new QDate( 2007, 3, 30), new QTime(11, 15, 0) );
        var EventDt1 = new QDateTime( new QDate( 2007, 3, 30), new QTime(9, 20, 0) );
        var EventDt2 = new QDateTime( new QDate( 2007, 3, 30), new QTime(11, 20, 0) );
        var EventDt3 = new QDateTime( new QDate( 2007, 3, 30), new QTime(13, 20, 0) );

        setDateTime( Today );

        Calendar.enterNewEvent({ Description: "event 1", StartDateTime: EventDt1, Reminder: "Silent" });
        Calendar.enterNewEvent({ Description: "event 3", StartDateTime: EventDt3, Reminder: "Silent" });
        Calendar.enterNewEvent({ Description: "event 2", StartDateTime: EventDt2, Reminder: "Silent" });

        verifyImage( "selected_events", "", "Verify that the second event is selected");
    },

/*
    \req QTOPIA-249
*/
    navigating_events_on_one_day_data: {
    },

    navigating_events_on_one_day: function()
    {
        var Today = new QDateTime( new QDate( 2007, 3, 30), new QTime(11, 15, 0) );
        var EventDt1 = new QDateTime( new QDate( 2007, 3, 30), new QTime(9, 20, 0) );
        var EventDt2 = new QDateTime( new QDate( 2007, 3, 30), new QTime(11, 20, 0) );
        var EventDt3 = new QDateTime( new QDate( 2007, 3, 30), new QTime(13, 20, 0) );

        setDateTime( Today );

        Calendar.enterNewEvent({ Description: "event 1", StartDateTime: EventDt1, Reminder: "Silent" });
        Calendar.enterNewEvent({ Description: "event 3", StartDateTime: EventDt3, Reminder: "Silent" });
        Calendar.enterNewEvent({ Description: "event 2", StartDateTime: EventDt2, Reminder: "Silent" });

        // FIXME: The next block of code needs to be replaced when custom widgets are supported.
        if (mousePreferred()) {
            skip( "Not implemented yet for touchscreen device" );
        } else {
            // Verify that the up and down buttons on the directional pad change the focus between the events.
            verifyImage( "current_event_selected", "", "The second event should be highlighted");
            keyClick( Qt.Key_Up );
            verifyImage( "prev_event_selected", "", "The first event should be highlighted");
            keyClick( Qt.Key_Down );
            keyClick( Qt.Key_Down );
            verifyImage( "next_event_selected", "", "The third event should be highlighted");
            keyClick( Qt.Key_Up );
        }
    },

/*
    \req QTOPIA-243
*/
    time_flies_data: {
    },

    time_flies: function()
    {
        gotoHome();
        var Today = new QDateTime( new QDate( 2007, 3, 29), new QTime(23, 59, 0) );
        setDateTime( Today );

        startApplication( "Calendar" );
        select( "Month", optionsMenu() );

        //FIXME: Wait for wait indicator (clock icon) to fade out
        wait(500);

        // FIXME: These verifyImage calls should be replaced by a compare( getText(), "foobar" ) when we support custom widgets
        verifyImage( "today", "", "The date should be 29 March 2007 in Month view" );

        var Tomorrow = new QDateTime( new QDate( 2007, 3, 30), new QTime(0, 0, 10) );
        setDateTime( Tomorrow );
        select( "Today", optionsMenu() );

//        expectFail( "BUG-187476" ); //Month view doesn't update the current day
        verifyImage( "tomorrow", "", "The date should be 30 March 2007 in Month view" );
    },

/*
    \req QTOPIA-249
*/
    navigating_days_data: {
        day_view: ["day"],
        month_view: ["month"]
    },

    navigating_days: function(View)
    {
        var Today = new QDateTime( new QDate( 2007, 3, 30), new QTime(11, 15, 0) );
        setDateTime( Today );

        if (View == "month") {
            // FIXME: Because of a bug the monthview doesn't update correctly unless re-started.
            // See BUG
            gotoHome();
            wait(1000);
            startApplication( "Calendar" );
            select( "Month", optionsMenu() );
        }
        else select( "Today", optionsMenu() );

        //FIXME: Wait for wait indicator (clock icon) to fade out
        wait(500);

        if (mousePreferred()) {
            skip( "Don't know how to test this on a touchscreen device" );
        } else {
            // FIXME: These verifyImage calls should be replaced by a compare( getText(), "foobar" ) when we support custom widgets
            verifyImage( "current_day", "", "The date should be 30 March 2007 in " + View + " view" );

            // Verify that the left and right buttons on the directional pad change to the next and previous date.
            keyClick( Qt.Key_Left );
            wait(1000);
            verifyImage( "prev_day", "", "The date should be 29 March 2007 in " + View + " view" );
            keyClick( Qt.Key_Right );
            keyClick( Qt.Key_Right );
            wait(1000);
            verifyImage( "next_day", "", "The date should be 31 March 2007 in " + View + " view" );
        }
    },

/*
    \req QTOPIA-249
*/
    navigating_weeks_data: {
    },

    navigating_weeks: function()
    {
        var Today = new QDateTime( new QDate( 2007, 3, 30), new QTime(11, 15, 0) );
        setDateTime( Today );
        select( "Month", optionsMenu() );
        select( "Today", optionsMenu() );

        if (mousePreferred()) {
            skip( "Test not implemented for touchscreen device" );
        } else {
            keyClick( Qt.Key_Up );
            // FIXME: These verifyImage calls should be replaced by a compare( getText(), "foobar" ) when we support custom widgets
            wait(1000);
            verifyImage( "prev_week_dayview", "", "The date should be 23 March 2007 in Month view" );
            keyClick( Qt.Key_Down );
            keyClick( Qt.Key_Down );
            wait(1000);
            verifyImage( "next_week_dayview", "", "The date should be 6 April 2007 in Month view" );
        }
    },

/*
    \req QTOPIA-249
*/
    navigating_months: function()
    {
        var Today = new QDateTime( new QDate( 2007, 3, 30), new QTime(11, 15, 0) );
        setDateTime( Today );
        if (getList(optionsMenu()).contains("Month")) {
            select( "Month", optionsMenu() );
        }
        select( "Today", optionsMenu() );

        if (mousePreferred()) {
            skip( "Don't know how to test this on a touchscreen device" );
        } else {
            for (var i=0; i<5; i++) keyClick( Qt.Key_Up );
            // FIXME: These verifyImage calls should be replaced by a compare( getText(), "foobar" ) when we support custom widgets
            wait(1000);
            verifyImage( "prev_month_dayview", "", "The date should be 23 February 2007");
            for (var i=0; i<7; i++) keyClick( Qt.Key_Down );
            wait(1000);
            verifyImage( "next_month_dayview", "", "The date should be 13 April 2007");
        }
    },

/*
    \req QTOPIA-249
*/
    navigating_years: function()
    {
        var JanDate = new QDateTime( new QDate( 2007, 2, 10), new QTime(11, 15, 0) );
        setDateTime( JanDate );

        select( "Month", optionsMenu() );
        select( "Today", optionsMenu() );

        if (mousePreferred()) {
            skip( "Don't know how to test this on a touchscreen device" );
        } else {
            for (var i=0; i<7; i++) keyClick( Qt.Key_Up );
            // FIXME: These verifyImage calls should be replaced by a compare( getText(), "foobar" ) when we support custom widgets
            wait(1000);
            verifyImage( "prev_year_view", "", "The date should be 23 December 2006");
        }

        if (mousePreferred()) {
            skip( "Don't know how to test this on a touchscreen device" );
        } else {
            for (var i=0; i<3; i++) keyClick( Qt.Key_Down );
            wait(1000);
            verifyImage( "next_year_view", "", "The date should be 13 January 2007");
        }
    },

/*
    \req QTOPIA-248
*/
    viewing_all_day_events: function()
    {
        var Today = new QDateTime( new QDate( 2007, 3, 30), new QTime(11, 15, 0) );
        var EventDt1 = new QDateTime( new QDate( 2007, 3, 30), new QTime(9, 20, 0) );

        gotoHome();
        setDateTime( Today );
//        wait(2000);
        startApplication( "Calendar" );

        Calendar.enterNewEvent({ Description: "all day event", StartDateTime: EventDt1, Reminder: "Silent", AllDayEvent: true });
        verifyImage( "alldayevent_dayview", "", "Verify that in day view the event is shown at the top of the screen, e.g. no reference is made to a time interval." );
    },

/*
    \req QTOPIA-251
*/
    configuring_day_view_data:
    {
        day_starts_at_3: ["03:00"],
        day_starts_at_18: ["18:00"]
    },

    configuring_day_view: function(StartTime)
    {
        // Need to set date/time to get consistent screenshots
        var Today = new QDateTime( new QDate( 2007, 3, 30), new QTime(11, 15, 0) );

        gotoHome();
        setDateTime( Today );
        startApplication( "Calendar" );

        select( "Settings...", optionsMenu() );
        // This is not a timeedit field, so need to use a string
        enter( StartTime, "View/Day starts at" );

        waitFor() { return ( getList(softMenu()).contains("Back") ); }
        select( "Back", softMenu() );

        // Verify that the day view is adjusted and begins at StartTime.
        verifyImage( "start_dayview", "", "Verify that the date starts at " + StartTime );
    },

/*
    \req QTOPIA-247
*/
    creating_daily_repeating_events_data:
    {
        //               EventDt
        wednesday_test: [new QDateTime( new QDate( 2007, 11, 14), new QTime(11, 11, 11) )]
    },

    creating_daily_repeating_events: function( EventDt )
    {
        gotoHome();
        setDateTime( new QDateTime( new QDate( 2007, 11, 7), new QTime(10, 0, 0) ) );
//        wait(2000);
        startApplication("Calendar");

        Calendar.enterNewEvent({ Description: "repeat event", StartDateTime: EventDt, Reminder: "Silent",
                        AllDayEvent: false, Recurrence: "Daily" });

        waitForTitle( "Calendar" );

        select( "Month", optionsMenu() );

        Calendar.enterDateKeyClicks( EventDt.date().addDays(-7) );

        // Verify that the event is shown on every day starting at the selected day
        verifyImage( "repeating_event_monthview", "", "Verify that the event is shown on every day starting " + EventDt.toString() );

        Calendar.enterDateKeyClicks( EventDt.date().addMonths(1) );
        verifyImage( "repeating_event_monthview_december", "", "Verify that the event is shown on every day in " + EventDt.date().addMonths(1).toString("MMMM") );

        Calendar.enterDateKeyClicks( EventDt.date().addMonths(2) );
        verifyImage( "repeating_event_monthview_january", "", "Verify that the event is shown on every day in " + EventDt.date().addMonths(2).toString("MMMM") );
    },

/*
    \req QTOPIA-246
*/
    editing_repeating_events_data:
    {
        //               EventDt
        wednesday_test: [new QDateTime( new QDate( 2007, 11, 14), new QTime(11, 11, 11) )]
    },

    editing_repeating_events: function( EventDt )
    {
        Calendar.enterNewEvent({ Description: "Repeat Event", StartDateTime: EventDt,
                        AllDayEvent: false, Recurrence: "Daily" });

        waitForTitle( "Calendar" );
        Calendar.enterDateKeyClicks( EventDt.date().addDays(7) );
        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );

        // Edit earlier instances of the event...
        select( "Edit event", optionsMenu() );
        waitForTitle( "Edit Event" );
        setChecked( true, "Earlier Appointments" );
        setChecked( false, "This Appointment" );
        setChecked( false, "Later Appointments" );
        select( "Next", softMenu() );

        // Update the Location
        Calendar.updateCurrentEvent({ Location: "First week" });
        Calendar.created_events.push( EventDt.date().addDays(7) );

        // BUG 207910 - remove the following line when this bug is resolved
        Calendar.created_events.push( EventDt.date().addDays(7) );

        // Edit later instances of the event...
        select( "Edit event", optionsMenu() );
        waitForTitle( "Edit Event" );
        setChecked( false, "Earlier Appointments" );
        setChecked( false, "This Appointment" );
        setChecked( true, "Later Appointments" );
        select( "Next", softMenu() );

        // Update the Location
        Calendar.updateCurrentEvent({ Location: "Later events" });

        // BUG 207910 - the following line should be uncommented when this bug is resolved
        //Calendar.created_events.push( EventDt.date().addDays(8) );

        // This instance is no longer part of the series (since it has no Location)
        verify( getText().indexOf("Repeat:") == -1 );

        // Update the Location - should not get prompted this time
        select( "Edit event", optionsMenu() );
        waitForTitle( "Edit Event" );
        Calendar.updateCurrentEvent({ Location: "One week on" });
        select( "Back", softMenu() );

        // Verify Event Details for earlier events...
        Calendar.enterDateKeyClicks( EventDt.date() );
        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );
        verify( getText().indexOf("Where: First week") != -1 );
        select( "Back", softMenu() );

        // Verify Event Details for earlier events...
        Calendar.enterDateKeyClicks( EventDt.date().addDays(7) );
        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );
        verify( getText().indexOf("Where: One week on") != -1 );
        select( "Back", softMenu() );

        // Verify Event Details for earlier events...
        Calendar.enterDateKeyClicks( EventDt.date().addDays(8) );
        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );
        verify( getText().indexOf("Where: Later events") != -1 );
        select( "Back", softMenu() );
    },

/*
    \req QTOPIA-242
*/
    creating_weekly_repeating_events_data:
    {
        //               EventDt
        wednesday_test: [ new QDateTime( new QDate( 2007, 11, 14), new QTime(11, 11, 11) ),
                            new QDate( 2008, 11, 14), new QDate( 2008, 1, 14), new QDate( 2007, 9, 14) ],
        sunday_test: [ new QDateTime( new QDate( 2007, 11, 18), new QTime(11, 11, 11) ),
                            new QDate( 2008, 11, 18), new QDate( 2008, 1, 18), new QDate( 2007, 9, 18) ]

    },

    creating_weekly_repeating_events: function( EventDt, EndDt1, EndDt2, EndDt3 )
    {
        // Create a new weekly event
        Calendar.enterNewEvent({ Description: "weekly event", StartDateTime: EventDt, Reminder: "Silent",
                        AllDayEvent: false, Recurrence: "Weekly", RepeatUntil: EndDt1 });

        waitForTitle( "Calendar" );

        // Change the view to "Month" and verify that the event is shown on Wednesday of every week for the next 12 months starting at the current week.
        if ( getList(optionsMenu()).contains("Month") ) {
            select( "Month", optionsMenu() );
        }
        var dayOfWeek = EventDt.date().toString("dddd");

        Calendar.enterDateKeyClicks( EventDt.date() );
        verifyImage( "weekly_repeating_event_start", "", "Verify that the event is shown every " + dayOfWeek + " starting " + EventDt.date().toString("MMMM dd yyyy") );

        Calendar.enterDateKeyClicks(EndDt1);
        verifyImage( "weekly_repeating_event_end1", "", "Verify that the event is shown every " + dayOfWeek + " until " + EndDt1.toString("MMMM dd yyyy") );

        Calendar.enterDateKeyClicks( EventDt.date() );
        select( "Select", softMenu() );

        // Edit the event. In the "Till" field select a date that lies two months in the future.
        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );

        Calendar.updateEvent({ RepeatUntil: EndDt2 });

        select( "Back", softMenu() );
        select( "Month", optionsMenu() );
        Calendar.enterDateKeyClicks(EndDt2);
        // Verify that the event is shown on Wednesday of every week starting at the current week and that it not shown anymore after the entered "Till" date.
        verifyImage( "weekly_repeating_event_end2", "", "Verify that the event is shown every " + dayOfWeek + " until " + EndDt2.toString("MMMM dd yyyy"));

        // Edit the event again this time selecting a "Till" date that lies before the start date.
        Calendar.enterDateKeyClicks( EventDt.date() );
        select( "Select", softMenu() );
        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );
        Calendar.updateEvent({ RepeatUntil: EndDt3 });

        // Verify that the "Till" date is adjusted to the start date.
        var txt = getText();
        var fromMatch = txt.match(/Repeat: From ([^\,]*)\, ([^\,]*)\,/);
        var untilMatch = txt.match(/ending on ([^\,]*)\, ([^\,]*)$/);
        compare(fromMatch[2], untilMatch[2]);

        // Delete the event.
        select( "Delete event", optionsMenu() );
        waitForTitle( "Delete Event" );
        // Verify that a dialog is displayed confirming the deletion of all or a single instance of the event.
        verify ( getText().contains("Earlier Appointments") );
        Calendar.created_events = [];

        waitFor() { return ( getList(softMenu()).contains("OK") ); }
        select( "OK", softMenu() );
        verifyImage( "weekly_repeating_event_deleted", "", "Verify that there are no events shown" );
    },

/*
    \req QTOPIA-242
*/
    creating_yearly_repeating_events_data:
    {
        annual_test: [ new QDateTime( new QDate( 2007, 11, 14), new QTime(11, 11, 11) ),
                        new QDate( 2011, 11, 14) ]
    },

    creating_yearly_repeating_events: function( EventDt, EndDt1 )
    {
        // Create a new yearly event
        Calendar.enterNewEvent({ Description: "annual repeat", StartDateTime: EventDt, Reminder: "Silent",
                        AllDayEvent: false, Recurrence: "Yearly", RepeatUntil: EndDt1 });

        // Change the view to "Month" and verify that the event is shown on Wednesday of every week for the next 12 months starting at the current week.
        if ( getList(optionsMenu()).contains("Month") ) {
            select( "Month", optionsMenu() );
        }

        // Verify that the event is shown on the same date every year starting at the Wednesday of the current week and verify the event repeats for the next five years
        for (var i=0; i<5; ++i) {
            var dt = EventDt.date().addYears(i);
            Calendar.enterDateKeyClicks( dt );
            verifyImage( "yearly_repeating_year"+i , "", "Verify that the event is shown on " + dt.toString() );
        }

        // Also verify that the event is not shown in at least the three years before the start of the event.
        var dt = EventDt.date().addYears(-3);
        Calendar.enterDateKeyClicks( dt );
        verifyImage( "yearly_repeating_3yearprev" , "", "Verify that the event is NOT shown on " + dt.toString() );

        // Edit the event and select "All" to displayed dialog.
        // In the "Till" field select a date that lies two years in the future.
        // Verify that the event is shown every year from the "Start" until the "Till" date.
        Calendar.enterDateKeyClicks( EventDt.date() );
        select( "Select", softMenu() );
        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );
        Calendar.updateEvent({ RepeatUntil: EventDt.date().addYears(2) });

        select( "Back", softMenu() );
        select( "Month", optionsMenu() );

        for (i=0; i<3; ++i) {
            var dt = EventDt.date().addYears(i);
            Calendar.enterDateKeyClicks( dt );
            verifyImage( "yearly_repeating_2_year"+i , "", "Verify that the event is shown on " + dt.toString() );
        }

//        expectFail( "BUG-204897" ); //Month view does not update when event details changed
        dt = EventDt.date().addYears(3);
        Calendar.enterDateKeyClicks( dt );
        verifyImage( "yearly_repeating_3yearlater" , "", "Verify that the event is NOT shown on " + dt.toString() );
    },

/*
    \req QTOPIA-242
*/
    creating_events_that_repeat_every_x_days_data:
    {
        day_repeat_test: [ new QDateTime( new QDate( 2008, 2, 1), new QTime(11, 11, 11) ),
                            3, new QDate( 2008, 3, 20) ]
    },

    creating_events_that_repeat_every_x_days: function( EventDt, Frequency, EndDt1 )
    {
        // Create a new event repeating every 3 days
        Calendar.enterNewEvent({ Description: "daily repeat", StartDateTime: EventDt, Reminder: "Silent",
                        AllDayEvent: false, Recurrence: "Daily", Frequency: Frequency, RepeatUntil: EndDt1 });

        // From the "Rept" (repeat) dialog select the "Other ..." option.
        // Verify that the "Repeat" dialog is displayed.
        // In the "Day" section select the "Every" field and enter "3".
        // Select the "Done" option to close the dialog then again to close the "New Event" dialog.
        // Verify in the month view that the event is shown every third day (so two days it's not, one day it is).

        if ( getList(optionsMenu()).contains("Month") ) {
            select( "Month", optionsMenu() );
        }

        // Verify all days/months until the first leap year and verify that the events are also show correctly near the end of February.
        verifyImage( "3day_repeating_event_monthview" , "", "Verify that the event occurs every " + Frequency + " days from " + EventDt.date().toString("dddd, MMMM dd yyyy") );

        Calendar.enterDateKeyClicks( EndDt1 );

        // Open the event again for editing and enter a "Till" date somewhere in the future.
        // Verify that the events are not shown past the "Till" date.
        verifyImage( "3day_repeating_event_monthview_end" , "", "Verify that the event occurs every " + Frequency + " days until " + EndDt1.toString("dddd, MMMM dd yyyy"));
    },

/*
    \req QTOPIA-242

*/
    creating_events_that_repeat_several_days_per_week_with_an_x_week_interval_data:
    {
        day_repeat_test: [ new QDateTime( new QDate( 2008, 2, 1), new QTime(11, 11, 11) ),
                            new QDate( 2008, 3, 20), ["Monday", "Thursday"] ]
    },

    creating_events_that_repeat_several_days_per_week_with_an_x_week_interval: function( EventDt, EndDt1, RepeatDays )
    {
        // Create new weekly event on specified days of the week
        Calendar.enterNewEvent({ Description: "weekly repeat", StartDateTime: EventDt, Reminder: "Silent",
                        AllDayEvent: false, Recurrence: "Weekly", RepeatUntil: EndDt1, DaysOfWeek: RepeatDays });

        // Verify in the month view that the events are shown on the correct days of the week.
        if ( getList(optionsMenu()).contains("Month") ) {
            select( "Month", optionsMenu() );
        }

        verifyImage( "repeating_events_monthview" , "", "Verify that the event occurs every " + EventDt.date().toString("dddd") + "," + RepeatDays +
                        " starting " + EventDt.date().toString() );

        // Verify that the events are not shown past the "Till" date.
        Calendar.enterDateKeyClicks( EndDt1 );
        verifyImage( "repeating_events_monthview_end" , "", "Verify that the event occurs every " + EventDt.date().toString("dddd") + "," + RepeatDays +
                        " until " + EndDt1.toString() );

        // Edit the repeat settings so that the event repeats every other week, e.g. "Every 2 weeks".
        Calendar.enterDateKeyClicks( EventDt.date() );
        select( "Select", softMenu() );
        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );
        Calendar.updateEvent({ Frequency: 2 });

        select( "Back", softMenu() );

        // Verify in the month view that the events are shown on the correct days of the week and are shown every second week
        select( "Month", optionsMenu() );

//        expectFail( "BUG-204897" ); //Month view does not update when event details changed
        verifyImage( "repeating_event_2weeks_monthview" , "", "Verify that the event occurs on " + EventDt.date().toString("dddd") + "," + RepeatDays +
                        " on alternating weeks" );
    },

/*
    \req QTOPIA-245
    \groups Acceptance
*/
    deleting_events_from_reminder: function()
    {
        var StartDt = new QDateTime( new QDate( 2007, 5, 27), new QTime(11, 20, 0) );
        var EventDt = new QDateTime( new QDate( 2007, 5, 27), new QTime(11, 21, 0) );
        var eventName = "test delete event";

        setDateTime( StartDt );
        select( "Today", optionsMenu() );

        Calendar.enterNewEvent({ Description: eventName, StartDateTime: EventDt, Reminder: "Silent" });

        // Wait for event alarm
        gotoHome();
        waitForTitle( "Calendar", 60000 );

        // Select the event from the reminder list
        select( eventName, signature(focusWidget(), -1) );

        waitForTitle( "Event Details" );
        expectMessageBox( "Calendar","Are you sure you want to delete:","Yes") {
            select( "Delete event", optionsMenu() );
        }
        Calendar.created_events = [];

        // Goes back to home screen now (4.4.0), so the following is no longer relevant
        //waitForTitle( "Calendar" );

        // Verify that the reminder for the deleted event is no longer shown
        //expectFail( "BUG-203717" ); //After deleting event from reminder screen, event is still listed
        //verify( getSelectedText() != eventName );
    },

/*
    \req QTOPIA-245
*/
    deleting_recurring_events_data:
    {
        day_repeat_test: [ new QDateTime( new QDate( 2008, 2, 1), new QTime(11, 11, 11) ),
                            new QDate( 2008, 3, 20) ]
    },

    deleting_recurring_events: function( EventDt, EndDt1 )
    {
        // Create a new repeating event
        Calendar.enterNewEvent({ Description: "delete test 1", StartDateTime: EventDt, Reminder: "Silent",
                        AllDayEvent: false, Recurrence: "Daily", RepeatUntil: EndDt1});

        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );
        waitForTitle( "Event Details" );

        // Delete the event
        select( "Delete event", optionsMenu() );
        waitForTitle( "Delete Event" );

        // Cancel the deletion
        select( "Cancel", optionsMenu() );
        waitForTitle( "Event Details" );

        // Delete the event (again)
        select( "Delete event", optionsMenu() );
        waitForTitle( "Delete Event" );

        // Verify that a dialog is displayed confirming the deletion of earlier/later or a single instance of the event
        verify ( getText().contains("Earlier Appointments") );
        select( "Earlier Appointments" );
        Calendar.created_events = [];

        waitFor() { return ( getList(softMenu()).contains("OK") ); }
        select( "OK", softMenu() );

        // Verify that all the events are removed from the day and month views
        verifyImage( "no_event_dayview", "", "Verify that all the events are removed from the day view." );
        select( "Month", optionsMenu() );
        verifyImage( "no_event_monthview", "", "Verify that all the events are removed from the month view." );
    },

/*
    \req QTOPIA-245
*/
    deleting_instance_of_recurring_events_data:
    {
        day_repeat_test: [ new QDateTime( new QDate( 2008, 2, 1), new QTime(11, 11, 11) ), new QDate( 2008, 3, 20) ]
    },

    deleting_instance_of_recurring_events: function( EventDt, EndDt1 )
    {
        // Create a new daily repeating event
        Calendar.enterNewEvent({ Description: "delete test 2", StartDateTime: EventDt, Reminder: "Silent",
                        AllDayEvent: false, Recurrence: "Daily", RepeatUntil: EndDt1});

        // Tomorrow...
        Calendar.enterDateKeyClicks( EventDt.date().addDays(1) );

        // View the event
        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );
        waitForTitle( "Event Details" );

        // Delete just this instance of the event
        select( "Delete event", optionsMenu() );
        waitForTitle( "Delete Event" );
        setChecked( false, "Earlier Appointments" );
        setChecked( true, "This Appointment" );
        setChecked( false, "Later Appointments" );

        waitFor() { return ( getList(softMenu()).contains("OK") ); }
        select( "OK", softMenu() );

        // Verify that only the event for that day is removed. All other events should still be visible in the day and month views.

//      The day view is not immediately updated, so comparing it now would fail...
//        expectFail( "BUG-205365" ); //Day view not updated when an instance of repeating event is deleted
//        verifyImage( "removed_current_event_dayview", "", "Verify that the event for " + EventDt.date().addDays(1).toString() + " is removed." );

        // Check the start date (event should be displayed)
        Calendar.enterDateKeyClicks( EventDt.date() );
        verifyImage( "not_removed_previous_event_dayview", "", "Verify that the displayed event for " + EventDt.date().toString() + " is NOT removed." );

        // Check the day when the event was deleted (event should not be displayed)
        Calendar.enterDateKeyClicks( EventDt.date().addDays(1) );
        verify( getList(softMenu()).contains("New") );

        // Check the following day (event should be displayed)
        Calendar.enterDateKeyClicks( EventDt.date().addDays(2) );
        verifyImage( "not_removed_following_event_dayview", "", "Verify that the displayed event for " + EventDt.date().addDays(2).toString() + " is NOT removed." );

        // Check the Month View
        select( "Month", optionsMenu() );
        wait(1000);
        verifyImage( "removed_current_event_monthview", "", "Verify that only the event for " + EventDt.date().addDays(1).toString() + " is removed." );

        // Go to the day view for an arbitrary day (on which the event is shown) and edit the event.
        Calendar.enterDateKeyClicks( EventDt.date().addDays(7) );
        select( "Select", softMenu() );
        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );

        // Edit just this instance of the event...
        select( "Edit event", optionsMenu() );
        waitForTitle( "Edit Event" );
        setChecked( false, "Earlier Appointments" );
        setChecked( true, "This Appointment" );
        setChecked( false, "Later Appointments" );
        select( "Next", softMenu() );

        // Update the Location
        Calendar.updateCurrentEvent({ Location: "Far far away" });

        // The updated event instance doesn't get deleted with the rest of the series,
        // so add it to Calendar.created_events for cleanup later
        Calendar.created_events.push( EventDt.date().addDays(7) );

        // Check that summary is updated properly
        verify( getText().contains("Where: Far far away") );
        select( "Back", softMenu() );

        // Verify that the previously deleted event instance is not re-inserted again
        Calendar.enterDateKeyClicks( EventDt.date().addDays(1) );
        verify( getList(softMenu()).contains("New") );

        // Select another arbitrary event...
        Calendar.enterDateKeyClicks( EventDt.date().addDays(10) );
        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );

        // Delete this and subsequent instances...
        select( "Delete event", optionsMenu() );
        waitForTitle( "Delete Event" );
        setChecked( false, "Earlier Appointments" );
        setChecked( true, "This Appointment" );
        setChecked( true, "Later Appointments" );
        select( "OK", softMenu() );

        // Go to the first event of the series
        Calendar.enterDateKeyClicks( EventDt.date() );
        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );
        waitForTitle( "Event Details" );

        // Confirm the event detail have been updated to show new end date
        verify( getText().contains("ending on " + EventDt.date().addDays(9).toString("ddd, d")) );
        select( "Back", softMenu() );
        waitForTitle( "Calendar" );

        // Confirm the Month View shows the changes
        select( "Month", optionsMenu() );
        wait(1000);
        verifyImage( "removed_events_monthview", "", "Verify that events after " + EventDt.date().addDays(9).toString() + " are removed." );
    },

/*
    \req QTOPIA-244
*/
    creating_an_event_in_a_different_timezone_data:
    {
        dubai_test: [ "Dubai", 6, new QDateTime( new QDate( 2007, 5, 27), new QTime(11, 30, 0) ) ]
    },

    creating_an_event_in_a_different_timezone: function( TimeZone, HoursDiff, EventDt )
    {
        var brizTime = EventDt.addSecs(HoursDiff*3600);
        gotoHome();
        setDateTime( brizTime.addSecs(-120) );
        startApplication( "Calendar" );
        waitForTitle( "Calendar" );

        // Create a new event with a silent alarm (at 0 minutes) and set its timezone
        Calendar.enterNewEvent({ Description: TimeZone + " event", StartDateTime: EventDt, Reminder: "Silent",
                        AllDayEvent: false, TimeZone: TimeZone });

        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );
        waitForTitle( "Event Details" );

        // Verify that the created event is shown in the correct local time
        var TimeString = brizTime.toString("h:mm") + " to " + brizTime.addSecs(3600).toString("h:mm") +" (" +
        EventDt.toString("h:mm") + " to " + EventDt.addSecs(3600).toString("h:mm") + " " + TimeZone + " time)";

        verify( getText().indexOf(TimeString) != -1 );
        select( "Back", softMenu() );
        gotoHome();

        // Should be an alarm within 2 minutes
        waitForTitle( "Calendar", 120000 );
    },

/*
    \req QTOPIA-248
*/
    creating_an_all_day_event_in_another_timezone_data:
    {
        dubai_test: [ "Dubai", 6, new QDateTime( new QDate( 2007, 5, 27), new QTime(11, 30, 0) ) ]
    },

    creating_an_all_day_event_in_another_timezone: function( TimeZone, HoursDiff, EventDt )
    {
        expectFail( "BUG-206353" );
        verify(false);
/*
        Open Date/Time and ensure that the current used timezone is set to 'Brisbane'.
        Create a new event for today, make it an all-day event and set it to a 'Brisbane' timezone.
        Verify that the event is shown as an all day event for today.
        Open Date/Time and change the timezone to 'Los Angeles'.
        Verify that the event is still shown on the same date as it was before.
        Open Date/Time and change the timezone to 'Oslo'.
        Verify that the event is still shown on the same date as it was before.
*/
    },

/*
    \req QTOPIA-244
*/
    creating_an_event_at_midnight_in_another_timezone_data:
    {
        la_test: [ "Los Angeles", 727, new QDateTime( new QDate( 2007, 5, 27), new QTime(23, 0, 0) ) ]
    },

    creating_an_event_at_midnight_in_another_timezone: function( TimeZone, HoursDiff, StartDt )
    {
        var EndDt = StartDt.addSecs(10800);
        var localTime = StartDt.addSecs(HoursDiff*3600);

        // Create a new event with a timezone 'None' (default)
        Calendar.enterNewEvent({ Description: "Midnight test", StartDateTime: StartDt, EndDateTime: EndDt });

        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );
        waitForTitle( "Event Details" );
        var eventText = getText();

        select( "Back", softMenu() );
        waitForTitle( "Calendar" );

        // In the day view verify that the event is shown at the end of today and also at the beginning of tomorrow
        verifyImage( "midnight_event_start_day", "", "Verify that the event starts at " + StartDt.time().toString() );
        Calendar.enterDateKeyClicks( EndDt.date() );
        wait(1000);
        verifyImage( "midnight_event_end_day", "", "Verify that the event ends at " + EndDt.time().toString() );

        // Switch Time Zone
        gotoHome();
        Calendar.setTimeZone( TimeZone );

        startApplication( "Calendar" );
        Calendar.enterDateKeyClicks( StartDt.date() );

        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );
        waitForTitle( "Event Details" );
        compare( getText(), eventText );
        select( "Back", softMenu() );
        waitForTitle( "Calendar" );
        verifyImage( "midnight_event_start_day", "", "Verify that the event starts at " + StartDt.time().toString() );

        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );
        waitForTitle( "Event Details" );
        Calendar.updateEvent({ TimeZone: "Brisbane" });
        waitForTitle( "Event Details" );

        // Verify that the presentation of the events has changed (Event is in Brisbane timezone, current timezone is Los Angeles)
        var TimeString = StartDt.addSecs(HoursDiff*3600).toString("h:mm") + " to " + EndDt.addSecs(HoursDiff*3600).toString("h:mm")
        +" (" + StartDt.toString("h:mm") + " to " + EndDt.toString("h:mm") + " Brisbane time)";

        verify( getText().indexOf(TimeString) != -1 );

        // Edit the event and set the timezone for the event to 'Los Angeles'
        Calendar.updateEvent({ TimeZone: TimeZone });
        waitForTitle( "Event Details" );
        select( "Back", softMenu() );
        waitForTitle( "Calendar" );
        verifyImage( "midnight_event_start_day", "", "Verify that the event starts at " + StartDt.time().toString() );
    },

/*
    \req QTOPIA-244
*/
    touchscreen_new_event: function()
    {
        // This test is valid for Greenphone screen resolution, need to modify to support others
        if ( getGeometry().width() != 240 ) {
            skip( "This test relies on Greenphone resolution");
        }

        mouseClick(120, 160);
        waitForTitle( "New Event" );
        compare( getText("Start/Time"), "9:00" );
        select("Back", softMenu());
        waitForTitle( "Calendar" );

        mouseClick(120, 240);
        waitForTitle( "New Event" );
        compare( getText("Start/Time"), "11:00" );
        select("Back", softMenu());
        waitForTitle( "Calendar" );
    },

/*
    \req QTOPIA-253
*/
    link_events_data:
    {
        // Test data expects a name, location, time (both to enter and expected display) and date (both entry and expected display),
        // eg for new event at some location at 11:15 on the 29th March 2007
        // enter [ new event, some location, 1115, 11:15:00, 03292007, Thu Mar 29 2007 ]
        "link_events1": [{
            Name1: "event1", StartDt1: new QDateTime( new QDate( 2007, 3, 29), new QTime(10, 0, 0) ),
            Name2: "event2", StartDt2: new QDateTime( new QDate( 2007, 3, 30), new QTime(10, 0, 0) )
        }]
    },

    link_events: function(testdata)
    {
        Calendar.enterNewEvent({ Description: testdata.Name1, StartDateTime: testdata.StartDt1 });
        Calendar.enterNewEvent({ Description: testdata.Name2, StartDateTime: testdata.StartDt2 });

        // Edit the first event
        Calendar.enterDateKeyClicks( testdata.StartDt1.date() );
        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );
        waitForTitle( "Event Details" );
        select( "Edit event", optionsMenu() );
        waitForTitle( "Edit Event" );

        // Insert link...
        select( "Notes", tabBar() );
        select( "Edit", softMenu() );
        select( "Insert Link", optionsMenu() );
        waitForTitle( "Select Source" );
        select( "Calendar" );
        waitForTitle( "Appointment Picker" );

        // Select the second event...
        Calendar.enterDateKeyClicks( testdata.StartDt2.date() );
        select( "Select", softMenu() );
        waitFor() { return ( getList(softMenu()).contains("Back") ); }
        select( "Select", softMenu() );
        waitForTitle( "Edit Event" );

        // Finished edit, go back to Event Details
        select( "Back", softMenu() );
        waitForTitle( "Event Details" );

        // Confirm that the link to second event is shown
        var pos1 = getText().indexOf(testdata.Name1);
        var pos2 = getText().indexOf(testdata.Name2);
        verify( (pos1 != -1) && (pos2 != -1) && (pos2 > pos1) );
    },

/*
    \req QTOPIA-252
*/
    beaming_an_event: function()
    {
        prompt(
            "Pre-requisite: Two devices that support IrDA, running Qt Extended.\n" +
            "* Device 1: Create a new event, and enter sensible information in all fields.\n" +
            "* Device 2: Select the 'Beaming' application (under Settings) and select the 'Receiver on' option.\n" +
            "* Device 1: While viewing the event details select 'Send' from the context menu.\n" +
            "* Device 1: Verify that a 'Send via...' dialog is displayed.\n" +
            "* Device 1: Select 'VCalendar via Infrared' from the list displayed.\n" +
            "* Device 2: Wait for the beamed event to be received.\n" +
            "* Device 2: Verify that the received event matches the original event.\n"
        );
    },

/*
    \req
*/
    perf_startup: function()
    {
        Performance.testStartupTime("Applications/Calendar");
    }
}
