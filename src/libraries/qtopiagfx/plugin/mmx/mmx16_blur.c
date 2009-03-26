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


#define DITHER

// line must be 16-bit aligned
// Basically mmx16_blurrow for columns
void mmx16_blurcol(unsigned short *line, int length, int step, int alpha32)
{
    int out = 0;
    int alpha_inv = 0x10000 - alpha32;
    alpha_inv |= alpha_inv << 16;
    alpha32 |= alpha32 << 16;

    asm volatile (
            // Setup masks
            "ldr r1, =0xF800F800\n\t"
            "ldr r2, =0x07E0001F\n\t" 
            "tmcrr wr10, r2, r1\n\t" // F800 F800 07E0 001F

            "ldr r1, =0x00010001\n\t"
            "ldr r2, =0x00200800\n\t"
            "tmcrr wr11, r2, r1\n\t" // 0001 0001 0020 0800

            "ldr r1, =0x00000020\n\t"
            "ldr r2, =0x00400020\n\t"
            "tmcrr wr12, r2, r1\n\t" // 0000 0020 0040 0020

            "ldr r1, =0x00000800\n\t"
            "ldr r2, =0x00200001\n\t"
            "tmcrr wr13, r2, r1\n\t" // 0000 0800 0020 0001

            "ldr r1, =0xF800F800\n\t"
            "ldr r2, =0xFC00F800\n\t"
            "tmcrr wr14, r2, r1\n\t" // F800 F800 FC00 F800

            "ldr r1, =0x07ff07ff\n\t"
            "ldr r2, =0x03ff07ff\n\t"
            "tmcrr wr6, r2, r1\n\t"  // 0x7FF 07FF 03FF 07FF

            "ldr r1, =0x80008000\n\t"
            "ldr r2, =0x08000020\n\t"
            "tmcrr wr15, r2, r1\n\t" // 8000 8000 0800 0020

#ifdef DITHER
            // Setup accum
            "wzero wr3\n\t"
#endif

            // Setup init values
            // "wzero wr9\n\t" // wr9 - accum
            "tbcstw wr8, %4\n\t" // wr8 = alpha
            "tbcstw wr7, %5\n\t" // wr7 = 1 - alpha

            // Setup initial accum
            "wldrh wr0, [%2]\n\t"
            "wshufh wr1, wr0, #0x00\n\t"
            "wand wr1, wr1, wr10\n\t"
            "wmulul wr9, wr1, wr11\n\t"

            // Pre load above

            "mov r3, %3\n\t"
            "add r2, %2, %6\n\t"

            "1:\n\t"

            // Load - lower
            "wshufh wr1, wr0, #0x00\n\t"
            "wmulul wr1, wr1, wr11\n\t"
            "wmulum wr9, wr9, wr7\n\t"
            "wor wr1, wr1, wr6\n\t"

            // Manip - lower
            "wmulum wr1, wr1, wr8\n\t"
            "subs r3, r3, #1\n\t"
            "waddhus wr9, wr9, wr1\n\t"

#ifdef DITHER
            "wsubhus wr4, wr9, wr3\n\t"
            "wor wr1, wr4, wr6\n\t"
            "wsubbus wr3, wr1, wr4\n\t"

            // Store - lower
            "wmulum wr1, wr1, wr12\n\t"
#else
            // Store - lower
            "wmulum wr1, wr9, wr12\n\t"
#endif
            "wldrhne wr0, [r2]\n\t"

            "wmacuz wr1, wr1, wr13\n\t"

            "add r2, r2, %6\n\t"
            "wstrh wr1, [%2]\n\t"

            "add %2, %2, %6\n\t"

            "bne 1b\n\t"
            
            // Reset
            "sub %2, %2, %6\n\t"
            "mov r3, %3\n\t"

            // Load (pre)
            "wldrh wr0, [%2]\n\t"
            "sub r2, %2, %6\n\t"

            "2:\n\t"

            // Load - lower
            "wshufh wr1, wr0, #0x00\n\t"
            "wmulul wr1, wr1, wr11\n\t"
            "wmulum wr9, wr9, wr7\n\t"
            "wor wr1, wr1, wr6\n\t"

            // Manip - lower
            "wmulum wr1, wr1, wr8\n\t"
            "subs r3, r3, #1\n\t"
            "waddhus wr9, wr9, wr1\n\t"

#ifdef DITHER
            "wsubhus wr4, wr9, wr3\n\t"
            "wor wr1, wr4, wr6\n\t"
            "wsubbus wr3, wr1, wr4\n\t"

            // Store - lower
            "wmulum wr1, wr1, wr12\n\t"
#else
            // Store - lower
            "wmulum wr1, wr9, wr12\n\t"
#endif
            "wldrhne wr0, [r2]\n\t"

            "wmacuz wr1, wr1, wr13\n\t"

            "sub r2, r2, %6\n\t"
            "wstrh wr1, [%2]\n\t"

            "sub %2, %2, %6\n\t"

            "bne 2b\n\t"


            : /* output */ "=r"(line), "=r"(out)
            : /* input */ "0"(line), "r"(length), "r"(alpha32), "r"(alpha_inv), "r"(step)
            : /* clobber */ "wr0", "wr1", "wr2", "wr3", "wr4", "wr5", "wr6", "wr7", "wr8", "wr9", "wr10", "wr11", "wr12", "wr13", "wr14", "wr15", "r1", "r2", "r3"
            );
}

