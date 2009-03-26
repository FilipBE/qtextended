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

#include "radialbackground_p.h"
#include "selecteditem.h"

#include <QPainter>
#include <QRadialGradient>
#include <QColor>
#include <QApplication>
#include <QPalette>


const QString RadialBackground::mDescription("Radial");


/*!
  \internal
  \class RadialBackground
    \inpublicgroup QtBaseModule

  \brief A class which provides an animated background for SelectedItem
  objects.
  RadialBackground displays as a wave of colour moving outwards
  from the centre of the SelectedItem's bounding area, over time.
  This class is intended to be used in conjunction with
  another Animation object, which animates the SelectedItem's image.
  
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/


/*!
  \internal
  \fn static const QString &RadialBackground::description()
  Returns description of this class, for configuration purposes.
*/

/*!
  \internal
  \fn void RadialBackground::animate(QPainter *painter,SelectedItem *item,qreal percent)
*/
void RadialBackground::animate(QPainter *painter,SelectedItem *item,qreal percent)
{
    static const qreal gradientWidth = 0.1;

    // Define a radial gradient with a centre point, a radius and a focal point.
    QPointF centre(item->rect().x()+item->rect().width()/2,
                   item->rect().y()+item->rect().height()/2);
    QRadialGradient gradient(centre,item->rect().width()/2,centre);

    //gradient.setColorAt(0.0,painter->brush().color());
    gradient.setColorAt(0.0,QColor(Qt::transparent));

    // The wave of colour moves outwards from the centre of the SelectedItem.
    QColor color = qApp->palette().highlight().color();
    gradient.setColorAt(percent,color);
    qreal val = 0.0;
    if ( (percent + gradientWidth) > 1.0 ) {
        val = 1.0;
    } else {
        val = percent + gradientWidth;
        gradient.setColorAt(val,QColor(Qt::transparent));
        //gradient.setColorAt(val,painter->brush().color());
    }

    // Also get this background to fade over time.
    painter->save();
    qreal opacity = painter->opacity();
    qreal newOpacity = opacity - (opacity * percent);
    painter->setOpacity(newOpacity);

    painter->fillRect(item->rect(),gradient);
    painter->restore();
}
