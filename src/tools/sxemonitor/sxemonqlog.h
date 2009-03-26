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

#include <qstring.h>
#include <qstringlist.h>

#ifndef SXEMONQLOG_H
#define SXEMONQLOG_H

class SxeMonQLog
{
#ifdef TEST_SXEMONITOR
public:
#else
private:
#endif
    enum QLogMessage{
                    LogPathSet,
                    ForceLog,
                    SuccessInit,
                    ForceRot,
                    DelayMsgPrep,
                    MsgDispatched,
                    DelayMsgDispatched,
                    KillApplication,
                    Lockdown,
                    Processing,
                    BreachDetail,
                    BreachDetail2,
                    AppBreach,
                    KernelBreach,
                    LogPathSize,
                    ProcAlreadyTerminated
                    };

    static QString getQLogString( QLogMessage, QStringList args = QStringList() );

    static QString messages[ ProcAlreadyTerminated + 1 ];

friend class SxeMonitor;
};
#endif
