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

#ifndef GFXCANVASLIST_H
#define GFXCANVASLIST_H

#include "gfxcanvas.h"

class GfxCanvasListItem;
class GfxCanvasListHighlight : public GfxCanvasItem
{
public:
    enum HighlightMode { AboveItems, BelowItems };
    GfxCanvasListHighlight(GfxCanvasItem *, HighlightMode = AboveItems);
    virtual ~GfxCanvasListHighlight();

    virtual QRect rectForItem(GfxCanvasListItem *);
    virtual void hide(GfxCanvasListItem *);
    virtual void show(GfxCanvasListItem *);
    virtual void change(GfxCanvasListItem *oldItem,
                        GfxCanvasListItem *newItem);

    HighlightMode highlightMode() const;
private:
    HighlightMode _mode;
};

class GfxCanvasList;
class GfxCanvasListItemGroup;
class GfxCanvasListItem : public GfxCanvasItem, private GfxValueSink
{
public:
    GfxCanvasListItem(GfxCanvasItem *parent);
    GfxCanvasListItem(GfxCanvasListItemGroup *parent);

    virtual ~GfxCanvasListItem();

    bool isOnScreen() const;
    bool isFocused() const;

    virtual void focusIn();
    virtual void focusOut();
    virtual void remove();
    virtual void add();
    virtual void move(const QPoint &);

    virtual void activate();
    virtual void deactivate();

    virtual QPoint point();

    QRect focusRect() const;
    QRect rect();
    virtual QSize focusSize() const;
    virtual QSize size() const;
    void setSize(const QSize &);

    GfxCanvasList *list() const;
    GfxCanvasListItem *previous() const;
    GfxCanvasListItem *next() const;

    GfxValue &raised();

    GfxCanvasListItemGroup *group() const;
    bool isGroup() const;
protected:
    enum Type { Item, Group };
    GfxCanvasListItem(GfxCanvasItem *parent, Type t);
private:
    virtual void valueChanged(GfxValue *, qreal old, qreal newValue);
    friend class GfxCanvasList;
    friend class GfxCanvasListPrivate;
    Type _type;
    bool _focused;
    GfxCanvasList *_p;
    GfxCanvasListItemGroup *_group;
    int _z;
    int _idx;
    int _line;
    GfxValueNotifying _raised;
    QSize _size;
};

class GfxCanvasListItemGroup : public GfxCanvasListItem
{
public:
    GfxCanvasListItemGroup(GfxCanvasItem *parent);

    QList<GfxCanvasListItem *> items() const;

private:
    friend class GfxCanvasListItem;
    QList<GfxCanvasListItem *> _items;
};

class GfxCanvasListStringGroup : public GfxCanvasListItemGroup
{
public:
    GfxCanvasListStringGroup(const QString &, GfxCanvasItem *parent);

    virtual QSize size() const;
    virtual void activate();
    virtual void deactivate();
private:
    GfxTimeLine tl;
    QSize _s;
};



class GfxCanvasListPrivate;
class GfxCanvasList : public QObject, public GfxCanvasItem
{
    Q_OBJECT
public:
    enum Mode { Vertical, Horizontal };
    GfxCanvasList(GfxCanvasItem *parent, Mode = Vertical);
    virtual ~GfxCanvasList();

    enum Spacing { Aligned, Centered };
    void setSpacing(Spacing);
    Spacing spacing() const;

    bool isActive() const;
    void setActive(bool);

    GfxValue &width();
    GfxValue &height();
    const GfxValue &width() const;
    const GfxValue &height() const;

    int idealHeight() const;

    GfxCanvasListItem *currentItem() const;
    int current() const;
    void setCurrent(int idx);
    void setCurrent(GfxCanvasListItem *);

    int count() const;
    GfxCanvasListItem *item(int idx) const;
    void setItems(const QList<GfxCanvasListItem *> &items);
    QList<GfxCanvasListItem *> items() const;
    void setSpacing(int, int);

    void setHighlight(GfxCanvasListHighlight *);
    GfxCanvasListHighlight *highlight() const;

    virtual void keyReleaseEvent(QKeyEvent *);
    virtual void keyPressEvent(QKeyEvent *);

    Mode mode() const;

    enum Position { TopLeft, BottomLeft, TopRight, BottomRight };
    GfxCanvasListItem *positionItem(Position) const;
signals:
    void activated(GfxCanvasListItem *);

private:
    void positionItem(GfxCanvasListItem *, bool, bool, GfxCanvasListItem *&, QPoint &) const;
    void ensureItemOnScreen(GfxCanvasListItem *);
    void setItem(GfxCanvasListItem *, int *, int);
    friend class GfxCanvasListItem;
    GfxCanvasListPrivate *d;
};

#endif
