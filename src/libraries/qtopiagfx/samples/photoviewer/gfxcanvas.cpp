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

#include "gfxcanvas.h"
#include <Gfx>
#include "gfximage.h"
#include <QWidget>
#include <GfxPainter>
#include <QMutex>
#include <QDebug>
#include <QTime>
#include <QCoreApplication>

template<class T, int s = 60>
class CircularList
{
public:
    CircularList()
        : _first(0), _size(0) {}

    void append(const T &t)
    {
        int entry = (_first + _size) % s;
        _array[entry] = t;
        if(_size == s)
            _first = (_first + 1) % s;
        else
            _size++;
    }

    int size() const
    {
        return _size;
    }

    T &operator[](int idx)
    {
        Q_ASSERT(idx < _size);
        int entry = (_first + idx) % s;
        return _array[entry];
    }

    void clear()
    {
        _first = 0;
        _size = 0;
    }
private:
    int _first;
    int _size;
    T _array[s];
};


GfxCanvasLayer::GfxCanvasLayer()
{
}

GfxCanvasLayer::GfxCanvasLayer(GfxCanvasItem *parent)
: GfxCanvasItem(parent)
{
}

void GfxCanvasLayer::addChild(GfxCanvasItem *c)
{
    GfxCanvasItem::addChild(c);
}

void GfxCanvasItem::focusIn()
{
}

void GfxCanvasItem::focusOut()
{
}

void GfxCanvasItem::keyPressEvent(QKeyEvent *e)
{
    GfxCanvasItem *p = parent();
    if(p)
        p->keyPressEvent(e);
}

void GfxCanvasItem::keyReleaseEvent(QKeyEvent *e)
{
    GfxCanvasItem *p = parent();
    if(p)
        p->keyReleaseEvent(e);
}

void GfxCanvasItem::clean()
{
    _dirty = false;
}

void GfxCanvasLayer::addDirty(GfxCanvasItem *)
{
}

void GfxCanvasLayer::remDirty(GfxCanvasItem *)
{
}

qreal GfxCanvasLayer::layerX()
{
    return 0.;
}

qreal GfxCanvasLayer::layerY()
{
    return 0.;
}

qreal GfxCanvasLayer::layerScale()
{
    return 1.;
}

qreal GfxCanvasLayer::layerVisible()
{
    return 1.;
}

GfxCanvasLayer *GfxCanvasLayer::layer() 
{
    return this;
}

class GfxCanvasRootLayer : public GfxCanvasLayer
{
public:
    GfxCanvasRootLayer(GfxCanvas *);
    virtual void addDirty(GfxCanvasItem *);
    virtual void remDirty(GfxCanvasItem *);

private:
    friend class GfxCanvasItem;
    GfxCanvas *_canvas;
};

GfxCanvasRootLayer::GfxCanvasRootLayer(GfxCanvas *c)
: _canvas(c)
{
    dirty();
}

void GfxCanvasRootLayer::addDirty(GfxCanvasItem *i)
{
    _canvas->addDirty(i);
}

void GfxCanvasRootLayer::remDirty(GfxCanvasItem *i)
{
    _canvas->remDirty(i);
}


GfxCanvasItem::GfxCanvasItem()
: _parent(0), _x(this), _y(this), _z(this),
  _scale(this), _visible(this), _dirty(false)
{
    _scale.setValue(1.);
    _visible.setValue(1.);
}

GfxCanvasItem::GfxCanvasItem(GfxCanvasItem *p)
: _parent(p), _x(this), _y(this), _z(this),
    _scale(this), _visible(this), _dirty(false)
{
    Q_ASSERT(p);

    _scale.setValue(1.);
    _visible.setValue(1.);
    _parent->addChild(this);
    dirty();
}

GfxCanvasItem::~GfxCanvasItem()
{ 
    dirty();
    if(_parent) _parent->remChild(this);
    if(_dirty) 
        layer()->remDirty(this);

    QList<GfxCanvasItem *> children = _children;
    qDeleteAll(children);
    _dirty = false;
}

GfxCanvasLayer *GfxCanvasItem::layer() 
{
    if(_parent)
        return _parent->layer();
    else
        return 0;
}

GfxCanvasWindow *GfxCanvasItem::window()
{
    if(_parent)
        return _parent->window();
    else
        return 0;
}

void GfxCanvasItem::dirty()
{
    if(_dirty || 0. == _visible.value()) return;

    GfxCanvasLayer *l = layer(); 
    if(l == this && _parent)
        l = _parent->layer();
    if(l) {
        l->addDirty(this);
        _dirty = true;
    }
}

GfxEvent GfxCanvasItem::destroyEvent()
{
    return GfxEvent(this, this,  &GfxCanvasItem::destroy);
}

void GfxCanvasItem::destroy()
{
    delete this;
}

const GfxValue &GfxCanvasItem::x() const
{
    return _x;
}

const GfxValue &GfxCanvasItem::y() const
{
    return _y;
}

const GfxValue &GfxCanvasItem::z() const
{
    return _z;
}

GfxValue &GfxCanvasItem::x()
{
    return _x;
}

GfxValue &GfxCanvasItem::y()
{
    return _y;
}

GfxValue &GfxCanvasItem::z()
{
    return _z;
}

GfxValue &GfxCanvasItem::scale()
{
    return _scale;
}

GfxValue &GfxCanvasItem::visible()
{
    return _visible;
}

GfxCanvas *GfxCanvasItem::canvas()
{
    GfxCanvasLayer * l = this->layer();
    while(l->parent())
        l = l->parent()->layer();

    return static_cast<GfxCanvasRootLayer *>(l)->_canvas;
}

