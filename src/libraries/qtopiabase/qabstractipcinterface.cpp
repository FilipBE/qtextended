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

#include <qabstractipcinterface.h>
#include <qvaluespace.h>
#include <QString>
#include <qtimer.h>
#include <qmetaobject.h>

/*!
    \class QAbstractIpcInterface
    \inpublicgroup QtBaseModule

    \brief The QAbstractIpcInterface class provides facilities for implementing the client and server sides of IPC communications paths.
    \ingroup ipc

    This class makes it easier to implement IPC mechanisms using Qtopia.
    It is used extensively by the Qt Extended Telephony, Bluetooth, Hardware Accessory,
    and Multimedia libraries.

    Subclasses that inherit this class implement the client side of the
    communication.  Subclasses of the client implement the server side,
    overriding the client functionality with server code.

    As an example, we will show how to implement a client/server "echo"
    interface.  The client side has a \c{send()} slot which sends a
    string to the server.  The server responds by emitting the \c{receive()}
    signal with the same string.  The signal is delivered back to the client
    and emitted as \c{receive()}.  First, the client class declaration:

    \code
    class QEcho : public QAbstractIpcInterface
    {
        Q_OBJECT
    public:
        QEcho( const QString& group = QString(),
               QObject *parent = 0, QAbstractIpcInterface::Mode mode = Client );

    public slots:
        virtual void send( const QString& msg );

    signals:
        void receive( const QString& msg );
    };
    \endcode

    This declares the basic interface that all client applications will see.
    A client application can instantiate this class and use it like a regular
    QObject instance:

    \code
    QEcho *echo = new QEcho();
    connect( echo, SIGNAL(receive(QString)),
             this, SLOT(myReceive(QString)) );
    echo->send( "foo" );
    \endcode

    In this example, we have left the group name null.  This tells
    QAbstractIpcInterface to look up the default implementation of the
    \c{QEcho} interface.

    If there is more than one group implementing the echo functionality,
    and the default is not suitable, the caller can supply an explicit
    group name to the \c{QEcho} constructor.  Or the
    QAbstractIpcInterfaceGroupManager class can be used to look up a
    list of all groups that implement a particular interface.

    As indicated above, the \c{QEcho} class is the client implementation
    of the functionality.  We need to supply some extra code to complete
    the client side:

    \code
    QEcho::QEcho( const QString& group, QObject *parent,
                  QAbstractIpcInterface::Mode mode )
        : QAbstractIpcInterface( "/EchoInterfaces", "QEcho",
                                 group, parent, mode )
    {
        proxy( SIGNAL(receive(QString)) );
        proxy( SLOT(send(QString)) );
    }

    void QEcho::send( const QString& msg )
    {
        invoke( SLOT(send(QString)), qVariantFromValue( msg ) );
    }
    \endcode

    The constructor calls \c{proxy()} for every signal and slot that it
    wishes to have delivered between the client and server.  It can also
    call proxyAll() to proxy all signals and public slots.  The
    \c{/EchoInterfaces} parameter specifies the location in the value
    space to place information about the interface.

    The implementation of the \c{send()} slot uses \c{invoke()} to
    pass the request on to the server.

    Now we turn our attention to the server side.  The server class,
    \c{QEchoServer}, inherits from \c{QEcho} and overrides the
    \c{send()} slot to implement the server functionality.

    \code
    class QEchoServer : public QEcho
    {
        Q_OBJECT
    public:
        QEchoServer( QObject *parent ) : QEcho( "Echo", parent, Server ) {}

    public slots:
        void send( const QString& msg ) { emit receive( msg ); }
    }
    \endcode

    We can also add another group implementing the echo functionality,
    which echos the value twice for every send:

    \code
    class QEchoTwiceServer : public QEcho
    {
        Q_OBJECT
    public:
        QEchoTwiceServer( QObject *parent )
            : QEcho( "EchoTwice", parent, Server )
            { setPriority(1); }

    public slots:
        void send( const QString& msg ) { emit receive( msg + msg ); }
    }
    \endcode

    Now that we have two group implementations, the client must choose
    between them.  It can either supply the group name explicitly
    or it can let the system choose.  The system will choose the group
    with the highest priority value.  If there is more than one group
    with the same priority, the one selected is undefined.

    Here, we have set the priority of \c{QEchoTwiceServer} to 1, which
    will give it higher priority than \c{QEchoServer} which has the
    default priority of 0.  Priorities can change over the lifetime of a
    server.  For example, a VoIP group might have a higher priority
    than a GSM group when the phone is in range of a WiFi access point,
    but a lower priority when WiFi is not available.  This way, the user
    gets the best group based on their current network coverage.

    \sa QAbstractIpcInterfaceGroup, QAbstractIpcInterfaceGroupManager
    \sa QCommInterface, QHardwareInterface
    \ingroup ipc
*/

