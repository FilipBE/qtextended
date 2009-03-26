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

#ifndef QPAINTENGINE_D3D_P_H
#define QPAINTENGINE_D3D_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtGui/qpaintengine.h"
#include <d3d9.h>

QT_BEGIN_NAMESPACE

class QDirect3DPaintEnginePrivate;
class QDirect3DPaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QDirect3DPaintEngine)
public:
    QDirect3DPaintEngine();
    ~QDirect3DPaintEngine();
    bool begin(QPaintDevice *device);

    void drawEllipse(const QRectF &rect);
    void drawEllipse(const QRect &rect);

    void drawImage(const QRectF &rectangle, const QImage &image, const QRectF &sr,
        Qt::ImageConversionFlags flags = Qt::AutoColor);

    void drawLines(const QLineF *lines, int lineCount);
    void drawLines(const QLine *lines, int lineCount);

    void drawPath(const QPainterPath &path);

    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);

    void drawPoints(const QPointF *points, int pointCount);
    void drawPoints(const QPoint *points, int pointCount);

    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode);

    void drawRects(const QRectF *rects, int rectCount);
    void drawRects(const QRect * rects, int rectCount);

    void drawTextItem(const QPointF &p, const QTextItem &textItem);

    void drawTiledPixmap(const QRectF &r, const QPixmap &pm, const QPointF &sr);

    bool end();

    Type type() const { return Direct3D; }
    void updateState(const QPaintEngineState &state);

    void cleanup();

    HDC getDC() const;
    void setFlushOnEnd(bool flushOnEnd);
    bool hasDirect3DSupport();

public:
    void scroll(QPaintDevice *pd, const RECT &srcrect, const RECT &destrect);
    LPDIRECT3DSWAPCHAIN9 swapChain(QPaintDevice *pd);
    void releaseSwapChain(QPaintDevice *pd);

private:
    Q_DISABLE_COPY(QDirect3DPaintEngine)
    friend class QPixmap;
    friend class QD3DGlyphCache;
};

QT_END_NAMESPACE

#endif
