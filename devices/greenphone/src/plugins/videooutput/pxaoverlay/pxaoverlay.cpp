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

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "qtopiavideo/qimageplanetransform.h"

#include <QDebug>

//for valgrind testing on sytem without overlay
#define FAKE_OVERLAY 0

class PxaFrameBuffer
{
public:
    PxaFrameBuffer();
    ~PxaFrameBuffer();

    enum Mode { RGB = 0x0, YUV444 = 0x2, YUV422 = 0x3, YUV420 = 0x4 };

    bool open(const char *device);
    bool openOverlay(const char *device, PxaFrameBuffer::Mode mode, 
                     int overlay_xres, int overlay_yres, 
                     int overlay_xoffset, int overlay_yoffset);

    bool setOverlayGeometry( int overlay_xres, int overlay_yres, 
                     int overlay_xoffset, int overlay_yoffset );

    void close();

    void print_finfo(FILE *f) const;
    void print_vinfo(FILE *f) const;

    unsigned char *framebuffer() { return data; }
    unsigned int width() const { return vinfo.xres; }
    unsigned int height() const { return vinfo.yres; }
    unsigned int size() const { return finfo.smem_len; }
    unsigned int bpp() const { return vinfo.bits_per_pixel; }
    unsigned int bpl() const { return finfo.line_length; }
    unsigned int roff() const { return vinfo.red.offset; }
    unsigned int rlen() const { return vinfo.red.length; }
    unsigned int goff() const { return vinfo.green.offset; }
    unsigned int glen() const { return vinfo.green.length; }
    unsigned int boff() const { return vinfo.blue.offset; }
    unsigned int blen() const { return vinfo.blue.length; }

protected:
    void getScreenInfo() const;

private:
    int fd;

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    PxaFrameBuffer::Mode mode;

    unsigned char *data;
};

PxaFrameBuffer::PxaFrameBuffer()
{
    data = 0;
    fd = -1;

    memset(&vinfo, 0, sizeof(struct fb_var_screeninfo));
    memset(&finfo, 0, sizeof(struct fb_fix_screeninfo));
}

PxaFrameBuffer::~PxaFrameBuffer()
{
    close();
}

bool PxaFrameBuffer::open(const char *device)
{
    fd = ::open( device, O_RDWR );
    if ( fd < 0 ) {
        fprintf( stderr, "Could not open %s\n", device );
        return false;
    }

    getScreenInfo();

    data = (unsigned char *)mmap(0, finfo.smem_len,
                                 PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if ( (void *) data == (void *) -1 ) {
        fprintf( stderr, "openOverlay: mmap error\n" );
        ::close( fd );
        fd = -1;
        data = 0;
        return false;
    }

    return true;
}

bool PxaFrameBuffer::openOverlay(const char *device, PxaFrameBuffer::Mode mode,
                              int overlay_xres, int overlay_yres,
                              int overlay_xoffset, int overlay_yoffset)
{
    fd = ::open( device, O_RDWR );
    if ( fd < 0 ) {
        fprintf( stderr, "Could not open %s\n", device );
        return false;
    }
    this->mode = mode;

    return setOverlayGeometry( overlay_xres, overlay_yres, overlay_xoffset, overlay_yoffset );
}

bool PxaFrameBuffer::setOverlayGeometry( int overlay_xres, int overlay_yres,
                                      int overlay_xoffset, int overlay_yoffset )
{
    getScreenInfo();

    //print_vinfo(stderr);
    //print_finfo(stderr);

    vinfo.xres = overlay_xres;
    vinfo.yres = overlay_yres;
    vinfo.nonstd = (static_cast<int>(mode) << 20) | 
                   (overlay_yoffset << 10) | overlay_xoffset;
    vinfo.bits_per_pixel = 16;

    int res = ioctl( fd, FBIOPUT_VSCREENINFO, &vinfo );

    if ( res != 0) {
        fprintf( stderr, "vscreeninfo setup error: %d, ( %d, %d, %d x %d ), format=%d\n",
                 res, 
                 overlay_xoffset, overlay_yoffset, overlay_xres, overlay_yres, 
                 static_cast<int>(mode) );

        ::close( fd );
        fd = -1;
        return false;
    } 

    fprintf(stderr, "FBIOPUT succeeded\n");

    getScreenInfo();

    data = (unsigned char *)mmap(0, finfo.smem_len, 
                                 PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if ( (void *) data == (void *) -1 ) {
        fprintf( stderr, "openOverlay: mmap error\n" );
        ::close( fd );
        fd = -1;
        data = 0;
        return false;
    }

    return true;
}

void PxaFrameBuffer::close()
{
    if (data != 0)
        munmap( data, finfo.smem_len );

    if ( fd != -1 ) {
        ::close(fd);
    }

    fd = -1;
    data = 0;
}

void PxaFrameBuffer::getScreenInfo() const
{
    if (fd < 0)
        return;

    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo)) {
        // Error reading fixed information.
        fprintf( stderr, "getScreenInfo: reading fixed screen info error\n" );
    }

    /* Get variable screen information */
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
        // Error reading variable information.
        fprintf( stderr, "getScreenInfo: reading variable screen info error\n" );
    }
}

