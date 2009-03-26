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

#include "def_transform.h"
#include "routines.h"
#include <QImage>
#include "gfximage.h"
#include <QDebug>
#include "def_blendhelper.h"

#ifdef QT_ARCH_ARMV5E
#define PLD(src) \
    asm volatile("pld [%0, #32]\n\t" \
            : : "r"(src));
#else
#define PLD(src)
#endif

static inline unsigned int byte_mul(unsigned int pixel, unsigned int alpha)
{
    return ((((pixel & 0xFF00FF) * alpha) >> 8) & 0xFF00FF) |
           ((((pixel & 0xFF00FF00) >> 8) * alpha) & 0xFF00FF00);
}

// transform_<in>_<out>
static inline bool transform_32_32(uchar *outpixel, const uchar *inbits,
                                   int height, int width, int step, int in_xx,
                                   int in_yy)
{
    in_xx = in_xx >> 16;
    in_yy = in_yy >> 16;

    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) {
        *((uint *)outpixel) = ((uint *)inbits)[step * in_yy + in_xx];
        return true;
    } else {
        return false;
    }
}

static inline bool transform_argb32p_32(uchar *outpixel, const uchar *inbits,
                                   int height, int width, int step, int in_xx,
                                   int in_yy)
{
    in_xx = in_xx >> 16;
    in_yy = in_yy >> 16;

    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) {
        argb32p_rgb32_inplace((uint *)outpixel, (uint *)inbits + step * in_yy + in_xx);
        return true;
    } else {
        return false;
    }
}

static inline bool transform_argb32p_32_op(uchar *outpixel,
                                           const uchar *inbits,
                                           int height, int width, int step,
                                           int in_xx, int in_yy, uchar op)
{
    in_xx = in_xx >> 16;
    in_yy = in_yy >> 16;

    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) {
        argb32p_rgb32_opacity_inplace((uint *)outpixel, (uint *)inbits + step * in_yy + in_xx, op);
        return true;
    } else {
        return false;
    }
}

static inline bool transform_16_32(uchar *outpixel, const uchar *inbits,
                                   int height, int width, int step, int in_xx,
                                   int in_yy)
{
    in_xx = in_xx >> 16;
    in_yy = in_yy >> 16;

    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) {
        *((uint *)outpixel) =
            qConvertRgb16To32(((ushort *)inbits)[step * in_yy + in_xx]);
        return true;
    } else {
        return false;
    }
}

static inline bool transform_16_32_op(uchar *outpixel, const uchar *inbits,
                                      int height, int width, int step, int in_xx,
                                      int in_yy, uchar op)
{
    in_xx = in_xx >> 16;
    in_yy = in_yy >> 16;

    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) {
        rgb16_rgb32_opacity_inplace((uint *)outpixel, (ushort *)inbits + step * in_yy + in_xx, op);
        return true;
    } else {
        return false;
    }
}

static inline bool transform_16_16(uchar *outpixel, const uchar *inbits,
                                   int height, int width, int step, int in_xx,
                                   int in_yy)
{
    in_xx = in_xx >> 16;
    in_yy = in_yy >> 16;

    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) {
        *((ushort *)outpixel) = ((ushort *)inbits)[step * in_yy + in_xx];
        return true;
    } else {
        return false;
    }
}

static inline bool transform_16_16_op(uchar *outpixel, const uchar *inbits,
                                      int height, int width, int step, int in_xx,
                                      int in_yy, uchar op)
{
    in_xx = in_xx >> 16;
    in_yy = in_yy >> 16;

    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) {
        rgb16_rgb16_opacity_inplace((ushort *)outpixel, (ushort *)inbits + step * in_yy + in_xx, op);
        return true;
    } else {
        return false;
    }
}

static inline bool transform_32_16(uchar *outpixel, const uchar *inbits,
                                   int height, int width, int step, int in_xx,
                                   int in_yy)
{
    in_xx = in_xx >> 16;
    in_yy = in_yy >> 16;

    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) {
        *((ushort *)outpixel) = qConvertRgb32To16(((uint *)inbits)[step * in_yy + in_xx]);
        return true;
    } else {
        return false;
    }
}

static inline bool transform_argb32p_16(uchar *outpixel, const uchar *inbits,
                                   int height, int width, int step, int in_xx,
                                   int in_yy)
{
    in_xx = in_xx >> 16;
    in_yy = in_yy >> 16;

    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) {
        argb32p_rgb16_inplace((ushort *)outpixel,
                              (uint *)inbits + step * in_yy + in_xx);
        return true;
    } else {
        return false;
    }
}

static inline bool transform_argb32p_16_op(uchar *outpixel, const uchar *inbits,
                                   int height, int width, int step, int in_xx,
                                   int in_yy, uchar op)
{
    in_xx = in_xx >> 16;
    in_yy = in_yy >> 16;

    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) {
        argb32p_rgb16_opacity_inplace((ushort *)outpixel,
                              (uint *)inbits + step * in_yy + in_xx, op);
        return true;
    } else {
        return false;
    }
}

