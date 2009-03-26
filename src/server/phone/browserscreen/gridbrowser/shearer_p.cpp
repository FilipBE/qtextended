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

#include "shearer_p.h"
#include "selecteditem.h"
#include "griditem.h"

#include <QPainter>
#include <QGraphicsRectItem>

const QString Shearer::mDescription("Shear");
const qreal Shearer::DEFAULT_MAX_SHEAR_VAL = 0.8;


Shearer::Shearer(qreal _maxShearVal)
    : maxShearVal(_maxShearVal)
    , parabola(0.5,maxShearVal) // maxX (i.e. half the percent), maxY
{
}

void Shearer::animate(QPainter *painter,SelectedItem *item,qreal percent)
{
    // Have to translate the entire coordinate system back to zero, so that the shear takes place
    // around the origin. Otherwise, the image will be pulled too far away, the further it is from
    // the origin.
    QPointF pos = item->mapToScene(item->rect().x(),item->rect().y());
    painter->translate(item->rect().width()/2 + pos.x(),item->rect().height()/2 + pos.y());

    qreal shearVal = 0;
    if ( percent <= 0.5 ) {
        // Pulling top left corner up and out, bottom right corner down and out.
        shearVal = parabola(percent);
    } else {
        // Negative shear - pulling the other way.
        shearVal = -(parabola(percent-0.5));
    }
    painter->shear(shearVal,shearVal);

    // Now that we've done the shear, translate back again.
    painter->translate(-item->rect().width()/2 - pos.x(),-item->rect().height()/2 - pos.y());

    int imageSize = item->current()->selectedImageSize();
    draw(painter,item,imageSize,imageSize);
}