/*!
    \enum QAbstractIpcInterface::Mode
    Defines the client/server mode for an interface object.

    \value Client Object is operating in client mode.
    \value Server Object is operating in server mode.
    \value Invalid Object is invalid.  This indicates that the connection to
           the server could not be established, or it has been lost.
*/

class QAbstractIpcInterfacePrivate
{
public:
    QAbstractIpcInterfacePrivate( QAbstractIpcInterface::Mode mode )
    {
        this->mode = mode;
        this->send = 0;
        this->receive = 0;
        this->settings = 0;
        this->lastProxy = QAbstractIpcInterface::staticMetaObject.methodCount();
        this->inConnectNotify = false;
    }
    ~QAbstractIpcInterfacePrivate()
    {
    }

    QString groupName;
    QString interfaceName;
    QString realInterfaceName;
    QString primaryInterfaceName;
    QString valueSpaceLocation;
    QAbstractIpcInterface::Mode mode;
    QtopiaIpcAdaptor *send;
    QtopiaIpcAdaptor *receive;
    QValueSpaceObject *settings;
    QString path;
    int lastProxy;
    bool inConnectNotify;
};

/*!
    Construct a new interface object for the interface called
    \a interfaceName on \a groupName and attach it to \a parent.
    If \a mode is \c Server, then the object is constructed in server
    mode and \a groupName must not be empty.  If \a mode is \c Client,
    then the object is constructed in client mode and \a groupName may
    be empty to indicate the default group that implements this
    type of interface.

    The \a valueSpaceLocation parameter specifies the location the
    value space to place all information about this interface.
    Subclasses such as QCommInterface will set this to a particular
    value meeting their requirements.
*/
QAbstractIpcInterface::QAbstractIpcInterface
        ( const QString& valueSpaceLocation, const QString& interfaceName,
          const QString& groupName, QObject *parent,
          QAbstractIpcInterface::Mode mode )
    : QObject( parent)
{
    QString group = groupName;
    QString requestChannel;
    QString responseChannel;
    QString valuePath;

    // Create the private data holder.
    d = new QAbstractIpcInterfacePrivate( mode );
    d->interfaceName = interfaceName;
    d->realInterfaceName = interfaceName;
    d->valueSpaceLocation = valueSpaceLocation;
    while ( d->valueSpaceLocation.endsWith("/") )
        d->valueSpaceLocation.chop(1);
    if ( !d->valueSpaceLocation.startsWith( "/" ) )
        d->valueSpaceLocation.prepend( "/" );

    // Determine the path to use for settings and published values.
    d->path = d->valueSpaceLocation + "/" + interfaceName + "/" + group;

    // Determine the name of the QCop channels to use for this interface.
    requestChannel = "QPE" + d->valueSpaceLocation + "/" + interfaceName +
                     "/Request/" + group;
    responseChannel = "QPE" + d->valueSpaceLocation + "/" + interfaceName +
                      "/Response/" + group;

    // Set up the QtopiaIpcAdaptor logic in server or client mode.
    if ( mode == Server ) {
        // Server side: register the object with QtopiaIpcAdaptor
        // and QValueSpace.
        d->send = new QtopiaIpcAdaptor( responseChannel, this );
        d->receive = new QtopiaIpcAdaptor( requestChannel, this );
        d->settings = new QValueSpaceObject( d->path, this );
        QString channelPath = d->valueSpaceLocation + "/_channels/" +
                              interfaceName + "/" + group;
        QValueSpaceObject *channelObject = new QValueSpaceObject( channelPath, this );
        channelObject->setAttribute
            ( QString(".requestChannel"), requestChannel );
        channelObject->setAttribute
            ( QString(".responseChannel"), responseChannel );

    } else {
        // Client side: look up the channel to connect to.
        QValueSpaceItem item( d->valueSpaceLocation + "/_channels/" + interfaceName );
        if ( group.isEmpty() ) {
            // Search for the default group implementing this interface.
            QStringList values = item.subPaths();
            QStringList::ConstIterator iter;
            int bestPriority = -2147483647;
            for ( iter = values.begin(); iter != values.end(); ++iter ) {
                QVariant value = item.value( *iter + "/priority" );
                int priority = ( value.isNull() ? 0 : value.toInt() );
                if ( priority > bestPriority ) {
                    group = QString( *iter );
                    requestChannel =
                        item.value( *iter + "/.requestChannel" ).toString();
                    responseChannel =
                        item.value( *iter + "/.responseChannel" ).toString();
                    valuePath =
                        item.value( *iter + "/.valuePath" ).toString();
                    bestPriority = priority;
                }
            }
            if ( group.isEmpty() ) {
                // Could not locate an appropriate default.
                QTimer::singleShot( 0, this, SIGNAL(disconnected()) );
                d->mode = Invalid;
                return;
            }
        } else {
            // Use the specified group name to look up the channel.
            requestChannel = item.value( group + "/.requestChannel" ).toString();
            responseChannel =
                item.value( group + "/.responseChannel" ).toString();
            valuePath = item.value( group + "/.valuePath" ).toString();
            if ( requestChannel.isEmpty() ) {
                QTimer::singleShot( 0, this, SIGNAL(disconnected()) );
                d->mode = Invalid;
                return;
            }
        }
        d->send = new QtopiaIpcAdaptor( requestChannel, this );
        d->receive = new QtopiaIpcAdaptor( responseChannel, this );
        if ( valuePath.isEmpty() ) {
            d->path = d->valueSpaceLocation + "/" + interfaceName +
                      "/" + group;
        } else {
            d->path = valuePath;
        }
    }
    d->groupName = group;
}

