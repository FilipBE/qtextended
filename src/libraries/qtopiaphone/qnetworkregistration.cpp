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

#include <qnetworkregistration.h>
#include <QDataStream>

/*!
    \class QNetworkRegistration
    \inpublicgroup QtTelephonyModule

    \brief The QNetworkRegistration class provides information about network operators.
    \ingroup telephony

    The network registration information that is reported includes the registrationState(),
    currentOperatorId(), currentOperatorName(), currentOperatorMode(), and
    currentOperatorTechnology().  On GSM networks, the information may also include
    the locationAreaCode() and cellId() for the current cell.

    All telephony services are expected to implement the QNetworkRegistration
    interface so that client applications can be made aware of when a service can
    be used to make phone calls or perform other telephony operations.  For simple
    telephony services (e.g. VoIP providers), it is sufficient that registrationState()
    be either \c RegistrationNone or \c RegistrationHome to indicate when the network is
    registered.

    \sa QNetworkRegistrationServer, QCommInterface
*/

/*!
    \class QNetworkRegistration::AvailableOperator
    \inpublicgroup QtTelephonyModule

    \brief The AvailableOperator class specifies information about a network operator that may be available for registration.

    The \c availability field indicates whether the operator is available
    for registration or not.  The \c name field indicates the primary
    name for the operator.  The \c shortName field indicates the secondary
    shorter name for the operator, which may be the same as \c name if a
    secondary name is not available The \c id field indicates an identifier
    for the operator, to be used with setCurrentOperator().
    The \c technology field indicates the type of network for the
    operator: \c GSM, \c UTRAN, \c VoIP, etc.
*/

