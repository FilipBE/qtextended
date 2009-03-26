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

#ifndef BASE_H
#define BASE_H

#include "animateditem.h"

class Base : public QObject, public AnimatedPixmapItem
{
    Q_OBJECT

public:
    Base(QGraphicsScene*);
    ~Base();
    void damageBase();
    int type() const;
    static bool baseDestroyed();

    void    reposition(void);

private:
    QList<QPixmap> basearray;
};
#endif
