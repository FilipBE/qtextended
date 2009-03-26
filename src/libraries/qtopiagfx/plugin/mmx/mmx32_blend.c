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

#include <stdio.h>

void mmx_blend_argb32p_rgb32(unsigned int *dest,
                             unsigned int *src,
                             unsigned char opacity,
                             int width,
                             unsigned int *output)
{
    int out = 0;
    // XXX
    opacity = opacity;

    asm volatile (
            "wzero wr12\n\t"
            "ldr r1, =0xFF000000\n\t"
            "tmcrr wr13, r1, r1\n\t"
            "ldr r1, =0x00FF00FF\n\t"
            "tmcrr wr14, r1, r1\n\t"

            "ldr r6, =0x00FF00FF\n\t"
            "ldr r8, =0x0000FF00\n\t"
            "ldr r3, =0xFF\n\t"
            "ldr r9, =0xFF000000\n\t"

            // Load (pre)
            "ldr r1, [%1], #4\n\t"

            "mov r2, %6\n\t"
            
            "1:\n\t"

            "movs r4, r1, lsr #24\n\t"
            "beq 2f\n\t"
            "subs r5, r3, r4\n\t"
            "beq 3f\n\t"

            "wldrw wr0, [%2], #4\n\t" // wr0 = DEST

            "tbcstw wr1, r1\n\t"
            "tbcsth wr4, r5, lsl #8\n\t" // wr3 = 0A0A0A0A

            "wunpckilb wr2, wr12, wr0\n\t" // wr2 = F0R0G0B0
            "wmulum wr2, wr2, wr4\n\t"     // wr2 = 0x0R0G0B

            "subs r2, r2, #1\n\t"

            "wpackhus wr2, wr2, wr12\n\t"    // wr2 = 0000xRGB

            "ldrne r1, [%1], #4\n\t"

            "waddb wr2, wr2, wr1\n\t"

            "wor wr2, wr2, wr13\n\t"

            "wstrw wr2, [%3], #4\n\t"

            "bne 1b\n\t"
            "b 4f\n\t"

            "2:\n\t"
            "subs r2, r2, #1\n\t"
            "ldrne r1, [%1], #4\n\t"
            "add %2, %2, #4\n\t"
            "add %3, %3, #4\n\t"
            "bne 1b\n\t"
            "b 4f\n\t"

            "3:\n\t"
            "subs r2, r2, #1\n\t"
            "ldrne r5, [%1], #4\n\t"
            "add %2, %2, #4\n\t"
            "str r1, [%3], #4\n\t"
            "mov r1, r5\n\t"
            "bne 1b\n\t"

            "4:\n\t"

            : /* output */ "=r"(out), "=r"(src), "=r"(dest), "=r"(output)
            : /* input */ "2"(dest), "1"(src), "r"(width), "3"(output)
            : /* clobber */ "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9"
    );
//    printf("%x\n", out);
}


