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

#include <qmodemadviceofcharge.h>
#include <qmodemservice.h>
#include <qmodemservice.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <qatutils.h>

/*!
    \class QModemAdviceOfCharge
    \inpublicgroup QtCellModule

    \brief The QModemAdviceOfCharge class provides advice of charge information on AT-based modems.
    \ingroup telephony::modem

    This class implements the QAdviceOfCharge telephony interface.
    Client applications should use QAdviceOfCharge instead of this class.

    The default implementation in this class uses the \c{AT+CAOC}, \c{AT+CACM},
    \c{AT+CAMM}, and \c{AT+CPUC} AT commands from 3GPP TS 27.007.  It also
    understands the \c{+CCCM} and \c{+CCWV} unsolicited results from 3GPP TS 27.007.

    \sa QAdviceOfCharge, QModemService
*/

class QModemAdviceOfChargePrivate
{
public:
    QModemAdviceOfChargePrivate( QModemService *service )
    {
        this->service = service;
    }

    QModemService *service;
};

/*!
    Create an AT-based advice of charge object for \a service.
*/
QModemAdviceOfCharge::QModemAdviceOfCharge( QModemService *service )
    : QAdviceOfCharge( service->service(), service, QCommInterface::Server )
{
    d = new QModemAdviceOfChargePrivate( service );

    // Register for unsolicited advice of charge notifications.
    service->primaryAtChat()->registerNotificationType
        ( "+CCCM:", this, SLOT(cccm(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "+CCWV:", this, SIGNAL(callMeterMaximumWarning()) );
}

/*!
    Destroy this AT-based advice of charge object.
*/
QModemAdviceOfCharge::~QModemAdviceOfCharge()
{
    delete d;
}

/*!
    \reimp
*/
void QModemAdviceOfCharge::requestCurrentCallMeter()
{
    d->service->primaryAtChat()->chat
        ( "AT+CAOC=0", this, SLOT(caoc(bool,QAtResult)) );
}

/*!
    \reimp
*/
void QModemAdviceOfCharge::requestAccumulatedCallMeter()
{
    d->service->primaryAtChat()->chat
        ( "AT+CACM?", this, SLOT(cacm(bool,QAtResult)) );
}

/*!
    \reimp
*/
void QModemAdviceOfCharge::requestAccumulatedCallMeterMaximum()
{
    d->service->primaryAtChat()->chat
        ( "AT+CAMM?", this, SLOT(camm(bool,QAtResult)) );
}

/*!
    \reimp
*/
void QModemAdviceOfCharge::requestPricePerUnit()
{
    d->service->primaryAtChat()->chat
        ( "AT+CPUC?", this, SLOT(cpuc(bool,QAtResult)) );
}

/*!
    \reimp
*/
void QModemAdviceOfCharge::resetAccumulatedCallMeter( const QString& password )
{
    d->service->primaryAtChat()->chat
        ( "AT+CACM=\"" + QAtUtils::quote( password ) + "\"",
          this, SLOT(cacmSet(bool,QAtResult)) );
}

/*!
    \reimp
*/
void QModemAdviceOfCharge::setAccumulatedCallMeterMaximum
        ( int value, const QString& password )
{
    QString hex = QString::number( value, 16 );
    if ( hex.length() < 6 )
        hex = QString("000000").left( 6 - hex.length() ) + hex;
    d->service->primaryAtChat()->chat
        ( "AT+CAMM=\"" + hex + "\",\"" + QAtUtils::quote( password ) + "\"",
          this, SLOT(cammSet(bool,QAtResult)) );
}

/*!
    \reimp
*/
void QModemAdviceOfCharge::setPricePerUnit
        ( const QString& currency, const QString& unitPrice,
          const QString& password )
{
    d->service->primaryAtChat()->chat
        ( "AT+CPUC=\"" + QAtUtils::quote( currency ) + "\",\"" +
                         QAtUtils::quote( unitPrice ) + "\",\"" +
                         QAtUtils::quote( password ) + "\"",
          this, SLOT(cpucSet(bool,QAtResult)) );
}

void QModemAdviceOfCharge::resetModem()
{
    // Turn on unsolicited advice of charge notifications.
    d->service->primaryAtChat()->chat( "AT+CAOC=2" );
    d->service->primaryAtChat()->chat( "AT+CCWE=1" );
}

void QModemAdviceOfCharge::caoc( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    if ( parser.next( "+CAOC:" ) ) {
        QString value = parser.readString();
        emit currentCallMeter( value.toInt( 0, 16 ), true );
    } else {
        emit currentCallMeter( 0, true );
    }
}

void QModemAdviceOfCharge::cccm( const QString& msg )
{
    uint posn = 6;
    QString value = QAtUtils::nextString( msg, posn );
    emit currentCallMeter( value.toInt( 0, 16 ), false );
}

void QModemAdviceOfCharge::cacm( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    if ( parser.next( "+CACM:" ) ) {
        QString value = parser.readString();
        emit accumulatedCallMeter( value.toInt( 0, 16 ) );
    } else {
        emit accumulatedCallMeter( 0 );
    }
}

void QModemAdviceOfCharge::cacmSet( bool, const QAtResult& result )
{
    emit resetAccumulatedCallMeterResult
            ( (QTelephony::Result)result.resultCode() );
}

void QModemAdviceOfCharge::camm( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    if ( parser.next( "+CAMM:" ) ) {
        QString value = parser.readString();
        emit accumulatedCallMeterMaximum( value.toInt( 0, 16 ) );
    } else {
        emit accumulatedCallMeterMaximum( 0 );
    }
}

void QModemAdviceOfCharge::cammSet( bool, const QAtResult& result )
{
    emit setAccumulatedCallMeterMaximumResult
            ( (QTelephony::Result)result.resultCode() );
}

void QModemAdviceOfCharge::cpuc( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    if ( parser.next( "+CACM:" ) ) {
        QString currency = parser.readString();
        QString ppu = parser.readString();
        emit pricePerUnit( currency, ppu );
    } else {
        emit pricePerUnit( QString(), QString() );
    }
}

void QModemAdviceOfCharge::cpucSet( bool, const QAtResult& result )
{
    emit setPricePerUnitResult( (QTelephony::Result)result.resultCode() );
}
