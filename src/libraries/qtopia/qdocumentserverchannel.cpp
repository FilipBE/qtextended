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
#include "qdocumentserverchannel_p.h"
#include <qtopianamespace.h>
#include <QtopiaApplication>

#include <QTimer>
#include <QtDebug>
#include <QMetaObject>
#include <QMetaMethod>
#include <QTime>

#ifndef QT_NO_SXE
#define QSXE_HEADER_LEN 24
#endif

Q_IMPLEMENT_USER_METATYPE_ENUM(QDocumentServerMessage::MessageType);

class QDocumentServerMessagePrivate : public QSharedData
{
public:
    QDocumentServerMessage::MessageType type;
    int transactionId;
    QByteArray signature;
    QVariantList arguments;
    QList< QUnixSocketRights > rights;
};

QDocumentServerMessage::QDocumentServerMessage()
    : d( 0 )
{
}

QDocumentServerMessage::QDocumentServerMessage( const QDocumentServerMessage &other )
    : d( other.d )
{
}

QDocumentServerMessage &QDocumentServerMessage::operator =( const QDocumentServerMessage &other )
{
    d = other.d;

    return *this;
}

QDocumentServerMessage::~QDocumentServerMessage()
{
}

QDocumentServerMessage QDocumentServerMessage::createMethodCall( const QByteArray &signature )
{
    static int nextTransactionId = 0;

    QDocumentServerMessage message;

    message.d = new QDocumentServerMessagePrivate;

    message.d->type = QDocumentServerMessage::MethodCallMessage;
    message.d->transactionId = nextTransactionId++;
    message.d->signature = signature;

    return message;
}

QDocumentServerMessage QDocumentServerMessage::createSignal( const QByteArray &signature )
{
    QDocumentServerMessage message;

    message.d = new QDocumentServerMessagePrivate;

    message.d->type = QDocumentServerMessage::SignalMessage;
    message.d->transactionId = 0;
    message.d->signature = signature;

    return message;
}

QDocumentServerMessage QDocumentServerMessage::createSlot( const QByteArray &signature )
{
    QDocumentServerMessage message;

    message.d = new QDocumentServerMessagePrivate;

    message.d->type = QDocumentServerMessage::SlotMessage;
    message.d->transactionId = 0;
    message.d->signature = signature;

    return message;
}

QDocumentServerMessage QDocumentServerMessage::createError( const QString &errorString ) const
{
    QDocumentServerMessage message;

    if( d != 0 )
    {
        message.d = new QDocumentServerMessagePrivate;

        message.d->type = QDocumentServerMessage::ErrorMessage;
        message.d->transactionId = d->transactionId;
        message.d->signature = d->signature;

        message.d->arguments.append( errorString );
    }

    return message;
}


QDocumentServerMessage QDocumentServerMessage::createReply( const QVariantList &arguments, const QList< QUnixSocketRights > &rights ) const
{
    QDocumentServerMessage message;

    if( d != 0 )
    {
        message.d = new QDocumentServerMessagePrivate;

        message.d->type = QDocumentServerMessage::ReplyMessage;
        message.d->transactionId = d->transactionId;
        message.d->signature = d->signature;
        message.d->arguments = arguments;
        message.d->rights = rights;
    }

    return message;
}

QDocumentServerMessage::MessageType QDocumentServerMessage::type() const
{
    return d != 0 ? d->type : QDocumentServerMessage::InvalidMessage;
}

int QDocumentServerMessage::transactionId() const
{
    return d != 0 ? d->transactionId : 0;
}

QByteArray QDocumentServerMessage::signature() const
{
    return d != 0 ? d->signature : QByteArray();
}

QVariantList QDocumentServerMessage::arguments() const
{
    return d != 0 ? d->arguments : QVariantList();
}

void QDocumentServerMessage::setArguments( const QVariantList &arguments )
{
    if( d != 0 )
    {
        d->arguments = arguments;
    }
}

QList< QUnixSocketRights > QDocumentServerMessage::rights() const
{
    return d != 0 ? d->rights : QList< QUnixSocketRights >();
}

void QDocumentServerMessage::setRights( const QList< QUnixSocketRights > &rights )
{
    if( d != 0 )
    {
        d->rights = rights;
    }
}

