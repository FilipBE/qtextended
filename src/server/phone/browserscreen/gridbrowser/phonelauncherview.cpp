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

#include "phonelauncherview.h"

#include "selecteditem.h"
#include "griditem.h"
#include "animator_p.h"
#include "animatorfactory_p.h"

#include <QApplication>
#include <QPixmap>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QFocusEvent>
#include <QColor>
#include <qtopiaapplication.h>


const QString PhoneLauncherView::SELECTED_BACKGROUND_FILE_MOUSE_PREFERRED(":image/phone/launcher_icon_sel_light");
const QString PhoneLauncherView::SELECTED_BACKGROUND_FILE_DEFAULT(":image/phone/launcher_icon_sel");


/*!
    \class PhoneLauncherView
    \inpublicgroup QtBaseModule

    \brief The PhoneLauncherView class is a view onto a scene, which is a grid of objects.

    PhoneLauncherView creates and maintains the scene itself; it is capable of creating and
    adding the grid objects which will populate the scene's grid. Each grid object represents
    a folder or application, i.e. something that can be 'opened'.

    In addition to the grid objects, a 'selected' item is positioned over the 'current'
    grid object. The selected item has a different appearance to its current grid object, and is
    also able to animate.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
    \sa GridBrowserScreen
*/

// Grid objects are modelled by the GridItem class. The selected item is modelled by the
// SelectedItem class.
//
// See also GridItem and SelectedItem
//
// Events:
//
// PhoneLauncherView listens for mouse events and keyboard events. Clicking on a GridItem
// object (mouse press followed by mouse release) causes that GridItem to become current (i.e.
// SelectedItem will now be positioned over it) and activated (i.e. the GridItem's underlying
// application will be invoked). Pressing any of the keyboard characters listed in 'iconMapping' also
// causes the GridItem at that index to become current and activated.
//
// Note that SelectedItem also listens for keyboard events -- it handles the arrow keys, that enable
// it to shift horizontally or vertically to a neighbour. Consequently, SelectedItem must always have
//keyboard focus.


/*!
  \fn PhoneLauncherView::PhoneLauncherView(int rows, int cols, const QString &mapping,const QString &animator,const QString &animatorBackground,QWidget *parent=0)

  \a rows: Number of rows that will be in the grid.
  \a cols: Number of columns that will be in the grid.
  \a mapping: A list of characters that map to grid objects according to their position. For example,
  the first character in mapping will refer to the grid object at row 0, column 0, the second
  character will refer to row 0, column 1 etc. The mapping is used by \l{function}{keyPressEvent(QKeyEvent *)}.
  \a animator: The name of an animation object, which generally comes from a configuration file. Note that this name must be known to AnimatorFactory, which is responsible for the creation of the animators. The string may be an empty string, in which case there will be no animation. Note, however, that if any movie files (.mng) are present for a QContent object, these will take precedence over any animator.
  \a animatorBackground: The name of an animation object (to be used for the background during animation), which generally comes from a configuration file. Note that this name must be known to AnimatorFactory, which is responsible for the creation of the animators. The string may be an empty string, in which case there will be no background animation.
  \a parent: Optional parent widget.
*/
PhoneLauncherView::PhoneLauncherView(int _rows, int _columns, const QString &mapping,
                                     const QString &_animatorDescription,const QString &_animatorBackgroundDescription,
                                     QWidget *parent)
    : QGraphicsView(parent)
    , m_rows(_rows)
    , m_columns(_columns)
    , scene(0)
    , selectedItem(0)
    , pressedItem(0)
    , pressedindex(-1)
    , animatorDescription(_animatorDescription)
    , animatorBackground(0)
    , iconMapping(mapping)
    , margin(MARGIN_DEFAULT)
{
    // Avoid using Base brush because it is semi-transparent (i.e. slow).
    setBackgroundRole(QPalette::Window);

    // Don't want an ugly frame around the launcher.
    setFrameShape(NoFrame);

    // We know that the Window brush is completely transparent,
    // so we can save some cycles by not painting it.
    viewport()->setAutoFillBackground(false);

    // Create & populate the scene.
    scene = new QGraphicsScene();
    setScene(scene);

    // Calculate the margin for the SelectedItem.
    bool isMousePreferred = Qtopia::mousePreferred();
    if ( isMousePreferred ) {
        margin = MARGIN_MOUSE_PREFERRED;
    } else {
        margin = MARGIN_DEFAULT;
    }

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Calculate the selectedItem's background pixmap file.
    QString backgroundFileName;
    if ( isMousePreferred ) {
        backgroundFileName = SELECTED_BACKGROUND_FILE_MOUSE_PREFERRED;
    } else {
        backgroundFileName = SELECTED_BACKGROUND_FILE_DEFAULT;
    }

    // Create and add selectedItem.
    addSelectedItem(backgroundFileName,DEFAULT_MOVE_TIME_DURATION);

    // Create the shared animator background object.
    animatorBackground = AnimatorFactory::animator(_animatorBackgroundDescription);
}

