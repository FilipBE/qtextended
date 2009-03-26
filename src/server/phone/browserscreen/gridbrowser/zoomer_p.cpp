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

#include "zoomer_p.h"
#include "selecteditem.h"
#include "griditem.h"

#include <QPainter>

const QString Zoomer::mDescription("Zoom");
const qreal Zoomer::growthFactor = 0.8;
const qreal Zoomer::stop1 = 0.6;
const qreal Zoomer::stop2 = 0.8;

void Zoomer::animate(QPainter *painter,SelectedItem *item,qreal percent)
{
    GridItem *currentItem = item->current();
    if ( !currentItem ) {
        return;
    }
    int imageSize = currentItem->selectedImageSize();

    if ( percent < Zoomer::stop1 ) {
        // decreasing opacity
        qreal decreaseOpacityBy = (percent) * Zoomer::stop1;
        painter->setOpacity(painter->opacity() - painter->opacity() * decreaseOpacityBy);

        // increasing up to growthFactor
        qreal increaseBy = percent * Zoomer::growthFactor * 2;
        int sz = static_cast<int>(imageSize + imageSize * increaseBy);
        draw(painter,item,sz,sz);
    } else if ( percent < Zoomer::stop2 ) {
        // inverse
        qreal increaseOpacityBy = (1.0-percent) * Zoomer::stop1;
        painter->setOpacity(painter->opacity() - painter->opacity() * increaseOpacityBy);

        qreal decreaseBy = (1.0-percent) * Zoomer::growthFactor * 2;
        int sz = static_cast<int>(imageSize + imageSize * decreaseBy);
        draw(painter,item,sz,sz);
    } else {
        qreal increaseOpacityBy = (1.0-percent) * Zoomer::stop1;
        painter->setOpacity(painter->opacity() - painter->opacity() * increaseOpacityBy);
        draw(painter,item,imageSize,imageSize);
    }
}

/*
void Zoomer::animate(QPainter *painter,QSvgRenderer *renderer,
                     int imageSize,QGraphicsRectItem *item,qreal percent)
{
    if ( percent < stop1 ) {
        // decreasing opacity
        qreal decreaseOpacityBy = (percent) * stop1;
        painter->setOpacity(painter->opacity() - painter->opacity() * decreaseOpacityBy);

        // increasing up to growthFactor
        qreal increaseBy = percent * growthFactor * 2;
        int sz = static_cast<int>(imageSize + imageSize * increaseBy);
        draw(painter,renderer,item,sz,sz);
    } else if ( percent < stop2 ) {
        // inverse
        qreal increaseOpacityBy = (1.0-percent) * stop1;
        painter->setOpacity(painter->opacity() - painter->opacity() * increaseOpacityBy);

        qreal decreaseBy = (1.0-percent) * growthFactor * 2;
        int sz = static_cast<int>(imageSize + imageSize * decreaseBy);
        draw(painter,renderer,item,sz,sz);
    } else {
        qreal increaseOpacityBy = (1.0-percent) * stop1;
        painter->setOpacity(painter->opacity() - painter->opacity() * increaseOpacityBy);
        draw(painter,renderer,item,imageSize,imageSize);
    }
}
*/
