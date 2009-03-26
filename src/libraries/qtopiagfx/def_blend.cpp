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

#include "def_blend.h"
#include <stdio.h>
#include "def_blendhelper.h"
#include <qglobal.h>
#include <unistd.h>

#ifdef QT_ARCH_ARMV5E
#define PLD(src) \
    asm volatile("pld [%0, #32]\n\t" \
            : : "r"(src));
#else
#define PLD(src)
#endif

static inline unsigned short rgba16_rgb16(unsigned short *dest,
                                          unsigned short *src,
                                          unsigned char alpha)
{
    if(alpha == 0x00)
        return *dest;
    else if(alpha == 0xFF)
        return *src;
    else {
        alpha = (alpha >> 2) + 1;
        unsigned char inv_alpha = 0x40 - alpha;

        return (((((*dest & 0xF81F) * inv_alpha) + ((*src & 0xF81F) * alpha)) >> 6) & 0xF81F) |
               (((((*dest & 0x07E0) * inv_alpha) + ((*src & 0x07E0) * alpha)) >> 6) & 0x07E0);
    }
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

void def_blend_rgba16_rgb16(unsigned short *dest, 
                            unsigned short *src, 
                            unsigned char *alpha, 
                            unsigned char opacity,
                            int width,
                            unsigned short *output)
{
    if(opacity == 0xFF) {
        if(dest == output) {
            while(width--)
                rgba16_rgb16_inplace(dest++, src++, *alpha++);
        } else {
            while(width--)
                *output++ = rgba16_rgb16(dest++, src++, *alpha++);
        }
    } else {
        ++opacity;
        if(dest == output) {
            while(width--)
                rgba16_rgb16_inplace(dest++, src++, 
                                     (*alpha++ * opacity) >> 8);
        } else {
            while(width--)
                *output++ = rgba16_rgb16(dest++, src++, 
                                         (*alpha++ * opacity) >> 8);
        }
    }
}

static unsigned short argb32p_rgb16(unsigned short *dest, unsigned int *src)
{
    unsigned char alpha = *src >> 24;

    if(alpha == 0x00) {
        return *dest;
    } else if(alpha == 0xFF) {
        return qConvertRgb32To16(*src);
    } else {
        alpha = 0xFF - alpha;

        return qConvertRgb32To16((premul_nozero(qConvertRgb16To32(*dest), alpha) & 0xFFFFFF) + *src);
    }
}

static inline unsigned short argb32p_rgb16(unsigned short *dest, unsigned int src, unsigned char alpha)
{
    return qConvertRgb32To16((premul_noextents(qConvertRgb16To32(*dest), alpha) & 0xFFFFFF) + src);
}

static inline unsigned short argb32p_rgb32_m(unsigned int dest, unsigned int src, unsigned char alpha)
{
    return qConvertRgb32To16((premul_noextents(dest, alpha) & 0xFFFFFF) + src);
}

void def_blend_color_rgb16(unsigned short *dest,
                           unsigned int src,
                           int width,
                           unsigned short *output)
{
    unsigned int alpha = src >> 24;
    alpha = 0xFF - alpha;

    if((intptr_t)dest & 0x2) {
        *output++ = argb32p_rgb16(dest++, src, alpha);
        width--;
    }

    unsigned int *dint = (unsigned int *)dest;
    unsigned int *oint = (unsigned int *)output;

    unsigned int c_red = (src & 0xFF0000) >> 8;
    c_red |= c_red << 16;
    c_red += 0x00070007 * alpha;

    unsigned int c_green = (src & 0xFF00);
    c_green |= c_green << 16;
    c_green += 0x00030003 * alpha;

    unsigned int c_blue = (src & 0xFF) << 8;
    c_blue |= c_blue << 16;
    c_blue += 0x00070007 * alpha;
    c_blue >>= 3;

    while(width > 1) {
        unsigned int d = *dint++;

        PLD(dint);
        // red
        uint result = ((((d & 0xF800F800)) >> 8) * alpha + c_red) & 0xF800F800;
        // green
        result |= ((((d & 0x07E007E0) >> 3) * alpha + c_green) >> 5) & 0x07E007E0;
        // blue
        result |= ((((d & 0x001F001F) * alpha) + c_blue) >> 8) & 0x001F001F;


        *oint++ = result;
        width -= 2;
    }

    if(width)
        *(unsigned short *)(oint) = argb32p_rgb16((unsigned short *)dint, src, alpha);
}

static void _def_blend_argb32p_rgb16(unsigned short *dest,
                                     unsigned int *src,
                                     int width,
                                     unsigned short *output)
{
    if(dest == output) {
        register int n = (width + 7) / 8;
        switch(width & 0x07) {
            case 0: do { argb32p_rgb16_inplace(dest++, src++);
            case 7:      argb32p_rgb16_inplace(dest++, src++);
            case 6:      argb32p_rgb16_inplace(dest++, src++);
            case 5:      argb32p_rgb16_inplace(dest++, src++);
            case 4:      argb32p_rgb16_inplace(dest++, src++);
            case 3:      argb32p_rgb16_inplace(dest++, src++);
            case 2:      argb32p_rgb16_inplace(dest++, src++);
            case 1:      argb32p_rgb16_inplace(dest++, src++);
                    } while(--n > 0);
        }
    } else {
        while(width--)  
            *output++ = argb32p_rgb16(dest++, src++);
    }
}

static unsigned short argb32p_rgb16_opacity(unsigned short *dest, unsigned int *src, unsigned char opacity)
{
    unsigned int srcval = *src;
    unsigned char alpha = srcval >> 24;

    if(alpha == 0x00) {
        return *dest;
    } else {
        srcval = premul(srcval, opacity);
        alpha = srcval >> 24;
        if(alpha == 0x00) {
            return *dest;
        } else if(alpha == 0xFF) {
            return qConvertRgb32To16(srcval);
        } else {
            alpha = 0xFF - alpha;

            return qConvertRgb32To16((premul_nozero(qConvertRgb16To32(*dest), alpha) & 0xFFFFFF) + srcval);
        }
    }
}

static void _def_blend_argb32p_rgb16(unsigned short *dest,
                                     unsigned int *src,
                                     unsigned char opacity,
                                     int width,
                                     unsigned short *output)
{
    ++opacity;
    if(dest == output)
        while(width--) 
            argb32p_rgb16_opacity_inplace(dest++, src++, opacity);
    else
        while(width--) 
            *output++ = argb32p_rgb16_opacity(dest++, src++, opacity);
}

void def_blend_argb32p_rgb16(unsigned short *dest,
                             unsigned int *src,
                             unsigned char opacity,
                             int width,
                             unsigned short *output)
{
    if(opacity == 0xFF)
        _def_blend_argb32p_rgb16(dest, src, width, output);
    else
        _def_blend_argb32p_rgb16(dest, src, opacity, width, output);
}

static inline unsigned int argb32p_rgb32_opacity(unsigned int *dest, unsigned int *src, unsigned char opacity)
{
    unsigned char alpha = *src >> 24;

    if(alpha == 0x00) {
        return *dest;
    } else {
        unsigned int srcval = premul(*src, opacity);
        alpha = srcval >> 24;
        if(alpha == 0x00) {
            return *dest;
        } else if(alpha == 0xFF) {
            return srcval;
        } else {
            alpha = 0xFF - alpha;
            return ((premul_nozero(*dest, alpha) & 0xFFFFFF) + srcval) | 0xFF000000;
        }
    }
}

static void _def_blend_argb32p_rgb32(unsigned int *dest,
                                     unsigned int *src,
                                     unsigned char opacity,
                                     int width,
                                     unsigned int *output)
{
    opacity++;
    if(dest == output) {
        while(width--)
            argb32p_rgb32_opacity_inplace(dest++, src++, opacity);
    } else {
        while(width--) 
            *output++ = argb32p_rgb32_opacity(dest++, src++, opacity);
    }
}

static inline unsigned int argb32p_rgb32(unsigned int *dest, unsigned int *src)
{
    unsigned char alpha = *src >> 24;

    if(alpha == 0x00) {
        return *dest;
    } else if(alpha == 0xFF) {
        return *src;
    } else {
        alpha = 0xFF - alpha;
        return ((premul_nozero(*dest, alpha) & 0xFFFFFF) + *src) | 0xFF000000;
    }
}

static inline unsigned int argb32p_rgb32(unsigned int *dest, unsigned int src)
{
    unsigned char alpha = src >> 24;
    alpha = 0xFF - alpha;
    return ((premul_nozero(*dest, alpha) & 0xFFFFFF) + src) | 0xFF000000;
}

static void _def_blend_argb32p_rgb32(unsigned int *dest,
                                     unsigned int *src,
                                     int width,
                                     unsigned int *output)
{
    if(output == dest) {
        register int n = (width + 7) / 8;
        switch(width & 0x07) {
            PLD(dest);
            PLD(src);
            case 0: do { argb32p_rgb32_inplace(dest++, src++);
            case 7:      argb32p_rgb32_inplace(dest++, src++);
            case 6:      argb32p_rgb32_inplace(dest++, src++);
            case 5:      argb32p_rgb32_inplace(dest++, src++);
            case 4:      argb32p_rgb32_inplace(dest++, src++);
            case 3:      argb32p_rgb32_inplace(dest++, src++);
            case 2:      argb32p_rgb32_inplace(dest++, src++);
            case 1:      argb32p_rgb32_inplace(dest++, src++);
            PLD(dest);
            PLD(src);
                    } while(--n > 0);
        }
    } else {
        while(width--)
            *output++ = argb32p_rgb32(dest++, src++);
    }
}

void def_blend_argb32p_rgb32(unsigned int *dest,
                             unsigned int *src,
                             unsigned char opacity,
                             int width,
                             unsigned int *output)
{
    if(opacity == 0xFF)
        _def_blend_argb32p_rgb32(dest, src, width, output);
    else
        _def_blend_argb32p_rgb32(dest, src, opacity, width, output);
}

void def_blend_color_rgb32(unsigned int *dest,
                           unsigned int src,
                           int width,
                           unsigned int *output)
{
    while(width--) 
        *output++ = argb32p_rgb32(dest++, src);
}

