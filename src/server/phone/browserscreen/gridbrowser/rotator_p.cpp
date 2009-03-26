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
#include "rotator_p.h"
#include "selecteditem.h"
#include "griditem.h"

#include <QPainter>

const QString Rotator::mDescription("Rotate");


Rotator::Rotator(int _numSpins)
    : numSpins(_numSpins)
{
}

void Rotator::animate(QPainter *painter,SelectedItem *item,qreal percent)
{
    painter->translate(item->rect().x() + item->rect().width()/2,item->rect().y() + item->rect().height()/2);
    painter->rotate(360.0 * percent * numSpins);
    painter->translate(-(item->rect().width()/2) - item->rect().x() ,-(item->rect().height()/2)-item->rect().y());

    int imageSize = item->current()->selectedImageSize();
    const QPixmap &pixmap = item->current()->selectedPic();
    if ( !(pixmap.isNull()) ) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        draw(painter,pixmap,item,imageSize,imageSize);
    }
}

