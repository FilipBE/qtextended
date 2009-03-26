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

#include "camerastateprocessor.h"
#include "cameravideosurface.h"
#include <qcameratools.h>
#include <QSet>
#include <QDebug>

static const qreal q_digitalZoomScales[] = { 1.0, 1.25, 1.5, 1.75, 2.0, 2.25, 2.5, 2.75, 4.0, 4.25, 4.5, 4.75, 6.0};

CameraStateProcessor::CameraStateProcessor(QCameraDevice *device, QObject* parent) :
    QObject(parent),
    m_device(device),
    still(0),
    video(0),
    preview(0),
    m_converter(0),
    m_fourccSrcFormat(0),
    m_videoFrameFormatSrc(QVideoFrame::Format_Invalid),
    m_videoFrameFormatDest(QVideoFrame::Format_Invalid),
    m_preview_size(QSize()),
    m_preview_fps(-1),
    m_fourccDestFormat(0),
    m_surface(0)
{

    m_hasVideo = false;
    m_busy = false;
    m_selfStill = false;
    m_doConversion = false;

    preview_active = false;

    //Digital Zoom
    m_doZoom = false;
    m_zoomlevel = 0;
    m_zoomfactor = 1.0;
    m_maxZoom = 12;
    m_minZoom = 0;

    m_opticalRange = QPair<unsigned int, unsigned int>(0,0);
    m_opticallevel = 0;
    m_doOptical = false;

    m_canCropVideoSurface = false;

}

bool CameraStateProcessor::initialize(int rotation)
{
    unsigned int modes = m_device->captureModes();

    //preview
    preview = m_device->previewCapture();
    if (preview && !preview->formats().isEmpty()) {
        QList<unsigned int> keys = preview->formats().keys();
        m_surface = new CameraVideoSurface( keys, rotation);

        if (!m_surface->isValid())
            return false;

        if (m_surface->isDefaultSurface()) {
            // a nateive camera format is not supported
            // can we do a color conversion to this default video surface?
            foreach(unsigned int format, CameraFormatConverter::supportedFormats()) {
                if (preview->formats().keys().contains( format )) {
                    m_doConversion = true;
                    m_fourccSrcFormat = format;
                    m_fourccDestFormat = QtopiaCamera::RGB565;

                    m_videoFrameFormatSrc = qFourccToVideoFormat(format);
                    m_videoFrameFormatDest = QVideoFrame::Format_RGB565;
                    break;
                }
            }
            if (!m_doConversion) {
                qWarning()<<"Camera: Color conversion from " << m_videoFrameFormatSrc << " to "
                            << m_videoFrameFormatDest << " not supported";
                return false;
            }

        } else {
            // select the first format
            m_fourccSrcFormat = m_surface->formatMap(m_videoFrameFormatSrc = m_surface->commonFormats().first());
            m_videoFrameFormatDest = m_videoFrameFormatSrc;

        }

        //device dependant optical zoom
        if(m_hasOptical = preview->hasZoom()) {
            m_opticalRange = preview->zoomRange();
            m_opticallevel = m_opticalRange.first;
            m_doOptical = true;
        }

        connect(m_surface, SIGNAL(videoSurfaceFormatsChanged()), this, SLOT(formatsChanged()));
        connect(preview, SIGNAL(frameReady(QVideoFrame const&)),
                this, SLOT(frameReady(QVideoFrame const&)), Qt::DirectConnection);


        m_canCropVideoSurface = true; //m_videoFrameFormatDest == QVideoFrame::Format_RGB565;
    }
    else
        return false;

    //video
    if( m_hasVideo  = (modes & QCameraDevice::VideoMode )) {
        video = m_device->videoCapture();
        if(video && !video->formats().isEmpty()) {
            connect(video, SIGNAL(readyRead()), this, SLOT(readyVideoRead()));
            video_format = video->formats().keys().first();
        }
        else
            m_hasVideo = false;
    }
    //still
    if ( m_hasStill = (modes & QCameraDevice::StillMode)) {
        still = m_device->stillCapture();
        if(still && !still->formats().isEmpty()) {
            connect(still, SIGNAL(imageReady(QByteArray&,QSize,bool)), this, SLOT(stillReady(QByteArray&,QSize,bool)));
            // Grab the first format for now
            still_format = still->formats().keys().first();
        }
        else
            m_hasStill = false;
    }
    else
        qWarning()<<"Camera does not support still capture";


    return true;
}

CameraStateProcessor::~CameraStateProcessor()
{
    if (m_surface)
        delete m_surface;
}

