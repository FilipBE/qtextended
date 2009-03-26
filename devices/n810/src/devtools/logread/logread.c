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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>

/* This value should be less than the value reported by
   "sysctl net.core.wmem_max" on the device */
#define MAX_CIRCULAR_BUFFER     65536

/* This value indicates the largest syslog message we will accept */
#define MAX_SYSLOG_BUFFER       4096

static char circular[MAX_CIRCULAR_BUFFER];
static int circularStart = 0;
static int circularEnd = 0;
static int circularSize = 0;

static void updateCircular(const char *data, int len)
{
    int space;

    /* Remove lines from the buffer to make room */
    while ((MAX_CIRCULAR_BUFFER - circularSize) < (len + 16)) {
        while (circularSize > 0 && circular[circularStart] != '\n') {
            /* Remove everything up to the next '\n' */
            circularStart = (circularStart + 1) % MAX_CIRCULAR_BUFFER;
            --circularSize;
        }
        if (circularSize > 0) {
            /* Remove the '\n' as well */
            circularStart = (circularStart + 1) % MAX_CIRCULAR_BUFFER;
            --circularSize;
        }
        if (!circularSize) {
            circularStart = 0;
            circularEnd = 0;
        }
    }

    /* Add the new data to the buffer */
    circularSize += len;
    space = MAX_CIRCULAR_BUFFER - circularEnd;
    if (space >= len) {
        memcpy(circular + circularEnd, data, len);
        circularEnd += len;
    } else {
        if (space > 0)
            memcpy(circular + circularEnd, data, space);
        memcpy(circular, data + space, len - space);
        circularEnd = len - space;
    }
}

int main(int argc, char *argv[])
{
    int follow = (argc > 1 && !strcmp(argv[1], "-f"));
    int daemon = (argc > 1 && !strcmp(argv[1], "-d"));
    int logFd, otherFd;
    struct sockaddr_un addr;
    struct sockaddr_un otherAddr;
    char buf[MAX_SYSLOG_BUFFER];
    char *buffer = buf;
    int buflen = sizeof(buf);
    int len, posn, level;
    int maxLevel = LOG_NOTICE;
    int sendToClient = 0;

    /* Determine if we need to adjust the logging level we will accept.
       Usage: logread -d -b N */
    if (argc > 3 && !strcmp(argv[2], "-b"))
        maxLevel = atoi(argv[3]);

    /* Bind to the logging socket.  We use /dev/log in server
       mode, because that is where syslog() sends its messages.
       We use /dev/logclient in client mode, and the server
       forwards the messages to that socket as they come in */
    logFd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (logFd < 0) {
        perror("socket");
        return 1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    if (daemon)
        strcpy(addr.sun_path, "/dev/log");
    else
        strcpy(addr.sun_path, "/dev/logclient");
    unlink(addr.sun_path);
    if (bind(logFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(logFd);
        return 1;
    }

    /* The daemon forwards all packets to /dev/logclient, where they
       can be picked up by logread running in client mode */
    if (daemon) {
        otherFd = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (otherFd < 0) {
            perror("socket");
            return 1;
        }
    } else {
        otherFd = -1;
        buffer = circular;
        buflen = sizeof(circular);
    }
    memset(&otherAddr, 0, sizeof(otherAddr));
    otherAddr.sun_family = AF_UNIX;
    if (daemon)
        strcpy(otherAddr.sun_path, "/dev/logclient");
    else
        strcpy(otherAddr.sun_path, "/dev/log");

    /* If we are the client, then request the server to send us data */
    if (!daemon) {
        buffer[0] = (char)(follow ? 0x1B : 0x1C);
        sendto(logFd, buffer, 1, 0,
                (struct sockaddr *)&otherAddr, sizeof(otherAddr));
    }

    /* Process logging packets as they arrive */
    for(;;) {
        len = read(logFd, buffer, buflen);
        if (len < 0) {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            perror("read");
            return 1;
        }
        if (len == 1 && buffer[0] == 0x1B && daemon) {
            /* Request from the client to start sending data */
            sendToClient = 1;
            continue;
        } else if (len == 1 && buffer[0] == 0x1C && daemon) {
            /* Request from the client to send the circular buffer contents */
            if (circularStart < circularEnd) {
                sendto(otherFd, circular + circularStart,
                        circularEnd - circularStart, 0,
                        (struct sockaddr *)&otherAddr, sizeof(otherAddr));
            } else if (circularStart > circularEnd) {
                if (circularStart < MAX_CIRCULAR_BUFFER) {
                    sendto(otherFd, circular + circularStart,
                            MAX_CIRCULAR_BUFFER - circularStart, 0,
                            (struct sockaddr *)&otherAddr, sizeof(otherAddr));
                }
                if (circularEnd > 0) {
                    sendto(otherFd, circular + circularEnd, circularEnd, 0,
                            (struct sockaddr *)&otherAddr, sizeof(otherAddr));
                }
            }
            buffer[0] = (char)0x1D;
            sendto(otherFd, buffer, 1, 0,
                    (struct sockaddr *)&otherAddr, sizeof(otherAddr));
            continue;
        } else if (len == 1 && buffer[0] == 0x1D && !daemon && !follow) {
            /* Indication from the server that circular buffer has been sent */
            break;
        }
        posn = 0;
        if (daemon) {
            /* Make sure that the line is LF-terminated with only one LF */
            while (len > 1 && buffer[len - 1] == '\n' && buffer[len - 2] == '\n')
                --len;
            if (len <= 0 || buffer[len - 1] != '\n') {
                if (len >= buflen)
                    --len;
                buffer[len++] = '\n';
            }

            /* Strip <N> from the front, which indicates the logging level */
            level = 0;
            if (buffer[posn] == '<') {
                ++posn;
                while (posn < len && buffer[posn] != '>') {
                    if (buffer[posn] >= '0' && buffer[posn] <= '9')
                        level = level * 10 + buffer[posn] - '0';
                    ++posn;
                }
                if (posn < len && buffer[posn] == '>') {
                    ++posn;
                } else {
                    posn = 0;
                    level = LOG_NOTICE;
                }
            } else {
                level = LOG_NOTICE;
            }
            level &= LOG_PRIMASK;

            /* Filter out messages whose level is too verbose */
            if (level > maxLevel)
                continue;

            /* Add the data to the circular buffer */
            updateCircular(buffer + posn, len - posn);
        } else {
            /* Client side: write the data to stdout */
            write(1, buffer, len);
        }
        if (otherFd != -1 && sendToClient) {
            if (sendto(otherFd, buffer + posn, len - posn, 0,
                        (struct sockaddr *)&otherAddr, sizeof(otherAddr)) < 0) {
                if (errno != EAGAIN && errno != EINTR)
                    sendToClient = 0;   /* Client probably went away */
            }
        }
    }

    /* Clean up the sockets and exit */
    close(logFd);
    if (otherFd != -1)
        close(otherFd);
    unlink(addr.sun_path);
    return 0;
}
