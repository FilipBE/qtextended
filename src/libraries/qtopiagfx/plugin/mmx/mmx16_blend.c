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

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned char uchar;

// argb32p + rgb16
static void mmx_blend_argb32p_rgb16_inplace_opacity(unsigned short *dest,
                                                    unsigned int *src,
                                                    unsigned char _opacity,
                                                    int width)
{
    unsigned int opacity = (_opacity + 1) << 8;

    asm volatile(
            "ldr r1, =0x00002000\n\t"
            "ldr r2, =0x40002000\n\t"
            "tmcrr wr11, r2, r1\n\t" // 0000 2000 4000 2000
            "ldr r1, =0x00000800\n\t"
            "ldr r2, =0x00200001\n\t"
            "tmcrr wr12, r2, r1\n\t" // 0000 0800 0020 0001
            "ldr r1, =0x00000001\n\t"
            "ldr r2, =0x00200800\n\t" 
            "tmcrr wr13, r2, r1\n\t" // 0000 0001 0020 0800
            "ldr r4, =0x00000FF\n\t"

            "tbcsth wr14, %5\n\t"

            "mov r1, %4\n\t"

            // Load(pre)
            "ldr r2, [%1], #4\n\t"

            "1:\n\t"

            "pld [%0, #32]\n\t"
            "pld [%1, #32]\n\t"

            "mov r3, r2, lsr #24\n\t"
            "mul r3, %5, r3\n\t"
            "movs r3, r3, lsr #16\n\t"

            "beq 2f\n\t" // handle 0 case

            "tbcstw wr0, r2\n\t" // wr0 == ARGB

            "subs r3, r4, r3\n\t"
            "wldrhne wr4, [%0]\n\t"
            "beq 5f\n\t" // handle 0xFF case

            "wshufh wr4, wr4, #0\n\t"
            "wmulul wr4, wr4, wr13\n\t" // wr4 = 00 r0 g0 b0
            "tbcsth wr5, r3\n\t" // wr5 = 0a 0a 0a 0a
            "wmulum wr4, wr4, wr5\n\t" // wr4 = 00 0r 0g 0b
            "wunpckelub wr0, wr0\n\t" // wr0 = 0A0R0G0B
            "wmulum wr0, wr0, wr14\n\t"
            // XXX data
            "waddbus wr0, wr0, wr4\n\t"

            "4:\n\t"
            "wmulum wr3, wr0, wr11\n\t" // wr0 = 00 0r 0g 0b
            "subs r1, r1, #1\n\t"
            "wmacuz wr3, wr12, wr3\n\t" // wr0 = rgb
            "ldrne r2, [%1], #4\n\t"
            "wstrh wr3, [%0], #2\n\t"

            "bne 1b\n\t"
            "b 3f\n\t"

            "5:\n\t"
            "wunpckelub wr0, wr0\n\t" // wr0 = 0A0R0G0B
            "wmulum wr3, wr0, wr11\n\t" // wr0 = 00 0r 0g 0b
            "subs r1, r1, #1\n\t"
            "wmacuz wr3, wr12, wr3\n\t" // wr0 = rgb
            "ldrne r2, [%1], #4\n\t"
            "wstrh wr3, [%0], #2\n\t"

            "bne 1b\n\t"
            "b 3f\n\t"

            "2:\n\t"
            "subs r1, r1, #1\n\t"
            "ldrne r2, [%1], #4\n\t"
            "add %0, %0, #2\n\t"
            "bne 1b\n\t"

            "3:\n\t"

            : /* out */ "=r"(dest), "=r"(src)
            : /* in */ "0"(dest), "1"(src), "r"(width), "r"(opacity)
            : /* clobber */ "r1", "r2", "r3", "r4"
            );
}

