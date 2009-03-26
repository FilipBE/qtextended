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

#include "qcopchannel_x11.h"

#if defined(Q_WS_X11)

#include "qcopchannel_x11_p.h"
#include "qlist.h"
#include "qmap.h"
#include "qdatastream.h"
#include "qpointer.h"
#include "qmutex.h"
#include "qregexp.h"
#include "qapplication.h"
#include "qeventloop.h"
#include "qdebug.h"
#include "qtimer.h"
#include "qtopianamespace.h"
#ifndef QT_NO_SXE
#include "qtransportauth_qws.h"
#include "qtransportauthdefs_qws.h"
#include "qvarlengtharray.h"
#endif

// Define this to enable debugging.  The code below is performance-critical,
// which is why this isn't turned on permanently or using qLog.
//#define QCOP_DEBUG 1

typedef QMap<QString, QList<QCopX11Client*> > QCopServerMap;
static QCopServerMap *qcopServerMap = 0;

class QCopServerRegexp
{
public:
    QCopServerRegexp( const QString& channel, QCopX11Client *client );
    QCopServerRegexp( const QCopServerRegexp& other );

    QString channel;
    QCopX11Client *client;
    QRegExp regexp;
};

QCopServerRegexp::QCopServerRegexp( const QString& channel, QCopX11Client *client )
{
    this->channel = channel;
    this->client = client;
    this->regexp = QRegExp( channel, Qt::CaseSensitive, QRegExp::Wildcard );
}

QCopServerRegexp::QCopServerRegexp( const QCopServerRegexp& other )
{
    channel = other.channel;
    client = other.client;
    regexp = other.regexp;
}

static QCopX11Client *clientConnection = 0;
static QCopServer *qcopServer = 0;

typedef QList<QCopServerRegexp> QCopServerRegexpList;
static QCopServerRegexpList *qcopServerRegexpList = 0;

typedef QMap<QString, QList< QPointer<QCopChannel> > > QCopClientMap;
static QCopClientMap *qcopClientMap = 0;

Q_GLOBAL_STATIC(QMutex, qcopClientMapMutex)

// Determine if a channel name contains wildcard characters.
static bool containsWildcards( const QString& channel )
{
    return channel.contains(QLatin1Char('*'));
}

class QCopChannelPrivate
{
public:
    QString channel;
};

/*!
    \class QCopChannel
    \inpublicgroup QtBaseModule
    \ingroup qws

    \brief The QCopChannel class provides communication capabilities
    between clients.

    QCOP is a many-to-many communication protocol for transferring
    messages on various channels. A channel is identified by a name,
    and anyone who wants to can listen to it. The QCOP protocol allows
    clients to communicate both within the same address space and
    between different processes, but it is currently only available
    for \l {Qt for Embedded Linux} (on X11 and Windows we are exploring the use
    of existing standards such as DCOP and COM).

    Typically, QCopChannel is either used to send messages to a
    channel using the provided static functions, or to listen to the
    traffic on a channel by deriving from the class to take advantage
    of the provided functionality for receiving messages.

    QCopChannel provides a couple of static functions which are usable
    without an object: The send() function, which sends the given
    message and data on the specified channel, and the isRegistered()
    function which queries the server for the existence of the given
    channel.

    In addition, the QCopChannel class provides the channel() function
    which returns the name of the object's channel, the virtual
    receive() function which allows subclasses to process data
    received from their channel, and the received() signal which is
    emitted with the given message and data when a QCopChannel
    subclass receives a message from its channel.

    \sa QCopServer, {Running Qt for Embedded Linux Applications}
*/

/*!
    Constructs a QCop channel with the given \a parent, and registers it
    with the server using the given \a channel name.

    \sa isRegistered(), channel()
*/

QCopChannel::QCopChannel(const QString& channel, QObject *parent) :
    QObject(parent)
{
    init(channel);
}

