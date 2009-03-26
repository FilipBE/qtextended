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

#ifndef QTREEWIDGET_P_H
#define QTREEWIDGET_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. This header file may change
// from version to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qabstractitemmodel.h>
#include <private/qabstractitemmodel_p.h>
#include <QtCore/qpair.h>
#include <QtGui/qtreewidget.h>

#ifndef QT_NO_TREEWIDGET

QT_BEGIN_NAMESPACE

class QTreeWidgetItem;
class QTreeWidgetItemIterator;
class QTreeModelPrivate;

class QTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    friend class QTreeWidget;
    friend class QTreeWidgetPrivate;
    friend class QTreeWidgetItem;
    friend class QTreeWidgetItemIterator;
    friend class QTreeWidgetItemIteratorPrivate;

public:
    QTreeModel(int columns = 0, QTreeWidget *parent = 0);
    ~QTreeModel();

    inline QTreeWidget *view() const
        { return qobject_cast<QTreeWidget*>(QObject::parent()); }

    void clear();
    void setColumnCount(int columns);

    QTreeWidgetItem *item(const QModelIndex &index) const;
    void itemChanged(QTreeWidgetItem *item);

    QModelIndex index(const QTreeWidgetItem *item, int column) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool hasChildren(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    QMap<int, QVariant> itemData(const QModelIndex &index) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
                       int role);

    Qt::ItemFlags flags(const QModelIndex &index) const;

    void sort(int column, Qt::SortOrder order);
    void ensureSorted(int column, Qt::SortOrder order,
                      int start, int end, const QModelIndex &parent);
    static bool itemLessThan(const QPair<QTreeWidgetItem*,int> &left,
                             const QPair<QTreeWidgetItem*,int> &right);
    static bool itemGreaterThan(const QPair<QTreeWidgetItem*,int> &left,
                                const QPair<QTreeWidgetItem*,int> &right);
    static QList<QTreeWidgetItem*>::iterator sortedInsertionIterator(
        const QList<QTreeWidgetItem*>::iterator &begin,
        const QList<QTreeWidgetItem*>::iterator &end,
        Qt::SortOrder order, QTreeWidgetItem *item);

    bool insertRows(int row, int count, const QModelIndex &);
    bool insertColumns(int column, int count, const QModelIndex &);

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    // dnd
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent);
    Qt::DropActions supportedDropActions() const;

    QMimeData *internalMimeData() const;

    inline QModelIndex createIndexFromItem(int row, int col, QTreeWidgetItem *item) const
    { return createIndex(row, col, item); }

protected:
    QTreeModel(QTreeModelPrivate &, QTreeWidget *parent = 0);
    void emitDataChanged(QTreeWidgetItem *item, int column);
    void beginInsertItems(QTreeWidgetItem *parent, int row, int count);
    void endInsertItems();
    void beginRemoveItems(QTreeWidgetItem *parent, int row, int count);
    void endRemoveItems();
    void sortItems(QList<QTreeWidgetItem*> *items, int column, Qt::SortOrder order);

private:
    QTreeWidgetItem *rootItem;
    QTreeWidgetItem *headerItem;

    mutable QModelIndexList cachedIndexes;
    QList<QTreeWidgetItemIterator*> iterators;

    mutable bool sortPending;
    bool executePendingSort() const;

    bool isChanging() const;

private:
    Q_DECLARE_PRIVATE(QTreeModel)
};

QT_BEGIN_INCLUDE_NAMESPACE
#include "private/qabstractitemmodel_p.h"
QT_END_INCLUDE_NAMESPACE

class QTreeModelPrivate : public QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(QTreeModel)
};

class QTreeWidgetItemPrivate
{
public:
    QTreeWidgetItemPrivate(QTreeWidgetItem *item)
        : q(item), disabled(false), selected(false), rowGuess(-1), policy(QTreeWidgetItem::DontShowIndicatorWhenChildless) {}
    void propagateDisabled(QTreeWidgetItem *item);
    QTreeWidgetItem *q;
    QVariantList display;
    uint disabled : 1;
    uint selected : 1;
    int rowGuess;
    QTreeWidgetItem::ChildIndicatorPolicy policy;
};

QT_END_NAMESPACE

#endif // QT_NO_TREEWIDGET

#endif // QTREEWIDGET_P_H
