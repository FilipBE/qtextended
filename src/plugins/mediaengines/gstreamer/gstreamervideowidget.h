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

#ifndef GSTREAMERVIDEOWIDGET_H
#define GSTREAMERVIDEOWIDGET_H

#include <QDirectPainter>

#include "gstreamersinkwidget.h"

class QVideoFrame;
class QVideoSurface;

namespace gstreamer
{

class VideoWidget :
    public SinkWidget
{
public:
    VideoWidget();
    ~VideoWidget();

    // Sink widget
    GstElement* element();

    virtual QVideoSurface *videoSurface();

    //int displayDepth() const;
    void setVideoSize(int width, int height);
    void paint( const QVideoFrame& frame );

    void repaintLastFrame();

    virtual int windowId() const;

private:
    GstElement* m_sink;
    QVideoSurface *m_videoSurface;
};

}   // ns gstreamer

#endif

