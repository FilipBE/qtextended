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

#include "qsmoothiconview.h"
#include "smoothimagemover.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QDebug>
#include <QKeyEvent>
#include <QPainterPath>
#include <QPushButton>
#include <math.h>
#include <QTime>
#include <QFont>
#include <QImage>
#include <QtopiaApplication>
#include <gfxpainter.h>
#include <gfxtimeline.h>
#include <qtopianamespace.h>
#include <QExportedBackground>
#include <QFocusEvent>
#include <QTimer>
#include <QSet>
#include <QItemDelegate>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QItemSelectionModel>

Q_GLOBAL_STATIC(QItemDelegate, standardDelegate);

const int rate = 18;
const int SelectionTimeout = 100; // highlight if finger stationary for SelectionTimeout ms.
const qreal nearZero = 0.01f;

class Scrollbar : public QWidget
{
Q_OBJECT
public:
    enum { BarHeight = 12 };
    enum { Margin = 4 };
    enum Orientation { Vertical, Horizontal };

    Scrollbar(QSmoothIconView *parent);

    int barHeight() const { return qMax( int(BarHeight), qMax( leftImage.height(), rightImage.height() ) )+Margin*2; }

    void setOrientation( Orientation orientation ) { m_orientation = orientation; sliderImage = QImage(); };

    void setRange(int);
    void setVisibleRange(int);
    void setPosition(int);

    void setImagesCount(int);

signals:
    void clickedAtPos(int);

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e) { sliderImage = QImage(); sliderBox = QImage(); QWidget::resizeEvent(e); }

    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);

private:
    QImage renderNumber(int);
    int sliderLength();

    QSmoothIconView *parent;
    Orientation m_orientation;
    int m_position;
    int m_range;
    int m_visibleRange;
    int m_imagesCount;
    QImage sliderImage;
    QImage sliderBox;
    QImage leftImage;
    QImage rightImage;
};

Scrollbar::Scrollbar(QSmoothIconView *_parent)
:QWidget(_parent), parent(_parent), m_orientation(Horizontal), m_position(0),m_range(0),m_visibleRange(0),m_imagesCount(-1)
{
}

void Scrollbar::setRange( int range )
{
    if ( m_range != range ) {
        m_range = range;
        sliderImage = QImage();
    }
    update();
}

void Scrollbar::setVisibleRange( int range )
{
    if ( m_visibleRange != range ) {
        m_visibleRange = range;
        sliderImage = QImage();
    }
    update();
}

void Scrollbar::setPosition( int pos )
{
    m_position = pos;
    update();
}

void Scrollbar::setImagesCount( int imagesCount )
{
    if ( m_imagesCount != imagesCount ) {
        m_imagesCount = imagesCount;
        leftImage = renderNumber(0);
        rightImage = renderNumber(imagesCount);
        sliderImage = QImage();
        sliderBox = QImage();
        setFixedHeight( barHeight() );
    }
}

void Scrollbar::mousePressEvent(QMouseEvent *e)
{
    mouseMoveEvent(e);
}

void Scrollbar::mouseMoveEvent(QMouseEvent *e)
{
    int mouseMin;
    int mouseMax;
    int mousePos;

    if ( m_orientation == Horizontal ) {
        mouseMin = leftImage.width()+Margin*2+sliderLength()/2;
        mouseMax = width()-rightImage.width()-Margin*2-sliderLength()/2;
        mouseMax = qMax( mouseMin+1, mouseMax );
        mousePos = e->pos().x();
    } else {
        mouseMin = Margin;
        mouseMax = height()-Margin;
        mousePos = e->pos().y();
    }

    int pos = ( mousePos - mouseMin )*(m_range-m_visibleRange)/( mouseMax-mouseMin );
    pos = qBound(0, pos, m_range-m_visibleRange);

    if ( pos != m_position )
        emit clickedAtPos( pos );

    e->accept();
}

void Scrollbar::mouseReleaseEvent(QMouseEvent *e)
{
    e->accept();
}

