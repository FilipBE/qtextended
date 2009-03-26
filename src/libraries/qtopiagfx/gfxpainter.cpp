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

#define private public
#include <QRegion>
#undef private
#include "gfxpainter.h"
#include "gfximage.h"
#include <QRgb>
#include "gfx.h"
#include <QDebug>
#include <routines.h>
#include <QImage>
#include <QMutex>
#include <QThread>
#include <QCoreApplication>
#include <QEvent>
#if defined(Q_WS_QWS)
#include <QDirectPainter>
#endif
#include <private/qwindowsurface_qws_p.h>
#include <private/qwidget_p.h>
#include <QPaintEvent>
#include "def_transform.h"
#include <unistd.h>

// XXX - needed until QRegion::isRect() is in build by default
struct QRegionPrivate : public QRegion::QRegionData {
    enum { Single, Vector } mode;
};
bool qt_region_is_rect(const QRegion &r)
{
    if(r.d->qt_rgn == r.d) {
        // QWS region
        return reinterpret_cast<QRegionPrivate *>(r.d->qt_rgn)->mode == QRegionPrivate::Single;
    } else {
        return false;
    }
}

extern bool gfx_use_qt;
extern bool gfx_report_hazards;

class MainThreadProxy : public QObject
{
public:
    MainThreadProxy(GfxPainter *);

    void call();

protected:
    virtual bool event(QEvent *);

private:
    QMutex _lock;
    bool _inProgress;
    GfxPainter *_fpainter;
};

MainThreadProxy::MainThreadProxy(GfxPainter *p)
: QObject(QCoreApplication::instance()), _inProgress(false), _fpainter(p)
{
}

void MainThreadProxy::call()
{
    _lock.lock();
    if(_inProgress) {
        _lock.unlock();
        return;
    }

    if(QThread::currentThread() == thread()) {
        _fpainter->mainThreadProxyFunc();
    } else {
        _inProgress = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    }
    _lock.unlock();

}

bool MainThreadProxy::event(QEvent *)
{
    _lock.lock();
    _fpainter->mainThreadProxyFunc();
    _inProgress = false;
    _lock.unlock();
    return false;
}


#if defined(Q_WS_QWS)

class GfxDirectPainter : public QDirectPainter
{
public:
    GfxDirectPainter(GfxPainter *_p)
        : p(_p) {}

protected:
    virtual void regionChanged(const QRegion &newRegion);

private:
    GfxPainter *p;
};

void GfxDirectPainter::regionChanged(const QRegion &newRegion)
{
    p->clipRegion = newRegion;
    if(!qt_region_is_rect(newRegion))
        p->clipRects = newRegion.rects();
    else
        p->clipRects.clear();

}

#endif // Q_WS_QWS

/*!
  \class GfxPainter
    \inpublicgroup QtBaseModule

  Input image types supported: ARGB32p, RGB16, RGB32
  Output image type supported: RGB32, RGB16

  Filters supported: Opacity, Transformation*

  Clipping supported: box clipping, image alpha clipping*,
                      horizontal alpha clipping*,
                      vertical alpha clipping*
 */

enum HazardType { Blend_RGB16_RGB32_Opacity = 0x00000001,
                  Blend_RGB16_RGB16_Opacity = 0x00000002 };
static inline void hazard(HazardType t)
{
    if(!gfx_report_hazards) return;

    static unsigned int hazards;
    if(!(t & hazards)) {

        QString h;
        switch(t) {
            case Blend_RGB16_RGB32_Opacity:
                h = "Blend_RGB16_RGB32_Opacity";
                break;
            case Blend_RGB16_RGB16_Opacity:
                h = "Blend_RGB16_RGB16_Opacity";
                break;
        }
        hazards |= t;

        qWarning() << "GfxPainter: Performance hazard" << h.toLatin1().constData();
    }
}

enum ImageType { RGB16 = 0, RGB32 = 1, ARGB32p = 2 };
typedef void(*SimpleBlendFunc)(uchar *in, uchar *out, int width, uchar opacity);

static void sbf_argb32p_rgb32(uchar *in, uchar *out, int width, uchar opacity)
{
    q_blendroutines.blend_argb32p_rgb32((uint *)out, (uint *)in,
                                        opacity, width, (uint *)out);
}

static void sbf_argb32p_rgb16(uchar *in, uchar *out, int width, uchar opacity)
{
    q_blendroutines.blend_argb32p_rgb16((ushort *)out, (uint *)in,
                                        opacity, width, (ushort *)out);
}

