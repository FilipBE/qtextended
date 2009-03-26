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

#include "codes.h"
#include "bullet.h"
#include "man.h"
#include "helicopter.h"
#include "interface.h"

#include <QFileInfo>
#include <QDir>
#include <QGraphicsScene>
#include <QtopiaApplication>
#include <math.h>

int Bullet::limit;
int Bullet::shotcount;
int Bullet::nobullets;

Bullet::Bullet(QGraphicsScene* parent, qreal angle, qreal cannonx, qreal cannony) :
        QObject(NULL),
        AnimatedPixmapItem(parent)
{
    QFileInfo fi(QLatin1String(":image/parashoot/bullet*"));
    foreach (QString entry, QDir(fi.path(), fi.fileName()).entryList())
        bulletarray << QPixmap(fi.path() + "/" + entry);

     setSequence(bulletarray);
     if (nobullets < limit) {
        nobullets++;
        setPos(cannonx, cannony);
        dy = 0;
        dx = 0;
        show();
        setXY(angle);
        setVelocity(-dx, -dy);
        ParaShoot::instance()->bang.play();
     } else
        return;
}

void Bullet::setXY(qreal angle)
{
   qreal ang = angle;
   if ( (y() < 0) || (x() < 0) || (y() > scene()->height()) || (x() > scene()->width()) )
        deleteLater();
   else {
        qreal radians = 0;
        radians = ang * 3.14159265/180;
        dx = (cos(radians)) *7;
        dy = (sin(radians)) *7;
   }
}

void Bullet::setLimit(int amount)
{
   limit = amount;
}

void Bullet::setNobullets(int amount)
{
   nobullets = amount;
}

void Bullet::checkCollision()
{
    QList<QGraphicsItem *> l=scene()->items();
    foreach(QGraphicsItem *item, l) {
        if ( (item->type()==man_type) && item->collidesWithItem(this, Qt::IntersectsItemBoundingRect) ) {
            Man* deadman = (Man*)item;
            if (deadman->frame() != 5) return;
            deadman->done();
            emit score(10);
            setShotCount(shotcount+1);
            nobullets--;
            deleteLater();
            return;
        }
        else if ( (item->type()==helicopter_type) && item->collidesWithItem(this, Qt::IntersectsItemBoundingRect) ) {
            Helicopter* deadchopper = (Helicopter*) item;
            deadchopper->done();
            emit score(50);
            setShotCount(shotcount+1);
            nobullets--;
            deleteLater();
            return;
        }
    }
    //check shot is not out of bounds
    if ( (y() < 0) || (x() < 0) ||
        (y() > scene()->height()) ||
        ( x() > scene()->width()))  {
        nobullets--;
        deleteLater();
        return;
    }
}

void Bullet::advance(int phase)
{
   AnimatedPixmapItem::advance(phase);

   if (phase == 0)
         checkCollision();

}

int Bullet::getShotCount()
{
   return shotcount;
}

void Bullet::setShotCount(int amount)
{
   shotcount = amount;
}

Bullet::~Bullet()
{
}

int Bullet::type() const
{
   return bullet_type;
}