QImage Scrollbar::renderNumber( int number )
{
    QImage res( 128, 64, QImage::Format_ARGB32_Premultiplied );
    res.fill(0);
    QRect textRect( 0, 0, res.width(), res.height() );

    {
        QPainter p(&res);
        p.setPen( palette().color(QPalette::Text) );
        p.setRenderHint(QPainter::Antialiasing);
        QFont font( p.font() );
        font.setPointSize( 8 );
        font.setWeight( QFont::Black );
        p.setFont( font );

        p.drawText( textRect, Qt::AlignLeft | Qt::AlignTop, QString::number(number) );
        textRect = p.boundingRect( textRect, Qt::AlignLeft | Qt::AlignTop, QString::number(number) );
    }

    return res.copy( textRect );
}


int Scrollbar::sliderLength()
{
    int w = 0;
    int h = 0;

    if ( m_orientation == Horizontal ) {
        w = width()-2*Margin;
        if (m_range > 0)
            w = w * m_visibleRange / m_range;
        w -= leftImage.width()+Margin;
        w -= rightImage.width()+Margin;
        h = barHeight()-Margin*2;
        w = qMax( w, h );
        return w;
    } else {
        w = BarHeight-Margin*2;
        h = height()-2*Margin;
        if (m_range > 0)
            h = h * m_visibleRange / m_range;
        h = qMax( h, BarHeight*2 );
        return h;
    }
}

void Scrollbar::paintEvent(QPaintEvent *e)
{
    GfxPainter p(this,e);

    if ( !m_range || !m_visibleRange )
        return;

    int barRadius = BarHeight/2;

    if ( sliderBox.isNull() ) {
        int w = 0;
        int h = 0;

        if ( m_orientation == Horizontal ) {
            w = (width()-2*Margin);
            w -= leftImage.width()+Margin;
            w -= rightImage.width()+Margin;
            h = barHeight()-Margin*2;
            barRadius = h/4;
        } else {
            w = BarHeight;
            h = height()-2*Margin;
            h = qMax( h, BarHeight*2 );
        }

        sliderBox = QImage(w, h, QImage::Format_ARGB32_Premultiplied);
        sliderBox.fill(0);
        QPainter cp(&sliderBox);
        QPen pen( palette().color(QPalette::Text) );
        pen.setWidth(3);
        cp.setPen( pen );
        cp.setBrush( QBrush() );
        cp.setRenderHint(QPainter::Antialiasing);
        cp.drawRoundedRect( QRect(1,1,w-2,h-2 ), barRadius, barRadius );
    }

    if ( sliderImage.isNull() ) {
        int w = 0;
        int h = 0;

        if ( m_orientation == Horizontal ) {
            w = sliderLength();
            h = barHeight()-Margin*2;
            barRadius = h/4;
        } else {
            w = BarHeight-Margin*2;
            h = sliderLength();
            barRadius = h/4;
        }

        sliderImage = QImage(w, h, QImage::Format_ARGB32_Premultiplied);
        sliderImage.fill(0);
        QPainter cp(&sliderImage);
        cp.setPen(QColor(38,87,117));
        QLinearGradient gradient( QPointF(0,0),  QPointF(0,sliderImage.height()-4) );
        gradient.setColorAt(0, palette().color(QPalette::Text) );
        gradient.setColorAt(1, palette().color(QPalette::Text).lighter(300) );
        cp.setBrush( gradient );
        cp.setRenderHint(QPainter::Antialiasing);
        cp.drawRoundedRect( QRect(2,2,w-4,h-4 ), barRadius, barRadius );

        QPen linesPen(QColor(48,48,48));
        linesPen.setWidth(1);
        cp.setPen( linesPen );
        cp.drawLine( barRadius+3, 2, barRadius+3, h-4 );
        cp.drawLine( barRadius+6, 2, barRadius+6, h-4 );

        cp.drawLine( w-barRadius-3, 2, w-barRadius-3, h-4 );
        cp.drawLine( w-barRadius-6, 2, w-barRadius-6, h-4 );
    }

    int pos = qBound( 0, m_position, m_range-m_visibleRange );
    int x = 0;
    int y = 0;
    int dx = 0;
    int dy = 0;
    if ( m_orientation == Horizontal ) {
        x = Margin + leftImage.width() + Margin;
        if ( m_range > m_visibleRange )
            dx = ( width() - 4*Margin - leftImage.width() - rightImage.width() - sliderLength() )*pos/(m_range-m_visibleRange);
        y = height() - sliderImage.height() - Margin;
    } else {
        y = Margin;
        dy = (height()-2*Margin);
        if (m_range > 0)
            dy = dy * pos / m_range;
        x = width() - sliderImage.width() - Margin;
    }

    if ( m_orientation == Horizontal ) {
        p.drawImage( Margin, height()-Margin-leftImage.height(), leftImage );
        p.drawImage( width()-Margin-rightImage.width(), height()-Margin-rightImage.height(), rightImage );
    }

    p.drawImage(x,y,sliderBox);
    p.drawImage(x+dx, y+dy, sliderImage);
}


