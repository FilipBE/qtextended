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

#include "qimageplanetransform.h"
#include <QVector>
#include <QDebug>
#include "def_blendhelper.h"

#define CHECK_SRC_BOUNDARIES 0


//internal, cut line [from,to] to range [fitFrom,fitTo], with cutting corresponding line [from2,to2].
//if mirrored is true, cutting out from left part [from,to], cuts from right part of [from2,to2]
static void cutRanges( int *from, int *to, int fitFrom, int fitTo,
                       int *from2, int *to2, bool mirrored )
{
    int l1 = *to - *from+1;
    int l2 = *to2 - *from2+1;

    if ( *from < fitFrom ) {
        if ( mirrored )
            *to2 -= ( fitFrom - *from ) * l2 / l1;
        else
            *from2 += ( fitFrom - *from) * l2 / l1;

        *from = fitFrom;
    }

    if ( *to > fitTo ) {
        if ( mirrored )
            *from2 += ( *to - fitTo ) * l2 / l1;
        else
            *to2 -= ( *to - fitTo ) * l2 / l1;
        *to = fitTo;
    }
}

static void cutRanges( int *from, int *to, int fitFrom, int fitTo,
                       int *from2, int *to2, int fitFrom2, int fitTo2, bool mirrored )
{
    cutRanges( from, to, fitFrom, fitTo,
               from2, to2, mirrored );

    cutRanges( from2, to2, fitFrom2, fitTo2,
               from, to, mirrored );
}

//internal: clip both srcRect and dstRect to fit srcArea and dstArea with maintaining the srcRect/dstRect aspect ratio
static void clipRects( QRect* srcRect, QRect srcArea,
                QRect* dstRect, QRect dstArea,
                QtopiaVideo::VideoRotation rotation )
{
    if ( !srcRect->intersects( srcArea ) || !dstRect->intersects( dstArea ) )  {
        *srcRect = QRect();
        *dstRect = QRect();
        return;
    }

    int sx1, sy1, sx2, sy2;
    srcRect->normalized().getCoords( &sx1,  &sy1, &sx2, &sy2 );
    int dx1, dy1, dx2, dy2;
    dstRect->normalized().getCoords( &dx1,  &dy1, &dx2, &dy2 );

    switch ( rotation ) {
    case QtopiaVideo::Rotate0:
        cutRanges( &sx1, &sx2, srcArea.left(), srcArea.right(),
                   &dx1, &dx2, dstArea.left(), dstArea.right(), false );

        cutRanges( &sy1, &sy2, srcArea.top(), srcArea.bottom(),
                   &dy1, &dy2, dstArea.top(), dstArea.bottom(), false );
        break;
    case QtopiaVideo::Rotate180:
        cutRanges( &sx1, &sx2, srcArea.left(), srcArea.right(),
                   &dx1, &dx2, dstArea.left(), dstArea.right(), true );

        cutRanges( &sy1, &sy2, srcArea.top(), srcArea.bottom(),
                   &dy1, &dy2, dstArea.top(), dstArea.bottom(), true );
        break;
    case QtopiaVideo::Rotate90:
        cutRanges( &sx1, &sx2, srcArea.left(), srcArea.right(),
                   &dy1, &dy2, dstArea.top(), dstArea.bottom(), false );

        cutRanges( &dx1, &dx2, dstArea.left(), dstArea.right(),
                   &sy1, &sy2, srcArea.top(), srcArea.bottom(), true );
        break;
    case QtopiaVideo::Rotate270:
        cutRanges( &sx1, &sx2, srcArea.left(), srcArea.right(),
                   &dy1, &dy2, dstArea.top(), dstArea.bottom(), true );

        cutRanges( &dx1, &dx2, dstArea.left(), dstArea.right(),
                   &sy1, &sy2, srcArea.top(), srcArea.bottom(), false );
        break;
    }

    if ( sx1>sx2 || sy1>sy2 || dx1>dx2 || dy1>dy2 )  {
        *srcRect = QRect();
        *dstRect = QRect();
        return;
    }

    srcRect->setCoords( sx1,  sy1, sx2,  sy2 );
    dstRect->setCoords( dx1,  dy1, dx2,  dy2 );

    Q_ASSERT( srcArea.contains( *srcRect ) );
    Q_ASSERT( dstArea.contains( *dstRect ) );

}