void PxaFrameBuffer::print_vinfo(FILE *f) const
{
    fprintf(f, "fb_var_screeninfo dump:\n");
    fprintf(f, "xres=%u, yres=%u, xres_virtual=%u, yres_virtual=%u\n",
            vinfo.xres, vinfo.yres,
            vinfo.xres_virtual, vinfo.yres_virtual);
    fprintf(f, "xoffset=%u, yoffset=%u\n", vinfo.xoffset, vinfo.yoffset);
    fprintf(f, "bits_per_pixel=%u, grayscale=%u\n",
            vinfo.bits_per_pixel, vinfo.grayscale);
    fprintf(f, "red.offset=%u, red.length=%u, red.msb_right=%u\n", 
            vinfo.red.offset, vinfo.red.length, vinfo.red.msb_right); 
    fprintf(f, "green.offset=%u, green.length=%u, green.msb_right=%u\n", 
            vinfo.green.offset, vinfo.green.length, vinfo.green.msb_right); 
    fprintf(f, "blue.offset=%u, blue.length=%u, blue.msb_right=%u\n", 
            vinfo.blue.offset, vinfo.blue.length, vinfo.blue.msb_right);
    fprintf(f, "nonstd=%u, height=%u, width=%u\n", vinfo.nonstd, vinfo.height, vinfo.width);
}

void PxaFrameBuffer::print_finfo(FILE *f) const
{
    fprintf(f, "fb_fix_screeninfo dump:\n");
    fprintf(f, "id:\n");
    for (int i = 0; i < 16; i++) {
        if (finfo.id[i] == '\0')
            break;
        fprintf(f, "%c", finfo.id[i]);
    }
    fprintf(f, "\n");
    fprintf(f, "smem_start=%lx, smem_len=%u\n", finfo.smem_start, finfo.smem_len);
    fprintf(f, "line_length=%u\n", finfo.line_length);
}


#include "pxaoverlay.h"

class PxaOverlayPrivate {
public:
    bool isValid;

    QRect geometry;

    bool isOpened;
    QString errorMessage;

#if FAKE_OVERLAY
    QVideoFrame *overlayFrame;
#else
    PxaFrameBuffer overlay2;
#endif

    uchar *yPlane;
    uchar *uPlane;
    uchar *vPlane;

    int yLineStep;
    int uLineStep;
    int vLineStep;

    int yPlaneSize;
    int uPlaneSize;
    int vPlaneSize;

    QImagePlaneTransformation *yTransformation;
    QImagePlaneTransformation *uvTransformation;

    QRect prevFrameRect;
    QRect prevOverlayRect;
    QtopiaVideo::VideoRotation prevRotation;
};

PxaOverlay::PxaOverlay( QObject *parent )
    : QObject(parent), d( new PxaOverlayPrivate )
{
    d->isValid = false;
    d->yTransformation = new QImagePlaneTransformation();
    d->yTransformation->setGeometryAlignment( 2 );

    d->uvTransformation = new QImagePlaneTransformation();
}

PxaOverlay::~PxaOverlay()
{
    delete d->yTransformation;
    delete d->uvTransformation;

    close();
    delete d;
}

