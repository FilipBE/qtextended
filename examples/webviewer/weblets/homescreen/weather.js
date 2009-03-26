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

var weather = {

    // Adjust this table to remove or add clouds/suns etc
    image_dims: [
//                    {zidx:-98, size:'100%', top:'0%'},
                    {zidx:-97, size:'40%' , top:'27%'},
//                    {zidx:-95, size:'10%', top:'40%'} 
                ],

    anim_timer: undefined,
    wind_timer: undefined,
    pos: [],
    divs: [],
    widths: [],
    scales: [],

    // Most of these weather icons came from the
    // Tango Desktop Project (http://tango.freedesktop.org)
    // A small number have been modified
    weathertable: {
        "smoky" :       {img: "3rdparty/hazy.png", txt: "Smoky"},   // Doesn't exist
        "hazy" :        {img: "3rdparty/hazy.png", txt: "Hazy"},    // Doesn't exist
        "unknown" :     {img: "3rdparty/unknown.png", txt: "Unknown"}, // Doesn't exist

        "fine" :        {img: "3rdparty/weather-clear.png", txt: "Fine"},   // Tango
        "clear" :       {img: "3rdparty/weather-clear.png", txt: "Clear"},  // Tango
        "sunny" :       {img: "3rdparty/weather-clear.png", txt: "Sunny"},  // Tango
        "hot" :         {img: "3rdparty/weather-clear.png", txt: "Hot"},    // Tango
        "partlycloudy" :{img: "3rdparty/weather-few-clouds.png", txt: "Partly Cloudy"}, // Tango
        "cloudy" :      {img: "3rdparty/weather-overcast.png", txt: "Cloudy"},  // Tango
        "drizzle" :     {img: "3rdparty/weather-showers-scattered.png", txt: "Drizzle"}, // Tango
        "showers" :     {img: "3rdparty/weather-showers.png", txt: "Showers"},  // Tango
        "rain" :        {img: "3rdparty/weather-showers.png", txt: "Rain"},     // Tango
        "heavyrain" :   {img: "3rdparty/weather-storm.png", txt: "Heavy rain"}, // Tango
        "stormy" :      {img: "3rdparty/weather-storm.png", txt: "Storms"},     // Tango
        "lightsnow" :   {img: "3rdparty/weather-snow.png", txt: "Light Snow"},  // Tango
        "snow" :        {img: "3rdparty/weather-snow.png", txt: "Snow"},        // Tango
        "heavysnow" :   {img: "3rdparty/weather-snow.png", txt: "Heavy Snow"},  // Tango
        "blizzard" :    {img: "3rdparty/weather-snow.png", txt: "Blizzard"},    // Tango
        "sleet" :         {img: "3rdparty/weather-sleet.png", txt: "Sleet"},    // Tango
        "hail" :         {img: "3rdparty/weather-hail.png", txt: "Hail"},   // Tango
        "rainandsnow" : {img: "3rdparty/weather-rainsnow.png", txt: "Rain and snow"}, // Tango
        "windy" :       {img: "3rdparty/image-x-generic-modified.png", txt: "Windy"}, // Tango
        "cyclone" :     {img: "3rdparty/dialog-warning.png", txt: "Cyclone"},   // Tango
        "hurricane" :   {img: "3rdparty/dialog-warning.png", txt: "Hurricane"}, // Tango

        "icy" :         {img: "3rdparty/Snow01.png", txt: "Icy"}, // public domain image ("Snow01.svg") from wikimedia, (Nicu B. at nicubunu.ro)
    },

    init: function(divid) {
        var oThis = this;
        frameTimer.add(function() {oThis.updateAnim.apply(oThis)});

        if (this.wind_timer == undefined) {
            this.wind_timer = window.setInterval(function() {oThis.updateWind.apply(oThis)}, 5000);
        }
        this.divid = divid;
        this.viewport_width = parseInt(window.getComputedStyle(document.getElementById(divid),"").width);

        var containerdiv = document.getElementById(divid);
        // Create our images
        for (var i=0; i < this.image_dims.length; i++) {
            var newimg = document.createElement("img");
            newimg.style.position = 'absolute';
            newimg.style.left = '0px';
            newimg.style.top = this.image_dims[i].top;
            newimg.style.width = this.image_dims[i].size;
            newimg.style.height = this.image_dims[i].size;
            newimg.style.zIndex = this.image_dims[i].zidx;

            containerdiv.appendChild(newimg);

            this.divs[i] = newimg;
            this.widths[i] = this.divs[i].scrollWidth;
            this.pos[i] = Math.random() * this.viewport_width;
            this.scales[i] = parseInt(this.divs[i].width) / this.viewport_width;
        }

        // and fetch our weather
        com.trolltech.weatherChanged.connect(this, this.updateWeather);
        com.trolltech.locationChanged.connect(this, this.updateWeather);
        this.updateWeather();
    },

    updateAnim: function() {
        for (var i=0; i < this.image_dims.length;i++) {
            this.pos[i] += this.wind * this.scales[i];
            if (this.pos[i] > this.viewport_width) {
                this.pos[i] = -this.widths[i];
            } else if (this.pos[i] < - (this.widths[i])) {
                this.pos[i] = this.viewport_width;
            }
            this.divs[i].style.left = this.pos[i] + 'px';
        }
    },

    updateWind: function() {
        // Add a random amount to our wind
        var delta = (Math.random()*2) - 1;
        this.wind += delta;
        if (this.wind > this.maxwind)
            this.wind = this.maxwind;
        else if (this.wind < -this.maxwind)
            this.wind = -this.maxwind;
    },

    // Called when the actual weather changes (e.g. user moves, starts raining etc)
    updateWeather: function() {
        // See what type of weather we have
        var location = com.trolltech.currentLocation();
        var weather = com.trolltech.weather(location);

        function sIH(id, text) {var e = document.getElementById(id); e.innerHTML = text;}

        sIH(this.divid + '_location', location.suburb + ' - ' + location.postcode);
        sIH(this.divid + '_highvalue', weather.high + '&deg;C');
        sIH(this.divid + '_lowvalue', weather.low + '&deg;C');
        sIH(this.divid + '_nowvalue', weather.now + '&deg;C');
        sIH(this.divid + '_conditions', this.weathertable[weather.conditions].txt) || 'Unknown';

        for (var i = 0; i < this.image_dims.length; i++) {
            this.divs[i].src = this.weathertable[weather.conditions].img || '3rdparty/unknown.png';
        }

        this.maxwind = weather.wind / 4;
        this.wind = (Math.random()*this.maxwind) - (this.maxwind / 2);
    }
};

window.setTimeout("weather.init('weather')", 100);

