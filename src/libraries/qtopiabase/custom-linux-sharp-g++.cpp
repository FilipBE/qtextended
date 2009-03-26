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

#include <qwindowsystem_qws.h>

#include <custom.h>

#include <qfile.h>

#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

QTOPIABASE_EXPORT int qpe_sysBrightnessSteps()
{
    return 4;
}


QTOPIABASE_EXPORT void qpe_setBrightness(int bright)
{
    if ( QFile::exists("/dev/fl") ) {
#define FL_IOCTL_STEP_CONTRAST    100
        int fd = open("/dev/fl", O_WRONLY);
        if (fd >= 0 ) {
            int steps = qpe_sysBrightnessSteps();
            int bl = ( bright * steps + 127 ) / 255;
            if ( bright && !bl ) bl = 1;
            bl = ioctl(fd, FL_IOCTL_STEP_CONTRAST, bl);
            close(fd);
        }
    }
}

#define SoftKey 0x8000

static int buzzer_fd()
{
    static int fd = -1;
    if (fd < 0)
        fd = ::open( "/dev/sharp_buz", O_RDWR|O_NONBLOCK );
    return fd;
}

extern int qtopia_muted;