void mmx_blend_argb32p_rgb16_inplace(unsigned short *dest,
                                     unsigned int *src,
                                     int width)
{
#ifdef SIMPLIFIED_ASM
    asm volatile(
            "ldr r1, =0x00002000\n\t"
            "ldr r2, =0x40002000\n\t"
            "tmcrr wr11, r2, r1\n\t" // 0000 2000 4000 2000
            "ldr r1, =0x00000800\n\t"
            "ldr r2, =0x00200001\n\t"
            "tmcrr wr12, r2, r1\n\t" // 0000 0800 0020 0001
            "ldr r1, =0x00000001\n\t"
            "ldr r2, =0x00200800\n\t" 
            "tmcrr wr13, r2, r1\n\t" // 0000 0001 0020 0800
            "ldr r1, =0x000007FF\n\t"
            "ldr r2, =0x03FF07FF\n\t" 
            "tmcrr wr14, r2, r1\n\t" 
            "ldr r4, =0x00000FF\n\t"

            "mov r1, %4\n\t"

            "1:\n\t"

            "ldr r2, [%1], #4\n\t"

            "movs r3, r2, lsr #24\n\t"
            "beq 2f\n\t" // handle 0 case
            "subs r3, r4, r3\n\t" // correct alpha

            "and r2, r2, #0xFFFFFF\n\t" // r2 = RGB
            "tbcstw wr0, r2\n\t" // wr0 == 0RGB
            "wunpckelub wr0, wr0\n\t" // wr0 = 000R0G0B

            "beq 4f\n\t" // handle 0xFF case

            "tbcsth wr5, r3\n\t" // wr5 = 0a 0a 0a 0a
            "wldrh wr4, [%0]\n\t"
            "wshufh wr4, wr4, #0\n\t"
            "wmulul wr4, wr4, wr13\n\t" // wr4 = 00 r0 g0 b0
            "wmulum wr4, wr4, wr5\n\t" // wr4 = 00 0r 0g 0b
            "waddbus wr0, wr0, wr4\n\t"

            "4:\n\t"
            "wmulum wr3, wr0, wr11\n\t" // wr0 = 00 0r 0g 0b
            "wmacuz wr3, wr12, wr3\n\t" // wr0 = rgb
            "wstrh wr3, [%0], #2\n\t"
            
            "subs r1, r1, #1\n\t"
            "bne 1b\n\t"
            "b 3f\n\t"

            "2:\n\t"
            "add %0, %0, #2\n\t"
            "subs r1, r1, #1\n\t"
            "bne 1b\n\t"

            "3:\n\t"


            : /* out */ "=r"(dest), "=r"(src)
            : /* in */ "0"(dest), "1"(src), "r"(width)
            : /* clobber */ "r1", "r2", "r3", "r4"
            );
#else
    asm volatile(
            "ldr r1, =0x00002000\n\t"
            "ldr r2, =0x40002000\n\t"
            "tmcrr wr11, r2, r1\n\t" // 0000 2000 4000 2000
            "ldr r1, =0x00000800\n\t"
            "ldr r2, =0x00200001\n\t"
            "tmcrr wr12, r2, r1\n\t" // 0000 0800 0020 0001
            "ldr r1, =0x00000001\n\t"
            "ldr r2, =0x00200800\n\t" 
            "tmcrr wr13, r2, r1\n\t" // 0000 0001 0020 0800
            "ldr r4, =0x00000FF\n\t"

            "mov r1, %4\n\t"

            // Load(pre)
            "ldr r2, [%1], #4\n\t"

            "1:\n\t"

            "pld [%0, #32]\n\t"
            "pld [%1, #32]\n\t"

            "movs r3, r2, lsr #24\n\t"
            "beq 2f\n\t" // handle 0 case

            "tbcstw wr0, r2\n\t" // wr0 == ARGB

            "subs r3, r4, r3\n\t" // correct alpha
            "wldrhne wr4, [%0]\n\t"
            "beq 5f\n\t" // handle 0xFF case

            "wshufh wr4, wr4, #0\n\t"
            "wmulul wr4, wr4, wr13\n\t" // wr4 = 00 r0 g0 b0
            "tbcsth wr5, r3\n\t" // wr5 = 0a 0a 0a 0a
            "wmulum wr4, wr4, wr5\n\t" // wr4 = 00 0r 0g 0b
            "wunpckelub wr0, wr0\n\t" // wr0 = 0A0R0G0B
            "waddbus wr0, wr0, wr4\n\t"

            "4:\n\t"
            "wmulum wr3, wr0, wr11\n\t" // wr0 = 00 0r 0g 0b
            "subs r1, r1, #1\n\t"
            "wmacuz wr3, wr12, wr3\n\t" // wr0 = rgb
            "ldrne r2, [%1], #4\n\t"
            "wstrh wr3, [%0], #2\n\t"

            "bne 1b\n\t"
            "b 3f\n\t"

            "5:\n\t"
            "wunpckelub wr0, wr0\n\t" // wr0 = 0A0R0G0B
            "wmulum wr3, wr0, wr11\n\t" // wr0 = 00 0r 0g 0b
            "subs r1, r1, #1\n\t"
            "wmacuz wr3, wr12, wr3\n\t" // wr0 = rgb
            "ldrne r2, [%1], #4\n\t"
            "wstrh wr3, [%0], #2\n\t"

            "bne 1b\n\t"
            "b 3f\n\t"

            "2:\n\t"
            "subs r1, r1, #1\n\t"
            "ldrne r2, [%1], #4\n\t"
            "add %0, %0, #2\n\t"
            "bne 1b\n\t"

            "3:\n\t"

            : /* out */ "=r"(dest), "=r"(src)
            : /* in */ "0"(dest), "1"(src), "r"(width)
            : /* clobber */ "r1", "r2", "r3", "r4"
            );
#endif
}

void mmx_blend_argb32p_rgb16(unsigned short *dest,
                             unsigned int *src,
                             unsigned char opacity,
                             int width,
                             unsigned short *output)
{
    if(dest == output) {
        // inplace
        if(opacity != 0xFF) 
            mmx_blend_argb32p_rgb16_inplace_opacity(dest, src, opacity, width);
        else
            mmx_blend_argb32p_rgb16_inplace(dest, src, width);
    } else {
        printf("mmx: Non-inplace argb32p + rgb16 blend not implemented\n");
    }
}








