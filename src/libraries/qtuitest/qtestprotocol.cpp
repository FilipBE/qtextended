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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

/*
    This cpp file contains a number of socket related classes that are used heavily in QtUiTest.
    The classes are documented later in the file.
*/

#include "qtestprotocol_p.h"
#include "qtuitestnamespace.h"

#include <QApplication>
#include <QString>
#include <QTimer>
#include <QUuid>
#include <QFileInfo>
#include <QDir>
#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>
#include <QHostInfo>
#include <QPointer>

#if defined QTOPIA_TARGET && defined QTUITEST_TARGET
# include "qtuitestnamespace.h"
# include <qtopialog.h>
#else
# include <QDebug>
# define qLog(A) if (1); else qDebug()
#endif

#if defined(Q_OS_WIN32) && !defined(Q_OS_TEMP)
# include <io.h>
#endif

static const int CONNECT_TIMEOUT = 20000;

static const quint32 TEST_MSG_SIGNATURE = 0xEDBAEDBA;
static const quint32 TEST_MSG_END       = 0xEDBAABDE;
static const quint8  TEST_MSG_VERSION   = 3;
static uint g_unique_id = 0;

bool waitForSignal(QObject* object, const char* signal, int timeout)
{
#ifdef QTUITEST_TARGET
    return QtUiTest::waitForSignal(object, signal, timeout);
#else
    QEventLoop loop;
    QTimer dummy;
    dummy.setInterval(1000);
    if (!QObject::connect(object, signal, &dummy, SLOT(start())))
        return false;
    if (!QObject::connect(object, signal, &loop, SLOT(quit())))
        return false;
    QTimer::singleShot(timeout, &loop, SLOT(quit()));
    loop.exec();
    return dummy.isActive();
#endif
}

void wait(int timeout)
{
#ifdef QTUITEST_TARGET
    QtUiTest::wait(timeout);
#else
    QEventLoop loop;
    QTimer::singleShot(timeout, &loop, SLOT(quit()));
    loop.exec();
#endif
}

// ********************************************************************************
// ****************************** QTestMessage ************************************
// ********************************************************************************

/*
  \class QTestMessage QTestMessage.h
    \inpublicgroup QtUiTestModule

  \brief The QTestMessage class can be used for exchanging messages between separate
  processes. The class is never used directly but instead is used by QTestProtocol.

  The class basically wraps a number of strings (i.e. 'event' and 'message') and binary data
  (i.e. a bytearray or a file) into a datastream that is sent to a peer using a socket
  connection. On the receiving side a QTestMessage instance is decoding the datastream
  and performs a number of sanity checks to make it a bit more robost.

  A TCP connection 'should' be reliable but in exceptional cases bytes may be lost. This
  would result in the connection becoming useless because all future data reception would
  be out of sync. To solve this a 'resync' function is implemented that can re-sync the datastream
  by throwing away bytes until the data seems in sync again. The obvious downside is that
  at least one message will be lost.

  Message format is as follows:

  Field:              Length:        Remarks:

  Start signature     4              Fixed value - 0xEDBAEDBA
  Protocol version    1              Fixed value - 3
  Message number      2
  Event length        4              Length of following string
  Event string        Event length   QString value
  Message length      4              Length of following string
  Messaga string      Message length QString value
  Data length         4              Length of following binary data
  File data           Data length    Binary data
  End signature       4              Fixed value - 0xEDBAABDE
*/

/*
   \internal

    Constructs a default (empty) message.
*/

QTestMessage::QTestMessage(QString const &event, QVariantMap const &map)
    : m_phase(0)
    , m_msg_id(0)
    , m_event(event)
    , m_map(map)
{
}

QTestMessage::QTestMessage(QString const &event, const QTestMessage &other )
    : m_phase(0)
    , m_msg_id(other.m_msg_id)
    , m_map(other.m_map)
{
    m_event = event;
}

QTestMessage::QTestMessage(QString const &event, QString const &queryApp, QString const &queryPath )
    : m_phase(0)
    , m_msg_id(0)
    , m_event(event)
    , m_map(QVariantMap())
{
    m_map["queryapp"] = queryApp;
    m_map["querypath"] = queryPath;
}