// line must be 32-bit aligned
// Copy of mmx16_blurcol_4 with word read/write
void mmx16_blurcol_2(unsigned short *line, int length, int step, int alpha32)
{
    int out = 0;

    int alpha_inv = 0x10000 - alpha32;
    alpha_inv |= alpha_inv << 16;
    alpha32 |= alpha32 << 16;

    asm volatile(
            // Setup masks
            "ldr r1, =0x07FF07FF\n\t"
            "tmcrr wr10, r1, r1\n\t" // 07FF 07FF 07FF 07FF

            "ldr r1, =0x03FF03FF\n\t"
            "tmcrr wr11, r1, r1\n\t" // 03FF 03FF 03FF 03FF

            "ldr r1, =0xF800F800\n\t"
            "tmcrr wr6, r1, r1\n\t" // F800 F800 F800 F800

            "ldr r1, =0xFC00FC00\n\t"
            "tmcrr wr12, r1, r1\n\t" // FC00 FC00 FC00 FC00

            "ldr r1, =5\n\t"
            "tmcr wCGR0, r1\n\t" // 5

            "ldr r1, =11\n\t"
            "tmcr wCGR1, r1\n\t" // 11

            // Reset values
            // wzero wr13 // red
            // wzero wr14 // green
            // wzero wr15 // blue
            "tbcstw wr9, %5\n\t" // wr9 = alpha
            "tbcstw wr8, %6\n\t" // wr8 = 1 - alpha


            // Setup initial accum 
            "wldrw wr0, [%2]\n\t"
            "wor wr13, wr0, wr10\n\t"
            "wsllhg wr1, wr0, wCGR0\n\t"
            "wor wr14, wr1, wr11\n\t"
            "wsllhg wr1, wr0, wCGR1\n\t"
            "wor wr15, wr1, wr10\n\t"

            // Load (pre)
            "wldrw wr0, [%2]\n\t"

            "mov r3, %3\n\t"

            "add r2, %2, %4\n\t"

            "1:\n\t"

            "subs r3, r3, #1\n\t"

            "wor wr1, wr0, wr10\n\t"
            "wmulum wr13, wr13, wr8\n\t"
            "wsllhg wr2, wr0, wCGR0\n\t"
            "wmulum wr1, wr1, wr9\n\t"
            "wor wr2, wr2, wr11\n\t"
            "waddhus wr13, wr13, wr1\n\t"


            "wmulum wr14, wr14, wr8\n\t"
            "wsllhg wr3, wr0, wCGR1\n\t"
            "wmulum wr2, wr2, wr9\n\t"
            "wor wr3, wr3, wr10\n\t"
            "waddhus wr14, wr14, wr2\n\t"


            // Load
            "wldrwne wr0, [r2]\n\t"
            "add r2, r2, %4\n\t"


            "wmulum wr15, wr15, wr8\n\t"
            "wand wr1, wr13, wr6\n\t" 

            "wmulum wr3, wr3, wr9\n\t"
            "wand wr2, wr14, wr12\n\t" 
            "waddhus wr15, wr15, wr3\n\t"

            "wsrlhg wr3, wr15, wCGR1\n\t" 
            "wsrlhg wr2, wr2, wCGR0\n\t"
            "wor wr2, wr2, wr3\n\t"
            "wor wr2, wr2, wr1\n\t"

            "wstrw wr2, [%2]\n\t"

            "add %2, %2, %4\n\t"

            "bne 1b\n\t"

            // Reset
            "sub %2, %2, %4\n\t"
            // Load (pre)
            "wldrw wr0, [%2]\n\t"
            "sub r2, %2, %4\n\t"
            "mov r3, %3\n\t"

            "2:\n\t"

            "subs r3, r3, #1\n\t"

            "wor wr1, wr0, wr10\n\t"
            "wmulum wr13, wr13, wr8\n\t"
            "wsllhg wr2, wr0, wCGR0\n\t"
            "wmulum wr1, wr1, wr9\n\t"
            "wor wr2, wr2, wr11\n\t"
            "waddhus wr13, wr13, wr1\n\t"


            "wmulum wr14, wr14, wr8\n\t"
            "wsllhg wr3, wr0, wCGR1\n\t"
            "wmulum wr2, wr2, wr9\n\t"
            "wor wr3, wr3, wr10\n\t"
            "waddhus wr14, wr14, wr2\n\t"

            // Load
            "wldrwne wr0, [r2]\n\t"
            "sub r2, r2, %4\n\t"

            "wmulum wr15, wr15, wr8\n\t"
            "wand wr1, wr13, wr6\n\t" 

            "wmulum wr3, wr3, wr9\n\t"
            "wand wr2, wr14, wr12\n\t" 
            "waddhus wr15, wr15, wr3\n\t"

            "wsrlhg wr3, wr15, wCGR1\n\t" 
            "wsrlhg wr2, wr2, wCGR0\n\t"
            "wor wr2, wr2, wr3\n\t"
            "wor wr2, wr2, wr1\n\t"

            "wstrw wr2, [%2]\n\t"

            "sub %2, %2, %4\n\t"

            "bne 2b\n\t"

            : /* output */ "=r"(line), "=r"(out)
            : /* input */ "0"(line), "r"(length), "r"(step), "r"(alpha32), "r"(alpha_inv)
            : /* clobber */ "wr0", "wr1", "wr2", "wr3", "wr4", "wr5", "wr6", "wr7", "wr8", "wr9", "wr10", "wr11", "wr12", "wr13", "wr14", "wr15", "r1", "r2", "r3");

}

