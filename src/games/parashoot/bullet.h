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

#ifndef BULLET_H
#define BULLET_H

#include "animateditem.h"

class Bullet : public QObject, public AnimatedPixmapItem
{
    Q_OBJECT
public:
    Bullet(QGraphicsScene*, qreal angle, qreal cannonx, qreal cannony);
    ~Bullet();
    void setXY(qreal angle);
    void checkCollision();
    void advance(int phase);
    int type() const;
    static int getShotCount();
    static void setShotCount(int amount);
    static void setLimit(int amount);
    static void setNobullets(int amount);
    static int noBullets() { return nobullets; }
    static int bulletLimit() { return limit; }

signals:
    void score(int);

private:
    QList<QPixmap> bulletarray;
    qreal dx;
    qreal dy;
    int damage;

    static int limit;
    static int shotcount;
    static int nobullets;
};
#endif
