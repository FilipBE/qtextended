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

include("log.js");

PerformancePrivate = {
    totalMemory: undefined,

    // called from builtins.js
    initTestCase_global: function()
    {
        /*
           Get the total system memory, so we can figure out the effectively used
           memory later on.

           Measure memory usage for every testfunction, but only when we're
           running on a device.
         */
        if (runsOnDevice()) {
            waitForQtopiaStart();
            PerformancePrivate.totalMemory = getMemoryInfo(TotalMemory);
            gotoHome();

            /*
               Wait a little while, to ensure that the first sample occurs before
               we take any action.
             */
            wait(1000);
            startSamplingMemory(500);
            wait(1000);
        }
    },

    cleanupTestCase_global: function()
    {
        /*
           Measure memory usage for every testfunction and output it in perflogs
           so it can be put in the database.  But, only when we're running on a
           device.
         */
        if (runsOnDevice()) {
            stopSamplingMemory();
            var min_free = getSampledMemoryInfo(EffectiveFreeMemory, SampleMinimum);
            var max_used_file = getSampledMemoryInfo(EffectiveFreeMemory, SampleMinimum|SampleFile);
            var max_used_line = getSampledMemoryInfo(EffectiveFreeMemory, SampleMinimum|SampleLine);
            var max_free = getSampledMemoryInfo(EffectiveFreeMemory, SampleMaximum);

            var min_used = PerformancePrivate.totalMemory - max_free;
            var max_used = PerformancePrivate.totalMemory - min_free;

            var loc = undefined;
            if (max_used_file != "") {
                loc = "At location: " + max_used_file + ":" + max_used_line;
            }

            // FIXME bug 207908: app binary and system test name may not match
            var appname = testCaseName().replace("sys_", "");

            Performance.log("min_used_memory", min_used, appname);
            Performance.log("max_used_memory", max_used, appname, loc);
        }
    },

    /* Special case of testStartupTime. */
    testStartupTime_qpe: function()
    {
        waitForQtopiaStart();

        // Turn on the desired log categories and formatting
        var format = "%t %n(%p): %s";
        Log.enableCategories(["QtopiaServer"]);
        Log.setFormat(format);
        Log.clear();
        setPerformanceTest(true, true);
        waitForQtopiaStart();
        Log.startCapture();
        // Wait for the log messages to stream in
        wait(5000);

        try {
            var main = Log.find(/QtopiaServer *: *begin qpe main/).pop();
            var loop = Log.find({text: /QtopiaServer *: *begin event loop/, pid: main.pid}).pop();
        } catch (e) {
            fail("Could not find qpe main / event loop timestamped messages in log!");
        }

        var type = "_cold";
        if (!runsOnDevice())
            type = "_warm";
        if (loop.timestamp > main.timestamp) {
            Performance.log("event_loop_time" + type, loop.timestamp - main.timestamp, "qpe" );
        }
    }

};