bool PxaOverlay::isValid() const
{
    return d->isValid;
}

QString PxaOverlay::errorMessage() const
{
    return d->errorMessage;
}

bool PxaOverlay::open( const QRect& geometry )
{
    d->geometry = geometry;

    int posAlign = 4;
    int sizeAlign = 4;
    d->geometry.setLeft( geometry.left()/posAlign*posAlign );
    d->geometry.setTop( geometry.top()/posAlign*posAlign );
    d->geometry.setWidth( geometry.width()/sizeAlign*sizeAlign );
    d->geometry.setHeight( geometry.height()/sizeAlign*sizeAlign );

    if ( d->geometry.isEmpty() ) {
        d->isValid = false;
        return false;
    }

#if FAKE_OVERLAY
    d->overlayFrame = new QVideoFrame( QVideoFrame::Format_YUV420P, d->geometry.size() );
    d->yPlane = d->overlayFrame->planeData(0);
    d->uPlane = d->overlayFrame->planeData(1);
    d->vPlane = d->overlayFrame->planeData(2);

    d->yLineStep = d->overlayFrame->bytesPerLine(0);
    d->uLineStep = d->overlayFrame->bytesPerLine(1);
    d->vLineStep = d->overlayFrame->bytesPerLine(2);

    d->yPlaneSize = d->overlayFrame->planeSize(0).height() * d->yLineStep;
    d->uPlaneSize = d->overlayFrame->planeSize(1).height() * d->uLineStep;
    d->vPlaneSize = d->overlayFrame->planeSize(2).height() * d->vLineStep;

    d->isValid = true;
#else

    //it's necessary to open the base bramebuffer before overlay2
    PxaFrameBuffer base;
    base.open("/dev/fb0");
    //base.print_finfo(stderr);
    //base.print_vinfo(stderr);
    base.close();

    //usleep( 100*1000 );

    //d->geometry = QRect(0,0,240,160);

    d->isValid = d->overlay2.openOverlay("/dev/fb2",
                                         PxaFrameBuffer::YUV420,
                                         d->geometry.width(),
                                         d->geometry.height(),
                                         d->geometry.left(),
                                         d->geometry.top() );

    if ( !d->isValid )
        return false;

    //d->overlay2.print_finfo(stderr);
    //d->overlay2.print_vinfo(stderr);

    uchar *framebuffer = d->overlay2.framebuffer();

    d->yPlane = framebuffer + d->overlay2.roff();
    d->uPlane = framebuffer + d->overlay2.goff();
    d->vPlane = framebuffer + d->overlay2.boff();

    d->yPlaneSize = d->overlay2.rlen();
    d->uPlaneSize = d->overlay2.glen();
    d->vPlaneSize = d->overlay2.blen();


    d->yLineStep = d->overlay2.bpl();
    d->uLineStep = d->yLineStep/2;
    d->vLineStep = d->yLineStep/2;

#endif

    fill( 16, 128, 128 );


    return d->isValid;
}

void PxaOverlay::fill( int y, int u, int v )
{
    if ( !isValid() )
        return;
    
    memset( d->yPlane, y, d->yPlaneSize );  //16 for black screen
    memset( d->uPlane, u, d->uPlaneSize );
    memset( d->vPlane, v, d->vPlaneSize );

}

void PxaOverlay::close()
{
#if FAKE_OVERLAY
    delete d->overlayFrame;
    d->overlayFrame = 0;
#else
    d->overlay2.close();
#endif
    d->yPlane = d->uPlane = d->vPlane = 0;
    d->isValid = false;
}

/** Return overlay geometry, the return value is not
 *  always the same as passed to open() becouse of restrictions
 *  to overlay size and position.
 */
QRect PxaOverlay::geometry() const
{
    return d->geometry;
}