/*
   \internal

   Copy constructor.
*/

QTestMessage::QTestMessage(const QTestMessage &other)
    : m_phase(0)
    , m_msg_id(other.m_msg_id)
    , m_event(other.m_event)
    , m_map(other.m_map)
{
}

/*
   \internal

    Destroys the message.
*/

QTestMessage::~QTestMessage()
{
}

/*
   \internal

   Assignment operator.
*/

QTestMessage& QTestMessage::operator=(const QTestMessage &other)
{
    m_msg_id = other.m_msg_id;
    m_event = other.m_event;
    m_map = other.m_map;

    return *this;
}

QVariant &QTestMessage::operator[](QString const &key)
{
    return m_map[key.toLower()];
}

QVariant const QTestMessage::operator[](QString const &key) const
{
    return m_map[key.toLower()];
}

bool QTestMessage::contains(QString const &key) const
{
    return m_map.contains(key.toLower());
}

QList<QString> QTestMessage::keys() const
{
    return m_map.keys();
}

QString QTestMessage::toString() const
{
    QString ret;
    foreach(QString k, m_map.keys()) {
        if (!m_map[k].isValid()) continue;
        ret += k + ": ";
        if (m_map[k].canConvert<QStringList>()) ret += "'" + m_map[k].toStringList().join("','") + "'";
        else if (m_map[k].canConvert<QString>()) ret +=  "'" + m_map[k].toString() + "'";
        else ret += "(data)";
        ret += "\n";
    }
    if (ret.endsWith("\n")) ret.chop(1);
    return ret;
}

/*
  \internal
    Returns the event that was received.
*/

QString QTestMessage::event() const
{
    return m_event;
}

/*
  \internal
    Returns the message number.
*/

quint16 QTestMessage::msgId() const
{
    return m_msg_id;
}

bool QTestMessage::statusOK() const
{
    return m_map.contains("status") && m_map["status"].toString() == "OK";
}

bool QTestMessage::isNull() const
{
    return m_map.isEmpty();
}


// ********************************************************************************
// ************************* QTestServerSocket ************************************
// ********************************************************************************

/* !
    \class QTestServerSocket qtestserversocket.h
    \inpublicgroup QtUiTestModule

    \brief The QTestServerSocket class provides a TCP-based server.

    This class is a convenience class for accepting incoming TCP
    connections. You can specify the port or have QTestServerSocket pick
    one, and listen on just one address or on all the machine's
    addresses.

    Using the API is very simple: subclass QTestServerSocket, call the
    constructor of your choice, and implement onNewConnection() to
    handle new incoming connections. There is nothing more to do.

    (Note that due to lack of support in the underlying APIs,
    QTestServerSocket cannot accept or reject connections conditionally.)

    \sa QTcpSocket, QTcpServer, QHostAddress, QSocketNotifier
*/


/* !
    Creates a server socket object, that will serve the given \a port
    on all the addresses of this host. If \a port is 0, QTestServerSocket
    will pick a suitable port in a system-dependent manner. Use \a
    backlog to specify how many pending connections the server can
    have.

    \warning On Tru64 Unix systems a value of 0 for \a backlog means
    that you don't accept any connections at all; you should specify a
    value larger than 0.
*/

QTestServerSocket::QTestServerSocket( quint16 port, int backlog )
    : QTcpServer()
{
    setMaxPendingConnections( backlog );
    listen( QHostAddress::Any, port );

    if (this->serverPort() == 0) {
        qWarning( QString("ERROR: port '%1' is already in use, application is aborted.").arg(port).toAscii() );
        QApplication::exit(777);
    }
}

/* !
    Destroys the socket.

    This causes any backlogged connections (connections that have
    reached the host, but not yet been completely set up
    to be severed.

    Existing connections continue to exist; this only affects the
    acceptance of new connections.
*/
QTestServerSocket::~QTestServerSocket()
{
}

/* !
    Returns true if the construction succeeded; otherwise returns false.
*/
bool QTestServerSocket::ok() const
{
    return serverPort() > 0;
}

/* !
    Returns the port number on which this server socket listens. This
    is always non-zero; if you specify 0 in the constructor,
    QTestServerSocket will pick a non-zero port itself. ok() must be true
    before calling this function.

    \sa address()
*/
quint16 QTestServerSocket::port() const
{
    return serverPort();
}

