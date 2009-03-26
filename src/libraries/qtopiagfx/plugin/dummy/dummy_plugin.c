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

#include <routines.h>
#include <stdio.h>

void dummy_blur32(unsigned int *data, int width, int height, 
                       int step_width, int alpha)
{
}

void dummy_blur16(unsigned short *data, int width, int height, 
                       int step_width, int alpha)
{
}

void dummy_blend_rgba16_rgb16(unsigned short *dest, 
                                   unsigned short *src, 
                                   unsigned char *alpha, 
                                   unsigned char opacity,
                                   int width,
                                   unsigned short *output)
{
}

void dummy_blend_argb32p_rgb16(unsigned short *dest,
                                    unsigned int *src,
                                    unsigned char opacity,
                                    int width,
                                    unsigned short *output)
{
}

void dummy_blend_argb32p_rgb32(unsigned int *dest,
                                    unsigned int *src,
                                    unsigned char opacity,
                                    int width,
                                    unsigned int *output)
{
}

void dummy_color_rgba16_argb32p(unsigned short *src,
                                    unsigned char *src_alpha,
                                    int width,
                                    unsigned int *output)
{
}

void dummy_color_argb32_rgba16(unsigned int *src,
                                    int width,
                                    unsigned short *output,
                                    unsigned char *output_alpha)
{
}

void dummy_color_argb32_argb32p(unsigned int *src,
                                     int width,
                                     unsigned int *output)
{
}

void dummy_grayscale_rgb16(unsigned short *src,
                                unsigned char opacity,
                                int width,
                                unsigned short *output)
{
}

void dummy_grayscale_argb32p(unsigned short *src,
                                  unsigned char opacity,
                                  int width,
                                  unsigned short *output)
{
}

void dummy_memcpy(unsigned char *dest, unsigned char *src, int len)
{
}

void dummy_memmove(unsigned char *dest, unsigned char *src, int len)
{
}

void dummy_memset_8(unsigned char *dest, char c, int len)
{
}

void dummy_memset_16(unsigned short *dest, short c, int len)
{
}

void dummy_memset_32(unsigned int *dest, int c, int len)
{
}

void gfx_init(struct PluginRoutines *p)
{
    printf("Initializing dummy plugin\n");

    p->blur->blur32 = dummy_blur32;
    p->blur->blur16 = dummy_blur16;
    p->blend->blend_rgba16_rgb16 = dummy_blend_rgba16_rgb16;
    p->blend->blend_argb32p_rgb16 = dummy_blend_argb32p_rgb16;
    p->blend->blend_argb32p_rgb32 = dummy_blend_argb32p_rgb32;
    p->gray->grayscale_rgb16 = dummy_grayscale_rgb16;
    p->gray->grayscale_argb32p = dummy_grayscale_argb32p;
    p->memory->memcpy = dummy_memcpy;
    p->memory->memmove = dummy_memmove;
    p->memory->memset_8 = dummy_memset_8;
    p->memory->memset_16 = dummy_memset_16;
    p->memory->memset_32 = dummy_memset_32;
}
