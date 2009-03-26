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

#ifndef QCONTENTFILTERSELECTOR_P_H
#define QCONTENTFILTERSELECTOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QAbstractProxyModel>
#include <qcontentfiltermodel.h>
#include <QListView>
#include <QtopiaItemDelegate>

class QTreePageProxyModelPrivate;

class QTreePageProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
public:

    enum ItemType
    {
        SelectionItem,
        ReturnItem,
        ParentItem
    };

    explicit QTreePageProxyModel( QObject *parent = 0 );

    virtual ~QTreePageProxyModel();

    virtual QModelIndex mapFromSource( const QModelIndex &index ) const;

    virtual QModelIndex mapToSource( const QModelIndex &index ) const;

    virtual void setSourceModel( QAbstractItemModel *sourceModel );

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual bool hasChildren( const QModelIndex &parent = QModelIndex() ) const;

    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;

    virtual QModelIndex parent( const QModelIndex &index ) const;

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual QVariant data( const QModelIndex &index, int role ) const;

    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

    virtual QSize span( const QModelIndex &index ) const;

    virtual QModelIndex buddy( const QModelIndex &index ) const;

    virtual void fetchMore(const QModelIndex &parent);

    virtual bool canFetchMore(const QModelIndex &parent) const;

    bool sourceHasChildren( const QModelIndex &index ) const;

    bool atRoot() const;

public slots:

    void browseToIndex( const QModelIndex &index );

    void back();

signals:
    void ensureVisible( const QModelIndex &index );
    void select( const QModelIndex &index );

private slots:
    void _columnsAboutToBeInserted( const QModelIndex &parent, int start, int end );

    void _columnsAboutToBeRemoved( const QModelIndex &parent, int start, int end );

    void _columnsInserted( const QModelIndex &parent, int start, int end );

    void _columnsRemoved( const QModelIndex &parent, int start, int end );

    void _dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight );

    void _modelReset();

    void _rowsAboutToBeInserted( const QModelIndex &parent, int start, int end );

    void _rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end );

    void _rowsInserted( const QModelIndex &parent, int start, int end );

    void _rowsRemoved( const QModelIndex &parent, int start, int end );

private:
    QTreePageProxyModelPrivate *d;
};

class QContentFilterView : public QListView
{
    Q_OBJECT
public:
    explicit QContentFilterView( QWidget *parent = 0 );
    virtual ~QContentFilterView();

    QContentFilter checkedFilter( const QModelIndex &parent = QModelIndex() ) const;
    QString checkedLabel() const;

    QContentFilterModel *filterModel() const;
    void setFilterModel( QContentFilterModel *model );

signals:
    void filterSelected( const QContentFilter &filter );

private slots:
    void indexSelected( const QModelIndex &index );
    void ensureVisible( const QModelIndex &index );


protected:
    void keyPressEvent( QKeyEvent *e );
    void focusInEvent( QFocusEvent *e );

private:
    QTreePageProxyModel *m_proxyModel;
    QContentFilterModel *m_filterModel;
};

class QTreePageItemDelegate : public QtopiaItemDelegate
{
public:
    explicit QTreePageItemDelegate( QObject *parent = 0 );
    virtual ~QTreePageItemDelegate();

    virtual void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    virtual QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const;

protected:
    QRect arrow( const QStyleOptionViewItem &option, const QRect &bounding, const QVariant &value ) const;

    void doLayout( const QStyleOptionViewItem &option, QRect *checkRect, QRect *pixmapRect, QRect *textRect, QRect *arrowRect, bool hint ) const;
    void drawArrow(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, QTreePageProxyModel::ItemType itemType ) const;
};

#endif
