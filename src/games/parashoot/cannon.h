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
#ifndef CANNON_H
#define CANNON_H

#include "animateditem.h"

class Cannon : public QObject, public AnimatedPixmapItem
{
    Q_OBJECT

public:
    Cannon(QGraphicsScene *);   //create cannon
    ~Cannon();          //destroy cannon

    enum Direction{ Left, Right, NoDir };

    void pointCannon(Direction dir);
    void setCoords();
    qreal shootAngle();
    void shoot();
    int type() const;

    int shotsFired() { return shotsfired; };

    void reposition(void);

protected:
    void advance(int stage);

signals:
    void score(int);

private:
    QList<QPixmap> cannonarray;
    int index;
    qreal cannonx;
    qreal cannony;
    qreal barrelxpos;
    qreal barrelypos;
    int moveDelay;
    Direction movedir;
    int shotsfired;
};

#endif
