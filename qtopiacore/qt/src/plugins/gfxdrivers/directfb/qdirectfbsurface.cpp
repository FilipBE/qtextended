/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the plugins of the Qt Toolkit.
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

#include "qdirectfbsurface.h"
#include "qdirectfbscreen.h"
#include "qdirectfbpaintengine.h"

#include <qwidget.h>
#include <qpaintdevice.h>
#include <qvarlengtharray.h>

QDirectFBSurface::QDirectFBSurface()
    : QCustomRasterPaintDevice(0)
#ifndef QT_NO_DIRECTFB_WM
    , dfbWindow(0)
#endif
    , dfbSurface(0), engine(new QDirectFBPaintEngine), surfaceImage(0)
{
    setSurfaceFlags(Opaque | Buffered);
}

QDirectFBSurface::QDirectFBSurface(QWidget *widget)
    : QWSWindowSurface(widget), QCustomRasterPaintDevice(widget)
#ifndef QT_NO_DIRECTFB_WM
    , dfbWindow(0)
#endif
    , dfbSurface(0), engine(new QDirectFBPaintEngine), surfaceImage(0)
{
    onscreen = widget->testAttribute(Qt::WA_PaintOnScreen);
    if (onscreen)
        setSurfaceFlags(Opaque | RegionReserved);
    else
        setSurfaceFlags(Opaque | Buffered);
}

QDirectFBSurface::~QDirectFBSurface()
{
}

bool QDirectFBSurface::isValid() const
{
    return true;
}

#ifndef QT_NO_DIRECTFB_WM
void QDirectFBSurface::createWindow()
{
    IDirectFBDisplayLayer *layer = QDirectFBScreen::instance()->dfbDisplayLayer();
    if (!layer)
        qFatal("QDirectFBWindowSurface: Unable to get primary display layer!");

    DFBWindowDescription  description;
    description.caps = DFBWindowCapabilities(DWCAPS_NODECORATION |
                                             DWCAPS_ALPHACHANNEL);
    description.flags = DWDESC_CAPS;

    DFBResult result = layer->CreateWindow(layer, &description, &dfbWindow);
    if (result != DFB_OK)
        DirectFBErrorFatal("QDirectFBWindowSurface::createWindow", result);

    dfbWindow->GetSurface(dfbWindow, &dfbSurface);
}
#endif // QT_NO_DIRECTFB_WM

void QDirectFBSurface::setGeometry(const QRect &rect, const QRegion &mask)
{
    if (rect.isNull()) {
#ifndef QT_NO_DIRECTFB_WM
        if (dfbWindow) {
            dfbWindow->Destroy(dfbWindow);
            dfbWindow = 0;
        }
#endif
        if (dfbSurface) {
            dfbSurface->Release(dfbSurface);
            dfbSurface = 0;
        }
    } else if (rect != geometry()) {
        const bool isResize = rect.size() != geometry().size();
        DFBResult result = DFB_OK;

        IDirectFBSurface *s = QDirectFBScreen::instance()->dfbSurface();
        if (onscreen && s) {
            if (dfbSurface)
                dfbSurface->Release(dfbSurface);

            DFBRectangle r = { rect.x(), rect.y(),
                               rect.width(), rect.height() };
            result = s->GetSubSurface(s, &r, &dfbSurface);
        } else {
#ifndef QT_NO_DIRECTFB_WM
            const QRect oldRect = geometry();
            const bool isMove = oldRect.isEmpty() ||
                                rect.topLeft() != oldRect.topLeft();

            if (!dfbWindow)
                createWindow();

            if (isResize && isMove)
                result = dfbWindow->SetBounds(dfbWindow, rect.x(), rect.y(),
                                              rect.width(), rect.height());
            else if (isResize)
                result = dfbWindow->Resize(dfbWindow,
                                           rect.width(), rect.height());
            else if (isMove)
                result = dfbWindow->MoveTo(dfbWindow, rect.x(), rect.y());
#else
            if (isResize) {
                if (dfbSurface)
                    dfbSurface->Release(dfbSurface);

                IDirectFB *dfb = QDirectFBScreen::instance()->dfb();
                if (!dfb) {
                    qFatal("QDirectFBWindowSurface::setGeometry(): "
                           "Unable to get DirectFB handle!");
                }

                DFBSurfaceDescription description;
                description.flags = DFBSurfaceDescriptionFlags(DSDESC_WIDTH |
                                                               DSDESC_HEIGHT |
                                                               DSDESC_PIXELFORMAT);
                description.width = rect.width();
                description.height = rect.height();
                description.pixelformat = DSPF_ARGB;

                result = dfb->CreateSurface(dfb, &description, &dfbSurface);
            } else {
                Q_ASSERT(dfbSurface);
            }
#endif
        }

        if (result != DFB_OK)
            DirectFBErrorFatal("QDirectFBSurface::setGeometry()", result);
    }

    QWSWindowSurface::setGeometry(rect, mask);
}

