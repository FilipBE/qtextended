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

#include "griditem.h"
#include "animator_p.h"
#include "renderer.h"

#include <QContent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QPixmap>
#include <QApplication>
#include <QMovie>
#include <QPen>
#include <QByteArray>


const QString GridItem::MOVIE_PREFIX(":image/");
const QString GridItem::MOVIE_SUFFIX("_48_anim");
const qreal GridItem::SELECTED_IMAGE_SIZE_FACTOR = 0.4;

/*!
  \internal
  \class GridItem
    \inpublicgroup QtBaseModule

  \brief GridItem is a graphical object used to populate the scene required by
  PhoneLauncherView.

  Each GridItem object stores its own row and column indices for its
  position in the grid in which it will be displayed.

  In addition to using \a basicPixmap to draw itself, GridItem creates and
  maintains the pixmap used to display SelectedItem (when it is positioned over this
  GridItem object), a renderer (if the GridItem can be loaded from a scalable image format)
  and the movie used to animate SelectedItem (when it is positioned
  over this GridItem object).

  Note that GridItem objects cannot be copied or assigned to.

  It is possible to create an 'empty' GridItem object, i.e. one that contains no content. Such an
  object can be used in those cases where a particular version of Qt Extended will not release all of
  the available applications.

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
  \sa PhoneLauncherView
  \sa SelectedItem
*/


/*!
  \internal
  \fn GridItem::GridItem(QContent *content,int row,int col,int totalRows,int totalCols,Animator *animator,Animator *animatorBackground,bool loadMovie,QGraphicsScene *scene)
  Creates the images and the movie required to display both this object and the SelectedItem object.

  \a content: Contains information used to create the pixmaps and the movie. This may be 0, in which case
      an 'empty' GridItem object will be created - see documentation above.
  \a row: The row index for this item, i.e. where it will be positioned within its containing grid.
  \a col: The column index for this item, i.e. where it will be positioned within its containing grid.
  \a totalRows: The total number of rows in the grid in which this item is contained.
  \a totalCols: The total number of columns in the grid in which this item is contained.
  \a animator: An object which is able to animate the GridItem when it is selected (i.e. by
  SelectedItem), in the absence of a QMovie object.
  \a animatorBackground: An object which (optionally) provides a background for coded animation.
  \a loadMovie: Determines whether or not the object should search for a movie file for its animation.
  \a scene: The (optional) scene on which this item should be drawn.
*/
GridItem::GridItem(QContent *_content,int _row,int _col,
                   int _totalRows,int _totalCols,
                   Animator *animator,Animator *animatorBackground,
                   bool _loadMovie,
                   QGraphicsScene *scene)
    : QGraphicsRectItem(0,scene)
    , mContent(_content)
    , mRow(_row)
    , mCol(_col)
    , totalRows(_totalRows)
    , totalCols(_totalCols)
    , mSelectedImageSize(-1)
    , mRenderer(0)
    , basicPixmap()
    , mSelectedPixmap()
    , mSelectedMovie(0)
    , mSelectedAnimator(animator)
    , mSelectedBackgroundAnimator(animatorBackground)
    , loadMovie(_loadMovie)
{
    // Set up a pen to use for drawing operations.
    paletteChanged();

    updateImages();
}

/*!
  \internal
  \fn GridItem::~GridItem()
*/
GridItem::~GridItem()
{
    if ( mSelectedAnimator ) {
        delete mSelectedAnimator;
    }
    if ( mSelectedMovie ) {
        delete mSelectedMovie;
    }
    if ( mRenderer )
        delete mRenderer;
}

/*!
  \internal
  \fn QContent *GridItem::content() const
*/

/*!
  \internal
  \fn void GridItem::setContent(QContent *content)
  Used to modify the contents (e.g. the icon) of this GridItem.
*/
void GridItem::setContent(QContent *_content)
{
    mContent = _content;
    updateImages();
}

/*!
  \internal
  \fn int GridItem::row() const
  Returns the grid row that this GridItem resides in. Rows are counted from 0.
*/

/*!
  \internal
  \fn int GridItem::column() const
  Returns the grid column that this GridItem resides in. Columns are counted from 0.
*/

