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

#ifndef DEF_BLEND_H
#define DEF_BLEND_H

void def_blend_rgba16_rgb16(unsigned short *dest,
                            unsigned short *src,
                            unsigned char *alpha,
                            unsigned char opacity,
                            int width,
                            unsigned short *output);

void def_blend_argb32p_rgb16(unsigned short *dest,
                            unsigned int *src,
                            unsigned char opacity,
                            int width,
                            unsigned short *output);

void def_blend_argb32p_rgb32(unsigned int *dest,
                            unsigned int *src,
                            unsigned char opacity,
                            int width,
                            unsigned int *output);

void def_blend_color_rgb32(unsigned int *dest,
                           unsigned int src,
                           int width,
                           unsigned int *output);

void def_blend_color_rgb16(unsigned short *dest,
                           unsigned int src,
                           int width,
                           unsigned short *output);

#endif