QDocumentServerMessage &QDocumentServerMessage::operator <<( const QVariant &argument )
{
    if( d != 0 )
    {
        d->arguments.append( argument );
    }

    return *this;
}

QDocumentServerMessage &QDocumentServerMessage::operator <<( const QUnixSocketRights &rights )
{
    if( d != 0 )
    {
        d->rights.append( rights );
    }

    return *this;
}

QDocumentServerChannel::QDocumentServerChannel( const QByteArray &interface, QObject *parent )
    : QObject( parent )
    , m_interface( interface )
#ifndef QT_NO_SXE
    , m_prefix( QLatin1String( "DocumentServer/" ) + m_interface + QLatin1String( "::" ) )
    , m_authData( 0 )
#endif
    , m_role( Client )
    , m_messageReceived( false )
{
    static int messageMetaId = qRegisterMetaType< QDocumentServerMessage >( "QDocumentServerMessage" );

    Q_UNUSED( messageMetaId  );

    m_socket.setRightsBufferSize( 1 );

    QObject::connect( &m_socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()) );
    QObject::connect( &m_socket, SIGNAL(stateChanged(SocketState)),
                       this, SLOT(stateChanged()) );
}

QDocumentServerChannel::~QDocumentServerChannel()
{
#ifndef QT_NO_SXE
    delete m_authData;
#endif
}

bool QDocumentServerChannel::messagesAvailable() const
{
    return !m_receivedMessages.isEmpty();
}

const QDocumentServerMessage &QDocumentServerChannel::peek() const
{
    return m_receivedMessages.first();
}

QDocumentServerMessage QDocumentServerChannel::read()
{
    return m_receivedMessages.takeFirst();
}

bool QDocumentServerChannel::write( const QDocumentServerMessage &message )
{
    return m_socket.write( marshalMessage( message ) ) > 0;
}

void QDocumentServerChannel::flush()
{
    m_socket.flush();
}

bool QDocumentServerChannel::connect()
{
    QByteArray socketPath = Qtopia::tempDir().toLocal8Bit() + m_interface;

    if( m_socket.connect( socketPath ) )
    {
        m_role = Client;
#ifndef QT_NO_SXE
        m_authData = QTransportAuth::getInstance()->connectTransport(
            QTransportAuth::Trusted | QTransportAuth::UnixStreamSock,
            m_socket.socketDescriptor() );
#endif
        return true;
    }
    else
        return false;
}

bool QDocumentServerChannel::setSocketDescriptor( int socketDescriptor )
{
    if( m_socket.setSocketDescriptor( socketDescriptor ) )
    {
        m_role = Server;

#ifndef QT_NO_SXE
        m_authData = QTransportAuth::getInstance()->connectTransport(
            QTransportAuth::Trusted | QTransportAuth::UnixStreamSock,
            m_socket.socketDescriptor() );
#endif

        return true;
    }
    else
        return false;
}

void QDocumentServerChannel::disconnect()
{
    m_socket.disconnect();

    m_role = Client;

#ifndef QT_NO_SXE
    delete m_authData; // QTransportAuth keeps a pointer and there is no apparent way to get it to release it.

    m_authData = 0;
#endif
}

bool QDocumentServerChannel::isConnected() const
{
    return m_socket.state() == QUnixSocket::ConnectedState;
}

bool QDocumentServerChannel::waitForReadyRead( int msecs )
{
    return m_socket.waitForReadyRead( msecs );
}