/* !
    Returns the address on which this object listens, or 0.0.0.0 if
    this object listens on more than one address. ok() must be true
    before calling this function.

    \sa port()
*/
QString QTestServerSocket::address() const
{
    return serverAddress().toString();
}

void QTestServerSocket::incomingConnection( int socket )
{
    onNewConnection( socket );
}

// ********************************************************************************
// *************************** QTestProtocol ************************************
// ********************************************************************************

/*
  \class QTestProtocol qtestprotocol.h
    \inpublicgroup QtUiTestModule

  \brief The QTestProtocol class can be used for exchanging messages between separate
  processes.

  It is intended to be fast but at the same time ultra reliable and robust communication protocol.

  The main function that is used on the sending side is:
  \code
  myConnection.postMessage( "My-Event", "My-Message", ptr_to_my_data );
  \endcode

  On the receiving side the main function is a re-implemented 'processMessage':
  \code
  void MyTestConnection::processMessage( QTestMessage *msg )
  {
    if (msg->event() == "My-Event") {
        print( msg->message() );
    }
  }
  \endcode
*/

#include <stdio.h>

QTestProtocol::QTestProtocol()
    : QTcpSocket()
    , tx_msg_id(1)
    , host()
    , port(0)
    , onData_busy(false)
    , enable_reconnect(false)
    , reconnect_interval(10000)
    , connect_timer()
    , last_data_received(false)
    , connection_valid(false)
    , ping_enabled(false)
    , ping_interval(10000)
    , ping_timer()
    , ping_count(0)
    , ping_timeout_warning_issued(false)
    , last_send_cmd("")
    , unique_id()
    , debug_on(false)
{
    static int id1 = qRegisterMetaType<QTestMessage>(); Q_UNUSED(id1);
    static int id2 = qRegisterMetaType<QTestMessage*>(); Q_UNUSED(id2);
    static int id3 = qRegisterMetaType<const QTestMessage*>(); Q_UNUSED(id3);

    unique_id = QString("%1").arg(++g_unique_id);
    if (debug_on) qLog(QtUitest) <<  QString("%1 QTestProtocol::QTestProtocol()").arg(uniqueId()).toLatin1();
    cur_message = 0;
    rx_busy = false;

    QObject::connect( &connect_timer, SIGNAL(timeout()), this, SLOT(connectTimeout()), Qt::DirectConnection );

    QObject::connect( this,SIGNAL(connected()),this,SLOT(onSocketConnected()), Qt::DirectConnection );
    QObject::connect( this,SIGNAL(disconnected()),this,SLOT(onSocketClosed()), Qt::DirectConnection );
    QObject::connect( this,SIGNAL(readyRead()),this,SLOT(onData()), Qt::DirectConnection );

    // initialize. Any time is better than no time.
    rx_timer.start();
}

/*!
    Destructs the instance of QTestProtocol.
*/

QTestProtocol::~QTestProtocol()
{
    if (debug_on) qLog(QtUitest) <<  QString("%1 QTestProtocol::~QTestProtocol()").arg(uniqueId()).toLatin1();
    enableReconnect( false, 0 );

    // we can't send any more messages so disable pinging
    enablePing( false );

    // anything that is still in the tx buffer gets lost
    abort();
    close();

    while (send_msg_replies.count() > 0)
        delete send_msg_replies.takeFirst();
    while (unhandled_msg.count() > 0)
        delete unhandled_msg.takeFirst();
}

void QTestProtocol::setSocket( int socket )
{
    if (debug_on) qLog(QtUitest) << ( QString("%1 QTestProtocol::setSocket(socket=%2)").
                arg(uniqueId()).
                arg(socket).toLatin1());
    setSocketDescriptor( socket );

    rx_timer.start();
    testConnection();
}

void QTestProtocol::enableReconnect( bool enable, uint reconnectInterval )
{
    if (debug_on) qLog(QtUitest) << ( QString("%1 QTestProtocol::enableReconnect( enable=%2, interval=%3)").
                                arg(uniqueId()).
                                arg(enable).
                                arg(reconnectInterval).toLatin1());
    enable_reconnect = enable;
    reconnect_interval = reconnectInterval;
}