/* internal:
 * Fill array "table" of size "to" and evenly fill it with numbers,
 * with total sum of (almost) "from".
 * Each element can be optionaly multiplied by multiplier parameter
 */
static void makeJumpsTable( int *table, int from, int to, int multiplier = 1 )
{
    int shift = 0;
    if ( from < to ) {
        shift = -to;
        for ( int i=0; i<to; i++ ) {
            shift += from;

            table[i] = 0;

            if ( shift > 0 ) {
                table[i]++;
                shift -= to;
            }
        }
    } else {
        shift = -from/2;
        for ( int i=0; i<to; i++ ) {
            table[i] = 0;

            while ( shift < 0 ) {
                table[i]++;
                shift += to;
            }

            shift -= from;
        }
    }

    //clip to avoid overflows:
    int total = 0;
    for ( int i=0; i<to; i++ ) {
        total += table[i];
        if ( total > from-1 )
            table[i] = 0;
    }

    if ( multiplier != 1 )
        for ( int i=0; i<to; i++ )
            table[i] *= multiplier;

    return;
}

#ifdef QT_ARCH_ARMV5E
#define PLD(src) \
    asm volatile("pld [%0, #32]\n\t" \
            : : "r"(src));
#else
#define PLD(src)
#endif

#include <unistd.h>

class qrgb888
{
public:
    uchar data[3];
} Q_PACKED;

class AssignHelperU8_U8 {
public:
    static inline void assign(uchar *out, const uchar *in) {
        *out = *in;
    }
};

class AssignHelperRGB16_RGB16 {
public:
    static inline void assign(quint16 *out, const quint16 *in) {
        *out = *in;
    }
};

class AssignHelperRGB888_RGB888 {
public:
    static inline void assign(qrgb888 *out, const qrgb888 *in) {
        *out = *in;
    }
};

class AssignHelperRGB888_BGR888 {
public:
    static inline void assign(qrgb888 *out, const qrgb888 *in) {
        out->data[0] = in->data[2];
        out->data[1] = in->data[1];
        out->data[2] = in->data[0];
    }
};

class AssignHelperRGB32_RGB32 {
public:
    static inline void assign(quint32 *out, const quint32 *in) {
        *out = *in;
    }
};

class AssignHelperRGB32_BGR32 {
public:
    static inline void assign(quint32 *out, const quint32 *in) {
        uchar *out8 = (uchar*)out;
        uchar *in8 = (uchar*)in;

        out8[0] = in8[3];
        out8[1] = in8[2];
        out8[2] = in8[1];
        out8[3] = in8[0];
    }
};

class AssignHelperRGB32_RGB16 {
public:
    static inline void assign(quint32 *out, const quint16 *in) {
        *out = qConvertRgb16To32(*in);
    }
};

class AssignHelperRGB16_RGB32 {
public:
    static inline void assign(quint16 *out, const quint32 *in) {
        *out = qConvertRgb32To16(*in);
    }
};

class AssignHelperRGB888_RGB32 {
public:
    static inline void assign(qrgb888 *out, const quint32 *in) {
        uchar *out8 = (uchar*)out;
        uchar *in8 = (uchar*)in;

        out8[0] = in8[0];
        out8[1] = in8[1];
        out8[2] = in8[2];
    }
};

/** Scale and rotate the image plane with elements of type T.
 *  srcRect and dstRect must be clipped to fit source and dest images.
 */
