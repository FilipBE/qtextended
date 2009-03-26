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

#include <QByteArray>
#include <QImage>
#include "greenphonecamera.h"
#include <qtopianamespace.h>

static const signed short redAdjust[] = {
-161,-160,-159,-158,-157,-156,-155,-153,
-152,-151,-150,-149,-148,-147,-145,-144,
-143,-142,-141,-140,-139,-137,-136,-135,
-134,-133,-132,-131,-129,-128,-127,-126,
-125,-124,-123,-122,-120,-119,-118,-117,
-116,-115,-114,-112,-111,-110,-109,-108,
-107,-106,-104,-103,-102,-101,-100, -99,
 -98, -96, -95, -94, -93, -92, -91, -90,
 -88, -87, -86, -85, -84, -83, -82, -80,
 -79, -78, -77, -76, -75, -74, -72, -71,
 -70, -69, -68, -67, -66, -65, -63, -62,
 -61, -60, -59, -58, -57, -55, -54, -53,
 -52, -51, -50, -49, -47, -46, -45, -44,
 -43, -42, -41, -39, -38, -37, -36, -35,
 -34, -33, -31, -30, -29, -28, -27, -26,
 -25, -23, -22, -21, -20, -19, -18, -17,
 -16, -14, -13, -12, -11, -10,  -9,  -8,
  -6,  -5,  -4,  -3,  -2,  -1,   0,   1,
   2,   3,   4,   5,   6,   7,   9,  10,
  11,  12,  13,  14,  15,  17,  18,  19,
  20,  21,  22,  23,  25,  26,  27,  28,
  29,  30,  31,  33,  34,  35,  36,  37,
  38,  39,  40,  42,  43,  44,  45,  46,
  47,  48,  50,  51,  52,  53,  54,  55,
  56,  58,  59,  60,  61,  62,  63,  64,
  66,  67,  68,  69,  70,  71,  72,  74,
  75,  76,  77,  78,  79,  80,  82,  83,
  84,  85,  86,  87,  88,  90,  91,  92,
  93,  94,  95,  96,  97,  99, 100, 101,
 102, 103, 104, 105, 107, 108, 109, 110,
 111, 112, 113, 115, 116, 117, 118, 119,
 120, 121, 123, 124, 125, 126, 127, 128,
};

static const signed short greenAdjust1[] = {
  34,  34,  33,  33,  32,  32,  32,  31,
  31,  30,  30,  30,  29,  29,  28,  28,
  28,  27,  27,  27,  26,  26,  25,  25,
  25,  24,  24,  23,  23,  23,  22,  22,
  21,  21,  21,  20,  20,  19,  19,  19,
  18,  18,  17,  17,  17,  16,  16,  15,
  15,  15,  14,  14,  13,  13,  13,  12,
  12,  12,  11,  11,  10,  10,  10,   9,
   9,   8,   8,   8,   7,   7,   6,   6,
   6,   5,   5,   4,   4,   4,   3,   3,
   2,   2,   2,   1,   1,   0,   0,   0,
   0,   0,  -1,  -1,  -1,  -2,  -2,  -2,
  -3,  -3,  -4,  -4,  -4,  -5,  -5,  -6,
  -6,  -6,  -7,  -7,  -8,  -8,  -8,  -9,
  -9, -10, -10, -10, -11, -11, -12, -12,
 -12, -13, -13, -14, -14, -14, -15, -15,
 -16, -16, -16, -17, -17, -17, -18, -18,
 -19, -19, -19, -20, -20, -21, -21, -21,
 -22, -22, -23, -23, -23, -24, -24, -25,
 -25, -25, -26, -26, -27, -27, -27, -28,
 -28, -29, -29, -29, -30, -30, -30, -31,
 -31, -32, -32, -32, -33, -33, -34, -34,
 -34, -35, -35, -36, -36, -36, -37, -37,
 -38, -38, -38, -39, -39, -40, -40, -40,
 -41, -41, -42, -42, -42, -43, -43, -44,
 -44, -44, -45, -45, -45, -46, -46, -47,
 -47, -47, -48, -48, -49, -49, -49, -50,
 -50, -51, -51, -51, -52, -52, -53, -53,
 -53, -54, -54, -55, -55, -55, -56, -56,
 -57, -57, -57, -58, -58, -59, -59, -59,
 -60, -60, -60, -61, -61, -62, -62, -62,
 -63, -63, -64, -64, -64, -65, -65, -66,
};

