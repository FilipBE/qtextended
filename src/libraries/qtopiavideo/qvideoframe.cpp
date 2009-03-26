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

#include "qvideoframe.h"
#include <QImage>
#include <QDebug>

const int maxPlanesCount = 3;

class QVideoFramePrivate : public QSharedData {
public:
    QVideoFramePrivate();
    QVideoFramePrivate( QVideoFrame::PixelFormat format, const QSize& size );
    QVideoFramePrivate( const QVideoFramePrivate& );

    ~QVideoFramePrivate();

    void copyPlanes();

    QSize planeSize( int planeNumber ) const;

    QSize size;
    QVideoFrame::PixelFormat format;
    bool ownData; //the frame allocated the data, and has to free it

    QImage img; // used only when constructed from QImage

    //maximum 3 planes, for planar formats ( like Format_YUV420P )
    const uchar* constPlanes[maxPlanesCount];
    uchar* planes[maxPlanesCount];
    int bytesPerLine[maxPlanesCount];

    double aspectRatio;
    bool isAspectRatioDefined;

    QVideoFrame::BufferHelper *bufferHelper;
private:
    void allocateOwnPlanes();
};

/*!
    \internal
*/
QVideoFramePrivate::QVideoFramePrivate()
    :format( QVideoFrame::Format_Invalid ),
    ownData( false ),
    aspectRatio( 0 ),
    isAspectRatioDefined( false ),
    bufferHelper(0)
{
    for ( int i=0; i<maxPlanesCount; i++ ) {
        constPlanes[i] = 0;
        planes[i] = 0;
        bytesPerLine[i] = 0;
    }

}

/*!
    \internal
*/
QVideoFramePrivate::QVideoFramePrivate( QVideoFrame::PixelFormat _format, const QSize& _size )
    :size(_size),
    format(_format),
    ownData(true),
    aspectRatio( 0 ),
    isAspectRatioDefined( false ),
    bufferHelper(0)
{
    for ( int i=0; i<maxPlanesCount; i++ ) {
        constPlanes[i] = 0;
        planes[i] = 0;
        bytesPerLine[i] = 0;
    }

    if ( !size.isEmpty() ) {
        allocateOwnPlanes();
    }
}

/*!
    \internal
*/
QVideoFramePrivate::QVideoFramePrivate( const QVideoFramePrivate& other )
:QSharedData(other)
{
    size = other.size;
    format = other.format;
    ownData = true;
    bufferHelper = 0;

    allocateOwnPlanes();

    for ( int plane=0; plane < maxPlanesCount; ++plane ) {
        bytesPerLine[plane] = other.bytesPerLine[plane];
        constPlanes[plane] = 0;

        if ( other.planes[plane] ) {
            memcpy( planes[plane], other.planes[plane], planeSize(plane).height()*bytesPerLine[plane] );
        } else if ( other.constPlanes[plane] ) {
            memcpy( planes[plane], other.constPlanes[plane], planeSize(plane).height()*bytesPerLine[plane] );
        }
    }

    aspectRatio = other.aspectRatio;
    isAspectRatioDefined = other.isAspectRatioDefined;
}

/*!
    \internal
*/
QVideoFramePrivate::~QVideoFramePrivate()
{
    if ( !ownData && bufferHelper ) {
        bufferHelper->unlock();
    }

    if ( ownData ) {
        free( planes[0] );
    }
}
/*!
    allocate planes[] and planeSizes[] according to size and format
    \internal
*/
void QVideoFramePrivate::allocateOwnPlanes()
{
    int totalSize  = 0;
    int planeSizes[maxPlanesCount];

    for ( int plane = 0; plane < maxPlanesCount; plane++ ) {
        int depth = QVideoFrame::colorDepth( format, plane );
        if ( depth ) {
            QSize pSize = planeSize( plane );

            bytesPerLine[plane] = (( pSize.width() * depth + 31)/32) * 32 / 8;
            planeSizes[plane] = bytesPerLine[plane] * pSize.height();
            //planes[plane] = (uchar *)malloc( bytesPerLine[plane] * pSize.height() );
            totalSize += planeSizes[plane];
        } else
            planeSizes[plane] = 0;
    }

    if ( totalSize ) {
        planes[0] = (uchar *)malloc( totalSize );
        for ( int plane = 1; plane < maxPlanesCount; plane++ ) {
            if ( planeSizes[plane] )
                planes[plane] = planes[plane-1] + planeSizes[plane-1];
            else
                break;
        }
    }
}

