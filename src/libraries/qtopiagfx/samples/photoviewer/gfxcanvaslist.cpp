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

#include "gfxcanvaslist.h"
#include <GfxTimeLine>
#include <QFont>
#include <GfxPainter>

// Take FOCUS_SHIFT_TIME milliseconds when a focus change requires the list to
// move to ensure that the focused item is still visible
#define FOCUS_SHIFT_TIME 150

struct GfxCanvasListPrivate
{
    GfxCanvasListPrivate(GfxCanvasList *l, GfxCanvasList::Mode);

    void layout();
    int layoutVert(QList<GfxCanvasListItem *>,
                   int width, int currentY);
    int layoutHoriz(QList<GfxCanvasListItem *>,
                    int height, int currentX);

    GfxCanvasList *q;
    GfxCanvasClip clip;
    GfxCanvasItem surface;

    QSet<GfxCanvasListItem *> oldItems;
    QSet<GfxCanvasListItem *> newItems;
    QList<GfxCanvasListItem *> items;
    GfxCanvasListHighlight *highlight;

    int leftMargin;
    int rightMargin;
    int topMargin;
    int bottomMargin;
    int horizontalSpacing;
    int verticalSpacing;

    GfxCanvasListItem *current;
    bool active;
    bool activeUnset;

    GfxTimeLine tl;

    int idealHeight;

    GfxCanvasList::Mode mode;
    GfxCanvasList::Spacing spacing;
};

GfxCanvasListHighlight::GfxCanvasListHighlight(GfxCanvasItem *parent, HighlightMode mode)
: GfxCanvasItem(parent), _mode(mode)
{
}

GfxCanvasListHighlight::HighlightMode GfxCanvasListHighlight::highlightMode() const
{
    return _mode;
}

GfxCanvasListHighlight::~GfxCanvasListHighlight()
{
}

QRect GfxCanvasListHighlight::rectForItem(GfxCanvasListItem *)
{
    return QRect();
}

void GfxCanvasListHighlight::hide(GfxCanvasListItem *)
{
}

void GfxCanvasListHighlight::show(GfxCanvasListItem *)
{
}

void GfxCanvasListHighlight::change(GfxCanvasListItem *,
                                    GfxCanvasListItem *)
{
}

GfxCanvasListItem::GfxCanvasListItem(GfxCanvasItem *parent, Type t)
: GfxCanvasItem(parent), _type(t), _focused(false), _p(0), _group(0), 
  _z(0), _idx(-1), _line(-1), _raised(this)
{
}

GfxCanvasListItem::GfxCanvasListItem(GfxCanvasItem *parent)
: GfxCanvasItem(parent), _type(Item), _focused(false), _p(0), _group(0),
  _z(0), _idx(-1), _line(-1), _raised(this)
{
}

GfxCanvasListItem::GfxCanvasListItem(GfxCanvasListItemGroup *parent)
: GfxCanvasItem(parent), _type(Item), _focused(false), _p(0), _group(parent), 
  _z(0), _idx(-1), _line(-1), _raised(this)
{
    Q_ASSERT(_group);
    _group->_items.append(this);
}

GfxCanvasListItem::~GfxCanvasListItem()
{
    if(_group)
        _group->_items.removeAll(this);
}

bool GfxCanvasListItem::isOnScreen() const
{
    if(!_p)
        return false;

    QSize nsize = size();
    QRect r(QPoint(int(x().value()) - nsize.width() / 2,
                   int(y().value()) - nsize.height() / 2),
            nsize);
    r = r.translated(0, int(_p->d->surface.y().value()));

    QRect clip(0, 0, int(_p->width().value()), int(_p->height().value()));

    return !QRect(r & clip).isEmpty();
}

bool GfxCanvasListItem::isFocused() const
{
    return _focused;
}

void GfxCanvasListItem::focusIn()
{
}

void GfxCanvasListItem::focusOut()
{
}

void GfxCanvasListItem::remove()
{
    visible().setValue(0.);
}

void GfxCanvasListItem::add()
{
    visible().setValue(1.);
}

void GfxCanvasListItem::move(const QPoint &p)
{
    x().setValue(p.x());
    y().setValue(p.y());
}

void GfxCanvasListItem::activate()
{
}

void GfxCanvasListItem::deactivate()
{
}

QPoint GfxCanvasListItem::point()
{
    return QPoint(int(x().value()), int(y().value()));
}

