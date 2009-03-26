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

#ifndef GFXCANVAS_H
#define GFXCANVAS_H

#include <QDebug>
#include <GfxValue>
#include <QTime>
#include <QList>
#include <QSet>
#include <QImage>
#include <QColor>
#include <QKeyEvent>
#include <QFont>

class GfxCanvasItem;
class GfxCanvasItemDirtyValue : public GfxValue
{
public:
    GfxCanvasItemDirtyValue(GfxCanvasItem *p)
        : _p(p) {}
    virtual void setValue(qreal v);

private:
    GfxCanvasItem *_p;
};

class GfxCanvas;
class GfxPainter;
class GfxCanvasLayer;
class GfxCanvasWindow;
class GfxCanvasItem : public GfxTimeLineObject
{
public:
    GfxCanvasItem(GfxCanvasItem *);
    virtual ~GfxCanvasItem();

    GfxEvent destroyEvent();

    GfxValue &x();
    GfxValue &y();
    GfxValue &z();
    const GfxValue &x() const;
    const GfxValue &y() const;
    const GfxValue &z() const;
    GfxValue &scale(); 
    GfxValue &visible();

    GfxCanvas *canvas();
    virtual GfxCanvasItem *focusProxy();
    void setFocused(bool);
    bool focused();

    QPoint mapFrom(GfxCanvasItem *, const QPoint &);
    QPoint mapTo(GfxCanvasItem *, const QPoint &);

    GfxCanvasItem *parent() const;
    void setParent(GfxCanvasItem *);
    void moveToParent(GfxCanvasItem *);

    virtual QRect boundingRect();
    virtual void paint(GfxPainter &);
    virtual void clean();

    virtual void keyPressEvent(QKeyEvent *);
    virtual void keyReleaseEvent(QKeyEvent *);

    void dirty();

    virtual GfxCanvasWindow *window();
    virtual GfxCanvasLayer *layer();
    virtual void focusIn();
    virtual void focusOut();

public:
    virtual void addChild(GfxCanvasItem *);
    virtual void remChild(GfxCanvasItem *);

    qreal globalX();
    qreal globalY();
    qreal globalScale();
    qreal globalVisible();

    virtual qreal layerX();
    virtual qreal layerY();
    virtual qreal layerScale();
    virtual qreal layerVisible();

    void zOrderChildren();

    QList<GfxCanvasItem *> _children;
private:
    void destroy();

    friend class GfxCanvas;
    friend class GfxCanvasLayer;
    GfxCanvasItem();

    GfxCanvasItem *_parent;

    GfxCanvasItemDirtyValue _x;
    GfxCanvasItemDirtyValue _y;
    GfxCanvasItemDirtyValue _z;
    GfxCanvasItemDirtyValue _scale;
    GfxCanvasItemDirtyValue _visible;

    bool _dirty;
};

class GfxCanvasLayer : public GfxCanvasItem
{
public:
    GfxCanvasLayer(GfxCanvasItem *parent);

    virtual void addChild(GfxCanvasItem *);
    virtual void addDirty(GfxCanvasItem *);
    virtual void remDirty(GfxCanvasItem *);
    virtual GfxCanvasLayer *layer();

    virtual qreal layerX();
    virtual qreal layerY();
    virtual qreal layerScale();
    virtual qreal layerVisible();

private:
    friend class GfxCanvas;
    friend class GfxCanvasRootLayer;
    GfxCanvasLayer();
};

class GfxCanvasCacheLayer : public GfxCanvasLayer
{
public:
    GfxCanvasCacheLayer(const QSize &, GfxCanvasItem *, bool = true);

    GfxValue &quality();

    virtual void addChild(GfxCanvasItem *);
    virtual void addDirty(GfxCanvasItem *);
    virtual void remDirty(GfxCanvasItem *);
    virtual void clean();
    virtual void paint(GfxPainter &);
    virtual QRect boundingRect();

    bool refreshOnUpdates() const;
    void setRefreshOnUpdates(bool);

    QSize size() const { return _s; }
private:
    QList<GfxCanvasItem *> _dirtyItems;

    enum { None, Invalid, Add } _optState;
    GfxCanvasItem *_addItem;
    QSize _s;
    QImage _img;
    GfxCanvasItemDirtyValue _quality; 
    bool _refreshOnUpdates;
};

class GfxCanvasWindow : public GfxCanvasItem, public GfxValueSink
{
public:
    GfxCanvasWindow(const QSize &, GfxCanvasItem *);
    virtual ~GfxCanvasWindow();

    GfxCanvasItem *windowItem();
    GfxValue &cached();

    static GfxCanvasWindow *activeWindow();
    bool isActive() const;
    void activate();
    void deactivate();

    virtual void activated();
    virtual void deactivated(GfxCanvasWindow *);

    QStringList attributes() const;
    bool attribute(const QString &);
    void setAttribute(const QString &);
    void removeAttribute(const QString &);

    virtual GfxCanvasWindow *window();

    GfxCanvasItem *focused();
    void setFocused(GfxCanvasItem *);
    void setFocused(bool);

private:
    virtual void valueChanged(GfxValue *, qreal old, qreal newValue);

    GfxValueNotifying _cached;
    GfxCanvasItem _window;
    QSize _s;
    GfxCanvasCacheLayer *_cache;
    QSet<QString> _attribute;
    static QList<GfxCanvasWindow *> _active;
    static QSet<GfxCanvasWindow *> _windows;
    GfxCanvasItem *_focused;
};


class GfxCanvasReflection : public GfxCanvasItem
{
public:
    GfxCanvasReflection(const QSize &, int dx, int dy, GfxCanvasItem *parent);
    GfxCanvasReflection(const QRect &, GfxCanvasItem *parent);

