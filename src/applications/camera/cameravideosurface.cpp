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

#include <QDebug>

#include "cameravideosurface.h"
#if defined(Q_WS_QWS)
#include <QtGui/qscreen_qws.h>
#elif defined(Q_WS_X11)
#include <X11/Xlib.h>
#include <QX11Info>
#endif
#include <QMap>
#include <QScreen>

#include <QCameraDevice>

using namespace QtopiaVideo;

static QVideoFrame::PixelFormat nativeScreenPixelFormat()
{
#if defined(Q_WS_QWS)
    if ( qt_screen->depth() == 16 )
         return qt_screen->pixelType() == QScreen::NormalPixel ? QVideoFrame::Format_RGB565 : QVideoFrame::Format_BGR565;
    if ( qt_screen->depth() == 24 )
        return qt_screen->pixelType() == QScreen::NormalPixel ? QVideoFrame::Format_RGB24 : QVideoFrame::Format_BGR24;
    if ( qt_screen->depth() == 32 )
        return qt_screen->pixelType() == QScreen::NormalPixel ? QVideoFrame::Format_RGB32 : QVideoFrame::Format_BGR32;
#elif defined(Q_WS_X11)
    QX11Info info;
    Visual* visual = (Visual*)info.appVisual();
    if ( info.depth() == 16 )
        return (visual->red_mask & 0xF800) ? QVideoFrame::Format_RGB565 : QVideoFrame::Format_BGR565;
    else if ( info.depth() == 24 )
        return (visual->red_mask & 0xff0000 ) ? QVideoFrame::Format_RGB24 : QVideoFrame::Format_BGR24;
    else if ( info.depth() == 32 )
        return (visual->red_mask & 0xff0000 ) ? QVideoFrame::Format_RGB32 : QVideoFrame::Format_BGR32;
#endif

    return QVideoFrame::Format_Invalid;
}

/*
    Represents a video surface for camera output
*/
CameraVideoSurface::CameraVideoSurface(QList<unsigned int>& expectedFormats,int defaultRotation):
    QObject(0),
    m_valid(false),
    m_useNativeFormat(false),
    m_surface(0)
{
    init(expectedFormats, defaultRotation);
}

CameraVideoSurface::~CameraVideoSurface()
{
    if(m_surface)
        delete m_surface;
}

void  CameraVideoSurface::init(QList<unsigned int>& expectedFormats, int rotation)
{

    m_formatMap.insert(0, QVideoFrame::Format_Invalid);
    m_formatMap.insert(QtopiaCamera::RGB32, QVideoFrame::Format_RGB32);
    m_formatMap.insert(QtopiaCamera::RGB24, QVideoFrame::Format_RGB24);
    m_formatMap.insert(QtopiaCamera::RGB565, QVideoFrame::Format_RGB565);
    m_formatMap.insert(QtopiaCamera::YUYV, QVideoFrame::Format_YUYV);
    m_formatMap.insert(QtopiaCamera::UYVY, QVideoFrame::Format_UYVY);


    // convert from fourcc to QVideoFrame format
    foreach (unsigned int format, expectedFormats)
    {
        if (format != 0 )
            if (m_formatMap.contains(format))
                m_cameraFormats.append( m_formatMap[format] );
    }

    m_surface = new QVideoSurface(this, m_cameraFormats );

    if (m_surface == 0) {
        qWarning()<< "Camera: Could not get Video Surface";
        m_valid = false;
        return;
    }

    updateCommonFormats();

    //check if we need to use the direct painter video surface
    if (m_commonFormats.isEmpty() ) {
        delete m_surface;
        m_surface = 0;

        if (nativeScreenPixelFormat() != QVideoFrame::Format_RGB565) {
            qWarning()<<"Camera:  Native pixel format for screen not supported by camera application,"
                      <<" currently only supports conversion to RGB565 format";
            m_valid = false;
            return;

        }
        // use RGB565 video surface
        m_surface = new QVideoSurface(this, QVideoFormatList()<< QVideoFrame::Format_RGB565);

        m_cameraFormats.clear();
        m_cameraFormats.append(QVideoFrame::Format_RGB565);

        qWarning()<<"Camera: Default to rgb565 video surface";
        updateCommonFormats();
        if (m_commonFormats.isEmpty()) {
            qWarning() <<"Camera: Cannot get native screen video surface";
            m_valid = false;
            return;
        }
        m_useNativeFormat = true;
    }
    m_valid = true;
    setRotation( rotation );
    setScaleMode( QtopiaVideo::FitWindow );
    connect(m_surface, SIGNAL(formatsChanged()), this, SLOT(formatsChanged()));
}

//slots
void CameraVideoSurface::formatsChanged()
{
    updateCommonFormats();

    emit videoSurfaceFormatsChanged();
}

/*
    find common intersection between camera/surface formats
*/
void CameraVideoSurface::updateCommonFormats()
{
    FormatSet surface_formats = m_surface->supportedFormats().toSet();
    FormatSet camera_formats = m_cameraFormats.toSet();

    FormatSet intersection = surface_formats.intersect( camera_formats );

    m_commonFormats = intersection.toList();
    if (m_commonFormats.isEmpty())
        qWarning()<<"CameraVideoSurface: No common pixel formats between camera device and video surface";
}


/*
    Returns true if the default video surface is being used
    i.e. direct painter
*/

bool CameraVideoSurface::isDefaultSurface() const
{
    return m_useNativeFormat;
}

/*
    Map the four possible screen rotation values (0,90,180.270) in degrees to a
    QtopiaVideo::VideoRotation
*/
QtopiaVideo::VideoRotation CameraVideoSurface::rotationMap(int rot)
{
    QtopiaVideo::VideoRotation _map[4] = {NoRotation, Rotate90, Rotate180, Rotate270};

    if (rot == 90 || rot == 180 || rot == 270)
        return  _map[(int)rot/90];

    return QtopiaVideo::NoRotation;
}

/*
    Map supported  QVideoFrame format to fourcc format
*/
unsigned int CameraVideoSurface::formatMap(QVideoFrame::PixelFormat p) const
{
    if(!m_formatMap.values().contains(p))
        return 0;
    return m_formatMap.key(p);
}

/*
    Map fourcc format to supported QVideoFrame format
*/
QVideoFrame::PixelFormat CameraVideoSurface::formatMap(unsigned int fourcc) const
{
    if (!m_formatMap.contains(fourcc))
        return QVideoFrame::Format_Invalid;

    return m_formatMap[fourcc];
}

/*
    Inform the video surface tp change orientation
*/
void CameraVideoSurface::setRotation(int rotation)
{
    if (m_valid && m_surface) {
        m_surface->setRotation(m_rotation = rotationMap(rotation));
    }
}

/*
    Inform the video surface how to scale the incoming video frames
*/
void CameraVideoSurface::setScaleMode(QtopiaVideo::VideoScaleMode s)
{
    if (m_valid && m_surface) {
        m_surface->setScaleMode(m_scale = s);
    }
}

QVideoSurface* CameraVideoSurface::surface()
{
    //if (!m_valid) qWarning<<"Camera: Using pointer to  NULL VideoSurface";
    return m_surface;
}

