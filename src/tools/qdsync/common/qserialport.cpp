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
#ifndef Q_QDOC
#include "qserialport.h"

#include <trace.h>
QD_LOG_OPTION(Modem)
QD_LOG_OPTION(Modem_Status)

#ifdef Q_OS_UNIX
#include <qsocketnotifier.h>
#include <qtimer.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define USE_POSIX_SYSCALLS  1
#define USE_TERMIOS         1
#else
#include "iothread.h"
#include <windows.h>
#include <QCopEnvelope>
static LPCTSTR qstring_to_lpctstr( const QString &string )
{
    LPCTSTR ret;
#ifdef UNICODE
    ret = (const unsigned short *)string.constData();
#else
    ret = string.toLocal8Bit().constData();
#endif
    return ret;
}
#endif

// =====================================================================

namespace QDSync {

class QSerialPortPrivate
{
public:
    QSerialPortPrivate( const QString& device, int rate, bool trackStatus )
    {
        this->device = device;
        this->rate   = rate;
#ifdef Q_OS_UNIX
        this->fd     = -1;
        this->status = 0;
        this->track  = trackStatus;
        this->isTty  = false;
        this->dtr    = true;
        this->rts    = true;
        this->flowControl = false;
        this->keepOpen = true;
        this->notifier = 0;
        this->timer  = 0;
#else
        this->io = 0;
        this->brokenSerial = false;
#endif
    }
    ~QSerialPortPrivate()
    {
#ifdef Q_OS_UNIX
        if ( notifier )
            delete notifier;
        if ( timer )
            delete timer;
#endif
    }

public:
    QString device;
    int     rate;
#ifdef Q_OS_UNIX
    int     fd;
    int     status;
    bool    track;
    bool    isTty;
    bool    dtr;
    bool    rts;
    bool    flowControl;
    bool    keepOpen;
    QSocketNotifier *notifier;
    QTimer *timer;
#else
    IOThread *io;
    bool brokenSerial;
#endif
};

};

/*!
    \class QSerialPort
    \brief The QSerialPort class provides a simple serial device interface.
    \ingroup io
    \ingroup telephony::serial

    This class manages a very simple serial device, which is accessed
    at a specific baud rate with no parity, 8 data bits, and 1 stop bit.
    It is intended for communicating with GSM modems and the like.

    The recommended way to create an instance of this class is to
    call the QSerialPort::create() method.

    \sa QSerialPort::create(), QSerialIODevice
*/

/*!
    Construct a new serial device handler object for the specified
    \a device at the given \a rate.  After construction, it is necessary
    to call QSerialPort::open() to complete initialization.

    If \a trackStatus is true, then the device should attempt to
    track changes in DSR, DCD, and CTS.  This may require the use
    of a regular timeout, which will be detrimental to battery life.

    Status tracking should only be enabled when absolutely required.
    It isn't required for modems that support GSM 07.10 multiplexing,
    as the multiplexing mechanism has its own method of tracking
    status changes that does not require the use of a timeout.

    The device name is usually something like \c{/dev/ttyS0}, but it can
    have the special form \c{sim:hostname}, where \c hostname is the name
    of a host running a phone simulator daemon (usually \c localhost).
    The phone simulator mode is intended for debugging purposes only.
*/
QSerialPort::QSerialPort( const QString& device, int rate, bool trackStatus )
{
    d = new QSerialPortPrivate( device, rate, trackStatus );
}

/*!
    Destruct this serial device.  If the device is currently open,
    it will be closed.
*/
QSerialPort::~QSerialPort()
{
    if ( isOpen() )
        close();
    delete d;
}