class QSmoothIconViewPrivate
{
public:
    QSmoothIconViewPrivate()
        : model(0),
        delegate(standardDelegate()),
        selectionModel(0),
        needRefreshOnShow(true),
        imageMover(0),
        scrollbar(0),
        rowCount(0),
        selectionMode(QSmoothIconView::SingleSelection),
        flow(QSmoothIconView::TopToBottom),
        fixedRowCount(-1),
        scrollbarEnabled(true),
        textElideMode(Qt::ElideRight),
        lastLayoutedPos(0),
        scrollTimerId(-1),
        spacing(4),
        iconSize(64,64),
        firstVisibleIndex(0), lastVisibleIndex(-1),
        invalidLayoutPos(0)
    {
    }


    QAbstractItemModel *model;
    QAbstractItemDelegate *delegate;
    QStyleOptionViewItemV3 defaultoption;
    QItemSelectionModel *selectionModel;
    QList<ListItem*> items;
    QSet<int> loadedItems;

    bool needRefreshOnShow;

    SmoothImageMover *imageMover;
    Scrollbar *scrollbar;

    int rowCount;
    QColor backColor;
    QSmoothIconView::SelectionMode selectionMode;
    QSmoothIconView::Flow flow;
    int fixedRowCount;
    QString emptyText;
    bool scrollbarEnabled;
    Qt::TextElideMode textElideMode;

    QSize totalSize;
    QRect visibleArea;
    int lastLayoutedPos;

    int scrollTimerId;

    int spacing;
    QSize iconSize;

    QSize itemSizeEstimate;
    int firstVisibleIndex;
    int lastVisibleIndex;

    QPoint mousePressPos;

    int invalidLayoutPos;

    int focusItem() const { return selectionModel ? selectionModel->currentIndex().row() : -1; }
    void setRowCount( int rowCount );

    void layoutItems( int toIndex );
    void loadVisibleItems();
    int indexOfPoint(const QPoint& p);
};

class ListItem
{
public:
    ListItem(QSmoothIconViewPrivate *_d, int _index )
        :d(_d),index(_index),selected(false),row(-1) {
    }

    void setSelected(bool);

    QSmoothIconViewPrivate *d;

    int index;
    bool selected;
    QPoint pos;
    QSize size;
    QImage image;
    int row; //visual row, or column in vertical mode


    QRect rect() const { return QRect(pos,size); }
    QModelIndex modelIndex() const { return d->model->index(index,0); }

    void load( bool &sizeChanged );
    void load();
    void unload() { image = QImage(); }
    bool isLoaded() const { return !image.isNull(); }
};


void ListItem::setSelected( bool _selected )
{
    if ( selected != _selected ) {
        selected = _selected;

        if ( !image.isNull() )
            load();
    }
}


void ListItem::load()
{
    bool sizeChanged;
    load(sizeChanged );
}

void ListItem::load( bool &sizeChanged )
{
    if ( d->delegate ) {
        QStyleOptionViewItemV3 opt = d->defaultoption;

        if ( selected )
            opt.state |= QStyle::State_Selected;

        opt.decorationSize = size;
        QSize newSize = d->delegate->sizeHint(opt, modelIndex());

        if ( newSize.isEmpty() )
            newSize = size;

        if ( size != newSize ) {
            sizeChanged = true;
            size = newSize;
        }

        opt.rect = QRect( QPoint(0,0), size );

        image = QImage( size, QImage::Format_RGB32 );
        image.fill(0);

        QPainter cp(&image);
        d->delegate->paint(&cp, opt, modelIndex());
    } else {
        image = QImage();
    }
}

