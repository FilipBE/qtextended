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

#include <QtGui/qscreen_qws.h>
#include <QtGui/qwsevent_qws.h>
#include <QtGui/qwsdisplay_qws.h>
#include <QtGui/private/qwsdisplay_qws_p.h>
#include <QtGui/private/qwindowsurface_qws_p.h>

#include <qpluginmanager.h>

#include <custom.h>

#include "helixvideowidget.h"
#include "qvideosurface.h"

VideoWidget::VideoWidget(GenericVideoSurface* surface, QWidget* parent):
    m_surface(surface)
{
    HX_ADDREF(m_surface);

    m_videoSurface = new QVideoSurface(0, QVideoFormatList() << QVideoFrame::Format_YUV420P << QVideoFrame::Format_YV12 );

    m_surface->setPaintObserver(this);
}

VideoWidget::~VideoWidget()
{
    m_surface->setPaintObserver(0);
    HX_RELEASE(m_surface);

    delete m_videoSurface;
}


QVideoFormatList VideoWidget::preferredFormats() const
{
    return m_videoSurface->preferredFormats();
}

QVideoFormatList VideoWidget::supportedFormats() const
{
    return m_videoSurface->supportedFormats();
}


QVideoSurface *VideoWidget::videoSurface()
{
    return m_videoSurface;
}

void VideoWidget::paint(  const QVideoFrame& frame  )
{
    m_videoSurface->renderFrame( frame );
}

int VideoWidget::isSupported()
{
    //return screenDepth() == 16 || screenDepth() == 32;
    return true;
}


int VideoWidget::winId() const
{
    return m_videoSurface->winId();
}