/*!
    Open the serial device with the specified \a mode.
*/
bool QSerialPort::open( OpenMode mode )
{
    TRACE(Modem) << "QSerialPort::open";
    if ( isOpen() ) {
        LOG() << "QSerialPort already open";
        return false;
    }

#ifdef Q_OS_UNIX
#ifdef USE_POSIX_SYSCALLS
    if ( d->device.startsWith( "sim:" ) ) {

        // Connect to a phone simulator via a TCP socket.
        d->fd = ::socket( AF_INET, SOCK_STREAM, 0 );
        if ( d->fd == -1 ) {
            LOG() << "could not open socket";
            return false;
        }
        QString host = d->device.mid( 4 );
        int index = host.indexOf(QChar(':'));
        int port = 12345;
        if ( index != -1 ) {
            port = host.mid( index + 1 ).toInt();
            host = host.left( index );
        }
        QByteArray hoststr = host.toLatin1();
        const char *lhost = (const char *)hoststr;
        struct hostent *ent;
        struct sockaddr_in addr;
        ::memset( &addr, 0, sizeof( addr ) );
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr( lhost );
        if ( addr.sin_addr.s_addr == INADDR_NONE ) {
            ent = gethostbyname( lhost );
            if ( !ent ) {
                LOG() << "could not resolve " << lhost;
                ::close( d->fd );
                d->fd = -1;
                return false;
            }
            ::memcpy( &(addr.sin_addr), ent->h_addr, sizeof( addr.sin_addr ) );
        }
        addr.sin_port = htons( (unsigned short)port );
        if ( ::connect( d->fd, (struct sockaddr *)&addr, sizeof(addr) ) < 0 ) {
            LOG() << "could not connect to phone simulator at " << lhost;
            ::close( d->fd );
            d->fd = -1;
            return false;
        }
        ::fcntl( d->fd, F_SETFL, O_NONBLOCK );
        d->isTty = 0;

    } else {

        // Must use a non-blocking open to prevent devices such as
        // "/dev/ircomm" and "/dev/ttyS0" from locking up if they
        // aren't presently connected to a remote machine.
        d->fd = ::open( (const char *)d->device.toLatin1(), O_RDWR | O_NONBLOCK, 0 );
        if ( d->fd == -1 ) {
            LOG() << "QSerialPort " << d->device << " cannot be openned";
            //perror("QSerialPort cannot open");
            return false;
        }
        d->isTty = ::isatty( d->fd );

        LOG() << "Device:" << d->device << "is a tty device:" << (d->isTty ? "True" : "False");

        int fdflags;
        if ((fdflags = fcntl(d->fd, F_GETFL)) == -1 ||
             fcntl(d->fd, F_SETFL, fdflags & ~O_NONBLOCK) < 0)
        {
            //perror("couldn't reset non-blocking mode");
            return false;
        }

        LOG() << "NONBLOCK successfully reset";
    }
#endif
#ifdef USE_TERMIOS
    if ( d->isTty ) {
        // Set the serial port attributes.
        struct termios t;
        speed_t speed;
        ::tcgetattr( d->fd, &t );
        t.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD);
        t.c_cflag |= (CREAD | CLOCAL | CS8);
        t.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG);
        t.c_iflag &= ~(INPCK | IGNPAR | PARMRK | ISTRIP | IXANY | ICRNL);
        t.c_iflag &= ~(IXON | IXOFF);
        t.c_oflag &= ~(OPOST | OCRNL);
    #ifdef CRTSCTS
        if ( d->flowControl )
            t.c_cflag |= CRTSCTS;
        else
            t.c_cflag &= ~CRTSCTS;
    #endif
        t.c_cc[VMIN] = 0;
        t.c_cc[VINTR] = _POSIX_VDISABLE;
        t.c_cc[VQUIT] = _POSIX_VDISABLE;
        t.c_cc[VSTART] = _POSIX_VDISABLE;
        t.c_cc[VSTOP] = _POSIX_VDISABLE;
        t.c_cc[VSUSP] = _POSIX_VDISABLE;
        switch( d->rate ) {
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
        if( ::tcsetattr( d->fd, TCSANOW, &t ) < 0 )
            LOG() << "tcsetattr(" << d->fd << ") errno = " << errno;
        int status = TIOCM_DTR | TIOCM_RTS;
        ::ioctl( d->fd, TIOCMBIS, &status );

        // Use a timer to track status changes.  This should be replaced
        // with a separate thread that uses TIOCMIWAIT instead.
        if ( d->track ) {
            ::ioctl( d->fd, TIOCMGET, &(d->status) );
            d->timer = new QTimer( this );
            connect( d->timer, SIGNAL(timeout()), this, SLOT(statusTimeout()) );
            d->timer->start( 500 );
        } else {
            LOG() << "NOT TRACKING STATUS!";
        }
    }
#endif
#ifdef F_SETFD
    // prevent the fd from being passed across a fork/exec.
    ::fcntl( d->fd, F_SETFD, FD_CLOEXEC );
#endif
    d->notifier = new QSocketNotifier( d->fd, QSocketNotifier::Read, this );
    connect( d->notifier, SIGNAL(activated(int)), this, SLOT(socketReady()) );