void PxaOverlay::drawFrame( const QVideoFrame& frame,
                            const QRect& frameRect,
                            const QRect& overlayRect,
                            QtopiaVideo::VideoRotation rotation )
{
    if ( !isValid() || frame.isNull() )
        return;

    const uchar *yImagePlane = frame.planeData( 0 );
    const uchar *uImagePlane;
    const uchar *vImagePlane;

    switch ( frame.format() ) {
    case QVideoFrame::Format_YUV420P: 
        uImagePlane = frame.planeData( 1 );
        vImagePlane = frame.planeData( 2 );
        break;
    case QVideoFrame::Format_YV12:
        uImagePlane = frame.planeData( 2 );
        vImagePlane = frame.planeData( 1 );
        break;
    default:
        qWarning() << "PxaOverlay doesn't support image format" << frame.format();
        return;
    }

    if ( d->prevFrameRect != frameRect ||
         d->prevOverlayRect != overlayRect ||
         d->prevRotation != rotation ) {

        d->prevFrameRect = frameRect;
        d->prevOverlayRect = overlayRect;
        d->prevRotation = rotation;

        d->yTransformation->setSrcGeometry( frameRect, QRect(QPoint(0,0),frame.size()), frame.bytesPerLine(0) );
        d->yTransformation->setDstGeometry( overlayRect, QRect(QPoint(0,0),geometry().size()), d->yLineStep );
        d->yTransformation->setRotation( rotation );

        //use the aligned clipped rects to ensure y u and v planes fit accurately
        QRect frameRect2 = d->yTransformation->clippedSrcGeometry();
        frameRect2 = QRect( frameRect2.x()/2, frameRect2.y()/2, frameRect2.width()/2, frameRect2.height()/2 );

        QRect overlayRect2 = d->yTransformation->clippedDstGeometry();
        overlayRect2 = QRect( overlayRect2.x()/2, overlayRect2.y()/2, overlayRect2.width()/2, overlayRect2.height()/2 );

        d->uvTransformation->setSrcGeometry( frameRect2, QRect(QPoint(0,0),frame.size()/2), frame.bytesPerLine(1) );
        d->uvTransformation->setDstGeometry( overlayRect2, QRect(QPoint(0,0),geometry().size()/2), d->uLineStep );
        d->uvTransformation->setRotation( rotation );
    }

    d->yTransformation->transformPlane( yImagePlane, d->yPlane );
    d->uvTransformation->transformPlane( uImagePlane, d->uPlane );
    d->uvTransformation->transformPlane( vImagePlane, d->vPlane );
}


//draw already rotated and prescaled image in YV12 or YUV420P formats
void PxaOverlay::drawFrame( const QVideoFrame& frame )
{
    if ( !isValid() )
        return;

    int w = qMin( frame.size().width(), geometry().width() );
    int h = qMin( frame.size().height(), geometry().height() );
    
    //qWarning() << h << frame.bytesPerLine(0) << frame.bytesPerLine(1) << d->yLineStep;

    const uchar *yImagePlane;
    const uchar *uImagePlane;
    const uchar *vImagePlane;

    yImagePlane = frame.planeData( 0 );

    switch ( frame.format() ) {
    case QVideoFrame::Format_YUV420P: 
        uImagePlane = frame.planeData( 1 );
        vImagePlane = frame.planeData( 2 );
        break;
    case QVideoFrame::Format_YV12:
        uImagePlane = frame.planeData( 2 );
        vImagePlane = frame.planeData( 1 );
        break;
    default:
        qWarning() << "PxaOverlay doesn't support image format" << frame.format();
        return;
    }

    //Y plane
    for ( int y=0; y<h; y++ ) {
        const uchar *srcLine = yImagePlane + y*frame.bytesPerLine(0);
        uchar *dstLine = d->yPlane + y*d->yLineStep;
        memcpy( dstLine, srcLine, w );
    }

    //U plane
    for ( int y=0; y<h/2; y++ ) {
        const uchar *srcLine = uImagePlane + y*frame.bytesPerLine(1);
        uchar *dstLine = d->uPlane + y*d->uLineStep;
        memcpy( dstLine, srcLine, w/2 );
    }

    //V plane
    for ( int y=0; y<h/2; y++ ) {
        const uchar *srcLine = vImagePlane + y*frame.bytesPerLine(2);
        uchar *dstLine = d->vPlane + y*d->vLineStep;
        memcpy( dstLine, srcLine, w/2 );
    }
}