static const signed short greenAdjust2[] = {
  74,  73,  73,  72,  71,  71,  70,  70,
  69,  69,  68,  67,  67,  66,  66,  65,
  65,  64,  63,  63,  62,  62,  61,  60,
  60,  59,  59,  58,  58,  57,  56,  56,
  55,  55,  54,  53,  53,  52,  52,  51,
  51,  50,  49,  49,  48,  48,  47,  47,
  46,  45,  45,  44,  44,  43,  42,  42,
  41,  41,  40,  40,  39,  38,  38,  37,
  37,  36,  35,  35,  34,  34,  33,  33,
  32,  31,  31,  30,  30,  29,  29,  28,
  27,  27,  26,  26,  25,  24,  24,  23,
  23,  22,  22,  21,  20,  20,  19,  19,
  18,  17,  17,  16,  16,  15,  15,  14,
  13,  13,  12,  12,  11,  11,  10,   9,
   9,   8,   8,   7,   6,   6,   5,   5,
   4,   4,   3,   2,   2,   1,   1,   0,
   0,   0,  -1,  -1,  -2,  -2,  -3,  -4,
  -4,  -5,  -5,  -6,  -6,  -7,  -8,  -8,
  -9,  -9, -10, -11, -11, -12, -12, -13,
 -13, -14, -15, -15, -16, -16, -17, -17,
 -18, -19, -19, -20, -20, -21, -22, -22,
 -23, -23, -24, -24, -25, -26, -26, -27,
 -27, -28, -29, -29, -30, -30, -31, -31,
 -32, -33, -33, -34, -34, -35, -35, -36,
 -37, -37, -38, -38, -39, -40, -40, -41,
 -41, -42, -42, -43, -44, -44, -45, -45,
 -46, -47, -47, -48, -48, -49, -49, -50,
 -51, -51, -52, -52, -53, -53, -54, -55,
 -55, -56, -56, -57, -58, -58, -59, -59,
 -60, -60, -61, -62, -62, -63, -63, -64,
 -65, -65, -66, -66, -67, -67, -68, -69,
 -69, -70, -70, -71, -71, -72, -73, -73,
};

static const signed short blueAdjust[] = {
-276,-274,-272,-270,-267,-265,-263,-261,
-259,-257,-255,-253,-251,-249,-247,-245,
-243,-241,-239,-237,-235,-233,-231,-229,
-227,-225,-223,-221,-219,-217,-215,-213,
-211,-209,-207,-204,-202,-200,-198,-196,
-194,-192,-190,-188,-186,-184,-182,-180,
-178,-176,-174,-172,-170,-168,-166,-164,
-162,-160,-158,-156,-154,-152,-150,-148,
-146,-144,-141,-139,-137,-135,-133,-131,
-129,-127,-125,-123,-121,-119,-117,-115,
-113,-111,-109,-107,-105,-103,-101, -99,
 -97, -95, -93, -91, -89, -87, -85, -83,
 -81, -78, -76, -74, -72, -70, -68, -66,
 -64, -62, -60, -58, -56, -54, -52, -50,
 -48, -46, -44, -42, -40, -38, -36, -34,
 -32, -30, -28, -26, -24, -22, -20, -18,
 -16, -13, -11,  -9,  -7,  -5,  -3,  -1,
   0,   2,   4,   6,   8,  10,  12,  14,
  16,  18,  20,  22,  24,  26,  28,  30,
  32,  34,  36,  38,  40,  42,  44,  46,
  49,  51,  53,  55,  57,  59,  61,  63,
  65,  67,  69,  71,  73,  75,  77,  79,
  81,  83,  85,  87,  89,  91,  93,  95,
  97,  99, 101, 103, 105, 107, 109, 112,
 114, 116, 118, 120, 122, 124, 126, 128,
 130, 132, 134, 136, 138, 140, 142, 144,
 146, 148, 150, 152, 154, 156, 158, 160,
 162, 164, 166, 168, 170, 172, 175, 177,
 179, 181, 183, 185, 187, 189, 191, 193,
 195, 197, 199, 201, 203, 205, 207, 209,
 211, 213, 215, 217, 219, 221, 223, 225,
 227, 229, 231, 233, 235, 238, 240, 242,
};


#define CLAMP(x) x < 0 ? 0 : x & 0xff

