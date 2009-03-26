/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtOpenGL module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include <private/qglpaintdevice_qws_p.h>
#include <private/qgl_p.h>
#include <private/qpaintengine_opengl_p.h>
#include <private/qglwindowsurface_qws_p.h>

QT_BEGIN_NAMESPACE

class QWSGLPaintDevicePrivate
{
public:
    QWidget *widget;
};

class QMetricAccessor : public QWidget {
public:
    int metric(PaintDeviceMetric m) {
        return QWidget::metric(m);
    }
};

QWSGLPaintDevice::QWSGLPaintDevice(QWidget *widget) :
    d_ptr(new QWSGLPaintDevicePrivate)
{
    Q_D(QWSGLPaintDevice);
    d->widget = widget;
}

QWSGLPaintDevice::~QWSGLPaintDevice()
{
    Q_D(QWSGLPaintDevice);
    delete d;
}

QPaintEngine* QWSGLPaintDevice::paintEngine() const
{
    return qt_qgl_paint_engine();
}

int QWSGLPaintDevice::metric(PaintDeviceMetric m) const
{
    Q_D(const QWSGLPaintDevice);
    Q_ASSERT(d->widget);

    return ((QMetricAccessor *) d->widget)->metric(m);
}

QWSGLWindowSurface* QWSGLPaintDevice::windowSurface() const
{
     Q_D(const QWSGLPaintDevice);
     return static_cast<QWSGLWindowSurface*>(d->widget->windowSurface());
}

QT_END_NAMESPACE
