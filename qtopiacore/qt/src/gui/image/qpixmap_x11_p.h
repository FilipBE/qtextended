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

#ifndef QPIXMAPDATA_X11_P_H
#define QPIXMAPDATA_X11_P_H

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

#include "QtGui/qx11info_x11.h"

QT_BEGIN_NAMESPACE

class QX11PaintEngine;

class QX11PixmapData : public QPixmapData
{
public:
    QX11PixmapData(PixelType type);
//     QX11PixmapData(PixelType type, int width, int height);
//     QX11PixmapData(PixelType type, const QImage &image,
//                    Qt::ImageConversionFlags flags);
    ~QX11PixmapData();

    void resize(int width, int height);
    void fromImage(const QImage &image, Qt::ImageConversionFlags flags);
    void copy(const QPixmapData *data, const QRect &rect);

    void fill(const QColor &color);
    QBitmap mask() const;
    void setMask(const QBitmap &mask);
    bool hasAlphaChannel() const;
    void setAlphaChannel(const QPixmap &alphaChannel);
    QPixmap alphaChannel() const;
    QPixmap transformed(const QTransform &transform,
                        Qt::TransformationMode mode) const;
    QImage toImage() const;
    QPaintEngine* paintEngine() const;

    Qt::HANDLE handle() const { return hd; }
    Qt::HANDLE x11ConvertToDefaultDepth();

protected:
    int metric(QPaintDevice::PaintDeviceMetric metric) const;

private:
    friend class QPixmap;
    friend class QBitmap;
    friend class QX11PaintEngine;

    void release();

    QBitmap mask_to_bitmap(int screen) const;
    static Qt::HANDLE bitmap_to_mask(const QBitmap &, int screen);
    void bitmapFromImage(const QImage &image);

    Qt::HANDLE hd;

    int w, h;
    short d;
    uint uninit:1;

    QX11Info xinfo;
    Qt::HANDLE x11_mask;
    Qt::HANDLE picture;
    Qt::HANDLE mask_picture;
    Qt::HANDLE hd2; // sorted in the default display depth
#ifndef QT_NO_XRENDER
    void convertToARGB32();
#endif

    QX11PaintEngine *pengine;
};

QT_END_NAMESPACE

#endif // QPIXMAPDATA_X11_P_H

