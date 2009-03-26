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

#include "selecteditem.h"
#include "griditem.h"
#include "selecteditemconnector.h"
#include "animator_p.h"

#include <QGraphicsScene>
#include <QPixmap>
#include <QMovie>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QApplication>
#include <QKeyEvent>
#include <QTimeLine>
#include <QTimer>
#include <Qtopia>


/*!
  \internal
  \class SelectedItem
    \inpublicgroup QtBaseModule

  \brief   SelectedItem is a graphical object which highlights the currently selected GridItem
  object in the PhoneLauncherView's grid.

  The SelectedItem maintains pointers to all of the GridItem objects in the grid, stored in
  a GridItemTable. This enables the SelectedItem to accept certain keyboard events which
  enable it to move across from the current item to its neighbours in the indicated Direction
  (Up, Down, Left or Right). The SelectedItem is also able to animate whenever a new GridItem
  object becomes current.

  The SelectedItem relies on the current GridItem object for both sizing information and the
  QPixmap, QMovie and/or QSvgRenderer that it uses to display itself.

  Since SelectedItem does not inherit from QObject, it contains a SelectedItemConnector to
  handle callbacks, thus avoiding multiple inheritance.

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
  \sa SelectedItemConnector
  \sa GridItem
*/


/*!
  \internal
  \fn SelectedItem::SelectedItem(const QString &backgroundFileName,int margin,int slideTimeInterval=500,QGraphicsScene *scene=0)
  \a backgroundFileName: Name of the file used to display the image's background.
  \a margin: The SelectedItem is larger than the GridItem that it magnifies. The 'margin' attribute
   is the number of extra pixels added to each border of the current GridItem, affecting
   the SelectedItem's size and position. For example, the width of SelectedItem is
   GridItem's width + (margin * 2).
   \a slideTimeInterval: Number of milliseconds that it should take for the SelectedItem to move
   across from the current GridItem object to a neighbour in the grid.
   \a scene: The (optional) scene on which this item should be drawn.
*/
SelectedItem::SelectedItem(const QString &_backgroundFileName,
                           int _margin,int _slideTimeInterval,QGraphicsScene *scene)
    : QGraphicsRectItem(0,scene)
    , mConnector(0)
    , background(0)
    , backgroundFileName(_backgroundFileName)
    , margin(_margin)
    , mSelectedSize(-1,-1)
    , table()
    , currentItem(0)
    , destItem(0)
    , movie(0)
    , active(false)
    , pressed(false)
    , moveTimeLine(0)
    , playTimeLine(0)
    , slideTimeInterval(_slideTimeInterval)
    , animationStage(0)
    , animationPending(false)
    , currentX(-1)
    , currentY(-1)
    , destX(-1)
    , destY(-1)
    , mXDrift(-1)
    , mYDrift(-1)
{
    // The item supports selection. Enabling this feature will enable setSelected()
    // to toggle selection for the item.
    setFlag(QGraphicsItem::ItemIsSelectable,true);

    // The item supports keyboard input focus (i.e., it is an input item). Enabling
    // this flag will allow the item to accept focus, which again allows the delivery
    // of key events to QGraphicsItem::keyPressEvent() and QGraphicsItem::keyReleaseEvent().
    setFlag(QGraphicsItem::ItemIsFocusable,true);

    // Create the connector object that supports the signals/slots mechanism.
    mConnector = new SelectedItemConnector(this);

    // Create the timeline that is responsible for moving the SelectedItem from one
    // GridItem to its neighbour.
    createMoveTimeLine();

    // And a timeline to control coded animation.
    createPlayTimeLine();
}

// Create the timeline that is responsible for moving the SelectedItem from one
// GridItem to its neighbour.
void SelectedItem::createMoveTimeLine()
{
    if ( moveTimeLine ) {
        delete moveTimeLine;
    }

    moveTimeLine = new QTimeLine(slideTimeInterval,mConnector);
    moveTimeLine->setFrameRange(15,85);
    QObject::connect(moveTimeLine,SIGNAL(frameChanged(int)),mConnector,SLOT(moving(int)));
    QObject::connect(moveTimeLine,SIGNAL(stateChanged(QTimeLine::State)),mConnector,SLOT(movingStateChanged(QTimeLine::State)));
}

