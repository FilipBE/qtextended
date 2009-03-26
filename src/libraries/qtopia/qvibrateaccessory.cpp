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

#include "qvibrateaccessory.h"

/*!
    \class QVibrateAccessory
    \inpublicgroup QtBaseModule


    \brief The QVibrateAccessory class provides access to the vibrate device on a phone.

    QVibrateAccessory can be used to control when the vibrate device activates
    with vibrateOnRing() and vibrateNow().  The usual way to turn on the
    vibrate device within a client application is as follows:

    \code
    QVibrateAccessory vib;
    vib.setVibrateNow( true );
    \endcode

    Vibrate device implementations should inherit from
    QVibrateAccessoryProvider.

    \sa QVibrateAccessoryProvider, QHardwareInterface

    \ingroup hardware
*/

/*!
    Construct a new vibrate abstraction object for provider \a id and attaches
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a id is empty, this class will use the first available
    provider that supports the vibrate interface.  If there is more
    than one service that supports the vibrate interface, the caller
    should enumerate them with QHardwareManager::providers()
    and create separate QVibrateAccessory objects for each.

    \sa QHardwareManager::providers()
*/
QVibrateAccessory::QVibrateAccessory
        ( const QString& id, QObject *parent,
          QAbstractIpcInterface::Mode mode )
    : QHardwareInterface( "QVibrateAccessory", id, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroys the vibrate accessory.
*/
QVibrateAccessory::~QVibrateAccessory()
{
}

/*!
    Returns true if the vibrate device will vibrate when an incoming call
    is detected; otherwise returns false.
*/
bool QVibrateAccessory::vibrateOnRing() const
{
    return value( "vibrateOnRing", false ).toBool();
}

/*!
    Returns true if the vibrate device is currently vibrating; otherwise returns false.
*/
bool QVibrateAccessory::vibrateNow() const
{
    return value( "vibrateNow", false ).toBool();
}

/*!
    Returns true if the vibrate device supports the vibrateOnRing() feature; otherwise returns false.
*/
bool QVibrateAccessory::supportsVibrateOnRing() const
{
    return value( "supportsVibrateOnRing", false ).toBool();
}

/*!
    Returns true if the vibrate device supports the vibrateNow() feature; otherwise returns false.
*/
bool QVibrateAccessory::supportsVibrateNow() const
{
    return value( "supportsVibrateNow", false ).toBool();
}

/*!
    Sets the vibrateOnRing() attribute to \a value.
*/
void QVibrateAccessory::setVibrateOnRing( const bool value )
{
    invoke( SLOT(setVibrateOnRing(bool)), value );
}

/*!
    Turns the vibrate device on if \a value is true; otherwise it is turned off.

    \sa vibrateNow()
*/
void QVibrateAccessory::setVibrateNow( const bool value )
{
    invoke( SLOT(setVibrateNow(bool)), value );
}

/*!
    \fn void QVibrateAccessory::vibrateOnRingModified()

    Signal that is emitted when vibrateOnRing() is modified.
*/

/*!
    \fn void QVibrateAccessory::vibrateNowModified()

    Signal that is emitted when vibrateNow() is modified.
*/

/*!
    \class QVibrateAccessoryProvider
    \inpublicgroup QtBaseModule


    \brief The QVibrateAccessoryProvider class provides an interface for vibrate devices to integrate into Qtopia.

    Vibrate devices inherit from this class and override setVibrateOnRing() and
    setVibrateNow() to implement the required functionality.  Subclasses should
    also call setSupportsVibrateOnRing() and setSupportsVibrateNow() to
    indicate the level of functionality that is supported.

    \sa QVibrateAccessory

    \ingroup hardware
*/

/*!
    Create a vibrate device provider called \a id and attaches it to \a parent.
*/
QVibrateAccessoryProvider::QVibrateAccessoryProvider
        ( const QString& id, QObject *parent )
    : QVibrateAccessory( id, parent, QAbstractIpcInterface::Server )
{
}

/*!
    Destroys the vibrate device provider.
*/
QVibrateAccessoryProvider::~QVibrateAccessoryProvider()
{
}

/*!
    Indicate whether this vibrate accessory supports vibrateOnRing()
    with \a value.  This is typically called from the constructor
    of subclass implementations.
*/
void QVibrateAccessoryProvider::setSupportsVibrateOnRing( bool value )
{
    setValue( "supportsVibrateOnRing", value );
}

/*!
    Indicate whether this vibrate provider supports vibrateNow()
    with \a value.  This is typically called from the constructor
    of subclass implementations.
*/
void QVibrateAccessoryProvider::setSupportsVibrateNow( bool value )
{
    setValue( "supportsVibrateNow", value );
}

/*!
    Sets the vibrateOnRing attribute to \a value. The default implementation
    updates the vibrateOnRing attribute as seen by vibrateOnRing() on the
    client.  Vibrate provider implementations should override this function
    to provide device-specific functionality and then call this implementation
    to update the client's view of the vibrateOnRing value.
*/
void QVibrateAccessoryProvider::setVibrateOnRing( const bool value )
{
    setValue( "vibrateOnRing", value );
    emit vibrateOnRingModified();
}

/*!
    Turns on the vibration device if \a value is true; otherwise it is turned
    off.  The default implementation updates the vibrateNow state as seen by
    vibrateNow() on the client.  Vibrate provider implementations should
    override this function to provide device-specific functionality and then
    call this implementation to update the client's view of the vibrateNow
    state.
*/
void QVibrateAccessoryProvider::setVibrateNow( const bool value )
{
    setValue( "vibrateNow", value );
    emit vibrateNowModified();
}