static void sbf_rgb16_rgb32(uchar *_in, uchar *_out, int width, uchar opacity)
{
    if(opacity == 0xFF) {
        q_colorroutines.color_rgb16_rgb32((ushort *)_in, width, (uint *)_out);
    } else {
        hazard(Blend_RGB16_RGB32_Opacity);

        ushort *in = (ushort *)_in;
        uint *out = (uint *)_out;

        uint in_conv[512];
        while(width) {
            int wm = qMin(width, 512);

            q_colorroutines.color_rgb16_rgb32(in, wm, in_conv);
            q_blendroutines.blend_argb32p_rgb32(out, in_conv, opacity, wm, out);

            in += wm;
            out += wm;
            width -= wm;
        }
    }
}

static void sbf_rgb16_rgb16(uchar *_in, uchar *_out, int width, uchar opacity)
{
    if(opacity == 0xFF) {
        q_memoryroutines.memcpy(_out, _in, width * 2);
    } else {
        hazard(Blend_RGB16_RGB16_Opacity);

        ushort *in = (ushort *)_in;
        ushort *out = (ushort *)_out;

        uint in_conv[512];
        while(width) {
            int wm = qMin(width, 512);

            q_colorroutines.color_rgb16_rgb32(in, wm, in_conv);
            q_blendroutines.blend_argb32p_rgb16(out, in_conv, opacity, wm, out);

            in += wm;
            out += wm;
            width -= wm;
        }
    }
}

static void sbf_rgb32_rgb32(uchar *in, uchar *out, int width, uchar opacity)
{
    if(opacity == 0xFF) {
        q_memoryroutines.memcpy(out, in, width * 4);
    } else {
        q_blendroutines.blend_argb32p_rgb32((uint *)out, (uint *)in, opacity,
                                            width, (uint *)out);
    }
}

static void sbf_rgb32_rgb16(uchar *in, uchar *out, int width, uchar opacity)
{
    if(opacity == 0xFF) {
        q_colorroutines.color_rgb32_rgb16((uint *)in, width, (ushort *)out);
    } else {
        q_blendroutines.blend_argb32p_rgb16((ushort *)out, (uint *)in,
                                            opacity, width, (ushort *)out);
    }
}

static const SimpleBlendFunc simpleBlendFuncs[RGB32 + 1][ARGB32p + 1] = {
    {
        sbf_rgb16_rgb16,
        sbf_rgb32_rgb16,
        sbf_argb32p_rgb16,
    },
    {
        sbf_rgb16_rgb32,
        sbf_rgb32_rgb32,
        sbf_argb32p_rgb32
    }
};


GfxPainter::GfxPainter()
: fBuffer(0), buffer(0), _opacity(0xFF), realOpacity(1.0f),
  mainThreadProxy(0), dp(0),  p(0), pImg(0), useQt(false), depth(Depth_16),
  opacityFunc(0), opacityFuncData(0)
{
    Gfx::init();
    useQt = gfx_use_qt;
    mainThreadProxy = new MainThreadProxy(this);
#if defined(Q_WS_QWS)
    dp = new GfxDirectPainter(this);
#endif
}

GfxPainter::GfxPainter(QImage &img, const QRegion &reg)
: fBuffer(0), buffer(0), _opacity(0xFF), realOpacity(1.0f),
  mainThreadProxy(0), dp(0),  p(0), pImg(0), useQt(false), depth(Depth_16),
  opacityFunc(0), opacityFuncData(0)
{
    Gfx::init();
    QImage::Format format = img.format();

    if(!gfx_use_qt && (format == QImage::Format_RGB16 || format == QImage::Format_RGB32)) {
        if(format == QImage::Format_RGB16) {
            depth = Depth_16;
        } else if(format == QImage::Format_RGB32) {
            depth = Depth_32;
        }

        buffer = img.bits();
        width = img.width();
        step = img.bytesPerLine() / depth;
        height = img.height();

        if(reg.isEmpty())
            clipRegion = img.rect();
        else
            clipRegion = reg;
        if(!qt_region_is_rect(clipRegion))
            clipRects = clipRegion.rects();
    } else {
        useQt = true;

        if(format == QImage::Format_ARGB32_Premultiplied) {
            depth = Depth_32;
            buffer = img.bits();
            width = img.width();
            step = img.bytesPerLine() / depth;
            height = img.height();
        }
    }

    if(useQt)
        p = new QPainter(&img);
}

