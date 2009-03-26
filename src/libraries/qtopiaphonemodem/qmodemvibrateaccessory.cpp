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

#include <qmodemvibrateaccessory.h>
#include <qmodemservice.h>

/*!
    \class QModemVibrateAccessory
    \inpublicgroup QtCellModule

    \brief The QModemVibrateAccessory class provides vibrate accessory support for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CVIB} command from 3GPP TS 27.007.

    QModemVibrateAccessory implements the QVibrateAccessory hardware interface.  Client
    applications should use QVibrateAccessory instead of this class to
    access the modem's vibration settings.

    \sa QVibrateAccessory
*/

/*!
    Create an AT-based vibrate accessory for \a service.
*/
QModemVibrateAccessory::QModemVibrateAccessory( QModemService *service )
    : QVibrateAccessoryProvider( service->service(), service )
{
    this->service = service;
    setSupportsVibrateOnRing( true );
}

/*!
    Destroy this AT-based vibrate accessory.
*/
QModemVibrateAccessory::~QModemVibrateAccessory()
{
}

/*!
    \reimp
*/
void QModemVibrateAccessory::setVibrateOnRing( const bool value )
{
    if ( value )
        service->chat( "AT+CVIB=1" );
    else
        service->chat( "AT+CVIB=0" );
    QVibrateAccessoryProvider::setVibrateOnRing( value );
}

/*!
    Sets the vibrateNow() state to \a value.  This is not used in this
    implementation because \c{AT+CVIB} does not support immediate vibrate.
    Subclasses should override this if the modem supports immediate vibrate.
*/
void QModemVibrateAccessory::setVibrateNow( const bool value )
{
    // Not used.
    Q_UNUSED(value);
}