QRect GfxCanvasListItem::focusRect() const
{
    QSize s = focusSize();
    QRect r(QPoint(int(x().value()) - s.width() / 2, int(y().value()) - s.height() / 2), s);
    return r;
}

QRect GfxCanvasListItem::rect()
{
    QSize s = size();
    QRect r(QPoint(int(x().value()) - s.width() / 2, int(y().value()) - s.height() / 2), s);
    return r;
}

QSize GfxCanvasListItem::focusSize() const
{
    return size();
}

QSize GfxCanvasListItem::size() const
{
    if(_size.isNull())
        return QSize(1, 1);
    else
        return _size;
}

void GfxCanvasListItem::setSize(const QSize &s)
{
    _size = s;
}

GfxCanvasList *GfxCanvasListItem::list() const
{
    return _p;
}

GfxValue &GfxCanvasListItem::raised()
{
    return _raised;
}

GfxCanvasListItemGroup *GfxCanvasListItem::group() const
{
    return _group;
}

bool GfxCanvasListItem::isGroup() const
{
    return _type == Group;
}

void GfxCanvasListItem::valueChanged(GfxValue *, qreal old, qreal newValue)
{
    if(old == 0. && newValue != 0.) {
        z().setValue(0.5f);
    } else if(old != 0. && newValue == 0.) {
        z().setValue(_z);
    }
}

GfxCanvasListItemGroup::GfxCanvasListItemGroup(GfxCanvasItem *parent)
: GfxCanvasListItem(parent, Group)
{
}

QList<GfxCanvasListItem *> GfxCanvasListItemGroup::items() const
{
    return _items;
}

int GfxCanvasListPrivate::layoutHoriz(QList<GfxCanvasListItem *> items, 
                                      int height, int currentX)
{
    int line = 0;
    while(!items.isEmpty()) {
        int vert = 0;
        int maxHoriz = 0;

        QList<GfxCanvasListItem *> layoutLine;

        while(!items.isEmpty()) {
            GfxCanvasListItem *item = items.first();
            if(item->isGroup() && !layoutLine.isEmpty())
                break;

            QSize s = item->size();
            int newVert = vert;
            if(!layoutLine.isEmpty())
                newVert += verticalSpacing;
            newVert += s.height();
            if(!layoutLine.isEmpty() && newVert > height) {
                break;
            } else {
                maxHoriz = qMax(maxHoriz, s.width());
                vert = newVert;
                layoutLine << item;
                items.removeFirst();
                if(item->isGroup())
                    break;
            }
        }

        int centerLine = currentX + maxHoriz / 2;
        currentX += maxHoriz + horizontalSpacing;

        int vertConsumed = vert - 
                            (layoutLine.count() - 1) * verticalSpacing;
        int vertSpacing = (height - vertConsumed) / layoutLine.count();
        if(spacing == GfxCanvasList::Aligned)
            vertSpacing = verticalSpacing;

        int currentY = topMargin + vertSpacing / 2;
        for(int ii = 0; ii < layoutLine.count(); ++ii) {
            GfxCanvasListItem *item = layoutLine.at(ii);
            QSize s = item->size();

            QPoint p(centerLine, currentY + s.height() / 2);
            currentY += s.height() + vertSpacing + verticalSpacing;

            item->_line = line;
            item->add();
            item->move(p);
            oldItems.remove(item);
            newItems.insert(item);
        }

        if(layoutLine.count() == 1 && 
           layoutLine.at(0)->isGroup()) {
            currentX = layoutHoriz(static_cast<GfxCanvasListItemGroup *>(layoutLine.at(0))->items(), height, currentX);
        }
        line++;
    }

    return currentX;
}


