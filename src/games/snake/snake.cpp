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

#include "snake.h"
#include "sprites.h"

#include <QTimer>
#include <QGraphicsScene>
#include <QDebug>

Snake::Snake(QGraphicsScene* c)
    : scene(c)
    , score(0)
{
    autoMoveTimer = new QTimer(this);
    autoMoveTimer->setSingleShot(false);
    connect( autoMoveTimer, SIGNAL(timeout()), this, SLOT(moveSnake()) );
    reset();
}

Snake::~Snake()
{
    clear();
}

void Snake::reset()
{
    // Delete the existing sprites
    clear();

    // Set initial speed and direction
    speed = 250;
    currentdir = SpriteDB::right;
    grow = 0;

    // Make new sprites for the head, body and tail
    QList<QPixmap> snakeparts = SpriteDB::spriteCache();

    QAnimatedPixmapItem* head = new QAnimatedPixmapItem(snakeparts, scene);
    QAnimatedPixmapItem* body = new QAnimatedPixmapItem(snakeparts, scene);
    QAnimatedPixmapItem* tail = new QAnimatedPixmapItem(snakeparts, scene);

    head->setFrame(SpriteDB::snakeHead(currentdir));
    body->setFrame(SpriteDB::snakeBody(currentdir, currentdir));
    tail->setFrame(SpriteDB::snakeTail(currentdir));

// TODO: We could randomly place the snake, but would have to make sure
// that it wouldn't immediately run into something, e.g. use scene->collisions()
// to make sure that there's nothing in the first four tiles in front of the head.
    head->setPos(2*SpriteDB::tileSize(), SpriteDB::tileSize());
    body->setPos(SpriteDB::tileSize(), SpriteDB::tileSize());
    tail->setPos(0, SpriteDB::tileSize());

    head->show();
    body->show();
    tail->show();

    snakelist.append(head);
    snakelist.append(body);
    snakelist.append(tail);
}

void Snake::start()
{
    autoMoveTimer->start(speed);
}

void Snake::stop()
{
    autoMoveTimer->stop();
}

void Snake::increaseSpeed()
{
    if (speed > 150)
        speed = speed - 25;
    if(autoMoveTimer->isActive())
        autoMoveTimer->stop();
    autoMoveTimer->start(speed);
}

bool Snake::go(int newkey)
{
    SpriteDB::Direction direction;
    switch (newkey) {
        case Qt::Key_6:
            // FALL THROUGH
        case Qt::Key_Right:
            direction = SpriteDB::right;
            if (currentdir == SpriteDB::left) return true;
            break;
        case Qt::Key_4:
            // FALL THROUGH
        case Qt::Key_Left:
            direction = SpriteDB::left;
            if (currentdir == SpriteDB::right) return true;
            break;
        case Qt::Key_2:
            // FALL THROUGH
        case Qt::Key_Up:
            direction = SpriteDB::up;
            if (currentdir == SpriteDB::down) return true;
            break;
        case Qt::Key_8:
            // FALL THROUGH
        case Qt::Key_Down:
            direction = SpriteDB::down;
            if (currentdir == SpriteDB::up) return true;
            break;
        case Qt::Key_1:
            // Up or left, depending on direction.
            if (currentdir == SpriteDB::left || currentdir == SpriteDB::right)
                direction = SpriteDB::up;
            else
                direction = SpriteDB::left;
            break;
        case Qt::Key_3:
            // Up or right, depending on direction.
            if (currentdir == SpriteDB::left || currentdir == SpriteDB::right)
                direction = SpriteDB::up;
            else
                direction = SpriteDB::right;
            break;
        case Qt::Key_7:
            // Down or left, depending on direction.
            if (currentdir == SpriteDB::left || currentdir == SpriteDB::right)
                direction = SpriteDB::down;
            else
                direction = SpriteDB::left;
            break;
        case Qt::Key_9:
            // Down or right, depending on direction.
            if (currentdir == SpriteDB::left || currentdir == SpriteDB::right)
                direction = SpriteDB::down;
            else
                direction = SpriteDB::right;
            break;
        default:
            return false;
    }
    moves.append( direction );
    return true;
}

