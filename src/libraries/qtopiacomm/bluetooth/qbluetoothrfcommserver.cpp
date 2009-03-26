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

#include <qbluetoothrfcommserver.h>
#include <qbluetoothrfcommsocket.h>
#include "qbluetoothnamespace_p.h"
#include <qbluetoothaddress.h>
#include "qbluetoothsocketengine_p.h"
#include "qbluetoothabstractserver_p.h"

class QBluetoothRfcommServerPrivate : public QBluetoothAbstractServerPrivate
{
public:
    QBluetoothRfcommServerPrivate(QBluetoothRfcommServer *parent);
    int m_channel;
    QBluetoothAddress m_address;
    QBluetooth::SecurityOptions m_options;
};

QBluetoothRfcommServerPrivate::QBluetoothRfcommServerPrivate(QBluetoothRfcommServer *parent) : QBluetoothAbstractServerPrivate(parent)
{
}

/*!
    \class QBluetoothRfcommServer
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothRfcommServer class represents an RFCOMM server socket.

    This class makes it possible to accept incoming RFCOMM connections.
    You can specify the address and the RFCOMM channel to listen on.

    Call listen() to make the server listen for new connections.  The
    newConnection() signal will be emmited each time a client connects
    to the server.

    Security options controlling the authentication and encryption of the
    Bluetooth link on the socket can be specified at any time by calling
    setSecurityOptions().  Currently this will only affect all future
    connections, not connections which are active or pending.

    Call nextPendingConnection() to accept the pending client connection.

    When listening for connections, server address and channel are available
    by calling serverAddress() and serverChannel().

    Calling close() will make the QBluetoothRfcommServer stop listening
    for connections and delete all pending connections.

    \sa QBluetoothRfcommSocket

    \ingroup qtopiabluetooth
*/

/*!
    Constructs a new QBluetoothRfcommServer with parent \a parent.
    The server is in the UnconnectedState.
*/
QBluetoothRfcommServer::QBluetoothRfcommServer(QObject *parent)
    : QBluetoothAbstractServer(new QBluetoothRfcommServerPrivate(this), parent)
{
    SERVER_DATA(QBluetoothRfcommServer);

    m_data->m_channel = -1;
    m_data->m_options = 0;
}

/*!
    Destroys the server.
*/
QBluetoothRfcommServer::~QBluetoothRfcommServer()
{
}

/*!
    Tells the server to listen for incoming connections on address \a local
    and channel \a channel.

    The channel can be any value between 1 and 30.

    Returns true on success; otherwise returns false.

    \sa isListening()
*/
bool QBluetoothRfcommServer::listen(const QBluetoothAddress &local, int channel)
{
    SERVER_DATA(QBluetoothRfcommServer);

    if (isListening()) {
        return false;
    }

    m_data->m_fd = m_data->m_engine->rfcommSocket();

    if (m_data->m_fd < 0) {
        return false;
    }

    m_data->m_engine->setSocketOption(m_data->m_fd,
                                      QBluetoothSocketEngine::NonBlockingOption);

    if (!m_data->m_engine->rfcommBind(m_data->m_fd, local, channel)) {
        m_data->m_engine->close(m_data->m_fd);
        m_data->m_fd = -1;
        return false;
    }

    if (!m_data->m_engine->listen(m_data->m_fd, 1)) {
        m_data->m_engine->close(m_data->m_fd);
        m_data->m_fd = -1;
        return false;
    }

    if (!_q_setSecurityOptions(m_data->m_fd, m_data->m_options))
        qWarning("Cannot set security options for RFCOMM socket %d", m_data->m_fd); 
    setListening();

    m_data->m_channel = channel;
    m_data->m_address = local;

    m_data->m_engine->getsocknameRfcomm(m_data->m_fd,
                                        &m_data->m_address, &m_data->m_channel);
    return true;
}

/*!
    \reimp
*/
void QBluetoothRfcommServer::close()
{
    SERVER_DATA(QBluetoothRfcommServer);

    QBluetoothAbstractServer::close();

    m_data->m_address = QBluetoothAddress::invalid;
    m_data->m_channel = -1;
}

/*!
    Returns the RFCOMM channel the server is currently listening on.  If the server
    is not listening, returns -1.

    \sa serverAddress()
*/
int QBluetoothRfcommServer::serverChannel() const
{
    SERVER_DATA(QBluetoothRfcommServer);
    return m_data->m_channel;
}

/*!
    Returns the address the server is currently listening on.  If the server
    is not listening, returns QBluetoothAddress::invalid.

    \sa serverChannel()
 */
QBluetoothAddress QBluetoothRfcommServer::serverAddress() const
{
    SERVER_DATA(QBluetoothRfcommServer);
    return m_data->m_address;
}

/*!
    Returns true if the socket is encrypted.

    \sa securityOptions(), isAuthenticated()
 */
bool QBluetoothRfcommServer::isEncrypted() const
{
    return securityOptions() & QBluetooth::Encrypted;
}

/*!
    Returns true if the socket is authenticated.

    \sa securityOptions(), isEncrypted()
 */
bool QBluetoothRfcommServer::isAuthenticated() const
{
    return securityOptions() & QBluetooth::Authenticated;
}

/*!
    Returns the security options currently active for the socket.

    \sa setSecurityOptions()
 */
QBluetooth::SecurityOptions QBluetoothRfcommServer::securityOptions() const
{
    SERVER_DATA(QBluetoothRfcommServer);

    if (!isListening()) {
        return m_data->m_options;
    }

    if (_q_getSecurityOptions(socketDescriptor(), m_data->m_options))
        return m_data->m_options;

    return 0;
}

/*!
    Returns true if able to set the security options of the socket to be \a options; otherwise returns false.

    \sa securityOptions()
 */
bool QBluetoothRfcommServer::setSecurityOptions(QBluetooth::SecurityOptions options)
{
    SERVER_DATA(QBluetoothRfcommServer);

    m_data->m_options = options;

    if (!isListening()) {
        return true;
    }

    return _q_setSecurityOptions(socketDescriptor(), m_data->m_options);
}

/*!
    \reimp
 */
QBluetoothAbstractSocket * QBluetoothRfcommServer::createSocket()
{
    return new QBluetoothRfcommSocket(this);
}
