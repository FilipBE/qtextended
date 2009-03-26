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

var news = {
    tracked: [ ],

    fallback_news: false,

    init: function(divid) {
        var oThis = this;
        frameTimer.add(function() {oThis.updateAnim.apply(oThis)});

        this.divid = divid;
        this.stockidprefix = this.divid + '_value_';
        this.tickerdiv = document.getElementById(divid + '_contents');
        var st = window.getComputedStyle(this.tickerdiv, "");
        this.ticker_width = this.tickerdiv.scrollWidth;
        this.viewport_width = parseInt(window.getComputedStyle(document.getElementById(divid),"").width);

        this.updateValues();
    },

    updateAnim: function() {
        // get the position, scroll to the left
        var pos = parseInt(this.tickerdiv.style.left);
        if (pos < - (this.ticker_width - 3)) {
            pos = this.viewport_width + 3;
        } else {
            pos = pos - 3;
        }
        this.tickerdiv.style.left = pos + 'px';
    },

    addSingleValue: function(source, title, link, body) {

        // ignore source for now
            // Create a new new item
        var elem = document.createElement("span");
        elem.className = 'newsitem';

        // add a bullet
        var bullet = document.createElement("img");
        bullet.src = "newsbullet.png";
        bullet.className = 'newsbullet';
        elem.appendChild(bullet);

        // we want something like <a href=link>title</a>
        var anchor = document.createElement("a");
        if (link && link != '')
            anchor.href = link;
        anchor.innerHTML = title;
        anchor.className = 'newslink'
        elem.appendChild(anchor);
        this.tickerdiv.appendChild(elem);

        // Update the width for scrolling
        this.ticker_width = this.tickerdiv.scrollWidth;
    },

    getNewsFeed: function(source, url) {
        var xhr = new XMLHttpRequest();
        xhr.origThis = this;
        xhr.origSource = source;
        xhr.origUrl = url;
        xhr.open("get", url);
        xhr.onreadystatechange = function() {
            if (xhr.readyState == 4) {
                if (xhr.status == 200) {
                    // 'parse' the RSS
                    var items = xhr.responseXML.getElementsByTagName('item');

                    for (var idx = 0; idx < items.length; idx++) {
                        var item = items(idx);
                        var title = item.getElementsByTagName('title');
                        var link = item.getElementsByTagName('link');
                        var desc = item.getElementsByTagName('description');

                        var titletext = title.item(0) ? title.item(0).textContent : '';
                        var linktext = link.item(0) ? link.item(0).textContent : '';
                        var desctext = desc.item(0) ? desc.item(0).textContent : '';
                        this.origThis.addSingleValue(this.origSource, titletext, linktext, desctext);
                    }
                } else {
                    if (this.origThis.fallback_news == false) {
                        this.origThis.fallback_news = true;
                        this.origThis.getNewsFeed(this.origSource, "http://localhost:8080/canned/news.xml");
                    }
                }
            }
        };

        xhr.send();
    },

    updateValues: function() {
        // this.getNewsFeed('fi', 'http://www.hs.fi/uutiset/rss/');
        // this.getNewsFeed('elmundo', 'http://rss.elmundo.es/rss/descarga.htm?data2=4');
        this.getNewsFeed('slashdot', 'http://rss.slashdot.org/Slashdot/slashdot');
    }
};

window.setTimeout("news.init('newsticker')", 100);
