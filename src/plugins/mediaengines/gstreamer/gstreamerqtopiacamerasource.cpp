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
#include "gstreamerqtopiacamerasource.h"

#include <QtDebug>
#include <QCameraPreviewCapture>

namespace gstreamer
{

static GstElementClass *camera_source_parent_class = 0;

static GstStaticPadTemplate camera_source_source_template = GST_STATIC_PAD_TEMPLATE(
        "src%d", GST_PAD_SRC, GST_PAD_REQUEST, GST_STATIC_CAPS_ANY);

void QtopiaCameraSource::instance_init(GTypeInstance *instance, gpointer g_class)
{
    Q_UNUSED(g_class);

    QtopiaCameraSource *source = reinterpret_cast<QtopiaCameraSource *>(instance);

    source->m_framesReceived = false;
}

GstStateChangeReturn QtopiaCameraSource::change_state(GstElement* element, GstStateChange transition)
{
    QtopiaCameraSource *source = reinterpret_cast<QtopiaCameraSource *>(element);

    bool noPreroll = false;

    switch (transition) {
    case GST_STATE_CHANGE_READY_TO_PAUSED:
        noPreroll = true;
        break;
    default:
        break;
    }

    GstStateChangeReturn result = GST_ELEMENT_CLASS(camera_source_parent_class)->change_state(
            element, transition);

    switch (transition) {
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
        noPreroll = true;
        break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
        if (source->m_framesReceived) {
            gst_element_send_event(element, gst_event_new_eos());

            source->m_framesReceived = false;
        }
        break;
    default:
        break;
    }

    return noPreroll && result == GST_STATE_CHANGE_SUCCESS
        ? GST_STATE_CHANGE_NO_PREROLL
        : result;
}

void QtopiaCameraSource::frameReady(const QVideoFrame &frame, int frameRate)
{
    if (element.current_state == GST_STATE_PLAYING) {
        const int size = frame.bytesPerLine(0) * frame.planeSize(0).height();

        if (GstBuffer* buffer = gst_buffer_new()) {
            GST_BUFFER_SIZE(buffer) = size;
            GST_BUFFER_MALLOCDATA(buffer) = (guint8*)g_malloc(size);
            GST_BUFFER_DATA(buffer) = GST_BUFFER_MALLOCDATA (buffer);

            if (GST_BUFFER_DATA(buffer) != 0) {
                memcpy(GST_BUFFER_DATA(buffer), frame.planeData(0), size);

                if (GstCaps *caps = createCaps(frame, frameRate)) {
                    gst_buffer_set_caps(buffer, caps);

                    GstClockTime timestamp = GST_CLOCK_TIME_NONE;

                    if (GstClock *clock = gst_element_get_clock(GST_ELEMENT(this))) {
                        timestamp = gst_clock_get_time(clock);

                        gst_object_unref(clock);
                    }

                    GST_BUFFER_TIMESTAMP(buffer) = timestamp;

                    if (!m_framesReceived) {
                        gst_element_send_event(GST_ELEMENT(this), gst_event_new_new_segment(
                                FALSE,
                                frameRate,
                                GST_FORMAT_DEFAULT,
                                timestamp,
                                GST_CLOCK_TIME_NONE,
                                0));

                        m_framesReceived = true;
                    }

                    for (GList *list = element.srcpads; list; list = list->next) {
                        GstPad *pad = GST_PAD(list->data);

                        gst_buffer_ref(buffer);

                        switch (gst_pad_push(pad, buffer)) {
                        case GST_FLOW_CUSTOM_SUCCESS:
                        case GST_FLOW_OK:
                        case GST_FLOW_RESEND:
                        case GST_FLOW_WRONG_STATE:
                            break;
                        case GST_FLOW_NOT_LINKED:
                            qWarning() << "Pad not linked";
                            break;
                        case GST_FLOW_UNEXPECTED:
                            qWarning() << "Unexpected data";
                            break;
                        case GST_FLOW_NOT_NEGOTIATED:
                            qWarning() << "Not negotiated";
                            break;
                        case GST_FLOW_ERROR:
                            qWarning() << "Flow error";
                            break;
                        case GST_FLOW_NOT_SUPPORTED:
                            qWarning() << "Not supported";
                            break;
                        case GST_FLOW_CUSTOM_ERROR:
                        default:
                            break;
                        }
                    }
                }
            }

            gst_buffer_unref(buffer);
        }
    }
}

GstPad *QtopiaCameraSource::requestNewPad(GstElement *element, GstPadTemplate *temp, const gchar *name)
{
    if (GstPad *pad = gst_pad_new_from_template(temp, name)) {
        gst_element_add_pad(element, pad);

        return pad;
    }
    return 0;
}

void QtopiaCameraSource::releasePad(GstElement *element, GstPad *pad)
{
    gst_element_remove_pad(element, pad);
}

GstCaps *QtopiaCameraSource::createCaps(const QVideoFrame &frame, int frameRate) const
{
    int fourcc = 0;
    int depth = 0;
    int bpp = 0;
    int endianess = G_BYTE_ORDER;
    int red = 0;
    int green = 0;
    int blue = 0;

    switch (frame.format()) {
    case QVideoFrame::Format_ARGB32:
        break;
    case QVideoFrame::Format_RGB32:
        depth = 32;
        bpp = 32;
        endianess = G_BIG_ENDIAN;
        red   = 0xFF0000;
        green = 0x00FF00;
        blue  = 0x0000FF;
        break;
    case QVideoFrame::Format_RGB24:
        depth = 24;
        bpp = 24;
        endianess = G_BIG_ENDIAN;
        red   = 0xFF0000;
        green = 0x00FF00;
        blue  = 0x0000FF;
        break;
    case QVideoFrame::Format_RGB565:
        depth = 16;
        bpp = 16;
        endianess = G_BIG_ENDIAN;
        red   = 0xF800;
        green = 0x07E0;
        blue  = 0x001F;
        break;
    case QVideoFrame::Format_BGRA32:
        break;
    case QVideoFrame::Format_BGR32:
        depth = 32;
        bpp = 32;
        endianess = G_BIG_ENDIAN;
        red   = 0x0000FF;
        green = 0x00FF00;
        blue  = 0xFF0000;
        break;
    case QVideoFrame::Format_BGR24:
        depth = 24;
        bpp = 24;
        endianess = G_BIG_ENDIAN;
        red   = 0x0000FF;
        green = 0x00FF00;
        blue  = 0xFF0000;
        break;
    case QVideoFrame::Format_BGR565:
        depth = 16;
        bpp = 16;
        endianess = G_BIG_ENDIAN;
        red   = 0x001F;
        green = 0x07E0;
        blue  = 0xF800;
        break;
    case QVideoFrame::Format_YUV444:
    case QVideoFrame::Format_YUV420P:
    case QVideoFrame::Format_YV12:
        break;
    case QVideoFrame::Format_UYVY:
        fourcc = GST_MAKE_FOURCC('U','Y','V','Y');
        break;
    case QVideoFrame::Format_YUYV:
        fourcc = GST_MAKE_FOURCC('Y','U','Y','2');
        break;
    case QVideoFrame::Format_Y8:
    default:
        break;
    }

    if (fourcc != 0) {
        return gst_caps_new_simple("video/x-raw-yuv",
                "format", GST_TYPE_FOURCC, fourcc,
                "width", G_TYPE_INT, frame.size().width(),
                "height", G_TYPE_INT, frame.size().height(),
                "framerate", GST_TYPE_FRACTION, frameRate, 1,
                NULL);
    } else if (depth != 0) {
        return gst_caps_new_simple("video/x-raw-rgb",
            "bpp", G_TYPE_INT, bpp,
            "depth", G_TYPE_INT, depth,
            "red-mask", G_TYPE_INT, red,
            "green-mask", G_TYPE_INT, green,
            "blue-mask", G_TYPE_INT, blue,
            "endianess", G_TYPE_INT, endianess,
            "width", G_TYPE_INT, frame.size().width(),
            "height", G_TYPE_INT, frame.size().height(),
            "framerate", GST_TYPE_FRACTION, frameRate, 1,
            NULL);
    } else {
        return 0;
    }


}

void QtopiaCameraSourceClass::class_init(gpointer g_class, gpointer class_data)
{
    Q_UNUSED(class_data);

    camera_source_parent_class = GST_ELEMENT_CLASS(g_type_class_peek_parent(g_class));

    GstElementClass *element_class = GST_ELEMENT_CLASS(g_class);

    element_class->change_state = QtopiaCameraSource::change_state;
    element_class->request_new_pad = QtopiaCameraSource::requestNewPad;
    element_class->release_pad = QtopiaCameraSource::releasePad;

    gst_element_class_add_pad_template(
            element_class, gst_static_pad_template_get(&camera_source_source_template));
}

GType QtopiaCameraSourceClass::get_type()
{
    static GType type = 0;

    if (!type) {
        static const GTypeInfo info =
        {
            sizeof(QtopiaCameraSourceClass),             // class_size
            NULL,                                        // base_init
            NULL,                                        // base_finalize

            QtopiaCameraSourceClass::class_init,         // class_init
            NULL,                                        // class_finalize
            NULL,                                        // class_data

            sizeof(QtopiaCameraSource),                  // instance_size
            0,                                           // n_preallocs
            QtopiaCameraSource::instance_init,           // instance_init
            0                                            // value_table
        };

        type = g_type_register_static(
                GST_TYPE_ELEMENT,
                "QtopiaCameraSource",
                &info,
                GTypeFlags(0));
    }
    return type;
}

}   // ns gstreamer