/*!
    \internal
*/
void QVideoFramePrivate::copyPlanes()
{
    if ( planes[0] )
        return;

    allocateOwnPlanes();

    bool copied = false;

    for ( int plane=0; plane < maxPlanesCount; ++plane ) {
        if ( constPlanes[plane] ) {
            Q_ASSERT( planes[plane] != 0 );
            memcpy( planes[plane], constPlanes[plane], size.height()*bytesPerLine[plane] );
            constPlanes[plane] = 0;
            copied = true;
        }
    }

    img = QImage();

    ownData = true;

    if ( copied && bufferHelper ) {
        bufferHelper->unlock();
        bufferHelper = 0;
    }
}

/*!
    \internal
*/
QSize QVideoFramePrivate::planeSize( int planeNumber ) const
{
    if ( planeNumber == 0 ) {
        return size;
    }

    switch ( format ) {
    case QVideoFrame::Format_YUV420P:
    case QVideoFrame::Format_YV12:
        return size/2;
        break;
    default:
        return QSize(0,0);
    }
}

/*!
    Returns the color depth of a data plane in \a planeNumber
    in the particualr pixel format \a format
*/
int QVideoFrame::colorDepth( QVideoFrame::PixelFormat format, int planeNumber )
{
    int depth[3] = {0,0,0}; //in bits per pixel

    switch ( format ) {
    case QVideoFrame::Format_Invalid:
        break;
    case QVideoFrame::Format_ARGB32:
    case QVideoFrame::Format_RGB32:
    case QVideoFrame::Format_BGRA32:
    case QVideoFrame::Format_BGR32:
        depth[0] = 32;
        break;
    case QVideoFrame::Format_RGB24:
    case QVideoFrame::Format_BGR24:
    case QVideoFrame::Format_YUV444:
        depth[0] = 24;
        break;
    case QVideoFrame::Format_RGB565:
    case QVideoFrame::Format_BGR565:
        depth[0] = 16;
        break;
    case QVideoFrame::Format_YUV420P:
    case QVideoFrame::Format_YV12:
        depth[0] = 8;
        depth[1] = 8;
        depth[2] = 8;
        break;
    case QVideoFrame::Format_UYVY:
        depth[0] = 16; // one Y, 1/2 V, 1/2 U, UYVY order
        break;
    case QVideoFrame::Format_YUYV:
        depth[0] = 16; // one Y, 1/2 V, 1/2 U, YUYV order
        break;
    case QVideoFrame::Format_Y8:
        depth[0] = 8;
        break;
    case QVideoFrame::NVideoFormats:
        break;
    }

    return depth[ planeNumber ];
}

/*!
    Returns the number fo planes for the pixel format \a format
*/
int QVideoFrame::planesCount( PixelFormat format )
{
    return isPlanar( format ) ? 3 : 1;
}

/*!
    Returns true if the pixel format \a format is planar,
    false otherwise
*/
bool QVideoFrame::isPlanar( PixelFormat format )
{
    return format == QVideoFrame::Format_YUV420P || format == QVideoFrame::Format_YV12;
}

/*!
    Contructs an empty QVideoFrame
*/
QVideoFrame::QVideoFrame()
:d( new QVideoFramePrivate )
{
}

/*!
    Contructs an empty video frame specific with pixel format \a format
    and  frame size with \a size
*/
QVideoFrame::QVideoFrame( PixelFormat format, const QSize& size )
:d( new QVideoFramePrivate(format, size) )
{
}

/*!
    Copy contructor.
    Create's a video frame from \a other
    Data is implicitly shared
*/
QVideoFrame::QVideoFrame( const QVideoFrame& other )
{
    d = other.d;
}

/*!
    \class QVideoFrame
    \inpublicgroup QtMediaModule
    \brief The QVideoFrame class represents a single frame of displayable data
*/

/*!
    \class QVideoFrame::BufferHelper
    \brief The BufferHelper class is a helper class that allows
     locked access to the raw image buffer.
*/

 /*!
    \fn QVideoFrame::BufferHelper::~BufferHelper()
    destructor
*/

