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


#include "qphonecall_p.h"
#include "qphonecallmanager_p.h"
#include <qatutils.h>
#include <qserialsocket.h>
#include <qtopialog.h>
#include <qtopiaipcadaptor.h>
#include <qvaluespace.h>
#include <custom.h>
#include <qtimer.h>

// Define the DTMF tone and pause times, to avoid sending tones too fast.
// These can be overridden in custom.h if desired.
#ifndef QTOPIA_DTMF_TONE_TIME
#define QTOPIA_DTMF_TONE_TIME   300
#endif
#ifndef QTOPIA_DTMF_PAUSE_TIME
#define QTOPIA_DTMF_PAUSE_TIME  100
#endif

/*!
    \class QPhoneCall
    \inpublicgroup QtTelephonyModule

    \brief The QPhoneCall class provides an interface for managing individual incoming and outgoing phone calls.
    \ingroup telephony

    \target qphonecall_overview
    QPhoneCall is the primary interface for managing a phone call that is in progress.
    Outgoing calls are created in client applications by a call to QPhoneCallManager::create(),
    and incoming calls are advertised to client applications via the
    QPhoneCallManager::newCall() signal, with the phone call's state set to \c Incoming.

    Client applications can see all of the calls in the system, even those created by
    other client applications.  The QPhoneCallManager::calls() method provides access
    to the complete call list, and the QPhoneCallManager::newCall() signal can be used
    to receive notification of when another client application creates a call.

    QPhoneCall instances act as identifiers for the actual call objects.  The actual
    call object will persist until the call ends, either due to explicit hangup(),
    remote disconnection, or some form of network failure.

    \section1 Outgoing calls

    Outgoing phone calls are created by QPhoneCallManager::create() and start in the
    \c Idle state.  They then transition to the \c Dialing state when the dial()
    method is called.  The following example demonstrates how to dial a voice call
    to the phone number \c 1234567:

    \code
        QPhoneCallManager mgr;
        QPhoneCall call = mgr.create( "Voice" );

        QDialOptions dialOptions;
        dialOptions.setNumber( "1234567" );
        call.dial( dialOptions );
    \endcode

    Once an outgoing call enters the \c Dialing state, it may transition to one of
    the following states, as reported by state():

    \table
        \row \o \c Alerting \o The network is alerting the other party that they have an
                    incoming call.  Not all networks support alerting, so an outgoing
                    call may never enter this state.  For most client applications,
                    \c Alerting can be considered identical to \c Dialing.
        \row \o \c Connected \o The call was accepted by the other party and is now
                    connected.
        \row \o \c HangupLocal \o This client application, or some other application,
                    called hangup() to abort the outgoing call before it could be
                    accepted by the other party.
        \row \o \c HangupRemote \o The other party rejected the call, the party was
                    busy, or it timed out without some kind of response.
        \row \o \c OtherFailure \o The dial attempt could not start because of
                    a local problem; usually because there is another call already
                    dialing, or there are insufficient resources to complete the request.
        \row \o \c ServiceHangup \o The dial attempt was for a supplementary service
                    number starting with \c{*} or \c{#}.  The request has been sent,
                    but there is no actual call from this point onwards.
                    While supplementary service requests can be sent with dial(),
                    it is better to use QSupplementaryServices::sendSupplementaryServiceData()
                    as it has better error reporting for failed supplementary service
                    requests.
    \endtable

    Changes in state() are advertised to client applications with the \c stateChanged()
    signal.  The client application must use connectStateChanged() to connect to this signal.

    Once connected, the call can be terminated locally with hangup(), put on hold
    with hold(), released from hold or split from a multi-party conversation with activate(),
    joined with another call for a multi-party conversation with join(), and sent DTMF
    tones with tone().

    Voice-Over-IP calls are initiated in a similar manner to GSM voice calls:

    \code
        QPhoneCallManager mgr;
        QPhoneCall call = mgr.create( "VoIP" );

        QDialOptions dialOptions;
        dialOptions.setNumber( "sip:jane@random.com" );
        call.dial( dialOptions );
    \endcode

    In this case, the phone number is replaced with the URI corresponding to the
    called party, and the call type is set to \c VoIP.  The system locates the
    relevant Voice-Over-IP telephony service and forwards the request to it.
    The complete list of active call types and services can be obtained from
    the QPhoneCallManager::callTypes() and QPhoneCallManager::services() methods.

    \section2 Incoming calls

    Incoming calls are advertised to client applications via the QPhoneCallManager::newCall()
    signal.  They can be distinguished from outgoing calls made by other client applications
    by checking the state() for \c Incoming.  The number() method will return the phone
    number of the calling party, if caller identification is available.

    An incoming call can be rejected by calling hangup(), and will transition to
    the \c HangupLocal state.

    Calls in the \c Incoming state can be transferred to a different number
    with transfer().  If the call has already been accepted, then to transfer the
    call requires the following steps be taken:

    \list
        \o Create a new outgoing QPhoneCall with QPhoneCall::dial().
        \o Join the two calls together with join(), passing true as the
           \c detachSubscriber parameter.
    \endlist

    Normally this process is performed by the dialer as part of the multi-party
    conversation logic in the user interface.

    Once connected, the call can be terminated locally with hangup(), put on hold
    with hold(), released from hold or split from a multi-party conversation with activate(),
    joined with another call for a multi-party conversation with join(), and sent DTMF
    tones with tone(); just as for outgoing calls.

    For a larger example that demonstrates the use of QPhoneCall for managing
    incoming and outgoing calls, see \l {Tutorial: Making Phone Calls}{Making Phone Calls}.

    \section2 Phone call providers

    Phone call functionality is provided by back-end telephony services in their
    QPhoneCallProvider implementation.  Each call is associated with with a
    QPhoneCallImpl object, whose functions mirror those in QPhoneCall.

    The Qt Extended telephony API automatically routes requests from client applications
    to the appropriate QPhoneCallProvider, based on the callType().  State changes
    in the provider are routed back to client applications and appear as
    \c stateChanged() signals (use connectStateChanged() to access these signals).

    See QPhoneCallProvider for more information on implementing phone call providers
    within telephony services.

    \sa QPhoneCallManager
    \sa QDialOptions
    \sa QPhoneCallProvider
    \sa QPhoneCallImpl
*/