/*!
  \fn PhoneLauncherView::~PhoneLauncherView()
*/
PhoneLauncherView::~PhoneLauncherView()
{
     delete scene;
     if ( animatorBackground )
         delete animatorBackground;
}

/*!
  \fn void PhoneLauncherView::pressed(QContent content)
  Indicates that a key or mouse press has occurred which, when released, will
  result in the item on the grid with the given content to be selected.
  \a content See description.
*/

/*!
  \fn void PhoneLauncherView::clicked(QContent content)
  Indicates that an item on the grid with the given content has been selected. Typically, this
  is handled by invoking that item's underlying application.
  \a content See description.
*/

/*!
  \fn void PhoneLauncherView::highlighted(QContent content)
  Indicates that the item on the grid with the given content has become the new current item.
  \a content See description.
*/

/*!
  \internal
  \fn GridItem *PhoneLauncherView::createItem(QContent *content,int row,int column) const
  Creates and returns a new GridItem object, adding it to the SelectedItem and to the scene.
  \a content: Contains information to be used by the GridItem object, such as the
  Icon used to display it, the name of its underlying application, etc.
  \a row: Item's row position in the grid (starts from 0).
  \a column: Item's column position in the grid (starts from 0).
*/
GridItem *PhoneLauncherView::createItem(QContent *content,int row,int column) const
{
    Animator *animator = 0;
    Animator *background = 0;
    bool loadMovie = false;

    if ( content ) {
        // Each GridItem has its own animator, since the Animators may need to cache
        // values (specific to GridItem objects) to speed up processing.
        // That design decision may need to be reassessed in the future.
        // Note: the animatorBackground IS shared among GridItems and thus must be cleaned up
        // by the phonelauncherview.
        // Note: there is no memory leak here, since QGraphicsScene will delete all of its
        // contained graphical items upon destruction. Note also that the new object has no
        // size - its size will be calculated and assigned during resizeEvent(...).
        animator = AnimatorFactory::animator(animatorDescription);
        background = animatorBackground;

        // If an animator has been specified (regardless of whether or not it was successfully
        // created), the GridItem SHOULD NOT attempt to load any movie files, so leave 'loadMovie'
        // as false.
        if ( (animatorDescription.isNull()) || (animatorDescription.isEmpty()) ) {
            loadMovie = true;
        }
    }

    GridItem *item = new GridItem(content,row,column,m_rows,m_columns,
                            animator,background,loadMovie);

    // Add the item to both the scene and the SelectedItem.
    if ( selectedItem->addItem(item) ) {
        scene->addItem(item);
    } else {
        qWarning("PhoneLauncherView::createItem(...): Failed to add new GridItem for row %d, column %d",row,column);
    }

    return item;
}

