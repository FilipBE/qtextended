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

#include <QSize>
#include <QRect>
#include <QPainter>
#include <QPaintEvent>
#include <QtGui/qscreen_qws.h>
#include <QDebug>

#include "gstreamerqtopiavideosink.h"

#include "gstreamervideowidget.h"

#include "qvideosurface.h"

namespace gstreamer
{

/*!
    \class gstreamer::VideoWidget
    \internal
*/

VideoWidget::VideoWidget()
{
    m_videoSurface = new QVideoSurface( 0, QVideoFormatList() << QVideoFrame::Format_YUV420P << QVideoFrame::Format_YV12 );
}

VideoWidget::~VideoWidget()
{
    delete m_videoSurface;
}

GstElement* VideoWidget::element()
{
    m_sink = GST_ELEMENT(g_object_new(QtopiaVideoSinkClass::get_type(), NULL));

    if (m_sink != 0) {
        QtopiaVideoSink*  sink = reinterpret_cast<QtopiaVideoSink*>(m_sink);

        sink->widget = this;
    }

    return m_sink;
}


void VideoWidget::repaintLastFrame()
{
    QtopiaVideoSink*  sink = reinterpret_cast<QtopiaVideoSink*>( m_sink );
    if ( sink )
        sink->renderLastFrame();
}


int VideoWidget::windowId() const
{
    return m_videoSurface->winId();
}

void VideoWidget::setVideoSize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

QVideoSurface *VideoWidget::videoSurface()
{
    return m_videoSurface;
}

void VideoWidget::paint( const QVideoFrame& frame)
{
    m_videoSurface->renderFrame( frame );
}

}   // ns gstreamer

