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

#ifndef THUMBNAILVIEW_P_H
#define THUMBNAILVIEW_P_H

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

#include <qlistwidget.h>
#include <qitemdelegate.h>
#include <qqueue.h>
#include <qtimer.h>

#include <QDateTime>
#include <QItemDelegate>
#include <QCache>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QEvent>
#include <QScrollBar>

class ThumbnailRequest
{
public:
    ThumbnailRequest() : index_( QModelIndex() ){}
    ThumbnailRequest( const QModelIndex& index, const QString& filename, const QSize& size, const QDateTime &time )
        : index_( index ), filename_( filename ), size_( size ), time_( time )
    { }

    QModelIndex index() const { return index_; }

    QString filename() const { return filename_; }

    QSize size() const { return size_; }

    QDateTime time() const { return time_; }

private:
    QPersistentModelIndex index_;
    QString filename_;
    QSize size_;
    QDateTime time_;
};

Q_DECLARE_METATYPE(ThumbnailRequest)

class ThumbnailCache : public QObject
{
    Q_OBJECT
public:
    // ### TODO: set cache size
    explicit ThumbnailCache( QObject* parent )
        : QObject( parent )
    { }

    // Return thumbnail if in cache
    QPixmap retrieve( const ThumbnailRequest& request );

    bool contains( const ThumbnailRequest &request ) const;

public slots:
    // Add thumbnail to cache
    void insert( const ThumbnailRequest& request, const QPixmap& pixamp );

private:
    static QString key( const ThumbnailRequest& request );

    QCache<QString, QPixmap> cache;
};

class ThumbnailView;

class VisibleRule
{
public:
    explicit VisibleRule( ThumbnailView* view = 0 )
        : view_( view )
    { }

    // Return true if thumbnail is currently visible in view
    bool isMetBy( const ThumbnailRequest& ) const;

private:
    ThumbnailView *view_;
};

class CacheRule
{
public:
    explicit CacheRule( ThumbnailCache* cache = 0 )
        : cache_( cache )
    { }

    // Return true if thumbnail is currently in cache
    bool isMetBy( const ThumbnailRequest& ) const;

private:
    ThumbnailCache *cache_;
};

class ThumbnailRequestHandler;

class ThumbnailLoader : public QThread
{
    Q_OBJECT
public:
    ThumbnailLoader( QObject *parent = 0 );

    void load( const ThumbnailRequest& );

    // ### TODO: could be generalized into load rule
    void setVisibleRule( const VisibleRule& rule ) { QMutexLocker l( &m_initMutex ); visible_rule = rule; }

    // ### TODO: could be generalized into load rule
    void setCacheRule( const CacheRule& rule ) { QMutexLocker l( &m_initMutex ); cache_rule = rule; }

    QImage loadThumbnail( const QString &filename, const QSize &size );

signals:
    void loaded( const ThumbnailRequest &request, const QPixmap &thumbnail );

private slots:
    void thumbnailLoaded( const QImage &thumbnail );
    void thumbnailLoaded( const QPixmap &thumbnail );

protected:
    void run();

private:
    QMutex m_initMutex;
    QWaitCondition m_initCondition;
    ThumbnailRequestHandler *m_requestHandler;
    QList< ThumbnailRequest > requests;
    CacheRule cache_rule;
    VisibleRule visible_rule;
};

class ThumbnailRequestHandler : public QObject
{
    Q_OBJECT
public:
    ThumbnailRequestHandler( ThumbnailLoader *loader );
protected:
    bool event( QEvent *event );

signals:
    void thumbnailLoaded( const QImage &thumbnail );

private:
    ThumbnailLoader *m_loader;

};

class ThumbnailRequestEvent : public QEvent
{
public:
    ThumbnailRequestEvent( const QString &fileName, const QSize &size )
        : QEvent( User )
        , m_fileName( fileName )
        , m_size( size )
    {
    }

    const QString &fileName() const{ return m_fileName; }
    const QSize &size() const{ return m_size; }

private:
    QString m_fileName;
    QSize m_size;
};

class ThumbnailRepository : public QObject
{
    Q_OBJECT
public:
    ThumbnailRepository( ThumbnailCache* cache, ThumbnailLoader* loader, QObject* parent );

    // Return thumbnail if in cache
    QPixmap thumbnail( const ThumbnailRequest& request );

signals:
    // Thumbnail request loaded into cache
    void loaded( const ThumbnailRequest& request, const QPixmap& thumbnail );

private:
    ThumbnailCache *cache_;
    ThumbnailLoader *loader_;
};

class ThumbnailDelegate : public QItemDelegate
{
public:
    ThumbnailDelegate( ThumbnailRepository* repository, QObject* parent )
        : QItemDelegate( parent ), repository_( repository )
    { }

    void paint( QPainter*, const QStyleOptionViewItem&, const QModelIndex& ) const;

    QSize sizeHint( const QStyleOptionViewItem&, const QModelIndex& ) const;

private:
    ThumbnailRepository *repository_;
};

class ThumbnailView : public QListView
{
    Q_OBJECT
public:
    explicit ThumbnailView( QWidget* parent=0 );

signals:
    // Select key pressed
    void selected();

    // ### TODO: stylus held on thumbnail
    void held( const QPoint& );

public slots:
    void repaintThumbnail( const ThumbnailRequest& request );

protected slots:
    void rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end );
    void rowsInserted( const QModelIndex &parent, int start, int end );

private slots:
    void emitSelected( const QModelIndex& index );

protected:
    void keyPressEvent( QKeyEvent* );
};

#endif
