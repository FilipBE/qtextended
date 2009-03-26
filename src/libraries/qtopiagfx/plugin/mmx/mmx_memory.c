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

static void mmx_memcpy_align(unsigned char *dest, unsigned char *src, int len)
{
    int mmx_len = len >> 5;
    if(mmx_len) {
        asm volatile(
                "mov r1, %2\n\t"

                "mov r2, %0\n\t"
                "mov r3, %1\n\t"

                "1:\n\t"

                "pld [r3, #32]\n\t"
                "pld [r2, #32]\n\t"

                "ldr r4, [r3], #4\n\t"
                "ldr r5, [r3], #4\n\t"
                "ldr r6, [r3], #4\n\t"
                "ldr r7, [r3], #4\n\t"
                "str r4, [r2], #4\n\t"
                "str r5, [r2], #4\n\t"
                "str r6, [r2], #4\n\t"
                "str r7, [r2], #4\n\t"

                "ldr r4, [r3], #4\n\t"
                "ldr r5, [r3], #4\n\t"
                "ldr r6, [r3], #4\n\t"
                "ldr r7, [r3], #4\n\t"
                "str r4, [r2], #4\n\t"
                "str r5, [r2], #4\n\t"
                "str r6, [r2], #4\n\t"
                "str r7, [r2], #4\n\t"

                "subs r1, r1, #1\n\t"
                "bne 1b\n\t"
                : /* out */
                : /* in */"r"(dest), "r"(src), "r"(mmx_len)
                : /* clobber */ "r1", "r2", "r3", "r4", "r5", "r6", "r7"
        );

        dest += mmx_len << 5;
        src += mmx_len << 5;
    }

    len = len & 0x1F;
    if(len >= 4) {
        unsigned int *dest32 = (unsigned int *)dest;
        unsigned int *src32 = (unsigned int *)src;
        switch((len >> 2)) {
            case 7: *dest32++ = *src32++;
            case 6: *dest32++ = *src32++;
            case 5: *dest32++ = *src32++;
            case 4: *dest32++ = *src32++;
            case 3: *dest32++ = *src32++;
            case 2: *dest32++ = *src32++;
            case 1: *dest32++ = *src32++;
        }
        dest = (unsigned char *)dest32;
        src = (unsigned char *)src32;
    }

    len &= 0x03;
    if(len) {
        switch(len) {
            case 3: *dest++ = *src++;
            case 2: *dest++ = *src++;
            case 1: *dest++ = *src++;
        }
    }
}

