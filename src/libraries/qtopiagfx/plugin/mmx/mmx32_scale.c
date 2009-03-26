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

#if 0
static inline void scale(uint in, uint *a, uint *r, uint *g, uint *b, uint scale)
{
    *a += scale * ((in & 0xFF000000) >> 24);
    *r += scale * ((in & 0xFF0000) >> 16);
    *g += scale * ((in & 0xFF00) >> 8);
    *b += scale * ((in & 0xFF) >> 0);
}
int scale_row_8(uint *out, const uint *in, uint outwidth, uint inwidth)
{
    int outpixel_to_inpixel = (inwidth * 0x100) / outwidth;
    int inpixel_unit_contrib = (outwidth * 0x100) / inwidth;
    int current_inpixel = 0;

    for(int ii = 0; ii < outwidth; ++ii) {
        uint a = 0;
        uint r = 0;
        uint g = 0;
        uint b = 0;

        int remaining = outpixel_to_inpixel;

        const uint *working_inpixel_data = in + (current_inpixel >> 8);

        if(current_inpixel & 0xFF) {
            int contrib = 0x100 - (current_inpixel) & 0xFF;

            // Scale working_inpixel_data
            scale(*working_inpixel_data, &a, &r, &g, &b, 
                  (contrib * inpixel_unit_contrib) >> 8);

            working_inpixel_data++;
            remaining -= contrib;
        }

        while(remaining) {
            int contrib = qMin(0x100, remaining);

            // Scale working_inpixel_data
            scale(*working_inpixel_data, &a, &r, &g, &b, 
                  (contrib * inpixel_unit_contrib) >> 8);

            working_inpixel_data++;
            remaining -= contrib;
        }

        uint result = (a & 0xFF00) << 16 |
                      (r & 0xFF00) << 8 |
                      (g & 0xFF00) >> 0 |
                      (b & 0xFF00) >> 8;
        *out++ = result;
        current_inpixel += outpixel_to_inpixel;
    }
}
#endif
typedef unsigned int uint;

void mmx_scale_argb32p(uint *out, uint outwidth, const uint *in, uint inwidth,
                       uint accum)
{
    int outpixel_to_inpixel = (inwidth * 0x100) / outwidth;
    int inpixel_unit_contrib = (outwidth * 0x100) / inwidth;
    inpixel_unit_contrib = (inpixel_unit_contrib * accum) >> 8;

    asm volatile(
            "mov r1, #8\n\t"
            "tmcr wCGR0, r1\n\t"

            "mov r1, #0\n\t" // current_inpixel(r1)
            "mov r2, %4\n\t" // ii
            "ldr r6, =0x100\n\t"

            "mov r4, %0\n\t"

            "1:\n\t"

            "wzero wr0\n\t" // wr0 = a r g b
            "mov r3, %5\n\t" // remaining(r3) = outpixel_to_inpixel

            "pld [r4, #32]\n\t"
            "wldrw wr4, [%1]\n\t"
            "pld [%1, #32]\n\t"

            // if(current_inpixel & 0xFF) 
            "ands r5, r1, #0xFF\n\t"  // current_inpixel & 0xFF
            "beq 2f\n\t"
            "wldrw wr2, [r4, #-4]\n\t"
            "sub r5, r6, r5\n\t" // contrib = 0x100 - current_inpixel & 0xFF
            "sub r3, r3, r5\n\t" // remaining -= contrib
            "mul r5, %6, r5\n\t" // contrib = contrib * inpixel_unit_contrib
            "mov r5, r5, lsr #8\n\t"
            "wunpckelub wr0, wr2\n\t"
            "tbcsth wr1, r5\n\t"
            "wmulul wr0, wr0, wr1\n\t"
            // }

            "2:\n\t"
            "cmp r3, #0\n\t" // while(remaining) {
            "3:\n\t"
            "beq 4f\n\t"

            "wldrw wr2, [r4], #4\n\t"
            "cmp r3, #0x100\n\t" 
            "mov r7, r3\n\t"
            "movgt r7, #0x100\n\t" // contrib(r5) = qMin(0x100, remaining)
            "mul r5, %6, r7\n\t" // contrib *= inpixel_unit_contrib

            "wunpckelub wr2, wr2\n\t"
            "mov r5, r5, lsr #8\n\t"
            "tbcsth wr1, r5\n\t"
            "wmulul wr2, wr2, wr1\n\t"
            "subs r3, r3, r7\n\t" // remaining -= contrib

            "waddhus wr0, wr0, wr2\n\t"

            "b 3b\n\t" // }

            "4:\n\t"

            "wsrlhg wr0, wr0, wCGR0\n\t"
            "wpackhus wr0, wr0, wr10\n\t"
            "add r1, r1, %5\n\t" // current_inpixel += outpixel_to_inpixel
            "waddbus wr0, wr0, wr4\n\t"
            "wstrw wr0, [%1], #4\n\t"
            "subs r2, r2, #1\n\t"
            "bne 1b\n\t"

            : /* out */"=r"(in), "=r"(out)
            : /* in */ "0"(in), "1"(out), "r"(outwidth),
                       "r"(outpixel_to_inpixel), "r"(inpixel_unit_contrib)
            : /* clobber */"r1", "r2", "r3", "r4", "r5", "r6", "r7"
            );
}

