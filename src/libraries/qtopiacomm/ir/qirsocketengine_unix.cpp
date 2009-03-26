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
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

#include <linux/types.h>
#include <linux/irda.h>

#include "qirsocketengine_p.h"
#include "qirnamespace_p.h"
#include <qirsocket.h>
#include <qirserver.h>

#include <QTime>
#include <QDebug>
#include <QApplication>

QIrSocketEngine::QIrSocketEngine()
{
    m_error = QIrSocket::NoError;
}

QIrSocketEngine::~QIrSocketEngine()
{

}

QString QIrSocketEngine::getErrorString(QIrSocket::SocketError error)
{
    QByteArray err;
    switch (error) {
        case QIrSocket::NoError:
            break;

        case QIrSocket::AccessError:
            err = QT_TRANSLATE_NOOP("QIrSocket", "Permission denied");
            break;

        case QIrSocket::ResourceError:
            err = QT_TRANSLATE_NOOP("QIrSocket", "Out of resources");
            break;

        case QIrSocket::BusyError:
            err = QT_TRANSLATE_NOOP("QIrSocket", "Device busy");
            break;

        case QIrSocket::HostUnreachableError:
            err = QT_TRANSLATE_NOOP("QIrSocket", "Host unreachable");
            break;

        case QIrSocket::ConnectionRefused:
            err = QT_TRANSLATE_NOOP("QIrSocket", "Connection refused");
            break;

        case QIrSocket::NetworkError:
            err = QT_TRANSLATE_NOOP("QIrSocket", "Network error");
            break;

        case QIrSocket::TimeoutError:
            err = QT_TRANSLATE_NOOP("QIrSocket", "Network operation timed out");
            break;

        case QIrSocket::RemoteHostClosedError:
            err = QT_TRANSLATE_NOOP("QIrSocket", "Remote host closed the connection");
            break;

        case QIrSocket::UnsupportedOperationError:
            err = QT_TRANSLATE_NOOP("QIrSocket", "Operation is not supported");
            break;

        case QIrSocket::AddressInUseError:
            err = QT_TRANSLATE_NOOP("QIrSocket", "Address is in use");
            break;

        case QIrSocket::AddressNotAvailableError:
            err = QT_TRANSLATE_NOOP("QIrSocket", "Address is not available");
            break;

        default:
            err = QT_TRANSLATE_NOOP("QIrSocket", "Unknown error");
            break;
    };

    return qApp->translate( "QIrSocket", err.constData() );
}

int QIrSocketEngine::socket()
{
    int fd = ::socket(AF_IRDA, SOCK_STREAM, 0);

    if (fd < 0) {
        switch (errno) {
            case EACCES:
                m_error = QIrSocket::AccessError;
                break;
            case EMFILE:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case EINVAL:
                m_error = QIrSocket::ResourceError;
            case EAFNOSUPPORT:
            case EPROTONOSUPPORT:
                m_error = QIrSocket::UnsupportedOperationError;
                break;
            default:
                m_error = QIrSocket::UnknownError;
        };
    } else {
        ::fcntl(fd, F_SETFD, FD_CLOEXEC);
    }

    return fd;
}