int GfxCanvasListPrivate::layoutVert(QList<GfxCanvasListItem *> items, 
                                     int width, int currentY)
{
    int line = 0;
    while(!items.isEmpty()) {
        int horiz = 0;
        int maxVert = 0;

        QList<GfxCanvasListItem *> layoutLine;

        while(!items.isEmpty()) {
            GfxCanvasListItem *item = items.first();
            if(item->isGroup() && !layoutLine.isEmpty())
                break;

            QSize s = item->size();
            int newHoriz = horiz;
            if(!layoutLine.isEmpty())
                newHoriz += horizontalSpacing;
            newHoriz += s.width();
            if(!layoutLine.isEmpty() && newHoriz > width) {
                break;
            } else {
                maxVert = qMax(maxVert, s.height());
                horiz = newHoriz;
                layoutLine << item;
                items.removeFirst();
                if(item->isGroup())
                    break;
            }
        }

        int centerLine = currentY + maxVert / 2;
        currentY += maxVert + verticalSpacing;

        int horizConsumed = horiz - 
                            (layoutLine.count() - 1) * horizontalSpacing;
        int horizSpacing = (width - horizConsumed) / layoutLine.count();
        if(spacing == GfxCanvasList::Aligned)
            horizSpacing = horizontalSpacing;

        int currentX = leftMargin + horizSpacing / 2;
        for(int ii = 0; ii < layoutLine.count(); ++ii) {
            GfxCanvasListItem *item = layoutLine.at(ii);
            QSize s = item->size();

            QPoint p(currentX + s.width() / 2, centerLine);
            currentX += s.width() + horizSpacing + horizontalSpacing;

            item->_line = line;
            item->add();
            item->move(p);
            oldItems.remove(item);
            newItems.insert(item);
        }

        if(layoutLine.count() == 1 && 
           layoutLine.at(0)->isGroup()) {
            currentY = layoutVert(static_cast<GfxCanvasListItemGroup *>(layoutLine.at(0))->items(), width, currentY);
        }
        line++;
    }

    return currentY;
}

void GfxCanvasListPrivate::layout()
{
    if(mode == GfxCanvasList::Vertical) {
        int width = int(q->width().value()) - leftMargin - rightMargin;
        if(width <= 0) {
            qWarning() << "GfxCanvasList: Cannot layout in zero width list";
            return;
        }

        int currentY = layoutVert(items, width, 0);
        idealHeight = qMax(0, currentY - verticalSpacing + bottomMargin);
    } else {
        int height = int(q->height().value()) - topMargin - bottomMargin;
        if(height <= 0) {
            qWarning() << "GfxCanvasList: Cannot layout in zero height list";
            return;
        }

        int currentX = layoutHoriz(items, height, 0);
        idealHeight = qMax(0, currentX - horizontalSpacing + rightMargin);
    }
}

GfxCanvasListPrivate::GfxCanvasListPrivate(GfxCanvasList *l, GfxCanvasList::Mode m)
: q(l), clip(q, (m==GfxCanvasList::Vertical)?GfxCanvasClip::Height:GfxCanvasClip::Width), surface(&clip), highlight(0), leftMargin(0), rightMargin(0),
  topMargin(0), bottomMargin(0), horizontalSpacing(0), verticalSpacing(0), 
  current(0), active(false), activeUnset(true), mode(m), 
  spacing(GfxCanvasList::Centered)
{
}

GfxCanvasList::GfxCanvasList(GfxCanvasItem *parent, Mode mode)
: GfxCanvasItem(parent), d(new GfxCanvasListPrivate(this, mode))
{
}

GfxCanvasList::~GfxCanvasList()
{
    delete d; d = 0;
}

void GfxCanvasList::setSpacing(Spacing s)
{
    if(s != d->spacing) {
        d->spacing = s;
        setItems(d->items);
    }
}

GfxCanvasList::Spacing GfxCanvasList::spacing() const
{
    return d->spacing;
}

bool GfxCanvasList::isActive() const
{
    return d->active;
}

void GfxCanvasList::setActive(bool a)
{
    if(d->active == a && !d->activeUnset)
        return;

    d->activeUnset = false;
    d->active = a;

    for(int ii = 0; ii < d->items.count(); ++ii) {
        if(a)
            d->items.at(ii)->activate();
        else
            d->items.at(ii)->deactivate();
    }

    if(d->highlight && currentItem()) {
        if(a)
            d->highlight->show(currentItem());
        else
            d->highlight->hide(currentItem());
    }
}

GfxValue &GfxCanvasList::width()
{
    return d->clip.width();
}

GfxValue &GfxCanvasList::height()
{
    return d->clip.height();
}

int GfxCanvasList::idealHeight() const
{
    return d->idealHeight;
}

const GfxValue &GfxCanvasList::width() const
{
    return d->clip.width();
}

const GfxValue &GfxCanvasList::height() const
{
    return d->clip.height();
}

int GfxCanvasList::count() const
{
    return d->items.count();
}

QList<GfxCanvasListItem *> GfxCanvasList::items() const
{
    return d->items;
}

void GfxCanvasList::setItem(GfxCanvasListItem *item, int *n, int idx)
{
    item->setParent(&d->surface);

    item->z().setValue(-*n);
    item->_z = -*n;
    item->_idx = idx;
    item->_p = this;
    (*n)++;

    if(item->isGroup()) {
        QList<GfxCanvasListItem *> items = static_cast<GfxCanvasListItemGroup *>(item)->items();
    for(int ii = 0; ii < items.count(); ++ii) 
        setItem(items.at(ii), n, ii);
    }
}