#define TRANSFORM_BIFUNC_IMPL(name, in_type, out_type, inTo32, outTo32, convert32ToOut) \
static inline bool name(uchar *_outpixel, const uchar *_inbits,  \
                        int height, int width, int step,  \
                        int ain_xx, int ain_yy) \
{ \
    ain_xx -= 0x8000; \
    ain_yy -= 0x8000; \
    out_type *outpixel = (out_type *)_outpixel; \
    const in_type *inbits = (const in_type *)_inbits; \
 \
    int in_xx = ain_xx >> 16; \
    int in_yy = ain_yy >> 16; \
 \
    ain_xx &= 0xFFFF; \
    ain_yy &= 0xFFFF; \
 \
    unsigned int pixel_ul = 0; \
    unsigned int pixel_ur = 0; \
    unsigned int pixel_ll = 0; \
    unsigned int pixel_lr = 0; \
 \
    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) \
        pixel_ul = inTo32(inbits[step * in_yy  + in_xx]); \
 \
    in_yy++; \
    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) \
        pixel_ll = inTo32(inbits[step * in_yy  + in_xx]); \
 \
    in_xx++; \
    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) \
        pixel_lr = inTo32(inbits[step * in_yy  + in_xx]); \
 \
    in_yy--; \
    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) \
        pixel_ur = inTo32(inbits[step * in_yy  + in_xx]); \
 \
    if(!pixel_ul && !pixel_ur && !pixel_ll && !pixel_lr) { \
        return false; \
    } else { \
        unsigned int pixel_upper =  \
            byte_mul(pixel_ul, 256 - (ain_xx >> 8)) + \
            byte_mul(pixel_ur, ain_xx >> 8); \
        unsigned int pixel_lower =  \
            byte_mul(pixel_ll, 256 - (ain_xx >> 8)) + \
            byte_mul(pixel_lr, ain_xx >> 8); \
        unsigned int pixel =  \
            byte_mul(pixel_upper, 256 - (ain_yy >> 8)) + \
            byte_mul(pixel_lower, ain_yy >> 8); \
        if(!pixel_ul || !pixel_ur || !pixel_ll || !pixel_lr) { \
            pixel += byte_mul(outTo32(*outpixel), 0x100 - (pixel & 0xFF000000) >> 24); \
        }  \
 \
        pixel |= 0xFF000000; \
        *outpixel = convert32ToOut(pixel); \
        return true; \
    } \
}


#define TRANSFORM_BIFUNC_OP_IMPL(name, in_type, out_type, inTo32, blend32WithOut) \
static inline bool name(uchar *_outpixel, const uchar *_inbits,  \
                        int height, int width, int step,  \
                        int ain_xx, int ain_yy, uchar opacity) \
{ \
    ain_xx -= 0x8000; \
    ain_yy -= 0x8000; \
    out_type *outpixel = (out_type *)_outpixel; \
    const in_type *inbits = (const in_type *)_inbits; \
 \
    int in_xx = ain_xx >> 16; \
    int in_yy = ain_yy >> 16; \
 \
    ain_xx &= 0xFFFF; \
    ain_yy &= 0xFFFF; \
 \
    unsigned int pixel_ul = 0; \
    unsigned int pixel_ur = 0; \
    unsigned int pixel_ll = 0; \
    unsigned int pixel_lr = 0; \
 \
    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) \
        pixel_ul = inTo32(inbits[step * in_yy  + in_xx]); \
 \
    in_yy++; \
    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) \
        pixel_ll = inTo32(inbits[step * in_yy  + in_xx]); \
 \
    in_xx++; \
    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) \
        pixel_lr = inTo32(inbits[step * in_yy  + in_xx]); \
 \
    in_yy--; \
    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) \
        pixel_ur = inTo32(inbits[step * in_yy  + in_xx]); \
 \
    if(!pixel_ul && !pixel_ur && !pixel_ll && !pixel_lr) { \
    } else { \
        unsigned int pixel_upper =  \
            byte_mul(pixel_ul, 256 - (ain_xx >> 8)) + \
            byte_mul(pixel_ur, ain_xx >> 8); \
        unsigned int pixel_lower =  \
            byte_mul(pixel_ll, 256 - (ain_xx >> 8)) + \
            byte_mul(pixel_lr, ain_xx >> 8); \
        unsigned int pixel =  \
            byte_mul(pixel_upper, 256 - (ain_yy >> 8)) + \
            byte_mul(pixel_lower, ain_yy >> 8); \
        blend32WithOut(outpixel, &pixel, opacity); \
    } \
    return true; \
}

#define TRANSFORM_BIFUNC_PREMUL_IMPL(name, in_type, out_type, blend32WithOut) \
static inline bool name(uchar *_outpixel, const uchar *_inbits,  \
                        int height, int width, int step,  \
                        int ain_xx, int ain_yy) \
{ \
    ain_xx -= 0x8000; \
    ain_yy -= 0x8000; \
    out_type *outpixel = (out_type *)_outpixel; \
    const in_type *inbits = (const in_type *)_inbits; \
 \
    int in_xx = ain_xx >> 16; \
    int in_yy = ain_yy >> 16; \
 \
    ain_xx &= 0xFFFF; \
    ain_yy &= 0xFFFF; \
 \
    unsigned int pixel_ul = 0; \
    unsigned int pixel_ur = 0; \
    unsigned int pixel_ll = 0; \
    unsigned int pixel_lr = 0; \
 \
    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) \
        pixel_ul = (inbits[step * in_yy  + in_xx]); \
 \
    in_yy++; \
    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) \
        pixel_ll = (inbits[step * in_yy  + in_xx]); \
 \
    in_xx++; \
    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) \
        pixel_lr = (inbits[step * in_yy  + in_xx]); \
 \
    in_yy--; \
    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) \
        pixel_ur = (inbits[step * in_yy  + in_xx]); \
 \
    unsigned int pixel_upper =  \
        byte_mul(pixel_ul, 256 - (ain_xx >> 8)) + \
        byte_mul(pixel_ur, ain_xx >> 8); \
    unsigned int pixel_lower =  \
        byte_mul(pixel_ll, 256 - (ain_xx >> 8)) + \
        byte_mul(pixel_lr, ain_xx >> 8); \
    unsigned int pixel =  \
        byte_mul(pixel_upper, 256 - (ain_yy >> 8)) + \
        byte_mul(pixel_lower, ain_yy >> 8); \
    blend32WithOut(outpixel, &pixel); \
    return true; \
}