/*!
    Opens a socket connection to the specified \a hostname and \a port.

    After a connection is successfully opened the instance will listen for and process
    incoming commands received from the remote host.
*/
void QTestProtocol::connect( const QString& hostname, int port )
{
    if (debug_on) qLog(QtUitest) << ( QString("%1 QTestProtocol::connect(%2:%3)").
                arg(uniqueId()).
                arg(hostname).
                arg(port).toLatin1());

    if (state() == ConnectedState) {
        if (hostname == this->host && port == this->port)
            return;
        disconnect();
    }

    rx_timer.start();

    this->host = hostname;
    this->port = port;

    reconnect();
}

void QTestProtocol::disconnect( bool disableReconnect )
{
    if (state() == ConnectedState) {
        if (debug_on) qLog(QtUitest) << ( QString("%1 QTestProtocol::disconnect(disableReconnect=%2)").
                    arg(uniqueId()).
                    arg(disableReconnect).toLatin1());
        // be polite and tell the other side we are closing
        postMessage( QTestMessage("QTEST_CLOSING_CONNECTION") );

        // we are closing ourselves, so we don't want to reconnect
        if (disableReconnect) enable_reconnect = false;

        onSocketClosed();
    }
}

bool QTestProtocol::isConnected()
{
    return (state() == ConnectedState && connection_valid);
}

bool QTestProtocol::waitForConnected( int timeout )
{
    QElapsedTimer t;
    if (QTcpSocket::waitForConnected(timeout)) {
        if (debug_on) qLog(QtUitest) <<  QString("%1 QTestProtocol::waitForConnected() ... testing connection").arg(uniqueId()).toLatin1();
        while (t.elapsed() < timeout && !isConnected()) {
            wait(500);
            postMessage( QTestMessage("QTEST_NEW_CONNECTION") );
        }
    }
    bool ok = isConnected();
    if (debug_on) qLog(QtUitest) <<  QString("%1 QTestProtocol::waitForConnected() ... %2").arg(uniqueId()).arg(ok ? "OK" : "FAILED" ).toLatin1();
    return ok;
}

/*!
   \internal
    Posts (e.g. non blocking) an \a event, \a message and contents of \a fileName to the remote host.
*/

uint QTestProtocol::postMessage(QTestMessage const &message )
{
    if (debug_on && message.event() != "PING" && message.event() != "PONG") 
        qLog(QtUitest) << ( QString("%1 QTestProtocol::postMessage(%2)").
                    arg(uniqueId()).
                    arg(message.event()).toLatin1());
    if (state() != ConnectedState)
        return 0;
    QTestMessage msg(message);
    msg.m_msg_id = tx_msg_id++;
    send( msg );
    return msg.m_msg_id;
}

void QTestProtocol::onReplyReceived( QTestMessage* /*reply*/ )
{
}