/*
   wldrw alpha, [alpha], #4
   wunpacklow unpacked_alpha, alpha
   wcmph add, unpacked_alpha, 0
   wslr add, 15
   waddh unpacked_alpha, add
   wsll upacked_alpha, 7
   wsub 0x8000800080008000 - upacked_alpha

   wldrd src, [src], #8
   wldrd dest, [dest], #8

   wor src_red, src, 0x07FF 07FF 07FF 07FF
   wor dest_red, dest, 0x07FF 07FF 07FF 07FF
   wsllh src, 5
   wsllh dest, 5
   wor src_green, src, 0x03FF 03FF 03FF 03FF
   wor dest_green, dest, 0x03FF 03FF 03FF 03FF
   wsllh src, 6 // unnecessary?
   wsllh dest, 6 // unnecessary?
   wor src_blue, src, 0x07FF 07FF 07FF 07FF
   wor dest_blue, dest, 0x07FF 07FF 07FF 07FF

   wmulum src_red, alpha
   wmulum dest_red, malpha
   wmulum src_green, alpha
   wmulum dest_green, malpha
   wmulum src_blue, alpha
   wmulum dest_blue, malpha

   waddh src_red, src_red, dest_red
   waddh src_green, src_green, dest_green
   waddh src_blue, src_blue, dest_blue

   wand src_red
   wshift src_red
   wand src_green
   wshift src_green
   wand src_blue // unneeded?
   wshift src_blue

   wor src, src_red, src_green
   wor src, src_blue

   wstrd src
 */


