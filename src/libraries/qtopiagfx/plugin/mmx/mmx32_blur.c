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

#include <mmintrin.h>
#include <stdio.h>

void mmx_blurcol(unsigned int *line, int length, int step, int alpha32)
{
    int out = 0;

    alpha32 |= alpha32 << 16;

    asm volatile (
            "wzero wr12\n\t" // wr12 = zero
            "tbcstw wr11, %4\n\t" // wr11 = alpha 0AAAAAAAAAAAAAAA.A.A.A
            // "wzero wr10\n\t" // wr10 = accumulator
            
            "mov r5, #7\n\t"
            "tbcstw wr13, r5\n\t" // wr13 = 7

            "mov r5, %3\n\t"

            // Load (pre)
            "wldrw wr1, [%2]\n\t" 

            // Setup accumulator default
            "wunpckelub wr10, wr1\n\t" // wr0 = 0A0R0G0B
            "wsllh wr10, wr10, wr13\n\t" // wr0 = 0aaaaaaaa0000000.r.g.b.

            "1:\n\t"

            "add r6, %2, %6\n\t"
            "subs r5, r5, #1\n\t" // Loop update

            // Adjust
            "wunpckelub wr0, wr1\n\t" // wr0 = 0A0R0G0B
            "wsllh wr0, wr0, wr13\n\t" // wr0 = 0aaaaaaaa0000000.r.g.b.

            // Adjust and accumulate
            "wsubh wr0, wr0, wr10\n\t" // wr0 = data - accum

            //"textrmuw %1, wr0, #0\n\t"
            "wmulsm wr0, wr0, wr11\n\t" // wr0 = (accum - data) * alpha

            "wldrwne wr1, [r6]\n\t" 

            "waddhss wr0, wr0, wr0\n\t" // wr0 = wr2 * 2
            "waddhss wr10, wr10, wr0\n\t" // wr10 += wr0

            // Adjust
            "wsrlh wr0, wr10, wr13\n\t" // wr0 = wr10 >> 7
            "wpackhus wr0, wr0, wr12\n\t" 

            // XXX 1 cycle STALL

            // Store
            "wstrw wr0, [%2]\n\t"
            "mov %2, r6\n\t"

            // Loop
            "bne 1b\n\t"

            //
            // Now go back again
            //

            "sub %2, %2, %6\n\t"
            "mov r5, %3\n\t"

            // Load (pre)
            "wldrw wr1, [%2]\n\t" 

            "2:\n\t"

            "sub r6, %2, %6\n\t"
            "subs r5, r5, #1\n\t" // Update loop

            // Adjust
            "wunpckelub wr0, wr1\n\t" // wr0 = 0A0R0G0B
            "wsllh wr0, wr0, wr13\n\t" // wr0 = 0aaaaaaaa0000000.r.g.b.

            // Adjust and accumulate
            "wsubh wr0, wr0, wr10\n\t" // wr0 = data - accum

            //"textrmuw %1, wr0, #0\n\t"
            "wmulsm wr0, wr0, wr11\n\t" // wr0 = (accum - data) * alpha

            "wldrwne wr1, [r6]\n\t" 

            "waddhss wr0, wr0, wr0\n\t" // wr0 = wr2 * 2
            "waddhss wr10, wr10, wr0\n\t" // wr10 += wr0

            // Adjust
            "wsrlh wr0, wr10, wr13\n\t" // wr0 = wr10 >> 7
            "wpackhus wr0, wr0, wr12\n\t" 

            // XXX 1 cycle STALL

            // Store
            "wstrw wr0, [%2]\n\t"
            "mov %2, r6\n\t"

            // Loop
            "bne 2b\n\t"



            : /* output */ "=r"(line), "=r"(out)
            : /* input */ "0"(line), "r"(length), "r"(alpha32), "1"(out), "r"(step)
            : /* clobber */ "r5", "r6", "wr12"
            );
}

