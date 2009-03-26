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
#ifndef MAN_H
#define MAN_H

#include "animateditem.h"

class Man : public QObject, public AnimatedPixmapItem
{
    Q_OBJECT

public:
   Man (QGraphicsScene *);
   Man (QGraphicsScene *scene, int x, int y);
   ~Man();
   void advance(int phase);
   void setInitialCoords();
   void checkCollision();
   void start();
   void done();
   static int getManCount();
   static void setManCount(int count);
   static int killCount();
   static void setKillCount(int count);
   virtual int type() const;
//   int mancount;

private:
    QList<QPixmap> manarray;
    int dx;
    int dy;
    bool dead;
    int count;
    static int mancount;
    static int killcount;
    static int f;
    bool fromHelicopter;

};

#endif