void blend_rgba16_rgb16_hiqual(ushort *dest, 
                               ushort *src, 
                               uchar *alpha,
                               unsigned char opacity,
                               int width, 
                               ushort *output)
{
    // XXX
    opacity = opacity;
    output = output;

    width &= ~0x3;
    if(!width)
        return;

#ifdef SIMPLIFIED_ASM
    asm volatile(
            "wzero wr10\n\t" // wr10 = 0000 0000 0000 0000
            "ldr r1, =0x80008000\n\t"
            "tmcrr wr11, r1, r1\n\t" // wr11 = 8000 8000 8000 8000
            "ldr r1, =0x07FF07FF\n\t"
            "tmcrr wr12, r1, r1\n\t" // wr12 = 07FF 07FF 07FF 07FF
            "ldr r1, =0x03FF03FF\n\t"
            "tmcrr wr13, r1, r1\n\t" // wr13 = 03FF 03FF 03FF 03FF
            "ldr r1, =0x01FF01FF\n\t"
            "tmcrr wr14, r1, r1\n\t" // wr14 = 01FF 01FF 01FF 01FF
            "ldr r1, =0\n\t"
            "ldr r2, =1\n\t"
            "tmcrr wr15, r2, r1\n\t" // wr15 = 1
            "ldr r2, =4\n\t"
            "tmcrr wr9, r2, r1\n\t" // wr9 = 4
            "ldr r2, =10\n\t"
            "tmcrr wr8, r2, r1\n\t" // wr8 = 10

            "ldr r1, =15\n\t"
            "tmcr wCGR0, r1\n\t" // wCGR0 = 15
            "ldr r1, =7\n\t"
            "tmcr wCGR1, r1\n\t" // wCGR1 = 7
            "ldr r1, =5\n\t"
            "tmcr wCGR2, r1\n\t" // wCGR2 = 5
            "ldr r1, =6\n\t"
            "tmcr wCGR3, r1\n\t" // wCGR2 = 6

            "mov r1, %6\n\t"

            "1:\n\t"

            "pld [%0, #32]\n\t"
            "pld [%1, #32]\n\t"
            "pld [%2, #32]\n\t"

            "ldr r2, [%2], #4\n\t"

#if 0
            "orrs r2, r2, r2\n\t"
            "beq 2f\n\t"
#endif

            "wldrd wr2, [%0]\n\t" // wr2 = dest (dest is always aligned)
            "wldrd wr3, [%1], #8\n\t" // wr3 = src

            "tbcstw wr0, r2\n\t"
            "wunpckelub wr0, wr0\n\t"
            "wcmpeqh wr1, wr0, wr10\n\t"
            "wsrlhg wr1, wr1, wCGR0\n\t"
            "waddhus wr0, wr0, wr1\n\t" 
            "wsllhg wr0, wr0, wCGR1\n\t" // wr0 = alpha
            "wsubhus wr1, wr11, wr0\n\t" // wr1 = inv_alpha

            "wor wr4, wr2, wr12\n\t" // dest_red
            "wor wr5, wr3, wr12\n\t" // src_red
            "wmulum wr4, wr4, wr1\n\t"
            "wmulum wr5, wr5, wr0\n\t"
            "waddhus wr4, wr4, wr5\n\t"
            "wandn wr4, wr4, wr13\n\t" 
            "wsllh wr6, wr4, wr15\n\t" // wr6 = r00 r00 r00 r00

            "wsllhg wr2, wr2, wCGR2\n\t" // wr2 = dest
            "wsllhg wr3, wr3, wCGR2\n\t" // wr3 = src
            "wor wr4, wr2, wr13\n\t" 
            "wor wr5, wr3, wr13\n\t" 
            "wmulum wr4, wr4, wr1\n\t"
            "wmulum wr5, wr5, wr0\n\t"
            "waddhus wr4, wr4, wr5\n\t"
            "wandn wr4, wr4, wr14\n\t" 
            "wsrlh wr4, wr4, wr9\n\t"
            "wor wr6, wr6, wr4\n\t" // wr6 = rg0 rg0 rg0 rg0

            "wsllhg wr2, wr2, wCGR3\n\t" // wr2 = dest
            "wsllhg wr3, wr3, wCGR3\n\t" // wr3 = src
            "wor wr4, wr2, wr12\n\t" 
            "wor wr5, wr3, wr12\n\t" 
            "wmulum wr4, wr4, wr1\n\t"
            "wmulum wr5, wr5, wr0\n\t"
            "waddhus wr4, wr4, wr5\n\t"
            "wsrlh wr4, wr4, wr8\n\t"
            "wor wr6, wr6, wr4\n\t" // wr6 = rgb rgb rgb rgb

            "wstrd wr6, [%0], #8\n\t"

            "subs r1, r1, #4\n\t"
            "bne 1b\n\t" // 41 instr
            "b 3f\n\t"

            "2:\n\t"
            "add %0, %0, #8\n\t"
            "add %1, %1, #8\n\t"
            "subs r1, r1, #4\n\t"
            "bne 1b\n\t"

            "3:\n\t"
            : /* out */ "=r"(dest), "=r"(src), "=r"(alpha)
            : /* in */ "0"(dest), "1"(src), "2"(alpha), "r"(width)
            : /* clobber */ "r1", "r2", "r3", "r4"
            );
#else
    unsigned int _opacity = opacity << 8;
    _opacity |= _opacity << 16;
    asm volatile(
            "wzero wr10\n\t" // wr10 = 0000 0000 0000 0000
            "ldr r1, =0x80008000\n\t"
            "tmcrr wr11, r1, r1\n\t" // wr11 = 8000 8000 8000 8000
            "ldr r1, =0x07FF07FF\n\t"
            "tmcrr wr12, r1, r1\n\t" // wr12 = 07FF 07FF 07FF 07FF
            "ldr r1, =0x03FF03FF\n\t"
            "tmcrr wr13, r1, r1\n\t" // wr13 = 03FF 03FF 03FF 03FF
            "ldr r1, =0x01FF01FF\n\t"
            "tmcrr wr14, r1, r1\n\t" // wr14 = 01FF 01FF 01FF 01FF
            "ldr r1, =0\n\t"
            "ldr r2, =1\n\t"
            "tmcrr wr15, r2, r1\n\t" // wr15 = 1
            "ldr r2, =4\n\t"
            "tmcrr wr9, r2, r1\n\t" // wr9 = 4
            "ldr r2, =10\n\t"
            "tmcrr wr8, r2, r1\n\t" // wr8 = 10

            "tbcstw wr7, %7\n\t"

            "ldr r1, =15\n\t"
            "tmcr wCGR0, r1\n\t" // wCGR0 = 15
            "ldr r1, =7\n\t"
            "tmcr wCGR1, r1\n\t" // wCGR1 = 7
            "ldr r1, =5\n\t"
            "tmcr wCGR2, r1\n\t" // wCGR2 = 5
            "ldr r1, =6\n\t"
            "tmcr wCGR3, r1\n\t" // wCGR2 = 6

            "mov r1, %6\n\t"

            // Load (pre)
            "ldr r2, [%2], #4\n\t"

            "1:\n\t"

            "orrs r2, r2, r2\n\t"
            "beq 2f\n\t"

            "wldrd wr2, [%0]\n\t" // wr2 = dest
            "tbcstw wr0, r2\n\t"
            "wunpckelub wr0, wr0\n\t"
            "wmulum wr0, wr0, wr7\n\t" // opacity adjust
            "wcmpeqh wr1, wr0, wr10\n\t"
            "wldrd wr3, [%1], #8\n\t" // wr3 = src
            "wsrlhg wr1, wr1, wCGR0\n\t"
            "waddhus wr0, wr0, wr1\n\t" 
            "wsllhg wr0, wr0, wCGR1\n\t" // wr0 = alpha
            "wsubhus wr1, wr11, wr0\n\t" // wr1 = inv_alpha

            "wor wr4, wr2, wr12\n\t" // dest_red
            "wmulum wr4, wr4, wr1\n\t"
            "wor wr5, wr3, wr12\n\t" // src_red
            "wmulum wr5, wr5, wr0\n\t"
            "wsllhg wr2, wr2, wCGR2\n\t" // wr2 = dest
            "waddhus wr4, wr4, wr5\n\t"
            "wandn wr4, wr4, wr13\n\t" 
            "wsllh wr6, wr4, wr15\n\t" // wr6 = r00 r00 r00 r00

            "wsllhg wr3, wr3, wCGR2\n\t" // wr3 = src
            "wor wr4, wr2, wr13\n\t" 
            "wmulum wr4, wr4, wr1\n\t"
            "wor wr5, wr3, wr13\n\t" 
            "wmulum wr5, wr5, wr0\n\t"
            "wsllhg wr2, wr2, wCGR3\n\t" // wr2 = dest
            "waddhus wr4, wr4, wr5\n\t"
            "wandn wr4, wr4, wr14\n\t" 
            "wsrlh wr4, wr4, wr9\n\t"
            "wor wr6, wr6, wr4\n\t" // wr6 = rg0 rg0 rg0 rg0

            "subs r1, r1, #4\n\t"

            "wsllhg wr3, wr3, wCGR3\n\t" // wr3 = src
            "wor wr4, wr2, wr12\n\t" 
            "wmulum wr4, wr4, wr1\n\t"
            "wor wr5, wr3, wr12\n\t" 
            "wmulum wr5, wr5, wr0\n\t"
            "ldrne r2, [%2], #4\n\t"
            "waddhus wr4, wr4, wr5\n\t"
            "wsrlh wr4, wr4, wr8\n\t"
            "wor wr6, wr6, wr4\n\t" // wr6 = rgb rgb rgb rgb

            "wstrd wr6, [%0], #8\n\t"

            "bne 1b\n\t" // 41 instr
            "b 3f\n\t"

            "2:\n\t"
            "subs r1, r1, #4\n\t"
            "ldrne r2, [%2], #4\n\t"
            "add %0, %0, #8\n\t"
            "add %1, %1, #8\n\t"
            "bne 1b\n\t"

            "3:\n\t"
            : /* out */ "=r"(dest), "=r"(src), "=r"(alpha)
            : /* in */ "0"(dest), "1"(src), "2"(alpha), "r"(width), "r"(_opacity)
            : /* clobber */ "r1", "r2"
            );
#endif
}

