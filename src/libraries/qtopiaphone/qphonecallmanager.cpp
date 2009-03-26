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

#include <qphonecallmanager.h>
#include "qphonecallmanager_p.h"
#include "qphonecall_p.h"
#include <qglobal.h>
#include <quuid.h>
#include <qtimer.h>

/*!
    \class QPhoneCallManager
    \inpublicgroup QtTelephonyModule

    \brief The QPhoneCallManager class provides access to the phone's call list.
    \ingroup telephony

    The current list of calls in the system can be inspected with QPhoneCallManager,
    and new outgoing calls can be created with create().

    The calls() method returns a list of all active calls in the system.  Active
    calls are those that are dialing, incoming, connected, or on hold.  Idle and
    dropped calls are not included in the list.

    New calls are added to the call list whenever a client application
    creates a call with create(), or when an incoming call is detected
    by the back-end telephony services.  The newCall() signal is
    emitted when either of these conditions occurs.

    As calls change state, the statesChanged() signal is emitted,
    providing a list of all calls affected by the state change.
    More than one call may be affected by a state change in cases where
    an entire group of calls are hung up, put on hold, or swapped with
    an active call group.

    State changes on individual calls can be tracked using QPhoneCall::connectStateChanged(),
    but this does not allow group operations to be tracked atomically.  The statesChanged()
    signal allows atomic tracking of group operations.

    The methods callTypes() and services() can be used to find the telephony
    handlers in the system, so that higher-level applications can choose
    the most appropriate type of call.

    For an example that demonstrates the use of QPhoneCallManager, see
    \l {Tutorial: Making Phone Calls}{Making Phone Calls}.  Also see the
    documentation for QPhoneCall.

    \sa QPhoneCall
*/

QPhoneCallManagerPrivate::QPhoneCallManagerPrivate( QObject *parent )
    : QObject( parent )
{
    // Hook onto the value space to find the supported call types.
    // The items under "/Communications/QPhoneCallProvider/CallTypes"
    // are the service names, each containing a QStringList of supported
    // call types.  e.g:
    //
    //      modem = { Voice, Data, Fax, Video }
    //      voip = { VoIP }
    //
    item = new QValueSpaceItem
        ( "/Communications/QPhoneCallProvider/CallTypes", this );
    connect( item, SIGNAL(contentsChanged()), this, SLOT(contentsChanged()) );

    // Hook onto the ipc channels for talking to the phone call providers.
    request = new QtopiaIpcAdaptor
        ( "QPE/Communications/QPhoneCallProvider/Request", this );
    response = new QtopiaIpcAdaptor
        ( "QPE/Communications/QPhoneCallProvider/Response", this );
    QtopiaIpcAdaptor::connect
        ( response, MESSAGE(stateChanged(QString,QPhoneCall::State,QString,QString,QString,int)),
          this, SLOT(callStateChanged(QString,QPhoneCall::State,QString,QString,QString,int)) );
    QtopiaIpcAdaptor::connect
        ( response, MESSAGE(stateTransaction(QByteArray)),
          this, SLOT(callStateTransaction(QByteArray)) );

    // Load the initial call type list.
    loadCallTypes();

    // Send a message to the providers to load the current call list.
    request->send( MESSAGE(listAllCalls()) );
}

QPhoneCallManagerPrivate::~QPhoneCallManagerPrivate()
{
    // Nothing to do here at present.
}

Q_GLOBAL_STATIC(QPhoneCallManagerPrivate, callManager);

QPhoneCallManagerPrivate *QPhoneCallManagerPrivate::instance()
{
    return callManager();
}

QPhoneCall QPhoneCallManagerPrivate::create
        ( const QString& service, const QString& callType )
{
    QPhoneCallPrivate *priv = new QPhoneCallPrivate
        ( this, service, callType, QUuid::createUuid().toString() );
    connect( priv, SIGNAL(stateChanged(QPhoneCall)),
             this, SLOT(trackStateChanged(QPhoneCall)) );
    return QPhoneCall( priv );
}

