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
#include <QPointer>

#include <qbluetoothabstractsocket.h>
#include <qbluetoothabstractserver.h>
#include "qbluetoothnamespace_p.h"
#include <qbluetoothaddress.h>
#include "qbluetoothsocketengine_p.h"
#include "qbluetoothabstractserver_p.h"

QBluetoothAbstractServerPrivate::QBluetoothAbstractServerPrivate(QBluetoothAbstractServer *parent)
{
    m_parent = parent;
    m_isListening = false;
    m_maxConnections = 1;
    m_readNotifier = 0;
    m_engine = new QBluetoothSocketEngine;
}

QBluetoothAbstractServerPrivate::~QBluetoothAbstractServerPrivate()
{
    if (m_readNotifier)
        delete m_readNotifier;

    delete m_engine;
}

void QBluetoothAbstractServerPrivate::incomingConnection()
{
    while (true) {
        if (m_pendingConnections.count() > m_maxConnections) {
            qWarning("QBluetoothAbstractServer::incomingConnection() too many connections");
            if (m_readNotifier->isEnabled())
                m_readNotifier->setEnabled(false);
            return;
        }

        int fd = m_engine->accept(m_fd);

        if (fd  < 0) {
            return;
        }

        QBluetoothAbstractSocket *socket = m_parent->createSocket();
        socket->setSocketDescriptor(fd, QBluetoothAbstractSocket::ConnectedState,
                                    QIODevice::ReadWrite);
        m_pendingConnections.append(socket);

        QPointer<QBluetoothAbstractServer> that = m_parent;
        emit m_parent->newConnection();
        if (!that || !m_parent->isListening())
            return;
    }
}

/*!
    \class QBluetoothAbstractServer
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothAbstractServer class represents an abstract Bluetooth server socket.

    This class is a common base class for all Bluetooth server
    socket implementations and makes it possible to accept incoming
    Bluetooth connections.  The users should not use this class
    directly, instead they should use concrete implementations.

    The subclasses will implement a \c{listen()} implementation in order
    to create a listening socket for each particular Bluetooth socket
    type.  In particular, QBluetoothL2CapServer, QBluetoothRfcommServer,
    and QBluetoothScoServer will create L2CAP, RFCOMM and SCO sockets
    respectively.

    Services that provide RFCOMM based profiles should use the
    QBluetoothRfcommServer class.  L2CAP based services should use
    the QBluetoothL2CapServer class.  Headset and Handsfree profile
    implementations will require the use of QBluetoothScoServer
    class.

    The typical use of this class is to call the \c{listen()} implementation
    in the subclass, and then hook onto the newConnection() signal to
    get an indication of an incoming connection. The newConnection()
    signal is emitted each time a client has connected to the server.
    Call nextPendingConnection() to accept the pending
    connection as a connected QBluetoothAbstractSocket.  The actual socket
    type depends on the type of the server.  E.g. the
    QBluetoothRfcommServer socket will return QBluetoothRfcommSocket type
    sockets.

    If an error occurs, serverError() returns the type of
    error that has occurred.

    Calling close() makes the QBluetoothAbstractServer stop
    listening for incoming connections and deletes all pending
    connections.

    \sa QBluetoothAbstractSocket

    \ingroup qtopiabluetooth
 */

/*!
    \internal
    
    Constructs a new QBluetoothAbstractServer with parent \a parent.  The \a data parameter holds the internal private
    data object.

    The server is in the UnconnectedState.
 */
QBluetoothAbstractServer::QBluetoothAbstractServer(QBluetoothAbstractServerPrivate *data,
                                                   QObject *parent)
    : QObject(parent)
{
    m_data = data;
}

/*!
    Destroys the server.
 */
QBluetoothAbstractServer::~QBluetoothAbstractServer()
{
    close();

    if (m_data)
        delete m_data;
}

/*! \fn void QBluetoothAbstractServer::newConnection()

    This signal is emitted every time a new connection is available.
    Call nextPendingConnection() in order to accept the connection.

    \sa hasPendingConnections(), nextPendingConnection()
 */

/*!
    \internal

    Subclasses of this class should call this function and hand off the
    server socket that will be used for accepting connections.  The
    \a socket parameter contains the socket file descriptor, the
    \a addr parameter contains the sockaddr structure of the address
    to bind and listen on and the \a len parameter contains the length of
    the \a addr structure.

    Returns true on successful completion of the request;
    otherwise returns false.

    \sa isListening()
 */
void QBluetoothAbstractServer::setListening()
{
    if (!m_data->m_readNotifier)
        m_data->m_readNotifier = new QSocketNotifier(m_data->m_fd, QSocketNotifier::Read);

    m_data->m_readNotifier->setEnabled(true);

    connect(m_data->m_readNotifier, SIGNAL(activated(int)),
            m_data, SLOT(incomingConnection()));

    m_data->m_isListening = true;
}

/*!
    Returns true if the server is currently listening for remote connections,
    and false otherwise.

    \sa close(), socketDescriptor()
 */
