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

#include "qsmoothlist.h"
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
#include <QImage>
#include <QtopiaApplication>
#include <gfxpainter.h>
#include <gfxtimeline.h>
#include <qtopianamespace.h>
#include <QExportedBackground>
#include <QFocusEvent>
#include <QTimer>
#include <QItemDelegate>
#include <QDesktopWidget>


// Define the following to suppress non-essential animation:
//#define SUPPRESS_QSMOOTHLIST_ANIMATION


Q_GLOBAL_STATIC(QItemDelegate, standardDelegate);

static const int rate = 18;
static const int selectionTimeout = 200; // highlight if finger stationary for selectionTimeout ms.
static const int stationaryTimeout = 200;
static const qreal nearZero = 0.01f;
static const qreal nearOne = 0.99f;


class MotionVelocity
{
public:
    MotionVelocity();

    void reset();

    void setPosition(qreal);
    bool positionSet() const;

    qreal velocity();

private:
    qreal m_originalPos;
    qreal m_lastPos;
    qreal m_velocity;
    int m_lastElapsed;
    QTime m_timer;
    bool m_reset;
};

MotionVelocity::MotionVelocity()
: m_originalPos(0.0f), m_lastPos(0.0f), m_velocity(0.0f), m_lastElapsed(0), m_reset(true)
{
}

void MotionVelocity::reset()
{
    m_velocity = 0.0f;
    m_originalPos = 0.0f;
    m_lastPos = 0.0f;
    m_lastElapsed = 0;
    m_reset = true;
}

void MotionVelocity::setPosition(qreal pos)
{
    if (m_reset) {
        m_reset = false;

        m_originalPos = pos;
        m_lastPos = pos;
        m_lastElapsed = 0;
        m_velocity = 0.0f;
        m_timer.start();
    } else {
        static const qreal minimumDelta = 3.0f;

        // Ignore changes that are too small to be meaningful
        qreal delta = (pos - m_lastPos);
        if (fabs(delta) > minimumDelta) {
            qreal totalDelta = (pos - m_originalPos);
            int totalElapsed = m_timer.elapsed();
            int elapsed = qMax(totalElapsed - m_lastElapsed, 1);

            if (((totalDelta > 0) && (delta < 0)) ||
                ((totalDelta < 0) && (delta > 0))) {
                // We have just reversed our flick direction - reset to point of inflection
                m_originalPos = m_lastPos;
                m_timer.start();
                m_velocity = 0.0f;

                totalDelta = (pos - m_originalPos);
                totalElapsed = 1;
            }

            // Positive velocity is upwards (scaled up 1000x)
            qreal adjustment = (delta * -1000.0) / elapsed;

            if (m_velocity) {
                // Average velocity with previous average, giving old figure twice the weighting
                m_velocity = (2 * m_velocity + adjustment) / 3;
            } else {
                m_velocity = adjustment;
            }

            m_lastElapsed = totalElapsed;
            m_lastPos = pos;
        }
    }
}

bool MotionVelocity::positionSet() const
{
    return !m_reset;
}

qreal MotionVelocity::velocity()
{
    // This code gets spuriously tripped early in the list's lifetime, so the threshold has to be huge
    const int sinceLastMove(m_timer.elapsed() - m_lastElapsed);
    if (sinceLastMove > stationaryTimeout) {
        // We stopped moving for so long, it shouldn't be treated as a flick
        return 0.0f;
    }

    return m_velocity;
}

class Scrollbar
{
public:
    enum { BarWidth = 10 };
    enum { Margin = 2 };

    Scrollbar(int height = 0);

    QRect rect() const;
    void setHeight(int height);
    void setPos(int pos);
    void setListHeight(int height);

    void paint(GfxPainter *);

    GfxValue *listPosition();
    GfxValue *alpha();

    static const qreal MinAlpha = 0.0f;
    static const qreal MaxAlpha = 0.9f;

private:
    void checkCache();
    GfxValue m_alpha;
    GfxValue m_listPos;
    int m_listHeight;
    int m_height;
    int m_pos;

    bool m_cacheValid;
    QImage imageCache;
public:
    int barHeight();

    void snap();
    QRect snapRect();
private:
    QRect m_snapRect;
};

Scrollbar::Scrollbar(int height)
: m_height(height), m_cacheValid(false)
{
}

QRect Scrollbar::rect() const
{
    return QRect(m_pos + Margin, Margin, BarWidth - (2 * Margin), m_height - (2 * Margin));
}

GfxValue *Scrollbar::alpha()
{
    return &m_alpha;
}

GfxValue *Scrollbar::listPosition()
{
    return &m_listPos;
}

void Scrollbar::setListHeight(int height)
{
    m_listHeight = height;
    m_cacheValid = false;
}

void Scrollbar::setHeight(int height)
{
    m_height = height;
    m_cacheValid = false;
}

void Scrollbar::setPos(int pos)
{
    m_pos = pos;
}

void Scrollbar::checkCache()
{
    if (!m_cacheValid) {
        int width = BarWidth - 2 * Margin;
        int height = barHeight();
        imageCache = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
        imageCache.fill(0);
        QPainter cp(&imageCache);
        cp.setPen(QPalette().color(QPalette::HighlightedText));
        cp.setBrush(QPalette().brush(QPalette::Highlight));
        cp.setRenderHint(QPainter::Antialiasing);
        cp.drawRoundedRect(imageCache.rect(), width, width);
        m_cacheValid = true;
    }
}

void Scrollbar::paint(GfxPainter *p)
{
    if (m_alpha.value() < nearZero)
        return;

    checkCache();

    int tbarHeight = barHeight();
    int barPosMax = rect().height() - tbarHeight;
    int barPos = static_cast<int>(barPosMax * (m_listPos.value() / (m_listHeight - m_height)));

    // Resizing can cause the list to be positioned beyond the scrollable limit
    barPos = qMax(qMin(barPos, barPosMax), 0);

    p->setOpacity(m_alpha.value());
    p->drawImage(rect().x(), rect().y() + barPos, imageCache);
    p->setOpacity(1.0f);
}

int Scrollbar::barHeight()
{
    int maxHeight = rect().height();
    int minHeight = maxHeight / 6;

    int barHeight = static_cast<int>(maxHeight * ((qreal)m_height / (qreal)m_listHeight));
    return qMax(barHeight, minHeight);
}

void Scrollbar::snap()
{
    m_snapRect = rect();
}

QRect Scrollbar::snapRect()
{
    return m_snapRect;
}


class ItemHighlight
{
public:
    ItemHighlight(QSmoothListPrivate *d);

    QRect rect() const;
    void setWidth(int width);

    void paint(GfxPainter *);
    void invalidateCache() { m_cacheValid = false; }

    GfxValue *listPosition();
    GfxValue *position();
    GfxValue *height();
    GfxValue *alpha();

    qreal screenPosition() const;

private:
    void checkCache();
    int m_width;

    GfxValue m_pos;
    GfxValue m_height;
    GfxValue m_listPos;
    GfxValue m_alpha;

    bool m_cacheValid;
    QImage imageCache;

    QSmoothListPrivate *m_d;

public:
    void snap();
    QRect snapRect();
private:
    QRect m_snapRect;
};


enum ChildrenState { None, Collapsed, Expanded };

struct ItemData
{
    ItemData(ItemData* parent, int row, ChildrenState);
    ~ItemData();

    int row() const;
    void setRow(int row);

    int depth() const;

    ChildrenState childrenState() const;

    int childCount() const;
    ItemData* childAt(int i) const;

    QModelIndex index(const QAbstractItemModel* model) const;

    int visibleDescendants() const;

    ItemData* m_parent;
    QList<ItemData*>* m_children;

    int m_row;
    ListItem* m_item;
};


struct QSmoothListPrivate
{
    QSmoothListPrivate()
        : m_model(0), m_delegate(standardDelegate()), m_detailsDelegate(0), m_concretePosition(0.0f),
          m_highlight(0), m_mouseClick(false), m_mousePressed(false), m_inFlick(false), m_position(0),
          m_focusItem(-1), m_rowCount(0), m_scrollbar(0), m_scrollOn(false),
          m_needRefreshOnShow(false), m_previouslyShown(false), m_forceRedraw(false),
          backgroundOffsetValid(false), m_iconSize(-1, -1), m_forceAlign(false),
          textElideMode(Qt::ElideRight),
          selectionMode(QSmoothList::SingleSelection), m_doneInit(false),
          m_opaqueItems(false), m_perfFrames(0), m_itemHeightEstimate(0),
          m_lastFillHeight(0), m_lastFillListHeight(0), m_lastFillPosition(0),
          m_lastPaintPosition(0), m_firstVisible(0), m_lastVisible(-1), m_lastItemIndex(-1),
          m_preloadIndex(-1), m_positionHint(QSmoothList::EnsureVisible), m_nonLeafNodes(0),
          m_unhandledResize(false), m_inWrap(false)
    {
        m_perfTime.start();
    }

    ~QSmoothListPrivate()
    {
        qDeleteAll(m_itemData);
    }

    QList<ItemData*> m_itemData;
    QSet<QPersistentModelIndex> m_expandedItems;

    QAbstractItemModel *m_model;
    QAbstractItemDelegate *m_delegate;
    QAbstractItemDelegate *m_detailsDelegate;
    QList<ItemData*> m_items;

    GfxTimeLine m_listTimeline;
    GfxValueGroup m_listPosition;
    GfxValue m_concretePosition;

    GfxTimeLine m_pauseTimeline;
    GfxTimeLineObject m_pauseObject;

    GfxTimeLine m_highlightAlpha;
    GfxTimeLine m_highlightTimeline;
    GfxTimeLine m_highlightHeight;
    ItemHighlight *m_highlight;

    QPoint m_origMousePos;
    QPoint m_prevMousePos;
    bool m_mouseClick;
    bool m_mousePressed;
    bool m_inFlick;

    int m_position;
    int m_focusItem;
    int m_rowCount;

    MotionVelocity m_velocity;

    GfxTimeLine m_scrollTime;
    Scrollbar *m_scrollbar;
    bool m_scrollOn;
    bool m_needRefreshOnShow;
    bool m_previouslyShown;
    bool m_forceRedraw;

    QImage background;
    QPoint backgroundOffset;
    bool backgroundOffsetValid;
    QColor backColor;

    QSize m_iconSize;

    bool m_forceAlign;

    GfxTimeLine m_activatedTimeline;
    GfxTimeLineObject m_activatedObject;

    Qt::TextElideMode textElideMode;
    QSmoothList::SelectionMode selectionMode;
    QTimer selectTimer;

    bool m_doneInit;

    void defaultViewOptionsV3(QSmoothList *w);
    QStyleOptionViewItemV3 defaultoption;

    QString m_emptyText;
    bool m_opaqueItems;

    QImage m_fpsImage;
    unsigned int m_perfFrames;
    QTime m_perfTime;

    QList<int> m_successorPosition;
    int m_itemHeightEstimate;

    void releaseModel();
    void syncToModel();

    void setRowCount(int count);

    bool insertRows(const QModelIndex &index, int start, int end);
    void insertItems(int start, int end);

    bool removeRows(const QModelIndex &index, int start, int end);
    void removeItems(int start, int end);

    void setItemHeightEstimate(int height);

    void setItemHeight(int index, int height);
    int itemHeight(int index) const;

    int itemPosition(int index) const;

    int listHeight() const;

    ItemData* createItemData(ItemData* parent, int row);

    ListItem* createItem(ItemData* data, int width);

    QList<int> m_nonunloadableItems;
    void setAllItemsUnloadable();

    ItemData* referenceItem(int index, int width);
    void dereferenceItem(ItemData* data);

    ItemData* mapFromItemIndex(int index, ItemData* parent, int hintIndex);
    ItemData* mapFromItemIndex(int index);

    ItemData* loadItem(int index, int width, bool unloadable = true);
    void unloadItem(int index, bool force = false);

    void unloadAllItems();

    int mapToItemIndex(ItemData* data) const;
    int mapToItemIndex(ItemData* data, ItemData* parent) const;

    int mapToItemIndex(const QModelIndex &index) const;
    int mapToItemIndex(const QModelIndex &parentIndex, int row) const;

    ItemData* locateChild(ItemData* parent, int row) const;
    ItemData* locateItem(const QModelIndex& index) const;

    int firstVisibleIndex(int visibleHeight, int listPosition = -1) const;
    int lastVisibleIndex(int visibleHeight, int listPosition = -1) const;

    int m_lastFillHeight;
    int m_lastFillListHeight;
    int m_lastFillPosition;
    int m_lastPaintPosition;

    int m_firstVisible;
    int m_lastVisible;
    int m_lastItemIndex;

    int m_preloadIndex;

    QSmoothList::ScrollHint m_positionHint;

    int m_nonLeafNodes;

    void loadVisibleItems(int height, int width);

    int indexOfPoint(const QPoint& p) const;

    void invalidateCache(void);

    void invalidateVisibleItems(void);

    bool isExpanded(const QModelIndex& index) const;

    void expandItem(ItemData* data, bool set, bool children, QSmoothList* list);

    void expandItem(const QModelIndex& index, bool set, QSmoothList* list);
    void expandAll(bool set, QSmoothList* list);

    bool validModel() const;
    bool validIndex(int index) const;

    void estimateItemHeight(int width);

    bool m_unhandledResize;
    QSize m_previousSize;

    QTime m_modificationTime;

    void scheduleMove(GfxTimeLine &timeline, GfxValue &value, qreal final, int period);
    void scheduleMove(GfxTimeLine &timeline, GfxValue &value, qreal initial, qreal final, int period);

    bool m_inWrap;
};

class ListItem
{
public:
    enum { LevelIndent = 16 };

    ListItem(QSmoothListPrivate *d, ItemData* data, int height, int width = 0);

    QRect rect() const;
    void setWidth(int width);
    void setHeight(int height);
    void setDetailsHeight(int height);
    void setOpaque(bool opaque) { m_opaque = opaque; }

    qreal screenPosition() const;

    void paint(GfxPainter *);

    int row() const { return m_data->row(); }
    int itemIndex() const { return m_itemIndex; }

    void setItemIndex(int index) { m_itemIndex = index; }

    bool itemValid() const { return m_cacheValid; }
    void invalidateCache() { m_cacheValid = false; }

    int height() const { return m_height; }
    int detailsHeight() const { return m_detailsHeight; }

    void snap();
    QRect snapRect();

private:
    void checkCache();
    void prepareCache(QImage &image, int &height, QAbstractItemDelegate *delegate);
    void drawBranches(QPainter *painter, const QRect &rect) const;

    QSmoothListPrivate *m_d;
    ItemData* m_data;

    int m_itemIndex;
    int m_width;

    bool m_cacheValid;
    QImage imageCache;
    QImage detailsImageCache;
    int m_height;
    int m_detailsHeight;
    bool m_opaque;
    QRect m_snapRect;
};


ItemData::ItemData(ItemData* parent, int row, ChildrenState state)
    : m_parent(parent), m_children(state == None ? 0 : new QList<ItemData*>), m_row(row), m_item(0)
{
}