// Can be sped up using technique from _16
void mmx_blurcol_8(unsigned int *line, int length, int step, int alpha32)
{
    int out = 0;

    alpha32 |= alpha32 << 16;

    /* 
       0 s
       1 s
       2 s
       3 s
       4 ac5
       5 ac6
       6 ac7
       7 ac1
       8 ac2
       9 ac3
       10 ac4
       11 alpha
       12 zero
       13 7
       14 ac8
       15 2
      */

    asm volatile (
            "wzero wr12\n\t" // wr12 = zero
            "tbcstw wr11, %4\n\t" // wr11 = alpha 0AAAAAAAAAAAAAAA.A.A.A

//            "wzero wr7\n\t" // wr7 = accumulator (1)
//            "wzero wr8\n\t" // wr8 = accumulator (2)
//            "wzero wr9\n\t" // wr9 = accumulator (3)
//            "wzero wr10\n\t" // wr10 = accumulator (4)
//            "wzero wr4\n\t" // wr4 = accumulator (5)
//            "wzero wr5\n\t" // wr5 = accumulator (6)
//            "wzero wr6\n\t" // wr6 = accumulator (7)
//            "wzero wr14\n\t" // wr14 = accumulator (8)
            
            "mov r5, #7\n\t"
            "tbcstw wr13, r5\n\t" // wr13 = 7

            // Setup accumulator default
            "wldrw wr7, [%2]\n\t"
            "wunpckelub wr7, wr7\n\t"
            "wsllh wr7, wr7, wr13\n\t"
            "wldrw wr8, [%2, #4]\n\t"
            "wunpckelub wr8, wr8\n\t"
            "wsllh wr8, wr8, wr13\n\t"
            "wldrw wr9, [%2, #8]\n\t"
            "wunpckelub wr9, wr9\n\t"
            "wsllh wr9, wr9, wr13\n\t"
            "wldrw wr10, [%2, #12]\n\t"
            "wunpckelub wr10, wr10\n\t"
            "wsllh wr10, wr10, wr13\n\t"
            "wldrw wr4, [%2, #16]\n\t"
            "wunpckelub wr4, wr4\n\t"
            "wsllh wr4, wr4, wr13\n\t"
            "wldrw wr5, [%2, #20]\n\t"
            "wunpckelub wr5, wr5\n\t"
            "wsllh wr5, wr5, wr13\n\t"
            "wldrw wr6, [%2, #24]\n\t"
            "wunpckelub wr6, wr6\n\t"
            "wsllh wr6, wr6, wr13\n\t"
            "wldrw wr14, [%2, #28]\n\t"
            "wunpckelub wr14, wr14\n\t"
            "wsllh wr14, wr14, wr13\n\t"

            "mov r5, %3\n\t"

            "1:\n\t"

            // wr0 & wr7
            // Load
            "wldrw wr0, [%2]\n\t" 
            // Adjust
            "wunpckelub wr0, wr0\n\t" // wr0 = 0A0R0G0B
            "wsllh wr0, wr0, wr13\n\t" // wr0 = 0aaaaaaaa0000000.r.g.b.
            // Adjust and accumulate
            "wsubh wr0, wr0, wr7\n\t" // wr0 = data - accum
            "wmulsm wr0, wr0, wr11\n\t" // wr0 = (accum - data) * alpha
            "wldrw wr1, [%2, #4]\n\t" // XXX 2
            "waddhss wr0, wr0, wr0\n\t" // wr0 = wr2 * 2
            "waddhss wr7, wr7, wr0\n\t" // wr7 += wr0
            // Adjust
            "wsrlh wr0, wr7, wr13\n\t" // wr0 = wr7 >> 7
            "wpackhus wr0, wr0, wr12\n\t" 
            // Store
            // "wstrw wr0, [%2]\n\t" // XXX 1

            // wr1 & wr8
            // Load
            // "wldrw wr1, [%2, #4]\n\t" // XXX 2
            // Adjust
            "wunpckelub wr1, wr1\n\t" // wr1 = 0A0R0G0B
            "wstrw wr0, [%2]\n\t" // XXX 1
            "wsllh wr1, wr1, wr13\n\t" // wr1 = 0aaaaaaaa0000000.r.g.b.
            // Adjust and accumulate
            "wsubh wr1, wr1, wr8\n\t" // wr1 = data - accum
            "wmulsm wr1, wr1, wr11\n\t" // wr1 = (accum - data) * alpha
            "wldrw wr2, [%2, #8]\n\t" // XXX 4
            "waddhss wr1, wr1, wr1\n\t" // wr1 = wr2 * 2
            "waddhss wr8, wr8, wr1\n\t" // wr8 += wr1
            // Adjust
            "wsrlh wr1, wr8, wr13\n\t" // wr1 = wr8 >> 7
            "wpackhus wr1, wr1, wr12\n\t" 
            // Store
            // "wstrw wr1, [%2, #4]\n\t" // XXX 3

            // wr2 & wr9
            // Load
            // "wldrw wr2, [%2, #8]\n\t" // XXX 4
            // Adjust
            "wunpckelub wr2, wr2\n\t" // wr2 = 0A0R0G0B
            "wstrw wr1, [%2, #4]\n\t" // XXX 3
            "wsllh wr2, wr2, wr13\n\t" // wr2 = 0aaaaaaaa0000000.r.g.b.
            // Adjust and accumulate
            "wsubh wr2, wr2, wr9\n\t" // wr2 = data - accum
            "wmulsm wr2, wr2, wr11\n\t" // wr2 = (accum - data) * alpha
            "wldrw wr3, [%2, #12]\n\t" // XXX 6
            "waddhss wr2, wr2, wr2\n\t" // wr1 = wr2 * 2
            "waddhss wr9, wr9, wr2\n\t" // wr8 += wr1
            // Adjust
            "wsrlh wr2, wr9, wr13\n\t" // wr1 = wr8 >> 7
            "wpackhus wr2, wr2, wr12\n\t" 
            // Store
            // "wstrw wr2, [%2, #8]\n\t" // XXX 5

            // wr3 & wr10
            // Load
            // "wldrw wr3, [%2, #12]\n\t" // XXX 6
            // Adjust
            "wunpckelub wr3, wr3\n\t" // wr3 = 0A0R0G0B
            "wstrw wr2, [%2, #8]\n\t" // XXX 5
            "wsllh wr3, wr3, wr13\n\t" // wr3 = 0aaaaaaaa0000000.r.g.b.
            // Adjust and accumulate
            "wsubh wr3, wr3, wr10\n\t" // wr3 = data - accum
            "wmulsm wr3, wr3, wr11\n\t" // wr3 = (accum - data) * alpha
            "wldrw wr0, [%2, #16]\n\t" // XXX 8
            "waddhss wr3, wr3, wr3\n\t" // wr1 = wr3 * 2
            "waddhss wr10, wr10, wr3\n\t" // wr8 += wr1
            // Adjust
            "wsrlh wr3, wr10, wr13\n\t" // wr1 = wr8 >> 7
            "wpackhus wr3, wr3, wr12\n\t" 
            // Store
            // "wstrw wr3, [%2, #12]\n\t" // XXX 7

            // wr0 & wr4
            // Load
            // "wldrw wr0, [%2, #16]\n\t" // XXX 8
            // Adjust
            "wunpckelub wr0, wr0\n\t" // wr0 = 0A0R0G0B
            "wstrw wr3, [%2, #12]\n\t" // XXX 7
            "wsllh wr0, wr0, wr13\n\t" // wr0 = 0aaaaaaaa0000000.r.g.b.
            // Adjust and accumulate
            "wsubh wr0, wr0, wr4\n\t" // wr0 = data - accum
            "wmulsm wr0, wr0, wr11\n\t" // wr0 = (accum - data) * alpha
            "wldrw wr1, [%2, #20]\n\t" // XXX 9
            "waddhss wr0, wr0, wr0\n\t" // wr0 = wr2 * 2
            "waddhss wr4, wr4, wr0\n\t" // wr4 += wr0
            // Adjust
            "wsrlh wr0, wr4, wr13\n\t" // wr0 = wr4 >> 7
            "wpackhus wr0, wr0, wr12\n\t" 
            // Store
            // "wstrw wr0, [%2, #16]\n\t" // XXX 8

            // wr1 & wr5
            // Load
            // "wldrw wr1, [%2, #20]\n\t" // XXX 9
            // Adjust
            "wunpckelub wr1, wr1\n\t" // wr1 = 0A0R0G0B
            "wstrw wr0, [%2, #16]\n\t" // XXX 8
            "wsllh wr1, wr1, wr13\n\t" // wr1 = 0aaaaaaaa0000000.r.g.b.
            // Adjust and accumulate
            "wsubh wr1, wr1, wr5\n\t" // wr1 = data - accum
            "wmulsm wr1, wr1, wr11\n\t" // wr1 = (accum - data) * alpha
            "wldrw wr2, [%2, #24]\n\t" // XXX 11
            "waddhss wr1, wr1, wr1\n\t" // wr1 = wr2 * 2
            "waddhss wr5, wr5, wr1\n\t" // wr5 += wr1
            // Adjust
            "wsrlh wr1, wr5, wr13\n\t" // wr1 = wr5 >> 7
            "wpackhus wr1, wr1, wr12\n\t" 
            // Store
            // "wstrw wr1, [%2, #20]\n\t" // XXX 10

            // wr2 & wr6
            // Load
            // "wldrw wr2, [%2, #24]\n\t" // XXX 11
            // Adjust
            "wunpckelub wr2, wr2\n\t" // wr2 = 0A0R0G0B
            "wstrw wr1, [%2, #20]\n\t" // XXX 10
            "wsllh wr2, wr2, wr13\n\t" // wr2 = 0aaaaaaaa0000000.r.g.b.
            // Adjust and accumulate
            "wsubh wr2, wr2, wr6\n\t" // wr2 = data - accum
            "wmulsm wr2, wr2, wr11\n\t" // wr2 = (accum - data) * alpha
            "wldrw wr3, [%2, #28]\n\t" // XXX 13
            "waddhss wr2, wr2, wr2\n\t" // wr1 = wr2 * 2
            "waddhss wr6, wr6, wr2\n\t" // wr8 += wr1
            // Adjust
            "wsrlh wr2, wr6, wr13\n\t" // wr1 = wr8 >> 7
            "wpackhus wr2, wr2, wr12\n\t" 
            // Store
            // "wstrw wr2, [%2, #24]\n\t" // XXX 12

            // wr3 & wr14
            // Load
            // "wldrw wr3, [%2, #28]\n\t" // XXX 13
            // Adjust
            "wunpckelub wr3, wr3\n\t" // wr3 = 0A0R0G0B
            "wstrw wr2, [%2, #24]\n\t" // XXX 12
            "wsllh wr3, wr3, wr13\n\t" // wr3 = 0aaaaaaaa0000000.r.g.b.
            // Adjust and accumulate
            "wsubh wr3, wr3, wr14\n\t" // wr3 = data - accum
            "wmulsm wr3, wr3, wr11\n\t" // wr3 = (accum - data) * alpha
            "waddhss wr3, wr3, wr3\n\t" // wr1 = wr3 * 2
            "waddhss wr14, wr14, wr3\n\t" // wr8 += wr1
            // Adjust
            "wsrlh wr3, wr14, wr13\n\t" // wr1 = wr8 >> 7
            "wpackhus wr3, wr3, wr12\n\t" 
            // Store
            "subs r5, r5, #1\n\t" // Loop update // XXX 14
            "wstrw wr3, [%2, #28]\n\t"

            "addne %2, %2, %6\n\t"
            // "subs r5, r5, #1\n\t" // Loop update // XXX 14

            // Loop
            "bne 1b\n\t"

            //
            // Now go back again
            //

            "mov r5, %3\n\t"

            "2:\n\t"

            // wr0 & wr7
            // Load
            "wldrw wr0, [%2]\n\t" // XXX 15
            // Adjust
            "wunpckelub wr0, wr0\n\t" // wr0 = 0A0R0G0B
            "wsllh wr0, wr0, wr13\n\t" // wr0 = 0aaaaaaaa0000000.r.g.b.
            // Adjust and accumulate
            "wsubh wr0, wr0, wr7\n\t" // wr0 = data - accum
            "wmulsm wr0, wr0, wr11\n\t" // wr0 = (accum - data) * alpha
            "wldrw wr1, [%2, #4]\n\t"  // XXX 17
            "waddhss wr0, wr0, wr0\n\t" // wr0 = wr2 * 2
            "waddhss wr7, wr7, wr0\n\t" // wr7 += wr0
            // Adjust
            "wsrlh wr0, wr7, wr13\n\t" // wr0 = wr7 >> 7
            "wpackhus wr0, wr0, wr12\n\t" 
            // Store
            // "wstrw wr0, [%2]\n\t" // XXX 16

            // wr1 & wr8
            // Load
            // "wldrw wr1, [%2, #4]\n\t"  // XXX 17
            // Adjust
            "wunpckelub wr1, wr1\n\t" // wr1 = 0A0R0G0B
            "wstrw wr0, [%2]\n\t" // XXX 16
            "wsllh wr1, wr1, wr13\n\t" // wr1 = 0aaaaaaaa0000000.r.g.b.
            // Adjust and accumulate
            "wsubh wr1, wr1, wr8\n\t" // wr1 = data - accum
            "wmulsm wr1, wr1, wr11\n\t" // wr1 = (accum - data) * alpha
            "wldrw wr2, [%2, #8]\n\t"  // XXX 19
            "waddhss wr1, wr1, wr1\n\t" // wr1 = wr2 * 2
            "waddhss wr8, wr8, wr1\n\t" // wr8 += wr1
            // Adjust
            "wsrlh wr1, wr8, wr13\n\t" // wr1 = wr8 >> 7
            "wpackhus wr1, wr1, wr12\n\t" 
            // Store
            // "wstrw wr1, [%2, #4]\n\t" // XXX 18

            // wr2 & wr9
            // Load
            // "wldrw wr2, [%2, #8]\n\t"  // XXX 19
            // Adjust
            "wunpckelub wr2, wr2\n\t" // wr2 = 0A0R0G0B
            "wstrw wr1, [%2, #4]\n\t" // XXX 18
            "wsllh wr2, wr2, wr13\n\t" // wr2 = 0aaaaaaaa0000000.r.g.b.
            // Adjust and accumulate
            "wsubh wr2, wr2, wr9\n\t" // wr2 = data - accum
            "wmulsm wr2, wr2, wr11\n\t" // wr2 = (accum - data) * alpha
            "wldrw wr3, [%2, #12]\n\t"  // XXX 21
            "waddhss wr2, wr2, wr2\n\t" // wr1 = wr2 * 2
            "waddhss wr9, wr9, wr2\n\t" // wr8 += wr1
            // Adjust
            "wsrlh wr2, wr9, wr13\n\t" // wr1 = wr8 >> 7
            "wpackhus wr2, wr2, wr12\n\t" 
            // Store
            // "wstrw wr2, [%2, #8]\n\t" // XXX 20

            // wr3 & wr10
            // Load
            // "wldrw wr3, [%2, #12]\n\t"  // XXX 21
            // Adjust
            "wunpckelub wr3, wr3\n\t" // wr3 = 0A0R0G0B
            "wstrw wr2, [%2, #8]\n\t" // XXX 20
            "wsllh wr3, wr3, wr13\n\t" // wr3 = 0aaaaaaaa0000000.r.g.b.
            // Adjust and accumulate
            "wsubh wr3, wr3, wr10\n\t" // wr3 = data - accum
            "wmulsm wr3, wr3, wr11\n\t" // wr3 = (accum - data) * alpha
            "wldrw wr0, [%2, #16]\n\t"  // XXX 23
            "waddhss wr3, wr3, wr3\n\t" // wr1 = wr3 * 2
            "waddhss wr10, wr10, wr3\n\t" // wr8 += wr1
            // Adjust
            "wsrlh wr3, wr10, wr13\n\t" // wr1 = wr8 >> 7
            "wpackhus wr3, wr3, wr12\n\t" 
            // Store
           // "wstrw wr3, [%2, #12]\n\t" // XXX 22

            // wr0 & wr4
            // Load
            // "wldrw wr0, [%2, #16]\n\t"  // XXX 23
            // Adjust
            "wunpckelub wr0, wr0\n\t" // wr0 = 0A0R0G0B
            "wstrw wr3, [%2, #12]\n\t" // XXX 22
            "wsllh wr0, wr0, wr13\n\t" // wr0 = 0aaaaaaaa0000000.r.g.b.
            // Adjust and accumulate
            "wsubh wr0, wr0, wr4\n\t" // wr0 = data - accum
            "wmulsm wr0, wr0, wr11\n\t" // wr0 = (accum - data) * alpha
            "wldrw wr1, [%2, #20]\n\t" // XXX 25
            "waddhss wr0, wr0, wr0\n\t" // wr0 = wr2 * 2
            "waddhss wr4, wr4, wr0\n\t" // wr4 += wr0
            // Adjust
            "wsrlh wr0, wr4, wr13\n\t" // wr0 = wr4 >> 7
            "wpackhus wr0, wr0, wr12\n\t" 
            // Store
            //"wstrw wr0, [%2, #16]\n\t" // XXX 24

            // wr1 & wr5
            // Load
            //"wldrw wr1, [%2, #20]\n\t" // XXX 25
            // Adjust
            "wunpckelub wr1, wr1\n\t" // wr1 = 0A0R0G0B
            "wstrw wr0, [%2, #16]\n\t" // XXX 24
            "wsllh wr1, wr1, wr13\n\t" // wr1 = 0aaaaaaaa0000000.r.g.b.
            // Adjust and accumulate
            "wsubh wr1, wr1, wr5\n\t" // wr1 = data - accum
            "wmulsm wr1, wr1, wr11\n\t" // wr1 = (accum - data) * alpha
            "wldrw wr2, [%2, #24]\n\t" // XXX 27
            "waddhss wr1, wr1, wr1\n\t" // wr1 = wr2 * 2
            "waddhss wr5, wr5, wr1\n\t" // wr5 += wr1
            // Adjust
            "wsrlh wr1, wr5, wr13\n\t" // wr1 = wr5 >> 7
            "wpackhus wr1, wr1, wr12\n\t" 
            // Store
            //"wstrw wr1, [%2, #20]\n\t" // XXX 26

            // wr2 & wr6
            // Load
            //"wldrw wr2, [%2, #24]\n\t" // XXX 27
            // Adjust
            "wunpckelub wr2, wr2\n\t" // wr2 = 0A0R0G0B
            "wstrw wr1, [%2, #20]\n\t" // XXX 26
            "wsllh wr2, wr2, wr13\n\t" // wr2 = 0aaaaaaaa0000000.r.g.b.
            // Adjust and accumulate
            "wsubh wr2, wr2, wr6\n\t" // wr2 = data - accum
            "wmulsm wr2, wr2, wr11\n\t" // wr2 = (accum - data) * alpha
            "wldrw wr3, [%2, #28]\n\t" // XXX 29
            "waddhss wr2, wr2, wr2\n\t" // wr1 = wr2 * 2
            "waddhss wr6, wr6, wr2\n\t" // wr8 += wr1
            // Adjust
            "wsrlh wr2, wr6, wr13\n\t" // wr1 = wr8 >> 7
            "wpackhus wr2, wr2, wr12\n\t" 
            // Store
            //"wstrw wr2, [%2, #24]\n\t" // XXX 28

            // wr3 & wr14
            // Load
            //"wldrw wr3, [%2, #28]\n\t" // XXX 29
            // Adjust
            "wunpckelub wr3, wr3\n\t" // wr3 = 0A0R0G0B
            "wstrw wr2, [%2, #24]\n\t" // XXX 28
            "wsllh wr3, wr3, wr13\n\t" // wr3 = 0aaaaaaaa0000000.r.g.b.
            // Adjust and accumulate
            "wsubh wr3, wr3, wr14\n\t" // wr3 = data - accum
            "wmulsm wr3, wr3, wr11\n\t" // wr3 = (accum - data) * alpha
            "waddhss wr3, wr3, wr3\n\t" // wr1 = wr3 * 2
            "waddhss wr14, wr14, wr3\n\t" // wr8 += wr1
            // Adjust
            "wsrlh wr3, wr14, wr13\n\t" // wr1 = wr8 >> 7
            "wpackhus wr3, wr3, wr12\n\t" 
            "subs r5, r5, #1\n\t" // Loop update
            // Store
            "wstrw wr3, [%2, #28]\n\t"

            "sub %2, %2, %6\n\t"

            // Loop
            "bne 2b\n\t"


            : /* output */ "=r"(line), "=r"(out)
            : /* input */ "0"(line), "r"(length), "r"(alpha32), "1"(out), "r"(step)
            : /* clobber */ "r5", "r6", "wr12"
            );
}