GfxPainter::GfxPainter(QWidget *wid, QPaintEvent *e)
: fBuffer(0), buffer(0), _opacity(0xFF), realOpacity(1.0f),
  mainThreadProxy(0), dp(0),  p(0), pImg(0), useQt(false), depth(Depth_16),
  opacityFunc(0), opacityFuncData(0)
{
    Gfx::init();
#ifdef Q_WS_QWS
    if(!gfx_use_qt) {
        // XXX all this relies on the memory or shared memory window surface
        // we should probably test for this
        QWSWindowSurface *surface =
            static_cast<QWSWindowSurface *>(wid->windowSurface());
        QImage *windowImage =
            static_cast<QImage *>(surface->paintDevice());
        QPoint offset =
            surface->painterOffset();

        if(windowImage->format() == QImage::Format_RGB16) {
            depth = Depth_16;
        } else if(windowImage->format() == QImage::Format_ARGB32_Premultiplied ||
                windowImage->format() == QImage::Format_ARGB32 ||
                windowImage->format() == QImage::Format_RGB32) {
            depth = Depth_32;
        } else {
            qWarning().nospace() << "GfxPainter: Unknown window image format (" << windowImage->format() << ").";
            useQt = true;
        }


        if(!useQt) {
            // Add child widget offset
            if (!wid->isWindow())
                offset += wid->mapTo(wid->window(), QPoint(0,0));

            buffer = windowImage->bits() + offset.x() * depth +
                     offset.y() * windowImage->bytesPerLine();
            width = windowImage->width() - offset.x();
            step = windowImage->width();
            height = windowImage->height() - offset.y();

            clipRegion = e->region();
            if(!qt_region_is_rect(clipRegion))
                clipRects = clipRegion.rects();
        }
    } else {
#endif
        useQt = true;
#ifdef Q_WS_QWS
    }
#endif

    if(useQt)
        p = new QPainter(wid);
}

GfxPainter::~GfxPainter()
{
    delete mainThreadProxy; mainThreadProxy = 0;
    if(p)
        delete p;
}

// must be called from main thread
void GfxPainter::setRect(const QRect &rect)
{
    // XXX - need to support 32-bit
    if(fRect == rect)
        return;

    fRect = rect;

    width = rect.width();
    step = width;
    height = rect.height();

#if defined(Q_WS_QWS)
    linestep = QDirectPainter::linestep();
    fBuffer = QDirectPainter::frameBuffer();
#endif

    if(buffer) {
        delete [] buffer;
        buffer = 0;
    }

    if(fRect.isEmpty()) {
        if(p) {
            delete p;
            delete pImg;
            p = 0;
            pImg = 0;
        }
    } else {
        buffer = new uchar[width * height * 2];
        if(p) {
            delete p;
            delete pImg;
        }
        pImg = new QImage(buffer, width, height, QImage::Format_RGB16);
        p = new QPainter(pImg);
    }

    QRegion newRegion(rect);
    clipRegion = newRegion;
    if(!qt_region_is_rect(newRegion))
        clipRects = newRegion.rects();
    else
        clipRects.clear();

//    mainThreadProxy->call();
}

void GfxPainter::clear()
{
    clear(0);
}

void GfxPainter::clear(ushort val)
{
    ushort *dest = (ushort *)buffer;
    for(int ii = 0; ii < height; ++ii) {
        q_memoryroutines.memset_16(dest, val, width);
        dest += step;
    }
}

qreal GfxPainter::opacity() const
{
    return realOpacity;
}

void GfxPainter::setOpacity(qreal value)
{
    if(value == 1.0f) {
        _opacity = 0xFF;
    } else if(value == 0.0f) {
        _opacity = 0x00;
    } else {
        _opacity = (uchar)((qreal)0xFF * value);
    }
    realOpacity = value;
    if(useQt)
        painter()->setOpacity(realOpacity);
}

void GfxPainter::setUserClipRect(const QRect &r)
{
    if(useQt) {
        if(r.isEmpty()) {
            // XXX - qt clipping bug
            painter()->setClipRect(QRect(0, 0, 100000, 100000));
        } else {
            painter()->setClipRect(r);
        }
    } 
    _userClipRect = r;
}

QRect GfxPainter::clipRect() const
{
    return clipRegion.boundingRect();
}

QRect GfxPainter::userClipRect() const
{
    return _userClipRect;
}

