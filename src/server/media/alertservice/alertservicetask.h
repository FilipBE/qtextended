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

#ifndef ALERTSERVICETASK_H
#define ALERTSERVICETASK_H

#include <qtopiaabstractservice.h>

class AlertService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    AlertService( QObject *parent );
    ~AlertService();

public slots:
    virtual void soundAlert() = 0;
};

class AlertServiceTask : public AlertService
{
Q_OBJECT
public:
    AlertServiceTask();
    virtual void soundAlert();

private slots:
    void playDone();
};

#endif
