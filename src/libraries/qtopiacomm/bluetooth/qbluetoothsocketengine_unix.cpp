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
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/sco.h>
#include <bluetooth/rfcomm.h>

#include "qbluetoothsocketengine_p.h"
#include "qbluetoothnamespace_p.h"

#include <QTime>
#include <QBluetoothAddress>
#include <QApplication>

QBluetoothSocketEngine::QBluetoothSocketEngine()
{
    m_error = QBluetoothAbstractSocket::NoError;
}

QBluetoothSocketEngine::~QBluetoothSocketEngine()
{

}

QString QBluetoothSocketEngine::getErrorString(QBluetoothAbstractSocket::SocketError error)
{
    QByteArray err;
    switch (error) {
        case QBluetoothAbstractSocket::NoError:
            break;

        case QBluetoothAbstractSocket::AccessError:
            err = QT_TRANSLATE_NOOP("QBluetoothAbstractSocket", "Permission denied");
            break;

        case QBluetoothAbstractSocket::ResourceError:
            err = QT_TRANSLATE_NOOP("QBluetoothAbstractSocket", "Out of resources");
            break;

        case QBluetoothAbstractSocket::BusyError:
            err = QT_TRANSLATE_NOOP("QBluetoothAbstractSocket", "Device busy");
            break;

        case QBluetoothAbstractSocket::HostUnreachableError:
            err = QT_TRANSLATE_NOOP("QBluetoothAbstractSocket", "Host unreachable");
            break;

        case QBluetoothAbstractSocket::BindError:
            err = QT_TRANSLATE_NOOP("QBluetoothAbstractSocket", "Could not bind socket");
            break;

        case QBluetoothAbstractSocket::ConnectionRefused:
            err = QT_TRANSLATE_NOOP("QBluetoothAbstractSocket", "Connection refused");
            break;

        case QBluetoothAbstractSocket::NetworkError:
            err = QT_TRANSLATE_NOOP("QBluetoothAbstractSocket", "Network error");
            break;

        case QBluetoothAbstractSocket::TimeoutError:
            err = QT_TRANSLATE_NOOP("QBluetoothAbstractSocket", "Network operation timed out");
            break;

        case QBluetoothAbstractSocket::RemoteHostClosedError:
            err = QT_TRANSLATE_NOOP("QBluetoothAbstractSocket", "Remote host closed the connection");
            break;

        case QBluetoothAbstractSocket::UnsupportedOperationError:
            err = QT_TRANSLATE_NOOP("QBluetoothAbstractSocket", "Operation is not supported");
            break;

        case QBluetoothAbstractSocket::AddressInUseError:
            err = QT_TRANSLATE_NOOP("QBluetoothAbstractSocket", "Address is in use");
            break;

        case QBluetoothAbstractSocket::AddressNotAvailableError:
            err = QT_TRANSLATE_NOOP("QBluetoothAbstractSocket", "Address is not available");
            break;

        default:
            err = QT_TRANSLATE_NOOP("QBluetoothAbstractSocket", "Unknown error");
            break;
    };

    return qApp->translate( "QBluetoothAbstractSocket", err.constData() );
}

void QBluetoothSocketEngine::handleSocketError(int error)
{
    switch (error) {
        case EACCES:
            m_error = QBluetoothAbstractSocket::AccessError;
            break;
        case EMFILE:
        case ENFILE:
        case ENOBUFS:
        case ENOMEM:
        case EINVAL:
            m_error = QBluetoothAbstractSocket::ResourceError;
        case EAFNOSUPPORT:
        case EPROTONOSUPPORT:
            m_error = QBluetoothAbstractSocket::UnsupportedOperationError;
            break;
        default:
            m_error = QBluetoothAbstractSocket::UnknownError;
    };
}

int QBluetoothSocketEngine::scoSocket()
{
    int fd = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);

    if (fd < 0) {
        handleSocketError(errno);
        return fd;
    }

    ::fcntl(fd, F_SETFD, FD_CLOEXEC);

    return fd;
}

int QBluetoothSocketEngine::rfcommSocket()
{
    int fd = socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    if (fd < 0) {
        handleSocketError(errno);
        return fd;
    }

    ::fcntl(fd, F_SETFD, FD_CLOEXEC);

    return fd;
}

int QBluetoothSocketEngine::l2capSocket()
{
    int fd = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

    if (fd < 0) {
        handleSocketError(errno);
        return fd;
    }

    ::fcntl(fd, F_SETFD, FD_CLOEXEC);

    return fd;
}