void GfxPainter::mainThreadProxyFunc()
{
#if defined(Q_WS_QWS)
    dp->QDirectPainter::setGeometry(fRect);
    dp->QDirectPainter::raise();
#endif
}

void GfxPainter::flipUnclipped(const QRect &rect)
{
    uchar *output = fBuffer + fRect.y() * linestep + fRect.x() * depth;
    uchar *input = buffer;

    output += rect.y() * linestep + rect.x() * depth;
    input += rect.y() * width * depth + rect.x() * depth;

    for(int ii = 0; ii < rect.height(); ++ii) {
        q_memoryroutines.memcpy((uchar *)output, (uchar *)input, rect.width() * depth);
        input += width * depth;
        output += linestep;
    }
}

void GfxPainter::flip(const QRect &r)
{
    if(clipRegion == QRegion(fRect)) {
        QRect fr = fRect.intersected(r);
        if(!fr.isEmpty()) flipUnclipped(fr);
    } else {
        QRegion allocated = clipRegion.translated(-fRect.topLeft());
        QVector<QRect> rects = allocated.rects();
        for(int ii = 0; ii < rects.count(); ++ii) {
            QRect fr = rects.at(ii).intersected(r);
            if(!fr.isEmpty()) flipUnclipped(fr);
        }
    }
}

void GfxPainter::flip()
{
    flip(QRect(0, 0, width, height));
}

QImage GfxPainter::string(const QString &str, const QColor &c, const QFont &f)
{
    QFontMetrics fm(f);
    QSize size = fm.boundingRect(str).size();
    size += QSize(2, 1);
    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    img.fill(0);
    QPainter p(&img);
    p.setPen(c);
    p.setFont(f);
    p.drawText(img.rect(), Qt::AlignVCenter, str);
    return img;
}

static inline ushort sp_blend_16(ushort p1, ushort p2, uint mul1, uint mul2)
{
    uint rv =
        ((((p1 & 0xF81F) * mul1) + ((p2 & 0xF81F) * mul2)) >> 5) & 0xF81F;
    rv |=
        ((((p1 & 0x07E0) * mul1) + ((p2 & 0x07E0) * mul2)) >> 5) & 0x07E0;
    return rv;
}

static inline uint sp_blend_32(uint p1, uint p2, uint mul1, uint mul2)
{
    uint rv =
        ((((p1 & 0x07E0F81F) * mul1) + ((p2 & 0x07E0F81F) * mul2)) >> 5) & 0x07E0F81F;
    rv |=
        ((((p1 & 0xF81F07E0) >> 5) * mul1) + (((p2 & 0xF81F07E0) >> 5) * mul2)) & 0xF81F07E0;
    return rv;
}

