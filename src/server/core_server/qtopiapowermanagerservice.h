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

#ifndef QTOPIAPOWERMANAGERSERVICE_H
#define QTOPIAPOWERMANAGERSERVICE_H

#include <qtopiaabstractservice.h>
#include "qtopiapowermanager.h"

class QtopiaPowerManager;

class QtopiaPowerManagerService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    QtopiaPowerManagerService( QtopiaPowerManager *manager, QObject *parent );
    ~QtopiaPowerManagerService();

public slots:
    void setIntervals( int dim, int lightOff, int suspend );
    void setDefaultIntervals();
    void setBacklight( int brightness );
    void setConstraint(int hint, const QString &app);
    void setActive( bool on);

private:
    QtopiaPowerManager *m_powerManager;
};

#endif