ItemData::~ItemData()
{
    if (m_children) {
        qDeleteAll(*m_children);
        m_children->clear();

        delete m_children;
        m_children = 0;
    }

    if (m_item) {
        delete m_item;
        m_item = 0;
    }
}

int ItemData::row() const
{
    return m_row;
}

void ItemData::setRow(int row)
{
    m_row = row;
}

int ItemData::depth() const
{
    int ancestorCount = 0;

    ItemData* ancestor = m_parent;
    while (ancestor) {
        ++ancestorCount;
        ancestor = ancestor->m_parent;
    }

    return ancestorCount;
}

ChildrenState ItemData::childrenState() const
{
    if (!m_children)
        return None;

    if (m_children->isEmpty())
        return Collapsed;

    return Expanded;
}

int ItemData::childCount() const
{
    if (m_children)
        return m_children->count();

    return 0;
}

ItemData* ItemData::childAt(int i) const
{
    return m_children->at(i);
}

QModelIndex ItemData::index(const QAbstractItemModel* model) const
{
    if (m_parent) {
        return model->index(m_row, 0, m_parent->index(model));
    } else {
        return model->index(m_row, 0);
    }
}

int ItemData::visibleDescendants() const
{
    int visibleCount = 0;

    if (m_children) {
        foreach (ItemData* child, *m_children)
            visibleCount += (1 + child->visibleDescendants());
    }

    return visibleCount;
}

void QSmoothListPrivate::defaultViewOptionsV3(QSmoothList *w)
{
    QStyleOptionViewItemV3 option;

    option.initFrom(w);

    option.state &= ~QStyle::State_MouseOver;
    option.font = w->font();

    if (!w->hasFocus())
        option.state &= ~QStyle::State_Active;

    option.state &= ~QStyle::State_HasFocus;
    option.decorationSize = w->iconSize();

    option.decorationPosition = QStyleOptionViewItem::Left;
    option.decorationAlignment = Qt::AlignCenter;
    option.displayAlignment = Qt::AlignLeft|Qt::AlignVCenter;
    option.textElideMode = w->textElideMode();
    option.rect = QRect();
    option.showDecorationSelected = QApplication::style()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, 0, 0);

    //if (wrapItemText)
    //    option.features = QStyleOptionViewItemV2::WrapText;
    option.locale = QLocale::system();
    option.locale.setNumberOptions(QLocale::OmitGroupSeparator);
    option.widget = w;
    option.palette = w->palette();

    option.fontMetrics = w->fontMetrics();
    option.direction = w->layoutDirection();

    defaultoption = option;
}

void QSmoothListPrivate::releaseModel()
{
    // Destroy any references to an existing model
    setRowCount(0);
    qDeleteAll(m_itemData);
    m_itemData.clear();
    m_expandedItems.clear();
    m_nonLeafNodes = 0;
}

void QSmoothListPrivate::syncToModel()
{
    if (m_model) {
        Q_ASSERT(m_itemData.count() == 0);

        QList<ItemData*> expandItems;

        // Create the index to the new model
        for (int i = 0; i < m_model->rowCount(); ++i) {
            ItemData* data = createItemData(0, i);
            m_itemData.append(data);

            if (data->childrenState() != None)
                ++m_nonLeafNodes;

            if (m_expandedItems.contains(QPersistentModelIndex(data->index(m_model))))
                expandItems.append(data);
        }

        setRowCount(m_model->rowCount());

        if (m_model->rowCount()) {
            foreach (ItemData* item, expandItems)
                expandItem(item, true, false, 0);
        }
    }
}

void QSmoothListPrivate::setRowCount(int count)
{
    if (count == m_rowCount)
        return;

    invalidateVisibleItems();

    m_items.clear();
    m_successorPosition.clear();

    m_rowCount = count;

    int position = m_itemHeightEstimate;
    while (count--) {
        m_items.append(0);
        m_successorPosition.append(position);
        position += m_itemHeightEstimate;
    }

    m_lastItemIndex = (m_rowCount - 1);
}

bool QSmoothListPrivate::insertRows(const QModelIndex &parentIndex, int start, int end)
{
    const int rowsAdded = (end - start) + 1;

    ItemData* parent = 0;
    if (parentIndex.isValid()) {
        parent = locateItem(parentIndex);
        if (!parent) {
            // This item must not be loaded due to collapsed parents - ignore the addition
            return false;
        } else {
            if (parent->childrenState() == Collapsed)
                return false;

            if (!parent->m_children) {
                parent->m_children = new QList<ItemData*>;

                if (!parent->m_parent) {
                    // If we just changed to have non-leaf nodes, any cached items are now invalid
                    if (!m_nonLeafNodes)
                        invalidateCache();

                    // The parent is now a non-leaf node
                    ++m_nonLeafNodes;
                }
            }

            // The parent should be updated
            if (parent->m_item)
                parent->m_item->invalidateCache();
        }
    }

    QList<ItemData*>* siblings = (parent ? parent->m_children : &m_itemData);

    // Insert the new elements
    QList<ItemData*>::iterator it = siblings->begin() + start;
    for (int current = start; current <= end; ++current, ++it) {
        ItemData* child = createItemData(parent, current);
        it = siblings->insert(it, child);

        if (!parent && child->childrenState() != None) {
            // If we just changed to have non-leaf nodes, any cached items are now invalid
            if (!m_nonLeafNodes)
                invalidateCache();

            ++m_nonLeafNodes;
        }
    }

    // Adjust the indexes of any trailing siblings
    int current = start + rowsAdded;
    for (int count = siblings->count(); current < count; ++current)
        siblings->at(current)->setRow(current);

    return true;
}

void QSmoothListPrivate::insertItems(int start, int end)
{
    const int rowsAdded = (end - start) + 1;
    const bool preExisting = !m_successorPosition.isEmpty();

    // Insert the new positions, with estimated height
    int position = 0;
    if (start != 0)
        position = itemPosition(start);

    int insertedHeight = 0;
    QList<int>::iterator it = m_successorPosition.begin() + start;
    QList<ItemData*>::iterator iit = m_items.begin() + start;
    for (int current = start; current <= end; ++current, ++it, ++iit) {
        insertedHeight += m_itemHeightEstimate;
        it = m_successorPosition.insert(it, position + insertedHeight);
        iit = m_items.insert(iit, 0);
        ++m_lastItemIndex;
    }

    // The visible items may have changed
    if (start <= m_firstVisible) {
        m_firstVisible += rowsAdded;
        m_lastVisible += rowsAdded;

        // Only move the list if we had an actual position
        if (preExisting)
            m_listPosition.setValue(m_listPosition.value() + insertedHeight);
    }

    // For each trailing item, translate the position by the added amount
    for (int current = end + 1; current <= m_lastItemIndex; ++current) {
        m_successorPosition[current] += insertedHeight;
        if (m_items[current])
            m_items[current]->m_item->setItemIndex(current);
    }
}

bool QSmoothListPrivate::removeRows(const QModelIndex &parentIndex, int start, int end)
{
    const int rowsRemoved = (end - start) + 1;

    ItemData* parent = 0;
    if (parentIndex.isValid()) {
        parent = locateItem(parentIndex);
        if (!parent) {
            // This item must not be loaded due to collapsed parents - ignore the addition
            return false;
        } else {
            if (!parent->m_parent) {
                // See if this parent still has remaining children
                if (!m_model->hasChildren(parent->index(m_model))) {
                    // The parent was previously a non-leaf node
                    --m_nonLeafNodes;

                    // If we just changed to have no non-leaf nodes, any cached items are now invalid
                    if (!m_nonLeafNodes)
                        invalidateCache();
                }
            }

            // The parent should be updated
            if (parent->m_item)
                parent->m_item->invalidateCache();

            // If there are no rows to be removed, we're finished
            if (parent->childCount() == 0)
                return false;
        }
    }

    // Remove the discarded items from their container
    QList<ItemData*>* siblings = (parent ? parent->m_children : &m_itemData);

    QList<ItemData*>::iterator it = siblings->begin() + start;
    if (!parent) {
        // These are top-level nodes - decrease the non-leaf count where necessary
        for (QList<ItemData*>::iterator end = it + rowsRemoved; it != end; ++it)
            if ((*it) && (*it)->childrenState() != None) {
                --m_nonLeafNodes;

                // If we just changed to have no non-leaf nodes, any cached items are now invalid
                if (!m_nonLeafNodes)
                    invalidateCache();
            }

        // Reset the iterator
        it = siblings->begin() + start;
    }

    qDeleteAll(it, it + rowsRemoved);
    siblings->erase(it, it + rowsRemoved);

    // Adjust the indexes of any trailing siblings
    int current = start;
    for (it = siblings->begin() + current; it != siblings->end(); ++it, ++current)
        (*it)->setRow(current);

    return true;
}

void QSmoothListPrivate::removeItems(int start, int end)
{
    const int rowsRemoved = (end - start) + 1;

    // Remove the discarded positions
    int removedHeight = 0;
    for (int current = start; current <= end; ++current) {
        removedHeight += itemHeight(current);
        unloadItem(current, true);
    }

    QList<int>::iterator it = m_successorPosition.begin() + start;
    m_successorPosition.erase(it, it + rowsRemoved);

    QList<ItemData*>::iterator iit = m_items.begin() + start;
    m_items.erase(iit, iit + rowsRemoved);

    m_lastItemIndex -= rowsRemoved;

    // The visible items may have changed
    if (start <= m_firstVisible) {
        m_firstVisible -= rowsRemoved;
        if (m_firstVisible < 0)
            m_firstVisible = 0;

        m_lastVisible -= rowsRemoved;
        if (m_lastVisible < 0)
            m_lastVisible = 0;

        if (m_listPosition.value() <= removedHeight)
            m_listPosition.setValue(0);
        else
            m_listPosition.setValue(m_listPosition.value() - removedHeight);
    }

    // For each trailing item, translate the position by the removed amount
    for (int current = start; current <= m_lastItemIndex; ++current) {
        m_successorPosition[current] -= removedHeight;
        if (m_items[current])
            m_items[current]->m_item->setItemIndex(current);
    }

    // This may invalidate our visible range
    invalidateVisibleItems();
}

void QSmoothListPrivate::setItemHeightEstimate(int height)
{
    if (height != m_itemHeightEstimate) {
        m_itemHeightEstimate = height;

        // We now have an estimate - make all items this size
        int position = m_itemHeightEstimate;
        QList<int>::iterator it = m_successorPosition.begin(), end = m_successorPosition.end();
        for ( ; it != end; ++it) {
            *it = position;
            position += m_itemHeightEstimate;
        }

        if (m_model && m_scrollbar)
            m_scrollbar->setListHeight(listHeight());
    }
}

void QSmoothListPrivate::estimateItemHeight(int width)
{
    if (m_model && m_delegate && m_model->rowCount()) {
        // Re-estimate the height for unloaded items, from the first item
        QStyleOptionViewItemV3 opt = defaultoption;

        int height(m_itemHeightEstimate == 1 ? 0 : m_itemHeightEstimate);
        opt.rect = QRect(0, 0, width, height);

        QSize hintSize(m_delegate->sizeHint(opt, m_model->index(0,0)));
        setItemHeightEstimate(hintSize.height());
    } else {
        setItemHeightEstimate(1);
    }
}

void QSmoothListPrivate::setItemHeight(int index, int height)
{
    Q_ASSERT(validIndex(index));

    int existing = itemHeight(index);
    if (existing != height) {
        // All offsets from this one must change
        int delta = height - existing;
        QList<int>::iterator it = m_successorPosition.begin() + index, end = m_successorPosition.end();
        for ( ; it != end; ++it)
            *it += delta;

        m_scrollbar->setListHeight(listHeight());

        const int position = itemPosition(index);
        if (position < static_cast<int>(m_listPosition.value())) {
            // This resize affects the current list position
            m_listPosition.setValue(m_listPosition.value() + delta);
        }
        if ((m_focusItem != -1) && (position < static_cast<int>(m_highlight->position()->value()))) {
            // This resize affects the current highlight position
            m_highlight->position()->setValue(m_highlight->position()->value() + delta);
        }
    }
}

int QSmoothListPrivate::itemHeight(int index) const
{
    Q_ASSERT(validIndex(index));

    int successorPosition(m_successorPosition[index]);
    if (index == 0)
        return successorPosition;
    else
        return successorPosition - m_successorPosition[index - 1];
}

int QSmoothListPrivate::itemPosition(int index) const
{
    Q_ASSERT((index >= 0) && m_model && ((index - 1) <= m_lastItemIndex));

    if (index == 0)
        return 0;
    else
        return m_successorPosition[index - 1];
}

int QSmoothListPrivate::listHeight() const
{
    if (m_lastItemIndex == -1)
        return 0;
    else
        return m_successorPosition[m_lastItemIndex];
}

void QSmoothListPrivate::setAllItemsUnloadable()
{
    m_nonunloadableItems.clear();
}

ItemData* QSmoothListPrivate::createItemData(ItemData* parent, int row)
{
    QModelIndex itemIndex;
    if (parent) {
        itemIndex = parent->index(m_model).child(row, 0);
    } else {
        itemIndex = m_model->index(row, 0);
    }

    ChildrenState state(m_model->hasChildren(itemIndex) ? Collapsed : None);

    return new ItemData(parent, row, state);
}

ListItem* QSmoothListPrivate::createItem(ItemData* data, int width)
{
    QStyleOptionViewItemV3 opt = defaultoption;
    opt.rect = QRect(0, 0, width, m_itemHeightEstimate);

    // Find the actual height of this item from the delegate
    int itemHeight = m_delegate->sizeHint(opt, data->index(m_model)).height();

    ListItem *item = new ListItem(this, data, itemHeight, width);
    item->setOpaque(m_opaqueItems);

    return item;
}

ItemData* QSmoothListPrivate::mapFromItemIndex(int index, ItemData* parent, int hintIndex)
{
    QList<ItemData*>* siblings = (parent ? parent->m_children : &m_itemData);

    // Optimization: A binary search may be more efficient
    int siblingIndex = (hintIndex != -1 ? hintIndex : siblings->count() - 1);

    ItemData* data = siblings->at(siblingIndex);
    int itemIndex = mapToItemIndex(data);

    // Search back through the indices at this level until we find it or its parent
    while (itemIndex > index) {
        --siblingIndex;
        data = siblings->at(siblingIndex);
        itemIndex = mapToItemIndex(data);
    }

    if (itemIndex < index)  {
        return mapFromItemIndex(index, data, -1);
    } else {
        return data;
    }
}

ItemData* QSmoothListPrivate::mapFromItemIndex(int index)
{
    // Start searching the top-level index
    int hintIndex = qMin(index, m_itemData.count() - 1);
    return mapFromItemIndex(index, 0, hintIndex);
}

