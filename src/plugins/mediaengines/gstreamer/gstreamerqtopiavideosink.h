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

#ifndef GSTREAMERQTOPIAVIDEOSINK_H
#define GSTREAMERQTOPIAVIDEOSINK_H

#include <gst/video/gstvideosink.h>

#include <qvideoframe.h>

namespace gstreamer
{

class SinkWidget;

class QtopiaVideoSink
{
public:
    GstVideoSink    videoSink;

    SinkWidget*     widget;
    gint            width;
    gint            height;
    gint            bpp;
    gint            depth;
    QVideoFrame::PixelFormat pixelFormat;
    GstBuffer       *lastBuffer;

    void renderLastFrame();

    static GstCaps* get_caps(GstBaseSink* sink);
    static gboolean set_caps(GstBaseSink* sink, GstCaps* caps);
    static GstStateChangeReturn change_state(GstElement* element, GstStateChange transition);
    static GstFlowReturn render(GstBaseSink* sink, GstBuffer* buf);
    static void base_init(gpointer g_class);
    static void instance_init(GTypeInstance *instance, gpointer g_class);
};

struct QtopiaVideoSinkClass
{
    GstVideoSinkClass   parent_class;

    static void class_init(gpointer g_class, gpointer class_data);
    static GType get_type();
};

}   // ns gstreamer

#endif
