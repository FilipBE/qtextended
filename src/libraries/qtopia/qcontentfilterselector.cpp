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

#include <qcontentfilterselector.h>
#include "qcontentfilterselector_p.h"
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QPainter>
#include <QApplication>
#include <QLocale>
#include <QtDebug>
#include <QFocusEvent>

class QTreePageProxyModelPrivate
{
public:
    QList< QModelIndex > parents;
    QString rootName;
    QIcon rootIcon;

    QModelIndex rootIndex() const
    {
        return parents.isEmpty() ? QModelIndex() : parents.last();
    }

    int parentCount() const
    {
        return parents.count();
    }

    QModelIndex parent( int row ) const
    {
        return parents[ row  ];
    }

    int parentRow( const QModelIndex &index )
    {
        return parents.indexOf( index );
    }
};

/*!
    \class QTreePageProxyModel
    \inpublicgroup QtBaseModule

    \internal

    Proxy model for navigating a tree model as a flat list.  The proxy model lists the immediate children of an index
    and prepended by any parent indices of that index and the index itself.  Selecting a child index with children
    of its own will make the index the new root parent.  Selecting a parent index will return to the list in which
    the parent index exists.
*/

/*!
    Constructs a tree page model with the parent \a parent.
*/
QTreePageProxyModel::QTreePageProxyModel( QObject *parent )
    : QAbstractProxyModel( parent )
{
    d = new QTreePageProxyModelPrivate;
}

/*!
    Destroys a tree page model.
*/
QTreePageProxyModel::~QTreePageProxyModel()
{
    delete d;
}

/*!
    \reimp
*/
QModelIndex QTreePageProxyModel::mapFromSource( const QModelIndex &index ) const
{
    if( index.isValid() )
    {
        int parentRow = d->parentRow( index );

        return parentRow != -1
                ? createIndex( parentRow, index.column() )
                : createIndex( index.row() + d->parentCount(), index.column() );
    }
    else
        return index;
}

/*!
    \reimp
*/
QModelIndex QTreePageProxyModel::mapToSource( const QModelIndex &index ) const
{
    if( !index.isValid() )
        return index;
    else if( index.row() < d->parentCount() )
        return d->parent( index.row() );
    else
        return sourceModel()->index( index.row() - d->parentCount(), index.column(), d->rootIndex() );
}

/*!
    \reimp
*/
void QTreePageProxyModel::setSourceModel( QAbstractItemModel *model )
{
    QAbstractItemModel *oldModel = sourceModel();

    if( oldModel )
    {
        disconnect( oldModel, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
                    this    , SLOT (_columnsAboutToBeInserted(QModelIndex,int,int)) );
        disconnect( oldModel, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                    this    , SLOT (_columnsAboutToBeRemoved(QModelIndex,int,int)) );
        disconnect( oldModel, SIGNAL(columnsInserted(QModelIndex,int,int)),
                    this    , SLOT (_columnsInserted(QModelIndex,int,int)) );
        disconnect( oldModel, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                    this    , SLOT (_columnsRemoved(QModelIndex,int,int)) );
        disconnect( oldModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                    this    , SLOT (_dataChanged(QModelIndex,QModelIndex)) );
        disconnect( oldModel, SIGNAL(modelReset()),
                    this    , SLOT (_modelReset()) );
        disconnect( oldModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                    this    , SLOT (_rowsAboutToBeInserted(QModelIndex,int,int)) );
        disconnect( oldModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                    this    , SLOT (_rowsAboutToBeRemoved(QModelIndex,int,int)) );
        disconnect( oldModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                    this    , SLOT (_rowsInserted(QModelIndex,int,int)) );
        disconnect( oldModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                    this    , SLOT (_rowsRemoved(QModelIndex,int,int)) );
    }

    d->parents.clear();

    QAbstractProxyModel::setSourceModel( model );

    connect( model, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
             this , SLOT (_columnsAboutToBeInserted(QModelIndex,int,int)) );
    connect( model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
             this , SLOT (_columnsAboutToBeRemoved(QModelIndex,int,int)) );
    connect( model, SIGNAL(columnsInserted(QModelIndex,int,int)),
             this , SLOT (_columnsInserted(QModelIndex,int,int)) );
    connect( model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
             this  , SLOT (_columnsRemoved(QModelIndex,int,int)) );
    connect( model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
             this , SLOT (_dataChanged(QModelIndex,QModelIndex)) );
    connect( model, SIGNAL(modelReset()),
             this , SLOT (_modelReset()) );
    connect( model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
             this , SLOT (_rowsAboutToBeInserted(QModelIndex,int,int)) );
    connect( model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
             this , SLOT (_rowsAboutToBeRemoved(QModelIndex,int,int)) );
    connect( model, SIGNAL(rowsInserted(QModelIndex,int,int)),
             this , SLOT (_rowsInserted(QModelIndex,int,int)) );
    connect( model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
             this , SLOT (_rowsRemoved(QModelIndex,int,int)) );
}

/*!
    \reimp
*/
int QTreePageProxyModel::columnCount( const QModelIndex &parent ) const
{
    if( parent.isValid() )
        return 0;

    return sourceModel()->columnCount( d->rootIndex() );
}

/*!
    \reimp
*/
bool QTreePageProxyModel::hasChildren( const QModelIndex &parent ) const
{
    Q_UNUSED( parent );

    return false;
}