GfxCanvasItem *GfxCanvasItem::focusProxy()
{
    return 0;
}

void GfxCanvasItem::setFocused(bool f)
{
    if(focusProxy()) {
        focusProxy()->setFocused(f);
        return;
    }
    GfxCanvasWindow *w = window();
    if(w) {
        if(f) {
            w->setFocused(this);
        } else {
            if(w->focused() == this)
                w->setFocused((GfxCanvasItem *)0);
        }
    } else {
        GfxCanvas *c = canvas();
        if(f) {
            c->setFocused(this);
        } else {
            if(c->focused() == this)
                c->setFocused(0);
        }
    }
}

bool GfxCanvasItem::focused()
{
    return (canvas()->focused() == this);
}

GfxCanvasItem *GfxCanvasItem::parent() const
{
    return _parent;
}

void GfxCanvasItem::setParent(GfxCanvasItem *p)
{
    if(p == _parent || !p) return;

    GfxCanvasLayer *o = layer();
    dirty();
    _parent->remChild(this);
    _parent = p;
    _parent->addChild(this);
    GfxCanvasLayer *n = layer();

    if(o != n) {
        o->remDirty(this);
        n->addDirty(this);
    }

}

QPoint GfxCanvasItem::mapFrom(GfxCanvasItem *i, const QPoint &p)
{
    QPoint rv = i->mapTo(this, p);
    return rv;
}

QPoint GfxCanvasItem::mapTo(GfxCanvasItem *i, const QPoint &p)
{
    qreal x = globalX();
    qreal y = globalY();
    qreal ix = i->globalX();
    qreal iy = i->globalY();

    QPoint rv = p;
    rv.setX(int(rv.x() - ix + x));
    rv.setY(int(rv.y() - iy + y));
    return rv;
}

void GfxCanvasItem::moveToParent(GfxCanvasItem *p)
{
    if(p == _parent || !p) return;

    // We (may) need to adjust x, y, scale and visible
    qreal x = globalX();
    qreal y = globalY();
    qreal s = globalScale();
    qreal v = globalVisible();

    setParent(p);

    if(x != globalX())  {
        this->x().setValue((x - _parent->globalX()) / _parent->globalScale());
    }
    if(y != globalY())  {
        this->y().setValue((y - _parent->globalY()) / _parent->globalScale());
    }
    if(s != globalScale())  {
        scale().setValue(s / _parent->globalScale());
    }
    if(v != globalVisible()) {
        qreal pv = _parent->globalVisible();
        if(pv)
            visible().setValue(v / pv);
    }
}

QRect GfxCanvasItem::boundingRect()
{
    QRect rv;
    for(int ii = 0; ii < _children.count(); ++ii) 
        rv |= _children.at(ii)->boundingRect();
    return rv;
}

void GfxCanvasItem::paint(GfxPainter &p)
{
    zOrderChildren();
    for(int ii = 0; ii < _children.count(); ++ii) {
        GfxCanvasItem *c = _children.at(ii);
        if(c->visible().value() != 0.) 
            c->paint(p);
    }
}

void GfxCanvasItem::zOrderChildren()
{
    if(_children.count() <= 1)
        return;

    // This is a bubble sort for a reason - it is the fastest sort for a mostly
    // ordered list.  We only expect z ordering to change infrequently.
    bool swap = true;
    int c = 0;
    while(swap) {
        ++c;
        swap = false;
        GfxCanvasItem *item = _children.first();
        qreal z = item->z().value();
        for(int ii = 1; ii < _children.count(); ++ii) {
            GfxCanvasItem *i2 = _children.at(ii);
            qreal z2 = i2->z().value();
            if(z2 < z) {
                swap = true;
                _children[ii] = item;
                _children[ii - 1] = i2;
            } else {
                item = i2;
                z = z2;
            }
        }
    }
}

void GfxCanvasItem::addChild(GfxCanvasItem *c)
{
    _children.append(c);
}

void GfxCanvasItem::remChild(GfxCanvasItem *c)
{
    _children.removeAll(c);
}


qreal GfxCanvasItem::globalX()
{
    if(_parent)
        return _parent->globalX() + _parent->globalScale() * _x.value();
    else
        return _x.value();
}

qreal GfxCanvasItem::globalY()
{
    if(_parent)
        return _parent->globalY() + _parent->globalScale() * _y.value();
    else
        return _y.value();
}

qreal GfxCanvasItem::globalScale()
{
    if(_parent)
        return _parent->globalScale() * _scale.value();
    else
        return _scale.value();
}

qreal GfxCanvasItem::globalVisible()
{
    if(_parent)
        return _parent->globalVisible() * _visible.value();
    else
        return _visible.value();
}

qreal GfxCanvasItem::layerX()
{
    if(_parent)
        return _parent->layerX() + _parent->layerScale() * _x.value();
    else
        return _x.value();
}

qreal GfxCanvasItem::layerY()
{
    if(_parent)
        return _parent->layerY() + _parent->layerScale() * _y.value();
    else
        return _y.value();
}

qreal GfxCanvasItem::layerScale()
{
    if(_parent)
        return _parent->layerScale() * _scale.value();
    else
        return _scale.value();
}

qreal GfxCanvasItem::layerVisible()
{
    if(_parent)
        return _parent->layerVisible() * _visible.value();
    else
        return _visible.value();
}

static bool gfxFullRepaint = false;
static bool gfxFullRepaintLoaded = false;
static CircularList<QPair<int, QRegion> > gfxCanvasTiming;