TRANSFORM_BIFUNC_PREMUL_IMPL(transform_argb32p_32_bi, uint, uint, argb32p_rgb32_inplace);
TRANSFORM_BIFUNC_PREMUL_IMPL(transform_argb32p_16_bi, uint, ushort, argb32p_rgb16_inplace);

TRANSFORM_BIFUNC_IMPL(transform_32_32_bi, uint, uint, , , );
TRANSFORM_BIFUNC_IMPL(transform_16_16_bi, ushort, ushort, qConvertRgb16To32, qConvertRgb16To32, qConvertRgb32To16);
TRANSFORM_BIFUNC_IMPL(transform_32_16_bi, uint, ushort, ,qConvertRgb16To32, qConvertRgb32To16);
TRANSFORM_BIFUNC_IMPL(transform_16_32_bi, ushort, uint, qConvertRgb16To32, , );

TRANSFORM_BIFUNC_OP_IMPL(transform_16_16_bi_op, ushort, ushort, qConvertRgb16To32, rgb32_rgb16_opacity_inplace);
TRANSFORM_BIFUNC_OP_IMPL(transform_32_16_bi_op, uint, ushort, ,rgb32_rgb16_opacity_inplace);
TRANSFORM_BIFUNC_OP_IMPL(transform_16_32_bi_op, ushort, uint, qConvertRgb16To32,argb32p_rgb32_opacity_inplace);
TRANSFORM_BIFUNC_OP_IMPL(transform_argb32p_32_bi_op, uint, uint, ,argb32p_rgb32_opacity_inplace);
TRANSFORM_BIFUNC_OP_IMPL(transform_argb32p_16_bi_op, uint, ushort, ,argb32p_rgb16_opacity_inplace);

#define TRANSFORM_IMPL_OP(name, func, in_type, out_type) \
static void name(GfxImageRef &out, const QImage &in, const QMatrix &m, uchar op) \
{ \
    QMatrix inv = m.inverted(); \
 \
    int inv_m11 = int(inv.m11() * qreal(0x10000)); \
    int inv_m12 = int(inv.m12() * qreal(0x10000)); \
    int inv_m21 = int(inv.m21() * qreal(0x10000)); \
    int inv_m22 = int(inv.m22() * qreal(0x10000)); \
    int inv_dx = int(inv.dx() * qreal(0x10000)); \
    int inv_dy = int(inv.dy() * qreal(0x10000)); \
 \
    const uchar *inbits = (const uchar *)in.bits(); \
    int instep = in.bytesPerLine() / sizeof(in_type); \
    int width = in.width(); \
    int height = in.height(); \
 \
    out_type *outbits_orig = (out_type *)out.bits(); \
    int outstep = out.bytesPerLine() / sizeof(out_type); \
 \
    QPoint ps[4]; \
    ps[0] = QPoint(0, 0) * m; \
    ps[1] = QPoint(0, height) * m; \
    ps[2] = QPoint(width, 0) * m; \
    ps[3] = QPoint(width, height) * m; \
 \
    int topLeft_x = ps[0].x(); \
    int topLeft_y = ps[0].y(); \
    int botRight_x = ps[0].x(); \
    int botRight_y = ps[0].y(); \
 \
    for(int ii = 1; ii < 4; ++ii) { \
        topLeft_x = qMin(topLeft_x, ps[ii].x()); \
        topLeft_y = qMin(topLeft_y, ps[ii].y()); \
        botRight_x = qMax(botRight_x, ps[ii].x()); \
        botRight_y = qMax(botRight_y, ps[ii].y()); \
    } \
 \
    QRect outRect(QPoint(topLeft_x, topLeft_y),  \
            QPoint(botRight_x, botRight_y)); \
    outRect &= out.rect(); \
 \
    for(int yy_base = outRect.top(); yy_base <= outRect.bottom(); yy_base += 64) { \
        for(int xx_base = outRect.left(); xx_base <= outRect.right(); xx_base += 32) { \
            for(int yy = yy_base; yy <= outRect.bottom() && yy < (yy_base + 64); ++yy) { \
 \
                out_type *outbits = outbits_orig + yy * outstep + xx_base; \
 \
                bool seen = false; \
                for(int xx = xx_base; xx < (xx_base + 32) && xx <= outRect.right(); ++xx) { \
 \
                    int ain_xx = xx * inv_m11 + yy * inv_m21 + inv_dx; \
                    int ain_yy = xx * inv_m12 + yy * inv_m22 + inv_dy; \
 \
                    if(!func((uchar *)outbits++, (uchar *)inbits, \
                             height, width, instep, ain_xx, ain_yy, op)) { \
                        if(seen) \
                            break; \
                    } else { \
                        seen = true; \
                    } \
                } \
 \
            } \
        } \
    } \
}