/*!
    \reimp
*/
QModelIndex QTreePageProxyModel::index( int row, int column, const QModelIndex &parent ) const
{
    return !parent.isValid() ? createIndex( row, column ) : QModelIndex();
}

/*!
    \reimp
*/
QModelIndex QTreePageProxyModel::parent( const QModelIndex &index ) const
{
    Q_UNUSED( index );

    return QModelIndex();
}

/*!
    \reimp
*/
int QTreePageProxyModel::rowCount( const QModelIndex &parent ) const
{
    int count = !parent.isValid()
            ? d->parentCount() + sourceModel()->rowCount( d->rootIndex() )
            : 0;

    return count;
}

/*!
    \reimp
*/
QModelIndex QTreePageProxyModel::buddy( const QModelIndex &index ) const
{
    return mapFromSource( sourceModel()->buddy( mapToSource( index ) ) );
}

/*!
    \reimp
*/
bool QTreePageProxyModel::canFetchMore( const QModelIndex &parent ) const
{
    return !parent.isValid()
            ? sourceModel()->canFetchMore( d->rootIndex() )
            : false;
}

/*!
    \reimp
*/
void QTreePageProxyModel::fetchMore( const QModelIndex &parent )
{
    if( !parent.isValid() )
        sourceModel()->fetchMore( d->rootIndex() );
}

/*!
    \reimp
*/
QVariant QTreePageProxyModel::data( const QModelIndex &index, int role ) const
{
    if( role == Qt::UserRole && index.isValid() )
    {
        if( index.row() < d->parentCount() )
            return QTreePageProxyModel::ReturnItem;
        else if( sourceModel()->hasChildren( mapToSource( index ) ) )
            return QTreePageProxyModel::ParentItem;
        else
            return QTreePageProxyModel::SelectionItem;
    }
    else
        return QAbstractProxyModel::data( index, role );
}

/*!
    \reimp
*/
bool QTreePageProxyModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
    return sourceModel()->setData( mapToSource(index), value, role );
}

/*!
    \reimp
*/
QSize QTreePageProxyModel::span( const QModelIndex &index ) const
{
    if( index.isValid() && sourceModel() )
    {
        if( index.row() < d->parentCount() )
            return QSize( sourceModel()->rowCount( d->rootIndex() ), 1 );
        else
            return sourceModel()->span( sourceModel()->index(
                    index.row() - d->parentCount(),
        index.column(),
        d->rootIndex() ) );
    }
    else
        return QAbstractProxyModel::span( index );
}

/*!
    Returns true if the source model index corresponding to \a index has children.
*/
bool QTreePageProxyModel::sourceHasChildren( const QModelIndex &index ) const
{
    return sourceModel()->hasChildren( mapToSource( index ) );
}

/*!
    Returns true if the root level of the tree is being displayed.
*/
bool QTreePageProxyModel::atRoot() const
{
    return d->parentCount() == 0;
}

/*!
    Displays the children of the index \a index.
*/
void QTreePageProxyModel::browseToIndex( const QModelIndex &index )
{
    if( !index.isValid() )
        return;

    QModelIndex sourceIndex = mapToSource( index );

    Q_ASSERT( sourceIndex.isValid() );

    if( !sourceModel()->hasChildren( sourceIndex ) )
        return;

    emit select( index );

    int row = d->parentRow( sourceIndex );

    if( row != -1 )
    {
        int sourceRows = sourceModel()->rowCount( d->rootIndex() );

        bool removeParents = row < d->parentCount() - 1;

        if( removeParents )
            beginRemoveRows(
                    QModelIndex(),
                    row + 1,
                    d->parentCount() - 1 );

        while( d->parentCount() > row )
            d->parents.removeLast();

        if( removeParents )
            endRemoveRows();

        if( sourceRows > 0 )
        {
            beginRemoveRows( QModelIndex(), d->parentCount() + 1, d->parentCount() + sourceRows );
            endRemoveRows();
        }

        if( sourceIndex.row() > 0 )
        {
            beginInsertRows(
                    QModelIndex(),
                    d->parentCount(),
                    d->parentCount() + sourceIndex.row() - 1 );
            endInsertRows();
        }

        sourceRows = sourceModel()->rowCount( sourceIndex.parent() );

        if( sourceIndex.row() < sourceRows  - 1 )
        {
            beginInsertRows(
                    QModelIndex(),
                    d->parentCount() + sourceIndex.row() + 1,
                    d->parentCount() + sourceRows - 1 );
            endInsertRows();
        }

         emit ensureVisible( createIndex( d->parentCount() + sourceIndex.row(), 0 ) );
    }
    else
    {
        int childCount = sourceModel()->rowCount( d->rootIndex() );

        if( sourceIndex.row() > 0 )
        {
            beginRemoveRows(
                    QModelIndex(),
                    d->parentCount(),
                    d->parentCount() + sourceIndex.row() - 1 );
            endRemoveRows();
        }

        d->parents.append( sourceIndex );

        if( sourceIndex.row() < childCount - 1 )
        {
            beginRemoveRows(
                    QModelIndex(),
                    d->parentCount(),
                    d->parentCount() + childCount - sourceIndex.row() - 1 );
            endRemoveRows();
        }

        childCount = sourceModel()->rowCount( sourceIndex );

        if( childCount > 0 )
        {
            beginInsertRows(
                    QModelIndex(),
                    d->parentCount(),
                    d->parentCount() + childCount - 1 );
                    endInsertRows();
        }

         emit ensureVisible( createIndex( d->parentCount() + sourceIndex.row(), 0 ) );
    }
}