GfxCanvas::GfxCanvas(QWidget *w, Mode mode)
: _timer(0), _root(0), _focused(0), _mode(mode), _w(w)
{
    if(!gfxFullRepaintLoaded) {
        gfxFullRepaintLoaded = true;
        const char *env = getenv("GFX_CANVAS_FULL_UPDATE");
        if(env && !QString(env).isEmpty())
            gfxFullRepaint = true;
    }

    _root = new GfxCanvasRootLayer(this);
}

GfxCanvas::~GfxCanvas()
{
    delete _root;
}

QRect GfxCanvas::dirtyItemClip() const
{
    QRect rv;
    for(int ii = 0; ii < _dirtyItems.count(); ++ii)
        rv = rv | _dirtyItems.at(ii)->boundingRect();
    return rv;
}

void GfxCanvas::paint(GfxPainter &p)
{
    _root->paint(p);
}

QRect GfxCanvas::dynamicArea() const
{
    return _dynArea;
}

void GfxCanvas::setDynamicArea(const QRect &a)
{
    _dynArea = a;
}

void GfxCanvas::remDynamicItem(GfxCanvasItem *ci)
{
    _dynItems.removeAll(ci);
}

void GfxCanvas::addDynamicItem(GfxCanvasItem *ci)
{
    _dynItems << ci;
}

void GfxCanvas::setFocused(GfxCanvasItem *f)
{
    if(_focused == f)
        return;

    if(_focused)
        _focused->focusOut();
    _focused = f;
    if(_focused)
        _focused->focusIn();
    emit focusChanged(f);
}

GfxCanvasItem *GfxCanvas::focused() const
{
    return _focused;
}

void GfxCanvas::keyPressEvent(QKeyEvent *e)
{
    e->setAccepted(false);
    GfxCanvasItem *i = _focused;
    while(i && !e->isAccepted()) {
        i->keyPressEvent(e);
        i = i->parent();
    }
}

void GfxCanvas::keyReleaseEvent(QKeyEvent *e)
{
    e->setAccepted(false);
    GfxCanvasItem *i = _focused;
    while(i && !e->isAccepted()) {
        i->keyReleaseEvent(e);
        i = i->parent();
    }
}

void GfxCanvas::remDirty(GfxCanvasItem *c)
{
    _dirtyItems.removeAll(c);
}

void GfxCanvas::addDirty(GfxCanvasItem *c)
{
    if(_mode == Widget && !_timer)  {
        QCoreApplication::postEvent(this, new QEvent(QEvent::User));
        _timer = 1;
    }

    _oldDirty |= c->boundingRect();

    _dirtyItems.append(c);
}

QRegion GfxCanvas::resetDirty()
{
    QRect r = _oldDirty | dirtyItemClip();
    for(int ii = 0; ii < _dirtyItems.count(); ++ii)
        _dirtyItems.at(ii)->clean();
    _dirtyItems.clear();
    _oldDirty = QRect();

    if(gfxFullRepaint)
        return QRegion();
    else if(!_dynArea.isEmpty() || !_dynItems.isEmpty()) {
        QRegion rv(r);
        if(rv.isEmpty())
            return rv;
        rv |= _dynArea;
        for(int ii = 0; ii <_dynItems.count(); ++ii)
            if(_dynItems.at(ii)->visible().value())
                rv |= _dynItems.at(ii)->boundingRect();
        return rv;
    } else
        return QRegion(r);
}

void GfxCanvas::addFrameTime(int time, const QRegion &region)
{
    gfxCanvasTiming.append(qMakePair(time, region));
}

bool GfxCanvas::event(QEvent *)
{
    GfxTimeLine::lock()->lock();
    _timer = 0;
    QRegion r = resetDirty();

    _frameTimer.start();
    GfxTimeLine::lock()->unlock();
    if(r.isEmpty() || gfxFullRepaint)
        _w->repaint();
    else 
        _w->repaint(r);

    GfxTimeLine::lock()->lock();
    addFrameTime(_frameTimer.elapsed(), r);
    GfxTimeLine::lock()->unlock();
    return true;
}

void GfxCanvas::dumpTiming()
{
    for(int ii = 0; ii < gfxCanvasTiming.size(); ++ii)
        qWarning() << gfxCanvasTiming[ii].first << gfxCanvasTiming[ii].second;
    gfxCanvasTiming.clear();
}

static bool gfx_show_image_bounds_loaded = false;
static bool gfx_show_image_bounds = false;

static void gfxImageBoundsEnv()
{
    if(!gfx_show_image_bounds_loaded) {
        gfx_show_image_bounds_loaded = true;
        if(!QString(getenv("GFX_SHOW_IMAGE_BOUNDS")).isEmpty())
            gfx_show_image_bounds = true;
    }
}

GfxCanvasImage::GfxCanvasImage(const QSize &s, GfxCanvasItem *parent)
: GfxCanvasItem(parent), _s(s), _rotate(this), _quality(this), _spp(false)
{
    gfxImageBoundsEnv();
}

GfxCanvasImage::GfxCanvasImage(const QImage &img, GfxCanvasItem *parent)
: GfxCanvasItem(parent), _s(img.size()), _rotate(this), _quality(this), _spp(false), _img(img)
{
    gfxImageBoundsEnv();
}

GfxValue &GfxCanvasImage::rotate()
{
    return _rotate;
}

GfxValue &GfxCanvasImage::quality()
{
    return _quality;
}

