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

#include "tagdialog.h"
#include <QPainter>

class TagHighlight : public GfxCanvasListHighlight
{
public:
    TagHighlight(GfxCanvasItem *parent)
    : GfxCanvasListHighlight(parent, BelowItems)
    {
        rr = new GfxCanvasRoundedRect(this);
        rr->setColor(QColor(255, 0, 0, 127));
        rr->width().setValue(120);
        rr->height().setValue(30);
        rr->setFilled(true);
        rr->setCornerCurve(5);
    }

    virtual void moveToPoint(GfxCanvasItem *parent, const QPoint &p, const QSize &size)
    {
        tl.complete();
        rr->moveToParent(parent);
        tl.move(rr->x(), p.x(), 150);
        tl.move(rr->y(), p.y(), 150);
        tl.move(rr->width(), size.width(), 150);
        tl.move(rr->height(), size.height(), 150);
    }


    virtual void show(GfxCanvasListItem *i)
    {
        tl.complete();
        rr->moveToParent(this);

        tl.move(rr->x(), i->point().x(), 150);
        tl.move(rr->y(), i->point().y(), 150);
        tl.move(rr->width(), 120, 150);
        tl.move(rr->height(), 30, 150);
    }

    virtual void change(GfxCanvasListItem *,
                        GfxCanvasListItem *i)
    {
        tl.clear();
        rr->moveToParent(this);
        tl.move(rr->x(), i->point().x(), 150);
        tl.move(rr->y(), i->point().y(), 150);
        tl.move(rr->width(), 120, 150);
        tl.move(rr->height(), 30, 150);
    }


private:
    GfxCanvasRoundedRect *rr;
    GfxTimeLine tl;
};

class TagItem : public GfxCanvasListItem
{
public:
    TagItem(const QString &str, GfxCanvasItem *parent)
    : GfxCanvasListItem(parent), state(NotSelected)
    {
        this->str = str;
        if(cross.isNull()) {
            cross = QImage(15, 15, QImage::Format_ARGB32_Premultiplied);
            cross.fill(0);
            QPainter p(&cross);
            p.setRenderHint(QPainter::Antialiasing);
            QPen pen(Qt::white);
            pen.setWidth(2);
            p.setPen(pen);
            p.drawLine(4, 4, 11, 11);
            p.drawLine(11, 4, 4, 11);
        }

        GfxCanvasText *gct = new GfxCanvasText(size(), this);
        QFont f;
        f.setPointSize(18);
        gct->setColor(Qt::white);
        gct->setText(str);
        gct->setFont(f);
        gct->x().setValue(33);
        gct->setAlignmentFlags((Qt::AlignmentFlag)(int)(Qt::AlignLeft | Qt::AlignVCenter));

        GfxCanvasRoundedRect *rr = new GfxCanvasRoundedRect(this);
        rr->x().setValue(-40);
        rr->setColor(Qt::white);
        rr->setCornerCurve(3);
        rr->setLineWidth(1);
        rr->width().setValue(15);
        rr->height().setValue(15);


        img = new GfxCanvasImage(cross, this);
        img->x().setValue(-40);
        img->visible().setValue(0.);
    }

    virtual QSize size() const
    {
        return QSize(120, 30);
    }

    virtual void add()
    {
        tl.move(visible(), 1., 150);
    }
    virtual void remove()
    {
        tl.move(visible(), 0., 150);
    }

    virtual void move(const QPoint &p)
    {
        tl.move(x(), p.x(), 150);
        tl.move(y(), p.y(), 150);
        _p = p;
    }

    virtual void focusIn()
    {
    }

    virtual void focusOut()
    {
    }

    enum SelectionState { NotSelected, Selected, PartiallySelected };
    SelectionState selectionState() const
    {
        return state;
    }

    void setSelectionState(SelectionState s)
    {
        if(s == state)
            return;

        dirty();
        state = s;

        tl.reset(img->visible());
        qreal v = 0.;
        switch(state) {
            case NotSelected:
                v = 0;
                break;
            case Selected:
                v = 1.;
                break;
            case PartiallySelected:
                v = .4;
                break;
        }
        tl.move(img->visible(), v, 150);
    }