#include "def_blendhelper.h"
#include <math.h>
void GfxPainter::drawImage(qreal x, int y, const QImage &img)
{
    if(useQt || _opacity != 0xFF || img.format() != QImage::Format_RGB16)  {
        drawImage(int(x), y, img);
        return;
    }

    qreal pos_x = x + 1000.;
    int int_x = int(pos_x);
    qreal float_x = pos_x - int_x;
    int mul_inv = int(32. * float_x);
    int mul = 32 - mul_inv;

    int src_depth = 2;

    QPoint pt((int)floor(x), y);
    QRect origImgRect(pt, img.size());
    if(!_userClipRect.isNull())
        origImgRect = origImgRect & _userClipRect;
    if(origImgRect.isEmpty())
        return;

    if(qt_region_is_rect(clipRegion)) {
        QRect imgRect = origImgRect.intersected(clipRegion.boundingRect());
        QRect baseRect = imgRect.translated(int(-floor(x)), -y);

        const uchar *srcBits = img.bits();
        int srcStep = img.bytesPerLine();
        srcBits += baseRect.y() * srcStep + src_depth * baseRect.x();

        int width = baseRect.width();
        if(!width)
            return;
        int height = baseRect.height();

        uchar *destBits = buffer;
        destBits += imgRect.y() * step * depth + imgRect.x() * depth;
        bool destExhaust = (imgRect.right() >= (this->width - 1));
        bool imgExhaust = (baseRect.right() >= (img.width() - 1));
        bool imgStart = (baseRect.x() == 0);
#ifdef QT_ARCH_ARMV5E
#define PLD(src) \
    asm volatile("pld [%0, #32]\n\t" \
            : : "r"(src));
#else
#define PLD(src)
#endif

#define SRC_OP \
            ushort *destBits_16 = (ushort *)destBits; \
            ushort *srcBits_16 = (ushort *)srcBits; \
            int cwidth = width; \
            uint left = 0; \
            if(imgStart) left = *destBits_16; \
            else left = *(srcBits_16 - 1); \
            if(((intptr_t)srcBits_16) & 0x3) { \
                uint right = *srcBits_16++; \
                *destBits_16++ = sp_blend_16(left, right, mul_inv, mul); \
                left = right; \
                cwidth--; \
            } \
            uint *srcBits_32 = (uint *)srcBits_16;\
            for(int xx = 0; xx < cwidth / 2; xx++) { \
                PLD(srcBits_32); \
                PLD(destBits_16); \
                uint right = *srcBits_32++; \
                left |= right << 16; \
                uint pixel = sp_blend_32(left, right, mul_inv, mul); \
                *destBits_16++ = pixel; \
                *destBits_16++ = pixel >> 16; \
                left = right >> 16; \
            } \
            if(cwidth && (cwidth & 0x1)) { \
                uint right = srcBits_16[cwidth - 1]; \
                *destBits_16++ = sp_blend_16(left, right, mul_inv, mul); \
                left = right; \
            } \
            if(!destExhaust) { \
                uint right = 0; \
                if(imgExhaust) right = *destBits_16; \
                else right = srcBits_16[cwidth]; \
                *destBits_16++ = sp_blend_16(left, right, mul_inv, mul); \
            } \
            destBits += step * depth; \
            srcBits += srcStep;

        for(int ii = 0; ii < height; ++ii) {
            SRC_OP;
        }
    } else {
        for(int ii = 0; ii < clipRects.count(); ++ii) {
            QRect imgRect = origImgRect & clipRects.at(ii);
            if(imgRect.isEmpty())
                continue;
            QRect baseRect = imgRect.translated(int(-floor(x)), -y);

            const uchar *srcBits = img.bits();
            int srcStep = img.bytesPerLine();
            srcBits += baseRect.y() * srcStep + src_depth * baseRect.x();

            int width = baseRect.width();
            if(!width)
                return;
            int height = baseRect.height();

            uchar *destBits = buffer;
            destBits += imgRect.y() * step * depth + imgRect.x() * depth;
            bool destExhaust = (imgRect.right() >= (this->width - 1));
            bool imgExhaust = (baseRect.right() >= (img.width() - 1));
            bool imgStart = (baseRect.x() == 0);

            for(int ii = 0; ii < height; ++ii) {
                SRC_OP;
            }
        }
    }
}

void GfxPainter::drawImage(int x, int y, const GfxImageRef &img)
{
    drawImage(x, y, img, false);
}

void GfxPainter::drawImage(const QPoint &p, const GfxImageRef &img)
{
    drawImage(p.x(), p.y(), img, false);
}

void GfxPainter::drawImageFlipped(int x, int y, const GfxImageRef &img)
{
    drawImage(x, y, img, true);
}

void GfxPainter::drawImageFlipped(const QPoint &p, const GfxImageRef &img)
{
    drawImage(p.x(), p.y(), img, true);
}

void GfxPainter::drawImageFlipped(int x, int y, const QImage &img)
{
    drawImage(x, y, GfxImageRef(img), true);
}

void GfxPainter::drawImageFlipped(const QPoint &p, const QImage &i)
{
    drawImageFlipped(p.x(), p.y(), i);
}

void GfxPainter::setHorizontalOpacityFunction(HorizontalOpacityFunction func,
                                              void *data)
{
    opacityFunc = func;
    opacityFuncData = data;
}

void GfxPainter::drawImage(int x, int y, const QImage &img)
{
    drawImage(x, y, img, false);
}

static inline uchar opacity_mul(uchar op1, uchar op2)
{
    if(op1 == 0xFF)
        return op2;
    else if(op1 == 0)
        return op1;
    else
        return op2 * (op1 + 1) >> 8;
}

