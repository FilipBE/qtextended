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

var calls = {

    update: function() {
        if (this.newcalls.value() > 0) {
            this.textdiv.innerHTML = '' + this.newcalls.value() + ' missed call(s)';
            this.callsdiv.style.display='inline-block';
            this.starttimer();
        } else {
            this.stoptimer();
            this.callsdiv.style.display='none';
        }
    },

    starttimer: function() { 
        this.throbbing = true;
        this.throbvalue = 25;
    },

    stoptimer: function() {
        this.throbbing = false;
    },


    throbvalue: 25,
    throb: function() {
        // go from 0 to 50, which is 25 to 100% alpha and back again
        var alpha = 0.5 + 0.5 * (this.throbvalue <= 25 ? (this.throbvalue / 25) : (2 - (this.throbvalue / 25)));
        this.callsdiv.style.backgroundColor = "rgba(200,100,100," + alpha + ")";
        this.throbvalue++;
        if (this.throbvalue == 50)
            this.throbvalue = 0;
    },

    clicked: function()
    {
        com.trolltech.services.CallHistory.showCallHistory(3);// 3 == missed calls
    },

    init: function() {
        this.newcalls = com.trolltech.valuespace('/Communications/Calls/MissedCalls');
        this.callsdiv = document.getElementById('calls_content');
        this.textdiv = document.getElementById('calls_text');
        this.newcalls.contentsChanged.connect(this, this.update);
        this.throbbing = false;
        var oThis = this;
        frameTimer.add(function() {oThis.throb.apply(oThis)});

        this.update();
    }
}

try {
calls.init();
} catch(e){dolog(e);}