void QDocumentServerChannel::socketReadyRead()
{
    if( !m_socket.bytesAvailable() )
        return;

    QUnixSocketMessage socketMessage = m_socket.read();

    m_messageBuffer += socketMessage.bytes();
    m_rightsBuffer += socketMessage.rights();

    while( m_messageBuffer.size() > int(sizeof( int )) * 2 )
    {
#ifndef QT_NO_SXE
        if( m_role == Server )
        {
            m_authData->processId = socketMessage.processId();

            unsigned char saveStatus = m_authData->status;
            if( !QTransportAuth::getInstance()->authFromMessage( *m_authData, m_messageBuffer, m_messageBuffer.size() ))
            {
                // not all arrived yet?  come back later
                if( (m_authData->status & QTransportAuth::ErrMask) == QTransportAuth::TooSmall )
                {
                    m_authData->status = saveStatus;
                    return;
                }
            }
            if( (m_authData->status & QTransportAuth::ErrMask) == QTransportAuth::NoMagic )
            {
                // no msg auth header, don't change the success status for connections
                if( m_authData->properties & QTransportAuth::Connection )
                    m_authData->status = saveStatus;
            }
            else
            {
                // msg auth header detected and auth determined, remove hdr
                m_messageBuffer = m_messageBuffer.mid( QSXE_HEADER_LEN );

                if( m_messageBuffer.size() < int(sizeof( int )) * 2 )
                    return;
            }
        }
#endif
        int messageSize = reinterpret_cast< const int * >( m_messageBuffer.constData() )[ 0 ] + sizeof( int ) * 2;
        int rightsCount = reinterpret_cast< const int * >( m_messageBuffer.constData() )[ 1 ];

        if( messageSize > m_messageBuffer.size() || rightsCount > m_rightsBuffer.count() )
            return;

        QByteArray messageData = m_messageBuffer.left( messageSize );
        QList< QUnixSocketRights > rights = m_rightsBuffer.mid( 0, rightsCount );

        m_messageBuffer = m_messageBuffer.mid( messageSize );
        m_rightsBuffer = m_rightsBuffer.mid( rightsCount );

        QDocumentServerMessage message  = unmarshalMessage( messageData, rights );

#ifndef QT_NO_SXE
        if( m_role != Server || QTransportAuth::getInstance()->authorizeRequest( *m_authData, m_prefix + message.signature() ) )
        {
#endif
            m_receivedMessages.append( message );

            m_messageReceived = true;
#ifndef QT_NO_SXE
        }
        else if( message.type() == QDocumentServerMessage::MethodCallMessage )
            write( message.createError( "Invalid permissions" ) );    // Send a response so the client doesn't block indefinately.
#endif                                                                // The alternative is to cut the connection
    }

    if( !m_receivedMessages.isEmpty() )
        emit readyRead();
}

void QDocumentServerChannel::stateChanged()
{
    if( m_socket.state() == QUnixSocket::UnconnectedState )
        emit disconnected();
}

QUnixSocketMessage QDocumentServerChannel::marshalMessage( const QDocumentServerMessage &message )
{
    QByteArray data;
    {
        QDataStream stream( &data, QIODevice::WriteOnly );

        stream << message.type();
        stream << message.transactionId();
        stream << message.signature();
        stream << message.arguments();
    }

    int sizes[] = { data.size(), message.rights().count() };

    data.prepend( QByteArray::fromRawData( reinterpret_cast< char * >( sizes ), sizeof( sizes ) ) );

#ifndef QT_NO_SXE
    if( m_role == Client )
    {
        char header[ QSXE_HEADER_LEN ];
        ::memset( header, 0, QSXE_HEADER_LEN );

        if( QTransportAuth::getInstance()->authToMessage( *m_authData, header, data.constData(), data.length() ) )
            data.prepend( QByteArray::fromRawData( header, QSXE_HEADER_LEN ) );
    }
#endif

    return QUnixSocketMessage( data, message.rights() );
}

QDocumentServerMessage QDocumentServerChannel::unmarshalMessage( const QByteArray &messageData, const QList< QUnixSocketRights > &rights )
{
    QDocumentServerMessage message;

    message.d = new QDocumentServerMessagePrivate;

    QDataStream stream( messageData );

    stream.skipRawData( sizeof( int ) * 2 );

    stream >> message.d->type;
    stream >> message.d->transactionId;
    stream >> message.d->signature;
    stream >> message.d->arguments;

    message.d->rights = rights;

    return message;
}

QDocumentServerHost::QDocumentServerHost( const QByteArray &interface, QObject *parent )
    : QObject( parent )
    , m_channel( interface )
{
    connect( &m_channel, SIGNAL(disconnected()), this, SIGNAL(disconnected()) );
    connect( &m_channel, SIGNAL(readyRead()), this, SLOT(handlePendingMessages()) );
}

bool QDocumentServerHost::setSocketDescriptor( int socketDescriptor )
{
    return m_channel.setSocketDescriptor( socketDescriptor );
}

void QDocumentServerHost::emitSignalWithArgumentList( const char *signal, const QVariantList &arguments, const QList< QUnixSocketRights > &rights )
{
    QDocumentServerMessage message  = QDocumentServerMessage::createSignal( signal );

    message.setArguments( arguments );
    message.setRights( rights );

    m_channel.write( message );
}

