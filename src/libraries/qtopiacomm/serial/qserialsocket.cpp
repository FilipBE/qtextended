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

#include <qserialsocket.h>
#include <QTcpSocket>
#include <QTcpServer>
#include <QTimer>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

enum TelnetOption
{
    TO_BINARY   = 0,
    TO_ECHO     = 1,
    TO_RCP      = 2,
    TO_SGA      = 3,
    TO_NAMS     = 4,
    TO_STATUS   = 5,
    TO_TM       = 6,
    TO_RCTE     = 7,
    TO_NAOL     = 8,
    TO_NAOP     = 9,
    TO_NAOCRD   = 10,
    TO_NAOHTS   = 11,
    TO_NAOHTD   = 12,
    TO_NAOFFD   = 13,
    TO_NAOVTS   = 14,
    TO_NAOVTD   = 15,
    TO_NAOLFD   = 16,
    TO_XASCII   = 17,
    TO_LOGOUT   = 18,
    TO_BM       = 19,
    TO_DET      = 20,
    TO_SUPDUP   = 21,
    TO_SUPDUPOUTPUT = 22,
    TO_SNDLOC   = 23,
    TO_TTYPE    = 24,
    TO_EOR      = 25,
    TO_TUID     = 26,
    TO_OUTMRK   = 27,
    TO_TTYLOC   = 28,
    TO_3270REGIME = 29,
    TO_X3PAD    = 30,
    TO_NAWS     = 31,
    TO_TSPEED   = 32,
    TO_LFLOW    = 33,
    TO_LINEMODE = 34,
    TO_XDISPLOC = 35,
    TO_OLD_ENVIRON = 36,
    TO_AUTHENTICATION = 37,
    TO_ENCRYPT  = 38,
    TO_NEW_ENVIRON = 39,
    TO_MODEM_SIGNALS = 78,   // Private extension for modem signals.
    TO_EXOPL    = 255
};

/*!
    \class QSerialSocket
    \inpublicgroup QtBaseModule

    \brief The QSerialSocket class provides a serial port abstraction via a TCP socket.
    \ingroup io

    This serial port abstraction implements the telnet protocol (RFC 854),
    to make it easy to debug AT command handlers using a telnet client.

    The QSerialSocket class is also used by data calls initiated by QPhoneCall
    to pass the raw modem data and handshaking signals to client applications.

    \sa QSerialSocketServer, QSerialIODevice, QPhoneCall
*/
#include <QDebug>
class QSerialSocketPrivate
{
public:
    QSerialSocketPrivate( QTcpSocket *sock )
    {
        socket = sock;
        valid = true;
        dtr = true;
        dsr = true;
        cts = true;
        rts = true;
        incomingCarrier = false;
        outgoingCarrier = false;
        canUseSignals = false;
        bufPosn = 0;
        bufLen = 0;
        optionsLen = 0;
        optionsMode = 0;
    }
    ~QSerialSocketPrivate()
    {
        //delete later -> crashes otherwise due to pending signals
        socket->deleteLater();
    }

    QTcpSocket *socket;
    bool valid;
    bool dtr, dsr, cts, rts;
    bool incomingCarrier;
    bool outgoingCarrier;
    bool canUseSignals;
    char buffer[1024];
    int bufPosn;
    int bufLen;
    char options[1024];
    int optionsLen;
    int optionsMode;
};

QSerialSocket::QSerialSocket( QTcpSocket *socket )
    : QSerialIODevice()
{
    d = new QSerialSocketPrivate( socket );
    socket->setParent( 0 ); //take ownership from QTcpServer. It might delete the socket in front of us
    init();
}

/*!
    Construct a serial-over-sockets session object and attach
    it to \a parent.  The session will connect to \a port on \a host.
*/
QSerialSocket::QSerialSocket
        ( const QString& host, quint16 port, QObject *parent )
    : QSerialIODevice( parent )
{
    d = new QSerialSocketPrivate( new QTcpSocket() );
    init();
    d->socket->connectToHost( host, port );
    if ( ! d->socket->waitForConnected() )
        QTimer::singleShot( 0, this, SIGNAL(closed()) );
}

/*!
    Destroy this socket-based serial port abstraction.
*/
QSerialSocket::~QSerialSocket()
{
    delete d;
}

/*!
    Opens this serial I/O device in \a mode.  This just sets the
    device to open.  The actual connect is done during the constructor.
    Returns true if the device can be opened; false otherwise.
*/
bool QSerialSocket::open( OpenMode mode )
{
    if ( isOpen() )
        return true;
    setOpenMode( mode | Unbuffered );
    return true;
}

