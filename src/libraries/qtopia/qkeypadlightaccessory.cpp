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

#include "qkeypadlightaccessory.h"

/*!
    \class QKeypadLightAccessory
    \inpublicgroup QtBaseModule


    \brief The QKeypadLightAccessory class provides access to the keypad light on a phone.

    QKeypadLightAccessory can be used to turn the keypad light on and off using
    setOn().  For example the following code can be used to turn on the keypad
    light from within a client application:

    \code
    QKeypadLightAccessory light;
    light.setOn( true );
    \endcode

    Keypad light implementations should inherit from
    QKeypadLightAccessoryProvider.

    \sa QKeypadLightAccessoryProvider, QHardwareInterface

    \ingroup hardware
*/

/*!
    Construct a new keypad light  bstraction for \a id and attaches
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a id is empty, this class will use the first available
    provider that supports the keypad light interface.  If there is
    more than one service that supports the keypad light interface, the caller
    should enumerate them with QHardwareManager::providers()
    and create separate QKeypadLightAccessory objects for each.

    \sa QHardwareManager::providers()
*/
QKeypadLightAccessory::QKeypadLightAccessory
      ( const QString& id, QObject *parent, QAbstractIpcInterface::Mode mode )
    : QHardwareInterface( "QKeypadLightAccessory", id, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroys the keypad light accessory object.
*/
QKeypadLightAccessory::~QKeypadLightAccessory()
{
}

/*!
    Returns true if the keypad light is currently on; otherwise returns false.
*/
bool QKeypadLightAccessory::on() const
{
    return value( "on", false ).toBool();
}

/*!
    Turns the keypad light on if \a value is true; otherwise it is turned off.
*/
void QKeypadLightAccessory::setOn( const bool value )
{
    invoke( SLOT(setOn(bool)), value );
}

/*!
    \fn void QKeypadLightAccessory::onModified()

    Signal that is emitted when the state of the keypad light changes.
*/

/*!
    \class QKeypadLightAccessoryProvider
    \inpublicgroup QtBaseModule


    \brief The QKeypadLightAccessoryProvider class provides an interface for keypad light devices to integrate into Qtopia.

    Keypad light devices inherit from this class and override setOn() to
    implement the required functionality.

    \sa QKeypadLightAccessory

    \ingroup hardware
*/

/*!
    Create a keypad light device provider called \a id and attaches it to \a parent.
*/
QKeypadLightAccessoryProvider::QKeypadLightAccessoryProvider
        ( const QString& id, QObject *parent )
    : QKeypadLightAccessory( id, parent, QAbstractIpcInterface::Server )
{
}

/*!
    Destroys the keypad light provider.
*/
QKeypadLightAccessoryProvider::~QKeypadLightAccessoryProvider()
{
}

/*!
    Turns the keypad light on if \a value is true; otherwise it is turned off.
    The default implementation updates the state of the keypad light as seen by
    on() on the client.  Keypad light implementations should override this
    function to provide device-specific functionality and then call this
    implementation to update the client's view of the state of the keypad light.
*/
void QKeypadLightAccessoryProvider::setOn( const bool value )
{
    setValue( "on", value );
    emit onModified();
}
