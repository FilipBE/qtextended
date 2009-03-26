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

#include "phoneserverdummymodem.h"
#include <qvaluespace.h>
#include <QSignalSourceProvider>
#include <qtimer.h>
#include "phoneserver.h"

QPhoneCallDummy::QPhoneCallDummy
        ( QPhoneCallProvider *provider, const QString& identifier,
          const QString& callType )
    : QPhoneCallImpl( provider, identifier, callType )
{
    // Nothing to do here.
}

QPhoneCallDummy::~QPhoneCallDummy()
{
    // Nothing to do here.
}

void QPhoneCallDummy::dial( const QDialOptions& options )
{
    // Change to the "Dialing" state and notify everyone who is interested.
    setNumber( options.number() );
    setState( QPhoneCall::Dialing );

    // Start a timer to transition to the hangup state after 3 seconds.
    QTimer::singleShot( 3000, this, SLOT(dialTimeout()) );
}

void QPhoneCallDummy::hangup( QPhoneCall::Scope )
{
    setState( QPhoneCall::HangupLocal );
}

void QPhoneCallDummy::accept()
{
    // Not supported.
}

void QPhoneCallDummy::hold()
{
    // Not supported.
    emit requestFailed( QPhoneCall::HoldFailed );
}

void QPhoneCallDummy::activate( QPhoneCall::Scope )
{
    // Not supported.
}

void QPhoneCallDummy::join( bool )
{
    // Not supported.
}

void QPhoneCallDummy::tone( const QString& )
{
    // Not supported.
}

void QPhoneCallDummy::transfer( const QString& )
{
    // Not supported.
}

void QPhoneCallDummy::dialTimeout()
{
    // If we are still dialing, then switch into the hangup state.
    if ( state() == QPhoneCall::Dialing )
        setState( QPhoneCall::HangupLocal );
}

QPhoneCallProviderDummy::QPhoneCallProviderDummy
        ( const QString& service, QObject *parent )
    : QPhoneCallProvider( service, parent )
{
    setCallTypes( QStringList( "Voice" ) );
}

QPhoneCallProviderDummy::~QPhoneCallProviderDummy()
{
}

QPhoneCallImpl *QPhoneCallProviderDummy::create
        ( const QString& identifier, const QString& callType )
{
    return new QPhoneCallDummy( this, identifier, callType );
}

QPhoneBookDummy::QPhoneBookDummy( const QString& service, QObject *parent )
    : QPhoneBook( service, parent, QCommInterface::Server )
{
    fixedDialingEnabled = false;
}

QPhoneBookDummy::~QPhoneBookDummy()
{
}

void QPhoneBookDummy::getEntries( const QString& store )
{
    QList<QPhoneBookEntry> list;
    if ( store == "SM" ) {
        QList<QPhoneBookEntry>::ConstIterator iter;
        for ( iter = ents.begin(); iter != ents.end(); ++iter ) {
            if ( ! (*iter).number().isEmpty() )
                list += *iter;
        }
    }
    emit entries( store, list );
}

void QPhoneBookDummy::add( const QPhoneBookEntry& entry, const QString& store, bool flush )
{
    if ( store != "SM" ) {
        if ( flush )
            getEntries( store );
        return;
    }

    int index;
    for ( index = 0; index < ents.size(); ++index ) {
        if ( ents[index].number().isEmpty() )
            break;
    }

    QPhoneBookEntry newEntry( entry );
    newEntry.setIndex( (uint)index );

    if ( index < ents.size() ) {
        ents[index] = newEntry;
    } else {
        ents += newEntry;
    }

    if ( flush )
        getEntries( store );
}

void QPhoneBookDummy::remove( uint index, const QString& store, bool flush )
{
    if ( store != "SM" ) {
        if ( flush )
            getEntries( store );
        return;
    }

    if ( ((int)index) < ents.size() ) {
        ents[(int)index].setNumber( "" );
    }

    if ( flush )
        getEntries( store );
}

void QPhoneBookDummy::update( const QPhoneBookEntry& entry, const QString& store, bool flush )
{
    if ( store != "SM" ) {
        if ( flush )
            getEntries( store );
        return;
    }

    int index = (int)entry.index();
    if ( index < ents.size() ) {
        ents[index] = entry;
    } else {
        add( entry, store, flush );
        return;
    }

    if ( flush )
        getEntries( store );
}

void QPhoneBookDummy::flush( const QString& store )
{
    getEntries( store );
}

void QPhoneBookDummy::setPassword( const QString&, const QString& )
{
    // Nothing to do here.
}

void QPhoneBookDummy::clearPassword( const QString& )
{
    // Nothing to do here.
}

void QPhoneBookDummy::requestLimits( const QString& store )
{
    QPhoneBookLimits l;
    l.setNumberLength( 20 );
    l.setTextLength( 18 );
    l.setFirstIndex( 1 );
    l.setLastIndex( 150 );
    emit limits( store, l );
}

void QPhoneBookDummy::requestFixedDialingState()
{
    emit fixedDialingState( fixedDialingEnabled );
}

void QPhoneBookDummy::setFixedDialingState( bool enabled, const QString& )
{
    fixedDialingEnabled = enabled;
    emit setFixedDialingStateResult( QTelephony::OK );
}

QNetworkRegistrationDummy::QNetworkRegistrationDummy
        ( const QString& service, QObject *parent )
    : QNetworkRegistrationServer( service, parent )
{
    QTimer::singleShot( 500, this, SLOT(searching()) );
    QTimer::singleShot( 5000, this, SLOT(home()) );
    QTimer::singleShot( 200, this, SLOT(initDone()) );

    updateRegistrationState( QTelephony::RegistrationNone );
}

