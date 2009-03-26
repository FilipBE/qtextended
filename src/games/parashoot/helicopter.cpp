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

#include "helicopter.h"
#include "man.h"
#include "codes.h"

#include <QFileInfo>
#include <QDir>
#include <QGraphicsScene>

static QList<Helicopter *> all;
int Helicopter::killcount = 0;

Helicopter::Helicopter(QGraphicsScene* parent) :
    QObject(NULL),
    AnimatedPixmapItem(parent),
    chikachika(":sound/parashoot/aland01")
{
    all.append(this);
    hits = 0;
    QFileInfo fi(QLatin1String(":image/parashoot/helicopter000*"));
    foreach (QString entry, QDir(fi.path(), fi.fileName()).entryList())
        helicopterarray << QPixmap(fi.path() + "/" + entry);

    setSequence(helicopterarray);
    setPos(scene()->width(), 5);
    setVelocity(-2, 0);
    chikachika.setLoops( -1 );
    chikachika.play();
    show();
}

Helicopter::~Helicopter()
{
    all.removeAll(this);
}

int fr = 0;

void Helicopter::advance(int phase)
{
   AnimatedPixmapItem::advance(phase);
   if (phase == 0) {
        if (frame() == 3) {
            deleteLater();
            return;
        }

        if (hits >= 2) {
            setFrame(3);
        } else {
            setFrame(fr%3);
            fr++;
            checkCollision();
        }
   }
}

void Helicopter::checkCollision()
{
    if (x() == 5 || x() == 6) {
        //setVelocity(0, 0);
        dropman();
    }
}

void Helicopter::dropman()
{
    (void)new Man(scene(), 15, 25);
    (void)new Man(scene(), 35, 25);
    takeOff();
}

void Helicopter::done()
{
    hits++;
    if(hits==2)
        setKillCount(killCount()+1);
}

void Helicopter::takeOff()
{
    setVelocity(-1, 0);
}

int Helicopter::type() const
{
    return helicopter_type;
}

void Helicopter::silenceAll()
{
    for (QList<Helicopter*>::const_iterator it = all.begin();
         it != all.end(); ++it) {
        (*it)->chikachika.stop();
    }
}

void Helicopter::deleteAll()
{
    foreach(Helicopter* heli, all)
        delete heli;
    all.clear();
}

int Helicopter::killCount()
{
    return killcount;
}

void Helicopter::setKillCount(int count)
{
    killcount = count;
}
