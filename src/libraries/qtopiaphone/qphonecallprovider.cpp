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

#include <qphonecallprovider.h>
#include <qtopiaipcadaptor.h>
#include <qvaluespace.h>
#include <quuid.h>

/*!
    \class QPhoneCallImpl
    \inpublicgroup QtTelephonyModule

    \brief The QPhoneCallImpl class provides a base class for specific phone call implementations.
    \ingroup telephony

    Phone call providers (e.g. GSM and VoIP) inherit from the
    QPhoneCallProvider class and override the create() method to
    create a QPhoneCallImpl instance that is specific to their
    requirements.

    The QPhoneCallProvider class provides the infrastructure to receive
    requests from client applications and dispatch them to the appropriate
    methods on QPhoneCallImpl.  As the calls change state, the state
    information is sent back to the client applications.

    Client applications should use QPhoneCall and QPhoneCallManager to make
    and receive phone calls.  Only the server-side providers need to use
    QPhoneCallProvider and QPhoneCallImpl.

    For AT-based modems, phone call providers should consider using
    QModemCall instead of QPhoneCallImpl.

    \sa QPhoneCallProvider, QModemCall
*/

/*!
    \enum QPhoneCallImpl::Action
    This enum defines actions that a phone call can perform in its current state.

    \value None No actions are possible.
    \value Accept Incoming call that can be accepted.
    \value Hold Connected call that can be put on hold.
    \value ActivateCall Held call that can be individually activated.
    \value ActivateGroup Held call that can be activated with its group.
    \value Join Call which can be joined to a multi-party conversation.
    \value JoinAndDetach Call which can be joined to a multi-party conversation, with the local user being detached.
    \value Tone DTMF tones are possible on this connected call.
    \value Transfer Call can be transferred to another number.
*/

class QPhoneCallImplPrivate
{
public:
    QPhoneCallProvider *provider;
    QString identifier;
    QString callType;
    QPhoneCall::State state;
    QPhoneCallImpl::Actions actions;
    bool actionsChanged;
    QString number;
    int dataPort;
};

/*!
    Creates a new phone call implementation object and attaches it to
    \a provider.  The \a identifier is a globally-unique value that
    identifies the call.  If \a identifier is empty, then this constructor
    will allocate a new identifier.

    The \a callType indicates the type of call to be constructed
    (\c Voice, \c Data, \c Fax, \c VoIP, etc).
*/
QPhoneCallImpl::QPhoneCallImpl
        ( QPhoneCallProvider *provider,
          const QString& identifier, const QString& callType )
    : QObject( provider )
{
    d = new QPhoneCallImplPrivate();
    d->provider = provider;
    if ( identifier.isEmpty() )
        d->identifier = QUuid::createUuid().toString();
    else
        d->identifier = identifier;
    d->callType = callType;
    d->state = QPhoneCall::Idle;
    d->dataPort = -1;
    d->actions = QPhoneCallImpl::None;
    d->actionsChanged = false;
    provider->registerCall( this );
}

/*!
    Destroys this phone call implementation object.  Providers should
    not delete call implementation objects themselves.  The objects
    will be automatically deleted when the call transitions into
    an end state (\c HangupLocal, \c HangupRemote, \c Missed, etc),
    or when the QPhoneCallProvider that owns this object is deleted.
*/
QPhoneCallImpl::~QPhoneCallImpl()
{
    d->provider->deregisterCall( this );
    delete d;
}

/*!
    Returns the provider that is associated with this call.
*/
QPhoneCallProvider& QPhoneCallImpl::provider() const
{
    return *d->provider;
}

/*!
    Returns the globally-unique identifier that is associated with this call.
*/
QString QPhoneCallImpl::identifier() const
{
    return d->identifier;
}

/*!
    Returns the call type for this call. The callType indicates the type of call
    to be constructed (\c Voice, \c Data, \c Fax, \c VoIP, etc).

    \sa QPhoneCall::callType()
*/
QString QPhoneCallImpl::callType() const
{
    return d->callType;
}

/*!
    Returns the state that this call is currently in.

    \sa setState(), stateChanged(), QPhoneCall::state()
*/
QPhoneCall::State QPhoneCallImpl::state() const
{
    return d->state;
}