ItemData* QSmoothListPrivate::referenceItem(int index, int width)
{
    ItemData* data = mapFromItemIndex(index);
    if (data->m_item == 0) {
        data->m_item = createItem(data, width);
        setItemHeight(index, data->m_item->height());
    }

    return data;
}

ItemData* QSmoothListPrivate::loadItem(int index, int width, bool unloadable)
{
    ItemData *item = m_items[index];
    if (!item) {
        item = referenceItem(index, width);
        m_items[index] = item;
    }

    if (!unloadable)
        m_nonunloadableItems.append(index);

    return item;
}

void QSmoothListPrivate::dereferenceItem(ItemData* data)
{
    delete data->m_item;
    data->m_item = 0;
}

void QSmoothListPrivate::unloadItem(int index, bool force)
{
    // Don't unload items marked as unloadable or during finger movement unless forced
    if (force || (!m_mousePressed && !m_nonunloadableItems.contains(index))) {
        // Don't unload the focus item if the activation timeline is running
        if (index != m_focusItem || !m_activatedTimeline.isActive()) {
            ItemData* item = m_items[index];
            if (item) {
                m_items[index] = 0;
                dereferenceItem(item);
            }
        }
    }
}

void QSmoothListPrivate::unloadAllItems()
{
    if (m_activatedTimeline.isActive())
        m_activatedTimeline.clear();

    for (int ii = 0; ii <= m_lastItemIndex; ++ii)
        unloadItem(ii, true);
}

int QSmoothListPrivate::mapToItemIndex(ItemData* data) const
{
    return mapToItemIndex(data, data->m_parent);
}

int QSmoothListPrivate::mapToItemIndex(ItemData* data, ItemData* parent) const
{
    const QList<ItemData*>* siblings = &m_itemData;
    int parentIndex = -1;

    if (parent != 0) {
        // If the parent is not expanded, we're not visible
        if (parent->childrenState() == Collapsed)
            return -1;

        // Find where the parent is in the list
        parentIndex = mapToItemIndex(parent);

        // If the parent is not visible, then neither are we
        if (parentIndex == -1)
            return -1;

        siblings = parent->m_children;
    }

    // See how many items are between our parent and ourselves
    int precedingDescendants = 0;

    // If there is no item specified, find the position for a new item in this parent
    int precedingSiblings = (data ? data->row() : siblings->count());

    for (int i = 0; i < precedingSiblings; ++i)
        if (ItemData* sibling = siblings->at(i))
            precedingDescendants += sibling->visibleDescendants();

    return parentIndex + precedingSiblings + precedingDescendants + 1;
}

int QSmoothListPrivate::mapToItemIndex(const QModelIndex &index) const
{
    if (ItemData* data = locateItem(index)) {
        return mapToItemIndex(data);
    } else {
        // This index must not be loaded
        return -1;
    }
}

int QSmoothListPrivate::mapToItemIndex(const QModelIndex &parentIndex, int row) const
{
    if (!parentIndex.isValid()) {
        if (!m_itemData.isEmpty()) {
            if (m_itemData.count() > row) {
                ItemData* data = m_itemData.at(row);
                return mapToItemIndex(data, 0);
            } else {
                // This index is currently out of scope for the top-level
                return mapToItemIndex(0, 0);
            }
        } else {
            // We have not yet sync'd to the model
            return row;
        }
    } else {
        if (ItemData* parent = locateItem(parentIndex)) {
            if (parent->childCount() > row) {
                return mapToItemIndex(parent->childAt(row), parent);
            } else {
                // This index is currently out of scope for the parent
                return mapToItemIndex(0, parent);
            }
        } else {
            // This index must not be loaded
            return -1;
        }
    }
}

ItemData* QSmoothListPrivate::locateChild(ItemData* parent, int row) const
{
    if (parent) {
        if (row < parent->childCount()) {
            return parent->childAt(row);
        }
    }

    return 0;
}

ItemData* QSmoothListPrivate::locateItem(const QModelIndex& index) const
{
    if (index.isValid()) {
        QModelIndex parentIndex(index.parent());
        if (!parentIndex.isValid()) {
            if (index.row() < m_itemData.count()) {
                return m_itemData.at(index.row());
            }
        } else {
            return locateChild(locateItem(parentIndex), index.row());
        }
    }

    return 0;
}

int QSmoothListPrivate::firstVisibleIndex(int visibleHeight, int listPosition) const
{
    if (m_lastItemIndex > 0) {
        if (listPosition == -1)
            listPosition = int(m_listPosition.value());

        int lowVisibleBound = qMin(listPosition, listHeight() - visibleHeight);
        for (int ii = 1; ii <= m_lastItemIndex; ++ii)
            if (itemPosition(ii) > lowVisibleBound)
                return ii - 1;

        return m_lastItemIndex;
    }

    return 0;
}

int QSmoothListPrivate::lastVisibleIndex(int visibleHeight, int listPosition) const
{
    if (m_lastItemIndex != -1) {
        if (listPosition == -1)
            listPosition = int(m_listPosition.value());

        int highVisibleBound = qMax(listPosition + visibleHeight, visibleHeight);
        for (int ii = m_lastItemIndex; ii > 0; --ii)
            if (itemPosition(ii) <= highVisibleBound)
                return ii;

        return 0;
    }

    return -1;
}

void QSmoothListPrivate::loadVisibleItems(int height, int width)
{
    if (m_lastItemIndex == -1)
        return;

    // Ensure all visible items are loaded
    m_firstVisible = firstVisibleIndex(height);
    m_lastVisible = lastVisibleIndex(height);

    // Load the items we think are visible
    bool changed(false);
    do {
        bool loaded(false);
        for (int ii = m_firstVisible; ii <= m_lastVisible; ++ii) {
            if (!m_items.at(ii)) {
                loadItem(ii, width);
                loaded = true;
            }
        }

        changed = false;
        if (loaded) {
            int first = firstVisibleIndex(height);
            int last = lastVisibleIndex(height);

            if ((first != m_firstVisible) || (last != m_lastVisible)) {
                m_firstVisible = first;
                m_lastVisible = last;
                changed = true;
            }
        }
    } while (changed);

    if (m_firstVisible > 0) {
        int firstLoaded = (m_firstVisible - 1);

        // Unload non-visible items that we don't currently need
        for (int ii = 0; ii < firstLoaded; ++ii)
            if (m_items.at(ii))
                unloadItem(ii);

        // Ensure one item before the first visible is loaded, to permit some scrolling without loading
        if (!m_items.at(firstLoaded))
            loadItem(firstLoaded, width);
    }

    if (m_lastVisible < m_lastItemIndex) {
        int lastLoaded = (m_lastVisible + 1);

        // Ensure one item after the last visible is loaded, to permit some scrolling without loading
        if (!m_items.at(lastLoaded))
            loadItem(lastLoaded, width);

        // Unload non-visible items that we don't currently need
        for (int ii = lastLoaded + 1; ii < m_lastItemIndex; ++ii)
            if (m_items.at(ii))
                unloadItem(ii);
    }
}

int QSmoothListPrivate::indexOfPoint(const QPoint& p) const
{
    if (validModel())
        for (int ii = m_firstVisible; ii <= m_lastVisible; ++ii)
            if (m_items.at(ii) && m_items.at(ii)->m_item->rect().contains(p))
                return ii;

    return -1;
}

void QSmoothListPrivate::invalidateCache(void)
{
    QList<ItemData*>::iterator it = m_items.begin(), end = m_items.end();
    for ( ; it != end; ++it)
        if (*it)
            (*it)->m_item->invalidateCache();
}

void QSmoothListPrivate::invalidateVisibleItems(void)
{
    m_lastFillHeight = 0;
    m_lastFillListHeight = 0;
    m_lastFillPosition = 0;
    m_firstVisible = 0;
    m_lastVisible = -1;
}

bool QSmoothListPrivate::isExpanded(const QModelIndex& index) const
{
    if (index.isValid())
        if (ItemData* data = locateItem(index))
            return (data->childrenState() == Expanded);

    return false;
}

void QSmoothListPrivate::expandItem(ItemData* data, bool set, bool childrenAlso, QSmoothList* list)
{
    if ((set && (data->childrenState() == Collapsed)) || (!set && (data->childrenState() == Expanded))) {
        int parentItemIndex = mapToItemIndex(data);

        QModelIndex itemIndex = data->index(m_model);

        if (set) {
            Q_ASSERT(data->childCount() == 0);
            if (!data->m_children)
                data->m_children = new QList<ItemData*>;

            // Add entries for each child
            int n = m_model->rowCount(itemIndex);
            for (int i = 0; i < n; ++i)
                data->m_children->append(createItemData(data, i));

            // Insert the child items beneath ourselves
            insertItems(parentItemIndex + 1, parentItemIndex + n);

            // This item is now expanded
            m_expandedItems.insert(QPersistentModelIndex(itemIndex));

            if (list)
                emit list->expanded(itemIndex);

            // Expand any children that should be expanded
            foreach (ItemData* item, *data->m_children)
                if (childrenAlso || m_expandedItems.contains(QPersistentModelIndex(item->index(m_model))))
                    expandItem(item, true, childrenAlso, (childrenAlso ? list : 0));
        } else {
            // Collapse any children that won't be visible
            foreach (ItemData* item, *data->m_children)
                expandItem(item, false, childrenAlso, (childrenAlso ? list : 0));

            // Remove the child items beneath ourselves
            removeItems(parentItemIndex + 1, parentItemIndex + data->childCount());

            // Remove the child entries
            qDeleteAll(*data->m_children);
            data->m_children->clear();

            if (list) {
                // This item is now collapsed
                m_expandedItems.remove(itemIndex);
                emit list->collapsed(itemIndex);
            }
        }
    }

    m_scrollbar->setListHeight(listHeight());
}

void QSmoothListPrivate::expandItem(const QModelIndex& index, bool set, QSmoothList* list)
{
    if (ItemData* data = locateItem(index)) {
        expandItem(data, set, false, list);

        if (data->m_item)
            data->m_item->invalidateCache();
    } else if (index.isValid()) {
        if (set) {
            m_expandedItems.insert(QPersistentModelIndex(index));
        } else {
            m_expandedItems.remove(QPersistentModelIndex(index));
        }
    }
}

void QSmoothListPrivate::expandAll(bool set, QSmoothList* list)
{
    foreach (ItemData* data, m_itemData) {
        expandItem(data, set, true, list);

        if (data->m_item)
            data->m_item->invalidateCache();
    }
}

bool QSmoothListPrivate::validModel() const
{
    return (m_doneInit && m_model && (m_rowCount > 0));
}

bool QSmoothListPrivate::validIndex(int index) const
{
    if ((index < 0) || !m_model || (index > m_lastItemIndex))
        return false;

    return true;
}

void QSmoothListPrivate::scheduleMove(GfxTimeLine &timeline, GfxValue &value, qreal final, int period)
{
#ifndef SUPPRESS_QSMOOTHLIST_ANIMATION
    if (period) {
        timeline.move(value, final, period);
    } else {
#endif
        timeline.set(value, final);
#ifndef SUPPRESS_QSMOOTHLIST_ANIMATION
    }
#else
    Q_UNUSED(period)
#endif
}

void QSmoothListPrivate::scheduleMove(GfxTimeLine &timeline, GfxValue &value, qreal initial, qreal final, int period)
{
#ifndef SUPPRESS_QSMOOTHLIST_ANIMATION
    if (period) {
        timeline.set(value, initial);
        timeline.move(value, final, period);
    } else {
#endif
        timeline.set(value, final);
#ifndef SUPPRESS_QSMOOTHLIST_ANIMATION
    }
#else
    Q_UNUSED(initial)
    Q_UNUSED(period)
#endif
}


ListItem::ListItem(QSmoothListPrivate *d, ItemData* data, int height, int width)
  : m_d(d), m_data(data), m_itemIndex(m_d->mapToItemIndex(m_data)), m_width(width),
    m_cacheValid(false), m_height(height), m_detailsHeight(0), m_opaque(false)
{
}

QRect ListItem::rect() const
{
    return QRect(0, int(screenPosition()), m_width, m_height);
}

void ListItem::setWidth(int width)
{
    m_width = width;
    m_cacheValid = false;
}

void ListItem::setHeight(int height)
{
    m_height = height;
    m_cacheValid = false;
}

void ListItem::setDetailsHeight(int height)
{
    m_detailsHeight = height;
    m_cacheValid = false;
}

qreal ListItem::screenPosition() const
{
    return qreal(m_d->itemPosition(m_itemIndex)) - m_d->m_listPosition.value();
}

void ListItem::prepareCache(QImage &image, int &height, QAbstractItemDelegate *delegate)
{
    if (delegate) {
        QModelIndex itemIndex(m_data->index(m_d->m_model));
        int indent = (m_d->m_nonLeafNodes ? (m_data->depth() + 1) * LevelIndent : 0);
        int maxWidth = m_width - indent;

        QStyleOptionViewItemV3 opt = m_d->defaultoption;
        if (m_opaque && (m_d->m_focusItem == m_itemIndex))
            opt.state |= QStyle::State_Selected;

        opt.rect = QRect(0, 0, maxWidth, height);
        QSize size = delegate->sizeHint(opt, itemIndex);

        height = size.height();
        int width = m_opaque ? maxWidth : qMin(maxWidth, size.width());
        opt.rect = QRect(0, 0, width, height);

        image = QImage(width, height, m_opaque ? QImage::Format_RGB32 : QImage::Format_ARGB32_Premultiplied);
        image.fill(0);

        QPainter cp(&image);
        delegate->paint(&cp, opt, itemIndex);
    } else {
        height = 0;
        image = QImage();
    }
}

void ListItem::checkCache()
{
    if (!m_cacheValid) {
        prepareCache(imageCache, m_height, m_d->m_delegate);
        prepareCache(detailsImageCache, m_detailsHeight, m_d->m_detailsDelegate);
        m_cacheValid = true;
    }
}

void ListItem::drawBranches(QPainter *painter, const QRect &rect) const
{
    static const bool rightToLeftMode(QtopiaApplication::layoutDirection() == Qt::RightToLeft);

    QRect primitive(rightToLeftMode ? rect.left() : rect.right() + 1, rect.top(), LevelIndent, rect.height());

    QStyleOptionViewItemV3 opt = m_d->defaultoption;

    ItemData* data = m_data;
    while (data) {
        primitive.moveLeft(rightToLeftMode ? primitive.left() : primitive.left() - LevelIndent);
        opt.rect = primitive;

        if (data == m_data) {
            opt.state = QStyle::State_Item;

            if (data->childrenState() != None)
                opt.state |= QStyle::State_Children;

            if (data->childrenState() == Expanded)
                opt.state |= QStyle::State_Open;
        } else {
            opt.state = 0;
        }

        if (m_data->m_item && m_d->m_focusItem == m_data->m_item->m_itemIndex)
            opt.state |= QStyle::State_Selected;

        QList<ItemData*>* siblings = (data->m_parent ? data->m_parent->m_children : &m_d->m_itemData);
        bool moreSiblings = (siblings->count() > (data->m_row + 1));

        if (moreSiblings)
            opt.state |= QStyle::State_Sibling;

        QApplication::style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, 0);
        data = data->m_parent;
    }
}

