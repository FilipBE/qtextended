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

#include "vendor_pstn_p.h"
#include <qserialiodevicemultiplexer.h>
#include <qmodemindicators.h>
#include <qatresultparser.h>
#include <qatutils.h>
#include <qvaluespace.h>
#include <QTimer>
#include <qpreferrednetworkoperators.h>
#include <qcallbarring.h>
#include <qcallforwarding.h>
#include <qcallsettings.h>
#include <qsmsreader.h>
#include <qsmssender.h>
#include <qsimtoolkit.h>
#include <qsimfiles.h>
#include <qsimgenericaccess.h>
#include <qcellbroadcast.h>
#include <qsupplementaryservices.h>
#include <qvibrateaccessory.h>
#include <qgprsnetworkregistration.h>
#include <qcallvolume.h>

PstnCallProvider::PstnCallProvider( PstnModemService *service )
    : QModemCallProvider( service )
{
    this->service = service;
}

PstnCallProvider::~PstnCallProvider()
{
}

QModemCallProvider::AtdBehavior PstnCallProvider::atdBehavior() const
{
    // There is no reliable way to determine when voice calls
    // transition from Dialing to Connected, so assume that
    // they are connected as soon as we get the OK from ATD.
    // In reality, they will be in the Alerting state when
    // the OK is received.  Presumably the user can tell by
    // listening as to when the call actually connects.
    return AtdOkIsConnect;
}

void PstnCallProvider::abortDial
	( uint modemIdentifier, QPhoneCall::Scope scope )
{
    Q_UNUSED(modemIdentifier);
    Q_UNUSED(scope);
    atchat()->abortDial();
    atchat()->chat( "ATH" );
}

QString PstnCallProvider::releaseCallCommand( uint modemIdentifier ) const
{
    Q_UNUSED(modemIdentifier);
    return "ATH";
}

QString PstnCallProvider::releaseActiveCallsCommand() const
{
    return "ATH";
}

QString PstnCallProvider::releaseHeldCallsCommand() const
{
    return "ATH";
}

QString PstnCallProvider::putOnHoldCommand() const
{
    // Hold is not supported yet, so return a command that will always fail.
    return "AT+NOHOLD";
}

QString PstnCallProvider::setBusyCommand() const
{
    // Rejecting an incoming call is done with ATH.
    return "ATH";
}

QString PstnCallProvider::activateCallCommand
	( uint modemIdentifier, bool otherActiveCalls ) const
{
    // Activate is not supported yet, so return a command that will always fail.
    Q_UNUSED(modemIdentifier);
    Q_UNUSED(otherActiveCalls);
    return "AT+NOACT";
}

QString PstnCallProvider::activateHeldCallsCommand() const
{
    // Activate is not supported yet, so return a command that will always fail.
    return "AT+NOACT";
}

QString PstnCallProvider::joinCallsCommand( bool detachSubscriber ) const
{
    // Join is not supported yet, so return a command that will always fail.
    Q_UNUSED(detachSubscriber);
    return "AT+NOJOIN";
}

QString PstnCallProvider::deflectCallCommand( const QString& number ) const
{
    // Deflect is not supported yet, so return a command that will always fail.
    Q_UNUSED(number);
    return "AT+NODEFLECT";
}

void PstnCallProvider::resetModem()
{
    atchat()->chat( "AT+VCID=1" );
    atchat()->chat( "AT+VLS=6" );
}

PstnSimInfo::PstnSimInfo( PstnModemService *service )
    : QSimInfo( service->service(), service, Server )
{
    // This type of modem does not have a SIM, but there are other
    // parts of Qtopia that may check that the SIM is inserted before
    // proceeding.  So we fake the presence of a SIM.
    setIdentity( "9876543210" );
}

PstnSimInfo::~PstnSimInfo()
{
}

PstnPinManager::PstnPinManager( PstnModemService *service )
    : QPinManager( service->service(), service, Server )
{
    this->service = service;
    readySent = false;
}

PstnPinManager::~PstnPinManager()
{
}

void PstnPinManager::querySimPinStatus()
{
    // We don't have a SIM, and therefore no PIN, so fake "READY".
    emit pinStatus( "READY", Valid, QPinOptions() );
    if ( !readySent ) {
	readySent = true;
	service->post( "simready" );
    }
}

void PstnPinManager::enterPin( const QString& type, const QString& pin )
{
    Q_UNUSED(pin);
    emit pinStatus( type, Valid, QPinOptions() );
}

void PstnPinManager::enterPuk
        ( const QString& type, const QString& puk, const QString& newPin )
{
    Q_UNUSED(puk);
    Q_UNUSED(newPin);
    emit pinStatus( type, Valid, QPinOptions() );
}

void PstnPinManager::cancelPin( const QString& type )
{
    Q_UNUSED(type);
}

void PstnPinManager::changePin
        ( const QString& type, const QString& oldPin, const QString& newPin )
{
    Q_UNUSED(type);
    Q_UNUSED(oldPin);
    Q_UNUSED(newPin);
    emit changePinResult( type, true );
}

