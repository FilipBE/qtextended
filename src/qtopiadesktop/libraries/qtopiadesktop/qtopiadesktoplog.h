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
#ifndef QTOPIADESKTOPLOG_H
#define QTOPIADESKTOPLOG_H

#include <qdglobal.h>
#include <qlog.h>
#include <trace.h>

QD_EXPORT void qtopiadesktopLogSemiVolatile( char *mem );
QD_EXPORT bool qtopiadesktopLogEnabled( const char *category );

#define QD_LOG_OPTION(x) QLOG_OPTION_SEMI_VOLATILE(x,qtopiadesktopLogEnabled(#x),qtopiadesktopLogSemiVolatile)\
                         TRACE_OPTION(x,qtopiadesktopLogEnabled(#x),qtopiadesktopLogSemiVolatile)\
                         static bool x##_Tracelog_reg = qtopiadesktopLogEnabled(#x);

QD_LOG_OPTION(QDA)       // This is reserved for QtopiaDesktopApplication
QD_LOG_OPTION(PM)        // Plugin Manager logging
QD_LOG_OPTION(I18N)      // I18n-related stuff
QD_LOG_OPTION(UI)        // General UI-related stuff
QD_LOG_OPTION(UI_tray)   // System Tray

// "Generic" plugin LOG (by class)
QD_LOG_OPTION(QDApp)
QD_LOG_OPTION(QDLink)
QD_LOG_OPTION(QDCon)
QD_LOG_OPTION(QDDev)
QD_LOG_OPTION(QDSync)

// For uncategorized tracing
QD_LOG_OPTION(TRACE)

#endif