int QBluetoothSocketEngine::l2capDgramSocket()
{
    int fd = socket(PF_BLUETOOTH, SOCK_DGRAM, BTPROTO_L2CAP);

    if (fd < 0) {
        handleSocketError(errno);
        return fd;
    }

    ::fcntl(fd, F_SETFD, FD_CLOEXEC);

    return fd;

}

void QBluetoothSocketEngine::handleBindError(int error)
{
    switch (error) {
        case EACCES:
            m_error = QBluetoothAbstractSocket::AccessError;
            break;
        case EADDRINUSE:
            m_error = QBluetoothAbstractSocket::AddressInUseError;
            break;
        case EINVAL:
            m_error = QBluetoothAbstractSocket::UnsupportedOperationError;
            break;
        case EADDRNOTAVAIL:
            m_error = QBluetoothAbstractSocket::AddressNotAvailableError;
            break;
        case EFAULT:
        case ENOMEM:
            m_error = QBluetoothAbstractSocket::ResourceError;
            break;
        default:
            m_error = QBluetoothAbstractSocket::UnknownError;
            break;
    };
}

bool QBluetoothSocketEngine::scoBind(int fd, const QBluetoothAddress &local)
{
    struct sockaddr_sco addr;
    bdaddr_t localBdaddr;

    str2bdaddr(local.toString(), &localBdaddr);

    memset(&addr, 0, sizeof(addr));
    addr.sco_family = AF_BLUETOOTH;
    memcpy(&addr.sco_bdaddr, &localBdaddr, sizeof(bdaddr_t));

    if (::bind(fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
        handleBindError(errno);
        return false;
    }

    return true;
}

bool QBluetoothSocketEngine::rfcommBind(int fd, const QBluetoothAddress &local,
                                        int channel)
{
    struct sockaddr_rc addr;
    bdaddr_t localBdaddr;

    str2bdaddr(local.toString(), &localBdaddr);

    memset(&addr, 0, sizeof(addr));
    addr.rc_family = AF_BLUETOOTH;
    memcpy(&addr.rc_bdaddr, &localBdaddr, sizeof(bdaddr_t));
    addr.rc_channel = channel;

    if (::bind(fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
        handleBindError(errno);
        return false;
    }

    return true;
}

bool QBluetoothSocketEngine::l2capBind(int fd, const QBluetoothAddress &local,
                                       int psm, int mtu)
{
    struct sockaddr_l2 addr;
    bdaddr_t localBdaddr;

    str2bdaddr(local.toString(), &localBdaddr);

    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    memcpy(&addr.l2_bdaddr, &localBdaddr, sizeof(bdaddr_t));
    addr.l2_psm = htobs(psm);

    if (!setL2CapIncomingMtu(fd, mtu)) {
        m_error = QBluetoothAbstractSocket::ResourceError;
        return false;
    }

    if (::bind(fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
        handleBindError(errno);
        return false;
    }

    return true;
}

bool QBluetoothSocketEngine::listen(int fd, int backlog)
{
    if (::listen(fd, backlog) < 0) {
        switch (errno) {
            case EADDRINUSE:
                m_error = QBluetoothAbstractSocket::AddressInUseError;
                break;
            case EOPNOTSUPP:
                m_error = QBluetoothAbstractSocket::UnsupportedOperationError;
                break;
            default:
                m_error = QBluetoothAbstractSocket::UnknownError;
                break;
        };

        return false;
    }

    return true;
}

int QBluetoothSocketEngine::accept(int serverFd)
{
    int fd = ::accept(serverFd, 0, 0);

    if (fd < 0)
        return fd;

    ::fcntl(fd, F_SETFD, FD_CLOEXEC);

    return fd;
}

void QBluetoothSocketEngine::handleConnectError(int error)
{
    switch (error) {
        case EADDRINUSE:
            m_error = QBluetoothAbstractSocket::AddressInUseError;
            break;
        case ETIMEDOUT:
            m_error = QBluetoothAbstractSocket::TimeoutError;
            break;
        case EINVAL:
            m_error = QBluetoothAbstractSocket::UnknownError;
            break;
        case EBUSY:
            m_error = QBluetoothAbstractSocket::BusyError;
            break;
        case EADDRNOTAVAIL:
            m_error = QBluetoothAbstractSocket::AddressNotAvailableError;
            break;
        case EHOSTDOWN:
        case ENETUNREACH:
        case EHOSTUNREACH:
            m_error = QBluetoothAbstractSocket::HostUnreachableError;
            break;
        case ECONNREFUSED:
            m_error = QBluetoothAbstractSocket::ConnectionRefused;
            break;
        case EAGAIN:
            m_error = QBluetoothAbstractSocket::UnknownError;
            break;
        case EACCES:
        case EPERM:
            m_error = QBluetoothAbstractSocket::AccessError;
            break;
        default:
            m_error = QBluetoothAbstractSocket::UnknownError;
            break;
    };
}

bool QBluetoothSocketEngine::testConnected(int fd)
{
    int error = 0;
    socklen_t len = sizeof(error);

    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        m_error = QBluetoothAbstractSocket::UnknownError;
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

QBluetoothAbstractSocket::SocketState QBluetoothSocketEngine::connectSco(int fd,
        const QBluetoothAddress &local, const QBluetoothAddress &remote)
{
    struct sockaddr_sco addr;

    bdaddr_t remoteBdaddr;
    str2bdaddr(remote.toString(), &remoteBdaddr);
    bdaddr_t localBdaddr;
    str2bdaddr(local.toString(), &localBdaddr);

    memset(&addr, 0, sizeof(addr));
    addr.sco_family = AF_BLUETOOTH;
    memcpy(&addr.sco_bdaddr, &localBdaddr, sizeof(bdaddr_t));

    if (::bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        handleBindError(errno);
        return QBluetoothAbstractSocket::UnconnectedState;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sco_family = AF_BLUETOOTH;
    memcpy(&addr.sco_bdaddr, &remoteBdaddr, sizeof(bdaddr_t));

    if (::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        if (errno == EISCONN) {
            return QBluetoothAbstractSocket::ConnectedState;
        }
        else if (errno == EAGAIN || errno == EINPROGRESS || errno == EALREADY) {
            return QBluetoothAbstractSocket::ConnectingState;
        }
        else {
            handleConnectError(errno);
            return QBluetoothAbstractSocket::UnconnectedState;
        }
    }

    return QBluetoothAbstractSocket::ConnectedState;
}

QBluetoothAbstractSocket::SocketState QBluetoothSocketEngine::connectL2Cap(int fd,
        const QBluetoothAddress &local, const QBluetoothAddress &remote,
        int psm, int incomingMtu, int outgoingMtu)
{
    struct sockaddr_l2 addr;
    bdaddr_t localBdaddr;

    str2bdaddr(local.toString(), &localBdaddr);

    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    memcpy(&addr.l2_bdaddr, &localBdaddr, sizeof(bdaddr_t));

    if (::bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        handleBindError(errno);
        return QBluetoothAbstractSocket::UnconnectedState;
    }

    if (!setL2CapIncomingMtu(fd, incomingMtu) || !setL2CapOutgoingMtu(fd, outgoingMtu)) {
        m_error = QBluetoothAbstractSocket::ResourceError;
        return QBluetoothAbstractSocket::UnconnectedState;
    }

    bdaddr_t remoteBdaddr;
    str2bdaddr(remote.toString(), &remoteBdaddr);

    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    memcpy(&addr.l2_bdaddr, &remoteBdaddr, sizeof(bdaddr_t));
    addr.l2_psm = htobs(psm);

    if (::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        if (errno == EISCONN) {
            return QBluetoothAbstractSocket::ConnectedState;
        }
        else if (errno == EAGAIN || errno == EINPROGRESS || errno == EALREADY) {
            return QBluetoothAbstractSocket::ConnectingState;
        }
        else {
            handleConnectError(errno);
            return QBluetoothAbstractSocket::UnconnectedState;
        }
    }

    return QBluetoothAbstractSocket::ConnectedState;
}

QBluetoothAbstractSocket::SocketState QBluetoothSocketEngine::connectRfcomm(int fd,
        const QBluetoothAddress &local, const QBluetoothAddress &remote,
        int channel)
{
    struct sockaddr_rc addr;
    bdaddr_t localBdaddr;

    str2bdaddr(local.toString(), &localBdaddr);

    memset(&addr, 0, sizeof(addr));
    addr.rc_family = AF_BLUETOOTH;
    memcpy(&addr.rc_bdaddr, &localBdaddr, sizeof(bdaddr_t));

    if (::bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        handleBindError(errno);
        return QBluetoothAbstractSocket::UnconnectedState;
    }

    bdaddr_t remoteBdaddr;
    str2bdaddr(remote.toString(), &remoteBdaddr);

    memset(&addr, 0, sizeof(addr));
    addr.rc_family = AF_BLUETOOTH;
    memcpy(&addr.rc_bdaddr, &remoteBdaddr, sizeof(bdaddr_t));
    addr.rc_channel = channel;

    if (::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        if (errno == EISCONN) {
            return QBluetoothAbstractSocket::ConnectedState;
        }
        else if (errno == EAGAIN || errno == EINPROGRESS || errno == EALREADY) {
            return QBluetoothAbstractSocket::ConnectingState;
        }
        else {
            handleConnectError(errno);
            return QBluetoothAbstractSocket::UnconnectedState;
        }
    }

    return QBluetoothAbstractSocket::ConnectedState;
}

qint64 QBluetoothSocketEngine::writeToSocket(int fd, const char *data, qint64 len)
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
            m_error = QBluetoothAbstractSocket::RemoteHostClosedError;
            break;
        case EAGAIN:
            writtenBytes = 0;
            break;
        case EMSGSIZE:
            m_error = QBluetoothAbstractSocket::NetworkError;
            break;
        default:
            m_error = QBluetoothAbstractSocket::UnknownError;
            break;
    }
    }

    return qint64(writtenBytes);
}

qint64 QBluetoothSocketEngine::readFromSocket(int fd, char *data, qint64 len)
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
                m_error = QBluetoothAbstractSocket::NetworkError;
                break;
            case ECONNRESET:
                r = 0;
                break;
            default:
                m_error = QBluetoothAbstractSocket::UnknownError;
                break;
        }
    }

    return qint64(r);
}

