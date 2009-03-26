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

#include "atcallmanager.h"
#include <qatutils.h>
#include <qphonecallmanager.h>
#include <qmap.h>
#include <qtimer.h>
#include <QDebug>
#include "atoptions.h"
#include "atcommands.h"

// Information about a single call in the system.  We allocate
// our own call identifiers, because the ones from the back-end
// handlers are not very helpful for the AT interface.  e.g.
// call id 1 in the back end might be used for two different
// calls on two different handlers.  The AT interface needs to
// give these different ids.
class AtCallInfo
{
public:
    int         id;
    QPhoneCall  call;
    AtCallManager::CallState prevState;
    bool        incoming;
    bool        waiting;
    AtCallInfo *next;
};

class AtCallManagerPrivate
{
public:
    AtCallManagerPrivate()
    {
        gprs = false;
        incoming_call = false;
        calls = 0;
        ringTimer = 0;
        handler = 0;
    }
    ~AtCallManagerPrivate()
    {
        AtCallInfo *call, *next;
        call = calls;
        while ( call != 0 ) {
            next = call->next;
            delete call;
            call = next;
        }
    }

    QPhoneCallManager *callManager;
    AtCallInfo *calls;
    QTimer *ringTimer;
    QPhoneCall ringingCall;
    AtCommands *handler;

    QPhoneCall callForId( int id );
    int idForCall( const QPhoneCall& call, bool create=false );
    QPhoneCall callForState( QPhoneCall::State state );
    void removeCall( const QPhoneCall& call );
    bool haveCalls( QPhoneCall::State state );
    bool haveMultiple( QPhoneCall::State state );
    bool didStateChange( int id, AtCallManager::CallState state );
    void setWaiting( int id );

    bool incoming_call;  // whether the call is incoming (or outgoing)
    bool gprs;           // whether the call is a GPRS call
};

QPhoneCall AtCallManagerPrivate::callForId( int id )
{
    AtCallInfo *call = calls;
    while ( call != 0 ) {
        if ( call->id == id )
            return call->call;
        call = call->next;
    }
    return QPhoneCall();
}

int AtCallManagerPrivate::idForCall( const QPhoneCall& call, bool create )
{
    AtCallInfo *current = calls;
    AtCallInfo *insert, *prev;
    quint64 mask = 0;
    while ( current != 0 ) {
        if ( current->call == call )
            return current->id;
        mask |= ((quint64)1) << ( current->id - 1 );
        current = current->next;
    }
    if ( create ) {
        int id = 1;
        while ( id <= 64 ) {
            if ( ( mask & (((quint64)1) << (id - 1)) ) == 0 )
                break;
            ++id;
        }
        if ( id > 64 ) {
            // We have used up too many identifiers.  Time to stop!
            // 64 should be more than sufficient.  GSM modems rarely
            // support even 8 simultaneous calls.
            return 0;
        }
        current = new AtCallInfo();
        current->id = id;
        current->call = call;
        current->prevState = (AtCallManager::CallState)(-1);
        current->incoming = ( call.state() == QPhoneCall::Incoming );
        current->waiting = false;
        insert = calls;
        prev = 0;
        while ( insert != 0 && insert->id < id ) {
            prev = insert;
            insert = insert->next;
        }
        current->next = insert;
        if ( prev )
            prev->next = current;
        else
            calls = current;
        return id;
    }
    return 0;
}

QPhoneCall AtCallManagerPrivate::callForState( QPhoneCall::State state )
{
    AtCallInfo *call = calls;
    while ( call != 0 ) {
        if ( call->call.state() == state )
            return call->call;
        call = call->next;
    }
    return QPhoneCall();
}

void AtCallManagerPrivate::removeCall( const QPhoneCall& call )
{
    AtCallInfo *current = calls;
    AtCallInfo *prev = 0;
    while ( current != 0 ) {
        if ( current->call == call ) {
            if ( prev )
                prev->next = current->next;
            else
                calls = current->next;
            delete current;
            break;
        }
        prev = current;
        current = current->next;
    }
}

bool AtCallManagerPrivate::haveCalls( QPhoneCall::State state )
{
    AtCallInfo *current = calls;
    while ( current != 0 ) {
        if ( current->call.state() == state )
            return true;
        current = current->next;
    }
    return false;
}