    QPoint point() 
    {
        return _p;
    }

    QString string()
    {
        return str;
    }
private:
    QString str;
    static QImage cross;
    SelectionState state;
    GfxTimeLine tl;
    GfxCanvasImage *img;
    QPoint _p;
};

QImage TagItem::cross;

TagDialog::TagDialog(GfxCanvasItem *parent)
: GfxCanvasItem(parent), list(this, GfxCanvasList::Horizontal), color(this),
  te(this), highlight(0)
{
    list.width().setValue(240);
    list.z().setValue(10);
    list.height().setValue(120);
    QList<GfxCanvasListItem *> items;
    items << new TagItem("Richard", &list);
    items << new TagItem("Pyramids", &list);
    items << new TagItem("Nice", &list);
    items << new TagItem("Linda", &list);
    items << new TagItem("Lucy", &list);
    items << new TagItem("Camel", &list);
    items << new TagItem("My Family", &list);
    items << new TagItem("Night", &list);
    items << new TagItem("Egypt", &list);
    items << new TagItem("Nile", &list);
    items << new TagItem("Paris", &list);
    items << new TagItem("Brisbane", &list);
    items << new TagItem("Rome", &list);
    items << new TagItem("Elephant", &list);
    items << new TagItem("Nelly", &list);
    items << new TagItem("Office", &list);

    allItems = items;
    curItems = items;
    list.setSpacing(GfxCanvasList::Aligned);
    list.setItems(items);
    list.setActive(false);
    QObject::connect(&list, SIGNAL(activated(GfxCanvasListItem*)), this, SLOT(activated(GfxCanvasListItem*)));

    highlight = new TagHighlight(&list);
    list.setHighlight(highlight);

    te.setFocused(true);
    te.y().setValue(140);
    te.x().setValue(20);
    te.z().setValue(2);
    QObject::connect(&te, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
    color.setColor(Qt::blue);
    color.setLineWidth(2);
    color.setCornerCurve(5);
    color.height().setValue(30);
    color.width().setValue(230);
    color.x().setValue(120);
    color.y().setValue(140);
    color.z().setValue(1);
    highlight->moveToPoint(color.parent(), QPoint(int(color.x().value()), int(color.y().value())), QSize(int(color.width().value()) + 10, int(color.height().value()) + 10));

}

void TagDialog::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Up) {
        if(list.count()) {
            list.setFocused(true);
            list.setActive(true);
            if(!list.currentItem())
                list.setCurrent(list.positionItem(GfxCanvasList::BottomLeft));
            tl.move(color.visible(), .3, 150);
            tl.move(te.visible(), .3, 150);
        }
        e->accept();
    } else if(e->key() == Qt::Key_Down) {
        te.setFocused(true);
        list.setActive(false);
        highlight->moveToPoint(color.parent(), QPoint(int(color.x().value()), int(color.y().value())), QSize(int(color.width().value()) + 10, int(color.height().value()) + 10));

        tl.move(color.visible(), 1, 150);
        tl.move(te.visible(), 1, 150);
        e->accept();
    } else if(e->key() == Qt::Key_Select) {
        if(te.focused() && !te.text().isEmpty()) {
            allItems << new TagItem(te.text(), &list);
            te.confirmText();
        }
        e->accept();
    }
}

void TagDialog::keyReleaseEvent(QKeyEvent *e)
{
    switch(e->key()) {
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Select:
        e->accept();
        break;
    default:
        break;
    }
}

void TagDialog::textChanged(const QString &t)
{
    QString txt = t.toLower();
    QList<GfxCanvasListItem *> items;
    for(int ii = 0; ii < allItems.count(); ++ii) {
        QString s = static_cast<TagItem *>(allItems.at(ii))->string().toLower();
        if(s.startsWith(txt))
            items << allItems.at(ii);
    }
    if(items != curItems) {
        curItems = items;
        list.setCurrent(-1);
        list.setItems(curItems);
    }
}

void TagDialog::activated(GfxCanvasListItem *i)
{
    TagItem *ti = static_cast<TagItem *>(i);
    ti->setSelectionState((ti->selectionState() == TagItem::NotSelected)?TagItem::Selected:TagItem::NotSelected);
}