void QPhoneCallManagerPrivate::callStateChanged
        ( const QString& identifier, QPhoneCall::State state,
          const QString& number, const QString& service,
          const QString& callType, int actions )
{
    // Look for new calls from the phone call providers.
    if ( state < QPhoneCall::HangupLocal ) {
        QList<QPhoneCall>::Iterator it;
        for ( it = calls.begin(); it != calls.end(); ++it ) {
            if ( (*it).identifier() == identifier )
                return;     // We already know about this call.
        }

        // This is a new call that we didn't create ourselves.
        QPhoneCallPrivate *priv = new QPhoneCallPrivate
            ( this, service, callType, identifier );
        connect( priv, SIGNAL(stateChanged(QPhoneCall)),
                 this, SLOT(trackStateChanged(QPhoneCall)) );
        priv->state = state;
        priv->actions = (QPhoneCallImpl::Actions)actions;
        priv->number = number;
        if ( priv->state == QPhoneCall::Incoming ||
             priv->state == QPhoneCall::Dialing ||
             priv->state == QPhoneCall::Alerting ) {
            // Incoming call, or call created by another process.
            // Set the initial start time correctly.
            priv->startTime = QDateTime::currentDateTime();
        }
        QPhoneCall call( priv );
        calls.append( call );
        emit newCall( call );
        QList<QPhoneCall> list;
        list.append( call );
        emit statesChanged( list );
    }
}

void QPhoneCallManagerPrivate::callStateTransaction
        ( const QByteArray& transaction )
{
    int count, actions, dataPort;
    QString identifier, number, service, callType;
    QPhoneCall::State state;
    QList<QPhoneCall> changedCalls;
    QList<QPhoneCall> newCalls;
    QList<QPhoneCall>::Iterator it;
    QPhoneCall call;
    QPhoneCallPrivate *priv;

    // Process the transaction.
    QDataStream stream( transaction );
    stream >> count;
    while ( count-- > 0 ) {
        // Get the information about the next call in the transaction.
        stream >> identifier;
        stream >> state;
        stream >> number;
        stream >> service;
        stream >> callType;
        stream >> actions;
        stream >> dataPort;

        // Find the call.
        call = QPhoneCall();
        for ( it = calls.begin(); it != calls.end(); ++it ) {
            if ( (*it).identifier() == identifier ) {
                call = *it;
                break;
            }
        }

        // Existing or new call?
        if ( !call.isNull() ) {

            // Existing call - update its properties, but don't
            // emit the stateChanged() signal just yet.
            priv = call.d;
            priv->state = state;
            priv->actions = (QPhoneCallImpl::Actions)actions;
            if ( priv->number.isEmpty() )
                priv->number = number;
            if ( dataPort != -1 && state == QPhoneCall::Connected )
                priv->dataPort = dataPort;
            changedCalls.append( call );

        } else if ( state < QPhoneCall::HangupLocal ) {

            // This is a new call that we didn't create ourselves.
            priv = new QPhoneCallPrivate
                ( this, service, callType, identifier );
            connect( priv, SIGNAL(stateChanged(QPhoneCall)),
                     this, SLOT(trackStateChanged(QPhoneCall)) );
            priv->state = state;
            priv->actions = (QPhoneCallImpl::Actions)actions;
            priv->number = number;
            if ( state == QPhoneCall::Connected )
                priv->dataPort = dataPort;
            call = QPhoneCall( priv );
            calls.append( call );
            newCalls.append( call );

        }
    }

    // Emit the signals corresponding to the changes we just made.
    for ( it = changedCalls.begin(); it != changedCalls.end(); ++it ) {
        (*it).d->emitStateChanged();
    }
    for ( it = newCalls.begin(); it != newCalls.end(); ++it ) {
        emit newCall( *it );
    }

    // Advertise the changes that were made during the transaction.
    emit statesChanged( changedCalls + newCalls );
}

void QPhoneCallManagerPrivate::trackStateChanged( const QPhoneCall& call )
{
    // Keep the call list up to date as the individual calls change state.
    if ( call.dropped() ) {
        calls.removeAll( call );
    } else if ( !call.idle() && !calls.contains( call ) ) {
        calls.append( call );
    }
}