/*!
    Displays the list the current root index belongs to.
*/
void QTreePageProxyModel::back()
{
    if( d->parentCount() > 0 )
        browseToIndex( index( d->parentCount() - 1, 0 ) );
}

void QTreePageProxyModel::_columnsAboutToBeInserted( const QModelIndex &parent, int start, int end )
{
    if( parent == d->rootIndex() )
        beginInsertColumns( QModelIndex(), start, end );
}

void QTreePageProxyModel::_columnsAboutToBeRemoved( const QModelIndex &parent, int start, int end )
{
    if( parent == d->rootIndex() )
        beginRemoveColumns( QModelIndex(), start, end );
}

void QTreePageProxyModel::_columnsInserted( const QModelIndex &parent, int start, int end )
{
    Q_UNUSED( start );
    Q_UNUSED( end );

    if( parent == d->rootIndex() )
        endInsertColumns();
}

void QTreePageProxyModel::_columnsRemoved( const QModelIndex &parent, int start, int end )
{
    Q_UNUSED( start );
    Q_UNUSED( end );

    if( parent == d->rootIndex() )
        endRemoveColumns();
}

void QTreePageProxyModel::_dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
{
    if( topLeft.parent() != bottomRight.parent() )
        return;

    if( !topLeft.isValid() && !bottomRight.isValid() )
    {
         dataChanged( topLeft, bottomRight );
    }
    else if( topLeft.parent() == d->rootIndex() )
    {
         dataChanged(
                createIndex( topLeft    .row() + d->parentCount(), topLeft    .column() ),
                createIndex( bottomRight.row() + d->parentCount(), bottomRight.column() ) );
    }
    else
    {
        int row = 0;

        foreach( QModelIndex parent, d->parents )
        {
            if( parent.parent() == topLeft.parent() &&
                parent.row   () >= topLeft.row   () && parent.row   () <= bottomRight.row   () &&
                parent.column() >= topLeft.column() && parent.column() <= bottomRight.column() )
            {
                 dataChanged(
                        createIndex( row, 0 ),
                        createIndex( rowCount() - 1, bottomRight.column() ) );

                return;
            }

            row++;
        }
    }
}

void QTreePageProxyModel::_modelReset()
{
    d->parents.clear();

    reset();
}

void QTreePageProxyModel::_rowsAboutToBeInserted( const QModelIndex &parent, int start, int end )
{
    if( parent == d->rootIndex() )
        beginInsertRows( QModelIndex(), start + d->parentCount(), end + d->parentCount() );
}

void QTreePageProxyModel::_rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end )
{
    if( parent == d->rootIndex() )
        beginRemoveRows( QModelIndex(), start + d->parentCount(), end + d->parentCount() );
    else
    {
        int row;
        QModelIndex index;

        for( row = 1; row < d->parentCount(); row++ )
        {
            index = d->parent( row );

            if( index.parent() == parent && index.row() >= start && index.row() <= end )
                break;
        }

        if( row != d->parentCount() )
        {
            beginRemoveRows( QModelIndex(), row, rowCount() - 1 );
            beginInsertRows( QModelIndex(), row, row + sourceModel()->rowCount( index.parent() ) - 1 );

            while( d->parentCount() > row )
                d->parents.removeLast();
        }
    }
}

void QTreePageProxyModel::_rowsInserted( const QModelIndex &parent, int start, int end )
{
    Q_UNUSED( start );
    Q_UNUSED( end );

    if( parent == d->rootIndex() )
        endInsertRows();
}

void QTreePageProxyModel::_rowsRemoved( const QModelIndex &parent, int start, int end )
{
    if( parent == d->rootIndex() )
        endRemoveRows();
    else
    {
        foreach( QModelIndex index, d->parents )
        {
            if( index.parent() == parent.parent() && index.row() >= start && index.row() <= end )
            {
                endRemoveRows();
                endInsertRows();

                return;
            }
        }
    }
}

static inline QIcon::Mode iconMode(QStyle::State state)
{
    if (!(state & QStyle::State_Enabled)) return QIcon::Disabled;
    if (state & QStyle::State_Selected) return QIcon::Selected;
    return QIcon::Normal;
}

static inline QIcon::State iconState(QStyle::State state)
{ return state & QStyle::State_Open ? QIcon::On : QIcon::Off; }

static QRect textLayoutBounds(const QStyleOptionViewItemV2 &option)
{
    QRect rect = option.rect;
    const bool wrapText = option.features & QStyleOptionViewItemV2::WrapText;
    switch (option.decorationPosition) {
    case QStyleOptionViewItem::Left:
    case QStyleOptionViewItem::Right:
        rect.setWidth(INT_MAX >> 6);
        break;
    case QStyleOptionViewItem::Top:
    case QStyleOptionViewItem::Bottom:
        rect.setWidth(wrapText ? option.decorationSize.width() : (INT_MAX >> 6));
        break;
    }

    return rect;
}


/*!
    \class QTreePageItemDelegate
    \inpublicgroup QtBaseModule

    \internal

    Item delegate which expands on the basic QItemDelegate by displaying arrows indicating if an item
    in the list has children, or is a parent item of the currently displayed list.
*/