QSize CameraStateProcessor::defaultPreviewSize()
{
    if(preview)
        return preview->resolution();
    return QSize(-1,-1);
}


void CameraStateProcessor::startPreview()
{
    if(preview_active ) return;
    m_preview_fps = preview->framerate();
    m_preview_size = preview->resolution();
    if(m_doConversion) {
        releaseFormatConverter();
        initFormatConverter(m_preview_size);
    }
    preview->start( m_fourccSrcFormat , m_preview_size, m_preview_fps);
    preview_active = true;
}

void CameraStateProcessor::stopPreview()
{
    if(preview_active) {
        preview->stop();
        preview_active = false;
    }
}

unsigned int CameraStateProcessor::previewFormat() const
{
    if(preview && !preview->formats().isEmpty())
        return preview->format();
    return 0;
}

QSize CameraStateProcessor::defaultVideoSize()
{
    if(video)
        return video->resolution();
    return QSize(-1,-1);
}

QSize CameraStateProcessor::defaultStillSize()
{
    if(still)
        return still->resolution();
    return QSize(-1,-1);
}

QList<QSize> CameraStateProcessor::videoSizes()
{
    if(still)
        return video->formats().value( video_format );
    return QList<QSize>();
}

QList<QSize> CameraStateProcessor::stillSizes()
{
    if(still)
        return still->formats().value( still_format );
    return QList<QSize>();
}

void CameraStateProcessor::start()
{
    startPreview();
}

void CameraStateProcessor::stop()
{
    stopPreview();
}

bool CameraStateProcessor::hasStill()
{
    return m_hasStill;
}

bool CameraStateProcessor::hasVideo()
{
    return m_hasVideo;
}

void CameraStateProcessor::stillReady(QByteArray& buf, QSize size, bool complete)
{
    Q_UNUSED(complete)
    //TODO
    if(still->format() == QtopiaCamera::JPEG) {
        qWarning()<<"Camera: JPEG format not supported";
        //emit stillDone(QContent);
        return;
    }

    unsigned int supportedConversionFormats[4] = { QtopiaCamera::YUYV, QtopiaCamera::UYVY, QtopiaCamera::SBGGR8};
    unsigned int format_match = 0;
    QImage::Format qImgFormat = QImage::Format_Invalid;
    unsigned int destFormat = 0;

    for(int i = 0; i < 4; ++i) {
        if(still->format() == supportedConversionFormats[i]) {
            format_match = supportedConversionFormats[i];
            qImgFormat = QImage::Format_RGB16;
            destFormat = QtopiaCamera::RGB565;
            break;
        }
    }

    if(!format_match) {
        bool rgb24 = still->format() == QtopiaCamera::RGB24;
        bool rgb16 = still->format() == QtopiaCamera::RGB565;
        bool rgb32 = still->format() == QtopiaCamera::RGB32;
        if(rgb24 || rgb16 || rgb32) {
            qImgFormat = (rgb24) ? QImage::Format_RGB888 : ((rgb32) ? QImage::Format_RGB32 : QImage::Format_RGB16);
            destFormat = (rgb24) ? QtopiaCamera::RGB24 : ((rgb32) ? QtopiaCamera::RGB32 : QtopiaCamera::RGB565);
        } else {
            qWarning()<<"Camera: Format not supported: Cannot convert raw still image to QImage format";
            return;
        }
    }

    CameraFormatConverter *img_converter = 0;
    if(format_match) {
        img_converter = CameraFormatConverter::createFormatConverter(format_match, size.width(), size.height());
    }

    QImage img = QImage( ((format_match)?img_converter->convert(reinterpret_cast<uchar*>(buf.data())): reinterpret_cast<uchar*>(buf.data())),
                        size.width(), size.height(), qImgFormat);

    if (m_doZoom)
        doStillZoom(img);

    emit stillDoneRaw(img);

    if(format_match)
        CameraFormatConverter::releaseFormatConverter(img_converter);
}

// data avaiable for reading
void CameraStateProcessor::readyVideoRead()
{
}

void CameraStateProcessor::takeStillImage(QSize resolution, int count)
{
    if(m_hasStill)
        still->takeStillImage(still_format, resolution, count);
}

void CameraStateProcessor::startVideoCapture(QSize resolution)
{
    if(m_hasVideo)
        video->start(video_format, resolution, video->framerate());
}

void CameraStateProcessor::endVideoCapture()
{
    if(m_hasVideo)
        video->stop();
}

void CameraStateProcessor::initFormatConverter(QSize s)
{
    m_converter =  CameraFormatConverter::createFormatConverter(m_fourccSrcFormat, s.width(), s.height());
}

