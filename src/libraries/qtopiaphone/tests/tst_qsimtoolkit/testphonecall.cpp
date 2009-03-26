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

//QTEST_SKIP_TEST_DOC

#include "testphonecall.h"
#include <QTimer>

TestPhoneCallImpl::TestPhoneCallImpl
        ( QPhoneCallProvider *provider, const QString& identifier,
          const QString& callType )
    : QPhoneCallImpl( provider, identifier, callType )
{
}

TestPhoneCallImpl::~TestPhoneCallImpl()
{
}

void TestPhoneCallImpl::dial( const QDialOptions& options )
{
    provider().beginStateTransaction();
    if ( hasCall( QPhoneCall::Dialing ) ) {
        // There is already a dialing call.
        setState( QPhoneCall::HangupLocal );
        provider().endStateTransaction();
        return;
    }
    if ( hasCall( QPhoneCall::Connected ) ) {
        if ( hasCall( QPhoneCall::Hold ) ) {
            // No free slots for the dialing call.
            setState( QPhoneCall::HangupLocal );
            provider().endStateTransaction();
            return;
        }
        findCall( QPhoneCall::Connected )->setState( QPhoneCall::Hold );
    }
    setNumber( options.number() );
    setState( QPhoneCall::Dialing );
    provider().endStateTransaction();
    QTimer::singleShot( 1000, this, SLOT(connectTimeout()) );
}

void TestPhoneCallImpl::hangup( QPhoneCall::Scope )
{
    if ( state() < QPhoneCall::HangupLocal )
        setState( QPhoneCall::HangupLocal );
}

void TestPhoneCallImpl::accept()
{
    // Incoming calls not simulated yet.
}

void TestPhoneCallImpl::hold()
{
    if ( state() == QPhoneCall::Connected && !hasCall( QPhoneCall::Connected ) )
        setState( QPhoneCall::Hold );
}

void TestPhoneCallImpl::activate( QPhoneCall::Scope )
{
    if ( state() == QPhoneCall::Hold ) {
        provider().beginStateTransaction();
        QPhoneCallImpl *other = findCall( QPhoneCall::Connected );
        if ( other )
            other->setState( QPhoneCall::Hold );
        setState( QPhoneCall::Connected );
        provider().endStateTransaction();
    }
}

void TestPhoneCallImpl::join( bool )
{
    // Joins are not supported in this implementation.
    emit requestFailed( QPhoneCall::JoinFailed );
}

void TestPhoneCallImpl::tone( const QString& )
{
    // Nothing to do here.
}

void TestPhoneCallImpl::transfer( const QString& )
{
    // Just hang up the call, simulating the transfer.
    hangup( QPhoneCall::CallOnly );
}

void TestPhoneCallImpl::requestFloor( int )
{
    // Nothing to do here.
}

void TestPhoneCallImpl::releaseFloor()
{
    // Nothing to do here.
}

void TestPhoneCallImpl::setState( QPhoneCall::State state )
{
    switch ( state ) {
        case QPhoneCall::Dialing:
        case QPhoneCall::Alerting:
            setActions( None ); break;
        case QPhoneCall::Connected:
            setActions( Hold | Tone  ); break;
        case QPhoneCall::Hold:
            setActions( ActivateCall | ActivateGroup ); break;
        case QPhoneCall::Incoming:
            setActions( Accept | Transfer ); break;
        default: setActions( None ); break;
    }
    QPhoneCallImpl::setState( state );
}

void TestPhoneCallImpl::connectTimeout()
{
    if ( state() == QPhoneCall::Dialing )
        setState( QPhoneCall::Connected );
}

QPhoneCallImpl *TestPhoneCallImpl::findCall( QPhoneCall::State state )
{
    QList<QPhoneCallImpl *> calls = provider().calls();
    foreach ( QPhoneCallImpl *call, calls ) {
        if ( call->state() == state )
            return call;
    }
    return 0;
}

bool TestPhoneCallImpl::hasCall( QPhoneCall::State state )
{
    return ( findCall( state ) != 0 );
}

TestPhoneCallProvider::TestPhoneCallProvider( const QString& service, QObject *parent )
    : QPhoneCallProvider( service, parent )
{
    QStringList types;
    types += "Voice";
    types += "Data";
    types += "Fax";
    types += "Video";
    types += "IP";
    setCallTypes( types );
}

TestPhoneCallProvider::~TestPhoneCallProvider()
{
}

QPhoneCallImpl *TestPhoneCallProvider::create
        ( const QString& identifier, const QString& callType )
{
    return new TestPhoneCallImpl( this, identifier, callType );
}
