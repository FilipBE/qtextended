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

#include "gfximage.h"
#include <QDebug>

GfxImageRef::GfxImageRef(const QImage &img, const QRect &r)
    : _bits(0), _height(0), _width(0), _bpl(0), _format(QImage::Format_Invalid)
{
    int bpp = 0;
    switch(img.format()) {
        case QImage::Format_ARGB32:
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32_Premultiplied:
            bpp = 4;
            break;
        case QImage::Format_RGB16:
            bpp = 2;
            break;
        default:
            qWarning() << "GfxImageRef: Unknown image type" << img.format();
            return;
            break;
    }

    QRect rect = r & img.rect();
    _height = rect.height();
    _width = rect.width();
    _bpl = img.bytesPerLine();
    _format = img.format();
    _bits = ((uchar *)img.bits()) + _bpl * rect.top() + bpp * rect.left();
}

GfxImageRef GfxImageRef::subImage(const QRect &r) const
{
    GfxImageRef rv;
    int bpp = 0;
    
    switch(_format) {
        case QImage::Format_Invalid:
            return rv;
            break;
        case QImage::Format_ARGB32:
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32_Premultiplied:
            bpp = 4;
            break;
        case QImage::Format_RGB16:
            bpp = 2;
            break;
        default:
            qWarning() << "GfxImageRef: Unknown image type" << _format;
            return rv;
            break;
    }

    QRect rect = r & this->rect();
    if(rect.isEmpty())
        return rv;
    rv._height = rect.height();
    rv._width = rect.width();
    rv._bpl = _bpl;
    rv._format = _format;
    rv._bits = bits() + _bpl * rect.top() + bpp * rect.left();
    return rv;
}


GfxImageRef::GfxImageRef(const QImage &img)
    : _bits(0), _height(0), _width(0), _bpl(0), _format(QImage::Format_Invalid)
{
    int bpp = 0;
    switch(img.format()) {
        case QImage::Format_ARGB32:
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32_Premultiplied:
            bpp = 4;
            break;
        case QImage::Format_RGB16:
            bpp = 2;
            break;
        default:
            qWarning() << "GfxImageRef: Unknown image type" << img.format();
            return;
            break;
    }

    QRect rect = img.rect();
    _height = rect.height();
    _width = rect.width();
    _format = img.format();
    _bpl = img.bytesPerLine();
    _bits = ((uchar *)img.bits()) + _bpl * rect.top() + bpp * rect.left();
}

GfxImageRef::GfxImageRef()
: _bits(0), _height(0), _width(0), _bpl(0), _format(QImage::Format_Invalid)
{
}

GfxImageRef::GfxImageRef(uchar *bytes, int width, int height, int step, QImage::Format format)
: _bits(bytes), _height(height), _width(width), _bpl(step), _format(format)
{
}

GfxImageRef::GfxImageRef(const GfxImageRef &other)
: _bits(other._bits), _height(other._height), _width(other._width), 
  _bpl(other._bpl), _format(other._format)
{
}

GfxImageRef::~GfxImageRef()
{
}

GfxImageRef &GfxImageRef::operator=(const GfxImageRef &other)
{
    _bits = other._bits;
    _height = other._height;
    _width = other._width;
    _bpl = other._bpl;
    _format = other._format;
    return *this;
}

QImage GfxImageRef::toImage() const
{  
#ifdef Q_WS_X11
    // XXX - not sure what the deal is here...
    return QImage(_bits, _width, _height, _bpl, _format).copy();
#else
    return QImage(_bits, _width, _height, _bpl, _format);
#endif
}