void blend_rgba16_rgb16_loqual(ushort *dest, 
                               ushort *src, 
                               uchar *alpha,
                               int width)
{
    int a;

    asm volatile( 
            // Setup
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
            "ldr r7, =0xFFFFFFFF\n\t"

            "mov r5, %7\n\t"

            // Load
            "1:\n\t"

            "pld [%5, #32]\n\t"
            "pld [%6, #32]\n\t"
            "pld [%4, #32]\n\t"

            "ldr r6, [%6], #4\n\t"
            "cmp r6, #0\n\t"
            "beq 2f\n\t"

            "wldrd wr8, [%5], #8\n\t" // wr8 = rgb(0)rgb(1)rgb(2)rgb(3)
            "wldrd wr10, [%4], #8\n\t" // wr10 = RGB(0)RGB(1)RGB(2)RGB(3)
            "tbcstw wr9, r6\n\t" // wr9 = 0000a(0)a(1)a(2)a(3)

            "wunpckelub wr2, wr9\n\t" // wr2 = 0a(0)0a(1)0a(2)0a(3)
            "wsrlhg wr2, wr2, wCGR2\n\t"
            "waddh wr2, wr2, wr15\n\t"

            // Create paired alpha
            "wsubh wr3, wr13, wr2\n\t" // wr3 = 0A(0)0A(1)0A(2)0A(3) (inverse)

            // Interleaved alpha and pixels
            "wunpckihh wr4, wr2, wr3\n\t" // wr4 = 0A(0)0a(0)0A(1)0a(1)
            "wunpckilh wr2, wr2, wr3\n\t" // wr2 = 0A(2)0a(2)0A(3)0a(3)
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

            "wstrd wr0, [%4, #-8]\n\t" 

            // Loop
            "subs r5, r5, #4\n\t"
            "bne 1b\n\t"
            "b 4f\n\t"

            "2:\n\t"
            "add %5, %5, #8\n\t"
            "add %4, %4, #8\n\t"
            "subs r5, r5, #4\n\t"
            "bne 1b\n\t" // 34 instr

            "4:\n\t"


        : /* output */ "=r"(a), "=r"(dest), "=r"(src), "=r"(alpha)
        : /* input */ "1"(dest), "2"(src), "3"(alpha), "r"(width)

        : /* clobber */ "r5", "r6", "r7",
        "wr0", "wr1", "wr2", "wr3", "wr4", "wr5", "wr8", "wr9", "wr10", "wr12", "wr13", "wr14", "wr15"
        );
}

