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

#ifndef PHONEPOWERMANAGER_H
#define PHONEPOWERMANAGER_H

#include "qtopiapowermanager.h"

class PhonePowerManager : public QtopiaPowerManager
{
    enum PowerMode {
        DimLight = 0,
        LightOff = 1,
        Suspend = 2,
    };

public:
    PhonePowerManager();
    virtual ~PhonePowerManager();

    bool save(int level);

    void setIntervals(int* a, int size);


private:
    void forceSuspend();
    bool m_suspendEnabled;
    QPowerStatus powerstatus;
};

#endif
