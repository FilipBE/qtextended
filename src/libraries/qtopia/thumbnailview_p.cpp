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

#include "thumbnailview_p.h"

#include <qthumbnail.h>

#ifndef QTOPIA_CONTENT_INSTALLER
#include <qtopiaapplication.h>
#else
#include <QDateTime>
#include <QApplication>
#endif
#include <qsoftmenubar.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qstring.h>
#include <qfileinfo.h>
#include <qlayout.h>

#include <QIcon>
#include <QImageReader>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QCursor>
#include <QDebug>
#include <QDesktopWidget>
#include <QContentSet>
#include <Qtopia>
#include "drmcontent_p.h"

void ThumbnailCache::insert( const ThumbnailRequest& request, const QPixmap& pixmap )
{
    cache.insert( key( request ), new QPixmap( pixmap ) );
}

QPixmap ThumbnailCache::retrieve( const ThumbnailRequest& request )
{
    QPixmap ret;
    // Retrieve thumbnail from cache
    QPixmap *pixmap = cache.object( key( request ) );
    // If retrieve successful, return thumbnail
    // Otherwise, return null thumbnail
    if( pixmap ) {
        ret = *pixmap;
    }

    return ret;
}

bool ThumbnailCache::contains( const ThumbnailRequest &request ) const
{
    return cache.contains( key( request ) );
};

QString ThumbnailCache::key( const ThumbnailRequest& request )
{
     return request.filename() + QString::number( request.size().width() ) + QString::number( request.size().height() ) + request.time().toString( Qt::ISODate );
}

bool VisibleRule::isMetBy( const ThumbnailRequest& request ) const
{
    // If index visible in view, return true
    // Otherwise, return false
    if( view_ ) {
        QModelIndex index = request.index();
        if( index.isValid() && view_->visualRect( index ).intersects( view_->viewport()->contentsRect() ) ) {
            return true;
        }
    }

    return false;
}

bool CacheRule::isMetBy( const ThumbnailRequest& request ) const
{
    // If thumbnail in cache, return true
    // Otherwise, return false
    return cache_ && cache_->contains( request );
}


ThumbnailLoader::ThumbnailLoader( QObject *parent )
    : QThread( parent )
    , m_requestHandler( 0 )
{

}

void ThumbnailLoader::load( const ThumbnailRequest &request )
{
    {
        QMutexLocker locker( &m_initMutex );

        if( !m_requestHandler )
            m_initCondition.wait( &m_initMutex );
    }

    if( requests.isEmpty() )
    {
        QContent content( request.filename(), false );

        if( content.drmState() == QContent::Protected )
        {
            QPixmap thumbnail = DrmContentPrivate::thumbnail( request.filename(), request.size(), Qt::KeepAspectRatio );

            QMetaObject::invokeMethod( this, "thumbnailLoaded", Qt::QueuedConnection, Q_ARG(QPixmap,thumbnail) );
        }
        else
        {
            ThumbnailRequestEvent *event = new ThumbnailRequestEvent( request.filename(), request.size() );

            QCoreApplication::postEvent( m_requestHandler, event );
        }
    }

    requests.append( request );
}

QImage ThumbnailLoader::loadThumbnail( const QString &filename, const QSize &size )
{
    QImageReader reader( filename );

    QImage image;

    bool scaled = false;

    if( reader.supportsOption( QImageIOHandler::Size ) && reader.supportsOption( QImageIOHandler::ScaledSize ) )
    {
        QSize maxSize = reader.size();

        maxSize.scale( size.boundedTo( reader.size() ), Qt::KeepAspectRatio );

        reader.setQuality( 49 ); // Otherwise Qt smooth scales
        reader.setScaledSize( maxSize );

        scaled = true;
    }

    if( reader.read( &image ) )
    {
        if( !scaled )
        {
            QSize maxSize = size.boundedTo( image.size() );

            image = image.scaled( maxSize, Qt::KeepAspectRatio, Qt::FastTransformation );
        }

        return image;
    }

    return QImage();
}

