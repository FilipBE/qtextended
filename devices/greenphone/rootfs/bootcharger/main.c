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
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>

#include <omega_sysdevs.h>

#define KBDDEVICE   "/dev/tty0"
#define DETDEVICE   "/dev/omega_detect"

#define POWER_KEYCODE 0x16

#define CHARGE_ANIMATION    "/usr/share/bootcharger/charging.gif"
#define FULLY_CHARGED       "/usr/share/bootcharger/fully_charged.gif"
#define POWERING_OFF        "/usr/share/bootcharger/powering_off.gif"

static pid_t animationPid = 0;

void stopAnimation(int now)
{
    if (animationPid) {
        if (now)
            kill(animationPid, SIGKILL);
        else
            kill(animationPid, SIGTERM);

        waitpid(animationPid, NULL, 0);

        animationPid = 0;
    }
}

void startAnimation(const char *file)
{
    stopAnimation(1);

    animationPid = fork();
    if (animationPid == 0) {
        // animation process
        execl("/bin/addtext", "/bin/addtext", "i", CHARGE_ANIMATION, NULL);
    }
}

void showImage(const char *file)
{
    stopAnimation(1);

    int bufferSize = 16 + strlen(file);
    char *buffer = malloc(bufferSize);

    snprintf(buffer, bufferSize, "/bin/addtext i %s", file);
    system(buffer);
    free(buffer);
}

int main(int argc, char *argv[])
{
    int detFd = open(DETDEVICE, O_RDONLY|O_NDELAY, 0);
    if (detFd < 0) {
        perror(DETDEVICE);
        return 1;
    }

    detect_device_t detectData[26];

    if (read(detFd, detectData, 104) < 104) {
        perror(DETDEVICE);
        return 1;
    }

    if (detectData[BOOTSRC_DETECT].status != OMEGABOOT_CHG) {
        // Boot not triggered by charger insertion
        // Continue normal boot
        return 0;
    } else if (detectData[CHARGER_DETECT].status != DEV_ON) {
        // Boot triggered by charger, but has since been removed
        // Power off
        system("/sbin/poweroff");
        return 0;
    }

    int kbdFd = open(KBDDEVICE, O_RDONLY, 0);
    if (kbdFd < 0) {
        perror(KBDDEVICE);
        return 1;
    }

    startAnimation(CHARGE_ANIMATION);

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

    unsigned char code = 0;

    while (1) {
        fd_set set;

        FD_ZERO(&set);
        FD_SET(kbdFd, &set);
        FD_SET(detFd, &set);

        // check if user has pressed any key
        int ret = select(FD_SETSIZE, &set, NULL, NULL, NULL);
        if ( ret < 0 ) 
            // continue boot if error
            break;
        if (FD_ISSET(kbdFd, &set)) {
            read(kbdFd, &code, 1);
            int boot = 0;
            switch (code & 0x7f)
            {
                // Keypad
                case 0x2e: //Qt::Key_0
                case 0x02: //Qt::Key_1
                case 0x03: //Qt::Key_2
                case 0x04: //Qt::Key_3
                case 0x05: //Qt::Key_4
                case 0x06: //Qt::Key_5
                case 0x08: //Qt::Key_6
                case 0x09: //Qt::Key_7
                case 0x0a: //Qt::Key_8
                case 0x0b: //Qt::Key_9

                case 0x1e: //Qt::Key_Asterisk;
                case 0x20: //Qt::Key_NumberSign;

                // Navigation+
                case 0x32: //Qt::Key_Call
                case 0x16: //Qt::Key_Hangup
                case 0x19: //Qt::Key_Context1
                case 0x26: //Qt::Key_Back
                case 0x12: //Qt::Key_Up
                case 0x24: //Qt::Key_Down
                case 0x21: //Qt::Key_Left
                case 0x17: //Qt::Key_Right
                case 0x22: //Qt::Key_Select
                    // stop animation now
                    boot = 1;
                    stopAnimation(1);

                    break;

                //don't react on volume keys
                //case 0x07: //Qt::Key_VolumeUp
                //case 0x14: //Qt::Key_VolumeDown

                //don't react on right hand side keys of device
                //case 0x31: //Qt::Key_F7   // Key +
                //case 0x30: //Qt::Key_F8   // Key -

                // don't react on Camera
                //case 0x23: //Qt::Key_F4

                // Lock key on top of device
                //case 0x36: //Qt::Key_F29

                // Key on headphones
                //case 0x33: //Qt::Key_F28
                default:
                       break;
            }

            if ( boot ) 
                // continue boot
                break;
        } 
        if (FD_ISSET(detFd, &set)) {
            if (read(detFd, detectData, 104) == 104) {
                // check if charger has been removed
                if (detectData[CHARGER_DETECT].status == DEV_OFF) {
                    // stop animation soon
                    stopAnimation(0);
                    showImage(POWERING_OFF);

                    system("/sbin/poweroff");

                    break;
                }

                // check battery level
                if (detectData[LOWPOWER_DETECT].status == NORMAL_POWER &&
                    detectData[LOWPOWER_DETECT].extra >= 4) {
                    stopAnimation(0);
                    showImage(FULLY_CHARGED);
                } else {
                    if (!animationPid)
                        startAnimation(CHARGE_ANIMATION);
                }
            }
        }
    }


    ioctl(kbdFd, KDSKBMODE, K_XLATE);
    tcsetattr(kbdFd, TCSANOW, &origTermData);
    close(kbdFd);

    close(detFd);

    return 0;
}