// line must be 64-bit aligned
// This is a trivial adaptation of mmx16_blurcol_4
void mmx16_blurcol_8(unsigned short *line, int length, int step, int alpha32)
{
    int out = 0;

    int alpha_inv = 0x10000 - alpha32;
    alpha_inv |= alpha_inv << 16;
    alpha32 |= alpha32 << 16;

    asm volatile(
            // Setup masks
            "ldr r1, =0x07FF07FF\n\t"
            "tmcrr wr10, r1, r1\n\t" // 07FF 07FF 07FF 07FF

            "ldr r1, =0x03FF03FF\n\t"
            "tmcrr wr11, r1, r1\n\t" // 03FF 03FF 03FF 03FF

            "ldr r1, =0xF800F800\n\t"
            "tmcrr wr6, r1, r1\n\t" // F800 F800 F800 F800

            "ldr r1, =0xFC00FC00\n\t"
            "tmcrr wr12, r1, r1\n\t" // FC00 FC00 FC00 FC00

            "ldr r1, =5\n\t"
            "tmcr wCGR0, r1\n\t" // 5

            "ldr r1, =11\n\t"
            "tmcr wCGR1, r1\n\t" // 11

            // Reset values
            // wzero wr13 // red
            // wzero wr14 // green
            // wzero wr15 // blue
            // wzero wr4 // red2
            // wzero wr5 // green2
            // wzero wr7 // blue2
            "tbcstw wr9, %5\n\t" // wr9 = alpha
            "tbcstw wr8, %6\n\t" // wr8 = 1 - alpha


            // Setup initial accum 
            "wldrd wr0, [%2]\n\t"
            "wor wr13, wr0, wr10\n\t"
            "wldrd wr3, [%2, #8]\n\t"
            "wsllhg wr1, wr0, wCGR0\n\t"
            "wor wr14, wr1, wr11\n\t"
            "wsllhg wr1, wr0, wCGR1\n\t"
            "wor wr15, wr1, wr10\n\t"
            "wor wr4, wr3, wr10\n\t"
            "wsllhg wr1, wr3, wCGR0\n\t"
            "wor wr5, wr1, wr11\n\t"
            "wsllhg wr1, wr3, wCGR1\n\t"
            "wor wr7, wr1, wr10\n\t"

            // Load (pre)
            "wldrd wr0, [%2]\n\t"

            "mov r3, %3\n\t"

            //"add r2, %2, %4\n\t"
            "mov r2, %2\n\t"

            "1:\n\t"

            "subs r3, r3, #1\n\t"

            "wor wr1, wr0, wr10\n\t"
            "wmulum wr13, wr13, wr8\n\t"
            "wsllhg wr2, wr0, wCGR0\n\t"
            "wmulum wr1, wr1, wr9\n\t"
            "wor wr2, wr2, wr11\n\t"
            "waddhus wr13, wr13, wr1\n\t"


            "wmulum wr14, wr14, wr8\n\t"
            "wsllhg wr3, wr0, wCGR1\n\t"
            "wmulum wr2, wr2, wr9\n\t"
            "wor wr3, wr3, wr10\n\t"
            "waddhus wr14, wr14, wr2\n\t"

            "wldrdne wr0, [r2, #8]\n\t"
            "add r2, r2, %4\n\t"

            "wmulum wr15, wr15, wr8\n\t"
            "wand wr1, wr13, wr6\n\t" 

            "wmulum wr3, wr3, wr9\n\t"
            "wand wr2, wr14, wr12\n\t" 
            "waddhus wr15, wr15, wr3\n\t"

            "wsrlhg wr3, wr15, wCGR1\n\t" 
            "wsrlhg wr2, wr2, wCGR0\n\t"
            "wor wr2, wr2, wr3\n\t"
            "wor wr2, wr2, wr1\n\t"

            "wstrd wr2, [%2]\n\t"

            "wor wr1, wr0, wr10\n\t"
            "wmulum wr4, wr4, wr8\n\t"
            "wsllhg wr2, wr0, wCGR0\n\t"
            "wmulum wr1, wr1, wr9\n\t"
            "wor wr2, wr2, wr11\n\t"
            "waddhus wr4, wr4, wr1\n\t"


            "wmulum wr5, wr5, wr8\n\t"
            "wsllhg wr3, wr0, wCGR1\n\t"
            "wmulum wr2, wr2, wr9\n\t"
            "wor wr3, wr3, wr10\n\t"
            "waddhus wr5, wr5, wr2\n\t"

            "wldrdne wr0, [r2]\n\t"

            "wmulum wr7, wr7, wr8\n\t"
            "wand wr1, wr4, wr6\n\t" 

            "wmulum wr3, wr3, wr9\n\t"
            "wand wr2, wr5, wr12\n\t" 
            "waddhus wr7, wr7, wr3\n\t"

            "wsrlhg wr3, wr7, wCGR1\n\t" 
            "wsrlhg wr2, wr2, wCGR0\n\t"
            "wor wr2, wr2, wr3\n\t"
            "wor wr2, wr2, wr1\n\t"

            "wstrd wr2, [%2, #8]\n\t"

            "add %2, %2, %4\n\t"

            "bne 1b\n\t"

            // Reset
            "sub %2, %2, %4\n\t"
            // Load (pre)
            "wldrd wr0, [%2]\n\t"
            "mov r2, %2\n\t"
            "mov r3, %3\n\t"

            "2:\n\t"

            "subs r3, r3, #1\n\t"

            "wor wr1, wr0, wr10\n\t"
            "wmulum wr13, wr13, wr8\n\t"
            "wsllhg wr2, wr0, wCGR0\n\t"
            "wmulum wr1, wr1, wr9\n\t"
            "wor wr2, wr2, wr11\n\t"
            "waddhus wr13, wr13, wr1\n\t"


            "wmulum wr14, wr14, wr8\n\t"
            "wsllhg wr3, wr0, wCGR1\n\t"
            "wmulum wr2, wr2, wr9\n\t"
            "wor wr3, wr3, wr10\n\t"
            "waddhus wr14, wr14, wr2\n\t"

            "wldrdne wr0, [r2, #8]\n\t"
            "sub r2, r2, %4\n\t"

            "wmulum wr15, wr15, wr8\n\t"
            "wand wr1, wr13, wr6\n\t" 

            "wmulum wr3, wr3, wr9\n\t"
            "wand wr2, wr14, wr12\n\t" 
            "waddhus wr15, wr15, wr3\n\t"

            "wsrlhg wr3, wr15, wCGR1\n\t" 
            "wsrlhg wr2, wr2, wCGR0\n\t"
            "wor wr2, wr2, wr3\n\t"
            "wor wr2, wr2, wr1\n\t"

            "wstrd wr2, [%2]\n\t"

            "wor wr1, wr0, wr10\n\t"
            "wmulum wr4, wr4, wr8\n\t"
            "wsllhg wr2, wr0, wCGR0\n\t"
            "wmulum wr1, wr1, wr9\n\t"
            "wor wr2, wr2, wr11\n\t"
            "waddhus wr4, wr4, wr1\n\t"


            "wmulum wr5, wr5, wr8\n\t"
            "wsllhg wr3, wr0, wCGR1\n\t"
            "wmulum wr2, wr2, wr9\n\t"
            "wor wr3, wr3, wr10\n\t"
            "waddhus wr5, wr5, wr2\n\t"

            "wldrdne wr0, [r2]\n\t"

            "wmulum wr7, wr7, wr8\n\t"
            "wand wr1, wr4, wr6\n\t" 

            "wmulum wr3, wr3, wr9\n\t"
            "wand wr2, wr5, wr12\n\t" 
            "waddhus wr7, wr7, wr3\n\t"

            "wsrlhg wr3, wr7, wCGR1\n\t" 
            "wsrlhg wr2, wr2, wCGR0\n\t"
            "wor wr2, wr2, wr3\n\t"
            "wor wr2, wr2, wr1\n\t"

            "wstrd wr2, [%2, #8]\n\t"

            "sub %2, %2, %4\n\t"

            "bne 2b\n\t"




            : /* output */ "=r"(line), "=r"(out)
            : /* input */ "0"(line), "r"(length), "r"(step), "r"(alpha32), "r"(alpha_inv)
            : /* clobber */ "wr0", "wr1", "wr2", "wr3", "wr4", "wr5", "wr6", "wr7", "wr8", "wr9", "wr10", "wr11", "wr12", "wr13", "wr14", "wr15", "r1", "r2", "r3");

}

/*
   // Load
   wldrd wr0, mem

   // Red
   wor wr1, wr0, 0x07FF 07FF 07FF 07FF

   // Green
   wsll wr1, wr0, #5
   wor wr1, wr1, 0x03FF 03FF 03FF 03FF

   // Blue
   wsll wr1, wr0, #11
   wor wr1, wr1, 0x07FF 07FF 07FF 07FF


   // Manip
   // acc_new = acc_old + (val - acc_old) * alpha
   // acc_new = acc_old * (1 - alpha) + val * alpha
   // wr10 = acc
   // wr11 = alpha * 0x10000
   // wr12 = (1 - alpha) * 0x10000
   wmulum wr10, wr10, wr12
   wmulum wr1, wr1, wr11
   waddhus wr10, wr10, wr1
   * 3

   // Store
   wand wr2, wr10, #0xF800 F800 F800 F800 // red
   wslr wr3, wr10, #11                    // blue
   wand wr1, wr10, #0xFC00 FC00 FC00 FC00 // green
   wslr wr1, wr1, #5
   wor wr1, wr1, wr2
   wor wr1, wr1, wr3

   wstrd wr1, mem

   =5.5 instructions/pixel
 */