void ListItem::paint(GfxPainter *p)
{
    static const bool rightToLeftMode(QtopiaApplication::layoutDirection() == Qt::RightToLeft);

    checkCache();

    p->setOpacity(1.0f);

    QRect mainRect = rect();
    QRect branchRect(mainRect);

    int indent = (m_d->m_nonLeafNodes ? (m_data->depth() + 1) * LevelIndent : 0);
    int left = (rightToLeftMode ? m_width - imageCache.width() - indent : indent);

    if (rightToLeftMode) {
        branchRect.setLeft(m_width - indent);
    } else {
        branchRect.setRight(indent);
    }

    p->drawImage(QPoint(left, mainRect.top()), imageCache);

    QImage branchImage(branchRect.size(), m_opaque ? QImage::Format_RGB32 : QImage::Format_ARGB32_Premultiplied);
    branchImage.fill(0);

    QPainter bp(&branchImage);
    bp.setPen(m_d->defaultoption.palette.color(QPalette::Text));
    drawBranches(&bp, QRect(QPoint(0, 0), branchImage.size()));

    p->drawImage(branchRect.topLeft(), branchImage);

    if (m_detailsHeight) {
        int followingItem = m_itemIndex + 1;
        if (m_d->m_lastItemIndex >= followingItem) {
            qreal followingPosition(m_d->itemPosition(followingItem));
            followingPosition -= m_d->m_listPosition.value();
            if (mainRect.bottom() < int(followingPosition)) {
                p->setUserClipRect(QRect(left, mainRect.bottom(),
                            mainRect.width(), int(followingPosition)-mainRect.bottom()));
                p->drawImage(QPoint(left, mainRect.bottom()), detailsImageCache);
                p->setUserClipRect(QRect());
            }
        }
    }
}

void ListItem::snap()
{
    m_snapRect = rect();
}

QRect ListItem::snapRect()
{
    return m_snapRect;
}

ItemHighlight::ItemHighlight(QSmoothListPrivate *d)
    : m_width(0), m_pos(0.0f), m_height(0.0f), m_listPos(0.0f), m_alpha(0.0f), m_cacheValid(false), m_d(d)
{
}

QRect ItemHighlight::rect() const
{
    return QRect(0, int(screenPosition()), m_width, int(m_height.value()));
}

void ItemHighlight::setWidth(int width)
{
    m_cacheValid = false;
    m_width = width;
}

void ItemHighlight::checkCache()
{
    if(!m_cacheValid) {
        //ideally, we would use the item delegate to determine what the highlight should look like
        imageCache = QImage(m_width, int(m_height.value()), QImage::Format_ARGB32_Premultiplied);
        imageCache.fill(0);
        QPainter cp(&imageCache);
        cp.setRenderHint(QPainter::Antialiasing);
        QColor color = m_d->defaultoption.palette.color(QPalette::Highlight);
        cp.setPen(color);
        QLinearGradient bgg(QPoint(0,0), QPoint(0, imageCache.rect().height()));
        bgg.setColorAt(0.0f, color.lighter(175));
        bgg.setColorAt(0.49f, color.lighter(105));
        bgg.setColorAt(0.5f, color);
        cp.setBrush(bgg);
        cp.drawRoundRect(imageCache.rect(), 800/imageCache.rect().width(),800/imageCache.rect().height());
        m_cacheValid = true;
    }
}

void ItemHighlight::paint(GfxPainter *p)
{
    if((m_width == 0) || (m_height.value() < 1))
        return;
    if(m_alpha.value() < nearZero)
        return;

    checkCache();
    p->setOpacity(m_alpha.value());
    p->drawImage(rect().topLeft(), imageCache);
    p->setOpacity(1.0f);
}

GfxValue *ItemHighlight::listPosition()
{
    return &m_listPos;
}

GfxValue *ItemHighlight::position()
{
    return &m_pos;
}

GfxValue *ItemHighlight::height()
{
    return &m_height;
}

GfxValue *ItemHighlight::alpha()
{
    return &m_alpha;
}

qreal ItemHighlight::screenPosition() const
{
    return m_pos.value() - m_listPos.value();
}

void ItemHighlight::snap()
{
    m_snapRect = rect();
}

QRect ItemHighlight::snapRect()
{
    return m_snapRect;
}

/*!
    \class QSmoothList
    \inpublicgroup QtBaseModule
    \brief The QSmoothList class provides a view onto a model using a finger scrollable user interface.
    \ingroup model-view

    QSmoothList implements the interfaces defined by the QAbstractItemView class to allow it to
    display data provided by models derived from the QAbstractItemModel class.  A QSmoothList
    presents items stored in a model as a simple non-hierarchical list, or in a single-column
    hierarchical tree.  If QSmoothList is provided with a hierarchical model, it will correctly
    reflect the hierarchy of model items, but only row zero of each QModelIndex is used to
    present a view of the items.

    QSmoothList is designed to offer a similar interface to QListView; the primary difference
    is that QSmoothList allows the list position to be manipulated with a finger, facilitating
    use in touch-screen devices.  QSmoothList also offers a subset of the QTreeView interface;
    items can be programmatically expanded or collapsed. Unlike QTreeView, however, QSmoothList
    does not support the use of a QHeaderView, and does not support multiple-column presentation.

    The location of the QSmoothList can be manipulated by dragging, or by a 'flick' gesture. A
    flick occurs when a vertical movement is interrupted by a release - a momentum is then imparted
    to the list which continues to move in the initial direction, subject to a deceleration. The
    list can also be moved by key events if a suitable input device is available.

    A QSmoothList can be used in single-selection mode, or in no-selection mode.  Multiple
    selection is not supported, and must instead be implemented outside the QSmoothList, via
    a 'checkable' property of the model's items, or similar.  Since multiple selection is not
    supported, QSmoothList does not differentiate between 'selected' and 'current' indices.

    QSmoothList supports items of variable heights, as reported by the sizeHint() function of
    the delegate used to present the list items.  However, QSmoothList does not support grid
    spacing.  It is recommended that the delegate used with the QSmoothList should enforce
    height constraints, to achieve a presentation where items with variable sizes are displayed
    with uniform space allocations.
*/

/*!
    \enum QSmoothList::ScrollHint

    \value EnsureVisible  Scroll to ensure that the item is visible.
    \value PositionAtTop  Scroll to position the item at the top of the viewport.
    \value PositionAtBottom  Scroll to position the item at the bottom of the viewport.
    \value PositionAtCenter  Scroll to position the item at the center of the viewport.
    \value ImmediateVisible  Reposition the list without animation to ensure that the item is visible.
*/

/*!
    \enum QSmoothList::SelectionMode

    This enum indicates how the list responds to user selections:

    \value SingleSelection  When the user selects an item, any already-selected item becomes unselected, and the user cannot unselect the selected item.
    \value NoSelection  Items cannot be selected.
*/

/*!
    \fn void QSmoothList::activated(const QModelIndex &index)

    This signal is emitted when the item specified by \a index is
    activated by the user. How to activate items depends on the
    platform; e.g., by single- or double-clicking the item, or by
    pressing the Return or Enter key when the item is current.

    \sa clicked(), doubleClicked(), pressed()
*/

/*!
    \fn void QSmoothList::pressed(const QModelIndex &index)

    This signal is emitted when a mouse button is pressed. The item
    the mouse was pressed on is specified by \a index. The signal is
    only emitted when the index is valid.

    Use the QApplication::mouseButtons() function to get the state
    of the mouse buttons.

    \sa activated(), clicked(), doubleClicked()
*/

/*!
    \fn void QSmoothList::clicked(const QModelIndex &index)

    This signal is emitted when a mouse button is clicked. The item
    the mouse was clicked on is specified by \a index. The signal is
    only emitted when the index is valid.

    \sa activated(), doubleClicked(), pressed()
*/

/*!
    \fn void QSmoothList::doubleClicked(const QModelIndex &index)

    This signal is emitted when a mouse button is double-clicked. The
    item the mouse was double-clicked on is specified by \a index.
    The signal is only emitted when the index is valid.

    \sa clicked(), activated()
*/

/*!
    \fn void QSmoothList::currentChanged(const QModelIndex &current, const QModelIndex &previous)

    This signal is emitted when a new item becomes the current item.
    The previous current item is specified by the \a previous index, and the new
    item by the \a current index.

    \sa currentIndex(), setCurrentIndex()
*/

/*!
    \fn void QSmoothList::expanded(const QModelIndex &index)

    This signal is emitted when the item specified by \a index is expanded.

    \sa collapsed(), setExpanded()
*/

/*!
    \fn void QSmoothList::collapsed(const QModelIndex &index)

    This signal is emitted when the item specified by \a index is collapsed.

    \sa expanded(), setExpanded()
*/

/*!
    Creates a new QSmoothList with the given \a parent and \a flags to view a model.
    Use setModel() to set the model.
*/
QSmoothList::QSmoothList(QWidget *parent, Qt::WFlags flags)
: QWidget(parent, flags), d(0)
{
    d = new QSmoothListPrivate;

    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    QExportedBackground *bg = new QExportedBackground(this);
    QObject::connect(bg, SIGNAL(changed(QPixmap)),
                     this, SLOT(backgroundChanged(QPixmap)));
    d->background = bg->background().toImage();
    d->backColor = palette().brush(QPalette::Window).color();

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled);

    int pm = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
    d->m_iconSize = QSize(pm, pm);

    d->m_listPosition.addGfxValue(&d->m_concretePosition);
    QObject::connect(&d->m_listTimeline, SIGNAL(completed()),
                     this, SLOT(updateCompleted()));
    QObject::connect(&d->m_listTimeline, SIGNAL(completed()),
                     this, SLOT(tryHideScrollBar()));

    d->m_highlight = new ItemHighlight(d);
    d->m_highlight->position()->setValue(0);
    d->m_highlight->height()->setValue(0);
    d->m_highlight->listPosition()->setValue(0);
    d->m_listPosition.addGfxValue(d->m_highlight->listPosition());
    QObject::connect(&d->m_highlightTimeline, SIGNAL(updated()),
                     this, SLOT(updateHighlight()));
    QObject::connect(&d->m_highlightHeight, SIGNAL(updated()),
                     this, SLOT(updateHighlight()));
    QObject::connect(&d->m_highlightAlpha, SIGNAL(updated()),
                     this, SLOT(updateHighlight()));
    QObject::connect(&d->m_highlightTimeline, SIGNAL(completed()),
                     this, SLOT(tryHideScrollBar()));

    d->m_scrollbar = new Scrollbar(0);

    QObject::connect(&d->m_pauseTimeline, SIGNAL(completed()),
                     this, SLOT(tryHideScrollBar()));

    QObject::connect(&d->m_activatedTimeline, SIGNAL(completed()),
                     this, SLOT(emitActivated()));

    d->selectTimer.setSingleShot(true);
    connect(&d->selectTimer, SIGNAL(timeout()), this, SLOT(selectTimeout()));

#ifdef QTOPIA_HOMEUI
    setOpaqueItemBackground(true);
#endif

    QObject::connect(GfxTimeLine::globalClock(), SIGNAL(tick()), this, SLOT(doUpdate()));
    QObject::connect(GfxTimeLine::globalClock(), SIGNAL(aboutToTick()), this, SLOT(tickStart()));

    init();
}

/*!
  Destroys the list.
*/
QSmoothList::~QSmoothList()
{
    if (d->m_highlight)
        delete d->m_highlight;
    if (d->m_scrollbar)
        delete d->m_scrollbar;
    delete d; d = 0;
}

/*! \internal */
void QSmoothList::updateCompleted()
{
    if (d->m_forceAlign) {
        d->m_forceAlign = false;
        fixupPosition(rate*6);
    }

    if (d->m_inFlick)
        d->m_inFlick = false;

    if (d->m_inWrap)
        d->m_inWrap = false;

    d->setAllItemsUnloadable();
    ensureVisibleItemsLoaded(true);
}

/*! \internal */
void QSmoothList::tickStart()
{
    for(int ii = d->m_firstVisible; ii <= d->m_lastVisible; ++ii)
        if (d->m_items[ii])
            d->m_items[ii]->m_item->snap();

    d->m_highlight->snap();
    d->m_scrollbar->snap();
}

/*! \internal */
void QSmoothList::doUpdate(bool force)
{
    //causes update if using QPainter, otherwise paints with direct painter
    static QTime lastTime;
    static int cleanFrames = 0;
    static int renderTime = 0;
    static int renderFrames = 0;

    QTime t = QTime::currentTime();
    int ft = lastTime.msecsTo(t);
    lastTime = t;

    ensureVisibleItemsLoaded();

    QRegion changeRect;
    if (force || d->m_forceRedraw) {
        changeRect = rect();

        if (d->m_forceRedraw)
            d->m_forceRedraw = false;
    } else {
        for(int ii = d->m_firstVisible; ii <= d->m_lastVisible; ++ii) {
            if (d->m_items[ii]) {
                QRect rect = d->m_items[ii]->m_item->rect();
                QRect snap = d->m_items[ii]->m_item->snapRect();

                if(snap != rect ||
                   ii == d->m_focusItem ||
                   !d->m_items[ii]->m_item->itemValid()) {
                    changeRect = changeRect.unite(rect);
                    changeRect = changeRect.unite(snap);
                }
            }
        }
        changeRect = changeRect.unite(d->m_highlight->snapRect());
        changeRect = changeRect.unite(d->m_highlight->rect());

        changeRect = changeRect.unite(d->m_scrollbar->snapRect());
        changeRect = changeRect.unite(d->m_scrollbar->rect());
    }

    repaint(changeRect);

    int nt = lastTime.msecsTo(QTime::currentTime());

    if(ft  + nt > 30) {
        cleanFrames = 0;
    } else {
        cleanFrames++;
    }

    renderFrames++;
    renderTime += nt;
    if(renderFrames == 100) {
        renderTime = 0;
        renderFrames = 0;
    }
}

/*! \internal */
void QSmoothList::tryHideScrollBar()
{
    //hide scrollbar is if isn't needed
    if(!d->m_listTimeline.isActive() &&
       !d->m_highlightTimeline.isActive() &&
       !d->m_pauseTimeline.isActive())
        hideScrollBar();
}

/*! \internal */
void QSmoothList::emitActivated()
{
    if (d->m_focusItem != -1)
        emit activated(currentIndex());
}

