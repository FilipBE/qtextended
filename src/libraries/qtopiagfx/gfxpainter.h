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

#ifndef GFXPAINTER_H
#define GFXPAINTER_H

#include <qglobal.h>
#include <QRect>
#include <QPainter>
#include "gfx.h"

class QImage;
class GfxImageRef;
class MainThreadProxy;
class QPaintEvent;
class GfxDirectPainter;
class QTOPIAGFX_EXPORT GfxPainter
{
public:
    GfxPainter();
    GfxPainter(QImage &, const QRegion & = QRegion());
    GfxPainter(QWidget *, QPaintEvent *);
    virtual ~GfxPainter();

    bool usingQt() const { return useQt; }
    void setRect(const QRect &);
    void clear();
    void clear(ushort);

    qreal opacity() const;
    void setOpacity(qreal);

    QRect clipRect() const;

    void setUserClipRect(const QRect &);
    QRect userClipRect() const;

    typedef uchar (*HorizontalOpacityFunction)(int, void *);
    void setHorizontalOpacityFunction(HorizontalOpacityFunction, void *);

    void drawImage(int x, int y, const QImage &);
    void drawImage(qreal x, int y, const QImage &);
    void drawImage(const QPoint &, const QImage &);
    void drawImageFlipped(int x, int y, const QImage &);
    void drawImageFlipped(const QPoint &, const QImage &);
    void drawImage(int x, int y, const GfxImageRef &);
    void drawImage(const QPoint &, const GfxImageRef &);
    void drawImageFlipped(int x, int y, const GfxImageRef &);
    void drawImageFlipped(const QPoint &, const GfxImageRef &);

    void drawImage(const QRect &, const GfxImageRef &, const QRect &);
    void drawImage(const QRect &, const QImage &, const QRect &);
    void drawImage(const QRect &, const QImage &);
    void drawImage(const QRect &, const GfxImageRef &);

    void drawImageTransformed(const QMatrix &, const QImage &, 
                              bool smooth = false);

    void fillRect(const QRect &, const QColor &);
    void fillRectTransformed(const QMatrix &, const QSize &, const QColor &, 
                             bool smooth = false);

    void flip();
    void flip(const QRect &);

    static QImage string(const QString &, const QColor & = Qt::black, const QFont & = QFont());

    uchar *frameBuffer() const { return fBuffer; }
    uchar *backBuffer() const { return buffer; }
    QImage *img() { return pImg; }
    QPainter *painter() { return p; }
    QImage img(const QRect &) const;
    GfxImageRef imgRef(const QRect &) const;

private:
    void drawImage(int x, int y, const GfxImageRef &, bool);
    void fillOpaque(const QRect &, const QRgb &c);
    void flipUnclipped(const QRect &);
    QRect fRect;
    uchar *fBuffer;
    int linestep;

    int width;
    int height;
    int step;
    uchar *buffer;

    uchar _opacity;
    qreal realOpacity;

    friend class MainThreadProxy;
    MainThreadProxy *mainThreadProxy;
    void mainThreadProxyFunc();

    friend class GfxDirectPainter;
    GfxDirectPainter *dp;
    QRegion clipRegion;
    QVector<QRect> clipRects;
    QRect _userClipRect;

    QPainter *p;
    QImage *pImg;

    bool useQt;

    enum { Depth_16 = 2, Depth_32 = 4 } depth;

    HorizontalOpacityFunction opacityFunc;
    void *opacityFuncData;
};

#endif