/*!
    \fn void QVideoFrame::BufferHelper::lock()
    Locks the buffer
*/

/*!
    \fn void QVideoFrame::BufferHelper::unlock()
    Unlocks the buffer
*/

/*!
    \enum QVideoFrame::PixelFormat
    Pixel formats (please see http://www.fourcc.org/yuv.php for formats description)
    \value Format_Invalid
    \value Format_ARGB32
    \value Format_RGB32
    \value Format_RGB24
    \value Format_RGB565

    \value Format_BGRA32
    \value Format_BGR32
    \value Format_BGR24
    \value Format_BGR565

    \value Format_YUV444
    \value Format_YUV420P
    \value Format_YV12
    \value Format_UYVY
    \value Format_YUYV

    \value Format_Y8
*/

/*!
  Constructs a video frame with the given width, height and \a format,
  that uses an existing memory buffer, data.
  The width and height must be specified in pixels and is given by \a size,
  \a data must be 32-bit aligned, and each plane and scanline of data in the image must also be 32-bit aligned.

  The buffer must remain valid throughout the life of the
  QVideoFrame and all copies that have not been modified or
  otherwise detached from the original buffer. The QVideoFrame
  does not delete the buffer at destruction.
  The \a helper parameter provides thread safety.
 */
QVideoFrame::QVideoFrame( PixelFormat format, const QSize& size, const uchar* data, QVideoFrame::BufferHelper *helper)
:d( new QVideoFramePrivate )
{
    d->format = format;
    d->size = size;
    d->bufferHelper = helper;

    const uchar* planeData = data;

    if ( !size.isEmpty() ) {
        for ( int plane = 0; plane < maxPlanesCount; ++plane ) {
            int depth = colorDepth( format, plane );
            QSize planeSize = d->planeSize( plane );

            if ( depth == 0 || planeSize.isEmpty() )
                break;

            d->constPlanes[ plane ] = planeData;
            d->bytesPerLine[ plane ] = (( planeSize.width() * depth + 31)/32) * 32 / 8;
            planeData += planeSize.height() * d->bytesPerLine[ plane ];
        }
    }

    if ( d->bufferHelper )
        d->bufferHelper->lock();
}

/*!
  Constructs a video frame with the given width, height and \a format,
  that uses an existing memory buffer, data.
  The width and height must be specified in pixels given by \a size,
  \a data must be 32-bit aligned, and each plane and scanline of data in the image must also be 32-bit aligned.

  The buffer must remain valid throughout the life of the
  QVideoFrame and all copies that have not been modified or
  otherwise detached from the original buffer. The QVideoFrame
  does not delete the buffer at destruction.
  The \a helper parameter provides thread safety.
 */
QVideoFrame::QVideoFrame( PixelFormat format, const QSize& size, uchar* data, QVideoFrame::BufferHelper *helper)
:d( new QVideoFramePrivate )
{
    d->format = format;
    d->size = size;
    d->bufferHelper = helper;

    uchar* planeData = data;

    if ( !size.isEmpty() ) {
        for ( int plane = 0; plane < maxPlanesCount; ++plane ) {
            int depth = colorDepth( format, plane );
            QSize planeSize = d->planeSize( plane );

            if ( depth == 0 || planeSize.isEmpty() )
                break;

            d->planes[ plane ] = planeData;
            d->bytesPerLine[ plane ] = (( planeSize.width() * depth + 31)/32) * 32 / 8;
            planeData += planeSize.height() * d->bytesPerLine[ plane ];
        }
    }

    if ( d->bufferHelper )
        d->bufferHelper->lock();
}

/*!
  Create Video frame for interlaced formats with one plane,
  like  Format_RGB24 or Format_YUYV.

  The pixel format is specified with \a format, the frame has image size \a size,
  \a planeData contains the raw image data, and the image stride is given by
  \a bytesPerLine.
  The buffer must remain valid throughout the life of the
  QVideoFrame and all copies that have not been modified or
  otherwise detached from the original buffer. The QVideoFrame
  does not delete the buffer at destruction.
  The \a helper parameter provides thread safety.
 */
