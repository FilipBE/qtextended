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

#include <qbluetoothl2capsocket.h>
#include "qbluetoothnamespace_p.h"
#include "qbluetoothabstractsocket_p.h"
#include <qbluetoothaddress.h>
#include "qbluetoothsocketengine_p.h"

class QBluetoothL2CapSocketPrivate : public QBluetoothAbstractSocketPrivate
{
public:
    QBluetoothL2CapSocketPrivate();

    QBluetoothAddress m_local;
    QBluetoothAddress m_remote;
    int m_remotePsm;
    int m_imtu;
    int m_omtu;
};

QBluetoothL2CapSocketPrivate::QBluetoothL2CapSocketPrivate()
    : QBluetoothAbstractSocketPrivate(true)
{
    m_remotePsm = -1;
    m_imtu = 0;
    m_omtu = 0;
}

/*!
    \class QBluetoothL2CapSocket
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothL2CapSocket class represents an L2CAP client socket.

    The Bluetooth L2CAP protocol provides reliable connection-oriented and unreliable
    connectionless data services.  It is a lower layer protocol than RFCOMM.
    The implementation of L2CAP protocol under Linux uses sockets, and is in general
    very similar to TCP socket programming.

    PSM stands for Port and Service Multiplexer.  The PSM value can be
    any \bold{odd} value the range of 1-32765.  For more information
    please see Bluetooth Specification Version 2.0 + EDR [vol 4]
    page 45.

    The PSM and address of the connected peer are fetched by calling remotePsm() and
    remoteAddress() respectively.  The localAddress() returns the address of the local
    device being used by the socket.

    Note that special attention should be paid to the incoming and outgoing
    MTU sizes.  These determine in what size chunks the data should be read / written.
    If preserving the record boundaries when reading is important, then the read buffer size
    should be set to the value of incomingMtu() using setReadBufferSize() when the
    socket is connected.  If record boundaries are important when writing,
    the data should only be fed one packet of size <= outgoingMtu() at a time by using
    write and then waiting for the bytesWritten() signal before sending more data.

    \ingroup qtopiabluetooth
    \sa QBluetoothL2CapServer
 */

/*!
    Constructs a new QBluetoothL2CapSocket object.  The \a parent parameter
    is passed to the QObject constructor.
 */
QBluetoothL2CapSocket::QBluetoothL2CapSocket(QObject *parent)
    : QBluetoothAbstractSocket(new QBluetoothL2CapSocketPrivate, parent)
{
}

/*!
    Destroys the socket.
 */
QBluetoothL2CapSocket::~QBluetoothL2CapSocket()
{
}

/*!
    Returns the address of the remote device.  If the socket is not currently
    connected, returns QBluetoothAddress::invalid.

    \sa remotePsm(), localAddress()
 */
QBluetoothAddress QBluetoothL2CapSocket::remoteAddress() const
{
    SOCKET_DATA(QBluetoothL2CapSocket);
    return m_data->m_remote;
}

/*!
    Returns the address of the local device.  If the socket is not currently
    connected, returns QBluetoothAddress::invalid.

    \sa remotePsm(), remoteAddress()
 */
QBluetoothAddress QBluetoothL2CapSocket::localAddress() const
{
    SOCKET_DATA(QBluetoothL2CapSocket);
    return m_data->m_local;
}

/*!
    Returns the PSM of the remote device.  If the socket is not
    currently connected, returns -1.

    \sa remoteAddress(), localAddress()
 */
int QBluetoothL2CapSocket::remotePsm() const
{
    SOCKET_DATA(QBluetoothL2CapSocket);
    return m_data->m_remotePsm;
}

/*!
    Returns true if the socket is encrypted.

    \sa securityOptions(), isAuthenticated()
 */
bool QBluetoothL2CapSocket::isEncrypted() const
{
    return securityOptions() & QBluetooth::Encrypted;
}

/*!
    Returns true if the socket is authenticated.

    \sa securityOptions(), isEncrypted()
 */
bool QBluetoothL2CapSocket::isAuthenticated() const
{
    return securityOptions() & QBluetooth::Authenticated;
}

/*!
    Returns the security options currently active for the socket.

    \sa isAuthenticated(), isEncrypted()
 */
