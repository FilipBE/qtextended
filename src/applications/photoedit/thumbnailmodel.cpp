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

#include "thumbnailmodel.h"
#include <QThread>
#include <QImage>
#include <QPixmap>
#include <QCache>
#include <QImageReader>
#include <QMutex>
#include <QWaitCondition>
#include <QMetaType>
#include <QDrmContent>
#include <QPainter>
#include <QScrollBar>
#include <Qtopia>
#include <QtDebug>

Q_DECLARE_METATYPE(Qt::AspectRatioMode)

class ThumbnailContentSetModelLoader : public QObject
{
    Q_OBJECT
public slots:
    void loadThumbnail( const QContent &content, const QSize &size, Qt::AspectRatioMode mode );

signals:
    void thumbnailLoaded( const QImage &image );
};

class ThumbnailContentSetModelThread : public QThread
{
    Q_OBJECT
public:
    ThumbnailContentSetModel *model;

    QMutex syncMutex;
    QWaitCondition syncCondition;

signals:
    void loadThumbnail( const QContent &content, const QSize &size, Qt::AspectRatioMode mode );

protected:
    void run();

    friend class ThumbnailContentSetModel;
};

void ThumbnailContentSetModelLoader::loadThumbnail( const QContent &content, const QSize &size, Qt::AspectRatioMode mode )
{
    QImage image = content.thumbnail( size, mode );

    emit thumbnailLoaded( image );
}

void ThumbnailContentSetModelThread::run()
{
    ThumbnailContentSetModelLoader loader;

    connect( this,  SIGNAL(loadThumbnail(QContent,QSize,Qt::AspectRatioMode)),
             &loader, SLOT(loadThumbnail(QContent,QSize,Qt::AspectRatioMode)) );

    connect( &loader, SIGNAL(thumbnailLoaded(QImage)),
             model,     SLOT(thumbnailLoaded(QImage)) );

    {
        QMutexLocker locker( &syncMutex );

        syncCondition.wakeAll();
    }

    exec();
}

ThumbnailContentSetModel::ThumbnailContentSetModel( QContentSet *set, QObject *parent )
    : QContentSetModel( set, parent )
    , m_thumbnailSize( 32, 32 )
    , m_aspectRatioMode( Qt::KeepAspectRatio )
    , m_thread( new ThumbnailContentSetModelThread )
    , m_view( 0 )
{
    static const int metaId = qRegisterMetaType< Qt::AspectRatioMode >();

    Q_UNUSED(metaId);

    connect( set, SIGNAL(changed(QContentIdList,QContent::ChangeType)), this, SLOT(contentChanged(QContentIdList,QContent::ChangeType)) );

    m_thread->model = this;

    QMutexLocker locker( &m_thread->syncMutex );

    m_thread->start();

    m_thread->syncCondition.wait( &m_thread->syncMutex );
}

ThumbnailContentSetModel::~ThumbnailContentSetModel()
{
    m_view = 0;

    m_thread->exit();

    m_thread->wait();

    delete m_thread;
}

QSize ThumbnailContentSetModel::thumbnailSize() const
{
    return m_thumbnailSize;
}

void ThumbnailContentSetModel::setThumbnailSize( const QSize &size )
{
    m_thumbnailSize = size;
}

Qt::AspectRatioMode ThumbnailContentSetModel::aspectRatioMode() const
{
    return m_aspectRatioMode;
}

void ThumbnailContentSetModel::setAspectRatioMode( Qt::AspectRatioMode mode )
{
    m_aspectRatioMode = mode;
}

QAbstractItemView *ThumbnailContentSetModel::view() const
{
    return m_view;
}

void ThumbnailContentSetModel::setView( QAbstractItemView *view )
{
    m_view = view;
}

QVariant ThumbnailContentSetModel::data( const QModelIndex &index, int role ) const
{
    if( role == Qt::DecorationRole && index.isValid() )
    {
        QPixmap pixmap = thumbnail( index, content( index ) );

        if( !pixmap.isNull() )
            return pixmap;
    }

    return QContentSetModel::data( index, role );
}

void ThumbnailContentSetModel::thumbnailLoaded( const QImage &image )
{
    QPair< QPersistentModelIndex, QContent > head = m_requestQueue.takeFirst();

    if( !image.isNull() )
    {
        QPixmap pixmap = QPixmap::fromImage( image );

        m_cache.insert( head.second.id(), new QPixmap( pixmap ) );

        if( head.first.isValid() )
            emit dataChanged( head.first, head.first );
    }

    while( !m_requestQueue.isEmpty() )
    {
        if( m_requestQueue.first().first.isValid() && (
            !m_view || m_view->visualRect( m_requestQueue.first().first ).intersects( m_view->rect() ) ) )
        {
            m_thread->loadThumbnail( m_requestQueue.first().second, m_thumbnailSize, m_aspectRatioMode );

            return;
        }
        else
        {
            m_cache.remove( m_requestQueue.first().second.id() );

            m_requestQueue.removeFirst();
        }
    }
}