/*!
    Returns the phone number associated with the party on the
    other end of this call.  Returns an empty string if the
    other party is unknown (e.g. caller id is disabled).

    \sa setNumber()
*/
QString QPhoneCallImpl::number() const
{
    return d->number;
}

/*!
    Returns the actions that can be performed on this call in its
    current state.

    \sa setActions()
*/
QPhoneCallImpl::Actions QPhoneCallImpl::actions() const
{
    return d->actions;
}

/*!
    Dials a call according to the specified \a options.
    The default implementation sets number() to the value
    specified in \a options, but does not do anything else.

    \sa QDialOptions
*/
void QPhoneCallImpl::dial( const QDialOptions& options )
{
    Q_UNUSED(options);
    setNumber( options.number() );
}

/*!
    Hangs up this call and/or all other calls of its type (active/held).
    The \a scope indicates whether the hangup relates to this call
    only (QPhoneCall::CallOnly), or the entire call group (QPhoneCall::Group).
    The default implementation does nothing.
*/
void QPhoneCallImpl::hangup( QPhoneCall::Scope scope )
{
    Q_UNUSED(scope);
    // Nothing to do here in the default implementation.
}

/*!
    Accepts an incoming call. The call must be in the Incoming state.
    The default implementation does nothing.
*/
void QPhoneCallImpl::accept()
{
    // Nothing to do here in the default implementation.
}

/*!
    Puts this call on hold.  The default implementation fails the
    request indicating that hold is not supported by emitting the
    requestFailed() signal.
*/
void QPhoneCallImpl::hold()
{
    emit requestFailed( QPhoneCall::HoldFailed );
}

/*!
    Activates this call and takes it off hold.  The \a scope indicates
    whether the activation relates to this call only (QPhoneCall::CallOnly),
    or the entire call group (QPhoneCall::Group).  The default implementation
    does nothing.
*/
void QPhoneCallImpl::activate( QPhoneCall::Scope scope )
{
    Q_UNUSED(scope);
    // Nothing to do here in the default implementation.
}

/*!
    Joins the active and held calls together and makes them all active.
    If \a detachSubscriber is true, then the current subscriber is
    detached from the conversation.  The default implementation fails
    the request indicating that join is not supported by emitting the
    requestFailed() signal.
*/
void QPhoneCallImpl::join( bool detachSubscriber )
{
    Q_UNUSED(detachSubscriber);
    emit requestFailed( QPhoneCall::JoinFailed );
}

/*!
    Sends a sequence of DTMF \a tones over the call.  Ignored
    if the call is not connected, or it is on hold.  The default
    implementation does nothing.
*/
void QPhoneCallImpl::tone( const QString& tones )
{
    Q_UNUSED(tones);
    // Nothing to do here in the default implementation.
}

/*!
    Transfers the call to a new \a number and then discontinues this call.
    The default implementation fails the request indicating that transfer
    is not supported by emitting the requestFailed() signal.
*/
void QPhoneCallImpl::transfer( const QString& number )
{
    Q_UNUSED(number);
    emit requestFailed( QPhoneCall::TransferFailed );
}

/*!
    Requests the floor in a voice group call for \a secs seconds.
    After the timeout, the floor will be automatically released.

    If \a secs is -1, then requests the floor until explicitly
    released by a call to releaseFloor().

    The floorChanged() signal should be emitted when the floor
    state changes.
*/
void QPhoneCallImpl::requestFloor( int secs )
{
    Q_UNUSED(secs);
    // Nothing to do here.
}

/*!
    Releases the floor in a voice group call.  The floorChanged() signal
    should be emitted when the floor state changes.  If the phone call type
    does not support the floor, this function should do nothing.
*/
void QPhoneCallImpl::releaseFloor()
{
    // Nothing to do here.
}

/*!
    Sets the call's \a state and emits the stateChanged() signal.
    If the call has already ended, the state change will be ignored.

    Once the call ends, the QPhoneCallProvider will arrange for this
    object to be deleted upon the next entry to the event loop.

    \sa state(), stateChanged()
*/
void QPhoneCallImpl::setState( QPhoneCall::State state )
{
    // Note: -1 is a special value used by QModemCall when swapping
    // connected and held calls.  We don't emit stateChanged() for it.
    if ( ( state != d->state || d->actionsChanged ) &&
         d->state < QPhoneCall::HangupLocal ) {
        d->state = state;
        d->actionsChanged = false;
        if ( state >= QPhoneCall::HangupLocal ) // Clear actions on hangup.
            d->actions = QPhoneCallImpl::None;
        if ( state != (QPhoneCall::State)(-1) )
            emit stateChanged();
        if ( state >= QPhoneCall::HangupLocal )
            deleteLater();
    }
}