/*!
   \internal
    Sends an \a event, \a message and \a data to the remote host and waits for up to
    \a timeout milliseconds for a reply.  If a reply is received, the reply's message
    string is placed in \a reply.
*/
bool QTestProtocol::sendMessage( QTestMessage const &message, QTestMessage &reply, int timeout )
{
    QTestMessage msg(message);
    QPointer<QTestProtocol> safeThis(this);
    bool safeDebugOn(debug_on);
    QString safeUniqueId(uniqueId());

    last_send_cmd = message.event();
    if (state() == ConnectedState) {
        msg.m_msg_id = tx_msg_id++;

        if (debug_on) qLog(QtUitest) << ( QString("%1 QTestProtocol::sendMessage(%2) msgid=%3)").
                    arg(uniqueId()).
                    arg(msg.event()).
                    arg(msg.msgId()).
                    toLatin1());

        send( msg );

        QElapsedTimer t;
        QElapsedTimer t2;
        bool first_time = true;
        while ( (state() == ConnectedState) && (timeout < 0 || t.elapsed() < timeout) ) {

            if (debug_on) {
                if (first_time || t2.elapsed() > 1000) {
                    qLog(QtUitest) << ( QString("%1 QTestProtocol::sendMessage(%2) ... waiting for reply").
                                            arg(uniqueId()).
                                            arg(message.event()).toLatin1());
                    t2.start();
                    first_time = false;
                }
            }


            waitForSignal(this, SIGNAL(replyReceived()), 500);

            if (!safeThis) {
                if (message["expectClose"].isValid()) {
                    return true;
                }
                if (safeDebugOn) qLog(QtUitest) << ( QString("%1 QTestProtocol::sendMessage(%2) ... object deleted unexpectedly").
                                            arg(safeUniqueId).
                                            arg(message.event()).toLatin1());
                reply["status"] = "ERROR: Connection was terminated unexpectedly. This may be caused by an application crash.";
                reply["_q_inResponseTo"] = QString("%1\n%2").arg(message.event()).arg(message.toString());
                return false;
            } else {
                if (send_msg_replies.count() > 0) {
                    if (debug_on) qLog(QtUitest) << ( QString("%1 QTestProtocol::sendMessage(%2) ... check replies").
                                            arg(uniqueId()).
                                            arg(message.event()).toLatin1());
                    for (int i=0; i<send_msg_replies.size(); i++) {
                        QTestMessage * possible_reply = send_msg_replies.at(i);
                        if (possible_reply && possible_reply->m_msg_id == msg.m_msg_id) {

                            reply = *possible_reply;
                            delete send_msg_replies.takeAt( i );

                            if (debug_on) qLog(QtUitest) << ( QString("%1 QTestProtocol::sendMessage(%2) ... reply received").
                                            arg(uniqueId()).
                                            arg(message.event()).toLatin1());

                            onReplyReceived(&reply);
                            return true;
                        }
                    }
                }
            }
        }
        if (state() != ConnectedState) {
            reply["status"] = "ERROR: Connection lost. This is likely caused by a crash in Qt Extended.";
            reply["_q_inResponseTo"] = QString("%1\n%2").arg(message.event()).arg(message.toString());
        }
        else {
            reply["status"] = "ERROR_REPLY_TIMEOUT";
            reply["_q_inResponseTo"] = QString("%1\n%2").arg(message.event()).arg(message.toString());
        }
        reply["location"] = QString("%1:%2").arg(__FILE__).arg(__LINE__);
    } else {
        reply["status"] = "ERROR_NO_CONNECTION";
        reply["_q_inResponseTo"] = QString("%1\n%2").arg(message.event()).arg(message.toString());
        reply["location"] = QString("%1:%2").arg(__FILE__).arg(__LINE__);
    }

    if (debug_on) qLog(QtUitest) << ( QString("%1 QTestProtocol::sendMessage(%2) ... done. Status: %3").
                             arg(uniqueId()).
                             arg(message.event()).arg(reply["status"].toString()).toLatin1());

    return false;
}

/*!
    Send the string \a result as a reply to \a originalMsg.
*/

void QTestProtocol::replyMessage( QTestMessage *originalMsg, QTestMessage const &message )
{
    if (debug_on) qLog(QtUitest) << ( QString("%1 QTestProtocol::replyMessage(%2)").
                    arg(uniqueId()).
                    arg(originalMsg->event()).toLatin1());
    QTestMessage msg(message);
    msg.m_msg_id = originalMsg->msgId();
    msg.m_event = "@REPLY@";
    send( msg );
}

bool QTestProtocol::lastDataReceived()
{
    return last_data_received;
}

QString QTestProtocol::errorStr()
{
    QString S = "Connection error: ";
    switch (error()) {
        case ConnectionRefusedError: S += "A connection attempt was rejected by the peer"; break;
        case HostNotFoundError: S +=  "Host not found"; break;
        case RemoteHostClosedError: S += "RemoteHostClosedError"; break;
        case SocketAccessError: S += "SocketAccessError"; break;
        case SocketResourceError: S += "SocketResourceError"; break;
        case SocketTimeoutError: S += "SocketTimeoutError"; break;
        case DatagramTooLargeError: S += "DatagramTooLargeError"; break;
        case NetworkError: S += "NetworkError"; break;
        case AddressInUseError: S += "AddressInUseError"; break;
        case SocketAddressNotAvailableError: S += "SocketAddressNotAvailableError"; break;
        case UnsupportedSocketOperationError: S += "UnsupportedSocketOperationError"; break;
        case UnknownSocketError: S += "UnknownSocketError"; break;
        default: S += " Unknown error";
    }

    return S;
}