void QBluetoothSocketEngine::close(int fd)
{
    ::close(fd);
}

qint64 QBluetoothSocketEngine::bytesAvailable(int fd) const
{
    size_t nbytes = 0;
    qint64 available = 0;

    if (::ioctl(fd, FIONREAD, (char *) &nbytes) >= 0)
        available = (qint64) *((int *) &nbytes);
    else
        ::perror("FIONREAD failed:");

    return available;
}

bool QBluetoothSocketEngine::getsocknameSco(int fd, QBluetoothAddress *address) const
{
    struct sockaddr_sco addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, sizeof(addr));

    if (::getsockname(fd, (struct sockaddr *) &addr, &len) == 0) {
        bdaddr_t localBdaddr;
        memcpy(&localBdaddr, &addr.sco_bdaddr, sizeof(bdaddr_t));
        QString str = bdaddr2str(&localBdaddr);
        *address = QBluetoothAddress(str);

        return true;
    }

    return false;
}

bool QBluetoothSocketEngine::getpeernameSco(int fd, QBluetoothAddress *address) const
{
    struct sockaddr_sco addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, sizeof(addr));

    if (::getpeername(fd, (struct sockaddr *) &addr, &len) == 0) {
        bdaddr_t remoteBdaddr;
        memcpy(&remoteBdaddr, &addr.sco_bdaddr, sizeof(bdaddr_t));
        QString str = bdaddr2str(&remoteBdaddr);
        *address = QBluetoothAddress(str);

        return true;
    }

    return false;
}

