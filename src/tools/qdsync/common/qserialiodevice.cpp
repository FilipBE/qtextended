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
#include "qserialiodevice.h"
#include "qserialiodevice_p.h"

#include <qsocketnotifier.h>
#include <qtimer.h>
#include <trace.h>
QD_LOG_OPTION(Modem)

#ifdef Q_OS_UNIX
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
// Windows
#endif

/*!
    \class QSerialIODevice
    \brief The QSerialIODevice class is the base for all serial devices.

    The abstract QSerialIODevice class extends QIODevice with functionality
    that is specific to serial devices.  Subclasses implement specific kinds
    of serial devices.  In particular, QSerialPort implements a physical hardware
    serial port.

    \sa QIODevice, QSerialPort
*/

/*!
    Construct a new QSerialIODevice object attached to \a parent.
*/
QSerialIODevice::QSerialIODevice( QObject *parent )
    : QIODevice( parent )
{
    process = 0;
}

/*!
    Destruct a QSerialIODevice object.
*/
QSerialIODevice::~QSerialIODevice()
{
    if ( process )
        process->clearDevice();
}

/*!
    Returns if this device is sequential in nature; false otherwise.  Serial devices are
    always sequential.
*/
bool QSerialIODevice::isSequential() const
{
    return true;
}

/*!
    Returns the serial device's baud rate.  The default implementation
    returns 115200.  Subclasses are expected to override this value.
*/
int QSerialIODevice::rate() const
{
    return 115200;
}

/*!
    \fn bool QSerialIODevice::dtr() const

    Returns true if the current state of the DTR modem status line is active; otherwise returns false.

    \sa setDtr()
*/

/*!
    \fn void QSerialIODevice::setDtr( bool value )

    Sets the state of the DTR modem status line to \a value.

    \sa dtr()
*/

/*!
    \fn bool QSerialIODevice::dsr() const

    Returns true if the current state of the DSR modem status line is active; otherwise returns false.

    \sa dsrChanged()
*/

/*!
    \fn bool QSerialIODevice::carrier() const

    Returns true if the current state of the DCD (carrier) modem status line is active; otherwise returns false.

    \sa setCarrier(), carrierChanged()
*/

/*!
    Sets the DCD (carrier) modem status line to \a value.  This is used by
    programs that accept incoming serial connections to indicate to the
    peer machine that the carrier has dropped.

    By default, this calls setDtr(), which supports the case where a
    null modem cable is connecting DTR to DSR and DCD on the peer machine.
    The caller should wait for a small delay and then setDtr(true)
    to restore DTR to the correct value.

    This function will return false if it uses setDtr() to transmit the
    carrier change, or true if it can use a real carrier signal
    (QGsm0710MultiplexerServer supports real carrier signals).

    \sa carrier()
*/
bool QSerialIODevice::setCarrier( bool value )
{
    setDtr( value );
    return false;
}

/*!
    \fn bool QSerialIODevice::rts() const

    Returns true if the current state of the RTS modem status line is active; otherwise returns false.

    \sa setRts()
*/

/*!
    \fn void QSerialIODevice::setRts( bool value )

    Sets the state of the RTS modem status line to \a value.

    \sa rts()
*/

/*!
    \fn bool QSerialIODevice::cts() const

    Returns true if current state of the CTS modem status line is active; otherwise returns false.

    \sa ctsChanged()
*/

/*!
    \fn void QSerialIODevice::discard()

    Discard pending buffered data without transmitting it.  This function
    will do nothing if the underlying serial device implementation cannot discard buffers.
*/

/*!
    Returns true if the caller should wait for the ready() signal to
    be emitted before sending data on this device.  Returns false if
    the caller can send data immediately after calling open().
    The default implementation returns false.

    \sa ready()
*/
bool QSerialIODevice::waitForReady() const
{
    return false;
}

/*!
    Returns true if this serial device can validly transfer data; false otherwise.
    If this function returns false, there is no point writing
    data to this device, or attempting to read data from it,
    because it will never give a useful result.

    The default implementation returns the same value as isOpen().
    The QNullSerialIODevice class overrides this to always return
    false, even if the device is technically open.
*/
bool QSerialIODevice::isValid() const
{
    return isOpen();
}

/*!
    \fn void QSerialIODevice::dsrChanged( bool value );

    Signal that is emitted when the state of the DSR modem
    status line changes to \a value.

    \sa dsr()
*/

/*!
    \fn void QSerialIODevice::carrierChanged( bool value );

    Signal that is emitted when the state of the DCD (carrier) modem
    status line changes to \a value.

    \sa carrier()
*/

