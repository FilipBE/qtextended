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

#include <qmodemindicators.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <qatutils.h>
#include <qvaluespace.h>
#include <qpowersource.h>
#include <custom.h>
#include <QSignalSourceProvider>
#include <QtopiaServiceRequest>
#include <QTimer>
#include <QDateTime>

// Timeout for the signal quality check, when polling.
#ifndef SIGNAL_QUALITY_TIMEOUT
#define SIGNAL_QUALITY_TIMEOUT      2000
#endif

// Timeout for battery charge checks, when not charging and polling.
#ifndef BATTERY_CHARGE_TIMEOUT
#define BATTERY_CHARGE_TIMEOUT      20000
#endif

// Timeout for battery charge checks, when charging and polling.
#ifndef BATTERY_CHARGING_TIMEOUT
#define BATTERY_CHARGING_TIMEOUT    500
#endif

/*!
    \class QModemIndicators
    \inpublicgroup QtCellModule

    \brief The QModemIndicators class supports indicators for signal quality, battery charge, etc.
    \ingroup telephony::modem

    Modem vendor plug-ins might have proprietary notifications for reporting
    this information.  When such a notification arrives, the plug-in should
    call setSignalQuality(), setBatteryCharge(), etc.

    The modem vendor plug-in will obtain an instance of this class by
    calling QModemService::indicators().

    \sa QModemService
*/

/*!
    \enum QModemIndicators::ChargeState
    This enum defines the current charging state of the battery, using the
    values defined by 3GPP TS 27.007.

    \value PoweredByBattery Currently powered by the battery.
    \value NotPoweredByBattery Battery is connected, but not powering the phone.
    \value NoBattery No battery connected.
    \value PowerFault There is a power fault and calls are inhibited.
*/

/*!
    \enum QModemIndicators::SmsMemoryFullStatus
    This enum defines the full status of the incoming SMS message store.

    \value SmsMemoryOK The store has slots available to receive new messages.
    \value SmsMemoryFull The store is full and cannot accept new messages.
    \value SmsMemoryMessageRejected The store is full and a new message
           was just rejected.
*/

class QModemIndicatorsPrivate
{
public:
    QModemIndicatorsPrivate( QModemService *service )
    {
        this->service = service;
        this->signalQualityUnsolicited = false;
        this->batteryChargeUnsolicited = false;
        this->pollForSignalQuality = true;
        this->pollForBatteryCharge = true;
    }

    QModemService *service;
    QValueSpaceObject *telephonyStatus;
    bool signalQualityUnsolicited;
    bool batteryChargeUnsolicited;
    bool pollForSignalQuality;
    bool pollForBatteryCharge;
    QList<QString> indicators;
    QSignalSourceProvider* signalProvider;
    QPowerSourceProvider *accessoryProvider;
};

