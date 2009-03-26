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
#include "cameraformatconverter.h"
#include <QCameraDevice>
/*
    Create a format converter
*/

using namespace QtopiaCamera;
CameraFormatConverter* CameraFormatConverter::createFormatConverter(unsigned int format, int width, int height)
{

    switch (format)
    {
    case RGB24:
        return new NullConverter;
    case YUYV:
    case UYVY:
        return new YUVConverter(format,width, height);
    case SBGGR8:
        return new BayerConverter(width, height);
       break;
    default:
        break;
    }

    return new NullConverter;
}

void CameraFormatConverter::releaseFormatConverter(CameraFormatConverter* converter)
{
    delete converter;
}


// Null Converter
unsigned char* NullConverter::convert(unsigned char* src)
{
    return src;
}

QList<unsigned int> CameraFormatConverter::supportedFormats()
{
    QList<unsigned int> list;
    list << YUYV << UYVY <<  SBGGR8;
    return list;
}

// YUV
YUVConverter::YUVConverter(unsigned int type,int width, int height):
    m_type(type),
    m_width(width),
    m_height(height)
{
    m_buf = new unsigned char[width * height * 2];

    if (m_type == YUYV)
    {

        m_y1i=0;
        m_ui =1;
        m_y2i=2;
        m_vi =3;

    }
    else if (m_type == UYVY)
    {
        m_ui =0;
        m_y1i=1;
        m_vi =2;
        m_y2i=3;


    }

}

YUVConverter::~YUVConverter()
{
    delete [] m_buf;
}


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

inline void yuv2rgb565(int y, int u, int v, quint16 *rgb)
{
    register int r, g, b;

    r = y + redAdjust[v];
    g = y + greenAdjust1[u] + greenAdjust2[v];
    b = y + blueAdjust[u];

#if 0
    y -= 16;
    u -= 128;
    v -= 128;
    r = y + 1.13983*v;
    g = y + 0.39465*u - 0.58060*v;
    b = y + 2.03211*u;
#endif

    r = CLAMP(r);
    g = CLAMP(g);
    b = CLAMP(b);

    *rgb = (quint16)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

inline void yuv2rgb24(int y, int u, int v, quint32 *rgb)
{
    register int r, g, b;

    r = y + redAdjust[v];
    g = y + greenAdjust1[u] + greenAdjust2[v];
    b = y + blueAdjust[u];

    r = CLAMP(r);
    g = CLAMP(g);
    b = CLAMP(b);

    *rgb = ((r<<24)|(g<<16) | b) | 0xff000000 ;
}

#define yuv2rgb(a,b,c,d) yuv2rgb565(a,b,c,d)

unsigned char* YUVConverter::convert(unsigned char* src)
{
    quint16* dest = (quint16*)m_buf;
    unsigned char *buf = src;
    int size = (m_width * m_height) >> 1;


#define CONV  \
{\
    register int u = buf[m_ui];\
    register int v = buf[m_vi];\
    yuv2rgb(buf[m_y1i], u, v, dest);\
    yuv2rgb(buf[m_y2i], u, v, ++dest);\
    dest++;\
    buf+= 4;\
}

    register int n = (size + 7) / 8;
    switch(size & 7)
    {
        case 0: do {CONV
        case 7:     CONV
        case 6:     CONV
        case 5:     CONV
        case 4:     CONV
        case 3:     CONV
        case 2:     CONV
        case 1:     CONV
                } while(--n);
    }

    return m_buf;

}

/*
 * convert ROUTINE TAKEN FROM:
 *
 * Sonix SN9C101 based webcam basic I/F routines
 * Copyright (C) 2004 Takafumi Mizuno <taka-qce@ls-a.jp>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

BayerConverter::BayerConverter(int width, int height):
    m_width(width),
    m_height(height)
{
    m_buf = new unsigned char[width * height * 4];  // 4 = 32bpp / 8 bpb
}

BayerConverter::~BayerConverter()
{
    delete m_buf;
}

unsigned char* BayerConverter::convert(unsigned char* src)
{
    int             size = m_width * m_height;
    unsigned long   *dst = (unsigned long*) m_buf;

    register unsigned long   dstVal;

    for (int i = 0; i < size; ++i)
    {
        dstVal = 0xFF000000;

        if ((i / m_width) % 2 == 0)
        {
            if (!(i & 1))
            {
                /* B */
                if ((i > m_width) && ((i % m_width) > 0) )
                {
                    dstVal |= ((*(src-m_width-1)+*(src-m_width+1)+ *(src+m_width-1)+*(src+m_width + 1)) / 4) << 16 |
                              ((*(src-1)+*(src+1)+*(src+m_width)+*(src-m_width))/4) << 8 |
                              *src;
                }
                else
                {
                    dstVal |= *(src + m_width + 1) << 16 |
                              (*(src + 1) + *(src + m_width)) / 2 << 8 |
                              *src;
                }
            }
            else
            {
                /* (B)G */
                if ((i > m_width) && ((i % m_width) < (m_width - 1)))
                {
                    dstVal |= (*(src+m_width)+*(src-m_width))/2 << 16 |
                              *src << 8 |
                              (*(src-1)+*(src+1))/2;
                }
                else
                {
                    dstVal |= *(src+m_width) << 16 |
                              *src << 8 |
                              *(src - 1);
                }
            }
        }
        else {

            if (!(i & 1))
            {
                /* G(R) */
                if ((i < (m_width * (m_height - 1))) && ((i % m_width) > 0) )
                {
                    dstVal |= ((*(src - 1) + *(src + 1)) / 2) << 16 |
                              *src << 8 |
                              (*(src + m_width) + *(src - m_width)) / 2;
                }
                else
                {
                    dstVal |= *(src + 1) << 16 |
                              *src << 8 |
                              *(src - m_width);
                }
            }
            else
            {
                /* R */
                if (i < (m_width * (m_height - 1)) && ((i % m_width) < (m_width - 1)) )
                {
                    dstVal |= *src << 16 |
                              (*(src-1)+*(src+1)+ *(src-m_width)+*(src+m_width))/4 << 8 |
                              (*(src-m_width-1)+*(src-m_width+1)+ *(src+m_width-1)+*(src+m_width+1))/4;
                }
                else
                {
                    dstVal |= *src << 16 |
                              (*(src-1)+*(src-m_width))/2 << 8 |
                              *(src-m_width-1);
                }
            }
        }

        *dst++ = dstVal;
            ++src;
    }

    return m_buf;
}