inline void yuv2rgb(int y, int u, int v, quint16 *rgb)
{
    register  int r, g, b;

    r = y + redAdjust[v];
    g = y + greenAdjust1[u] + greenAdjust2[v];
    b = y + blueAdjust[u];

    r = CLAMP(r);
    g = CLAMP(g);
    b = CLAMP(b);

    *rgb = (quint16)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

/*  Four-character-code (FOURCC) */
#define v4l2_fourcc(a,b,c,d)\
	(((__u32)(a)<<0)|((__u32)(b)<<8)|((__u32)(c)<<16)|((__u32)(d)<<24))

#define V4L2_PIX_FMT_YUYV    v4l2_fourcc('Y','U','Y','V') /* 16  YUV 4:2:2     */
#define V4L2_PIX_FMT_UYVY    v4l2_fourcc('U','Y','V','Y') /* 16  YUV 4:2:2     */
#define V4L2_PIX_FMT_RGB565    v4l2_fourcc('R','G','B','P') /* 16  YUV 4:2:2     */

#define ERROR qWarning()<<"greenphonecamera error"<<__FILE__<<__LINE__;
bool Greenphone::open()
{
    if(isOpen) return true;
    fd = ::open( "/dev/video0", O_RDWR );
    if ( fd == -1 ) {
        ERROR
        return false;
    }

    return isOpen = true;

}

void Greenphone::close()
{
    ::close(fd);
    isOpen = false;
}



///////// PREVIEW///////////
void GPreview::start(unsigned int format, QSize resolution, int framerate)
{
    Q_UNUSED(format);
    current_resolution = resolution;
    Q_UNUSED(framerate);

    if(!gp->isOpen)
        if(!gp->open()) return;

    ufds.fd = gp->fd;
    ufds.events = POLLIN;

    preview_active = true;
    struct { int r1; int r2; } _pm = { 15, 15 };
    ioctl(gp->fd, 215, &_pm);
    struct video_window wind;
    memset(&wind, 0, sizeof(wind));
    wind.width = current_resolution.width();
    wind.height = current_resolution.height();
    if ( ioctl( gp->fd, VIDIOCSWIN, &wind ) < 0 ) {
        ERROR
    }
    manageBuffers(true);
    if(notifier == 0) {
        notifier = new QSocketNotifier(gp->fd, QSocketNotifier::Read , this);
        notifier->setEnabled(false);
        connect(notifier, SIGNAL(activated(int)), this, SLOT(captureFrame()));
    }
    if(ioctl(gp->fd, VIDIOCCAPTURE, 0) < 0)
        ERROR
    notifier->setEnabled(true);
}

void GPreview::stop()
{
    preview_active = false;

    notifier->setEnabled(false);
    if(ioctl(gp->fd, VIDIOCCAPTURE, -1) < 0)
        ERROR
    manageBuffers(false);
    sequence = 0;

    if(notifier) {
        disconnect(notifier);
        delete notifier;
        notifier = 0;
    }

    gp->close();

}

void GPreview::captureFrame()
{

    convertFrame();
    preview_buffer.sequence = sequence;
    still_buffer.sequence = sequence;

    QVideoFrame vframe( QVideoFrame::Format_RGB565,
                        QSize(preview_buffer.width, preview_buffer.height),
                        reinterpret_cast<uchar*>(preview_buffer.data));
    emit frameReady(vframe);
}

void GPreview::forceFrameCapture()
{
    bool got_frame = false;
    int retval;
    int iter = 0;
    while(!got_frame || ++iter < 10)
    {
        retval = poll(&ufds, 1, 200);
        if (retval > 0) {
            convertFrame();
            got_frame = true;
        }
        else if (retval == 0) {
        }
        else if(retval < 0) {
            break;
        }
    }
    if (!got_frame)
        qWarning()<<"camera: failed to capture frame";
}

QList<unsigned int> GPreview::framerates()
{
    return QList<unsigned int>()<< 15;
}

unsigned int GPreview::framerate()
{
    return 15;
}

QtopiaCamera::FormatResolutionMap GPreview::formats() const
{
    QtopiaCamera::FormatResolutionMap f;
    f.insert(V4L2_PIX_FMT_RGB565, QList<QSize>()<<QSize(320,240));

    return f;
}

unsigned int GPreview::format()
{
    return V4L2_PIX_FMT_RGB565;
}

QSize GPreview::resolution()
{
    return current_resolution;
}

bool GPreview::hasZoom() const
{
    return false;
}

QPair<unsigned int,unsigned int> GPreview::zoomRange()
{
    return QPair<unsigned int,unsigned int>(0,0);
}

void GPreview::zoomIn()
{
}

void GPreview::zoomOut()
{
}

QList<QCameraControl*> GPreview::controls() const
{
    return QList< QCameraControl* >();
}

void GPreview::setValue(quint32 id, int value)
{
    Q_UNUSED(id);
    Q_UNUSED(value);
}


void GPreview::manageBuffers(bool b)
{
    if(b) {

        // Enable mmap-based access to the camera.
        memset( &mbuf, 0, sizeof( struct video_mbuf ) );
        if ( ioctl( gp->fd, VIDIOCGMBUF, &mbuf ) < 0 ) {
            ERROR
        }
        // Mmap the designated memory region.
        frames = (unsigned char *)mmap( 0, mbuf.size, PROT_READ | PROT_WRITE,
				    MAP_SHARED, gp->fd, 0 );
        if ( !frames || frames == (unsigned char *)(long)(-1) ) {
            ERROR
        }
        int w = resolution().width(),
            h = resolution().height();

        imageBuf = (unsigned char*) malloc(w*h* 2);


        preview_buffer.width = h;
        preview_buffer.height = w;
        preview_buffer.bytesused = w*h*2;
        preview_buffer.data = imageBuf;

        still_buffer.width = h;
        still_buffer.height = w;
        still_buffer.bytesused = w*h*2;
        still_buffer.data = imageBuf;


    } else {
        if ( frames != 0 ) {
            munmap( frames, mbuf.size );
            frames =0;
        }
        free(imageBuf);
    }
}

void GPreview::lock()
{
    notifier->setEnabled(false);
}

void GPreview::unlock()
{
    notifier->setEnabled(true);
}

void  GPreview::convertFrame()
{
    static unsigned int _lastFrame = 1;
    static int lock = 0;
    if (lock) return;

    lock = 1;

    currentFrame = (_lastFrame - 1) % mbuf.frames;
    int width = current_resolution.width();
    int height = current_resolution.height();
    unsigned char *buf_orig = frames + mbuf.offsets[currentFrame];

    for(int yy_base = 0; yy_base < width; yy_base += 64)
    {
        for(int x = 0; x < height && x < width; ++x)
        {
            for (int y = yy_base; y < width && y < yy_base + 64; y++)
            {
                unsigned short *dest = (unsigned short*)imageBuf + x + y * height;
                unsigned char *buf = buf_orig + ((height - 1) - x) * 2 * width + (y & 0xFFFFFFFE) * 2;
                int u = buf[0];
                int v = buf[2];
                yuv2rgb(buf[1 + (y & 0x1) * 2], u, v, dest);
            }
        }
    }
    unsigned int cap = 0;
    ioctl(gp->fd, 228, &cap);
    if(cap != _lastFrame) {
        _lastFrame = cap;
        sequence = _lastFrame;
    }
    lock = 0;
}

QtopiaCamera::FormatResolutionMap GStill::formats() const
{
    QtopiaCamera::FormatResolutionMap f;
    f.insert(V4L2_PIX_FMT_RGB565, QList<QSize>() << QSize(640,480) << QSize(320,240) << QSize(176,144));
    return f;
}

unsigned int GStill::format()
{
    return current_format = V4L2_PIX_FMT_RGB565;
}

QSize GStill::resolution()
{
    return current_resolution;
}

void GStill::takeStillImage(unsigned int format, QSize resolution, int count, unsigned int msecs )
{
    if(count < 1 || format != V4L2_PIX_FMT_RGB565 || !formats().value(format).contains(resolution)) return;
    wasStarted = false;
    revert = false;

    if(!gp->preview->preview_active) {
        gp->preview->start(format, resolution, gp->preview->framerate());
        wasStarted = true;
    } else if (resolution != gp->preview->resolution()) {
        gp->preview->stop();
        gp->preview->start(format, resolution, gp->preview->framerate());
        revert = true;
    }

    current_resolution = resolution;
    current_format = format;

    if (revert)
        gp->preview->lock();
    takeShot(count,msecs);
}

void GStill::takeShot(int count, int msec)
{
    count--;

    if (revert)
       gp->preview->forceFrameCapture();

    QByteArray ba(reinterpret_cast<char*>(gp->preview->preview_buffer.data), gp->preview->preview_buffer.bytesused);
    emit imageReady(ba, QSize(gp->preview->preview_buffer.width, gp->preview->preview_buffer.height), true);

    if (count == 0)
        takeShotsDone();
    else
        QTimer::singleShot(msec, this , SLOT(takeShot(count)));
}

void GStill::takeShotsDone()
{
    if(wasStarted) {
        gp->preview->stop();
    } else if(revert) {
        gp->preview->stop();
        gp->preview->start(current_format, QSize(320,240), gp->preview->framerate());
    }

    if (revert)
        gp->preview->unlock();
}

QList<QCameraControl*> GStill::controls() const
{
    return QList<QCameraControl*>();
}

void GStill::setValue(quint32 id, int value)
{
    Q_UNUSED(id);
    Q_UNUSED(value);
}