QNetworkRegistrationDummy::~QNetworkRegistrationDummy()
{
}

void QNetworkRegistrationDummy::setCurrentOperator
        ( QTelephony::OperatorMode, const QString&, const QString&)
{
    emit setCurrentOperatorResult( QTelephony::OK );
}

void QNetworkRegistrationDummy::requestAvailableOperators()
{
    QList<QNetworkRegistration::AvailableOperator> opers;
    QNetworkRegistration::AvailableOperator oper;
    oper.availability = QTelephony::OperatorAvailable;
    oper.name = "Qt Extended";       // No tr
    oper.id = "0QtExtended";       // No tr
    oper.technology = "GSM";       // No tr
    opers.append( oper );
    emit availableOperators( opers );
}

void QNetworkRegistrationDummy::searching()
{
    updateRegistrationState( QTelephony::RegistrationSearching );
}

void QNetworkRegistrationDummy::home()
{
    updateRegistrationState( QTelephony::RegistrationHome );
    updateCurrentOperator( QTelephony::OperatorModeAutomatic,
                           "0QtExtended", "Qt Extended", "GSM" );    // No tr
}

void QNetworkRegistrationDummy::initDone()
{
    updateInitialized( true );
}

QSimInfoDummy::QSimInfoDummy( const QString& service, QObject *parent )
    : QSimInfo( service, parent, QCommInterface::Server )
{
    setIdentity( "9876543210" );
}

QSimInfoDummy::~QSimInfoDummy()
{
}

QPhoneRfFunctionalityDummy::QPhoneRfFunctionalityDummy
        ( const QString& service, QObject *parent )
    : QPhoneRfFunctionality( service, parent, QCommInterface::Server )
{
}

QPhoneRfFunctionalityDummy::~QPhoneRfFunctionalityDummy()
{
}

void QPhoneRfFunctionalityDummy::forceLevelRequest()
{
    setValue( "level", qVariantFromValue( Full ) );
    emit levelChanged();
}

void QPhoneRfFunctionalityDummy::setLevel( QPhoneRfFunctionality::Level level )
{
    Q_UNUSED(level);
}

QPinManagerDummy::QPinManagerDummy( const QString& service, QObject *parent )
    : QPinManager( service, parent, Server )
{
}

QPinManagerDummy::~QPinManagerDummy()
{
}

void QPinManagerDummy::querySimPinStatus()
{
    emit pinStatus( "READY", Valid, QPinOptions() );
}

void QPinManagerDummy::enterPin( const QString& type, const QString& pin )
{
    Q_UNUSED(pin);
    emit pinStatus( type, Valid, QPinOptions() );
}

void QPinManagerDummy::enterPuk
        ( const QString& type, const QString& puk, const QString& newPin )
{
    Q_UNUSED(puk);
    Q_UNUSED(newPin);
    emit pinStatus( type, Valid, QPinOptions() );
}

void QPinManagerDummy::cancelPin( const QString& type )
{
    Q_UNUSED(type);
}

void QPinManagerDummy::changePin
        ( const QString& type, const QString& oldPin, const QString& newPin )
{
    Q_UNUSED(type);
    Q_UNUSED(oldPin);
    Q_UNUSED(newPin);
    emit changePinResult( type, true );
}

QTelephonyServiceDummy::QTelephonyServiceDummy
        ( const QString& service, QObject *parent )
    : QTelephonyService( service, parent )
{
    if ( service == "modem" ) {
        // Fake out signal quality values if this is the modem service.
        QSignalSourceProvider* prov = new QSignalSourceProvider( QLatin1String("modem"),  QLatin1String("modem"), this );
        prov->setAvailability( QSignalSource::Available );
        prov->setSignalStrength( 100 );
    }
}

QTelephonyServiceDummy::~QTelephonyServiceDummy()
{
}

void QTelephonyServiceDummy::initialize()
{
    if ( !supports<QSimInfo>() )
        addInterface( new QSimInfoDummy( service(), this ) );

    if ( !supports<QPhoneBook>() )
        addInterface( new QPhoneBookDummy( service(), this ) );

    if ( !supports<QNetworkRegistration>() )
        addInterface( new QNetworkRegistrationDummy( service(), this ) );

    if ( !supports<QPhoneRfFunctionality>() )
        addInterface( new QPhoneRfFunctionalityDummy( service(), this ) );

    if ( !supports<QPinManager>() )
        addInterface( new QPinManagerDummy( service(), this ) );

    if ( !callProvider() )
        setCallProvider( new QPhoneCallProviderDummy( service(), this ) );

    QTelephonyService::initialize();
}

class DummyTelephonyServiceFactory : public TelephonyServiceFactory 
{
    Q_OBJECT
public: 
    DummyTelephonyServiceFactory( QObject *parent = 0 )
    {
        Q_UNUSED(parent);
    }

    QTelephonyService* service()
    {
        return new QTelephonyServiceDummy( "modem" );
    }

    QByteArray serviceName() const 
    {
        //synchronize with phoneserver.cpp 
        return QByteArray("DummyModemService");    
    }
};
#include "phoneserverdummymodem.moc"
QTOPIA_TASK(DummyTelephonyServiceFactory, DummyTelephonyServiceFactory);
QTOPIA_TASK_PROVIDES(DummyTelephonyServiceFactory, TelephonyServiceFactory);