void QSmoothIconViewPrivate::layoutItems( int toIndex )
{
    if ( toIndex < invalidLayoutPos )
        return;

    lastLayoutedPos = 0;

    if ( rowCount == 0 )
        return;

    if ( flow == QSmoothIconView::TopToBottom ) {
        int rowHeight = items[0]->size.height()+spacing;
        int layoutRows = qMax( 1, totalSize.height()/rowHeight );

        QVector<int> rowMargin(layoutRows);
        int maxRightMargin = 0;
        rowMargin.fill(0);

        int rowsFilled = 0;
        for ( int i=invalidLayoutPos-1; i>=0 && rowsFilled < layoutRows; i-- ) {
            int row = items[i]->row;
            int right = items[i]->rect().right();
            maxRightMargin = qMax( maxRightMargin, right );
            if ( right > rowMargin[row]  ) {
                rowMargin[row] = right;
                rowsFilled++;
            }
        }

        int currentItem = 0;
        int currentRow = 0;

        if ( invalidLayoutPos > 0 ) {
            currentItem = invalidLayoutPos;
            currentRow = items[currentItem-1]->row;
        }

        for ( ; currentItem <= toIndex; currentItem++ ) {
            ListItem *item = items[currentItem];

            //check if we still can fit the image to the current row
            if ( currentItem>0 && rowMargin[currentRow]+spacing+item->size.width() > maxRightMargin )
                currentRow = (currentRow+1) % layoutRows;

            item->row = currentRow;
            item->pos = QPoint( rowMargin[currentRow]+spacing, currentRow*rowHeight+spacing );
            int right = item->rect().right();
            rowMargin[currentRow] = right;
            maxRightMargin = qMax( maxRightMargin, right );
        }

        invalidLayoutPos = toIndex+1;
        lastLayoutedPos = maxRightMargin + spacing;
    } else {
        //left to right flow, but only in one colum yet
        int yPos = 0;
        if ( invalidLayoutPos > 0 )
            yPos = items[invalidLayoutPos-1]->rect().bottom();

        for ( int i = invalidLayoutPos; i<=toIndex; i++ ) {
            items[i]->pos = QPoint( spacing, yPos+spacing );
            yPos = items[i]->rect().bottom();
        }

        invalidLayoutPos = toIndex+1;
        lastLayoutedPos = yPos+spacing;
    }
}

void QSmoothIconViewPrivate::loadVisibleItems()
{
    layoutItems( rowCount-1 );

    firstVisibleIndex = 0;
    lastVisibleIndex = -1;

    //TODO: use binary search
    for ( int i=0; i<rowCount; i++ ) {
        if ( items[i]->rect().intersects(visibleArea) ) {
            firstVisibleIndex = i;
            break;
        }
    }

    bool layoutChanged = false;
    do {
        int invisibleItems = 0;
        for ( int i=firstVisibleIndex; i<rowCount; i++ ) {
            if ( items[i]->rect().intersects(visibleArea) ) {
                lastVisibleIndex = i;
                invisibleItems = 0;
            } else {
                invisibleItems++;
                // look at a few items after the currently visible one,
                // since next image is not always placed after previously one
                if ( invisibleItems > 12 )
                    break;
            }
        }

        layoutChanged = false;
        for ( int i=firstVisibleIndex; i<=lastVisibleIndex; i++ ) {
            ListItem *item = items[i];
            if ( !item->isLoaded() ) {
                bool sizeChanged = false;
                item->load( sizeChanged );
                loadedItems.insert(i);

                if ( sizeChanged ) {
                    layoutChanged = true;
                    invalidLayoutPos = qMin( invalidLayoutPos, i );
                }
            }

        }

        if ( layoutChanged )
            layoutItems( rowCount-1 );
    } while( layoutChanged );


    if ( flow == QSmoothIconView::TopToBottom ) {
        int width = qMax( lastLayoutedPos, visibleArea.width() );
        totalSize = QSize( width, visibleArea.height() );
        int allowedWidth = totalSize.width()-visibleArea.width();
        imageMover->setAllowedRect( QRect( 0, 0, allowedWidth+1, 1 ) );
    } else {
        int height = qMax( lastLayoutedPos, visibleArea.height() );
        totalSize = QSize( visibleArea.width(), height );
        int allowedHeight = totalSize.height()-visibleArea.height();
        imageMover->setAllowedRect( QRect( 0, 0, 1, allowedHeight+1 ) );
    }

    //unload invisible items
    foreach( int i, loadedItems.toList() ) {
        if ( i < firstVisibleIndex-4 || i > lastVisibleIndex + 4 ) {
            loadedItems.remove(i);
            items[i]->unload();
        }
    }
}