template <typename SrcT, typename DstT, class AssignHelper >
static void scalePlane( int* htable, int*vtable,
                 const void *vsrc, int srcLineStep, const QRect& srcRect,
                       void *vdst, int dstLineStep, const QRect& dstRect,
                       QtopiaVideo::VideoRotation rotation )
{
    const SrcT *src = (SrcT *)vsrc;
    DstT *dst = (DstT *)vdst;

    int w = dstRect.width();
    int h = dstRect.height();

    switch ( rotation ) {
    case QtopiaVideo::Rotate0:
    {
        //makeJumpsTable( htable, srcRect.width(), dstRect.width() );
        //makeJumpsTable( vtable, srcRect.height(), dstRect.height() );

        int src_start_x = srcRect.left();
        int src_start_y = srcRect.top();

        int dst_start_x = dstRect.left();
        int dst_start_y = dstRect.top();

        int src_y = src_start_y;
        for ( int y=0; y<h; y++ ) {
            src_y += vtable[y];
            const SrcT *srcbits = src + src_y*srcLineStep + src_start_x;
            DstT *dstbits = dst + (dst_start_y+y)*dstLineStep + dst_start_x;

            int x = 0;
            while ( x<w-4 ) {
                PLD( srcbits );
                PLD( dstbits );
                PLD( htable+x );

                AssignHelper::assign( dstbits, srcbits );
                dstbits++;
                srcbits += htable[x];
                x++;

                AssignHelper::assign( dstbits, srcbits );
                dstbits++;
                srcbits += htable[x];
                x++;

                AssignHelper::assign( dstbits, srcbits );
                dstbits++;
                srcbits += htable[x];
                x++;

                AssignHelper::assign( dstbits, srcbits );
                dstbits++;
                srcbits += htable[x];
                x++;
            }

            while ( x<w ) {
                AssignHelper::assign( dstbits, srcbits );
                dstbits++;
                srcbits += htable[x];
                x++;
            }
        }
    }
    break;
    case QtopiaVideo::Rotate180:
    {
        //makeJumpsTable( htable, srcRect.width(), dstRect.width(), -1 );
        //makeJumpsTable( vtable, srcRect.height(), dstRect.height(), -1 );

        int src_start_x = srcRect.right();
        int src_start_y = srcRect.bottom();

        int dst_start_x = dstRect.left();
        int dst_start_y = dstRect.top();

        int src_y = src_start_y;
        for ( int y=0; y<h; y++ ) {
            src_y += vtable[y];
            const SrcT *srcbits = src + src_y*srcLineStep + src_start_x;
            DstT *dstbits = dst + (dst_start_y+y)*dstLineStep + dst_start_x;

            for ( int x=0; x<w; x++ ) {
                AssignHelper::assign( dstbits, srcbits );
                dstbits++;
                srcbits += htable[x];
            }
        }
    }
    break;
    case QtopiaVideo::Rotate90:
    {
        //makeJumpsTable( htable, srcRect.width(), dstRect.height() );
        //makeJumpsTable( vtable, srcRect.height(), dstRect.width(), -srcLineStep );

        int src_start_x = srcRect.left();
        int src_start_y = srcRect.bottom();

        int dst_start_x = dstRect.left();
        int dst_start_y = dstRect.top();


        //scale/rotate not the whole image at once, but
        //split to columns of width columnWidth for better cache hits
        int columnWidth = 32;

        int from_x = 0;
        int src_shift = 0; //since we are starting to copy src not from top/bottom, we need to calculate the shift
        while ( from_x < dstRect.width()-1 ) {
            int to_x = qMin( from_x+columnWidth, dstRect.width() );

            int src_x = src_start_x;
            for ( int y=0; y<h; y++ ) {
                src_x += htable[y];
                const SrcT *srcbits = src + src_start_y*srcLineStep + src_x + src_shift;
                PLD(srcbits);
                DstT *dstbits = dst + (dst_start_y+y)*dstLineStep + from_x + dst_start_x;
                PLD(dstbits);


                for ( int x=from_x; x<to_x; x++ ) {
                    #if CHECK_SRC_BOUNDARIES
                    {
                        int _offset = srcbits - src;
                        int _src_y = _offset/srcLineStep;
                        int _src_x = _offset - _src_y*srcLineStep;

                        Q_ASSERT( srcRect.contains( _src_x, _src_y ) );
                    }
                    #endif

                    AssignHelper::assign( dstbits, srcbits );
                    dstbits++;
                    srcbits+=vtable[x];
                }
            }

            for (int x=from_x; x<to_x; x++ ) {
                src_shift += vtable[x];
            }

            from_x = to_x;
        }
    }
    break;
    case QtopiaVideo::Rotate270:
    {
        //makeJumpsTable( htable, srcRect.width(), dstRect.height(), -1 );
        //makeJumpsTable( vtable, srcRect.height(), dstRect.width(), srcLineStep );

        int src_start_x = srcRect.right();
        int src_start_y = srcRect.top();

        int dst_start_x = dstRect.left();
        int dst_start_y = dstRect.top();

        //scale/rotate not the whole image at once, but
        //split to columns of width columnWidth for better cache hits

        int columnWidth = 32;

        int from_x = 0;
        int src_shift = 0; //since we are starting to copy src not from top/bottom, we need to calculate the shift
        while ( from_x < dstRect.width()-1 ) {
            int to_x = qMin( from_x+columnWidth, dstRect.width() );

            int src_x = src_start_x;
            for ( int y=0; y<h; y++ ) {
                src_x += htable[y];
                const SrcT *srcbits = src + src_start_y*srcLineStep + src_x + src_shift;
                DstT *dstbits = dst + (dst_start_y+y)*dstLineStep + from_x + dst_start_x;

                for ( int x=from_x; x<to_x; x++ ) {
                    #if CHECK_SRC_BOUNDARIES
                    {
                        int _offset = srcbits - src;
                        int _src_y = _offset/srcLineStep;
                        int _src_x = _offset - _src_y*srcLineStep;

                        Q_ASSERT( srcRect.contains( _src_x, _src_y ) );
                    }
                    #endif

                    AssignHelper::assign( dstbits, srcbits );
                    dstbits++;
                    srcbits+=vtable[x];
                }
            }

            for (int x=from_x; x<to_x; x++ ) {
                src_shift += vtable[x];
            }

            from_x = to_x;
        }

    }
    break;
    }
}

