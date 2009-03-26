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

static void mmx_grayscale_rgb16_opacity_inplace(unsigned short *src, 
                                                unsigned char opacity,
                                                int width)
{
    // XXX alignment!!!
    asm volatile (
            // Grayscale Setup
            "ldr r6, =0\n\t"
            "ldr r5, =11\n\t"
            "tmcrr wr6, r5, r6\n\t" // wr6 = 11
            "ldr r5, =5\n\t"
            "tmcrr wr7, r5, r6\n\t" // wr7 = 5
            "ldr r5, =0xFC00\n\t"
            "tbcsth wr11, r5\n\t" // wr11 = 0xFC00FC00FC00FC00

            // Blend setup
            "wzero wr12\n\t"
            "ldr r6, =0x00400040\n\t"
            "tmcrr wr13, r6, r6\n\t"
            "ldr r6, =0xF81FF81F\n\t"
            "tmcrr wr14, r6, r6\n\t"
            "ldr r6, =0x00010001\n\t"
            "tmcrr wr15, r6, r6\n\t"

            "ldr r6, =1\n\t"
            "tmcr wCGR3, r6\n\t"
            "ldr r6, =2\n\t"
            "tmcr wCGR2, r6\n\t"
            "ldr r6, =6\n\t"
            "tmcr wCGR1, r6\n\t"
            "ldr r6, =15\n\t"
            "tmcr wCGR0, r6\n\t"

            // Setup paired alpha in wr2 and wr4
            "mov r5, %3, lsr #2\n\t"
            "add r5, r5, #1\n\t"
            "ldr r6, =0x40\n\t"
            "sub r6, r6, r5\n\t"
            "orr r5, r5, r6, LSL #16\n\t"
            "tbcstw wr4, r5\n\t"
            "wmov wr2, wr4\n\t"

            "mov r6, %2\n\t"

            "1:\n\t"
            // Data is in wr10
            "wldrd wr10, [%1]\n\t"

            "subs r6, r6, #4\n\t"
            "pld [%1, #32]\n\t"

             // Convert to grey scale
            "wsllh wr1, wr10, wr6\n\t"
            "wavg2h wr1, wr1, wr10\n\t"

            // may not be necessary (omitting would introduce a small error)
            // "wand wr1, wr1, wr14\n\t"
            "wsllh wr3, wr10, wr7\n\t"
            "wavg2h wr1, wr3, wr1\n\t" // top 6 are grayscale, rest is bogus
            "wand wr1, wr1, wr11\n\t" // top 6 are grayscale, rest is zero

            "wsrlh wr3, wr1, wr6\n\t"
            "wor wr1, wr3, wr1\n\t" // top 6 are gray scale, bottom 5 are gray
            "wsrlh wr3, wr1, wr7\n\t"
            "wand wr1, wr1, wr14\n\t" // g506g5
            "wor wr8, wr3, wr1\n\t" // g5g6g5

            //
            // BLEND START
            // 
            "wunpckihh wr3, wr8, wr10\n\t" // wr3 = RGB(0)rgb(0)RGB(1)rgb(1)
            "wunpckilh wr1, wr8, wr10\n\t" // wr1 = RGB(2)rgb(2)RGB(3)rgb(3)

            // Mask colors
            "wand wr0, wr1, wr14\n\t" // wr0 = R0B(2)r0b(2)R0B(3)r0b(3)
            "wandn wr1, wr1, wr14\n\t" // wr1 = 0G0(2)0g0(2)0G0(3)0g0(3)
            "wandn wr5, wr3, wr14\n\t" // wr5 = 0G0(0)0g0(0)0G0(1)0g0(1)
            "wand wr3, wr3, wr14\n\t" // wr3 = R0B(0)r0b(0)R0B(1)r0b(1)

            // Multiply and align 
            "wmaddu wr0, wr0, wr2\n\t" 
            "wsrlwg wr0, wr0, wCGR1\n\t"  // 0000 ROB(2) 0000 R0B(3)
            "wmaddu wr1, wr1, wr2\n\t" 
            "wsrlwg wr1, wr1, wCGR1\n\t"  // 0000 0G0(2) 0000 0G0(2)

            "wmaddu wr3, wr3, wr4\n\t" 
            "wsrlwg wr3, wr3, wCGR1\n\t"  // 0000 R0B(0) 0000 R0B(1)
            "wmaddu wr5, wr5, wr4\n\t" 
            "wsrlwg wr5, wr5, wCGR1\n\t"  // 0000 0G0(0) 0000 0G0(1)

            // Repack
            "wpackwus wr0, wr0, wr3\n\t" // R0B(0) R0B(1) R0B(2) R0B(3)
            "wpackwus wr1, wr1, wr5\n\t" // 0G0(0) 0G0(1) 0G0(2) 0G0(3)

            // Clean
            "wand wr0, wr0, wr14\n\t" // R0B(0) R0B(1) R0B(2) R0B(3)
            "wandn wr1, wr1, wr14\n\t" // 0G0(0) 0G0(1) 0G0(2) 0G0(3)

            // Unite and store
            "wor wr0, wr0, wr1\n\t" // RGB(0) RGB(1) RGB(2) RGB(3)
            //
            // BLEND END
            // 


            // Store
            "wstrd wr0, [%1], #8\n\t"

            "bne 1b\n\t"

            : /*output*/"=r"(src)
            : /*input*/"r"(src), "r"(width), "r"(opacity)
            : /*clobber*/ "r5", "r6", "wr0", "wr1", "wr2"
        );
}

