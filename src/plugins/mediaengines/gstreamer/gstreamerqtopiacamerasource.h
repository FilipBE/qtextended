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
#ifndef GSTREAMERQTOPIACAMERASOURCE_H
#define GSTREAMERQTOPIACAMERASOURCE_H

#include <gst/gst.h>

class QCameraPreviewCapture;
class QSize;
class QVideoFrame;

namespace gstreamer
{

class QtopiaCameraSourceClass;

class QtopiaCameraSource
{
public:
    GstElement element;
    static void instance_init(GTypeInstance *instance, gpointer g_class);
    static GstStateChangeReturn change_state(GstElement* element, GstStateChange transition);

    void frameReady(const QVideoFrame &frame, int frameRate);

    static GstPad *requestNewPad(GstElement *element, GstPadTemplate *temp, const gchar *name);
    static void releasePad(GstElement *element, GstPad *pad);

private:
    GstCaps *createCaps(const QVideoFrame &frame, int frameRate) const;

    bool m_framesReceived;
};

class QtopiaCameraSourceClass
{
public:
    GstElementClass parent_class;

    static void class_init(gpointer g_class, gpointer class_data);
    static GType get_type();
};

} // ns gstreamer

#endif