/*! \internal */
void QSmoothList::selectTimeout()
{
    d->selectTimer.stop();
    if (d->m_mouseClick && selectionMode() != NoSelection) {
        int idx = d->indexOfPoint(d->m_origMousePos);
        // Do not 'select' an invalid index outside the list
        if ((idx != -1) && (idx != d->m_focusItem)) {
            QModelIndex oldIndex = currentIndex();
            updateCurrentIndex(idx);

            bool slow = true;
#ifndef SUPPRESS_QSMOOTHLIST_ANIMATION
            if (d->m_highlight->alpha()->value() >= nearZero) {
                // Need to fade out
                d->m_highlightAlpha.move(*d->m_highlight->alpha(), 0.0f, rate*3);
                d->m_highlightTimeline.pause(*d->m_highlight->position(), rate*3);
                slow = false;
            }
#endif

            d->m_highlightTimeline.set(*d->m_highlight->position(), d->itemPosition(idx));
            d->m_highlightHeight.set(*d->m_highlight->height(), d->itemHeight(idx));

            d->scheduleMove(d->m_highlightAlpha, *d->m_highlight->alpha(), 1.0f, slow ? rate*6 : rate*3);

            ensureCurrentItemVisible(rate*6);
        }
    }
}

/*! \internal */
void QSmoothList::updateHighlight()
{
    static int priorHeight = 0;

    int currentHeight = int(d->m_highlight->height()->value());
    if (currentHeight != priorHeight) {
        priorHeight = currentHeight;
        d->m_highlight->invalidateCache();
    }

    bool isMouseDown = !d->m_prevMousePos.isNull();
    bool isMoving = isMouseDown || d->m_listTimeline.isActive() || d->m_highlightTimeline.isActive();
    QRect rect = d->m_highlight->rect();

    if(!isMoving && rect.top() < 0) {
        d->m_listPosition.setValue(d->m_listPosition.value() + rect.top());
    } else if(!isMoving && rect.bottom() > height()) {
        d->m_listPosition.setValue(d->m_listPosition.value() + rect.bottom() - height());
    } else if (d->m_focusItem == -1) {
        doUpdate();
    }
}

/*! \reimp */
void QSmoothList::mousePressEvent(QMouseEvent *e)
{
    const bool haltFlick = d->m_inFlick;

    d->m_inFlick = false;
    d->m_listTimeline.clear();
    d->m_mouseClick = !haltFlick;
    d->m_mousePressed = true;
    d->m_origMousePos = e->pos();
    d->m_prevMousePos = e->pos();

    d->m_velocity.reset();
    if (haltFlick) {
        // Don't start a new flick with this position info - wait for some movement
    } else {
        d->m_velocity.setPosition(e->pos().y());
    }

    e->accept();

    int idx = d->indexOfPoint(e->pos());
    if (idx != -1)
        emit pressed(d->m_items.at(idx)->index(d->m_model));

    if (!haltFlick && (selectionMode() != NoSelection))
        d->selectTimer.start(selectionTimeout);
}

/*! \reimp */
void QSmoothList::mouseMoveEvent(QMouseEvent *e)
{
    QPoint pos = e->pos();
    int delta = d->m_prevMousePos.y() - pos.y();

    // No need to redraw for horizontal movements
    if (delta == 0)
        return;

    tickStart();
    d->m_listPosition.setValue(d->m_listPosition.value() + delta);
    d->m_prevMousePos = pos;

    if (d->m_velocity.positionSet()) {
        // Require at least a eighth-inch movement to indicate a flick
        static const int moveThreshold = physicalDpiY() / 8;
        if(abs(d->m_prevMousePos.y() - d->m_origMousePos.y()) >= moveThreshold) {
            d->selectTimer.stop();
            d->m_mouseClick = false;
            showScrollBar();
        }
    } else {
        // Make this the initial position
        d->m_origMousePos = pos;

        if (selectionMode() != NoSelection)
            d->selectTimer.start(selectionTimeout);
    }

    d->m_velocity.setPosition(e->pos().y());
    prepareFlick(d->m_velocity.velocity());

    doUpdate();
}

/*! \reimp */
void QSmoothList::mouseReleaseEvent(QMouseEvent *e)
{
    d->selectTimer.stop();
    d->m_mousePressed = false;
    d->m_prevMousePos = QPoint();

    e->accept();

    if (!d->validModel())
        return;

    bool fixup = true;
    if (d->m_mouseClick) {
        d->m_activatedTimeline.clear();

        int xpos = e->pos().x();
        int idx = d->indexOfPoint(e->pos());

        ItemData* data = (idx == -1 ? 0 : d->m_items.at(idx));

        // See if this was in the branch or the item
        if (data && (xpos < ((data->depth() + 1) * ListItem::LevelIndent))) {
            // Click in the branch area
            if (data->childrenState() == Expanded) {
                collapse(data->index(d->m_model));
            } else if (data->childrenState() == Collapsed) {
                expand(data->index(d->m_model));
            }

            fixup = false;
        } else {
            if (selectionMode() != NoSelection) {
                // Do not 'select' an invalid index outside the list
                if ((idx != -1) && (idx != d->m_focusItem)) {
                    QModelIndex oldIndex = currentIndex();
                    updateCurrentIndex(idx);

                    d->m_highlightAlpha.clear();
                    d->m_highlightHeight.clear();
                    d->m_highlightTimeline.clear();

                    int oldItem = d->mapToItemIndex(oldIndex);
                    bool preExisting = (oldItem != -1);
                    if (!preExisting) {
                        // Make the highlight arrive in the final location rather than moving
                        d->m_highlight->position()->setValue(d->itemPosition(idx));
                        d->m_highlight->height()->setValue(d->itemHeight(idx));
                    }

                    bool slow = true;
#ifndef SUPPRESS_QSMOOTHLIST_ANIMATION
                    if (d->m_highlight->alpha()->value() >= nearZero) {
                        // Need to fade out
                        d->m_highlightAlpha.move(*d->m_highlight->alpha(), 0.0f, rate*3);
                        d->m_highlightHeight.pause(*d->m_highlight->height(), rate*3);
                        d->m_highlightTimeline.pause(*d->m_highlight->position(), rate*3);
                        slow = false;
                    }
#endif

                    d->m_highlightTimeline.set(*d->m_highlight->position(), d->itemPosition(idx));
                    d->m_highlightHeight.set(*d->m_highlight->height(), d->itemHeight(idx));

                    d->scheduleMove(d->m_highlightAlpha, *d->m_highlight->alpha(), 1.0f, slow ? rate*6 : rate*3);

                    ensureCurrentItemVisible(rate*6);

                    d->m_activatedTimeline.pause(d->m_activatedObject, rate*6);

                    emit clicked(currentIndex());
                } else if (idx == d->m_focusItem) {
                    emit clicked(currentIndex());
                    emit activated(currentIndex());
                }
            }
        }
    } else {
        if (flickList(d->m_velocity.velocity()))
            fixup = false;
    }

    if (fixup) {
        ensureVisibleItemsLoaded(true);
        fixupPosition(rate*6);
    }

    tryHideScrollBar();
}

/*! \reimp */
void QSmoothList::moveEvent(QMoveEvent *)
{
    d->backgroundOffsetValid = false;
}

/*! \reimp */
void QSmoothList::mouseDoubleClickEvent(QMouseEvent *e)
{
    int idx = d->indexOfPoint(e->pos());
    if (idx != -1)
        emit doubleClicked(d->m_items.at(idx)->index(d->m_model));
}

/*! \internal */
void QSmoothList::prepareFlick(qreal velocity)
{
    int maxIndex = d->m_lastItemIndex;
    if (maxIndex <= 0)
        return;

    if (d->m_preloadIndex != -1) {
        // Ensure that we have preloaded in the right direction
        if (((d->m_preloadIndex < d->m_firstVisible) && (velocity > 0.0f)) ||
            ((d->m_preloadIndex > d->m_lastVisible) && (velocity < 0.0f))) {
            // We have preloaded for the other direction, so start again
            d->m_preloadIndex = -1;
        }
    }

    if (d->m_preloadIndex == -1) {
        const int listWidth = width();
        const int listHeight = height();
        const int listPosition = static_cast<int>(d->m_listPosition.value());

        // Configure the amount of preloading to perform:
        const int flickableHeight = listHeight * 3 / 5;

        // We haven't preloaded any items
        if (velocity < 0.0f) {
            // Preload items corresponding to the flickable height (as far as the
            // screen can be scrolled during a flick operation) above those visible
            const int preloadPosition = listPosition - flickableHeight;

            d->m_preloadIndex = d->m_firstVisible;
            do {
                if (d->m_preloadIndex > 0) {
                    --d->m_preloadIndex;
                    d->loadItem(d->m_preloadIndex, listWidth, false);
                } else {
                    break;
                }
            } while (d->itemPosition(d->m_preloadIndex) > preloadPosition);
        } else if (velocity > 0.0f) {
            // Preload below those visible
            const int preloadPosition = listPosition + listHeight + flickableHeight;

            d->m_preloadIndex = d->m_lastVisible;
            do {
                if (d->m_preloadIndex < maxIndex) {
                    ++d->m_preloadIndex;
                    d->loadItem(d->m_preloadIndex, listWidth, false);
                } else {
                    break;
                }
            } while (d->itemPosition(d->m_preloadIndex) < preloadPosition);
        }
    }
}

/*! \internal */
bool QSmoothList::flickList(qreal velocity)
{
    // Input velocity is in pixels/second
    if (fabs(velocity) < 50)
        return false;

    const int visibleHeight = height();
    const int listHeight = d->listHeight();

    // Don't bother with flicking if we don't even have a screenfull
    if (listHeight < visibleHeight)
        return false;

    // Fix deceleration relative to total list height - for small lists, we want
    // quicker deceleration, but for big lists we don't.
    const qreal listScreens = listHeight / visibleHeight;
    const qreal extraDeceleration = (1.0f / qMax((listScreens - 1.0f), qreal(1.0f))) * 300.0f;

    const qreal minimumDeceleration = 150.00;
    const qreal decel = (minimumDeceleration + extraDeceleration) * (velocity > 0.0f ? -1.0f : 1.0f);

    const qreal time = -1.0f * velocity / decel;
    qreal distance = velocity * time + 0.5f * decel * time * time;

    // Limit movement to half-an-item past the list end (unless positionAtCentre
    // is in use, then allow the top/bottom item to become centred)
    qreal endPosition = d->m_listPosition.value() + distance;
    qreal maxPosition = listHeight - visibleHeight;

    if (endPosition < 0.0f) {
        if (d->m_positionHint == PositionAtCenter) {
            endPosition = (visibleHeight - d->itemHeight(0)) * -0.5f;
        } else {
            endPosition = d->itemHeight(0) * -0.5f;
        }
    } else if (endPosition > maxPosition) {
        if (d->m_positionHint == PositionAtCenter) {
            endPosition = maxPosition + (visibleHeight - d->itemHeight(d->m_lastItemIndex)) * 0.5f;
        } else {
            endPosition = maxPosition + d->itemHeight(d->m_lastItemIndex) * 0.5f;
        }
    }

    distance = endPosition - d->m_listPosition.value();
    if ((distance < 0.0f) != (velocity < 0.0f))
        return false;

    d->m_position = int(endPosition);
    d->m_listTimeline.clear();
    d->m_listTimeline.accelDistance(d->m_listPosition, velocity, distance);
    d->m_inFlick = true;
    d->m_forceAlign = true;
    d->m_preloadIndex = -1;
    return true;
}

/*! \internal */
qreal QSmoothList::maxListPos() const
{
    qreal rv = d->listHeight() - height();
    if(rv < 0.0f)
        rv = 0.0f;
    return rv;
}

/*! \reimp */
void QSmoothList::wheelEvent(QWheelEvent *e)
{
    go(-e->delta(), false);
}

/*! \reimp */
void QSmoothList::keyPressEvent(QKeyEvent *e)
{   
    bool handled(false);

    if (selectionMode() != NoSelection) {
        if (e->key() == Qt::Key_Select) {
            if (d->m_focusItem != -1) {
                emit activated(currentIndex());
                handled = true;
            }
        } else if (e->key() == Qt::Key_Left || e->key() == Qt::Key_Right) {
            static const bool rightToLeftMode(QtopiaApplication::layoutDirection() == Qt::RightToLeft);
            static const Qt::Key collapseKey(rightToLeftMode ? Qt::Key_Right : Qt::Key_Left);

            QModelIndex itemIndex(currentIndex());

            if (e->key() == collapseKey) {
                if (isExpanded(itemIndex)) {
                    collapse(itemIndex);
                    handled = true;
                } else {
                    // Move to the item parent, if possible
                    QModelIndex parentIndex(itemIndex.parent());
                    if (parentIndex.isValid()) {
                        setCurrentIndex(parentIndex);
                        handled = true;
                    }
                }
            } else {
                if (ItemData *data = d->locateItem(itemIndex)) {
                    if (data->childrenState() == Collapsed) {
                        expand(itemIndex);
                        handled = true;
                    }
                }
            }
        } else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
            go(e->key() == Qt::Key_Up ? -1 : +1, e->isAutoRepeat());
            handled = true;
        }
    }

    if (handled) {
        e->accept();
    } else {
        QWidget::keyPressEvent(e);
    }
}