// alpha32 = 0 - 0x10000 (but obviously can never *be* 0x10000)
// line must be 64 bit aligned
void mmx16_blurcol_4(unsigned short *line, int length, int step, int alpha32)
{
    int out = 0;

    int alpha_inv = 0x10000 - alpha32;
    alpha_inv |= alpha_inv << 16;
    alpha32 |= alpha32 << 16;

#ifdef SIMPLIFIED_ASM
    asm volatile(
            // Setup masks
            "ldr r1, =0x07FF07FF\n\t"
            "tmcrr wr10, r1, r1\n\t" // 07FF 07FF 07FF 07FF

            "ldr r1, =0x03FF03FF\n\t"
            "tmcrr wr11, r1, r1\n\t" // 03FF 03FF 03FF 03FF

            "ldr r1, =0xF800F800\n\t"
            "tmcrr wr6, r1, r1\n\t" // F800 F800 F800 F800

            "ldr r1, =0xFC00FC00\n\t"
            "tmcrr wr12, r1, r1\n\t" // FC00 FC00 FC00 FC00

            "ldr r1, =5\n\t"
            "tmcr wCGR0, r1\n\t" // 5

            "ldr r1, =11\n\t"
            "tmcr wCGR1, r1\n\t" // 11

            // Reset values
            // wzero wr13 // red
            // wzero wr14 // green
            // wzero wr15 // blue
            "tbcstw wr9, %5\n\t" // wr9 = alpha
            "tbcstw wr8, %6\n\t" // wr8 = 1 - alpha

#ifdef DITHER
            // Setup accum
            "wzero wr4\n\t" // red
            "wzero wr5\n\t" // green
            "wzero wr7\n\t" // blue
#endif

            // Setup initial accum 
            "wldrd wr0, [%2]\n\t"
            "wor wr13, wr0, wr10\n\t"
            "wsllhg wr1, wr0, wCGR0\n\t"
            "wor wr14, wr1, wr11\n\t"
            "wsllhg wr1, wr0, wCGR1\n\t"
            "wor wr15, wr1, wr10\n\t"

            "mov r3, %3\n\t"

            "1:\n\t"

            // Load
            "wldrd wr0, [%2]\n\t"

            // Red
            "wor wr1, wr0, wr10\n\t"
            "wmulum wr13, wr13, wr8\n\t"
            "wmulum wr1, wr1, wr9\n\t"
            "waddhus wr13, wr13, wr1\n\t"

            // Green
            "wsllhg wr1, wr0, wCGR0\n\t"
            "wor wr1, wr1, wr11\n\t"
            "wmulum wr14, wr14, wr8\n\t"
            "wmulum wr1, wr1, wr9\n\t"
            "waddhus wr14, wr14, wr1\n\t"

            // Blue
            "wsllhg wr1, wr0, wCGR1\n\t"
            "wor wr1, wr1, wr10\n\t"
            "wmulum wr15, wr15, wr8\n\t"
            "wmulum wr1, wr1, wr9\n\t"
            "waddhus wr15, wr15, wr1\n\t"

#ifdef DITHER
            // Dither
            // Red
            "wsubhus wr2, wr13, wr4\n\t"
            "wor wr3, wr2, wr10\n\t"
            "wsubbus wr4, wr3, wr2\n\t"
            "wand wr1, wr2, wr6\n\t"  // wr1 == dithered red

            // Green
            "wsubhus wr2, wr14, wr5\n\t"
            "wor wr3, wr2, wr11\n\t"
            "wsubbus wr5, wr3, wr2\n\t"
            "wand wr2, wr2, wr12\n\t" 
            "wsrlhg wr2, wr2, wCGR0\n\t"
            "wor wr1, wr1, wr2\n\t" // wr1 == dithered red/green/blue

            // Blue
            "wsubhus wr2, wr15, wr7\n\t"
            "wor wr3, wr2, wr10\n\t"
            "wsubbus wr7, wr3, wr2\n\t"
            "wsrlhg wr2, wr2, wCGR1\n\t" 
            "wor wr1, wr1, wr2\n\t" // wr1 == dithered red/blue

            "wstrd wr1, [%2]\n\t"
#else
            // Store
            "wand wr1, wr13, wr6\n\t" 
            "wsrlhg wr2, wr15, wCGR1\n\t" 
            "wand wr3, wr14, wr12\n\t" 
            "wsrlhg wr3, wr3, wCGR0\n\t"
            "wor wr3, wr3, wr2\n\t"
            "wor wr3, wr3, wr1\n\t"

            "wstrd wr3, [%2]\n\t"
#endif


            "add %2, %2, %4\n\t"
            "subs r3, r3, #1\n\t"

            "bne 1b\n\t"

            // Reset
            "sub %2, %2, %4\n\t"
            "mov r3, %3\n\t"

            "2:\n\t"

            // Load
            "wldrd wr0, [%2]\n\t"

            // Red
            "wor wr1, wr0, wr10\n\t"
            "wmulum wr13, wr13, wr8\n\t"
            "wmulum wr1, wr1, wr9\n\t"
            "waddhus wr13, wr13, wr1\n\t"

            // Green
            "wsllhg wr1, wr0, wCGR0\n\t"
            "wor wr1, wr1, wr11\n\t"
            "wmulum wr14, wr14, wr8\n\t"
            "wmulum wr1, wr1, wr9\n\t"
            "waddhus wr14, wr14, wr1\n\t"

            // Blue
            "wsllhg wr1, wr0, wCGR1\n\t"
            "wor wr1, wr1, wr10\n\t"
            "wmulum wr15, wr15, wr8\n\t"
            "wmulum wr1, wr1, wr9\n\t"
            "waddhus wr15, wr15, wr1\n\t"


#ifdef DITHER
            // Dither
            // Red
            "wsubhus wr2, wr13, wr4\n\t"
            "wor wr3, wr2, wr10\n\t"
            "wsubbus wr4, wr3, wr2\n\t"
            "wand wr1, wr2, wr6\n\t"  // wr1 == dithered red

            // Green
            "wsubhus wr2, wr14, wr5\n\t"
            "wor wr3, wr2, wr11\n\t"
            "wsubbus wr5, wr3, wr2\n\t"
            "wand wr2, wr2, wr12\n\t" 
            "wsrlhg wr2, wr2, wCGR0\n\t"
            "wor wr1, wr1, wr2\n\t" // wr1 == dithered red/green/blue

            // Blue
            "wsubhus wr2, wr15, wr7\n\t"
            "wor wr3, wr2, wr10\n\t"
            "wsubbus wr7, wr3, wr2\n\t"
            "wsrlhg wr2, wr2, wCGR1\n\t" 
            "wor wr1, wr1, wr2\n\t" // wr1 == dithered red/blue

            "wstrd wr1, [%2]\n\t"
#else
            // Store
            "wand wr1, wr13, wr6\n\t" 
            "wsrlhg wr2, wr15, wCGR1\n\t" 
            "wand wr3, wr14, wr12\n\t" 
            "wsrlhg wr3, wr3, wCGR0\n\t"
            "wor wr3, wr3, wr2\n\t"
            "wor wr3, wr3, wr1\n\t"

            "wstrd wr3, [%2]\n\t"
#endif

            "sub %2, %2, %4\n\t"
            "subs r3, r3, #1\n\t"

            "bne 2b\n\t"

            : /* output */ "=r"(line), "=r"(out)
            : /* input */ "0"(line), "r"(length), "r"(step), "r"(alpha32), "r"(alpha_inv)
            : /* clobber */ "wr0", "wr1", "wr2", "wr3", "wr4", "wr5", "wr6", "wr7", "wr8", "wr9", "wr10", "wr11", "wr12", "wr13", "wr14", "wr15", "r1", "r2", "r3");

#else // SIMPLIFIED_ASM
    asm volatile(
            // Setup masks
            "ldr r1, =0x07FF07FF\n\t"
            "tmcrr wr10, r1, r1\n\t" // 07FF 07FF 07FF 07FF

            "ldr r1, =0x03FF03FF\n\t"
            "tmcrr wr11, r1, r1\n\t" // 03FF 03FF 03FF 03FF

            "ldr r1, =5\n\t"
            "tmcr wCGR0, r1\n\t" // 5

            "ldr r1, =11\n\t"
            "tmcr wCGR1, r1\n\t" // 11

            // Reset values
            // wzero wr13 // red
            // wzero wr14 // green
            // wzero wr15 // blue
            "tbcstw wr9, %5\n\t" // wr9 = alpha
            "tbcstw wr8, %6\n\t" // wr8 = 1 - alpha

#ifdef DITHER
            // Setup accum
            "wzero wr4\n\t" // red
            "wzero wr5\n\t" // green
            "wzero wr7\n\t" // blue
#endif

            // Setup initial accum 
            "wldrd wr0, [%2]\n\t"
            "wor wr13, wr0, wr10\n\t"
            "wsllhg wr1, wr0, wCGR0\n\t"
            "wor wr14, wr1, wr11\n\t"
            "wsllhg wr1, wr0, wCGR1\n\t"
            "wor wr15, wr1, wr10\n\t"

            // Load (pre)
            "wldrd wr0, [%2]\n\t"

            "mov r3, %3\n\t"

            "add r2, %2, %4\n\t"

            "1:\n\t"

            "subs r3, r3, #1\n\t"

            "wor wr1, wr0, wr10\n\t"
            "wmulum wr13, wr13, wr8\n\t"
            "wsllhg wr2, wr0, wCGR0\n\t"
            "wmulum wr1, wr1, wr9\n\t"
            "wor wr2, wr2, wr11\n\t"
            "waddhus wr13, wr13, wr1\n\t"


            "wmulum wr14, wr14, wr8\n\t"
            "wsllhg wr3, wr0, wCGR1\n\t"
            "wmulum wr2, wr2, wr9\n\t"
            "wor wr3, wr3, wr10\n\t"
            "waddhus wr14, wr14, wr2\n\t"


            // Load
            "wldrdne wr0, [r2]\n\t"
            "add r2, r2, %4\n\t"

            "wmulum wr15, wr15, wr8\n\t"
#ifdef DITHER
            "wsubhus wr2, wr13, wr4\n\t"
            "wor wr6, wr2, wr10\n\t"
            "wsubhus wr4, wr6, wr2\n\t"
            "wandn wr1, wr2, wr10\n\t"  // wr1 == dithered red
#else
            "wandn wr1, wr13, wr10\n\t" 
#endif

            "wmulum wr3, wr3, wr9\n\t"
#ifdef DITHER
            "wsubhus wr2, wr14, wr5\n\t"
            "wor wr6, wr2, wr11\n\t"
            "wsubhus wr5, wr6, wr2\n\t"
            "wandn wr2, wr2, wr11\n\t" 
            "wsrlhg wr2, wr2, wCGR0\n\t"
            "wor wr1, wr1, wr2\n\t" // wr1 == dithered red/green
#else
            "wandn wr2, wr14, wr11\n\t" 
#endif
            "waddhus wr15, wr15, wr3\n\t"

#ifdef DITHER
            "wsubhus wr2, wr15, wr7\n\t"
            "wor wr3, wr2, wr10\n\t"
            "wsubhus wr7, wr3, wr2\n\t"
            "wsrlhg wr2, wr2, wCGR1\n\t" 
            "wor wr1, wr1, wr2\n\t" // wr1 == dithered red/green/blue
#else
            "wsrlhg wr3, wr15, wCGR1\n\t" 
            "wsrlhg wr2, wr2, wCGR0\n\t"
            "wor wr2, wr2, wr3\n\t"
            "wor wr2, wr2, wr1\n\t"
#endif

#ifdef DITHER
            "wstrd wr1, [%2]\n\t"
#else
            "wstrd wr2, [%2]\n\t"
#endif

            "add %2, %2, %4\n\t"

            "bne 1b\n\t"

            // Reset
            "sub %2, %2, %4\n\t"
            // Load (pre)
            "wldrd wr0, [%2]\n\t"
            "sub r2, %2, %4\n\t"
            "mov r3, %3\n\t"

            "2:\n\t"

            "subs r3, r3, #1\n\t"

            "wor wr1, wr0, wr10\n\t"
            "wmulum wr13, wr13, wr8\n\t"
            "wsllhg wr2, wr0, wCGR0\n\t"
            "wmulum wr1, wr1, wr9\n\t"
            "wor wr2, wr2, wr11\n\t"
            "waddhus wr13, wr13, wr1\n\t"


            "wmulum wr14, wr14, wr8\n\t"
            "wsllhg wr3, wr0, wCGR1\n\t"
            "wmulum wr2, wr2, wr9\n\t"
            "wor wr3, wr3, wr10\n\t"
            "waddhus wr14, wr14, wr2\n\t"


            // Load
            "wldrdne wr0, [r2]\n\t"
            "sub r2, r2, %4\n\t"

            "wmulum wr15, wr15, wr8\n\t"
#ifdef DITHER
            "wsubhus wr2, wr13, wr4\n\t"
            "wor wr6, wr2, wr10\n\t"
            "wsubhus wr4, wr6, wr2\n\t"
            "wandn wr1, wr2, wr10\n\t"  // wr1 == dithered red
#else
            "wandn wr1, wr13, wr10\n\t" 
#endif

            "wmulum wr3, wr3, wr9\n\t"
#ifdef DITHER
            "wsubhus wr2, wr14, wr5\n\t"
            "wor wr6, wr2, wr11\n\t"
            "wsubhus wr5, wr6, wr2\n\t"
            "wandn wr2, wr2, wr11\n\t" 
            "wsrlhg wr2, wr2, wCGR0\n\t"
            "wor wr1, wr1, wr2\n\t" // wr1 == dithered red/green
#else
            "wandn wr2, wr14, wr11\n\t" 
#endif
            "waddhus wr15, wr15, wr3\n\t"

#ifdef DITHER
            "wsubhus wr2, wr15, wr7\n\t"
            "wor wr3, wr2, wr10\n\t"
            "wsubhus wr7, wr3, wr2\n\t"
            "wsrlhg wr2, wr2, wCGR1\n\t" 
            "wor wr1, wr1, wr2\n\t" // wr1 == dithered red/green/blue
#else
            "wsrlhg wr3, wr15, wCGR1\n\t" 
            "wsrlhg wr2, wr2, wCGR0\n\t"
            "wor wr2, wr2, wr3\n\t"
            "wor wr2, wr2, wr1\n\t"
#endif

#ifdef DITHER
            "wstrd wr1, [%2]\n\t"
#else
            "wstrd wr2, [%2]\n\t"
#endif

            "sub %2, %2, %4\n\t"

            "bne 2b\n\t"



            : /* output */ "=r"(line), "=r"(out)
            : /* input */ "0"(line), "r"(length), "r"(step), "r"(alpha32), "r"(alpha_inv)
            : /* clobber */ "wr0", "wr1", "wr2", "wr3", "wr4", "wr5", "wr6", "wr7", "wr8", "wr9", "wr10", "wr11", "wr12", "wr13", "wr14", "wr15", "r1", "r2", "r3");
#endif
}

