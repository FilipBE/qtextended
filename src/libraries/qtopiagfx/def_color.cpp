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

#include "def_color.h"

static inline unsigned int premul(unsigned int val, unsigned char alpha)
{
    if(alpha == 0xFF)
        return val;
    else if(alpha == 0x00)
        return 0;
    else
        return ((val & 0x00FF00FF) * (alpha + 1) >> 8) & 0x00FF00FF |
               (((val & 0xFF00FF00) >> 8) * (alpha + 1)) & 0xFF00FF00;
}

static inline unsigned int rgba16_argb32(unsigned short src,
                                         unsigned int src_alpha)
{
    if(!src_alpha) return 0;
    return premul( (src >> 8 | 0x07) << 16 |
                   (src >> 3 | 0x03) << 8 |
                   (src << 3 | 0x07), src_alpha ) | src_alpha << 24;
}

void def_color_rgba16_argb32(unsigned short *src,
                             unsigned char *src_alpha,
                             int width,
                             unsigned int *output)
{
    while(width--)
        *output++ = rgba16_argb32(*src++, *src_alpha++);
}

static inline unsigned short argb32_rgb16(unsigned int src)
{
    return ((src >> 16) & 0xF800) |
           ((src >> 5) & 0x07E0) |
           ((src >> 3) & 0x001F);
}

void def_color_argb32_rgba16(unsigned int *src, int width,
                             unsigned short *output,
                             unsigned char *output_alpha)
{
    while(width--) {
        *output++ = argb32_rgb16(*src);
        *output_alpha = *src++ >> 24;
    }
}

void def_color_argb32_argb32p(unsigned int *src,
                              int width,
                              unsigned int *output)
{
    while(width--) {
        register unsigned int src_val = *src++;
        *output++ = premul(src_val, src_val >> 24) | src_val & 0xFF000000;
    }
}

void def_color_rgb16_rgb32(unsigned short *src, int width, unsigned int *output)
{
    while(width--) {
        unsigned short _src = *src++;
        *output++ = (_src & 0xF800) << 8 |
                    (_src & 0x7E0) << 5 |
                    (_src & 0x1F) << 3 |
                    0xFF070307;
    }
}

void def_color_rgb32_rgb16(unsigned int *src, int width, unsigned short *output)
{
    while(width--) {
        unsigned int _src = *src++;
        *output++ = (_src & 0xF80000) >> 8 |
                    (_src & 0xFC00) >> 5 |
                    (_src & 0xF8) >> 3;
    }
}

