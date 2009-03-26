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
    "Builtin" functions available to all tests.
    This file gets evaluated once at the beginning of every test.

    Documentation for these functions should be placed in qsystemtest.cpp.

    Note, if a function exists with the same name in both here and QSystemTest,
    the QtScript version takes precedence.  To force a call to the QSystemTest
    function, use ParentTestObject[signature], e.g. see expectMessageBox.
*/

include("performance.js");

/* called before every init() */
function init_global()
{
}

/* called after every cleanup() */
function cleanup_global()
{
    ParentTestObject['waitExpectedMessageBox(int,bool,QString,QString)'](500,true,"","");
}

/* called before every initTestCase() */
function initTestCase_global()
{
    PerformancePrivate.initTestCase_global();
}

/* called after every cleanupTestCase() */
function cleanupTestCase_global()
{
    PerformancePrivate.cleanupTestCase_global();
    ParentTestObject['shutdownQtopia()']();
}

function verify() {
    if (arguments.length == 0) {
        fail( "You need to specify at least a boolean expression" );
        return false;
    }
    var msg = "";
    if (arguments.length > 1) { msg = arguments[1] };
    return ParentTestObject['verify(bool,QString)'](arguments[0],msg);
}

function print() {
    var str = "";
    for (var i = 0; i < arguments.length; ++i) {
        if (arguments[i] != undefined) str = str + arguments[i];
    }
    ParentTestObject['print(QVariant)'](str);
}

function waitForQtopiaStart() {
    ParentTestObject['waitForQtopiaStart()']();
    var WAIT_TEXT = "Please Wait...";
    var MAXTIME   = 60000;

    var text = getText();
    for (var i = 0; i < MAXTIME && text.startsWith(WAIT_TEXT); wait(1000), i += 1000) {
        text = getText();
    }
    if (text.startsWith(WAIT_TEXT)) {
        fail("Qtopia started, but timed out after waiting " + (MAXTIME/1000) +
             " seconds for \"" + WAIT_TEXT + "\" to disappear from the screen.");
    }
}

/* Expect conditionFunction to return true after executing code, or fail with message */
function _expect(conditionFunction, message) {
    if (message == undefined) {
        message = "Expected condition did not become true";
    }
    this.__defineSetter__("code", function(code){
        try {
            code.apply(this);
        } catch (e) {
            if (this.onFail != undefined) {
                this.onFail.apply(this);
            }
            throw e;
        }
        if (!conditionFunction.call()) {
            fail(message);
        }
    });
}

function expect(a,b) {
    return new _expect(a,b);
}

function expectMessageBox(title, text, option, timeout) {
    ParentTestObject['addExpectedMessageBox(QString,QString,QString)'](title, text, option);
    if (timeout == undefined) {
        timeout = 10000;
    }
    var ret = expect(function() { return waitExpectedMessageBox(timeout, false, title, text); }, "Message box '" + title + "' did not appear");
    ret.onFail = function() { clearExpectedMessageBox(title, text); };
    return ret;
}

function enter(text, queryPath, mode) {
    if (text != undefined) {
        if (queryPath == undefined) queryPath = "";
        if (text instanceof QDate)
            ParentTestObject['enter(QDate,QString,EnterMode)'](text, queryPath, mode);
        else if (text instanceof QTime)
            ParentTestObject['enter(QTime,QString,EnterMode)'](text, queryPath, mode);
        else
            ParentTestObject['enter(QString,QString,EnterMode)'](text, queryPath, mode);
    }
    return true;
}

function select( item, queryPath ) {
    if (item != undefined) {
        if (queryPath == undefined) queryPath = "";
        ParentTestObject['select(QString,QString)'](item, queryPath);
    }
    return true;
}

function skip(text, mode) {
    ParentTestObject['skip(QString,QSystemTest::SkipMode)'](text, mode);
}

