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

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/kd.h>

#include <stdio.h>

#define KBDDEVICE   "/dev/tty0"

int main(int argc, char *argv[])
{
    int kbdFd = open(KBDDEVICE, O_RDONLY, 0);
    if (kbdFd < 0) {
        return 1;
    }

    struct termios origTermData;
    struct termios termData;

    tcgetattr(kbdFd, &origTermData);
    tcgetattr(kbdFd, &termData);

    ioctl(kbdFd, KDSKBMODE, K_RAW);

    termData.c_iflag = (IGNPAR | IGNBRK) & (~PARMRK) & (~ISTRIP);
    termData.c_oflag = 0;
    termData.c_cflag = CREAD | CS8;
    termData.c_lflag = 0;
    termData.c_cc[VTIME]=0;
    termData.c_cc[VMIN]=1;
    cfsetispeed(&termData, 9600);
    cfsetospeed(&termData, 9600);
    tcsetattr(kbdFd, TCSANOW, &termData);

    unsigned char code;
    int n = read(kbdFd, &code, 1);
    if (n == 1)
        printf("%02x\n", code);

    ioctl(kbdFd, KDSKBMODE, K_XLATE);
    tcsetattr(kbdFd, TCSANOW, &origTermData);
    close(kbdFd);

    return 0;
}