static void mmx_grayscale_rgb16_inplace(unsigned short *src, 
                                        int width)
{
    // XXX alignment!!!
    asm volatile (
            // Setup
            // wcgr0 == 11
            // wcgr1 == 5
            "mov r5, #11\n\t"
            "tmcr wcgr0, r5\n\t"
            "mov r5, #5\n\t"
            "tmcr wcgr1, r5\n\t"

            "mov r5, #0xF800\n\t"
            "orr r5, r5, #0x1F\n\t"
            "tbcsth wr15, r5\n\t" // wr15 = 0xF81FF81FF81FF81F
            "mov r5, #0xFC00\n\t"
            "tbcsth wr14, r5\n\t" // wr14 = 0xFC00FC00FC00FC00

            "mov r6, %2\n\t"

            "1:\n\t"
            // Data is in wr0
            "wldrd wr0, [%1]\n\t"

            "subs r6, r6, #4\n\t"

            "pld [%1, #32]\n\t"

             // Convert to grey scale
            "wsllhg wr1, wr0, wcgr0\n\t"
            "wavg2h wr1, wr1, wr0\n\t"
            // may not be necessary (omitting would introduce a small error)
            // "wand wr1, wr1, wr15\n\t"
            "wsllhg wr2, wr0, wcgr1\n\t"
            "wavg2h wr1, wr2, wr1\n\t" // top 6 are grayscale, rest is bogus
            "wand wr1, wr1, wr14\n\t" // top 6 are grayscale, rest is zero

            "wsrlhg wr2, wr1, wcgr0\n\t"
            "wor wr1, wr2, wr1\n\t" // top 6 are gray scale, bottom 5 are gray
            "wsrlhg wr2, wr1, wcgr1\n\t"
            "wand wr1, wr1, wr15\n\t" // g506g5
            "wor wr2, wr2, wr1\n\t" // g5g6g5

            // Store
            "wstrd wr2, [%1], #8\n\t"

            "bne 1b\n\t"

            : /*output*/"=r"(src)
            : /*input*/"r"(src), "r"(width)
            : /*clobber*/ "r5", "r6", "wr0", "wr1", "wcgr0", "wcgr1", "wr2"
        );
}



void mmx_grayscale_rgb16(unsigned short *src, unsigned char opacity,
                         int width, unsigned short *output)
{
    // XXX dither?
    if(src == output) {
        if(opacity != 0xFF) 
            mmx_grayscale_rgb16_opacity_inplace(src, opacity, width);
        else
            mmx_grayscale_rgb16_inplace(src, width);
    } else {
        printf("mmx: Non-inplace grayscale not implemented\n");
    }
}
