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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

int openDevice( void );
int openServer( int port );
void writeAll( int fd, const char *buffer, int len );

static const char *device;
static int baudRate;
static int listenFd = -1;
static int connectionFd = -1;
static int deviceFd = -1;

int main(int argc, char *argv[])
{
    const char *colon;
    int port;
    struct stat st;
    char buffer[BUFSIZ];
    int len;

    // Validate the command-line parameters.
    if ( argc > 1 ) {
        // Use the supplied device name.
        device = argv[1];
    } else {
        // Look for QTOPIA_PHONE_DEVICE and use that if set.
        device = getenv( "QTOPIA_PHONE_DEVICE" );
    }
    if ( argc > 2 ) {
        // Use the supplied port number.
        port = atoi( argv[2] );
    } else {
        // Use the default phonesim port.
        port = 12345;
    }
    if ( !device || port <= 0 ) {
        fprintf( stderr, "Usage: %s device[:baudrate] [port]\n", argv[0] );
        return 1;
    }

    // Split the device specification into name and baud rate.
    colon = strchr( device, ':' );
    if ( colon ) {
        baudRate = atoi( colon + 1 );
        char *dev = strdup( device );
        dev[colon - device] = '\0';
        device = dev;
    } else {
        baudRate = 115200;
    }

    // Check to see if the device seems to exist.  We only open
    // it when the connection arrives from the host.
    if ( stat( device, &st ) < 0 ) {
        perror( "stat" );
        return 1;
    }

    // Bind to the server port to wait for incoming connections.
    listenFd = openServer( port );
    if ( listenFd < 0 ) {
        return 1;
    }

    // Enter the select loop and process events as they arrive.
    for (;;) {
        fd_set readSet;
        int maxFd = listenFd;
        FD_ZERO(&readSet);
        FD_SET(listenFd, &readSet);
        if ( connectionFd != -1 ) {
            FD_SET(connectionFd, &readSet);
            if ( connectionFd > maxFd )
                maxFd = connectionFd;
        }
        if ( deviceFd != -1 ) {
            FD_SET(deviceFd, &readSet);
            if ( deviceFd > maxFd )
                maxFd = deviceFd;
        }
        int result = ::select( maxFd + 1, &readSet, (fd_set *)0,
                               (fd_set *)0, (struct timeval *)0 );
        if ( result < 0 ) {
            // Signal interrupts are ok, but everything else should report.
            if ( errno != EINTR )
                perror( "select" );
        } else {
            if ( FD_ISSET(listenFd, &readSet) ) {
                // Accept a new incoming connection.
                if ( connectionFd != -1 ) {
                    // There is an existing connection - kill it because
                    // the host on the other end has probably crashed.
                    ::close( connectionFd );
                }
                connectionFd = ::accept( listenFd, (struct sockaddr *)0, 0 );
                if ( connectionFd < 0 )
                    perror( "accept" );
                deviceFd = openDevice();
                if ( deviceFd < 0 ) {
                    // Abort the connection if device cannot be opened.
                    ::close( connectionFd );
                    connectionFd = -1;
                }
            }
            if ( connectionFd != -1 && FD_ISSET(connectionFd, &readSet) ) {
                // Read data from the connection and write to the device.
                len = ::read( connectionFd, buffer, sizeof(buffer) );
                if ( len == 0 ) {
                    // The connection has closed.
                    ::close( connectionFd );
                    ::close( deviceFd );
                    connectionFd = -1;
                    deviceFd = -1;
                } else if ( len > 0 ) {
                    writeAll( deviceFd, buffer, len );
                }
            }
            if ( deviceFd != -1 && FD_ISSET(deviceFd, &readSet) ) {
                // Read data from the device and write to the connection.
                len = ::read( deviceFd, buffer, sizeof(buffer) );
                if ( len == 0 ) {
                    // The device has closed.
                    ::close( connectionFd );
                    ::close( deviceFd );
                    connectionFd = -1;
                    deviceFd = -1;
                } else if ( len > 0 ) {
                    writeAll( connectionFd, buffer, len );
                }
            }
        }
    }

    return 0;
}