/*!
    Constructs a new tree page item delegate with the given \a parent.
*/
QTreePageItemDelegate::QTreePageItemDelegate( QObject *parent )
    : QtopiaItemDelegate( parent )
{
}

/*!
    Destroys a tree page item delegate.
*/
QTreePageItemDelegate::~QTreePageItemDelegate()
{
}


QRect QTreePageItemDelegate::arrow( const QStyleOptionViewItem &option, const QRect &bounding, const QVariant &value ) const
{
    if( value.isValid() && value.toInt() != QTreePageProxyModel::SelectionItem )
    {
        QStyleOptionViewItem opt;
        opt.QStyleOption::operator=(option);
        opt.rect = bounding;
        return QRect( 0,
                      0,
                      QApplication::style()->pixelMetric(QStyle::PM_IndicatorWidth, &opt ),
                      QApplication::style()->pixelMetric(QStyle::PM_IndicatorHeight, &opt ) );
    }
    return QRect();
}

void QTreePageItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    Q_ASSERT(index.isValid());
    QStyleOptionViewItemV2 opt = setOptions(index, option);
    const QStyleOptionViewItemV2 *v2 = qstyleoption_cast<const QStyleOptionViewItemV2 *>(&option);
    opt.features = v2 ? v2->features : QStyleOptionViewItemV2::ViewItemFeatures(QStyleOptionViewItemV2::None);

    // prepare
    painter->save();
    //if (d->clipPainting)
    painter->setClipRect(opt.rect);

    // get the data and the rectangles

    QVariant value;

    QIcon icon;
    QIcon::Mode iconMode = ::iconMode(option.state);
    QIcon::State iconState = ::iconState(option.state);

    QPixmap pixmap;
    QRect decorationRect;
    value = index.data(Qt::DecorationRole);
    if (value.isValid()) {
        if (value.type() == QVariant::Icon) {
            icon = qvariant_cast<QIcon>(value);
            decorationRect = QRect(QPoint(0, 0),
                                   icon.actualSize(option.decorationSize, iconMode, iconState));
        } else {
            pixmap = decoration(opt, value);
            decorationRect = QRect(QPoint(0, 0), option.decorationSize).intersected(pixmap.rect());
        }
    }

    QString text;
    QRect displayRect;
    value = index.data(Qt::DisplayRole);
    if (value.isValid()) {
        if (value.type() == QVariant::Double)
            text = QLocale().toString(value.toDouble());
        else
            text = value.toString();
        displayRect = textRectangle(painter, ::textLayoutBounds(opt), opt.font, text);
    }

    QRect checkRect;
    Qt::CheckState checkState = Qt::Unchecked;
    value = index.data(Qt::CheckStateRole);
    if (value.isValid()) {
        checkState = static_cast<Qt::CheckState>(value.toInt());
        checkRect = check(opt, opt.rect, value);
    }

    QRect arrowRect;
    QTreePageProxyModel::ItemType itemType = QTreePageProxyModel::SelectionItem;
    value = index.data(Qt::UserRole);
    if (value.isValid()) {
        itemType = static_cast< QTreePageProxyModel::ItemType >( value.toInt() );
        arrowRect = arrow( opt, opt.rect, value );
    }
    // do the layout

    doLayout(opt, &checkRect, &decorationRect, &displayRect, &arrowRect, false);

    // draw the item

    drawBackground(painter, opt, index);
    drawCheck(painter, opt, checkRect, checkState);
    drawArrow(painter, opt, arrowRect, itemType);
    if (!icon.isNull())
        icon.paint(painter, decorationRect, option.decorationAlignment, iconMode, iconState);
    else
        drawDecoration(painter, opt, decorationRect, pixmap);

    drawDisplay(painter, opt, displayRect, text);
    drawFocus(painter, opt, text.isEmpty() ? QRect() : displayRect);

    // done
    painter->restore();
}

QSize QTreePageItemDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    QVariant value = index.data(Qt::SizeHintRole);
    if (value.isValid())
        return qvariant_cast<QSize>(value);
    QRect decorationRect = rect(option, index, Qt::DecorationRole);
    QRect displayRect = rect(option, index, Qt::DisplayRole);
    QRect checkRect = rect(option, index, Qt::CheckStateRole);
    QRect arrowRect = arrow(option, option.rect, index.data( Qt::UserRole ) );

    doLayout(option, &checkRect, &decorationRect, &displayRect, &arrowRect, true);

    return (decorationRect|displayRect|checkRect|arrowRect).size();
}

/*!
    \internal
 */

