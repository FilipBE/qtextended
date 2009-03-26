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

#ifndef BROWSER_H
#define BROWSER_H

#include "menumodel.h"

#include <qcontentset.h>

class BrowserMenu : public MenuModel
{
public:
    explicit BrowserMenu( QObject* parent = 0 )
        : MenuModel( parent )
    { }

    virtual QContentFilter filter() const = 0;
    virtual void setFilter( const QContentFilter& filter ) = 0;
};

class PropertyBrowser : public BrowserMenu
{
    Q_OBJECT
public:
    enum Property { Artist, Album, Genre };

    PropertyBrowser( Property property, const QContentFilter& filter = QContentFilter(), QObject* parent = 0 );

    Property property() const { return m_property; }

    // Return filter string for given row
    QString filterString( const QModelIndex& index ) const;

    // BrowserMenu
    QContentFilter filter() const;
    void setFilter( const QContentFilter& filter );

    // AbstractListModel
    int rowCount( const QModelIndex& = QModelIndex() ) const;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

protected:
    static QString filterKey( Property property );

private:
    void populateData(bool force=false);

    Property m_property;
    QContentFilter m_filter;
    QStringList m_args, m_values;
    QString m_unknownValue;
    bool dataInitialised;
};

class ContentBrowser : public BrowserMenu
{
public:
    explicit ContentBrowser( const QContentFilter& filter = QContentFilter(), QObject* parent = 0 );
    ~ContentBrowser();

    void setSortOrder( const QStringList& list );

    // Return QContent for given row
    QContent content( const QModelIndex& index ) const;

    // BrowserMenu
    QContentFilter filter() const;
    void setFilter( const QContentFilter& filter );

    // AbstractListModel
    int rowCount( const QModelIndex& index = QModelIndex() ) const;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

private:
    QContentSet m_set;
    QContentSetModel *m_setmodel;
};

#endif