void QSmoothIconViewPrivate::setRowCount( int _rowCount )
{
    rowCount = _rowCount;
    scrollbar->setImagesCount( rowCount );
}

int QSmoothIconViewPrivate::indexOfPoint(const QPoint& p)
{
    layoutItems( rowCount-1 );

    for ( int i=0; i<rowCount; i++ ) {
        if ( items[i]->rect().contains( p ) )
            return i;
    }
    return -1;
}

QSmoothIconView::QSmoothIconView(QWidget *parent)
    :QWidget(parent), d(new QSmoothIconViewPrivate)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);

    setFocusPolicy(Qt::TabFocus);

    d->scrollbar = new Scrollbar(this);
    d->scrollbar->setOrientation( Scrollbar::Horizontal );

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addStretch(1);
    layout->addWidget(d->scrollbar);

    setLayout( layout );

    d->imageMover = new SmoothImageMover( this );
    d->imageMover->fixMovements( SmoothImageMover::Vertical );

    connect( d->imageMover, SIGNAL(positionChanged(const QPoint&)), SLOT(scrollViewport(const QPoint&)) );
    connect( d->scrollbar, SIGNAL(clickedAtPos(int)), SLOT(scrollViewport(int)) );
}

QSmoothIconView::~QSmoothIconView()
{
    qDeleteAll( d->items );
    delete d;
}

QAbstractItemModel *QSmoothIconView::model() const
{
    return d->model;
}

/*!
  Sets the \a model for the list to present.

  The current selection index is reset.
*/
void QSmoothIconView::setModel(QAbstractItemModel *model, QItemSelectionModel *selectionModel )
{
    if ( d->model )
        disconnect( d->model, 0, this, 0 );

    d->model = model;

    if(d->model) {
        connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(dataChanged(QModelIndex,QModelIndex)));
        connect(d->model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(rowsInserted(QModelIndex,int,int)));
        connect(d->model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(rowsRemoved(QModelIndex,int,int)));
        connect(d->model, SIGNAL(modelReset()),
                this, SLOT(modelReset()));

        if ( selectionModel )
            setSelectionModel( selectionModel );
        else
            setSelectionModel( new QItemSelectionModel(model,model) );

        d->setRowCount( d->model->rowCount() );
    } else {
        setSelectionModel(0);
        d->setRowCount(0);
    }

    reset();
}

QAbstractItemDelegate * QSmoothIconView::itemDelegate() const
{
    return d->delegate;
}

void QSmoothIconView::setItemDelegate(QAbstractItemDelegate *delegate)
{
    d->delegate = delegate;
    reset();
}

QModelIndex QSmoothIconView::currentIndex() const
{
    return d->selectionModel->currentIndex();
}

QModelIndex QSmoothIconView::indexAt(const QPoint &point)
{
    int idx = d->indexOfPoint( point+d->visibleArea.topLeft() );
    if (idx != -1)
        return d->items.at(idx)->modelIndex();

    return QModelIndex();
}

QSmoothIconView::SelectionMode QSmoothIconView::selectionMode() const
{
    return d->selectionMode;
}

void QSmoothIconView::setSelectionMode( QSmoothIconView::SelectionMode mode )
{
    if ( mode != selectionMode() ) {
        d->selectionMode = mode;
        update();
    }
}

int QSmoothIconView::spacing() const
{
    return d->spacing;
}

