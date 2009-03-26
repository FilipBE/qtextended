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
#include <qlog.h>

#ifndef QTOPIALOG_H
#define QTOPIALOG_H

#include <qtopiaglobal.h>

QTOPIABASE_EXPORT void qtopiaLogSemiVolatile( char * );
QTOPIABASE_EXPORT bool qtopiaLogRequested( const char* );
QTOPIABASE_EXPORT bool qtopiaLogEnabled( const char* );
QTOPIABASE_EXPORT bool qtopiaLogOptional( const char* );

#define QTOPIA_LOG_OPTION(x) QLOG_OPTION_SEMI_VOLATILE(x,qtopiaLogRequested(#x),qtopiaLogSemiVolatile)

#include <qtopialog-config.h>

#endif