void QTestProtocol::onConnectionFailed( const QString &reason )
{
    emit connectionFailed( this, reason );
}

void QTestProtocol::testConnection()
{
    if (debug_on) qLog(QtUitest) <<  QString("%1 QTestProtocol::testConnection()").arg(uniqueId()).toLatin1();

    while (send_msg_replies.count() > 0)
        delete send_msg_replies.takeFirst();

    enablePing( true );
    postMessage( QTestMessage("QTEST_NEW_CONNECTION") );
}

void QTestProtocol::send( QTestMessage const &message )
{
    QByteArray data;
    if (!message.m_map.isEmpty()) {
        QDataStream s(&data, QIODevice::WriteOnly);
        s << message.m_map;
    }

    QDataStream tmp(this);
    sendPreamble( &tmp, message.msgId(),  message.event() );

    quint32 len = data.count();
    tmp << len;           // phase 2
    if (len > 0) {
        tmp.writeRawData( data.data(), (int)len ); // phase 3
    }

    tmp << TEST_MSG_END;       // phase 4

    flush();    // Force socket to send data now
}

void QTestProtocol::sendPreamble( QDataStream *ds, quint16 msgId, const QString &event )
{
    quint32 len;
    *ds << TEST_MSG_SIGNATURE;        // phase 0
    *ds << TEST_MSG_VERSION;
    *ds << msgId;

    len = (event.length() *2) + 4;
    *ds << len;
    *ds << event;      // phase 1
}

bool QTestProtocol::receive( QTestMessage *msg, bool &syncError )
{
    syncError = false;

    QDataStream stream(this);

    quint8 rx_version;
    if (msg->m_phase == uint(0)) {
        msg->m_len = 0;
        quint32 sig;
        if (bytesAvailable() >= sizeof( sig ) + sizeof( rx_version ) + sizeof( msg->m_msg_id ) + sizeof( msg->m_len )) {
            stream >> sig;
            if (sig != TEST_MSG_SIGNATURE) {
                qWarning( QString("QTestMessage::receive(), Invalid start signature (0x%1)").arg(sig,8,16).toLatin1() );
                syncError = true;
                return false;
            } else {
                stream >> rx_version;  // FIXME: something should be done to verify the version.
                stream >> msg->m_msg_id;
                stream >> msg->m_len;
                msg->m_phase++;
            }
        }
    }

    if (msg->m_phase == uint(1)) {
            if (bytesAvailable() >= msg->m_len) {
                stream >> msg->m_event;
                msg->m_phase++;
        }
    }

    if (msg->m_phase == uint(2)) {
        if (bytesAvailable() >= sizeof( msg->m_len )) {
            stream >> msg->m_len;
            msg->m_phase++;
        }
    }

    if (msg->m_phase == uint(3)) {
        if (msg->m_len > 0) {
            QByteArray buf;
            quint32 len = msg->m_len;
            uint bytes_available = bytesAvailable();
            if (bytes_available < len)
                len = bytes_available;
            buf.resize( len );
            stream.readRawData( buf.data(), len );

            static QMap<QTestMessage*, QByteArray> data;
            data[msg].append(buf);

            msg->m_len -= len;
            if (msg->m_len == 0) {
                QDataStream s(&data[msg], QIODevice::ReadOnly);
                s >> msg->m_map;
                data.remove(msg);
                msg->m_phase++;
                // received OK
            } else {
                // waiting for more data
                return false;
            }
        } else {
            msg->m_phase++;
        }
    }

    if (msg->m_phase == uint(4)) {
        quint32 id2;
        if (bytesAvailable() >= sizeof( id2 )) {
            stream >> id2;
            msg->m_phase = 0;
            if (id2 != TEST_MSG_END) {
                qWarning( QString("QTestMessage::receive(), Invalid end signature (0x%2)").arg(id2,8,16).toLatin1() );
                syncError = true;
                return false;
            } else {
                return true;
            }
        }
    }

    return false;
}