bool AtCallManagerPrivate::haveMultiple( QPhoneCall::State state )
{
    AtCallInfo *current = calls;
    int count = 0;
    while ( current != 0 ) {
        if ( current->call.state() == state )
            ++count;
        current = current->next;
    }
    return ( count > 1 );
}

bool AtCallManagerPrivate::didStateChange
        ( int id, AtCallManager::CallState state )
{
    AtCallInfo *current = calls;
    while ( current != 0 ) {
        if ( current->id == id ) {
            AtCallManager::CallState prevState = current->prevState;
            current->prevState = state;
            return ( prevState != state );
        }
        current = current->next;
    }
    return false;
}

void AtCallManagerPrivate::setWaiting( int id )
{
    AtCallInfo *current = calls;
    while ( current != 0 ) {
        if ( current->id == id ) {
            current->waiting = true;
            return;
        }
        current = current->next;
    }
}

AtCallManager::AtCallManager( QObject *parent )
    : QObject( parent )
{
    d = new AtCallManagerPrivate();
    d->callManager = new QPhoneCallManager( this );
    connect( d->callManager, SIGNAL(newCall(QPhoneCall)),
             this, SLOT(newCall(QPhoneCall)) );
    connect( d->callManager, SIGNAL(statesChanged(QList<QPhoneCall>)),
             this, SLOT(callManagerStatesChanged()) );
}

AtCallManager::~AtCallManager()
{
    delete d;
}

void AtCallManager::setHandler( AtCommands *handler )
{
    d->handler = handler;
}


QAtResult::ResultCode AtCallManager::dial( const QString& _dialString )
{
    // Strip off T and P prefixes, as they are meaningless in GSM systems.
    QString dialString = _dialString;
    if ( dialString.isEmpty() ) {
        return QAtResult::Error;
    } else if ( dialString[0] == 'T' || dialString[0] == 't' ||
                dialString[0] == 'P' || dialString[0] == 'p' ) {
        dialString = dialString.mid(1);
    }

    // If we currently have a dialing call, then stop now.
    if ( d->haveCalls( QPhoneCall::Dialing ) ||
         d->haveCalls( QPhoneCall::Alerting ) )
        return QAtResult::Error;
    if ( !_dialString.endsWith( ";" ) ) {
        //TODO fax calls -> assuming data call only at this stage
        QPhoneCall dataCall = d->callManager->create( "Data" );
        if ( dataCall.isNull() ) return QAtResult::NoCarrier;
        AtOptions *options = d->handler->options();
        QDialOptions opts;
        opts.setNumber( dialString );
        if ( options->contextSet ) {
            // We have GPRS context information.
            opts.setContextId( 1 );
            opts.setPdpType( "IP" );
            opts.setApn( options->apn );
        }
        if ( options->cbstSpeed != -1 ) {
            // We have bearer information, so pass it along as well.
            opts.setGsmSpeed( options->cbstSpeed );
            opts.setBearer( (QDialOptions::Bearer)options->cbstName );
            opts.setTransparentMode
                ( (QDialOptions::TransparentMode)options->cbstCe );
        }
        if ( opts.contextId() != 0 )
        {
            d->gprs = true;
        }

        dataCall.dial( opts );
        newCall( dataCall );

        // Clear the data call options - the next call needs to start
        // again when setting up the necessary options.  Otherwise it
        // isn't possible to do a GPRS call, and then a normal data call.
        options->clearDataOptions();

        // The connect state will be reported later.
        return AtCallManager::Defer;
    }

    // Determine the call type to use.
    int len = dialString.length() - 1;
    QDialOptions::CallerId callerid = QDialOptions::DefaultCallerId;
    bool closedUserGroup = false;
    QString callType;
    if ( dialString.startsWith( "sip:", Qt::CaseInsensitive ) ) {
        callType = "VoIP";      // No tr
    } else {
        callType = "Voice";     // No tr

        // check for the 'g' and 'i' modifiers.
        if ( len > 0 &&
             ( dialString[len - 1] == 'g' || dialString[len - 1] == 'G' ) ) {
            dialString = dialString.left( len - 1 );
            closedUserGroup = true;
        } else {
            closedUserGroup = false;
        }

        if ( len > 0 && dialString[len - 1] == 'i' ) {
            dialString = dialString.left( len - 1 );
            callerid = QDialOptions::SendCallerId;
        } else if ( len > 0 && dialString[len - 1] == 'I' ) {
            dialString = dialString.left( len - 1 );
            callerid = QDialOptions::SuppressCallerId;
        } else {
            callerid = QDialOptions::DefaultCallerId;
        }
    }

    // Strip off the ';'
    dialString = dialString.left( len );
    if ( dialString.isEmpty() )
        return QAtResult::Error;

    // Create a new call and dial it if it is valid.
    QPhoneCall call = d->callManager->create( callType );
    if ( call.isNull() ) return QAtResult::NoCarrier;
    QDialOptions opts;
    opts.setNumber( dialString );
    opts.setCallerId( callerid );
    opts.setClosedUserGroup( closedUserGroup );
    call.dial( opts );

    // Force the call status tracking system to notice this call.
    newCall( call );

    // Indicate "OK" to the user while the dial is in progress.
    return QAtResult::OK;
}