// And a timeline to control coded animation.
void SelectedItem::createPlayTimeLine()
{
    if ( playTimeLine ) {
        delete playTimeLine;
    }

    playTimeLine = new QTimeLine(ANIMATION_TIME,mConnector);
    playTimeLine->setFrameRange(0,100);
    QObject::connect(playTimeLine,SIGNAL(frameChanged(int)),mConnector,SLOT(playing(int)));
    QObject::connect(playTimeLine,SIGNAL(stateChanged(QTimeLine::State)),mConnector,SLOT(animationStateChanged(QTimeLine::State)));
}

/*!
  \internal
  \fn SelectedItem::~SelectedItem()
*/
SelectedItem::~SelectedItem()
{
    // Make sure the movie isn't still running.
    detachAnimation();

    delete moveTimeLine;
    delete playTimeLine;
    delete mConnector;
    if ( background ) {
        delete background;
    }
}

/*!
  \internal
  \fn bool SelectedItem::addItem(GridItem *item)
  Adds a new item to the store, and returns true if the item was added successfully, or false if
  there was already an object in the item's row:column position.
*/
bool SelectedItem::addItem(GridItem *item)
{
    return table.add(item);
}

/*!
  \internal
  \fn GridItem *SelectedItem::current() const
  Returns the current GridItem object, or 0 if no item is current.
*/
GridItem *SelectedItem::current() const
{
    if ( destItem ) {
        return destItem;
    }

    return currentItem;
}

/*!
  \internal
  \fn GridItem *SelectedItem::item(int row,int column) const
  Returns the GridItem object that is stored at the given row and column position, or 0 if
  no object is stored at that position.
*/
GridItem *SelectedItem::item(int row,int column) const
{
    return table.item(row,column);
}

/*!
  \internal
  \fn QObject *SelectedItem::connector() const
  Returns the object responsible for handling the SelectedItem's signals and slots.
*/
QObject *SelectedItem::connector() const
{
    // Upcast.
    return static_cast<QObject *>(mConnector);
}

/*!
  \internal
  \fn QPoint SelectedItem::pos(GridItem *_item) const
  Returns the appropriate pixel position for the SelectedItem when positioned over the given
  item.
*/
QPoint SelectedItem::pos(GridItem *item) const
{
    int x = 0, y = 0;
    QSize _selectedSize = selectedSize(item);

    if ( item->column() <= 0 ) { // sitting on left border
        x = static_cast<int>(item->rect().x());
    } else if ( item->column() >= table.topColumn() ) { // sitting on right border
        x = static_cast<int>(item->rect().x()) + static_cast<int>(item->rect().width()) - _selectedSize.width();
    } else {
        // This object should be centred over the GridItem.
        x = static_cast<int>(item->rect().x()) + static_cast<int>(item->rect().width())/2 - _selectedSize.width()/2;
    }
    if ( item->row() <= 0 ) { // sitting on top row
        y = static_cast<int>(item->rect().y());
    } else if ( item->row() >= table.topRow() ) { // sitting on bottom row
        y = static_cast<int>(item->rect().y()) + static_cast<int>(item->rect().height()) - _selectedSize.height();
    } else {
        // This object should be centred over the GridItem.
        y = static_cast<int>(item->rect().y()) + static_cast<int>(item->rect().height())/2 - selectedSize(item).height()/2;
    }

    return QPoint(x,y);
}

/*!
  \internal
  \fn QSize SelectedItem::selectedSize(GridItem *item) const
  Returns the size for this SelectedItem, wrt the given GridItem object. The size
  of this item must be larger than the GridItem size, but must retain the same
  width/height ratio, and also be large enough to comfortably accommodate the
  selected image.
  Note that the dimensions are created on demand, and rely on the assumption that the
  GridItem's size and the selected image's size don't change at runtime.
*/
QSize SelectedItem::selectedSize(GridItem *item) const
{
    if ( mSelectedSize.width() == -1 || mSelectedSize.height() == -1 ) {
        // Recalculate the size.
        mSelectedSize.setWidth(static_cast<int>(item->rect().width()) + (margin * 2));
        mSelectedSize.setHeight(static_cast<int>(item->rect().height()) + (margin * 2));

        int minimum = item->selectedImageSize() + (margin * 2);

        if ( mSelectedSize.width() < minimum ) {
            // Increase width.
            int diff = minimum - mSelectedSize.width();
            mSelectedSize.setWidth(minimum);
            mSelectedSize.setHeight(mSelectedSize.height() + diff);
        }
        if ( mSelectedSize.height() < minimum ) {
            // Increase height.
            int diff = minimum - mSelectedSize.height();
            mSelectedSize.setHeight(minimum);
            mSelectedSize.setWidth(mSelectedSize.width() + diff);
        }
    }

    return mSelectedSize;
}

