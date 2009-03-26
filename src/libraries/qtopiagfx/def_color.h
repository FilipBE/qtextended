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

#ifndef DEF_COLOR_H
#define DEF_COLOR_H

void def_color_rgba16_argb32(unsigned short *src, unsigned char *src_alpha, int width, unsigned int *output);
void def_color_argb32_rgba16(unsigned int *src, int width, unsigned short *output, unsigned char *output_alpha);
void def_color_argb32_argb32p(unsigned int *src, int width, unsigned int *output);
void def_color_rgb16_rgb32(unsigned short *src, int width, unsigned int *output);
void def_color_rgb32_rgb16(unsigned int *src, int width, unsigned short *output);

#endif