int openDevice(void)
{
    int fd;

    // Must use a non-blocking open to prevent devices such as
    // "/dev/ircomm" and "/dev/ttyS0" from locking up if they
    // aren't presently connected to a remote machine.
    fd = ::open( device, O_RDWR | O_NONBLOCK, 0 );
    if ( fd == -1 ) {
        perror("openDevice");
        return -1;
    }
    if ( ::isatty( fd ) ) {
        // Set the serial port attributes.
        struct termios t;
        speed_t speed;
        ::tcgetattr( fd, &t );
        t.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD);
        t.c_cflag |= (CREAD | CLOCAL | CS8);
        t.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG);
        t.c_iflag &= ~(INPCK | IGNPAR | PARMRK | ISTRIP | IXANY | ICRNL);
        t.c_iflag &= ~(IXON | IXOFF);
        t.c_oflag &= ~(OPOST | OCRNL);
    #ifdef CRTSCTS
        t.c_cflag |= CRTSCTS;
    #endif
        t.c_cc[VMIN] = 0;
        t.c_cc[VINTR] = _POSIX_VDISABLE;
        t.c_cc[VQUIT] = _POSIX_VDISABLE;
        t.c_cc[VSTART] = _POSIX_VDISABLE;
        t.c_cc[VSTOP] = _POSIX_VDISABLE;
        t.c_cc[VSUSP] = _POSIX_VDISABLE;
        switch( baudRate ) {
            case 50:            speed = B50; break;
            case 75:            speed = B75; break;
            case 110:           speed = B110; break;
            case 134:           speed = B134; break;
            case 150:           speed = B150; break;
            case 200:           speed = B200; break;
            case 300:           speed = B300; break;
            case 600:           speed = B600; break;
            case 1200:          speed = B1200; break;
            case 1800:          speed = B1800; break;
            case 2400:          speed = B2400; break;
            case 4800:          speed = B4800; break;
            case 9600:          speed = B9600; break;
            case 19200:         speed = B19200; break;
            case 38400:         speed = B38400; break;
        #ifdef B57600
            case 57600:         speed = B57600; break;
        #endif
        #ifdef B115200
            case 115200:        speed = B115200; break;
        #endif
        #ifdef B230400
            case 230400:        speed = B230400; break;
        #endif
        #ifdef B460800
            case 460800:        speed = B460800; break;
        #endif
        #ifdef B500000
            case 500000:        speed = B500000; break;
        #endif
        #ifdef B576000
            case 576000:        speed = B576000; break;
        #endif
        #ifdef B921600
            case 921600:        speed = B921600; break;
        #endif
        #ifdef B1000000
            case 1000000:       speed = B1000000; break;
        #endif
        #ifdef B1152000
            case 1152000:       speed = B1152000; break;
        #endif
        #ifdef B1500000
            case 1500000:       speed = B1500000; break;
        #endif
        #ifdef B2000000
            case 2000000:       speed = B2000000; break;
        #endif
        #ifdef B2500000
            case 2500000:       speed = B2500000; break;
        #endif
        #ifdef B3000000
            case 3000000:       speed = B3000000; break;
        #endif
        #ifdef B3500000
            case 3500000:       speed = B3500000; break;
        #endif
        #ifdef B4000000
            case 4000000:       speed = B4000000; break;
        #endif
            default:            speed = 9600; break;
        }
        ::cfsetispeed( &t, speed );
        ::cfsetospeed( &t, speed );
        if( ::tcsetattr( fd, TCSAFLUSH, &t ) < 0 )
            perror( "tcsetattr" );
        int status = TIOCM_DTR | TIOCM_RTS;
        ::ioctl( fd, TIOCMBIS, &status );
    }

    return fd;
}

int openServer( int port )
{
    int fd;
    fd = ::socket( AF_INET, SOCK_STREAM, 0 );
    if ( fd == -1 ) {
        perror("socket");
        return -1;
    }
    struct sockaddr_in addr;
    ::memset( &addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons( (unsigned short)port );
    if ( ::bind( fd, (struct sockaddr *)&addr, sizeof(addr) ) < 0 ) {
        perror("bind");
        ::close( fd );
        return -1;
    }
    ::fcntl( fd, F_SETFL, O_NONBLOCK );
    if ( ::listen( fd, 5 ) < 0 ) {
        perror("listen");
        ::close( fd );
        return -1;
    }
    return fd;
}

void writeAll( int fd, const char *buffer, int len )
{
    int temp;
    while ( len > 0 ) {
        temp = ::write( fd, buffer, len );
        if ( temp >= 0 ) {
            len -= temp;
            buffer += temp;
        } else if ( errno != EINTR && errno != EWOULDBLOCK ) {
            // Non-recoverable write error, so abort the connection.
            perror("write");
            ::close( connectionFd );
            ::close( deviceFd );
            connectionFd = -1;
            deviceFd = -1;
            return;
        }
    }
}