void GfxCanvasList::setItems(const QList<GfxCanvasListItem *> &items)
{
    d->items = items;
    int n = 0;
    for(int ii = 0; ii < items.count(); ++ii) 
        setItem(items.at(ii), &n, ii);
    d->layout();

    for(QSet<GfxCanvasListItem *>::Iterator iter = d->oldItems.begin();
            iter != d->oldItems.end();
            ++iter)
        (*iter)->remove();

    if(d->current) {
        if(d->newItems.contains(d->current)) {
            if(d->highlight) 
                if(d->active)
                    d->highlight->change(d->current, d->current);
                else
                    d->highlight->hide(d->current);
        } else {
            if(d->highlight)
                d->highlight->hide(d->current);
            d->current = 0;
        }
    } 

    if(d->current) {
        ensureItemOnScreen(d->current);
    } else if(d->items.count()) {
        ensureItemOnScreen(d->items.first());
    } else {
        ensureItemOnScreen(0);
    }


    d->oldItems = d->newItems;
    d->newItems.clear();
}

void GfxCanvasList::setSpacing(int horiz, int vert)
{
    d->horizontalSpacing = horiz;
    d->verticalSpacing = vert;
    d->layout();
}

int GfxCanvasList::current() const
{
    if(!d->current)
        return -1;
    if(d->current->_group)
        return d->current->_group->_idx;
    else
        return d->current->_idx;
}

void GfxCanvasList::ensureItemOnScreen(GfxCanvasListItem *foc)
{
    if(!foc) {
        d->surface.x().setValue(0);
        d->surface.y().setValue(0);
    } else {
        QSize nsize = foc->size();
        QSize fsize = foc->focusSize();
        QSize size(qMax(nsize.width(), fsize.width()), 
                   qMax(nsize.height(), fsize.height()));

        QRect r(QPoint(int(foc->point().x()) - size.width() / 2,
                       int(foc->point().y()) - size.height() / 2),
                size);
        if(d->highlight)
            r |= d->highlight->rectForItem(foc);
        if(foc->_line == 0 && foc->_group && mode() == Vertical)
            r |= foc->_group->focusRect();

        r = r.translated(int(d->surface.x().value()), 
                         int(d->surface.y().value()));

        int diff = 0;
        if(mode() == Vertical) {
            int listheight = int(height().value());
            if(r.top() < 0)
                diff = -r.top();
            else if(r.bottom() > listheight && listheight)
                diff = listheight - r.bottom();

            if(diff) {
                d->tl.reset(d->surface.y());
                d->tl.moveBy(d->surface.y(), diff, FOCUS_SHIFT_TIME);
            }
        } else {
            int listwidth = int(width().value());
            if(r.left() < 0) 
                diff = -r.left();
            else if(r.right() > listwidth)
                diff = listwidth - r.right();

            if(diff) {
                d->tl.reset(d->surface.x());
                d->tl.moveBy(d->surface.x(), diff, FOCUS_SHIFT_TIME);
            }
        }
    }
}

void GfxCanvasList::setCurrent(GfxCanvasListItem *item)
{
    if(item && item->isGroup()) {
        if(static_cast<GfxCanvasListItemGroup *>(item)->items().isEmpty())
            item = 0;
        else
            item = static_cast<GfxCanvasListItemGroup *>(item)->items().first();
    }

    if(item == d->current)
        return;

    GfxCanvasListItem *old = d->current;
    d->current = item;
    GfxCanvasListItem *foc = d->current;

    if(old) old->_focused = false;
    if(foc) foc->_focused = true;
    if(old) old->focusOut();
    if(foc) foc->focusIn();

    if(d->highlight) {
        if(!old) d->highlight->show(foc);
        else if(!foc) d->highlight->hide(old);
        else d->highlight->change(old, foc);
    }

    if(foc) 
        ensureItemOnScreen(foc);
}

void GfxCanvasList::setCurrent(int idx)
{
    GfxCanvasListItem *item = 0;
    if(idx >= 0 && idx < count()) 
        item = d->items.at(idx);
    setCurrent(item);
}

GfxCanvasListItem *GfxCanvasList::currentItem() const
{
    return d->current;
}