/*!
    \enum QPhoneCall::State
    This enum defines the different states a QPhoneCall can have.
    For discussion on the state transitions see the \l{qphonecall_overview}{class overview}.

    \value Idle New outgoing call, not dialed yet.
    \value Incoming Incoming connection from remote party.
    \value Dialing Dialing, but not yet connected.
    \value Alerting Alerting other party during outgoing dial.  This state
                    will appear between \c Dialing and \c Connected only if the
                    network and call type supports alerting notifications.
    \value Connected Connected to the other party, but currently not on hold.
    \value Hold Connected to the other party, but currently on hold.
    \value HangupLocal Local side hung up the call.
    \value HangupRemote Remote side hung up the call, or call lost.
    \value Missed Incoming call that was missed.
    \value NetworkFailure Network has failed in some way.
    \value OtherFailure Unknown failure.
    \value ServiceHangup Supplementary service request caused the
           call to "hangup", notifying higher layers that the
           request has been sent.
*/

/*!
    \enum QPhoneCall::Scope
    Defines the scope of a QPhoneCall::hangup() or QPhoneCall::activate() operation.

    \value CallOnly Only the referenced call.
    \value Group All calls in the same active/held group.
*/

/*!
    \enum QPhoneCall::Request
    Defines the request types, for reporting failures.

    \value HoldFailed A call to QPhoneCall::hold() failed and the call could not be put on hold.
    \value JoinFailed A call to QPhoneCall::join() failed and the specified calls could not be joined.
    \value TransferFailed A call to QPhoneCall::transfer() failed and the call was not transferred.
    \value ActivateFailed A call to QPhoneCall::activate() failed and the call was not activated.
*/

/*!
    \enum QPhoneCall::Notification
    Defines auxiliary value notifications that may be associated
    with a phone call.

    \value CallingName The name of the calling party.
    \value DataStateUpdate Notification that is sent when the GPRS data
           state changes.  Use QPhoneCall::parseDataState() to decode
           this value.
    \value RemoteHold Notification that is sent when the remote party
           requests that the call be held or unheld.  This is used with
           VoIP implementations where either party can place a call on hold.
           This notification may have one of the values \c{hold} or \c{unhold}
           indicating whether the remote party is requesting a hold
           or an unhold.
    \value ConnectedLineId Full phone number of the other party, according
           to the connected line identification presentation (COLP) service.
           This notification may be emitted on outgoing calls where COLP
           is available.  May be an empty string if the COLP service is
           available but the other party is blocking the provision of
           their phone number.
*/

/*!
    \enum QPhoneCall::DataState
    This enum lists the flags that may be returned from QPhoneCall::parseDataState()

    \value PPPdStarted The pppd daemon has been started.  Usually combined
                       with DataActive or DataInactive.
    \value PPPdStopped The pppd daemon stopped in response to an explicit
                       call on QPhoneCall::hangup()
    \value PPPdFailed The pppd daemon stopped for some other reason.
    \value DataActive A data session is currently active.
    \value DataInactive No data session is currently active.
    \value Connecting The pppd daemon is attempting to connect.  Usually
                      combined with PPPdStarted and DataInactive.
    \value ConnectFailed The pppd daemon failed in its connection attempt.
*/

QPhoneCall::QPhoneCall( QPhoneCallPrivate *priv )
{
    d = priv;
    if ( d )
        d->ref();
}

