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
#include <QDebug>
#include <QtAlgorithms>
#include <QPointer>

#include <qirsocket.h>
#include <qirserver.h>
#include "qirnamespace_p.h"
#include "qirsocketengine_p.h"

class QIrServerPrivate : public QObject
{
    Q_OBJECT

public:
    QIrServerPrivate(QIrServer *parent);
    ~QIrServerPrivate();

    int m_fd;

    bool m_isListening;
    int m_maxConnections;
    QList<QIrSocket *> m_pendingConnections;
    QByteArray m_service;
    QIr::DeviceClasses m_classes;

    QIrServer *m_parent;
    QSocketNotifier *m_readNotifier;
    QIrSocketEngine *m_engine;

public slots:
    void incomingConnection();
};

QIrServerPrivate::QIrServerPrivate(QIrServer *parent)
{
    m_parent = parent;
    m_isListening = false;
    m_maxConnections = 1;
    m_readNotifier = 0;
    m_engine = new QIrSocketEngine;
}

QIrServerPrivate::~QIrServerPrivate()
{
    if (m_readNotifier)
        delete m_readNotifier;

    delete m_engine;
}

void QIrServerPrivate::incomingConnection()
{
    while (true) {
        if (m_pendingConnections.count() > m_maxConnections) {
            qWarning("QIrServer::incomingConnection() too many connections");
            if (m_readNotifier->isEnabled())
                m_readNotifier->setEnabled(false);
            return;
        }

        int fd = m_engine->accept(m_fd);

        if (fd  < 0)
            return;

        QIrSocket *socket = new QIrSocket(m_parent);
        socket->setSocketDescriptor(fd, QIrSocket::ConnectedState,
                                    QIODevice::ReadWrite);
        m_pendingConnections.append(socket);

        QPointer<QIrServer> that = m_parent;
        emit m_parent->newConnection();
        if (!that || !m_parent->isListening())
            return;
    }
}

/*!
    \class QIrServer

    \brief The QIrServer class represents an Infrared TinyTP server socket.

    This class makes it possible to accept incoming Infrared
    connections.  You can specify the service identifier that
    this socket represents to be automatically advertised in the
    IAS database.

    Call listen() to make the server listen for new connections.  The
    newConnection() signal will be emmited each time a client connects
    to the server.

    Call nextPendingConnection() to accept the pending client connection.

    When listening for connections, server service descriptor is available
    by calling serverService().

    Calling close() will make the QIrServer stop listening
    for connections and delete all pending connections.

    \sa QIrSocket

    \ingroup qtopiair
 */

/*!
    Constructs a new QIrServer with parent \a parent.
    The server is in the UnconnectedState.
 */
QIrServer::QIrServer(QObject *parent)
    : QObject(parent)
{
    m_data = new QIrServerPrivate(this);
}

/*!
    Destroys the server.
 */
QIrServer::~QIrServer()
{
    close();

    if (m_data)
        delete m_data;
}

/*! \fn void QIrServer::newConnection()

    This signal is emitted every time a new connection is available.
    Call nextPendingConnection() in order to accept the connection.

    \sa hasPendingConnections(), nextPendingConnection()
 */

/*!
    Returns true if the server is currently listening for remote connections,
    and false otherwise.

    \sa close(), socketDescriptor()
 */
bool QIrServer::isListening() const
{
    return m_data->m_isListening;
}

/*!
    Closes the server. The server will no longer listen for incoming
    connections and all pending connections will be closed.

    \sa isListening(), socketDescriptor()
 */
void QIrServer::close()
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
    m_data->m_classes = 0;
    m_data->m_service = QByteArray();
}

/*!
    Returns the last error that has occurred.

    \sa errorString()
 */
QIrSocket::SocketError QIrServer::serverError() const
{
    return m_data->m_engine->error();
}

/*!
    Returns a human-readable description of the last device error that occurred.

    \sa serverError()
*/
QString QIrServer::errorString() const
{
    return QIrSocketEngine::getErrorString(m_data->m_engine->error());
}

