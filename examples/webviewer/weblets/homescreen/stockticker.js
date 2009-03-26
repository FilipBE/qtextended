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

var ticker = {
    anim_timer: undefined,

    // Flags for failed network fetches (to fall back to canned data)
    fallback_gold: false,
    fallback_stocks: false,
    fallback_xrate: false,

    tracked: [ {type:"wsx", key:"^DJI", name:"Dow Jones"},
                {type:"wsx", key:"SPY", name:"S&P"},
                {type:"wsx", key:"INTC", name: "INTC"},
                {type:"wsx", key:"^AORD", name:"All Ords"},
                {type:"wsx", key:"^FTSE", name:"FTSE"},
                {type:"wsx", key:"^N225", name:"Nikkei 225"},
                {type:"xrate", key:"AUD_USD", name:"AUD:USD"},
                {type:"xrate", key:"AUD_GBP", name:"AUD:GBP"},
                {type:"wsx", key:"^HSI", name:"Hang Seng"},
                {type:"lpme", key:"Gold", name:"Gold"},
                {type:"Oil", key:"Oil", name:"Oil"}],

    init: function(divid) {
        var oThis = this;
        frameTimer.add(function() {oThis.updateAnim.apply(oThis)});

        this.stockQueue = [];
        this.exchangeQueue = [];
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
        if (pos < - (this.ticker_width - 2)) {
            pos = this.viewport_width + 2;
        } else {
            pos = pos - 2;
        }
        this.tickerdiv.style.left = pos + 'px';
    },

    // Fetch the gold price
    updateGold: function() {

        // set a loading indicator
        this.updateSingleValue(this.stockidprefix + 'lpme_Gold', 'Gold', true);

        var xhr = new XMLHttpRequest();
        xhr.origThis = this;
        xhr.origId = this.stockidprefix + 'lpme_Gold';
        var url = "http://www.webservicex.net/LondonGoldFix.asmx/GetLondonGoldAndSilverFix?";
        if (this.fallback_gold)
            url = "http://localhost:8080/canned/gold.xml";
        xhr.onreadystatechange = function() {
            if (xhr.readyState == 4) {
                if (xhr.status == 200) {
                    // Parse the responses..
                    // We're interested in the values of the "Gold_AM_USD" and "Gold_PM_USD" elements
                    // this webservice returns an actual XML document, which is nice
                    try {
                        var am = parseFloat(xhr.responseXML.getElementsByTagName('Gold_AM_USD').item(0).textContent);
                        var pm = parseFloat(xhr.responseXML.getElementsByTagName('Gold_PM_USD').item(0).textContent);
                        this.origThis.updateSingleValue(this.origId, 'Gold', false, 'USD' + pm.toFixed(2), pm - am);
                    } catch(e) {
                        dolog('error delving gold price' + ':' + e);
                    }
                } else {
                    // fallback
                    if (this.origThis.fallback_gold != true) {
                        this.origThis.fallback_gold = true;
                        this.origThis.updateGold();
                    }
                }
            }
        };
        xhr.open("get", url);
        xhr.send();
    },


    updateExchangeRates: function() {
        if (this.exchangeQueue.length > 0) {
            // Mark the whole lot as loading
            this.exchangeQueue.forEach(function(a) {this.updateSingleValue(this.stockidprefix + 'xrate_' + a.key, a.name, true);}, this);

            // and load the first item
            var xrate = this.exchangeQueue.pop();
            var tf = xrate.key.split('_');

            var xhr = new XMLHttpRequest();
            xhr.origThis = this;
            xhr.origId = this.stockidprefix + 'xrate_' + xrate.key;
            xhr.origItem = xrate;
            xhr.origTo = tf[1];
            var url = "http://www.webservicex.net/CurrencyConvertor.asmx/ConversionRate?FromCurrency=" + tf[0] + "&ToCurrency=" + tf[1];
            if (this.fallback_xrate)
                url = "http://localhost:8080/canned/exch" + (2 - this.exchangeQueue.length) + ".xml";
            xhr.open("get", url);
            xhr.onreadystatechange = function() {
                if (xhr.readyState == 4) {
                    if (xhr.status == 200) {
                        // Parse the response..
                        try {
                            var val = parseFloat(xhr.responseXML.getElementsByTagName('double').item(0).textContent);
                            var strval;
                            if (val < 10)
                                strval = val.toFixed(4);
                            else if (val < 100)
                                strval = val.toFixed(3);
                            else if (val < 1000)
                                strval = val.toFixed(2);
                            else
                                strval = val.toFixed(1);
                            this.origThis.updateSingleValue(this.origId, this.origItem.name, false, this.origTo + strval);
                        } catch(e) {
                            dolog('error delving currency ' + this.origTo + ':' + e);
                        }
                    } else {
                        if (this.origThis.fallback_xrate == false) {
                            this.origThis.fallback_xrate = true;
                            // re-add this queue entry
                            this.origThis.exchangeQueue.push(this.origItem);
                            this.origThis.updateExchangeRates();
                        }
                    }
                }

                // If we finished, queue up another
                if (xhr.readyState == 4 && this.origThis.exchangeQueue.length > 0) {
                    var othis = this.origThis;
                    window.setTimeout(function() {othis.updateExchangeRates.apply(othis)}, 1000);
                }
            };
            xhr.send();
        }
    },

    // Fetch all stocks from webservicex
    updateStocks: function() {
        if (this.stockQueue.length > 0) {
            // Mark the whole lot as loading
            this.stockQueue.forEach(function(a) {this.updateSingleValue(this.stockidprefix + 'wsx_' + a.key, a.name, true);}, this);

            // Store a mapping from stock symbol to name
            var revmap = [];
            this.stockQueue.forEach(function(a){revmap[a.key.toLowerCase()] = a.name;});

            // and fetch them all
            var xhr = new XMLHttpRequest();
            xhr.origThis = this;
            xhr.origPrefix = this.stockidprefix + 'wsx_';
            var url = "http://www.webservicex.net/stockquote.asmx/GetQuote?symbol=" + this.stockQueue.map(function(a) { return a.key }).join('+');
            if (this.fallback_stocks)
                url = "http://localhost:8080/canned/stocks.xml";
            xhr.open("get", url);
            xhr.onreadystatechange = function() {
                if (xhr.readyState == 4) {
                    if (xhr.status == 200) {
                        // Parse the responses..
                        // We're interested in the values of the "Last" and "Change" elements
                        // this webservice returns a string of XML.. so parse it
                        // Sadly, it isn't escaped properly (e.g. get SPY, it has "S & P DEP RECEIPTS")
                        var parser = new DOMParser();
                        var xmlText = xhr.responseXML.firstChild.firstChild.textContent;
                        xmlText = xmlText.replace('&', '&amp;');
                        var xdoc = parser.parseFromString(xmlText, 'text/xml');

                        var stocks = xdoc.getElementsByTagName('Stock');

                        // Each <Stock> element has <Symbol>XXXXX</Symbol><Last>YYYYYY</Last><Change>ZZZZZ</Change>
                        // content that we are interested in

                        for (var idx = 0; idx < stocks.length; idx++) {
                            try {
                                var symbol = stocks.item(idx).getElementsByTagName('Symbol').item(0);
                                var last = stocks.item(idx).getElementsByTagName('Last').item(0);
                                var change = stocks.item(idx).getElementsByTagName('Change').item(0);
                                this.origThis.updateSingleValue(this.origPrefix + symbol.textContent, revmap[symbol.textContent.toLowerCase()] || "Unk", false, parseFloat(last.textContent), parseFloat(change.textContent));
                            } catch(e) {
                                dolog('error delving ' + idx + ':' + e);
                            }
                        }

                        this.origThis.stockQueue.length = 0;
                    } else {
                        if (this.origThis.fallback_stocks == false) {
                            this.origThis.fallback_stocks = true;
                            this.origThis.updateStocks();
                        }
                    }
                }
            };
            xhr.send();
        }
    },

    updateSingleValue: function(id, name, loading, value, change) {
        var elem = document.getElementById(id);
        var stockname = document.getElementById(id + "_name");
        var stockvalue = document.getElementById(id + "_value");
        var stockimg = document.getElementById(id + "_image");
        var stockdelta = document.getElementById(id + "_delta");

        if (!elem) {
            // Create it
//            dolog('creating ' + name);
            elem = document.createElement("span");
            elem.className = 'stock';
            elem.id = id;

            stockname = document.createElement("span");
            stockname.id = id + "_name";
            stockname.innerText = name;
            elem.appendChild(stockname);
            elem.appendChild(document.createTextNode(': '));

            stockvalue = document.createElement("span");
            stockvalue.id = id + "_value";
            stockvalue.innerText = "loading ";
            elem.appendChild(stockvalue);

            stockimg = document.createElement("img");
            stockimg.id = id + "_image";
            elem.appendChild(stockimg);

            stockdelta = document.createElement("span");
            stockdelta.id = id + "_delta";
            stockdelta.innerText = "";
            elem.appendChild(stockdelta);

            this.tickerdiv.appendChild(elem);
        } else {
//            dolog('updating ' + id + ', name:' + name + ', loading:' + loading);
        }


        if (loading) {
            stockname.className = "stockunk";
            stockdelta.className = "stockdeltaunk";
            stockimg.className = "stockimgunk";
            stockvalue.className = "stockpriceunk";
            stockimg.src = "stockloading.png";
        } else {
            if (typeof value == "undefined") {
                // not loading, but errored somehow
                stockname.className = "stockunk";
                stockdelta.className = "stockdeltaunk";
                stockimg.className = "stockimgunk";
                stockvalue.className = "stockpriceunk";
                stockimg.src = "stockloading.png";
                stockdelta.innerText = "";
                stockvalue.innerText = "?? ";
            } else {
                if (typeof change == "undefined") {
                    stockname.className = "stockup";
                    stockdelta.className = "stockdeltaunk";
                    stockvalue.className = "stockpriceup";
                    stockimg.className = "stockimgnone";
                    stockdelta.innerText = "";
                } else {
                    if (change < 0) {
                        stockname.className = "stockdown";
                        stockdelta.className = "stockdeltadown";
                        stockimg.className = "stockimgdown";
                        stockvalue.className = "stockpricedown";
                        stockimg.src = "stockdown.png";
                    } else {
                        stockname.className = "stockup";
                        stockdelta.className = "stockdeltaup";
                        stockimg.className = "stockimgup";
                        stockvalue.className = "stockpriceup";
                        stockimg.src = "stockup.png";
                    }
                    stockdelta.innerText = change.toFixed(2);
                }

                if (typeof value == "number")
                    stockvalue.innerText = value.toFixed(2);
                else
                    stockvalue.innerText = value;
            }
        }

        // Update the width for scrolling
        this.ticker_width = this.tickerdiv.scrollWidth;
    },

    updateValues: function() {
        // Fetch new values
        this.stockQueue.length = 0;
        this.exchangeQueue.length = 0;
        var dogold = false;
        for (var idx in this.tracked) {
            var spl = this.tracked[idx];
            switch (spl.type) {
                case 'wsx':
                    this.stockQueue.push(spl);
                    break;
                case 'xrate':
                    this.exchangeQueue.push(spl);
                    break;
                case 'lpme':
                    dogold=true;
                    break;

                case 'opec':

                    break;
            }
        }

        // Now process the loads
        this.updateStocks();

        if (dogold)
            this.updateGold();

        this.updateExchangeRates();
    }
};

window.setTimeout("ticker.init('stockticker')", 100);

