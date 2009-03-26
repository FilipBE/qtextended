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

#include "qpixmapdata_p.h"
#include <QtGui/qbitmap.h>
#include <QtGui/qimagereader.h>

QT_BEGIN_NAMESPACE

const uchar qt_pixmap_bit_mask[] = { 0x01, 0x02, 0x04, 0x08,
                                     0x10, 0x20, 0x40, 0x80 };

QPixmapData::QPixmapData(PixelType pixelType, int objectId)
    : ref(0), detach_no(0), type(pixelType), id(objectId), ser_no(0)
{

}

QPixmapData::~QPixmapData()
{
}

void QPixmapData::fromFile(const QString &fileName, const char *format,
                           Qt::ImageConversionFlags flags)
{
    const QImage image = QImageReader(fileName, format).read();
    if (image.isNull())
        return;

    fromImage(image, flags);
}

void QPixmapData::copy(const QPixmapData *data, const QRect &rect)
{
    fromImage(data->toImage().copy(rect), Qt::AutoColor);
}

void QPixmapData::setMask(const QBitmap &mask)
{
    if (mask.size().isEmpty()) {
        if (depth() != 1)
            fromImage(toImage().convertToFormat(QImage::Format_RGB32),
                      Qt::AutoColor);
    } else {
        QImage image = toImage();
        const int w = image.width();
        const int h = image.height();

        switch (image.depth()) {
        case 1: {
            const QImage imageMask = mask.toImage().convertToFormat(image.format());
            for (int y = 0; y < h; ++y) {
                const uchar *mscan = imageMask.scanLine(y);
                uchar *tscan = image.scanLine(y);
                int bytesPerLine = image.bytesPerLine();
                for (int i = 0; i < bytesPerLine; ++i)
                    tscan[i] &= mscan[i];
            }
            break;
        }
        default: {
            const QImage imageMask = mask.toImage().convertToFormat(QImage::Format_MonoLSB);
            image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            for (int y = 0; y < h; ++y) {
                const uchar *mscan = imageMask.scanLine(y);
                QRgb *tscan = (QRgb *)image.scanLine(y);
                for (int x = 0; x < w; ++x) {
                    if (!(mscan[x>>3] & qt_pixmap_bit_mask[x&7]))
                        tscan[x] = 0;
                }
            }
            break;
        }
        }
        fromImage(image, Qt::AutoColor);
    }
}

QBitmap QPixmapData::mask() const
{
    QImage image = toImage();
    if (!image.hasAlphaChannel())
        return QBitmap();

    const int w = image.width();
    const int h = image.height();

    QImage mask(w, h, QImage::Format_MonoLSB);
    mask.setNumColors(2);
    mask.setColor(0, QColor(Qt::color0).rgba());
    mask.setColor(1, QColor(Qt::color1).rgba());

    const int bpl = mask.bytesPerLine();

    if (image.depth() < 32)
        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    for (int y = 0; y < h; ++y) {
        QRgb *src = (QRgb *)image.scanLine(y);
        uchar *dest = mask.scanLine(y);
        memset(dest, 0, bpl);
        for (int x = 0; x < w; ++x) {
            if (qAlpha(*src) > 0)
                dest[x >> 3] |= qt_pixmap_bit_mask[x & 7];
            ++src;
        }
    }

    return QBitmap::fromImage(mask);
}

QPixmap QPixmapData::transformed(const QTransform &matrix,
                                 Qt::TransformationMode mode) const
{
    return QPixmap::fromImage(toImage().transformed(matrix, mode));
}

void QPixmapData::setAlphaChannel(const QPixmap &alphaChannel)
{
    QImage image = toImage();
    image.setAlphaChannel(alphaChannel.toImage());
    fromImage(image, Qt::AutoColor);
}

QPixmap QPixmapData::alphaChannel() const
{
    return QPixmap::fromImage(toImage().alphaChannel());
}

void QPixmapData::setSerialNumber(int serNo)
{
    ser_no = serNo;
}

QImage* QPixmapData::buffer()
{
    return 0;
}

QT_END_NAMESPACE