#define TRANSFORM_IMPL(name, func, in_type, out_type) \
void name(GfxImageRef &out, const QImage &in, const QMatrix &m) \
{ \
    QMatrix inv = m.inverted(); \
 \
    int inv_m11 = int(inv.m11() * qreal(0x10000)); \
    int inv_m12 = int(inv.m12() * qreal(0x10000)); \
    int inv_m21 = int(inv.m21() * qreal(0x10000)); \
    int inv_m22 = int(inv.m22() * qreal(0x10000)); \
    int inv_dx = int(inv.dx() * qreal(0x10000)); \
    int inv_dy = int(inv.dy() * qreal(0x10000)); \
 \
    const uchar *inbits = (const uchar *)in.bits(); \
    int instep = in.bytesPerLine() / sizeof(in_type); \
    int width = in.width(); \
    int height = in.height(); \
 \
    out_type *outbits_orig = (out_type *)out.bits(); \
    int outstep = out.bytesPerLine() / sizeof(out_type); \
 \
    QPoint ps[4]; \
    ps[0] = QPoint(0, 0) * m; \
    ps[1] = QPoint(0, height) * m; \
    ps[2] = QPoint(width, 0) * m; \
    ps[3] = QPoint(width, height) * m; \
 \
    int topLeft_x = ps[0].x(); \
    int topLeft_y = ps[0].y(); \
    int botRight_x = ps[0].x(); \
    int botRight_y = ps[0].y(); \
 \
    for(int ii = 1; ii < 4; ++ii) { \
        topLeft_x = qMin(topLeft_x, ps[ii].x()); \
        topLeft_y = qMin(topLeft_y, ps[ii].y()); \
        botRight_x = qMax(botRight_x, ps[ii].x()); \
        botRight_y = qMax(botRight_y, ps[ii].y()); \
    } \
 \
    QRect outRect(QPoint(topLeft_x, topLeft_y),  \
            QPoint(botRight_x, botRight_y)); \
    outRect &= out.rect(); \
 \
    for(int yy_base = outRect.top(); yy_base <= outRect.bottom(); yy_base += 64) { \
        for(int xx_base = outRect.left(); xx_base <= outRect.right(); xx_base += 32) { \
            for(int yy = yy_base; yy <= outRect.bottom() && yy < (yy_base + 64); ++yy) { \
 \
                out_type *outbits = outbits_orig + yy * outstep + xx_base; \
 \
                bool seen = false; \
                for(int xx = xx_base; xx < (xx_base + 32) && xx <= outRect.right(); ++xx) { \
 \
                    int ain_xx = xx * inv_m11 + yy * inv_m21 + inv_dx; \
                    int ain_yy = xx * inv_m12 + yy * inv_m22 + inv_dy; \
 \
                    if(!func((uchar *)outbits++, (uchar *)inbits, \
                             height, width, instep, ain_xx, ain_yy)) { \
                        if(seen) \
                            break; \
                    } else { \
                        seen = true; \
                    } \
                } \
 \
            } \
        } \
    } \
}

TRANSFORM_IMPL(def_transform_16_16, transform_16_16, ushort, ushort);
TRANSFORM_IMPL(def_transform_16_32, transform_16_32, ushort, uint);
TRANSFORM_IMPL(def_transform_32_32, transform_32_32, uint, uint);
TRANSFORM_IMPL(def_transform_32_16, transform_32_16, uint, ushort);
TRANSFORM_IMPL(def_transform_argb32p_16, transform_argb32p_16, uint, ushort);
TRANSFORM_IMPL(def_transform_argb32p_32, transform_argb32p_32, uint, uint);

TRANSFORM_IMPL_OP(def_transform_16_16_op, transform_16_16_op, ushort, ushort);
TRANSFORM_IMPL_OP(def_transform_16_32_op, transform_16_32_op, ushort, uint);
TRANSFORM_IMPL_OP(def_transform_argb32p_16_op, transform_argb32p_16_op, uint, ushort);
TRANSFORM_IMPL_OP(def_transform_argb32p_32_op, transform_argb32p_32_op, uint, uint);

TRANSFORM_IMPL(def_transform_32_32_bi, transform_32_32_bi, uint, uint);
TRANSFORM_IMPL(def_transform_16_16_bi, transform_16_16_bi, ushort, ushort);
TRANSFORM_IMPL(def_transform_32_16_bi, transform_32_16_bi, uint, ushort);
TRANSFORM_IMPL(def_transform_16_32_bi, transform_16_32_bi, ushort, uint);
TRANSFORM_IMPL(def_transform_argb32p_16_bi, transform_argb32p_16_bi, uint, ushort);
TRANSFORM_IMPL(def_transform_argb32p_32_bi, transform_argb32p_32_bi, uint, uint);

TRANSFORM_IMPL_OP(def_transform_16_16_bi_op, transform_16_16_bi_op, ushort, ushort);
TRANSFORM_IMPL_OP(def_transform_32_16_bi_op, transform_32_16_bi_op, uint, ushort);
TRANSFORM_IMPL_OP(def_transform_16_32_bi_op, transform_16_32_bi_op, ushort, uint);
TRANSFORM_IMPL_OP(def_transform_argb32p_16_bi_op, transform_argb32p_16_bi_op, uint, ushort);
TRANSFORM_IMPL_OP(def_transform_argb32p_32_bi_op, transform_argb32p_32_bi_op, uint, uint);

typedef void (*TransformFunc)(GfxImageRef &out, const QImage &in, const QMatrix &m);
typedef void (*TransformFuncOpacity)(GfxImageRef &out, const QImage &in, const QMatrix &m, uchar);

enum ImageType { RGB16 = 0, RGB32 = 1, ARGB32p = 2, None = 3 };
static const TransformFunc transformFuncs[RGB32 + 1][ARGB32p + 1] = {
    {
        def_transform_16_16,
        def_transform_32_16,
        def_transform_argb32p_16
    },
    {
        def_transform_16_32,
        def_transform_32_32,
        def_transform_argb32p_32
    }
};

static const TransformFuncOpacity transformOpFuncs[RGB32 + 1][ARGB32p + 1] = {
    {
        def_transform_16_16_op,
        def_transform_argb32p_16_op,
        def_transform_argb32p_16_op
    },
    {
        def_transform_16_32_op,
        def_transform_argb32p_32_op,
        def_transform_argb32p_32_op
    }
};