void QCopChannel::init(const QString& channel)
{
    d = new QCopChannelPrivate;
    d->channel = channel;

    if (!qApp) {
        qFatal("QCopChannel: Must construct a QApplication "
                "before QCopChannel");
        return;
    }

    {
	QMutexLocker locker(qcopClientMapMutex());

	if (!qcopClientMap)
	    qcopClientMap = new QCopClientMap;

	// do we need a new channel list ?
	QCopClientMap::Iterator it = qcopClientMap->find(channel);
	if (it != qcopClientMap->end()) {
	    it.value().append(this);
	    return;
	}

	it = qcopClientMap->insert(channel, QList< QPointer<QCopChannel> >());
	it.value().append(QPointer<QCopChannel>(this));
    }

    // inform server about this channel
    if ( !clientConnection )
	clientConnection = new QCopX11Client();
    clientConnection->registerChannel(channel);
}

/*!
  \internal

  Resend all channel registrations
  */
void QCopChannel::reregisterAll()
{
    if(qcopClientMap) {
        for(QCopClientMap::Iterator iter = qcopClientMap->begin();
            iter != qcopClientMap->end();
            ++iter) {
            if ( !clientConnection )
	        clientConnection = new QCopX11Client();
            clientConnection->registerChannel(iter.key());
        }
    }
}

/*!
    Destroys the client's end of the channel and notifies the server
    that the client has closed its connection. The server will keep
    the channel open until the last registered client detaches.

    \sa QCopChannel()
*/

QCopChannel::~QCopChannel()
{
    QMutexLocker locker(qcopClientMapMutex());
    QCopClientMap::Iterator it = qcopClientMap->find(d->channel);
    Q_ASSERT(it != qcopClientMap->end());
    it.value().removeAll(this);
    // still any clients connected locally ?
    if (it.value().isEmpty()) {
        if (clientConnection)
            clientConnection->detachChannel(d->channel);
        qcopClientMap->remove(d->channel);
    }

    delete d;
}

/*!
    Returns the name of the channel.

    \sa QCopChannel()
*/

QString QCopChannel::channel() const
{
    return d->channel;
}

/*!
    \fn void QCopChannel::receive(const QString& message, const QByteArray &data)

    This virtual function allows subclasses of QCopChannel to process
    the given \a message and \a data received from their channel. The default
    implementation emits the received() signal.

    Note that the format of the given \a data has to be well defined
    in order to extract the information it contains. In addition, it
    is recommended to use the DCOP convention. This is not a
    requirement, but you must ensure that the sender and receiver
    agree on the argument types.

    Example:

    \code
        void MyClass::receive(const QString &message, const QByteArray &data)
        {
            QDataStream in(data);
            if (message == "execute(QString,QString)") {
                QString cmd;
                QString arg;
                in >> cmd >> arg;
                ...
            } else if (message == "delete(QString)") {
                QString fileName;
                in >> fileName;
                ...
            } else {
                ...
            }
        }
    \endcode

    This example assumes that the \c message is a DCOP-style function
    signature and the \c data contains the function's arguments.

    \sa send()
 */
void QCopChannel::receive(const QString& msg, const QByteArray &data)
{
    emit received(msg, data);
}

/*!
    \fn void QCopChannel::received(const QString& message, const QByteArray &data)

    This signal is emitted with the given \a message and \a data whenever the
    receive() function gets incoming data.

    \sa receive()
*/

/*!
    Queries the server for the existence of the given \a channel. Returns true
    if the channel is registered; otherwise returns false.

    \sa channel(), QCopChannel()
*/

bool QCopChannel::isRegistered(const QString&  channel)
{
    if ( !clientConnection )
        clientConnection = new QCopX11Client();
    clientConnection->requestRegistered(channel);
    return clientConnection->waitForIsRegistered();
}

/*!
    \fn bool QCopChannel::send(const QString& channel, const QString& message)
    \overload

    Sends the given \a message on the specified \a channel.  The
    message will be distributed to all clients subscribed to the
    channel.
*/

bool QCopChannel::send(const QString& channel, const QString& msg)
{
    QByteArray data;
    return send(channel, msg, data);
}

