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

#include "gfxmipimage.h"
#include "gfxpainter.h"
#include <QDebug>

static bool gfx_show_mip_matches_loaded = false;
static bool gfx_show_mip_matches = false;

GfxMipImage::GfxMipImage(const QSize &s)
: _s(s)
{
    _zoom.setValue(1.);

    if(!gfx_show_mip_matches_loaded) {
        gfx_show_mip_matches_loaded = true;
        if(!QString(getenv("GFX_SHOW_MIP_MATCHES")).isEmpty())
            gfx_show_mip_matches = true;
    }
}

GfxMipImage::~GfxMipImage()
{
}

void GfxMipImage::addImage(const QImage &img)
{
    if(img.format() != QImage::Format_RGB32 && 
       img.format() != QImage::Format_RGB16) {
        qWarning() << "GfxMipImage: Mip image only supports RGB32 and RGB16 "
                      "images";
        return;
    }

    // Imgs are stored in ascending order of size.
    int width = img.width();
    for(int ii = 0; ii < _imgs.count(); ++ii) {
        if(width < _imgs.at(ii).width()) {
            _imgs.insert(ii, img);
            return;
        }
    }
    _imgs.append(img);
}

QRect GfxMipImage::boundingRect() const
{
    // XXX Unimplemented if there is a rotation
    if(_rotate.value() == 0.) {
        QSize s = _zoom.value() * _s;
        return QRect(QPoint(int(_x.value()) - s.width() / 2, int(_y.value()) - s.height() / 2 ), s);
    } else {
        qreal rotate = _rotate.value();
        qreal zoom = _zoom.value();
        QMatrix m;
        m.translate(_x.value(), _y.value());
        if(rotate) m.rotate(rotate);
        if(zoom != 1.) m.scale(zoom, zoom);
        m.translate(-_s.width() / 2, -_s.height() / 2);
        return m.mapRect(QRect(QPoint(0, 0), _s));
    }
}

void GfxMipImage::paint(GfxPainter &p)
{
    if(_imgs.isEmpty() || _zoom.value() == 0.) return;

    // Find correct image
    QSize s = _zoom.value() * _s;
    int imgNum;
    for(imgNum = 0; imgNum < _imgs.count(); ++imgNum) 
        if(_imgs.at(imgNum).width() >= s.width())
            break;
    if(imgNum == _imgs.count())
        imgNum = _imgs.count() - 1;


    const QImage &img = _imgs.at(imgNum);

    // Adjust zoom
    qreal zoom =  qreal(s.width()) / qreal(img.width());
    qreal rotate = _rotate.value();

    if(zoom == 1. && rotate == 0.) {
        // XXX - until copy and drawImage are unified
        QRect r(int(_x.value()) - s.width() / 2, int(_y.value()) - s.height() / 2, s.width(), s.height());
        p.drawImage(r.topLeft(), img);  
        if(gfx_show_mip_matches)
            p.fillRect(r, QColor(255, 0, 0, 127));
    } else {
        QMatrix m;
        m.translate(_x.value(), _y.value());
        if(rotate)
            m.rotate(rotate);
        if(zoom != 1.)
            m.scale(zoom, zoom);
        m.translate(-img.width() / 2, -img.height() / 2);
        p.drawImageTransformed(m, img, _quality.value() != 0.);
    }
}

