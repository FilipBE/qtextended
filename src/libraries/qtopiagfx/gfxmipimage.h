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

#ifndef GFXMIPIMAGE_H
#define GFXMIPIMAGE_H

#include <gfxtimeline.h>
#include <QSize>
#include <QImage>
#include <QVector>

class GfxPainter;
class GfxMipImage
{
public:
    GfxMipImage(const QSize &);
    virtual ~GfxMipImage();

    const QSize &size() const { return _s; }
    void clear() { _imgs.clear(); }

    void addImage(const QImage &);
    QRect boundingRect() const;

    GfxValue &x() { return _x; }
    GfxValue &y() { return _y; }
    const GfxValue &x() const { return _x; }
    const GfxValue &y() const { return _y; }
    GfxValue &rotate() { return _rotate; }
    const GfxValue &rotate() const { return _rotate; }
    GfxValue &zoom() { return _zoom; }
    const GfxValue &zoom() const { return _zoom; }
    GfxValue &quality() { return _quality; }
    const GfxValue &quality() const { return _quality; }

    virtual void paint(GfxPainter &);

private:
    QSize _s;

    GfxValue _x;
    GfxValue _y;
    GfxValue _rotate;
    GfxValue _zoom;
    GfxValue _quality;

    QVector<QImage> _imgs;
};

#endif