/*!
    Constructs a new phone call object, initially in the Idle state.
*/
QPhoneCall::QPhoneCall()
{
    d = 0;
}

/*!
    Constructs a copy of \a call.
*/
QPhoneCall::QPhoneCall( const QPhoneCall& call )
{
    d = call.d;
    if ( d ) {
        d->ref();
    }
}

/*!
    Destroys this phone call object.  This will not destroy or hangup the
    actual underlying phone call, which will persist until hung up by
    the user or some other action.
*/
QPhoneCall::~QPhoneCall()
{
    if ( d && d->deref() ) {
        delete d;
    }
    d = 0;
}

/*!
    Copies \a call object to this object.
*/

QPhoneCall& QPhoneCall::operator=( const QPhoneCall& call )
{
    if ( call.d ) {
        call.d->ref();
    }
    if ( d && d->deref() ) {
        delete d;
    }
    d = call.d;
    return *this;
}

/*!
    Returns the globally-unique identifier for this call.
*/
QString QPhoneCall::identifier() const
{
    if ( d )
        return d->identifier;
    else
        return QString();
}

/*!
    Returns the number that the modem uses to identify the call,
    or -1 if the number is unknown or the call has terminated.

    Modem identifiers usually range between 1 and 9. Once a call
    terminates, its identifier will be reused for another call.

    Modem identifiers are used with AT commands like \c{AT+CHLD} to identify
    specific calls.  They may also be used in GCF key sequences such as
    \c{1x SEND} to hang up the call with modem identifier \c x.

    \sa QPhoneCallManager::fromModemIdentifier()
*/
int QPhoneCall::modemIdentifier() const
{
    // Read the modem identifier from the value space.
    QValueSpaceItem item
        ( "/Communications/QPhoneCallProvider/ModemIdentifiers" );
    return item.value( identifier(), -1 ).toInt();
}

/*!
    Returns the full number of the remote party.  This is the literal number that was
    dialed including affixes. Returns a null QString if the number is unknown.

    \sa number()
*/
QString QPhoneCall::fullNumber() const
{
    if( d && d->fullNumber.length() )
        return d->fullNumber;
    return number();
}

/*!
    Returns the phone number of the remote party, or a null QString if the
    number is unknown.

    \sa fullNumber()
*/
QString QPhoneCall::number() const
{
    if ( d )
        return d->number;
    else
        return QString();
}

/*!
    Returns the unique identifier of the contact of the remote party.  Returns a
    null QUniqueId if the remote party's unique identifier is unknown.
*/
QUniqueId QPhoneCall::contact() const
{
    if ( d )
        return d->contact;
    else
        return QUniqueId();
}

/*!
    Returns the state of this call.

    \sa connectStateChanged(), idle(), incoming(), dialing(), established()
    \sa connected(), onHold(), dropped(), missed()
*/
QPhoneCall::State QPhoneCall::state() const
{
    if ( d )
        return d->state;
    else
        return Idle;
}

/*!
    Returns the type of call (\c Voice, \c Data, etc).

    \sa QPhoneCallManager::callTypes()
*/
QString QPhoneCall::callType() const
{
    if ( d )
        return d->callType;
    else
        return "Voice";     // No tr
}

/*!
  Returns the date and time that this call started.  Usually this is the
  first time it enters the Dialing or Incoming state.

  \sa connectTime(), endTime()
*/
QDateTime QPhoneCall::startTime() const
{
    if ( d )
        return d->startTime;
    else
        return QDateTime();
}

/*!
    Returns the date and time that this call last entered the QPhoneCall::Connected state.
    Returns a null QDateTime instance if the call has never connected.

    \sa startTime(), endTime()
*/
QDateTime QPhoneCall::connectTime() const
{
    if ( d )
        return d->connectTime;
    else
        return QDateTime();
}

/*!
    Returns the date and time this call ended.
    If the call never ended a null QDateTime is returned.

    \sa startTime(), connectTime()
*/
QDateTime QPhoneCall::endTime() const
{
    if ( d )
        return d->endTime;
    else
        return QDateTime();
}

/*!
    Returns true if this call has been in the state QPhoneCall::Connected
    at some stage, otherwise false.

    \sa dialed()
*/
bool QPhoneCall::hasBeenConnected() const
{
    if ( d )
        return d->hasBeenConnected;
    else
        return false;
}

/*!
    Returns the pending tones for this call.  Pending tones are accumulated
    when the call is dialed with dial(), or new tones are added with tone().
    As the tones are sent, they are removed from the pendingTones() list.

    Use connectPendingTonesChanged() to be notified of changes in the
    pending tones.

    \sa connectPendingTonesChanged()
*/
QString QPhoneCall::pendingTones() const
{
    if ( d )
        return d->pendingTones;
    else
        return QString();
}

