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

#ifndef GFXMENU_H
#define GFXMENU_H

#include "gfxcanvas.h"
#include "gfxcanvaslist.h"
#include <QObject>

class GfxMenuItem : public QObject
{
    Q_OBJECT
public:
    GfxMenuItem(const QString &, QObject *parent);

    int count() const;
    GfxMenuItem *item(int idx);

    void appendItem(GfxMenuItem *);
    void prependItem(GfxMenuItem *);
    void insertItem(int idx, GfxMenuItem *);

    QString text() const;

signals:
    void activated();

private:
    friend class GfxMenu;
    QString _str;
    QList<GfxMenuItem *> _children;
};

class GfxMenu : public QObject, public GfxCanvasItem
{
    Q_OBJECT
public:
    GfxMenu(GfxCanvasItem *);

    GfxMenuItem *menu() const;
    void setMenu(GfxMenuItem *);

    void show();

    virtual void keyPressEvent(QKeyEvent *);
    virtual void keyReleaseEvent(QKeyEvent *);

    GfxEvent resetEvent();
    void reset();

private:
    void select();
    void itemactivated();
    GfxCanvasList *list() const;
    GfxMenuItem *menuItem() const;

    GfxCanvasRoundedRect *_highlight;
    QList<GfxCanvasList *> _listStack;
    QList<GfxMenuItem *> _miStack;
    GfxTimeLine tl;
    GfxCanvasList *listForItem(GfxMenuItem *, const QPoint &);
};

#endif