void blend_rgba16_rgb16_loqual_opacity(ushort *dest, 
                                       ushort *src, 
                                       uchar *alpha,
                                       int width, 
                                       unsigned char opacity)
{
    int a;

    unsigned int _opacity = opacity << 8;
    _opacity |= _opacity << 16;

    asm volatile( 
            // Setup
            "wzero wr12\n\t"
            "ldr r6, =0x00400040\n\t"
            "tmcrr wr13, r6, r6\n\t"
            "ldr r6, =0xF81FF81F\n\t"
            "tmcrr wr14, r6, r6\n\t"
            "ldr r6, =0x00010001\n\t"
            "tmcrr wr15, r6, r6\n\t"

            "ldr r6, =2\n\t"
            "tmcr wCGR2, r6\n\t"
            "ldr r6, =6\n\t"
            "tmcr wCGR1, r6\n\t"

            "tbcstw wr6, %8\n\t"

            "mov r5, %7\n\t"

            // Load
            "1:\n\t"

            "pld [%5, #32]\n\t"
            "pld [%6, #32]\n\t"
            "pld [%4, #32]\n\t"

            "ldr r6, [%6], #4\n\t"
            "cmp r6, #0\n\t"
            "beq 2f\n\t"

            "wldrd wr8, [%5], #8\n\t" // wr8 = rgb(0)rgb(1)rgb(2)rgb(3)
            "tbcstw wr9, r6\n\t" // wr9 = 0000a(0)a(1)a(2)a(3)

            "wunpckelub wr2, wr9\n\t" // wr2 = 0a(0)0a(1)0a(2)0a(3)
            "wmulum wr2, wr2, wr6\n\t"
            "wldrd wr10, [%4], #8\n\t" // wr10 = RGB(0)RGB(1)RGB(2)RGB(3)
            "wsrlhg wr2, wr2, wCGR2\n\t"
            "waddh wr2, wr2, wr15\n\t"

            // Create paired alpha
            "wsubh wr3, wr13, wr2\n\t" // wr3 = 0A(0)0A(1)0A(2)0A(3) (inverse)

            // Interleaved alpha and pixels
            "wunpckihh wr4, wr2, wr3\n\t" // wr4 = 0A(0)0a(0)0A(1)0a(1)
            "wunpckilh wr2, wr2, wr3\n\t" // wr2 = 0A(2)0a(2)0A(3)0a(3)
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

            "wstrd wr0, [%4, #-8]\n\t" 

            // Loop
            "subs r5, r5, #4\n\t"
            "bne 1b\n\t"
            "b 4f\n\t"

            "2:\n\t"
            "add %5, %5, #8\n\t"
            "add %4, %4, #8\n\t"
            "subs r5, r5, #4\n\t"
            "bne 1b\n\t" // 34 instr

            "4:\n\t"


        : /* output */ "=r"(a), "=r"(dest), "=r"(src), "=r"(alpha)
        : /* input */ "1"(dest), "2"(src), "3"(alpha), "r"(width), "r"(_opacity)

        : /* clobber */ "r5", "r6", "r7",
        "wr0", "wr1", "wr2", "wr3", "wr4", "wr5", "wr6", "wr8", "wr9", "wr10", "wr12", "wr13", "wr14", "wr15"
        );
}

void blend_rgba16_rgb16_loqual_align(ushort *dest, 
                                     ushort *src, 
                                     uchar *alpha,
                                     int width)
{
    int a;

    int src_align = (((unsigned int)src) >> 1) & 0x3;
    src = (ushort *)(((unsigned int)src) & ~0x7);
    alpha = (uchar *)(((unsigned int)alpha) & ~0x3);

    asm volatile( 
            // Setup
            "wzero wr12\n\t"
            "ldr r6, =0x00400040\n\t"
            "tmcrr wr13, r6, r6\n\t"
            "ldr r6, =0xF81FF81F\n\t"
            "tmcrr wr14, r6, r6\n\t"
            "ldr r6, =0x00010001\n\t"
            "tmcrr wr15, r6, r6\n\t"

            "ldr r6, =2\n\t"
            "tmcr wCGR2, r6\n\t"
            "ldr r6, =6\n\t"
            "tmcr wCGR1, r6\n\t"

            // dword alignment stored in wCGR0
            "mov r6, %8, lsl #1\n\t"
            "tmcr wCGR0, r6\n\t"
            // alpha alignment stored in r8 and r9
            "mov r8, %8, lsl #3\n\t"
            "mov r9, #32\n\t"
            "sub r9, r9, r8\n\t"

            "mov r5, %7\n\t"

            // r7 stores previous alpha word, wr7 stores previous dword
            "ldr r7, [%6], #4\n\t"
            "orrs r7, r7, r7\n\t"
            "wldrdne wr7, [%5], #8\n\t"

            // Load
            "1:\n\t"

            "pld [%5, #32]\n\t"
            "pld [%6, #32]\n\t"
            "pld [%4, #32]\n\t"

            "mov r6, r7, lsr r8\n\t"
            "ldr r7, [%6], #4\n\t"
            "cmp r7, #0\n\t"
            // if r6 (ie. next alpha word) is not zero, we're going to need
            // the next src dword at some point
            "wldrdne wr9, [%5], #8\n\t" // wr8 = rgb(0)rgb(1)rgb(2)rgb(3)
            
            "orrs r6, r6, r7, lsl r9\n\t"
            "beq 2f\n\t"

            "walignr0 wr8, wr7, wr9\n\t"
            "wmov wr7, wr9\n\t"

            "wldrd wr10, [%4], #8\n\t" // wr10 = RGB(0)RGB(1)RGB(2)RGB(3)
            "tbcstw wr9, r6\n\t" // wr9 = 0000a(0)a(1)a(2)a(3)

            "wunpckelub wr2, wr9\n\t" // wr2 = 0a(0)0a(1)0a(2)0a(3)
            "wsrlhg wr2, wr2, wCGR2\n\t"
            "waddh wr2, wr2, wr15\n\t"

            // Create paired alpha
            "wsubh wr3, wr13, wr2\n\t" // wr3 = 0A(0)0A(1)0A(2)0A(3) (inverse)

            // Interleaved alpha and pixels
            "wunpckihh wr4, wr2, wr3\n\t" // wr4 = 0A(0)0a(0)0A(1)0a(1)
            "wunpckilh wr2, wr2, wr3\n\t" // wr2 = 0A(2)0a(2)0A(3)0a(3)
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

            "wstrd wr0, [%4, #-8]\n\t" 

            // Loop
            "subs r5, r5, #4\n\t"
            "bne 1b\n\t"
            "b 4f\n\t"

            "2:\n\t"
            "wmov wr7, wr9\n\t" // push new dword into old dword
            "add %5, %5, #8\n\t"
            "add %4, %4, #8\n\t"
            "subs r5, r5, #4\n\t"
            "bne 1b\n\t" // 34 instr

            "4:\n\t"


        : /* output */ "=r"(a), "=r"(dest), "=r"(src), "=r"(alpha)
        : /* input */ "1"(dest), "2"(src), "3"(alpha), "r"(width),
                      "r"(src_align)

        : /* clobber */ "r5", "r6", "r7", "r8", "r9",
        "wr0", "wr1", "wr2", "wr3", "wr4", "wr5", "wr8", "wr9", "wr10", "wr12", "wr13", "wr14", "wr15"
        );
}