/*!
    Returns the maximum number of pending accepted connections. The
    default is 1.

    \sa setMaxPendingConnections(), hasPendingConnections()
 */
int QIrServer::maxPendingConnections() const
{
    return m_data->m_maxConnections;
}

/*!
    Sets the maximum number of pending accepted connections to \a
    numConnections. QIrServer will accept no more than \a
    numConnections incoming connections before
    nextPendingConnection() is called. By default, the limit is 1
    pending connection.

    \sa maxPendingConnections(), hasPendingConnections()
 */
void QIrServer::setMaxPendingConnections(int numConnections)
{
    m_data->m_maxConnections = numConnections;
}

/*!
    Returns true if the server has a pending connection(s); otherwise
    returns false.

    \sa nextPendingConnection(), setMaxPendingConnections()
 */
bool QIrServer::hasPendingConnections() const
{
    return !m_data->m_pendingConnections.isEmpty();
}

/*!
    Returns the next pending connection as a connected
    QIrSocket object.  The function returns a
    pointer to a QIrSocket in QIrSocket::ConnectedState that you can
    use for communicating with the client.

    The socket is created as a child of the server, which means that
    it is automatically deleted when the QIrServer object is
    destroyed. It is still a good idea to delete the object
    explicitly when you are done with it, to avoid wasting memory.

    A NULL pointer is returned if this function is called when
    there are no pending connections.

    \sa hasPendingConnections()
 */
QIrSocket *QIrServer::nextPendingConnection()
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
bool QIrServer::waitForNewConnection(int msecs, bool *timedOut)
{
    if (!m_data->m_isListening)
        return false;

    if (!m_data->m_engine->waitFor(QIrSocketEngine::SelectRead, m_data->m_fd, msecs, timedOut)) {
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
int QIrServer::socketDescriptor() const
{
    return m_data->m_fd;
}

/*!
    Tells the server to listen for incoming connections.  The
    \a service will be automatically added to the IAS database
    and an Ir LSAP selector will be automatically chosen.

    The \a classes contains the service hints for this
    service.  If the system supports setting of the service
    hints they will be set into the IAS database automatically,
    otherwise passing this parameter will have no effect.

    Returns true on success; otherwise returns false.

    \sa isListening()

*/
bool QIrServer::listen(const QByteArray &service, QIr::DeviceClasses classes)
{
    if (isListening()) {
        return false;
    }

    m_data->m_fd = m_data->m_engine->socket();

    if (m_data->m_fd < 0) {
        return false;
    }

    m_data->m_engine->setSocketOption(m_data->m_fd, QIrSocketEngine::NonBlockingOption);

    if (!m_data->m_engine->bind(m_data->m_fd, service)) {
        m_data->m_engine->close(m_data->m_fd);
        m_data->m_fd = -1;
        return false;
    }

    if (!m_data->m_engine->listen(m_data->m_fd, 1)) {
        m_data->m_engine->close(m_data->m_fd);
        m_data->m_fd = -1;
        return false;
    }

    if (!m_data->m_readNotifier)
        m_data->m_readNotifier = new QSocketNotifier(m_data->m_fd, QSocketNotifier::Read);

    m_data->m_readNotifier->setEnabled(true);

    connect(m_data->m_readNotifier, SIGNAL(activated(int)),
            m_data, SLOT(incomingConnection()));

    m_data->m_isListening = true;

    m_data->m_service = service;

    if (m_data->m_engine->setServiceHints(m_data->m_fd, classes)) {
        m_data->m_classes = classes;
    }

    return true;
}

/*!
    Returns the service descriptor this socket has been created with.
    The service descriptor is added to the IAS database automatically.

    \sa listen(), serverDeviceClasses()
*/
QByteArray QIrServer::serverService() const
{
    return m_data->m_service;
}

/*!
    Returns the service hint set that this socket has been created
    with.  The service hints are added to the IAS database automatically.

    Note that this functionality might not be supported on some systems
    in which case this function will always return an empty set.

    \sa listen(), serverService()
*/
QIr::DeviceClasses QIrServer::serverDeviceClasses() const
{
    return m_data->m_classes;
}

#include "qirserver.moc"