void QSmoothIconView::setSpacing(int spacing)
{
    if ( d->spacing != spacing ) {
        d->spacing = spacing;
        reset();
    }
}

QSize QSmoothIconView::iconSize() const
{
    return d->iconSize;
}

void QSmoothIconView::setIconSize( const QSize& size )
{
    if ( size != d->iconSize ) {
        d->iconSize = size;
        reset();
    }
}

void QSmoothIconView::setFixedRowCount( int rowCount )
{
    if ( d->fixedRowCount != rowCount ) {
        d->fixedRowCount = rowCount;
        reset();
    }
}

int QSmoothIconView::fixedRowCount() const
{
    return d->fixedRowCount;
}

QSmoothIconView::Flow QSmoothIconView::flow () const
{
    return d->flow;
}

void QSmoothIconView::setFlow( QSmoothIconView::Flow flow )
{
    if ( d->flow != flow ) {
        d->flow = flow;

        if ( flow == QSmoothIconView::TopToBottom ) {
            d->imageMover->allowMovements( SmoothImageMover::Horizontal );
            d->imageMover->fixMovements( SmoothImageMover::Vertical );
            d->scrollbar->setOrientation( Scrollbar::Horizontal );
        } else {
            d->imageMover->fixMovements( SmoothImageMover::Horizontal );
            d->imageMover->allowMovements( SmoothImageMover::Vertical );
            d->scrollbar->setOrientation( Scrollbar::Vertical );
        }

        reset();
    }
}

void QSmoothIconView::scrollTo(const QModelIndex &index, QSmoothIconView::ScrollHint hint )
{
    if ( index.isValid() && index.row() < d->rowCount ) {
        d->layoutItems( d->rowCount-1 );
        QRect itemRect = d->items[ index.row() ]->rect();
        QRect newVisibleArea = d->visibleArea;
        newVisibleArea.moveCenter( itemRect.center() );

        if( hint == EnsureVisible ) {
            if ( !d->visibleArea.contains( itemRect ) )
                d->imageMover->setPosition( newVisibleArea.topLeft() );
        } else
            d->imageMover->setPosition( newVisibleArea.topLeft() );
    }
}

void QSmoothIconView::setEmptyText(const QString &text)
{
    d->emptyText = text;
    update();
}

void QSmoothIconView::setScrollbarEnabled( bool flag )
{
    if ( d->scrollbarEnabled != flag ) {
        d->scrollbarEnabled = flag;
        reset();
    }
}

Qt::TextElideMode QSmoothIconView::textElideMode() const
{
    return d->textElideMode;
}

void QSmoothIconView::setTextElideMode(Qt::TextElideMode mode)
{
    if (d->textElideMode != mode) {
        d->textElideMode = mode;

        reset();
    }
}

void QSmoothIconView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    if (d->selectionModel)
        disconnect( d->selectionModel, 0, this, 0 );

    d->selectionModel = selectionModel;

    connect( d->selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             this, SLOT(setCurrentIndex(QModelIndex,QModelIndex)) );

    if ( selectionModel->currentIndex().isValid() )
        setCurrentIndex( selectionModel->currentIndex() );
}

QItemSelectionModel* QSmoothIconView::selectionModel()
{
    return d->selectionModel;
}


void QSmoothIconView::setCurrentIndex(const QModelIndex &current, const QModelIndex &previous)
{
    emit currentChanged( current, previous );
    update();
}

void QSmoothIconView::setCurrentIndex(const QModelIndex &index)
{
    if ( d->model ) {
        QModelIndex oldIndex = currentIndex();
        d->selectionModel->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect );
    }
    update();
}