/*
    Wait for code to return true for up to timeout ms, with given number of intervals,
    and message on failure
*/
function _waitFor(timeout, intervals, message) {
    if (timeout == undefined) {
        timeout = 10000;
    }
    if (intervals == undefined) {
        intervals = 20;
    }
    if (message == undefined) {
        message = "Waited-for condition did not become true after " + timeout + " milliseconds";
    }
    this.__defineSetter__("code", function(code){
        ok = code.apply(this);
        i = 0;
        while (!ok && i < intervals) {
            wait(timeout/intervals);
            ok = code.apply(this);
            i = i + 1;
        }
        if (!ok) {
            fail(message);
        }
    });
}
function waitFor(a,b,c) {
    return new _waitFor(a,b,c);
}


Array.prototype.isEmpty = function()
{
    return (this.length <= 0);
}

Array.prototype.contains = function()
{
    obj = arguments[0];
    for (var i=0; i<this.length; i++) {
        if (this[i] == obj) {
            return true;
        }
    }
    return false;
}

// Ensure we don't get the 'contains' function when using for-in on arrays
setFlags(Array.prototype, "contains", QScriptValue.SkipInEnumeration);

Array.prototype.containsOne = function()
{
    obj = arguments[0];
    var count = 0;
    for (var i=0; i<this.length; i++) {
        if (this[i] == obj) {
            count++;
        }
    }
    return count == 1;
}
// Ensure we don't get the 'containsOne' function when using for-in on arrays
setFlags(Array.prototype, "containsOne", QScriptValue.SkipInEnumeration);

String.prototype.contains = function()
{
    var obj = arguments[0];
    if (arguments[1] != undefined) {
        obj = arguments[1];
    }
    if (obj instanceof Date || obj instanceof QDate) {
        var strs = [];
        strs.push(obj.toString("ddd, M/d/yy"));
        strs.push(obj.toString("dddd, MMMM d, yyyy"));
        strs.push(obj.toString("ddd, d MMM"));
        strs.push(obj.toString("dd/MM/yyyy"));
        for (var i = 0; i < strs.length; ++i) {
            if (this.toString().contains(strs[i])) return true;
        }
        return false;
    } else if (obj instanceof RegExp) {
        return obj.test(this.toString());
    } else if (obj instanceof QRegExp) {
        return (-1 != obj.indexIn(this.toString()));
    } else {
        var ref = this.toString();
        return (-1 != ref.indexOf(obj));
    }
}

String.prototype.toDate = function()
{
    var obj = this.toString();
    return toDate(obj);
}

String.prototype.startsWith = function()
{
    var obj = arguments[0];
    var ref = this.toString();
    return ref.indexOf( obj ) == 0;
}

String.prototype.left = function()
{
    var start = 0;
    var end = arguments[0];
    var ref = this.toString();
    return ref.slice( start, end );
}

String.prototype.right = function()
{
    var ref = this.toString();
    var start = ref.length - arguments[0];
    return ref.slice( start );
}

String.prototype.mid = function()
{
    var start = arguments[0];
    var end = arguments[1]+1;
    var ref = this.toString();
    return ref.slice( start, end );
}

String.prototype.toValue = function()
{
    var ret = 0;
    var ref = this.toString();
    for (var i=0; i<ref.length; i++) {
        var ch = ref.charAt(i);
        if (ch == '-') {
            ret = ret * -1;
        } else {
            ret = ret * 10;
            if (ch == '0') {
            } else if (ch == '1') {
                ret = ret + 1;
            } else if (ch == '2') {
                ret = ret + 2;
            } else if (ch == '3') {
                ret = ret + 3;
            } else if (ch == '4') {
                ret = ret + 4;
            } else if (ch == '5') {
                ret = ret + 5;
            } else if (ch == '6') {
                ret = ret + 6;
            } else if (ch == '7') {
                ret = ret + 7;
            } else if (ch == '8') {
                ret = ret + 8;
            } else if (ch == '9') {
                ret = ret + 9;
            } else {
                fail( "ERROR: Couldn't convert '" + ref + "' into a value." );
                return undefined;
            }
        }
    }
    return ret;
}

function anyDefined()
{
    for (var i = 0; i < arguments.length; ++i) {
        if (arguments[i] != undefined) return true;
    }
    return false;
}

