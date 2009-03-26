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
#include "man.h"
#include "base.h"
#include "interface.h"

#include <QFileInfo>
#include <QDir>
#include <QGraphicsScene>
#include <QTime>
#include <QtopiaApplication>

int Man::mancount = 0;
int Man::killcount = 0;

Man::Man(QGraphicsScene* parent) :
    QObject(NULL),
    AnimatedPixmapItem(parent),
    fromHelicopter(false)
{
    QFileInfo fi(QLatin1String(":image/parashoot/man*"));
    foreach (QString entry, QDir(fi.path(), fi.fileName()).entryList())
        if(entry != "manicon.xpm")
            manarray << QPixmap(fi.path() + "/" + entry);

    setSequence(manarray);
    mancount++;
    dead = false;
    start();
}

Man::Man(QGraphicsScene* parent, int x, int y) :
    QObject(NULL),
    AnimatedPixmapItem(parent),
    fromHelicopter(true)
{
    QFileInfo fi(QLatin1String(":image/parashoot/man*"));
    foreach (QString entry, QDir(fi.path(), fi.fileName()).entryList())
        manarray << QPixmap(fi.path() + "/" + entry);

    setSequence(manarray);
    setPos(x, y);
    setFrame(5);
    setZValue(300);
    show();

    static bool first_time = true;
    if (first_time) {
        first_time = false;
        QTime midnight(0, 0, 0);
        qsrand(midnight.secsTo(QTime::currentTime()) );
    }
    int yfallspeed = 0;
    yfallspeed = (qrand() % 3) + 1;
    setVelocity(0, yfallspeed);

    mancount++;
    dead = false;
}

int Man::f = 0;

void Man::advance(int phase)
{
    AnimatedPixmapItem::advance(phase);
    if (phase == 0) {
        checkCollision();
        if (dead) {
            if (count < 10) {
                setFrame(6);
                setVelocity(0,0);
                count++;
            } else {
                deleteLater();
                return;
            }
        }
        if (y() > scene()->height()-43) {
            setFrame(f%5);
            f++;
            setPos(x(), scene()->height()-26);
            setVelocity(-2.0, 0);
        } else if (xVelocity() == -2.0) {
            //
            // There's been a resize event while this Man has
            // been on the ground.  Move the man back to the
            // new ground location.  This is not neat.
            //
            setPos(x(), scene()->height()-26);
        }
    }
}

void Man::setInitialCoords()
{
    static bool first_time = true;
    if (first_time) {
        first_time = false;
        QTime midnight(0, 0, 0);
        qsrand(midnight.secsTo(QTime::currentTime()) );
    }
    dx = qrand() % int(scene()->width()-16);
    dy = -43;  //height of a man off the screen
}

//check if man has reached the base
void Man::checkCollision()
{
    if ( (x() < 23) && (y() == scene()->height()-26)) {
       QList<QGraphicsItem *> l=scene()->items();
       foreach(QGraphicsItem *item, l) {
             if ( (item->type()== base_type) && (item->collidesWithItem(this)) ) {
                 Base* base = (Base*) item;
                 base->damageBase();
                 start();
             }
       }
    }

    //
    // resize events may cause Man objects to appear
    // outside the screen.  Get rid of them if this
    // is the case.
    //
    if ((x() < 0) || (x() > scene()->width())) {
        deleteLater();
        return;
    }
}

void Man::start()
{
   setInitialCoords();
   setPos(dx, dy);
   setFrame(5);
   setZValue(300);
   show();

   static bool first_time = true;
   if (first_time) {
      first_time = false;
      QTime midnight(0, 0, 0);
      qsrand(midnight.secsTo(QTime::currentTime()) );
   }
   int yfallspeed = 0;
   yfallspeed = (qrand() % 3) + 1;
   setVelocity(0, yfallspeed);
}

void Man::done()
{
    if(!fromHelicopter)
        ParaShoot::instance()->splat1.play();
    else
        ParaShoot::instance()->splat2.play();
    count = 0;
    dead = true;
    setFrame(6);
    setKillCount(killCount()+1);
}

int Man::getManCount()
{
   return mancount;
}

void Man::setManCount(int count)
{
    mancount = count;
}


int Man::type() const
{
   return man_type;
}

Man::~Man()
{
   mancount--;
}

int Man::killCount()
{
    return killcount;
}

void Man::setKillCount(int count)
{
    killcount = count;
}
