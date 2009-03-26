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

#include "softkeybar.h"
#include <QFont>
#include <GfxPainter>

SoftKeyBar::SoftKeyBar(const QSize &s, GfxCanvasItem *parent)
: GfxCanvasItem(parent), _s(s), left(0), middle(0), right(0),
  lc(Left, this), mc(Middle, this), rc(Right, this)
{
    QObject::connect(canvas(), SIGNAL(focusChanged(GfxCanvasItem*)), this, SLOT(focusChanged()));
    keyBars.insert(this);
}

SoftKeyBar::~SoftKeyBar()
{
    keyBars.remove(this);
}

void SoftKeyBar::change(Key key)
{
    GfxCanvasImage ** ptr = 0;
    QString *str = 0;
    int x = 0;
    switch(key) {
        case Left:
            ptr = &left;
            x = _s.width() / 6;
            str = &nleft;
            break;
        case Right:
            ptr = &right;
            x = 5 * _s.width() / 6;
            str = &nright;
            break;
        case Middle:
            x = _s.width() / 2;
            ptr = &middle;
            str = &nmiddle;
            break;
    };

    if(*ptr)
        delete *ptr;
    *ptr = 0;

    QImage img;
    if(!str->isEmpty()) {
        QFont f;
        if(key == Middle)
            f.setPointSize(28);
        else
            f.setPointSize(20);
        img = GfxPainter::string(*str, Qt::white, f);
        str->clear();

        *ptr = new GfxCanvasImage(img, this);
        (*ptr)->quality().setValue(1.);
        (*ptr)->x().setValue(x);
        (*ptr)->scale().setValue(0.6);
        tl.move((*ptr)->scale(), 1., 150);
        (*ptr)->visible().setValue(0.0);
        tl.move((*ptr)->visible(), 1., 150);
    }
}

void SoftKeyBar::setLabel(Key key, const QString &str)
{
    GfxCanvasImage ** ptr = 0;
    GfxValue *val = 0;
    QString *s = 0;
    QString *cur = 0;
    switch(key) {
        case Left:
            ptr = &left;
            s = &nleft;
            val = &lc;
            cur = &lleft;
            break;
        case Right:
            ptr = &right;
            s = &nright;
            val = &rc;
            cur = &lright;
            break;
        case Middle:
            ptr = &middle;
            s = &nmiddle;
            val = &mc;
            cur = &lmiddle;
            break;
    };

    if(str == *cur)
        return;
    *cur = str;

    if(!s->isEmpty()) {
        *s = str;
    } else if(!*ptr) {
        *s = str;
        tl.reset(*val);
        tl.set(*val, 2.);
    } else {
        *s = str;
        tl.reset((*ptr)->scale());
        tl.reset(*val);
        tl.move((*ptr)->visible(), 0., 150);
        tl.move((*ptr)->scale(), 0.6, 150);
        tl.pause(*val, 150);
        tl.set(*val, 2.);
    }
}

void SoftKeyBar::release(Key key)
{
    GfxCanvasItem *item  = 0;
    switch(key) {
        case Left:
            item = left;
            break;
        case Right:
            item = right;
            break;
        case Middle:
            item = middle;
            break;
    };

    if(!item)
        return;

    tl.reset(item->scale());
    tl.move(item->scale(), 1.0, 150);
}

void SoftKeyBar::press(Key key)
{
    GfxCanvasItem *item  = 0;
    switch(key) {
        case Left:
            item = left;
            break;
        case Right:
            item = right;
            break;
        case Middle:
            item = middle;
            break;
    };

    if(!item)
        return;

    tl.reset(item->scale());
    tl.move(item->scale(), 0.8, 150);
}

void SoftKeyBar::focusChanged()
{
    GfxCanvasItem *f = canvas()->focused();

    QString keys[3];

    while(f) {
        QHash<GfxCanvasItem *, QStringList>::ConstIterator iter = 
            keyMap.find(f);
        if(iter != keyMap.end()) {
            if(keys[0].isNull()) keys[0] = (*iter)[0];
            if(keys[1].isNull()) keys[1] = (*iter)[1];
            if(keys[2].isNull()) keys[2] = (*iter)[2];
        }
        f = f->parent();
    }

    setLabel(Left, keys[0]);
    setLabel(Middle, keys[1]);
    setLabel(Right, keys[2]);
}

QHash<GfxCanvasItem *, QStringList> SoftKeyBar::keyMap;
QSet<SoftKeyBar *> SoftKeyBar::keyBars;

void SoftKeyBar::setLabel(GfxCanvasItem *item, Key k, const QString &str)
{
    QHash<GfxCanvasItem *, QStringList>::Iterator iter = keyMap.find(item);
    if(iter == keyMap.end()) {
        QStringList l;
        l << QString() << QString() << QString();
        iter = keyMap.insert(item, l);
    }

    (*iter)[k] = str;
    GfxCanvas *c = item->canvas();
    foreach(SoftKeyBar *bar, keyBars) {
        if(bar->canvas() == c)
            bar->focusChanged();
    }
}

void SoftKeyBar::remove(GfxCanvasItem *item)
{
    keyMap.remove(item);
    GfxCanvas *c = item->canvas();
    foreach(SoftKeyBar *bar, keyBars) {
        if(bar->canvas() == c)
            bar->focusChanged();
    }
}

