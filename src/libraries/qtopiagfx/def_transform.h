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

#ifndef DEF_TRANSFORM_H
#define DEF_TRANSFORM_H

class QImage;
class QMatrix;
class QSize;
class GfxImageRef;
void def_transform(GfxImageRef &out, const QImage &in, const QMatrix &m, unsigned char op);
void def_transform_bilinear(GfxImageRef &out, const QImage &in, const QMatrix &m, unsigned char op);

void def_transform_fill(GfxImageRef &out, const QMatrix &m, const QSize &, unsigned int color, unsigned char op);
void def_transform_fill_bilinear(GfxImageRef &out, const QMatrix &m, const QSize &, unsigned int color, unsigned char op);

#endif