/*!
    Closes this serial I/O device.  The socket will remain connected
    until this object is destroyed.
*/
void QSerialSocket::close()
{
    setOpenMode( NotOpen );
}

/*!
    \reimp
*/
bool QSerialSocket::waitForReadyRead(int msecs)
{
    if ( d->bufPosn < d->bufLen )
        return true;
    return d->socket->waitForReadyRead( msecs );
}

/*!
    \reimp
*/
qint64 QSerialSocket::bytesAvailable() const
{
    return d->bufLen - d->bufPosn;
}

/*!
    \reimp
*/
bool QSerialSocket::dtr() const
{
    return d->dtr;
}

/*!
    \reimp
*/
void QSerialSocket::setDtr( bool value )
{
    if ( d->dtr != value ) {
        d->dtr = value;
        sendModemSignal( value ? 'D' : 'd' );
    }
}

/*!
    \reimp
*/
bool QSerialSocket::dsr() const
{
    return d->dsr;
}

/*!
    \reimp
*/
bool QSerialSocket::carrier() const
{
    return d->incomingCarrier;
}

/*!
    \reimp
*/
bool QSerialSocket::setCarrier( bool value )
{
    if ( d->outgoingCarrier != value ) {
        d->outgoingCarrier = value;
        sendModemSignal( value ? 'C' : 'c' );
    }
    return true;
}

/*!
    \reimp
*/
bool QSerialSocket::rts() const
{
    return d->rts;
}

/*!
    \reimp
*/
void QSerialSocket::setRts( bool value )
{
    if ( d->rts != value ) {
        d->rts = value;
        sendModemSignal( value ? 'R' : 'r' );
    }
}

/*!
    \reimp
*/
bool QSerialSocket::cts() const
{
    return d->cts;
}

/*!
    \reimp
*/
void QSerialSocket::discard()
{
    // Nothing to do here.
}

/*!
    \reimp
*/
bool QSerialSocket::isValid() const
{
    return d->valid;
}

/*!
    \fn void QSerialSocket::closed()

    Signal that is emitted when the socket is closed by
    the other end of the connection.
*/

/*!
    \reimp
*/
qint64 QSerialSocket::readData( char *data, qint64 maxlen )
{
    if ( d->valid ) {
        int retlen;
        if ( maxlen > ( d->bufLen - d->bufPosn ) ) {
            retlen = d->bufLen - d->bufPosn;
        } else {
            retlen = (int)maxlen;
        }
        memcpy( data, d->buffer + d->bufPosn, retlen );
        d->bufPosn += retlen;
        return retlen;
    } else {
        return 0;
    }
}

/*!
    \reimp
*/
qint64 QSerialSocket::writeData( const char *data, qint64 len )
{
    if ( d->valid ) {
        const char *s = (const char *)::memchr( data, (char)0xFF, (int)len );
        if ( !s )
            return d->socket->write( data, len );
        qint64 written = 0;
        qint64 towrite, temp;
        do {
            if ( s != data ) {
                // Write all data up to the next 0xFF.
                towrite = (qint64)( s - data );
                temp = d->socket->write( data, towrite );
                if ( temp < towrite ) {
                    if ( temp > 0 )
                        written += temp;
                    break;
                }
                ++written;
                data = s;
                len -= temp;
            } else {
                // Escape 0xFF for other side's telnet decoding.
                temp = d->socket->write( "\377\377", 2 );
                if ( temp != 2 )
                    break;
                ++written;
                ++data;
                --len;
                s = (const char *)::memchr( data, (char)0xFF, (int)len );
                if ( !s )
                    s = data + (int)len;
            }
        } while ( len > 0 );
        return written;
    } else {
        return len;
    }
}

