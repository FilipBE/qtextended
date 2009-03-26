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

#ifndef QSMOOTHLISTWIDGET_P_H
#define QSMOOTHLISTWIDGET_P_H

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
#include "qsmoothlist.h"
#include <qtopiaglobal.h>
#include <QStandardItem>

class QSmoothListWidget;
class QSmoothListWidgetItem;
class QSmoothListWidgetPrivate;

class QTOPIA_EXPORT QSmoothListWidget : public QSmoothList
{
    Q_OBJECT
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged USER true)
    Q_PROPERTY(bool sortingEnabled READ isSortingEnabled WRITE setSortingEnabled)

public:
    QSmoothListWidget(QWidget *parent=0);
    ~QSmoothListWidget();

    void addItem(const QString &label);
    void addItem(QSmoothListWidgetItem *item);
    void addItems(const QStringList &labels);
    int count() const;
    QSmoothListWidgetItem * currentItem () const;
    int currentRow () const;
    QList<QSmoothListWidgetItem *> findItems ( const QString & text, Qt::MatchFlags flags ) const;
    void insertItem ( int row, QSmoothListWidgetItem * item );
    void insertItem ( int row, const QString & label );
    void insertItems ( int row, const QStringList & labels );
    bool isSortingEnabled () const;
    QSmoothListWidgetItem * item ( int row ) const;
    QSmoothListWidgetItem * itemAt ( const QPoint & p ) const;
    QSmoothListWidgetItem * itemAt ( int x, int y ) const;
    int row ( const QSmoothListWidgetItem * item ) const;
    void setCurrentItem ( QSmoothListWidgetItem * item );
    void setCurrentRow ( int row );
    void setSortingEnabled ( bool enable );
    void sortItems ( Qt::SortOrder order = Qt::AscendingOrder );
    QSmoothListWidgetItem * takeItem ( int row );
    QRect visualItemRect ( const QSmoothListWidgetItem * item ) const;

public slots:
    void clear();
    void scrollToItem ( const QSmoothListWidgetItem * item, QSmoothList::ScrollHint hint = EnsureVisible );

private slots:
    void emit_currentItemRowChanged ( const QModelIndex& current, const QModelIndex& previous );
    void emit_itemActivated (const QModelIndex &index);
    void emit_itemClicked (const QModelIndex &index);
    void emit_itemDoubleClicked (const QModelIndex &index);
    void emit_itemPressed (const QModelIndex &index);

signals:
    void currentItemChanged ( QSmoothListWidgetItem * current, QSmoothListWidgetItem * previous );
    void currentRowChanged ( int currentRow );
    void itemActivated ( QSmoothListWidgetItem * item );
    void itemClicked ( QSmoothListWidgetItem * item );
    void itemDoubleClicked ( QSmoothListWidgetItem * item );
    void itemPressed ( QSmoothListWidgetItem * item );

protected:
    QModelIndex indexFromItem ( QSmoothListWidgetItem * item ) const;
    QSmoothListWidgetItem * itemFromIndex ( const QModelIndex & index ) const;

private:
    QSmoothListWidgetPrivate *d;
};

class QSmoothListWidgetItemPrivate
{
public:
    QSmoothListWidgetItemPrivate(int t)
        :type(t), smoothListWidget(0) {}
    int type;
    QSmoothListWidget *smoothListWidget;
};

class QTOPIA_EXPORT QSmoothListWidgetItem : public QStandardItem
{
    friend class QSmoothListWidget;
public:
    QSmoothListWidgetItem ( QSmoothListWidget * parent = 0, int type = Type )
        : QStandardItem(), d(new QSmoothListWidgetItemPrivate(type))
    {
        parent->addItem(this);
    }

    QSmoothListWidgetItem ( const QString & text, QSmoothListWidget * parent = 0, int type = Type )
        : QStandardItem(text), d(new QSmoothListWidgetItemPrivate(type))
    {
        parent->addItem(this);
    }

    QSmoothListWidgetItem ( const QIcon & icon, const QString & text, QSmoothListWidget * parent = 0, int type = Type )
        : QStandardItem(icon, text), d(new QSmoothListWidgetItemPrivate(type))
    {
        parent->addItem(this);
    }

    //QSmoothListWidgetItem ( const QSmoothListWidgetItem & other );

    using QStandardItem::setData;

    virtual void setData ( int role, const QVariant & value )
    {
        QStandardItem::setData(value, role);
    }

    int type() const
    {
        return d->type;
    }

    QSmoothListWidget *smoothListWidget() const
    {
        return d->smoothListWidget;
    }

private:
    QSmoothListWidgetItemPrivate* d;
};
#endif
