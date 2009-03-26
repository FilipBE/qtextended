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

#ifndef QVIDEOSURFACE_H
#define QVIDEOSURFACE_H

#include <QObject>

#include "qtopiavideo.h"
#include "qvideoframe.h"

class QRect;
class QRegion;
class QScreen;

class QVideoSurfacePrivate;

class QTOPIAVIDEO_EXPORT QVideoSurface : public QObject
{
    friend class QVideoSurfacePrivate;
    Q_OBJECT
public:
    QVideoSurface( QObject *parent=0,
                   const QVideoFormatList& expectedFormats = QVideoFormatList() );
    ~QVideoSurface();

    void setExpectedFormats( const QVideoFormatList& expectedFormats );

    int winId() const;
    QWidget *videoWidget();

    QtopiaVideo::VideoRotation rotation() const;
    QtopiaVideo::VideoScaleMode scaleMode() const;

    QRect geometry() const;
    QRegion requestedRegion() const;

    QVideoFormatList supportedFormats() const;
    QVideoFormatList preferredFormats() const;

public slots:
    void renderFrame( const QVideoFrame& frame );

    virtual void setRotation( QtopiaVideo::VideoRotation );
    virtual void setScaleMode( QtopiaVideo::VideoScaleMode );

    void setGeometry( const QRect& );
    void setRegion( const QRegion& );

    void raise();
    void lower();

signals:
    void formatsChanged();
    void clipRegionChanged( const QRegion& );
    void updateRequested();

private slots:
    void updateDirtyRegion();
    void reloadVideoOutput();
    void updateVideoOutputClipRegion();

private:
    QVideoSurfacePrivate *d;
};

#endif

