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

#include <qcallsettings.h>

/*!
    \class QCallSettings
    \inpublicgroup QtTelephonyModule

    \brief The QCallSettings class provides access to auxiliary settings in a GSM phone related to calls.
    \ingroup telephony

    Other classes such as QCallForwarding, QCallBarring, QNetworkRegistration,
    etc, are used for specific call settings that have rich interfaces.
    The QCallSettings class is for minor settings that don't require more
    than simple request and set methods.

    \sa QCallForwarding, QCallBarring, QNetworkRegistration
*/

/*!
    \enum QCallSettings::CallerIdRestriction
    This enum defines the states that the caller ID restriction service can be in.

    \value Subscription Caller ID restriction is according to the network operator's subscription preferences.
    \value Invoked Caller ID restriction is currently invoked.  That is, the
           caller ID information of the local party will not be sent during
           calls.
    \value Suppressed Caller ID restriction is currently suppressed.  That is,
           the caller ID information of the local party will be sent
           during calls.
*/

/*!
    \enum QCallSettings::CallerIdRestrictionStatus
    This enum defines the status of the caller ID restriction service within the network, indicating whether it can be modified or not.

    \value NotProvisioned The caller ID restriction service is not provisioned.
    \value Permanent The caller ID restriction service is provisioned in permanent mode and the local user cannot change it.
    \value Unknown The caller ID restriction state is currently unknown (e.g. no network).
    \value TemporaryRestricted The caller ID restriction service is provisioned in temporary restricted mode.
    \value TemporaryAllowed The caller ID restriction service is provisioned in temporary allowed mode.
*/

/*!
    \enum QCallSettings::SmsTransport
    This enum defines the transport to be used for sending SMS messages.

    \value SmsTransportPD Use GPRS packet domain to send SMS messages.
    \value SmsTransportCS Use non-GPRS circuit switching to send SMS messages.
    \value SmsTransportPDPreferred Use GPRS packet domain if available, or non-GPRS circuit switched otherwise.
    \value SmsTransportCSPreferred Use non-GPRS circuit switching if available, or GPRS packet domain otherwise.
    \value SmsTransportUnavailable The modem does not support selecting an SMS transport.
*/

/*!
    \enum QCallSettings::PresentationStatus
    This enum defines the presentation status of caller id information.

    \value PresentationNotProvisioned Presentation of caller id has not been provisioned by the network operator.
    \value PresentationProvisioned Presentation of caller id has been provisioned by the network operator.
    \value PresentationUnknown Presentation status is unknown, probably because of no network service.
*/