QVideoFrame::QVideoFrame( PixelFormat format, const QSize& size,
                          const uchar* planeData,
                          int bytesPerLine,
                          QVideoFrame::BufferHelper *helper )
:d( new QVideoFramePrivate )

{
    d->size = size;
    d->format = format;
    d->bufferHelper = helper;

    d->constPlanes[0] = planeData;
    d->bytesPerLine[0] = bytesPerLine;

    if ( d->bufferHelper )
        d->bufferHelper->lock();
}

/*!
  Create Video frame for interlaced formats with one plane,
  like  Format_RGB24 or Format_YUYV.

  The pixel format is specified with \a format, the frame has image size \a size,
  \a planeData contains the raw image data, and the image stride is given by
  \a bytesPerLine.

  The buffer must remain valid throughout the life of the
  QVideoFrame and all copies that have not been modified or
  otherwise detached from the original buffer. The QVideoFrame
  does not delete the buffer at destruction.
  The \a helper parameter provides thread safety.
 */
QVideoFrame::QVideoFrame( PixelFormat format, const QSize& size,
                          uchar* planeData,
                          int bytesPerLine,
                          QVideoFrame::BufferHelper *helper )
:d( new QVideoFramePrivate )

{
    d->size = size;
    d->format = format;
    d->bufferHelper = helper;

    d->planes[0] = planeData;
    d->bytesPerLine[0] = bytesPerLine;

    if ( d->bufferHelper )
        d->bufferHelper->lock();
}

/*!
  Create Video frame for planar formats (with planes separated),
  like  Format_YUV420P or Format_YUV422P.

  The pixel format is specified with \a format, the frame has image size \a size,
  \a planeData1, \a planeData2, \a planeData3 contain the raw image data, and the
  their respective strides are given by
  \a bytesPerLine1,   \a bytesPerLine2,  \a bytesPerLine3.


  The buffers must remain valid throughout the life of the
  QVideoFrame and all copies that have not been modified or
  otherwise detached from the original buffer. The QVideoFrame
  does not delete the buffers at destruction.
  The \a helper parameter provides thread safety.
 */
QVideoFrame::QVideoFrame( PixelFormat format, const QSize& size,
                          const uchar* planeData1,
                          const uchar* planeData2,
                          const uchar* planeData3,
                          int bytesPerLine1,
                          int bytesPerLine2,
                          int bytesPerLine3,
                          QVideoFrame::BufferHelper *helper )
:d( new QVideoFramePrivate )
{
    d->size = size;
    d->format = format;

    d->constPlanes[0] = planeData1;
    d->constPlanes[1] = planeData2;
    d->constPlanes[2] = planeData3;

    d->bytesPerLine[0] = bytesPerLine1;
    d->bytesPerLine[1] = bytesPerLine2;
    d->bytesPerLine[2] = bytesPerLine3;

    d->bufferHelper = helper;

    if ( d->bufferHelper )
        d->bufferHelper->lock();
}

/*!
  Create Video frame for planar formats (with planes separated),
  like  Format_YUV420P or Format_YUV422P.

  The pixel format is specified with \a format, the frame has image size \a size,
  \a planeData1, \a planeData2, \a planeData3 contain the raw image data, and the
  their respective image strides are given by
  \a bytesPerLine1,   \a bytesPerLine2,  \a bytesPerLine3.

  The buffers must remain valid throughout the life of the
  QVideoFrame and all copies that have not been modified or
  otherwise detached from the original buffer. The QVideoFrame
  does not delete the buffers at destruction.
  The \a helper parameter provides thread safety.
 */
QVideoFrame::QVideoFrame( PixelFormat format, const QSize& size,
                          uchar* planeData1,
                          uchar* planeData2,
                          uchar* planeData3,
                          int bytesPerLine1,
                          int bytesPerLine2,
                          int bytesPerLine3,
                          QVideoFrame::BufferHelper *helper )
:d( new QVideoFramePrivate )
{
    d->size = size;
    d->format = format;

    d->planes[0] = planeData1;
    d->planes[1] = planeData2;
    d->planes[2] = planeData3;

    d->bytesPerLine[0] = bytesPerLine1;
    d->bytesPerLine[1] = bytesPerLine2;
    d->bytesPerLine[2] = bytesPerLine3;

    d->bufferHelper = helper;

    if ( d->bufferHelper )
        d->bufferHelper->lock();
}