void QTreePageItemDelegate::doLayout(const QStyleOptionViewItem &option,
                                     QRect *checkRect, QRect *pixmapRect, QRect *textRect, QRect *arrowRect,
                                     bool hint) const
{
    Q_ASSERT(checkRect && pixmapRect && textRect && arrowRect);
    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    int x = option.rect.left();
    int y = option.rect.top();
    int w, h;

    textRect->adjust(-textMargin, 0, textMargin, 0); // add width padding
    if (textRect->height() == 0)
        textRect->setHeight(option.fontMetrics.lineSpacing());

    QSize pm(0, 0);
    if (pixmapRect->isValid()) {
        pm = pixmapRect->size();
        pm.rwidth() += 2 * textMargin;
    }
    if (hint) {
        h = qMax(checkRect->height(), qMax(textRect->height(), pm.height()));
        if (option.decorationPosition == QStyleOptionViewItem::Left
            || option.decorationPosition == QStyleOptionViewItem::Right) {
            w = textRect->width() + pm.width();
            } else {
                w = qMax(textRect->width(), pm.width());
            }
    } else {
        w = option.rect.width();
        h = option.rect.height();
    }

    int aw = 0;
    QRect arrow;
    if (arrowRect->isValid()) {
        aw = arrowRect->width() + 2 * textMargin;
        if (hint) w += aw;
        if (option.direction == Qt::RightToLeft) {
            arrow.setRect(x, y, aw, h);
        } else {
            arrow.setRect(x + w - aw, y, aw, h);
        }
    }

    int cw = 0;
    QRect check;
    if (checkRect->isValid()) {
        cw = checkRect->width() + 2 * textMargin;
        if (hint) w += cw;
        if (option.direction == Qt::RightToLeft) {
            check.setRect(x + w - cw, y, cw, h);
        } else {
            check.setRect(x, y, cw, h);
        }
    }

    // at this point w should be the *total* width

    QRect display;
    QRect decoration;
    switch (option.decorationPosition) {
        case QStyleOptionViewItem::Left: {
            if (option.direction == Qt::LeftToRight) {
                decoration.setRect(x + cw, y, pm.width(), h);
                display.setRect(decoration.right() + 1, y, w - pm.width() - aw - cw, h);
            } else {
                display.setRect(x + aw, y, w - pm.width() - aw - cw, h);
                decoration.setRect(display.right() + 1, y, pm.width(), h);
            }
            break; }
            case QStyleOptionViewItem::Right: {
                if (option.direction == Qt::LeftToRight) {
                    display.setRect(x + cw, y, w - pm.width() - aw - cw, h);
                    decoration.setRect(display.right() + 1, y, pm.width(), h);
                } else {
                    decoration.setRect(x + aw, y, pm.width(), h);
                    display.setRect(decoration.right() + 1, y, w - pm.width() - aw - cw, h);
                }
                break; }
        default:
            qWarning("doLayout: decoration position is invalid");
            decoration = *pixmapRect;
            break;
    }

    if (!hint) { // we only need to do the internal layout if we are going to paint
        *checkRect = QStyle::alignedRect(option.direction, Qt::AlignCenter, checkRect->size(), check);
        *arrowRect = QStyle::alignedRect(option.direction, Qt::AlignCenter, arrowRect->size(), arrow);
        *pixmapRect = QStyle::alignedRect(option.direction, option.decorationAlignment, pixmapRect->size(), decoration);
        // the text takes up all awailable space, unless the decoration is not shown as selected
        if (option.showDecorationSelected)
            *textRect = display;
        else
            *textRect = QStyle::alignedRect(option.direction, option.displayAlignment,
                                            textRect->size().boundedTo(display.size()), display);
    } else {
        *checkRect = check;
        *arrowRect = arrow;
        *pixmapRect = decoration;
        *textRect = display;
    }
}

void QTreePageItemDelegate::drawArrow(QPainter *painter,
                                      const QStyleOptionViewItem &option,
                                      const QRect &rect, QTreePageProxyModel::ItemType itemType ) const
{
    if (!rect.isValid())
        return;

    QStyleOptionViewItem opt(option);
    opt.rect = rect;

    switch (itemType) {
    case QTreePageProxyModel::ReturnItem:
        QApplication::style()->drawPrimitive(QStyle::PE_IndicatorArrowUp, &opt, painter);
        break;
    case QTreePageProxyModel::ParentItem:
        QApplication::style()->drawPrimitive(
                option.direction == Qt::LeftToRight
                        ? QStyle::PE_IndicatorArrowRight
                        : QStyle::PE_IndicatorArrowLeft
                , &opt
                , painter);
        break;
    case QTreePageProxyModel::SelectionItem:
        break;
    }
}


/*!
    \class QContentFilterView
    \inpublicgroup QtBaseModule

    \internal

    Provides a view for navigating QContentFilterModels.  Selecting items in the model will
    display a list of their children.  If an item has no children selecting it will cause
    the filterSelected signal to be ted.
*/

/*!
    Creates a new QCotentFilterView with the given \a parent.
*/
QContentFilterView::QContentFilterView( QWidget *parent )
    : QListView( parent )
{
    setFocusPolicy( Qt::StrongFocus );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setFrameStyle(QFrame::NoFrame);

    m_proxyModel = new QTreePageProxyModel( this );

    setItemDelegate( new QTreePageItemDelegate( this ) );

    connect( this, SIGNAL(activated(QModelIndex)), this, SLOT(indexSelected(QModelIndex)) );
    connect( m_proxyModel, SIGNAL(select(QModelIndex)), this, SLOT(setCurrentIndex(QModelIndex)) );
    connect( m_proxyModel, SIGNAL(ensureVisible(QModelIndex)), this, SLOT(ensureVisible(QModelIndex)) );
}

/*!
    Destroys a QContentFilterView.
*/
QContentFilterView::~QContentFilterView()
{
}

QContentFilterModel *QContentFilterView::filterModel() const
{
    return m_filterModel;
}

void QContentFilterView::setFilterModel( QContentFilterModel *model )
{
    m_filterModel = model;

    m_proxyModel->setSourceModel( model );

    QListView::setModel( m_proxyModel );
}