static const TransformFunc transformFuncsBi[RGB32 + 1][ARGB32p + 1] = {
    {
        def_transform_16_16_bi,
        def_transform_32_16_bi,
        def_transform_argb32p_16_bi
    },
    {
        def_transform_16_32_bi,
        def_transform_32_32_bi,
        def_transform_argb32p_32_bi
    }
};

static const TransformFuncOpacity transformOpFuncsBi[RGB32 + 1][ARGB32p + 1] = {
    {
        def_transform_16_16_bi_op,
        def_transform_32_16_bi_op,
        def_transform_argb32p_16_bi_op
    },
    {
        def_transform_16_32_bi_op,
        def_transform_argb32p_32_bi_op,
        def_transform_argb32p_32_bi_op
    }
};


static bool isScale(const QMatrix &m)
{
    return m.m11() == m.m22() && m.m11() > 0 && m.m12() == 0. && m.m21() == 0.;
}

#define TRANSFORM_SCALE_IMPL(name, out_type, in_type, blend) \
void name(GfxImageRef &out, const GfxImageRef &in, const QMatrix &m) \
{ \
    QRect inrect(0, 0, in.width(), in.height()); \
    inrect = m.mapRect(inrect); \
    if ( !inrect.intersects(QRect(0, 0, out.width(), out.height())) ) \
        return;\
\
    int widthratio = int(0x10000 * qreal(in.width()) / qreal(inrect.width())); \
    int heightratio = int(0x10000 * qreal(in.width()) / qreal(inrect.width())); \
\
\
    int in_start_x = 0; \
    int in_start_y = 0; \
    if(inrect.left() < 0) { \
        in_start_x = (-inrect.left() * widthratio); \
        inrect.setLeft(0); \
    } \
\
    if(inrect.top() < 0) { \
        in_start_y = (-inrect.top() * heightratio); \
        inrect.setTop(0); \
    } \
\
    inrect &= QRect(0, 0, out.width(), out.height()); \
\
    int iny = in_start_y; \
    for(int yy = 0; yy < inrect.height(); ++yy) { \
        out_type *outbits = ((out_type *)out.bits()) + (yy + inrect.top()) * (out.bytesPerLine() / sizeof(out_type)) + inrect.x(); \
        in_type *inbits = ((in_type *)in.bits()) + (iny >> 16) * (in.bytesPerLine() / sizeof(in_type)); \
        int inx = in_start_x; \
        if(inrect.width() > 32) { \
            for(int ii = 0; ii < inrect.width(); ++ii) { \
                PLD(outbits); \
                PLD(inbits + (inx >> 16)); \
                blend(outbits++, inbits + (inx >> 16)); \
                inx += widthratio; \
            } \
        } else { \
            for(int ii = 0; ii < inrect.width(); ++ii) { \
                blend(outbits++, inbits + (inx >> 16)); \
                inx += widthratio; \
            } \
        } \
        iny += heightratio; \
    } \
} \

#define TRANSFORM_SCALE_OP_IMPL(name, out_type, in_type, blend) \
void name(GfxImageRef &out, const GfxImageRef &in, const QMatrix &m, uchar op) \
{ \
    QRect inrect(0, 0, in.width(), in.height()); \
    inrect = m.mapRect(inrect); \
    if ( !inrect.intersects(QRect(0, 0, out.width(), out.height())) ) \
        return;\
\
    int widthratio = int(0x10000 * qreal(in.width()) / qreal(inrect.width())); \
    int heightratio = int(0x10000 * qreal(in.width()) / qreal(inrect.width())); \
\
\
    int in_start_x = 0; \
    int in_start_y = 0; \
    if(inrect.left() < 0) { \
        in_start_x = (-inrect.left() * widthratio); \
        inrect.setLeft(0); \
    } \
\
    if(inrect.top() < 0) { \
        in_start_y = (-inrect.top() * heightratio); \
        inrect.setTop(0); \
    } \
\
    inrect &= QRect(0, 0, out.width(), out.height()); \
\
    int iny = in_start_y; \
    for(int yy = 0; yy < inrect.height(); ++yy) { \
        out_type *outbits = ((out_type *)out.bits()) + (yy + inrect.top()) * (out.bytesPerLine() / sizeof(out_type)) + inrect.x(); \
        in_type *inbits = ((in_type *)in.bits()) + (iny >> 16) * (in.bytesPerLine() / sizeof(in_type)); \
        int inx = in_start_x; \
        if(inrect.width() > 32) { \
            for(int ii = 0; ii < inrect.width(); ++ii) { \
                PLD(outbits); \
                PLD(inbits + (inx >> 16)); \
                blend(outbits++, inbits + (inx >> 16), op); \
                inx += widthratio; \
            } \
        } else { \
            for(int ii = 0; ii < inrect.width(); ++ii) { \
                blend(outbits++, inbits + (inx >> 16), op); \
                inx += widthratio; \
            } \
        } \
        iny += heightratio; \
    } \
} \


static inline void assign_rgb16_rgb16(ushort *out, ushort *in)
{
    *out = *in;
}
static inline void assign_rgb32_rgb32(uint *out, uint *in)
{
    *out = *in;
}
static inline void assign_rgb32_rgb16(ushort *out, uint *in)
{
    *out = qConvertRgb32To16(*in);
}
static inline void assign_rgb16_rgb32(uint *out, ushort *in)
{
    *out = qConvertRgb16To32(*in);
}

