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

#ifndef QPIXMAP_MAC_P_H
#define QPIXMAP_MAC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGui/private/qpixmapdata_p.h>
#include <QtGui/private/qpixmapdatafactory_p.h>

QT_BEGIN_NAMESPACE

class QMacPixmapData : public QPixmapData
{
public:
    QMacPixmapData(PixelType type);
    ~QMacPixmapData();

    void resize(int width, int height);
    void fromImage(const QImage &image, Qt::ImageConversionFlags flags);
    void copy(const QPixmapData *data, const QRect &rect);

    int metric(QPaintDevice::PaintDeviceMetric metric) const;
    void fill(const QColor &color);
    QBitmap mask() const;
    void setMask(const QBitmap &mask);
    bool hasAlphaChannel() const;
//     QPixmap transformed(const QTransform &matrix,
//                         Qt::TransformationMode mode) const;
    void setAlphaChannel(const QPixmap &alphaChannel);
    QPixmap alphaChannel() const;
    QImage toImage() const;
    QPaintEngine* paintEngine() const;

private:
    int w, h, d;

    uint has_alpha : 1, has_mask : 1, uninit : 1;

    void macSetHasAlpha(bool b);
    void macGetAlphaChannel(QMacPixmapData *, bool asMask) const;
    void macSetAlphaChannel(const QMacPixmapData *, bool asMask);
    void macCreateCGImageRef();
    void macCreatePixels();
    void macReleaseCGImageRef();
    /*
        pixels stores the pixmap data. pixelsToFree is either 0 or some memory
        block that was bound to a CGImageRef and released, and for which the
        release callback has been called. There are two uses to pixelsToFree:

        1. If pixels == pixelsToFree, then we know that the CGImageRef is done\
           with the data and we can modify pixels without breaking CGImageRef's
           mutability invariant.

        2. If pixels != pixelsToFree and pixelsToFree != 0, then we can reuse
           pixelsToFree later on instead of malloc'ing memory.
    */
    quint32 *pixels;
    quint32 *pixelsToFree;
    uint bytesPerRow;
    QRectF cg_mask_rect;
    CGImageRef cg_data, cg_dataBeingReleased, cg_mask;
#ifndef QT_MAC_NO_QUICKDRAW
    GWorldPtr qd_data, qd_alpha;
    void macQDDisposeAlpha();
    void macQDUpdateAlpha();
#endif

    QPaintEngine *pengine;

    friend class QPixmap;
    friend class QRasterBuffer;
    friend class QRasterPaintEngine;
    friend class QCoreGraphicsPaintEngine;
    friend QPixmap qt_mac_unmultiplyPixmapAlpha(const QPixmap&);
    friend CGImageRef qt_mac_create_imagemask(const QPixmap&, const QRectF&);
    friend quint32 *qt_mac_pixmap_get_base(const QPixmap*);
    friend int qt_mac_pixmap_get_bytes_per_line(const QPixmap*);
    friend void qt_mac_cgimage_data_free(void *, const void*, size_t);
    friend IconRef qt_mac_create_iconref(const QPixmap&);
    friend CGContextRef qt_mac_cg_context(const QPaintDevice*);
};

QT_END_NAMESPACE

#endif // QPIXMAP_MAC_P_H