void GfxPainter::drawImage(int x, int y, const GfxImageRef &img, bool flipped)
{
    if(useQt) {
        painter()->drawImage(x, y, img.toImage());
        return;
    }

    if(_opacity == 0)
        return;


    int src_depth = (img.format() == QImage::Format_RGB16)?(2):(4);
    SimpleBlendFunc sbf = 0;
    switch(img.format()) {
        case QImage::Format_RGB32:
            sbf = simpleBlendFuncs[depth / 2 - 1][RGB32];
            break;
        case QImage::Format_ARGB32_Premultiplied:
            sbf = simpleBlendFuncs[depth / 2 - 1][ARGB32p];
            break;
        case QImage::Format_RGB16:
            sbf = simpleBlendFuncs[depth / 2 - 1][RGB16];
            break;
        default:
            qWarning() << "GfxPainter: Source format" << img.format()
                       << "not supported";
            return;
            break;
    }

    QRect origImgRect(QPoint(x, y), img.size());
    if(!_userClipRect.isNull())
        origImgRect = origImgRect & _userClipRect;
    if(origImgRect.isEmpty())
        return;

    if(qt_region_is_rect(clipRegion)) {
        QRect imgRect = origImgRect.intersected(clipRegion.boundingRect());
        QRect baseRect = imgRect.translated(-x, -y);

        const uchar *srcBits = img.bits();
        int srcStep = img.bytesPerLine();
        srcBits += baseRect.y() * srcStep + src_depth * baseRect.x();

        int width = baseRect.width();
        int height = baseRect.height();

        uchar *destBits = buffer;
        destBits += imgRect.y() * step * depth + imgRect.x() * depth;

        if(flipped) {
            srcBits += (height - 1) * srcStep;
            srcStep *= -1;
        }

        if(opacityFunc) {
            for(int ii = 0; ii < height; ++ii) {
                uchar opacity = opacityFunc(ii + imgRect.y(), opacityFuncData);
                opacity = opacity_mul(opacity, _opacity);
                sbf((uchar *)srcBits, destBits, width, opacity);
                destBits += step * depth;
                srcBits += srcStep;
            }
        } else {
            for(int ii = 0; ii < height; ++ii) {
                sbf((uchar *)srcBits, destBits, width, _opacity);
                destBits += step * depth;
                srcBits += srcStep;
            }
        }
    } else {
        for(int ii = 0; ii < clipRects.count(); ++ii) {
            QRect imgRect = origImgRect & clipRects.at(ii);
            if(imgRect.isEmpty())
                continue;
            QRect baseRect = imgRect.translated(-x, -y);

            const uchar *srcBits = img.bits();
            int srcStep = img.bytesPerLine();
            srcBits += baseRect.y() * srcStep + src_depth * baseRect.x();

            int width = baseRect.width();
            int height = baseRect.height();

            uchar *destBits = buffer;
            destBits += imgRect.y() * step * depth + imgRect.x() * depth;

            if(flipped) {
                srcBits += (height - 1) * srcStep;
                srcStep *= -1;
            }
            if(opacityFunc) {
                for(int ii = 0; ii < height; ++ii) {
                    uchar opacity = opacityFunc(ii + imgRect.y(), opacityFuncData);
                    opacity = opacity_mul(opacity, _opacity);
                    sbf((uchar *)srcBits, destBits, width, opacity);
                    destBits += step * depth;
                    srcBits += srcStep;
                }
            } else {
                for(int ii = 0; ii < height; ++ii) {
                    sbf((uchar *)srcBits, destBits, width, _opacity);
                    destBits += step * depth;
                    srcBits += srcStep;
                }
            }
        }
    }
}

void GfxPainter::fillOpaque(const QRect &r, const QRgb &rgb)
{
    uint color;
    if(depth == Depth_16) {
        color = (rgb & 0xF80000) >> 8 |
                (rgb & 0xFC00) >> 5 |
                (rgb & 0xF8) >> 3;
    } else {
        color = rgb;
    }

    if(qt_region_is_rect(clipRegion)) {
        QRect fillRect = r.intersected(clipRegion.boundingRect());

        int width = fillRect.width();
        int height = fillRect.height();

        if(depth == Depth_16) {
            ushort *destBits = (ushort *)buffer;
            uint destStep = step;
            destBits += fillRect.y() * destStep + fillRect.x();

            for(int ii = 0; ii < height; ++ii) {
                q_memoryroutines.memset_16(destBits, color, width);
                destBits += destStep;
            }
        } else {
            uint *destBits = (uint *)buffer;
            uint destStep = step;
            destBits += fillRect.y() * destStep + fillRect.x();

            for(int ii = 0; ii < height; ++ii) {
                q_memoryroutines.memset_32(destBits, color, width);
                destBits += destStep;
            }
        }
    } else {
        QRect p;
        for(int ii = 0; ii < clipRects.count(); ++ii) {
            QRect fillRect = r.intersected(clipRects.at(ii));

            int width = fillRect.width();
            int height = fillRect.height();

            if(depth == Depth_16) {
                ushort *destBits = (ushort *)buffer;
                uint destStep = step;
                destBits += fillRect.y() * destStep + fillRect.x();

                for(int ii = 0; ii < height; ++ii) {
                    q_memoryroutines.memset_16(destBits, color, width);
                    destBits += destStep;
                }
            } else {
                uint *destBits = (uint *)buffer;
                uint destStep = step;
                destBits += fillRect.y() * destStep + fillRect.x();

                for(int ii = 0; ii < height; ++ii) {
                    q_memoryroutines.memset_32(destBits, color, width);
                    destBits += destStep;
                }
            }

        }
    }
}

