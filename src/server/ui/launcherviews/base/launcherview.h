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
#ifndef LAUNCHERVIEW_H
#define LAUNCHERVIEW_H

#include <QWidget>
#include <QListView>
#include <QList>
#include <QSize>
#include <QContentSet>
#include <QSortFilterProxyModel>
#include <QSmoothList>

class QContentSetMultiColumnProxyModel;
class QAbstractMessageBox;
class TypeDialog;
class QCategoryDialog;
class QLauncherProxyModel;
class QAbstractProxyModel;
class QVBoxLayout;
class LauncherViewDelegate;

class LauncherViewListView : public QListView
{
    Q_OBJECT
public:
    LauncherViewListView( QWidget *parent ) : QListView(parent) {}

signals:
    void currentIndexChanged( const QModelIndex &current, const QModelIndex &previous );

protected slots:
    virtual void currentChanged( const QModelIndex &current, const QModelIndex &previous );
    virtual void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    virtual void rowsInserted(const QModelIndex &parent,int start, int end);
protected:
    virtual void focusOutEvent(QFocusEvent *);
    virtual void focusInEvent(QFocusEvent *);
    virtual bool viewportEvent(QEvent *e);
};

class LauncherView : public QWidget 
{
    Q_OBJECT

public:
    LauncherView( QWidget* parent = 0, Qt::WFlags fl = 0);

    static LauncherView* createLauncherView( const QByteArray &name, QWidget *parent = 0, Qt::WFlags flags = 0);
    virtual ~LauncherView() {};
    virtual void resetSelection();
    void clearSelection();
    void setBusy(bool);
    void setBusy(const QModelIndex &, bool);
    void setItemDelegate(QAbstractItemDelegate *);
    void setViewMode( QListView::ViewMode m );
    QListView::ViewMode viewMode() const;
    void setFlow(QListView::Flow f);
    QListView::Flow flow() const;
    QContentSetModel *model() const { return m_model; }
    void setModel(QContentSetModel* model);
    static QSize listIconSize(QWidget *widget);

    void removeAllItems();
    void addItem(QContent* app, bool resort=true);
    void removeItem(const QContent &);

    void setColumns(int);
    virtual void setFilter(const QContentFilter &filter);
    const QContent currentItem() const;
    QModelIndex currentIndex() const;

    enum SortingStyle { NoSorting, LanguageAwareSorting };

protected:
    virtual void changeEvent(QEvent *e);
    virtual void showEvent(QShowEvent *e);
    virtual void calculateGridSize(bool force = false);
    virtual void timerEvent( QTimerEvent * event );

    virtual void handleReturnPressed(const QModelIndex &item);
    virtual void handleItemClicked(const QModelIndex &item, bool setCurrentIndex);
    virtual void handleItemPressed(const QModelIndex &item);
signals:
    void clicked( QContent );
    void rightPressed( QContent );

protected slots:
    void returnPressed(const QModelIndex &item) { handleReturnPressed(item); }
    void itemClicked(const QModelIndex &item) { handleItemClicked(item, true); }
    void itemPressed(const QModelIndex &item){ handleItemPressed(item); }
    void resizeEvent(QResizeEvent *);
    virtual void currentChanged( const QModelIndex &current, const QModelIndex &previous );

public slots:
    void showType( const QContentFilter & );
    void showCategory( const QContentFilter & );
    void setAuxiliaryFilter( const QContentFilter & );

protected:
    QContentSet *m_contentSet;
    QContentSetModel *m_model;
    QContentFilter mainFilter;
    QContentFilter categoryFilter;
    QContentFilter typeFilter;
    QContentFilter auxiliaryFilter;
    QVBoxLayout *m_mainLayout;
    LauncherViewListView *m_icons;
    QSmoothList *m_smoothList;

private:
    void init();
    void initListView();
    void initIconView();

    int nColumns;
    bool mNeedGridSize;
    QAbstractItemDelegate *m_itemDelegate;
};

#endif
