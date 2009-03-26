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

#include "sprites.h"

#include <QPixmap>
#include <QImage>
#include <QPainter>

// TODO: Might want to make this a menu option, or choose based on screen resolution.
// e.g, pick your snake size, (label with images of snakes head + tail)
// can smoothscale 3/4 very nicely with medium snake, giving sizes of 8, 12, 16.
// Warning though, the code is not currently set up for this to be a dynamic change.
int SpriteDB::tilesize = 8;
const QString tilename = ":image/snake/smallsnake";
//int SpriteDB::tilesize = 16;
//const QString tilename = ":image/snake/mediumsnake";

QList<QPixmap> SpriteDB::parts;

// left, right, up, down
static int headKey[4] = { 0, 1, 3, 2 };
static int bodyKey[4][4] = { {8, 8, 12, 10}, {8, 8, 13, 11}, { 11, 10, 9, 9 }, {13, 12, 9, 9 } };
static int tailKey[4] = { 4, 5, 6, 7 };

// left = 1, right = 2, up = 4, down = 8, mask
static int wallKey[16] = {
    27, // 0x00, null
    15, // 0x01, left
    14, // 0x02, right
    29, // 0x03, left, right
    17, // 0x04, up
    21, // 0x05, up left
    18, // 0x06, up right
    25, // 0x07, up left right
    16, // 0x08, down
    19, // 0x09, down left
    20, // 0x0A, down right
    24, // 0x0B, down left right
    28, // 0x0C, down up
    23, // 0x0D, down up left
    22, // 0x0E, down up right
    26  // 0x0F, down up right left
};

QList<QPixmap> &SpriteDB::spriteCache()
{
    if (parts.isEmpty()) {
        QImage image(tilename);
        if (!image.valid(tilesize*2-1,tilesize*7-1)) {
            qFatal("Couldn't create sprites");
        }
        int x = 0;
        int y = 0;

        // snake
        int i = 0;
        for (; i < 14; ++i) {
            parts.append( QPixmap::fromImage(image.copy(x, y, tilesize, tilesize)) );
            if (i % 2) {
                x = 0;
                y += tilesize;
            } else {
                x = tilesize;
            }
        }
        // walls
        x = 2*tilesize;
        y = 0;
        for (; i < 30; ++i) {
            parts.append( QPixmap::fromImage(image.copy(x, y, tilesize, tilesize)) );
            if (i % 2) {
                x = 2*tilesize;
                y += tilesize;
            } else {
                x = 3*tilesize;
            }
        }
        // mouse
        parts.append( QPixmap::fromImage(image.copy(0, tilesize*7, tilesize, tilesize)) );
        // ground
        parts.append( QPixmap::fromImage(image.copy(tilesize, tilesize*7, tilesize, tilesize)) );
    }
    return parts;
}

int SpriteDB::snakeHead(Direction dir)
{
    return headKey[dir];
}

int SpriteDB::snakeBody(Direction olddir, Direction newdir)
{
    return bodyKey[olddir][newdir];
}

int SpriteDB::snakeTail(Direction dir)
{
    return tailKey[dir];
}

int SpriteDB::mouse()
{
    return 30;
}

int SpriteDB::ground()
{
    return 31;
}

int SpriteDB::wall(bool l, bool r, bool u, bool d)
{
    return wallKey[ (l ? 0x01 : 0) | (r ? 0x02 : 0) | (u ? 0x04 : 0) | (d ? 0x08 : 0) ];
}


QAnimatedPixmapItem::QAnimatedPixmapItem( const QList<QPixmap> &animation, QGraphicsScene *scene )
        : QGraphicsItem( 0, scene ), currentFrame( 0 ), vx( 0 ), vy( 0 )
{
    frames = animation;
}

QAnimatedPixmapItem::QAnimatedPixmapItem(QGraphicsScene *scene)
        : QGraphicsItem( 0, scene ), currentFrame( 0 ), vx( 0 ), vy( 0 )
{
}

void QAnimatedPixmapItem::setFrame( int frame )

{
    if ( !frames.isEmpty() ) {
        prepareGeometryChange();
        currentFrame = frame % frames.size();
    }
}

void QAnimatedPixmapItem::advance( int phase )
{
    if ( phase == 1 && !frames.isEmpty() ) {
        setFrame( currentFrame + 1 );

        if ( vx || vy )
            moveBy( vx, vy );
    }
}

QRectF QAnimatedPixmapItem::boundingRect() const
{
    if(frames.empty())
        return QRectF();
    else
        return frames.at( currentFrame ).rect();
}

void QAnimatedPixmapItem::paint( QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/ )
{
    if(!frames.empty())
        painter->drawPixmap( 0, 0, frames.at( currentFrame ) );
}

void QAnimatedPixmapItem::setSequence(const QList<QPixmap> &animation)
{
    frames = animation;
}