/*!
    \fn bool QCopChannel::send(const QString& channel, const QString& message,
                       const QByteArray &data)

    Sends the given \a message on the specified \a channel with the
    given \a data.  The message will be distributed to all clients
    subscribed to the channel. Returns true if the message is sent
    successfully; otherwise returns false.

    It is recommended to use the DCOP convention. This is not a
    requirement, but you must ensure that the sender and receiver
    agree on the argument types.

    Note that QDataStream provides a convenient way to fill the byte
    array with auxiliary data.

    Example:

    \code
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out << QString("cat") << QString("file.txt");
        QCopChannel::send("System/Shell", "execute(QString,QString)", data);
    \endcode

    Here the channel is \c "System/Shell". The \c message is an
    arbitrary string, but in the example we've used the DCOP
    convention of passing a function signature. Such a signature is
    formatted as \c "functionname(types)" where \c types is a list of
    zero or more comma-separated type names, with no whitespace, no
    consts and no pointer or reference marks, i.e. no "*" or "&".

    \sa receive()
*/

bool QCopChannel::send(const QString& channel, const QString& msg,
                       const QByteArray &data)
{
    if (!qApp) {
        qFatal("QCopChannel::send: Must construct a QApplication "
                "before using QCopChannel");
        return false;
    }

    if (!clientConnection)
	clientConnection = new QCopX11Client();

    clientConnection->send(channel, msg, data);

    return true;
}

/*!
    \since 4.2

    Flushes any pending messages queued through QCopChannel::send() to any subscribed clients.
    Returns false if no QApplication has been constructed, otherwise returns true.
    When using QCopChannel::send(), messages are queued and actually sent when Qt re-enters the event loop.
    By using this function, an application can immediately flush these queued messages,
    and therefore reliably know that any subscribed clients will receive them.
*/
bool QCopChannel::flush()
{
    if (!qApp) {
        qFatal("QCopChannel::flush: Must construct a QApplication "
                "before using QCopChannel");
        return false;
    }

    if (clientConnection)
        clientConnection->flush();

    return true;
}

/*!
    \internal
    Server side: subscribe client \a cl on channel \a ch.
*/

void QCopChannel::registerChannel(const QString& ch, QCopX11Client *cl)
{
    if (!qcopServerMap)
        qcopServerMap = new QCopServerMap;

    // do we need a new channel list ?
    QCopServerMap::Iterator it = qcopServerMap->find(ch);
    if (it == qcopServerMap->end())
      it = qcopServerMap->insert(ch, QList<QCopX11Client*>());

    // If the channel name contains wildcard characters, then we also
    // register it on the server regexp matching list.
    if (containsWildcards( ch )) {
	QCopServerRegexp item(ch, cl);
	if (!qcopServerRegexpList)
	    qcopServerRegexpList = new QCopServerRegexpList;
	qcopServerRegexpList->append( item );
    }

    // If this is the first client in the channel, announce the channel as being created.
    if (it.value().count() == 0) {
      emit qcopServer->newChannel(ch);
    }

    it.value().append(cl);
}

/*!
    \internal
    Server side: request subscription state from client \a cl for channel \a ch.
*/
void QCopChannel::requestRegistered(const QString& ch, QCopX11Client *cl)
{
    bool known = qcopServerMap && qcopServerMap->contains(ch)
                 && !((*qcopServerMap)[ch]).isEmpty();
    cl->isRegisteredReply(ch, known);
}

/*!
    \internal
    Server side: unsubscribe \a cl from all channels.
*/

void QCopChannel::detach(QCopX11Client *cl)
{
    if (!qcopServerMap)
        return;

    QCopServerMap::Iterator it = qcopServerMap->begin();
    for (; it != qcopServerMap->end(); it++) {
      if (it.value().contains(cl)) {
        it.value().removeAll(cl);
        // If this was the last client in the channel, announce the channel as dead.
        if (it.value().count() == 0) {
	  emit qcopServer->removedChannel(it.key());
        }
      }
    }

    if (!qcopServerRegexpList)
	return;

    QCopServerRegexpList::Iterator it2 = qcopServerRegexpList->begin();
    while(it2 != qcopServerRegexpList->end()) {
	if ((*it2).client == cl)
	    it2 = qcopServerRegexpList->erase(it2);
	else
	    ++it2;
    }
}