QRect GfxCanvasImage::boundingRect()
{
    QMatrix m;
    m.translate(layerX(), layerY());
    m.rotate(_rotate.value());
    m.scale(layerScale(), layerScale());
    m.translate(-_s.width() / 2, -_s.height() / 2);
    QRect r(QPoint(0, 0), _s);
    r = m.mapRect(r);
    r.setX(r.x() - 1);
    r.setY(r.y() - 1);
    r.setWidth(r.width() + 2);
    r.setHeight(r.height() + 2);
    return r | GfxCanvasItem::boundingRect();;
}

QImage GfxCanvasImage::image() const
{
    return _img;
}

void GfxCanvasImage::setImage(const QImage &img)
{
    if(img.size() != _s) {
        qWarning() << "GfxCanvasImage: Cannot set image of incorrect size";
        return;
    }
    _img = img;
    dirty();
}

bool GfxCanvasImage::subPixelPlacement() const
{
    return _spp;
}

void GfxCanvasImage::setSubPixelPlacement(bool spp)
{
    if(_spp == spp)
        return;

    dirty();
    _spp = spp;
}

void GfxCanvasImage::paint(GfxPainter &p)
{
    if(_img.isNull())
        return;

    qreal op = p.opacity();
    p.setOpacity(op * layerVisible());

    if(_rotate.value() || 1. != layerScale()) {
        QMatrix m;
        m.translate(layerX(), layerY());
        m.rotate(_rotate.value());
        m.scale(layerScale(), layerScale());
        m.translate(-_s.width() / 2, -_s.height() / 2);
        p.drawImageTransformed(m, _img, quality().value() != 0.);
    } else {
        if(subPixelPlacement()) {
            p.drawImage(qreal(layerX() - qreal(_s.width()) / 2.), 
                        int(layerY() - qreal(_s.height()) / 2.), _img);
        } else {
            p.drawImage(int(layerX() - qreal(_s.width()) / 2.), 
                        int(layerY() - qreal(_s.height()) / 2.), _img);
        }
        if(gfx_show_image_bounds) {
            QRect r(int(layerX() - qreal(_s.width()) / 2.), 
                    int(layerY() - qreal(_s.height()) / 2.),
                    _s.width(), _s.height());
            p.fillRect(r, QColor(0, 0, 255, 127));
        }
    }

    p.setOpacity(op);
    GfxCanvasItem::paint(p);
}

GfxCanvasText::GfxCanvasText(const QSize &s, GfxCanvasItem *parent)
: GfxCanvasItem(parent), _dirty(true), _color(Qt::white), _size(s),
  _quality(this), _flags(Qt::AlignCenter)
{
    gfxImageBoundsEnv();
}

QString GfxCanvasText::text() const
{
    return _text;
}

void GfxCanvasText::setText(const QString &t)
{
    if(_text != t) {
        _text = t;
        dirty();
        _dirty = true;
    }
}

QColor GfxCanvasText::color() const
{
    return _color;
}

void GfxCanvasText::setColor(const QColor &c)
{
    if(_color != c) {
        _color = c;
        dirty();
        _dirty = true;
    }
}

QFont GfxCanvasText::font() const
{
    return _font;
}

void GfxCanvasText::setFont(const QFont &f)
{
    if(_font != f) {
        _font = f;
        dirty();
        _dirty = true;
    }
}


GfxValue &GfxCanvasText::quality()
{
    return _quality;
}

QRect GfxCanvasText::boundingRect()
{
    checkCache();
    if(_img.isNull())
        return QRect();

    QMatrix m;
    m.translate(layerX(), layerY());
    m.scale(layerScale(), layerScale());
    m.translate(-_size.width() / 2, -_size.height() / 2);
    QRect r(getImgPoint(), _img.size());
    r = m.mapRect(r);
    r.setX(r.x() - 1);
    r.setY(r.y() - 1);
    r.setWidth(r.width() + 2);
    r.setHeight(r.height() + 2);
    return r | GfxCanvasItem::boundingRect();;
}

Qt::AlignmentFlag GfxCanvasText::alignmentFlags() const
{
    return _flags;
}

void GfxCanvasText::setAlignmentFlags(Qt::AlignmentFlag f)
{
    if(_flags == f)
        return;
    dirty();
    _flags = f;
}

QPoint GfxCanvasText::getImgPoint()
{
    int x = 0;
    int y = 0;
    if(Qt::AlignLeft & _flags) {
    } else if(Qt::AlignRight & _flags) {
        x = _size.width() - _img.width();
    } else if(Qt::AlignHCenter & _flags) {
        x = (_size.width() - _img.width()) / 2;
    }

    if(Qt::AlignTop & _flags) {
    } else if(Qt::AlignBottom & _flags) {
        y = _size.height() - _img.height();
    } else if(Qt::AlignVCenter & _flags) {
        y = (_size.height() - _img.height()) / 2;
    }

    return QPoint(x, y);
}

QImage &GfxCanvasText::image() 
{
    checkCache();
    return _img;
}

void GfxCanvasText::paint(GfxPainter &p)
{
    if(_img.isNull())
        return;

    qreal op = p.opacity();
    p.setOpacity(op * layerVisible());

    QSize _s = _img.size();
    QPoint point = getImgPoint();
    if(1. != layerScale()) {
        QMatrix m;
        m.translate(layerX(), layerY());
        m.scale(layerScale(), layerScale());
        m.translate(-_size.width() / 2, -_size.height() / 2);
        m.translate(point.x(), point.y());
        p.drawImageTransformed(m, _img, quality().value() != 0.);
    } else {
        p.drawImage(int(layerX()) + point.x() - _size.width() / 2, 
                    int(layerY()) + point.y() - _size.height() / 2, _img);
        if(gfx_show_image_bounds) {
            QRect r(int(layerX()) + point.x() - _size.width() / 2, 
                    int(layerY()) + point.y() - _size.height() / 2,
                    _img.width(),
                    _img.height());
            p.fillRect(r, QColor(0, 0, 255, 127));
        }
    }

    p.setOpacity(op);
    GfxCanvasItem::paint(p);
}

