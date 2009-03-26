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

#include <qpinmanager.h>

/*!
    \class QPinManager
    \inpublicgroup QtTelephonyModule

    \brief The QPinManager class provides a method for the system to query the user interface for pin values.
    \ingroup telephony

    The following pin types are available:

    \table
    \row \o \c{SIM PIN} \o Primary PIN for the SIM.
    \row \o \c{SIM PUK} \o Primary PUK for the SIM.
    \row \o \c{SIM PIN2} \o Secondary PIN for the SIM.
    \row \o \c{SIM PUK2} \o Secondary PUK for the SIM.
    \row \o \c{PH-SIM PIN} \o Lock phone to SIM PIN.
    \row \o \c{PH-FSIM PIN} \o Lock phone to first SIM PIN.
    \row \o \c{PH-FSIM PUK} \o Lock phone to first SIM PUK.
    \row \o \c{PH-NET PIN} \o Network personalization PIN.
    \row \o \c{PH-NET PUK} \o Network personalization PUK.
    \row \o \c{PH-NETSUB PIN} \o Network subset personalization PIN.
    \row \o \c{PH-NETSUB PUK} \o Network subset personalization PUK.
    \row \o \c{PH-SP PIN} \o Service provider personalization PIN.
    \row \o \c{PH-SP PUK} \o Service provider personalization PUK.
    \row \o \c{PH-CORP PIN} \o Corporate personalization PIN.
    \row \o \c{PH-CORP PUK} \o Corporate personalization PUK.
    \row \o \c{CNTRL PIN} \o Lock control surface PIN.
    \endtable

    \sa QCommInterface
*/

