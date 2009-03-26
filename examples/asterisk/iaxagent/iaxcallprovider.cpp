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

#include "iaxcallprovider.h"
#include "iaxtelephonyservice.h"
#include "iaxnetworkregistration.h"
#include "iaxclient.h"
#include <qtopialog.h>
#include <QUuid>
#include <QSettings>

IaxPhoneCall::IaxPhoneCall
        ( IaxCallProvider *provider, const QString& identifier,
          const QString& callType, int callNo )
    : QPhoneCallImpl( provider, identifier, callType )
{
    this->provider = provider;
    this->callNo = callNo;
}

IaxPhoneCall::~IaxPhoneCall()
{
}

void IaxPhoneCall::dial( const QDialOptions& options )
{
    qLog(VoIP) << "IaxPhoneCall::dial(" << options.number() << ")";

    // Compose the full URI for the call request.
    QString callUri = provider->callUri();
    if ( callUri.isEmpty() ) {
        // No registration, so cannot make a call at this time.
        provider->beginStateTransaction();
        setActions( QPhoneCallImpl::None );
        setState( QPhoneCall::NetworkFailure );
        provider->endStateTransaction();
        return;
    }
    callUri += "/" + options.number();

    // Start the calling sequence.
    provider->beginStateTransaction();
    if ( provider->calls().size() >= IAXAGENT_MAX_CALLS ) {
        // Too many calls currently in use.
        setState( QPhoneCall::OtherFailure );
    } else {
        // Put the current call on hold and start a new one.
        provider->putActiveOnHold();
        setNumber( options.number() );
        callNo = iaxc_first_free_call();
        iaxc_call( callUri.toUtf8().data() );
        setActions( QPhoneCallImpl::None );
        setState( QPhoneCall::Dialing );
    }
    provider->endStateTransaction();
}

void IaxPhoneCall::hangup( QPhoneCall::Scope )
{
    qLog(VoIP) << "IaxPhoneCall::hangup()";
    provider->beginStateTransaction();
    if ( state() == QPhoneCall::Incoming && callNo != -1 ) {

        // Reject the incoming call.
        iaxc_reject_call_number( callNo );
        setActions( QPhoneCallImpl::None );
        setState( QPhoneCall::HangupLocal );

    } else if ( state() < QPhoneCall::HangupLocal && callNo != -1 ) {

        // The iaxclient API only allows us to dump the selected call.
        int selected = iaxc_selected_call();
        if ( callNo == selected ) {
            // Hanging up the selected call is easy.
            iaxc_dump_call();
            iaxc_select_call( -1 );
        } else if ( selected >= 0 ) {
            // Another call is selected, so we need to deselect it first.
            iaxc_select_call( callNo );
            iaxc_dump_call();
            iaxc_select_call( selected );
        } else {
            // No call selected, so select this one and then kill it.
            iaxc_select_call( callNo );
            iaxc_dump_call();
            iaxc_select_call( -1 );
        }

        // Advertise the state change.
        setActions( QPhoneCallImpl::None );
        setState( QPhoneCall::HangupLocal );

    }
    provider->endStateTransaction();
}

void IaxPhoneCall::accept()
{
    qLog(VoIP) << "IaxPhoneCall::accept()";
    provider->beginStateTransaction();
    if ( state() == QPhoneCall::Incoming && callNo != -1 ) {
        provider->putActiveOnHold();
        iaxc_answer_call( callNo );
        iaxc_select_call( callNo );
        setActions( QPhoneCallImpl::Hold | QPhoneCallImpl::Tone );
        setState( QPhoneCall::Connected );
    }
    provider->endStateTransaction();
}

void IaxPhoneCall::hold()
{
    qLog(VoIP) << "IaxPhoneCall::hold()";
    provider->beginStateTransaction();
    if ( state() == QPhoneCall::Connected && callNo != -1 ) {
        iaxc_quelch( callNo, 0 );
        iaxc_select_call( -1 );
        setActions( QPhoneCallImpl::ActivateCall |
                    QPhoneCallImpl::ActivateGroup );
        setState( QPhoneCall::Hold );
    }
    provider->endStateTransaction();
}

void IaxPhoneCall::activate( QPhoneCall::Scope )
{
    qLog(VoIP) << "IaxPhoneCall::activate()";
    provider->beginStateTransaction();
    if ( state() == QPhoneCall::Hold && callNo != -1 ) {
        provider->putActiveOnHold();
        iaxc_unquelch( callNo );
        iaxc_select_call( callNo );
        setActions( QPhoneCallImpl::Hold | QPhoneCallImpl::Tone );
        setState( QPhoneCall::Connected );
    }
    provider->endStateTransaction();
}

