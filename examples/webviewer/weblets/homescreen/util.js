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

document.write("<div id='logwindow' style='position:fixed; top: 25%; left 20%; width:90%; height:50%; color: white; overflow:auto; z-index:10'></div>");
function dolog(x) {
    var lx = document.getElementById('logwindow');
    lx.innerHTML = lx.innerHTML + "<br>" + x;
};

function do_windowShade(divid)
{
    var d = document.getElementById(divid + '_details_background');
    var t = document.getElementById(divid + '_title_background');
    var i = document.getElementById(divid + '_windowshade_img');
    if (d.style.visibility == 'hidden') {
        t.className = 'roundedtop';
        d.style.visibility = 'visible';
        i.src = '3rdparty/list-remove.png';
    } else {
        d.style.visibility = 'hidden';
        t.className = 'rounded';
        i.src = '3rdparty/list-add.png';
    }
}

// Centralized animation timer thing
var frameTimer = {
    init: function() {
        var othis = this;
        this.timerId = window.setInterval(function() {othis.timerTick.apply(othis)}, 50);
        this.callbacks = [];
    },

    timerTick: function() {
        for (var t in this.callbacks) {
            this.callbacks[t]();
        }
    },

    add: function(f) {
        if (typeof f == "function") {
            this.callbacks.push(f);
        } else {
            try {
                var g = eval(f);
                if (typeof g == "function")
                    this.callbacks.append(g);
            } catch(e) {}
        }
    }

};

frameTimer.init();


// Hack for using moz/konq/saf etc
if (typeof com == "undefined") {
    com = new Object();
}

if (typeof com.trolltech == "undefined") {
    com.trolltech = new Object();
    com.trolltech.services = new Object();
    com.trolltech.valuespace = new Object();
    com.trolltech.appointments = new Function('return {modelReset: {connect: function(){}}, rowCount: function() {return 0;}}');
    com.trolltech.locationChanged = new Object();
    com.trolltech.locationChanged.connect = new Function('x','y', "return;");
    com.trolltech.weatherChanged = new Object();
    com.trolltech.weatherChanged.connect = new Function('x','y', "return;");
    com.trolltech.currentLocation = new Function("return {suburb:'Demoville', postcode: '1234'}");
    com.trolltech.weather = new Function("return {high:33, low:17, now:28, conditions:'heavyrain'};");
}
