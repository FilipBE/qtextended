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

#include "renderer.h"
#include <QSvgRenderer>
#include <QPicture>
#include <QPainter>
#include <QFileInfo>
#include <QDebug>

class SvgRenderer : public Renderer
{
public:
    bool load(const QString &filename);
    void render(QPainter *painter, const QRectF &bounds);

private:
    QSvgRenderer renderer;
};

bool SvgRenderer::load(const QString &filename)
{
    if (renderer.load(filename) && renderer.isValid())
        return true;

    return false;
}

void SvgRenderer::render(QPainter *painter, const QRectF &bounds)
{
    renderer.render(painter, bounds);
}

//===========================================================================
#ifndef QT_NO_PICTURE
class PictureRenderer : public Renderer
{
public:
    bool load(const QString &filename);
    void render(QPainter *painter, const QRectF &bounds);

private:
    QPicture picture;
    QString fname;
};

bool PictureRenderer::load(const QString &filename)
{
    fname = filename;
    if (picture.load(filename) && picture.size() > 0)
        return true;

    return false;
}

void PictureRenderer::render(QPainter *painter, const QRectF &bounds)
{
    QRect br = picture.boundingRect();
    QTransform worldTransform = painter->worldTransform();
    painter->translate(bounds.topLeft());
    painter->scale(bounds.width()/br.width(), bounds.height()/br.height());
    painter->drawPicture(br.topLeft(), picture);
    painter->setWorldTransform(worldTransform);
}
#endif
//===========================================================================

Renderer *Renderer::rendererFor(const QString &filename)
{
    QFileInfo fi(filename);
    Renderer *r = 0;
    if (fi.suffix() == "svg") {
        r = new SvgRenderer;
#ifndef QT_NO_PICTURE
    } else if (fi.suffix() == "pic") {
        r = new PictureRenderer;
#endif
    }

    if (r && !r->load(fi.filePath())) {
        delete r;
        r = 0;
    }

    return r;
}

