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

#ifndef REPORTERROR_H
#define REPORTERROR_H

#define REPORT_ERROR(code) report_error(code,__FILE__,__LINE__)

enum ErrorCode {
    ERR_UNKNOWN,
    ERR_HELIX,
    ERR_INTERFACE,
    ERR_UNSUPPORTED,
    ERR_UNEXPECTED,
    ERR_TEST
};

void report_error( ErrorCode code, const char * file, int line );

#endif