QSize GfxCanvasText::size() const
{
    return _size;
}

void GfxCanvasText::checkCache()
{
    if(_dirty) {
        if(_text.isEmpty()) {
            _img = QImage();
        } else {
            _img = GfxPainter::string(_text, _color, _font);
        }
        _dirty = false;
    }
}

static bool gfx_show_mip_matches_loaded = false;
static bool gfx_show_mip_matches = false;

GfxCanvasMipImage::GfxCanvasMipImage(const QSize &s, GfxCanvasItem *parent)
: GfxCanvasImage(s, parent)
{
    if(!gfx_show_mip_matches_loaded) {
        gfx_show_mip_matches_loaded = true;
        if(!QString(getenv("GFX_SHOW_MIP_MATCHES")).isEmpty())
            gfx_show_mip_matches = true;
    }
}

void GfxCanvasMipImage::clear()
{
    _imgs.clear();
}

void GfxCanvasMipImage::addImage(const QImage &img)
{
    if(img.format() != QImage::Format_RGB32 && 
       img.format() != QImage::Format_RGB16) {
        qWarning() << "GfxCanvasMipImage: Mip image only supports RGB32 and "
                      "RGB16 images";
        return;
    }

    dirty();
    // Imgs are stored in ascending order of size.
    int width = img.width();
    for(int ii = 0; ii < _imgs.count(); ++ii) {
        if(width < _imgs.at(ii).width()) {
            _imgs.insert(ii, img);
            return;
        }
    }
    _imgs.append(img);
}

void GfxCanvasMipImage::paint(GfxPainter &p)
{
    if(_imgs.isEmpty() || scale().value() == 0.) return;

    qreal op = p.opacity();
    p.setOpacity(op * layerVisible());

    // Find correct image
    QSize s = layerScale() * _s;
    int imgNum;
    for(imgNum = 0; imgNum < _imgs.count(); ++imgNum) 
        if(_imgs.at(imgNum).width() >= s.width())
            break;
    if(imgNum == _imgs.count())
        imgNum = _imgs.count() - 1;


    const QImage &img = _imgs.at(imgNum);

    // Adjust zoom
    qreal zoom =  qreal(s.width()) / qreal(img.width());
    qreal rotate = _rotate.value();

    int x = int(layerX());
    int y = int(layerY());
    if(zoom == 1. && rotate == 0.) {
        if(subPixelPlacement()) {
            p.drawImage(qreal(layerX() - qreal(s.width()) / 2.), 
                        int(layerY() - qreal(s.height()) / 2.), img);
        } else {
            p.drawImage(x - s.width() / 2, y - s.height() / 2, img);
        }

        if(gfx_show_mip_matches) {
            QRect r(x - s.width() / 2, y - s.height() / 2, s.width(), s.height());
            p.fillRect(r, QColor(255, 0, 0, 127));
        }
    } else {
        QMatrix m;
        m.translate(x, y);
        if(rotate)
            m.rotate(rotate);
        if(zoom != 1.)
            m.scale(zoom, zoom);
        m.translate(-img.width() / 2, -img.height() / 2);
        p.drawImageTransformed(m, img, _quality.value() != 0.);
    }

    p.setOpacity(op);
    GfxCanvasItem::paint(p);
}

GfxCanvasRoundedRect::GfxCanvasRoundedRect(GfxCanvasItem *parent)
: GfxCanvasItem(parent), _vdirty(true), _color(Qt::white), _cornerCurve(15), _lineWidth(2), _filled(false), _width(this), _height(this)
{
    _width.setValue(_cornerCurve * 2);
    _height.setValue(_cornerCurve * 2);
}

QColor GfxCanvasRoundedRect::color() const
{
    return _color;
}

int GfxCanvasRoundedRect::cornedCurve() const
{
    return _cornerCurve;
}

int GfxCanvasRoundedRect::lineWidth() const
{
    return _lineWidth;
}

bool GfxCanvasRoundedRect::filled() const
{
    return _filled;
}

void GfxCanvasRoundedRect::setColor(const QColor &c)
{
    if(_color == c)
        return;
    _color = c;
    _vdirty = true;
}

void GfxCanvasRoundedRect::setCornerCurve(int c)
{
    if(_cornerCurve == c)
        return;
    _cornerCurve = c;
    _vdirty = true;
}

void GfxCanvasRoundedRect::setLineWidth(int c)
{
    if(_lineWidth == c)
        return;
    _lineWidth = c;
    _vdirty = true;
}

void GfxCanvasRoundedRect::setFilled(bool f)
{
    if(_filled == f)
        return;
    _filled = f;
    _vdirty = true;
}

QRect GfxCanvasRoundedRect::boundingRect()
{
    int x = int(layerX()) - int(_width.value() / 2);
    int y = int(layerY()) - int(_height.value() / 2);
    int width = int(_width.value());
    int height = int(_height.value());

    QRect rv(x, y, width, height);
    return rv | GfxCanvasItem::boundingRect();;
}

