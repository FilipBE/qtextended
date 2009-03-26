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

#include "animator_p.h"
#include "selecteditem.h"
#include "griditem.h"
#include "renderer.h"

#include <QPainter>
#include <QPixmap>
#include <QImage>
#include <QGraphicsRectItem>
#include <QDebug>

static const qreal gradientWidth = 0.1;


/*!
  \internal
  \class Animator
    \inpublicgroup QtBaseModule

  \brief Animator is the superclass of all the objects that animate icons within the PhoneLauncherView.

  An animation is generally controlled by a timeline, or similar. At each step
  during the timeline's life cycle, the animate(...) method should be called. This method
  will know how to transform a SelectedItem's representation, and has the option of
  calling Animator's draw(QPainter *,SelectedItem *,int width,int height).

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/


/*!
 \internal
  \fn virtual void Animator::animate(QPainter *,SelectedItem *,qreal percent) = 0

   Derived classes should implement this function to transform the appearance of the
   SelectedItem at each step during the animation process.
   \a painter: Used to draw the SelectedItem's image.
   \a item: The item that is to be drawn, by asking its current GridItem for the renderer
   or for the pixmap.
   \a percent: Value between 0 and 1 which will indicate how far into the animation
   we are, and therefore how the object should be drawn.
*/

/*!
  \internal
  \fn void Animator::draw(QPainter *painter,SelectedItem *item,int width,int height)

  This method can be called by derived classes to draw the SelectedItem for the
  given width and height parameters. It uses the item's SVG renderer, if it can -
  otherwise it attempts to retrieve the pixmap for the selected item.
  The method is intended to be called at each step of the animation, from the
  method animate(...).
*/
void Animator::draw(QPainter *painter,SelectedItem *item,int w,int h)
{
    // Get the GridItem object that is currently associated with the SelectedItem, and
    // find out whether we have a valid renderer we can use to draw it, or whether we
    // must use the pixmap.
    GridItem *currentItem = item->current();
    if ( currentItem ) {
        Renderer *renderer = currentItem->renderer();
        if ( renderer ) {
            draw(painter,renderer,item,w,h);
            return;
        }
        const QPixmap &pixmap = currentItem->selectedPic(); // use the pixmap
        if ( !(pixmap.isNull()) ) {
            draw(painter,pixmap,item,w,h);
        }
    }
}

/*!
  \internal
  \fn void Animator::draw(QPainter *painter,const QPixmap &pixmap,QGraphicsRectItem *item,int width,int height)
   
  Draws the SelectedItem's pixmap, for the given width and height. This method should be called 
  by derived classes if they want to always use pixmaps for the animation (usually for performance reasons). 
*/
void Animator::draw(QPainter *painter,const QPixmap &pixmap,QGraphicsRectItem *item,int w,int h)
{
    // Find out if we have to scale the image to fit the given width/height parameters.
    if ( pixmap.width() != w || pixmap.height() != h ) {
        painter->translate(static_cast<int>(item->rect().x() + (item->rect().width() - w)/2),
                           static_cast<int>(item->rect().y() + (item->rect().height() - h)/2));
        painter->scale((float)w/pixmap.width(), (float)h/pixmap.height());
        
        // The graphical item contains the pixmap. If the pixmap is too large for it, we will
        // need to temporarily enlarge the graphical item, so that the pixmap will display
        // completely, without being clipped.
        QRectF oldRect = item->rect();
        if ( w > oldRect.width() || h > oldRect.height() ) {
            QRectF bounds = renderingBounds(item,w,h);
            item->setRect(bounds);
            painter->drawPixmap(0,0,pixmap);
            // After drawing the pixmap, put the grahical item's dimensions back the way they were.
            item->setRect(oldRect);
        } else {
            painter->drawPixmap(0,0,pixmap);
        }
    } else {
        // No scaling involved.
        painter->drawPixmap(static_cast<int>(item->rect().x() + (item->rect().width() - w)/2),
                            static_cast<int>(item->rect().y() + (item->rect().height() - h)/2),
                            pixmap);
    }
}

/*!
  \internal
  \fn void Animator::draw(QPainter *painter,Renderer *renderer,QGraphicsRectItem *item,int width,int height)
   Draws the SelectedItem for the given width and height, using the renderer.
*/
void Animator::draw(QPainter *painter,Renderer *renderer,QGraphicsRectItem *item,int w,int h)
{
    // If what we want to draw is going to be too large for the graphical item, we will
    // need to temporarily enlarge the graphical item, so that the drawing will display
    // completely, without being clipped.
    qreal oldWidth = item->rect().width();
    qreal oldHeight = item->rect().height();
    if ( w > oldWidth || h > oldHeight ) {
        QRectF bounds = renderingBounds(item,w,h);
        item->setRect(QRectF(item->rect().x()-1,item->rect().y()-1,w+2,h+2));
        renderer->render(painter,bounds);
        // After drawing, put the grahical item's dimensions back the way they were.
        item->setRect(QRectF(item->rect().x()+1,item->rect().y()+1,oldWidth,oldHeight));
    } else {
        QRectF bounds = renderingBounds(item,w,h);
        renderer->render(painter,bounds);
    }
}

/*!
  \internal
  \fn QRectF Animator::renderingBounds(QGraphicsRectItem *item,int width,int height)
  Returns the geometry for the renderer, i.e. position, width and height.
*/
QRectF Animator::renderingBounds(QGraphicsRectItem *item,int w,int h)
{
    // Position the image in the middle of the drawing area.
    qreal x = item->rect().x();
    qreal y = item->rect().y();
    x += (item->rect().width() - w)/2;
    y += (item->rect().height() - h)/2;

    { // TODO: this is a workaround. not casting to ints at 
      // higher levels (GridItem, SelectedItem, etc) would be better solution.
        GridItem *currentItem = ((SelectedItem*)item)->current();
        if ( !currentItem ) {
            return QRectF();
        }
        int imageSize = currentItem->selectedImageSize();
        qreal xoffset = (static_cast<int>(item->rect().width()) - imageSize)/2 - (item->rect().width() - imageSize)/2;
        qreal yoffset = (static_cast<int>(item->rect().height()) - imageSize)/2 - (item->rect().height() - imageSize)/2;
        x += xoffset;
        y += yoffset;
    }

    return QRectF(x,y,w,h);
}

/*!
  \internal
  Allows animations to do any needed initialization (such as caching) based on \a item.
  This function is also called when a grid item's image needs to be updated (so, for exmaple,
  the animation's cache could be updated).

  The default implementation does nothing.
*/
void Animator::initFromGridItem(GridItem *item)
{
   Q_UNUSED(item);
}