/*!
    Construct a new network registration object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports network registration.  If there is more
    than one service that supports network registration, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QNetworkRegistration objects for each.

    \sa QCommServiceManager::supports()
*/
QNetworkRegistration::QNetworkRegistration
        ( const QString& service, QObject *parent, QCommInterface::Mode mode )
    : QCommInterface( "QNetworkRegistration", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this network registration object.
*/
QNetworkRegistration::~QNetworkRegistration()
{
}

/*!
    \property QNetworkRegistration::initialized
    \brief true if the network registration service has initialized the modem.
*/

/*!
    Returns true if the network registration service has initialized the
    modem and is ready to attempt to register to the network; false otherwise.
    Attempts to change the operator with setCurrentOperator() will
    always fail before this function returns true.

    \sa initializedChanged()
*/
bool QNetworkRegistration::initialized() const
{
    return value( "initialized" ).toBool();
}

/*!
    \property QNetworkRegistration::registrationState
    \brief the current network registration state.
*/

/*!
    Returns the current network registration state.

    \sa registrationStateChanged()
*/
QTelephony::RegistrationState QNetworkRegistration::registrationState() const
{
    return (QTelephony::RegistrationState)
            ( value( "state", (int)QTelephony::RegistrationNone ).toInt() );
}

/*!
    \property QNetworkRegistration::currentOperatorId
    \brief the identifier of the current network operator.
*/

/*!
    Returns the identifier of the current network operator, which can be used
    in a setCurrentOperator() request do change between OperatorModeAutomatic
    and OperatorModeManual.

    \sa currentOperatorMode(), currentOperatorTechnology()
    \sa currentOperatorChanged()
*/
QString QNetworkRegistration::currentOperatorId() const
{
    return value( "id", QString() ).toString();
}

/*!
    \property QNetworkRegistration::currentOperatorName
    \brief the name of the current network operator.
*/

/*!
    Returns the name of the current network operator.

    \sa currentOperatorMode(), currentOperatorTechnology()
    \sa currentOperatorChanged()
*/
QString QNetworkRegistration::currentOperatorName() const
{
    return value( "name", QString() ).toString();
}

/*!
    \property QNetworkRegistration::currentOperatorMode
    \brief the mode that the current network operator is operating in.
*/

/*!
    Returns the mode that the current network operator is operating in.

    \sa currentOperatorName(), currentOperatorTechnology()
    \sa currentOperatorChanged()
*/
QTelephony::OperatorMode QNetworkRegistration::currentOperatorMode() const
{
    return (QTelephony::OperatorMode)
        ( value( "mode", (int)QTelephony::OperatorModeAutomatic ).toInt() );
}

/*!
    \property QNetworkRegistration::currentOperatorTechnology
    \brief the technology used the current network operator.
*/

/*!
    Returns the technology used by the current network operator
    (\c GSM, \c UTRAN, \c VoIP, etc).

    \sa currentOperatorName(), currentOperatorMode(), currentOperatorChanged()
*/
QString QNetworkRegistration::currentOperatorTechnology() const
{
    return value( "technology", QString() ).toString();
}

/*!
    \property QNetworkRegistration::locationAreaCode
    \brief the GSM location area code, or \c lac.
*/

/*!
    Returns the GSM location area code, or \c lac.  Returns -1 if the location area
    code is unknown.  The locationChanged() signal can be used to detect
    changes in this value.

    \sa cellId(), locationChanged()
*/
int QNetworkRegistration::locationAreaCode() const
{
    return value( "locationAreaCode", -1 ).toInt();
}

/*!
    \property QNetworkRegistration::cellId
    \brief the GSM cell id, or \c ci.
*/

/*!
    Returns the GSM cell id, or \c ci.  Returns -1 if the cell id is unknown.
    The locationChanged() signal can be used to detect changes in
    this value.

    \sa locationAreaCode(), locationChanged()
*/
int QNetworkRegistration::cellId() const
{
    return value( "cellId", -1 ).toInt();
}

/*!
    Sets the current network operator information to \a mode, \a id,
    and \a technology.  The server will respond with the
    setCurrentOperatorResult() signal when the request completes.

    If the request succeeds, then the client may also receive the
    currentOperatorChanged() signal to indicate the newly selected values.
    The currentOperatorChanged() signal may be suppressed if the request
    succeeded, but the new values are the same as the previous ones.

    \sa setCurrentOperatorResult()
*/
void QNetworkRegistration::setCurrentOperator
        ( QTelephony::OperatorMode mode, const QString& id,
          const QString& technology )
{
    invoke( SLOT(setCurrentOperator(QTelephony::OperatorMode,QString,QString)),
            qVariantFromValue( mode ),
            qVariantFromValue( id ),
            qVariantFromValue( technology ) );
}

/*!
    Request a list of all available network operators.  The server will
    respond with the availableOperators() signal.

    The preferred operator list on the SIM card can be accessed with the
    QPreferredNetworkOperators class.

    \sa availableOperators(), QPreferredNetworkOperators
*/
void QNetworkRegistration::requestAvailableOperators()
{
    invoke( SLOT(requestAvailableOperators()) );
}

/*!
    \fn void QNetworkRegistration::initializedChanged()

    Signal that is emitted when the state of initialized() changes.

    \sa initialized()
*/

/*!
    \fn void QNetworkRegistration::registrationStateChanged()

    Signal that is emitted when the registration state changes.

    \sa registrationState()
*/

/*!
    \fn void QNetworkRegistration::locationChanged()

    Signal that is emitted when the locationAreaCode() or
    cellId() changes.

    If both the registrationState() and the location has changed,
    then registrationStateChanged() will be sent before locationChanged(),
    but the new location will already be available in slots connected to
    registrationStateChanged().

    \sa locationAreaCode(), cellId()
*/

/*!
    \fn void QNetworkRegistration::currentOperatorChanged()

    Signal that is emitted when the current operator information changes.

    \sa currentOperatorId(), currentOperatorName(), currentOperatorMode()
    \sa currentOperatorTechnology()
*/

/*!
    \fn void QNetworkRegistration::setCurrentOperatorResult( QTelephony::Result result )

    Signal that is emitted when a setCurrentOperator() request completes.
    The \a result parameter indicates the result of the request.

    \sa setCurrentOperator()
*/

/*!
    \fn void QNetworkRegistration::availableOperators( const QList<QNetworkRegistration::AvailableOperator>& opers )

    Signal that is emitted when a requestAvailableOperators() request
    completes.  The \a opers parameter contains the list of available
    operators.

    \sa requestAvailableOperators()
*/

/*!
    \class QNetworkRegistrationServer
    \inpublicgroup QtTelephonyModule

    \brief The QNetworkRegistrationServer class provides server-side support for updating the network registration state.
    \ingroup telephony

    Most telephony services will need to implement the QNetworkRegistration interface
    to indicate when network service becomes available or is no longer available.
    The QNetworkRegistrationServer provides some utility routines to make it easier
    to implement the QNetworkRegistration interface within telephony services.

    A telephony service that supports network registration should inherit from
    this class and call updateRegistrationState() and updateCurrentOperator()
    whenever the network parameters change.  Telephony services should also override
    setCurrentOperator() and requestAvailableOperators().

    \sa QNetworkRegistration, QCommInterface
*/

/*!
    Construct a server-side network registration object for \a service and
    attach it to \a parent.
*/
QNetworkRegistrationServer::QNetworkRegistrationServer
        ( const QString& service, QObject *parent )
    : QNetworkRegistration( service, parent, QCommInterface::Server )
{
}

/*!
    Destroy this network registration server object.
*/
QNetworkRegistrationServer::~QNetworkRegistrationServer()
{
}

/*!
    Update the state of initialized() to \a value and emit the initializedChanged() signal.

    \sa initialized(), initializedChanged()
*/
void QNetworkRegistrationServer::updateInitialized( bool value )
{
    if ( value != initialized() ) {
        setValue( "initialized", value );
        emit initializedChanged();
    }
}

/*!
    Update the registration \a state and notify all interested clients
    if it is different than the previous value.  The locationAreaCode()
    and cellId() will be set to -1.

    \sa registrationState(), registrationStateChanged()
    \sa locationAreaCode(), cellId()
*/
void QNetworkRegistrationServer::updateRegistrationState
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

    \sa registrationState(), registrationStateChanged()
    \sa locationAreaCode(), cellId()
*/
void QNetworkRegistrationServer::updateRegistrationState
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

/*!
    Update the current operator information to \a mode, \a id, \a name,
    and \a technology, and then notify all interested clients if the
    information is different than the previous values.

    \sa currentOperatorId(), currentOperatorName(), currentOperatorMode()
    \sa currentOperatorTechnology(), currentOperatorChanged()
*/
void QNetworkRegistrationServer::updateCurrentOperator
        ( QTelephony::OperatorMode mode, const QString& id,
          const QString& name, const QString& technology )
{
    if ( mode != currentOperatorMode() ||
         id != currentOperatorId() ||
         name != currentOperatorName() ||
         technology != currentOperatorTechnology() ) {
        setValue( "mode", (int)mode, Delayed );
        setValue( "id", id, Delayed );
        setValue( "name", name, Delayed );
        setValue( "technology", technology );
        emit currentOperatorChanged();
    }
}

/*!
    \internal
    \fn void QNetworkRegistration::AvailableOperator::serialize(Stream &stream) const
*/
template <typename Stream>
        void QNetworkRegistration::AvailableOperator::serialize(Stream &stream) const
{
    stream << (int)(availability);
    stream << name;
    stream << shortName;
    stream << id;
    stream << technology;
}

/*!
    \internal
    \fn void QNetworkRegistration::AvailableOperator::deserialize(Stream &stream)
*/
template <typename Stream>
        void QNetworkRegistration::AvailableOperator::deserialize(Stream &stream)
{
    int avail;
    stream >> avail;
    availability = (QTelephony::OperatorAvailability)avail;
    stream >> name;
    stream >> shortName;
    stream >> id;
    stream >> technology;
}

Q_IMPLEMENT_USER_METATYPE(QNetworkRegistration::AvailableOperator)
Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(QList<QNetworkRegistration::AvailableOperator>)
