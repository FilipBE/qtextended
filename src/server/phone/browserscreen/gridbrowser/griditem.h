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

#ifndef GRIDITEM_H
#define GRIDITEM_H

#include <QGraphicsRectItem>
#include <QPixmap>

class QGraphicsScene;
class QMovie;
class QPainter;
class QStyleOptionGraphicsItem;
class QContent;
class Animator;
class Renderer;

class GridItem : public QGraphicsRectItem
{
public:

    // Percentage of basic image size that needs to be added to retrieve size of
    // the selected (magnified) image.
    static const qreal SELECTED_IMAGE_SIZE_FACTOR;

    GridItem(QContent *content,int r,int c,
             int totalRows,int totalCols,
             Animator *animator,Animator *animatorBackground,
             bool loadMovie,
             QGraphicsScene *scene = 0);

    ~GridItem();

    void paint(QPainter *painter,const QStyleOptionGraphicsItem *option,QWidget *widget);

    QContent *content() const { return mContent; }

    void setContent(QContent *);

    int row() const { return mRow; }

    int column() const { return mCol; }

    const QPixmap &basicPic() const { return basicPixmap; }

    const QPixmap &selectedPic() const;

    Renderer *renderer();

    QRectF selectedRenderingBounds();

    QMovie *movie() const { return mSelectedMovie; }

    Animator *selectedAnimator() const { return mSelectedAnimator; }

    Animator *selectedBackgroundAnimator() const { return mSelectedBackgroundAnimator; }

    void updateImages();

    int selectedImageSize() const;

    int basicImageSize() const;

    void paletteChanged();

private:

    // Number of pixels which are 'chopped off' the lines which are drawn to frame the basic image.
    static const int IMAGE_LINE_OFFSET = 5;

    // Information for the movie file.
    static const QString MOVIE_PREFIX;
    static const QString MOVIE_SUFFIX;

    // Line colour used when drawing GridItem objects.
    static const QColor DEFAULT_PEN_COLOR;
    // Line style used when drawing GridItem objects.
    static const Qt::PenStyle DEFAULT_PEN_STYLE;

    // Unimplemented methods to prevent copying and assignment.
    GridItem(const GridItem &);
    GridItem & operator=(const GridItem &);

    // Creates on demand the bounding infomration for the SVG renderer.
    QRectF renderingBounds();

    // Contains information used to create the images/movie for this item.
    QContent *mContent;

    // This item's row index.
    int mRow;
    // This item's column index;
    int mCol;
    // The total number of rows in the grid in which this object belongs.
    int totalRows;
    // The total number of columns in the grid in which this object belongs.
    int totalCols;

    // Calculated in selectedImageSize()
    int mSelectedImageSize;

    Renderer *mRenderer;
    QRectF mRenderingBounds;

    // Pixmap used to draw this item.
    QPixmap basicPixmap;
    // Pixmap used to draw SelectedItem when it is positioned over this GridItem object.
    QPixmap mSelectedPixmap;

    // Movie object used to animate the SelectedItem when it is positoned over this GridItem object.
    QMovie *mSelectedMovie;
    // Animator object used to codedly animate the SelectedItem when it is positioned over this GridItem
    // object, if selectedMovie is not available.
    Animator *mSelectedAnimator;
    // When selectedAnimator is running, it may or may not have an animated background to go
    // with it.
    Animator *mSelectedBackgroundAnimator;
    // Determines whether or not this object should search for a movie file for animation.
    bool loadMovie;
};

#endif
