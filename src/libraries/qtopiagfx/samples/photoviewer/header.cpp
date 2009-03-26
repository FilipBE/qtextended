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

#include "header.h"
#include <QPainter>

Header::Header(GfxCanvasItem *parent)
: GfxCanvasItem(parent)
{

    {
        QFont f;
        f.setPointSize(18);
        GfxCanvasText *t = new GfxCanvasText(QSize(240, 24), this);
        t->x().setValue(120);
        t->y().setValue(12);
        t->setText("Optus");
        t->setColor(Qt::white);
        t->setFont(f);
    }


    {
        QImage img(14, 24, QImage::Format_ARGB32_Premultiplied);
        img.fill(0);
        QPainter p(&img);
        p.fillRect(0, 14, 2, 5, Qt::white);
        p.fillRect(3, 12, 2, 7, Qt::white);
        p.fillRect(6, 10, 2, 9, Qt::white);
        p.fillRect(9, 8, 2, 11, Qt::white);
        p.fillRect(12, 6, 2, 13, Qt::white);

        GfxCanvasImage *i = new GfxCanvasImage(img, this);
        i->y().setValue(12);
        i->x().setValue(17);
    }

    {
        QImage img(28, 24, QImage::Format_ARGB32_Premultiplied);
        img.fill(0);
        QPainter p(&img);
        p.setPen(Qt::white);
        p.drawRect(2, 5, 25, 13);
        p.fillRect(0, 8, 2, 8, Qt::white);

        p.fillRect(24, 7, 2, 10, Qt::white);
        p.fillRect(21, 7, 2, 10, Qt::white);
        p.fillRect(18, 7, 2, 10, Qt::white);
        p.fillRect(15, 7, 2, 10, Qt::white);
        p.fillRect(12, 7, 2, 10, Qt::white);

        GfxCanvasImage *i = new GfxCanvasImage(img, this);
        i->y().setValue(12);
        i->x().setValue(216);
    }
}