/*!
    Sets the phone \a number associated with this call.  This should be
    set on an outgoing call before setting the state to QPhoneCall::Dialing,
    and on an incoming call before setting the state to QPhoneCall::Incoming.

    \sa number()
*/
void QPhoneCallImpl::setNumber( const QString& number )
{
    d->number = number;
}

/*!
    Sets the \a actions that can be performed on this call.
    This should be called before setState() as the actions
    are only advertised to client applications when the
    state changes.

    \sa actions(), setState()
*/
void QPhoneCallImpl::setActions( QPhoneCallImpl::Actions actions )
{
    if ( d->actions != actions ) {
        d->actions = actions;
        d->actionsChanged = true;
    }
}

/*!
    Returns the data port that will be used for passing the body of a
    data call to client applications.  Returns -1 if not set.

    \sa setDataPort()
*/
int QPhoneCallImpl::dataPort() const
{
    return d->dataPort;
}

/*!
    Sets the data port that will be used for passing the body of
    a data call to client applications to \a port.  This should be
    set before changing the call's state to QPhoneCall::Connected
    with setState().

    \sa dataPort()
*/
void QPhoneCallImpl::setDataPort( int port )
{
    d->dataPort = port;
}

/*!
    Emits the notification() signal with the parameters \a type and \a value.

    This is a public method that emits the protected notification() signal.
    It is intended for use in QPhoneCallProvider implementations.

    \sa notification()
*/
void QPhoneCallImpl::emitNotification
        ( QPhoneCall::Notification type, const QString& value )
{
    emit notification( type, value );
}

/*!
    \fn void QPhoneCallImpl::stateChanged()

    Signal that is emitted when the call's state changes.

    \sa state(), setState()
*/

/*!
    \fn void QPhoneCallImpl::requestFailed( QPhoneCall::Request request )

    Signal that is emitted when a specific type of \a request fails.

    \sa hold(), join()
*/

/*!
    \fn void QPhoneCallImpl::notification( QPhoneCall::Notification type, const QString& value )

    Signal that is emitted when the auxiliary notification \a type is
    set to \a value.

    \sa emitNotification()
*/

/*!
    \fn void QPhoneCallImpl::floorChanged( bool haveFloor, bool floorAvailable )

    Signal that is emitted when the floor state changes to \a haveFloor
    in response to a requestFloor(), releaseFloor(), or implicit release.
    Also emitted when the upstream \a floorAvailable state changes.
*/

/*!
    \class QPhoneCallProvider
    \inpublicgroup QtTelephonyModule

    \brief The QPhoneCallProvider class implements a mechanism for phone call providers to hook into the telephony system.
    \ingroup telephony

    Phone call providers (e.g. GSM and VoIP) inherit from the
    QPhoneCallProvider class and override the create() method.
    They also separately inherit their own call implementation class
    from QPhoneCallImpl.

    Subclasses should call setCallTypes() within their constructor to
    indicate the call types that they can handle.

    The QPhoneCallProvider class provides the infrastructure to receive
    requests from client applications and dispatch them to the appropriate
    methods on QPhoneCallImpl.  As the calls change state, the state
    information is sent back to the client applications.

    Client applications should use QPhoneCallManager and QPhoneCall
    to manage phone calls.  Only the server-side providers need to use
    QPhoneCallProvider and QPhoneCallImpl.

    For AT-based modems, phone call providers should consider using
    QModemCallProvider instead of QPhoneCallProvider.

    \sa QPhoneCallImpl, QPhoneCallManager, QPhoneCall, QModemCallProvider
*/

class QPhoneCallProviderPrivate
{
public:
    QList<QPhoneCallImpl *> calls;
    QString service;
    QtopiaIpcAdaptor *request;
    QtopiaIpcAdaptor *response;
    QValueSpaceObject *callTypes;
    int transactionCount;
    QList<QPhoneCallImpl *> transaction;
};

