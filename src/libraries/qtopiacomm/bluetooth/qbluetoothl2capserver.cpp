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
#include <QTimer>
#include <QTime>
#include <QtAlgorithms>

#include <qbluetoothl2capserver.h>
#include <qbluetoothl2capsocket.h>
#include "qbluetoothnamespace_p.h"
#include <qbluetoothaddress.h>

#include "qbluetoothabstractserver_p.h"
#include "qbluetoothsocketengine_p.h"

class QBluetoothL2CapServerPrivate : public QBluetoothAbstractServerPrivate
{
public:
    QBluetoothL2CapServerPrivate(QBluetoothL2CapServer *parent);
    int m_psm;
    QBluetoothAddress m_address;
    QBluetooth::SecurityOptions m_options;
};

QBluetoothL2CapServerPrivate::QBluetoothL2CapServerPrivate(QBluetoothL2CapServer *parent)
    : QBluetoothAbstractServerPrivate(parent)
{

}

/*!
    \class QBluetoothL2CapServer
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothL2CapServer class represents an L2CAP server socket.

    This class makes it possible to accept incoming L2CAP connections.
    You can specify the address and the L2CAP PSM to listen on.

    Call listen() to make the server listen for new connections.  The
    newConnection() signal will be emmited each time a client connects
    to the server.  The MTU to use should be specified during the listen()
    call.

    Security options controlling the authentication and encryption of the
    Bluetooth link on the socket can be specified at any time by calling
    setSecurityOptions().  Currently this will only affect all future
    connections, not connections which are active or pending.

    Call nextPendingConnection() to accept the pending client connection.

    When listening for connections, server address and PSM are available
    by calling serverAddress() and serverPsm().

    Calling close() will make the QBluetoothL2CapServer stop listening
    for connections and delete all pending connections.

    \code
        // Create a new L2CapServer on PSM 25
        QBluetoothL2CapServer *server = new QBluetoothL2CapServer;
        server->listen(QBluetoothAddress::any, 25);

        // wait for newConnection() signal

        QBluetoothL2CapSocket *sock =
            qobject_cast<QBluetoothL2CapSocket *>( sock->nextPendingConnection() );

        server->close();
    \endcode

    \ingroup qtopiabluetooth
    \sa QBluetoothL2CapSocket
 */

/*!
    Constructs a new QBluetoothL2CapServer with parent \a parent.
    The server is in the UnconnectedState.
 */
QBluetoothL2CapServer::QBluetoothL2CapServer(QObject *parent)
    : QBluetoothAbstractServer(new QBluetoothL2CapServerPrivate(this), parent)
{
    SERVER_DATA(QBluetoothL2CapServer);
    m_data->m_psm = -1;
}

/*!
    Destroys the server.
*/
QBluetoothL2CapServer::~QBluetoothL2CapServer()
{
}

/*!
    Tells the server to listen for incoming connections on address \a local
    and PSM \a psm.  The \a mtu parameter holds the MTU to use for the server.

    PSM stands for Port and Service Multiplexer.  The PSM value can be
    any \bold{odd} value the range of 1-32765.  For more information
    please see Bluetooth Specification Version 2.0 + EDR [vol 4]
    page 45.

    Returns true if the server successfully started listening;
    otherwise returns false.
    If the server was already listening, returns false.

    \sa isListening()
 */
bool QBluetoothL2CapServer::listen(const QBluetoothAddress &local, int psm, int mtu)
{
    SERVER_DATA(QBluetoothL2CapServer);

    if (isListening()) {
        return false;
    }

    m_data->m_fd = m_data->m_engine->l2capSocket();

    if (m_data->m_fd < 0) {
        return false;
    }

    m_data->m_engine->setSocketOption(m_data->m_fd,
                                      QBluetoothSocketEngine::NonBlockingOption);

    if (!m_data->m_engine->l2capBind(m_data->m_fd, local, psm, mtu)) {
        m_data->m_engine->close(m_data->m_fd);
        m_data->m_fd = -1;
        return false;
    }

    if (!m_data->m_engine->listen(m_data->m_fd, 1)) {
        m_data->m_engine->close(m_data->m_fd);
        m_data->m_fd = -1;
        return false;
    }

    setListening();

    m_data->m_psm = psm;
    m_data->m_address = local;

    m_data->m_engine->getsocknameL2Cap(m_data->m_fd, &m_data->m_address, &m_data->m_psm);

    return true;
}

/*!
    \reimp
 */
void QBluetoothL2CapServer::close()
{
    SERVER_DATA(QBluetoothL2CapServer);

    QBluetoothAbstractServer::close();

    m_data->m_address = QBluetoothAddress::invalid;
    m_data->m_psm = -1;
}

/*!
    Returns the L2CAP PSM the server is currently listening on.  If the server
    is not listening, returns -1.

    \sa serverAddress()
 */
int QBluetoothL2CapServer::serverPsm() const
{
    SERVER_DATA(QBluetoothL2CapServer);

    return m_data->m_psm;
}

/*!
    Returns the address the server is currently listening on.  If the server
    is not listening, returns QBluetoothAddress::invalid.

    \sa serverPsm()
 */
QBluetoothAddress QBluetoothL2CapServer::serverAddress() const
{
    SERVER_DATA(QBluetoothL2CapServer);

    return m_data->m_address;
}

/*!
    Returns true if the socket is encrypted.

    \sa securityOptions(), isAuthenticated()
 */
bool QBluetoothL2CapServer::isEncrypted() const
{
    return securityOptions() & QBluetooth::Encrypted;
}

/*!
    Returns true if the socket is authenticated.

    \sa securityOptions(), isEncrypted()
 */
bool QBluetoothL2CapServer::isAuthenticated() const
{
    return securityOptions() & QBluetooth::Authenticated;
}

/*!
    Returns the security options currently active for the socket.

    \sa setSecurityOptions()
 */
QBluetooth::SecurityOptions QBluetoothL2CapServer::securityOptions() const
{
    SERVER_DATA(QBluetoothL2CapServer);

    if (!isListening())
        return m_data->m_options;

    if (_q_getL2CapSecurityOptions(socketDescriptor(), m_data->m_options))
        return m_data->m_options;

    return 0;
}

/*!
    Returns true if able to set the security options of the socket to be \a options; otherwise returns false.
    Note that under the current Linux implementation only new connections will be affected by the change in security
    options.  Existing and pending connections will not be affected.

    \sa securityOptions()
 */
bool QBluetoothL2CapServer::setSecurityOptions(QBluetooth::SecurityOptions options)
{
    SERVER_DATA(QBluetoothL2CapServer);

    m_data->m_options = options;

    if (!isListening())
        return true;

    return _q_setL2CapSecurityOptions(socketDescriptor(), m_data->m_options);
}

/*!
    \reimp
 */
QBluetoothAbstractSocket * QBluetoothL2CapServer::createSocket()
{
    return new QBluetoothL2CapSocket(this);
}

/*!
    Returns the MTU that should be used for this server.  By default the MTU
    is set to be 672 bytes.  If the socket is not listening, or an
    error occurred, returns -1.
*/
int QBluetoothL2CapServer::mtu() const
{
    SERVER_DATA(QBluetoothL2CapServer);

    if (!isListening()) {
        return -1;
    }

    int imtu;
    if (m_data->m_engine->getL2CapIncomingMtu(socketDescriptor(), &imtu)) {
        return imtu;
    }

    return -1;
}