PstnRfFunctionality::PstnRfFunctionality
        ( const QString& service, QObject *parent )
    : QPhoneRfFunctionality( service, parent, QCommInterface::Server )
{
}

PstnRfFunctionality::~PstnRfFunctionality()
{
}

void PstnRfFunctionality::forceLevelRequest()
{
    // The only functionality level we support is "full".
    setValue( "level", qVariantFromValue( Full ) );
    emit levelChanged();
}

void PstnRfFunctionality::setLevel( QPhoneRfFunctionality::Level level )
{
    Q_UNUSED(level);
}

PstnNetworkRegistration::PstnNetworkRegistration
        ( const QString& service, QObject *parent )
    : QNetworkRegistrationServer( service, parent )
{
    QTimer::singleShot( 0, this, SLOT(initDone()) );
}

PstnNetworkRegistration::~PstnNetworkRegistration()
{
}

void PstnNetworkRegistration::setCurrentOperator
        ( QTelephony::OperatorMode, const QString&, const QString&)
{
    emit setCurrentOperatorResult( QTelephony::OK );
}

void PstnNetworkRegistration::requestAvailableOperators()
{
    QList<QNetworkRegistration::AvailableOperator> opers;
    QNetworkRegistration::AvailableOperator oper;
    oper.availability = QTelephony::OperatorAvailable;
    oper.name = "Telephone";       // No tr
    oper.id = "0Telephone";        // No tr
    oper.technology = "GSM";       // No tr
    opers.append( oper );
    emit availableOperators( opers );
}

void PstnNetworkRegistration::initDone()
{
    updateInitialized( true );
    updateRegistrationState( QTelephony::RegistrationHome );
    updateCurrentOperator( QTelephony::OperatorModeAutomatic,
                           "0Telephone", "Telephone", "GSM" );    // No tr
}

PstnPhoneBook::PstnPhoneBook( const QString& service, QObject *parent )
    : QPhoneBook( service, parent, QCommInterface::Server )
{
    fixedDialingEnabled = false;
}

PstnPhoneBook::~PstnPhoneBook()
{
}

void PstnPhoneBook::getEntries( const QString& store )
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

void PstnPhoneBook::add( const QPhoneBookEntry& entry, const QString& store, bool flush )
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

void PstnPhoneBook::remove( uint index, const QString& store, bool flush )
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

void PstnPhoneBook::update( const QPhoneBookEntry& entry, const QString& store, bool flush )
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

void PstnPhoneBook::flush( const QString& store )
{
    getEntries( store );
}

void PstnPhoneBook::setPassword( const QString&, const QString& )
{
    // Nothing to do here.
}

void PstnPhoneBook::clearPassword( const QString& )
{
    // Nothing to do here.
}

void PstnPhoneBook::requestLimits( const QString& store )
{
    QPhoneBookLimits l;
    l.setNumberLength( 20 );
    l.setTextLength( 18 );
    l.setFirstIndex( 1 );
    l.setLastIndex( 150 );
    emit limits( store, l );
}

void PstnPhoneBook::requestFixedDialingState()
{
    emit fixedDialingState( fixedDialingEnabled );
}

void PstnPhoneBook::setFixedDialingState( bool enabled, const QString& )
{
    fixedDialingEnabled = enabled;
    emit setFixedDialingStateResult( QTelephony::OK );
}

PstnServiceNumbers::PstnServiceNumbers( QModemService *service )
    : QModemServiceNumbers( service )
{
}

PstnServiceNumbers::~PstnServiceNumbers()
{
}

void PstnServiceNumbers::requestServiceNumber
	( QServiceNumbers::NumberId id )
{
    // Store all service numbers in a file because there is no SIM.
    requestServiceNumberFromFile( id );
}

void PstnServiceNumbers::setServiceNumber
	( QServiceNumbers::NumberId id, const QString& number )
{
    // Store all service numbers in a file because there is no SIM.
    setServiceNumberInFile( id, number );
}

PstnConfiguration::PstnConfiguration( PstnModemService *service )
    : QTelephonyConfiguration( service->service(), service, Server )
{
    this->service = service;
}

PstnConfiguration::~PstnConfiguration()
{
}

void PstnConfiguration::update( const QString&, const QString& )
{
    // All values are read-only.
}