/*!
    Construct a modem indicator object for \a service.  Normally,
    a modem vendor plug-in will call QModemService::indicators().
*/
QModemIndicators::QModemIndicators( QModemService *service )
    : QObject( service )
{
    d = new QModemIndicatorsPrivate( service );
    d->telephonyStatus = new QValueSpaceObject( "/Telephony/Status", this );
    d->signalProvider = new QSignalSourceProvider( service->service(), "modem", this );
    d->accessoryProvider = new QPowerSourceProvider(QPowerSource::Battery, service->service(), this);
    connect( service, SIGNAL(resetModem()), this, SLOT(resetModem()) );

    service->primaryAtChat()->registerNotificationType
        ( "+CSQ:", this, SLOT(csqn(QString)), true );
    service->primaryAtChat()->registerNotificationType
        ( "+CBC:", this, SLOT(cbcn(QString)), true );
    service->primaryAtChat()->registerNotificationType
        ( "+CIEV:", this, SLOT(ciev(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "+CTZV:", this, SLOT(ctzv(QString)) );

    QTimer::singleShot( SIGNAL_QUALITY_TIMEOUT,
                        this, SLOT(pollSignalQuality()) );
    pollBatteryCharge(); //poll now to get first update immediately
    QTimer::singleShot( BATTERY_CHARGE_TIMEOUT,
                        this, SLOT(pollBatteryCharge()) );

    // Turn on time zone reporting.
    service->primaryAtChat()->chat( "AT+CTZR=1" );
}

/*!
    Destroy this modem indicator object.
*/
QModemIndicators::~QModemIndicators()
{
    delete d;
}

/*!
    Report an unsolicited signal quality \a value.  The \a maxValue
    parameter indicates the maximum value for the valid range of values.
    e.g. if \a value can vary between 0 and 31, then \a maxValue will be 31.

    If \a value is -1, then it indicates that the signal quality is
    currently undetectable (e.g. no network).

    This function is typically called by modem vendor plug-ins that have
    a proprietary command for reporting the current signal quality.
    If the modem vendor plug-in does not call this function, the default
    implementation will poll the \c{AT+CSQ} command for signal quality.

    Client applications get access to the signal quality value using the
    QSignalSource class.

    \sa QSignalSource
*/
void QModemIndicators::setSignalQuality( int value, int maxValue )
{
    d->signalQualityUnsolicited = true;
    d->pollForSignalQuality = false;
    setSignalQualityInternal( value, maxValue );
}

/*!
    Report an unsolicited battery charge \a value and \a state.  The
    \a maxValue parameter indicates the maximum value for the valid range
    of values.  e.g. if \a value can vary between 0 and 100,
    then \a maxValue will be 100.

    If \a value is -1, then it indicates that the battery charge is
    currently undetectable.

    This function is typically called by modem vendor plug-ins that have
    a proprietary command for reporting the current battery charge.
    If the modem vendor plug-in does not call this function, the default
    implementation will poll the \c{AT+CBC} command for battery charge.

    Client applications get access to the battery charge value using the
    QPowerSource class.

    \sa QPowerSource
*/
void QModemIndicators::setBatteryCharge( int value, int maxValue,
                                         QModemIndicators::ChargeState state )
{
    d->batteryChargeUnsolicited = true;
    d->pollForBatteryCharge = false;
    setBatteryChargeInternal( value, maxValue, state );
}

/*!
    Sets the SMS memory full indicator to \a value.

    Client applications get access to the SMS memory full indicator by
    querying the \c{/Telephony/Status/SMSMemoryFull} value space item.
*/
void QModemIndicators::setSmsMemoryFull
        ( QModemIndicators::SmsMemoryFullStatus value )
{
    // Store the value to /Telephony/Status/SMSMemoryFull.
    // 0 = OK, 1 = full, 2 = message rejected.
    d->telephonyStatus->setAttribute( "SMSMemoryFull", (int)value );
}

/*!
    Sets the network time zone information to \a zone and \a dst.  The \a zone parameter
    indicates the number of minutes east of GMT.  The \a dst
    parameter indicates the number of minutes of daylight savings
    adjustment that have been applied to \a zone.

    The parameter values are placed into the value space as
    \c{/Telephony/Status/TimeZone} and \c{/Telephony/Status/TimeZoneDST}.

    This function is deprecated in Qtopia 4.3 an later.  Use setNetworkTime()
    instead.

    \sa setNetworkTime()
*/
void QModemIndicators::setNetworkTimeZone( int zone, int dst )
{
    d->telephonyStatus->setAttribute( "TimeZone", zone );
    d->telephonyStatus->setAttribute( "TimeZoneDST", dst );
    QtopiaServiceRequest req("TimeUpdate", "storeExternalSource(QString,uint,int,int)");
    req << QString("Modem") << (uint)0 << zone << dst;
    req.send();
}

/*!
    Sets the network time information to \a time, \a zone, and \a dst.
    The \a time parameter indicates the time from the network in UTC,
    as seconds past midnight, 1 Jan 1970.

    The \a zone parameter indicates the number of minutes east of GMT.
    The \a dst parameter indicates the number of minutes of daylight
    savings adjustment that have been applied to \a zone.

    The parameter values are placed into the value space as
    \c{/Telephony/Status/Time}, \c{/Telephony/Status/TimeZone},
    and \c{/Telephony/Status/TimeZoneDST}.  In addition, the
    value space item \c{/Telephony/Status/TimeTimeStamp} will be
    set to the current time on the phone, also in UTC.

    The values of \c{Time} and \c{TimeTimeStamp} in the value space
    can be used to determine the network time at some point in the
    future relative to the current time on the phone.  Subtract
    \c{TimeTimeStamp} from the current time and add it to \c{Time}
    to determine the current network time.
*/
void QModemIndicators::setNetworkTime( uint time, int zone, int dst )
{
    d->telephonyStatus->setAttribute( "TimeZone", zone );
    d->telephonyStatus->setAttribute( "TimeZoneDST", dst );
    QtopiaServiceRequest req("TimeUpdate", "storeExternalSource(QString,uint,int,int)");
    req << QString("Modem") << time << zone << dst;
    req.send();
}

void QModemIndicators::resetModem()
{
    // Turn on automatic signal quality updates, if supported.
    // The updates will be reported via unsolicited "+CSQ:" notifications.
    d->service->primaryAtChat()->chat( "AT+CCED=1,8" );

    // Ask the modem what indicators it supports.
    d->service->primaryAtChat()->chat
        ( "AT+CIND=?", this, SLOT(cind(bool,QAtResult)) );
}

void QModemIndicators::csq( bool ok, const QAtResult& result )
{
    // Result of polling for signal quality.
    QAtResultParser parser( result );
    if ( ok && parser.next( "+CSQ:" ) ) {
        setSignalQualityInternal( (int)parser.readNumeric(), 31 );

        // when modem responds with a signal level of 99, this means
        // not known or undetectable. We need to handle this
        int rssi = parser.readNumeric();
        if (rssi == 99)
            rssi = -1;
        setSignalQualityInternal( rssi, 31 );

    } else {
        // Modem probably does not support AT+CSQ, so turn off polling
        // and then act as though the signal is fully present.
        d->pollForSignalQuality = false;
        setSignalQualityInternal( 100, 100 );
    }
}

void QModemIndicators::csqn( const QString& msg )
{
    // Unsolicited signal quality notification.
    uint posn = 5;
    int rssi = QAtUtils::parseNumber( msg, posn );
    if (rssi == 99) // if signal is 99, it mean not known, or undetectable
        rssi = -1;
    setSignalQuality( rssi, 31 );
}

void QModemIndicators::cbc( bool ok, const QAtResult& result )
{
    QAtResultParser parser( result );
    if ( ok && parser.next( "+CBC:" ) ) {
        uint bcs = parser.readNumeric();
        uint bcl = parser.readNumeric();
        if ( !bcs && !bcl ) {
            // Wavecom: no battery present in the modem, so stop polling.
            d->pollForBatteryCharge = false;
            bcs = (int)NoBattery;
        }
        setBatteryChargeInternal( (int)bcl, 100, (ChargeState)bcs );
    }
}

void QModemIndicators::cbcn( const QString& msg )
{
    // Unsolicited battery charge notification.
    uint posn = 5;
    uint bcs = QAtUtils::parseNumber( msg, posn );
    uint bcl = QAtUtils::parseNumber( msg, posn );
    setBatteryCharge( (int)bcl, 100, (ChargeState)bcs );
}

void QModemIndicators::ciev( const QString& msg )
{
    // Handle indicator notifications.
    uint posn = 6;
    uint ind = QAtUtils::parseNumber( msg, posn );
    uint value = QAtUtils::parseNumber( msg, posn );
    if ( ind > 0 && ind <= (uint)(d->indicators.count()) ) {
        QString name = d->indicators[ind - 1];
        if ( name == "signal" && !d->signalQualityUnsolicited ) {

            // Signal quality indication: (0-5).
            d->pollForSignalQuality = false;
            setSignalQualityInternal( value, 5 );

        } else if ( name == "battchg" && !d->batteryChargeUnsolicited ) {

            // Battery charge indication: (0-5).
            if ( value >= 5 )
                value = 100;
            else if ( value == 0 )
                value = 1;
            else
                value *= 20;
            d->pollForBatteryCharge = false;
            setBatteryChargeInternal( value, 100, PoweredByBattery );

        } else if ( name == "smsfull" ) {

            // "SMS message box is full" indication: (0-1).
            setSmsMemoryFull( (SmsMemoryFullStatus)value );

        }
    }
}

void QModemIndicators::cind( bool ok, const QAtResult& result )
{
    if ( ok ) {
        // Turn on unsolicited notifications for indicators.
        d->service->primaryAtChat()->chat( "AT+CMER=1,0,0,1,0" );

        // Parse the indicator information to determine name->number mappings.
        QAtResultParser cmd( result );
        d->indicators.clear();
        cmd.next( "+CIND:" );
        QString name;
        for (;;) {
            name = cmd.readString();
            if ( name.length() == 0 )
                break;
            d->indicators.append( name );
        }
   }
}

void QModemIndicators::ctzv( const QString& msg )
{
    // Timezone value is in 15 minute intervals.  Convert to real minutes.
    int tz = msg.mid(6).toInt();
    setNetworkTimeZone( tz * 15, 0 );
}

void QModemIndicators::setSignalQualityInternal( int value, int maxValue )
{
    // Normalize the value to the range 0..100.
    if ( value < 0 ) {
        value = -1;
    } else if ( value >= maxValue ) {
        value = 100;
    } else {
        value = value * 100 / maxValue;
    }

    //publish the values to the rest of the system
    if ( value < 0 ) {
        d->signalProvider->setAvailability( QSignalSource::NotAvailable );
    } else {
        d->signalProvider->setAvailability( QSignalSource::Available );
    }
    d->signalProvider->setSignalStrength( value );

    // Arrange for the next poll to be performed.
    if ( d->pollForSignalQuality ) {
        QTimer::singleShot( SIGNAL_QUALITY_TIMEOUT,
                            this, SLOT(pollSignalQuality()) );
    }
}

void QModemIndicators::setBatteryChargeInternal
        ( int value, int maxValue, QModemIndicators::ChargeState state )
{
    // Normalize the value to the range 0..100.
    if ( value < 0 ) {
        value = 0;
    } else if ( value >= maxValue ) {
        value = 100;
    } else {
        value = value * 100 / maxValue;
    }

    // Publish the values to the rest of the system.
    d->accessoryProvider->setCharge( value );
    d->accessoryProvider->setCharging( state != PoweredByBattery );
    d->accessoryProvider->setAvailability((state != PowerFault)?QPowerSource::Available:QPowerSource::Failed);
    //d->accessoryProvider->setValue( "ModemChargeState", (int)state );

    // Poll for the next update.
    if ( d->pollForBatteryCharge ) {
        if ( value < 99 && state != PoweredByBattery ) {
            // Assume that a power cord is connected and we are charging.
            // Switch to updating the status every half a second.
            QTimer::singleShot( BATTERY_CHARGING_TIMEOUT,
                                this, SLOT(pollBatteryCharge()) );
        } else {

            // We are running off the battery or the battery is fully charged,
            // so update every 20 seconds.
            QTimer::singleShot( BATTERY_CHARGE_TIMEOUT,
                                this, SLOT(pollBatteryCharge()) );
        }
    }
}

void QModemIndicators::pollSignalQuality()
{
    if ( d->pollForSignalQuality ) {
        d->service->primaryAtChat()->chat
            ( "AT+CSQ", this, SLOT(csq(bool,QAtResult)) );
    }
}

void QModemIndicators::pollBatteryCharge()
{
    if ( d->pollForBatteryCharge ) {
        d->service->primaryAtChat()->chat
            ( "AT+CBC", this, SLOT(cbc(bool,QAtResult)) );
    }
}