bool QIrSocketEngine::bind(int fd, const QByteArray &service)
{
    struct sockaddr_irda addr;
    memset(&addr, 0, sizeof(addr));
    addr.sir_family = AF_IRDA;
    addr.sir_lsap_sel = LSAP_ANY;
    addr.sir_addr = 0;
    strncpy(addr.sir_name, service.constData(), 25);

    if (::bind(fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
        switch (errno) {
            case EACCES:
                m_error = QIrSocket::AccessError;
                break;
            case EADDRINUSE:
                m_error = QIrSocket::AddressInUseError;
                break;
            case EINVAL:
                m_error = QIrSocket::UnsupportedOperationError;
                break;
            case EADDRNOTAVAIL:
                m_error = QIrSocket::AddressNotAvailableError;
                break;
            case EFAULT:
            case ENOMEM:
                m_error = QIrSocket::ResourceError;
                break;
            default:
                m_error = QIrSocket::UnknownError;
                break;
        };

        return false;
    }

    return true;
}

bool QIrSocketEngine::listen(int fd, int backlog)
{
    if (::listen(fd, backlog) < 0) {
        switch (errno) {
            case EADDRINUSE:
                m_error = QIrSocket::AddressInUseError;
                break;
            case EOPNOTSUPP:
                m_error = QIrSocket::UnsupportedOperationError;
                break;
            default:
                m_error = QIrSocket::UnknownError;
                break;
        };

        return false;
    }

    return true;
}

int QIrSocketEngine::accept(int serverFd)
{
    int fd = ::accept(serverFd, 0, 0);

    if (fd < 0)
        return fd;

    ::fcntl(fd, F_SETFD, FD_CLOEXEC);

    return fd;
}

void QIrSocketEngine::handleConnectError(int error)
{
    switch (error) {
        case EADDRINUSE:
            m_error = QIrSocket::NetworkError;
            break;
        case ETIMEDOUT:
            m_error = QIrSocket::TimeoutError;
            break;
        case EINVAL:
            m_error = QIrSocket::UnknownError;
            break;
        case EBUSY:
            m_error = QIrSocket::BusyError;
            break;
        case EADDRNOTAVAIL:
            m_error = QIrSocket::AddressNotAvailableError;
            break;
        case EHOSTDOWN:
        case ENETUNREACH:
        case EHOSTUNREACH:
            m_error = QIrSocket::HostUnreachableError;
            break;
        case ECONNREFUSED:
            m_error = QIrSocket::ConnectionRefused;
            break;
        case EAGAIN:
            m_error = QIrSocket::UnknownError;
            break;
        case EACCES:
        case EPERM:
            m_error = QIrSocket::AccessError;
            break;
        default:
            m_error = QIrSocket::UnknownError;
            break;
    };
}

bool QIrSocketEngine::testConnected(int fd)
{
    int error = 0;
    socklen_t len = sizeof(error);

    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        m_error = QIrSocket::UnknownError;
        return false;
    }
    else {
        if (error) {
            handleConnectError(error);
            return false;
        }
    }

    return true;
}

QIrSocket::SocketState QIrSocketEngine::connect(int fd,
        const QByteArray &service, quint32 remote)
{
    struct sockaddr_irda addr;
    memset(&addr, 0, sizeof(addr));
    addr.sir_family = AF_IRDA;
    addr.sir_addr = remote;
    strncpy(addr.sir_name, service.constData(), 25);

    if (::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        if (errno != EINPROGRESS && errno != EALREADY && errno != EISCONN)
            qWarning("QIrSocketPrivate::initiateConnect: error during connect: %d, %s",
                     errno, strerror(errno));

        if (errno == EISCONN) {
            return QIrSocket::ConnectedState;
        }
        else if (errno == EINPROGRESS || errno == EALREADY) {
            return QIrSocket::ConnectingState;
        }
        else {
            handleConnectError(errno);
            return QIrSocket::UnconnectedState;
        }
    }

    return QIrSocket::ConnectedState;
}

qint64 QIrSocketEngine::writeToSocket(int fd, const char *data, qint64 len)
{
    ssize_t writtenBytes;
    do {
        writtenBytes = ::write(fd, data, len);
    } while (writtenBytes < 0 && errno == EINTR);

    if (writtenBytes < 0) {
    switch (errno) {
        case EPIPE:
        case ECONNRESET:
            writtenBytes = -1;
            m_error = QIrSocket::RemoteHostClosedError;
            break;
        case EAGAIN:
            writtenBytes = 0;
            break;
        case EMSGSIZE:
            m_error = QIrSocket::NetworkError;
            break;
        default:
            m_error = QIrSocket::UnknownError;
            break;
    }
    }

    return qint64(writtenBytes);
}

