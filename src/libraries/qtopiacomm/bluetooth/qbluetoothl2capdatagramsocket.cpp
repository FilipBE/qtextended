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

#include <QSocketNotifier>

#include <qbluetoothl2capdatagramsocket.h>
#include "qbluetoothnamespace_p.h"
#include "qbluetoothabstractsocket_p.h"
#include <qbluetoothaddress.h>
#include "qbluetoothsocketengine_p.h"

class QBluetoothL2CapDatagramSocketPrivate : public QBluetoothAbstractSocketPrivate
{
public:
    QBluetoothL2CapDatagramSocketPrivate();

    QBluetoothAddress m_local;
    QBluetoothAddress m_remote;
    int m_remotePsm;
    int m_localPsm;
    int m_imtu;
    int m_omtu;
    QBluetooth::SecurityOptions m_options;
};

QBluetoothL2CapDatagramSocketPrivate::QBluetoothL2CapDatagramSocketPrivate()
    : QBluetoothAbstractSocketPrivate(false)
{
    m_remotePsm = -1;
    m_localPsm = -1;
    m_imtu = 0;
    m_omtu = 0;
    m_options = 0;
}

/*!
    \class QBluetoothL2CapDatagramSocket
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothL2CapDatagramSocket class represents an L2CAP Datagram Socket.

    The Bluetooth L2CAP protocol provides reliable connection-oriented
    and unreliable connectionless data services.  It is a lower
    layer protocol than RFCOMM.

    PSM stands for Port and Service Multiplexer.  The PSM value can be
    any \bold{odd} value the range of 1-32765.  For more information
    please see Bluetooth Specification Version 2.0 + EDR [vol 4]
    page 45.

    The implementation of L2CAP Datagram protocol under Linux uses sockets,
    and is in general very similar to UDP socket programming.

    The most common way to use this class is to call connect() and then
    write() and read() to write and read the data.  If you only wish
    to receive data, call bind() and use the readDatagram() function.

    Note that unlike UDP, the L2Cap Datagram socket must be connected
    first by using connect().

    Note that special attention should be paid to the incoming and outgoing
    MTU sizes.  These determine in what size chunks the data should be
    read / written.

    \bold{NOTE:} The socket is not buffered.  All reads and writes happen
    immediately.  The user must be prepared to deal with conditions where
    no more data could be read/written to the socket.

    \ingroup qtopiabluetooth
    \sa QBluetoothAbstractSocket
 */

/*!
    Constructs a new QBluetoothL2CapDatagramSocket object.
    The \a parent parameter is passed to the QObject constructor.
 */
QBluetoothL2CapDatagramSocket::QBluetoothL2CapDatagramSocket(QObject *parent)
    : QBluetoothAbstractSocket(new QBluetoothL2CapDatagramSocketPrivate, parent)
{
}

/*!
    Destroys the socket.
 */
QBluetoothL2CapDatagramSocket::~QBluetoothL2CapDatagramSocket()
{
}

/*!
    Returns the address of the remote device.  If the socket is not currently
    connected, returns QBluetoothAddress::invalid.

    \sa remotePsm()
 */
QBluetoothAddress QBluetoothL2CapDatagramSocket::remoteAddress() const
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);
    return m_data->m_remote;
}

/*!
    Returns the address of the local device.  If the socket is not currently
    connected or bound, returns QBluetoothAddress::invalid.

    \sa localPsm()
 */
QBluetoothAddress QBluetoothL2CapDatagramSocket::localAddress() const
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);
    return m_data->m_local;
}

/*!
    Returns the PSM of the remote device.  If the socket is not
    currently connected, returns -1.

    \sa remoteAddress()
 */
int QBluetoothL2CapDatagramSocket::remotePsm() const
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);
    return m_data->m_remotePsm;
}

/*!
    Returns the PSM that the socket is bound to on the local device.
    If the socket is not bound, returns -1

    \sa localAddress()
*/
int QBluetoothL2CapDatagramSocket::localPsm() const
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);
    return m_data->m_localPsm;
}

/*!
    Returns true if the socket is encrypted.

    \sa securityOptions(), isAuthenticated()
 */
bool QBluetoothL2CapDatagramSocket::isEncrypted() const
{
    return securityOptions() & QBluetooth::Encrypted;
}

/*!
    Returns true if the socket is authenticated.

    \sa securityOptions(), isEncrypted()
 */
bool QBluetoothL2CapDatagramSocket::isAuthenticated() const
{
    return securityOptions() & QBluetooth::Authenticated;
}

/*!
    Returns the security options currently active for the socket.

    \sa isAuthenticated(), isEncrypted()
 */
QBluetooth::SecurityOptions QBluetoothL2CapDatagramSocket::securityOptions() const
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);
    if (state() == QBluetoothL2CapDatagramSocket::UnconnectedState)
        return m_data->m_options;

    if (_q_getL2CapSecurityOptions(socketDescriptor(), m_data->m_options))
        return m_data->m_options;

    return 0;
}

/*!
    Sets the security options on the socket to \a options.  Returns true if the
    options could be set successfully and false otherwise.

    \sa securityOptions(), isAuthenticated(), isEncrypted()
*/
bool QBluetoothL2CapDatagramSocket::setSecurityOptions(QBluetooth::SecurityOptions options)
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);
    m_data->m_options = options;

    if (state() == QBluetoothL2CapDatagramSocket::UnconnectedState)
        return true;

    return _q_setL2CapSecurityOptions(socketDescriptor(), m_data->m_options);
}

