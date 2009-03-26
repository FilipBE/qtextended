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

#include "animateditem.h"

#include <QPainter>
#include <QDebug>

AnimatedPixmapItem::AnimatedPixmapItem(const QList<QPixmap> &animation,
                                       QGraphicsScene *scene)
    : QGraphicsItem(0, scene), currentFrame(0), vx(0), vy(0)
{
    for (int i = 0; i < animation.size(); ++i) {
        QPixmap pixmap = animation.at(i);
        Frame frame;
        frame.pixmap = pixmap;
        frame.boundingRect = pixmap.rect();
        frames << frame;
    }
}

AnimatedPixmapItem::AnimatedPixmapItem(QGraphicsScene *scene)
        : QGraphicsItem( 0, scene ), currentFrame( 0 ), vx( 0 ), vy( 0 )
{
}

void AnimatedPixmapItem::setFrame(int frame)
{
    if (!frames.isEmpty()) {
        prepareGeometryChange();
        currentFrame = frame % frames.size();
    }
}

void AnimatedPixmapItem::advance(int phase)
{
    if (phase == 1 && !frames.isEmpty()) {
        //setFrame(currentFrame + 1);
        if (vx || vy)
            moveBy(vx, vy);
    }
}

QRectF AnimatedPixmapItem::boundingRect() const
{
    if(frames.empty())
        return QRectF();
    else
        return frames.at( currentFrame ).boundingRect;
}

void AnimatedPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/,
                               QWidget * /*widget*/)
{
    if(!frames.empty())
        painter->drawPixmap(0, 0, frames.at(currentFrame).pixmap);
}

void AnimatedPixmapItem::setSequence(const QList<QPixmap> &animation)
{
    frames.clear();
    for ( int i = 0; i < animation.size(); ++i ) {
        QPixmap pixmap = animation.at( i );
        Frame frame;
        frame.pixmap = pixmap;
        frame.boundingRect = pixmap.rect();
        frames << frame;
    }
}
