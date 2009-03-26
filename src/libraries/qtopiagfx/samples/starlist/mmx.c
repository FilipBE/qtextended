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

#define CLAMP0(val) (((val) < 0)?0:(val))


void tint(unsigned short *img, int width, int height, int stepwidth,
          unsigned char *color, int bpl, unsigned int blendcolor, 
          unsigned int blendoffset)
{
    unsigned char rc = 0;
    unsigned char gc = 0;
    unsigned char bc = 0;
    int yy;
    int ii;

#ifdef SLOW_ASM
    unsigned int color1 = blendcolor & 0x00FF00FF;
    unsigned int color2 = (blendcolor & 0xFF00FF00) >> 8;



    asm volatile(
        "wzero wr1\n\t"

        "ldr r1, =0x00070307\n\t"
        "tbcstw wr11, r1\n\t"

        : /* out */
        : /* int */
        : /* clobber */ "r1"
    );
#endif

    // blendcolor = 0RGB
    blendcolor &= 0x00FFFFFF;
    blendoffset & 0x00FFFFFF;

    for(yy = 0; yy < height; ++yy) {
        unsigned char *bits = color + yy * bpl;

        rc = rand() & 0x7;
#if 0
        gc = rc;
        bc = rc;
#endif

#ifndef SLOW_ASM
        asm volatile(
    //            "ldr r2, =0x00007777\n\t"
                "tbcstw wr10, %5\n\t"
                "wzero wr11\n\t"
                "wunpckilb wr10, wr11, wr10\n\t"
                "tbcstw wr14, %7\n\t"
                "wunpckilb wr14, wr14, wr11\n\t"

                "ldr r1, =0x00002000\n\t"
                "ldr r2, =0x40002000\n\t"
                "tmcrr wr11, r2, r1\n\t" // 0000 2000 4000 2000
                "ldr r1, =0x00000800\n\t"
                "ldr r2, =0x00200001\n\t"
                "tmcrr wr12, r2, r1\n\t" // 0000 0800 0020 0001
                "ldr r1, =0x00000007\n\t"
                "ldr r2, =0x00030007\n\t"
                "tmcrr wr13, r2, r1\n\t"  // 0000 0007 0003 0007


                // Accum
                "tbcsth wr1, %6\n\t"

                "mov r1, %4\n\t"

                // Load (pre)
                "ldrb r2, [%0], #1\n\t"

                "1:\n\t"

                "pld [%0, #32]\n\t"
                "pld [%1, #32]\n\t"

                "orrs r2, r2, r2\n\t"
                "addne r2, r2, #1\n\t"
                "tbcsth wr0, r2\n\t"

                "wmulum wr0, wr0, wr10\n\t"
                "subs r1, r1, #1\n\t"
                "ldrneb r2, [%0], #1\n\t"
                "waddbus wr0, wr0, wr14\n\t"

                // Dither
                "wsubbus wr0, wr0, wr1\n\t"

                // Store
                "wmulum wr3, wr0, wr11\n\t" // wr0 = 00 0r 0g 0b
                "wor wr2, wr0, wr13\n\t"
                "wmacuz wr3, wr12, wr3\n\t" // wr0 = rgb
                "wsubbus wr1, wr2, wr0\n\t"

                "wstrh wr3, [%1], #2\n\t"
                "bne 1b\n\t"

                : /* out */"=r"(bits), "=r"(img)
                : /* in */"0"(bits), "1"(img), "r"(width), "r"(blendcolor), "r"(rc), "r"(blendoffset)
                : /* clobber */ "r1", "r2"
                );

        img += (stepwidth - width);
#else
        for(ii = 0; ii < width; ++ii)
        {
            unsigned char color = *bits++;

            // tint
            unsigned int pixel = ((color1 * color) >> 8) & 0x00FF00FF;
            pixel |= (color2 * color) & 0xFF00FF00;
            pixel += blendoffset;

            // dither
#if 1

            asm volatile (
                    "tbcstw wr0, %0\n\t"

                    "wsubbus wr0, wr0, wr1\n\t"
                    "wor wr2, wr0, wr11\n\t"
                    "wsubbus wr1, wr2, wr0\n\t"

                    //"wstrw wr2, [%1], #4\n\t"
                    "textrmuw %2, wr2, #0\n\t"

                    : /* out */ "=r"(pixel), "=r"(img)
                    : /* int */ "0"(pixel), "1"(img)
                    );

            *img++ = (pixel & 0xF80000) >> 8 |
                     (pixel & 0xFC00) >> 5 |
                     (pixel & 0xF8) >> 3; 
#else
            /*
               tbcstw wr0, pixel
             */
            unsigned int r = (pixel >> 16) & 0xFF;
            unsigned int g = (pixel >> 8) & 0xFF;
            unsigned int b = pixel & 0xFF; 

            /*
               wsubbus wr0, wr0, wr1
             */
            r = CLAMP0((int)r - rc);
            g = CLAMP0((int)g - gc);
            b = CLAMP0((int)b - bc);

            /*
               wand wr2, wr0, wr11
               wsub wr1, wr11, wr2
             */
            rc = 0x07 - (r & 0x07);
            gc = 0x03 - (g & 0x03);
            bc = 0x07 - (b & 0x07);

            /*
               wand wr3, wr0, wr10
             */
            /*
            r &= 0xF8;
            g &= 0xFC;
            b &= 0xF8;
            */
            r |= 0x07;
            g |= 0x03;
            b |= 0x07;

            pixel = 0xFF000000 | r << 16 | g << 8 | b;

            /*
               wstrw wr3, newbits++
             */
            *img++ = (pixel & 0xF80000) >> 8 |
                     (pixel & 0xFC00) >> 5 |
                     (pixel & 0xF8) >> 3; 
#endif

        }
#endif
    }

}