/*! \internal */
void QSmoothIconView::refresh()
{
    d->setRowCount( d->model ? d->model->rowCount() : 0 );

    d->needRefreshOnShow = false;
    qDeleteAll( d->items );
    d->items.clear();
    d->loadedItems.clear();

    for (int i=0; i<d->rowCount; i++)
        d->items.append( new ListItem( d, i ) );

    d->invalidLayoutPos = 0;
    d->firstVisibleIndex = 0;
    d->lastVisibleIndex = -1;
    d->lastLayoutedPos = 0;

    if ( d->rowCount ) {
        bool sizeChanged;

        QSize defaultSize = d->iconSize;

        if ( fixedRowCount() > 0 && flow() == TopToBottom ) {
            int h = 0;
            if ( d->scrollbarEnabled )
                h = ( height() - d->scrollbar->barHeight() )/fixedRowCount() - spacing();
            else
                h = ( height() - spacing() )/fixedRowCount() - spacing();

            defaultSize = QSize(h*3/2,h);
        }

        d->items[0]->size = defaultSize;
        d->items[0]->load(sizeChanged);
        d->itemSizeEstimate = d->items[0]->size;

        if ( d->itemSizeEstimate.isEmpty() )
            d->itemSizeEstimate = QSize(64,64);

        for (int i=0; i<d->rowCount; i++)
            d->items[i]->size = d->itemSizeEstimate;
    }

    d->imageMover->setPosition( QPoint(0,0) );
    ensureCurrentVisible();
    update();
}

/*!
  Reset the internal state of the list.
*/
void QSmoothIconView::reset()
{
    if (isVisible()) {
        refresh();
    } else {
        d->needRefreshOnShow = true;
    }
}


void QSmoothIconView::ensureCurrentVisible()
{
    scrollTo( currentIndex(), EnsureVisible);
}


void QSmoothIconView::updateItem(const QModelIndex &)
{
    update();
}

void QSmoothIconView::paintEvent(QPaintEvent *pe)
{
    d->defaultoption.initFrom(this);
    d->defaultoption.textElideMode = d->textElideMode;

    if (d->items.count() == 0) {
        QPainter painter(this);
        painter.fillRect(rect(), d->backColor);
        painter.drawText(rect(), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextWordWrap, d->emptyText);

        if ( d->scrollbarEnabled ) {
            d->scrollbar->setRange(1);
            d->scrollbar->setVisibleRange(1);
            d->scrollbar->setPosition(0);
        }
        return;
    }

    d->visibleArea.setSize( rect().size() );

    d->loadVisibleItems();

    GfxPainter p(this, pe);
    p.fillRect( rect(), d->backColor );
    int focusItem = d->focusItem();
    for ( int i=d->firstVisibleIndex; i<=d->lastVisibleIndex; i++ ) {
        ListItem *item = d->items[i];
        item->setSelected( selectionMode() == SingleSelection && i == focusItem );
        p.drawImage( item->pos - d->visibleArea.topLeft(), item->image );
    }

    if ( d->scrollbarEnabled ) {
        if ( flow() == TopToBottom ) {
            d->scrollbar->setRange( d->totalSize.width() );
            d->scrollbar->setVisibleRange( d->visibleArea.width() );
            d->scrollbar->setPosition( d->visibleArea.left() );
        } else {
            d->scrollbar->setRange( d->totalSize.height() );
            d->scrollbar->setVisibleRange( d->visibleArea.height() );
            d->scrollbar->setPosition( d->visibleArea.top() );
        }
    }
}

void QSmoothIconView::resizeEvent(QResizeEvent *)
{
    //we have to ensure the current item is visible and stay current after resize
    QModelIndex current = currentIndex();
    reset();
    d->visibleArea.setSize( size() );
    d->loadVisibleItems();
    setCurrentIndex( current );
    ensureCurrentVisible();
}

void QSmoothIconView::showEvent(QShowEvent *e)
{
    if (d->needRefreshOnShow)
        refresh();

    ensureCurrentVisible();

    QWidget::showEvent(e);
}

void QSmoothIconView::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Select) {
        if (d->focusItem() != -1) {
            emit activated( currentIndex() );
            e->accept();
            return;
        }
    }

    if ( selectionMode() != NoSelection ) {
        int currentRow = d->focusItem();
        switch( e->key() ) {
            case Qt::Key_Up:
            case Qt::Key_Left:
                currentRow--;
                e->accept();
                break;
            case Qt::Key_Down:
            case Qt::Key_Right:
                currentRow++;
                e->accept();
                break;
            default:
                QWidget::keyPressEvent(e);
        }

        currentRow = (currentRow+d->rowCount) % d->rowCount;
        if ( currentRow != d->focusItem() ) {
            setCurrentIndex( d->model->index( currentRow, 0 ) );
            ensureCurrentVisible();
        }

        return;
    }

    QWidget::keyPressEvent(e);
}

