/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
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

#include <QtGui/QPaintDevice>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QWidget>

#include "private/qt_x11_p.h"
#include "qx11info_x11.h"
#include "qwindowsurface_x11_p.h"

QT_BEGIN_NAMESPACE

struct QX11WindowSurfacePrivate
{
    QWidget *widget;
    QPixmap device;
};

QX11WindowSurface::QX11WindowSurface(QWidget *widget)
    : QWindowSurface(widget), d_ptr(new QX11WindowSurfacePrivate)
{
    d_ptr->widget = widget;
}


QX11WindowSurface::~QX11WindowSurface()
{
    delete d_ptr;
}

QPaintDevice *QX11WindowSurface::paintDevice()
{
    return &d_ptr->device;
}


void QX11WindowSurface::flush(QWidget *widget, const QRegion &rgn, const QPoint &offset)
{
    if (d_ptr->device.isNull())
        return;

#ifndef Q_FLATTEN_EXPOSE
    extern void *qt_getClipRects(const QRegion &r, int &num); // in qpaintengine_x11.cpp
    extern QWidgetData* qt_widget_data(QWidget *);
    QPoint wOffset = qt_qwidget_data(widget)->wrect.topLeft();
    GC gc = XCreateGC(X11->display, d_ptr->device.handle(), 0, 0);
    QRegion wrgn(rgn);
    QRect br = rgn.boundingRect();
    if (!wOffset.isNull())
        wrgn.translate(-wOffset);
    QRect wbr = wrgn.boundingRect();

    // #### why dirty widget here? should be up to date already
//     if (br.right() + offset.x() >= d_ptr->device.size().width() || br.bottom() + offset.y() >= d_ptr->device.size().height()) {
// //             QRegion dirty = rgn - QRect(-offset, d_ptr->device.size());
// //             qDebug() << dirty;
//         widget->d_func()->dirtyWidget_sys(rgn - QRect(-offset, d_ptr->device.size()));
//     }

    int num;
    XRectangle *rects = (XRectangle *)qt_getClipRects(wrgn, num);
//         qDebug() << "XSetClipRectangles";
//         for  (int i = 0; i < num; ++i)
//             qDebug() << " " << i << rects[i].x << rects[i].x << rects[i].y << rects[i].width << rects[i].height;
    XSetClipRectangles(X11->display, gc, 0, 0, rects, num, YXBanded);
#else
    Q_UNUSED(rgn);
    XGCValues values;
    values.subwindow_mode = IncludeInferiors;
    GC gc = XCreateGC(X11->display, d_ptr->device.handle(), GCSubwindowMode, &values);
#endif
    XSetGraphicsExposures(X11->display, gc, False);
//         XFillRectangle(X11->display, widget->handle(), gc, 0, 0, widget->width(), widget->height());
#ifndef Q_FLATTEN_EXPOSE
    XCopyArea(X11->display, d_ptr->device.handle(), widget->handle(), gc,
              br.x() + offset.x(), br.y() + offset.y(), br.width(), br.height(), wbr.x(), wbr.y());
#else
    Q_ASSERT(widget->isWindow());
    XCopyArea(X11->display, d_ptr->device.handle(), widget->handle(), gc,
              offset.x(), offset.y(), widget->width(), widget->height(), 0, 0);
#endif
    XFreeGC(X11->display, gc);
}

void QX11WindowSurface::setGeometry(const QRect &rect)
{
    QWindowSurface::setGeometry(rect);

    const QSize size = rect.size();
    if (d_ptr->device.size() == size)
        return;
    QPixmap::x11SetDefaultScreen(d_ptr->widget->x11Info().screen());
    d_ptr->device = QPixmap(size);
}

bool QX11WindowSurface::scroll(const QRegion &area, int dx, int dy)
{
    QRect rect = area.boundingRect();

    if (d_ptr->device.isNull())
        return false;

    GC gc = XCreateGC(X11->display, d_ptr->device.handle(), 0, 0);
    XCopyArea(X11->display, d_ptr->device.handle(), d_ptr->device.handle(), gc,
              rect.x(), rect.y(), rect.width(), rect.height(),
              rect.x()+dx, rect.y()+dy);
    XFreeGC(X11->display, gc);

    return true;
}

QPixmap QX11WindowSurface::grabWidget(const QWidget *widget,
                                      const QRect& rect) const
{
    if (d_ptr->device.isNull())
        return QPixmap();

    QRect br = rect;
    QRect wbr(widget->geometry());

    if (wbr.isNull())
        return QPixmap();

    int w = qMin(rect.size().width(), wbr.size().width());
    if (!w)
        w = qMax(rect.size().width(), wbr.size().width());

    int h = qMin(rect.size().height(), wbr.size().height());
    if (!h)
        h = qMax(rect.size().height(), wbr.size().height());

    if (br.isNull())
        br = wbr;

    QPixmap::x11SetDefaultScreen(widget->x11Info().screen());
    QPixmap px(w, h);

    GC gc = XCreateGC(X11->display, d_ptr->device.handle(), 0, 0);
    XRectangle xrect;
    xrect.x = short(wbr.x());
    xrect.y = short(wbr.y());
    xrect.width = ushort(wbr.width());
    xrect.height = ushort(wbr.height());
    XSetClipRectangles(X11->display, gc, 0, 0, &xrect, 1, YXBanded);
    XSetGraphicsExposures(X11->display, gc, False);
    XCopyArea(X11->display, d_ptr->device.handle(), px.handle(), gc,
              br.x(), br.y(), br.width(), br.height(), 0, 0);
    XFreeGC(X11->display, gc);

    return px;
}

QT_END_NAMESPACE
