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

#include "dialerservice.h"

/*!
    \service DialerService Dialer
    \inpublicgroup QtTelephonyModule
    \brief The DialerService class provides the Dialer service.

    The \i Dialer service enables applications to access the system dialer
    to place outgoing calls.

    The DialProxy server task implements this service for Qtopia.

    \sa DialProxy
*/

/*!
    \fn DialerService::DialerService( QObject *parent )
    \internal
*/

/*!
    \internal
*/
DialerService::~DialerService()
{
}

/*!
    \fn void DialerService::dialVoiceMail()

    Dials the user's voice mail service.

    This slot corresponds to the QCop service message
    \c{Dialer::dialVoiceMail()}.
*/

/*!
    \fn void DialerService::dial( const QString& name, const QString& number )

    Dials the specified \a number, tagged with the optional \a name.

    This slot corresponds to the QCop service message
    \c{Dialer::dial(QString,QString)}.
*/

/*!
    \fn void DialerService::dial( const QString& number, const QUniqueId& contact )

    Dials the specified \a contact, using the given \a number.

    This slot corresponds to the QCop service message
    \c{Dialer::dial(QUniqueId,QString)}.
*/

/*!
    \fn void DialerService::showDialer()

    Displays the dialer and clears all digits.

    This slot corresponds to the QCop service message
    \c{Dialer::showDialer()}.
*/
void DialerService::showDialer()
{
    showDialer(QString());
}

/*!
    \fn void DialerService::showDialer( const QString& digits )

    Displays the dialer, preloaded with \a digits.

    This slot corresponds to the QCop service message
    \c{Dialer::showDialer(QString)}.
*/

/*!
    \fn void DialerService::offHook()

    Displays the dialer if the application's guesture hint is QtopiaApplication::ShowDialer
    otherwise plays dial tone.
*/

/*!
    \fn void DialerService::onHook()

    Hangs up a call if any, otherwise closes dialer screen and stops playing dial tone.
*/

/*!
    \fn void DialerService::headset()

    Turns on or off the headset.
*/

/*!
    \fn void DialerService::speaker()

    Turns on or off the speaker.
*/

/*!
    \fn void DialerService::setDialToneOnlyHint( const QString &app )

    Sets the dial tone only hint for \a app.

    This hint is used by the phone server to determine
    whether the dialer should be shown or not when a dialing guesture has occurred
    depending on the active applications preference.

    For example, if the application does not have the capability to initiate phone calls
    and a dialing guesture occurred by picking up the receiver or pressing the speaker phone button,
    that means the user wants to type in a phone number. Hence the dialer should be shown.
    However, applications such as contacts and call history can initiate phone calls
    by selecting the number or pressing the button within the application
    and the dialing guesture doesn't mean typing the number in.
*/

/*!
    \fn void DialerService::redial()

    Redials the last phone number.
*/