QBluetooth::SecurityOptions QBluetoothL2CapSocket::securityOptions() const
{
    if (state() == QBluetoothL2CapSocket::UnconnectedState)
        return 0;

    QBluetooth::SecurityOptions options;

    if (_q_getL2CapSecurityOptions(socketDescriptor(), options))
        return options;

    return 0;
}

/*!
    Returns the MTU for incoming data.  The underlying implementation will accept
    data packets of size no bigger than the incoming MTU.  The MTU information is
    provided so that the read buffer size can be set appropriately.

    \sa outgoingMtu()
*/
int QBluetoothL2CapSocket::incomingMtu() const
{
    SOCKET_DATA(QBluetoothL2CapSocket);
    return m_data->m_imtu;
}

/*!
    Returns the MTU for outgoing data.  The underlying implementation will accept
    outgoing data packets of size no bigger than the outgoing MTU.

    \sa incomingMtu()
*/
int QBluetoothL2CapSocket::outgoingMtu() const
{
    SOCKET_DATA(QBluetoothL2CapSocket);
    return m_data->m_omtu;
}

/*!
    \reimp
*/
bool QBluetoothL2CapSocket::readSocketParameters(int socket)
{
    SOCKET_DATA(QBluetoothL2CapSocket);

    int dummy;
    m_data->m_engine->getsocknameL2Cap(socket, &m_data->m_local, &dummy);
    m_data->m_engine->getpeernameL2Cap(socket, &m_data->m_remote, &m_data->m_remotePsm);

    if (m_data->m_engine->getL2CapIncomingMtu(socket, &m_data->m_imtu)) {
        setReadMtu(m_data->m_imtu);
    }

    if (m_data->m_engine->getL2CapOutgoingMtu(socket, &m_data->m_omtu)) {
        setWriteMtu(m_data->m_omtu);
    }

    return true;
}

/*!
    Attempts to open a L2CAP connection between the local device with
    address \a local and the remote device with address \a remote.  The
    L2CAP PSM to use is given by \a psm.  This function should
    generally return immediately, and the socket will enter into the
    \c ConnectingState.

    The \a incomingMtu and \a outgoingMtu represent the MTU sizes to use
    for incoming and outgoing data respectively.  These parameters should
    only be considered hints and are subject to negotiation by the underlying
    stack implementation.  If the connection succeeds the actual MTUs should
    be queried by using incomingMtu() and outgoingMtu() methods.

    Optionally, the client can request that that the connection be secured
    by specifying the \a options parameter.  \bold NOTE: This feature
    might not work under some systems.

    The function returns true if the connection could be started,
    and false otherwise.

    Note that the connection could still fail, the state of the socket
    will be sent in the stateChanged() signal.

    \sa state(), connected(), waitForConnected()
 */
bool QBluetoothL2CapSocket::connect(const QBluetoothAddress &local,
                                     const QBluetoothAddress &remote,
                                     int psm,
                                     int incomingMtu,
                                     int outgoingMtu,
                                     QBluetooth::SecurityOptions options)
{
    if (state() != QBluetoothAbstractSocket::UnconnectedState)
        return false;

    resetSocketParameters();

    int sockfd = m_data->m_engine->l2capSocket();

    if (sockfd < 0) {
        setError(m_data->m_engine->error());
        return false;
    }

    m_data->m_engine->setSocketOption(sockfd, QBluetoothSocketEngine::NonBlockingOption);

    _q_setL2CapSecurityOptions(sockfd, options);

    QBluetoothAbstractSocket::SocketState connectState =
            m_data->m_engine->connectL2Cap(sockfd, local, remote, psm,
                                           incomingMtu, outgoingMtu);

    return handleConnect(sockfd, connectState);
}

/*!
    \reimp
*/
void QBluetoothL2CapSocket::resetSocketParameters()
{
    SOCKET_DATA(QBluetoothL2CapSocket);

    m_data->m_local = QBluetoothAddress::invalid;
    m_data->m_remote = QBluetoothAddress::invalid;
    m_data->m_remotePsm = -1;
    m_data->m_omtu = 0;
    m_data->m_imtu = 0;
}