/*!
  \internal
  \fn void PhoneLauncherView::addSelectedItem(const QString &backgroundFileName,int moveTimeDuration)
  Called by constructor to create and add the SelectedItem, which highlights the current
  GridItem object.
  \a backgroundFileName: an image file used to supply a background for the SelectedItem's image.
  \a moveTimeDuration: time, in milliseconds, that it should take to move the SelectedItem
  from one GridItem to the next in response to a keyboard command.
*/
void PhoneLauncherView::addSelectedItem(const QString &backgroundFileName,int moveTimeDuration)
{
    if ( !scene ) {
        qWarning("PhoneLauncherView::addSelectedItem(...): Can't populate scene -- need scene.");
        return;
    }

    // Make the moveable SelectedItem object that slides is positioned over the current
    // GridItem object. Note: there is no memory leak here, since QGraphicsScene will delete
    // all of its contained graphical items.
    selectedItem = new SelectedItem(backgroundFileName,margin,moveTimeDuration);
    // Signal for when a GridItem object is invoked, eg by pressing the Qt::Select key.
    connect(selectedItem->connector(),SIGNAL(itemSelected(GridItem*)),this,SLOT(itemSelectedHandler(GridItem*)));
    // Signal for when a GridItem object is about to be invoked, eg by pressing the Qt::Select key.
    connect(selectedItem->connector(),SIGNAL(itemPressed(GridItem*)),this,SLOT(itemPressedHandler(GridItem*)));
    // Signal for when the current GridItem object changes.
    connect(selectedItem->connector(),SIGNAL(selectionChanged(GridItem*)),this,SLOT(selectionChangedHandler(GridItem*)));

    // Make sure that selectedItem is higher in the stacking order than the (default) GridItem objects.
    selectedItem->setZValue(selectedItem->zValue()+1);

    scene->addItem(selectedItem);

    // The next 2 statements ensure that the SelectedItem can get keyboard events.
    selectedItem->setSelected(true);
    scene->setFocusItem(selectedItem);

    // The other items (GridItem objects) are added by external calls of addItem(...)
}

/*!
  \internal
  \fn void PhoneLauncherView::itemDimensions(int *itemWidth,int *itemHeight) const
  Retrieves the appropriate width and height for each GridItem object, according to the
  current dimensions of the view.
*/
void PhoneLauncherView::itemDimensions(int *itemWidth,int *itemHeight) const
{
    // Get the pixel size of the view's margins.
    int left,top,right,bottom;
    getContentsMargins(&left,&top,&right,&bottom);

    int viewWidth = size().width() - left - right - 1;
    int viewHeight = size().height() - top - bottom - 1;
    if (itemWidth)  *itemWidth  = viewWidth/m_columns;
    if (itemHeight) *itemHeight = viewHeight/m_rows;
}

/*!
  \internal
  \fn void PhoneLauncherView::rowAndColumn(int idx,int &row,int &column) const
  Translates an item index (one-dimensional) into a row and a column (two-dimensional)
  as used by the SelectedItem to refer to the GridItem objects.
*/
void PhoneLauncherView::rowAndColumn(int idx,int &row,int &column) const
{
    row = idx/m_columns;
    column = idx - (m_columns*row);
}

/*!
  \internal
  Returns the row count for this view.
*/
int PhoneLauncherView::rows() const
{
    return m_rows;
}

/*!
  \internal
  Returns the column count for this view.
*/
int PhoneLauncherView::columns() const
{
    return m_columns;
}

/*!
  \fn void PhoneLauncherView::addItem(QContent* content, int pos)
  Creates a new GridItem object and adds it to the scene, or else updates the GridItem
  at the given position, if it already exists. When a new GridItem object is added,
  its position and size will be calculated and set in response to resizeevent(...).
  When an existing GridItem object is modified, its position and size do not
  change in response to \a content -- position and size are determined by other factors,
  such as the size of the view and the current resolution.

  \a content: Contains information to be used by the GridItem object, such as the
  Icon used to display it, the name of its underlying application, etc.
  \a pos: The index of the GridItem object, which will be converted into a
  row and a column position.
*/
void PhoneLauncherView::addItem(QContent* content, int pos)
{
    if ( !selectedItem ) {
        qWarning("PhoneLauncherView::addItem(...): Could not add GridItem for pos %d - no selected item.",pos);
        return;
    }

    // Calculate row and column from pos.
    int row, column;
    rowAndColumn(pos,row,column);

    GridItem *item = 0;
    if ( (item = selectedItem->item(row,column)) ) {
        // Already have an item for that row and column. That means we need to update it, not
        // create it.
        item->setContent(content);
        scene->update();
    } else {
        // Create the new GridItem object.
        createItem(content,row,column);
    }
}

/*!
  \fn QContent *PhoneLauncherView::currentItem() const
  Returns the content information for the current GridItem object, or 0 if no item is current.
*/
QContent *PhoneLauncherView::currentItem() const
{
    GridItem *currentItem = currentGridItem();
    if ( currentItem ) {
        return currentItem->content();
    } else {
        return 0;
    }
}