TRANSFORM_SCALE_IMPL(def_transform_scale_rgb16_rgb16, ushort, ushort, assign_rgb16_rgb16);
TRANSFORM_SCALE_IMPL(def_transform_scale_rgb32_rgb32, uint, uint, assign_rgb32_rgb32);
TRANSFORM_SCALE_IMPL(def_transform_scale_rgb16_rgb32, uint, ushort, assign_rgb16_rgb32);
TRANSFORM_SCALE_IMPL(def_transform_scale_rgb32_rgb16, ushort, uint, assign_rgb32_rgb16);
TRANSFORM_SCALE_IMPL(def_transform_scale_argb32p_rgb16, ushort, uint, argb32p_rgb16_inplace);
TRANSFORM_SCALE_IMPL(def_transform_scale_argb32p_rgb32, uint, uint, argb32p_rgb32_inplace);

TRANSFORM_SCALE_OP_IMPL(def_transform_scale_rgb16_rgb16_op, ushort, ushort, rgb16_rgb16_opacity_inplace);
TRANSFORM_SCALE_OP_IMPL(def_transform_scale_rgb32_rgb32_op, uint, uint, argb32p_rgb32_opacity_inplace);
TRANSFORM_SCALE_OP_IMPL(def_transform_scale_rgb16_rgb32_op, uint, ushort, rgb16_rgb32_opacity_inplace);
TRANSFORM_SCALE_OP_IMPL(def_transform_scale_rgb32_rgb16_op, ushort, uint, rgb32_rgb16_opacity_inplace);
TRANSFORM_SCALE_OP_IMPL(def_transform_scale_argb32p_rgb16_op, ushort, uint, argb32p_rgb16_opacity_inplace);
TRANSFORM_SCALE_OP_IMPL(def_transform_scale_argb32p_rgb32_op, uint, uint, argb32p_rgb32_opacity_inplace);

typedef void (*ScaleFunc)(GfxImageRef &, const GfxImageRef &, const QMatrix &);
ScaleFunc scaleFuncs[RGB32 + 1][ARGB32p + 1] = {
    {
        def_transform_scale_rgb16_rgb16,
        def_transform_scale_rgb32_rgb16,
        def_transform_scale_argb32p_rgb16
    },
    {
        def_transform_scale_rgb16_rgb32,
        def_transform_scale_rgb32_rgb32,
        def_transform_scale_argb32p_rgb32
    }
};

typedef void (*ScaleOpFunc)(GfxImageRef &, const GfxImageRef &, const QMatrix &, uchar);
ScaleOpFunc scaleOpFuncs[RGB32 + 1][ARGB32p + 1] = {
    {
        def_transform_scale_rgb16_rgb16_op,
        def_transform_scale_rgb32_rgb16_op,
        def_transform_scale_argb32p_rgb16_op
    },
    {
        def_transform_scale_rgb16_rgb32_op,
        def_transform_scale_rgb32_rgb32_op,
        def_transform_scale_argb32p_rgb32_op
    }
};

void def_transform(GfxImageRef &out, const QImage &in, const QMatrix &m, uchar op)
{
    ImageType outType = None;
    ImageType inType = None;
    switch(out.format()) {
        case QImage::Format_RGB16:
            outType = RGB16;
            break;
        case QImage::Format_RGB32:
            outType = RGB32;
            break;
        default:
            break;
    }
    switch(in.format()) {
        case QImage::Format_RGB16:
            inType = RGB16;
            break;
        case QImage::Format_RGB32:
            inType = RGB32;
            break;
        case QImage::Format_ARGB32_Premultiplied:
            inType = ARGB32p;
            break;
        default:
            break;
    }

    if(outType == None || inType == None) {
            qWarning() << "GfxPainter: Cannot transform" << in.format()
                       << "/" << out.format();
            return;
    }

    if(isScale(m)) {
        if(op == 0xFF)
            scaleFuncs[outType][inType](out, in, m);
        else
            scaleOpFuncs[outType][inType](out, in, m, op);
    } else {
        if(op == 0xFF)
            transformFuncs[outType][inType](out, in, m);
        else
            transformOpFuncs[outType][inType](out, in, m, op);
    }
}

void def_transform_bilinear(GfxImageRef &out, const QImage &in, const QMatrix &m, uchar op)
{
    ImageType outType = None;
    ImageType inType = None;
    switch(out.format()) {
        case QImage::Format_RGB16:
            outType = RGB16;
            break;
        case QImage::Format_RGB32:
            outType = RGB32;
            break;
        default:
            break;
    }
    switch(in.format()) {
        case QImage::Format_RGB16:
            inType = RGB16;
            break;
        case QImage::Format_RGB32:
            inType = RGB32;
            break;
        case QImage::Format_ARGB32_Premultiplied:
            inType = ARGB32p;
            break;
        default:
            break;
    }

    if(outType == None || inType == None) {
            qWarning() << "GfxPainter: Cannot bilinearly transform" << in.format()
                       << "/" << out.format();
            return;
    }

    if(op == 0xFF)
        transformFuncsBi[outType][inType](out, in, m);
    else
        transformOpFuncsBi[outType][inType](out, in, m, op);
}