void IaxPhoneCall::tone( const QString& tones )
{
    qLog(VoIP) << "IaxPhoneCall::tone(" << tones << ")";
    if ( state() == QPhoneCall::Connected && callNo != -1 ) {
        for ( int posn = 0; posn < tones.length(); ++posn ) {
            int ch = tones[posn].unicode();
            if ( ( ch >= '0' && ch <= '9' ) || ch == '*' || ch == '#' )
                iaxc_send_dtmf( (char)ch );
        }
    }
}

void IaxPhoneCall::transfer( const QString& number )
{
    qLog(VoIP) << "IaxPhoneCall::transfer(" << number << ")";
    if ( state() == QPhoneCall::Incoming && callNo != -1 )
        iaxc_blind_transfer_call( callNo, number.toUtf8().data() );
}

void IaxPhoneCall::stateEvent( struct iaxc_ev_call_state *e )
{
    provider->beginStateTransaction();
    if ( e->state == IAXC_CALL_STATE_FREE && callNo != -1 &&
         state() < QPhoneCall::HangupLocal ) {

        // Call has been hung up by the remote side.
        setActions( QPhoneCallImpl::None );
        if ( state() == QPhoneCall::Incoming )
            setState( QPhoneCall::Missed );
        else
            setState( QPhoneCall::HangupRemote );

    }
    if ( ( e->state & IAXC_CALL_STATE_COMPLETE ) != 0 && callNo != -1 &&
         state() == QPhoneCall::Dialing ) {

        // Dialing call has connected.
        provider->putActiveOnHold();
        if ( iaxc_selected_call() != callNo )
            iaxc_select_call( callNo );
        setActions( QPhoneCallImpl::Hold | QPhoneCallImpl::Tone );
        setState( QPhoneCall::Connected );

    }
    if ( e->state == IAXC_CALL_STATE_FREE ) {
        // Make sure the call number is free when iaxclient's data goes away.
        callNo = -1;
    }
    provider->endStateTransaction();
}

IaxCallProvider::IaxCallProvider( IaxTelephonyService *service )
    : QPhoneCallProvider( service->service(), service )
{
    iaxservice = service;

    // Register the "Asterisk" call type for this provider.
    setCallTypes( QStringList( "Asterisk" ) );

    // Load the initial caller id configuration.
    updateCallerIdConfig();
}

IaxCallProvider::~IaxCallProvider()
{
}

void IaxCallProvider::stateEvent( struct iaxc_ev_call_state *e )
{
    IaxPhoneCall *call = fromCallNo( e->callNo );
    if ( call ) {
        // State change on a known call.
        call->stateEvent( e );
    } else if ( ( e->state & IAXC_CALL_STATE_RINGING ) != 0 ) {
        // Newly arrived incoming call.
        beginStateTransaction();
        QString identifier = QUuid::createUuid().toString();
        IaxPhoneCall *call = new IaxPhoneCall
            ( this, identifier, "Asterisk", e->callNo );
        call->setNumber( e->remote );
        call->setActions( QPhoneCallImpl::Transfer );
        call->setState( QPhoneCall::Incoming );
        endStateTransaction();
    }
}

void IaxCallProvider::putActiveOnHold()
{
    QList<QPhoneCallImpl *> list = calls();
    foreach ( QPhoneCallImpl *call, list ) {
        if ( call->state() == QPhoneCall::Connected )
            call->hold();
    }
}

void IaxCallProvider::endStateTransaction()
{
    // Force a poll on the iaxclient library because it may need to
    // turn audio on or off based on the state changes we just made.
    iaxservice->serviceIaxClient();

    // Do the normal state transaction end.
    QPhoneCallProvider::endStateTransaction();
}

void IaxCallProvider::updateCallerIdConfig()
{
    QSettings config( "Trolltech", "Asterisk" );
    config.beginGroup( "CallerId" );
    QString name = config.value( "Name" ).toString();
    QString number = config.value( "Number" ).toString();
    iaxc_set_callerid( name.toUtf8().data(), number.toUtf8().data() );
}

QString IaxCallProvider::callUri() const
{
    IaxNetworkRegistration *reg;
    reg = qobject_cast<IaxNetworkRegistration *>
        ( iaxservice->interface<QNetworkRegistration>() );
    if ( reg )
        return reg->callUri();
    else
        return QString();
}

QPhoneCallImpl *IaxCallProvider::create
        ( const QString& identifier, const QString& callType )
{
    return new IaxPhoneCall( this, identifier, callType, -1 );
}

IaxPhoneCall *IaxCallProvider::fromCallNo( int callNo )
{
    QList<QPhoneCallImpl *> list = calls();
    foreach ( QPhoneCallImpl *call, list ) {
        IaxPhoneCall *iaxcall = qobject_cast<IaxPhoneCall *>( call );
        if ( iaxcall && iaxcall->callNo == callNo )
            return iaxcall;
    }
    return 0;
}