/*!
    Returns true if this call was dialed, false otherwise.

    \sa hasBeenConnected()
*/
bool QPhoneCall::dialed() const
{
    if ( d )
        return d->dialedCall;
    else
        return false;
}

/*!
    Dials a \a number on this call. The QPhoneCall object must be in the Idle state.

    If a \a contact is specified, it is used to display information on
    the party being called, and will be stored as part of the call history.

    If \a sendcallerid is true, the caller ID is enabled
    (on the assumption that it is normally disabled).

    If the call is not in the \c Idle state, the request will be quietly ignored.

    \sa hangup(), accept()
*/
void QPhoneCall::dial( const QString& number, bool sendcallerid, const QUniqueId& contact )
{
    QDialOptions options;
    options.setNumber( number );
    options.setCallerId( sendcallerid ? QDialOptions::SendCallerId
                                      : QDialOptions::DefaultCallerId );
    options.setContact( contact );
    dial( options );
}

/*!
    Dials a call according to the specified \a options.
    The QPhoneCall object must be in the Idle state.

    If the call is not in the \c Idle state, the request will be quietly ignored.

    \sa QDialOptions, hangup(), accept()
*/
void QPhoneCall::dial( const QDialOptions& options )
{
    QString num;
    if (callType() != "VoIP" && !options.number().startsWith(">")) // No tr
        num = QAtUtils::stripNumber( options.number() );
    else
        num = options.number();

    if ( d && d->state == Idle )
    {
        d->state = Dialing;
        d->fullNumber = options.number();
        d->number = num;
        d->dialedCall = true;
        d->contact = options.contact();

        int c = num.indexOf(QChar(','));
        QString baseNumber;
        if ( c >= 0 )
        {
            baseNumber = num.left(c);
            d->addPendingTones( num.mid(c+1) );
            //phone call will check for pending tones when connected
        }
        else
        {
            baseNumber = num;
        }

        QDialOptions modifiedOptions( options );
        modifiedOptions.setNumber( baseNumber );

        d->request->send( MESSAGE(dial(QString,QString,QString,QDialOptions)) )
            << d->identifier << d->service << d->callType
            << qVariantFromValue( modifiedOptions );
        d->emitStateChanged();

        // Advertise the state change through the manager's list interface.
        QList<QPhoneCall> list;
        list.append( *this );
        emit d->manager->statesChanged( list );
    }
}

/*!
    Accepts an incoming call.
    All active calls are automatically put on hold. Note that this object
    must be in the \c Incoming state for this to succeed.  If the call is
    not currently in the \c Incoming state, the request is ignored.

    To reject an incoming call, use hangup().

    \sa canAccept(), dial(), hangup()
*/

void QPhoneCall::accept()
{
    if ( d && d->state == Incoming ) {
        d->request->send( MESSAGE(accept(QString)), d->identifier );
    }
}

/*!
    Hangs up this call and/or all other calls of its type that are active or held.
    The \a scope indicates whether the hangup relates to this call
    only (CallOnly), or the entire call group (Group).

    \sa dial(), accept()
*/

void QPhoneCall::hangup( QPhoneCall::Scope scope )
{
    if ( d ) {
        if ( d->state == Dialing || d->state == Alerting ||
             d->state == Connected || d->state == Hold ||
             d->state == Incoming ) {
            d->request->send( MESSAGE(hangup(QString,QPhoneCall::Scope)),
                              d->identifier, qVariantFromValue( scope ) );
        }
    }
}

/*!
    Puts this active call and all other active calls on hold.
    Also activates any calls that are on hold or waiting.

    \sa activate(), canHold()
*/

void QPhoneCall::hold()
{
    if ( d && d->state == Connected ) {
        d->request->send( MESSAGE(hold(QString)), d->identifier );
    }
}

/*!
    Activates this call and takes it off hold.  The \a scope indicates
    whether the activation relates to this call only (CallOnly),
    or the entire call group (Group).

    \sa hold(), canActivate()
*/

void QPhoneCall::activate( QPhoneCall::Scope scope )
{
    if ( d && ( d->state == Connected || d->state == Hold ) ) {
        d->request->send( MESSAGE(activate(QString,QPhoneCall::Scope)),
                          d->identifier, qVariantFromValue( scope ) );
    }
}

/*!
    Joins the active and held calls together and makes them all active.
    If \a detachSubscriber is true, the current subscriber is
    detached from the conversation.

    Join requests may fail and cause the \c{requestFailed()} signal to
    be emitted.

    \sa connectRequestFailed(), canJoin()
*/

void QPhoneCall::join( bool detachSubscriber )
{
    if ( d && ( d->state == Connected || d->state == Hold ) ) {
        d->request->send( MESSAGE(join(QString,bool)),
                          d->identifier, detachSubscriber );
    }
}

/*!
    Sends a sequence of DTMF \a tones over the call.  Ignored
    if the call is not connected, or it is on hold.

    \sa pendingTones(), canTone()
*/