/*!
    \fn void QSerialIODevice::ctsChanged( bool value );

    Signal that is emitted when the state of the CTS modem
    status line changes to \a value.

    \sa cts()
*/

/*!
    \fn void QSerialIODevice::ready()

    Signal that is emitted when the device is ready to accept data
    after being opened.  This signal will only be emitted if the
    waitForReady() function returns true.

    \sa waitForReady()
*/

#ifdef USE_POSIX_SYSCALLS

// We would like to use "openpty", but it isn't in libc on some systems.
static bool createPseudoTty(int& masterFd, int& slaveFd, char *ttyname)
{
    // try opening Unix98 pseudo tty
    if ((masterFd = ::open("/dev/ptmx", O_RDWR | O_NONBLOCK, 0)) >= 0) {
        if (grantpt(masterFd) == 0) {
            if (unlockpt(masterFd) == 0) {
                ptsname_r(masterFd, ttyname, BUFSIZ);
                if ((slaveFd = ::open(ttyname, O_RDWR | O_NOCTTY, 0)) >= 0)
                    return true;
            }
        }
        ::close(masterFd);
    }

    // opening Unix98 pseudo tty failed, fall back to BSD style pseudo tty
    static char const firstChars[]  = "pqrstuvwxyzabcde";
    static char const secondChars[] = "0123456789abcdef";
    const char *first;
    const char *second;
    char ptyname[16];
    for ( first = firstChars; *first != '\0'; ++first ) {
        for ( second = secondChars; *second != '\0'; ++second ) {
            sprintf( ptyname, "/dev/pty%c%c", *first, *second );
            sprintf( ttyname, "/dev/tty%c%c", *first, *second );
            if ( ( masterFd = ::open( ptyname, O_RDWR | O_NONBLOCK, 0 ) ) >= 0 ) {
                if ( ( slaveFd = ::open( ttyname, O_RDWR | O_NOCTTY, 0 ) )
                        >= 0 ) {
                    return true;
                }
                ::close( masterFd );
            }
        }
    }
    return false;
}

#endif // USE_POSIX_SYSCALLS

/*!
    Run a program with the supplied \a arguments, redirecting the
    device's data to stdin and stdout on the new process via a
    pseudo-tty.  Redirection will stop when the process exits.

    This function returns a QProcess object that can be used to control the
    new process.  The caller is responsible for destroying this object when it
    is no longer required.  Returns null if the process could not be started
    for some reason.

    If \a addPPPdOptions is true, then the caller is attempting to start
    pppd.  The base class implementation will insert the pseudo-tty's device
    name at the beginning of the command arguments.

    Subclasses may override this method to run pppd directly on an
    underlying device.

    The readyRead() signal will be suppressed while the process is running.
*/
QProcess *QSerialIODevice::run( const QStringList& arguments,
                                  bool addPPPdOptions )
{
#ifdef USE_POSIX_SYSCALLS
    // Bail out if we are already running a process on this device.
    if ( process )
        return 0;

    // Create a pseudo-tty to manage communication with the process.
    int masterFd = -1;
    int slaveFd = -1;
    char slaveName[BUFSIZ];
    if ( !createPseudoTty( masterFd, slaveFd, slaveName ) ) {
        qWarning( "qdsync::QSerialIODevice::run: could not find a pseudo-tty" );
        return 0;
    }

    // Make sure that the device is open.
    if ( !isOpen() )
        open( QIODevice::ReadWrite );

    // Insert the name of the slave into the argument list if necessary.
    QStringList newargs;
    if ( addPPPdOptions ) {
        newargs << QString( slaveName );
    }
    for ( int index = 1; index < arguments.size(); ++index )
        newargs << arguments[index];

    // Construct a process object to manage the communication.
    process = new QPseudoTtyProcess( this, masterFd, slaveFd, addPPPdOptions );
    process->setReadChannelMode( QProcess::ForwardedChannels );
    process->start( arguments[0], newargs );

    // We don't need the slave in the parent process any more.
    ::close( slaveFd );

    // Return to the caller with the new process.
    return process;
#else
    return 0;
#endif
}

/*!
    Abort an \c{ATD} dial command.  The default implementation transmits a
    single CR character.

    If a modem needs to change the way abortDial() works, a multiplexer plug-in
    should be written which overrides the abortDial() function on
    the channels that need different abort logic.

    \sa QSerialIODeviceMultiplexerPlugin
*/
void QSerialIODevice::abortDial()
{
    static char const abortString[] = "\r";
    write( abortString, 1 );
}