QAtResult::ResultCode AtCallManager::accept()
{
    QPhoneCall call = d->callForState( QPhoneCall::Incoming );
    if ( ! call.isNull() ) {
        // TODO: data and fax calls.
        call.accept();
        return QAtResult::OK;
    } else {
        return QAtResult::NoCarrier;
    }
}

QAtResult::ResultCode AtCallManager::online()
{
    // TODO for data calls.
    return QAtResult::NoCarrier;
}

QAtResult::ResultCode AtCallManager::hangup()
{
    // Find a connected, dialing, or incoming call to be hung up.
    QPhoneCall call = d->callForState( QPhoneCall::Connected );
    if ( call.isNull() ) {
        call = d->callForState( QPhoneCall::Dialing );
        if ( call.isNull() ) {
            call = d->callForState( QPhoneCall::Alerting );
        }
        if ( call.isNull() ) {
            call = d->callForState( QPhoneCall::Incoming );
        }
    }
    if ( ! call.isNull() ) {
        call.hangup();
    }
    return QAtResult::OK;
}

QAtResult::ResultCode AtCallManager::hangup( int callID )
{
    QPhoneCall call = d->callForId( callID );
    if ( ! call.isNull() ) {
        call.hangup( QPhoneCall::CallOnly );
    }
    return QAtResult::OK;
}

QAtResult::ResultCode AtCallManager::hangupIncomingCall()
{
    QPhoneCall call = d->callForState( QPhoneCall::Incoming );
    if ( ! call.isNull() ) {
        call.hangup();
    }
    return QAtResult::OK;
}

QAtResult::ResultCode AtCallManager::hangupHeldCalls()
{
    QPhoneCall call = d->callForState( QPhoneCall::Hold );
    if ( ! call.isNull() ) {
        call.hangup();
    }
    return QAtResult::OK;
}

QAtResult::ResultCode AtCallManager::activateHeldCalls()
{
    QPhoneCall call = d->callForState( QPhoneCall::Hold );
    if ( ! call.isNull() ) {
        call.activate();
    }
    return QAtResult::OK;
}

QAtResult::ResultCode AtCallManager::activate( int callID )
{
    QPhoneCall call = d->callForId( callID );
    if ( ! call.isNull() ) {
        call.activate( QPhoneCall::CallOnly );
    }
    return QAtResult::OK;
}

QAtResult::ResultCode AtCallManager::join()
{
    QPhoneCall call = d->callForState( QPhoneCall::Connected );
    if ( ! call.isNull() ) {
        call.join();
    }
    return QAtResult::OK;
}

QAtResult::ResultCode AtCallManager::transfer()
{
    QPhoneCall call = d->callForState( QPhoneCall::Connected );
    if ( ! call.isNull() ) {
        call.join( true );
    }
    return QAtResult::OK;
}

QAtResult::ResultCode AtCallManager::transferIncoming( const QString& number )
{
    QPhoneCall call = d->callForState( QPhoneCall::Incoming );
    if ( ! call.isNull() ) {
        call.transfer( number );
    }
    return QAtResult::OK;
}

QAtResult::ResultCode AtCallManager::tone( const QString& value )
{
    QPhoneCall call = d->callForState( QPhoneCall::Connected );
    if ( ! call.isNull() ) {
        call.tone( value );
    }
    return QAtResult::OK;
}

void AtCallManager::newCall( const QPhoneCall &call )
{
    call.connectStateChanged( this, SLOT(callStateChanged(QPhoneCall)) );
    callStateChanged( call );
}