void QSmoothIconView::mousePressEvent(QMouseEvent *e)
{
    d->mousePressPos = e->pos();

    d->imageMover->setPosition( d->visibleArea.topLeft() );
    d->imageMover->startMoving( -e->pos() );

    e->accept();
}

void QSmoothIconView::mouseMoveEvent(QMouseEvent *e)
{
    d->imageMover->moveTo( -e->pos() );
    e->accept();
}

void QSmoothIconView::mouseReleaseEvent(QMouseEvent *e)
{
    d->imageMover->endMoving( -e->pos() );

    if ( (d->mousePressPos-e->pos()).manhattanLength() < (width()+height())/64 ) {
        QModelIndex index = indexAt( e->pos() );
        if ( index.isValid() ) {
            setCurrentIndex( index );
            emit activated( index );
        }
    }
    e->accept();
}

void QSmoothIconView::mouseDoubleClickEvent(QMouseEvent *e)
{
    QModelIndex index = indexAt( e->pos() );
    if ( index.isValid() ) {
        setCurrentIndex( index );
        emit activated( index );
    }
    e->accept();
}

void QSmoothIconView::wheelEvent(QWheelEvent *e)
{
    QWidget::wheelEvent(e);
}


void QSmoothIconView::timerEvent(QTimerEvent *e)
{
    if ( e->timerId() == d->scrollTimerId ) {
        killTimer( d->scrollTimerId );
        d->scrollTimerId = -1;

        if ( isVisible() ) {
            //set current item to one of visible icons
            d->layoutItems( d->rowCount-1 );
            QRect itemRect;
            if ( d->focusItem() !=-1 )
                itemRect = d->items[ d->focusItem() ]->rect();

            if ( !d->visibleArea.intersects( itemRect ) ) {
                QPoint center = d->visibleArea.center();
                int visibleItem = -1;
                int partiallyVisibleitem = -1;

                for ( int i=0; i<d->rowCount; i++ ) {
                    itemRect = d->items[ i ]->rect();

                    if ( d->visibleArea.contains( itemRect) ) {
                        visibleItem = i;
                        break;
                    } else if ( partiallyVisibleitem == -1 && d->visibleArea.intersects( itemRect) ) {
                        partiallyVisibleitem = i;
                    }
                }

                int currentItem = visibleItem;
                if ( currentItem == -1 )
                    currentItem = partiallyVisibleitem;
                if ( currentItem == -1 )
                    currentItem = 0;

                setCurrentIndex( d->model->index(currentItem,0) );
            }
        }

    }

    QWidget::timerEvent(e);
}


void QSmoothIconView::scrollViewport( const QPoint &pos )
{
    d->visibleArea.moveTo(pos);

    if ( d->rowCount ) {
        d->layoutItems( d->rowCount-1 );

        if ( d->scrollTimerId == -1 )
            d->scrollTimerId = startTimer( 250 );
    }

    update();
}


void QSmoothIconView::scrollViewport( int pos )
{
    d->imageMover->setPosition( QPoint(pos,0) );
}


void QSmoothIconView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if ( !d->model || d->model->rowCount() <= 0 || d->needRefreshOnShow )
        return;

    int start = qMax(topLeft.row(), 0);
    int end = qMin(bottomRight.row(), d->rowCount-1);

    for (int i = start; i <= end; ++i)
        d->items[i]->unload();

    d->invalidLayoutPos = qMin( d->invalidLayoutPos, start );

    // If any of these items was visible, we need to redraw
    if ( d->invalidLayoutPos <= d->lastVisibleIndex )
        update();
}

void QSmoothIconView::rowsInserted(const QModelIndex &, int, int)
{
    reset();
}

void QSmoothIconView::rowsRemoved(const QModelIndex &, int, int)
{
    reset();
}

void QSmoothIconView::modelReset()
{
    reset();
}

#include "qsmoothiconview.moc"

