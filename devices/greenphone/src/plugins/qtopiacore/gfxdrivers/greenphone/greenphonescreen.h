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

#ifndef GREENPHONESCREEN_H
#define GREENPHONESCREEN_H

#include <QLinuxFbScreen>

class GreenphoneScreenPrivate;
class GreenphoneScreen : public QLinuxFbScreen
{
public:
    GreenphoneScreen(int displayId);
    ~GreenphoneScreen();

    void exposeRegion(QRegion region, int windowIndex);
    virtual void addReserved(int id, const QRect &rect);
    virtual void removeReserved(int id);

    int cookie;
private:
    GreenphoneScreenPrivate *d;
};

#endif
