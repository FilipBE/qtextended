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

#ifndef MEDIASERVER_H
#define MEDIASERVER_H

#include "qtopiaserverapplication.h"
#include "applicationlauncher.h"

class MediaServerControlTaskPrivate;

class MediaServerControlTask : public SystemShutdownHandler
{
    Q_OBJECT

public:
    MediaServerControlTask();

    bool systemRestart();
    bool systemShutdown();

protected:
    void timerEvent(QTimerEvent *e);

private slots:
    void soundServerExited();
    void killtimeout();
    void applicationTerminated(QString const& name,
                               ApplicationTypeLauncher::TerminationReason reason,
                               bool filtered = false);

private:
    bool doShutdown();

    MediaServerControlTaskPrivate*  d;
};

#endif
