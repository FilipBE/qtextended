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

#include "sxemonqlog.h"

/*!
  \internal
  \class SxeMonQLog
    \inpublicgroup QtPkgManagementModule
  \brief produces strings for qLog outputs

  The SxeMonQLog class is a utility class to that provides strings
  for qlog output by sxemonitor.  It's purpose is to improve
  extensibility and ease of testing by centrally grouping all
  qlog outputs in a single location.
*/

/*!
  Produces a string depending on the value of \a m and accompanying
  \a args.  See implementation of SxeMonQLog::messages for the
  message types and appropriate args
*/
QString SxeMonQLog::getQLogString( QLogMessage m, QStringList args )
{
    QString s( messages[m] );
    for( int i = 0; i < args.size(); i++ )
        s = s.arg( args[i] );
    return s;
}

//ensure this is kept in sync with QLogMessage enum
QString SxeMonQLog::messages[]= {
"SxeMonitor::init() Log Path set to: %1", //LogPathSet param %1= log file path
"SxeMonitor::init() Forcing log file creation or rotation", //forceLog
"SxeMonitor:: init() Successful initialization", //SuccessInit
"SxeMonitor::logUpdated() Forced log rotation attempted", //ForceRot
"SxeMonitor::dispatchMessage() delayed message prepared", //delayMsgPrep
"SxeMonitor::dispatchMessage() message dispatched", //MsgDispatched
"SxeMonitor::dispatchMessaged() delayed message dispatched", //DelayMsgDispatched
"SxeMonitor::killApplication() killing all instances of application: %1 %2", //KillApplication param %1 = executable path, %2 = identifier
"SxeMonitor::lockdown() lockdown initiated", //Lockdown
"SxeMonitor::processLogFile() processing log", //Processing
"SxeMonitor::processingLogFile() Detected policy breach \n" //BreachDetail param %1 = errant process pid
                                 "PID: %1 \t Request: %2",  //                   %2 = qcop request
"SxeMonitor::processingLogFile()  Exe Link: %1 Name: %2", //BreachDetail2 param %1 = executable path
                                                         //                    %2 = application name
"SxeMonitor::processingLogFile() Application level breach: %1", //AppBreach %1 = log entry
"SxeMonitor::processingLogFile() Kernel level breach: %1", //KernelBreach %1 = log entry
"SxeMonitor::init() Log Size set to %1", //LogPathSize param %1 = log file size
"SxeMonitor::processingLogFile() Process has already terminated, PID: %1" //ProcAlreadyTerminated param %1 = PID
};