/*! \internal */
void QSmoothList::go(int dir, bool autoRepeatEvent)
{
#ifndef SUPPRESS_QSMOOTHLIST_ANIMATION
    static const int preScrollPeriod = rate*12;
#else
    static const int preScrollPeriod = 0;
#endif

    if (!d->validModel())
        return;

    int maxIndex = d->m_lastItemIndex;
    if ((maxIndex == 0) && (d->m_focusItem == 0))
        return;

    // Do not interrupt the wrap animation for auto-repeat key events
    if (autoRepeatEvent && d->m_inWrap)
        return;

    bool incompleteListMovement(d->m_listTimeline.isActive());
    if (incompleteListMovement) {
        // Abort the incomplete animation
        d->m_listTimeline.clear();
        d->m_listPosition.setValue(d->m_position);

        // We need to force all visible items to be redrawn, since the movement
        // resulting from animation will no longer cause them to be repainted
        d->m_forceRedraw = true;
    }

    qreal newHighlightPos;
    qreal newHighlightHeight;
    qreal newHighlightAlpha = d->m_highlight->alpha()->value();

    int visibleHeight = height();

    QModelIndex oldIndex = currentIndex();
    int focusIndex = d->m_focusItem;

    bool noPreviousFocus(focusIndex == -1);

    if (noPreviousFocus) {
        // Just go back to the top
        focusIndex = 0;
        newHighlightAlpha = 1.0f;
    } else if (dir < 0) {
        if (focusIndex > 0) {
            focusIndex -= 1;
        } else if (autoRepeatEvent) {
            // Do not allow wrap - no movement possible
            return;
        } else {
            updateCurrentIndex(maxIndex);

            // Only re-locate the list position if the list size exceeds the visible area
            bool relocate(d->listHeight() > visibleHeight);

            if (relocate) {
                // Ensure all items at the low end are loaded
                int firstVisible = 0;
                int finalListPosition = d->listHeight() - visibleHeight;
                do {
                    // Loop over this, since loading may change list height and item heights
                    firstVisible = d->firstVisibleIndex(visibleHeight, finalListPosition);
                    for (int ii = firstVisible; ii <= d->m_focusItem; ++ii)
                        d->loadItem(ii, width(), false);

                    finalListPosition = d->listHeight() - visibleHeight;
                } while (firstVisible != d->firstVisibleIndex(visibleHeight, finalListPosition));
            }

            newHighlightPos = d->itemPosition(d->m_focusItem);
            newHighlightHeight = d->itemHeight(d->m_focusItem);

            if (relocate) {
                d->m_position = d->listHeight() - visibleHeight;

                if (preScrollPeriod) {
                    d->m_highlightAlpha.clear();
                    d->m_highlightAlpha.pause(*d->m_highlight->alpha(), preScrollPeriod);
                    d->m_highlightAlpha.set(*d->m_highlight->alpha(), 0.0f);
                    d->m_highlightAlpha.move(*d->m_highlight->alpha(), 1.0f, rate*6);

                    d->m_highlightHeight.clear();
                    d->m_highlightHeight.pause(*d->m_highlight->height(), preScrollPeriod);
                    d->m_highlightHeight.set(*d->m_highlight->height(), newHighlightHeight);

                    d->m_highlightTimeline.clear();
                    d->m_highlightTimeline.pause(*d->m_highlight->position(), preScrollPeriod);
                    d->m_highlightTimeline.set(*d->m_highlight->position(), newHighlightPos);

                    d->m_listTimeline.clear();
                    d->m_listTimeline.move(d->m_listPosition, -visibleHeight - 20, preScrollPeriod); //20 = magic number
                    d->m_listTimeline.set(d->m_listPosition, d->listHeight());
                    d->m_listTimeline.accelDistance(d->m_listPosition, -1250, -visibleHeight);
                    d->m_inWrap = true;
                } else {
                    d->m_listTimeline.clear();
                    d->m_listPosition.setValue(d->m_position);

                    fixupHighlight();
                    doUpdate(true);
                }

                // Show scrollbar after re-positioning
                showScrollBar(preScrollPeriod);
                d->m_pauseTimeline.clear();
                d->m_pauseTimeline.pause(d->m_pauseObject, preScrollPeriod + 1000);
            } else {
#ifndef SUPPRESS_QSMOOTHLIST_ANIMATION
                // Hide the highlight while we re-position it
                d->m_highlightAlpha.clear();
                d->m_highlightAlpha.move(*d->m_highlight->alpha(), 0.0f, rate*3);
                d->m_highlightAlpha.move(*d->m_highlight->alpha(), 1.0f, rate*3);

                d->m_highlightTimeline.clear();
                d->m_highlightTimeline.pause(*d->m_highlight->position(), rate*3);
                d->m_highlightTimeline.set(*d->m_highlight->position(), newHighlightPos);

                if (newHighlightHeight != d->m_highlight->height()->value()) {
                    d->m_highlightHeight.clear();
                    d->m_highlightHeight.pause(*d->m_highlight->height(), rate*3);
                    d->m_highlightHeight.set(*d->m_highlight->height(), newHighlightHeight);
                }
#else
                fixupHighlight();
#endif
            }
            return;
        }
    } else if (dir > 0) {
        if (focusIndex < maxIndex) {
            focusIndex += 1;
        } else if (autoRepeatEvent) {
            // Do not allow wrap - no movement possible
            return;
        } else {
            updateCurrentIndex(0);

            // Only re-locate the list position if the list size exceeds the visible area
            bool relocate(d->listHeight() > visibleHeight);

            if (relocate) {
                // Ensure all items at the high end are loaded
                int lastVisible = 0;
                do {
                    // Loop over this, since loading may change list height and item heights
                    lastVisible = d->lastVisibleIndex(visibleHeight, 0);
                    for (int ii = 0; ii <= lastVisible; ++ii)
                        d->loadItem(ii, width(), false);
                } while (lastVisible != d->lastVisibleIndex(visibleHeight, 0));
            }

            newHighlightPos = d->itemPosition(d->m_focusItem);
            newHighlightHeight = d->itemHeight(d->m_focusItem);

            if (relocate) {
                d->m_position = 0;

                if (preScrollPeriod) {
                    d->m_highlightAlpha.clear();
                    d->m_highlightAlpha.pause(*d->m_highlight->alpha(), preScrollPeriod);
                    d->m_highlightAlpha.set(*d->m_highlight->alpha(), 0.0f);
                    d->m_highlightAlpha.move(*d->m_highlight->alpha(), 1.0f, rate*6);

                    d->m_highlightHeight.clear();
                    d->m_highlightHeight.pause(*d->m_highlight->height(), preScrollPeriod);
                    d->m_highlightHeight.set(*d->m_highlight->height(), newHighlightHeight);

                    d->m_highlightTimeline.clear();
                    d->m_highlightTimeline.pause(*d->m_highlight->position(), preScrollPeriod);
                    d->m_highlightTimeline.set(*d->m_highlight->position(), newHighlightPos);

                    d->m_listTimeline.clear();
                    d->m_listTimeline.move(d->m_listPosition, d->listHeight() + visibleHeight + 20, preScrollPeriod);   //20 = magic number
                    d->m_listTimeline.set(d->m_listPosition, -visibleHeight);
                    d->m_listTimeline.accelDistance(d->m_listPosition, 1250, visibleHeight);
                    d->m_inWrap = true;
                } else {
                    d->m_listTimeline.clear();
                    d->m_listPosition.setValue(d->m_position);

                    fixupHighlight();
                    doUpdate(true);
                }

                // Show scrollbar after re-positioning
                showScrollBar(preScrollPeriod);
                d->m_pauseTimeline.clear();
                d->m_pauseTimeline.pause(d->m_pauseObject, preScrollPeriod + 1000);
            } else {
#ifndef SUPPRESS_QSMOOTHLIST_ANIMATION
                // Hide the highlight while we re-position it
                d->m_highlightAlpha.clear();
                d->m_highlightAlpha.move(*d->m_highlight->alpha(), 0.0f, rate*3);
                d->m_highlightAlpha.move(*d->m_highlight->alpha(), 1.0f, rate*3);

                d->m_highlightTimeline.clear();
                d->m_highlightTimeline.pause(*d->m_highlight->position(), rate*3);
                d->m_highlightTimeline.set(*d->m_highlight->position(), newHighlightPos);

                if (newHighlightHeight != d->m_highlight->height()->value()) {
                    d->m_highlightHeight.clear();
                    d->m_highlightHeight.pause(*d->m_highlight->height(), rate*3);
                    d->m_highlightHeight.set(*d->m_highlight->height(), newHighlightHeight);
                }
#else
                fixupHighlight();
#endif
            }
            return;
        }
    }

    if (focusIndex != -1) {
        updateCurrentIndex(focusIndex);

        // Ensure the item we're moving to is loaded
        d->loadItem(d->m_focusItem, width(), false);

        bool animate(!noPreviousFocus && !incompleteListMovement);

        ensureCurrentItemVisible(animate ? rate*6 : 0);
        fixupHighlight(animate ? rate*6 : 0);

        showScrollBar();
        d->m_pauseTimeline.clear();
        d->m_pauseTimeline.pause(d->m_pauseObject, 500);
    }
}

/*!
    Sets the string \a text to be displayed instead of the list, when no items are present.
*/
void QSmoothList::setEmptyText(const QString &text)
{
    d->m_emptyText = text;
}

/*!
    Sets the list items to be painted with opaque backgrounds, if \a opaque is true.
    Otherwise, the list items will be painted with transparent backgrounds.
*/
void QSmoothList::setOpaqueItemBackground(bool opaque)
{
    d->m_opaqueItems = opaque;
}

/*!
    Returns true if the list has been set to paint items with opaque backgrounds.
*/
bool QSmoothList::hasOpaqueItemBackground() const
{
    return d->m_opaqueItems;
}

/*! \reimp */
QSize QSmoothList::sizeHint() const
{
    QSize sz(100, 3 * (style()->pixelMetric(QStyle::PM_ListViewIconSize) + 2));

    if (d->validModel()) {
        QModelIndex idx = d->m_model->index(0, 0);
        QStyleOptionViewItemV3 opt = d->defaultoption;
        opt.rect = QRect(0, 0, width(), d->m_itemHeightEstimate);
        sz = d->m_delegate->sizeHint(opt, idx);
        sz.setHeight(sz.height()*3);
    }

    return sz;
}

/*!
    Sets the item referred to by \a index to either collapsed or expanded, depending on the value of \a set.

    \sa expanded(), expand(), isExpanded()
*/
void QSmoothList::setExpanded(const QModelIndex& index, bool set)
{
    d->expandItem(index, set, this);
}

/*!
    Returns true if the model item \a index is expanded; otherwise returns false.

    \sa expand(), expanded(), setExpanded()
*/
bool QSmoothList::isExpanded(const QModelIndex& index) const
{
    return d->isExpanded(index);
}

/*!
    Expands the model item specified by the \a index.

    \sa expanded()
*/
void QSmoothList::expand(const QModelIndex& index)
{
    d->expandItem(index, true, this);
    doUpdate(true);
}

/*!
    Collapses the model item specified by the \a index.

    \sa collapsed()
*/
void QSmoothList::collapse(const QModelIndex& index)
{
    d->expandItem(index, false, this);
    doUpdate(true);
}

/*!
    Expands all expandable items.

    \sa collapseAll(), expand(), collapse(), setExpanded()
*/
void QSmoothList::expandAll()
{
    d->expandAll(true, this);
    doUpdate(true);
}

/*!
    Collapses all expanded items.

    \sa expandAll(), expand(), collapse(), setExpanded()
*/
void QSmoothList::collapseAll()
{
    d->expandAll(true, this);
    doUpdate(true);
}

QTOPIA_EXPORT int qtopia_background_brush_rotation(int screen);

static QMatrix brushRotationMatrix(int rotation, const QPoint& offset, const QImage& image)
{
    QMatrix matrix;

    // Note: QMatrix transformations are applied in the reverse
    // of the order listed below.

    // Translate the image to its final location, accounting for the window offset.
    matrix.translate(offset.x(), offset.y());

    // Translate the rotated image to the screen mid-point.
    switch (rotation) {
        case  0: case 180: matrix.translate(image.width() / 2, image.height() / 2); break;
        case 90: case 270: matrix.translate(image.height() / 2, image.width() / 2); break;
    }

    // Rotate the image about its mid-point.
    matrix.rotate(rotation);

    // Translate the mid-point of the image to the origin.
    matrix.translate(-image.width() / 2, -image.height() / 2);

    return matrix;
}

/*! \reimp */
void QSmoothList::paintEvent(QPaintEvent *pe)
{
    static enum { Perf, NoPerf, NoPaint, Unknown } displayPerf = Unknown;

    if (d->m_unhandledResize) {
        const int currentHeight(height());
        const int currentWidth(width());

        const bool heightChanged(currentHeight != d->m_previousSize.height());
        const bool widthChanged(currentWidth != d->m_previousSize.width());

        if (heightChanged) {
            d->m_scrollbar->setHeight(currentHeight);
            d->m_scrollbar->setListHeight(d->m_model ? d->listHeight() : currentHeight);

            d->m_highlight->setWidth(currentWidth);

            // If the delegate sizes its items depending on list size, then loaded items are now wrong
            d->unloadAllItems();
            ensureVisibleItemsLoaded(true);

            // The position hint may also cause different positioning
            if (d->m_focusItem != -1)
                scrollTo(d->m_focusItem, d->m_positionHint);
        }

        if (widthChanged) {
            // The width must have changed
            d->m_highlight->setWidth(currentWidth);

            static const bool rightToLeftMode(QtopiaApplication::layoutDirection() == Qt::RightToLeft);
            d->m_scrollbar->setPos(rightToLeftMode ? 0 : currentWidth - Scrollbar::BarWidth);

            // Adjust width of loaded items
            if (!heightChanged) {
                for (int ii = 0; ii < d->m_items.count(); ++ii)
                    if (d->m_items.at(ii))
                        d->m_items.at(ii)->m_item->setWidth(currentWidth);
            }
        }

        d->m_unhandledResize = false;
        d->m_previousSize = QSize(currentWidth, currentHeight);
    }

    if(displayPerf == Unknown) {
        QString str(getenv("QTOPIA_SMOOTHLIST_PERF"));
        str = str.toLower();
        if(str.isEmpty())
            displayPerf = NoPerf;
        else if(str == "nopaint")
            displayPerf = NoPaint;
        else
            displayPerf = Perf;
    }

    if (d->m_items.count() == 0) {
        QPainter painter(this);
        if(d->background.isNull()) {
            painter.fillRect(QRect(0, 0, width(), height()), d->backColor);
        } else {
            if (!d->backgroundOffsetValid) {
                d->backgroundOffset = -mapToGlobal(QPoint(0,0));
                d->backgroundOffsetValid = true;
            }
            int screen = QApplication::desktop()->screenNumber(this);
            int rotation = qtopia_background_brush_rotation(screen);
            if (rotation != 0) {
                QMatrix matrix = brushRotationMatrix
                    (rotation, d->backgroundOffset, d->background);
                painter.setMatrix(matrix);
                painter.drawImage(QPoint(0, 0), d->background);
                painter.setMatrix(QMatrix());
            } else {
                painter.drawImage(d->backgroundOffset, d->background);
            }
        }
        painter.drawText(rect(), Qt::AlignCenter | Qt::AlignHCenter | Qt::TextWordWrap, d->m_emptyText);
        return;
    }

    GfxPainter p(this, pe);
    if(NoPaint != displayPerf) {
        if(d->background.isNull()) {
            p.fillRect(QRect(0, 0, width(), height()), d->backColor);
        } else {
            if (!d->backgroundOffsetValid) {
                d->backgroundOffset = -mapToGlobal(QPoint(0,0));
                d->backgroundOffsetValid = true;
            }
            int screen = QApplication::desktop()->screenNumber(this);
            int rotation = qtopia_background_brush_rotation(screen);
            if (rotation != 0) {
                QMatrix matrix = brushRotationMatrix
                    (rotation, d->backgroundOffset, d->background);
                p.drawImageTransformed(matrix, d->background);
            } else {
                p.drawImage(d->backgroundOffset, d->background);
            }
        }
        d->m_highlight->paint(&p);

        for(int ii = d->m_firstVisible; ii <= d->m_lastVisible; ++ii)
            if (d->m_items.at(ii))
                d->m_items.at(ii)->m_item->paint(&p);

        if (d->m_scrollbar->barHeight() < height())
            d->m_scrollbar->paint(&p);
    }

    if(NoPerf != displayPerf) {
        d->m_perfFrames++;
        int elapsed = d->m_perfTime.elapsed();
        if(elapsed >= 1000) {
            qreal fps = qreal(d->m_perfFrames) * 1000. / qreal(elapsed);
            qreal mspf = qreal(elapsed) / qreal(d->m_perfFrames);
            QString disp = QString::number(int(fps)) + " " + QString::number(mspf, 'g', 3);
            d->m_fpsImage = GfxPainter::string(disp);
            d->m_perfTime.restart();
            d->m_perfFrames = 0;
        }

        if(!d->m_fpsImage.isNull()) {
            if(NoPaint == displayPerf)
                p.fillRect(QRect(QPoint(width() - d->m_fpsImage.width(), 0),
                                  d->m_fpsImage.size()), Qt::white);
            p.drawImage(width() - d->m_fpsImage.width(), 0, d->m_fpsImage);
        }
        if(hasFocus())
            QWidget::update(QRect(QPoint(width() - d->m_fpsImage.width(), 0),
                                  d->m_fpsImage.size()));
    }
}

