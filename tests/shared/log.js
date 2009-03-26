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

/*
    Log handling functions for QtUiTest.
*/

LogPrivate = {
    capture:            false,
    output:             false,
    handlerInstalled:   false,

    // Handler for log testmessages.
    handler: function(event, data) {
        // Discard the event if logging is turned off.
        if (!LogPrivate.capture && (event == "log" || event == "logError")) {
            return true;
        }

        if (event == "log") {
            LogImpl.addMessages(data.lines);
            if (LogPrivate.output) {
                for (var i = 0; i < data.lines.length; ++i)
                    print(data.lines[i]);
            }
            return true;
        }
        else if (event == "logError") {
            fail("An error occurred while processing log messages: " + data.error);
            return true;
        }
        return false;
    },

    setCategories: function(on, categories) {
        for (var i = 0; i < categories.length; ++i)
            setSetting("Trolltech", "Log", categories[i], "Enabled", on);
    },

    logConfChanged: function() {
        ipcSend("QPE/System", "LogConfChanged()");
    }
};

Log = {
    /*
        Set the maximum amount of log messages which will be buffered.
        A circular buffer is maintained.
        When the amount of messages exceeds \a limit, the earliest stored messages will be
        discarded.
    */
    setBufferSize: function(limit) {
        LogImpl.setBufferSize(limit);
    },

    /*
        Start capturing log messages from the test system.
    */
    startCapture: function() {
        Log.clear();

        LogPrivate.capture = true;

        waitForQtopiaStart();
        if (runsOnDevice()) {
            sendRaw("startLogRead", { commands: [
                // Default command covers any device which has logread or which writes logs to a
                // file whose path is in QTOPIA_LOG.
                // If it becomes necessary to support other configurations, this should be moved
                // to something like logsettings.js in tests/shared under device profiles.
                "/bin/sh -c \""                                 +
                    "if [ x$QTOPIA_LOG != x ]; then "           +
                        "tail -n500 --follow=name $QTOPIA_LOG; "+
                    "else "                                     +
                        "logread | tail -n500; logread -f; "    +
                    "fi"                                        +
                "\""
            ] });
        }
        // When on the desktop, Qtopia is launched directly by qtuitestrunner, so reading the log
        // can be done more directly.
        else {
            startDirectLogRead();
        }

        // Already installed?
        if (LogPrivate.handlerInstalled)
            return;

        LogPrivate.handlerInstalled = true;
        installMessageHandler(LogPrivate.handler);
    },

    /*
        Stop capturing log messages from the test system.
    */
    stopCapture: function() {
        LogPrivate.capture = false;
    },

    /*
        Set whether log messages from the test system will be \a output to the testcase log.
    */
    setOutput: function(output) {
        LogPrivate.output = output;
    },

    /*
        Clear the buffer of log messages.
    */
    clear: function() {
        LogImpl.clear();
    },

    /*
        Set the expected formatting of log messages.
        This will be used to parse the retrieved log messages.
    */
    setFormat: function(format) {
        setSetting("Trolltech", "Log2", "MessageHandler", "Format", format);
        LogPrivate.logConfChanged();
        LogImpl.setFormat(format);
    },

    /*
        Ensure the specified qLog categories are turned on in Qtopia.
    */
    enableCategories: function(categories) {
        LogPrivate.setCategories(1, categories);
        LogPrivate.logConfChanged();
    },

    /*
        Ensure the specified qLog categories are turned off in Qtopia.
    */
    disableCategories: function(categories) {
        LogPrivate.setCategories(0, categories);
        LogPrivate.logConfChanged();
    },

    /*
        Find and return all log messages which match the given \a expression.

        Returned is an array of message objects, each with the following properties:
        \list
        \o raw: the full raw text of the log message.
        \o text: the text of the message body.
        \o timestamp: monotonic timestamp, in milliseconds, attached to the log message.
        \o pid: process identifier of the process which generated the log message.
        \o application: name of the application which generated the log message.
        \endlist

        All of the properties other than \c raw are only available if the log message can be
        parsed according to the currently set log format.

        \a expression may be:
        \list
        \o  A string or regular expression: find log messages with text which exactly matches the
            string or matches the regular expression.
        \o  An object with one or more string, number or regular expression properties: find log
            messages whose properties match the properties in \a expression.
        \endlist

        Example:
        \code
            // Find the most recent key event processed by MyWidget
            var keyevent = Log.find(/MyWidget: got key event/).pop();

            // Now find the first message from the same process for
            // completing paint of the calendar view
            var paintevent = Log.find({text: /painted calendar/, pid: keyevent.pid}).shift();

            // Calculate how long it took to paint the calendar after the key click.
            // If neither of the log messages were found, the test case will simply fail.
            var interval = paintevent.timestamp - keyevent.timestamp;
            // Ensure we painted in less than 800 milliseconds.
            verify( interval < 800, "Unacceptable painting performance!" );
        \endcode
    */
    find: function(expression) {
        var ret = [];

        var matches = function(message, expression) {
            var submatches = function(thing, expr) {
                thing = String(thing);
                if (expr instanceof RegExp) {
                    return (null != thing.match(expr));
                }
                return thing == String(expr);
            }

            // If it's not an object or regexp, do a stringwise compare.
            if (!(expression instanceof Object || expression instanceof RegExp)) {
                expression = new String(expression);
            }

            if (expression instanceof String || expression instanceof RegExp) {
                return submatches(message.text, expression);
            }

            // If we get here, we have an object.  Try matching each property.
            var ret = true;
            for (var prop in expression) {
                try {
                    ret = submatches(message[prop], expression[prop]);
                } catch(e) {
                    ret = false;
                }
                if (!ret) break;
            }

            return ret;
        } // matches

        var count = LogImpl.count();
        for (var i = 0; i < count; ++i) {
            var msg = LogImpl.message(i);
            if (matches(msg, expression)) {
                ret.push(msg);
            }
        }

        return ret;
    }
};

