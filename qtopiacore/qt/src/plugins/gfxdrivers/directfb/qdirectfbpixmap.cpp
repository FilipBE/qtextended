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

#include "qdirectfbpixmap.h"

#include "qdirectfbscreen.h"
#include "qdirectfbpaintengine.h"

#include <QtGui/qbitmap.h>
#include <directfb.h>

static int global_ser_no = 0;

QDirectFBPixmapData::QDirectFBPixmapData(PixelType pixelType)
    : QPixmapData(pixelType, DirectFBClass), surface(0), engine(0), image(0)
{
    setSerialNumber(0);
}

QDirectFBPixmapData::~QDirectFBPixmapData()
{
    unlockDirectFB();
    if (surface)
        surface->Release(surface);
    delete engine;
}

void QDirectFBPixmapData::resize(int width, int height)
{
    if (width <= 0 || height <= 0) {
        setSerialNumber(0);
        return;
    }

    IDirectFB *dfb = QDirectFBScreen::instance()->dfb();
    if (!dfb)
        qFatal("QDirectFBPixmapData::resize(): "
               "Unable to get DirectFB handle!");

    DFBSurfaceDescription description;
    description.flags = DFBSurfaceDescriptionFlags(DSDESC_WIDTH |
                                                   DSDESC_HEIGHT);
    description.width = width;
    description.height = height;

    DFBResult result = dfb->CreateSurface(dfb, &description, &surface);
    if (result != DFB_OK) {
        DirectFBErrorFatal("QDirectFBPixmapData::resize(): "
                           "Unable to allocate surface", result);
    }

    setSerialNumber(++global_ser_no);
}

void QDirectFBPixmapData::fromImage(const QImage &img,
                                    Qt::ImageConversionFlags)
{
    QImage image;
    if (QDirectFBScreen::getSurfacePixelFormat(img) == DSPF_UNKNOWN)
        image = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    else
        image = img;

    DFBSurfaceDescription description;
    description = QDirectFBScreen::getSurfaceDescription(image);
    IDirectFB *fb = QDirectFBScreen::instance()->dfb();
    DFBResult result;

#ifndef QT_NO_DIRECTFB_PREALLOCATED
    IDirectFBSurface *imgSurface;
    result = fb->CreateSurface(fb, &description, &imgSurface);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBPixmapData::fromImage()", result);
        setSerialNumber(0);
        return;
    }
#ifndef QT_NO_DIRECTFB_PALETTE
    QDirectFBScreen::setSurfaceColorTable(imgSurface, image);
#endif
#endif // QT_NO_DIRECTFB_PREALLOCATED

    description.flags = DFBSurfaceDescriptionFlags(description.flags
                                                   ^ DSDESC_PREALLOCATED);
    result = fb->CreateSurface(fb, &description, &surface);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBPixmapData::fromImage()", result);
        setSerialNumber(0);
        return;
    }

#ifndef QT_NO_DIRECTFB_PALETTE
    QDirectFBScreen::setSurfaceColorTable(surface, image);
#endif

#ifdef QT_NO_DIRECTFB_PREALLOCATED
    char *mem;
    int bpl;
    surface->Lock(surface, DSLF_WRITE, (void**)&mem, &bpl);
    const int w = image.width() * image.depth() / 8;
    for (int i = 0; i < image.height(); ++i) {
        memcpy(mem, image.scanLine(i), w);
        mem += bpl;
    }
    surface->Unlock(surface);
#else
    surface->SetBlittingFlags(surface, DSBLIT_NOFX);
    result = surface->Blit(surface, imgSurface, 0, 0, 0);
    if (result != DFB_OK)
        DirectFBError("QDirectFBPixmapData::fromImage()", result);
    surface->Flip(surface, 0, DSFLIP_NONE);
    imgSurface->Release(imgSurface);
#endif // QT_NO_DIRECTFB_PREALLOCATED

    setSerialNumber(++global_ser_no);
}

void QDirectFBPixmapData::copy(const QPixmapData *data, const QRect &rect)
{
    if (data->classId() != DirectFBClass) {
        QPixmapData::copy(data, rect);
        return;
    }

    IDirectFBSurface *src = static_cast<const QDirectFBPixmapData*>(data)->surface;

    DFBSurfaceDescription description;
    description.flags = DFBSurfaceDescriptionFlags(DSDESC_WIDTH |
                                                   DSDESC_HEIGHT |
                                                   DSDESC_PIXELFORMAT);
    description.width = rect.width();
    description.height = rect.height();
    src->GetPixelFormat(src, &description.pixelformat);

    IDirectFB *fb = QDirectFBScreen::instance()->dfb();

    DFBResult result = fb->CreateSurface(fb, &description, &surface);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBPixmapData::copy()", result);
        setSerialNumber(0);
        return;
    }