/*! \reimp */
bool QSmoothList::event(QEvent *event)
{
    switch(event->type()) {
        case QEvent::PaletteChange:
            refresh(false);
            d->backColor = palette().brush(QPalette::Window).color();
            break;
        default:
            break;
    }
    return QWidget::event(event);
}

/*! \reimp */
void QSmoothList::resizeEvent(QResizeEvent *)
{
    d->m_unhandledResize = true;
    doUpdate(true);
}

/*!
  Sets the \a model for the list to present.

  The current selection index is reset.
*/
void QSmoothList::setModel(QAbstractItemModel *model)
{
    if (d->m_model) {
        disconnect(d->m_model, 0, this, 0);
        d->releaseModel();
    }

    d->m_model = model;
    d->m_focusItem = -1;
    d->m_unhandledResize = false;
    d->m_modificationTime = QTime::currentTime();

    d->syncToModel();

    if (d->m_model) {
        connect(d->m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(dataChanged(QModelIndex,QModelIndex)));
        connect(d->m_model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(rowsInserted(QModelIndex,int,int)));
        connect(d->m_model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(rowsRemoved(QModelIndex,int,int)));
        connect(d->m_model, SIGNAL(modelAboutToBeReset()),
                this, SLOT(modelAboutToBeReset()));
        connect(d->m_model, SIGNAL(modelReset()),
                this, SLOT(modelReset()));

        if ((d->m_model->rowCount() > 0) && (d->selectionMode != NoSelection)) {
            // Re-estimate the height for unloaded items
            d->estimateItemHeight(width());

            d->m_positionHint = QSmoothList::EnsureVisible;
            updateCurrentIndex(0);
        }
    }

    reset();
}

/*! \internal */
void QSmoothList::refresh(bool animated)
{
    if (!isVisible() && !d->m_model) {
        d->defaultViewOptionsV3(this); //re-initialize view options
        d->m_focusItem = -1;
        d->m_needRefreshOnShow = true;
        return;
    }

    if (d->m_lastItemIndex != -1) {
        d->invalidateCache();
    }

    // Without any rows, any existing selection is invalid
    int oldFocusItem = d->m_focusItem;
    int oldListPosition = static_cast<int>(d->m_listPosition.value());

    init();

    int rowCount = d->m_model ? d->m_model->rowCount() : 0;
    if (rowCount > 0) {
        if (d->m_lastItemIndex == -1) {
            oldFocusItem = -1;
        }

        // don't lose the selection when we leave the list and come back
        int idx = oldFocusItem;
        if (selectionMode() == NoSelection) {
            idx = -1;
        } else {
            if (!d->validIndex(idx))
                idx = 0;
        }

        updateCurrentIndex(idx);

        d->m_highlightAlpha.clear();
        d->m_highlightHeight.clear();
        d->m_highlightTimeline.clear();

        if (-1 != idx) {
            // We have discarded any item height info we previously stored;
            // ensure all items visible before the selection are loaded
            d->loadItem(idx, width(), false);

            int firstVisible = 0;
            int finalListPosition = d->itemPosition(idx) + d->itemHeight(idx) - height();
            do {
                // Loop over this, since loading may change list height and item heights
                firstVisible = d->firstVisibleIndex(height(), finalListPosition);
                for (int ii = firstVisible; ii < idx; ++ii)
                    d->loadItem(ii, width(), false);

                finalListPosition = d->itemPosition(idx) + d->itemHeight(idx) - height();
            } while (firstVisible != d->firstVisibleIndex(height(), finalListPosition));

            int itemPosition = d->itemPosition(idx);
            int itemHeight = d->itemHeight(idx);

#ifndef SUPPRESS_QSMOOTHLIST_ANIMATION
            if (animated) {
                d->m_highlightTimeline.set(*d->m_highlight->position(), itemPosition);
                d->m_highlightHeight.set(*d->m_highlight->height(), itemHeight);
                d->m_highlightAlpha.move(*d->m_highlight->alpha(), 1.0f, rate*6);

                ensureCurrentItemVisible(rate*6);
            } else {
#endif
                // Jump to the required location directly
                int listPosition = itemPosition;

                // See if we can leave the list at the prior position
                if ((oldListPosition <= listPosition) &&
                    ((oldListPosition + height()) >= (listPosition + itemHeight))) {
                    // The prior position does not need to be changed
                    listPosition = oldListPosition;
                }

                if (d->m_forceAlign) {
                    d->m_forceAlign = false;

                    // Ensure we fill the screen if possible
                    if ((listPosition + height()) > d->listHeight())
                        listPosition = d->listHeight() - height();
                    if (listPosition < 0)
                        listPosition = 0;
                }

                d->m_listPosition.setValue(listPosition);

                d->m_highlight->position()->setValue(itemPosition);
                d->m_highlight->height()->setValue(itemHeight);
                d->m_highlight->alpha()->setValue(1.0f);
#ifndef SUPPRESS_QSMOOTHLIST_ANIMATION
            }
#else
            Q_UNUSED(animated)
#endif
        }
    } else if (d->m_focusItem != -1) {
        d->m_positionHint = QSmoothList::EnsureVisible;
        updateCurrentIndex(-1);
    }

    d->m_needRefreshOnShow = false;
    doUpdate(true);
}

/*!
  Reset the internal state of the list.
*/
void QSmoothList::reset()
{
    refresh();

    if (!isVisible()) {
        d->m_previouslyShown = false;
    }
}

/*!
    Returns the mode used to elide text when painting list items.
*/
Qt::TextElideMode QSmoothList::textElideMode() const
{
    return d->textElideMode;
}

/*!
    Sets the mode used to elide text when painting list items to \a mode.
*/
void QSmoothList::setTextElideMode(Qt::TextElideMode mode)
{
    if (d->textElideMode != mode) {
        d->textElideMode = mode;
        reset();
    }
}

/*!
    Returns the model index of the current item, if any.

    \sa setCurrentIndex()
*/
QModelIndex QSmoothList::currentIndex() const
{
    if ((d->m_focusItem == -1) || !d->validIndex(d->m_focusItem)) {
        return QModelIndex();
    } else {
        if (!d->m_items.at(d->m_focusItem)) {
            d->loadItem(d->m_focusItem, width());
        }

        return d->m_items.at(d->m_focusItem)->index(d->m_model);
    }
}

/*!
    Returns the model index of the item at the viewport coordinates \a point.

    \sa visualRect()
*/
QModelIndex QSmoothList::indexAt(const QPoint &point) const
{
    int idx = d->indexOfPoint(point);
    if (idx != -1)
        return d->m_items.at(idx)->index(d->m_model);

    return QModelIndex();
}

/*!
    Returns the item delegate used by this view and model. This is
    either one set with setItemDelegate(), or the default one.

    \sa setItemDelegate()
*/
QAbstractItemDelegate *QSmoothList::itemDelegate() const
{
    return d->m_delegate;
}

/*!
    Returns the model that this list is viewing.
*/
QAbstractItemModel *QSmoothList::model() const
{
    return d->m_model;
}

/*!
    Returns the rectangle on the viewport occupied by the item at \a index.

    \sa indexAt()
*/
QRect QSmoothList::visualRect(const QModelIndex &index) const
{
    if (index.isValid())
        if (ItemData* item = d->m_items.at(d->mapToItemIndex(index)))
            return item->m_item->rect() & rect();

    return QRect();
}

/*!
    Returns the selection mode of the list.
*/
QSmoothList::SelectionMode QSmoothList::selectionMode() const
{
    return d->selectionMode;
}

/*!
    Sets the selection mode of the list to \a mode.
    Multiple-item selection modes are not supported by QSmoothList.
*/
void QSmoothList::setSelectionMode(SelectionMode mode)
{
    d->selectionMode = mode;
    if(mode == NoSelection)
        clearSelection();
}

/*! \internal */
void QSmoothList::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (!d->validModel())
        return;

    int start = qMax(d->mapToItemIndex(topLeft), 0);
    int end = qMin(d->mapToItemIndex(bottomRight), d->m_lastItemIndex);
    if ((start == -1) || (end == -1))
        return;

    for (int ii = start; ii <= end; ++ii)
        invalidateItem(ii);

    // If any of these items was visible, we need to redraw
    if (end >= d->m_firstVisible && start <= d->m_lastVisible)
        doUpdate(false);
}

/*! \internal */
void QSmoothList::backgroundChanged(const QPixmap &n)
{
    d->background = n.toImage();
    QWidget::update();
}

/*! \internal */
void QSmoothList::rowsInserted(const QModelIndex &index, int first, int last)
{
    if (!d->m_doneInit || !d->m_model)
        return;

    if (!d->insertRows(index, first, last))
        return;

    bool preExisting(d->m_rowCount > 0);

    if (!index.isValid())
        d->m_rowCount += (last - first + 1);

    // Find whether these rows are visible
    int start = d->mapToItemIndex(index, first);
    int end = d->mapToItemIndex(index, last);
    if ((start == -1) || (end == -1))
        return;

    d->insertItems(start, end);
    d->m_scrollbar->setListHeight(d->listHeight());

    // Ensure visible items are loaded
    ensureVisibleItemsLoaded();

    // If any of these items was visible, we need to redraw
    bool redrawRequired(false);
    if (end >= d->m_firstVisible && start <= d->m_lastVisible)
        redrawRequired = true;

    if (d->m_focusItem >= start) {
        // Our selected item has not changed, but has moved to a new index
        d->m_focusItem += (end - start + 1);
        d->loadItem(d->m_focusItem, width());
        setCurrentIndex(d->m_items.at(d->m_focusItem)->index(d->m_model));
    } else if (!preExisting) {
        // Re-estimate the height for unloaded items
        d->estimateItemHeight(width());

        // If these are the first rows in a single-selection list, select row 0
        if ((selectionMode() != NoSelection) && (d->m_focusItem == -1)) {
            d->loadItem(0, width());
            setCurrentIndex(d->m_items.at(0)->index(d->m_model));
        }

        redrawRequired = true;
    }

    if (redrawRequired && d->m_focusItem >= 0) {
        QTime now(QTime::currentTime());
        bool animate(!d->m_highlightTimeline.isActive() && (d->m_modificationTime.msecsTo(now) < rate*6));
        const int period = (animate ? rate*6 : 0);

        // Ensure the highlight is correctly sized and placed for the selected item
        ensureCurrentItemVisible(period);
        fixupHighlight(period);
        fixupPosition(period);

        d->m_modificationTime = now;
    }

    doUpdate(true);
}

/*! \internal */
void QSmoothList::rowsRemoved(const QModelIndex &index, int first, int last)
{
    if (!d->validModel())
        return;

    // Map these items to their visible indexes, before we remove them
    int start = d->mapToItemIndex(index, first);
    int end = d->mapToItemIndex(index, last);

    if (end != -1) {
        // Any visible descendants of the last item must also be removed
        if (ItemData* item = d->referenceItem(end, width())) {
            end += item->visibleDescendants();
        }
    }

    // Remove the objects from our indexing
    if (!d->removeRows(index, first, last))
        return;

    if (!index.isValid())
        d->m_rowCount -= (last - first + 1);

    if ((start == -1) || (end == -1))
        return;

    // If any of these items was visible, we need to redraw
    bool redrawRequired(false);
    if (end >= d->m_firstVisible && start <= d->m_lastVisible)
        redrawRequired = true;

    d->removeItems(start, end);
    d->m_scrollbar->setListHeight(d->listHeight());

    // Ensure visible items are loaded
    ensureVisibleItemsLoaded();

    if (d->m_rowCount == 0) {
        if (d->m_focusItem != -1) {
            clearSelection();
        }
    } else {
        if (d->m_focusItem >= start) {
            if (d->m_focusItem <= end) {
                // The removal has removed our focus item - select the replacement at the same position
                d->m_focusItem = qMin(start, d->m_lastItemIndex);

                d->loadItem(d->m_focusItem, width());
                QModelIndex newIndex(d->m_items.at(d->m_focusItem)->index(d->m_model));
                setCurrentIndex(newIndex);

                //current row may not have changed but item under index has changed
                emit currentChanged(newIndex, newIndex);
            } else {
                // Our selected item has not changed, but has moved to a new index
                d->m_focusItem -= (end - start + 1);
            }

            redrawRequired = true;
        }

        if (redrawRequired && d->m_focusItem >= 0) {
            QTime now(QTime::currentTime());
            bool animate(!d->m_highlightTimeline.isActive() && (d->m_modificationTime.msecsTo(now) < rate*6));
            const int period = (animate ? rate*6 : 0);

            // Ensure the highlight is correctly sized and placed for the selected item
            ensureCurrentItemVisible(period);
            fixupHighlight(period);
            fixupPosition(period);

            d->m_modificationTime = now;
        }
    }

    doUpdate(true);
}

/*! \internal */
void QSmoothList::modelAboutToBeReset()
{
}

/*! \internal */
void QSmoothList::modelReset()
{
    // Stop any animations
    d->m_listTimeline.clear();
    d->m_pauseTimeline.clear();
    d->m_highlightAlpha.clear();
    d->m_highlightHeight.clear();
    d->m_highlightTimeline.clear();

    d->releaseModel();
    d->syncToModel();

    reset();

    // Show the scrollbar to indicate current list position
    showScrollBar();
    d->m_pauseTimeline.pause(d->m_pauseObject, 1000);
    d->m_modificationTime = QTime::currentTime();
}

/*! \internal */
void QSmoothList::updateCurrentIndex(int row)
{
    if (row != d->m_focusItem) {
        if (!d->m_doneInit) {
            d->m_focusItem = row;
        } else {
            // Force the items to redraw, in case they are affected by selection state
            if (d->m_focusItem != -1)
                invalidateItem(d->m_focusItem);
            if (row != -1)
                invalidateItem(row);

            QModelIndex oldIndex = currentIndex();
            d->m_focusItem = row;

            emit currentChanged(currentIndex(), oldIndex);
            doUpdate(true);
        }
    }
}

/*!
    Deselects the selected item, if any.
*/
void QSmoothList::clearSelection()
{
    if (d->m_focusItem != -1) {
        d->m_positionHint = QSmoothList::EnsureVisible;
        updateCurrentIndex(-1);
    }

    d->m_highlightAlpha.clear();
    d->m_highlightHeight.clear();
    d->m_highlightTimeline.clear();

    if (d->m_highlight->alpha()->value() >= nearZero) {
        // Need to fade out
        d->scheduleMove(d->m_highlightAlpha, *d->m_highlight->alpha(), 0.0f, rate*3);
    }
}

