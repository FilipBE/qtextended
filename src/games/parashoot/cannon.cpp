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
#include "cannon.h"
#include "bullet.h"

#include <QFileInfo>
#include <QDir>
#include <QGraphicsScene>
#include <QDebug>

Cannon::Cannon(QGraphicsScene* parent) :
        QObject(NULL),
        AnimatedPixmapItem(parent)
{
    shotsfired=0;
    index = 8;
    cannonx = 0;
    cannony = 0;

    QFileInfo fi(QLatin1String(":image/parashoot/can*.png"));
    foreach (QString entry, QDir(fi.path(), fi.fileName()).entryList())
    {
        cannonarray << QPixmap(fi.path() + "/" + entry);
    }

    setSequence(cannonarray);
    setFrame(index);

    reposition();

    movedir = NoDir;
    moveDelay = 0;
    show();
}

void Cannon::advance(int stage)
{
    if ( stage == 1  && moveDelay-- == 0 ) {
        if (movedir == Left) {
            if (index > 0) {
                setFrame(index-1);
                index--;
            }
        }
        if (movedir == Right) {
            if (index < 16) {
                setFrame(index+1);
                index++;
            }
        }
        moveDelay = 0;
    }
}

void Cannon::pointCannon(Direction dir)
{
    movedir = dir;
    moveDelay = 0;
    advance(1);
    moveDelay = 1;
}

void Cannon::setCoords()
{
   switch(index) {
      case 0: cannonx = barrelxpos-29.0;  cannony = barrelypos-8.0; break;
      case 1: cannonx = barrelxpos-27.0;  cannony = barrelypos-8.0; break;
      case 2: cannonx = barrelxpos-25.0;  cannony = barrelypos-6.0; break;
      case 3: cannonx = barrelxpos-23.0;  cannony = barrelypos-4.0; break;
      case 4: cannonx = barrelxpos-21.0;  cannony = barrelypos-2.0; break;
      case 5: cannonx = barrelxpos-19.0;  cannony = barrelypos; break;
      case 6: cannonx = barrelxpos-15.0;  cannony = barrelypos; break;
      case 7: cannonx = barrelxpos-10.0;  cannony = barrelypos; break;
      case 8: cannonx = barrelxpos;     cannony = barrelypos; break;
      case 9: cannonx = barrelxpos+2.0;   cannony = barrelypos; break;
      case 10: cannonx = barrelxpos+6.0;  cannony = barrelypos; break;
      case 11: cannonx = barrelxpos+8.0;  cannony = barrelypos; break;
      case 12: cannonx = barrelxpos+12.0; cannony = barrelypos-2.0; break;
      case 13: cannonx = barrelxpos+18.0; cannony = barrelypos-4.0; break;
      case 14: cannonx = barrelxpos+22.0; cannony = barrelypos-6.0; break;
      case 15: cannonx = barrelxpos+26.0; cannony = barrelypos-8.0; break;
      case 16: cannonx = barrelxpos+28.0; cannony = barrelypos-8.0; break;
   }
}

qreal Cannon::shootAngle()
{
   switch(index) {
       case 0: return 30.0;
       case 1: return 37.5;
       case 2: return 45.0;
       case 3: return 52.5;
       case 4: return 60.0;
       case 5: return 67.5;
       case 6: return 75.0;
       case 7: return 82.5;
       case 8: return 90.0;
       case 9: return 97.5;
       case 10: return 105.0;
       case 11: return 112.5;
       case 12: return 120.0;
       case 13: return 127.5;
       case 14: return 135.0;
       case 15: return 142.5;
       case 16: return 150.0;
    }
    return 0;
}

void Cannon::shoot()
{
    setCoords();
    if(Bullet::noBullets() < Bullet::bulletLimit())
    {
        Bullet* bullet = new Bullet(scene(), shootAngle(), cannonx, cannony);
        connect(bullet, SIGNAL(score(int)), this, SIGNAL(score(int)));
        shotsfired++;
    }
}

Cannon::~Cannon()
{
}

int Cannon::type() const
{
   return cannon_type;
}

void Cannon::reposition(void)
{
    setPos(scene()->width()/2.0-20.0, scene()->height()-32.0);
    // co ords for barrel of cannon when upright
    barrelypos = scene()->height()-32.0;
    barrelxpos = scene()->width()/2.0;

    setFrame(index);
    setCoords();
}
