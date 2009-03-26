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

#ifndef ROUTINES_H
#define ROUTINES_H

#ifdef __cplusplus
extern "C" {
#endif

    struct BlurRoutines
    {
        void (*blur32)(unsigned int *data, int width, int height, 
                       int step_width, int alpha);
        void (*blur16)(unsigned short *data, int width, int height, 
                       int step_width, int alpha);
    };

    extern struct BlurRoutines q_blurroutines;

    struct BlendRoutines
    {
        // 16 + 16 -> 16
        void (*blend_rgba16_rgb16)(unsigned short *dest, 
                                   unsigned short *src, 
                                   unsigned char *alpha, 
                                   unsigned char opacity,
                                   int width,
                                   unsigned short *output);

        // 32 + 16 -> 16
        void (*blend_argb32p_rgb16)(unsigned short *dest,
                                    unsigned int *src,
                                    unsigned char opacity,
                                    int width,
                                    unsigned short *output);

        // 32 + 32 -> 32
        void (*blend_argb32p_rgb32)(unsigned int *dest,
                                    unsigned int *src,
                                    unsigned char opacity,
                                    int width,
                                    unsigned int *output);

        // 32 + 32 -> 32
        void (*blend_color_rgb32)(unsigned int *dest,
                                  unsigned int src,
                                  int width,
                                  unsigned int *output);

        // 32 + 16 -> 16
        void (*blend_color_rgb16)(unsigned short *dest,
                                  unsigned int src,
                                  int width,
                                  unsigned short *output);
    };

    extern struct BlendRoutines q_blendroutines;

    struct ColorRoutines
    {
        // 16 -> 32
        void (*color_rgba16_argb32p)(unsigned short *src,
                                    unsigned char *src_alpha,
                                    int width,
                                    unsigned int *output);

        // 32 -> 16
        void (*color_argb32_rgba16)(unsigned int *src,
                                    int width,
                                    unsigned short *output,
                                    unsigned char *output_alpha);

        // 32 -> 32p
        void (*color_argb32_argb32p)(unsigned int *src,
                                     int width,
                                     unsigned int *output);

        // 16 -> 32
        void (*color_rgb16_rgb32)(unsigned short *src, 
                                  int width, 
                                  unsigned int *output);

        // 32 -> 16
        void (*color_rgb32_rgb16)(unsigned int *src,
                                  int width,
                                  unsigned short *output);
    };

    extern struct ColorRoutines q_colorroutines;

    struct GrayscaleRoutines
    {
        // 16
        void (*grayscale_rgb16)(unsigned short *src,
                                unsigned char opacity,
                                int width,
                                unsigned short *output);
        // 32
        void (*grayscale_argb32p)(unsigned short *src,
                                  unsigned char opacity,
                                  int width,
                                  unsigned short *output);
    };
    
    extern struct GrayscaleRoutines q_grayscaleroutines;

    struct MemoryRoutines
    {
        void (*memcpy)(unsigned char *dest, unsigned char *src, int len);
        void (*memmove)(unsigned char *dest, unsigned char *src, int len);
        void (*memset_8)(unsigned char *dest, char c, int len);
        void (*memset_16)(unsigned short *dest, short c, int len);
        void (*memset_32)(unsigned int *dest, int c, int len);
    };

    extern struct MemoryRoutines q_memoryroutines;

    struct ScaleRoutines
    {
        void (*scale_argb32p)(unsigned int *dest, unsigned int destwidth, 
                              const unsigned int *src, unsigned int srcwidth,
                              unsigned int adjust);
        void (*scale_argb32p_accum)(unsigned int *dest, unsigned int destwidth, 
                                    const unsigned int *src, 
                                    unsigned int srcwidth,
                                    unsigned int adjust);
    };

    extern struct ScaleRoutines q_scaleroutines;

    struct PluginRoutines
    {
        struct BlurRoutines *blur;
        struct BlendRoutines *blend;
        struct GrayscaleRoutines *gray;
        struct MemoryRoutines *memory;
        struct ScaleRoutines *scale;
    };
#ifdef __cplusplus
};
#endif

#endif