/*!
    \internal
    Server side: unsubscribe \a cl from channel \a ch.
*/

void QCopChannel::detach(const QString& ch, QCopX11Client *cl)
{
    if (!qcopServerMap)
        return;

    QCopServerMap::Iterator it = qcopServerMap->find(ch);
    if (it != qcopServerMap->end()) {
        it.value().removeAll(cl);
        if (it.value().isEmpty()) {
          // If this was the last client in the channel, announce the channel as dead
          emit qcopServer->removedChannel(it.key());
          qcopServerMap->erase(it);
        }
    }
    if (qcopServerRegexpList && containsWildcards(ch)) {
        // Remove references to a wildcarded channel.
        QCopServerRegexpList::Iterator it
            = qcopServerRegexpList->begin();
        while(it != qcopServerRegexpList->end()) {
            if ((*it).client == cl && (*it).channel == ch)
                it = qcopServerRegexpList->erase(it);
            else
                ++it;
        }
    }
}

/*!
    \internal
    Server side: transmit the message to all clients registered to the
    specified channel.
*/

void QCopChannel::answer(QCopX11Client * /*cl*/, const QString& ch,
                          const QString& msg, const QByteArray &data)
{
    if (qcopServerMap) {
        QList<QCopX11Client*> clist = qcopServerMap->value(ch);
        for (int i=0; i < clist.size(); ++i) {
            QCopX11Client *c = clist.at(i);
	    c->send( ch, msg, data );
        }
    }

    if(qcopServerRegexpList && !containsWildcards(ch)) {
	// Search for wildcard matches and forward the message on.
	QCopServerRegexpList::ConstIterator it = qcopServerRegexpList->constBegin();
	for (; it != qcopServerRegexpList->constEnd(); ++it) {
	    if ((*it).regexp.exactMatch(ch))
		(*it).client->forward(ch, msg, data, (*it).channel);
	}
    }
}

/*!
    \internal
    Client side: distribute received event to the QCop instance managing the
    channel.
*/
void QCopChannel::sendLocally(const QString& ch, const QString& msg,
                                const QByteArray &data)
{
    Q_ASSERT(qcopClientMap);

    // filter out internal events
    if (ch.isEmpty())
        return;

    // feed local clients with received data
    QList< QPointer<QCopChannel> > clients;
    {
	QMutexLocker locker(qcopClientMapMutex());
	clients = (*qcopClientMap)[ch];
    }
    for (int i = 0; i < clients.size(); ++i) {
	QCopChannel *channel = (QCopChannel *)clients.at(i);
	if ( channel )
	    channel->receive(msg, data);
    }
}

struct QCopPacketHeader
{
    int totalLength;
    int command;
    int chLength;
    int msgLength;
    int forwardToLength;
    int dataLength;
};

#define	QCopCmd_RegisterChannel	    1
#define	QCopCmd_Send		    2
#define	QCopCmd_IsRegistered        3
#define	QCopCmd_IsNotRegistered     4
#define	QCopCmd_RequestRegistered   5
#define	QCopCmd_DetachChannel       6
#define	QCopCmd_Forward             7

#ifndef QT_NO_SXE

class QCopRequestAnalyzer : public RequestAnalyzer
{
public:
    QCopRequestAnalyzer() : RequestAnalyzer() {}

protected:
    QString analyze( QByteArray * );
};

// Read an integer value in host byte order.
static int read_int( QIODevice *dev )
{
    union {
        char buf[sizeof(qint32)];
        qint32 value;
    } un;
    un.buf[0] = un.buf[1] = un.buf[2] = un.buf[3] = 0;
    dev->getChar( &(un.buf[0]) );
    dev->getChar( &(un.buf[1]) );
    dev->getChar( &(un.buf[2]) );
    dev->getChar( &(un.buf[3]) );
    return un.value;
}

// Read a UTF-16 string value.
static QString read_string( QIODevice *dev, int length )
{
    QVarLengthArray<ushort> buf;
    buf.reserve( length );
    dev->read( (char *)(buf.data()), length * 2 );
    return QString::fromUtf16( buf.data(), length );
}

