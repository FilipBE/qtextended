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

#include <qgprsnetworkregistration.h>

/*!
    \class QGprsNetworkRegistration
    \inpublicgroup QtTelephonyModule

    \brief The QGprsNetworkRegistration class provides information about GPRS network registration.
    \ingroup telephony

    The QNetworkRegistration class provides information about general
    GSM network registration.  The QGprsNetworkRegistration class
    provides information about an ongoing GPRS session.

    \sa QNetworkRegistration, QGprsNetworkRegistrationServer, QCommInterface
*/

/*!
    Construct a new GPRS network registration object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports GPRS network registration.  If there is more
    than one service that supports GPRS network registration, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QGprsNetworkRegistration objects for each.

    \sa QCommServiceManager::supports()
*/
QGprsNetworkRegistration::QGprsNetworkRegistration
        ( const QString& service, QObject *parent, QCommInterface::Mode mode )
    : QCommInterface( "QGprsNetworkRegistration", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this GPRS network registration object.
*/
QGprsNetworkRegistration::~QGprsNetworkRegistration()
{
}

/*!
    \property QGprsNetworkRegistration::registrationState
    \brief the current GPRS network registration state.
*/

/*!
    Returns the current GPRS network registration state.

    \sa registrationStateChanged()
*/
QTelephony::RegistrationState
    QGprsNetworkRegistration::registrationState() const
{
    return (QTelephony::RegistrationState)
            ( value( "state", (int)QTelephony::RegistrationNone ).toInt() );
}

/*!
    \property QGprsNetworkRegistration::locationAreaCode
    \brief the GSM location area code, or \c lac.
*/

/*!
    Returns the GSM location area code, or \c lac.  Returns -1 if the location area
    code is unknown.  The locationChanged() signal can be used to detect
    changes in this value.

    \sa cellId(), locationChanged()
*/
int QGprsNetworkRegistration::locationAreaCode() const
{
    return value( "locationAreaCode", -1 ).toInt();
}

/*!
    \property QGprsNetworkRegistration::cellId
    \brief the GSM cell id, or \c ci.
*/

/*!
    Returns the GSM cell id, or \c ci.  Returns -1 if the cell id is unknown.
    The locationChanged() signal can be used to detect changes in
    this value.

    \sa locationAreaCode(), locationChanged()
*/
int QGprsNetworkRegistration::cellId() const
{
    return value( "cellId", -1 ).toInt();
}

/*!
    \fn void QGprsNetworkRegistration::registrationStateChanged()

    Signal that is emitted when the registration state changes.

    \sa registrationState()
*/

/*!
    \fn void QGprsNetworkRegistration::locationChanged()

    Signal that is emitted when the locationAreaCode() or
    cellId() changes.

    If both the registrationState() and the location has changed,
    then registrationStateChanged() will be sent before locationChanged(),
    but the new location will already be available in slots connected to
    registrationStateChanged().

    \sa locationAreaCode(), cellId()
*/

/*!
    \class QGprsNetworkRegistrationServer
    \inpublicgroup QtTelephonyModule

    \brief The QGprsNetworkRegistrationServer class provides server-side support for updating the GPRS network registration state.
    \ingroup telephony

    A telephony service that supports GPRS network registration should
    create an instance of this class and call updateRegistrationState()
    whenever the GPRS network registration parameters change.

    \sa QGprsNetworkRegistration, QCommInterface
*/

/*!
    Construct a server-side GPRS network registration object for \a service
    and attach it to \a parent.
*/
QGprsNetworkRegistrationServer::QGprsNetworkRegistrationServer
            ( const QString& service, QObject *parent )
    : QGprsNetworkRegistration( service, parent, Server )
{
}

/*!
    Destroy this GPRS network registration server object.
*/
QGprsNetworkRegistrationServer::~QGprsNetworkRegistrationServer()
{
}

/*!
    Update the registration \a state and notify all interested clients
    if it is different than the previous value.  The locationAreaCode()
    and cellId() will be set to -1.

    \sa QGprsNetworkRegistration::registrationState()
    \sa QGprsNetworkRegistration::registrationStateChanged()
*/
void QGprsNetworkRegistrationServer::updateRegistrationState
        ( QTelephony::RegistrationState state )
{
    if ( locationAreaCode() != -1 || cellId() != -1 ) {
        if ( state != registrationState() ) {
            // Update everything.
            setValue( "state", (int)state, Delayed );
            setValue( "locationAreaCode", -1, Delayed );
            setValue( "cellId", -1 );
            emit registrationStateChanged();
            emit locationChanged();
        } else {
            // Update only the location.
            setValue( "locationAreaCode", -1, Delayed );
            setValue( "cellId", -1 );
            emit locationChanged();
        }
    } else if ( state != registrationState() ) {
        // Update only registration.
        setValue( "state", (int)state );
        emit registrationStateChanged();
    }
}

/*!
    Update the registration \a state, \a locationAreaCode, and \a cellId,
    and notify all interested clients if they are different than the
    previous values.

    \sa QGprsNetworkRegistration::registrationState()
    \sa QGprsNetworkRegistration::locationAreaCode()
    \sa QGprsNetworkRegistration::cellId()
    \sa QGprsNetworkRegistration::registrationStateChanged()
    \sa QGprsNetworkRegistration::locationChanged()
*/
void QGprsNetworkRegistrationServer::updateRegistrationState
        ( QTelephony::RegistrationState state,
          int locationAreaCode, int cellId )
{
    if ( locationAreaCode != this->locationAreaCode() ||
         cellId != this->cellId() ) {
        if ( state != registrationState() ) {
            // Update everything.
            setValue( "state", (int)state, Delayed );
            setValue( "locationAreaCode", locationAreaCode, Delayed );
            setValue( "cellId", cellId );
            emit registrationStateChanged();
            emit locationChanged();
        } else {
            // Update only the location.
            setValue( "locationAreaCode", locationAreaCode, Delayed );
            setValue( "cellId", cellId );
            emit locationChanged();
        }
    } else if ( state != registrationState() ) {
        // Update only registration.
        setValue( "state", (int)state );
        emit registrationStateChanged();
    }
}