/*!
    Sets the current item to be the item at \a index, and if necessary scrolls
    the list to position the item according to \a hint.  The item will be
    selected unless selectionMode() is \c NoSelection.

    \sa currentIndex(), currentChanged(), selectionMode()
*/
void QSmoothList::setCurrentIndex(const QModelIndex &index, ScrollHint hint)
{
    if (!index.isValid()) {
        clearSelection();
        return;
    }

    if (selectionMode() != NoSelection) {
        updateCurrentIndex(d->mapToItemIndex(index));
    }

    if (d->m_previouslyShown) {
        scrollTo(index, hint);
    } else {
        d->m_needRefreshOnShow = true;
        d->m_forceAlign = true;
    }

    d->m_positionHint = hint;
}

/*!
    Updates the area occupied by the given \a index.
*/
void QSmoothList::update(const QModelIndex &index)
{
    QWidget::update(visualRect(index));
}

/*!
    Sets the item delegate for this list to \a delegate.  This is useful if you
    want complete control over the editing and display of items.

    Setting this property when the view is visible will cause the items to be laid out again.

    \sa itemDelegate()
*/
void QSmoothList::setItemDelegate(QAbstractItemDelegate *delegate)
{
    if(!delegate)
        delegate = standardDelegate();
    d->m_delegate = delegate;

    // Re-estimate the height for unloaded items
    d->estimateItemHeight(width());

    refresh(false);
}

/*!
    Returns the size at which icons should be drawn when painting list items.
*/
QSize QSmoothList::iconSize() const
{
    return d->m_iconSize;
}

/*!
    Sets the size at which icons should be drawn when painting list items to \a size.

    Setting this property when the view is visible will cause the items to be laid out again.
*/
void QSmoothList::setIconSize(const QSize &size)
{
    d->m_iconSize = size;
    d->defaultoption.decorationSize = d->m_iconSize;

    // Re-estimate the height for unloaded items
    d->estimateItemHeight(width());

    // Invalidate the visible items and redraw
    d->invalidateVisibleItems();
    if (isVisible())
        doUpdate(true);

    if (d->validIndex(d->m_focusItem)) {
        // Ensure the highlight is sized to match its element
        d->m_highlight->position()->setValue(d->itemPosition(d->m_focusItem));
        d->m_highlight->height()->setValue(d->itemHeight(d->m_focusItem));
    }
}

/*! \reimp */
void QSmoothList::showEvent(QShowEvent *e)
{
    d->m_previouslyShown = true;

    if (d->m_needRefreshOnShow) {
        refresh(false);
    } else {
        ensureCurrentItemVisible();
        ensureVisibleItemsLoaded(true);
    }

    QWidget::showEvent(e);

    // Show the scrollbar to indicate current list position
    showScrollBar();
    d->m_pauseTimeline.clear();
    d->m_pauseTimeline.pause(d->m_pauseObject, 1000);
}

/*! \reimp */
void QSmoothList::hideEvent(QHideEvent *e)
{
    // Unload any items we have loaded to free up memory
    d->setAllItemsUnloadable();
    for (int ii = 0; ii <= d->m_lastItemIndex; ++ii)
        if (d->m_items.at(ii))
            d->unloadItem(ii);

    QWidget::hideEvent(e);
}

/*! \internal */
void QSmoothList::init()
{
    d->defaultViewOptionsV3(this); //re-initialize view options

    d->m_highlight->setWidth(width());
    d->m_highlight->position()->setValue(0.0f);
    d->m_highlight->alpha()->setValue(0.0f);

    // Re-estimate the height for unloaded items
    d->estimateItemHeight(width());

    static const bool rightToLeftMode(QtopiaApplication::layoutDirection() == Qt::RightToLeft);
    d->m_scrollbar->setPos(rightToLeftMode ? 0 : width() - Scrollbar::BarWidth);
    d->m_scrollbar->setHeight(height());
    d->m_scrollbar->setListHeight(d->m_model ? d->listHeight() : height());

    d->m_listPosition.addGfxValue(d->m_scrollbar->listPosition());
    d->m_listPosition.setValue(0.0f);

    d->m_doneInit = true;
}

/*! \internal */
void QSmoothList::ensureVisibleItemsLoaded(bool force)
{
    if (!d->validModel())
        return;

    int currentHeight = height();
    int currentListHeight = d->listHeight();
    int currentPosition = static_cast<int>(d->m_listPosition.value());

    if (force ||
        currentHeight != d->m_lastFillHeight ||
        currentListHeight != d->m_lastFillListHeight ||
        currentPosition != d->m_lastFillPosition) {
        d->m_lastFillHeight = currentHeight;
        d->m_lastFillListHeight = currentListHeight;
        d->m_lastFillPosition = currentPosition;

        d->loadVisibleItems(currentHeight, width());
    }
}

/*!
    Scrolls the list if necessary to ensure that the item at \a index is visible.
    The list will try to position the item according to the given \a hint.
*/
void QSmoothList::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    return scrollTo(d->mapToItemIndex(index), hint);
}

/*! \internal */
void QSmoothList::scrollTo(int idx, ScrollHint hint)
{
    if (!d->validIndex(idx))
        return;

    int visibleHeight = height();
    int visibleWidth = width();

    // Load the target item, to ensure we have an estimate for the position
    d->loadItem(idx, visibleWidth, false);

    int idxPos = d->itemPosition(idx);
    int itemHeight = d->itemHeight(idx);

    qreal listpos = d->m_listPosition.value();
    int initialPos = int(listpos);

    int maxTraversal = visibleHeight * 3;
    bool truncated(false);
    if (abs(initialPos - idxPos) > maxTraversal) {
        // We would traverse more than three screen heights - just animate the
        // last part of the scroll movement to indicate which way we have moved
        initialPos = (idxPos > initialPos ? (idxPos - maxTraversal) : (idxPos + maxTraversal));
        truncated = true;
    }

    // Load all items to be traversed, so that we know the final position of the target item
    int current = d->firstVisibleIndex(visibleHeight, initialPos);
    if (current < idx) {
        for (int ii = current; ii <= idx; ++ii)
            d->loadItem(ii, visibleWidth, false);
    } else {
        for (int ii = current; ii >= idx; --ii)
            d->loadItem(ii, visibleWidth, false);

        // Find where the item would move to if it were placed at the bottom of the list
        int highest = d->firstVisibleIndex(visibleHeight, d->itemPosition(idx) + visibleHeight);
        for (int ii = highest; ii < idx; ++ii)
            d->loadItem(ii, visibleWidth, false);
    }

    // These values may have been adjusted
    idxPos = d->itemPosition(idx);
    itemHeight = d->itemHeight(idx);

    switch(hint) {
        case ImmediateVisible:
        case EnsureVisible: {
            if (initialPos > idxPos) {
                // Put item at top
                listpos = idxPos;
            } else if ((initialPos + visibleHeight) < (idxPos + itemHeight)) {
                // Put item at bottom
                listpos = idxPos - visibleHeight + itemHeight;
            }
            break;
        }

        case PositionAtTop:
            listpos = idxPos;
            break;

        case PositionAtBottom:
            listpos = (idxPos + itemHeight) - visibleHeight;
            break;

        case PositionAtCenter:
            listpos = idxPos - ((visibleHeight - itemHeight) / 2);
            break;
    }

    if ((hint == EnsureVisible) || (hint == ImmediateVisible))
        d->m_forceAlign = true;

    if (hint == ImmediateVisible) {
        // Make the change without animation
        d->m_inFlick = false;
        d->m_position = static_cast<int>(listpos);

        d->m_listPosition.setValue(d->m_position);

        d->m_listTimeline.clear();
        d->m_pauseTimeline.clear();

        d->m_highlightTimeline.clear();
        d->m_highlightHeight.clear();
        d->m_highlightAlpha.clear();

        d->m_highlight->position()->setValue(idxPos);
        d->m_highlight->height()->setValue(itemHeight);
        d->m_highlight->alpha()->setValue(1.0f);

        if (!d->m_unhandledResize)
            doUpdate(true);
    } else {
        scrollTo(listpos, (truncated ? initialPos : -1));
    }
}

/*!
    Scrolls the list to the bottom.

    \sa scrollTo(), scrollToTop()
*/
void QSmoothList::scrollToBottom()
{
    if (d->m_model && d->m_model->rowCount())
        scrollTo(d->m_lastItemIndex, EnsureVisible);
}

/*!
    Scrolls the list to the top.

    \sa scrollTo(), scrollToBottom()
*/
void QSmoothList::scrollToTop()
{
    if (d->m_model && d->m_model->rowCount())
        scrollTo(0, PositionAtTop);
}

/*! \internal */
void QSmoothList::scrollTo(qreal top, int from)
{
    int current = int(d->m_listPosition.value());
    if (from == -1)
        from = current;

    if (int(top) != from) {
        d->m_inFlick = false;
        d->m_position = int(top);
        d->m_listTimeline.clear();

        d->scheduleMove(d->m_listTimeline, d->m_listPosition, from, top, rate*6);

        // Show scrollbar location for context
        showScrollBar();
        d->m_pauseTimeline.clear();
        d->m_pauseTimeline.pause(d->m_pauseObject, rate*6 + 1000);
    } else {
        // Ensure the constraints are respected
        fixupPosition(rate*6);
    }

    // Ensure the highlight is correctly updated
    fixupHighlight();
}

/*! \internal */
void QSmoothList::ensureCurrentItemVisible(int period)
{
    if (d->m_focusItem != -1) {
        // Don't calculate this adjustment until we know the size
        // the list will be when shown
        if (d->m_previouslyShown) {
            int listTop = int(d->m_listPosition.value());
            int listBottom = int(listTop) + height();
            int itemTop = d->itemPosition(d->m_focusItem);
            int itemBottom = itemTop + d->itemHeight(d->m_focusItem);

            int newPosition = listTop;
            if (itemTop < listTop) {
                newPosition = itemTop;
            } else if (itemBottom > listBottom) {
                newPosition = itemBottom - height();
            }

            if (newPosition != listTop) {
                d->m_inFlick = false;
                d->m_position = newPosition;
                d->m_listTimeline.clear();

                d->scheduleMove(d->m_listTimeline, d->m_listPosition, newPosition, period);
                doUpdate(true);
            }
        }
    }
}

/*! \internal */
void QSmoothList::fixupPosition(int period)
{
    // Don't calculate this adjustment until we know the size
    // the list will be when shown
    if (d->m_previouslyShown) {
        int visibleHeight = height();
        int newPosition = static_cast<int>(d->m_listPosition.value());

        if (d->listHeight() >= visibleHeight) {
            int topLimit, bottomLimit;

            if (d->m_positionHint == PositionAtCenter) {
                // Since we can move the list as far out of position as to place the first
                // or last item in the centre, allow the list to be scrolled that far also
                topLimit = -(visibleHeight - d->itemHeight(0)) / 2;
                bottomLimit = d->listHeight() + ((visibleHeight - d->itemHeight(d->m_lastItemIndex)) / 2);
            } else {
                // The list should fill the entire screen
                topLimit = 0;
                bottomLimit = d->listHeight();
            }

            if ((newPosition + visibleHeight) > bottomLimit) {
                // Move the list so the last item is at the bottom edge
                newPosition = bottomLimit - visibleHeight;
            } else if (newPosition < topLimit) {
                // Move the list so that the first item is at the top
                newPosition = topLimit;
            }
        } else {
            // The list is smaller than the visible area
            if (d->m_positionHint == EnsureVisible ||
                d->m_positionHint == ImmediateVisible ||
                d->m_positionHint == PositionAtTop) {
                // Move the list so that the first item is at the top
                newPosition = 0;
            } else if (d->m_positionHint == PositionAtBottom) {
                // Move the list so the last item is at the bottom edge
                newPosition = d->listHeight() - visibleHeight;
            } else {
                // Move the list so the selected item is centered
                if (d->m_focusItem != -1) {
                    int idx = d->m_focusItem;
                    int idxPos = d->itemPosition(idx);
                    int itemHeight = d->itemHeight(idx);
                    newPosition = (idxPos + ((itemHeight - visibleHeight) / 2));
                } else {
                    newPosition = 0;
                }
            }
        }

        d->m_inFlick = false;

        if (newPosition != static_cast<int>(d->m_listPosition.value())) {
            d->m_position = newPosition;
            d->m_listTimeline.clear();

            if (period) {
                d->m_listTimeline.move(d->m_listPosition, newPosition, period);
            } else {
                d->m_listPosition.setValue(newPosition);
            }
        }
    }
}

/*! \internal */
void QSmoothList::fixupHighlight(int period)
{
    if (d->m_focusItem != -1) {
        int highlightPos = d->itemPosition(d->m_focusItem);
        int highlightHeight = d->itemHeight(d->m_focusItem);

        if (highlightPos != static_cast<int>(d->m_highlight->position()->value())) {
            d->m_highlightTimeline.clear();
            d->scheduleMove(d->m_highlightTimeline, *d->m_highlight->position(), highlightPos, period);
        }

        if (highlightHeight != static_cast<int>(d->m_highlight->height()->value())) {
            d->m_highlightHeight.clear();
            d->scheduleMove(d->m_highlightHeight, *d->m_highlight->height(), highlightHeight, period);
        }

        if (d->m_highlight->alpha()->value() <= nearOne) {
            d->m_highlightAlpha.clear();
            d->scheduleMove(d->m_highlightAlpha, *d->m_highlight->alpha(), 1.0f, period);
        }
    } else {
        if (d->m_highlight->alpha()->value() >= nearZero) {
            d->m_highlightAlpha.clear();
            d->scheduleMove(d->m_highlightAlpha, *d->m_highlight->alpha(), 0.0f, period);
        }
    }

#ifdef SUPPRESS_QSMOOTHLIST_ANIMATION
    doUpdate(false);
#endif
}

/*! \internal */
void QSmoothList::invalidateItem(int idx)
{
    if (d->validIndex(idx) && d->m_items.at(idx))
        d->m_items.at(idx)->m_item->invalidateCache();
}

/*! \internal */
void QSmoothList::showScrollBar(int pause)
{
    if(!pause && scrollBarIsVisible())
        return;

    //fade scrollbar in
    d->m_scrollOn = true;
    d->m_scrollTime.clear();

    if (pause)
        d->m_scrollTime.pause(*d->m_scrollbar->alpha(), pause);

    d->scheduleMove(d->m_scrollTime, *d->m_scrollbar->alpha(), Scrollbar::MaxAlpha, rate*6);
}

/*! \internal */
void QSmoothList::hideScrollBar()
{
    if(!scrollBarIsVisible())
        return;

    //fade scrollbar out
    d->m_scrollOn = false;
    d->m_scrollTime.clear();

    d->scheduleMove(d->m_scrollTime, *d->m_scrollbar->alpha(), Scrollbar::MinAlpha, rate*6);
}

/*! \internal */
bool QSmoothList::scrollBarIsVisible() const
{
    //return whether scrollbar is visible
    return d->m_scrollOn;
}