void PstnConfiguration::request( const QString& name )
{
    if ( name == "manufacturer" ) {
	if ( service->modemType() == PstnModemService::Rockwell ) {
	    service->primaryAtChat()->chat
		( "AT#MFR?", this, SLOT(gmi(bool,QAtResult)) );
	} else {
	    service->primaryAtChat()->chat
		( "AT+GMI", this, SLOT(gmi(bool,QAtResult)) );
	}
    } else if ( name == "model" ) {
	if ( service->modemType() == PstnModemService::Rockwell ) {
	    service->primaryAtChat()->chat
		( "AT#MDL?", this, SLOT(gmm(bool,QAtResult)) );
	} else {
	    service->primaryAtChat()->chat
		( "AT+GMM", this, SLOT(gmm(bool,QAtResult)) );
	}
    } else if ( name == "revision" ) {
	if ( service->modemType() == PstnModemService::Rockwell ) {
	    service->primaryAtChat()->chat
		( "AT#REV?", this, SLOT(gmr(bool,QAtResult)) );
	} else {
	    service->primaryAtChat()->chat
		( "AT+GMR", this, SLOT(gmr(bool,QAtResult)) );
	}
    } else if ( name == "serial" ) {
	if ( service->modemType() == PstnModemService::Rockwell ) {
	    service->primaryAtChat()->chat
		( "AT\"?", this, SLOT(gsn(bool,QAtResult)) );
	} else {
	    service->primaryAtChat()->chat
		( "AT+GSN", this, SLOT(gsn(bool,QAtResult)) );
	}
    } else if ( name == "extraVersion" ) {
	// Rockwell modems report the name of the modem on ATI4.
	// e.g. "V.90 & K56Flex Voice Modem".
	service->primaryAtChat()->chat
	    ( "ATI4", this, SLOT(extra(bool,QAtResult)) );
    } else {
        // Not supported - return an empty string.
        emit notification( name, QString() );
    }
}

void PstnConfiguration::gmi( bool, const QAtResult& result )
{
    emit notification( "manufacturer", result.content().trimmed() );
}

void PstnConfiguration::gmm( bool, const QAtResult& result )
{
    emit notification( "model", result.content().trimmed() );
}

void PstnConfiguration::gmr( bool, const QAtResult& result )
{
    emit notification( "revision", result.content().trimmed() );
}

void PstnConfiguration::gsn( bool, const QAtResult& result )
{
    emit notification( "serial", result.content().trimmed() );
}

void PstnConfiguration::extra( bool, const QAtResult& result )
{
    emit notification( "extraVersion", result.content().trimmed() );
}

PstnModemService::PstnModemService
        ( const QString& service, QSerialIODeviceMultiplexer *mux,
          QObject *parent )
    : QModemService( service, mux, parent )
{
    // The default modem type is V.253-compatible.
    type = V253;

    // We need to do some extra stuff at reset time.
    connect( this, SIGNAL(resetModem()), this, SLOT(reset()) );

    // Disable signal quality and battery charge polling because
    // neither will make any sense on a fixed line modem.
    indicators()->setSignalQuality( 100, 100 );
    indicators()->setBatteryCharge
	( -1, 100, QModemIndicators::NotPoweredByBattery );

    // Determine the type of modem, for AT command variations.
    chat( "AT#MFR?", this, SLOT(mfr(bool)) );
}

PstnModemService::~PstnModemService()
{
}

void PstnModemService::initialize()
{
    if ( !supports<QSimInfo>() )
	addInterface( new PstnSimInfo( this ) );

    if ( !supports<QPinManager>() )
	addInterface( new PstnPinManager( this ) );

    if ( !supports<QPhoneRfFunctionality>() )
	addInterface( new PstnRfFunctionality( service(), this ) );

    if ( !supports<QNetworkRegistration>() )
	addInterface( new PstnNetworkRegistration( service(), this ) );

    if ( !supports<QPhoneBook>() )
	addInterface( new PstnPhoneBook( service(), this ) );

    if ( !supports<QServiceNumbers>() )
	addInterface( new PstnServiceNumbers( this ) );

    if ( !supports<QTelephonyConfiguration>() )
	addInterface( new PstnConfiguration( this ) );

    if ( !callProvider() )
        setCallProvider( new PstnCallProvider( this ) );

    // Suppress interfaces that are GSM-specific so that the
    // QModemService::initialize() method will not create them.
    suppressInterface<QPreferredNetworkOperators>();
    suppressInterface<QCallBarring>();
    suppressInterface<QCallForwarding>();
    suppressInterface<QCallSettings>();
    suppressInterface<QSMSReader>();
    suppressInterface<QSMSSender>();
    suppressInterface<QSimToolkit>();
    suppressInterface<QSimFiles>();
    suppressInterface<QSimGenericAccess>();
    suppressInterface<QCellBroadcast>();
    suppressInterface<QSupplementaryServices>();
    suppressInterface<QVibrateAccessory>();
    suppressInterface<QGprsNetworkRegistration>();
    suppressInterface<QCallVolume>();

    QModemService::initialize();
}

void PstnModemService::reset()
{
    // Make sure that the modem's speaker is enabled.
    chat( "ATL2" );
    chat( "ATM2" );
}

void PstnModemService::mfr( bool ok )
{
    if ( ok ) {
	type = Rockwell;
	chat( "AT#CID=1" );
	chat( "AT#VLS=6" );
	chat( "AT#VRA=70" );
	chat( "AT#VRN=100" );
    }
}