#define TRANSFORM_FILL_IMPL(name, out_type, out_bit_size, inToOut, blendWithOut) \
static void name(GfxImageRef &out, const QMatrix &m, const QSize &s, \
                 unsigned int color) \
{ \
    QMatrix inv = m.inverted(); \
 \
    int xxo_min = int(qreal(0x10000) * (0 - inv.dx()) / inv.m11());\
    int xxo_max = int(qreal(0x10000) * (s.width() - inv.dx()) / inv.m11());\
    int xxo_adj = int(qreal(0x10000) * inv.m21() / inv.m11());\
\
    int yxo_min = int(qreal(0x10000) * (0 - inv.dy()) / inv.m12());\
    int yxo_max = int(qreal(0x10000) * (s.height() - inv.dy()) / inv.m12());\
    int yxo_adj = int(qreal(0x10000) * inv.m22() / inv.m12());\
\
    QRect r(QPoint(0, 0), s);\
    r = m.mapRect(r);\
    r &= out.rect(); \
\
    xxo_min -= r.top() * xxo_adj;\
    xxo_max -= r.top() * xxo_adj;\
    yxo_min -= r.top() * yxo_adj;\
    yxo_max -= r.top() * yxo_adj;\
    if(xxo_min > xxo_max) qSwap(xxo_min, xxo_max);\
    if(yxo_min > yxo_max) qSwap(yxo_min, yxo_max);\
\
    out_type *outbits = (out_type *)out.bits(); \
    int outstep = out.bytesPerLine() / sizeof(out_type);\
    outbits += r.top() * outstep;\
    int width = out.width() - 1; \
\
    if((color & 0xFF000000) == 0xFF000000) { \
        out_type out_color = inToOut(color);\
                             \
        for(int yy = r.top(); yy <= r.bottom(); ++yy) {\
    \
            int x_start = qMax(0, qMax(xxo_min, yxo_min) >> 16);\
            int x_end = qMin(width, qMin(xxo_max, yxo_max) >> 16);\
    \
            if(x_start <= x_end)\
                q_memoryroutines.memset_##out_bit_size(outbits + x_start, out_color, x_end - x_start + 1);\
    \
            outbits+=outstep;\
    \
            xxo_min -= xxo_adj;\
            xxo_max -= xxo_adj;\
            yxo_min -= yxo_adj;\
            yxo_max -= yxo_adj;\
        }\
    } else { \
        for(int yy = r.top(); yy <= r.bottom(); ++yy) {\
            \
            int x_start = qMax(0, qMax(xxo_min, yxo_min) >> 16);\
            int x_end = qMin(width, qMin(xxo_max, yxo_max) >> 16);\
    \
            if(x_start <= x_end)\
                blendWithOut(outbits + x_start, color, x_end - x_start + 1, outbits + x_start); \
    \
            outbits+=outstep;\
    \
            xxo_min -= xxo_adj;\
            xxo_max -= xxo_adj;\
            yxo_min -= yxo_adj;\
            yxo_max -= yxo_adj;\
        }\
    } \
}

#define TRANSFORM_BIFUNC_FILL_IMPL(name, out_type, blend32WithOut) \
static inline void name(uchar *_outpixel, int height, int width, \
                        int ain_xx, int ain_yy, uint color) \
{ \
    ain_xx -= 0x8000; \
    ain_yy -= 0x8000; \
    out_type *outpixel = (out_type *)_outpixel; \
 \
    int in_xx = ain_xx >> 16; \
    int in_yy = ain_yy >> 16; \
 \
    unsigned int pixel_ul = 0; \
    unsigned int pixel_ur = 0; \
    unsigned int pixel_ll = 0; \
    unsigned int pixel_lr = 0; \
 \
    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) \
        pixel_ul = 1; \
 \
    in_yy++; \
    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) \
        pixel_ll = 1; \
 \
    in_xx++; \
    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) \
        pixel_lr = 1; \
 \
    in_yy--; \
    if(in_xx >= 0 && in_yy >= 0 && in_xx < width && in_yy < height) \
        pixel_ur = 1; \
 \
    if(pixel_ul && pixel_ur && pixel_ll && pixel_lr) { \
        blend32WithOut(outpixel, &color); \
    } else { \
        ain_xx &= 0xFFFF; \
        ain_yy &= 0xFFFF; \
        ain_xx >>= 8; \
        ain_yy >>= 8; \
\
        unsigned int pixel_upper = (256 - ain_xx) * pixel_ul + \
                                   ain_xx * pixel_ur; \
        unsigned int pixel_lower = (256 - ain_xx) * pixel_ll + \
                                   ain_xx * pixel_lr; \
        unsigned int pixel = pixel_upper * (256 - ain_yy) + \
                                   pixel_lower * ain_yy; \
        unsigned int out = byte_mul(color, pixel >> 8);\
        blend32WithOut(outpixel, &out); \
    } \
}

