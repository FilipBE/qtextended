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

#include <QRect>
#include <QRegion>
#include <QMutex>
#include <QEvent>
#include <QTimer>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QWSEmbedWidget>
#include <QPointer>
#include <QThread>

#include "qtopiavideo.h"
#include "qvideosurface.h"
#include "private/qwindowsurface_qws_p.h"
#include "qwsevent_qws.h"
#include "qwsdisplay_qws.h"
#include "qabstractvideooutput.h"
#include "qvideooutputloader.h"

//#define VIDEO_SURFACE_GEOMETRY_DEBUG
//#define VIDEO_SURFACE_PAINT_DEBUG

class QVideoSurfacePrivate
{
public:
    QVideoSurfacePrivate(QVideoSurface *parent)
        :q(parent),
         videoOutput(0),
         surface(0),
         mutex(0),
         disableUpdates(false)
    {
    }

    QVideoSurface *q;
    QAbstractVideoOutput *videoOutput;
    QWSDirectPainterSurface *surface;
    QScreen *screen;
    QPointer< QWSEmbedWidget > videoWidget;

    QRegion prevClipRegion;

    QMutex *mutex;
    bool disableUpdates;

    QRegion requestedRegion;
    QRegion dirtyRegion;
    QTimer dirtyRegionUpdateTimer;

    QVideoFormatList expectedFormats;

    QVideoFormatList supportedFormats;
    QVideoFormatList preferredFormats;
    QtopiaVideo::VideoScaleMode scaleMode;
    QtopiaVideo::VideoRotation rotation;

    void processRegionEvent( QWSRegionEvent *e );
    void processEmbedEvent( QWSEmbedEvent *e );

    static QCoreApplication::EventFilter oldEventFilter;
    static bool eventFilterInstalled;
    static QMap<int,QVideoSurfacePrivate*> videoSurfaces;
};


QCoreApplication::EventFilter QVideoSurfacePrivate::oldEventFilter = 0;
bool QVideoSurfacePrivate::eventFilterInstalled = false;
QMap<int,QVideoSurfacePrivate*> QVideoSurfacePrivate::videoSurfaces;

Q_GLOBAL_STATIC(QMutex, qt_video_surface_mutex);

static bool qws_video_surface_event_filter(void *message, long *res )
{
    QWSEvent* event = reinterpret_cast<QWSEvent *>(message);

    if ( event->type == QWSEvent::Region || event->type == QWSEvent::Embed ) {
        QVideoSurfacePrivate *vo = QVideoSurfacePrivate::videoSurfaces.value( event->window() );
        if ( vo ) {
            if ( event->type == QWSEvent::Region )
                vo->processRegionEvent( static_cast<QWSRegionEvent*>(event) );

            if ( event->type == QWSEvent::Embed )
                vo->processEmbedEvent( static_cast<QWSEmbedEvent*>(event) );

            return true;
        }
    }

    if ( QVideoSurfacePrivate::oldEventFilter )
        return QVideoSurfacePrivate::oldEventFilter( message, res );

    return false;
}

void QVideoSurfacePrivate::processRegionEvent( QWSRegionEvent *e )
{
    QRegion r;
    r.setRects( e->rectangles, e->simpleData.nrectangles );

#ifdef VIDEO_SURFACE_GEOMETRY_DEBUG
    qWarning() << "QVideoSurfacePrivate::processRegionEvent, is allocation event:" << (e->simpleData.type == QWSRegionEvent::Allocation) \
               << "; region br:" << r.boundingRect();
#endif

    if ( r.isEmpty() )
        disableUpdates = true;

    if ( e->simpleData.type == QWSRegionEvent::Allocation ) {
#ifdef VIDEO_SURFACE_GEOMETRY_DEBUG
        qWarning() << "Set clip region to" << r.boundingRect();
#endif

        disableUpdates = true;
        dirtyRegion += surface->region();
        dirtyRegion -= r;

        QRegion oldRegion = videoOutput->clipRegion();

        if ( r != oldRegion ) {
            QMutexLocker locker( mutex );
            surface->setClipRegion( r );
            videoOutput->setClipRegion( r );
            QTimer::singleShot( 20, q, SLOT(updateVideoOutputClipRegion()) );
        }

        disableUpdates = r.isEmpty();

        dirtyRegionUpdateTimer.stop();
        if ( !dirtyRegion.isEmpty() ) {
            dirtyRegionUpdateTimer.start(2000);
        }
    }
}

