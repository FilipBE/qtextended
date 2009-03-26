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

#ifndef PHOTORIVER_H
#define PHOTORIVER_H

#include "gfxcanvas.h"
#include "gfxcanvaslist.h"
#include <GfxTimeLine>

class Photo;
class PhotoRiver : public QObject, public GfxCanvasWindow
{
    Q_OBJECT
public:
    PhotoRiver(GfxCanvasItem *);
    virtual ~PhotoRiver();

    void complete(Photo *);
    void go(Photo *);

protected:
    virtual void timerEvent(QTimerEvent *);
    virtual void keyReleaseEvent(QKeyEvent *);

private slots:
    void itemActivated();

private:
    GfxTimeLine tl;
    QStringList files;
    GfxCanvasClip clip;
    QDateTime datetime;
    GfxCanvasText *date;
    GfxCanvasText *time;

    void go1();
    void go2();
    void go3();
    void go4();
    QList<Photo *> photos[4];
    GfxTimeLine tls[4];
    GfxTimeLineObject objs[4];
    GfxCanvasItem *ref;
    GfxCanvasList list;
    QList<Photo *> catched;
    bool viewing;
    GfxCanvasItem viewItem;
};

#endif