#else

    QString port = QString("\\\\.\\%1").arg(d->device);
    HANDLE handle;
    handle = CreateFile(::qstring_to_lpctstr(port),
        GENERIC_READ|GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
        0);
    if ( handle == INVALID_HANDLE_VALUE ) {
        if ( GetLastError() == ERROR_FILE_NOT_FOUND ) {
            //serial port does not exist. Inform user.
            LOG() << "QSerialPort " << d->device << " does not exist";
            QCopEnvelope e("QD/Connection", "clearHint()");
            return false;
        }
        //some other error occurred. Inform user.
        LOG() << "QSerialPort " << d->device << " cannot be opened";
        if ( d->brokenSerial ) {
            QCopEnvelope e("QD/Connection", "setHint(QString,QString)");
            e << QString("Unplug the Greenphone and reconnect it.")
              << QString("bar");
        }
        return false;
    }
    {
        QCopEnvelope e("QD/Connection", "clearHint()");
    }

    if ( !d->brokenSerial ) {
        DCB dcbSerialParams;
        dcbSerialParams.DCBlength = sizeof(DCB);
        if ( ! GetCommState(handle, &dcbSerialParams) ) {
            LOG() << "Error getting state. Continuing anyway.";
        } else {
            dcbSerialParams.BaudRate = d->rate;
            dcbSerialParams.ByteSize = 8;
            dcbSerialParams.StopBits = ONESTOPBIT;
            dcbSerialParams.Parity = NOPARITY;
            if ( ! SetCommState(handle, &dcbSerialParams) ) {
                LOG() << "Error setting state. Continuing anyway.";
            }
        }

        // Turn off timeouts
        COMMTIMEOUTS timeouts;
        timeouts.ReadIntervalTimeout = MAXDWORD;
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.ReadTotalTimeoutConstant = 0;
        timeouts.WriteTotalTimeoutMultiplier = 0;
        timeouts.WriteTotalTimeoutConstant = 0;
        if ( ! SetCommTimeouts(handle, &timeouts) ) {
            LOG() << "Cannot set timeouts. Continuing without them.";
        }
    }

    // Start the read thread
    d->io = new IOThread;
    d->io->brokenSerial = d->brokenSerial;
    connect( d->io, SIGNAL(emitReadyRead()), this, SLOT(internalReadyRead()) );
    connect( d->io, SIGNAL(ioThreadStopped()), this, SIGNAL(disconnected()) );
    connect( d->io, SIGNAL(dsrChanged(bool)), this, SIGNAL(dsrChanged(bool)) );
    d->io->handle = handle;
    d->io->start();
#endif

    QIODevice::setOpenMode( mode | QIODevice::Unbuffered );

    return true;
}

/*!
    Close the serial device.
*/
void QSerialPort::close()
{
    TRACE(Modem) << "QSerialPort::close";
#ifdef Q_OS_UNIX
    if ( d->notifier ) {
        delete d->notifier;
        d->notifier = 0;
    }
    if ( d->timer ) {
        delete d->timer;
        d->timer = 0;
    }
#ifdef USE_POSIX_SYSCALLS
    if ( d->fd != -1 ) {
        ::close( d->fd );
        d->fd = -1;
    }
#endif

#else

    Q_ASSERT(d->io);
    d->io->quit();
    d->io->wait();
    CloseHandle(d->io->handle);
    delete d->io;
    d->io = 0;
#endif
    setOpenMode( NotOpen );
}

/*!
    Returns true if able to flush all buffered data to the physical serial device; otherwise returns false.
*/
bool QSerialPort::flush()
{
#ifdef Q_OS_UNIX
#ifdef USE_TERMIOS
    if ( d->fd != -1 && d->isTty ) {
        ::tcdrain( d->fd );
    }
#endif
#endif
    return true;
}

/*!
    \reimp
*/
bool QSerialPort::waitForReadyRead(int msecs)
{
#ifdef Q_OS_UNIX
#ifdef USE_POSIX_SYSCALLS
    struct timeval tv;
    fd_set readSet;
    if ( d->fd != -1 ) {
        tv.tv_sec = msecs / 1000;
        tv.tv_usec = msecs * 1000;
        FD_ZERO( &readSet );
        FD_SET( d->fd, &readSet );
        return ( select( d->fd + 1, &readSet, 0, 0, &tv ) > 0 );
    } else {
        return false;
    }
#endif
#endif
    return false;
}

