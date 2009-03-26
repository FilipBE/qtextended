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

//TESTED_COMPONENT=QA: Testing Framework (18707)

testcase = {

/* Tests for functionality implemented entirely on script-side */
/*
    \req QTOPIA-78

    \groups
    String.prototype.contains
*/
    string_contains_data: {
        simple_true:  [ "some haystack", "hay",                        true  ],
        simple_false: [ "some haystack", "hey",                        false ],
        regex_true:   [ "s0me hay5tack", new QRegExp("hay[0-9]ta.k$"), true  ],
        regex_false:  [ "s0me hay5tack", new QRegExp("haysta.k"),      false ],
        fakeregex1:   [ "s0me hay5tack", "hay[0-9]ta.k$",              false  ],
        fakeregex2:   [ "s0me hay5tack", "haysta.k",                   false ],
        date_true1:   [ "01/01/2001",    new QDate(2001, 1, 1),        true ],
        date_true2:   [ "Friday, November 9, 2007", new QDate(2007, 11, 9), true ],
        date_false1:  [ "01/01/2005",    new QDate(2001, 1, 1),        false ]
    },

    string_contains: function(haystack, needle, expected) {
        compare( haystack.contains(needle), expected );
    },

/*
    \req QTOPIA-78

    \groups
    String.prototype.contains
    Basic Qt bindings: QObjects
*/
    qtcore_objects: function() {
        var a = new QTimer;
        compare( a.toString(), "QTimer" );

        var b = 0;
        var timeoutTest = function() {
            b = 1;
        }
        a.timeout.connect(timeoutTest);
        a.start(100);
        wait(500);
        compare(b, 1);
    },

/*
    \req QTOPIA-78

    \groups
    Basic Qt bindings: value-types
*/
    qtcore_values: function() {

        {
            var a = new QDate(2001, 1, 1);
            var e = new QDate(2001, 1, 5);
            compare( a.addDays(4), e );
            verify( a.addDays(5) != e );
        }

        {
            var timer = new QTime;
            timer.start();
            wait(500);
            verify(timer.elapsed() > 400);
        }

        {
            var d = new QDate(2001, 1, 1);
            var t = new QTime(12, 34, 56);
            var dt = new QDateTime(d, t);
            compare( dt.date(), d );
            compare( dt.time(), t );
        }
    },

/*
    \req QTOPIA-78
    Test the include() function works.
*/
    include: function() {
        x = 0; // must be global variable
        include(baseDataPath() + "/increment_x.js");
        compare(x, 1);

        // Ensure we cannot doubly-include
        include(baseDataPath() + "/increment_x.js");
        compare(x, 1);
    }
} // end of test

