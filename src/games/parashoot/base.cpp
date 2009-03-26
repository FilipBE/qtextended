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
#include "base.h"
#include "man.h"
#include "interface.h"

#include <QFileInfo>
#include <QDir>
#include <QGraphicsScene>
#include <QDebug>
#include <QtopiaApplication>

int damage;

Base::Base(QGraphicsScene* parent) :
    QObject(NULL),
    AnimatedPixmapItem(parent)
{
    QFileInfo fi(QLatin1String(":image/parashoot/b*"));
    foreach (QString entry, QDir(fi.path(), fi.fileName()).entryList())
    {
        if(entry != "bullet.xpm")
            basearray << QPixmap(fi.path() + "/" + entry);
    }

   setSequence(basearray);
   setFrame(0);
   setPos(2, scene()->height()-50);
   setZValue(10);
   show();
   damage = 0;
}

void Base::damageBase()
{
   damage++;

   switch(damage) {
      case 1: setFrame(1); ParaShoot::instance()->ohdear.play(); break;
      case 2: setFrame(2); ParaShoot::instance()->ohdear.play(); break;
      case 3: setFrame(3); ParaShoot::instance()->kaboom.play(); break;
   }
   show();
}

bool Base::baseDestroyed()
{
   return (damage >= 3);
}

Base::~Base()
{
}

int Base::type() const
{
   return base_type;
}

void
Base::reposition(void)
{
    setPos(2, scene()->height()-50);
}