void GfxCanvasRoundedRect::paint(GfxPainter &g)
{
    qreal opacity = layerVisible();

    init();

    int x = int(layerX()) - int(_width.value() / 2);
    int y = int(layerY()) - int(_height.value() / 2);
    int width = qMax(int(_width.value()) - _cornerCurve * 2, 0);
    int height = qMax(int(_height.value()) - _cornerCurve * 2, 0);

    if(opacity != 1.)
        g.setOpacity(opacity);

    if(_cornerCurve) {
        g.drawImage(x, y, ul);
        g.drawImage(width + x + _cornerCurve, y, ur);
        g.drawImage(x, height + _cornerCurve + y, ll);
        g.drawImage(width + x + _cornerCurve, height + _cornerCurve + y, lr);
    }

    int drawWidth = width;
    while(drawWidth > 0) {
        g.drawImage(x + _cornerCurve + width - drawWidth, y, GfxImageRef(upper, QRect(0, 0, qMin(drawWidth, 15), _lineWidth)));
        g.drawImage(x + _cornerCurve + width - drawWidth, y + height + 2 * _cornerCurve - _lineWidth, GfxImageRef(lower, QRect(0, 0, qMin(drawWidth, 15), _lineWidth)));
        drawWidth -= 15;
    }

    if(_filled) {
        int cfill = _cornerCurve - _lineWidth;
        if(cfill > 0) {
            g.fillRect(QRect(x + _cornerCurve, y + _lineWidth, width, cfill), _color);
            g.fillRect(QRect(x + _cornerCurve, y + height + _cornerCurve ,width,cfill), _color);
        }
        g.fillRect(QRect(x + _lineWidth, y + _cornerCurve, width + _cornerCurve * 2 - _lineWidth * 2, height), 
                   _color); 
    }
    int drawHeight = height;
    while(drawHeight > 0) {
        g.drawImage(x, y + _cornerCurve + height - drawHeight, GfxImageRef(left, QRect(0, 0, _lineWidth, qMin(drawHeight, 15))));
        g.drawImage(x + width + 2 * _cornerCurve - _lineWidth, y + _cornerCurve + height - drawHeight, GfxImageRef(left, QRect(0, 0, _lineWidth, qMin(drawHeight, 15))));
        drawHeight -= 15;
    }

    g.setOpacity(1.);

    GfxCanvasItem::paint(g);
}

GfxValue &GfxCanvasRoundedRect::width()
{
    return _width;
}

GfxValue &GfxCanvasRoundedRect::height()
{
    return _height;
}

const GfxValue &GfxCanvasRoundedRect::width() const
{
    return _width;
}

const GfxValue &GfxCanvasRoundedRect::height() const
{
    return _height;
}

void GfxCanvasRoundedRect::init()
{
    if(!_vdirty)
        return;
    dirty();
    _vdirty = false;

    if(_cornerCurve) {
        ul = QImage(_cornerCurve, _cornerCurve, 
                    QImage::Format_ARGB32_Premultiplied); 
        ul.fill(0);
        QPainter p;
        p.begin(&ul);
        p.setPen(QPen(_color, _lineWidth));
        p.setBrush(_color);
        p.setRenderHint(QPainter::Antialiasing);

        if(_filled) {
            p.drawEllipse(QRectF(qreal(_lineWidth) / 2, qreal(_lineWidth) / 2, 
                             _cornerCurve * 2, _cornerCurve * 2));
        } else {
            p.drawArc(QRectF(qreal(_lineWidth) / 2, qreal(_lineWidth) / 2, 
                             _cornerCurve * 2, _cornerCurve * 2), 
                      90 * 16, 90 * 16);
        }
        p.end();
        ur = ul.mirrored(true, false);
        ll = ul.mirrored(false, true);
        lr = ul.mirrored(true, true);
    } else {
        ul = QImage();
        ur = ul;
        ll = ul;
        lr = ul;
    }

    upper = QImage(15, _lineWidth, QImage::Format_ARGB32_Premultiplied);
    left = QImage(_lineWidth, 15, QImage::Format_ARGB32_Premultiplied);
  
    upper.fill(0); 
    left.fill(0);

    QPainter p;
    p.begin(&upper);
    p.setPen(QPen(_color, _lineWidth));
    p.setRenderHint(QPainter::Antialiasing);
    p.drawLine(QPointF(0, qreal(_lineWidth) / 2), QPointF(15, qreal(_lineWidth) / 2));
    p.end();
    p.begin(&left);
    p.setPen(QPen(_color, _lineWidth));
    p.setRenderHint(QPainter::Antialiasing);
    p.drawLine(QPointF((qreal)(_lineWidth) / 2, 0), QPointF(qreal(_lineWidth) / 2, 15));
    p.end();
    lower = upper.mirrored(false, true);
    right = left.mirrored(true, false);
}

GfxCanvasColor::GfxCanvasColor(const QColor &color, const QSize &s, GfxCanvasItem *parent)
: GfxCanvasItem(parent), _rotate(this), _quality(this), _color(color), _s(s)
{
}

QRect GfxCanvasColor::boundingRect()
{
    QMatrix m;
    m.translate(layerX(), layerY());
    m.rotate(_rotate.value());
    m.scale(layerScale(), layerScale());
    m.translate(-_s.width() / 2, -_s.height() / 2);
    QRect r(QPoint(0, 0), _s);
    r = m.mapRect(r);
    r.setX(r.x() - 1);
    r.setY(r.y() - 1);
    r.setWidth(r.width() + 2);
    r.setHeight(r.height() + 2);
    return r | GfxCanvasItem::boundingRect();;
}

GfxValue &GfxCanvasColor::rotate()
{
    return _rotate;
}

GfxValue &GfxCanvasColor::quality()
{
    return _quality;
}

QColor GfxCanvasColor::color() const
{
    return _color;
}

QSize GfxCanvasColor::size() const
{
    return _s;
}