// Analyse an incoming message so that it can be checked against SXE policies.
QString QCopRequestAnalyzer::analyze( QByteArray *msgQueue )
{
    dataSize = 0;
    moreData = false;

    QBuffer cmdBuf( msgQueue );
    cmdBuf.open( QIODevice::ReadOnly | QIODevice::Unbuffered );

    // Bail out if we don't have enough bytes for a complete packet.
    int size = read_int( &cmdBuf );
    if ( size < (int)sizeof(qint32) ) {
        moreData = true;
        return QString();
    }
    if ( size < QCopX11Client::minPacketSize )
        size = QCopX11Client::minPacketSize;
    if ( size > ( msgQueue->size() - cmdBuf.pos() + sizeof(qint32) ) ) {
        moreData = true;
        return QString();
    }

    // Read the command code and map it to a string.
    int cmd = read_int( &cmdBuf );
    QString request;
    bool needChannel = false;
    bool needMessage = false;
    switch ( cmd ) {

        case QCopCmd_RegisterChannel:
            request = "QCopRegisterChannel/QCop/RegisterChannel/";
            needChannel = true;
            break;

        case QCopCmd_Send:
            request = "QCopSend/QCop/";
            needChannel = true;
            needMessage = true;
            break;

        case QCopCmd_DetachChannel:
            request = "QCopSend/QCop//detach()";
            break;

        case QCopCmd_RequestRegistered:
            request = "QCopSend/QCop//isRegistered()";
            break;

        default: return QString();  // Shouldn't happen in the server.
    }

    // Read the rest of the packet header.
    int chLength = read_int( &cmdBuf );
    int msgLength = read_int( &cmdBuf );
    read_int( &cmdBuf );    // forwardToLength - don't need this
    read_int( &cmdBuf );    // dataLength - don't need this

    // Add the channel and message names if necessary.
    if ( needChannel )
        request += read_string( &cmdBuf, chLength );
    if ( needMessage )
        request += "/" + read_string( &cmdBuf, msgLength );

    // Remove the packet we just processed from the queue.
    dataSize = size;

    // Return the policy string to the caller.
    return request;
}

#endif // QT_NO_SXE

int qtopia_display_id(); // from qtopianamespace.cpp

QCopX11Client::QCopX11Client()
    : QObject()
{
    socket = new QUnixSocket( this );
    device = socket;
    server = false;
    init();
    connectToServer();
}

QCopX11Client::QCopX11Client( QIODevice *device, QUnixSocket *socket )
    : QObject()
{
    this->device = device;
    this->socket = socket;
    server = true;
    init();
}

void QCopX11Client::init()
{
    if ( server )
        connectSignals();

    inBufferUsed = 0;
    inBufferExpected = minPacketSize;
    inBufferPtr = inBuffer;

    isRegisteredResponse = false;
    isRegisteredWaiter = 0;

    retryCount = 0;
    connecting = false;
}

QCopX11Client::~QCopX11Client()
{
    if ( socket )
        delete socket;
}

void QCopX11Client::registerChannel( const QString& ch )
{
    sendChannelCommand( QCopCmd_RegisterChannel, ch );
}

void QCopX11Client::detachChannel( const QString& ch )
{
    sendChannelCommand( QCopCmd_DetachChannel, ch );
}

void QCopX11Client::sendChannelCommand( int cmd, const QString& ch )
{
    int len = ch.length() * 2 + sizeof(QCopPacketHeader);
    int writelen;
    char *buf;
    if ( len <= minPacketSize ) {
	buf = outBuffer;
	memset( buf + len, 0, minPacketSize - len );
        writelen = minPacketSize;
    } else {
	buf = new char [len];
        writelen = len;
    }
    QCopPacketHeader *header = (QCopPacketHeader *)buf;
    header->command = cmd;
    header->totalLength = len;
    header->chLength = ch.length();
    header->msgLength = 0;
    header->forwardToLength = 0;
    header->dataLength = 0;
    char *ptr = buf + sizeof(QCopPacketHeader);
    memcpy( ptr, ch.constData(), ch.length() * 2 );
    write( buf, writelen );
    if ( buf != outBuffer )
	delete[] buf;
}