void QPhoneCallManagerPrivate::loadCallTypes()
{
    QStringList services = item->subPaths();
    QStringList serviceTypes;
    callTypes.clear();
    callTypeMap.clear();
    foreach ( QString service, services ) {
        serviceTypes = item->value( service ).toStringList();
        foreach ( QString type, serviceTypes ) {
            if ( !callTypes.contains( type ) ) {
                callTypes += type;
                callTypeMap[type] = QStringList( service );
            } else {
                callTypeMap[type] += service;
            }
        }
    }

    // Abort any calls that relate to services that are no longer active.
    // We abort upon the next entry to the event loop to prevent the
    // "calls" list from being side-effected while we are scanning it.
    QList<QPhoneCall>::ConstIterator it;
    QPhoneCallPrivate *priv;
    for ( it = calls.begin(); it != calls.end(); ++it ) {
        priv = (*it).d;
        if ( !services.contains(priv->service) )
            QTimer::singleShot(0, priv, SLOT(abortCall()));
    }
}

void QPhoneCallManagerPrivate::contentsChanged()
{
    loadCallTypes();
    emit callTypesChanged();
}

/*!
    Creates a new phone call manager and attaches it to \a parent.
*/
QPhoneCallManager::QPhoneCallManager( QObject *parent )
    : QObject( parent )
{
    d = QPhoneCallManagerPrivate::instance();
    connect( d, SIGNAL(newCall(QPhoneCall)),
             this, SIGNAL(newCall(QPhoneCall)) );
    connect( d, SIGNAL(callTypesChanged()), this, SIGNAL(callTypesChanged()) );
    connect( d, SIGNAL(statesChanged(QList<QPhoneCall>)),
             this, SIGNAL(statesChanged(QList<QPhoneCall>)) );
}

/*!
    Destroys this phone call manager.
*/
QPhoneCallManager::~QPhoneCallManager()
{
    // Nothing to do here - the QPhoneCallManagerPrivate instance is shared
    // between all QPhoneCallManager instances, both current and future.
}

/*!
    Returns a list of all active calls in the system.  Active calls are
    those that are dialing, incoming, connected, or on hold.  Idle and
    dropped calls are not included in this list.

    New calls will be added to this list whenever a client application
    creates a call with create(), or when an incoming call is detected
    by the back-end telephony services.  The newCall() signal is
    emitted when either of these conditions occurs.

    As calls change state, the statesChanged() signal is emitted,
    providing a list of all calls affected by the state change.

    \sa create(), newCall(), statesChanged()
*/
QList<QPhoneCall> QPhoneCallManager::calls() const
{
    return d->calls;
}

/*!
    Creates a call with the specified call \a type.  If there is
    more than one service that supports the \a type, this method
    will choose the first that it finds.  Returns a null QPhoneCall
    if no services support \a type.

    The \a type is usually one of \c Voice, \c VoIP, \c Data, \c Fax,
    \c Video, etc.  Use callTypes() to get a complete list of all
    call types that are supported by the system.

    The new call will initially be in the \c Idle state, ready to dial.
    It will not appear in the calls() list until it is actually dialed.

    \sa callTypes()
*/
QPhoneCall QPhoneCallManager::create( const QString& type )
{
    QStringList list = services( type );
    if ( list.isEmpty() )
        return QPhoneCall();
    else
        return create( type, list[0] );
}

/*!
    Create a call with the specified call \a type on \a service.
    Returns a null QPhoneCall if the combination of \a type and
    \a service is invalid.

    The \a type is usually one of \c Voice, \c VoIP, \c Data, \c Fax,
    \c Video, etc.  Use callTypes() to get a complete list of
    call types that are supported by the system.

    The \a service is a telephony service name such as \c modem or
    \c voip.  Use services() to get a complete list of service names
    that are supported by the system.

    The new call will initially be in the "idle" state, ready to dial.
    It will not appear in the calls() list until it is actually dialed.

    \sa QTelephonyService, callTypes(), services()
*/
QPhoneCall QPhoneCallManager::create
        ( const QString& type, const QString& service )
{
    // Validate the arguments.
    QMap< QString, QStringList >::ConstIterator it;
    it = d->callTypeMap.find( type );
    if ( it == d->callTypeMap.end() || !it.value().contains( service ) )
        return QPhoneCall();

    // Create the call.
    return d->create( service, type );
}

