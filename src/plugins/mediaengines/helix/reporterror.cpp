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

#include "reporterror.h"

#include <qtopialog.h>

void report_error( ErrorCode code, const char* file, int line )
{
    switch( code )
    {
    case ERR_HELIX:
        qLog(Media) << file << line << "Helix library error.";
        break;
    case ERR_INTERFACE:
        qLog(Media) << file << line << "Interface error.";
        break;
    case ERR_UNSUPPORTED:
        qLog(Media) << file << line << "Unsupported functionality error.";
        break;
    case ERR_UNEXPECTED:
        qLog(Media) << file << line << "Unexpected error.";
        break;
    case ERR_TEST:
        qLog(Media) << file << line << "Test.";
        break;
    default:
        qLog(Media) << file << line << "Unknown error.";
        break;
    }
}
