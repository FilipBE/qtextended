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
#include <QDebug>
#include <QApplication>
#include <QScreen>

#include "qabstractvideooutput.h"

class QAbstractVideoOutputPrivate
{
public:
    QAbstractVideoOutputPrivate()
    :
      rotation( QtopiaVideo::NoRotation ),
      scaleMode(QtopiaVideo::FitWindow),
      screenOrientation(0),
      isModified(true)
    {
    }


    ~QAbstractVideoOutputPrivate()
    {
    }

    QScreen *screen;

    QtopiaVideo::VideoRotation rotation;
    QtopiaVideo::VideoScaleMode scaleMode;

    int screenOrientation;

    QRegion requestedRegion;
    QRegion clipRegion;

    bool isModified;
};


QAbstractVideoOutput::QAbstractVideoOutput( QScreen *screen, QObject *parent ):
    QObject(parent)
{
    d = new QAbstractVideoOutputPrivate();
    d->screen = screen;
}

QAbstractVideoOutput::~QAbstractVideoOutput()
{
    delete d;
}

void QAbstractVideoOutput::renderFrame( const QVideoFrame& frame )
{
    if ( screen() && d->screenOrientation != screen()->transformOrientation() ) {
        d->screenOrientation = screen()->transformOrientation();
        setModified( true );
    }

    doRenderFrame( frame );
}

QScreen * QAbstractVideoOutput::screen() const
{
    return d->screen;
}

QtopiaVideo::VideoRotation QAbstractVideoOutput::rotation() const
{
    return d->rotation;
}

void QAbstractVideoOutput::setRotation( QtopiaVideo::VideoRotation videoRotation )
{
    if ( d->rotation != videoRotation ) {
        d->rotation = videoRotation;
        setModified( true );
    }
}

QtopiaVideo::VideoScaleMode QAbstractVideoOutput::scaleMode() const
{
    return d->scaleMode;
}

void QAbstractVideoOutput::setScaleMode( QtopiaVideo::VideoScaleMode videoScaleMode )
{
    if ( d->scaleMode != videoScaleMode ) {
        d->scaleMode = videoScaleMode;
        setModified( true );
    }
}

QRect QAbstractVideoOutput::geometry() const
{
    return d->requestedRegion.boundingRect();
}

QRegion QAbstractVideoOutput::requestedRegion() const
{
    return d->requestedRegion;
}

void QAbstractVideoOutput::setGeometry( const QRect& geometry )
{
    setRegion( QRegion(geometry) );
}

void QAbstractVideoOutput::setRegion( const QRegion& region )
{
    if ( d->requestedRegion != region ) {
        d->requestedRegion = region;
        setModified( true );
        geometryChanged();
    }
}

QRegion QAbstractVideoOutput::clipRegion() const
{
    return d->clipRegion;
}


void QAbstractVideoOutput::setClipRegion( const QRegion &region )
{
    if ( d->clipRegion != region ) {
        d->clipRegion = region;
        setModified( true );
        clipRegionChanged();
    }
}

/** Return true if there were modifications to image output
 *  parameters, like setRotation() or setClipRegion() since the
 *  last setModified(false) call
 */
bool QAbstractVideoOutput::isModified() const
{
    return d->isModified;
}

void QAbstractVideoOutput::setModified( bool flag )
{
    d->isModified = flag;
}

//!The default implementation does nothing, but some video outputs
// should update the geometry without waiting for drawFrame()
void QAbstractVideoOutput::geometryChanged()
{
}

void QAbstractVideoOutput::clipRegionChanged()
{
}

QRegion QAbstractVideoOutput::deviceMappedClipRegion() const
{
    if ( d->screen ) {
        QRegion r = clipRegion().translated( -d->screen->offset() );
        if ( d->screen->isTransformed() ) {
            QSize s( d->screen->width(), d->screen->height() );
            r = d->screen->mapToDevice( r, s );
        }
        return r;
    } else
        return QRegion();
}

QRect QAbstractVideoOutput::deviceMappedGeometry() const
{
    if ( d->screen ) {
        QRect r = geometry().translated( -d->screen->offset() );
        if ( d->screen->isTransformed() ) {
            QSize s( d->screen->width(), d->screen->height() );
            r = d->screen->mapToDevice( r, s );
        }
        return r;
    } else
        return QRect();
}