/*!
    Constructs a new phone call provider for \a service and attaches
    it to \a parent.

    In a subclass, the constructor will typically contain a call
    to setCallTypes() to advertise the list of call types that
    the provider supports.  The list is advertised in the Qt Extended value space and can be queried by client applications
    using QPhoneCallManager::callTypes().

    The following example demonstrates the recommended contents for
    the constructor in a subclass of QPhoneCallProvider():

    \code
    VoIPCallProvider::VoIPCallProvider( QTelephonyService *service )
        : QPhoneCallProvider( service->service(), service )
    {
        setCallTypes( QStringList( "VoIP" ) );
    }
    \endcode

    \sa setCallTypes()
*/
QPhoneCallProvider::QPhoneCallProvider
            ( const QString& service, QObject *parent )
    : QObject( parent )
{
    d = new QPhoneCallProviderPrivate();
    d->service = service;
    d->transactionCount = 0;

    // Listen to requests from QPhoneCall and QPhoneCallManager.
    // All requests for all services are sent on the same channel.
    d->request = new QtopiaIpcAdaptor
        ( "QPE/Communications/QPhoneCallProvider/Request", this );
    QtopiaIpcAdaptor::connect
        ( d->request, MESSAGE(listAllCalls()), this, SLOT(listAllCalls()) );
    QtopiaIpcAdaptor::connect
        ( d->request, MESSAGE(dial(QString,QString,QString,QDialOptions)),
          this, SLOT(dial(QString,QString,QString,QDialOptions)) );
    QtopiaIpcAdaptor::connect
        ( d->request, MESSAGE(hangup(QString,QPhoneCall::Scope)),
          this, SLOT(hangup(QString,QPhoneCall::Scope)) );
    QtopiaIpcAdaptor::connect
        ( d->request, MESSAGE(accept(QString)),
          this, SLOT(accept(QString)) );
    QtopiaIpcAdaptor::connect
        ( d->request, MESSAGE(hold(QString)),
          this, SLOT(hold(QString)) );
    QtopiaIpcAdaptor::connect
        ( d->request, MESSAGE(activate(QString,QPhoneCall::Scope)),
          this, SLOT(activate(QString,QPhoneCall::Scope)) );
    QtopiaIpcAdaptor::connect
        ( d->request, MESSAGE(join(QString,bool)),
          this, SLOT(join(QString,bool)) );
    QtopiaIpcAdaptor::connect
        ( d->request, MESSAGE(tone(QString,QString)),
          this, SLOT(tone(QString,QString)) );
    QtopiaIpcAdaptor::connect
        ( d->request, MESSAGE(transfer(QString,QString)),
          this, SLOT(transfer(QString,QString)) );

    // Send responses back to QPhoneCall and QPhoneCallManager.
    // All responses for all services are sent on the same channel.
    d->response = new QtopiaIpcAdaptor
        ( "QPE/Communications/QPhoneCallProvider/Response", this );
    d->callTypes = new QValueSpaceObject
        ( "/Communications/QPhoneCallProvider/CallTypes", this );
}

/*!
    Destroys this phone call provider and all calls associated with it.
*/
QPhoneCallProvider::~QPhoneCallProvider()
{
    delete d;
}

/*!
    Returns the name of the service that this provider is associated with.
*/
QString QPhoneCallProvider::service() const
{
    return d->service;
}

/*!
    Returns the list of all phone calls associated with this provider.

    \sa create(), findCall()
*/
QList<QPhoneCallImpl *> QPhoneCallProvider::calls() const
{
    return d->calls;
}

/*!
    Returns a call with a specific \a identifier.  Returns null if there is no such call.

    \sa calls()
*/
QPhoneCallImpl *QPhoneCallProvider::findCall( const QString& identifier ) const
{
    QList<QPhoneCallImpl *>::ConstIterator it;
    for ( it = d->calls.begin(); it != d->calls.end(); ++it ) {
        if ( (*it)->identifier() == identifier )
            return *it;
    }
    return 0;
}