bool QBluetoothSocketEngine::getsocknameRfcomm(int fd,
                                               QBluetoothAddress *address,
                                               int *channel) const
{
    struct sockaddr_rc addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, sizeof(addr));

    if (::getsockname(fd, (struct sockaddr *) &addr, &len) == 0) {
        bdaddr_t localBdaddr;
        memcpy(&localBdaddr, &addr.rc_bdaddr, sizeof(bdaddr_t));
        QString str = bdaddr2str(&localBdaddr);
        *address = QBluetoothAddress(str);
        *channel = addr.rc_channel;

        return true;
    }

    return false;
}

bool QBluetoothSocketEngine::getpeernameRfcomm(int fd, QBluetoothAddress *address,
                                               int *channel) const
{
    struct sockaddr_rc addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, sizeof(addr));

    if (::getpeername(fd, (struct sockaddr *) &addr, &len) == 0) {
        bdaddr_t remoteBdaddr;
        memcpy(&remoteBdaddr, &addr.rc_bdaddr, sizeof(bdaddr_t));
        QString str = bdaddr2str(&remoteBdaddr);
        *address = QBluetoothAddress(str);
        *channel = addr.rc_channel;

        return true;
    }

    return false;
}

static void decode_sockaddr_l2(struct sockaddr_l2 *addr, QBluetoothAddress *ret, int *psm)
{
    if (ret) {
        bdaddr_t bdaddr;
        memcpy(&bdaddr, &addr->l2_bdaddr, sizeof(bdaddr_t));
        QString str = bdaddr2str(&bdaddr);
        *ret = QBluetoothAddress(str);
    }

    if (psm) {
        *psm = btohs(addr->l2_psm);
    }
}

bool QBluetoothSocketEngine::getsocknameL2Cap(int fd,
                                              QBluetoothAddress *address,
                                              int *psm) const
{
    struct sockaddr_l2 addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, sizeof(addr));

    if (::getsockname(fd, (struct sockaddr *) &addr, &len) == 0) {
        decode_sockaddr_l2(&addr, address, psm);
        return true;
    }

    return false;
}