void GfxPainter::drawImage(const QRect &target, const QImage &img)
{
    if(useQt)
        painter()->drawImage(target, img);
    else
        drawImage(target, GfxImageRef(img));
}

void GfxPainter::drawImage(const QRect &target, const GfxImageRef &img)
{
    if(useQt) {
        painter()->drawImage(target, img.toImage(), img.rect());
        return;
    }

    if(img.rect().isEmpty())
        return;

    QRect imgRect = img.rect();
    if(target.size() == imgRect.size()) {
        drawImage(target.topLeft(), img);
    } else {
        QMatrix matrix;
        matrix.translate(target.x(), target.y());
        matrix.scale(qreal(target.width()) / qreal(imgRect.width()), 
                     qreal(target.height()) / qreal(imgRect.height()));
        drawImageTransformed(matrix, img.toImage());
    }
 
}

void GfxPainter::drawImage(const QRect &target, const GfxImageRef &img, 
                           const QRect &source)
{
    drawImage(target, img.subImage(source));
}

void GfxPainter::drawImage(const QRect &target, const QImage &img, 
                           const QRect &source)
{
    if(0 && useQt) {
        painter()->drawImage(target, img, source);
    } else {
        drawImage(target, GfxImageRef(img), source);
    }
}


void GfxPainter::fillRect(const QRect &_r, const QColor &c)
{
    if(useQt) {
        painter()->fillRect(_r, c);
        return;
    }

    if(!_opacity)
        return;

    QRect r = _r;
    if(!_userClipRect.isEmpty())
        r &= _userClipRect;

    QRgb rgba = c.rgba();
    if(_opacity != 0xFF)
        rgba = (((rgba & 0xFF00FF) * _opacity) >> 8) & 0xFF00FF |
               (((rgba >> 8) & 0xFF00FF) * _opacity) & 0xFF00FF00;

    unsigned char alpha = rgba >> 24;
    if(!alpha) {
        return;
    } else if(alpha == 0xFF) {
        fillOpaque(r, rgba);
        return;
    } else {
        // Pre mul
        rgba = (((rgba & 0xFF00FF) * alpha) >> 8) & 0xFF00FF |
               (((rgba & 0xFF00) * alpha) >> 8) & 0xFF00 |
               (rgba & 0xFF000000);
    }

    if(qt_region_is_rect(clipRegion)) {
        QRect fillRect = r.intersected(clipRegion.boundingRect());

        int width = fillRect.width();
        int height = fillRect.height();

        if(depth == Depth_16) {
            ushort *destBits = (ushort *)buffer;
            uint destStep = step;
            destBits += fillRect.y() * destStep + fillRect.x();

            for(int ii = 0; ii < height; ++ii) {
                q_blendroutines.blend_color_rgb16(destBits, rgba, width, destBits);
                destBits += destStep;
            }
        } else {
            uint *destBits = (uint *)buffer;
            uint destStep = step;
            destBits += fillRect.y() * destStep + fillRect.x();

            for(int ii = 0; ii < height; ++ii) {
                q_blendroutines.blend_color_rgb32(destBits, rgba, width, destBits);
                destBits += destStep;
            }
        }
    } else {
        QRect p;
        for(int ii = 0; ii < clipRects.count(); ++ii) {
            QRect fillRect = r.intersected(clipRects.at(ii));

            int width = fillRect.width();
            int height = fillRect.height();

            if(depth == Depth_16) {
                ushort *destBits = (ushort *)buffer;
                uint destStep = step;
                destBits += fillRect.y() * destStep + fillRect.x();

                for(int ii = 0; ii < height; ++ii) {
                    q_blendroutines.blend_color_rgb16(destBits, rgba, width, destBits);
                    destBits += destStep;
                }
            } else {
                uint *destBits = (uint *)buffer;
                uint destStep = step;
                destBits += fillRect.y() * destStep + fillRect.x();

                for(int ii = 0; ii < height; ++ii) {
                    q_blendroutines.blend_color_rgb32(destBits, rgba, width, destBits);
                    destBits += destStep;
                }
            }

        }
    }
}