void CameraStateProcessor::releaseFormatConverter()
{
    if(m_converter)
        CameraFormatConverter::releaseFormatConverter(m_converter);
}

void CameraStateProcessor::frameReady(QVideoFrame const& frame)
{
    if (m_doConversion) {
        QVideoFrame vframe (m_videoFrameFormatDest, frame.size(), m_converter->convert(reinterpret_cast<unsigned char*>(const_cast<unsigned char*>(frame.planeData(0)))));

        if(m_doZoom)
            emit previewFrameReady(doPreviewZoom(vframe));
        else
            emit previewFrameReady(vframe);
    } else {
        if (m_doZoom)
             emit previewFrameReady(doPreviewZoom(frame));
        else
            emit previewFrameReady(frame);
    }
}


QList<QSize> CameraStateProcessor::previewSizes()
{
    if(preview)
        return preview->formats().value(preview->format());
    return QList<QSize>();
}

void CameraStateProcessor::zoomIn()
{

    if (m_doOptical && m_hasOptical  && (m_opticallevel+1) <= (int)m_opticalRange.second) {
        ++m_opticallevel;
        preview->zoomIn();
    } else if (m_canCropVideoSurface){
        m_zoomlevel = (m_zoomlevel+1<=m_maxZoom)?++m_zoomlevel:m_zoomlevel;
        m_doZoom = (m_zoomlevel != 0)?true: false;
        m_doOptical = !m_doZoom;
        m_zoomfactor = 1.0/q_digitalZoomScales[m_zoomlevel];
    }
}

void CameraStateProcessor::zoomOut()
{
    if (m_doOptical && m_zoomlevel == 0 && m_hasOptical && (m_opticallevel-1) >= (int)m_opticalRange.first) {
        --m_opticallevel;
        preview->zoomOut();
    } else if (m_canCropVideoSurface) {
        m_zoomlevel = (m_zoomlevel-1>=m_minZoom)?--m_zoomlevel:m_zoomlevel;
        m_doZoom = (m_zoomlevel != 0)?true: false;
        m_doOptical = (m_zoomlevel==0);
        m_zoomfactor = 1.0/q_digitalZoomScales[m_zoomlevel];
    }
}

int CameraStateProcessor::minZoom() const
{
    return (m_hasOptical) ? m_opticalRange.first : m_minZoom;
}

int CameraStateProcessor::maxZoom() const
{
    return (m_hasOptical) ? m_opticalRange.second + m_maxZoom : m_maxZoom;
}

void CameraStateProcessor::doStillZoom(QImage& frame)
{
    QVideoFrame::PixelFormat out;

    switch (frame.format())
    {
        case QImage::Format_RGB32:
            out = QVideoFrame::Format_RGB32;
            break;
        case QImage::Format_RGB888:
            out = QVideoFrame::Format_RGB24;
            break;
        case  QImage::Format_RGB16:
            out = QVideoFrame::Format_RGB565;
            break;
        default:
            qWarning()<<"Format not supported";
            return;
    }

    int fw = frame.width(), fh=frame.height();
    int w = (int)(fw * m_zoomfactor);
    int h = (int)(fh * m_zoomfactor);

    QRect d ((fw-w)>>1,(fh-h)>>1 , w, h);

    QImage crop(frame);

    m_transform.setSrcGeometry(d, QRect(0, 0, fw,fh), fw);
    m_transform.setDstGeometry(QRect(0,0,fw,fh), QRect(0, 0, fw,fh), fw);

    m_transform.transformPlane(crop.bits() , out,
                               frame.bits(), out);

}

QVideoFrame CameraStateProcessor::doPreviewZoom(QVideoFrame const& frame)
{
    int fw=frame.size().width(), fh=frame.size().height();

    int w = (int)(fw * m_zoomfactor);
    int h = (int)(fh * m_zoomfactor);

    QRect d ((fw-w)>>1,(fh-h)>>1 , w, h);
    QVideoFrame crop(QVideoFrame::Format_RGB565, QSize(fw,fh));
    uchar *croppedData = crop.planeData(0);


    m_transform.setSrcGeometry(d, QRect(0, 0, fw,fh), fw);
    m_transform.setDstGeometry(QRect(0,0,fw,fh), QRect(0,0,fw,fh), fw);

    m_transform.transformPlane(frame.planeData(0), QVideoFrame::Format_RGB565,
                               croppedData, QVideoFrame::Format_RGB565);

    return crop;
}

void CameraStateProcessor::autoFocus()
{
    if(m_hasStill)
        still->autoFocus();
}
// slot
void CameraStateProcessor::formatsChanged()
{
}