void QDocumentServerHost::invokeSlot( const QDocumentServerMessage &message )
{
    Q_UNUSED( message );
}

void QDocumentServerHost::handlePendingMessages()
{
    while( m_channel.messagesAvailable() )
    {
        switch( m_channel.peek().type() )
        {
        case QDocumentServerMessage::MethodCallMessage:
            m_channel.write( invokeMethod( m_channel.read() ) );
            m_channel.flush();
            break;
        case QDocumentServerMessage::SlotMessage:
            invokeSlot( m_channel.read() );
            break;
        case QDocumentServerMessage::SignalMessage:
        case QDocumentServerMessage::ReplyMessage:
        case QDocumentServerMessage::ErrorMessage:
        case QDocumentServerMessage::InvalidMessage:
            qWarning() << "Received invalid message" << m_channel.peek().type() << m_channel.peek().signature();
            m_channel.read();
        }
    }
}

QDocumentServerClient::QDocumentServerClient( const QByteArray &interface, QObject *parent )
    : QObject( parent )
    , m_channel( interface )
{
    QObject::connect( &m_channel, SIGNAL(disconnected()), this, SIGNAL(disconnected()) );
    QObject::connect( &m_channel, SIGNAL(readyRead()), this, SLOT(handlePendingMessages()) );
}

bool QDocumentServerClient::connect()
{
    return m_channel.connect();
}

QDocumentServerMessage QDocumentServerClient::callWithArgumentList( const char *method, const QVariantList &arguments, const QList< QUnixSocketRights > &rights )
{
    if( m_channel.isConnected() )
    {
        QDocumentServerMessage message = QDocumentServerMessage::createMethodCall( method );

        message.setArguments( arguments );
        message.setRights( rights );

        return blockingCall( message );
    }
    else
    {
        return QDocumentServerMessage();
    }
}

void QDocumentServerClient::callSlotWithArgumentList( const char *method, const QVariantList &arguments, const QList< QUnixSocketRights > &rights )
{
    if( m_channel.isConnected() )
    {
        QDocumentServerMessage message = QDocumentServerMessage::createSlot( method );

        message.setArguments( arguments );
        message.setRights( rights );

        m_channel.write( message );
    }
}

void QDocumentServerClient::invokeSignal( const QDocumentServerMessage &message )
{
    Q_UNUSED( message );
}

void QDocumentServerClient::handlePendingMessages()
{
    while( m_channel.messagesAvailable() )
    {
        switch( m_channel.peek().type() )
        {
        case QDocumentServerMessage::SignalMessage:
            QMetaObject::invokeMethod( this, "invokeSignal", Qt::QueuedConnection, Q_ARG(QDocumentServerMessage,m_channel.read()) );
            break;
        case QDocumentServerMessage::ReplyMessage:
        case QDocumentServerMessage::ErrorMessage:
            return;
        case QDocumentServerMessage::MethodCallMessage:
        case QDocumentServerMessage::SlotMessage:
        case QDocumentServerMessage::InvalidMessage:
            qWarning() << "Received invalid message" << m_channel.peek().type() << m_channel.peek().signature();
            m_channel.read();
        }
    }
}

QDocumentServerMessage QDocumentServerClient::blockingCall( const QDocumentServerMessage &message )
{
    Q_ASSERT( message.type() == QDocumentServerMessage::MethodCallMessage );

    m_channel.write( message );

    m_channel.flush();

    for( ;; )
    {
        for( int i = 0; !m_channel.waitForReadyRead(); i++ )
        {
            qWarning() << "Response time out" << message.signature();

            if( !m_channel.isConnected() )
                return message.createError( "Server disconnected" );
            else if( i == 5 )
                return message.createError( "Response timeout" );
        }

        handlePendingMessages();

        if( m_channel.messagesAvailable() )
        {
            QDocumentServerMessage reply = m_channel.read();

            Q_ASSERT( reply.type() == QDocumentServerMessage::ReplyMessage ||
                    reply.type() == QDocumentServerMessage::ErrorMessage );

            handlePendingMessages();

            if( reply.transactionId() == message.transactionId() )
                return reply;
        }
    }
}