void blend_rgba16_rgb16_loqual_align_opacity(ushort *dest, 
                                             ushort *src, 
                                             uchar *alpha,
                                             int width,
                                             unsigned char opacity)
{
    int a;

    unsigned int _opacity = opacity << 8;
    _opacity |= _opacity << 16;

    int src_align = (((unsigned int)src) >> 1) & 0x3;
    src = (ushort *)(((unsigned int)src) & ~0x7);
    alpha = (uchar *)(((unsigned int)alpha) & ~0x3);

    asm volatile( 
            // Setup
            "wzero wr12\n\t"
            "ldr r6, =0x00400040\n\t"
            "tmcrr wr13, r6, r6\n\t"
            "ldr r6, =0xF81FF81F\n\t"
            "tmcrr wr14, r6, r6\n\t"
            "ldr r6, =0x00010001\n\t"
            "tmcrr wr15, r6, r6\n\t"

            "ldr r6, =2\n\t"
            "tmcr wCGR2, r6\n\t"
            "ldr r6, =6\n\t"
            "tmcr wCGR1, r6\n\t"

            // dword alignment stored in wCGR0
            "mov r6, %8, lsl #1\n\t"
            "tmcr wCGR0, r6\n\t"
            // alpha alignment stored in r8 and r9
            "mov r8, %8, lsl #3\n\t"
            "mov r9, #32\n\t"
            "sub r9, r9, r8\n\t"

            "tbcstw wr6, %8\n\t"

            "mov r5, %7\n\t"

            // r7 stores previous alpha word, wr7 stores previous dword
            "ldr r7, [%6], #4\n\t"
            "orrs r7, r7, r7\n\t"
            "wldrdne wr7, [%5], #8\n\t"

            // Load
            "1:\n\t"

            "pld [%5, #32]\n\t"
            "pld [%6, #32]\n\t"
            "pld [%4, #32]\n\t"

            "mov r6, r7, lsr r8\n\t"
            "ldr r7, [%6], #4\n\t"
            "cmp r7, #0\n\t"
            // if r6 (ie. next alpha word) is not zero, we're going to need
            // the next src dword at some point
            "wldrdne wr9, [%5], #8\n\t" // wr8 = rgb(0)rgb(1)rgb(2)rgb(3)
            
            "orrs r6, r6, r7, lsl r9\n\t"
            "beq 2f\n\t"

            "walignr0 wr8, wr7, wr9\n\t"
            "wmov wr7, wr9\n\t"

            "tbcstw wr9, r6\n\t" // wr9 = 0000a(0)a(1)a(2)a(3)

            "wunpckelub wr2, wr9\n\t" // wr2 = 0a(0)0a(1)0a(2)0a(3)
            "wmulum wr2, wr2, wr6\n\t"
            "wldrd wr10, [%4], #8\n\t" // wr10 = RGB(0)RGB(1)RGB(2)RGB(3)
            "wsrlhg wr2, wr2, wCGR2\n\t"
            "waddh wr2, wr2, wr15\n\t"

            // Create paired alpha
            "wsubh wr3, wr13, wr2\n\t" // wr3 = 0A(0)0A(1)0A(2)0A(3) (inverse)

            // Interleaved alpha and pixels
            "wunpckihh wr4, wr2, wr3\n\t" // wr4 = 0A(0)0a(0)0A(1)0a(1)
            "wunpckilh wr2, wr2, wr3\n\t" // wr2 = 0A(2)0a(2)0A(3)0a(3)
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

            "wstrd wr0, [%4, #-8]\n\t" 

            // Loop
            "subs r5, r5, #4\n\t"
            "bne 1b\n\t"
            "b 4f\n\t"

            "2:\n\t"
            "wmov wr7, wr9\n\t" // push new dword into old dword
            "add %5, %5, #8\n\t"
            "add %4, %4, #8\n\t"
            "subs r5, r5, #4\n\t"
            "bne 1b\n\t" // 34 instr

            "4:\n\t"


        : /* output */ "=r"(a), "=r"(dest), "=r"(src), "=r"(alpha)
        : /* input */ "1"(dest), "2"(src), "3"(alpha), "r"(width),
                      "r"(src_align), "r"(_opacity)

        : /* clobber */ "r5", "r6", "r7", "r8", "r9",
        "wr0", "wr1", "wr2", "wr3", "wr4", "wr5", "wr6", "wr7", "wr8", "wr9", "wr10", "wr12", "wr13", "wr14", "wr15"
        );
}