void Snake::moveSnake()
{
    if (moves.isEmpty()) {
        move(currentdir);
    } else {
        SpriteDB::Direction direction = moves.takeFirst();

        if (currentdir == direction) {
            move(direction);
        }
        move(direction);
    }
}

void Snake::setScore(int amount)
{
    score += amount;
    emit scoreChanged();
}

int Snake::getScore() const
{
    return score;
}

void Snake::move(SpriteDB::Direction newdir)
{
    // Add a new segment in the same place as the head
    QAnimatedPixmapItem* sprite = new QAnimatedPixmapItem(SpriteDB::spriteCache(), scene );
    sprite->setFrame(SpriteDB::snakeBody(currentdir, newdir));
    sprite->setPos(snakelist.first()->x(), snakelist.first()->y() );
    sprite->show();
    snakelist.insert(1, sprite);

    // Move the head to its new position
    int x = 0;
    int y = 0;
    switch (newdir) {
        case SpriteDB::right: x = SpriteDB::tileSize(); break;
        case SpriteDB::left: x = -SpriteDB::tileSize(); break;
        case SpriteDB::down: y = SpriteDB::tileSize(); break;
        case SpriteDB::up: y = -SpriteDB::tileSize(); break;
    }
    snakelist.first()->moveBy(x, y);

    // Make sure the head is pointing in the right direction
    if (currentdir != newdir) {
        currentdir = newdir;
        snakelist.first()->setFrame(SpriteDB::snakeHead(currentdir));
    }

    // Move the tail along unless the snake is growing
    if (grow <= 0) {
        // Remove the tail (deleting it automatically removes it from the scene).
        delete snakelist.takeLast();

        // Change the new last segment to the tail image with correct direction
        QAnimatedPixmapItem * last = snakelist.last();
        QAnimatedPixmapItem * prev = snakelist[snakelist.size()-2];

        SpriteDB::Direction taildir;
        if ( prev->x() == last->x() ) {  //vertical
            taildir = prev->y() > last->y() ? SpriteDB::down : SpriteDB::up;
        } else {  //horizontal
            taildir = prev->x() > last->x() ? SpriteDB::left : SpriteDB::right;
        }

        snakelist.last()->setFrame(SpriteDB::snakeTail(taildir));
    }
    else
        grow--;

    // Redraw the scene
    scene->update();

    // Decide if the snake collided with a wall or mouse
    detectCrash();
}

void Snake::detectCrash()
{
    QAnimatedPixmapItem* head = snakelist.first();
    QList<QGraphicsItem *> l = head->collidingItems();
    for (QList<QGraphicsItem *>::Iterator it=l.begin(); it!=l.end(); ++it) {
        QGraphicsItem* item = *it;
        // check if snake ate a mouse
        if ( item->type() == mouse_rtti && item->collidesWithItem(head) ) {
            delete item;
            emit ateMouse();
            grow++;   // make the snake grow the next time it moves
            setScore(5);
            return;
        }
        // check if snake hit a wall
        if ( item->type() == wall_rtti && item->collidesWithItem(head) ) {
            emit dead();
            autoMoveTimer->stop();
            return;
        }
    }
    // check if snake hit itself
    for (int i = 3; i < snakelist.count(); i++) {
        if (head->collidesWithItem(snakelist.at(i)) ) {
            emit dead();
            autoMoveTimer->stop();
            return;
        }
    }
}

void Snake::clear()
{
    // Stop moving
    autoMoveTimer->stop();
    // Delete all of the sprites, which will also remove them from the scene
    for (QList<QAnimatedPixmapItem*>::iterator it = snakelist.begin();
         it != snakelist.end();
         ++it) {
        delete *it;
    }
    snakelist.clear();
}