/*!
    Returns a filter combining all the checked filters.
*/
QContentFilter QContentFilterView::checkedFilter( const QModelIndex &parent ) const
{
    return m_filterModel ? m_filterModel->checkedFilter( m_proxyModel->mapToSource( parent ) ) : QContentFilter();
}

/*!
    Returns a short summary describing the checked filters, if a single filter is checked the display text for that label
    is returned concatenated with a list of it's parent filter's display text.  If multiple filters are checked then the
    text 'Multi' is returned concatenated to a list of parents common to all checked filters.
 */
QString QContentFilterView::checkedLabel() const
{
    QStringList labels;

    QModelIndex parent;

    while( true )
    {
        QModelIndex firstChecked;
        bool partialCheck = false;

        for( int i = 0; i < m_filterModel->rowCount( parent ); i++ )
        {
            QModelIndex index = m_filterModel->index( i, 0, parent );

            QVariant value = index.data( Qt::CheckStateRole );

            if( value.isValid() && value.toInt() != Qt::Unchecked )
            {
                if( firstChecked.isValid() )
                {
                    labels.append( tr( "(Multi)..." ) );

                    return labels.join( "/" );
                }
                else
                {
                    firstChecked = index;

                    partialCheck = value.toInt() == Qt::PartiallyChecked;
                }
            }
        }

        if( firstChecked.isValid() )
        {
            labels.append( firstChecked.data( Qt::DisplayRole ).toString() );

            if( partialCheck )
            {
                parent = firstChecked;

                continue;
            }
        }

        return labels.join( "/" );
    }
}

/*!
    Responds to the  activated signal to delve further into the model or
    the filterSelected signal depending on the selected index.
*/
void QContentFilterView::indexSelected( const QModelIndex &index )
{
    if( m_filterModel )
    {
        if( m_proxyModel->sourceHasChildren( index ) )
        {
            m_proxyModel->browseToIndex( index );
        }
        else
        {
            QContentFilter filter = m_filterModel->filter( m_proxyModel->mapToSource( index ) );

            if( !(index.flags() & Qt::ItemIsUserCheckable) && filter.isValid() )
                 filterSelected( filter );
        }
    }
}


void QContentFilterView::ensureVisible( const QModelIndex &index )
{
    scrollTo( index );
}

/*!
    \reimp
    Respond to the back key to back a level on the selected filter.
*/
void QContentFilterView::keyPressEvent( QKeyEvent *event )
{
    if( event->key() == Qt::Key_Select && (currentIndex().flags() & Qt::ItemIsUserCheckable) )
    {
        edit( currentIndex(), AnyKeyPressed, event );

        event->accept();
    }
    else if( event->key() == Qt::Key_Back && !m_proxyModel->atRoot() )
    {
        m_proxyModel->back();

        event->accept();
    }
    else if( event->key() == Qt::Key_Right )
    {
        if( QApplication::layoutDirection() == Qt::LeftToRight )
            m_proxyModel->browseToIndex( currentIndex() );
        else
            m_proxyModel->back();

        event->accept();
    }
    else if( event->key() == Qt::Key_Left )
    {
        if( QApplication::layoutDirection() == Qt::LeftToRight )
            m_proxyModel->back();
        else
            m_proxyModel->browseToIndex( currentIndex() );

        event->accept();
    }
    else
        QListView::keyPressEvent( event );
}

void QContentFilterView::focusInEvent( QFocusEvent *event )
{
    QListView::focusInEvent( event );

    setCurrentIndex( currentIndex() );
}

/*!
    \fn QContentFilterView::filterSelected( const QContentFilter &filter )

    Signals that item with the given \a filter has been selected.
*/

class QContentFilterSelectorPrivate : public QContentFilterView
{
public:
    QContentFilterSelectorPrivate( QWidget *parent )
    : QContentFilterView( parent )
    {
    }

    virtual ~QContentFilterSelectorPrivate()
    {
    }
};

/*!
    \class QContentFilterSelector
    \inpublicgroup QtBaseModule

    \brief The QContentFilterSelector widget allows the user to select filters to refine the visible selection of content on a device.

    QContentFilterDialog is based on \l QContentFilterModel and its contents are defined using a \l QContentFilterModel::Template.

    A content filter selector may be either single or multiple selection depending on its set model template.  In a single
    selection the \l filterSelected() signal is emitted when a filter is selected.  In a multiple selection scenario the
    \l checkedFilter() method will return the checked filters and brief summary of the checked filter's labels may also be
    retrieved using the \l checkedLabel() method.

    Constructing a content filter selector to select a an album filter from a list grouped by artist.
    \code
    QContentFilterModel::Template albums( QContent::Album );
    QContentFilterModel::Template artists( album, QContent::Artist );

    QContentFilterSelector *selector = new QContentFilterSelector( this );

    selector->setModelTemplate( artists );

    connect( selector, SIGNAL(filterSelected(QContentFilter)), this, SLOT(filterSelected(QContentFilter)) );
    \endcode

    \sa QContentFilterModel, QContentFilterDialog

  \ingroup content
*/

/*!
    Constructs a filter selector with the parent \a parent.
 */
    QContentFilterSelector::QContentFilterSelector( QWidget *parent )
    : QWidget( parent )
{
    init();
}

/*!
    Constructs a filter selector widget with the parent \a parent and the template \a modelTemplate.
 */