/*!
    Begins a state change transaction.  Calls to QPhoneCallImpl::setState()
    will be queued.  The notification of the state change will not be sent
    to client applications until endStateTransaction() is called.  This is
    typically used to update several calls at once; for example when
    swapping active and held calls.

    If beginStateTransaction() is called multiple times before
    endStateTransaction(), then the notifications will be sent once
    the same number of endStateTransaction() calls have been made.

    \sa endStateTransaction(), QPhoneCallImpl::setState()
*/
void QPhoneCallProvider::beginStateTransaction()
{
    ++( d->transactionCount );
}

/*!
    Ends a state change transaction and transmits any queued notifications.

    \sa beginStateTransaction(), QPhoneCallImpl::setState()
*/
void QPhoneCallProvider::endStateTransaction()
{
    if ( --( d->transactionCount ) <= 0 ) {
        sendStateTransaction();
    }
}

/*!
    \fn void QPhoneCallProvider::callStatesChanged()

    Signal that is emitted when one or more calls within this provider
    have changed state.

    \sa QPhoneCallImpl::stateChanged()
*/

/*!
    \fn QPhoneCallImpl *QPhoneCallProvider::create( const QString& identifier, const QString& callType )

    Creates a new phone call implementation object for a call of type
    \a callType, and assigns it the specified \a identifier.  The object
    constructed will be of a subtype of QPhoneCallImpl.  The typical
    implementation in the subclass is as follows:

    \code
    return new QMyPhoneCallImpl( this, identifier, callType );
    \endcode

    This function is called when a dial request is received from a
    client application.  It will be followed by a call to
    QPhoneCallImpl::dial().

    For incoming calls, the provider implementation should construct a
    QPhoneCallImpl object directly, with an empty identifier as the
    argument.

    Providers should not delete call implementation objects themselves.
    The objects will be automatically deleted when the call transitions into
    an end state (\c HangupLocal, \c HangupRemote, \c Missed, etc).
*/

/*!
    Sets the list of supported call \a types for this provider.  The list is
    advertised in the Qt Extended value space, and can be queried by client
    applications using QPhoneCallManager::callTypes().
*/
void QPhoneCallProvider::setCallTypes( const QStringList& types )
{
    d->callTypes->setAttribute( d->service, types );
}

void QPhoneCallProvider::listAllCalls()
{
    beginStateTransaction();
    QList<QPhoneCallImpl *>::Iterator it;
    for ( it = d->calls.begin(); it != d->calls.end(); ++it ) {
        sendState( *it );
    }
    endStateTransaction();
}

void QPhoneCallProvider::dial
        ( const QString& identifier, const QString& service,
          const QString& callType, const QDialOptions& options )
{
    // Ignore if the request was not for this service.
    if ( service != d->service )
        return;

    // There should not be an existing call with this identifier.
    if ( fromIdentifier( identifier ) )
        return;

    // Create a new call object and start dialing.
    QPhoneCallImpl *call = create( identifier, callType );
    if ( call )
        call->dial( options );
}

void QPhoneCallProvider::hangup
        ( const QString& identifier, QPhoneCall::Scope scope )
{
    QPhoneCallImpl *call = fromIdentifier( identifier );
    if ( call )
        call->hangup( scope );
}

void QPhoneCallProvider::accept( const QString& identifier )
{
    QPhoneCallImpl *call = fromIdentifier( identifier );
    if ( call )
        call->accept();
}

void QPhoneCallProvider::hold( const QString& identifier )
{
    QPhoneCallImpl *call = fromIdentifier( identifier );
    if ( call )
        call->hold();
}

void QPhoneCallProvider::activate
        ( const QString& identifier, QPhoneCall::Scope scope )
{
    QPhoneCallImpl *call = fromIdentifier( identifier );
    if ( call )
        call->activate( scope );
}

void QPhoneCallProvider::join
        ( const QString& identifier, bool detachSubscriber )
{
    QPhoneCallImpl *call = fromIdentifier( identifier );
    if ( call )
        call->join( detachSubscriber );
}

void QPhoneCallProvider::tone
        ( const QString& identifier, const QString& tones )
{
    QPhoneCallImpl *call = fromIdentifier( identifier );
    if ( call )
        call->tone( tones );
}

void QPhoneCallProvider::transfer
        ( const QString& identifier, const QString& number )
{
    QPhoneCallImpl *call = fromIdentifier( identifier );
    if ( call )
        call->transfer( number );
}