void mmx_blurrow(unsigned int *line, int length, int alpha32)
{
    int out = 0;

    alpha32 |= alpha32 << 16;

    asm volatile (
            "wzero wr12\n\t" // wr12 = zero
            "tbcstw wr11, %4\n\t" // wr11 = alpha 0AAAAAAAAAAAAAAA.A.A.A
            // "wzero wr10\n\t" // wr10 = accumulator
            
            "mov r5, #7\n\t"
            "tbcstw wr13, r5\n\t" // wr13 = 7

            "mov r5, %3\n\t"

            // Load (pre)
            "wldrw wr1, [%2]\n\t" 

            // Setup accumulator default
            "wunpckelub wr10, wr1\n\t" // wr0 = 0A0R0G0B
            "wsllh wr10, wr10, wr13\n\t" // wr0 = 0aaaaaaaa0000000.r.g.b.

            "1:\n\t"

            "subs r5, r5, #1\n\t" // Loop adjust

            // Adjust
            "wunpckelub wr0, wr1\n\t" // wr0 = 0A0R0G0B
            "wsllh wr0, wr0, wr13\n\t" // wr0 = 0aaaaaaaa0000000.r.g.b.

            // Adjust and accumulate
            "wsubh wr0, wr0, wr10\n\t" // wr0 = data - accum

            //"textrmuw %1, wr0, #0\n\t"
            "wmulsm wr0, wr0, wr11\n\t" // wr0 = (accum - data) * alpha

            "wldrwne wr1, [%2, #4]\n\t" 

            "waddhss wr0, wr0, wr0\n\t" // wr0 = wr2 * 2
            "waddhss wr10, wr10, wr0\n\t" // wr10 += wr0

            // Adjust
            "wsrlh wr0, wr10, wr13\n\t" // wr0 = wr10 >> 7
            "wpackhus wr0, wr0, wr12\n\t" 

            // XXX 1 cycle STALL

            // Store
            "wstrw wr0, [%2], #4\n\t"

            // Loop
            "bne 1b\n\t"

            //
            // Now go back again
            //

            "sub %2, %2, #4\n\t"
            "mov r5, %3\n\t"

            // Load (pre)
            "wldrw wr1, [%2]\n\t" 

            "2:\n\t"

            "subs r5, r5, #1\n\t" // Loop adjust

            // Adjust
            "wunpckelub wr0, wr1\n\t" // wr0 = 0A0R0G0B
            "wsllh wr0, wr0, wr13\n\t" // wr0 = 0aaaaaaaa0000000.r.g.b.

            // Adjust and accumulate
            "wsubh wr0, wr0, wr10\n\t" // wr0 = data - accum

            //"textrmuw %1, wr0, #0\n\t"
            "wmulsm wr0, wr0, wr11\n\t" // wr0 = (accum - data) * alpha

            "wldrwne wr1, [%2, #-4]\n\t" 

            "waddhss wr0, wr0, wr0\n\t" // wr0 = wr2 * 2
            "waddhss wr10, wr10, wr0\n\t" // wr10 += wr0

            // Adjust
            "wsrlh wr0, wr10, wr13\n\t" // wr0 = wr10 >> 7
            "wpackhus wr0, wr0, wr12\n\t" 

            // XXX 1 cycle STALL

            // Store
            "wstrw wr0, [%2], #-4\n\t"

            // Loop
            "bne 2b\n\t"

            : /* output */ "=r"(line), "=r"(out)
            : /* input */ "0"(line), "r"(length), "r"(alpha32), "1"(out)
            : /* clobber */ "r5", "wr12"
            );
}

void mmx_blur(unsigned int *img, int width, int height, int step_width, int alpha32)
{
    int row, col;

    for(row = 0; row < height; ++row) 
        mmx_blurrow(img + row * step_width, width, alpha32);

    while(width >= 8) {
        mmx_blurcol_8(img, height, step_width * 4, alpha32);
        img += 8;
        width -= 8;
    }

    while(width--)
        mmx_blurcol(img++, height, step_width * 4, alpha32);
}