    virtual QRect boundingRect();
    virtual void paint(GfxPainter &);

private:
    QRect reflectRect();
    QRect internalBoundingRect();
    QSize _s;
    int _dx;
    int _dy;
    QRect _r;
};

class GfxCanvasClip : public GfxCanvasItem
{
public:
    enum ClipType { Height = 0x01, Width = 0x02, Both = 0x03 };
    GfxCanvasClip(GfxCanvasItem *, ClipType = Both);

    GfxValue &width();
    GfxValue &height();
    const GfxValue &width() const;
    const GfxValue &height() const;

    virtual QRect boundingRect();
    virtual void paint(GfxPainter &g);
private:
    QRect clip();
    GfxCanvasItemDirtyValue _width;
    GfxCanvasItemDirtyValue _height;
    ClipType _type;
};

class GfxCanvasImage : public GfxCanvasItem
{
public:
    GfxCanvasImage(const QSize &s, GfxCanvasItem *parent);
    GfxCanvasImage(const QImage &img, GfxCanvasItem *parent);

    virtual QRect boundingRect();
    GfxValue &rotate();
    GfxValue &quality();

    virtual void paint(GfxPainter &);
    QImage image() const;
    void setImage(const QImage &);

    bool subPixelPlacement() const;
    void setSubPixelPlacement(bool);
protected:
    QSize _s;
    GfxCanvasItemDirtyValue _rotate;
    GfxCanvasItemDirtyValue _quality; 

private:
    bool _spp;
    QImage _img;
};

class GfxCanvasText : public GfxCanvasItem
{
public:
    GfxCanvasText(const QSize &, GfxCanvasItem *parent);

    QString text() const;
    void setText(const QString &);
    QColor color() const;
    void setColor(const QColor &);
    QFont font() const;
    void setFont(const QFont &);
    Qt::AlignmentFlag alignmentFlags() const;
    void setAlignmentFlags(Qt::AlignmentFlag);

    QSize size() const;

    GfxValue &quality();
    virtual QRect boundingRect();
    virtual void paint(GfxPainter &);

    QImage &image();
private:
    QPoint getImgPoint();
    void checkCache();
    QImage _img;
    bool _dirty;
    QString _text;
    QColor _color;
    QFont _font;
    QSize _size;

    GfxCanvasItemDirtyValue _quality;
    Qt::AlignmentFlag _flags;
};

class GfxCanvasColor : public GfxCanvasItem
{
public:
    GfxCanvasColor(const QColor &color, const QSize &s, GfxCanvasItem *parent);

    virtual QRect boundingRect();
    GfxValue &rotate();
    GfxValue &quality();

    QColor color() const;
    QSize size() const;

    virtual void paint(GfxPainter &);

private:
    GfxCanvasItemDirtyValue _rotate;
    GfxCanvasItemDirtyValue _quality;
    QColor _color;
    QSize _s;
};

class GfxCanvasMipImage : public GfxCanvasImage
{
public:
    GfxCanvasMipImage(const QSize &, GfxCanvasItem *parent);

    void clear();
    void addImage(const QImage &);

    virtual void paint(GfxPainter &);

private:
    QVector<QImage> _imgs;
};

class GfxCanvasRoundedRect : public GfxCanvasItem
{
public:
    GfxCanvasRoundedRect(GfxCanvasItem *parent);

    QColor color() const;
    int cornedCurve() const;
    int lineWidth() const;
    bool filled() const;
    void setColor(const QColor &);
    void setCornerCurve(int);
    void setLineWidth(int);
    void setFilled(bool);

    virtual QRect boundingRect();
    virtual void paint(GfxPainter &g);

    GfxValue &width();
    GfxValue &height();
    const GfxValue &width() const;
    const GfxValue &height() const;

private:
    void init();

    bool _vdirty;
    QColor _color;

    int _cornerCurve;
    int _lineWidth;
    bool _filled;

    GfxCanvasItemDirtyValue _width;
    GfxCanvasItemDirtyValue _height;

    QImage ul;
    QImage ur;
    QImage ll;
    QImage lr;
    QImage lower;
    QImage upper;
    QImage left;
    QImage right;
};

class GfxCanvas : public QObject
{
    Q_OBJECT
public:
    enum Mode { Widget, Threaded };
    GfxCanvas(QWidget *w, Mode = Widget);
    virtual ~GfxCanvas();

    QRect dirtyItemClip() const;
    virtual void paint(GfxPainter &p);

    QRect dynamicArea() const;
    void setDynamicArea(const QRect &);
    void addDynamicItem(GfxCanvasItem *);
    void remDynamicItem(GfxCanvasItem *);

    GfxCanvasItem *root() { return _root; }

    void setFocused(GfxCanvasItem *);
    GfxCanvasItem *focused() const;

    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);

    void dumpTiming();

    QRegion resetDirty();

    void addFrameTime(int, const QRegion &);

signals:
    void focusChanged(GfxCanvasItem *);

protected:
    virtual bool event(QEvent *);

private:
    friend class GfxCanvasRootLayer;
    void addDirty(GfxCanvasItem *);
    void remDirty(GfxCanvasItem *);
    QRect _oldDirty;

    QRect _dynArea;
    QList<GfxCanvasItem *> _dynItems;

    int _timer;
    friend class GfxCanvasItem;
    GfxCanvasLayer *_root;
    QList<GfxCanvasItem *> _dirtyItems;
    GfxCanvasItem *_focused;

    QTime _frameTimer;

    Mode _mode;
    QWidget *_w;
};

inline void GfxCanvasItemDirtyValue::setValue(qreal v)
{
    if(v == value()) return;
    _p->dirty();
    GfxValue::setValue(v);
}

#endif