void QPhoneCallProvider::stateChanged()
{
    sendState( qobject_cast<QPhoneCallImpl *>( sender() ) );
}

void QPhoneCallProvider::requestFailed( QPhoneCall::Request request )
{
    QPhoneCallImpl *call = qobject_cast<QPhoneCallImpl *>( sender() );
    d->response->send
        ( MESSAGE(requestFailed(QString,QPhoneCall::Request)),
          call->identifier(), qVariantFromValue( request ) );
}

void QPhoneCallProvider::notification
        ( QPhoneCall::Notification type, const QString& value )
{
    QPhoneCallImpl *call = qobject_cast<QPhoneCallImpl *>( sender() );
    d->response->send
        ( MESSAGE(notification(QString,QPhoneCall::Notification,QString)),
          call->identifier(), qVariantFromValue( type ), value );
}

void QPhoneCallProvider::floorChanged( bool haveFloor, bool floorAvailable )
{
    QPhoneCallImpl *call = qobject_cast<QPhoneCallImpl *>( sender() );
    d->response->send
        ( MESSAGE(floorChanged(QString,bool,bool)),
          call->identifier(), haveFloor, floorAvailable );
}

/*!
    Called when \a call is first registered with this provider.
    This function is called automatically by the constructor for QPhoneCallImpl.

    \sa deregisterCall()
*/
void QPhoneCallProvider::registerCall( QPhoneCallImpl *call )
{
    connect( call, SIGNAL(stateChanged()), this, SLOT(stateChanged()) );
    connect( call, SIGNAL(requestFailed(QPhoneCall::Request)),
             this, SLOT(requestFailed(QPhoneCall::Request)) );
    connect( call, SIGNAL(notification(QPhoneCall::Notification,QString)),
             this, SLOT(notification(QPhoneCall::Notification,QString)) );
    connect( call, SIGNAL(floorChanged(bool,bool)),
             this, SLOT(floorChanged(bool,bool)) );
    d->calls.append( call );
}

/*!
    Called when \a call is deregistered from this provider.
    This function is called automatically by the destructor for QPhoneCallImpl.

    \sa registerCall()
*/
void QPhoneCallProvider::deregisterCall( QPhoneCallImpl *call )
{
    d->calls.removeAll( call );
}

QPhoneCallImpl *QPhoneCallProvider::fromIdentifier( const QString& identifier )
{
    QList<QPhoneCallImpl *>::Iterator it;
    for ( it = d->calls.begin(); it != d->calls.end(); ++it ) {
        QPhoneCallImpl *call = *it;
        if ( call->identifier() == identifier )
            return call;
    }
    return 0;
}

void QPhoneCallProvider::sendState( QPhoneCallImpl *call )
{
    // If we have an open transaction, then queue the notification for later.
    if ( d->transactionCount > 0 ) {
        d->transaction.append( call );
        return;
    }

    // Send the state information for this single call immediately.
    if ( call->dataPort() != -1 && call->state() == QPhoneCall::Connected ) {
        d->response->send( MESSAGE(dataPort(QString,int)),
                           call->identifier(), call->dataPort() );
    }
    d->response->send( MESSAGE(stateChanged(QString,QPhoneCall::State,QString,QString,QString,int)) )
        << call->identifier()
        << qVariantFromValue( call->state() )
        << call->number()
        << d->service
        << call->callType()
        << (int)( call->actions() );

    emit callStatesChanged();
}

void QPhoneCallProvider::sendStateTransaction()
{
    QList<QPhoneCallImpl *>::ConstIterator it;
    QByteArray transaction;
    {
        QDataStream stream
            ( &transaction, QIODevice::WriteOnly | QIODevice::Append );
        stream << d->transaction.size();
        for ( it = d->transaction.begin(); it != d->transaction.end(); ++it ) {
            QPhoneCallImpl *call = *it;
            stream << call->identifier();
            stream << call->state();
            stream << call->number();
            stream << d->service;
            stream << call->callType();
            stream << (int)( call->actions() );
            stream << call->dataPort();
        }
    }
    d->response->send( MESSAGE(stateTransaction(QByteArray)), transaction );
    d->transaction.clear();
    d->transactionCount = 0;
    emit callStatesChanged();
}