QByteArray QDirectFBSurface::permanentState() const
{
    QByteArray array;
#ifdef QT_NO_DIRECTFB_WM
    array.resize(sizeof(SurfaceFlags) + sizeof(IDirectFBSurface*));
#else
    array.resize(sizeof(SurfaceFlags));
#endif
    char *ptr = array.data();

    *reinterpret_cast<SurfaceFlags*>(ptr) = surfaceFlags();
    ptr += sizeof(SurfaceFlags);

#ifdef QT_NO_DIRECTFB_WM
    *reinterpret_cast<IDirectFBSurface**>(ptr) = dfbSurface;
#endif
    return array;
}

void QDirectFBSurface::setPermanentState(const QByteArray &state)
{
    SurfaceFlags flags;
    const char *ptr = state.constData();

    flags = *reinterpret_cast<const SurfaceFlags*>(ptr);
    setSurfaceFlags(flags);

#ifdef QT_NO_DIRECTFB_WM
    ptr += sizeof(SurfaceFlags);
    dfbSurface = *reinterpret_cast<IDirectFBSurface* const*>(ptr);
#endif
}

bool QDirectFBSurface::scroll(const QRegion &region, int dx, int dy)
{
    if (!dfbSurface)
        return false;

    const QVector<QRect> rects = region.rects();
    const int n = rects.size();

    QVarLengthArray<DFBRectangle, 8> dfbRects(n);
    QVarLengthArray<DFBPoint, 8> dfbPoints(n);

    for (int i = 0; i < n; ++i) {
        const QRect r = rects.at(i);
        dfbRects[i].x = r.x();
        dfbRects[i].y = r.y();
        dfbRects[i].w = r.width();
        dfbRects[i].h = r.height();
        dfbPoints[i].x = r.x() + dx;
        dfbPoints[i].y = r.y() + dy;
    }

    dfbSurface->SetBlittingFlags(dfbSurface, DSBLIT_NOFX);
    dfbSurface->BatchBlit(dfbSurface, dfbSurface,
                          dfbRects.data(), dfbPoints.data(), n);

    return true;
}

bool QDirectFBSurface::move(const QPoint &offset)
{
    QWSWindowSurface::move(offset);

#ifdef QT_NO_DIRECTFB_WM
    return true; // buffered
#else
    if (!dfbWindow)
        return false;

    DFBResult status = dfbWindow->Move(dfbWindow, offset.x(), offset.y());
    return (status == DFB_OK);
#endif
}

QRegion QDirectFBSurface::move(const QPoint &offset, const QRegion &newClip)
{
#ifdef QT_NO_DIRECTFB_WM
    return QWSWindowSurface::move(offset, newClip);
#else
    Q_UNUSED(offset);
    Q_UNUSED(newClip);

    // DirectFB handles the entire move, so there's no need to blit.
    return QRegion();
#endif
}

QPaintEngine* QDirectFBSurface::paintEngine() const
{
    return engine;
}

