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

#ifndef QIMAGEPLANETRANSFORM
#define QIMAGEPLANETRANSFORM

#include <QRect>

#include "qtopiavideo.h"
#include "qvideoframe.h"


class QImagePlaneTransformationPrivate;

class QTOPIAVIDEO_EXPORT QImagePlaneTransformation
{
public:
    QImagePlaneTransformation();
    ~QImagePlaneTransformation();

    void setSrcGeometry( const QRect& srcRect, const QRect& srcClipRect, int srcLineStep );
    void setDstGeometry( const QRect& dstRect, const QRect& dstClipRect, int dstLineStep );
    void setRotation( QtopiaVideo::VideoRotation rotation );
    void setGeometryAlignment( int align );

    QRect clippedSrcGeometry() const;
    QRect clippedDstGeometry() const;

    void transformPlane( const uchar *src, uchar *dst );
    void transformPlane( const quint16 *src, quint16 *dst );
    void transformPlane( const quint32 *src, quint32 *dst );

    bool isTransformationSupported( QVideoFrame::PixelFormat srcFormat, QVideoFrame::PixelFormat dstFormat );
    bool transformPlane( const void *src, QVideoFrame::PixelFormat srcFormat, void *dst, QVideoFrame::PixelFormat dstFormat );


private:
    Q_DISABLE_COPY( QImagePlaneTransformation );
    QImagePlaneTransformationPrivate *d;
};

#endif

