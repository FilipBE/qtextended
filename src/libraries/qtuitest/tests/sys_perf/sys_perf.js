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

    /* Tests for performance test functionality */

    initTestCase: function() {
        waitForQtopiaStart();
    },

/*
    \req QTOPIA-78

    \groups
*/
    meminfo: function() {
        var total = getMemoryInfo(TotalMemory);
        var buffers = getMemoryInfo(BuffersMemory);
        var cache = getMemoryInfo(CacheMemory);
        var free = getMemoryInfo(FreeMemory);
        var active = getMemoryInfo(ActiveMemory);
        var inactive = getMemoryInfo(InactiveMemory);
        var effective_free = getMemoryInfo(FreeMemory|BuffersMemory|CacheMemory);
        var effective_used = total - effective_free;

        print("TOTAL: " + total/1024 + " Mb");
        print("FREE: " + free/1024 + " Mb");
        print("BUFFERS: " + buffers/1024 + " Mb");
        print("CACHE: " + cache/1024 + " Mb");
        print("ACTIVE: " + active/1024 + " Mb");
        print("INACTIVE: " + inactive/1024 + " Mb");
        print("EFFECTIVE FREE: " + effective_free/1024 + " Mb");
        print("EFFECTIVE USED: " + effective_used/1024 + " Mb");

        verify( total > buffers );
        verify( total > cache );
        verify( total > free );
        verify( total > effective_free );
        verify( total > active );
        verify( total > inactive );

        /* Since we got the buffers, cache and free stats in a separate call,
            * the memory usage may have changed from when we got them all
            * together.  We allow the value to change by as much as 5Mb.
            */
        verify( Math.abs(effective_free - (free+buffers+cache)) < 5000 );
    },

/*
    \req QTOPIA-78

    \groups
*/
    memsampling: function() {
        startSamplingMemory(1000);

        wait(2500);
        wait(2500);

        stopSamplingMemory();

        var min_val = getSampledMemoryInfo(EffectiveFreeMemory, SampleMinimum);
        var min_line = getSampledMemoryInfo(EffectiveFreeMemory, SampleMinimum|SampleLine);
        var max_val = getSampledMemoryInfo(EffectiveFreeMemory, SampleMaximum);
        var max_line = getSampledMemoryInfo(EffectiveFreeMemory, SampleMaximum|SampleLine);
        var mean_val = getSampledMemoryInfo(EffectiveFreeMemory, SampleMean);
        print("EFFECTIVE FREE MEMORY:");
        print("  MIN: " + min_val/1024 + " Mb (line " + min_line + ")");
        print("  MAX: " + max_val/1024 + " Mb (line " + max_line + ")");
        print("  MEAN: " + mean_val/1024 + " Mb");

        verify( mean_val >= min_val );
        verify( mean_val <= max_val );
    },

    logging: function() {
        Performance.log("IGNORE one, no app",     1)
        Performance.log("IGNORE two, myapp",      2, "myapp");
        Performance.log("IGNORE three with desc", 3, "myapp", "some description");
    }
} // end of test

