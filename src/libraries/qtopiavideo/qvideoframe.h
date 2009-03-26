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

#ifndef QVIDEOFRAME_H
#define QVIDEOFRAME_H

#include <QSize>
#include <QSharedDataPointer>

#include <qtopiaglobal.h>

class QImage;
class QVideoFramePrivate;

class QTOPIAVIDEO_EXPORT QVideoFrame
{
public:
    class BufferHelper {
    public:
        virtual ~BufferHelper() {};
        virtual void lock() = 0;
        virtual void unlock() = 0;
    };

    enum PixelFormat { // check http://www.fourcc.org/yuv.php for formats description
                       Format_Invalid,
                       Format_ARGB32,
                       Format_RGB32,
                       Format_RGB24,
                       Format_RGB565,

                       Format_BGRA32,
                       Format_BGR32,
                       Format_BGR24,
                       Format_BGR565,

                       Format_YUV444,
                       Format_YUV420P, // also called IYUV and I420, is used very often
                       Format_YV12, // the same as YUV420P with U and V planes reversed, is used very often
                       Format_UYVY, // packed format known as YUV422 with macroblock component sequence UYVY
                       Format_YUYV, // packed format known as YUV422 with macroblock component sequence YUYV

                       Format_Y8,
#ifndef qdoc
                       NVideoFormats
#endif
                     };

    QVideoFrame();
    QVideoFrame( const QVideoFrame& other );

    QVideoFrame( PixelFormat format, const QSize& size );

    QVideoFrame( PixelFormat format, const QSize& size, const uchar* data, BufferHelper *helper = 0 );

    QVideoFrame( PixelFormat format, const QSize& size, uchar* data, BufferHelper *helper = 0 );

    QVideoFrame( PixelFormat format, const QSize& size,
                 const uchar* planeData1,
                 int bytesPerLine1,
                 BufferHelper *helper = 0 );

    QVideoFrame( PixelFormat format, const QSize& size,
                 uchar* planeData1,
                 int bytesPerLine1,
                 BufferHelper *helper = 0 );

    QVideoFrame( PixelFormat format, const QSize& size,
                 const uchar* planeData1,
                 const uchar* planeData2,
                 const uchar* planeData3,
                 int bytesPerLine1,
                 int bytesPerLine2,
                 int bytesPerLine3,
                 BufferHelper *helper = 0 );

    QVideoFrame( PixelFormat format, const QSize& size,
                 uchar* planeData1,
                 uchar* planeData2,
                 uchar* planeData3,
                 int bytesPerLine1,
                 int bytesPerLine2,
                 int bytesPerLine3,
                 BufferHelper *helper = 0 );

    QVideoFrame( const QImage& );

    virtual ~QVideoFrame();

    QVideoFrame &operator=( const QVideoFrame& other );

    bool isNull() const;

    PixelFormat format() const;
    QSize size() const;

    uchar* planeData( int planeNumber );
    const uchar* planeData( int planeNumber ) const;
    const uchar* constPlaneData( int planeNumber ) const;
    int bytesPerLine( int planeNumber ) const;
    QSize planeSize( int planeNumber ) const;

    void setAspectRatio( double ); //set custom aspect ratio, if different from width/height
    double aspectRatio() const;
    bool hasCustomAspectRatio() const;

    static int colorDepth( QVideoFrame::PixelFormat format, int plane );
    static int planesCount( QVideoFrame::PixelFormat format );
    static bool isPlanar( QVideoFrame::PixelFormat format );

private:
    QSharedDataPointer<QVideoFramePrivate> d;
};

typedef QList<QVideoFrame::PixelFormat> QVideoFormatList;

#endif // Q_VIDEO_FRAME_H

