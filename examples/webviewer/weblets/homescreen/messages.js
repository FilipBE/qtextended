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

var messages = {

    update: function() {
        if (this.newmessages.value() > 0) {
            if (this.smsfull.value() > 0) {
                this.textdiv.innerHTML = '' + this.newmessages.value() + ' new message(s)<br>SMS Full';
            } else {
                this.textdiv.innerHTML = '' + this.newmessages.value() + ' new message(s)';
            }
            this.messagesdiv.style.display='inline-block';
            this.starttimer();
        } else {
            if (this.smsfull.value() > 0) {
                this.textdiv.textContent = 'SMS Full';
                this.messagesdiv.style.display='inline-block';
                this.starttimer();
            } else {
                this.messagesdiv.style.display='none';
                this.stoptimer();
            }
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
        this.messagesdiv.style.backgroundColor = "rgba(100,200,100," + alpha + ")";
        this.throbvalue++;
        if (this.throbvalue == 50)
            this.throbvalue = 0;
    },

    clicked: function()
    {
        // Figure out what to do
        // Probably launch messaging.
        com.trolltech.services.Messages.viewNewMessages(true);
    },

    init: function() {
        this.messagesdiv = document.getElementById('messages_content');
        this.textdiv = document.getElementById('messages_text');
        this.newmessages = com.trolltech.valuespace('/Communications/Messages/NewMessages');
        this.smsfull = com.trolltech.valuespace('/Telephony/Status/SMSMemoryFull');
        this.newmessages.contentsChanged.connect(this, this.update);
        this.smsfull.contentsChanged.connect(this, this.update);
        this.throbbing = false;
        var oThis = this;
        frameTimer.add(function() {oThis.throb.apply(oThis)});
        this.update();
    }
}

try {
messages.init();
} catch(e){dolog(e);}