/*!
    \reimp
*/
qint64 QSerialPort::bytesAvailable() const
{
    TRACE(Modem) << "QSerialPort::bytesAvailable";
#ifdef Q_OS_UNIX
#ifdef USE_POSIX_SYSCALLS
    if ( d->fd != -1 ) {
        // See comments in Qt4's qsocketlayer_unix.cpp for the reason
        // why this ioctl call sequence is a little bizarre in structure.
        size_t nbytes = 0;
        qint64 available = 0;
        if (::ioctl( d->fd, FIONREAD, (char *) &nbytes ) >= 0)
            available = (qint64) *((int *) &nbytes);
        return available;
    } else {
        return 0;
    }
#endif

#else

    Q_ASSERT(d->io);
    QMutexLocker( &d->io->rbm );
    return d->io->rb.count();
#endif
}

/*!
    \reimp
*/
qint64 QSerialPort::readData( char *data, qint64 maxlen )
{
    TRACE(Modem) << "QSerialPort::readData";

#ifdef Q_OS_UNIX
#ifdef USE_POSIX_SYSCALLS
    if ( d->fd == -1 ) {
        return -1;
    }

    d->notifier->setEnabled( true );

    int result;
    while ( ( result = ::read( d->fd, data, (int)maxlen ) ) < 0 ) {
        if ( errno != EINTR ) {
            if ( errno == EWOULDBLOCK ) {
                return 0;
            }
            LOG() << "read errno = " << errno;
            return -1;
        }
    }

    if ( !result && ( !d->isTty || ( !d->keepOpen && !dsr() ) ) ) {
        // We've received a close notification.  This might happen
        // with the phone simulator if it is shut down before Qtopia.
        // Don't do this for tty devices because there are some systems
        // that return zero when they should be returning EWOULDBLOCK.
        LOG() << "QSerialPort::readData: other end closed the connection" ;
        close();
    }

    return result;
#endif

#else

    Q_ASSERT(d->io);
    QMutexLocker locker( &d->io->rbm );
    qint64 bytes = qMin( (qint64)d->io->rb.count(), maxlen );
    memcpy( data, d->io->rb.constData(), bytes );
    d->io->rb = d->io->rb.mid( bytes );
    return bytes;
#endif
}


/*!
    \reimp
*/
qint64 QSerialPort::writeData( const char *data, qint64 len )
{
    TRACE(Modem) << "QSerialPort::writeData";

#ifdef Q_OS_UNIX
#ifdef USE_POSIX_SYSCALLS
    if ( d->fd == -1 ) {
        return -1;
    }

    int result = 0;
    int temp;
    while ( len > 0 ) {
        temp = ::write( d->fd, data, (int)len );
        if ( temp >= 0 ) {
            result += temp;
            len -= (qint64)temp;
            data += (uint)temp;
        } else if ( errno != EINTR && errno != EWOULDBLOCK ) {
            LOG() << "write(" << d->fd << ") errno = " << errno;
            return -1;
        }
    }
    return result;
#endif

#else

    Q_ASSERT(d->io);
    d->io->wbm.lock();
    d->io->wb.append( QByteArray( data, len ) );
    d->io->wbm.unlock();
    d->io->write();
    return len;
#endif
}


/*!
    \reimp
*/
int QSerialPort::rate() const
{
    return d->rate;
}

/*!
    Returns the state of CTS/RTS flow control on the serial device.
    The default value is false.

    \sa setFlowControl()
*/
bool QSerialPort::flowControl() const
{
#ifdef Q_OS_UNIX
    return d->flowControl;
#else
    return false;
#endif
}

/*!
    Sets the state of CTS/RTS flow control on the serial device to \a value.
    This must be called before QSerialPort::open().  Changes to
    the value after opening will be ignored.

    \sa flowControl()
*/
void QSerialPort::setFlowControl( bool value )
{
#ifdef Q_OS_UNIX
    d->flowControl = value;
#else
    Q_UNUSED(value)
#endif
}

/*!
    Returns true if tty devices should be kept open on a zero-byte read; otherwise returns false.
    The default value is true.

    Some serial devices return zero bytes when there is no data available,
    instead of returning the system error \c EWOULDBLOCK.  When keepOpen()
    is true, a zero-byte read will be treated the same as \c EWOULDBLOCK.
    When keepOpen() is false, a zero-byte read will be interpreted as an
    unrecoverable error and the serial port will be closed.

    For Bluetooth RFCOMM sockets, keepOpen() should be false.

    The keepOpen() state is ignored if the underlying serial device is a
    socket connection to a phone simulator.

    \sa setKeepOpen()
*/
bool QSerialPort::keepOpen() const
{
#ifdef Q_OS_UNIX
    return d->keepOpen;
#else
    return false;
#endif
}