Performance = {

    /*
        Logs a performance value for the performance metric
        identified by identifier.

        app is the application name this value applies to.
        desc is a human-readable descriptive message.
        Both are optional.

        Example:
            Performance.log("click_latency", 120.2, "addressbook");

    */
    log: function(identifier, value, app, desc)
    {
        if (value instanceof Array) {
            for (var i = 0; i < value.length; ++i) {
                Performance.log(identifier, value[i], app, desc);
            }
            return;
        }

        var id = identifier;
        id = id.replace(/[ \(\):]/g, "_");
        if (value instanceof String)
            value = value.replace(/[ \(\):]/g, "_");

        if (app != undefined) {
            app = app.replace(/[ \(\):]/g, "_");
            id = id + ":" + app;
        }

        var out = "QTOPIAPERF(" + id + ") : " + value + ((desc != undefined) ? (" : " + desc) : "");
        print(out);
    },

    /*
        Tests the startup time of the given application.

        displayName is the application's display name for selecting from
        the launcher menu (e.g. "Applications/Contacts").
    */
    testStartupTime: function(displayName)
    {
        waitForQtopiaStart();
        if (displayName == "qpe") {
            return PerformancePrivate.testStartupTime_qpe();
        }
        var WAIT_SETTLE_TIME = runsOnDevice() ? 15000 : 5000;
        var WAIT_RUN_TIME =    runsOnDevice() ? 10000 : 4000;

        // How many times to test warm startup time.
        var REPEAT = 1;

        var baseName = displayName.slice(displayName.indexOf("/")+1);

        // Escaped for use in regular expressions
        var escapedBaseName = baseName;
        escapedBaseName = baseName.replace("/", "\\/");
        var appName;

        var launch;
        var firstDraw;
        var drawCount;
        var eventLoop;
        var linkBegin;
        var linkEnd;

        var event_loop_time_warm  = [];
        var first_paint_time_warm = [];
        var linking_time_warm     = [];
        var paint_count           = [];

        // Turn on the desired log categories and formatting
        var format = "%t %n(%p): %s";
        var categories = [
            "QtUitest",
            "QtopiaServer",
            "Quicklauncher",
            "ApplicationLauncher",
            "UI",
            "DocAPI"
        ];
        Log.enableCategories(categories);
        Log.setFormat(format);

        setPerformanceTest(true, false);
        Log.startCapture();

        gotoHome();

        wait(3*WAIT_SETTLE_TIME);

        for (var i = 0; i < 1 + REPEAT; ++i) {
            Log.clear();

            select(displayName, launcherMenu());

            wait(WAIT_RUN_TIME);

            try {
                launch    = Log.find(/QtUitest *: *about to simulate (key|mouse) click/).pop();
                if (launch == undefined) throw "Missing simulate key/mouse click";

                var execline = Log.find(new RegExp("DocAPI *: *QFSContentEngine::execute \"" + baseName + "\"")).pop();
                if (execline == undefined) throw "Missing DocAPI execute message for " + baseName;
                appName = (new RegExp("DocAPI :  QFSContentEngine::execute \"" + baseName + "\" \"([^\"]+)\"")).exec(execline.text)[1];
                if (appName == undefined) throw "Could not figure out real application name";

                var drawTimes = Log.find(new RegExp(appName + ".*expose_region"));
                drawCount = drawTimes.length;
                if (drawTimes.length == 0) throw "Missing paint messages";

                firstDraw = drawTimes.shift();

                eventLoop = Log.find(new RegExp("Quicklauncher *: *begin event loop " + appName)).shift();
                if (eventLoop == undefined) throw "Missing begin event loop";

                linkBegin = Log.find({text:/Quicklauncher *: *begin library loading/, pid:eventLoop.pid}).shift();
                linkEnd   = Log.find({text:/Quicklauncher *: *end library loading/, pid:linkBegin.pid}).shift();
                if (linkBegin == undefined || linkEnd == undefined) throw "Missing library loading";

                launch    = launch.timestamp;
                firstDraw = firstDraw.timestamp;
                eventLoop = eventLoop.timestamp;
                linkBegin = linkBegin.timestamp;
                linkEnd   = linkEnd.timestamp;
            } catch (e) {
                fail("Could not find one or more expected log messages for " + displayName + "\n" + e);
            }

            verify( launch > 0,    "Failed to simulate key/mouse click to launch app" );
            verify( firstDraw > 0, "Failed to find first paint message for app" );
            verify( drawCount > 0, "Failed to find paint messages for app" );
            verify( eventLoop > 0, "Failed to find event loop message for app" );
            verify( linkBegin > 0, "Failed to find link begin message for app" );
            verify( linkEnd > 0,   "Failed to find link end message for app" );

            var type = (i == 0) ? "cold" : "warm";

            if ((eventLoop - launch) > 0.1) {
                if (type == "cold") {
                    Performance.log("event_loop_time_cold", eventLoop - launch, appName );
                } else {
                    event_loop_time_warm.push(eventLoop - launch);
                }
            }
            if ((firstDraw - launch) > 0.1) {
                if (type == "cold") {
                    Performance.log("first_paint_time_cold", firstDraw - launch, appName );
                } else {
                    first_paint_time_warm.push(firstDraw - launch);
                }
            }
            if ((linkEnd - linkBegin) > 0.1) {
                if (type == "cold") {
                    Performance.log("linking_time_cold", linkEnd - linkBegin, appName );
                } else {
                    linking_time_warm.push(linkEnd - linkBegin);
                }
            }

            paint_count.push(drawCount);

            gotoHome();
            wait(WAIT_SETTLE_TIME);
        }

        // If we have enough data, use that data to throw away extremes.
        if (REPEAT > 5) {
            // Throw away the spikes, determined by a value > factor times the min.
            function cmp(a, b) { return a - b; }
            function removeExtremes(list, factor) {
                list.sort(cmp);
                while (list[list.length-1] > factor*list[0]) {
                    list.pop();
                }
                return list;
            }
            event_loop_time_warm  = removeExtremes(event_loop_time_warm,  2.5);
            first_paint_time_warm = removeExtremes(first_paint_time_warm, 2.5);
            linking_time_warm     = removeExtremes(linking_time_warm,     2.5);

            // paint count should be constant except for freak occurrences.
            // Unfortunately, since this means the standard deviation is typically 0,
            // even a single "freak occurrence" can set off a regression detection
            // system for weeks.  Prevent this by throwing away the top and bottom
            // two.
            paint_count.sort(cmp);
            paint_count.pop();   paint_count.pop();
            paint_count.shift(); paint_count.shift();
        }

        Performance.log("event_loop_time_warm",  event_loop_time_warm,  appName);
        Performance.log("first_paint_time_warm", first_paint_time_warm, appName);
        Performance.log("linking_time_warm",     linking_time_warm,     appName);
        Performance.log("paint_count",           paint_count,           appName);
    }
    
};