void QCopX11Client::send
    ( const QString& ch, const QString& msg, const QByteArray& data )
{
    int len = ch.length() * 2 + msg.length() * 2 + data.length();
    len += sizeof(QCopPacketHeader);
    int writelen;
    char *buf;
    if ( len <= minPacketSize ) {
	buf = outBuffer;
	memset( buf + len, 0, minPacketSize - len );
        writelen = minPacketSize;
    } else {
	buf = new char [len];
        writelen = len;
    }
    QCopPacketHeader *header = (QCopPacketHeader *)buf;
    header->command = QCopCmd_Send;
    header->totalLength = len;
    header->chLength = ch.length();
    header->msgLength = msg.length();
    header->forwardToLength = 0;
    header->dataLength = data.length();
    char *ptr = buf + sizeof(QCopPacketHeader);
    memcpy( ptr, ch.constData(), ch.length() * 2 );
    ptr += ch.length() * 2;
    memcpy( ptr, msg.constData(), msg.length() * 2 );
    ptr += msg.length() * 2;
    memcpy( ptr, data.constData(), data.length() );
    write( buf, writelen );
    if ( buf != outBuffer )
	delete[] buf;
}

void QCopX11Client::forward
    ( const QString& ch, const QString& msg, const QByteArray& data,
      const QString& forwardTo)
{
    int len = ch.length() * 2 + msg.length() * 2 + data.length();
    len += forwardTo.length() * 2;
    len += sizeof(QCopPacketHeader);
    int writelen;
    char *buf;
    if ( len <= minPacketSize ) {
	buf = outBuffer;
	memset( buf + len, 0, minPacketSize - len );
        writelen = minPacketSize;
    } else {
	buf = new char [len];
        writelen = len;
    }
    QCopPacketHeader *header = (QCopPacketHeader *)buf;
    header->command = QCopCmd_Forward;
    header->totalLength = len;
    header->chLength = ch.length();
    header->msgLength = msg.length();
    header->forwardToLength = forwardTo.length();
    header->dataLength = data.length();
    char *ptr = buf + sizeof(QCopPacketHeader);
    memcpy( ptr, ch.constData(), ch.length() * 2 );
    ptr += ch.length() * 2;
    memcpy( ptr, msg.constData(), msg.length() * 2 );
    ptr += msg.length() * 2;
    memcpy( ptr, forwardTo.constData(), forwardTo.length() * 2 );
    ptr += forwardTo.length() * 2;
    memcpy( ptr, data.constData(), data.length() );
    write( buf, writelen );
    if ( buf != outBuffer )
	delete[] buf;
}

void QCopX11Client::isRegisteredReply( const QString& ch, bool known )
{
    if ( known )
        sendChannelCommand( QCopCmd_IsRegistered, ch );
    else
        sendChannelCommand( QCopCmd_IsNotRegistered, ch );
}

void QCopX11Client::requestRegistered( const QString& ch )
{
    sendChannelCommand( QCopCmd_RequestRegistered, ch );
}

void QCopX11Client::flush()
{
    if ( socket )
        socket->flush();
}

bool QCopX11Client::waitForIsRegistered()
{
    if ( isRegisteredWaiter )
        return false;       // Recursive re-entry!
    isRegisteredWaiter = new QEventLoop( this );
    isRegisteredWaiter->exec();
    delete isRegisteredWaiter;
    isRegisteredWaiter = 0;
    return isRegisteredResponse;
}

