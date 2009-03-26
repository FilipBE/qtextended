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

#include "simplehighlight.h"

SimpleHighlight::SimpleHighlight(GfxCanvasItem *p)
: GfxCanvasListHighlight(p)
{
    c = new GfxCanvasRoundedRect(this);
    c->setCornerCurve(15);
    c->setLineWidth(5);

    visible().setValue(0.);

}

QRect SimpleHighlight::rectForItem(GfxCanvasListItem *i)
{
    return QRect(int(i->x().value()) - 60, int(i->y().value()) - 60, 120, 120);
}

void SimpleHighlight::hide(GfxCanvasListItem *)
{
    tl.clear();
    tl.move(visible(), 0., 150);
}

void SimpleHighlight::show(GfxCanvasListItem *i)
{
    tl.clear();
    x().setValue(i->x().value());
    y().setValue(i->y().value());

    c->width().setValue(70);
    c->height().setValue(70);
    tl.pause(visible(), 600);
    tl.sync();
    tl.move(visible(), 1., 150);
    tl.move(c->width(), 120, 150);
    tl.move(c->height(), 120, 150);

}

void SimpleHighlight::change(GfxCanvasListItem *,
                         GfxCanvasListItem *newItem)
{
    tl.reset(x());
    tl.reset(y());
    tl.move(x(), newItem->x().value(), 150);
    tl.move(y(), newItem->y().value(), 150);
}