/*!
    Destroy this interface object.  If the object is operating in
    client mode, then the connection to the server will be severed.
    If the object is operating in server mode, then it will be
    deregistered and all communicating clients will receive the
    disconnected() signal.
*/
QAbstractIpcInterface::~QAbstractIpcInterface()
{
    if ( d->mode == Server ) {
        d->send->send( MESSAGE(ifaceServerDisconnect()) );
    }
    delete d;
}

/*!
    Returns the name of the group associated with this interface object.

    \sa interfaceName()
*/
QString QAbstractIpcInterface::groupName() const
{
    return d->groupName;
}

/*!
    Returns the interface name associated with this interface object.
    If this object is known by more than one interface name, the interface
    name passed in the last call to proxyAll() will be returned.

    \sa groupName()
*/
QString QAbstractIpcInterface::interfaceName() const
{
    return d->realInterfaceName;
}

/*!
    Returns the mode that this interface object is operating in.
    Returns QAbstractIpcInterface::Client for client mode,
    QAbstractIpcInterface::Server for server mode, and
    QAbstractIpcInterface::Invalid if the client's connection to the
    server could not be established or has been lost.

    \sa available()
*/
QAbstractIpcInterface::Mode QAbstractIpcInterface::mode() const
{
    return d->mode;
}

