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

#include <private/qt_mac_p.h>
#include "qcoreapplication.h"
#include <qlibrary.h>

QT_BEGIN_NAMESPACE

QRegion::QRegionData QRegion::shared_empty = { Q_BASIC_ATOMIC_INITIALIZER(1), 0, 0 };

#define RGN_CACHE_SIZE 200
#ifdef RGN_CACHE_SIZE
static bool rgncache_init = false;
static int rgncache_used;
static RgnHandle rgncache[RGN_CACHE_SIZE];
static void qt_mac_cleanup_rgncache()
{
    rgncache_init = false;
    for(int i = 0; i < RGN_CACHE_SIZE; ++i) {
        if(rgncache[i]) {
            --rgncache_used;
            DisposeRgn(rgncache[i]);
            rgncache[i] = 0;
        }
    }
}
#endif

Q_GUI_EXPORT RgnHandle qt_mac_get_rgn()
{
#ifdef RGN_CACHE_SIZE
    if(!rgncache_init) {
        rgncache_used = 0;
        rgncache_init = true;
        for(int i = 0; i < RGN_CACHE_SIZE; ++i)
            rgncache[i] = 0;
        qAddPostRoutine(qt_mac_cleanup_rgncache);
    } else if(rgncache_used) {
        for(int i = 0; i < RGN_CACHE_SIZE; ++i) {
            if(rgncache[i]) {
                RgnHandle ret = rgncache[i];
                SetEmptyRgn(ret);
                rgncache[i] = 0;
                --rgncache_used;
                return ret;
            }
        }
    }
#endif
    return NewRgn();
}

Q_GUI_EXPORT void qt_mac_dispose_rgn(RgnHandle r)
{
#ifdef RGN_CACHE_SIZE
    if(rgncache_init && rgncache_used < RGN_CACHE_SIZE) {
        for(int i = 0; i < RGN_CACHE_SIZE; ++i) {
            if(!rgncache[i]) {
                ++rgncache_used;
                rgncache[i] = r;
                return;
            }
        }
    }
#endif
    DisposeRgn(r);
}

static OSStatus qt_mac_get_rgn_rect(UInt16 msg, RgnHandle, const Rect *rect, void *reg)
{
    if(msg == kQDRegionToRectsMsgParse) {
        QRect rct(rect->left, rect->top, (rect->right - rect->left), (rect->bottom - rect->top));
        if(!rct.isEmpty())
            *((QRegion *)reg) += rct;
    }
    return noErr;
}

Q_GUI_EXPORT QRegion qt_mac_convert_mac_region(RgnHandle rgn)
{
    QRegion ret;
    ret.detach();
    RegionToRectsUPP cbk = NewRegionToRectsUPP(qt_mac_get_rgn_rect);
    OSStatus oss = QDRegionToRects(rgn, kQDParseRegionFromTopLeft, cbk, (void *)&ret);
    DisposeRegionToRectsUPP(cbk);
    if(oss != noErr)
        return QRegion();
    return ret;
}

typedef OSStatus (*PtrHIShapeGetAsQDRgn)(HIShapeRef, RgnHandle);
static PtrHIShapeGetAsQDRgn ptrHIShapeGetAsQDRgn = 0;

QRegion qt_mac_convert_mac_region(HIShapeRef shape)
{
    if (ptrHIShapeGetAsQDRgn == 0) {
        QLibrary library(QLatin1String("/System/Library/Frameworks/Carbon.framework/Carbon"));
        library.setLoadHints(QLibrary::ExportExternalSymbolsHint);
		ptrHIShapeGetAsQDRgn = reinterpret_cast<PtrHIShapeGetAsQDRgn>(library.resolve("HIShapeGetAsQDRgn"));
    }

    RgnHandle rgn = qt_mac_get_rgn();
    ptrHIShapeGetAsQDRgn(shape, rgn);
    QRegion ret = qt_mac_convert_mac_region(rgn);
    qt_mac_dispose_rgn(rgn);
    return ret;
}

/*!
    \internal
*/
RgnHandle QRegion::handle(bool require_rgn) const
{
    if(!d->rgn && (require_rgn || (d->qt_rgn && d->qt_rgn->numRects > 1))) {
        d->rgn = qt_mac_get_rgn();
        if(d->qt_rgn && d->qt_rgn->numRects) {
            RgnHandle tmp_rgn = qt_mac_get_rgn();
            int n = d->qt_rgn->numRects;
            const QRect *qt_r = (n == 1 ? &d->qt_rgn->extents : d->qt_rgn->rects.constData());
            while (n--) {
                SetRectRgn(tmp_rgn,
                           qMax(SHRT_MIN, qt_r->x()),
                           qMax(SHRT_MIN, qt_r->y()),
                           qMin(SHRT_MAX, qt_r->right() + 1),
                           qMin(SHRT_MAX, qt_r->bottom() + 1));
                UnionRgn(d->rgn, tmp_rgn, d->rgn);
                ++qt_r;
            }
            qt_mac_dispose_rgn(tmp_rgn);
        }
    }
    return d->rgn;
}

QT_END_NAMESPACE
