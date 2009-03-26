/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QTREEVIEW_P_H
#define QTREEVIEW_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "private/qabstractitemview_p.h"

#ifndef QT_NO_TREEVIEW

QT_BEGIN_NAMESPACE

struct QTreeViewItem
{
    QTreeViewItem() : expanded(false), spanning(false), total(0), level(0), height(0) {}
    QModelIndex index; // we remove items whenever the indexes are invalidated
    uint expanded : 1;
    uint spanning : 1;
    uint total : 30; // total number of children visible
    uint level : 16; // indentation
    int height : 16; // row height
};

class QTreeViewPrivate : public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QTreeView)
public:

    QTreeViewPrivate()
        : QAbstractItemViewPrivate(),
          header(0), indent(20), lastViewedItem(0), defaultItemHeight(-1),
          uniformRowHeights(false), rootDecoration(true),
          itemsExpandable(true), sortingEnabled(false),
          expandsOnDoubleClick(true),
          allColumnsShowFocus(false),
          animationsEnabled(false), columnResizeTimerID(0),
          autoExpandDelay(-1), hoverBranch(-1), geometryRecursionBlock(false) {}

    ~QTreeViewPrivate() {}
    void initialize();

    struct AnimatedOperation
    {
        enum Type { Expand, Collapse };
        int item;
        int top;
        int duration;
        Type type;
        QPixmap before;
        QPixmap after;
    };

    void expand(int item, bool emitSignal);
    void collapse(int item, bool emitSignal);

    void prepareAnimatedOperation(int item, AnimatedOperation::Type type);
    void beginAnimatedOperation();
    void _q_endAnimatedOperation();
    void drawAnimatedOperation(QPainter *painter) const;
    QPixmap renderTreeToPixmap(const QRect &rect) const;

    inline QRect animationRect() const
        { return QRect(0, animatedOperation.top, viewport->width(),
                       viewport->height() - animatedOperation.top); }

    void _q_currentChanged(const QModelIndex&, const QModelIndex&);
    void _q_columnsAboutToBeRemoved(const QModelIndex &, int, int);
    void _q_columnsRemoved(const QModelIndex &, int, int);
    void _q_modelAboutToBeReset();

    void layout(int item);

    int pageUp(int item) const;
    int pageDown(int item) const;

    inline int above(int item) const
        { return (--item < 0 ? 0 : item); }
    inline int below(int item) const
        { return (++item >= viewItems.count() ? viewItems.count() - 1 : item); }
    inline void invalidateHeightCache(int item) const
        { viewItems[item].height = 0; }

    int itemHeight(int item) const;
    int indentationForItem(int item) const;
    int coordinateForItem(int item) const;
    int itemAtCoordinate(int coordinate) const;

    int viewIndex(const QModelIndex &index) const;
    QModelIndex modelIndex(int i) const;

    int firstVisibleItem(int *offset = 0) const;
    int columnAt(int x) const;
    bool hasVisibleChildren( const QModelIndex& parent) const;

    void relayout(const QModelIndex &parent);
    void reexpandChildren(const QModelIndex &parent);

    void updateScrollBars();

    int itemDecorationAt(const QPoint &pos) const;

    QList<QPair<int, int> > columnRanges(const QModelIndex &topIndex, const QModelIndex &bottomIndex) const;
    void select(const QModelIndex &start, const QModelIndex &stop, QItemSelectionModel::SelectionFlags command);

    QPair<int,int> startAndEndColumns(const QRect &rect) const;

    void updateChildCount(const int parentItem, const int delta);
    void rowsRemoved(const QModelIndex &parent,
                     int start, int end, bool before);

    void paintAlternatingRowColors(QPainter *painter, QStyleOptionViewItemV4 *option, int y, int bottom) const;

    QHeaderView *header;
    int indent;

    mutable QVector<QTreeViewItem> viewItems;
    mutable int lastViewedItem;
    int defaultItemHeight; // this is just a number; contentsHeight() / numItems
    bool uniformRowHeights; // used when all rows have the same height
    bool rootDecoration;
    bool itemsExpandable;
    bool sortingEnabled;
    bool expandsOnDoubleClick;
    bool allColumnsShowFocus;

    // used for drawing
    mutable QPair<int,int> leftAndRight;
    mutable int current;
    mutable bool spanning;

    // used when expanding and collapsing items
    QList<QPersistentModelIndex> expandedIndexes;
    QStack<bool> expandParent;
    AnimatedOperation animatedOperation;
    bool animationsEnabled;

    inline bool storeExpanded(const QPersistentModelIndex &idx) {
        QList<QPersistentModelIndex>::iterator it;
        it = qLowerBound(expandedIndexes.begin(), expandedIndexes.end(), idx);
        if (it == expandedIndexes.end() || *it != idx) {
            expandedIndexes.insert(it, idx);
            return true;
        }
        return false;
    }

    inline bool isIndexExpanded(const QPersistentModelIndex idx) const {
        QList<QPersistentModelIndex>::const_iterator it;
        it = qBinaryFind(expandedIndexes.constBegin(), expandedIndexes.constEnd(), idx);
        return (it != expandedIndexes.constEnd());
    }

    // used when hiding and showing items
    QVector<QPersistentModelIndex> hiddenIndexes;

    inline bool isRowHidden(const QPersistentModelIndex idx) const {
        QVector<QPersistentModelIndex>::const_iterator it;
        it = qBinaryFind(hiddenIndexes.constBegin(), hiddenIndexes.constEnd(), idx);
        return (it != hiddenIndexes.constEnd());
    }

    // used for spanning rows
    QVector<QPersistentModelIndex> spanningIndexes;

    // used for updating resized columns
    int columnResizeTimerID;
    QList<int> columnsToUpdate;

    // used for the automatic opening of nodes during DND
    int autoExpandDelay;
    QBasicTimer openTimer;

    // used for drawing hilighted expand/collapse indicators
    int hoverBranch;

    // used for blocking recursion when calling setViewportMargins from updateGeometries
    bool geometryRecursionBlock;
};

QT_END_NAMESPACE

#endif // QT_NO_TREEVIEW

#endif // QTREEVIEW_P_H