bool QTestProtocol::rxBusy()
{
    return rx_busy;
}

/*!
    Reads the remote control connection and responds to received commands.
*/

void QTestProtocol::onData()
{
    if (onData_busy) return;
    onData_busy = true;

    int sync_error_count = 0;
    bool msg_received = true;
    while (msg_received && bytesAvailable() > 0) {
        msg_received = false;
        // set the time to now :-)
        rx_timer.start();
        ping_timeout_warning_issued = false;

        bool sync_error;
        if (cur_message == 0)
            cur_message = new QTestMessage();

        if (receive( cur_message, sync_error )) {
            msg_received = true;
            QString last_event = cur_message->event();

            if (debug_on) qLog(QtUitest) << ( QString("%1 QTestProtocol::onData(%2) msgid = %3").
                                            arg(uniqueId()).
                                            arg(last_event).
                                            arg(cur_message->m_msg_id).
                                            toLatin1());

            // We received a full message
            if (last_event == "@REPLY@") {
                send_msg_replies.append( cur_message ); // add the reply to a list
                int id = cur_message->m_msg_id;
                QTestMessage *received_message(cur_message);
                cur_message = 0; // and make sure we create a new one
                emit replyReceived( id, received_message );
            } else if (last_event == "QTEST_NEW_CONNECTION") {
                postMessage( QTestMessage("QTEST_ACK_CONNECTION") ); // Acknowledge the other side we can receive data
            } else if (last_event == "QTEST_ACK_CONNECTION") {
                connection_valid = true; // we don't assume we have a connection until both sides have actually received data from the other side
                onConnected();
                connect_timer.stop();
            } else if (last_event == "QTEST_CLOSING_CONNECTION") {
                last_data_received = true;
                QTimer::singleShot( 0, this, SLOT(onSocketClosed()));
            } else if (last_event == "PONG") {
                // Do nothing
            } else if (last_event == "TEST") {
                if (!debug_on) // don't show the same information twice
                    qLog(QtUitest) <<  QString("%1 Test message received").arg(uniqueId()).toLatin1() ;
            } else if (last_event == "PING") {
                postMessage( QTestMessage("PONG") );
            } else {
                unhandled_msg.append( cur_message ); // add the msg to a list
                cur_message = 0;
                QTimer::singleShot(0,this,SLOT(processMessages()));
            }

            delete cur_message;
            cur_message = 0;
        } else {
            // We didn't receive a full message
            if (sync_error) {
                sync_error_count++;
                if (sync_error_count > 10)
                    return;
                // receiving garbage messages - nothing can be done but closing the connection and try again.
                delete cur_message;
                cur_message = 0;
                disconnect(!enable_reconnect);
                reconnect();
            }
            // else we are waiting on more fragments to arrive
        }
    }
    onData_busy = false;
}

void QTestProtocol::processMessages()
{
    while (!rx_busy && unhandled_msg.count() > 0) {
        QTestMessage *tmp = unhandled_msg.takeFirst();
        if (tmp) {
            rx_busy = true;
            processMessage( tmp );
            delete tmp;
            rx_busy = false;
        }
    }
}

/*!
    Signals the instance that the other side has closed the connection.
*/
void QTestProtocol::onSocketClosed()
{
    if (debug_on) qLog(QtUitest) <<  QString("%1 QTestProtocol::onSocketClosed()").arg(uniqueId()).toLatin1();

    // we can't send any more messages so disable pinging
    enablePing( false );

    // anything that is still in the tx buffer gets lost
    abort();

    close();

    connection_valid = false;

    // if the close was spontaneous and we want to keep the connection alive, we try to reconnect
    if (enable_reconnect) {
        if (debug_on) qLog(QtUitest) <<  QString("%1 QTestProtocol::onSocketClosed() singleshot reconnect in .5 seconds").arg(uniqueId()).toLatin1();
        QTimer::singleShot(500,this,SLOT(reconnect()));
    }

    // tell the world we are closed
    QTimer::singleShot(0, this, SLOT(emitConnectionClosed()));
}