/*!
    Emit the readyRead() signal.  If a process is currently running
    on this device, then the readyRead() signal will be suppressed.
    Subclasses should call this slot rather than emit readyRead()
    directly.
*/
void QSerialIODevice::internalReadyRead()
{
    TRACE(Modem) << "QSerialIODevice::internalReadyRead";
    if ( process )
        process->deviceReadyRead();
    else
        emit readyRead();
}

QPseudoTtyProcess::QPseudoTtyProcess
    ( QSerialIODevice *device, int masterFd, int slaveFd, bool isPPP )
    : QProcess()
{
    this->device = device;
    this->masterFd = masterFd;
    this->slaveFd = slaveFd;
    this->isPPP = isPPP;
    this->readySeen = false;

    connect( this, SIGNAL(stateChanged(QProcess::ProcessState)),
             this, SLOT(childStateChanged(QProcess::ProcessState)) );

    if ( device->waitForReady() ) {
        // We cannot start writing data until the device indicates
        // that it is ready for us to do so.
        connect( device, SIGNAL(ready()), this, SLOT(deviceReady()) );
    } else {
        // We can start writing data immediately.
        deviceReady();
    }
}

QPseudoTtyProcess::~QPseudoTtyProcess()
{
    if ( device )
        device->process = 0;
#ifdef USE_POSIX_SYSCALLS
    if ( masterFd != -1 )
        ::close( masterFd );
#endif
}

void QPseudoTtyProcess::deviceReadyRead()
{
    if ( device ) {
        qint64 len = device->read( buffer, sizeof(buffer) );
#ifdef USE_POSIX_SYSCALLS
        if ( len > 0 && masterFd != -1 ) {
            // Write the data into the master side of the pseudo-tty.
            int temp;
            const char *data = buffer;
            while ( len > 0 ) {
                temp = ::write( masterFd, data, (int)len );
                if ( temp >= 0 ) {
                    len -= (qint64)temp;
                    data += (uint)temp;
                } else if ( errno != EINTR && errno != EWOULDBLOCK ) {
                    qLog(Modem) << "write(" << masterFd << ") errno = "
                                << errno;
                    break;
                }
            }
        }
#endif
    }
}

void QPseudoTtyProcess::setupChildProcess()
{
#ifdef USE_POSIX_SYSCALLS
    if ( isPPP ) {
        // Convert pppd's stdin into the slave, but leave stdout and stderr
        // as-is so that debug output goes to our parent's stdout/stderr.
        ::dup2( slaveFd, 0 );
    } else {
        // Convert stdin and stdout into the slave, but leave stderr alone.
        ::dup2( slaveFd, 0 );
        ::dup2( slaveFd, 1 );
    }

    // We don't need the master and slave fd's any more.
    ::close( masterFd );
    ::close( slaveFd );
    masterFd = -1;

    //we have to change pppd's process group because pppd terminates its process group
    //when receiving SIGTERM while it is running the chat script
    int ret = ::setpgrp();
    if (  ret < 0 )
        perror("change process grp");
#endif
}

void QPseudoTtyProcess::masterReadyRead()
{
#ifdef USE_POSIX_SYSCALLS
    if ( masterFd == -1 )
        return;
    int len;
    for(;;) {
        len = ::read( masterFd, buffer, sizeof(buffer) );
        if ( !len ) {
            // Process has probably exited, so clean up the master device.
            ::close( masterFd );
            masterFd = -1;
            break;
        } else if ( len > 0 ) {
            // Write the data we got from the master to the raw device.
            if ( device )
                device->write( buffer, len );
        } else if ( errno == EWOULDBLOCK ) {
            break;
        } else if ( errno != EINTR ) {
            qLog(Modem) << "read(" << masterFd << ") errno = " << errno;
            break;
        }
    }
#endif
}

void QPseudoTtyProcess::childStateChanged( QProcess::ProcessState state )
{
    if ( state == QProcess::NotRunning ) {
        // Detach ourselves from the QSerialIODevice instance.
        if ( device ) {
            device->process = 0;
            device = 0;
        }
    }
}

void QPseudoTtyProcess::deviceReady()
{
    if ( !readySeen ) {
        // Now that the device is ready, we can start listening for
        // data on the master fd.
        QSocketNotifier *notifier = new QSocketNotifier
            ( masterFd, QSocketNotifier::Read, this );
        connect( notifier, SIGNAL(activated(int)),
                 this, SLOT(masterReadyRead()) );
        readySeen = true;
    }
}

#endif
