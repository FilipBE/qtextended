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

#ifndef QD_COMMON_H
#define QD_COMMON_H

#include <QtTest>
#include <qdebug.h>
#include <QMessageBox>
#ifdef Q_OS_WINDOWS
#include <windows.h>
#endif
#include <desktopsettings.h>
#include <QDir>

extern bool &qdTraceEnabled();

void (*messagehandler)(QtMsgType type, const char *msg) = 0;
void aBetterMessageHandler(QtMsgType type, const char *msg)
{
    if ( strlen(msg) < 1000 )
        messagehandler( type, msg );
    else {
#ifdef Q_OS_WINDOWS
        // Output for DebugView
        QString fstr(msg);
        OutputDebugString((fstr + "\n").utf16());
#endif
        // Output for console
        fprintf(stdout, "%s\n", msg);
        fflush(stdout);
    }
}

#define QD_COMMON_INIT_TEST_CASE_BODY(enable_trace,wait_for_debugger)\
    /* Ensure our data dir exists */\
    QString homeDir = DesktopSettings::homePath();\
    QDir().mkpath( homeDir );\
\
    /* disable TRACE/LOG output */\
    bool &trace = qdTraceEnabled();\
    trace = enable_trace;\
    messagehandler = qInstallMsgHandler(aBetterMessageHandler);\
\
    if ( wait_for_debugger )\
        QMessageBox::aboutQt( 0 );

#endif
