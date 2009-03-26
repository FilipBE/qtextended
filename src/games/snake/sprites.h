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

#ifndef SPRITES_H
#define SPRITES_H

#include <QGraphicsItem>

const int mouse_rtti = 1500;
const int wall_rtti = 1600;

class QAnimatedPixmapItem : public QGraphicsItem
{
public:
    QAnimatedPixmapItem(const QList<QPixmap> &animation, QGraphicsScene *scene = 0);
    QAnimatedPixmapItem(QGraphicsScene *scene = 0);

    void setFrame(int frame);
    inline int frame() const
    { return currentFrame; }
    inline int frameCount() const
    { return frames.size(); }
    inline QPixmap image(int frame) const
    { return frames.isEmpty() ? QPixmap() : frames.at(frame % frames.size()); }
    inline void setVelocity(qreal xvel, qreal yvel)
    { vx = xvel; vy = yvel; }
    inline qreal xVelocity() const
    { return vx; }
    inline qreal yVelocity() const
    { return vy; }

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    virtual void advance(int phase);

    void setSequence(const QList<QPixmap> &animation);

private:

    int currentFrame;
    QList<QPixmap> frames;
    qreal vx, vy;
};

class SpriteDB
{
public:
    enum Direction {
        left = 0,
        right = 1,
        up = 2,
        down = 3,
    };

    static QList<QPixmap> &spriteCache();

    static int snakeHead(Direction);
    static int snakeBody(Direction olddir, Direction newdir);
    static int snakeTail(Direction);

    static int mouse();

    static int wall(bool east, bool west, bool north, bool south);

    static int ground();

    static int tileSize() { return tilesize; }
private:

    static int tilesize;
    static QList<QPixmap> parts;
};

#endif