/*
    // Load
    wlrdw wr0, mem
    wshufh wr1, wr0, #0x00
    wmulul wr1, wr1, #0x0001 0001 0020 0800 // wr1 = rrrr r000 0000 0000
                                            //       rrrr r000 0000 0000
                                            //       gggg gg00 0000 0000
                                            //       bbbb b000 0000 0000
    wor wr1, wr1, 0x07FF 07FF 03FF 07FF
    
    // Manip
    // acc_new = acc_old + (val - acc_old) * alpha
    // acc_new = acc_old * (1 - alpha) + val * alpha
    // wr10 = acc
    // wr11 = alpha * 0x10000
    // wr12 = (1 - alpha) * 0x10000
    wmulum wr10, wr10, wr12
    wmulum wr1, wr1, wr11
    waddhus wr10, wr10, wr1




    // Store
    // Lower
    wmulum wr1, wr10, #0x0000 0020 0040 0020 // wr1 = 0000 0000 0000 0000
                                            //       0000 0000 000r rrrr
                                            //       0000 0000 00gg gggg
                                            //       0000 0000 000b bbbb
    wmacuz wr1, wr1, #0x0000 0800 0020 0001 // wr1 = 0000 0000 0000 0000
                                            //       0000 0000 0000 0000
                                            //       0000 0000 0000 0000
                                            //       rrrr rggg gggb bbbb

    // Upper
    wand wr2, wr10, #0xF800 F800 FC00 F800
    wmacu wr1, wr2, #0x8000 8000 0800 0020 // wr1 = 0000 0000 0000 0000
                                           //       0000 0000 0000 0000
                                           //       rrrr rggg gggb bbbb
                                           //       rrrr rggg gggb bbbb
    
    wstrw wr1, mem

    = 9 instructions/pixel
 */