/*!
    Sets the keep open flag to \a value.  The default value is true.

    Some serial devices return zero bytes when there is no data available,
    instead of returning the system error \c EWOULDBLOCK.  When \a value
    is true, a zero-byte read will be treated the same as \c EWOULDBLOCK.
    When \a value is false, a zero-byte read will be interpreted as an
    unrecoverable error and the serial port will be closed.

    For Bluetooth RFCOMM sockets, \a value should be false.

    The state of \a value is ignored if the underlying serial device is a
    socket connection to a phone simulator.

    \sa keepOpen()
*/
void QSerialPort::setKeepOpen( bool value )
{
#ifdef Q_OS_UNIX
    d->keepOpen = value;
#else
    Q_UNUSED(value)
#endif
}

/*!
    \reimp
*/
bool QSerialPort::dtr() const
{
#ifdef Q_OS_UNIX
    return d->dtr;
#else
    return false;
#endif
}

/*!
    \reimp
*/
void QSerialPort::setDtr( bool value )
{
#ifdef Q_OS_UNIX
#ifdef USE_TERMIOS
    if ( d->fd != -1 && d->isTty ) {
        int status = TIOCM_DTR;
        if ( value )
            ::ioctl( d->fd, TIOCMBIS, &status );
        else
            ::ioctl( d->fd, TIOCMBIC, &status );
        d->dtr = value;
    }
#endif
#else
    Q_UNUSED(value)
#endif
}

/*!
    \reimp
*/
bool QSerialPort::dsr() const
{
    TRACE(Modem) << "QSerialPort::dsr";
#ifdef Q_OS_UNIX
#ifdef USE_TERMIOS
    if ( d->fd != -1 && d->isTty ) {
        int status = 0;
        ::ioctl( d->fd, TIOCMGET, &status );
        LOG() << ( ( status & TIOCM_DSR ) != 0 );
        return ( ( status & TIOCM_DSR ) != 0 );
    }
#endif
#else
    if ( d->io ) {
        LOG() << d->io->dsr;
        return d->io->dsr;
    }
#endif
    LOG() << false;
    return false;
}

/*!
    \reimp
*/
bool QSerialPort::carrier() const
{
#ifdef Q_OS_UNIX
#ifdef USE_TERMIOS
    if ( d->fd != -1 && d->isTty ) {
        int status = 0;
        ::ioctl( d->fd, TIOCMGET, &status );
        return ( ( status & TIOCM_CAR ) != 0 );
    }
#endif
#endif
    return false;
}

/*!
    \reimp
*/
bool QSerialPort::rts() const
{
#ifdef Q_OS_UNIX
    return d->rts;
#else
    return false;
#endif
}

/*!
    \reimp
*/
void QSerialPort::setRts( bool value )
{
#ifdef Q_OS_UNIX
#ifdef USE_TERMIOS
    if ( d->fd != -1 && d->isTty ) {
        int status = TIOCM_RTS;
        if ( value )
            ::ioctl( d->fd, TIOCMBIS, &status );
        else
            ::ioctl( d->fd, TIOCMBIC, &status );
        d->rts = value;
    }
#endif
#else
    Q_UNUSED(value)
#endif
}

/*!
    \reimp
*/
bool QSerialPort::cts() const
{
#ifdef Q_OS_UNIX
#ifdef USE_TERMIOS
    if ( d->fd != -1 && d->isTty ) {
        int status = 0;
        ::ioctl( d->fd, TIOCMGET, &status );
        return ( ( status & TIOCM_CTS ) != 0 );
    }
#endif
#endif
    return false;
}

/*!
    \reimp
*/
void QSerialPort::discard()
{
#ifdef Q_OS_UNIX
#ifdef USE_TERMIOS
    if ( d->fd != -1 && d->isTty ) {
        ::tcflush( d->fd, TCIOFLUSH );
    }
#endif
#endif
}

/*!
    \reimp
*/
bool QSerialPort::isValid() const
{
#ifdef Q_OS_UNIX
#ifdef USE_POSIX_SYSCALLS
    return ( d->fd != -1 );
#else
    return QSerialIODevice::isValid();
#endif
#else
    return QSerialIODevice::isValid();
#endif
}

