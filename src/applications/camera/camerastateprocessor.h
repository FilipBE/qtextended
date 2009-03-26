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

#include <QCameraDevice>
#include <QImagePlaneTransformation>
#include <QImage>
#include <QPair>
#include <QContent>
#include <QPainter>
#include <math.h>

#include "cameraformatconverter.h"

class QVideoFrame;

class CameraVideoSurface;

class CameraStateProcessor : public QObject
{
    Q_OBJECT
public:
    CameraStateProcessor(QCameraDevice *device, QObject* parent = 0);
    ~CameraStateProcessor();

    bool initialize(int);

    //begin preview capturing
    void start();
    void stop();

    void takeStillImage(QSize s, int count = 1);

    void startVideoCapture(QSize resolution);
    void endVideoCapture();

    QSize defaultVideoSize();
    QSize defaultStillSize();
    QSize defaultPreviewSize();

    QList<QSize> videoSizes();
    QList<QSize> stillSizes();
    QList<QSize> previewSizes();

    bool hasVideo();
    bool hasStill();

    void startPreview();
    void stopPreview();
    unsigned int previewFormat() const;
    QSize previewSize() const { return preview->resolution(); }

    void zoomIn();
    void zoomOut();
    int minZoom() const;
    int maxZoom() const;

    void autoFocus();

    CameraVideoSurface* surface() const { return m_surface; }

protected slots:
    void formatsChanged();

private  slots:
    void frameReady(QVideoFrame const&);

    void stillReady(QByteArray &, QSize, bool);
    void readyVideoRead();

signals:
    void stillDone(QContent&);
    void stillDoneRaw(QImage&);
    void videoDone(QContent&);
    void previewFrameReady(QVideoFrame const&);

private:
    QCameraDevice::CaptureMode activeMode;
    QCameraDevice* m_device;

    QCameraStillCapture  *still;
    QCameraVideoCapture *video;
    QCameraPreviewCapture *preview;

    QImage m_stillImage;
    bool m_convertStill;
    bool m_busy;
    bool preview_active;

    unsigned int video_format;
    unsigned int still_format;

    bool m_hasVideo;
    bool m_hasStill;
    bool m_selfStill;

    // from viewfinder
    void initFormatConverter(QSize size);
    void releaseFormatConverter();
    QVideoFrame doPreviewZoom(QVideoFrame const& frame);
    void doStillZoom(QImage& img);
    QImage m_frame;
    CameraFormatConverter *m_converter;
    unsigned int m_fourccSrcFormat;

    QVideoFrame::PixelFormat m_videoFrameFormatSrc;
    QVideoFrame::PixelFormat m_videoFrameFormatDest;

    bool m_preview_active;
    QSize m_preview_size;
    int m_preview_fps;
    unsigned int m_fourccDestFormat;
    bool m_doConversion;

    //zoom
    int m_zoomlevel;
    float m_zoomfactor;
    bool m_doZoom;
    int m_maxZoom;
    int m_minZoom;

    QPair<unsigned int,unsigned int> m_opticalRange;
    bool m_hasOptical;
    int m_opticallevel;
    bool m_doOptical;

    CameraVideoSurface *m_surface;
    QImagePlaneTransformation m_transform;
    bool m_canCropVideoSurface;
};


