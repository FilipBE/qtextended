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

/*!

\extension qtuitest

The qtuitest extension is loaded for all projects when using a test-enabled build.
It makes certain symbols visible which otherwise would not be.

*/

function qtuitest_init()
{
###
    DEFINES*=QTOPIA_TEST_EXTRA_SYMBOLS
    !x11:DEFINES+=QTOPIA_USE_TEST_SLAVE
###
}