void QPhoneCall::tone( const QString& tones )
{
    if ( d && d->state == Connected ) {
        QString t = QAtUtils::stripNumber( tones );
        d->addPendingTones( t );
    }
}

/*!
    Transfers the call to a new \a number and then discontinues this call.

    Transfer requests may fail and cause the \c{requestFailed()} signal to
    be emitted.

    \sa connectRequestFailed(), canTransfer()
*/

void QPhoneCall::transfer( const QString& number )
{
    QString num = QAtUtils::stripNumber( number );

    if ( d && d->state == Incoming ) {
        d->request->send( MESSAGE(transfer(QString,QString)),
                          d->identifier, num );
    }
}

/*!
    Requests the floor in a voice group call for \a secs seconds.
    After the timeout, the floor will be automatically released.

    If \a secs is -1, requests the floor until explicitly
    released by a call to releaseFloor().

    \sa floorAvailable()
*/
void QPhoneCall::requestFloor( int secs )
{
    if ( d && d->state == Connected ) {
        d->request->send( MESSAGE(requestFloor(QString,int)),
                          d->identifier, secs );
    }
}

/*!
    Releases the floor in a voice group call. \sa floorAvailable()
*/
void QPhoneCall::releaseFloor()
{
    if ( d ) {
        d->request->send( MESSAGE(releaseFloor(QString)), d->identifier );
    }
}

/*!
    Returns true if this object currently has the floor in a voice group call.
    \sa floorAvailable()
*/
bool QPhoneCall::haveFloor() const
{
    if ( d )
        return d->haveFloor;
    else
        return false;
}

/*!
    Returns true if the floor is available in a voice group call.
    The "floor" is available if no one else is currently talking.

    If it isn't possible to detect whether or not the floor is available,
    even if the call type supports floors, this method will return true;
    however, calls to requestFloor() might not succeed.

    If this call type does not support the floor, this method will return false.
*/
bool QPhoneCall::floorAvailable() const
{
    if ( d )
        return d->floorAvailable;
    else
        return false;
}

/*!
    Registers a specified \a slot on \a object to receive notification of
    changes in the floor. The signal prototype is
    "floorChanged( const QPhoneCall& call )". \sa floorAvailable()
*/
void QPhoneCall::connectFloorChanged( QObject *object, const char *slot ) const
{
    if ( d ) {
        QObject::connect( d, SIGNAL(floorChanged(QPhoneCall)),
                          object, slot );
    }
}

/*!
    Deregisters a specified \a slot on \a object to receive notification of
    changes in the floor. The signal prototype is
    "floorChanged( const QPhoneCall& call )". \sa floorAvailable()
*/
void QPhoneCall::disconnectFloorChanged( QObject *object, const char *slot ) const
{
    if ( d ) {
        QObject::disconnect( d, SIGNAL(floorChanged(QPhoneCall)),
                             object, slot );
    }
}

/*!
    Returns true if the accept() action is valid in the call's current state.

    \sa accept()
*/
bool QPhoneCall::canAccept() const
{
    return ( d && ( d->actions & QPhoneCallImpl::Accept ) != 0 );
}

/*!
    Returns true if the hold() action is valid in the call's current state.

    \sa hold()
*/
bool QPhoneCall::canHold() const
{
    return ( d && ( d->actions & QPhoneCallImpl::Hold ) != 0 );
}

/*!
    Returns true if the activate() action is valid in the call's current state
    and for the specified \a scope.

    \sa activate()
*/
bool QPhoneCall::canActivate( QPhoneCall::Scope scope ) const
{
    if ( scope == Group )
        return ( d && ( d->actions & QPhoneCallImpl::ActivateGroup ) != 0 );
    else
        return ( d && ( d->actions & QPhoneCallImpl::ActivateCall ) != 0 );
}

/*!
    Returns true if the join() action is valid in the call's current state
    and for the specified \a detachSubscriber state.

    \sa join()
*/
bool QPhoneCall::canJoin( bool detachSubscriber ) const
{
    if ( detachSubscriber )
        return ( d && ( d->actions & QPhoneCallImpl::JoinAndDetach ) != 0 );
    else
        return ( d && ( d->actions & QPhoneCallImpl::Join ) != 0 );
}

/*!
    Returns true if DTMF tones can be sent with tone() in the call's
    current state.

    \sa tone()
*/
bool QPhoneCall::canTone() const
{
    return ( d && ( d->actions & QPhoneCallImpl::Tone ) != 0 );
}

/*!
    Returns true if the transfer() action is valid in the call's current state.

    \sa transfer()
*/
bool QPhoneCall::canTransfer() const
{
    return ( d && ( d->actions & QPhoneCallImpl::Transfer ) != 0 );
}