/*!
    Returns the MTU for incoming data.  The underlying implementation will accept
    data packets of size no bigger than the incoming MTU.  The MTU information is
    provided so that the read buffer size can be set appropriately.

    \sa outgoingMtu()
 */
int QBluetoothL2CapDatagramSocket::incomingMtu() const
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);
    return m_data->m_imtu;
}

/*!
    Returns the MTU for outgoing data.  The underlying implementation will accept
    outgoing data packets of size no bigger than the outgoing MTU.

    \sa incomingMtu()
 */
int QBluetoothL2CapDatagramSocket::outgoingMtu() const
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);
    return m_data->m_omtu;
}

/*!
    \internal

    \reimp
*/
bool QBluetoothL2CapDatagramSocket::readSocketParameters(int socket)
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);

    bool ret = m_data->m_engine->getsocknameL2Cap(socket,
            &m_data->m_local, &m_data->m_localPsm);

    if (!ret)
        return ret;

    ret = m_data->m_engine->getpeernameL2Cap(socket,
            &m_data->m_remote, &m_data->m_remotePsm);

    if (!ret)
        return ret;

    m_data->m_engine->getL2CapIncomingMtu(socket, &m_data->m_imtu);
    m_data->m_engine->getL2CapOutgoingMtu(socket, &m_data->m_omtu);

    return true;
}

/*!
    Attempts to open a L2CAP connection between the local device with
    address \a local and the remote device with address \a remote.  The
    L2CAP PSM to use is given by \a psm.  This function should
    generally return immediately, and the socket will enter into the
    \c ConnectingState.

    The \a incomingMtu and \a outgoingMtu represent the MTU sizes to use
    for incoming and outgoing data respectively.

    The function returns true if the connection could be started,
    and false otherwise.

    Note that the connection could still fail, the state of the socket
    will be sent in the stateChanged() signal.

    \sa connected(), stateChanged(), waitForConnected()
 */
bool QBluetoothL2CapDatagramSocket::connect(const QBluetoothAddress &local,
                                            const QBluetoothAddress &remote,
                                            int psm,
                                            int incomingMtu,
                                            int outgoingMtu)
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);

    if (state() != QBluetoothAbstractSocket::UnconnectedState)
        return false;

    resetSocketParameters();

    int sockfd = m_data->m_engine->l2capDgramSocket();

    if (sockfd < 0) {
        setError(m_data->m_engine->error());
        return false;
    }

    m_data->m_engine->setSocketOption(sockfd, QBluetoothSocketEngine::NonBlockingOption);

    _q_setL2CapSecurityOptions(sockfd, m_data->m_options);

    QBluetoothAbstractSocket::SocketState connectState =
            m_data->m_engine->connectL2Cap(sockfd, local, remote, psm,
                                           incomingMtu, outgoingMtu);

    return handleConnect(sockfd, connectState);
}

/*!
    \internal

    \reimp
*/
void QBluetoothL2CapDatagramSocket::resetSocketParameters()
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);

    m_data->m_local = QBluetoothAddress::invalid;
    m_data->m_remote = QBluetoothAddress::invalid;
    m_data->m_remotePsm = -1;
    m_data->m_localPsm = -1;
    m_data->m_omtu = 0;
    m_data->m_imtu = 0;
    m_data->m_options = 0;
}

/*!
    Binds an L2CAP socket to a specific \a local address and \a psm, returning true if
    successful; otherwise returns false. The \a mtu specifies the MTU to use.

    \sa 
*/
bool QBluetoothL2CapDatagramSocket::bind(const QBluetoothAddress &local, int psm, int mtu)
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);

    if (state() != QBluetoothAbstractSocket::UnconnectedState)
        return false;

    int fd = m_data->m_engine->l2capDgramSocket();

    if (fd < 0) {
        setError(m_data->m_engine->error());
        emit error(m_data->m_error);
        return false;
    }

    if (!m_data->m_engine->l2capBind(fd, local, psm, mtu)) {
        m_data->m_engine->close(fd);
        setError(m_data->m_engine->error());
        emit error(m_data->m_error);
        return false;
    }

    m_data->m_engine->setSocketOption(fd, QBluetoothSocketEngine::NonBlockingOption);

    m_data->m_fd = fd;

    m_data->m_engine->getsocknameL2Cap(fd, &m_data->m_local, &m_data->m_localPsm);
    m_data->setupNotifiers();

    m_data->m_state = QBluetoothAbstractSocket::BoundState;
    emit stateChanged(m_data->m_state);

    return true;
}

/*!
    Reads a datagram from the socket.  The \a data specifies the pointer
    to a buffer of at least \a maxSize.  The \a address and \a psm parameters
    specify where to store the address and psm of the remote device
    that sent the datagram.  The result is discarded if address
    or psm are NULL.

    If the \a data buffer \a maxSize is smaller than the incoming MTU,
    the data that does not fit will be discarded.

    Note: Some Linux implementations do not currently return
    remote address information.

    \sa incomingMtu()
*/
qint64 QBluetoothL2CapDatagramSocket::readDatagram(char * data, qint64 maxSize,
                                                   QBluetoothAddress *address,
                                                   int *psm)
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);

    if (m_data->m_state != QBluetoothAbstractSocket::BoundState)
        return -1;

    qint64 r = m_data->m_engine->recvfrom(m_data->m_fd, data, maxSize, address, psm);
    if (r < 0) {
        setError(m_data->m_engine->error());
        emit error(m_data->m_error);
    }

    m_data->m_readNotifier->setEnabled(true);

    return qint64(r);
}