typedef void (*ScalePlaneFunction)(int*,int*,
                               const void*, int, const QRect&,
                               void*, int, const QRect&,
                               QtopiaVideo::VideoRotation);




class QImagePlaneTransformationPrivate
{
public:
    QRect srcRect;
    QRect srcClippedRect;
    QRect srcArea;
    int srcLineStep;

    QRect dstRect;
    QRect dstClippedRect;
    QRect dstArea;
    int dstLineStep;

    QVector<int> htable;
    QVector<int> vtable;

    QtopiaVideo::VideoRotation rotation;
    int geometryAlignment;

    bool isModified;

    void initTables();
    QRect aligned( const QRect& );
    ScalePlaneFunction scaleFunction( QVideoFrame::PixelFormat srcFormat, QVideoFrame::PixelFormat dstFormat );

private:
    QMap<QVideoFrame::PixelFormat, QMap<QVideoFrame::PixelFormat, ScalePlaneFunction > > scaleFunctions;
};

void QImagePlaneTransformationPrivate::initTables()
{
    if ( !isModified )
        return;

    isModified = false;

    srcClippedRect = srcRect;
    dstClippedRect = dstRect;
    clipRects( &srcClippedRect, srcArea,
               &dstClippedRect, dstArea,
               rotation );

    srcClippedRect = aligned( srcClippedRect );
    dstClippedRect = aligned( dstClippedRect );

    if ( srcClippedRect.isEmpty() || dstClippedRect.isEmpty() )
        return;

    int sw = srcClippedRect.width();
    int sh = srcClippedRect.height();
    int dw = dstClippedRect.width();
    int dh = dstClippedRect.height();

    switch ( rotation ) {
    case QtopiaVideo::Rotate0:
        htable.resize(dw);
        vtable.resize(dh);
        makeJumpsTable( htable.data(), sw, dw );
        makeJumpsTable( vtable.data(), sh, dh );
        break;
    case QtopiaVideo::Rotate180:
        htable.resize(dw);
        vtable.resize(dh);
        makeJumpsTable( htable.data(), sw, dw, -1 );
        makeJumpsTable( vtable.data(), sh, dh, -1 );
        break;
    case QtopiaVideo::Rotate90:
        htable.resize(dh);
        vtable.resize(dw);
        makeJumpsTable( htable.data(), sw, dh );
        makeJumpsTable( vtable.data(), sh, dw, -srcLineStep );
        break;
    case QtopiaVideo::Rotate270:
        htable.resize(dh);
        vtable.resize(dw);
        makeJumpsTable( htable.data(), sw, dh, -1 );
        makeJumpsTable( vtable.data(), sh, dw, srcLineStep );
    };
}