/*!
    Registers a specified \a slot on \a object to receive notification of
    pending tone changes. The signal prototype is \c{pendingTonesChanged( const QPhoneCall& call )}.

    \sa pendingTones()
*/
void QPhoneCall::connectPendingTonesChanged( QObject *object, const char *slot ) const
{
    if( d )
    {
        QObject::connect( d, SIGNAL(pendingTonesChanged(QPhoneCall)),
                                                                        object, slot );
    }
}

/*!
    Registers a specified \a slot on \a object to receive notification of
    QPhoneCall::State state changes. The signal prototype is \c{stateChanged( const QPhoneCall& call )}.

    More than one call may be affected by a state change in cases where
    an entire group of calls are hung up, put on hold, or swapped with
    an active call group.  The connectStateChanged() method does not allow
    such group operations to be tracked atomically.  The QPhoneCallManager::statesChanged()
    signal does allow atomic tracking of group operations across all calls
    that are active in the system.

    \sa state(), disconnectStateChanged(), QPhoneCallManager::statesChanged()
*/
void QPhoneCall::connectStateChanged( QObject *object, const char *slot ) const
{
    if ( d ) {
        QObject::connect( d, SIGNAL(stateChanged(QPhoneCall)),
                          object, slot );
    }
}

/*!
    Deregisters a specified \a slot on \a object to receive notification of
    QPhoneCall::States changes. The signal prototype is \c{stateChanged( const QPhoneCall& call )}.

    \sa state(), connectStateChanged()
*/

void QPhoneCall::disconnectStateChanged( QObject *object, const char *slot ) const
{
    if ( d ) {
        QObject::disconnect( d, SIGNAL(stateChanged(QPhoneCall)),
                          object, slot );
    }
}

/*!
    Registers a specified \a slot on \a object to receive notification of
    QPhoneCall::Request request failures.  The signal prototype is \c{requestFailed( const QPhoneCall& call, QPhoneCall::Request request )}.

    \sa join(), transfer(), disconnectRequestFailed()
*/

void QPhoneCall::connectRequestFailed( QObject *object, const char *slot ) const
{
    if ( d ) {
        QObject::connect( d, SIGNAL(requestFailed(QPhoneCall,QPhoneCall::Request)),
                          object, slot );
    }
}

/*!
    Deregisters a specified \a slot on \a object to receive notification of
    request failures.  The signal prototype is \c{requestFailed( const QPhoneCall& call, QPhoneCall::Request request )}.

    \sa join(), transfer(), connectRequestFailed()
*/

void QPhoneCall::disconnectRequestFailed( QObject *object, const char *slot ) const
{
    if ( d ) {
        QObject::disconnect( d, SIGNAL(requestFailed(QPhoneCall,QPhoneCall::Request)),
                          object, slot );
    }
}

/*!
    Registers the specified \a slot on \a object to receive notification of
    auxiliary values that are associated with a call.  The signal prototype is
    \c{notification( const QPhoneCall& call, QPhoneCall::Notification type, const QString& value )}.

    \sa QPhoneCall::Notification, disconnectNotification()
*/
void QPhoneCall::connectNotification( QObject *object, const char *slot ) const
{
    if ( d ) {
        QObject::connect( d, SIGNAL(notification(QPhoneCall,QPhoneCall::Notification,QString)),
                          object, slot );
    }
}

/*!
    Deregisters the specified \a slot on \a object to receive notification of
    auxiliary values.  The signal prototype is \c{notification( const QPhoneCall& call, QPhoneCall::Notification type, const QString& value )}.

    \sa QPhoneCall::Notification, connectNotification()
*/
void QPhoneCall::disconnectNotification( QObject *object, const char *slot ) const
{
    if ( d ) {
        QObject::disconnect( d, SIGNAL(notification(QPhoneCall,QPhoneCall::Notification,QString)),
                          object, slot );
    }
}

/*!
    \fn bool QPhoneCall::idle() const

    Returns true if the phone call is currently in the QPhoneCall::Idle state;
    false otherwise.

    \sa state()
*/

/*!
    \fn bool QPhoneCall::incoming() const

    Returns true if the phone call is currently in the QPhoneCall::Incoming state;
    false otherwise.

    \sa state(), missed()
*/

/*!
    \fn bool QPhoneCall::dialing() const

    Returns true if the phone call is currently in the QPhoneCall::Dialing or
    QPhoneCall::Alerting states; false otherwise.

    \sa state()
*/

/*!
    \fn bool QPhoneCall::established() const

    Returns true if the phone call is currently in the QPhoneCall::Connected or
    QPhoneCall::Hold states; false otherwise.

    \sa state(), connected(), onHold()
*/

/*!
    \fn bool QPhoneCall::connected() const

    Returns true if the phone call is currently in the QPhoneCall::Connected state;
    false otherwise.

    \sa state(), onHold(), established()
*/

/*!
    \fn bool QPhoneCall::onHold() const

    Returns true if the phone call is currently in the QPhoneCall::Hold state;
    false otherwise.

    \sa state(), connected(), established()
*/