GfxCanvasListItem *GfxCanvasList::positionItem(Position pos) const
{
    bool top = pos & TopLeft || pos & TopRight;
    bool left = pos & TopLeft || pos & BottomLeft;

    GfxCanvasListItem *item = 0;
    QPoint p;

    for(int ii = 0; ii < d->items.count(); ++ii) 
        positionItem(d->items.at(ii), top, left, item, p);

    return item;
}

void GfxCanvasList::positionItem(GfxCanvasListItem *item, bool top, bool left, GfxCanvasListItem *&cur, QPoint &curPoint) const
{
    if(item->isGroup()) {
        QList<GfxCanvasListItem *>items = static_cast<GfxCanvasListItemGroup *>(item)->items();
        for(int ii = 0; ii < items.count(); ++ii)
            positionItem(items.at(ii), top, left, cur, curPoint);
    } else {
        if(!cur) {
            cur = item;
            curPoint = item->point();
        } else {
            QPoint p = item->point();
            bool mtop = false;
            bool mleft = false;
            if(top) mtop = p.y() <= curPoint.y();
            else mtop = p.y() >= curPoint.y();

            if(left) mleft = p.x() <= curPoint.x();
            else mleft = p.x() >= curPoint.x();

            if(mtop && mleft) {
                cur = item;
                curPoint = p;
            }
        }
    }
}

GfxCanvasListItem *GfxCanvasList::item(int idx) const
{
    Q_ASSERT(idx < count());
    return d->items.at(idx);
}

void GfxCanvasList::setHighlight(GfxCanvasListHighlight *h)
{
    if(d->highlight && d->current)
        d->highlight->hide(currentItem());
    d->highlight = h;
    if(d->highlight) {
        d->highlight->setParent(&d->surface);
        if(d->highlight->highlightMode() == GfxCanvasListHighlight::AboveItems)
            d->highlight->z().setValue(1.);
        else
            d->highlight->z().setValue(-10000000.);
    }
    if(d->highlight && d->current && d->active)
        d->highlight->show(currentItem());
    else
        d->highlight->hide(currentItem());
}

GfxCanvasListHighlight *GfxCanvasList::highlight() const
{
    return d->highlight;
}

GfxCanvasListItem *GfxCanvasListItem::previous() const
{
    if(!_p)
        return 0;

    if(_idx > 0) {
        if(_group)
            return _group->items().at(_idx - 1);
        else
            return _p->d->items.at(_idx - 1);
    } else {
        if(_group)  {
            if(_group->_idx > 0)
                return _p->d->items.at(_group->_idx - 1);
            else
                return 0;
        } else
            return 0;
    }

    return 0;
}

GfxCanvasListItem *GfxCanvasListItem::next() const
{
    if(!_p)
        return 0;
    int max = _group?_group->items().count():_p->d->items.count();

    if(_idx < (max - 1)) {
        if(_group) 
            return _group->items().at(_idx + 1);
        else
            return _p->d->items.at(_idx + 1);
    } else {
        if(_group) {
            if(_group->_idx < (_p->d->items.count() - 1)) 
                return _p->d->items.at(_group->_idx + 1);
            else
                return 0;
        } else {
            return 0;
        }
    }

    return 0;
}

void GfxCanvasList::keyReleaseEvent(QKeyEvent *e)
{
    switch(e->key()) {
    case Qt::Key_Select:
        break;
    default:
        return;
    }

    e->accept();

    GfxCanvasListItem *item = currentItem();
    if(e->key() == Qt::Key_Select) {
        if(item)
            emit activated(item);
    }
}

GfxCanvasList::Mode GfxCanvasList::mode() const
{
    return d->mode;
}

