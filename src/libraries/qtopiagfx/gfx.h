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

#ifndef GFX_H
#define GFX_H

#include <qglobal.h>

//do this here rather than in qtopiaglobal.h so we don't have a dependency on qtopiabase
#if defined(QT_VISIBILITY_AVAILABLE)
#   define QTOPIAGFX_EXPORT __attribute__((visibility("default")))
#else
#   define QTOPIAGFX_EXPORT
#endif

class QImage;
class GfxImageRef;

namespace Gfx
{
    QTOPIAGFX_EXPORT void init(const char *arch = 0);
    QTOPIAGFX_EXPORT void blur(GfxImageRef &img, qreal radius);
    QTOPIAGFX_EXPORT void blur(QImage &img, qreal radius);
};

#endif