void QSerialSocket::socketReadyRead()
{
    // Read data into our internal buffer.
    if ( d->bufPosn > 0 && d->bufPosn < d->bufLen ) {
        memmove( d->buffer, d->buffer + d->bufPosn, d->bufLen - d->bufPosn );
        d->bufLen -= d->bufPosn;
        d->bufPosn = 0;
    } else if ( d->bufPosn > 0 && d->bufPosn == d->bufLen ) {
        d->bufPosn = 0;
        d->bufLen = 0;
    }
    if ( d->bufLen >= (int)sizeof( d->buffer ) ) {
        // Buffer is full!  Notify the higher layers to read it and then exit.
        internalReadyRead();
        return;
    }
    int size = d->socket->read
        ( d->buffer + d->bufLen, sizeof( d->buffer ) - d->bufLen );
    if ( size <= 0 )
        return;
    d->bufLen += size;

    // Search for telnet options within the data stream and extract them.
    int index = d->bufPosn;
    int newIndex = index;
    while ( index < d->bufLen ) {
        char ch = d->buffer[index++];
        if ( d->optionsMode == 0 ) {
            if ( ch == (char)255 ) {
                // Start of a telnet option.
                d->options[(d->optionsLen)++] = ch;
                d->optionsMode = 1;
            } else {
                // Ordinary character.
                d->buffer[newIndex++] = ch;
            }
        } else if ( d->optionsMode == 1 ) {
            if ( ch == (char)255 ) {
                // Escaped 0xFF character.
                d->buffer[newIndex++] = ch;
                d->optionsMode = 0;
            } else if ( ch == (char)251 || ch == (char)252 ||
                        ch == (char)253 || ch == (char)254 ) {
                // WILL, WONT, DO, DONT option.
                d->options[(d->optionsLen)++] = ch;
                d->optionsMode = 2;
            } else if ( ch == (char)250 ) {
                // SB option.
                d->options[(d->optionsLen)++] = ch;
                d->optionsMode = 3;
            } else {
                // Don't know, so ignore it.
                d->optionsMode = 0;
            }
        } else if ( d->optionsMode == 2 ) {
            // WILL, WONT, DO, DONT option code.  Remove the 0xFF and type
            // and then dispatch the command.
            d->optionsLen -= 2;
            char type = d->options[d->optionsLen + 1];
            if ( type == (char)251 )
                receiveWill( ch & 0xFF );
            else if ( type == (char)252 )
                receiveWont( ch & 0xFF );
            else if ( type == (char)253 )
                receiveDo( ch & 0xFF );
            else
                receiveDont( ch & 0xFF );
            d->optionsMode = 0;
        } else if ( d->optionsMode == 3 ) {
            // Contents of an SB option.
            if ( ch == (char)255 ) {
                d->optionsMode = 4;
            } else if ( d->optionsLen < (int)sizeof( d->options ) ) {
                d->options[(d->optionsLen)++] = ch;
            }
        } else if ( d->optionsMode == 4 ) {
            // Encountered 0xFF within an SB block.
            if ( ch == (char)240 ) {
                // This is the SE at the end of the block.
                receiveSubOption( d->options[2] & 0xFF, d->options + 3,
                                  d->optionsLen - 3 );
                d->optionsMode = 0;
                d->optionsLen = 0;
            } else {
                // Normal escaped character within an SB block.
                d->options[(d->optionsLen)++] = ch;
                d->optionsMode = 3;
            }
        }
    }
    d->bufLen = newIndex;

    // Notify higher layers about the new data.
    internalReadyRead();
}

void QSerialSocket::socketClosed()
{
    d->valid = false;
    emit closed();
}

void QSerialSocket::init()
{
    // Connect up the interesting signals.
    connect( d->socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()) );
    connect( d->socket, SIGNAL(disconnected()), this, SLOT(socketClosed()) );
    connect( d->socket, SIGNAL(error(QAbstractSocket::SocketError)),
             this, SLOT(socketClosed()) );
}

void QSerialSocket::sendModemSignal( int ch )
{
    if ( d->valid && d->canUseSignals ) {
        char sig[1];
        sig[0] = (char)ch;
        sendSubOption( TO_MODEM_SIGNALS, sig, 1 );
    }
}

void QSerialSocket::sendCommand( const char *buf, int len )
{
    if ( d->valid )
        d->socket->write( buf, len );
}

// Send a telnet "DO" command.
void QSerialSocket::sendDo( int option )
{
    char cmd[3];
    cmd[0] = (char)255;         // IAC
    cmd[1] = (char)253;         // DO
    cmd[2] = (char)option;
    sendCommand( cmd, 3 );
}

void QSerialSocket::sendDont( int option )
{
    char cmd[3];
    cmd[0] = (char)255;         // IAC
    cmd[1] = (char)254;         // DO
    cmd[2] = (char)option;
    sendCommand( cmd, 3 );
}

void QSerialSocket::sendWill( int option )
{
    char cmd[3];
    cmd[0] = (char)255;         // IAC
    cmd[1] = (char)251;         // WILL
    cmd[2] = (char)option;
    sendCommand( cmd, 3 );
}

void QSerialSocket::sendWont( int option )
{
    char cmd[3];
    cmd[0] = (char)255;         // IAC
    cmd[1] = (char)252;         // WONT
    cmd[2] = (char)option;
    sendCommand( cmd, 3 );
}

