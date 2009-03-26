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

#ifndef TAGDIALOG_H
#define TAGDIALOG_H

#include <QObject>
#include "gfxcanvas.h"
#include "gfxcanvaslist.h"
#include "textedit.h"

class TagHighlight;
class TagDialog : public QObject, public GfxCanvasItem
{
    Q_OBJECT
public:
    TagDialog(GfxCanvasItem *parent);

    virtual void keyPressEvent(QKeyEvent *);
    virtual void keyReleaseEvent(QKeyEvent *);

private slots:
    void textChanged(const QString &t);
    void activated(GfxCanvasListItem *i);
private:
    QList<GfxCanvasListItem *> allItems;
    QList<GfxCanvasListItem *> curItems;

    GfxCanvasList list;
    TextEdit te;
    GfxCanvasRoundedRect color;
    TagHighlight *highlight;
    GfxTimeLine tl;
};

#endif