/*!
    Returns a list of all services that provide phone call functionality
    within the system.  The returned list will contain strings
    such as \c{modem}, \c{voip}, etc, indicating the name of the
    associated telephony service.

    \sa QTelephonyService, callTypes()
*/
QStringList QPhoneCallManager::services() const
{
    return d->item->subPaths();
}

/*!
    Returns a list of all services that provide phone call functionality
    for calls of \a type within the system.

    \sa QTelephonyService, callTypes()
*/
QStringList QPhoneCallManager::services( const QString& type ) const
{
    QMap< QString, QStringList >::ConstIterator it;
    it = d->callTypeMap.find( type );
    if ( it != d->callTypeMap.end() )
        return it.value();
    else
        return QStringList();
}

/*!
    Returns a list of all call types that are supported by the system at present.
    The returned list will contain strings such as \c{Voice}, \c{VoIP},
    \c{Data}, etc, indicating the valid call types that can be
    created with create().  For example, the following can be used to
    determine if \c{VoIP} calls are possible:

    \code
    QPhoneCallManager mgr;
    if (mgr.callTypes().contains("VoIP"))
        ...
    \endcode

    Call types are added to this list whenever new telephony services
    are registered with the system, and removed when the telephony
    services are deregistered.  The callTypesChanged() signal can be
    used to track changes to the call type list.

    \sa create(), callTypesChanged(), services()
*/
QStringList QPhoneCallManager::callTypes() const
{
    return d->callTypes;
}

/*!
    Returns a list of all call types that are supported by \a service.
    The returned list will contain strings such as \c{Voice}, \c{VoIP},
    \c{Data}, etc, indicating the valid call types that can be
    created with create() when \a service is supplied as its
    second parameter.

    For example, the following can be used to determine if the \c{modem}
    service can create \c{Data} calls, irrespective of whether other
    services can create \c{Data} calls:

    \code
    QPhoneCallManager mgr;
    if (mgr.callTypes("modem").contains("Data"))
        ...
    \endcode

    Call types are added to this list when the specified telephony
    \a service is registered, and removed when the specified telephony
    \a service is deregistered and there are no other services that
    provide the same call type.  The callTypesChanged() signal can be
    used to track changes to the call type list.

    \sa create(), callTypesChanged(), services()
*/
QStringList QPhoneCallManager::callTypes( const QString& service ) const
{
    return d->item->value( service ).toStringList();
}

/*!
    Returns the phone call associated with modem identifier \a id.
    Returns a null QPhoneCall object if there are no phone calls currently
    associated with that modem identifier.

    This function is intended for use with GSM-style key sequences
    such as \c{1x SEND} and \c{2x SEND}, which affect a specific call.

    \sa QPhoneCall::modemIdentifier()
*/
QPhoneCall QPhoneCallManager::fromModemIdentifier( int id ) const
{
    QList<QPhoneCall>::ConstIterator it;
    for ( it = d->calls.begin(); it != d->calls.end(); ++it ) {
        if ( (*it).modemIdentifier() == id )
            return (*it);
    }
    return QPhoneCall();
}

/*!
    \fn void QPhoneCallManager::newCall( const QPhoneCall& call )

    Provides notification that a new \a call has been added to the calls() list.
    New calls are added to the call list whenever a client application
    creates a call with create(), or when an incoming call is detected
    by the back-end telephony services.

    \sa calls(), create(), statesChanged()
*/

/*!
    \fn void QPhoneCallManager::callTypesChanged()

    Signal that is emitted when the list of call types that are supported
    by the system changes, or if the services that implement the call types
    changes.  This is usually an indication that a new telephony handler
    has been created, or an existing telephony handler has shut down.

    \sa callTypes(), services()
*/

/*!
    \fn void QPhoneCallManager::statesChanged( const QList<QPhoneCall>& calls )

    Signal that is emitted when the states within \a calls change at once
    during an atomic operation; e.g. swapping held and active calls.
    State changes that affect only a single call will also be reported
    via this signal.

    This signal will be sent after the individual state changes have been
    reported via newCall() and QPhoneCall::connectStateChanged().

    \sa QPhoneCall::connectStateChanged(), newCall()
*/