void GfxCanvasList::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Select:
        break;
    default:
        return;
    }


    if(e->key() == Qt::Key_Select) {
        e->accept();
        return;
    }

    GfxCanvasListItem *item = currentItem();

    if(item == 0)
        return;

    GfxCanvasListItem *newItem = 0;
    if(e->key() == Qt::Key_Left) {

        if(mode() == Vertical) {
            newItem = item->previous();
            while(newItem && newItem->isGroup() && 
                  static_cast<GfxCanvasListItemGroup *>(newItem)->items().isEmpty())
                newItem = newItem->previous();

            if(newItem && newItem->isGroup())
                newItem = static_cast<GfxCanvasListItemGroup *>(newItem)->items().last();
        } else {
            newItem = item->previous();
            while(newItem && 
                  (newItem->isGroup() || newItem->x().value() >= item->x().value())) {
                if(newItem->isGroup() && !static_cast<GfxCanvasListItemGroup *>(newItem)->items().isEmpty())
                    newItem = static_cast<GfxCanvasListItemGroup *>(newItem)->items().last();
                else
                    newItem = newItem->previous();
            }

            if(newItem) {
                GfxCanvasListItem *vertItem = newItem;
                GfxCanvasListItem *prev = newItem;
                qreal x = newItem->x().value();
                while(vertItem && vertItem->x().value() == x && vertItem->y().value() >= item->y().value()) {
                    prev = vertItem;
                    vertItem = vertItem->previous();
                }
                if(prev)
                    newItem = prev;
            }

            Q_ASSERT(!newItem || !newItem->isGroup());
        }

    } else if(e->key() == Qt::Key_Right) {
        if(mode() == Vertical) {
            newItem = item->next();
            while(newItem && newItem->isGroup() && 
                  static_cast<GfxCanvasListItemGroup *>(newItem)->items().isEmpty())
                newItem = newItem->next();

            if(newItem && newItem->isGroup())
                newItem = static_cast<GfxCanvasListItemGroup *>(newItem)->items().first();
        } else {
            newItem = item->next();
            while(newItem && 
                    (newItem->isGroup() || newItem->x().value() <= item->x().value())) {
                if(newItem->isGroup() && !static_cast<GfxCanvasListItemGroup *>(newItem)->items().isEmpty())
                    newItem = static_cast<GfxCanvasListItemGroup *>(newItem)->items().first();
                else
                    newItem = newItem->next();
            }

            if(newItem) {
                GfxCanvasListItem *vertItem = newItem;
                GfxCanvasListItem *prev = newItem;
                qreal x = newItem->x().value();

                while(vertItem && vertItem->x().value() == x && vertItem->y().value() <= item->y().value()) {
                    prev = vertItem;
                    vertItem = vertItem->next();
                }
                if(prev)
                    newItem = prev;
            }

            Q_ASSERT(!newItem || !newItem->isGroup());
        }
    } else if(e->key() == Qt::Key_Up) {
        newItem = item->previous();
        while(newItem && 
              (newItem->isGroup() || newItem->y().value() >= item->y().value())) {
            if(newItem->isGroup() && !static_cast<GfxCanvasListItemGroup *>(newItem)->items().isEmpty())
                newItem = static_cast<GfxCanvasListItemGroup *>(newItem)->items().last();
            else
                newItem = newItem->previous();
        }

        if(newItem) {
            GfxCanvasListItem *horizItem = newItem;
            GfxCanvasListItem *prev = horizItem;
            qreal y = newItem->y().value();
            while(horizItem && horizItem->y().value() == y && horizItem->x().value() >= item->x().value()) {
                prev = horizItem;
                horizItem = horizItem->previous();
            }
            if(prev)
                newItem = prev;
        }

        Q_ASSERT(!newItem || !newItem->isGroup());

    } else if(e->key() == Qt::Key_Down) {

        newItem = item->next();
        while(newItem && 
              (newItem->isGroup() || newItem->y().value() <= item->y().value())) {
            if(newItem->isGroup() && !static_cast<GfxCanvasListItemGroup *>(newItem)->items().isEmpty())
                newItem = static_cast<GfxCanvasListItemGroup *>(newItem)->items().first();
            else
                newItem = newItem->next();
        }

        if(newItem) {
            GfxCanvasListItem *horizItem = newItem;
            GfxCanvasListItem *prev = horizItem;
            qreal y = newItem->y().value();

            while(horizItem && horizItem->y().value() == y && horizItem->x().value() <= item->x().value()) {
                prev = horizItem;
                horizItem = horizItem->next();
            }
            if(prev)
                newItem = prev;
        }
        
        Q_ASSERT(!newItem || !newItem->isGroup());
    }

    if(newItem) {
        e->accept();
        setCurrent(newItem);
    }
}

GfxCanvasListStringGroup::GfxCanvasListStringGroup(const QString &str, 
                                                   GfxCanvasItem *parent)
: GfxCanvasListItemGroup(parent)
{
    QFont f;
    f.setPointSize(24);
    QImage img = GfxPainter::string(str, Qt::white, f);
    (void *)new GfxCanvasImage(img, this);
    _s = img.size();
}

QSize GfxCanvasListStringGroup::size() const
{
    return _s;
}

void GfxCanvasListStringGroup::activate()
{
    tl.move(visible(), 1., 150);
}

void GfxCanvasListStringGroup::deactivate()
{
    tl.move(visible(), 0., 150);
}