QContentFilterSelector::QContentFilterSelector( const QContentFilterModel::Template &modelTemplate, QWidget *parent )
    : QWidget( parent )
{
    init();

    d->filterModel()->setModelTemplate( modelTemplate );
}

/*!
    Constructs a filter selector widget parented to \a parent that lists the possible values of \a property
    according to the filter model template options \a options.

    The properties listed in the \a checked list will be checked by default.
 */
QContentFilterSelector::QContentFilterSelector( QContent::Property property, QContentFilterModel::TemplateOptions options, const QStringList &checked, QWidget *parent )
    : QWidget( parent )
{
    init();

    d->filterModel()->setModelTemplate( QContentFilterModel::Template( property, options, checked ) );
}

/*!
    Constructs a filter selector widget parented to \a parent that lists filters which are of type \a type and within the given
    \a scope according to the filter model template options \a options.

    Any filters with an argument that matches one in the \a checked list will be checked by default.
 */
QContentFilterSelector::QContentFilterSelector( QContentFilter::FilterType type, const QString &scope, QContentFilterModel::TemplateOptions options, const QStringList &checked,  QWidget *parent )
    : QWidget( parent )
{
    init();

    d->filterModel()->setModelTemplate( QContentFilterModel::Template( type, scope, options, checked ) );
}

void QContentFilterSelector::init()
{
    d = new QContentFilterSelectorPrivate( this );

    d->setFilterModel( new QContentFilterModel( this ) );

    QVBoxLayout *layout = new QVBoxLayout( this );

    layout->setMargin( 0 );
    layout->setSpacing( 0 );

    layout->addWidget( d );

    connect( d, SIGNAL(filterSelected(QContentFilter)), this, SIGNAL(filterSelected(QContentFilter)) );

    setFocusProxy( d );
}

/*!
    \internal
 */
QContentFilterSelector::~QContentFilterSelector()
{
}

/*!
    Returns the base filter.
 */
QContentFilter QContentFilterSelector::filter()
{
    return d->filterModel()->baseFilter();
}

/*!
    Sets the base \a filter.
 */
void QContentFilterSelector::setFilter( const QContentFilter &filter )
{
    d->filterModel()->setBaseFilter( filter );
}

/*!
    Returns the model template.
 */
QContentFilterModel::Template QContentFilterSelector::modelTemplate() const
{
    return d->filterModel()->modelTemplate();
}

/*!
    Sets the model template to \a modelTemplate.
 */
void QContentFilterSelector::setModelTemplate( const QContentFilterModel::Template &modelTemplate )
{
    d->filterModel()->setModelTemplate( modelTemplate );
}

/*!
    Sets the model template to a list of possible values of \a property according to the given \a options.

    The properties listed in the \a checked list will be checked by default.
 */
void QContentFilterSelector::setModelTemplate( QContent::Property property, QContentFilterModel::TemplateOptions options, const QStringList &checked )
{
    d->filterModel()->setModelTemplate( QContentFilterModel::Template( property, options, checked ) );
}

/*!
    Set the model template to a list of filters of type \a type with the specialization \a scope
    according to the given \a options.

    Any filters with an argument that matches one in the \a checked list will be checked by default.
 */
void QContentFilterSelector::setModelTemplate( QContentFilter::FilterType type, const QString &scope, QContentFilterModel::TemplateOptions options, const QStringList &checked )
{
    d->filterModel()->setModelTemplate( QContentFilterModel::Template( type, scope, options, checked ) );
}

/*!
    Returns a filter combining all filters checked by the user.
 */
QContentFilter QContentFilterSelector::checkedFilter() const
{
    return d->checkedFilter();
}

/*!
    Returns a short summary describing the checked filters, if a single filter is checked the display text for that label
    is returned concatenated with a list of it's parent filter's display text.  If multiple filters are checked then the
    text 'Multi' is returned concatenated to a list of parents common to all checked filters.
 */
QString QContentFilterSelector::checkedLabel() const
{
    return d->checkedLabel();
}

/*!
    \fn QContentFilterSelector::filterSelected( const QContentFilter &filter )

    Signals that a \a filter has been selected.

    This signal is only emitted for filters that are not part of a checklist and have no children.
 */


class QContentFilterDialogPrivate : public QContentFilterView
{
public:
    QContentFilterDialogPrivate( QWidget *parent )
        : QContentFilterView( parent )
    {
    }

    virtual ~QContentFilterDialogPrivate()
    {
    }

    QContentFilter selectedFilter;
};

/*!
    \class QContentFilterDialog
    \inpublicgroup QtBaseModule

    \brief The QContentFilterDialog widget provides a dialog box for selecting content filters.

    QContentFilterDialog is based on \l QContentFilterModel and its contents are defined using a \l QContentFilterModel::Template.

    A content filter dialog may be either single or multiple selection depending on its set model template.  The filter
    selected in a single selection scenario may be retrieved using the \l selectedFilter() method, and the \l checkedFilter()
    method will return the checked filters in a multiple selection scenario.  A brief summary of the checked filter's labels
    may also be retrieved using the \l checkedLabel() method.

    Using a content filter dialog to select an or'ed combination of mime type filters:
    \code
    QContentFilter selectMimeTypes()
    {
        QContentFilterDialog dialog(
            QContentFilter::MimeType, QString(), QContentFilterModel::CheckList | QContentFilterModel::SelectAll );

        dialog.setFilter( QContentFilter( QContent::Document ) );

        QtopiaApplication::execDialog( &dialog );

        return dialog.checkedFilters();
    }
    \endcode

    Using a content filter dialog to select a an album filter from a list grouped by artist.
    \code
    QContentFilter selectAlbum()
    {
        QContentFilterModel::Template albums( QContent::Album );
        QContentFilterModel::Template artists( album, QContent::Artist );

        QContentFilterDialog dialog;

        dialog.setModelTemplate( artists );

        if( QtopiaApplication::execDialog( &dialog ) == QDialog::Accept )
            return dialog.selectedFilter();
        else
            return QContentFilter();
    }
    \endcode

    \sa QContentFilterModel, QContentFilterSelector

  \ingroup content
 */