/*!
    \fn bool QAbstractIpcInterface::available() const

    Returns true if the server is available for use; otherwise returns false.  This is normally
    used by a client just after the constructor is called to determine
    if the constructor could locate a suitable server.

    \sa mode()
*/

/*!
    Sets the priority of this server to \a value.  Ignored on the client.
*/
void QAbstractIpcInterface::setPriority( int value )
{
    if ( d->settings ) {
        d->settings->setAttribute( QString("priority"), value );
        QValueSpaceObject::sync();
    }
}

/*!
    Proxies \a member so that it will be delivered between the client and server.
    This is typically called from the constructor of the immediate subclass
    of QAbstractIpcInterface for all signals and slots that will cross the
    client/server boundary.

    This method is useful when the subclass has only a few signals and slots
    that need to be proxied.  Or it contains signals and slots that must not
    be proxied because they are private to the client or server sides.
    The related proxyAll() method is simpler to use if the subclass has
    many signals and slots and they must all be proxied.

    \sa proxyAll()
*/
void QAbstractIpcInterface::proxy( const QByteArray& member )
{
    if ( d->mode == Server ) {
        if ( member.size() > 0 && member[0] == '1' ) {
            // Proxying a slot from the channel.
            QtopiaIpcAdaptor::connect( d->receive, "3" + member.mid(1),
                                 this, member );
        } else if ( member.size() > 0 && member[0] == '2' ) {
            // Proxying a signal to the channel.
            QtopiaIpcAdaptor::connect( this, member, d->send, "3" + member.mid(1) );
        }
    }
}

/*!
    Sets up remote invocation proxies for all signals and public slots
    on this object, starting at the class specified by \a meta.

    Normally \a meta will be the \c staticMetaObject value for the
    immediate subclass of QAbstractIpcInterface.  It allows the proxying
    to be limited to a subset of the class hierarchy if more derived
    classes have signals and slots that should not be proxied.

    This method is useful when the subclass has many signals
    and slots, and it would be error-prone to proxy them individually
    with proxy().

    \sa proxy()
*/
void QAbstractIpcInterface::proxyAll( const QMetaObject& meta )
{
    // Only needed for the server at present.
    if ( d->mode != Server )
        return;

    // Find the starting point within the metaobject tree.
    const QMetaObject *current = metaObject();
    while ( current != 0 && current != &meta ) {
        current = current->superClass();
    }
    if ( ! current )
        return;

    // If we have been called multiple times, then only proxy
    // the ones we haven't done yet.
    int last = current->methodCount();
    if ( last > d->lastProxy ) {
        int index = d->lastProxy;
        d->lastProxy = last;
        for ( ; index < last; ++index ) {

            QMetaMethod method = current->method( index );
            if ( method.methodType() == QMetaMethod::Slot &&
                 method.access() == QMetaMethod::Public ) {
                QByteArray name = method.signature();
                QtopiaIpcAdaptor::connect( d->receive, "3" + name, this, "1" + name );
            } else if ( method.methodType() == QMetaMethod::Signal ) {
                QByteArray name = method.signature();
                QtopiaIpcAdaptor::connect( this, "2" + name, d->send, "3" + name );
            }
        }
    }
}