void ThumbnailLoader::run()
{
    ThumbnailRequestHandler requestHandler( this );

    {
        QMutexLocker locker( &m_initMutex );

        m_requestHandler = &requestHandler;

        connect( &requestHandler, SIGNAL(thumbnailLoaded(QImage)),
                 this           , SLOT  (thumbnailLoaded(QImage)) );

        m_initCondition.wakeAll();
    }

    exec();

    QMutexLocker locker( &m_initMutex );

    disconnect( &requestHandler );

    m_requestHandler = 0;

    m_initCondition.wakeAll();
}

void ThumbnailLoader::thumbnailLoaded( const QImage &image )
{
    thumbnailLoaded( QPixmap::fromImage( image ) );
}

void ThumbnailLoader::thumbnailLoaded( const QPixmap &pixmap )
{
    emit loaded( requests.takeFirst(), pixmap );

    while( !requests.isEmpty() )
    {
        ThumbnailRequest request = requests.first();

        if( !cache_rule.isMetBy( request ) && visible_rule.isMetBy( request ) )
        {
            QContent content( request.filename(), false );

            if( content.drmState() == QContent::Protected )
            {
                QPixmap thumbnail = DrmContentPrivate::thumbnail( request.filename(), request.size(), Qt::KeepAspectRatio );

                QMetaObject::invokeMethod( this, "thumbnailLoaded", Qt::QueuedConnection, Q_ARG(QPixmap,thumbnail) );
            }
            else
            {
                ThumbnailRequestEvent *event = new ThumbnailRequestEvent( request.filename(), request.size() );

                QCoreApplication::postEvent( m_requestHandler, event );
            }
            return;

        }
        else
            requests.removeFirst();
    }
}

ThumbnailRequestHandler::ThumbnailRequestHandler( ThumbnailLoader *loader )
    : m_loader( loader )
{

}

bool ThumbnailRequestHandler::event( QEvent *event )
{
    if( event->type() == QEvent::User )
    {
        ThumbnailRequestEvent *e = static_cast< ThumbnailRequestEvent * >( event );

        emit thumbnailLoaded( m_loader->loadThumbnail( e->fileName(), e->size() ) );

        return true;
    }
    else
        return QObject::event( event );
}


ThumbnailRepository::ThumbnailRepository( ThumbnailCache* cache, ThumbnailLoader* loader, QObject* parent )
    : QObject( parent ), cache_( cache ), loader_( loader )
{
    connect( loader_, SIGNAL(loaded(ThumbnailRequest,QPixmap)),
        this, SIGNAL(loaded(ThumbnailRequest,QPixmap)) );
}

QPixmap ThumbnailRepository::thumbnail( const ThumbnailRequest& request )
{
    QPixmap pixmap = cache_->retrieve( request );
    if( pixmap.isNull() && !cache_->contains( request ) ) {
        loader_->load( request );
    }

    return pixmap;
}

void ThumbnailDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    Q_ASSERT(index.isValid());
    const QContentSetModel *model = qobject_cast< const QContentSetModel * >( index.model() );
    Q_ASSERT(model);

    QStyleOptionViewItem opt = option;

    // set text alignment
    opt.displayAlignment = Qt::AlignCenter;

    // do layout
    QContent content = model->content( index );
    QPixmap pixmap;

    if( content.fileKnown() ) {
        pixmap = repository_->thumbnail( ThumbnailRequest( index, content.file(), option.decorationSize, content.lastUpdated() ) );
    }
    if( pixmap.isNull() ) {
        QIcon icon = qvariant_cast<QIcon>(model->data(index, Qt::DecorationRole));
        pixmap = icon.pixmap( option.decorationSize );
    }
    QRect pixmapRect = QRect(0, 0, option.decorationSize.width(),
                           option.decorationSize.height());

    QFontMetrics fontMetrics(opt.font);
    QString text = model->data(index, Qt::DisplayRole).toString();
    QRect textRect(0, 0, option.decorationSize.width(), fontMetrics.lineSpacing());

    QVariant value = model->data(index, Qt::CheckStateRole);
    QRect checkRect = check(opt, opt.rect, value);
    Qt::CheckState checkState = static_cast<Qt::CheckState>(value.toInt());

    doLayout(opt, &checkRect, &pixmapRect, &textRect, false);

    // draw the background color
    if (option.showDecorationSelected && (option.state & QStyle::State_Selected)) {
        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
        painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
    } else {
        value = model->data(index, Qt::BackgroundColorRole);
        if (value.isValid() && qvariant_cast<QColor>(value).isValid())
            painter->fillRect(option.rect, qvariant_cast<QColor>(value));
    }

    // draw the item
    drawCheck(painter, opt, checkRect, checkState);
    drawDecoration(painter, opt, pixmapRect, pixmap);
    drawDisplay(painter, opt, textRect, text);
    drawFocus(painter, opt, textRect);
}