/*!
    \fn bool QPhoneCall::dropped() const

    Returns true if the phone call is currently "dropped"; false otherwise.
    A "dropped" phone call is no longer connected due to normal call termination,
    network error, etc.  Use QPhoneCall::state() to determine the precise reason
    why the call was dropped.

    \sa state()
*/

/*!
    \fn bool QPhoneCall::missed() const

    Returns true if the phone call is currently in the QPhoneCall::Missed state.

    \sa state(), incoming()
*/


/*!
    Returns an I/O device for accessing the raw data of a non-voice call.
    When a data, fax or video call is made and the state() transitions
    to \c Connected, this device can be used to read or write data
    during the call.  The returned device will already be open in
    \c ReadWrite mode.

    Returns null if the call does not support access to the raw data
    of the call, or the call has not connected yet.

    The returned device object is guaranteed to have at least two
    signals: \c{readyRead()} and \c{closed()}.  The \c{readyRead()}
    signal indicates when data is available to be read, and the
    \c{closed()} signal indicates that the call has ended and the
    device is no longer valid.

    The returned device object will be deleted automatically when the
    phone call terminates.  The caller should not delete this object.
*/
QIODevice *QPhoneCall::device() const
{
    if ( d ) {
        if ( !d->device && d->dataPort != -1 ) {
            // We have a port number, but we haven't opened the device yet.
            d->device = new QSerialSocket
                ( "localhost", (quint16)( d->dataPort ) );
            if ( !d->device->open( QIODevice::ReadWrite ) ) {
                qLog(Modem) << "Could not open data socket to port"
                            << d->dataPort;
                delete d->device;
                d->device = 0;
                d->dataPort = -1;
            }
        }
        return d->device;
    } else {
        return 0;
    }
}

/*!
    Returns true if this \c QPhoneCall object is null.
*/
bool QPhoneCall::isNull() const
{
    return ( d == 0 );
}

/*!
    Parses the \a value from a DataStateUpdate notification into a
    set of QPhoneCall::DataState flags.

    \sa connectNotification()
*/
QPhoneCall::DataState QPhoneCall::parseDataState( const QString& value )
{
    return (QPhoneCall::DataState)( value.toInt() );
}

Q_IMPLEMENT_USER_METATYPE_ENUM(QPhoneCall::State)
Q_IMPLEMENT_USER_METATYPE_ENUM(QPhoneCall::Scope)
Q_IMPLEMENT_USER_METATYPE_ENUM(QPhoneCall::Request)
Q_IMPLEMENT_USER_METATYPE_ENUM(QPhoneCall::Notification)

QPhoneCallPrivate::QPhoneCallPrivate
        ( QPhoneCallManagerPrivate *manager,
          const QString& service, const QString& type,
          const QString& identifier )
{
    this->manager = manager;
    this->service = service;
    this->callType = type;
    this->identifier = identifier;

    count = 0;

    sentAllTones = true;
    toneTimer = new QTimer( this );
    toneTimer->setSingleShot( true );
    connect( toneTimer, SIGNAL(timeout()), this, SLOT(sendPendingTones()) );

    state = QPhoneCall::Idle;
    actions = QPhoneCallImpl::None;
    hasBeenConnected = false;
    dialedCall = false;
    haveFloor = false;
    floorAvailable = false;
    startTime = QDateTime::currentDateTime();

    device = 0;
    dataPort = -1;

    request = new QtopiaIpcAdaptor
        ( "QPE/Communications/QPhoneCallProvider/Request", this );

    response = new QtopiaIpcAdaptor
        ( "QPE/Communications/QPhoneCallProvider/Response", this );
    QtopiaIpcAdaptor::connect
        ( response, MESSAGE(stateChanged(QString,QPhoneCall::State,QString,QString,QString,int)),
          this, SLOT(callStateChanged(QString,QPhoneCall::State,QString,QString,QString,int)) );
    QtopiaIpcAdaptor::connect
        ( response, MESSAGE(requestFailed(QString,QPhoneCall::Request)),
          this, SLOT(callRequestFailed(QString,QPhoneCall::Request)) );
    QtopiaIpcAdaptor::connect
        ( response, MESSAGE(notification(QString,QPhoneCall::Notification,QString)),
          this, SLOT(callNotification(QString,QPhoneCall::Notification,QString)) );
    QtopiaIpcAdaptor::connect
        ( response, MESSAGE(floorChanged(QString,bool,bool)),
          this, SLOT(callFloorChanged(QString,bool,bool)) );
    QtopiaIpcAdaptor::connect
        ( response, MESSAGE(dataPort(QString,int)),
          this, SLOT(callDataPort(QString,int)) );
}

QPhoneCallPrivate::~QPhoneCallPrivate()
{
    if ( device )
        delete device;
}

