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

#include "def_memory.h"
#include <string.h>
#include <unistd.h>

void def_memcpy(unsigned char *dest, unsigned char *src, int len)
{
    ::memcpy(dest, src, len);
}

void def_memmove(unsigned char *dest, unsigned char *src, int len)
{
    ::memmove(dest, src, len);
}

void def_memset_8(unsigned char *dest, char c, int len)
{
    ::memset(dest, c, len);
}

#ifdef QT_ARCH_ARMV5E
#define PLD(src) \
    asm volatile("pld [%0, #32]\n\t" \
            : : "r"(src));
#else
#define PLD(src) 
#endif

#define MEMSET_SPAN(dest, val, length) \
    do { \
        register int n = ((length) + 7) / 8; \
        PLD(dest); \
        switch((length) & 0x07) { \
            case 0: do { *(dest)++ = (val); \
            case 7:      *(dest)++ = (val); \
            case 6:      *(dest)++ = (val); \
            case 5:      *(dest)++ = (val); \
            case 4:      *(dest)++ = (val); \
            case 3:      *(dest)++ = (val); \
            case 2:      *(dest)++ = (val); \
            case 1:      *(dest)++ = (val); \
                         PLD(dest); \
                       } while(--n > 0); \
        } \
   } while(0)


void def_memset_16(unsigned short *dest, short c, int len)
{
    if(((intptr_t)dest) & 0x2) {
        *dest++ = c;
        --len;
    }
    
    int _len = len >> 1;
    if(_len) {
        unsigned int _val = (unsigned short)(c) | ((unsigned short)(c) << 16);
        unsigned int *_dest = (unsigned int *)dest;
        MEMSET_SPAN(_dest, _val, _len);
    }

    if(len & 0x1)
        dest[len - 1] = c;
}

void def_memset_32(unsigned int *dest, int c, int len)
{
    MEMSET_SPAN(dest, c, len);
}

