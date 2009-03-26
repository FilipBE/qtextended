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
#include <QX11EmbedWidget>
#include <QVBoxLayout>

#include "qtopiavideo.h"
#include "qvideosurface.h"

#include "qgenericvideowidget.h"

#define VIDEO_SURFACE_GEOMETRY_DEBUG 0
#define VIDEO_SURFACE_PAINT_DEBUG 0


class QVideoSurfacePrivate {
public:
    QGenericVideoWidget *videoWidget;
    QRegion requestedRegion;
};

QVideoSurface::QVideoSurface( QObject *parent, const QVideoFormatList& )
    :QObject(parent)
{
    d = new QVideoSurfacePrivate;
    d->videoWidget = new QGenericVideoWidget;
    d->videoWidget->show();
}

QVideoSurface::~QVideoSurface()
{
    delete d->videoWidget;
    delete d;
}

int QVideoSurface::winId() const
{
    return d->videoWidget->winId();
}

QWidget *QVideoSurface::videoWidget()
{
    return d->videoWidget;
}

QtopiaVideo::VideoRotation QVideoSurface::rotation() const
{
    return d->videoWidget->rotation();
}

QtopiaVideo::VideoScaleMode QVideoSurface::scaleMode() const
{
    return d->videoWidget->scaleMode();
}

QRect QVideoSurface::geometry() const
{
    return d->videoWidget->geometry();
}

QRegion QVideoSurface::requestedRegion() const
{
    return QRegion(geometry());
}

QVideoFormatList QVideoSurface::supportedFormats() const
{
    return d->videoWidget->supportedFormats();
}

QVideoFormatList QVideoSurface::preferredFormats() const
{
    return d->videoWidget->preferredFormats();
}

void QVideoSurface::renderFrame( const QVideoFrame& frame )
{
    d->videoWidget->renderFrame( frame );
}

void QVideoSurface::setRotation( QtopiaVideo::VideoRotation rotation )
{
    d->videoWidget->setRotation( rotation );
}

void QVideoSurface::setScaleMode( QtopiaVideo::VideoScaleMode scaleMode )
{
    d->videoWidget->setScaleMode( scaleMode );
}

void QVideoSurface::setGeometry( const QRect &r )
{
    setRegion( QRegion(r) );
}

void QVideoSurface::setRegion( const QRegion& region )
{
    d->videoWidget->setGeometry( region.boundingRect() );
}

void QVideoSurface::updateDirtyRegion()
{
}

void QVideoSurface::raise()
{
    d->videoWidget->raise();
}

void QVideoSurface::lower()
{
    d->videoWidget->lower();
}


void QVideoSurface::reloadVideoOutput()
{
}

void QVideoSurface::updateVideoOutputClipRegion()
{
}