/*!
    Sets up remote invocation proxies for all signals and public slots
    on this object, starting at the class specified by \a meta.
    Also register \a subInterfaceName as a sub-interface name for
    this object's primary interface name.  After this call,
    interfaceName() will return \a subInterfaceName.
*/
void QAbstractIpcInterface::proxyAll
        ( const QMetaObject& meta, const QString& subInterfaceName )
{
    proxyAll( meta );
    d->realInterfaceName = subInterfaceName;
    if ( d->mode == Server ) {

        // Add extra information under "/Communications/Interfaces"
        // to redirect clients to the original interface channels.
        QString path = d->valueSpaceLocation + "/_channels/" + subInterfaceName + "/" +
                       d->groupName;
        QString requestChannel = "QPE" + d->valueSpaceLocation + "/" +
                                 d->interfaceName + "/Request/" + d->groupName;
        QString responseChannel = "QPE" + d->valueSpaceLocation + "/" +
                                 d->interfaceName + "/Response/" + d->groupName;
        QValueSpaceObject *values = new QValueSpaceObject( path, this );
        values->setAttribute( QString(".requestChannel"), requestChannel );
        values->setAttribute( QString(".responseChannel"), responseChannel );
        values->setAttribute( QString(".valuePath"), d->path );

    } else if ( d->mode == Client ) {

        // Verify that there is a registration for the new interface name
        // that points back to the primary interface name.
        QValueSpaceItem item( d->valueSpaceLocation + "/_channels/" +
                              subInterfaceName + "/" + d->groupName );
        QString path = item.value( ".valuePath" ).toString();
        if ( path != d->path ) {
            qWarning( "QAbstractIpcInterface: %s is not a sub-interface of %s",
                      subInterfaceName.toLatin1().constData(),
                      d->interfaceName.toLatin1().constData() );
            QTimer::singleShot( 0, this, SIGNAL(disconnected()) );
            d->mode = Invalid;
        }

    }
}

/*!
    Performs a call to the zero-argument slot \a name from the client to
    the server.  Ignored on the server.
*/
QtopiaIpcSendEnvelope QAbstractIpcInterface::invoke( const QByteArray& name )
{
    if ( d->mode == Client )
        return d->send->send( name );
    else
        return QtopiaIpcSendEnvelope();
}

/*!
    Performs a call to the one-argument slot \a name from the client to
    the server, with the argument \a arg1.  Ignored on the server.
*/
void QAbstractIpcInterface::invoke( const QByteArray& name, QVariant arg1 )
{
    if ( d->mode == Client )
        d->send->send( name, arg1 );
}

/*!
    Performs a call to the two-argument slot \a name from the client to
    the server, with the arguments \a arg1 and \a arg2.  Ignored on the server.
*/
void QAbstractIpcInterface::invoke( const QByteArray& name, QVariant arg1, QVariant arg2 )
{
    if ( d->mode == Client )
        d->send->send( name, arg1, arg2 );
}

/*!
    Performs a call to the three-argument slot \a name from the client to
    the server, with the arguments \a arg1, \a arg2, and \a arg3.
    Ignored on the server.
*/
void QAbstractIpcInterface::invoke( const QByteArray& name, QVariant arg1, QVariant arg2, QVariant arg3 )
{
    if ( d->mode == Client )
        d->send->send( name, arg1, arg2, arg3 );
}

/*!
    Performs a call to the multi-argument slot \a name from the client to
    the server, with the arguments in the list \a args.  Ignored on the server.
*/
void QAbstractIpcInterface::invoke( const QByteArray& name, const QList<QVariant>& args )
{
    if ( d->mode == Client )
        d->send->send( name, args );
}

/*!
    \enum QAbstractIpcInterface::SyncType
    Defines whether interface values should be published by QAbstractIpcInterface::setValue() immediately or not.

    \value Immediate Synchronize values with the client immediately.
    \value Delayed Delay synchronization until the server re-enters
           the Qt event loop.
*/

/*!
    Sets \a name to \a value in the auxiliary value space for
    this interface object.  This is called by the server to notify
    clients of a change in \a name.  Ignored on the client.

    If \a sync is \c Delayed, then delay publication of the value to
    clients until the server re-enters the Qt event loop.  This may
    be more efficient if the server needs to set or remove several values
    at once.  The default value for \a sync is \c Immediate.

    \sa value(), removeValue()
*/
void QAbstractIpcInterface::setValue
        ( const QString& name, const QVariant& value,
          QAbstractIpcInterface::SyncType sync )
{
    if ( d->settings ) {
        d->settings->setAttribute( name, value );
        if ( sync == Immediate )
            QValueSpaceObject::sync();
    }
}

