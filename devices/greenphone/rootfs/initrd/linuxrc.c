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

#include <unistd.h>

const char bb_applet_name[] = "insmod";

void bb_show_usage (void)
{
}

int main(int argc, char *argv[])
{
    char *i_argv[2];

    i_argv[0] = "/sbin/insmod";
    i_argv[1] = "/tffs.o";

    insmod_main(2, i_argv);

    return 0;
}