QRect QImagePlaneTransformationPrivate::aligned( const QRect& rect )
{
    if ( geometryAlignment == 1 )
        return rect;

    int align = geometryAlignment;

    QRect r = rect;
    r.setLeft( ( r.left()+align-1 )/align*align );
    r.setTop( ( r.top()+align-1 )/align*align );
    r.setRight( ( r.right()+align )/align*align - 1 );
    r.setBottom( ( r.bottom()+align )/align*align - 1 );

    return r;
}


ScalePlaneFunction QImagePlaneTransformationPrivate::scaleFunction( QVideoFrame::PixelFormat srcFormat, QVideoFrame::PixelFormat dstFormat )
{
    if ( scaleFunctions.isEmpty() ) {
        scaleFunctions[QVideoFrame::Format_BGR32][QVideoFrame::Format_BGR32] = scalePlane<quint32,quint32,AssignHelperRGB32_RGB32>;
        scaleFunctions[QVideoFrame::Format_RGB32][QVideoFrame::Format_RGB32] = scalePlane<quint32,quint32,AssignHelperRGB32_RGB32>;

        scaleFunctions[QVideoFrame::Format_RGB32][QVideoFrame::Format_BGR32] = scalePlane<quint32,quint32,AssignHelperRGB32_BGR32>;
        scaleFunctions[QVideoFrame::Format_BGR32][QVideoFrame::Format_RGB32] = scalePlane<quint32,quint32,AssignHelperRGB32_BGR32>;

        scaleFunctions[QVideoFrame::Format_RGB24][QVideoFrame::Format_RGB24] = scalePlane<qrgb888,qrgb888,AssignHelperRGB888_RGB888>;
        scaleFunctions[QVideoFrame::Format_BGR24][QVideoFrame::Format_BGR24] = scalePlane<qrgb888,qrgb888,AssignHelperRGB888_RGB888>;

        scaleFunctions[QVideoFrame::Format_RGB24][QVideoFrame::Format_BGR24] = scalePlane<qrgb888,qrgb888,AssignHelperRGB888_BGR888>;
        scaleFunctions[QVideoFrame::Format_BGR24][QVideoFrame::Format_RGB24] = scalePlane<qrgb888,qrgb888,AssignHelperRGB888_BGR888>;

        scaleFunctions[QVideoFrame::Format_RGB565][QVideoFrame::Format_RGB565] = scalePlane<quint16,quint16,AssignHelperRGB16_RGB16>;
        scaleFunctions[QVideoFrame::Format_BGR565][QVideoFrame::Format_BGR565] = scalePlane<quint16,quint16,AssignHelperRGB16_RGB16>;

        scaleFunctions[QVideoFrame::Format_Y8][QVideoFrame::Format_Y8] = scalePlane<uchar,uchar,AssignHelperU8_U8>;
        scaleFunctions[QVideoFrame::Format_YUV420P][QVideoFrame::Format_YUV420P] = scalePlane<uchar,uchar,AssignHelperU8_U8>;
        scaleFunctions[QVideoFrame::Format_YV12][QVideoFrame::Format_YV12] = scalePlane<uchar,uchar,AssignHelperU8_U8>;

        scaleFunctions[QVideoFrame::Format_RGB32][QVideoFrame::Format_RGB565] = scalePlane<quint32,quint16,AssignHelperRGB16_RGB32>;
        scaleFunctions[QVideoFrame::Format_RGB565][QVideoFrame::Format_RGB32] = scalePlane<quint16,quint32,AssignHelperRGB32_RGB16>;

        scaleFunctions[QVideoFrame::Format_RGB32][QVideoFrame::Format_RGB24] = scalePlane<quint32,qrgb888,AssignHelperRGB888_RGB32>;
    }

    if ( srcFormat == QVideoFrame::Format_ARGB32 )
        srcFormat = QVideoFrame::Format_RGB32;
    if ( srcFormat == QVideoFrame::Format_BGRA32 )
        srcFormat = QVideoFrame::Format_BGR32;

    if ( dstFormat == QVideoFrame::Format_ARGB32 )
        dstFormat = QVideoFrame::Format_RGB32;
    if ( dstFormat == QVideoFrame::Format_BGRA32 )
        dstFormat = QVideoFrame::Format_BGR32;

    return scaleFunctions[ srcFormat ].value( dstFormat, 0 );
}


