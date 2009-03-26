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

static inline unsigned int premul_nozero(unsigned int val, unsigned char alpha)
{
    return ((((val & 0x00FF00FF) * alpha) >> 8) & 0x00FF00FF) |
        ((((val & 0xFF00FF00) >> 8) * alpha) & 0xFF00FF00);
}

static inline unsigned short qConvertRgb32To16(unsigned int c)
{
    return (((c) >> 3) & 0x001f) |
           (((c) >> 5) & 0x07e0) |
           (((c) >> 8) & 0xf800);
}

static inline unsigned int qConvertRgb16To32(unsigned short c)
{
    return ((c << 3) & 0x00F8) |
           ((c << 5) & 0xFC00) |
           ((c << 8) & 0xF80000) |
           0xFF070307;
}

static inline void argb32p_rgb16(unsigned short *dest, 
                                 unsigned int src)
{
    unsigned char alpha = src >> 24;
    alpha = 0xff - alpha;
    *dest = qConvertRgb32To16((premul_nozero(qConvertRgb16To32(*dest), alpha) & 0xFFFFFF) + src);
}

void mmx_blend_color_rgb16(unsigned short *dest,
                           unsigned int src,
                           int width,
                           unsigned short *output)
{
    if(dest != output) {
        printf("mmx: Non-inplace color + rgb16 blend not implemented\n");
        return;
    }

    unsigned int alpha = src >> 24;
    alpha = 0xFF - alpha;

    while((int)dest & 0x7 && width) {
        argb32p_rgb16(dest++, src);
        width--;
    }

    int w4 = width >> 2;
    width &= 0x3;
    if(w4) {
        unsigned int c_red = (src & 0xFF0000) >> 8;
        c_red |= c_red << 16;
        c_red += 0x00070007 * alpha;

        unsigned int c_green = (src & 0xFF00);
        c_green |= c_green << 16;
        c_green >>= 5;
        c_green += (0x00180018 * alpha) >> 8;

        unsigned int c_blue = (src & 0xFF) << 5;
        c_blue += (0x0007 * alpha) >> 3;
        c_blue |= c_blue << 16;

        asm volatile ( 
                // Setup
                "tbcsth wr7, %7\n\t" // wr7 = alpha 00
                "tbcsth wr8, %8\n\t" // wr8 = 00 alpha

                "ldr r6, =8\n\t"
                "tmcr wCGR0, r6\n\t" // wCGR0 = 8

                "ldr r6, =0xF800F800\n\t"
                "tmcrr wr10, r6, r6\n\t" // wr10 = f800 f800 f800 f800
                "ldr r6, =0x07E007E0\n\t"
                "tmcrr wr11, r6, r6\n\t" // wr11 = 07e0 07e0 07e0 07e0
                "ldr r6, =0x001F001F\n\t"
                "tmcrr wr12, r6, r6\n\t" // wr12 = 001f 001f 001f 001f

                "tmcrr wr13, %2, %2\n\t" // wr13 = c_red
                "tmcrr wr14, %3, %3\n\t" // wr14 = c_green
                "tmcrr wr15, %4, %4\n\t" // wr15 = c_blue

                "1:\n\t"

                "wldrd wr0, [%0]\n\t"

                "pld [%0, #32]\n\t"
                "subs %1, %1, #1\n\t"

                "wand wr1, wr0, wr10\n\t" // r
                "wmulum wr1, wr1, wr7\n\t" // r
                "wand wr2, wr0, wr11\n\t" // g
                "waddhus wr1, wr1, wr13\n\t" // r
                "wand wr1, wr1, wr10\n\t" // r

                "wmulum wr2, wr2, wr7\n\t" // g
                "wand wr3, wr0, wr12\n\t" // b
                "waddhus wr2, wr2, wr14\n\t" // g
                "wmulul wr3, wr3, wr8\n\t" // b
                "wand wr2, wr2, wr11\n\t" // g

                "waddhus wr3, wr3, wr15\n\t" // b
                "wsrlhg wr3, wr3, wCGR0\n\t" // b

                // Recombine
                "wor wr3, wr3, wr1\n\t"
                "wor wr3, wr3, wr2\n\t"

                "wstrd wr3, [%0], #8\n\t"

                "bne 1b\n\t"

                : /* out */ "=r"(dest), "=r"(w4)
                : /* in */ "r"(c_red), "r"(c_green), "r"(c_blue), "0"(dest), 
                           "1"(w4), "r"(alpha << 8), "r"(alpha)
                : /* clobber */ "r6", "wr0", "wr1", "wr2", "wr3", "wr7", "wr8", "wr10", "wr11", "wr12", "wr13", "wr14", "wr15"
                );
    }

    while(width--) 
        argb32p_rgb16(dest++, src);

}

