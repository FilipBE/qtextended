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

/* Test for tests/shared/log.js */

//TESTED_COMPONENT=QA: Testing Framework (18707)

include("log.js");

testcase = {
    initTestCase: function() {
        waitForQtopiaStart();
    },

    // Bug 231961: qtuitestrunner uses up all memory, then dies.
    // Must be the first testfunction in order to reproduce the bug.
    bug231961: function() {
        startApplication("testapp1");
        Log.setBufferSize(5000);

        prompt("Observe and record the memory usage (RSS) of qtuitestrunner.");

        // Generate lots of log messages.  The bug is that, since we aren't capturing
        // log messages yet, the buffer holding them expands indefinitely.
        for (var i = 0; i < 15; ++i) {
            select("Lots o' Logs");
        }

        prompt("Verify that the current memory usage (RSS) of qtuitestrunner is "
            + "not more than 5MB greater than the previously recorded value.");

        Log.startCapture();
        select("Lots o' Logs");
        wait(2000);

        // Verify that we can find messages from both before and after the startCapture()
        var found = Log.find({raw: /A log message 14-3999/});
        compare(found.length, 1);
        found = Log.find({raw: /A log message 15-3999/});
        compare(found.length, 1);
    },

    log: function() {
        Log.setOutput(true);
        Log.startCapture();
        wait(2000);

        prompt("Verify that the current memory usage (RSS) of qtuitestrunner is within "
            + "appropriate limits.");
        prompt("Verify that log messages from Qtopia were output to the test log.");

        Log.stopCapture();
    },

    parse: function(raw, format, expectedParsed) {
        Log.setFormat(format);
        Log.clear();
        LogImpl.addMessages([raw]);
        var parsed = LogImpl.message(0);

        for (var prop in expectedParsed) {
            compare(parsed[prop], expectedParsed[prop]);
        }
    },

    parse_data: {
        simple: [
            "Hi there",
            "%s",
            { text: "Hi there", raw: "Hi there" }
        ],
        prefix: [
            "Hi there",
            "Hi %s",
            { text: "there", raw: "Hi there" }
        ],
        full:   [
            "somestring 12345 myapp(88): Nice day today.",
            "%t %n(%p): %s",
            {   raw: "somestring 12345 myapp(88): Nice day today.",
                text: "Nice day today.",
                timestamp: 12345,
                pid: 88,
                application: "myapp"
            }
        ]
    },

    find: function() {
        Log.clear();
        Log.setOutput(false);
        Log.setFormat("%t %n(%p): %s");
        LogImpl.addMessages(
            [ "0 qpe(123): starting"
            , "10 qpe(123): doing foo"
            , "75 qpe(123): doing bar"
            , "85 qpe(125): child doing baz"
            , "250 mediaserver(200): doing something neat"
            , "unformatted message from rootfs daemon"
            , "510 mediaserver(200): done something neat"
            , "1200 addressbook(220): creating a contact"
            , "another raw message"
            , "1310 addressbook(220): done creating a contact"
        ]);

        var found;



        found = Log.find("starting");
        compare(found.length, 1);
        compare(found[0].raw,           "0 qpe(123): starting");
        compare(found[0].text,          "starting");
        compare(found[0].timestamp,     0);
        compare(found[0].pid,           123);
        compare(found[0].application,   "qpe");



        found = Log.find(/doing/);
        compare(found.length, 4);
        compare(found[0].raw,           "10 qpe(123): doing foo");
        compare(found[0].text,          "doing foo");
        compare(found[0].timestamp,     10);
        compare(found[0].pid,           123);
        compare(found[0].application,   "qpe");

        compare(found[1].raw,           "75 qpe(123): doing bar");
        compare(found[1].text,          "doing bar");
        compare(found[1].timestamp,     75);
        compare(found[1].pid,           123);
        compare(found[1].application,   "qpe");

        compare(found[2].raw,           "85 qpe(125): child doing baz");
        compare(found[2].text,          "child doing baz");
        compare(found[2].timestamp,     85);
        compare(found[2].pid,           125);
        compare(found[2].application,   "qpe");

        compare(found[3].raw,           "250 mediaserver(200): doing something neat");
        compare(found[3].text,          "doing something neat");
        compare(found[3].timestamp,     250);
        compare(found[3].pid,           200);
        compare(found[3].application,   "mediaserver");



        // find by pid
        found = Log.find({pid: 220});
        compare(found.length, 2);
        compare(found[0].raw,           "1200 addressbook(220): creating a contact");
        compare(found[0].text,          "creating a contact");
        compare(found[0].timestamp,     1200);
        compare(found[0].pid,           220);
        compare(found[0].application,   "addressbook");

        compare(found[1].raw,           "1310 addressbook(220): done creating a contact");
        compare(found[1].text,          "done creating a contact");
        compare(found[1].timestamp,     1310);
        compare(found[1].pid,           220);
        compare(found[1].application,   "addressbook");



        // find by multiple
        found = Log.find({pid: 220, text: /done/});
        compare(found.length, 1);

        compare(found[0].raw,           "1310 addressbook(220): done creating a contact");
        compare(found[0].text,          "done creating a contact");
        compare(found[0].timestamp,     1310);
        compare(found[0].pid,           220);
        compare(found[0].application,   "addressbook");



        // Can only find unformatted messages by using 'raw'
        found = Log.find(/raw message/);
        compare(found.length, 0);
        found = Log.find({raw: /raw message/});
        compare(found[0].raw,           "another raw message");
        compare(found[0].text,          undefined);
        compare(found[0].timestamp,     undefined);
        compare(found[0].pid,           undefined);
        compare(found[0].application,   undefined);
    },

    // Test that CPU / memory usage is OK with a lot of messages.
    many: function() {
        var a_lot = 10000;

        Log.setBufferSize(a_lot);
        Log.clear();
        Log.setOutput(false);
        Log.setFormat("%t %n(%p): %s");
        for (var i = 0; i < a_lot/2; ++i) {
            LogImpl.addMessages(
                [ "0 qpe(123): starting"
                , "10 qpe(123): doing foo"
                , "a non matching message"
            ]);
        }

        prompt("Verify that the current memory usage (RSS) of qtuitestrunner is within "
            + "appropriate limits.");
    }
} // end of test