// hw: XXX: copied from QWidgetPrivate::isOpaque()
inline bool isWidgetOpaque(const QWidget *w)
{
    if (w->testAttribute(Qt::WA_OpaquePaintEvent)
        || w->testAttribute(Qt::WA_PaintOnScreen))
        return true;

    const QPalette &pal = w->palette();

    if (w->autoFillBackground()) {
        const QBrush &autoFillBrush = pal.brush(w->backgroundRole());
        if (autoFillBrush.style() != Qt::NoBrush && autoFillBrush.isOpaque())
            return true;
    }

    if (!w->testAttribute(Qt::WA_NoSystemBackground)) {
        const QBrush &windowBrush = w->palette().brush(QPalette::Window);
        if (windowBrush.style() != Qt::NoBrush && windowBrush.isOpaque())
            return true;
    }

    return false;
}

void QDirectFBSurface::flush(QWidget *widget, const QRegion &region,
                             const QPoint &offset)
{
    QWidget *win = window();

    // hw: make sure opacity information is updated before compositing
    const bool opaque = isWidgetOpaque(win);
    if (opaque != isOpaque()) {
        SurfaceFlags flags = Buffered;
        if (opaque)
            flags |= Opaque;
        setSurfaceFlags(flags);
    }

#ifndef QT_NO_DIRECTFB_WM
    const quint8 winOpacity = quint8(win->windowOpacity() * 255);
    quint8 opacity;

    if (dfbWindow) {
        dfbWindow->GetOpacity(dfbWindow, &opacity);
        if (winOpacity != opacity)
            dfbWindow->SetOpacity(dfbWindow, winOpacity);
    }
#endif

    // XXX: have to call the base function first as the decoration is
    // currently painted there
    QWSWindowSurface::flush(widget, region, offset);

#ifndef QT_NO_DIRECTFB_WM
    const QRect br = region.boundingRect().translated(painterOffset());
    DFBRegion r = { br.topLeft().x(), br.topLeft().y(),
                    br.bottomRight().x(), br.bottomRight().y() };

    dfbSurface->Flip(dfbSurface, &r, DSFLIP_NONE);
#endif
}

void* QDirectFBSurface::memory() const
{
    if (!surfaceImage)
        const_cast<QDirectFBSurface*>(this)->lockDirectFB();

    return surfaceImage ? surfaceImage->bits() : 0;
}

int QDirectFBSurface::bytesPerLine() const
{
    if (!surfaceImage)
        const_cast<QDirectFBSurface*>(this)->lockDirectFB();

    return surfaceImage ? surfaceImage->bytesPerLine() : 0;
}

void QDirectFBSurface::beginPaint(const QRegion &)
{
}

void QDirectFBSurface::endPaint(const QRegion &)
{
    qDeleteAll(bufferImages);
    unlockDirectFB();
}

bool QDirectFBSurface::lockDirectFB()
{
    if (surfaceImage) // already locked
        return true;

    void *mem;
    int stride;

    DFBResult result = dfbSurface->Lock(dfbSurface, DSLF_WRITE, &mem, &stride);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBPixmapData::buffer()", result);
        return false;
    }

    int w, h;
    dfbSurface->GetSize(dfbSurface, &w, &h);

    DFBSurfacePixelFormat format;
    dfbSurface->GetPixelFormat(dfbSurface, &format);

    surfaceImage = new QImage(static_cast<uchar*>(mem), w, h, stride,
                              QDirectFBScreen::getImageFormat(format));
    return true;
}

void QDirectFBSurface::unlockDirectFB()
{
    if (!surfaceImage)
        return;

    dfbSurface->Unlock(dfbSurface);
    delete surfaceImage;
    surfaceImage = 0;
}

QImage* QDirectFBSurface::buffer(const QWidget *widget)
{
    if (!surfaceImage)
        return 0;

    const QRect rect = QRect(offset(widget), widget->size())
                       & surfaceImage->rect();
    if (rect.isEmpty())
        return 0;

    QImage *img = new QImage(surfaceImage->scanLine(rect.y())
                             + rect.x() * (surfaceImage->depth() / 8),
                             rect.width(), rect.height(),
                             surfaceImage->bytesPerLine(),
                             surfaceImage->format());
    bufferImages.append(img);

    return img;
}

