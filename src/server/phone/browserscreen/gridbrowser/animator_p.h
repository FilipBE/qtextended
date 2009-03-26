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

#ifndef ANIMATOR_P_H
#define ANIMATOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGlobal>

class QPainter;
class QRectF;
class QGraphicsRectItem;
class Renderer;
class SelectedItem;
class GridItem;
class QPixmap;

class Animator
{
public:
    virtual ~Animator() {}

    virtual void animate(QPainter *,SelectedItem *,qreal percent) = 0;

    virtual void initFromGridItem(GridItem *);

protected:
    void draw(QPainter *,SelectedItem *,int w,int h);
    void draw(QPainter *,const QPixmap &,QGraphicsRectItem *,int w,int h);

private:
    void draw(QPainter *,Renderer *,QGraphicsRectItem *,int w,int h);

    QRectF renderingBounds(QGraphicsRectItem *item,int w,int h);
};

#endif
