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

#ifndef SELECTEDITEM_H
#define SELECTEDITEM_H

#include "griditemtable.h"

#include <QGraphicsRectItem>
#include <QColor>

class QGraphicsScene;
class GridItem;
class QPixmap;
class GridItem;
class QPainter;
class QStyleOptionGraphicsItem;
class QKeyEvent;
class SelectedItemConnector;
class QTimeLine;
class QMovie;
class QImage;


class SelectedItem : public QGraphicsRectItem
{
public:

    SelectedItem(const QString &backgroundFileName,
                 int margin,int moveTimeInterval = 500,QGraphicsScene *scene = 0);

    ~SelectedItem();

    bool addItem(GridItem *);

    GridItem *current() const;

    GridItem *item(int row,int column) const;

    void setCurrent(int row,int column,bool animate = true);

    void setCurrent(GridItem *gridItem,bool animate = true);

    QObject *connector() const;

    void setActive(bool isActive);

    void updateImages();

    void resize();

    void paint(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *widget);

    void triggerItemPressed(GridItem *);
    void triggerItemSelected(GridItem *);

    void moveStep(int percent);

    void moveFinished();

    void playStep(int);

    void startAnimating();

    void startAnimationDelayed();

    // There is no animationChanged() function, since the SelectedItemConnector just calls update
    // on this object, and the paint(...) method takes care of the rest.

    void animationFinished();

    void resetAnimation();

    void paletteChanged();

protected:

    void keyPressEvent(QKeyEvent *event);

    void keyReleaseEvent(QKeyEvent *event);

private:

    // Direction for keys to move in.
    typedef enum{Up,Down,Left,Right} Direction;

    typedef enum{NotAnimating,Animating,AnimationPending} AnimationState;

    // Get coded animations to run for 1.5 seconds.
    // TODO: make this configurable.
    static const int ANIMATION_TIME = 1500;

    static const int ANIMATION_DELAY_INTERVAL = 200;

    // Colour used to blend images during painting when the SelectedItem is 'active'.
    const QColor highlightColor;
    // Colour used to blend the background image during painting.
    const QColor backgroundHighlightColor;

    // Unimplemented methods to prevent copying and assignment.
    SelectedItem(const SelectedItem &);
    SelectedItem & operator=(const SelectedItem &);

    void createMoveTimeLine();

    void createPlayTimeLine();

    void moveTo(GridItem *);

    void moveRequested(Direction);

    AnimationState animationState() const;

    bool isAnimating() const;

    void detachAnimation();

    void stopAnimation();

    QSize selectedSize(GridItem *item) const;

    QPoint pos(GridItem *) const;

    QRect geometry(GridItem *) const;

    void drawBackground(QPainter *);

    void draw(QPainter *,const QPixmap &,int x,int y) const;

    void drawAnimated(QPainter *);

    static void blendColor(QImage &img,QColor color);

    qreal xDrift();
    qreal yDrift();

    // Provides the signals/slots mechanism for this object.
    SelectedItemConnector *mConnector;

    // The background image - created on demand.
    QPixmap *background;
    // Filename for the background image.
    QString backgroundFileName;

    // The SelectedItem is larger than the GridItem that it magnifies. The 'margin' attribute
    // is the number of extra pixels added to each border of the current GridItem.
    int margin;

    // The dimensions of this item.
    mutable QSize mSelectedSize;

    // Table of all items. Note that the scene is responsible for deleting the items.
    GridItemTable table;

    // The SelectedItem is positioned over whichever GridItem object is deemed to be current.
    GridItem *currentItem;
    // When SelectedItem is moving from the currentItem to a neighbouring GridItem object,
    // the destination object is stored in this attribute.
    GridItem *destItem;

    // Used to animate the current item. The movie object itself is obtained from currentItem,
    // and SelectedItem has no ownership of it. The movie is connected to connector's
    // animationChanged(), animationFinished() and animationError(...) slots.
    QMovie *movie;

    // When true, the SelectedItem is drawn differently. False by default.
    bool active;
    // Used by keyPressEvent(...) and keyReleaseEvent(...).
    bool pressed;

    // Used to move the SelectedItem from the current GridItem object to a destination object.
    // The amount of time it takes to move an item is determine by the ctor's 'slideTimeInterval'
    // parameter. The timeline is connected to connector's moving(...) and movingStateChanged(...)
    // slots.
    QTimeLine *moveTimeLine;

    // Used to codedly animate SVG items. (If a movie is available, it will be used to do the
    // animation instead.)
    QTimeLine *playTimeLine;

    // Number of milliseconds that it should take for the SelectedItem to move
    // across from the current GridItem object to a neighbour in the grid.
    int slideTimeInterval;

    // Used during coded animations, i.e. in response to a playTimeLine signal.
    // This will be a value in the range [0,100].
    int animationStage;

    bool animationPending;

    // The following values are used during moving - i.e. during moveStep(...) we have the means
    // to figure out what the magnified space between the two pixmaps shoudl be during this step,
    // and we use this to find the SelectedItem positions for the two GridItems, currentItem and
    // destItem. We can then use these values during paint(...).
    int currentX;
    int currentY;
    int destX;
    int destY;

    // These are also used during moving. They give the amount the selected
    // item has to shift (to give the magnified effect) during a move.
    // For example, when moving from Column 0 to Column 1, the image in
    // column 0 will gradually shift leftwards until, by the end of the move,
    // its x position is originalXPosition - xDrift.
    // Meanwhile, the image in column 1 (the destination) will gradually
    // shift to the right; at the start of the move its x position is
    // originalXPosition + xDrift, while at the end of the
    // move its x position is originalXPosition.
    qreal mXDrift;
    qreal mYDrift;
};

#endif