void GfxCanvasColor::paint(GfxPainter &p)
{
    qreal op = p.opacity();
    p.setOpacity(op * layerVisible());

    QSize s(int(_s.width() * layerScale()), int(_s.height() * layerScale()));
    if(_rotate.value()) {
        QMatrix m;
        m.translate(layerX(), layerY());
        m.rotate(_rotate.value());
        m.translate(-s.width() / 2, -s.height() / 2);
        p.fillRectTransformed(m, s, color(), quality().value() != 0.);
    } else {
        p.fillRect(QRect(QPoint(int(layerX() - qreal(s.width()) / 2.), 
                                int(layerY() - qreal(s.height()) / 2.)), s), color());
    }

    p.setOpacity(op);
    GfxCanvasItem::paint(p);
}

GfxCanvasClip::GfxCanvasClip(GfxCanvasItem *parent, ClipType t)
: GfxCanvasItem(parent), _width(this), _height(this), _type(t)
{
}

GfxValue &GfxCanvasClip::width()
{
    return _width;
}

GfxValue &GfxCanvasClip::height()
{
    return _height;
}

const GfxValue &GfxCanvasClip::width() const
{
    return _width;
}

const GfxValue &GfxCanvasClip::height() const
{
    return _height;
}

QRect GfxCanvasClip::boundingRect()
{
    return GfxCanvasItem::boundingRect() & clip();
}

void GfxCanvasClip::paint(GfxPainter &g)
{
    QRect ucr = g.userClipRect();
    QRect cr = clip();

    if(!(_type & Height) && !g.clipRect().isEmpty()) {
        cr.setY(g.clipRect().y());
        cr.setHeight(g.clipRect().height());
    }
    if(!(_type & Width) && !g.clipRect().isEmpty()) {
        cr.setX(g.clipRect().x());
        cr.setWidth(g.clipRect().width());
    }

    if(!ucr.isEmpty())
        cr &= ucr;
    if(!cr.isEmpty()) {
        g.setUserClipRect(cr);
        GfxCanvasItem::paint(g);
        g.setUserClipRect(ucr);
    }
}

QRect GfxCanvasClip::clip() 
{
    return QRect(int(layerX()), int(layerY()), int(_width.value()), int(_height.value()));
}

GfxCanvasReflection::GfxCanvasReflection(const QSize &s, int dx, int dy, GfxCanvasItem *parent)
: GfxCanvasItem(parent), _s(s), _dx(dx), _dy(dy)
{
}

GfxCanvasReflection::GfxCanvasReflection(const QRect &r, GfxCanvasItem *parent)
: GfxCanvasItem(parent), _r(r)
{
    _s = _r.size();
}

QRect GfxCanvasReflection::boundingRect()
{
    return internalBoundingRect() | reflectRect();
}

static uchar reflectFunc(int yy, void *data)
{
    int *vals = (int *)data;
    return 255 - 255 * (yy - vals[0]) / vals[1];
}

QRect GfxCanvasReflection::reflectRect()
{
    QRect rect = _r;
    if(rect.isEmpty()) 
        rect = internalBoundingRect().translated(_dx, _dy);
    return rect;
}

QRect GfxCanvasReflection::internalBoundingRect()
{
    return QRect(QPoint(int(layerX()), int(layerY())), _s);
}

void GfxCanvasReflection::paint(GfxPainter &p)
{
    enum State { Yes, No, Unknown };
    static State blur = Unknown;
    if(blur == Unknown) {
        bool b = QString(getenv("GFX_REFLECTION_NO_BLUR")).isEmpty();
        blur = b?Yes:No;
    }
    int vals[2];
    vals[0] = int(layerY());
    vals[1] = _s.height();
    qreal op = p.opacity();
    p.setOpacity(op * layerVisible());
    p.setHorizontalOpacityFunction(reflectFunc, (void *)vals);
    QRect rect = reflectRect();
    p.drawImageFlipped(int(layerX()), int(layerY()), p.imgRef(rect));
    p.setHorizontalOpacityFunction(0, 0);
    if(blur == Yes && !p.usingQt()) {
        QRect br = internalBoundingRect();
        QImage img = p.img(br);
        Gfx::blur(img, 3);
        p.setOpacity(op);
        GfxCanvasItem::paint(p);
    }
}

GfxCanvasCacheLayer::GfxCanvasCacheLayer(const QSize &s, GfxCanvasItem *parent, bool trans)
: GfxCanvasLayer(parent), _optState(Invalid), _s(s), _quality(this), 
  _refreshOnUpdates(true)
{
    if(trans) {
        _img = QImage(_s, QImage::Format_ARGB32_Premultiplied);
        _img.fill(0);
    } else {
        _img = QImage(_s, QImage::Format_RGB16);
        _img.fill(0xFF000000);
    }
}

GfxValue &GfxCanvasCacheLayer::quality()
{
    return _quality;
}

void GfxCanvasCacheLayer::addChild(GfxCanvasItem *ci)
{
    if(_optState == None) {
        Q_ASSERT(!_addItem);
        _addItem = ci;
        _optState = Add;
    } else {
        _optState = Invalid;
        _addItem = 0;
    }

    GfxCanvasLayer::addChild(ci);
}

QRect GfxCanvasCacheLayer::boundingRect()
{
    return QRect(int(GfxCanvasItem::layerX()), int(GfxCanvasItem::layerY()), _s.width(), _s.height());
}

bool GfxCanvasCacheLayer::refreshOnUpdates() const
{
    return _refreshOnUpdates;
}

