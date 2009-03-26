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
#include <sys/ioctl.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int read;

    if (argc == 3)
        read = 1;
    else if (argc == 4)
        read = 0;
    else
    {
        printf("Usage: %s device ioctl [param]\n", argv[0]);
        return 1;
    }

    unsigned int encoded_ioctl = strtoul(argv[2], NULL, 0);

    // decode ioctl
    unsigned char ioctl_number = encoded_ioctl & 0xff;
    unsigned char ioctl_type = (encoded_ioctl >> 8) & 0xff;
    unsigned short ioctl_size = (encoded_ioctl >> 16) & 0x3fff;
    unsigned char ioctl_direction = (encoded_ioctl >> 30) & 0x03;

    if (ioctl_direction & 0x01)
        printf("write ioctl %d.%d %d bytes\n", ioctl_type, ioctl_number, ioctl_size);
    else if (ioctl_direction & 0x02)
        printf("read ioctl %d.%d %d bytes\n", ioctl_type, ioctl_number, ioctl_size);
    else
        printf("ioctl %d.%d %d bytes\n", ioctl_type, ioctl_number, ioctl_size);

    unsigned int param = 0xA5A5;

    int fd = open(argv[1], O_RDWR);
    if (fd < 0)
    {
        perror(argv[1]);
        return 1;
    }

    int ret;
    if (read)
    {
        ret = ioctl(fd, encoded_ioctl, &param);
        if (ret < 0)
        {
            perror("IOCTL_read");
            return 1;
        }
        printf("0x%0X\n", param);
    }
    else
    {
        if (argv[3][0] == 'p')
        {
            printf("writing as pointer\n");
            param = strtoul(argv[3]+1, NULL, 0);
            ret = ioctl(fd, encoded_ioctl, &param);
        }
        else
        {
            param = strtoul(argv[3], NULL, 0);
            ret = ioctl(fd, encoded_ioctl, param);
        }

        if (ret < 0)
        {
            perror("IOCTL_write");
            return 1;
        }
    }

    close(fd);

    return 0;
}