QImagePlaneTransformation::QImagePlaneTransformation()
    :d( new QImagePlaneTransformationPrivate )
{
    d->isModified = true;
    d->rotation = QtopiaVideo::Rotate0;
    d->geometryAlignment = 1;
}

QImagePlaneTransformation::~QImagePlaneTransformation()
{
    delete d;
}

void QImagePlaneTransformation::setSrcGeometry( const QRect& srcRect, const QRect& srcClipRect, int srcLineStep )
{
    d->srcRect = srcRect.normalized();
    d->srcArea = srcClipRect.normalized();
    d->srcLineStep = srcLineStep;
    d->isModified = true;
}

void QImagePlaneTransformation::setDstGeometry( const QRect& dstRect, const QRect& dstClipRect, int dstLineStep )
{
    d->dstRect = dstRect.normalized();
    d->dstArea = dstClipRect.normalized();
    d->dstLineStep = dstLineStep;
    d->isModified = true;
}

void QImagePlaneTransformation::setRotation( QtopiaVideo::VideoRotation rotation )
{
    d->rotation = rotation;
    d->isModified = true;
}


void QImagePlaneTransformation::setGeometryAlignment( int align )
{
    d->geometryAlignment = align;
    d->isModified = true;
}

QRect QImagePlaneTransformation::clippedSrcGeometry() const
{
    d->initTables();
    return d->srcClippedRect;
}

QRect QImagePlaneTransformation::clippedDstGeometry() const
{
    d->initTables();
    return d->dstClippedRect;
}


void QImagePlaneTransformation::transformPlane( const uchar *src, uchar *dst )
{
    d->initTables();

    if ( d->srcClippedRect.isEmpty() || d->dstClippedRect.isEmpty() )
        return;

    scalePlane<uchar,uchar,AssignHelperU8_U8>( d->htable.data(), d->vtable.data(),
                       src, d->srcLineStep, d->srcClippedRect,
                       dst, d->dstLineStep, d->dstClippedRect,
                       d->rotation );
}

void QImagePlaneTransformation::transformPlane( const quint16 *src, quint16 *dst )
{
    d->initTables();

    if ( d->srcClippedRect.isEmpty() || d->dstClippedRect.isEmpty() )
        return;

    scalePlane<quint16,quint16,AssignHelperRGB16_RGB16>( d->htable.data(), d->vtable.data(),
                       src, d->srcLineStep, d->srcClippedRect,
                       dst, d->dstLineStep, d->dstClippedRect,
                       d->rotation );
}

void QImagePlaneTransformation::transformPlane( const quint32 *src, quint32 *dst )
{
    d->initTables();

    if ( d->srcClippedRect.isEmpty() || d->dstClippedRect.isEmpty() )
        return;

    scalePlane<quint32,quint32,AssignHelperRGB32_RGB32>( d->htable.data(), d->vtable.data(),
                       src, d->srcLineStep, d->srcClippedRect,
                       dst, d->dstLineStep, d->dstClippedRect,
                       d->rotation );
}



/*!
   Copy data from src to dst applying scalling and rotating transformation,
   defined with setSrcGeometry() setDstGeometry(), setRotation()
   and RGB colorspace if necessary.
   Return true if the colorspace conversion is supported
   and plane was successwfully transformed.
*/
bool QImagePlaneTransformation::transformPlane( const void *src, QVideoFrame::PixelFormat srcFormat, void *dst, QVideoFrame::PixelFormat dstFormat )
{
    d->initTables();

    if ( d->srcClippedRect.isEmpty() || d->dstClippedRect.isEmpty() )
        return true;

    ScalePlaneFunction scalePlaneFunction = d->scaleFunction( srcFormat, dstFormat );
    if ( scalePlaneFunction ) {
        scalePlaneFunction( d->htable.data(), d->vtable.data(),
                            src, d->srcLineStep, d->srcClippedRect,
                            dst, d->dstLineStep, d->dstClippedRect,
                            d->rotation );
        return true;
    } else
        return false;
}



bool QImagePlaneTransformation::isTransformationSupported( QVideoFrame::PixelFormat srcFormat, QVideoFrame::PixelFormat dstFormat )
{
    return d->scaleFunction( srcFormat, dstFormat ) != 0;
}