#ifndef QT_NO_DIRECTFB_PALETTE
    IDirectFBPalette *palette;
    src->GetPalette(src, &palette);
    surface->SetPalette(surface, palette);
#endif

    surface->SetBlittingFlags(surface, DSBLIT_NOFX);
    const DFBRectangle blitRect = { rect.x(), rect.y(),
                                    rect.width(), rect.height() };
    result = surface->Blit(surface, src, &blitRect, 0, 0);
    if (result != DFB_OK)
        DirectFBError("QDirectFBPixmapData::copy()", result);

    setSerialNumber(++global_ser_no);
}

extern int qt_defaultDpiX();
extern int qt_defaultDpiY();

int QDirectFBPixmapData::metric(QPaintDevice::PaintDeviceMetric metric) const
{
    if (!serialNumber())
        return 0;

    switch (metric) {
    case QPaintDevice::PdmWidth: {
        int w, h;
        surface->GetSize(surface, &w, &h);
        return w;
    }
    case QPaintDevice::PdmHeight: {
        int w, h;
        surface->GetSize(surface, &w, &h);
        return h;
    }
    case QPaintDevice::PdmWidthMM: {
        int w, h;
        surface->GetSize(surface, &w, &h);
        return qRound(w * qreal(25.4) / qt_defaultDpiX());
    }
    case QPaintDevice::PdmHeightMM: {
        int w, h;
        surface->GetSize(surface, &w, &h);
        return qRound(h * qreal(25.4) / qt_defaultDpiY());
    }
    case QPaintDevice::PdmNumColors: {
        return (1 << depth()); // hw: make inline
    }
    case QPaintDevice::PdmDepth: {
        DFBSurfacePixelFormat format;
        surface->GetPixelFormat(surface, &format);
        return QDirectFBScreen::depth(format);
    }
    case QPaintDevice::PdmPhysicalDpiX:
    case QPaintDevice::PdmDpiX: {
        return qt_defaultDpiX();
    }
    case QPaintDevice::PdmPhysicalDpiY:
    case QPaintDevice::PdmDpiY: {
        return qt_defaultDpiY();
    }
    default:
        qCritical("QDirectFBScreen::metric(): Unhandled metric!");
        return 0;
    }
}

void QDirectFBPixmapData::fill(const QColor &color)
{
    if (!serialNumber())
        return;

    Q_ASSERT(surface);

    if (color.alpha() < 255 && !hasAlphaChannel()) {
        // convert to surface supporting alpha channel
        DFBSurfacePixelFormat format;
        surface->GetPixelFormat(surface, &format);
        switch (format) {
        case DSPF_YUY2:
        case DSPF_UYVY:
            format = DSPF_AYUV;
            break;
#if (Q_DIRECTFB_VERSION >= 0x010100)
        case DSPF_RGB444:
            format = DSPF_ARGB4444;
            break;
        case DSPF_RGB555:
#endif
        case DSPF_RGB18:
            format = DSPF_ARGB6666;
            break;
        default:
            format = DSPF_ARGB;
            break;
        }

        DFBSurfaceDescription description;
        description.flags = DFBSurfaceDescriptionFlags(DSDESC_WIDTH |
                                                       DSDESC_HEIGHT |
                                                       DSDESC_PIXELFORMAT);
        surface->GetSize(surface, &description.width, &description.height);
        description.pixelformat = format;
        surface->Release(surface); // release old surface

        IDirectFB *fb = QDirectFBScreen::instance()->dfb();
        DFBResult result = fb->CreateSurface(fb, &description, &surface);
        if (result != DFB_OK) {
            DirectFBError("QDirectFBPixmapData::fill()", result);
            setSerialNumber(0);
            return;
        }
    }

    surface->Clear(surface, color.red(), color.green(), color.blue(),
                   color.alpha());
}