/*!
  \internal
  \fn void GridItem::updateImages()
  Called by the ctor. Creates the QPixmaps and the QMovie used to display this object and the
  SelectedItem object, based on information in content.
*/
void GridItem::updateImages()
{
    if ( mSelectedMovie ) {
        delete mSelectedMovie;
        mSelectedMovie = 0;
    }

    if ( !mContent ) {
        // We won't be able to make any images. It is valid to have an empty content - see doco above.
        basicPixmap = QPixmap();
        mSelectedPixmap = QPixmap();
        if (mRenderer) {
            // Make sure it's not anymore!
            delete mRenderer;
            mRenderer = 0;
        }
        return;
    }

    // Retrieve the static images, one for displaying the GridItem (basicPixmap)
    // the other for displaying the SelectedItem when it is positioned over this
    // GridItem.
    int sz = basicImageSize();

    // First, attempt to create the renderer.
    mRenderer = Renderer::rendererFor(":image/" + mContent->iconName());
    if (mRenderer) {
        // We use the renderer to create two Pixmaps -- one to display the GridItem itself, and
        // one to display the SelectedItem when it is magnifying the GridItem object.
        // Obviously, the SelectedItem's Pixmap is exactly the same as the GridItem's Pixmap,
        // except that it has a different size. We hang on to the renderer, because we may need
        // that to do coded animations.
        QImage image = QImage(sz,sz,QImage::Format_ARGB32_Premultiplied);
        image.fill(0);
        QPainter painter(&image);
        mRenderer->render(&painter,QRectF(0,0,sz,sz));
        painter.end();
        basicPixmap = QPixmap::fromImage(image);
    } else {
        // Probably not a scalable image - use the content's icon, directly, to get pixmaps.
        basicPixmap = mContent->icon().pixmap(sz);
    }

    // Create the movie.
    if ( loadMovie && !(mContent->iconName().isNull()) ) {
        mSelectedMovie = new QMovie(MOVIE_PREFIX  + mContent->iconName() + MOVIE_SUFFIX);
        if ( mSelectedMovie->isValid() ) {
            // TODO !!!!!!!!!!!! THIS HAS GOT TO GO. Waiting for Oslo...
            mSelectedMovie->setCacheMode(QMovie::CacheAll);
        } else {
            // This movie was never meant to be...
            delete mSelectedMovie;
            mSelectedMovie = 0;
        }
    }

    //clear selected pixmap
    mSelectedPixmap = QPixmap();

    //let animation know the grid item image may have changed
    if (mSelectedAnimator)
        mSelectedAnimator->initFromGridItem(this);
}


/*!
  \internal
  Returns the SelectedItem object, based on information in content.
*/
const QPixmap &GridItem::selectedPic() const
{
    if (mSelectedPixmap.isNull()) {
        int sz = selectedImageSize();
        if (mRenderer) {
            QImage image = QImage(sz,sz,QImage::Format_ARGB32_Premultiplied);
            image.fill(0);
            QPainter painter(&image);
            const_cast<GridItem *>(this)->mRenderer->render(&painter,QRectF(0,0,sz,sz));
            painter.end();
            const_cast<GridItem *>(this)->mSelectedPixmap = QPixmap::fromImage(image);
        } else if ( mContent ) {
            const_cast<GridItem *>(this)->mSelectedPixmap = mContent->icon().pixmap(sz);
        }
    }

    return mSelectedPixmap;
}

/*!
  \internal
  \fn const QPixmap &GridItem::basicPic() const
  This function should be used for drawing the GridItem if there is no valid renderer.
  \sa renderer()
*/

/*!
  \internal
  \fn QMovie *GridItem::movie() const
  This function should be used for animating a SelectedItem object, if the animated fil
  is available. If there is no animated file, the SelectedItem will use its own contrived
  animation objects.
*/

/*!
  \internal
  \fn Renderer *GridItem::renderer()
  Returns a valid renderer, or 0.
  The renderer can be used during painting. If there is no valid renderer for this GridItem
  object, the pixmap methods can be used to retrieve pixmaps to draw. NOTE that if possible, the
  pixmap methods should be used, since rendering is very expensive.
  \sa selectedPic()
*/
Renderer *GridItem::renderer()
{
    return mRenderer;
}