#define TRANSFORM_FILL_BI_IMPL(name, out_type, bifunc, inToOut, out_bit_size, blendWithOut) \
static void name(GfxImageRef &out, const QMatrix &m, const QSize &s, \
                 unsigned int color) \
{ \
    QMatrix inv = m.inverted(); \
 \
    int inv_m11 = int(inv.m11() * qreal(0x10000)); \
    int inv_m12 = int(inv.m12() * qreal(0x10000)); \
    int inv_m21 = int(inv.m21() * qreal(0x10000)); \
    int inv_m22 = int(inv.m22() * qreal(0x10000)); \
    int inv_dx = int(inv.dx() * qreal(0x10000)); \
    int inv_dy = int(inv.dy() * qreal(0x10000)); \
\
    int xxo_min = int(qreal(0x10000) * (0 - inv.dx()) / inv.m11());\
    int xxo_max = int(qreal(0x10000) * (s.width() - inv.dx()) / inv.m11());\
    int xxo_adj = int(qreal(0x10000) * inv.m21() / inv.m11());\
    int xxo_1adj = int(qreal(0x10000) / inv.m11()); \
\
    int yxo_min = int(qreal(0x10000) * (0 - inv.dy()) / inv.m12());\
    int yxo_max = int(qreal(0x10000) * (s.height() - inv.dy()) / inv.m12());\
    int yxo_adj = int(qreal(0x10000) * inv.m22() / inv.m12());\
    int yxo_1adj = int(qreal(0x10000) / inv.m12()); \
\
    QRect r(QPoint(0, 0), s);\
    r = m.mapRect(r);\
    r &= out.rect(); \
\
    xxo_min -= r.top() * xxo_adj;\
    xxo_max -= r.top() * xxo_adj;\
    yxo_min -= r.top() * yxo_adj;\
    yxo_max -= r.top() * yxo_adj;\
    if(xxo_min > xxo_max) { qSwap(xxo_min, xxo_max); xxo_1adj *= -1; }\
    if(yxo_min > yxo_max) { qSwap(yxo_min, yxo_max); yxo_1adj *= -1; }\
    int xxo_1min = xxo_min + xxo_1adj; \
    int xxo_1max = xxo_max - xxo_1adj; \
    int yxo_1min = yxo_min + yxo_1adj; \
    int yxo_1max = yxo_max - yxo_1adj; \
    int height = s.height(); \
    int width = s.width(); \
    int iwidth = out.width() - 1; \
\
    out_type *outbits = (out_type *)out.bits(); \
    int outstep = out.bytesPerLine() / sizeof(out_type);\
    outbits += r.top() * outstep;\
\
    out_type out_color = 0; \
    bool opaque = (color & 0xFF000000) == 0xFF000000; \
    if(opaque) out_color = inToOut(color); \
    for(int yy = r.top(); yy <= r.bottom(); ++yy) {\
    \
        int x_start = qMax(0, qMax(xxo_min, yxo_min) >> 16);\
        int x_end = qMin(iwidth, (qMin(xxo_max, yxo_max) + 0xFFFF) >> 16);\
        int x_inner_start = qMax(0, qMax(xxo_1min, yxo_1min) >> 16);\
        int x_inner_end = qMin(iwidth, (qMin(xxo_1max, yxo_1max) + 0xFFFF) >> 16);\
    \
        for(int xx = x_start; xx < x_inner_start; ++xx) { \
            int ain_xx = xx * inv_m11 + yy * inv_m21 + inv_dx; \
            int ain_yy = xx * inv_m12 + yy * inv_m22 + inv_dy; \
            bifunc((uchar *)(outbits + xx), height, width, ain_xx, ain_yy, color); \
        } \
        if(x_inner_start <= x_inner_end) { \
            if(opaque) { \
                q_memoryroutines.memset_##out_bit_size(outbits + x_inner_start, out_color, x_inner_end - x_inner_start + 1); \
            } else { \
                blendWithOut(outbits + x_inner_start, color, x_inner_end - x_inner_start + 1, outbits + x_inner_start); \
            } \
        } \
        for(int xx = x_inner_end + 1; xx <= x_end; ++xx) { \
            int ain_xx = xx * inv_m11 + yy * inv_m21 + inv_dx; \
            int ain_yy = xx * inv_m12 + yy * inv_m22 + inv_dy; \
            bifunc((uchar *)(outbits + xx), height, width, ain_xx, ain_yy, color); \
        } \
        outbits+=outstep;\
        \
        xxo_min -= xxo_adj;\
        xxo_max -= xxo_adj;\
        yxo_min -= yxo_adj;\
        yxo_max -= yxo_adj;\
        xxo_1min -= xxo_adj;\
        xxo_1max -= xxo_adj;\
        yxo_1min -= yxo_adj;\
        yxo_1max -= yxo_adj;\
    }  \
}



TRANSFORM_FILL_IMPL(def_transform_fill_16, ushort, 16, qConvertRgb32To16, q_blendroutines.blend_color_rgb16);
TRANSFORM_FILL_IMPL(def_transform_fill_32, uint, 32, , q_blendroutines.blend_color_rgb32);

typedef void (*TransformFillFunc)(GfxImageRef &out, const QMatrix &m, const QSize &s, unsigned int color);

static const TransformFillFunc transformFillFuncs[RGB32 + 1] = {
    def_transform_fill_16,
    def_transform_fill_32
};

void def_transform_fill(GfxImageRef &out, const QMatrix &m,
                        const QSize &s, unsigned int color, unsigned char op)
{
    ImageType outType = None;
    switch(out.format()) {
        case QImage::Format_RGB16:
            outType = RGB16;
            break;
        case QImage::Format_RGB32:
            outType = RGB32;
            break;
        default:
            break;
    }

    if(outType == None) {
        qWarning() << "GfxPainter: Cannot fill" << out.format();
        return;
    }

    color = premul(color, op);
    transformFillFuncs[outType](out, m, s, color);
}

TRANSFORM_BIFUNC_FILL_IMPL(transform_color_16_bi, ushort, argb32p_rgb16_inplace);
TRANSFORM_BIFUNC_FILL_IMPL(transform_color_32_bi, uint, argb32p_rgb32_inplace);
TRANSFORM_FILL_BI_IMPL(def_transform_fill_16_bi, ushort, transform_color_16_bi,qConvertRgb32To16,16,q_blendroutines.blend_color_rgb16);
TRANSFORM_FILL_BI_IMPL(def_transform_fill_32_bi, uint, transform_color_32_bi,,32,q_blendroutines.blend_color_rgb32);

static const TransformFillFunc transformFillBiFuncs[RGB32 + 1] = {
    def_transform_fill_16_bi,
    def_transform_fill_32_bi
};

void def_transform_fill_bilinear(GfxImageRef &out, const QMatrix &m,
                                 const QSize &s, unsigned int color, unsigned char op)
{
    ImageType outType = None;
    switch(out.format()) {
        case QImage::Format_RGB16:
            outType = RGB16;
            break;
        case QImage::Format_RGB32:
            outType = RGB32;
            break;
        default:
            break;
    }

    if(outType == None) {
        qWarning() << "GfxPainter: Cannot fill" << out.format();
        return;
    }

    color = premul(color, op);
    transformFillBiFuncs[outType](out, m, s, color);
}