void QCopX11Client::readyRead()
{
    qint64 len;
    while ( device->bytesAvailable() > 0 ) {
        if ( inBufferUsed < inBufferExpected ) {
            len = device->read
                ( inBufferPtr + inBufferUsed, inBufferExpected - inBufferUsed );
            if ( len <= 0 )
                break;
            inBufferUsed += (int)len;
        }
        if ( inBufferUsed >= minPacketSize ) {
            // We have the full packet header and minimal payload.
            QCopPacketHeader *header = (QCopPacketHeader *)inBufferPtr;
            if ( header->totalLength > inBufferExpected ) {
                // Expand the buffer and continue reading.
                inBufferExpected = header->totalLength;
                inBufferPtr = new char [header->totalLength];
                memcpy( inBufferPtr, inBuffer, minPacketSize );
                continue;
            }
        }
        if ( inBufferUsed >= inBufferExpected ) {
            // We have a full packet to be processed.  Parse it.
            QCopPacketHeader *header = (QCopPacketHeader *)inBufferPtr;
            int command = header->command;
            QString channel, msg, forwardTo;
            QByteArray data;
            char *ptr = inBufferPtr + sizeof(QCopPacketHeader);
            if ( header->chLength > 0 ) {
                channel = QString::fromUtf16
                    ( (const ushort *)ptr, header->chLength );
                ptr += header->chLength * 2;
            }
            if ( header->msgLength > 0 ) {
                msg = QString::fromUtf16
                    ( (const ushort *)ptr, header->msgLength );
                ptr += header->msgLength * 2;
            }
            if ( header->forwardToLength > 0 ) {
                forwardTo = QString::fromUtf16
                    ( (const ushort *)ptr, header->forwardToLength );
                ptr += header->forwardToLength * 2;
            }
            if ( header->dataLength > 0 )
                data = QByteArray ( (const char *)ptr, header->dataLength );

            // Discard the input buffer contents.
            if ( inBufferPtr != inBuffer )
                delete[] inBufferPtr;
            inBufferPtr = inBuffer;
            inBufferUsed = 0;
            inBufferExpected = minPacketSize;

            // Dispatch the command that we received.
            if ( server ) {
                // Processing command on server side.
                if ( command == QCopCmd_Send ) {
#ifdef QCOP_DEBUG
                    qWarning() << "Answer" << channel << msg;
#endif
                    QCopChannel::answer( this, channel, msg, data );
                } else if ( command == QCopCmd_RegisterChannel ) {
#ifdef QCOP_DEBUG
                    qWarning() << "Register" << channel;
#endif
                    QCopChannel::registerChannel( channel, this );
                } else if ( command == QCopCmd_DetachChannel ) {
#ifdef QCOP_DEBUG
                    qWarning() << "Detach" << channel;
#endif
                    QCopChannel::detach( channel, this );
                } else if ( command == QCopCmd_RequestRegistered ) {
#ifdef QCOP_DEBUG
                    qWarning() << "IsRegistered" << channel;
#endif
                    QCopChannel::requestRegistered( channel, this );
                }
            } else {
                // Processing command on client side.
                if ( command == QCopCmd_Send ) {
#ifdef QCOP_DEBUG
                    qWarning() << "SendLocally" << channel << msg;
#endif
                    QCopChannel::sendLocally( channel, msg, data );
                } else if ( command == QCopCmd_Forward ) {
#ifdef QCOP_DEBUG
                    qWarning() << "Forward" << channel << msg << forwardTo;
#endif
		    QByteArray newData;
		    {
                        QDataStream stream
                            (&newData, QIODevice::WriteOnly | QIODevice::Append);
                        stream << channel;
                        stream << msg;
                        stream << data;
                        // Stream is flushed and closed at this point.
                    }
                    QCopChannel::sendLocally
                        ( forwardTo,
                          "forwardedMessage(QString,QString,QByteArray)",
                          newData );
                } else if ( command == QCopCmd_IsRegistered ||
                            command == QCopCmd_IsNotRegistered ) {
#ifdef QCOP_DEBUG
                    if ( command == QCopCmd_IsRegistered )
                        qWarning() << "IsRegisteredReply" << channel;
                    else
                        qWarning() << "IsNotRegisteredReply" << channel;
#endif
                    if ( isRegisteredWaiter ) {
                        isRegisteredResponse =
                            ( command == QCopCmd_IsRegistered );
                        isRegisteredWaiter->quit();
                    }
                }
            }
        }
    }
}

void QCopX11Client::disconnected()
{
    if ( connecting )
        return;
    if ( server )
	QCopChannel::detach( this );
    else
	clientConnection = 0;
    deleteLater();
}

