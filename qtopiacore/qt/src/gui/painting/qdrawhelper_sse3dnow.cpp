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

#include <private/qdrawhelper_x86_p.h>

#if defined(QT_HAVE_3DNOW) && defined(QT_HAVE_SSE)

#include <private/qdrawhelper_sse_p.h>
#include <mm3dnow.h>

QT_BEGIN_NAMESPACE

struct QSSE3DNOWIntrinsics : public QSSEIntrinsics
{
    static inline void end() {
        _m_femms();
    }
};

CompositionFunctionSolid qt_functionForModeSolid_SSE3DNOW[numCompositionFunctions] = {
    comp_func_solid_SourceOver<QSSE3DNOWIntrinsics>,
    comp_func_solid_DestinationOver<QSSE3DNOWIntrinsics>,
    comp_func_solid_Clear<QSSE3DNOWIntrinsics>,
    comp_func_solid_Source<QSSE3DNOWIntrinsics>,
    0,
    comp_func_solid_SourceIn<QSSE3DNOWIntrinsics>,
    comp_func_solid_DestinationIn<QSSE3DNOWIntrinsics>,
    comp_func_solid_SourceOut<QSSE3DNOWIntrinsics>,
    comp_func_solid_DestinationOut<QSSE3DNOWIntrinsics>,
    comp_func_solid_SourceAtop<QSSE3DNOWIntrinsics>,
    comp_func_solid_DestinationAtop<QSSE3DNOWIntrinsics>,
    comp_func_solid_XOR<QSSE3DNOWIntrinsics>
};

CompositionFunction qt_functionForMode_SSE3DNOW[numCompositionFunctions] = {
    comp_func_SourceOver<QSSE3DNOWIntrinsics>,
    comp_func_DestinationOver<QSSE3DNOWIntrinsics>,
    comp_func_Clear<QSSE3DNOWIntrinsics>,
    comp_func_Source<QSSE3DNOWIntrinsics>,
    0,
    comp_func_SourceIn<QSSE3DNOWIntrinsics>,
    comp_func_DestinationIn<QSSE3DNOWIntrinsics>,
    comp_func_SourceOut<QSSE3DNOWIntrinsics>,
    comp_func_DestinationOut<QSSE3DNOWIntrinsics>,
    comp_func_SourceAtop<QSSE3DNOWIntrinsics>,
    comp_func_DestinationAtop<QSSE3DNOWIntrinsics>,
    comp_func_XOR<QSSE3DNOWIntrinsics>
};

void qt_blend_color_argb_sse3dnow(int count, const QSpan *spans, void *userData)
{
    qt_blend_color_argb_x86<QSSE3DNOWIntrinsics>(count, spans, userData,
                                                 (CompositionFunctionSolid*)qt_functionForModeSolid_SSE3DNOW);
}

void qt_memfill32_sse3dnow(quint32 *dest, quint32 value, int count)
{
    return qt_memfill32_sse_template<QSSE3DNOWIntrinsics>(dest, value, count);
}


void qt_bitmapblit16_sse3dnow(QRasterBuffer *rasterBuffer, int x, int y,
                              quint32 color,
                              const uchar *src,
                              int width, int height, int stride)
{
    return qt_bitmapblit16_sse_template<QSSE3DNOWIntrinsics>(rasterBuffer, x,y,
                                                             color, src, width,
                                                             height, stride);
}

QT_END_NAMESPACE

#endif // QT_HAVE_3DNOW && QT_HAVE_SSE