static void mmx_memcpy_dest_align(unsigned char *dest, unsigned char *src, 
                                  int len)
{
    unsigned int alignment = ((unsigned int)src) & 0x3;
    unsigned int *src32 = (unsigned int *)(((unsigned int)src) & ~0x3);
    unsigned int *dest32 = (unsigned int *)dest;

    int mmx_len = len >> 5;
    int align_in = *src32;

    if(mmx_len) {
        asm volatile(
                "mov r1, %3\n\t"

                "mov r2, %1\n\t"
                "mov r3, %2\n\t"

                "mov r8, %5, lsl #3\n\t"

                "mov r9, #32\n\t"
                "sub r9, r9, r8\n\t"

                "mov %0, %4\n\t"

                "1:\n\t"

                "pld [r3, #32]\n\t"
                "pld [r2, #32]\n\t"
                "ldr r4, [r3], #4\n\t"
                "ldr r5, [r3], #4\n\t"
                "ldr r6, [r3], #4\n\t"
                "ldr r7, [r3], #4\n\t"

                "mov %0, %0, lsr r8\n\t"
                "orr %0, %0, r4, lsl r9\n\t"

                "mov r4, r4, lsr r8\n\t"
                "orr r4, r4, r5, lsl r9\n\t"

                "mov r5, r5, lsr r8\n\t"
                "orr r5, r5, r6, lsl r9\n\t"
                
                "mov r6, r6, lsr r8\n\t"
                "orr r6, r6, r7, lsl r9\n\t"

                "str %0, [r2], #4\n\t"
                "str r4, [r2], #4\n\t"
                "str r5, [r2], #4\n\t"
                "str r6, [r2], #4\n\t"


                "ldr r4, [r3], #4\n\t"
                "ldr r5, [r3], #4\n\t"
                "ldr r6, [r3], #4\n\t"
                "ldr %0, [r3], #4\n\t"

                "mov r7, r7, lsr r8\n\t"
                "orr r7, r7, r4, lsl r9\n\t"

                "mov r4, r4, lsr r8\n\t"
                "orr r4, r4, r5, lsl r9\n\t"

                "mov r5, r5, lsr r8\n\t"
                "orr r5, r5, r6, lsl r9\n\t"
                
                "mov r6, r6, lsr r8\n\t"
                "orr r6, r6, %0, lsl r9\n\t"

                "str r7, [r2], #4\n\t"
                "str r4, [r2], #4\n\t"
                "str r5, [r2], #4\n\t"
                "str r6, [r2], #4\n\t"

                "subs r1, r1, #1\n\t"
                "bne 1b\n\t"

                : /* out */"=r"(align_in)
                : /* in */"r"(dest), "r"(src32 + 1), "r"(mmx_len), "0"(align_in), 
                          "r"(alignment)
                : /* clobber */"r1", "r2", "r3", "r4", "r5", "r6", "r7", 
                               "r8", "r9"
        );

        dest += mmx_len << 5;
        src += mmx_len << 5;
    }

    len = len & 0x1F;
    // XXX - a little slow 
    // underperforms memcpy for anything less than 32 :(
    while(len--) 
        *dest++ = *src++;
}

// XXX - This routine is not mmx specific and should be in a more generic ARM
// plugin
void mmx_memcpy(unsigned char *dest, unsigned char *src, int len)
{
    while(((unsigned int)dest) & 0x3 && len--) 
        *dest++ = *src++;

    if(len <= 0)
        return;

    if((unsigned int)src & 0x3)
        mmx_memcpy_dest_align(dest, src, len);
    else
        mmx_memcpy_align(dest, src, len);
}

void mmx_memset_16(unsigned short *dest, short c, int len)
{
    if(((unsigned long long)dest) & 0x2) {
        *dest++ = c;
        --len;
    }
    
    int _len = len >> 1;
    if(_len) {
        unsigned int _val = (unsigned short)(c) | ((unsigned short)(c) << 16);
        unsigned int *_dest = (unsigned int *)dest;
        int _len8 = _len >> 1;
        if(_len8) {
            asm volatile (
                    "tbcstw wr0, %1\n\t"
                    "mov r6, %3\n\t"

                    "1:\n\t"
                    "wstrd wr0, [%0], #8\n\t"
                    "pld [%0, #32]\n\t"
                    "subs r6, r6, #1\n\t"
                    "bne 1b\n\t"
                    : /* out */ "=r"(_dest)
                    : /* in */ "r"(_val), "0"(_dest), "r"(_len8)
                    : /* clobber */ "r6", "wr0");
        }
        if(_len & 0x1)
            *_dest = _val;
    }

    if(len & 0x1)
        dest[len - 1] = c;
}

void mmx_memset_32(unsigned int *dest, int c, int len)
{
    int len8 = len >> 1;
    if(len8) {
        asm volatile (
                "tbcstw wr0, %1\n\t"
                "mov r6, %3\n\t"

                "1:\n\t"
                "wstrd wr0, [%0], #8\n\t"
                "pld [%0, #32]\n\t"
                "subs r6, r6, #1\n\t"
                "bne 1b\n\t"
                : /* out */ "=r"(dest)
                : /* in */ "r"(c), "0"(dest), "r"(len8)
                : /* clobber */ "r6", "wr0");
    }
    if(len & 0x1)
        *dest = c;
}