QPixmap ThumbnailContentSetModel::thumbnail( const QModelIndex &index, const QContent &content ) const
{
    QPixmap *pixmap = m_cache.object( content.id() );

    if( pixmap )
    {
        return *pixmap;
    }
    else
    {
        m_requestQueue.append( QPair< QPersistentModelIndex, QContent >( index, content ) );

        m_cache.insert( content.id(), new QPixmap );

        if( m_requestQueue.count() == 1 )
            m_thread->loadThumbnail( m_requestQueue.first().second, m_thumbnailSize, m_aspectRatioMode );

        return QPixmap();
    }
}

void ThumbnailContentSetModel::contentChanged( const QContentIdList &contentIds, QContent::ChangeType type )
{
    if( type != QContent::Added )
        foreach( QContentId contentId, contentIds )
            m_cache.remove( contentId );
}

ContentThumbnailDelegate::ContentThumbnailDelegate( QObject *parent )
    : QItemDelegate( parent )
{
}

void ContentThumbnailDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    Q_ASSERT(index.isValid());

    QStyleOptionViewItemV3 opt = setOptions(index, option);

    const QStyleOptionViewItemV2 *v2 = qstyleoption_cast<const QStyleOptionViewItemV2 *>(&option);
    opt.features = v2 ? v2->features
                    : QStyleOptionViewItemV2::ViewItemFeatures(QStyleOptionViewItemV2::None);
    const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option);
    opt.locale = v3 ? v3->locale : QLocale();
    opt.widget = v3 ? v3->widget : 0;

    opt.decorationAlignment = Qt::AlignCenter;

    // prepare
    painter->save();
    if (hasClipping())
        painter->setClipRect(opt.rect);

    // get the data and the rectangles

    QVariant value;

    QPixmap pixmap;
    QRect decorationRect;
    value = index.data(Qt::DecorationRole);
    if (value.isValid()) {
        // ### we need the pixmap to call the virtual function
        pixmap = decoration(opt, value);
    }
    decorationRect = QRect(QPoint(0, 0), option.decorationSize);

    QString text;
    QRect displayRect = opt.rect;
    if( opt.decorationPosition == QStyleOptionViewItem::Top ||
        opt.decorationPosition == QStyleOptionViewItem::Bottom )
        displayRect.setWidth( opt.decorationSize.width());
    value = index.data(Qt::DisplayRole);
    if (value.isValid()) {
        text = value.toString();
        displayRect = textRectangle(painter, displayRect , opt.font, text);
    }

    QRect checkRect;
    Qt::CheckState checkState = Qt::Unchecked;
    value = index.data(Qt::CheckStateRole);
    if (value.isValid()) {
        checkState = static_cast<Qt::CheckState>(value.toInt());
        checkRect = check(opt, opt.rect, value);
    }

    // do the layout

    doLayout(opt, &checkRect, &decorationRect, &displayRect, false);

    // draw the item

    drawBackground(painter, opt, index);
    drawCheck(painter, opt, checkRect, checkState);
    drawDecoration(painter, opt, decorationRect, pixmap);
    drawDisplay(painter, opt, displayRect, text);
    drawFocus(painter, opt, displayRect);

    // done
    painter->restore();
}

QSize ContentThumbnailDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    QVariant value = index.data(Qt::SizeHintRole);
    if (value.isValid())
        return qvariant_cast<QSize>(value);
    QRect decorationRect( QPoint( 0, 0 ), option.decorationSize );
    QRect displayRect = rect(option, index, Qt::DisplayRole);
    QRect checkRect = rect(option, index, Qt::CheckStateRole);

    if( option.decorationPosition == QStyleOptionViewItem::Top || option.decorationPosition == QStyleOptionViewItem::Bottom )
        displayRect.setWidth( option.decorationSize.width() );

    doLayout(option, &checkRect, &decorationRect, &displayRect, true);

    return (decorationRect|displayRect|checkRect).size();
}

ContentThumbnailView::ContentThumbnailView( QWidget *parent )
    : QListView( parent )
{
}


void ContentThumbnailView::rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end )
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

void ContentThumbnailView::rowsInserted( const QModelIndex &parent, int start, int end )
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


void ContentThumbnailView::resizeEvent( QResizeEvent*e )
{
    QListView::resizeEvent( e );
    scrollTo( currentIndex() );
}

#include "thumbnailmodel.moc"