/*!
    Constructs a content filter dialog with the parent \a parent.
 */
QContentFilterDialog::QContentFilterDialog( QWidget *parent )
    : QDialog( parent )
{
    init();
}

/*!
    Constructs a content filter selector dialog with the parent \a parent and the initial template \a modelTemplate.
 */
QContentFilterDialog::QContentFilterDialog( const QContentFilterModel::Template &modelTemplate, QWidget *parent )
    : QDialog( parent )
{
    init();

    d->filterModel()->setModelTemplate( modelTemplate );
}

/*!
    Constructs a content filter dialog parented to \a parent that lists the possible values of \a property
    according to the filter model template options \a options.

    The properties listed in the \a checked list will be checked by default.
 */
QContentFilterDialog::QContentFilterDialog( QContent::Property property, QContentFilterModel::TemplateOptions options, const QStringList &checked, QWidget *parent )
    : QDialog( parent )
{
    init();

    d->filterModel()->setModelTemplate( QContentFilterModel::Template( property, options, checked ) );
}

/*!
    Constructs a content filter dialog parented to \a parent that lists filters which are of type \a type and within the given
    \a scope according to the filter model template options \a options.

    Any filters with an argument that matches one in the \a checked list will be checked by default.
 */
QContentFilterDialog::QContentFilterDialog( QContentFilter::FilterType type, const QString &scope, QContentFilterModel::TemplateOptions options, const QStringList &checked,  QWidget *parent )
    : QDialog( parent )
{
    init();

    d->filterModel()->setModelTemplate( QContentFilterModel::Template( type, scope, options, checked ) );
}

/*!
    Initializes the dialog layout.
*/
void QContentFilterDialog::init()
{
    d = new QContentFilterDialogPrivate( this );

    d->setFilterModel( new QContentFilterModel( this ) );

    QVBoxLayout *layout = new QVBoxLayout( this );

    layout->setMargin( 0 );
    layout->setSpacing( 0 );

    layout->addWidget( d );

    connect( d, SIGNAL(filterSelected(QContentFilter)), this, SLOT(filterSelected(QContentFilter)) );

    setFocusProxy( d );
}

/*!
    \internal
 */
QContentFilterDialog::~QContentFilterDialog()
{
}

/*!
    Returns the base filter.
 */
QContentFilter QContentFilterDialog::filter()
{
    return d->filterModel()->baseFilter();
}

/*!
    Sets the base \a filter.
 */
void QContentFilterDialog::setFilter( const QContentFilter &filter )
{
    d->filterModel()->setBaseFilter( filter );
}

/*!
    Returns the current model template
 */
QContentFilterModel::Template QContentFilterDialog::modelTemplate() const
{
    return d->filterModel()->modelTemplate();
}

/*!
    Sets the model template to \a modelTemplate.
 */
void QContentFilterDialog::setModelTemplate( const QContentFilterModel::Template &modelTemplate )
{
    d->filterModel()->setModelTemplate( modelTemplate );
}

/*!
    Sets the model template to a list of possible values of \a property according to the given \a options.

    The properties listed in the \a checked list will be checked by default.
 */
void QContentFilterDialog::setModelTemplate( QContent::Property property, QContentFilterModel::TemplateOptions options, const QStringList &checked )
{
    d->filterModel()->setModelTemplate( QContentFilterModel::Template( property, options, checked ) );
}

/*!
    Set the model template to a list of filters of type \a type with the specialization \a scope
    according to the given \a options.

    Any filters with an argument that matches one in the \a checked list will be checked by default.
 */
void QContentFilterDialog::setModelTemplate( QContentFilter::FilterType type, const QString &scope, QContentFilterModel::TemplateOptions options, const QStringList &checked )
{
    d->filterModel()->setModelTemplate( QContentFilterModel::Template( type, scope, options, checked ) );
}

/*!
    Returns the filter selected by the user.
 */
QContentFilter QContentFilterDialog::selectedFilter() const
{
    return d->selectedFilter;
}

/*!
    Returns a filter combining all filters checked by the user.
 */
QContentFilter QContentFilterDialog::checkedFilter() const
{
    return d->checkedFilter();
}

/*!
    Returns a short summary describing the checked filters, if a single filter is checked the display text for that label
    is returned concatenated with a list of it's parent filter's display text.  If multiple filters are checked then the
    text 'Multi' is returned concatenated to a list of parents common to all checked filters.
*/
QString QContentFilterDialog::checkedLabel() const
{
    return d->checkedLabel();
}

/*!
    Sets the selected filter to \a filter and closes the dialog.
*/
void QContentFilterDialog::filterSelected( const QContentFilter &filter )
{
    d->selectedFilter = filter;

    accept();
}
