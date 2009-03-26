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

#include <qbluetoothscoserver.h>
#include <qbluetoothscosocket.h>
#include "qbluetoothnamespace_p.h"
#include <qbluetoothaddress.h>

#include "qbluetoothabstractserver_p.h"
#include "qbluetoothsocketengine_p.h"

class QBluetoothScoServerPrivate : public QBluetoothAbstractServerPrivate
{
public:
    QBluetoothScoServerPrivate(QBluetoothScoServer *parent);
    QBluetoothAddress m_address;
};

QBluetoothScoServerPrivate::QBluetoothScoServerPrivate(QBluetoothScoServer *parent)
    : QBluetoothAbstractServerPrivate(parent)
{
    
}

/*!
    \class QBluetoothScoServer
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothScoServer class represents a SCO server socket.

    This class makes it possible to accept incoming SCO connections.
    Synchronous Connection Oriented connections are used to transfer
    audio data over a Bluetooth link.  The audio data is usually
    compressed by a lossy codec and should not be used to transfer
    bulk data.

    Call listen() to make the server listen for new connections.  The
    newConnection() signal will be emmited each time a client connects
    to the server.

    Call nextPendingConnection() to accept the pending client connection.

    When listening for connections, server address is available by
    calling serverAddress().

    Calling close() will make the QBluetoothScoServer stop listening
    for connections and delete all pending connections.

    \sa QBluetoothScoSocket

    \ingroup qtopiabluetooth
 */

/*!
    Constructs a new QBluetoothScoServer with parent \a parent.
    The server is in the UnconnectedState.
 */
QBluetoothScoServer::QBluetoothScoServer(QObject *parent)
    : QBluetoothAbstractServer(new QBluetoothScoServerPrivate(this), parent)
{
}

/*!
    Destroys the server.
*/
QBluetoothScoServer::~QBluetoothScoServer()
{
}

/*!
    Tells the server to listen for incoming connections on
    address \a local.

    Returns true on success; otherwise returns false.

    \sa isListening()
 */
bool QBluetoothScoServer::listen(const QBluetoothAddress &local)
{
    SERVER_DATA(QBluetoothScoServer);

    if (isListening()) {
        return false;
    }

    m_data->m_fd = m_data->m_engine->scoSocket();

    if (m_data->m_fd < 0) {
        return false;
    }

    m_data->m_engine->setSocketOption(m_data->m_fd,
                                      QBluetoothSocketEngine::NonBlockingOption);

    if (!m_data->m_engine->scoBind(m_data->m_fd, local)) {
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

    m_data->m_address = local;

    m_data->m_engine->getsocknameSco(m_data->m_fd, &m_data->m_address);

    return true;
}

/*!
    \reimp
 */
void QBluetoothScoServer::close()
{
    SERVER_DATA(QBluetoothScoServer);

    QBluetoothAbstractServer::close();
    m_data->m_address = QBluetoothAddress::invalid;
}

/*!
    Returns the address the server is currently listening on.  If the server
    is not listening, returns QBluetoothAddress::invalid.
 */
QBluetoothAddress QBluetoothScoServer::serverAddress() const
{
    SERVER_DATA(QBluetoothScoServer);
    return m_data->m_address;
}

/*!
    \reimp
*/
QBluetoothAbstractSocket * QBluetoothScoServer::createSocket()
{
    return new QBluetoothScoSocket(this);
}