bool QDirectFBPixmapData::hasAlphaChannel() const
{
    if (!serialNumber())
        return false;

    DFBSurfacePixelFormat format;
    surface->GetPixelFormat(surface, &format);
    switch (format) {
    case DSPF_ARGB1555:
    case DSPF_ARGB:
    case DSPF_LUT8:
    case DSPF_AiRGB:
    case DSPF_A1:
    case DSPF_ARGB2554:
    case DSPF_ARGB4444:
    case DSPF_AYUV:
    case DSPF_A4:
    case DSPF_ARGB1666:
    case DSPF_ARGB6666:
    case DSPF_LUT2:
        return true;
    default:
        return false;
    }
}

QPixmap QDirectFBPixmapData::transformed(const QTransform &transform,
                                         Qt::TransformationMode mode) const
{
    if (!surface || transform.type() != QTransform::TxScale
        || mode != Qt::FastTransformation)
    {
        QDirectFBPixmapData *that = const_cast<QDirectFBPixmapData*>(this);
        const QImage *image = that->buffer();
        if (image) { // avoid deep copy
            const QImage transformed = image->transformed(transform, mode);
            that->unlockDirectFB();
            QDirectFBPixmapData *data = new QDirectFBPixmapData(pixelType());
            data->fromImage(transformed, Qt::AutoColor);
            return QPixmap(data);
        }
        return QPixmapData::transformed(transform, mode);
    }

    int w, h;
    surface->GetSize(surface, &w, &h);

    const QSize size = transform.mapRect(QRect(0, 0, w, h)).size();
    if (size.isEmpty())
        return QPixmap();

    QDirectFBPixmapData *data = new QDirectFBPixmapData(pixelType());
    data->resize(size.width(), size.height());

    IDirectFBSurface *dest = data->surface;
    dest->SetBlittingFlags(dest, DSBLIT_NOFX);

    const DFBRectangle srcRect = { 0, 0, w, h };
    const DFBRectangle destRect = { 0, 0, size.width(), size.height() };
    dest->StretchBlit(dest, surface, &srcRect, &destRect);

    return QPixmap(data);
}

QImage QDirectFBPixmapData::toImage() const
{
    if (!surface)
        return QImage();

#ifdef QT_NO_DIRECTFB_PREALLOCATED
    QDirectFBPixmapData *that = const_cast<QDirectFBPixmapData*>(this);
    const QImage *img = that->buffer();
    const QImage copied = img->copy();
    that->unlockDirectFB();
    return copied;
#else

    int w, h;
    surface->GetSize(surface, &w, &h);

    DFBSurfacePixelFormat format;
    surface->GetPixelFormat(surface, &format);

    QImage::Format imageFormat = QDirectFBScreen::getImageFormat(format);
    if (imageFormat == QImage::Format_Invalid)
        imageFormat = QImage::Format_ARGB32_Premultiplied;
    imageFormat = QImage::Format_ARGB32;

    QImage image(w, h, imageFormat);

    DFBSurfaceDescription description;
    description = QDirectFBScreen::getSurfaceDescription(image);

    IDirectFB *fb = QDirectFBScreen::instance()->dfb();
    IDirectFBSurface *imgSurface;
    DFBResult result = fb->CreateSurface(fb, &description, &imgSurface);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBPixmapData::toImage()", result);
        return QImage();
    }

    imgSurface->SetBlittingFlags(imgSurface, DSBLIT_NOFX);
    result = imgSurface->Blit(imgSurface, surface, 0, 0, 0);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBPixmapData::toImage() blit failed", result);
        return QImage();
    }
    imgSurface->Release(imgSurface);

    return image;
#endif // QT_NO_DIRECTFB_PREALLOCATED
}

QPaintEngine* QDirectFBPixmapData::paintEngine() const
{
    if (!engine) {
        QDirectFBPixmapData *that = const_cast<QDirectFBPixmapData*>(this);
        that->engine = new QDirectFBPaintEngine;
    }
    return engine;
}

QImage* QDirectFBPixmapData::buffer()
{
    if (image)
        return image;

    void *mem;
    int w, h, stride;
    DFBSurfacePixelFormat format;

    DFBResult result = surface->Lock(surface, DSLF_WRITE, &mem, &stride);
    if (result != DFB_OK || !mem) {
        DirectFBError("QDirectFBPixmapData::buffer()", result);
        return 0;
    }

    surface->GetSize(surface, &w, &h);
    surface->GetPixelFormat(surface, &format);

    image = new QImage(static_cast<uchar*>(mem), w, h, stride,
                       QDirectFBScreen::getImageFormat(format));
    return image;
}

void QDirectFBPixmapData::unlockDirectFB()
{
    if (!image)
        return;

    surface->Unlock(surface);
    delete image;
    image = 0;
}
