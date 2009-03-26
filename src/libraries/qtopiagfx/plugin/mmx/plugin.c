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

void mmx_blur(unsigned int *img, 
              int width, 
              int height, 
              int step_width, 
              int alpha32);
void mmx_blur16(unsigned short *img, 
                int width, 
                int height, 
                int step_width, 
                int alpha32);
void mmx_blend_argb32p_rgb16(unsigned short *dest,
                             unsigned int *src,
                             unsigned char opacity,
                             int width,
                             unsigned short *output);
void mmx_blend_rgba16_rgb16(unsigned short *dest, 
                            unsigned short *src, 
                            unsigned char *alpha,
                            unsigned char opacity,
                            int width, 
                            unsigned short *output);
void mmx_grayscale_rgb16(unsigned short *src, 
                         unsigned char opacity,
                         int width, 
                         unsigned short *output);
void mmx_memcpy(unsigned char *dest, 
                unsigned char *src, 
                int len);
void mmx_scale_argb32p_accum(unsigned int *out, 
                             unsigned int outwidth, 
                             const unsigned int *in, 
                             unsigned int inwidth, 
                             unsigned int accum);
void  mmx_scale_argb32p(unsigned int *out, 
                        unsigned int outwidth, 
                        const unsigned int *in, 
                        unsigned int inwidth,
                        unsigned int accum);
void mmx_blend_color_rgb16(unsigned short *dest,
                           unsigned int src,
                           int width,
                           unsigned short *output);
void mmx_memset_16(unsigned short *dest, short c, int len);
void mmx_memset_32(unsigned int *dest, int c, int len);

void gfx_init(struct PluginRoutines *p)
{
    printf("Initializing mmx plugin\n");

    // Blur
    p->blur->blur32 = mmx_blur;
    p->blur->blur16 = mmx_blur16;

    // Blend
    p->blend->blend_argb32p_rgb16 = mmx_blend_argb32p_rgb16;
    p->blend->blend_rgba16_rgb16 = mmx_blend_rgba16_rgb16;
    p->blend->blend_color_rgb16 = mmx_blend_color_rgb16;

    // Grayscale
    p->gray->grayscale_rgb16 = mmx_grayscale_rgb16;

    // Memory
    p->memory->memcpy = mmx_memcpy;
    p->memory->memset_16 = mmx_memset_16;
    p->memory->memset_32 = mmx_memset_32;

    // Scale
    p->scale->scale_argb32p = mmx_scale_argb32p;
    p->scale->scale_argb32p_accum = mmx_scale_argb32p_accum;

}