/*!
  \internal
  \fn QRect SelectedItem::geometry(GridItem *item) const
  Returns the appropriate pixel position and dimensions for the SelectedItem when positioned
  over the given item.
*/
QRect SelectedItem::geometry(GridItem *item) const
{
    // First, get the (x,y) top left-hand corner of this item, when positioned over
    // the GridItem.
    QPoint point = pos(item);

    // Also width & height.
    QSize mSelectedSize = selectedSize(item);
    int w = mSelectedSize.width();
    int h = mSelectedSize.height();

    return QRect(point.x(),point.y(),w,h);
}

/*!
  \internal
  \fn void SelectedItem::drawBackground(QPainter *painter)
  The background image is created on demand, within this method.
*/
void SelectedItem::drawBackground(QPainter *painter)
{
    QRect rectangle = rect().toRect();

    if ( !background ) {
        // Create on demand.
        QImage bgImage(backgroundFileName);

        blendColor(bgImage,QApplication::palette().color(QPalette::Highlight));
        background = new QPixmap(QPixmap::fromImage(
                    bgImage.scaled(rectangle.width(),rectangle.height(),
                        Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    }

    painter->drawPixmap(rectangle.x(),rectangle.y(),*background);
}

/*!
  \internal
  \fn void SelectedItem::paint(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *widget)
  Overrides the superclass method. Handles the static display of the SelectedItem, object
  movement and animation. Calls drawBackground(...) and draw(...).
*/
void SelectedItem::paint(QPainter *painter,const QStyleOptionGraphicsItem *,QWidget *)
{
    if ( !currentItem ) {
        qWarning("SelectedItem::paint(...): No current item.");
        return;
    }

    if (!Qtopia::mousePreferred() || active)
        drawBackground(painter);

    if ( animationState() == Animating ) {
        // During animation, we get multiple calls to paint(...). In each, we paint the next
        // frame of the movie.
        if ( movie ) {
            QPixmap moviePixmap = movie->currentPixmap();
            draw(painter,moviePixmap,
                    static_cast<int>(rect().x()),static_cast<int>(rect().y()));
        } else {
            // We'll try to do a coded animation using the GridItem's Animator object.
            // This call to paint(...) will have been invoked via a signal from playTimeLine
            // which ultimatey triggers playStep(...), which saves the point at which the
            // animation has run to (animationStage) - drawAnimated(...) will make use of
            // animationStage.
            drawAnimated(painter);
        }
    } else {
        // Not currently animating, but we may be sliding across from one item to its
        // neighbour.
        if ( destItem ) {
            // Yes, moving across -- we're going to draw selected images for both the
            // source item and the destination item, but we're going to use this item as
            // the clipping region. First, set up the clipping region.
            painter->setClipRect(rect());

            // During moveStep(...), we calculated new positions for the two items when they
            // are drawn as SelectedItems. This calculation takes into account the magnified
            // area between the two pixmaps.
            draw(painter,currentItem->selectedPic(),currentX,currentY);
            draw(painter,destItem->selectedPic(),destX,destY);
        } else {
            // We're not sliding, we're just drawing 'item' as the selected item.
            if (!Qtopia::mousePreferred() || active)
                draw(painter,currentItem->selectedPic(),static_cast<int>(rect().x()),static_cast<int>(rect().y()));
        }
    }
}

/*!
  \internal
  \fn void SelectedItem::draw(QPainter *painter,const QPixmap &pixmap,int x,int y) const
  Draws the given pixmap in the centre of an area where \a x and \a y indicate the
  top left-hand corner of the area. If this SelectedItem is 'active', draws the pixmap
  with a different appearance.
*/
void SelectedItem::draw(QPainter *painter,const QPixmap &pixmap,int x,int y) const
{
    if ( !pixmap.isNull() ) {
        // Position the image in the centre of this item.
        x += (static_cast<int>(rect().width()) - pixmap.width())/2;
        y += (static_cast<int>(rect().height()) - pixmap.height())/2;

        // The 'active' image has a different appearance.
        if ( active ) {
            QImage img = pixmap.toImage();
            blendColor(img,QApplication::palette().color(QPalette::Highlight));
            painter->drawPixmap(x,y,QPixmap::fromImage(img));
        } else {
            painter->drawPixmap(x,y,pixmap);
        }
    }
}

/*!
  \internal
  \fn void SelectedItem::drawAnimated(QPainter *painter)
  Draws a single step of the animation for a coded animation, using the current
  GridItem's Animator object.
*/
void SelectedItem::drawAnimated(QPainter *painter)
{
    // Pass painter and the percentage of the animation through to the current Animator.
    Animator *animator = currentItem->selectedAnimator();
    if ( !animator ) {
        draw(painter,currentItem->selectedPic(),static_cast<int>(rect().x()),static_cast<int>(rect().y()));
        return;
    }

    // Finds how far we are through the animation - 'percent' will be a value between
    // 0 and 1.
    qreal percent = static_cast<qreal>(animationStage)/100;
    // Draw the animated background first - if there is one - then the animator for
    // this stage of the animation.
    Animator *backgroundAnimator = currentItem->selectedBackgroundAnimator();
    painter->setBrush(QApplication::palette().color(QPalette::Button));
    if ( backgroundAnimator ) {
        backgroundAnimator->animate(painter,this,percent);
    }
    animator->animate(painter,this,percent);
}

/*!
  \internal
  \fn void SelectedItem::blendColor(QImage &img,QColor color)
  Used when this SelectedItem is 'active'.
*/
void SelectedItem::blendColor(QImage &img,QColor color)
{
    //if (img.format() == QImage::Format_ARGB32_Premultiplied) {
    //    img = img.convertToFormat(QImage::Format_ARGB32);
    //}
    color.setAlphaF(0.5);

    // Blend the pixels in 'img' with those in 'color'.
    QPainter blendPainter(&img);
    //blendPainter.setRenderHint(QPainter::Antialiasing,true);
    blendPainter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    blendPainter.fillRect(0, 0, img.width(), img.height(), color.darker());
    blendPainter.end();
}

/*!
  \internal
  \fn void SelectedItem::detachAnimation()
  Stops animation and disconnects the current movie.
*/
void SelectedItem::detachAnimation()
{
    stopAnimation();

    // If there was a movie, make sure it isn't still connected to any slots that we're responsible
    // for, and kiss it goodbye (but do not delete it - it is owned by a GridItem).
    if ( movie ) {
        QObject::disconnect(movie,SIGNAL(frameChanged(int)),mConnector,SLOT(animationChanged()));
        QObject::disconnect(movie,SIGNAL(finished()),mConnector,SLOT(animationFinished()));
        QObject::disconnect(movie,SIGNAL(error(QImageReader::ImageReaderError)),mConnector,SLOT(animationError(QImageReader::ImageReaderError)));
        movie = 0;
    }
}

/*!
  \internal
  \fn void SelectedItem::resetAnimation()
  Stops and disconnects the current animation, fetches new animation information from
  the current GridItem object and connects it up to this object's connector.
*/
void SelectedItem::resetAnimation()
{
    // Stop and disconnect any current movie.
    detachAnimation();

    if ( !currentItem ) {
        // Nothing now selected.
        return;
    }

    // Set up the movie for the new current item.
    movie = currentItem->movie();
    if ( movie ) {
        // Item can animate.
        QObject::connect(movie,SIGNAL(frameChanged(int)),mConnector,SLOT(animationChanged()));
        QObject::connect(movie,SIGNAL(finished()),mConnector,SLOT(animationFinished()));
        QObject::connect(movie,SIGNAL(error(QImageReader::ImageReaderError)),mConnector,SLOT(animationError(QImageReader::ImageReaderError)));
    }
}

/*!
  \internal
  \fn void SelectedItem::updateImages()
  Updates all the Pixmaps for all the GridItem objects.
*/
void SelectedItem::updateImages()
{
    // The images have changed. First, get the SelectedItem to disconnect from the movie.
    detachAnimation();

    // Get all the GridItem objects to rebuild their pixmaps.
    for ( int row = 0; row <= table.topRow(); row++ ) {
        for ( int col = 0; col <= table.topColumn(); col++ ) {
            GridItem *item = table.item(row,col);
            if ( item ) {
                item->updateImages();
            }
        }
    }

    // SelectedItem now has to grab the revised movie.
    resetAnimation();
}

/*!
  \internal
  \fn void SelectedItem::resize()
  Should be called in response to a resize event. Resizes this object with respect to the
  current item. If there is no current item, does nothing.
*/
void SelectedItem::resize()
{
    mSelectedSize = QSize(-1,-1);
    delete background;
    background = 0;

    if ( currentItem ) {
        // Position this object over currentItem.
        QRect rectangle = geometry(currentItem);
        setRect(rectangle);
    }
}

/*!
  \internal
  \fn void SelectedItem::setActive(bool isActive)
  Modifies the \a active attribute, which affects the way that this object is drawn.
*/
void SelectedItem::setActive(bool isActive)
{
    if ( isActive == active ) {
        return;
    }

    active = isActive;

    // Get it to redraw (active/inactive causes a change in appearance).
    update();
}

/*!
  \internal
  \fn void SelectedItem::setCurrent(GridItem *gridItem,bool animate = true);
  Changes the current GridItem object, and causes it to start animating. The item may be 0,
  indicating that no item is currently selected. Emits the selectionChanged signal.
*/
void SelectedItem::setCurrent(GridItem *item,bool animate)
{
    currentItem = item;

    resetAnimation();

    // Resize this object.
    resize();

    // Now we are current - tell the world.
    mConnector->triggerSelectionChanged(currentItem);

    if ( currentItem && animate ) {
        // When a new item is current (i.e. a move is finished, or the view has been shown) we animate
        // the SelectdItem.
        startAnimating();
    }
}

/*!
  \internal
  \fn void SelectedItem::setCurrent(int row,int column,bool animate);
  Changes the current GridItem object by calling \l{function}{SelectedItem::setCurrent(GridItem *item)}.
*/
void SelectedItem::setCurrent(int row,int column,bool animate)
{
    GridItem *item = table.item(row,column);
    setCurrent(item,animate);
}

/*!
  \internal
  \fn void SelectedItem::keyPressEvent(QKeyEvent *event)
  Calls \l{function}{moveRequested(Direction)} in response to Qt::Key_Right, Qt::Key_Left,
  Qt::Key_Up or Qt::Key_Down. Calls triggerItemSelected() in response to Qt::Key_Select or
  Qt::Key_Return. For all other keys, the default functionality applies.
  If there is no current item, does nothing.
*/
void SelectedItem::keyPressEvent(QKeyEvent *event)
{
    if ( !currentItem ) {
        qWarning("SelectedItem::keyPressEvent(...): No current item.");
        QGraphicsRectItem::keyPressEvent(event);
        return;
    }

    switch( event->key() ) {
    case Qt::Key_Right:
        if ( event->isAutoRepeat() && destItem )
            return;
        moveRequested(Right);
        break;
    case Qt::Key_Left:
        if ( event->isAutoRepeat() && destItem )
            return;
        moveRequested(Left);
        break;
    case Qt::Key_Up:
        if ( event->isAutoRepeat() && destItem )
            return;
        moveRequested(Up);
        break;
    case Qt::Key_Down:
        if ( event->isAutoRepeat() && destItem )
            return;
        moveRequested(Down);
        break;
    case Qt::Key_Select: // Qtopia pad
    case Qt::Key_Return: // otherwise
        if ( event->isAutoRepeat())
            return;
        setActive(true);
        pressed = true;
        if ( destItem ) {
            // User has chosen to move to a new destination already, so they want to select
            // that one.
            triggerItemPressed(destItem);
        } else {
            triggerItemPressed(currentItem);
        }
        break;
    default:
        if ( event->isAutoRepeat())
            return;
        QGraphicsRectItem::keyPressEvent(event);
        return;
    }
}

void SelectedItem::keyReleaseEvent(QKeyEvent *event)
{
    if ( !currentItem ) {
        qWarning("SelectedItem::keyReleaseEvent(...): No current item.");
        QGraphicsRectItem::keyReleaseEvent(event);
        return;
    }

    switch( event->key() ) {
    case Qt::Key_Select: // Qtopia pad
    case Qt::Key_Return: // otherwise
        if ( event->isAutoRepeat() || !pressed)
            return;
        if ( destItem ) {
            // User has chosen to move to a new destination already, so they want to select
            // that one.
            triggerItemSelected(destItem);
        } else if ( currentItem ) {
            triggerItemSelected(currentItem);
        } // Otherwise, nothing to select
        setActive(false);
        pressed = false;
        break;
    default:
        QGraphicsRectItem::keyReleaseEvent(event);
        return;
    }
}

/*!
  \internal
  \fn void SelectedItem::moveRequested(Direction direction)
  Moves this object in the given direction. Calls \l{function}{moveTo(GridItem *_destItem)}.
*/
void SelectedItem::moveRequested(Direction direction)
{
    int row;
    int col;
    if ( destItem ) {
        // Move already in progress.
        row = destItem->row();
        col = destItem->column();
    } else {
        row = currentItem->row();
        col = currentItem->column();
    }

    switch( direction) {
    case Right:
        col++;
        break;
    case Left:
        col--;
        break;
    case Up:
        row--;
        break;
    case Down:
        row++;
        break;
    default:
        qWarning("SelectedItem::moveRequested(...): Error - invalid direction of %d.",direction);
        return;
    }

    if ( (row < 0) || (row > table.topRow()) || (col < 0) || (col > table.topColumn()) ) {
        // We ain't going nowhere.
        return;
    }

    // Retrieve the destination item.
    GridItem *_destItem = table.item(row,col);
    if ( !_destItem ) {
        qWarning("SelectedItem::moveRequested(...): Error - no item for row %d, column %d.",row,col);
        return;
    }

    // Move the selection.
    moveTo(_destItem);
}

/*!
  \internal
  \fn void SelectedItem::triggerItemSelected(GridItem *)
  Called when an item has been selected/invoked.
  Emits SelectedItemConnector::itemSelected() signal.
*/
void SelectedItem::triggerItemSelected(GridItem *item)
{
    if ( isAnimating() ) {
        stopAnimation();
    }

    // Cause connector to emit itemSelected(GridItem *) signal.
    mConnector->triggerItemSelected(item);
}

/*!
  \internal
  \fn void SelectedItem::triggerItemPressed(GridItem *)
  Called when a press occurs as part of an item's
  selection.
  Emits SelectedItemConnector::itemPressed() signal.
*/
void SelectedItem::triggerItemPressed(GridItem *item)
{
    // Cause connector to emit itemPressed(GridItem *) signal.
    mConnector->triggerItemPressed(item);
}

qreal SelectedItem::xDrift()
{
    if ( mXDrift == -1 ) {
        // Hasn't been worked out yet.
        mXDrift = ((1 + GridItem::SELECTED_IMAGE_SIZE_FACTOR) * rect().width())
                 - rect().width();
    }

    return mXDrift;
}

qreal SelectedItem::yDrift()
{
    if ( mYDrift == -1 ) {
        // Hasn't been worked out yet.
        mYDrift = ((1 + GridItem::SELECTED_IMAGE_SIZE_FACTOR) * rect().height())
                 - rect().height();
    }

    return mYDrift;
}

/*!
  \internal
  \fn void SelectedItem::moveTo(GridItem *_destItem)
  Positions the SelectedItem over the given GridItem, by starting the timer that will
  gradually shift this SelectedItem. The timer will cause moveStep(int) to be
  called, periodically.
*/
void SelectedItem::moveTo(GridItem *_destItem)
{
    // Stop any previous move and any animations.
    if ( destItem ) {
        // Setting destItem to 0 means that setCurrent(...) won't be called by moveFinished,
        // which would be overkill, as it respositions the object, restarts animation, etc.
        // What WAS the destination item becomes the new current item.
        currentItem = destItem;
        destItem = 0;
    }
    if ( moveTimeLine->state() != QTimeLine::NotRunning ) {
        moveTimeLine->stop();  // triggers moveFinished
        // TODO: This is because Qt's QTimeLine has a bug which doesn't reset the frames
        // when you stop the timeline. When you restart, it acts as though it has been paused.
        // Repair when Qt bug is resolved.
        //createMoveTimeLine();
    }

    // The order is important here. It is possible that this move request is interrupting an
    // animation. If that's the case, we'll want to stop the animation - BUT that will trigger
    // animationFinished() which calls for a redraw that we don't actually want. We set destItem
    // FIRST (so it's non-zero), so that animationFinished() can check if there's a move in the
    // pipeline, and refrain from drawing.
    destItem = _destItem;

    if ( isAnimating() ) {
        stopAnimation(); // triggers animationFinished()
    }

    // Start the timer. This will cause moveStep(int) to be called, periodically.
    moveTimeLine->start();
}

/*!
  \internal
  \fn void SelectedItem::moveStep(int percent)
  Called periodically during a move operation. Moves this object between currentItem and
  destItem.
*/
void SelectedItem::moveStep(int n)
{
    qreal percent = n/100.0;

    // Sanity check.
    if ( !currentItem || !destItem ) {
        qWarning("SelectedItem::moveStep(...): Error - either current item missing or destination item missing.");
        return;
    }

    // Find out how where the SelectedItem box ought to be at this stage in the move, and
    // shift it accordingly.
    QRect destRect = geometry(destItem);
    QRect srcRect = geometry(currentItem); // we might be in the middle of a move, so don't use rect() here!

    qreal moveByX = (destRect.x() - srcRect.x()) * percent;
    qreal moveByY = (destRect.y() - srcRect.y()) * percent;

    if ( isAnimating() ) {
        // THis should not be happening, as we stop the movie as soon as a move starts.
        qWarning("SelectedItem::moveStep(...): Still animating when it should not be.");
        stopAnimation();
    }

    setRect(srcRect.x() + moveByX,srcRect.y() + moveByY,srcRect.width(),srcRect.height());

    // We also have to find where the 2 images are going to sit within their designated areas,
    // because SelectedItem is essentially a magnified version of an underlying GridItem, and
    // when we're moving between two, the area between them is magnified too, i.e. the dest
    // item will move TOWARDS its usual position, while the current (soon-to-be-old) item will
    // move AWAY FROM its usual position.

    // Find the "ordinary" positions of the two items, i.e. the top left-hand corners
    // of the 2 items when they are magnified as SelectedItems.
    currentX = srcRect.x();
    currentY = srcRect.y();
    destX = destRect.x();
    destY = destRect.y();

    // Find the x position of current and dest items.
    if ( currentItem->column() < destItem->column() ) {
        // Moving from left to right.
        currentX -= qRound(percent * xDrift()); // current (left) drifts away to the left over time
        destX += qRound((1-percent) * xDrift()); // destination (right) drifts in from the right over time
    } else if ( currentItem->column() > destItem->column() ) {
        // Moving from right to left.
        currentX += qRound(percent * xDrift()); // current (right) drifts away to the right over time
        destX -= qRound((1-percent) * xDrift()); // destination (left) drifts in from the left over time
    } // else, if they're equal, currentX & destX will be their usual positions

    // Find the y position of current and dest items.
    if ( currentItem->row() < destItem->row() ) {
        // Moving down.
        currentY -= qRound(percent * yDrift()); // current (top) drifts upwards over time
        destY += qRound((1-percent) * yDrift()); // destination (bottom) drifts up from below over time
    } else if ( currentItem->row() > destItem->row() ) {
        // Moving up.
        currentY += qRound(percent * yDrift()); // current (bottom) drifts downwards over time
        destY -= qRound((1-percent) * yDrift()); // destination (top) drifts down from above over time
    } // else, if they're equal, currentY & destY will be their usual positions
}

/*!
  \internal
  \fn void SelectedItem::moveFinished()
  Called when timeline's state changed to NotRunning. Calls \l{function}{setCurrent(GridItem *)}
  and switches off \a destItem.
*/
void SelectedItem::moveFinished()
{
    if ( destItem ) {
        setCurrent(destItem);
    }

    destItem = 0;
}

/*!
  \internal
  \fn void SelectedItem::playStep(int _animationStage)
  Called during coded animation (i.e. when no movie is available).
  Causes the item to be redrawn for this step of the animation.
*/
void SelectedItem::playStep(int _animationStage)
{
    animationStage = _animationStage;

    update(boundingRect());
}

SelectedItem::AnimationState SelectedItem::animationState() const
{
    if ( movie && (movie->state() == QMovie::Running) ) {
        return Animating;
    }
    if ( playTimeLine && (playTimeLine->state() == QTimeLine::Running) ) {
        return Animating;
    }
    if ( playTimeLine && animationPending ) {
        return AnimationPending;
    }
    return NotAnimating;
}

/*!
  \internal
  Returns true if the item is currently animating or about to animate.
*/
bool SelectedItem::isAnimating() const
{
    AnimationState state = animationState();
    return ( state == Animating || state == AnimationPending );
}

/*!
  \internal
  \fn void SelectedItem::stopAnimation()
*/
void SelectedItem::stopAnimation()
{
    // Theoretically, these should not both be running at the same time. But you should never
    // trust the wayward devices of the Dark Hand of maintenance...
    if ( movie ) {
        movie->stop();
    }
    if ( playTimeLine ) {
        animationPending = false;
        playTimeLine->stop(); // triggers animationFinished()
        // TODO: This is because Qt's QTimeLine has a bug which doesn't reset the frames
        // when you stop the timeline. When you restart, it acts as though it has been paused.
        // Repair when Qt bug is resolved.
        createPlayTimeLine();
    }
}

/*!
  \internal
  \fn void SelectedItem::startAnimating()
   Delays for a short interval (ANIMATION_DELAY_INTERVAL) to avoid animting when the user is
   quickly moving between objects, then starts the animation timeline.
*/
void SelectedItem::startAnimating()
{
    // Delay animation, so that if users are quickly moving around icons with the arrow keys,
    // they won't be annoyed by weird effects caused by animations starting up and then
    // immediately stopping, which also affects performance.
    animationPending = true; // I hate these stupid little flags, but it gets around the problem that
    // we're delaying an event and we can find out about it in the meantime.
    QTimer::singleShot(ANIMATION_DELAY_INTERVAL,mConnector,SLOT(startAnimation()));
}

void SelectedItem::startAnimationDelayed()
{
    if ( movie ) {
        // Start the movie -- this will advance frames so you will get signals to draw.
        // If the movie is already running, this function does nothing.
        movie->start();
    } else {
        if ( !animationPending ) {
            // Animation was prematurely halted.
            return;
        }
        animationPending = false; // not pending anymore, we're doing it
        if ( !currentItem ) {
            qWarning("SelectedItem::startAnimating() - trying to start animation, but no current item!");
            return;
        }
        if ( currentItem->selectedAnimator() ) {
            // We can start animating codedly.
            playTimeLine->start();
        }
        // Otherwise, no animation for this item.
    }
}

/*!
  \internal
  \fn void SelectedItem::animationFinished()
  Refreshes this item when the animation is done, unless a move request has interrupted
  the animation, in which case this method does nothing and lets the move handle drawing
  from now on.
*/
void SelectedItem::animationFinished()
{
    // We only want to do a redraw IF a move is not already in progress. This is because
    // a move request can interrupt an animation. The move request resets values such as
    // currentX and Y, so if we try to draw to finish off the animation we'll get bogus values.
    // We let the move callbacks take care of it, instead.
    if ( !destItem ) {
        // No move in progress, so redraw.
       update(boundingRect());
    }
}

/*!
  \internal
  Updates the colors used by the SelectedItem.
*/
void SelectedItem::paletteChanged()
{
    // Force colorized background to be reloaded.
    delete background;
    background = 0;
    update(boundingRect());
}