void QVideoSurfacePrivate::processEmbedEvent( QWSEmbedEvent *e )
{
#ifdef VIDEO_SURFACE_GEOMETRY_DEBUG
    qWarning() << "QVideoSurfacePrivate::processEmbedEvent" << e->region.boundingRect() << "type | Region:" << (e->type|QWSEmbedEvent::Region);
#endif
    if ( e->type | QWSEmbedEvent::Region ) {
        if ( e->region.isEmpty() )
            disableUpdates = true;
        q->setRegion( e->region );
    }
}


QVideoSurface::QVideoSurface( QObject *parent, const QVideoFormatList& expectedFormats )
    :QObject(parent)
{
    d = new QVideoSurfacePrivate(this);
    d->mutex = qt_video_surface_mutex();

    d->expectedFormats = expectedFormats;

    d->surface = new QWSDirectPainterSurface( true );
    d->surface->setLocking(true);
    QVideoSurfacePrivate::videoSurfaces[ winId() ] = d;

    d->screen = qt_screen;
    if ( !d->screen->subScreens().isEmpty() ) {
        d->screen = d->screen->subScreens()[0];
    }

    d->videoOutput = QVideoOutputLoader::instance()->create( d->screen, expectedFormats, this );
    d->preferredFormats = d->videoOutput->preferredFormats();
    d->supportedFormats = d->videoOutput->supportedFormats();
    d->scaleMode = d->videoOutput->scaleMode();
    d->rotation = d->videoOutput->rotation();

    if ( !QVideoSurfacePrivate::eventFilterInstalled ) {
        QVideoSurfacePrivate::oldEventFilter = qApp->setEventFilter( qws_video_surface_event_filter );
        QVideoSurfacePrivate::eventFilterInstalled = true;
    }

    moveToThread( qApp->thread() );

    d->dirtyRegionUpdateTimer.setSingleShot(true);
    connect( &d->dirtyRegionUpdateTimer, SIGNAL(timeout()),
             SLOT(updateDirtyRegion()) );

    QVideoOutputLoader::instance()->load();
}

QVideoSurface::~QVideoSurface()
{
    d->mutex->lock();
    delete d->videoOutput;
    d->videoOutput = 0;
    d->disableUpdates = true;
    QVideoSurfacePrivate::videoSurfaces.remove( winId() );
    d->mutex->unlock();

    //force server to repaint the region
    d->dirtyRegion |= d->surface->region();
    updateDirtyRegion();

    d->surface->setRegion( QRegion() );

    delete d->surface;
    delete d;
}


int QVideoSurface::winId() const
{
    return d->surface->winId();
}


void QVideoSurface::setExpectedFormats( const QVideoFormatList& expectedFormats )
{
    if ( d->expectedFormats != expectedFormats ) {
        QMutexLocker locker( d->mutex );
        d->expectedFormats = expectedFormats;
        reloadVideoOutput();
    }
}


QWidget *QVideoSurface::videoWidget()
{
    if (!d->videoWidget)
        d->videoWidget = new QWSEmbedWidget( winId() );

    return d->videoWidget;
}

QtopiaVideo::VideoRotation QVideoSurface::rotation() const
{
    return d->rotation;
}

QtopiaVideo::VideoScaleMode QVideoSurface::scaleMode() const
{
    return d->scaleMode;
}

QRect QVideoSurface::geometry() const
{
    return d->requestedRegion.boundingRect();
}

QRegion QVideoSurface::requestedRegion() const
{
    return d->requestedRegion;
}

QVideoFormatList QVideoSurface::supportedFormats() const
{
    return d->supportedFormats;
}

QVideoFormatList QVideoSurface::preferredFormats() const
{
    if ( !d->videoOutput ) {
        QMutexLocker locker( d->mutex );
    }
    return d->preferredFormats;
}

void QVideoSurface::renderFrame( const QVideoFrame& frame )
{
    if ( !d->disableUpdates ) {
        bool isMainThread = ( QThread::currentThread() == qApp->thread() );

        if ( isMainThread ) {
            d->surface->beginPaint( requestedRegion() );
            if ( !d->disableUpdates )
                d->videoOutput->renderFrame( frame );
            d->surface->endPaint( requestedRegion() );
        } else {
            if ( !d->surface->hasPendingRegionEvents() ) {
                QMutexLocker locker( d->mutex );
                d->surface->lock();
                if ( !d->disableUpdates && !d->surface->hasPendingRegionEvents() )
                    d->videoOutput->renderFrame( frame );
                d->surface->unlock();
            }
        }
    }
}

void QVideoSurface::setRotation( QtopiaVideo::VideoRotation rotation )
{
    bool changed = false;
    d->mutex->lock();
    if ( d->rotation != rotation ) {
        d->rotation = rotation;
        d->videoOutput->setRotation( rotation );
        changed = true;
    }
    d->mutex->unlock();

    if ( changed )
        emit updateRequested();
}