bool QBluetoothAbstractServer::isListening() const
{
    return m_data->m_isListening;
}

/*!
    Closes the server. The server will no longer listen for incoming
    connections and all pending connections will be closed.

    \sa isListening(), socketDescriptor()
 */
void QBluetoothAbstractServer::close()
{
    qDeleteAll(m_data->m_pendingConnections);
    m_data->m_pendingConnections.clear();

    if (m_data->m_readNotifier) {
        delete m_data->m_readNotifier;
        m_data->m_readNotifier = 0;
    }

    m_data->m_engine->close(m_data->m_fd);
    m_data->m_fd = -1;

    m_data->m_isListening = false;
}

/*!
    Returns the last error that has occurred.

    \sa errorString()
 */
QBluetoothAbstractSocket::SocketError QBluetoothAbstractServer::serverError() const
{
    return m_data->m_engine->error();
}

/*!
    Returns a human-readable description of the last device error that occurred.

    \sa serverError()
*/
QString QBluetoothAbstractServer::errorString() const
{
    return QBluetoothSocketEngine::getErrorString(m_data->m_engine->error());
}

/*!
    Returns the maximum number of pending accepted connections. The
    default is 1.

    \sa setMaxPendingConnections(), hasPendingConnections()
 */
int QBluetoothAbstractServer::maxPendingConnections() const
{
    return m_data->m_maxConnections;
}

/*!
    Sets the maximum number of pending accepted connections to \a
    numConnections. QBluetoothAbstractServer will accept no more than \a
    numConnections incoming connections before
    nextPendingConnection() is called. By default, the limit is 1
    pending connection.

    \sa maxPendingConnections(), hasPendingConnections()
 */
void QBluetoothAbstractServer::setMaxPendingConnections(int numConnections)
{
    m_data->m_maxConnections = numConnections;
}

/*!
    Returns true if the server has a pending connection(s); otherwise
    returns false.

    \sa nextPendingConnection(), setMaxPendingConnections()
 */
bool QBluetoothAbstractServer::hasPendingConnections() const
{
    return !m_data->m_pendingConnections.isEmpty();
}

/*!
    Returns the next pending connection as a connected
    QBluetoothAbstractSocket object.  The function returns a
    pointer to a QBluetoothAbstractSocket in
    QBluetoothAbstractSocket::ConnectedState that you can
    use for communicating with the client.

    The type of the socket returned will be based on the server
    socket type used.  You will need to use qobject_cast to convert
    the result into the concrete socket type required.

    \code
        QBluetoothAbstractSocket *sock = server->nextPendingConnection();
        if (sock) {
            QBluetoothRfcommSocket *rfcomm =
                qobject_cast<QBluetoothRfcommSocket *>(sock);
            if (rfcomm) {
                // use RFCOMM socket
            }
        }
    \endcode

    The socket is created as a child of the server, which means that
    it is automatically deleted when the QBluetoothAbstractServer object is
    destroyed. It is still a good idea to delete the object
    explicitly when you are done with it, to avoid wasting memory.

    A NULL pointer is returned if this function is called when
    there are no pending connections.

    \sa hasPendingConnections()
 */
QBluetoothAbstractSocket *QBluetoothAbstractServer::nextPendingConnection()
{
    if (m_data->m_pendingConnections.isEmpty())
        return 0;

    if (!m_data->m_readNotifier->isEnabled())
        m_data->m_readNotifier->setEnabled(true);

    return m_data->m_pendingConnections.takeFirst();
}

/*!
    Waits for at most \a msecs milliseconds or until an incoming
    connection is available. Returns true if a connection is
    available; otherwise returns false. If the operation timed out
    and \a timedOut is not 0, *\a timedOut will be set to true.

    This is a blocking function call. Its use is not advised in a
    single-threaded GUI application, since the whole application will
    stop responding until the function returns.
    waitForNewConnection() is mostly useful when there is no event
    loop available.

    The non-blocking alternative is to connect to the newConnection()
    signal.

    \sa hasPendingConnections(), nextPendingConnection()
 */
bool QBluetoothAbstractServer::waitForNewConnection(int msecs, bool *timedOut)
{
    if (!m_data->m_isListening)
        return false;

    if (!m_data->m_engine->waitFor(QBluetoothSocketEngine::SelectRead,
                                    m_data->m_fd, msecs, timedOut)) {
        return false;
    }

    m_data->incomingConnection();

    return true;

}

/*!
    Returns the socket descriptor the server is currently listening on.  If the
    server is not listening, then -1 is returned.

    \sa isListening()
*/
int QBluetoothAbstractServer::socketDescriptor() const
{
    return m_data->m_fd;
}

/*!
    \internal

    \fn QBluetoothAbstractSocket * QBluetoothAbstractServer::createSocket();

    Clients of this class should override this function to provide sockets specific
    to the protocol that the client server implements.
*/

#include "qbluetoothabstractserver.moc"
