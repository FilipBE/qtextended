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
#ifndef HELICOPTER_H
#define HELICOPTER_H

#include <QSound>
#include <QList>
#include <QPixmap>
#include "animateditem.h"

class Helicopter : public QObject, public AnimatedPixmapItem
{
    Q_OBJECT

public:
    Helicopter(QGraphicsScene *);
    ~Helicopter();
    void advance(int phase);
    void checkCollision();
    void dropman();
    void takeOff();
    void done();
    static int killCount();
    static void setKillCount(int count);

    static void silenceAll();
    static void deleteAll();

    int type() const;

private:
    QList<QPixmap> helicopterarray;
    int hits;
    QSound chikachika;
    static int killcount;
};

#endif