/*!
    Construct a new pin manager object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.
*/
QPinManager::QPinManager( const QString& service, QObject *parent,
                          QCommInterface::Mode mode )
    : QCommInterface( "QPinManager", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this pin manager object.
*/
QPinManager::~QPinManager()
{
}

/*!
    \enum QPinManager::Status
    This enum defines whether a pin is required, valid, or locked.

    \value NeedPin The pin value is required to be entered with enterPin()
    \value NeedPuk The puk and a new pin is required to be entered
           with enterPuk()
    \value Valid The pin that was entered with enterPin() or enterPuk()
           is valid.
    \value Locked Too many retries have occurred, the SIM is now locked,
           and attempts to use a puk will fail.
*/

/*!
    \enum QPinOptions::Format
    This enum defines the format of a pin, as a hint to input widgets
    as to what type of characters are valid within the pin.

    \value Number the widget needs numeric input.
    \value PhoneNumber the widget needs phone-style numeric input.
    \value Words the widget needs word input.
    \value Text the widget needs non-word input.
*/

/*!
    Query the status of the main \c{SIM PIN}.  The service will respond
    with a pinStatus() signal indicating the current status.  This method
    is typically used during system start up.

    \sa pinStatus()
*/
void QPinManager::querySimPinStatus()
{
    invoke( SLOT(querySimPinStatus()) );
}

/*!
    Enter a \a pin of a specific \a type.  This is typically sent in
    response to a pinStatus() signal that indicates NeedPin.

    If the pin is valid, then the manager will respond with a pinStatus()
    signal indicating a status of Valid.  If the pin is invalid, then the
    manager will response with a pinStatus() signal indicating a status
    of NeedPin or NeedPuk.

    \sa pinStatus(), cancelPin()
*/
void QPinManager::enterPin( const QString& type, const QString& pin )
{
    invoke( SLOT(enterPin(QString,QString)), type, pin );
}

/*!
    Enter a \a puk of a specific \a type, and change the associated pin
    to \a newPin.  This is typically sent in response to a pinStatus()
    signal that indicates NeedPuk.

    If the puk is valid, then the manager will respond with a pinStatus()
    signal indicating a status of Valid.  If the puk is invalid, then the
    manager will response with a pinStatus() signal indicating a status
    of NeedPuk or Locked.

    \sa pinStatus()
*/
void QPinManager::enterPuk( const QString& type, const QString& puk,
                            const QString& newPin )
{
    invoke( SLOT(enterPuk(QString,QString,QString)), type, puk, newPin );
}

/*!
    Indicate that the user has canceled entry of the pin of a
    specified \a type.  The operation that was waiting on the pin
    will be aborted with an error.

    \sa enterPin()
*/
void QPinManager::cancelPin( const QString& type )
{
    invoke( SLOT(cancelPin(QString)), type );
}

/*!
    Change a pin of a specific \a type from \a oldPin to \a newPin.
    The manager will respond by emitting the changePinResult() signal.

    \sa changePinResult()
*/
void QPinManager::changePin( const QString& type, const QString& oldPin,
                             const QString& newPin )
{
    invoke( SLOT(changePin(QString,QString,QString)), type, oldPin, newPin );
}

/*!
    Request the lock status of a pin with a specific \a type.
    The manager will respond by emitting the lockStatus() signal.

    \sa lockStatus(), setLockStatus()
*/
void QPinManager::requestLockStatus( const QString& type )
{
    invoke( SLOT(requestLockStatus(QString)), type );
}

/*!
    Sets the lock status of a pin with a specific \a type to \a enabled.
    If \a enabled is true, then the feature will be locked and a pin
    is required whenever the feature is used.  If \a enabled is false,
    then the feature will be available at any time without a pin.

    The \a password parameter is the password corresponding to the lock,
    to turn it on or off.

    The manager will respond by emitting the setLockStatusResult() signal.

    \sa setLockStatusResult(), requestLockStatus()
*/
void QPinManager::setLockStatus
    ( const QString& type, const QString& password, bool enabled )
{
    invoke( SLOT(setLockStatus(QString,QString,bool)),
            type, password, enabled );
}

/*!
    \fn void QPinManager::pinStatus( const QString& type, QPinManager::Status status, const QPinOptions& options )

    Signal that is emitted to indicate the \a status of a pin with the
    specified \a type.  If the status is NeedPin, then the user interface
    should ask for the pin and return it via enterPin().  If the status
    is NeedPuk, then the user interface should ask for a puk and a new pin,
    and return them via enterPuk().  For both NeedPin and NeedPuk,
    the function cancelPin() can be called to cancel the pin entry.

    If the status is NeedPin or NeedPuk, then \a options provides
    additional information that may affect the type of user interface
    to use when asking the user for the pin or puk.

    The \a type will be \c READY and \a status will be \c Valid when
    the SIM has been given sufficient pin's and puk's to return it
    to a ready state.

    \sa querySimPinStatus(), enterPin(), enterPuk(), cancelPin()
*/

/*!
    \fn void QPinManager::changePinResult( const QString& type, bool valid )

    Signal that is emitted in response to a changePin() request for \a type.
    The \a valid flag indicates whether the request succeeded or failed.

    \sa changePin()
*/

/*!
    \fn void QPinManager::lockStatus( const QString& type, bool enabled )

    Signal that is emitted in response to requestLockStatus() for \a type.
    If \a enabled is true, then the feature will be locked and a pin
    is required whenever the feature is used.  If \a enabled is false,
    then the feature will be available at any time without a pin.

    \sa requestLockStatus()
*/

/*!
    \fn void QPinManager::setLockStatusResult( const QString& type, bool valid )

    Signal that is emitted in response to setLockStatus() for \a type.
    The \a valid flag indicates whether the request succeeded or failed.

    \sa setLockStatus()
*/

/*!
    \class QPinOptions
    \inpublicgroup QtTelephonyModule

    \brief The QPinOptions class provides information about a pin that may be useful in determining the type of user interface to use.
    \ingroup telephony

    Pin options are sent to client applications as a parameter to the
    QPinManager::pinStatus() signal.

    \sa QPinManager
*/

class QPinOptionsPrivate
{
public:
    QPinOptionsPrivate()
    {
        format = QPinOptions::Number;
        minLength = 0;
        maxLength = 32;
        canCancel = false;
    }

    QString prompt;
    QPinOptions::Format format;
    int minLength;
    int maxLength;
    bool canCancel;

    void copy( const QPinOptionsPrivate& d )
    {
        prompt = d.prompt;
        format = d.format;
        minLength = d.minLength;
        maxLength = d.maxLength;
        canCancel = d.canCancel;
    }
};

/*!
    \internal
    \fn void QPinOptions::deserialize(Stream &stream)
*/
template <typename Stream> void QPinOptions::deserialize(Stream &stream)
{
    QString tempstr;
    int tempval;
    stream >> tempstr;
    setPrompt( tempstr );
    stream >> tempval;
    setFormat( (QPinOptions::Format)tempval );
    stream >> tempval;
    setMinLength( tempval );
    stream >> tempval;
    setMaxLength( tempval );
    stream >> tempval;
    setCanCancel( tempval != 0 );
}

/*!
    \internal
    \fn void QPinOptions::serialize(Stream &stream) const
*/
template <typename Stream> void QPinOptions::serialize(Stream &stream) const
{
    stream << prompt();
    stream << (int)format();
    stream << minLength();
    stream << maxLength();
    stream << (int)canCancel();
}

/*!
    Construct an empty pin options block with default values.
*/
QPinOptions::QPinOptions()
{
    d = new QPinOptionsPrivate();
}

/*!
    Construct a copy of \a other.
*/
QPinOptions::QPinOptions( const QPinOptions& other )
{
    d = new QPinOptionsPrivate();
    d->copy( *other.d );
}

/*!
    Destroy this pin options block.
*/
QPinOptions::~QPinOptions()
{
    delete d;
}

/*!
    Make a copy of \a other.
*/
QPinOptions& QPinOptions::operator=( const QPinOptions& other )
{
    if ( this != &other )
        d->copy( *other.d );
    return *this;
}

/*!
    Returns the prompt to display to the user.  May be the empty string if
    the prompt is implicitly specified by the pin type.  The default value
    is an empty string.

    \sa setPrompt()
*/
QString QPinOptions::prompt() const
{
    return d->prompt;
}

/*!
    Sets the prompt to \a value.

    \sa prompt()
*/
void QPinOptions::setPrompt( const QString& value )
{
    d->prompt = value;
}

/*!
    Returns the format of the pin.  The default value is QPinManager::Number.

    \sa setFormat()
*/
QPinOptions::Format QPinOptions::format() const
{
    return d->format;
}

/*!
    Sets the pin format to \a value.

    \sa format()
*/
void QPinOptions::setFormat( QPinOptions::Format value )
{
    d->format = value;
}

/*!
    Returns the minimum length of pin to be supplied.  The default
    value is zero.

    \sa setMinLength()
*/
int QPinOptions::minLength() const
{
    return d->minLength;
}

/*!
    Sets the minimum pin length to \a value.

    \sa minLength()
*/
void QPinOptions::setMinLength( int value )
{
    d->minLength = value;
}

/*!
    Returns the maximum length of pin to be supplied.  The default
    value is 32.

    \sa setMaxLength()
*/
int QPinOptions::maxLength() const
{
    return d->maxLength;
}

/*!
    Sets the maximum pin length to \a value.

    \sa maxLength()
*/
void QPinOptions::setMaxLength( int value )
{
    d->maxLength = value;
}

/*!
    Returns true if the request can be canceled by the user and normal
    modem operations will continue after failing the current operation.
    If this returns false, then the user must supply a pin for normal modem
    operations to proceed.

    \sa setCanCancel()
*/
bool QPinOptions::canCancel() const
{
    return d->canCancel;
}

/*!
    Sets the pin cancel state to \a value.

    \sa canCancel()
*/
void QPinOptions::setCanCancel( bool value )
{
    d->canCancel = value;
}

Q_IMPLEMENT_USER_METATYPE_ENUM(QPinManager::Status)
Q_IMPLEMENT_USER_METATYPE(QPinOptions)