/*!
  \internal
  \fn GridItem *PhoneLauncherView::currentGridItem() const
  Returns the current GridItem object, or 0 if no item is current.
*/
GridItem *PhoneLauncherView::currentGridItem() const
{
    if ( selectedItem ) {
        return selectedItem->current();
    } else {
        return 0;
    }
}

/*!
  \fn void PhoneLauncherView::setCurrentItem(int idx)
  Causes the item positioned at the given index to become the current item. If \a idx is
  invalid, no change will occur.
  \a idx: The index of a GridItem object.
*/
void PhoneLauncherView::setCurrentItem(int idx)
{
    if ( selectedItem ) {
        // Calculate the row and column of the required item according to the index.
        int row, column;
        rowAndColumn(idx,row,column);

        // Make sure we've actually got an item for that row and column, even if it's
        // an item with empty content.
        GridItem *item = selectedItem->item(row,column);
        if ( !item ) {
            item = createItem(0,row,column);
        }

        selectedItem->setCurrent(item,false); // 2nd param means no animation
    }
}

/*!
  \fn void PhoneLauncherView::setBusy(bool flag)
  Currently does nothing. In the future, this function could allow
  PhoneLauncherView to change the appearance of the SelectedItem object
  to indicate that it is 'busy' (for example, starting an application).
  \a flag: Not currently used.
*/
void PhoneLauncherView::setBusy(bool flag)
{
    /*if ( selectedItem ) {
        selectedItem->setActive(flag);
    }*/ //active means 'pressed', so don't use it here
    Q_UNUSED(flag);
}

/*!
  \fn void PhoneLauncherView::updateImages()
  Causes all GridItem objects to query their QContent counterparts to refresh their image
  information. This function should be called when image data has/may have changed.
*/
void PhoneLauncherView::updateImages()
{
    if ( selectedItem ) {
        selectedItem->updateImages();
    }
}

/*!
  \internal
  \fn void PhoneLauncherView::resizeEvent(QResizeEvent *event)
  Resizes all the scene objects, relative to the new size of this view.
*/
void PhoneLauncherView::resizeEvent(QResizeEvent *event)
{
    scene->setSceneRect(0,0,event->size().width(),event->size().height());

    if ( !selectedItem ) {
        return;
    }

    // Get the width and height of each item.
    int itemWidth;
    int itemHeight;
    itemDimensions(&itemWidth,&itemHeight);

    // Loop through all the GridItem objects and reset their sizes.
    for ( int row = 0; row < m_rows; row++ ) {
        for ( int col = 0; col < m_columns; col++ ) {
            GridItem *item = selectedItem->item(row,col);
            if ( !item ) {
                // Create and add an empty content for this row and column.
                item = createItem(0,row,col);
            }

            // Calculate the item's position in the scene.
            int x = col * itemWidth;
            int y = row * itemHeight;
            item->setRect(x,y,itemWidth,itemHeight);
        }
    }

    // Ask the SelectedItem to also recalculate its size.
    selectedItem->resize();

    QGraphicsView::resizeEvent(event);
}

/*!
  \internal
  \fn void PhoneLauncherView::focusInEvent(QFocusEvent *event)
  Ensures that when the PhoneLauncherView is focused after an activity such as launching an
  application, the current icon will animate.
*/
void PhoneLauncherView::focusInEvent(QFocusEvent *event)
{
    QGraphicsView::focusInEvent(event);

    if ( selectedItem && event->reason() != Qt::PopupFocusReason && !Qtopia::mousePreferred()) {
        selectedItem->startAnimating();
    }

    if (Qtopia::mousePreferred())
        topLevelWidget()->setWindowTitle(tr("Main Menu"));
    else if (currentItem())
        topLevelWidget()->setWindowTitle(currentGridItem()->content()->name());
}

/*!
  \internal
  \fn void PhoneLauncherView::itemSelectedHandler(GridItem *item)
  Called in response to the current item being activated (e.g. by Ok button, by mouse click, etc).
  Extracts information from the item and emits the signal clicked(QContent).
  Signals emitted: \l{function}{clicked(QContent)}.
*/
void PhoneLauncherView::itemSelectedHandler(GridItem *item)
{
    if ( !item ) {
        qWarning("PhoneLauncherView::itemSelectedHandler(...): Error - no item in itemSelectedHandler");
        return;
    }

    if ( item->content() && !(item->content()->name().isEmpty()) ) {
        emit clicked(*(item->content()));
    }
}