/*!
    \reimp

    This class overrides QSerialIODevice::run() to run pppd directly on the
    underlying device name so that it isn't necessary to create a pseudo-tty.
    This is generally more efficient for running pppd.
*/
QProcess *QSerialPort::run( const QStringList& arguments,
                              bool addPPPdOptions )
{
#ifdef Q_OS_UNIX
    if ( addPPPdOptions && d->isTty ) {
        // Make sure we aren't using the device before handing it off to pppd.
        if ( isOpen() )
            close();

        // Build a new argument list for pppd, with the device name in place.
        QStringList newargs;
        newargs << d->device;
        newargs << QString::number( d->rate );
        for ( int index = 1; index < arguments.size(); ++index )
            newargs << arguments[index];

        // Run the process.  When it exits, open() will be called again.
        QProcess *process = new QProcess();
        process->setReadChannelMode( QProcess::ForwardedChannels );
        connect( process, SIGNAL(stateChanged(QProcess::ProcessState)),
                 this, SLOT(pppdStateChanged(QProcess::ProcessState)) );
        connect( process, SIGNAL(destroyed()), this, SLOT(pppdDestroyed()) );
        process->start( arguments[0], newargs );
        return process;
    } else {
        return QSerialIODevice::run( arguments, addPPPdOptions );
    }
#else
    Q_UNUSED(arguments)
    Q_UNUSED(addPPPdOptions)
    return 0;
#endif
}

void QSerialPort::statusTimeout()
{
#ifdef Q_OS_UNIX
    TRACE(Modem_Status) << "QSerialPort::statusTimeout";
#ifdef USE_TERMIOS
    if ( d->fd != -1 && d->isTty ) {
        int old_status = d->status;
        int status = 0;
        ::ioctl( d->fd, TIOCMGET, &status );
        d->status = status;
        LOG() << "DTR" << ((status & TIOCM_DTR) != 0?"1":"0");
        LOG() << "DSR" << ((status & TIOCM_DSR) != 0?"1":"0");
        if ( ( ( old_status ^ status ) & TIOCM_DSR ) != 0 )
            emit dsrChanged( ( status & TIOCM_DSR ) != 0 );
        LOG() << "CARRIER" << ((status & TIOCM_CAR) != 0?"1":"0");
        if ( ( ( old_status ^ status ) & TIOCM_CAR ) != 0 )
            emit carrierChanged( ( status & TIOCM_CAR ) != 0 );
    }
#endif
#endif
}

void QSerialPort::pppdStateChanged( QProcess::ProcessState state )
{
    if ( state == QProcess::NotRunning && !isOpen() ) {
        // The process has exited, so re-open the device for our own use.
        open( QIODevice::ReadWrite );
    }
}

void QSerialPort::pppdDestroyed()
{
    if ( !isOpen() ) {
        // The process has been killed, so re-open the device for our own use.
        open( QIODevice::ReadWrite );
    }
}

/*!
    Create and open a serial device from a \a name of the form \c{device:rate}.
    Returns NULL if the device could not be opened.  The \a defaultRate
    parameter indicates the default rate to use if \c{:rate} was not included
    in the device name.  If \a flowControl is true, then CTS/RTS flow
    control should be enabled on the device.

    The \a name parameter can have the special form \c{sim:hostname},
    where \c hostname is the name of a host running a phone simulator daemon
    (usually \c localhost).  The phone simulator mode is intended for
    debugging purposes only.
*/
QSerialPort *QSerialPort::create( const QString& name, int defaultRate,
                                    bool flowControl )
{
    TRACE(Modem) << "QSerialPort::create" << "name" << name;
    int rate = defaultRate;
    QString dev = name;
    if ( dev.length() > 0 && dev[0] == '/' ) {
        int index = dev.indexOf(QChar(':'));
        if ( index != -1 ) {
            rate = dev.mid( index + 1 ).toInt();
            dev = dev.left( index );
        }
    }
    LOG() << "opening serial device " << dev << " at " << rate;
    QSerialPort *device = new QSerialPort( dev, rate );
    device->setFlowControl( flowControl );
    if ( !device->open( ReadWrite ) ) {
        WARNING() << "Failed opening " << dev << __FILE__ << __LINE__;
        delete device;
        return 0;
    } else {
        LOG() << "Opened " << dev;
        return device;
    }
}

#ifndef Q_OS_UNIX
void QSerialPort::setBrokenSerial()
{
    d->brokenSerial = true;
}
#endif

void QSerialPort::socketReady()
{
#ifdef Q_OS_UNIX
    TRACE(Modem) << "QSerialPort::socketReady";
    d->notifier->setEnabled( false );
    internalReadyRead();
#endif
}

#include "qserialport.moc"
#endif
