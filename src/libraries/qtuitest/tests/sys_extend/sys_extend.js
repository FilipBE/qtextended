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

/* Test that extending the QtUiTest API purely from script works OK. */

//TESTED_COMPONENT=QA: Testing Framework (18707)

testcase = {
    initTestCase: function() {
        waitForQtopiaStart();
    },

    handler: function() {
        var loglines = [];
        var myhandler = function(thing, data) {
            if (thing == "log") {
                for (var i = 0; i < data.lines.length; ++i)
                    loglines.push(data.lines[i]);
                return true;
            }
            return false;
        }

        installMessageHandler(myhandler);

        sendRaw("startLogRead", { commands: [ "echo foo", "echo bar" ] });
        wait(5000);
        compare(loglines, [ "foo", "bar" ]);
    }
} // end of test