/*!
  \internal
  \fn void PhoneLauncherView::itemPressedHandler(GridItem *item)
  Called when the key press for activating an \a item occurs.
  Extracts information from the item and emits the signal pressed(QContent).
*/
void PhoneLauncherView::itemPressedHandler(GridItem *item)
{
    if ( item && item->content() && !(item->content()->name().isEmpty()) ) {
        emit pressed(*(item->content()));
    }
}

/*!
  \internal
  \fn void PhoneLauncherView::selectionChangedHandler(GridItem *item)
  Called in response to the current item changing (i.e. SelectedItem has shifted position,
  highlight a different GridItem object). Updates the window title, and emits the
  signal highlighted(...).
  Signals emitted: \l{function}{highlighted(QContent)}.
*/
void PhoneLauncherView::selectionChangedHandler(GridItem *item)
{
    if ( !item ) {
        qWarning("PhoneLauncherView::selectionChangedHandler(...): Error - item parameter is null.");
        // Set the title to a space rather than an empty string. This causes the title bar to
        // remain the same, but just without a title. An empty string will cause a different kind of
        // title bar to appear.
        if (!Qtopia::mousePreferred())
            topLevelWidget()->setWindowTitle(" ");
        return;
    }

    if ( !(item->content()) ) {
        // Set the title to a space rather than an empty string. This causes the title bar to
        // remain the same, but just without a title. An empty string will cause a different kind of
        // title bar to appear.
        if (!Qtopia::mousePreferred())
            topLevelWidget()->setWindowTitle(" ");
        return;
    }

    emit highlighted(*(item->content()));
    if (!Qtopia::mousePreferred())
        topLevelWidget()->setWindowTitle(item->content()->name());
}

/*!
  \internal
  \fn QContent *PhoneLauncherView::itemAt(int row, int column) const
  Returns the QContent object that resides at the given row and column.
*/
QContent *PhoneLauncherView::itemAt(int row, int column) const
{
    QContent *ret = 0;
    GridItem *gridItem = gridItemAt(row, column);
    if (gridItem) {
        ret = gridItem->content();
    }
    return ret;
}

/*!
  \internal
  \fn QContent *PhoneLauncherView::itemAt(const QPoint &point) const
  Returns the QContent object that resides at the given pixel position.
*/
QContent *PhoneLauncherView::itemAt(const QPoint &point) const
{
    QContent *ret = 0;
    GridItem *gridItem = gridItemAt(point);
    if (gridItem) {
        ret = gridItem->content();
    }
    return ret;
}

/*!
  \internal
  \fn GridItem *PhoneLauncherView::gridItemAt(int row, int column) const
  Returns the GridItem object that resides at the given row and column.
*/
GridItem *PhoneLauncherView::gridItemAt(int row, int column) const
{
    return (selectedItem) ? selectedItem->item(row,column) : 0;
}

/*!
  \internal
  \fn GridItem *PhoneLauncherView::gridItemAt(const QPoint &point) const
  Returns the GridItem object that resides at the given pixel position.
*/
GridItem *PhoneLauncherView::gridItemAt(const QPoint &point) const
{
    // Get the item at position 'point'.
    int itemWidth, itemHeight;
    itemDimensions(&itemWidth,&itemHeight);
    int column = point.x()/itemWidth;
    int row = point.y()/itemHeight;

    return gridItemAt(row, column);
}

