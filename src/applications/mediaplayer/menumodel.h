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

#ifndef MENUMODEL_H
#define MENUMODEL_H

#include <QtGui>
#include <qtopiaglobal.h>
#include <QMediaPlaylist>

enum NavigationHint { NodeHint, LeafHint };
static const int NAVIGATION_HINT_ROLE = Qt::UserRole + 0xf;

class QTOPIAMEDIA_EXPORT PlaylistModel
{
public:
    virtual ~PlaylistModel() { }

    // Return current playlist
    virtual const QMediaPlaylist &playlist() const = 0;
};

Q_DECLARE_INTERFACE(PlaylistModel,
    "com.trolltech.Qtopia.QtopiaMedia.PlaylistModel/1.0")

class ServiceRequest;

class MenuModel : public QAbstractListModel
{
public:
    enum ActionContext { Select, Hold, LongHold };

    explicit MenuModel( QObject* parent = 0 )
        : QAbstractListModel( parent )
    { }

    virtual ServiceRequest* action( const QModelIndex& index, ActionContext context = Select ) const = 0;
};

class SimpleMenuModel : public MenuModel
{
public:
    explicit SimpleMenuModel( QObject* parent = 0 )
        : MenuModel( parent )
    { }

    ~SimpleMenuModel();

    void addItem( const QString& text, const QIcon& icon, NavigationHint hint, const ServiceRequest& request );
    void addItem( const QString& text, const QIcon& icon, NavigationHint hint, QObject* receiver, const char* member );
    void addItem( const QString& text, const QIcon& icon, NavigationHint hint, MenuModel* menu );

    void addItem( const QString& text, const ServiceRequest& request );
    void addItem( const QString& text, QObject* receiver, const char* member );
    void addItem( const QString& text, MenuModel* menu );

    void removeRow( int row );

    // MenuModel
    ServiceRequest* action( const QModelIndex& index, ActionContext context = Select ) const;

    // AbstractListModel
    int rowCount( const QModelIndex& parent = QModelIndex() ) const;
    QVariant data( const QModelIndex& index, int role ) const;

private:
    struct TextRequestPair
    {
        QString text;
        QIcon icon;
        NavigationHint hint;
        ServiceRequest *request;
    };

    QList<TextRequestPair> m_items;
};

#endif
