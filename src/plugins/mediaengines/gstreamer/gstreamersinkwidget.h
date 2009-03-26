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

#ifndef GSTREAMERSINKWIDGET_H
#define GSTREAMERSINKWIDGET_H

#include <QWidget>

#include <gst/video/gstvideosink.h>

class QVideoFrame;
class QVideoSurface;

namespace gstreamer
{

class SinkWidget
{
public:
    virtual ~SinkWidget();

    virtual GstElement* element() = 0;
    virtual QVideoSurface *videoSurface() = 0;

    virtual void setVideoSize(int width, int height) = 0;

    virtual int windowId() const = 0;
    virtual void paint( const QVideoFrame& ) = 0;
    virtual void repaintLastFrame() = 0;
};

}   // ns gstreamer

#endif