/*!
  \internal
  \fn void PhoneLauncherView::keyPressEvent(QKeyEvent *event)
  If a key that is listed in the icon mapping is pressed, the GridItem object
  which maps to that key becomes the current item and 'active'.
  Signals emitted: \l{function}{highlighted(QContent)}, \l{function}{clicked(QContent)}.
*/
void PhoneLauncherView::keyPressEvent(QKeyEvent *event)
{
    const char a = event->text()[0].toLatin1();
    int index = iconMapping.toLatin1().indexOf(a);

    if ( selectedItem && (index >= 0) && (index < m_rows * m_columns) && !event->isAutoRepeat()) {
        pressedindex = index;
        // Find out what row/column index maps to, and set that GridItem to
        // be current (i.e. SelectedItem will shift over to it).
        int row, column;
        rowAndColumn(index,row,column);
        // setCurrent(int,int) causes selectionChanged() to be emitted, which invokes
        // this object's selectionChangedHandler(...) slot, which updates the window
        // title and emits the signal highlighted(...).
        selectedItem->setCurrent(row,column, false);
        // Change the selected item's appearance.
        selectedItem->setActive(true);

        if (selectedItem->current()->content())
            emit pressed(*selectedItem->current()->content());
    } else {
        // Key not handled by PhoneLauncherView.
        QGraphicsView::keyPressEvent(event);
    }
}

/*!
  \internal
*/
void PhoneLauncherView::keyReleaseEvent(QKeyEvent *event)
{
    const char a = event->text()[0].toLatin1();
    int index = iconMapping.toLatin1().indexOf(a);

    if ( selectedItem && (index >= 0) && (index < m_rows * m_columns) && pressedindex == index && !event->isAutoRepeat()) {
        pressedindex = -1;
        // Change the selected item's appearance.
        selectedItem->setActive(false);

        // We also cause the item to be activated, i.e. its underlying application will be
        // invoked. Note that this auses SelectedItem's connector's itemSelected(...)
        // signal to be emitted, which is connected to this object's slot itemSelectedHandler(),
        // which emits clicked(...).
        selectedItem->triggerItemSelected(selectedItem->current());
    } else {
        QGraphicsView::keyReleaseEvent(event);    
    }
}

/*!
  \internal
  \fn void PhoneLauncherView::mousePressEvent(QMouseEvent *event)
  Stores the GridItem object that is being clicked.
*/
void PhoneLauncherView::mousePressEvent(QMouseEvent *event)
{
    pressedItem = gridItemAt(event->pos());
    if ( selectedItem ) {
        if ( selectedItem->current() != pressedItem ) {
            // This is different to the current item - so we make the new one current.
            // Note that this causes SelectedItem's connector's selectionChanged(...)
            // signal to be emitted, which is connected to this object's slot
            // selectionChangedHandler(), which emits highlighted(...) and causes the
            // window title to be updated.
            selectedItem->setCurrent(pressedItem, false);
            if (selectedItem->current()->content())
                emit pressed(*selectedItem->current()->content());
        }
        selectedItem->setActive(true);
    }
}

/*!
  \internal
  \fn void PhoneLauncherView::mouseReleaseEvent(QMouseEvent *event)
  If a GridItem object has been clicked, it becomes the current item (i.e. SelectedItem
  will highlight it) and signals will be issued to indicate that that particular item
  has been invoked.
  Signals emitted: \l{function}{highlighted(QContent)}, \l{function}{clicked(QContent)}.
*/
void PhoneLauncherView::mouseReleaseEvent(QMouseEvent *event)
{
    GridItem *releasedItem = gridItemAt(event->pos());

    if ( releasedItem && (pressedItem == releasedItem) ) {
        // We've got an item that we've released the button on, and it's the
        // same as the one we've pressed on.
        if ( selectedItem ) {
            // The clicked-on item becomes activated, i.e. its underlying application will be
            // invoked. Note that this auses SelectedItem's connector's itemSelected(...)
            // signal to be emitted, which is connected to this object's slot itemSelectedHandler(),
            // which emits clicked(...).
            selectedItem->triggerItemSelected(selectedItem->current());
        }
    }

    if ( selectedItem )
        selectedItem->setActive(false);

    pressedItem = 0;

    // If a double-click occurs the SelectedItem loses its selection for some reason, so it is
    // advisable to do the following.
    if ( selectedItem ) {
        selectedItem->setSelected(true);
    }
}

/*!
  \internal
*/
void PhoneLauncherView::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::PaletteChange && selectedItem) {
        for ( int row = 0; row < m_rows; row++ ) {
            for ( int col = 0; col < m_columns; col++ ) {
                GridItem *item = selectedItem->item(row,col);
                if ( item )
                    item->paletteChanged();
            }
        }
        selectedItem->paletteChanged();
    } else if (e->type() == QEvent::StyleChange) {
        //icons may have changed size
        updateImages();
    }
}