static QByteArray qcopSocketPath()
{
    return (Qtopia::tempDir() + "qcop-server").toUtf8();
}

void QCopX11Client::connectToServer()
{
    if ( !socket )  {
        // We are retrying the socket connection.
        socket = new QUnixSocket( this );
        device = socket;
    }

    if ( socket->connect( qcopSocketPath() ) ) {
        connecting = false;
#ifndef QT_NO_SXE
        QTransportAuth * a = QTransportAuth::getInstance();
        QTransportAuth::Data * d = a->connectTransport(
                QTransportAuth::UnixStreamSock |
                QTransportAuth::Trusted,
                socket->socketDescriptor());
        QAuthDevice *dev = a->authBuf( d, socket );
        dev->setClient( socket );
        device = dev;
#else
        device = socket;
#endif
        connectSignals();
        if ( pendingData.size() > 0 ) {
            device->write( pendingData.constData(), pendingData.size() );
            pendingData = QByteArray();
        }
    } else {
        connecting = false;
        delete socket;
        socket = 0;
        device = 0;
        if ( ++retryCount < 30 )
            QTimer::singleShot( 200, this, SLOT(connectToServer()) );
        else
            qWarning() << "Could not connect to QCop server; probably not running.";
    }
}

void QCopX11Client::connectSignals()
{
    connect( device, SIGNAL(readyRead()), this, SLOT(readyRead()) );
    connect( socket, SIGNAL(stateChanged(SocketState)), this, SLOT(disconnected()) );
}

void QCopX11Client::write( const char *buf, int len )
{
    // If the socket is open, then send it immediately.
    if ( device ) {
        device->write( buf, len );
        return;
    }

    // Queue up the data for when the socket is open.
    pendingData += QByteArray( buf, len );
}

QCopX11Server::QCopX11Server()
    : QUnixSocketServer()
{
    if ( !listen( qcopSocketPath() ) )
        qWarning() << "Could not listen for qcop connections on"
                   << QString( qcopSocketPath() )
                   << "; another qcop server may already be running.";
}

QCopX11Server::~QCopX11Server()
{
}

void QCopX11Server::incomingConnection( int socketDescriptor )
{
    QUnixSocket * sock = new QUnixSocket;
    sock->setSocketDescriptor(socketDescriptor);

    QCopX11Client *client;
#ifndef QT_NO_SXE
    QTransportAuth *a = QTransportAuth::getInstance();
    QTransportAuth::Data *d = a->connectTransport(
            QTransportAuth::UnixStreamSock |
            QTransportAuth::Trusted, sock->socketDescriptor() );
    QAuthDevice *ad = a->recvBuf( d, sock );
    ad->setRequestAnalyzer( new QCopRequestAnalyzer() );
    ad->setClient(sock);
    client = new QCopX11Client(ad, sock);
#else
    client = new QCopX11Client(sock, sock);
#endif
    sock->setParent(client);
}

/*!
    \class QCopServer
    \inpublicgroup QtBaseModule
    \ingroup qws
    \brief The QCopServer class provides the server-side implementation of QCopChannel.

    QCopServer is used internally by Qt Extended to implement the server-side
    counterpart to QCopChannel on X11 configurations.  It is not used for QWS.

    \sa QCopChannel
*/

/*!
    Construct the QCop server and attach it to \a parent.
*/
QCopServer::QCopServer(QObject *parent)
    : QObject(parent)
{
    server = new QCopX11Server();
    qcopServer = this;
}

/*!
    Destruct the QCop server.
*/
QCopServer::~QCopServer()
{
    delete server;
    qcopServer = 0;
}

/*!
    Returns the active QCopServer instance; or null if there is no active
    QCopServer.
*/
QCopServer *QCopServer::instance()
{
    return qcopServer;
}

/*!
    \fn void QCopServer::newChannel(const QString& channel)

    Signal that is emitted the first time \a channel is registered
    on the system by any of the clients.
*/

/*!
    \fn void QCopServer::removedChannel(const QString& channel)

    Signal that is emitted the last time \a channel is removed
    from the system by any of the clients.
*/

#endif
