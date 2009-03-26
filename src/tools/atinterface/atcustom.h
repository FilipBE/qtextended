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

#ifndef ATCUSTOM_H
#define ATCUSTOM_H

#include <custom.h>
#include <version.h>

// Values that can be used to customise manufacturer information.
#ifndef QTOPIA_AT_MANUFACTURER
#define QTOPIA_AT_MANUFACTURER      "QTOPIA PHONE"
#endif
#ifndef QTOPIA_AT_MODEL
#define QTOPIA_AT_MODEL             "QTOPIA PHONE"
#endif
#ifndef QTOPIA_AT_REVISION
#define QTOPIA_AT_REVISION          QTOPIA_VERSION_STR
#endif
#ifndef QTOPIA_AT_SERIAL_NUMBER
#define QTOPIA_AT_SERIAL_NUMBER     "1234567890"
#endif

// Define this to get a strict GSM AT command set with no AT*Q extensions.
//#define QTOPIA_AT_STRICT 1

#endif