bool QBluetoothSocketEngine::getpeernameL2Cap(int fd, QBluetoothAddress *address,
                                              int *psm) const
{
    struct sockaddr_l2 addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, sizeof(addr));

    if (::getpeername(fd, (struct sockaddr *) &addr, &len) == 0) {
        decode_sockaddr_l2(&addr, address, psm);
        return true;
    }

    return false;
}

bool QBluetoothSocketEngine::setL2CapIncomingMtu(int fd, int imtu)
{
    struct l2cap_options opts;
    memset(&opts, 0, sizeof(opts));
    socklen_t optlen = sizeof(opts);

    if (::getsockopt(fd, SOL_L2CAP, L2CAP_OPTIONS, &opts, &optlen) == -1) {
        return false;
    }

    opts.imtu = imtu;

    if (setsockopt(fd, SOL_L2CAP, L2CAP_OPTIONS, &opts, sizeof(opts)) == -1) {
        return false;
    }

    return true;
}

bool QBluetoothSocketEngine::getL2CapIncomingMtu(int fd, int *imtu) const
{
    struct l2cap_options opts;
    memset(&opts, 0, sizeof(opts));
    socklen_t optlen = sizeof(opts);

    if (::getsockopt(fd, SOL_L2CAP, L2CAP_OPTIONS, &opts, &optlen) == -1) {
        return false;
    }

    *imtu = opts.imtu;

    return true;
}

bool QBluetoothSocketEngine::setL2CapOutgoingMtu(int fd, int omtu)
{
    struct l2cap_options opts;
    memset(&opts, 0, sizeof(opts));
    socklen_t optlen = sizeof(opts);

    if (::getsockopt(fd, SOL_L2CAP, L2CAP_OPTIONS, &opts, &optlen) == -1) {
        return false;
    }

    opts.omtu = omtu;

    if (setsockopt(fd, SOL_L2CAP, L2CAP_OPTIONS, &opts, sizeof(opts)) == -1) {
        return false;
    }

    return true;
}

bool QBluetoothSocketEngine::getL2CapOutgoingMtu(int fd, int *omtu) const
{
    struct l2cap_options opts;
    memset(&opts, 0, sizeof(opts));
    socklen_t optlen = sizeof(opts);

    if (::getsockopt(fd, SOL_L2CAP, L2CAP_OPTIONS, &opts, &optlen) == -1) {
        return false;
    }

    *omtu = opts.omtu;

    return true;
}

bool QBluetoothSocketEngine::getScoMtu(int fd, int *mtu) const
{
    struct sco_options opts;
    socklen_t len = sizeof(opts);

    if (::getsockopt(fd, SOL_SCO, SCO_OPTIONS, &opts, &len) == 0) {
        *mtu = opts.mtu;

        return true;
    }

    qWarning("Wasn't able to get Sco MTU!");
    return false;
}

bool QBluetoothSocketEngine::setSocketOption(int fd, QBluetoothSocketEngine::SocketOption option)
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

int QBluetoothSocketEngine::select(QBluetoothSocketEngine::SelectTypes types, int fd, int timeout,
                           bool *canRead, bool *canWrite) const
{
    fd_set write_fds;
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);

    if (types & QBluetoothSocketEngine::SelectRead) {
        FD_SET(fd, &read_fds);
    }

    if (types & QBluetoothSocketEngine::SelectWrite) {
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

bool QBluetoothSocketEngine::waitFor(QBluetoothSocketEngine::SelectTypes types,
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

            m_error = QBluetoothAbstractSocket::TimeoutError;
            return false;
        }

        if (ret > 0)
            return true;

        // Have an error condition, continue on interrupt
        if (errno == EINTR)
            continue;

        break;
    }

    m_error = QBluetoothAbstractSocket::UnknownError;
    return false;
}

qint64 QBluetoothSocketEngine::recvfrom(int fd, char *data, qint64 maxSize,
                                        QBluetoothAddress *address, int *psm)
{
    struct sockaddr_l2 cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    socklen_t len = sizeof(cliaddr);

    ssize_t r = 0;
    do {
        r = ::recvfrom(fd, data, maxSize, 0, (struct sockaddr *)&cliaddr, &len);
    } while (r == -1 && errno == EINTR);

    if (r < 0) {
        m_error = QBluetoothAbstractSocket::NetworkError;
        return r;
    }

    if (address || psm) {
        decode_sockaddr_l2(&cliaddr, address, psm);
    }

    return qint64(r);
}