// alpha32 = 0 - 0x10000 (but obviously can never *be* 0x10000)
void mmx16_blurrow(unsigned short *line, int length, int alpha32)
{
    int out = 0;

    int pre_post = 0; // 0x00000001 - pre, 0x00000002 - pos
    int alpha_inv = 0x10000 - alpha32;
    alpha_inv |= alpha_inv << 16;
    alpha32 |= alpha32 << 16;

    // Make 32-bit aligned
    if((unsigned int)line & 0x3) {
        pre_post |= 0x00000001;
        length--;
    }
    // Make length a multiple of 2
    if(length & 0x1) {
        pre_post |= 0x00000002;
        length--;
    }

    length >>= 1;

#ifdef SIMPLIFIED_ASM
    asm volatile (
            // Setup masks
            "ldr r1, =0xF800F800\n\t"
            "ldr r2, =0x07E0001F\n\t" 
            "tmcrr wr10, r2, r1\n\t" // F800 F800 07E0 001F

            "ldr r1, =0x00010001\n\t"
            "ldr r2, =0x00200800\n\t"
            "tmcrr wr11, r2, r1\n\t" // 0001 0001 0020 0800

            "ldr r1, =0x00000020\n\t"
            "ldr r2, =0x00400020\n\t"
            "tmcrr wr12, r2, r1\n\t" // 0000 0020 0040 0020

            "ldr r1, =0x00000800\n\t"
            "ldr r2, =0x00200001\n\t"
            "tmcrr wr13, r2, r1\n\t" // 0000 0800 0020 0001

            "ldr r1, =0xF800F800\n\t"
            "ldr r2, =0xFC00F800\n\t"
            "tmcrr wr14, r2, r1\n\t" // F800 F800 FC00 F800

            "ldr r1, =0x07ff07ff\n\t"
            "ldr r2, =0x03ff07ff\n\t"
            "tmcrr wr6, r2, r1\n\t"  // 07FF 07FF 03FF 07FF

            "ldr r1, =0x80008000\n\t"
            "ldr r2, =0x08000020\n\t"
            "tmcrr wr15, r2, r1\n\t" // 8000 8000 0800 0020


            // Setup accum
            "wzero wr3\n\t"

            // Setup init values
            // "wzero wr9\n\t" // wr9 - accum
            "tbcstw wr8, %4\n\t" // wr8 = alpha
            "tbcstw wr7, %5\n\t" // wr7 = 1 - alpha

            // Setup initial accum
            "wldrh wr0, [%2]\n\t"
            "wshufh wr1, wr0, #0x00\n\t"
            "wand wr1, wr1, wr10\n\t"
            "wmulul wr9, wr1, wr11\n\t"

            // Do pre-pixel
            "ands r3, %6, #0x1\n\t"
            "wstrhne wr0, [%2], #2\n\t"

            "movs r3, %3\n\t"
            "beq 2f\n\t"

            "1:\n\t"

            "wldrw wr0, [%2]\n\t"

            // Load - lower
            "wshufh wr1, wr0, #0x00\n\t"
            "wmulul wr1, wr1, wr11\n\t"
            "wor wr1, wr1, wr6\n\t"

            // Manip - lower
            "wmulum wr9, wr9, wr7\n\t"
            "wmulum wr1, wr1, wr8\n\t"
            "waddhus wr9, wr9, wr1\n\t"

#ifdef DITHER
            // Dither - lower
            "wsubhus wr4, wr9, wr3\n\t"
            "wor wr2, wr4, wr6\n\t"
            "wsubbus wr3, wr2, wr4\n\t"

            // Store - lower
            "wmulum wr1, wr2, wr12\n\t" // XXX
#else
            "wmulum wr1, wr9, wr12\n\t"
#endif
            "wmacuz wr1, wr1, wr13\n\t"

            // Load - upper
            "wshufh wr2, wr0, #0x55\n\t"
            "wmulul wr2, wr2, wr11\n\t"
            "wor wr2, wr2, wr6\n\t"

            // Manip - upper
            "wmulum wr9, wr9, wr7\n\t"
            "wmulum wr2, wr2, wr8\n\t"
            "waddhus wr9, wr9, wr2\n\t"

#ifdef DITHER
            // Dither - upper
            "wsubhus wr4, wr9, wr3\n\t"
            "wor wr2, wr4, wr6\n\t"
            "wsubbus wr3, wr2, wr4\n\t"

            // Store - upper
            "wand wr2, wr2, wr14\n\t" // XXX
#else
            "wand wr2, wr9, wr14\n\t"
#endif
            "wmacu wr1, wr2, wr15\n\t"

            "wstrw wr1, [%2], #4\n\t"

            "subs r3, r3, #1\n\t"

            "bne 1b\n\t"
            
            // Reset
            "sub %2, %2, #4\n\t"

            "2:\n\t"

            // Do post-pixel
            "ands r2, %6, #0x2\n\t"
            "beq 3f\n\t"
            "wldrh wr0, [%2, #2]\n\t"
            "wshufh wr1, wr0, #0x00\n\t"
            "wmulul wr1, wr1, wr11\n\t"
            "wor wr1, wr1, wr6\n\t"
            "wmulum wr9, wr9, wr7\n\t"
            "wmulum wr1, wr1, wr8\n\t"
            "waddhus wr9, wr9, wr1\n\t"
            "wmulum wr1, wr9, wr12\n\t"
            "wmacuz wr1, wr1, wr13\n\t"
            "wstrh wr1, [%2, #2]\n\t"


            "3:\n\t"
            "movs r3, %3\n\t"
            "beq 5f\n\t"


            "4:\n\t"

            "wldrw wr0, [%2]\n\t"

            // Load - upper
            "wshufh wr2, wr0, #0x55\n\t"
            "wmulul wr2, wr2, wr11\n\t"
            "wor wr2, wr2, wr6\n\t"

            // Manip - upper
            "wmulum wr9, wr9, wr7\n\t"
            "wmulum wr2, wr2, wr8\n\t"
            "waddhus wr9, wr9, wr2\n\t"

#ifdef DITHER
            // Dither - upper
            "wsubhus wr4, wr9, wr3\n\t"
            "wor wr2, wr4, wr6\n\t"
            "wsubbus wr3, wr2, wr4\n\t"

            // Store - upper
            "wand wr2, wr2, wr14\n\t" // XXX
#else
            // Store - upper
            "wand wr2, wr9, wr14\n\t"
#endif

            // Load - lower
            "wshufh wr1, wr0, #0x00\n\t"
            "wmulul wr1, wr1, wr11\n\t"
            "wor wr1, wr1, wr6\n\t"

            // Manip - lower
            "wmulum wr9, wr9, wr7\n\t"
            "wmulum wr1, wr1, wr8\n\t"
            "waddhus wr9, wr9, wr1\n\t"

#ifdef DITHER
            // Dither - lower
            "wsubhus wr4, wr9, wr3\n\t"
            "wor wr1, wr4, wr6\n\t"
            "wsubbus wr3, wr1, wr4\n\t"

            // Store - lower
            "wmulum wr1, wr1, wr12\n\t" // XXX
#else
            // Store - lower
            "wmulum wr1, wr9, wr12\n\t"
#endif
            "wmacuz wr1, wr1, wr13\n\t"
            "wmacu wr1, wr2, wr15\n\t"

            "wstrw wr1, [%2], #-4\n\t"

            "subs r3, r3, #1\n\t"

            "bne 4b\n\t"

            "5:\n\t"
            // Do pre-pixel
            "ands r2, %6, #0x1\n\t"
            "beq 6f\n\t"
            "wldrh wr0, [%2, #2]\n\t"
            "wshufh wr1, wr0, #0x00\n\t"
            "wmulul wr1, wr1, wr11\n\t"
            "wor wr1, wr1, wr6\n\t"
            "wmulum wr9, wr9, wr7\n\t"
            "wmulum wr1, wr1, wr8\n\t"
            "waddhus wr9, wr9, wr1\n\t"
            "wmulum wr1, wr9, wr12\n\t"
            "wmacuz wr1, wr1, wr13\n\t"
            "wstrh wr1, [%2, #2]\n\t"

            "6:\n\t"


            : /* output */ "=r"(line), "=r"(out)
            : /* input */ "0"(line), "r"(length), "r"(alpha32), "r"(alpha_inv), "r"(pre_post)
            : /* clobber */ "wr0", "wr1", "wr2", "wr3", "wr4", "wr5", "wr6", "wr7", "wr8", "wr9", "wr10", "wr11", "wr12", "wr13", "wr14", "wr15", "r1", "r2", "r3"
            );

#else
    asm volatile (
            // Setup masks
            "ldr r1, =0xF800F800\n\t"
            "ldr r2, =0x07E0001F\n\t"
            "tmcrr wr10, r2, r1\n\t" // F800 F800 07E0 001F

            "ldr r1, =0x00010001\n\t"
            "ldr r2, =0x00200800\n\t"
            "tmcrr wr11, r2, r1\n\t" // 0001 0001 0020 0800

            "ldr r1, =0x00000020\n\t"
            "ldr r2, =0x00400020\n\t"
            "tmcrr wr12, r2, r1\n\t" // 0000 0020 0040 0020

            "ldr r1, =0x00000800\n\t"
            "ldr r2, =0x00200001\n\t"
            "tmcrr wr13, r2, r1\n\t" // 0000 0800 0020 0001

            "ldr r1, =0xF800F800\n\t"
            "ldr r2, =0xFC00F800\n\t"
            "tmcrr wr14, r2, r1\n\t" // F800 F800 FC00 F800

            "ldr r1, =0x80008000\n\t"
            "ldr r2, =0x08000020\n\t"
            "tmcrr wr15, r2, r1\n\t" // 8000 8000 0800 0020

            "ldr r1, =0x07FF07FF\n\t"
            "ldr r2, =0x03FF07FF\n\t"
            "tmcrr wr6, r2, r1\n\t"  // 07FF 07FF 03FF 07FF

#ifdef DITHER
            // Setup accum
            "wzero wr4\n\t"
#endif

            // Setup init values
            // "wzero wr9\n\t" // wr9 - accum
            "tbcstw wr8, %4\n\t" // wr8 = alpha
            "tbcstw wr7, %5\n\t" // wr7 = 1 - alpha

            // Setup initial accum
            "wldrh wr0, [%2]\n\t"
            "wshufh wr1, wr0, #0x00\n\t"
            "wand wr1, wr1, wr10\n\t"
            "wmulul wr9, wr1, wr11\n\t"

            // Do pre-pixel
            "ands r2, %6, #0x1\n\t"
            "wstrhne wr0, [%2], #2\n\t"

            // Setup counter
            "movs r3, %3\n\t"
            "beq 2f\n\t"

            // Preload condition reg with negative
            "ldr r2, =0\n\t"
            "subs r2, r2, #1\n\t" 

            // Load (pre)
            "wldrw wr0, [%2]\n\t"

            "1:\n\t"
            "pld [%2, #32]\n\t"

            "wmulum wr9, wr9, wr7\n\t" 

            "wshufh wr1, wr0, #0x00\n\t"
            "wmulul wr1, wr1, wr11\n\t"
            "wshufh wr2, wr0, #0x55\n\t"
            "wmulul wr2, wr2, wr11\n\t"
            "wor wr1, wr1, wr6\n\t" 
            "wmulum wr1, wr1, wr8\n\t"
            "wor wr2, wr2, wr6\n\t" 

            "waddhus wr9, wr9, wr1\n\t"
#ifdef DITHER
            "wsubhus wr5, wr9, wr4\n\t"
            "wor wr1, wr5, wr6\n\t"
            "wsubbus wr4, wr1, wr5\n\t"

            "wmulum wr1, wr1, wr12\n\t"
#else
            "wmulum wr1, wr9, wr12\n\t"
#endif

            "wstrwpl wr3, [%2], #4\n\t"
            "wmulum wr9, wr9, wr7\n\t"
            "subs r3, r3, #1\n\t"

            "wmulum wr2, wr2, wr8\n\t"
            "wldrwne wr0, [%2, #4]\n\t" 
            "waddhus wr9, wr9, wr2\n\t"

            "wmacuz wr3, wr1, wr13\n\t"
            
#ifdef DITHER
            "wsubhus wr5, wr9, wr4\n\t"
            "wor wr2, wr5, wr6\n\t"
            "wsubbus wr4, wr2, wr5\n\t"

            "wand wr2, wr2, wr14\n\t"
#else
            "wand wr2, wr9, wr14\n\t"
#endif
            "wmacu wr3, wr2, wr15\n\t"
            "bne 1b\n\t"

            "wstrw wr3, [%2], #4\n\t"


            "2:\n\t"

            // Do post-pixel
            "ands r2, %6, #0x2\n\t"
            "beq 3f\n\t"
            "wldrh wr0, [%2, #2]\n\t"
            "wshufh wr1, wr0, #0x00\n\t"
            "wmulul wr1, wr1, wr11\n\t"
            "wor wr1, wr1, wr6\n\t"
            "wmulum wr9, wr9, wr7\n\t"
            "wmulum wr1, wr1, wr8\n\t"
            "waddhus wr9, wr9, wr1\n\t"
            "wmulum wr1, wr9, wr12\n\t"
            "wmacuz wr1, wr1, wr13\n\t"
            "wstrh wr1, [%2, #2]\n\t"

            "3:\n\t"
            "movs r3, %3\n\t"
            "beq 5f\n\t"

            // Load(pre)
            "wldrw wr0, [%2, #-4]\n\t"
            // Reset
            "ldr r2, =0\n\t"
            "subs r2, r2, #1\n\t" 
            "sub %2, %2, #4\n\t"

            "4:\n\t"
            "pld [%2, #-32]\n\t"
            "wmulum wr9, wr9, wr7\n\t"

            "wshufh wr2, wr0, #0x55\n\t"
            "wmulul wr2, wr2, wr11\n\t"
            "wshufh wr1, wr0, #0x00\n\t"
            "wmulul wr1, wr1, wr11\n\t"
            "wor wr2, wr2, wr6\n\t" 
            "wmulum wr2, wr2, wr8\n\t"
            "wor wr1, wr1, wr6\n\t"

            "waddhus wr9, wr9, wr2\n\t"

            "wmulum wr1, wr1, wr8\n\t"

#ifdef DITHER
            "wsubhus wr5, wr9, wr4\n\t"
            "wor wr2, wr5, wr6\n\t"
            "wsubbus wr4, wr2, wr5\n\t"

            "wand wr2, wr2, wr14\n\t"
#else
            "wand wr2, wr9, wr14\n\t"
#endif

            "wmulum wr9, wr9, wr7\n\t"
            "wstrwpl wr3, [%2], #-4\n\t"
            "waddhus wr9, wr9, wr1\n\t"

#ifdef DITHER
            "wsubhus wr5, wr9, wr4\n\t"
            "wor wr1, wr5, wr6\n\t"
            "wsubbus wr4, wr1, wr5\n\t"

            "wmulum wr1, wr1, wr12\n\t"
#else
            "wmulum wr1, wr9, wr12\n\t"
#endif
            "subs r3, r3, #1\n\t"

            "wmacuz wr3, wr1, wr13\n\t"
            "wldrwne wr0, [%2, #-4]\n\t"

            "wmacu wr3, wr2, wr15\n\t"
            "bne 4b\n\t"

            "wstrw wr3, [%2], #-4\n\t"

            "5:\n\t"
            // Do pre-pixel
            "ands r2, %6, #0x1\n\t"
            "beq 6f\n\t"
            "wldrh wr0, [%2, #2]\n\t"
            "wshufh wr1, wr0, #0x00\n\t"
            "wmulul wr1, wr1, wr11\n\t"
            "wor wr1, wr1, wr6\n\t"
            "wmulum wr9, wr9, wr7\n\t"
            "wmulum wr1, wr1, wr8\n\t"
            "waddhus wr9, wr9, wr1\n\t"
            "wmulum wr1, wr9, wr12\n\t"
            "wmacuz wr1, wr1, wr13\n\t"
            "wstrh wr1, [%2, #2]\n\t"

            "6:\n\t"

            : /* output */ "=r"(line), "=r"(out)
            : /* input */ "0"(line), "r"(length), "r"(alpha32), "r"(alpha_inv), "r"(pre_post)
            : /* clobber */ "wr0", "wr1", "wr2", "wr3", "wr4", "wr5", "wr6", "wr7", "wr8", "wr9", "wr10", "wr11", "wr12", "wr13", "wr14", "wr15", "r1", "r2", "r3"
            );
#endif
}

