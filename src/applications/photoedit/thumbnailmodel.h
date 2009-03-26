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
#ifndef THUMBNAILMODEL_H
#define THUMBNAILMODEL_H

#include <QObject>
#include <QContentSet>
#include <QItemDelegate>
#include <QCache>
#include <QListView>

class QSize;
class QPixmap;
class QImage;
class QAbstractItemView;
class ThumbnailContentSetModelThread;

class ThumbnailContentSetModel : public QContentSetModel
{
    Q_OBJECT
public:
    ThumbnailContentSetModel( QContentSet *set, QObject *parent = 0 );
    ~ThumbnailContentSetModel();

    QSize thumbnailSize() const;
    void setThumbnailSize( const QSize &size );

    Qt::AspectRatioMode aspectRatioMode() const;
    void setAspectRatioMode( Qt::AspectRatioMode mode );

    QAbstractItemView *view() const;
    void setView( QAbstractItemView *view );

    QVariant data( const QModelIndex &index, int role ) const;

private slots:
    void thumbnailLoaded( const QImage &image );
    void contentChanged( const QContentIdList &contentIds, QContent::ChangeType type );

private:
    QPixmap thumbnail( const QModelIndex &index, const QContent &content ) const;

    QSize m_thumbnailSize;
    Qt::AspectRatioMode m_aspectRatioMode;
    mutable QCache< QContentId, QPixmap > m_cache;
    mutable QList< QPair< QPersistentModelIndex, QContent > > m_requestQueue;
    mutable ThumbnailContentSetModelThread *m_thread;
    QAbstractItemView *m_view;
};

class ContentThumbnailDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ContentThumbnailDelegate( QObject *parent = 0 );

    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const;
};

class ContentThumbnailView : public QListView
{
    Q_OBJECT
public:
    ContentThumbnailView( QWidget *parent = 0 );

protected:
    void resizeEvent( QResizeEvent*e );

protected slots:
    void rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end );
    void rowsInserted( const QModelIndex &parent, int start, int end );
};

#endif