void QSerialSocket::sendSubOption( int option, const char *buf, int len )
{
    char *block = new char [ len + 5 ];
    block[0] = (char)255;       // IAC
    block[1] = (char)250;       // SB
    block[2] = (char)option;
    memcpy( block + 3, buf, len );
    block[len + 3] = (char)255; // IAC
    block[len + 4] = (char)240; // SE
    sendCommand( block, len + 5 );
    delete block;
}

void QSerialSocket::initTelnet()
{
    // Send enough options to pretend to be a telnetd implementation
    // to a regular Unix-like telnet client.
    sendDo( TO_ECHO );
    sendDo( TO_LINEMODE );
    sendDo( TO_NAWS );
    sendWill( TO_STATUS );
    sendDo( TO_LFLOW );
    sendWill( TO_ECHO );

    // Negotiate our extension for modem signals when we are talking
    // to another instance of QSerialSocket on the other end.
    sendWill( TO_MODEM_SIGNALS );
}

void QSerialSocket::receiveModemSignal( int ch )
{
    switch ( ch ) {

        case 'D':
        {
            d->dsr = true;
            emit dsrChanged( d->dsr );
        }
        break;

        case 'd':
        {
            d->dsr = false;
            emit dsrChanged( d->dsr );
        }
        break;

        case 'C':
        {
            d->incomingCarrier = true;
            emit carrierChanged( d->incomingCarrier );
        }
        break;

        case 'c':
        {
            d->incomingCarrier = false;
            emit carrierChanged( d->incomingCarrier );
        }
        break;

        case 'R':
        {
            d->cts = true;
            emit ctsChanged( d->cts );
        }
        break;

        case 'r':
        {
            d->cts = false;
            emit ctsChanged( d->cts );
        }
        break;
    }
}

void QSerialSocket::receiveDo( int option )
{
    switch ( (TelnetOption)option ) {

        case TO_MODEM_SIGNALS:
        {
            // We are talking to something capable of modem signals.
            if ( !d->canUseSignals ) {
                sendWill( option );
                d->canUseSignals = true;
            }
        }
        break;

        default: break;
    }
}

void QSerialSocket::receiveDont( int option )
{
    // Nothing to do here yet.
    Q_UNUSED(option);
}

void QSerialSocket::receiveWill( int option )
{
    switch ( (TelnetOption)option ) {

        case TO_MODEM_SIGNALS:
        {
            // We are talking to something that will send us modem signals,
            // so we can now send them in reply.
            d->canUseSignals = true;
        }
        break;

        default: break;
    }
}

void QSerialSocket::receiveWont( int option )
{
    // Nothing to do here yet.
    Q_UNUSED(option);
}

void QSerialSocket::receiveSubOption( int option, const char *buf, int len )
{
    if ( option != TO_MODEM_SIGNALS || len < 1 )
        return;
    receiveModemSignal( buf[0] );
}

/*!
    \class QSerialSocketServer
    \inpublicgroup QtBaseModule

    \brief The QSerialSocketServer class provides a server to handle incoming serial-over-sockets connections.
    \ingroup io
    \ingroup ipc

    As each connection arrives, the incoming() signal is emitted with a QSerialSocket
    instance as its argument.

    \sa QSerialSocket
*/

/*!
    Construct a new serial-over-sockets server, bound to \a port,
    and attach it to \a parent.  If \a localHostOnly is true
    (the default), then the socket will only be accessible from
    applications on the same host.
*/
QSerialSocketServer::QSerialSocketServer
        ( quint16 port, bool localHostOnly, QObject *parent )
    : QObject( parent )
{
    server = new QTcpServer( this );
    connect( server, SIGNAL(newConnection()), this, SLOT(newConnection()) );
    if ( localHostOnly )
        server->listen( QHostAddress::LocalHost, port );
    else
        server->listen( QHostAddress::Any, port );
}

/*!
    Destroy this serial-over-sockets server.
*/
QSerialSocketServer::~QSerialSocketServer()
{
}

/*!
    Returns true if this server was able to listen on the specified
    port number when it was constructed; otherwise returns false.
*/
bool QSerialSocketServer::isListening() const
{
    return server->isListening();
}

/*!
    Returns the port number that this serial-over-sockets server is bound to.
*/
quint16 QSerialSocketServer::port() const
{
    return server->serverPort();
}

/*!
    \fn void QSerialSocketServer::incoming( QSerialSocket *socket )

    Signal that is emitted when an incoming connection is detected.
    The session is represented by \a socket.
*/

void QSerialSocketServer::newConnection()
{
    while ( server->hasPendingConnections() ) {
        QSerialSocket *socket
            = new QSerialSocket( server->nextPendingConnection() );
        socket->open( QIODevice::ReadWrite );
        socket->initTelnet();
        emit incoming( socket );
    }
}