void mmx_blur16(unsigned short *img, int width, int height, int step_width, int alpha32)
{
    int row, col;
    int ii;

    // Blur rows
    for(row = 0; row < height; ++row) 
        mmx16_blurrow(img + row * step_width, width, alpha32);

#if 1
#if 0
    while(width--)
        mmx16_blurcol(img++, height, step_width * 2, alpha32);
#else
    // Blur cols
    if(width && (unsigned int)img & 0x2) { // 16-bit aligned
        mmx16_blurcol(img++, height, step_width * 2, alpha32);
        width--;
    }
    if(width >= 2 && (unsigned int)img & 0x3) { // 32-bit aligned
        mmx16_blurcol_2(img, height, step_width * 2, alpha32);
        img += 2;
        width -= 2;
    }

#if 0
    while(width >= 8) {
        mmx16_blurcol_8(img, height, step_width * 2, alpha32);
        img += 8;
        width -= 8;
    }

    if(width >= 4) {
        mmx16_blurcol_4(img, height, step_width * 2, alpha32);
        img += 4;
        width -= 4;
    }
#else
    while(width >= 4) {
        mmx16_blurcol_4(img, height, step_width * 2, alpha32);
        img += 4;
        width -= 4;
    }
#endif

    if(width >= 2) {
        mmx16_blurcol_2(img, height, step_width * 2, alpha32);
        img += 2;
        width -= 2;
    }

    if(width) {
        mmx16_blurcol(img, height, step_width * 2, alpha32);
        img += 1;
        width -= 1;
    }
#endif
#endif
}

