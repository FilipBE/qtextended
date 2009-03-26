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

#include "atindicators.h"
#include <qlist.h>
#include <qvaluespace.h>
#include <qnetworkregistration.h>
#ifdef QTOPIA_CELL
#include <qsmsreader.h>
#endif

class AtIndicatorsPrivate
{
public:
    QStringList indicators;
    QList<int> values;
    QList<int> maxValues;
};

AtIndicators::AtIndicators( QObject *parent )
    : QObject( parent )
{
    d = new AtIndicatorsPrivate();
}

AtIndicators::~AtIndicators()
{
    delete d;
}

int AtIndicators::addIndicator
        ( const QString& name, int maxValue, int initValue )
{
    int ind = d->indicators.size();
    d->indicators += name;
    d->values += initValue;
    d->maxValues += maxValue;
    return ind;
}

QStringList AtIndicators::indicators() const
{
    return d->indicators;
}

int AtIndicators::indicator( const QString& name ) const
{
    return d->indicators.indexOf( name );
}

int AtIndicators::numIndicators() const
{
    return d->indicators.size();
}

QString AtIndicators::name( int ind ) const
{
    if ( ind >= 0 && ind < d->indicators.size() )
        return d->indicators[ind];
    else
        return QString();
}

int AtIndicators::value( int ind ) const
{
    if ( ind >= 0 && ind < d->indicators.size() )
        return d->values[ind];
    else
        return 0;
}

int AtIndicators::value( const QString& name ) const
{
    return value( indicator( name ) );
}

int AtIndicators::maxValue( int ind ) const
{
    if ( ind >= 0 && ind < d->indicators.size() )
        return d->maxValues[ind];
    else
        return 0;
}

void AtIndicators::setValue( int ind, int value )
{
    if ( ind >= 0 && ind < d->indicators.size() ) {
        if ( value < 0 )
            value = 0;
        else if ( value > d->maxValues[ind] )
            value = d->maxValues[ind];
        if ( d->values[ind] != value ) {
            d->values[ind] = value;
            emit indicatorChanged( ind, value );
        }
    }
}

void AtIndicators::setValue( const QString& name, int value )
{
    setValue( indicator( name ), value );
}

class AtPhoneIndicatorsPrivate
{
public:
    int battchg;
    int signal;
    int service;
    int message;
    int call;
    int roam;
    int smsfull;
    int callsetup;
    int callheld;
    QValueSpaceItem *signalQuality;
    QValueSpaceItem *batteryCharge;
    QNetworkRegistration *netReg;
#ifdef QTOPIA_CELL
    QSMSReader *smsReader;
    QValueSpaceItem *smsMemoryFull;
#endif
};

AtPhoneIndicators::AtPhoneIndicators( QObject *parent )
    : AtIndicators( parent )
{
    d = new AtPhoneIndicatorsPrivate();

    d->signalQuality = new QValueSpaceItem
            ( "/Hardware/Accessories/QSignalSource/modem/SignalStrength", this );
    connect( d->signalQuality, SIGNAL(contentsChanged()),
             this, SLOT(updateSignalQuality()) );

    d->batteryCharge = new QValueSpaceItem( "/Hardware/Accessories/QPowerSource/modem", this );
    connect( d->batteryCharge, SIGNAL(contentsChanged()),
             this, SLOT(updateBatteryCharge()) );

#ifdef QTOPIA_CELL
    d->smsMemoryFull = new QValueSpaceItem
        ( "/Telephony/Status/SMSMemoryFull", this );
    connect( d->smsMemoryFull, SIGNAL(contentsChanged()),
             this, SLOT(updateSmsMemoryFull()) );
#endif

    d->battchg = addIndicator( "battchg", 5, 0 );
    d->signal = addIndicator( "signal", 5, 0 );
    d->service = addIndicator( "service", 1, 0 );
    d->message = addIndicator( "message", 1, 0 );
    d->call = addIndicator( "call", 1, 0 );
    d->roam = addIndicator( "roam", 1, 0 );
    d->smsfull = addIndicator( "smsfull", 1, 0 );
    d->callsetup = addIndicator( "callsetup", 3, 0 );
    d->callheld = addIndicator( "callheld", 2, 0 );

    d->netReg = new QNetworkRegistration( "modem", this );
    if ( !d->netReg->available() ) {
        // May be a VoIP-only phone, so use the default network reg object.
        delete d->netReg;
        d->netReg = new QNetworkRegistration( QString(), this );
    }
    connect( d->netReg, SIGNAL(registrationStateChanged()),
             this, SLOT(updateRegistrationState()) );

#ifdef QTOPIA_CELL
    d->smsReader = new QSMSReader( QString(), this );
    connect( d->smsReader, SIGNAL(unreadCountChanged()),
             this, SLOT(unreadCountChanged()) );
#endif

    updateRegistrationState();
    updateSignalQuality();
    updateBatteryCharge();
}

AtPhoneIndicators::~AtPhoneIndicators()
{
    delete d;
}

int AtPhoneIndicators::signalQuality() const
{
    return d->signalQuality->value( QByteArray(), -1 ).toInt();
}

int AtPhoneIndicators::batteryCharge() const
{
    return d->batteryCharge->value( "Charge", QVariant(-1) ).toInt();
}

void AtPhoneIndicators::setOnCall( bool value )
{
    setValue( d->call, value ? 1 : 0 );
}

void AtPhoneIndicators::setCallSetup( AtCallManager::CallSetup callSetup )
{
    setValue( d->callsetup, (int)callSetup );
}

void AtPhoneIndicators::setCallHold( AtCallManager::CallHoldState callHold )
{
    setValue( d->callheld, (int)callHold );
}

void AtPhoneIndicators::updateRegistrationState()
{
    QTelephony::RegistrationState regState = d->netReg->registrationState();
    if ( regState == QTelephony::RegistrationHome ||
         regState == QTelephony::RegistrationUnknown ) {
        setValue( d->service, 1 );
        setValue( d->roam, 0 );
    } else if ( regState == QTelephony::RegistrationRoaming ) {
        setValue( d->service, 1 );
        setValue( d->roam, 1 );
    } else {
        setValue( d->service, 0 );
        setValue( d->roam, 0 );
    }
}

void AtPhoneIndicators::updateSignalQuality()
{
    int value = signalQuality();
    if ( value < 0 )
        setValue( d->signal, 0 );
    else if ( value > 95 )
        setValue( d->signal, 5 );
    else
        setValue( d->signal, value * 5 / 100 );
    emit signalQualityChanged( value );
}

void AtPhoneIndicators::updateBatteryCharge()
{
    int value = batteryCharge();
    if ( value < 0 )
        setValue( d->battchg, 0 );
    else if ( value > 95 )
        setValue( d->battchg, 5 );
    else
        setValue( d->battchg, value * 5 / 100 );
    emit batteryChargeChanged( value );
}

#ifdef QTOPIA_CELL

void AtPhoneIndicators::unreadCountChanged()
{
    if ( !d->smsReader->unreadCount() )
        setValue( d->message, 0 );
    else
        setValue( d->message, 1 );
}

void AtPhoneIndicators::updateSmsMemoryFull()
{
    setValue( d->smsfull, d->smsMemoryFull->value().toInt() );
}

#endif
