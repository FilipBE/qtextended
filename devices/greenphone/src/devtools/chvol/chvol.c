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

#include "../../../include/soundcard.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

const char *deviceNames[] = {
    "SYSTEM",
    "CALL",
    NULL };
const unsigned int readIoctls[] = {
    SOUND_MIXER_READ_VOLUME,
    SOUND_MIXER_READ_ALTPCM,
    0 };
const unsigned int writeIoctls[] = {
    SOUND_MIXER_WRITE_VOLUME,
    SOUND_MIXER_WRITE_ALTPCM,
    0 };

int main(int argc, char *argv[])
{ 
    if (argc != 3) {
        printf("Usage: chvol [");
        int i = 0;
        while (deviceNames[i] != NULL) {
            printf("%s|", deviceNames[i++]);
        }
        printf("\x08] [absolute volume|+/-volume change]\n");
        return 1;
    }

    int device = -1;
    int i = 0;
    while (deviceNames[i] != NULL) {
        if (strcmp(deviceNames[i], argv[1]) == 0) {
            device = i;
            break;
        }
        i++;
    }

    if (device == -1) {
        fprintf(stderr, "Invalid device\n");
        return 2;
    }

    int relative = argv[2][0] == '+' || argv[2][0] == '-';
    int value = atoi(argv[2]);

    int mixerFd = open("/dev/mixer", O_RDWR);
    if (mixerFd >= 0) {
        unsigned int leftright;
        signed char left;
        signed char right;

        if (relative) {
            ioctl(mixerFd, readIoctls[device], &leftright);

            left = (leftright & 0xff00) >> 8;
            right = (leftright & 0x00ff);
            left = left + value;
            right = right + value;
        } else {
            left = value;
            right = value;
        }

        if (left < 0)
            left = 0;
        if (left > 100)
            left = 100;

        if (right < 0)
            right = 0;
        if (right > 100)
            right = 100;

        leftright = (left << 8) | right;
        ioctl(mixerFd, writeIoctls[device], &leftright);

        if (writeIoctls[device] == SOUND_MIXER_WRITE_ALTPCM) {
            leftright = (100 << 8) | 100;
            ioctl(mixerFd, SOUND_MIXER_WRITE_SPEAKER, &leftright);
        }

        close(mixerFd);
    }

    return 0;
}