/*!
    Signals the instance that a connection is established with a remote control host.
*/
void QTestProtocol::onSocketConnected()
{
    connect_timer.stop();
    if (debug_on) qLog(QtUitest) <<  QString("%1 QTestProtocol::onSocketConnected()").arg(uniqueId()).toLatin1();
    testConnection();
}

void QTestProtocol::reconnect()
{
    if (host != "" && state() != ConnectedState) {
        if (debug_on) qLog(QtUitest) <<  QString("%1 QTestProtocol::reconnect()").arg(uniqueId()).toLatin1();
        connect_timer.stop();
        connect_timer.start( CONNECT_TIMEOUT );

        // if we are trying to connect to the local machine, always use 127.0.0.1
        // (and avoid the need for dns)
        QString hostname = QHostInfo::localHostName().toUpper();
        if (hostname == host.toUpper() || hostname.startsWith( host.toUpper() + "." ))
            host = "127.0.0.1";

        close();
        connectToHost( host, port );
    } else {
        if (host == "") {
            qWarning( "QTestProtocol::reconnect() FAILED, no host specified" );
            enable_reconnect = false;
        }
    }
}

void QTestProtocol::connectTimeout()
{
    if (debug_on) qLog(QtUitest) <<  QString("%1 QTestProtocol::connectTimeout()").arg(uniqueId()).toLatin1();
    connect_timer.stop();
    if (enable_reconnect) {
        reconnect();
    } else {
        onConnectionFailed( errorStr() );
    }
}

void QTestProtocol::pingTimeout()
{
    if (!ping_enabled) return;

    if (state() == ConnectedState) {
        int elapsed = rx_timer.elapsed();
        if (state() == ClosingState) {
            if (elapsed > 30000) { // no activity for x seconds when we are closing?
                enablePing( false );
                if (enable_reconnect) {
                    disconnect();
                    // close has reset the enable_reconnect value, so enable it again
                    enableReconnect( true, reconnect_interval );
                    reconnect();
                } else {
                    disconnect();
                }
            } else if (elapsed > 2000 ) {
                postMessage( QTestMessage("PING") );
            }
        } else {
            if (elapsed > 10000) {
                postMessage( QTestMessage("PING") );
                if (elapsed > 300000) { // no activity for 5 minutes in 'normal' cases??
                    if (!ping_timeout_warning_issued)
                        qWarning( QString("%1 QTestProtocol::pingTimeout() WARNING: Did not receive a msg for %2 ms").arg(uniqueId()).arg(elapsed).toLatin1() );
                    ping_timeout_warning_issued = true;
                } 
            }
        }
    } else {
        if (enable_reconnect) {
            // Connection seems to be closed, try to reconnect
            disconnect();

            // disconnect has reset the enable_reconnect value, so enable it again
            enableReconnect( true, reconnect_interval );
            reconnect();
        }
    }
}

void QTestProtocol::emitConnectionClosed()
{
    if (debug_on) qLog(QtUitest) << ( QString("%1 QTestProtocol::emitConnectionClosed()").
                    arg(uniqueId()).toLatin1());

    emit replyReceived(); // force sendMessage to quit
    emit connectionClosed( this );
}

void QTestProtocol::enablePing( bool enable )
{
    if (ping_enabled != enable) {
        ping_timer.stop();
        ping_enabled = enable;
        if (enable) {
            QObject::connect( &ping_timer, SIGNAL(timeout()), this, SLOT(pingTimeout()), Qt::DirectConnection );
            ping_timer.start( ping_interval );
        } else {
            QObject::disconnect( &ping_timer, SIGNAL(timeout()), this, SLOT(pingTimeout()) );
        }
    }
}

QString QTestProtocol::uniqueId()
{
    return QString("%1 %2").arg(unique_id).arg(qApp->applicationName());
}

void QTestProtocol::enableDebug( bool debugOn )
{
    debug_on = debugOn;
    qLog(QtUitest) <<  QString("Debugging is switched %1 for Test Protocol %2").arg(debugOn ? "ON" : "OFF").arg(uniqueId()).toLatin1() ;
}

void QTestProtocol::disableDebug()
{
    debug_on = false;
    qLog(QtUitest) <<  QString("Debugging is switched %1 for Test Protocol %2").arg(debug_on ? "ON" : "OFF").arg(uniqueId()).toLatin1() ;
}