qint64 QIrSocketEngine::readFromSocket(int fd, char *data, qint64 len)
{
    ssize_t r = 0;
    do {
        r = ::read(fd, data, len);
    } while (r == -1 && errno == EINTR);

    if (r < 0) {
        r = -1;
        switch (errno) {
            case EAGAIN:
                r = -2;
                break;
            case EBADF:
            case EINVAL:
            case EIO:
                m_error = QIrSocket::NetworkError;
                break;
            case ECONNRESET:
                r = 0;
                break;
            default:
                m_error = QIrSocket::UnknownError;
                break;
        }
    }

    return qint64(r);
}

void QIrSocketEngine::close(int fd)
{
    ::close(fd);
}

qint64 QIrSocketEngine::bytesAvailable(int fd) const
{
    size_t nbytes = 0;
    qint64 available = 0;

    if (::ioctl(fd, FIONREAD, (char *) &nbytes) >= 0)
        available = (qint64) *((int *) &nbytes);
    else
        ::perror("FIONREAD failed:");

    return available;
}

void QIrSocketEngine::readSocketParameters(int fd, quint32 *remote) const
{
    struct sockaddr_irda addr;
    socklen_t len = sizeof(addr);

    memset(&addr, 0, sizeof(addr));
    if (::getpeername(fd, (struct sockaddr *) &addr, &len) == 0) {
        if (remote)
            *remote = addr.sir_addr;
    }
}

bool QIrSocketEngine::setSocketOption(int fd, QIrSocketEngine::SocketOption option)
{
    switch (option) {
        case NonBlockingOption:
        {
            int flags = ::fcntl(fd, F_GETFL, 0);
            return ::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
        }
        default:
            return false;
    };

    return false;
}

bool QIrSocketEngine::setServiceHints(int fd, QIr::DeviceClasses classes)
{
    // Try to set the hint bits, this should almost always work.
    unsigned char hints[4];

    hints[0] = 0;
    hints[1] = 0;
    hints[2] = 0;
    hints[3] = 0;

    convert_to_hints(classes, hints);

    int status = setsockopt(fd, SOL_IRLMP, IRLMP_HINTS_SET, hints, sizeof(hints));

    return status == 0;
}

int QIrSocketEngine::select(QIrSocketEngine::SelectTypes types, int fd, int timeout,
                           bool *canRead, bool *canWrite) const
{
    fd_set write_fds;
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);

    if (types & QIrSocketEngine::SelectRead) {
        FD_SET(fd, &read_fds);
    }

    if (types & QIrSocketEngine::SelectWrite) {
        FD_SET(fd, &write_fds);
    }

    struct timeval tv;

    if (timeout >= 0) {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
    }

    int status = ::select(fd+1, &read_fds, &write_fds, 0, timeout < 0 ? 0 : &tv);

    if (status <= 0)
        return status;

    if (canRead)
        *canRead = FD_ISSET(fd, &read_fds);

    if (canWrite)
        *canWrite = FD_ISSET(fd, &write_fds);

    return status;
}

static int timeout_value(int timeout, int elapsed)
{
    if (timeout < 0)
        return -1;

    int msecs = timeout - elapsed;

    return msecs < 0 ? 0 : msecs;
}

bool QIrSocketEngine::waitFor(QIrSocketEngine::SelectTypes types,
                              int fd, int timeout, bool *timedOut,
                              bool *canRead, bool *canWrite)
{
    if (timedOut)
        *timedOut = false;

    QTime stopWatch;
    stopWatch.start();

    while ((timeout < 0) || (stopWatch.elapsed() < timeout)) {
        int msecs = timeout_value(timeout, stopWatch.elapsed());
        int ret = select(types, fd, msecs, canRead, canWrite);

        if (ret == 0) {
            if (timedOut)
                *timedOut = true;

            m_error = QIrSocket::TimeoutError;
            return false;
        }

        if (ret > 0)
            return true;

        // Have an error condition, continue on interrupt
        if (errno == EINTR)
            continue;

        break;
    }

    m_error = QIrSocket::UnknownError;
    return false;
}