static inline void rgba16_rgb16_inplace(unsigned short *dest, 
                                        unsigned short *src,
                                        unsigned char alpha)
{
    if(alpha == 0x00) {
    } else if(alpha == 0xFF) {
        *dest = *src;
    }else {
        alpha = (alpha >> 2) + 1;
        unsigned char inv_alpha = 0x40 - alpha;

        *dest = (((((*dest & 0xF81F) * inv_alpha) + ((*src & 0xF81F) * alpha)) >> 6) & 0xF81F) |
                (((((*dest & 0x07E0) * inv_alpha) + ((*src & 0x07E0) * alpha)) >> 6) & 0x07E0);
    }
}

static void mmx_blend_rgba16_rgb16_noopacity(ushort *dest, 
                                             ushort *src, 
                                             uchar *alpha,
                                             int width)
{
    // Bring dest up to 64-bit alignment
    int leadin = 0x4 - (((unsigned int)dest) >> 1) & 0x3;

    switch((leadin < width)?leadin:width) {
        case 3: rgba16_rgb16_inplace(dest++, src++, *alpha++); --width;
        case 2: rgba16_rgb16_inplace(dest++, src++, *alpha++); --width;
        case 1: rgba16_rgb16_inplace(dest++, src++, *alpha++); --width;
        case 0: break;
    }

    // XXX we assume alpha and src are equally aligned
    if(width & ~0x03) {
        if((unsigned int)src & 0x7) {
            blend_rgba16_rgb16_loqual_align(dest, src, alpha, width & ~0x03);
        } else {
            blend_rgba16_rgb16_loqual(dest, src, alpha, width & ~0x03);
        }
    }

    src += (width & ~0x03);
    dest += (width & ~0x03);
    alpha += (width & ~0x03);
    width &= 0x03;

    switch(width) {
        case 3: rgba16_rgb16_inplace(dest++, src++, *alpha++); 
        case 2: rgba16_rgb16_inplace(dest++, src++, *alpha++);
        case 1: rgba16_rgb16_inplace(dest++, src++, *alpha++);
        case 0: break;
    }


}

static void mmx_blend_rgba16_rgb16_opacity(ushort *dest, 
                                           ushort *src, 
                                           uchar *alpha,
                                           int width,
                                           unsigned char opacity)
{
    // Bring dest up to 64-bit alignment
    int leadin = 0x4 - (((unsigned int)dest) >> 1) & 0x3;

    switch((leadin < width)?leadin:width) {
        case 3: rgba16_rgb16_inplace(dest++, src++, ((*alpha++) * opacity) >> 8); --width;
        case 2: rgba16_rgb16_inplace(dest++, src++, ((*alpha++) * opacity) >> 8); --width;
        case 1: rgba16_rgb16_inplace(dest++, src++, ((*alpha++) * opacity) >> 8); --width;
        case 0: break;
    }

    // XXX we assume alpha and src are equally aligned
    if(width & ~0x03) {
        if((unsigned int)src & 0x7) {
            blend_rgba16_rgb16_loqual_align_opacity(dest, src, alpha, width & ~0x03, opacity);
        } else {
            blend_rgba16_rgb16_loqual_opacity(dest, src, alpha, width & ~0x03, opacity);
        }
    }

    src += (width & ~0x03);
    dest += (width & ~0x03);
    alpha += (width & ~0x03);
    width &= 0x03;

    switch(width) {
        case 3: rgba16_rgb16_inplace(dest++, src++, ((*alpha++) * opacity) >> 8); 
        case 2: rgba16_rgb16_inplace(dest++, src++, ((*alpha++) * opacity) >> 8);
        case 1: rgba16_rgb16_inplace(dest++, src++, ((*alpha++) * opacity) >> 8);
        case 0: break;
    }
}

void mmx_blend_rgba16_rgb16(ushort *dest, 
                            ushort *src, 
                            uchar *alpha,
                            uchar opacity,
                            int width, 
                            ushort *output)
{
    if(dest != output) {
        printf("mmx: Non-inplace rgba16 + rgb16 blend not implemented\n");
        return;
    }

    if(opacity != 0xFF)
        mmx_blend_rgba16_rgb16_opacity(dest, src, alpha, width, opacity);
    else
        mmx_blend_rgba16_rgb16_noopacity(dest, src, alpha, width);

}