/*!
    Construct a new call settings object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports call settings.  If there is more than one
    service that supports call settings, the caller should enumerate
    them with QCommServiceManager::supports() and create separate
    QCallSettings objects for each.

    \sa QCommServiceManager::supports()
*/
QCallSettings::QCallSettings( const QString& service, QObject *parent,
                              QCommInterface::Mode mode )
    : QCommInterface( "QCallSettings", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this call settings object.
*/
QCallSettings::~QCallSettings()
{
}

/*!
    Request the current call waiting flags.  The service responds by
    emitting the callWaiting() signal.

    On AT-based modems, this will typically use the \c{AT+CCWA} command.

    \sa callWaiting()
*/
void QCallSettings::requestCallWaiting()
{
    invoke( SLOT(requestCallWaiting()) );
}

/*!
    Enable or disable call waiting for the call classes specified
    in \a cls, according to the \a enable flag.  The service responds
    by emitting the setCallWaitingResult() signal.

    On AT-based modems, this will typically use the \c{AT+CCWA} command.

    \sa setCallWaitingResult()
*/
void QCallSettings::setCallWaiting( bool enable, QTelephony::CallClass cls )
{
    invoke( SLOT(setCallWaiting(bool,QTelephony::CallClass)),
            enable, qVariantFromValue( cls ) );
}

/*!
    Request the current state of the caller ID restriction service.
    The service responds by emitting the callerIdRestriction() signal.

    On AT-based modems, this will typically use the \c{AT+CLIR} command.

    \sa callerIdRestriction()
*/
void QCallSettings::requestCallerIdRestriction()
{
    invoke( SLOT(requestCallerIdRestriction()) );
}

/*!
    Sets the caller ID restriction state to \a clir.  The service responds
    by emitting the setCallerIdRestrictionResult() signal.

    On AT-based modems, this will typically use the \c{AT+CLIR} command.

    \sa setCallerIdRestrictionResult()
*/
void QCallSettings::setCallerIdRestriction
        ( QCallSettings::CallerIdRestriction clir )
{
    invoke( SLOT(setCallerIdRestriction(QCallSettings::CallerIdRestriction)),
            qVariantFromValue( clir ) );
}

/*!
    Request the transport that is being used to send SMS messages.
    The service responds by emitting the smsTransport() signal.

    On AT-based modems, this will typically use the \c{AT+CGSMS} command.

    \sa smsTransport()
*/
void QCallSettings::requestSmsTransport()
{
    invoke( SLOT(requestSmsTransport()) );
}

/*!
    Sets the \a transport to be used for sending SMS messages.  The service responds
    by emitting the setSmsTransportResult() signal.

    On AT-based modems, this will typically use the \c{AT+CGSMS} command.

    \sa setSmsTransportResult()
*/
void QCallSettings::setSmsTransport( QCallSettings::SmsTransport transport )
{
    invoke( SLOT(setSmsTransport(QCallSettings::SmsTransport)),
            qVariantFromValue( transport ) );
}

/*!
    Request the presentation state of caller id information on
    incoming calls.  The state will be returned via the
    callerIdPresentation() signal.

    On AT-based modems, this will typically use the \c{AT+CLIP} command.

    \sa callerIdPresentation()
*/
void QCallSettings::requestCallerIdPresentation()
{
    invoke( SLOT(requestCallerIdPresentation()) );
}

/*!
    Request the presentation state of connected line identification
    information (COLP).  The state will be returned via the
    connectedIdPresentation() signal.

    On AT-based modems, this will typically use the \c{AT+COLP} command.

    \sa connectedIdPresentation()
*/
void QCallSettings::requestConnectedIdPresentation()
{
    invoke( SLOT(requestConnectedIdPresentation()) );
}

/*!
    \fn void QCallSettings::callWaiting( QTelephony::CallClass cls )

    Signal that is emitted in response to a requestCallWaiting() request.
    The call classes in \a cls currently have call waiting enabled.
    All other classes have call waiting disabled.

    \sa requestCallWaiting()
*/

/*!
    \fn void QCallSettings::setCallWaitingResult( QTelephony::Result result )

    Signal that is emitted to report the \a result of a
    setCallWaiting() request.

    \sa setCallWaiting()
*/

/*!
    \fn void QCallSettings::callerIdRestriction( QCallSettings::CallerIdRestriction clir, QCallSettings::CallerIdRestrictionStatus status )

    Signal that is emitted in response to a requestCallerIdRestriction()
    request.  The current state is specified by the \a clir and \a status
    parameters.

    \sa requestCallerIdRestriction()
*/

/*!
    \fn void QCallSettings::setCallerIdRestrictionResult( QTelephony::Result result )

    Signal that is emitted to report the \a result of a
    setCallerIdRestriction() request.

    \sa setCallerIdRestriction()
*/

/*!
    \fn void QCallSettings::smsTransport( QCallSettings::SmsTransport transport )

    Signal that is emitted in response to a requestSmsTransport() request.
    The current transport is specified by the \a transport parameter.
    If \a transport is \c SmsTransportUnavailable, then the modem does
    not support selecting an SMS transport.

    \sa requestSmsTransport()
*/

/*!
    \fn void QCallSettings::setSmsTransportResult( QTelephony::Result result )

    Signal that is emitted to report the \a result of a
    setSmsTransport() request.

    \sa setSmsTransport()
*/

/*!
    \fn void QCallSettings::callerIdPresentation( QCallSettings::PresentationStatus status )

    Signal that is emitted to report the current caller id presentation
    \a status.

    \sa requestCallerIdPresentation()
*/

/*!
    \fn void QCallSettings::connectedIdPresentation( QCallSettings::PresentationStatus status )

    Signal that is emitted to report the current connected line id
    presentation \a status.

    \sa requestConnectedIdPresentation()
*/

Q_IMPLEMENT_USER_METATYPE_ENUM(QCallSettings::CallerIdRestriction)
Q_IMPLEMENT_USER_METATYPE_ENUM(QCallSettings::CallerIdRestrictionStatus)
Q_IMPLEMENT_USER_METATYPE_ENUM(QCallSettings::SmsTransport)
Q_IMPLEMENT_USER_METATYPE_ENUM(QCallSettings::PresentationStatus)