void AtCallManager::callStateChanged( const QPhoneCall& call )
{
    int id = d->idForCall( call );
    bool newCall = false;
    if ( !id ) {
        // This is a new call that we've never seen before.
        // Ignore it if it is in the dropped state as it is
        // about to disappear anyway.
        if ( call.dropped() )
            return;
        id = d->idForCall( call, true );
        newCall = true;
    }

    // Determine how to report the state.
    CallState reportState(AtCallManager::CallIdle);
    if ( call == d->ringingCall && call.state() != QPhoneCall::Incoming ) {
        // The call that was ringing is no longer "Incoming", so stop ringing.
        d->ringingCall = QPhoneCall();
        if ( d->ringTimer ) {
            delete d->ringTimer;
            d->ringTimer = 0;
        }
    }
    switch ( call.state() ) {

        case QPhoneCall::Incoming:
        {
            d->incoming_call = true;
            reportState = CallWaiting;
            if ( newCall ) {
                // If there are connected or held calls, then this
                // call is waiting, otherwise it is simply ringing.
                if ( d->haveCalls( QPhoneCall::Connected ) ||
                     d->haveCalls( QPhoneCall::Hold ) ) {
                    d->setWaiting( id );
                    emit callWaiting( call.number(), call.callType() );
                } else {
                    // Send "RING" notifications every 2 seconds until
                    // the call stops being in the "Incoming" state.
                    d->ringingCall = call;
                    if ( !d->ringTimer ) {
                        d->ringTimer = new QTimer( this );
                        connect( d->ringTimer, SIGNAL(timeout()),
                                 this, SLOT(repeatRing()) );
                    }
                    d->ringTimer->start( 2000 );
                    emit ring( call.number(), call.callType() );
                }
            }
        }
        break;

        case QPhoneCall::Dialing:
        case QPhoneCall::Alerting:
        {
            d->incoming_call = false;
            reportState = CallCalling;
        }
        break;

        case QPhoneCall::Connected:
        {
            reportState = CallActive;

            // first, do any +COLP we may need to present.
            if ( !d->incoming_call ) {
                // we have a new outgoing connection.
                emit outgoingConnected( call.number() );
            }

            if ( call.callType() == "Data" ) {
                // do service reporting control +CR - note that this is for data calls only (automatic at this stage)
                AtOptions *options = d->handler->options();
                bool asynchronous = ( (QDialOptions::Bearer)options->cbstName == QDialOptions::DataCircuitAsyncUDI ||
                                      (QDialOptions::Bearer)options->cbstName == QDialOptions::PadAccessUDI ||
                                      (QDialOptions::Bearer)options->cbstName == QDialOptions::DataCircuitAsyncRDI ||
                                      (QDialOptions::Bearer)options->cbstName == QDialOptions::PadAccessRDI );
                bool transparent = ( (QDialOptions::TransparentMode)options->cbstCe == QDialOptions::NonTransparent );
                emit dialingOut( asynchronous, transparent, d->gprs );

                //a data call is always deferred
                QIODevice* dev = call.device();
                d->handler->frontEnd()->setDataSource( dev );
                emit deferredResult( d->handler, QAtResult::Connect );
            }

        }
        break;

        case QPhoneCall::Hold:
        {
            reportState = CallHold;
        }
        break;

        case QPhoneCall::HangupRemote:
        {
            // If the call was hung up remotely and it was connected
            // then assume that we have "NO CARRIER" on it.
            // We only do this if there are no other active calls.
            if ( call.hasBeenConnected() &&
                 !d->haveCalls( QPhoneCall::Connected ) &&
                 !d->haveCalls( QPhoneCall::Hold ) ) {
                emit noCarrier();
            }
        }
        // Fall through to the next case.

        case QPhoneCall::Idle:
        case QPhoneCall::HangupLocal:
        case QPhoneCall::Missed:
        case QPhoneCall::NetworkFailure:
        case QPhoneCall::OtherFailure:
        case QPhoneCall::ServiceHangup:
        {
            // Call has finished so report as idle, and then remove it.
            reportState = CallIdle;
            d->removeCall( call );
        }
        break;

    }

    // Notify higher layers of the change.
    if ( reportState == CallIdle || d->didStateChange( id, reportState ) ) {
        emit stateChanged( id, reportState, call.number(), call.callType() );
    }

    notifyCallStates();
}

