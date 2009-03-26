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

#include "routines.h"
#include "def_blur.h"
#include "def_color.h"
#include "def_blend.h"
#include "def_memory.h"

struct BlurRoutines q_blurroutines =
{
    &def_blur32,
    &def_blur16
};

struct BlendRoutines q_blendroutines =
{
    &def_blend_rgba16_rgb16,
    &def_blend_argb32p_rgb16,
    &def_blend_argb32p_rgb32,
    &def_blend_color_rgb32,
    &def_blend_color_rgb16
};

struct ColorRoutines q_colorroutines =
{
    &def_color_rgba16_argb32,
    &def_color_argb32_rgba16,
    &def_color_argb32_argb32p,
    &def_color_rgb16_rgb32,
    &def_color_rgb32_rgb16
};

struct GrayscaleRoutines q_grayscaleroutines =
{
    0,
    0
};

struct MemoryRoutines q_memoryroutines = 
{
    &def_memcpy,
    &def_memmove,
    &def_memset_8,
    &def_memset_16,
    &def_memset_32
};

struct ScaleRoutines q_scaleroutines =
{
    0,
    0
};