/*!
  Create Video frame from QImage \a img,
*/
QVideoFrame::QVideoFrame( const QImage& img )
:d( new QVideoFramePrivate )
{
    d->img = img; //to ensure bits are not deleted
    d->size = img.size();
    d->constPlanes[0] = img.bits();
    d->bytesPerLine[0] = img.bytesPerLine();

    switch ( img.format() ) {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        d->format = Format_ARGB32;
        break;
    case QImage::Format_RGB16:
        d->format = Format_RGB565;
        break;
    default:
        d->format = Format_Invalid;
    }

}

/*!
    Does nothing
*/
QVideoFrame::~QVideoFrame()
{
}

/*!
    Do a copy of the frame \a other, data is implicitly shared
*/
QVideoFrame& QVideoFrame::operator=( const QVideoFrame& other )
{
    d = other.d;
    return *this;
}

/*!
    Returns true if the frame is valid, false otherwise
*/
bool QVideoFrame::isNull() const
{
    return d->size.isNull() || d->format == Format_Invalid;
}

/*!
    Returns the current format of the video frame
*/
QVideoFrame::PixelFormat QVideoFrame::format() const
{
    return d->format;
}

/*!
    Returns the current size of the video frame
*/
QSize QVideoFrame::size() const
{
    return d->size;
}

/*!
    Returns a pointer to the pixel data given by plane number \a planeNumber
*/
const uchar* QVideoFrame::planeData( int planeNumber ) const
{
    Q_ASSERT( planeNumber >= 0 );
    Q_ASSERT( planeNumber < maxPlanesCount );

    return d->constPlanes[planeNumber] ? d->constPlanes[planeNumber] : d->planes[planeNumber];
}

/*!
    Returns a constant  pointer to the pixel data given by plane number
    \a planeNumber
*/
const uchar* QVideoFrame::constPlaneData( int planeNumber ) const
{
    Q_ASSERT( planeNumber >= 0 );
    Q_ASSERT( planeNumber < maxPlanesCount );

    return d->constPlanes[planeNumber] ? d->constPlanes[planeNumber] : d->planes[planeNumber];
}

/*!
    Returns a pointer to the first pixel index by \a planeNumber.
    This function performs a deep copy of the shared pixel
    data, thus ensuring that this QVideoFrame is the only one using the
    current return value.
 */
uchar* QVideoFrame::planeData( int planeNumber )
{
    Q_ASSERT( planeNumber >= 0 );
    Q_ASSERT( planeNumber < maxPlanesCount );

    d->copyPlanes();

    return d->planes[planeNumber];
}
/*!
  Returns the number of bytes per scanline of plane \a planeNumber.
 */
int QVideoFrame::bytesPerLine( int planeNumber ) const
{
    Q_ASSERT( planeNumber >= 0 );
    Q_ASSERT( planeNumber < maxPlanesCount );

    return d->bytesPerLine[planeNumber];
}

/*!
  Returns the plane number \a planeNumber plane size in pixels.
  In most cases it equals to the image size,
  except of U and V planes of planar YUV formats.
 */
QSize QVideoFrame::planeSize( int planeNumber ) const
{
    return d->planeSize( planeNumber );
}

/*!
  Set the frame aspect ratio to \a customRatio, if the actual frame proportion differs from width/height.

  \sa aspectRatio()
  \sa hasCustomAspectRatio()
 */
void QVideoFrame::setAspectRatio( double customRatio )
{
    d->aspectRatio = customRatio;
    d->isAspectRatioDefined = true;
}

/*!
  Returns the frame aspect ratio, which is equal to width/height
  or previously assigned with setAspectRatio() custom value.

  \sa setAspectRatio()
  \sa hasCustomAspectRatio()
 */
double QVideoFrame::aspectRatio() const
{
    if ( d->isAspectRatioDefined )
        return d->aspectRatio;
    else
        return double( d->size.width() ) / d->size.height();
}

/*!
  Returns true if custom aspect ratio was set with setAspectRatio().

  \sa aspectRatio()
  \sa setAspectRatio()
 */
bool QVideoFrame::hasCustomAspectRatio() const
{
    return d->isAspectRatioDefined;
}

