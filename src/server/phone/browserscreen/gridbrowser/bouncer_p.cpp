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

#include "bouncer_p.h"
#include "oscillator_p.h"
#include "selecteditem.h"
#include "griditem.h"
#include "renderer.h"

#include <QPainter>
#include <QPixmap>

/*!
  \internal
  \class Bouncer
    \inpublicgroup QtBaseModule

  \brief A class which knows how to animate SelectedItem objects within
  the PhoneLauncherView.
  The effect is that of the object shaking. Note
  that this operation is fairly expensive, since Bouncer has been
  implemented as a bounded sin wave.
  
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/


const QString Bouncer::mDescription("Bounce");
const qreal Bouncer::DEFAULT_MIN_VARIATION = -0.3;
const qreal Bouncer::DEFAULT_MAX_VARIATION = 0.4;
const qreal Bouncer::SPEED_FACTOR = 0.3;


/*!
  \internal
  \fn Bouncer::Bouncer(qreal minVariation,qreal maxVariation,int frameMax)
  \a minVariation: Percent (+/- 0-1) of the object's normal dimensions, this
  being the amount that the object should shrink down to. For example, if
  the SelectedItem's image has a width and height of 50 pixels and the
  minimum variation is -0.2, the object can be reduced by 20%, and so will
  'bounce down' to 40 pixels.
  \a maxVariation: Percent (+/- 0-1) of the object's normal dimensions, this
  being the amount that the object should increase up to. For example, if
  the SelectedItem's image has a width and height of 50 pixels and the
  minimum variation is 0.5, the object can be increased by 50%, and so will
  'bounce up' to 75 pixels.
  \a frameMax Maximum number of frames in the timeline controlling the
  animation.
*/
Bouncer::Bouncer(qreal _minVariation,qreal _maxVariation,
                 int _frameMax)
    : minVariation(_minVariation)
    , maxVariation(_maxVariation)
    , frameMax(_frameMax)
{
}

/*!
  \internal
  \fn static const QString &Bouncer::description()
  Returns description of this class, for configuration purposes.
*/

/*!
  \internal
*/
void Bouncer::initFromGridItem(GridItem *item)
{
    if (!item)
        return;

    int imageSize = item->selectedImageSize();
    imageSize +=  static_cast<int>(imageSize * maxVariation);
    Renderer *renderer = item->renderer();
    if ( renderer ) {
        bigpixmap = QPixmap(imageSize,imageSize);
        bigpixmap.fill(QColor(0,0,0,0));
        QPainter p(&bigpixmap);
        renderer->render(&p,QRectF(0,0,imageSize,imageSize));
        p.end();
    }
}

/*!
  \internal
  \fn void Bouncer::animate(QPainter *painter,SelectedItem *item,qreal percent)
*/
void Bouncer::animate(QPainter *painter,SelectedItem *item,qreal percent)
{
    GridItem *currentItem = item->current();
    if ( !currentItem ) {
        return;
    }
    int imageSize = currentItem->selectedImageSize();

    // Create an oscillator which will produce a sin wave whose lower bound is the
    // first parameter (i.e. the lowest dimension the image will take, and whose
    // upper bound is the second parameter (i.e. the highest dimension the image
    // will take).
    Oscillator oscillator(imageSize + minVariation * imageSize,
                          imageSize + maxVariation * imageSize,
                          frameMax, // upper bound of x
                          SPEED_FACTOR,
                          imageSize);

    // Ask the oscillator to produce a suitable width/height value for this stage
    // of the animation.
    int y = qRound(oscillator(percent*frameMax));
    if (((y-imageSize) % 2) != 0) {
        y -= 1;
    }

    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    //lazy man's version of mipmapping
    if (y > imageSize) {
        if (!bigpixmap.isNull())
            draw(painter,bigpixmap,item,y,y);
    } else {
        const QPixmap &pixmap = currentItem->selectedPic();
        if ( !(pixmap.isNull()) ) {
            draw(painter,pixmap,item,y,y);
        }
    }
}