void  mmx_scale_argb32p_accum(uint *out, uint outwidth, const uint *in, uint inwidth, uint accum)
{
    int outpixel_to_inpixel = (inwidth * 0x100) / outwidth;
    int inpixel_unit_contrib = (outwidth * 0x100) / inwidth;
    inpixel_unit_contrib = (inpixel_unit_contrib * accum) >> 8;

    asm volatile(
            "mov r1, #8\n\t"
            "tmcr wCGR0, r1\n\t"

            "mov r1, #0\n\t" // current_inpixel(r1)
            "mov r2, %4\n\t" // ii
            "ldr r6, =0x100\n\t"

            "mov r4, %0\n\t"

            "1:\n\t"

            "wzero wr0\n\t" // wr0 = a r g b
            "mov r3, %5\n\t" // remaining(r3) = outpixel_to_inpixel

            "pld [r4, #32]\n\t"

            // if(current_inpixel & 0xFF) 
            "ands r5, r1, #0xFF\n\t"  // current_inpixel & 0xFF
            "beq 2f\n\t"
            "wldrw wr2, [r4, #-4]\n\t"
            "sub r5, r6, r5\n\t" // contrib = 0x100 - current_inpixel & 0xFF
            "sub r3, r3, r5\n\t" // remaining -= contrib
            "mul r5, %6, r5\n\t" // contrib = contrib * inpixel_unit_contrib
            "mov r5, r5, lsr #8\n\t"
            "wunpckelub wr0, wr2\n\t"
            "tbcsth wr1, r5\n\t"
            "wmulul wr0, wr0, wr1\n\t"
            // }

            "2:\n\t"
            "cmp r3, #0\n\t" // while(remaining) {
            "3:\n\t"
            "beq 4f\n\t"

            "wldrw wr2, [r4], #4\n\t"
            "cmp r3, #0x100\n\t" 
            "mov r7, r3\n\t"
            "movgt r7, #0x100\n\t" // contrib(r5) = qMin(0x100, remaining)
            "mul r5, %6, r7\n\t" // contrib *= inpixel_unit_contrib

            "wunpckelub wr2, wr2\n\t"
            "mov r5, r5, lsr #8\n\t"
            "tbcsth wr1, r5\n\t"
            "wmulul wr2, wr2, wr1\n\t"
            "subs r3, r3, r7\n\t" // remaining -= contrib

            "waddhus wr0, wr0, wr2\n\t"

            "b 3b\n\t" // }

            "4:\n\t"

            "wsrlhg wr0, wr0, wCGR0\n\t"
            "wpackhus wr0, wr0, wr10\n\t"
            "add r1, r1, %5\n\t" // current_inpixel += outpixel_to_inpixel
            "wstrw wr0, [%1], #4\n\t"
            "subs r2, r2, #1\n\t"
            "bne 1b\n\t"

            : /* out */"=r"(in), "=r"(out)
            : /* in */ "0"(in), "1"(out), "r"(outwidth),
                       "r"(outpixel_to_inpixel), "r"(inpixel_unit_contrib)
            : /* clobber */"r1", "r2", "r3", "r4", "r5", "r6", "r7"
            );
}