void GfxPainter::drawImage(const QPoint &p, const QImage &i)
{
    drawImage(p.x(), p.y(), i);
}

GfxImageRef GfxPainter::imgRef(const QRect &r) const
{
    QRect imgRect = r & QRect(0, 0, width, height);
    if(imgRect.isEmpty())
        return GfxImageRef();

    if(depth == Depth_16) {
        return GfxImageRef(buffer + step * 2 * imgRect.y() + imgRect.x() * 2,
                      imgRect.width(), imgRect.height(),
                      step * 2, QImage::Format_RGB16);
    } else {
        return GfxImageRef(buffer + step * 4 * imgRect.y() + imgRect.x() * 4,
                      imgRect.width(), imgRect.height(),
                      step * 4, QImage::Format_RGB32);
    }

}

QImage GfxPainter::img(const QRect &r) const
{
    QRect imgRect = r & QRect(0, 0, width, height);
    if(imgRect.isEmpty())
        return QImage();

    if(depth == Depth_16) {
        return QImage(buffer + step * 2 * imgRect.y() + imgRect.x() * 2,
                      imgRect.width(), imgRect.height(),
                      step * 2, QImage::Format_RGB16);
    } else {
        return QImage(buffer + step * 4 * imgRect.y() + imgRect.x() * 4,
                      imgRect.width(), imgRect.height(),
                      step * 4, QImage::Format_RGB32);
    }

}

void GfxPainter::fillRectTransformed(const QMatrix &m, const QSize &s, const QColor &c, bool smooth)
{
    if(useQt) {
        painter()->setMatrix(m);
        if(smooth) {
            painter()->setRenderHint(QPainter::SmoothPixmapTransform);
            painter()->setRenderHint(QPainter::Antialiasing);
        }
        painter()->fillRect(0, 0, s.width(), s.height(), c);
        painter()->setMatrix(QMatrix());
        if(smooth) {
            painter()->setRenderHint(QPainter::SmoothPixmapTransform, false);
            painter()->setRenderHint(QPainter::Antialiasing, false);
        }
    } else {
        QRect cr = clipRegion.boundingRect();
        if(!_userClipRect.isEmpty())
            cr &= _userClipRect;
        if(cr.isEmpty())
            return;
        QMatrix m2;
        m2.translate(-cr.x(), -cr.y());
        m2 = m * m2;

        uint color = c.rgba();
        if(0xFF000000 != (color & 0xFF000000)) {
            uint alpha = ((color & 0xFF000000) >> 24) + 1;
            color = (color & 0xFF000000) |
                    ((((color & 0xFF00FF) * alpha) >> 8) & 0xFF00FF) |
                    ((((color & 0xFF00) * alpha) >> 8) & 0xFF00);

        }
        GfxImageRef img = imgRef(cr);
        if(smooth)
            def_transform_fill_bilinear(img, m2, s, color, _opacity);
        else
            def_transform_fill(img, m2, s, color, _opacity);
    }
}

void GfxPainter::drawImageTransformed(const QMatrix &m, const QImage &img, bool smooth)
{
    if(useQt) {
        painter()->setMatrix(m);
        if(smooth) {
            painter()->setRenderHint(QPainter::SmoothPixmapTransform);
            painter()->setRenderHint(QPainter::Antialiasing);
        }
        painter()->drawImage(0, 0, img);
        painter()->setMatrix(QMatrix());
        if(smooth) {
            painter()->setRenderHint(QPainter::SmoothPixmapTransform, false);
            painter()->setRenderHint(QPainter::Antialiasing, false);
        }
    } else {
        QRect cr = clipRegion.boundingRect();
        if(!_userClipRect.isEmpty())
            cr &= _userClipRect;
        if(cr.isEmpty())
            return;
        QMatrix m2;
        m2.translate(-cr.x(), -cr.y());
        m2 = m * m2;

        GfxImageRef out = imgRef(cr);
        if(smooth)
            def_transform_bilinear(out, img, m2, _opacity);
        else
            def_transform(out, img, m2, _opacity);
    }
}

