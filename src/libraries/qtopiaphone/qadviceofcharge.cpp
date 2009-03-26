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

#include <qadviceofcharge.h>

/*!
    \class QAdviceOfCharge
    \inpublicgroup QtTelephonyModule

    \brief The QAdviceOfCharge class provides advice of charge information about a user's phone account.

    The 3GPP TS 02.86 Advice of Charge supplementary service provides the phone with the
    information to produce an estimate of the cost of the service used.  This telephony
    interface only applies to GSM-style mobile telephone networks.

    Charges are indicated for the call, or calls, in progress when mobile originated or
    for the roaming leg only when mobile terminated.  Any charges for non-call related
    transactions, and for certain supplementary services, such as Call Forwarding are
    not indicated.

    The phone will receive at the beginning of each call (and as necessary during the call) a
    message, the Charge Advice Information.  This message contains the elements which
    together define the rate at which the call is to be charged, time dependence,
    data dependence and for unit increments.  The message is delivered to client
    applications via the currentCallMeter() signal in this class.

    The charge information is reported in \i units, which vary depending upon the
    operator and the user's home currency.  When the client calls requestPricePerUnit(),
    the service will respond with a pricePerUnit() signal to report the international
    currency name (for example, \c USD, \c EUR, \c AUD, \c GBP, \c DEM, etc)
    and the price per unit.

    In addition to the call-specific call meter, there is an accumulated call
    meter that keeps track of all calls made since the last time the meter was
    reset with requestAccumulatedCallMeter().

    The client can set a maximum accumulated call meter value with
    setAccumulatedCallMeterMaximum().  When the accumulated call meter is within
    30 seconds of expiry, the callMeterMaximumWarning() signal will be emitted to
    warn the user that their account is almost depleted.

    \sa QCommInterface
    \ingroup telephony
*/

