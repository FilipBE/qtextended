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

#ifndef QCAMERADEVICE_H
#define QCAMERADEVICE_H

#include "qvideoframe.h"
#include <QIODevice>
#include <QPair>

#include "qcamera.h"

class QCameraControl;

class QTOPIAVIDEO_EXPORT QControlInterface
{
public:
    virtual ~QControlInterface() {}
    virtual QList<QCameraControl*> controls() const = 0;
    virtual void setValue(quint32 id, int value) = 0;
};


class QTOPIAVIDEO_EXPORT QCameraStreamInterface
{
public:
    virtual ~QCameraStreamInterface() {}
    virtual void start(unsigned int format, QSize resolution, int framerate) = 0;
    virtual void stop() = 0;

    virtual QList<unsigned int> framerates() = 0;
    virtual unsigned int framerate() = 0;
};

class QTOPIAVIDEO_EXPORT QCameraZoomInterface
{
public:
    virtual ~QCameraZoomInterface() {}
    virtual bool hasZoom() const = 0;

    virtual QPair<unsigned int,unsigned int> zoomRange() = 0;

    virtual void zoomIn() = 0;
    virtual void zoomOut() = 0;

};

class QTOPIAVIDEO_EXPORT QCameraPreviewCapture :
    public QObject,
    public QCameraStreamInterface,
    public QCameraZoomInterface
{
    Q_OBJECT
public:
    virtual ~QCameraPreviewCapture() {}
    virtual QtopiaCamera::FormatResolutionMap formats() const = 0;
    virtual unsigned int format()  = 0;

    virtual QSize resolution() = 0;

signals:
    void frameReady(QVideoFrame const& frame);
};


class QTOPIAVIDEO_EXPORT QCameraStillCapture :
    public QObject,
    public QControlInterface
{
    Q_OBJECT
public:

    virtual QtopiaCamera::FormatResolutionMap formats() const = 0;
    virtual unsigned int format()   = 0;

    virtual QSize resolution() = 0;

    virtual void autoFocus() {}

    virtual void takeStillImage(unsigned int format, QSize resolution, int count = 1, unsigned int msecs = 0) = 0;

signals:
    void imageReady(QByteArray& buffer, QSize resolution, bool complete);
    void notifyPreSnap();
};


class QTOPIAVIDEO_EXPORT QCameraVideoCapture :
    public QIODevice,
    public QControlInterface,
    public QCameraStreamInterface
{
    Q_OBJECT
public:

    virtual QtopiaCamera::FormatResolutionMap formats() const = 0;
    virtual unsigned int format()  = 0;
    virtual QSize resolution() = 0;
};


class QTOPIAVIDEO_EXPORT QCameraDevice : public QObject
{
    Q_OBJECT
public:

    enum CaptureMode
    {
        StillMode   = 1,
        VideoMode   = 2
    };

    virtual unsigned int captureModes() const = 0;
    virtual QCameraPreviewCapture* previewCapture() const = 0;
    virtual QCameraStillCapture* stillCapture() const = 0;
    virtual QCameraVideoCapture* videoCapture() const = 0;

    enum Orientation
    {
        FrontFacing,
        BackFacing,
        Changing
    };

    virtual Orientation orientation() const = 0;
    virtual QString description() const = 0;
    virtual QString name() const = 0;

signals:
    void cameraError(QtopiaCamera::CameraError err, QString errorMessage);
};

#endif