void QVideoSurface::setScaleMode( QtopiaVideo::VideoScaleMode scaleMode )
{
    bool changed = false;
    d->mutex->lock();
    if ( d->scaleMode != scaleMode ) {
        d->scaleMode = scaleMode;
        d->videoOutput->setScaleMode( scaleMode );
        changed = true;
    }
    d->mutex->unlock();

    if ( changed )
        emit updateRequested();
}

void QVideoSurface::setGeometry( const QRect &r )
{
    setRegion( QRegion(r) );
}

void QVideoSurface::setRegion( const QRegion& region )
{
    QMutexLocker locker( d->mutex );

    if ( d->requestedRegion == region )
        return;

    d->disableUpdates = true;

#ifdef VIDEO_SURFACE_GEOMETRY_DEBUG
    qWarning() << "QVideoSurface::setRegion" << region.boundingRect() << ";   prev region:" << d->requestedRegion.boundingRect();
#endif

    d->requestedRegion = region;

    if ( !region.isEmpty() ) {
        //are we still on the same screen?
        int screenNum = qt_screen->subScreenIndexAt( region.boundingRect().center() );
        QScreen *screen = screenNum<0 ? qt_screen : qt_screen->subScreens()[screenNum];

        if ( screen != d->screen ) {
#ifdef VIDEO_SURFACE_GEOMETRY_DEBUG
            qWarning() << "Screen changed";
#endif
            d->screen = screen;
            reloadVideoOutput();
        }
    }

    //d->videoOutput->setClipRegion( QRegion() );
    d->videoOutput->setRegion( region );
    d->surface->setGeometry( region.boundingRect() );
}


void QVideoSurface::reloadVideoOutput()
{
    delete d->videoOutput;
    d->videoOutput = 0;

    d->videoOutput = QVideoOutputLoader::instance()->create( d->screen, d->expectedFormats, this );
    d->videoOutput->setScaleMode( scaleMode() );
    d->videoOutput->setRotation( rotation() );

    if ( d->videoOutput->supportedFormats() != d->supportedFormats ||
         d->videoOutput->preferredFormats() != d->preferredFormats ) {
        d->preferredFormats = d->videoOutput->preferredFormats();
        d->supportedFormats = d->videoOutput->supportedFormats();
        emit formatsChanged();
    }
}


void QVideoSurface::updateVideoOutputClipRegion()
{
    if ( !d->surface->hasPendingRegionEvents() ) {
        QRegion r = d->videoOutput->clipRegion();
        emit clipRegionChanged(r);
        if ( !(r - d->prevClipRegion).isEmpty() )
            emit updateRequested();
        d->prevClipRegion = r;
    } else
        QTimer::singleShot( 100, this, SLOT(updateVideoOutputClipRegion()) );
}

void QVideoSurface::updateDirtyRegion()
{
    // maybe video output was drawing on the space already belonging to other surface,
    // let's ask the server to update space where we were
    if ( !d->dirtyRegion.isEmpty() ) {
#ifdef VIDEO_SURFACE_GEOMETRY_DEBUG
        qWarning() << "update dirty region";
        if ( d->dirtyRegion.numRects() < 2 )
            qWarning() << d->dirtyRegion.boundingRect();
        else
            qWarning() << d->dirtyRegion;
#endif
        //this doesn't work for surfaces on top of video surface, for example after leaving fullscreeen mode
        //QWSDisplay::instance()->repaintRegion( winId(), 0, true, d->dirtyRegion );
        //d->dirtyRegion = QRegion();

        QDirectPainter dp;
        dp.setRegion( d->dirtyRegion );
        d->dirtyRegion = QRegion();
        //ensure the region is really on top
        QWidget::qwsDisplay()->setAltitude( dp.winId(), 1 /* QWSChangeAltitudeCommand::StaysOnTop */, true );
    }
}

/*!
    Raises the reserved region to the top of the widget stack.

    After this call the reserved region will be visually in front of
    any overlapping widgets.

    \sa lower(), requestedRegion()
*/
void QVideoSurface::raise()
{
    QWidget::qwsDisplay()->setAltitude(winId(), 0 /* QWSChangeAltitudeCommand::Raise */ );
}

/*!
    Lowers the reserved region to the bottom of the widget stack.

    After this call the reserved region will be visually behind (and
    therefore obscured by) any overlapping widgets.

    \sa raise(), requestedRegion()
*/
void QVideoSurface::lower()
{
    QWidget::qwsDisplay()->setAltitude(winId(), -1 /* QWSChangeAltitudeCommand::Lower */ );
}