/*!
  \internal
  \fn Animator *GridItem::selectedAnimator() const
  Returns the Animator object for coded animation by the SelectedItem, or 0 if none is available.
*/

/*!
  \internal
  \fn Animator *GridItem::selectedBackgroundAnimator() const
  Returns the background animator for coded animation by the SelectedItem, or 0 if none is
  available.
*/

/*!
  \internal
  \fn void GridItem::paint(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *widget)
  Draws this object, using \a basicPixmap.
*/
void GridItem::paint(QPainter *painter,const QStyleOptionGraphicsItem *,QWidget *)
{
    painter->setPen(pen());

    // Draw the broken lines around the edge of the image.
    int x1 = static_cast<int>(rect().x());
    int y1 = static_cast<int>(rect().y());
    int x2 = x1 + static_cast<int>(rect().width()) - 1;
    int y2 = y1 + static_cast<int>(rect().height()) - 1;
    if ( mRow < totalRows-1 ) {
        // Draw bottom line.
        painter->drawLine(x1+IMAGE_LINE_OFFSET, y2, x2-IMAGE_LINE_OFFSET, y2);
    }
    if ( mCol < totalCols-1 ) {
        // Draw right line.
        painter->drawLine(x2, y1+IMAGE_LINE_OFFSET, x2, y2-IMAGE_LINE_OFFSET);
    }

    // Position the image in the middle of the drawing area.
    if ( !(basicPixmap.isNull()) ) {
        // Position the image in the middle of the drawing area.
        int imgX = x1 + (static_cast<int>(rect().width()) - basicPixmap.width())/2;
        int imgY = y1 + (static_cast<int>(rect().height()) - basicPixmap.height())/2;
        painter->drawPixmap(imgX,imgY,basicPixmap); //x1,y1,image);
    }
    // If there's no valid pixmap, we don't have much chance of drawing anything, my friend...
}

/*!
  \internal
  \fn QRectF GridItem::renderingBounds()
  Creates on demand the bounding infomration for the renderer.
*/
QRectF GridItem::renderingBounds()
{
    int requiredSize = basicImageSize();

    // We update the rendering bounds if a) we have a valid renderer (otherwise there's no point)
    // and b) either the bounds have not yet been created, or something has caused the basic
    // image size to change - which generally it will not.
    if (mRenderer &&
         (mRenderingBounds.isNull() || (requiredSize != mRenderingBounds.width())) ) {
        // Position the image in the middle of the drawing area.
        int x1 = static_cast<int>(rect().x());
        int y1 = static_cast<int>(rect().y());
        x1 += (static_cast<int>(rect().width()) - requiredSize)/2;
        y1 += (static_cast<int>(rect().height()) - requiredSize)/2;

        mRenderingBounds.setX(x1);
        mRenderingBounds.setY(y1);
        mRenderingBounds.setWidth(requiredSize);
        mRenderingBounds.setHeight(requiredSize);
    }

    return mRenderingBounds;
}

/*!
  \internal
  \fn int GridItem::basicImageSize() const;
  Returns the image size (in pixels) for GridItem's \a basicImage, for the current resolution.
*/
int GridItem::basicImageSize() const
{
    int imageSize = qApp->style()->pixelMetric(QStyle::PM_IconViewIconSize);
    return imageSize;
}

/*!
  \internal
  \fn int GridItem::selectedImageSize() const;
  Returns the size of this GridItem when it is selected (i.e. by SelectedItem).
*/
int GridItem::selectedImageSize() const
{
    int sz = basicImageSize();

    return sz + qRound(sz * SELECTED_IMAGE_SIZE_FACTOR);
}

/*!
  \internal
  Updates the colors used by the GridItem.
*/
void GridItem::paletteChanged()
{
    QColor col(QApplication::palette().color(QPalette::Midlight));
    col.setAlpha(64);
    QPen pen(col);
    setPen(pen);
}

