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

#ifndef SNAKE_H
#define SNAKE_H

#include "sprites.h"
#include <QObject>

class QGraphicsScene;
class QAnimatedPixmapItem;
class QTimer;

class Snake : public QObject
{
    Q_OBJECT

public:
    Snake(QGraphicsScene*);
    ~Snake();

    void reset();
    void start();
    void stop();
    void increaseSpeed();
    bool go(int newkey);
    void setScore(int amount);
    int getScore() const;

signals:
    void dead();
    void ateMouse();
    void scoreChanged();

private slots:
    void moveSnake();

private:
    void move(SpriteDB::Direction newdir);
    void detectCrash();
    void clear();

    QList<QAnimatedPixmapItem*> snakelist;
    QTimer* autoMoveTimer;
    QGraphicsScene* scene;
    int grow;
    int speed;
    int score;
    SpriteDB::Direction currentdir;
    QList< SpriteDB::Direction > moves;
};

#endif