void QPhoneCallPrivate::emitStateChanged()
{
    QDateTime ct = QDateTime::currentDateTime();
    if( state == QPhoneCall::Dialing || state == QPhoneCall::Incoming )
    {
        startTime = ct;
    }
    else if ( state == QPhoneCall::Alerting && startTime.isNull() ) {
        startTime = ct;
    }
    else if( ( state == QPhoneCall::Connected || state == QPhoneCall::Hold ) &&
             !hasBeenConnected )
    {
        //state has changed to connect for the first time
        connectTime = ct;
        hasBeenConnected = true;
    }
    else if( state >= QPhoneCall::HangupLocal )
    {
        if( pendingTones.length() ) {
            pendingTones = QString();
            emit pendingTonesChanged( QPhoneCall( this ) );
        }
        if( hasBeenConnected )
            endTime = ct;
        else
            //never connected, missed call or didn't pickup
            startTime = ct;


    }

    // Deal with pending DTMF tones.
    sendPendingTones();

    // Notify interested parties about the state change.
    emit stateChanged( QPhoneCall(this) );
}

void QPhoneCallPrivate::addPendingTones( const QString &_tones )
{
    QString tones = _tones;
    while ( tones.startsWith( QChar(',') ) )
        tones = tones.mid(1);
    pendingTones += tones;
    emit pendingTonesChanged( QPhoneCall( this ) );
    if( !pendingTones.isEmpty() )
        sentAllTones = false;
    if( state == QPhoneCall::Connected && !toneTimer->isActive() )
        sendPendingTones();
}

void QPhoneCallPrivate::abortCall()
{
    callStateChanged( identifier, QPhoneCall::OtherFailure, number, service, callType, 0 );
}

void QPhoneCallPrivate::callStateChanged
        ( const QString& identifier, QPhoneCall::State state,
          const QString& number, const QString& service,
          const QString& callType, int actions )
{
    Q_UNUSED(service);      // Used for new calls, not existing calls.
    Q_UNUSED(callType);     // Used for new calls, not existing calls.
    if ( identifier != this->identifier )
        return;
    this->state = state;
    this->actions = (QPhoneCallImpl::Actions)actions;
    if ( this->number.isEmpty() )
        this->number = number;

    // Increase the reference count temporarily.  We may have been called by a
    // QSlotInvoker that is only holding a pointer reference to us, but not a
    // QPhoneCall referenced-counted value.  If we don't do this, we may
    // end up deleting ourselves accidentally within "emitStateChanged()" even
    // before we are actually ready to be deleted.
    ++count;

    // Emit the state change locally.
    emitStateChanged();

    // Advertise the single state change through the manager's list interface.
    QList<QPhoneCall> list;
    list.append( QPhoneCall( this ) );
    emit manager->statesChanged( list );

    // Undo the reference count adjustment.  If this is truly the
    // last reference, then arrange for this object to be deleted later.
    if ( --count == 0 )
        deleteLater();
}

void QPhoneCallPrivate::callRequestFailed
        ( const QString& identifier, QPhoneCall::Request request )
{
    if ( identifier == this->identifier )
        emit requestFailed( QPhoneCall( this ), request );
}

void QPhoneCallPrivate::callNotification
        ( const QString& identifier, QPhoneCall::Notification type,
          const QString& value )
{
    if ( identifier == this->identifier )
        emit notification( QPhoneCall( this ), type, value );
}

void QPhoneCallPrivate::callFloorChanged
        ( const QString& identifier, bool haveFloor, bool floorAvailable )
{
    if ( identifier == this->identifier ) {
        this->haveFloor = haveFloor;
        this->floorAvailable = floorAvailable;
        emit floorChanged( QPhoneCall( this ) );
    }
}

void QPhoneCallPrivate::callDataPort( const QString& identifier, int port )
{
    if ( identifier == this->identifier )
        dataPort = port;
}

void QPhoneCallPrivate::sendPendingTones()
{
    if( state != QPhoneCall::Connected )
        return;
    if( sentAllTones ) {
        if(toneTimer->isActive())
            toneTimer->stop();
        return;
    }

    //send the next set of pending tones
    if( pendingTones.length() )
    {
        QString nextTones = pendingTones.left(1);
        pendingTones = pendingTones.mid(1);
        while ( pendingTones.startsWith( QChar(',') ) )
            pendingTones = pendingTones.mid(1);
        if (nextTones.length()) {
            request->send( MESSAGE(tone(QString,QString)),
                           identifier, nextTones );
        }
        toneTimer->stop();
        int timeout = QTOPIA_DTMF_TONE_TIME*nextTones.length()+QTOPIA_DTMF_PAUSE_TIME;
        toneTimer->start( timeout );
    }

    emit pendingTonesChanged( QPhoneCall( this ) );
    if( pendingTones.isEmpty() )
        sentAllTones = true;
}
