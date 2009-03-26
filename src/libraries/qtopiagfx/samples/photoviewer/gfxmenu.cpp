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

#include "gfxmenu.h"
#include <GfxPainter>
#include <QFont>

GfxMenuItem::GfxMenuItem(const QString &s, QObject *parent)
: QObject(parent), _str(s)
{
}

int GfxMenuItem::count() const
{
    return _children.count();
}

GfxMenuItem *GfxMenuItem::item(int idx)
{
    return _children.at(idx);
}

void GfxMenuItem::appendItem(GfxMenuItem *i)
{
    _children.append(i);
}

void GfxMenuItem::prependItem(GfxMenuItem *i)
{
    _children.prepend(i);
}

void GfxMenuItem::insertItem(int idx, GfxMenuItem *i)
{
    _children.insert(idx, i);
}

QString GfxMenuItem::text() const
{
    return _str;
}

class MenuListHighlight : public GfxCanvasListHighlight
{
public:
    MenuListHighlight(GfxCanvasItem *, GfxCanvasRoundedRect *c);

    void hide(GfxCanvasListItem *);
    void show(GfxCanvasListItem *);
    void change(GfxCanvasListItem *,
                GfxCanvasListItem *newItem);

private:
    GfxCanvasRoundedRect *color;
    GfxTimeLine tl;
};

MenuListHighlight::MenuListHighlight(GfxCanvasItem *p, GfxCanvasRoundedRect *c)
: GfxCanvasListHighlight(p, BelowItems), color(c)
{
}

void MenuListHighlight::hide(GfxCanvasListItem *)
{
}

void MenuListHighlight::show(GfxCanvasListItem *i)
{
    color->moveToParent(this);
    tl.clear();
    tl.move(color->x(), i->x().value(), 150);
    tl.move(color->y(), i->y().value(), 150);
}

void MenuListHighlight::change(GfxCanvasListItem *,
                               GfxCanvasListItem *i)
{
    color->moveToParent(this);
    tl.clear();
    tl.move(color->x(), i->x().value(), 150);
    tl.move(color->y(), i->y().value(), 150);
}

class MenuListItem : public GfxCanvasListItem
{
public:
    MenuListItem(GfxMenuItem *item, GfxCanvasItem *parent);
    virtual QSize size() const;
    QRect rect() const;

private:
    GfxMenuItem *_item;
    QImage _img;
};

MenuListItem::MenuListItem(GfxMenuItem *item, GfxCanvasItem *parent)
: GfxCanvasListItem(parent), _item(item)
{
    QString t = _item->text();
    if(!t.isEmpty()) {
        QFont f;
        f.setPointSize(24);
        _img = GfxPainter::string(t, Qt::white, f);

        GfxCanvasImage *i = new GfxCanvasImage(_img, this);
        i->x().setValue(-90 + _img.width() / 2);

        if(item->count()) {
            QImage next = GfxPainter::string(">", Qt::white, f);
            GfxCanvasImage *i2 =new GfxCanvasImage(next, this);
            i2->x().setValue(100 - next.width() / 2 - 3);
        }
    }
}

QSize MenuListItem::size() const
{
    return QSize(int(list()->width().value()), int(_img.height()));
}

QRect MenuListItem::rect() const
{
    return QRect(int(-list()->width().value() / 2), int(-_img.height() / 2),
                 int(list()->width().value()), int(_img.height()));
}

GfxMenu::GfxMenu(GfxCanvasItem *parent)
: GfxCanvasItem(parent), _highlight(0)
{
    _highlight = new GfxCanvasRoundedRect(this);

    _highlight->setColor(QColor(255, 0, 0, 127));
    _highlight->width().setValue(200);
    _highlight->height().setValue(30);
    _highlight->setFilled(true);
    _highlight->setCornerCurve(5);

}

GfxMenuItem *GfxMenu::menu() const
{
    if(_miStack.isEmpty())
        return 0;
    else
        return _miStack.first();;
}

void GfxMenu::setMenu(GfxMenuItem *m)
{
    _listStack << listForItem(m, QPoint());
    list()->x().setValue(0.);
    _miStack << m;
}

void GfxMenu::show()
{
}

GfxCanvasList *GfxMenu::list() const
{
    return _listStack.last();
}

GfxMenuItem *GfxMenu::menuItem() const
{
    return _miStack.last();
}

void GfxMenu::itemactivated()
{
    emit menuItem()->item(list()->current())->activated();
}

GfxEvent GfxMenu::resetEvent()
{
    return GfxEvent(this, this, &GfxMenu::reset);
}

void GfxMenu::reset()
{
    tl.clear();

    _listStack.first()->setCurrent(0);
    _listStack.first()->setActive(true);
    _listStack.first()->x().setValue(0.);

    while(_listStack.count() > 1) {
        delete _listStack.last();
        _listStack.removeLast();
        _miStack.removeLast();
    }
}

void GfxMenu::select()
{
    GfxMenuItem *mi = menuItem()->item(list()->current());
    if(mi->count()) {
        GfxCanvasList *l = listForItem(mi, mapFrom(list()->currentItem(), static_cast<MenuListItem *>(list()->currentItem())->rect().topLeft()));
        tl.move(list()->x(), -240, 300);
        list()->setActive(false);
        tl.move(l->x(), 0, 300);
        _listStack << l;
        list()->setActive(true);
        _miStack << mi;
    } else {
        itemactivated();
    }
}

void GfxMenu::keyReleaseEvent(QKeyEvent *e)
{
    switch(e->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Select:
            break;
        default:
            return;
    }
    e->accept();

    if(e->key() == Qt::Key_Select)
        select();
}

void GfxMenu::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Select:
            break;
        default:
            return;
    }
    e->accept();

    if(e->key() == Qt::Key_Up) {
        int nf = list()->current() - 1;
        if(nf >= 0) list()->setCurrent(nf);
    } else if(e->key() == Qt::Key_Down) {
        int nf = list()->current() + 1;
        if(nf < list()->count()) list()->setCurrent(nf);
    } else if(e->key() == Qt::Key_Right) {
        select();
    } else if(e->key() == Qt::Key_Back ||
              e->key() == Qt::Key_Left) {
        if(_listStack.count() != 1) {
            tl.move(list()->x(), 240, 300);
            list()->setActive(false);
            tl.pause(*list(), 300);
            tl.execute(list()->destroyEvent());
            _listStack.removeLast();
            _miStack.removeLast();
            list()->setActive(true);
            tl.move(list()->x(), 0, 300);
        }
    } else {
        GfxCanvasItem::keyPressEvent(e);
    }
}

GfxCanvasList *GfxMenu::listForItem(GfxMenuItem *item, const QPoint &point)
{
    GfxCanvasList *list = new GfxCanvasList(this);
    list->width().setValue(200);
    list->setHighlight(new MenuListHighlight(this, _highlight));

    QList<GfxCanvasListItem *> items;
    for(int ii = 0; ii < item->count(); ++ii) 
        items << new MenuListItem(item->item(ii), this);
    list->setItems(items);

    int idealHeight = list->idealHeight();
    if(idealHeight < 180) {
        if(point.isNull() || (point.y() + idealHeight) >= 180) {
            list->y().setValue(180 - idealHeight);
            list->height().setValue(list->idealHeight());
        } else {
            list->y().setValue(point.y());
            list->height().setValue(list->idealHeight());
        }

    } else {
        list->height().setValue(180);
    }
    list->x().setValue(240);
    list->setCurrent(0);
    return list;
}

