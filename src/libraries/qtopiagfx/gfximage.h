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

#ifndef GFXIMAGE_H
#define GFXIMAGE_H

#include <QImage>
#include "gfx.h"

class QTOPIAGFX_EXPORT GfxImageRef
{
public:
    GfxImageRef();
    GfxImageRef(const QImage &);
    GfxImageRef(const QImage &, const QRect &);
    GfxImageRef(uchar *bytes, int width, int height, int step, QImage::Format);
    GfxImageRef(const GfxImageRef &);
    ~GfxImageRef();
    GfxImageRef &operator=(const GfxImageRef &);
    QImage toImage() const;
    GfxImageRef subImage(const QRect &) const;
    QRect rect() const { return QRect(0, 0, _width, _height); }
    int bytesPerLine() const { return _bpl; }
    uchar *bits() const { return _bits; }
    int height() const { return _height; }
    int width() const { return _width; }
    QSize size() const { return QSize(_width, _height); }
    QImage::Format format() const { return _format; }

    void overrideFormat(QImage::Format f) { _format = f; }
private:
    uchar *_bits;
    int _height;
    int _width;
    int _bpl;
    QImage::Format _format;
};

#endif