/*!
    Construct a new advice of charge object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports advice of charge.  If there is more
    than one service that supports advice of charge, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QAdviceOfCharge objects for each.

    \sa QCommServiceManager::supports()
*/
QAdviceOfCharge::QAdviceOfCharge
        ( const QString& service, QObject *parent, QCommInterface::Mode mode )
    : QCommInterface( "QAdviceOfCharge", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this advice of charge object.
*/
QAdviceOfCharge::~QAdviceOfCharge()
{
}

/*!
    Request the current call meter value, which will be reported via
    the currentCallMeter() signal.

    \sa currentCallMeter()
*/
void QAdviceOfCharge::requestCurrentCallMeter()
{
    invoke( SLOT(requestCurrentCallMeter()) );
}

/*!
    Request the accumulated call meter value, which will be reported
    via the accumulatedCallMeter() signal.

    \sa accumulatedCallMeter(), requestAccumulatedCallMeterMaximum()
    \sa resetAccumulatedCallMeter()
*/
void QAdviceOfCharge::requestAccumulatedCallMeter()
{
    invoke( SLOT(requestAccumulatedCallMeter()) );
}

/*!
    Request the accumulated call meter maximum value, which will be
    reported via the accumulatedCallMeterMaximum() signal.

    \sa accumulatedCallMeterMaximum(), requestAccumulatedCallMeter()
    \sa setAccumulatedCallMeterMaximum(), callMeterMaximumWarning()
*/
void QAdviceOfCharge::requestAccumulatedCallMeterMaximum()
{
    invoke( SLOT(requestAccumulatedCallMeterMaximum()) );
}

/*!
    Request the price per unit information, which will be reported
    via the pricePerUnit() signal.

    \sa pricePerUnit(), setPricePerUnit()
*/
void QAdviceOfCharge::requestPricePerUnit()
{
    invoke( SLOT(requestPricePerUnit()) );
}

/*!
    Reset the accumulated call meter, with \a password supplying the
    \c{SIM PIN2} value.  The resetAccumulatedCallMeterResult() signal
    will be emitted to report the result of resetting the accumulated
    call meter.

    \sa accumulatedCallMeter(), requestAccumulatedCallMeter()
    \sa resetAccumulatedCallMeterResult()
*/
void QAdviceOfCharge::resetAccumulatedCallMeter( const QString& password )
{
    invoke( SLOT(resetAccumulatedCallMeter(QString)), password );
}

/*!
    Sets the accumulated call meter maximum \a value, with \a password
    supplying the \c{SIM PIN2} value.  The setAccumulatedCallMeterMaximumResult()
    signal will be emitted to report the result of setting the
    accumulated call meter maximum value.

    \sa requestAccumulatedCallMeterMaximum(), accumulatedCallMeterMaximum()
    \sa callMeterMaximumWarning(), setAccumulatedCallMeterMaximumResult()
*/
void QAdviceOfCharge::setAccumulatedCallMeterMaximum
        ( int value, const QString& password )
{
    invoke( SLOT(setAccumulatedCallMeterMaximum(int,QString)),
            value, password );
}

/*!
    Sets the price per unit information to \a unitPrice in \a currency,
    with \a password supplying the \c{SIM PIN2} value.  The setPricePerUnit()
    signal will be emitted to report the result of setting the price per unit.

    \sa pricePerUnit(), setPricePerUnitResult()
*/
void QAdviceOfCharge::setPricePerUnit
        ( const QString& currency, const QString& unitPrice,
          const QString& password )
{
    invoke( SLOT(setPricePerUnit(QString,QString,QString)),
            currency, unitPrice, password );
}

/*!
    \fn void QAdviceOfCharge::currentCallMeter( int value, bool explicitRequest )

    Signal that is emitted to report the \a value of the current call
    meter in units (zero if not set).  If \a explicitRequest is true,
    then the signal resulted because of a call to requestCurrentCallMeter().
    If \a explicitRequest is false, then the signal resulted because of
    an unsolicited notification from the modem of a change in the call meter.

    \sa requestCurrentCallMeter()
*/

/*!
    \fn void QAdviceOfCharge::accumulatedCallMeter( int value )

    Signal that is emitted to report the \a value of the accumulated call
    meter in units (zero if not set).

    \sa requestAccumulatedCallMeter(), resetAccumulatedCallMeter()
*/

/*!
    \fn void QAdviceOfCharge::accumulatedCallMeterMaximum( int value )

    Signal that is emitted in response to requestAccumulatedCallMeterMaximum()
    to report the maximum \a value that the call meter can obtain.
    The \a value will be zero if the call meter is unlimited, or not configured.

    \sa requestAccumulatedCallMeterMaximum(), callMeterMaximumWarning()
*/

/*!
    \fn void QAdviceOfCharge::callMeterMaximumWarning()

    Signal that is emitted when there is approximately 30 seconds of call
    time remaining, or there is less than 30 seconds of call time remaining
    when starting a new call.

    \sa accumulatedCallMeterMaximum(), requestAccumulatedCallMeterMaximum()
*/

/*!
    \fn void QAdviceOfCharge::pricePerUnit( const QString& currency, const QString& unitPrice )

    Signal that is emitted to response to requestPricePerUnit().
    The \a currency parameter will be a three-character currency code
    (for example, \c USD, \c EUR, \c AUD, \c GBP or \c DEM).
    The \a unitPrice parameter will be the price per unit, with a dot used
    as a decimal separator (for example, \c 2.66).

    The price is reported as a string to give maximum flexibility to
    client applications to convert it into a fixed-point or floating-point
    value to perform unit calculations.

    If \a currency is an empty string, then the price per unit information
    could not be obtained.

    \sa requestPricePerUnit()
*/

/*!
    \fn void QAdviceOfCharge::resetAccumulatedCallMeterResult( QTelephony::Result result )

    Signal that is emitted in response to resetAccumulatedCallMeter() to report
    the \a result of the operation.

    \sa resetAccumulatedCallMeter(), requestAccumulatedCallMeter()
*/

/*!
    \fn void QAdviceOfCharge::setAccumulatedCallMeterMaximumResult( QTelephony::Result result )

    Signal that is emitted in response to setAccumulatedCallMeterMaximum() to
    report the \a result of the operation.

    \sa setAccumulatedCallMeterMaximum(), requestAccumulatedCallMeterMaximum()
*/

/*!
    \fn void QAdviceOfCharge::setPricePerUnitResult( QTelephony::Result result )

    Signal that is emitted in response to setPricePerUnit() to
    report the \a result of the operation.

    \sa setPricePerUnit(), requestPricePerUnit()
*/