/*!
    Returns the value associated with \a name in the auxiliary value space
    for this interface object.  Returns \a def if the name does not exist.
    This can be called on either the client or the server (but usually
    the client).

    \sa setValue(), removeValue()
*/
QVariant QAbstractIpcInterface::value
            ( const QString& name, const QVariant& def ) const
{
    QValueSpaceItem item( d->path );
    return item.value( name, def );
}

/*!
    Removes the value associated with \a name from the auxiliary value space.

    If \a sync is \c Delayed, then delay publication of the value to
    clients until the server re-enters the Qt event loop.  This may
    be more efficient if the server needs to set or remove several values
    at once.  The default value for \a sync is \c Immediate.

    \sa value(), setValue()
*/
void QAbstractIpcInterface::removeValue
        ( const QString& name, QAbstractIpcInterface::SyncType sync)
{
    if ( d->settings ) {
        d->settings->removeAttribute( name );
        if ( sync == Immediate )
            QValueSpaceObject::sync();
    }
}

/*!
    Returns a list of all value names in the auxiliary value space
    for this interface object contained within \a path.

    \sa value()
*/
QList<QString> QAbstractIpcInterface::valueNames(
    const QString& path ) const
{
    QList<QString> values;
    if ( path.isEmpty())
        values = QValueSpaceItem( d->path ).subPaths();
    else
        values = QValueSpaceItem( d->path + "/" + path ).subPaths();
    for(QList<QString>::iterator iter = values.begin(); iter != values.end();) {
        if(iter->at(0) == QChar('.'))
            iter = values.erase(iter);
        else
            ++iter;
    }

    return values;
}

/*!
    Initializes server-side functionality within this interface after
    all interfaces in the group have been created.  This is called by
    QAbstractIpcInterfaceGroup::initialize() after all interface
    handling objects on \a group have been created.  It will not be
    called if the interface is not associated with a QAbstractIpcInterfaceGroup
    instance.

    Subclasses typically override this method so that they can connect
    to signals on other interfaces which may not have existed when the
    interface constructor was called.

    \sa QAbstractIpcInterfaceGroup::initialize()
*/
void QAbstractIpcInterface::groupInitialized( QAbstractIpcInterfaceGroup * )
{
    // Nothing to do here.
}

/*!
    \internal
*/
void QAbstractIpcInterface::connectNotify( const char *signal )
{
    // This function will be re-entered when QtopiaIpcAdaptor::connect
    // is called.
    if ( d->inConnectNotify ) {
        QObject::connectNotify( signal );
        return;
    }
    d->inConnectNotify = true;

    // Deal with the connection details.
    if ( d->mode == Client ) {
        // If the message has not been connected to the signal yet,
        // then do it now.  We try to do this only on demand so that
        // client objects that don't need information from the server
        // via a signal won't incur the QtopiaChannel overhead.
        if ( QLatin1String( signal ) == SIGNAL(disconnected()) ) {
            if ( !d->receive->isConnected
                        ( MESSAGE(ifaceServerDisconnect()) ) ) {
                QtopiaIpcAdaptor::connect
                    ( d->receive, MESSAGE(ifaceServerDisconnect()),
                      this, signal );
            }
        } else if ( !d->receive->isConnected( signal ) ) {
            QtopiaIpcAdaptor::connect
                ( d->receive, "3" + QByteArray(signal + 1), this, signal );
        }
    }

    // Safe to re-enter once more.
    d->inConnectNotify = false;

    // Pass control to the base implementation.
    QObject::connectNotify( signal );
}

/*!
    \fn void QAbstractIpcInterface::disconnected()

    Signal that is emitted on the client if the server disconnects
    before the client object is destroyed, or if the client could
    not connect at all because the server does not exist.  Not
    used on the server.

    The mode() of the interface object will be \c Invalid when
    this signal is emitted.

    \sa mode()
*/

void QAbstractIpcInterface::remoteDisconnected()
{
    if ( d->send ) {
        d->send->deleteLater();
        d->send = 0;
        d->receive->deleteLater();
        d->receive = 0;
        d->mode = Invalid;
        emit disconnected();
    }
}