void AtCallManager::notifyCallStates()
{
    // Report the current "on call" state.
    if ( d->haveCalls( QPhoneCall::Connected ) ||
         d->haveCalls( QPhoneCall::Hold ) ) {
        emit setOnCall( true );
    } else {
        emit setOnCall( false );
    }

    // Report the current "call setup" state.
    if ( d->haveCalls( QPhoneCall::Incoming ) ) {
        emit setCallSetup( IncomingCallSetup );
    } else if ( d->haveCalls( QPhoneCall::Dialing ) ||
                d->haveCalls( QPhoneCall::Alerting ) ) {
        emit setCallSetup( OutgoingCallSetup );
    } else {
        emit setCallSetup( NoCallSetup );
    }

    // Report the current "call hold" state.
    if ( d->haveCalls( QPhoneCall::Connected ) ) {
        if ( d->haveCalls( QPhoneCall::Hold ) ) {
            emit setCallHold( CallsActiveAndHeld );
        } else {
            emit setCallHold( NoCallsHeld );
        }
    } else if ( d->haveCalls( QPhoneCall::Hold ) ) {
        emit setCallHold( CallsHeldOnly );
    } else {
        emit setCallHold( NoCallsHeld );
    }
}

void AtCallManager::repeatRing()
{
    emit ring( d->ringingCall.number(), d->ringingCall.callType() );
}

void AtCallManager::callManagerStatesChanged()
{
    disconnect( d->callManager, SIGNAL(statesChanged(QList<QPhoneCall>)),
             this, SLOT(callManagerStatesChanged()) );
    emit callStateInitialized();
}

int AtCallManager::clccCallType( const QString& callType )
{
    if ( callType == "Video" )          // No tr
        return 9;
    else if ( callType == "Data" )      // No tr
        return 1;
    else if ( callType == "Fax" )       // No tr
        return 2;
    else
        return 0;
}

int AtCallManager::numCallType( const QString& callType )
{
    if ( callType == "Video" )          // No tr
        return 32;
    else if ( callType == "Data" )      // No tr
        return 2;
    else if ( callType == "Fax" )       // No tr
        return 4;
    else
        return 1;
}

QString AtCallManager::strCallType( const QString& callType )
{
    if ( callType == "Video" )          // No tr
        return "ASYNC";
    else if ( callType == "Data" )      // No tr
        return "ASYNC";
    else if ( callType == "Fax" )       // No tr
        return "FAX";
    else
        return "VOICE";
}

QStringList AtCallManager::formatCallList()
{
    QStringList list;
    QString line;
    AtCallInfo *current = d->calls;
    while ( current != 0 ) {
        line = "+CLCC: " + QString::number( current->id );
        if ( current->incoming )
            line += ",1";
        else
            line += ",0";
        switch ( current->call.state() ) {
            case QPhoneCall::Incoming:
            {
                if ( current->waiting )
                    line += ",5";
                else
                    line += ",4";
            }
            break;

            case QPhoneCall::Dialing:       line += ",2"; break;
            case QPhoneCall::Alerting:      line += ",3"; break;
            case QPhoneCall::Connected:     line += ",0"; break;
            case QPhoneCall::Hold:          line += ",1"; break;

            case QPhoneCall::Idle:
            case QPhoneCall::HangupLocal:
            case QPhoneCall::HangupRemote:
            case QPhoneCall::Missed:
            case QPhoneCall::NetworkFailure:
            case QPhoneCall::OtherFailure:
            case QPhoneCall::ServiceHangup:
            {
                // Call is dead - don't add it to the list.
                current = current->next;
                continue;
            }
        }
        line += "," + QString::number
                ( clccCallType( current->call.callType() ) );
        if ( current->call.established() ) {
            // Determine if the call appears to be within a multiparty call.
            // This isn't 100% accurate, as it won't work across GSM and VoIP
            // calls but it should be close enough.
            if ( d->haveMultiple( current->call.state() ) )
                line += ",1";
            else
                line += ",0";
        } else {
            line += ",0";
        }
        line += "," + QAtUtils::encodeNumber( current->call.number() );
        list += line;
        current = current->next;
    }
    return list;
}

bool AtCallManager::ringing() const
{
    return d->haveCalls( QPhoneCall::Incoming );
}

bool AtCallManager::callInProgress() const
{
    return d->haveCalls( QPhoneCall::Connected ) ||
           d->haveCalls( QPhoneCall::Dialing ) ||
           d->haveCalls( QPhoneCall::Alerting ) ||
           d->haveCalls( QPhoneCall::Hold );
}
