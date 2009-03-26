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

#include "qcopimpl.h"

#include <qtopiasxe.h>

#ifdef SINGLE_EXEC
#include <QtopiaApplication>
QTOPIA_ADD_APPLICATION(QTOPIA_TARGET,qcop)
#define MAIN_FUNC main_qcop
#else
#define MAIN_FUNC main
#endif

int doqcopimpl(int argc, char **argv);

QSXE_APP_KEY
int MAIN_FUNC(int argc, char **argv)
{
    QSXE_SET_APP_KEY(argv[0])

    return doqcopimpl(argc, argv);
}

