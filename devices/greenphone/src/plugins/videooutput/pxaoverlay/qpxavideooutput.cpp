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

#include "qpxavideooutput.h"
#include <QTransform>
#include <QImage>
#include <QDirectPainter>
#include <QPainter>
#include <QDebug>
#include <QTime>
#include <QMutex>

#include <QtGui/qscreen_qws.h>

#include "pxaoverlay.h"

#include <gfxpainter.h>
#include <unistd.h>


class QPxaVideoOutputPrivate
{
public:
    QPxaVideoOutputPrivate()
        :overlay(0)
    {
    }

    PxaOverlay *overlay;
    QSize videoSize;
    QRect imageRect;
};

QPxaVideoOutput::QPxaVideoOutput(  QScreen *screen, QObject *parent )
    : QAbstractVideoOutput(screen, parent)
    , d( new QPxaVideoOutputPrivate )
{
}

QPxaVideoOutput::~QPxaVideoOutput()
{
    delete d;
}

void QPxaVideoOutput::doRenderFrame( const QVideoFrame& frame )
{
    //qWarning() << "QPxaVideoOutput::renderFrame" << geometry();
    if ( frame.isNull() ) {
        if ( d->overlay )
            d->overlay->fill( 16,128,128 ); // yuv:black
        return;
    }

    if ( frame.size() != d->videoSize ) {
        d->videoSize = frame.size();
        setModified(true);
    }

    //if something has changed, recalculate position of the image:
    if ( isModified() ) {
        setModified(false);

        QRegion paintRegion = deviceMappedClipRegion();
        QRect geometry = deviceMappedGeometry();

        QSize imageSize = frame.size();
        //respect frame aspect ratio
        if ( frame.hasCustomAspectRatio() ) {
            imageSize.setWidth( int(imageSize.height() * frame.aspectRatio()) );
        }

        switch ( effectiveRotation() ) {
            case QtopiaVideo::Rotate0:
            case QtopiaVideo::Rotate180:
                break;
            case QtopiaVideo::Rotate90:
            case QtopiaVideo::Rotate270:
                imageSize = QSize( imageSize.height(), imageSize.width() );
        };

        if ( scaleMode() == QtopiaVideo::FitWindow ) {
            double scaleFactor = qMin( double(geometry.width())/imageSize.width(),
                                       double(geometry.height())/imageSize.height() );

            //don't scale if the size is close to required
            if ( scaleFactor < 0.95 || scaleFactor > 1.1 ) {
                imageSize *= scaleFactor;
            }
        }

        d->imageRect = QRect( QPoint(0,0), imageSize );
        d->imageRect.moveCenter( QPoint( geometry.width()/2, geometry.height()/2 ) );

        if ( d->overlay )
            d->overlay->fill( 16, 128, 128 );//black color in yuv
    }

    if ( d->overlay )
        d->overlay->drawFrame( frame,
                               QRect( QPoint(0,0), frame.size() ),
                               d->imageRect,
                               effectiveRotation() );
}

void QPxaVideoOutput::geometryChanged()
{
    delete d->overlay;
    d->overlay = 0;

    if ( !geometry().isEmpty() ) {
        d->overlay = new PxaOverlay;
        d->overlay->open( deviceMappedGeometry() );
    }
}

QVideoFormatList QPxaVideoOutput::supportedFormats()
{
    QVideoFormatList res;
    res << QVideoFrame::Format_YUV420P << QVideoFrame::Format_YV12;
    return res;
}

QVideoFormatList QPxaVideoOutput::preferredFormats()
{
    QVideoFormatList res;
    res << QVideoFrame::Format_YUV420P << QVideoFrame::Format_YV12;
    return res;
}

QtopiaVideo::VideoRotation QPxaVideoOutput::effectiveRotation() const
{
    return QtopiaVideo::VideoRotation( ( int(rotation()) - screen()->transformOrientation() + 4 ) % 4 );
}


QPxaVideoOutputFactory::QPxaVideoOutputFactory()
    :QObject(), QVideoOutputFactory()
{
    //qWarning() << "QPxaVideoOutputFactory::QPxaVideoOutputFactory";
}

QPxaVideoOutputFactory::~QPxaVideoOutputFactory()
{
    //qWarning() << "QPxaVideoOutputFactory::~QPxaVideoOutputFactory";
}

QVideoFormatList QPxaVideoOutputFactory::supportedFormats()
{
    QVideoFormatList res;
    res << QVideoFrame::Format_YUV420P;
    res << QVideoFrame::Format_YV12;
    return res;
}

QVideoFormatList QPxaVideoOutputFactory::preferredFormats()
{
    return supportedFormats();
}

QAbstractVideoOutput*  QPxaVideoOutputFactory::create( QScreen *screen, QObject *parent )
{
    //qWarning() << "QQPxaVideoOutputFactory::create";
    return new QPxaVideoOutput( screen, parent );
}


QTOPIA_EXPORT_PLUGIN(QPxaVideoOutputFactory);