QSize ThumbnailDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_ASSERT(index.isValid());
    const QAbstractItemModel *model = index.model();
    Q_ASSERT(model);

    QFont fnt = option.font;
    QString text = model->data(index, Qt::DisplayRole).toString();
    QRect pixmapRect = QRect(0, 0, option.decorationSize.width(),
                           option.decorationSize.height());

    QFontMetrics fontMetrics(fnt);
    QRect textRect(0, 0, option.decorationSize.width(), fontMetrics.lineSpacing());
    QRect checkRect = check(option, textRect, model->data(index, Qt::CheckStateRole));
    doLayout(option, &checkRect, &pixmapRect, &textRect, true);

    return (pixmapRect|textRect|checkRect).size();
}

ThumbnailView::ThumbnailView( QWidget* parent )
    : QListView( parent )
{
    setMovement( QListView::Static );
    setFrameStyle(QFrame::NoFrame);
    connect( this, SIGNAL(pressed(QModelIndex)), this, SLOT(emitSelected(QModelIndex)) );
}

void ThumbnailView::repaintThumbnail( const ThumbnailRequest& request )
{
    dataChanged( request.index(), request.index() );
}

void ThumbnailView::emitSelected( const QModelIndex& index )
{
    // If item selected, emit selected
    if( index.isValid() ) {
        emit selected();
    }
}

void ThumbnailView::rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end )
{
    QModelIndex index = currentIndex();

    QScrollBar *vScroll = verticalScrollBar();

    int startY = visualRect( index.sibling( start, index.column() ) ).top();
    int endY = visualRect( index.sibling( end + 1, index.column() ) ).top();

    int scrollValue = vScroll->value() - endY + startY;

    QListView::rowsAboutToBeRemoved( parent, start, end );

    if( index.row() >= start )
    {
        if( index.row() <= end && end + 1 < model()->rowCount( parent ) )
        {
            selectionModel()->setCurrentIndex(
                    model()->index( end + 1, index.column(), parent ),
                    QItemSelectionModel::ClearAndSelect);
        }
    }

    if( startY <= 0 )
        vScroll->setValue( scrollValue );
}

void ThumbnailView::rowsInserted( const QModelIndex &parent, int start, int end )
{
    QListView::rowsInserted( parent, start, end );

    if( !Qtopia::mousePreferred() )
    {
        if( !currentIndex().isValid() )
            selectionModel()->setCurrentIndex( model()->index( 0, 0, parent ), QItemSelectionModel::ClearAndSelect );
    }
    else
        selectionModel()->clearSelection();

    QScrollBar *vScroll = verticalScrollBar();

    int startY = visualRect( model()->index( start, 0 ) ).top();
    int endY = visualRect( model()->index( end + 1, 0 ) ).top();

    int scrollValue = vScroll->value() + endY - startY;

    if( startY <= 0 )
        vScroll->setValue( scrollValue );
}


void ThumbnailView::keyPressEvent( QKeyEvent* e )
{
    QListView::keyPressEvent( e );
    switch( e->key() ) {
    // If select key, emit selected signal
    case Qt::Key_Select:
    case Qt::Key_Enter:
        emit selected();
        break;
    default:
        // Ignore
        break;
    }

}