void GfxCanvasCacheLayer::setRefreshOnUpdates(bool r)
{
    _refreshOnUpdates = r;
}

void GfxCanvasCacheLayer::paint(GfxPainter &p)
{
    zOrderChildren();
    if(_children.isEmpty())
        return;

    if(Add == _optState && !_children.isEmpty() && 
       _children.last() == _addItem) {
        GfxPainter p(_img);
        if(_addItem->visible().value() != 0.)
            _addItem->paint(p);
    } else if(None != _optState) {
        if(_img.format() == QImage::Format_RGB32)
            _img.fill(0xFF000000);
        else
            _img.fill(0x0);
        GfxPainter p(_img);
        for(int ii = 0; ii < _children.count(); ++ii) {
            GfxCanvasItem *c = _children.at(ii);
            if(c->visible().value() != 0.) 
                c->paint(p);
        }
    }  

    _optState = None;
    _addItem = 0;

    qreal op = p.opacity();
    p.setOpacity(op * GfxCanvasItem::layerVisible() * visible().value());

    qreal s = GfxCanvasItem::layerScale();
    if(1. != s) {
        QMatrix m;
        m.translate(layerX() + _s.width() / 2, layerY() + _s.height() / 2);
        m.scale(s, s);
        m.translate(-_s.width() / 2, -_s.height() / 2);
        p.drawImageTransformed(m, _img, quality().value() != 0.);

    } else {
        p.drawImage(int(GfxCanvasItem::layerX()), 
                    int(GfxCanvasItem::layerY()), 
                    _img);
    }

    p.setOpacity(op);
}

void GfxCanvasCacheLayer::addDirty(GfxCanvasItem *c)
{
    dirty();
    switch(_optState) {
        case None:
            _optState = Invalid;
            break;
        case Invalid:
            break;
        case Add:
            if(c != _addItem) {
                _optState = Invalid;
                _addItem = 0;
            }
            break;
    }
    _dirtyItems.append(c);
}

void GfxCanvasCacheLayer::remDirty(GfxCanvasItem *c)
{
    _dirtyItems.removeAll(c);

    _optState = Invalid;
    _addItem = 0;
}

void GfxCanvasCacheLayer::clean()
{
    for(int ii = 0; ii < _dirtyItems.count(); ++ii)
        _dirtyItems.at(ii)->clean();
    _dirtyItems.clear();
    GfxCanvasLayer::clean();
}

QList<GfxCanvasWindow *> GfxCanvasWindow::_active;
QSet<GfxCanvasWindow *> GfxCanvasWindow::_windows;
GfxCanvasWindow::GfxCanvasWindow(const QSize &s, GfxCanvasItem *parent)
: GfxCanvasItem(parent), _cached(this), _window(this), _s(s), _cache(0),
  _focused(0)
{
    _windows.insert(this);
}

GfxCanvasWindow::~GfxCanvasWindow()
{
    bool wasActive = isActive();
    _windows.remove(this);
    _active.removeAll(this);

    if(wasActive) {
        if(_active.isEmpty()) {
            if(_focused) {
                canvas()->setFocused(0);
            } 
        } else {
            _active.last()->activate();
        }
    }
}

void GfxCanvasWindow::activated()
{
}

void GfxCanvasWindow::deactivated(GfxCanvasWindow *)
{
}

GfxCanvasWindow *GfxCanvasWindow::activeWindow()
{
    if(_active.isEmpty())
        return 0;
    return 
        _active.last();
}

bool GfxCanvasWindow::isActive() const
{
    return activeWindow() == this;
}

GfxCanvasWindow *GfxCanvasWindow::window()
{
    return this;
}

GfxCanvasItem *GfxCanvasWindow::focused()
{
    return _focused;
}

void GfxCanvasWindow::setFocused(GfxCanvasItem *f)
{
    if(isActive())
        canvas()->setFocused(f);
    _focused = f;
}

void GfxCanvasWindow::setFocused(bool focused)
{
    static_cast<GfxCanvasItem *>(this)->setFocused(focused);
}

void GfxCanvasWindow::activate()
{
    if(isActive())
        return;
    _active.removeAll(this);
    GfxCanvasWindow *last = activeWindow();
    _active.append(this);
    if(last)
        last->deactivated(this);
    activated();
    canvas()->setFocused(focused());
}

void GfxCanvasWindow::deactivate()
{
    bool wasActive = isActive();

    if(wasActive) {
        if(_active.count() == 1) {
            if(_focused) {
                canvas()->setFocused(0);
            } 
        } else {
            _active.at(_active.count() - 2)->activate();
        }
    }
    _active.removeAll(this);
}

QStringList GfxCanvasWindow::attributes() const
{
    return _attribute.toList();
}

bool GfxCanvasWindow::attribute(const QString &attr)
{
    return _attribute.contains(attr);
}

void GfxCanvasWindow::setAttribute(const QString &attr)
{
    _attribute.insert(attr);
}

void GfxCanvasWindow::removeAttribute(const QString &attr)
{
    _attribute.remove(attr);
}

GfxCanvasItem *GfxCanvasWindow::windowItem()
{
    if(_cache)
        return _cache;
    else
        return &_window;
}

GfxValue &GfxCanvasWindow::cached()
{
    return _cached;
}

void GfxCanvasWindow::valueChanged(GfxValue *, qreal, qreal newValue)
{
    if(newValue == 0. && _cache) {
        _window.moveToParent(this);
        delete _cache;
        _cache = 0;
    } else if(newValue != 0. && !_cache) {
        _cache = new GfxCanvasCacheLayer(_s, this, false);
        _window.moveToParent(_cache);
    }
}

