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

#include "qabstractcallpolicymanager.h"

/*!
    \class QAbstractCallPolicyManager
    \inpublicgroup QtTelephonyModule
    \brief The QAbstractCallPolicyManager class provides an interface for the management of calls across multiple call types.
    \ingroup QtopiaServer::Task::Interfaces
    \ingroup QtopiaServer::Telephony

    When the user enters a number or URI to be dialed, or selects one from
    their list of contacts, Qt Extended needs to decide how to place and
    manage the call.  This decision can be quite complex when multiple
    call types (e.g. GSM and VoIP) are registered with the system.
    Some examples are:

    \list
    \o Emergency numbers such as 112 and 911 must be placed via a cellular
       network because VoIP network typically won't have the facility.
    \o Ordinary numbers could be placed via either GSM or VoIP, if the user's
       configured VoIP server can carry PSTN traffic, and the user is in
       range of a VoIP-capable access point.
    \o If the "number" is actually a SIP URI, it can only be placed via VoIP.
    \endlist

    The purpose of the QAbstractCallPolicyManager class is to allow these
    decisions to be separated from the core Qt Extended code, and to ease the
    integration of new call types in the future.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
    \sa CellModemManager, VoIPManager
*/

/*!
    \fn QAbstractCallPolicyManager::QAbstractCallPolicyManager(QObject *parent)

    Create a new call policy manager and attach it to \a parent.
*/

/*!
    \fn void QAbstractCallPolicyManager::updateOnThePhonePresence( bool isOnThePhone )
    
    If \a isOnThePhone is true, updates the local presence to away.
    Otherwise updates the local presence to online. 
    If the call policy manager does
    not support presence, this function should do nothing.
*/

/*!
    \fn bool QAbstractCallPolicyManager::doDnd()

    Toggles the status between Do not disturb and normal status.
    Returns true if Do Not Disturb state is activated, otherwise returns false.
*/

/*!
    \enum QAbstractCallPolicyManager::CallHandling
    This enum defines how to handle a call that is about to be placed,
    based on the number to be dialed.

    \value CannotHandle The number cannot be handled by this policy manager
           at all, or network registration is not available so it cannot
           be handled at this time.
    \value CanHandle The call can be handled by this policy manager.  If
           multiple policy managers return this value, the user will be given
           a choice of call types before the call is placed.
    \value MustHandle The call must be handled by this policy manager, and
           all other policy managers that return \c CanHandle should be
           ignored.  This is typically used for emergency calls that must
           be placed via a specific network (e.g. GSM).  It could also be
           returned if the user has configured a specific policy manager
           as their preferred call type.
    \value NeverHandle The call must never be handled by any policy manager.
           The call will be rejected.  This is typically used for emergency
           calls when the device is in airplane mode.  The policy manager
           would normally return \c MustHandle, but cannot right now.
*/

/*!
    \fn QString QAbstractCallPolicyManager::callType() const

    Returns the call type that will be used to create the QPhoneCall object
    with QPhoneCallManager::create().

    \sa trCallType(), callTypeIcon()
*/

/*!
    \fn QString QAbstractCallPolicyManager::trCallType() const

    Returns the translated, human-readable, version of the call type.

    \sa callType(), callTypeIcon()
*/

/*!
    \fn QString QAbstractCallPolicyManager::callTypeIcon() const

    Returns the icon to display to indicate the call type.  This is typically
    displayed next to the trCallType() text.

    \sa callType(), trCallType()
*/

/*!
    \fn QTelephony::RegistrationState QAbstractCallPolicyManager::registrationState() const

    Returns the current registration state of the telephony service being used
    by this call policy manager.

    \sa registrationChanged(), registrationMessage(), registrationIcon()
*/

/*!
    \fn QAbstractCallPolicyManager::CallHandling QAbstractCallPolicyManager::handling(const QString& number)

    Determine how to handle calls to \a number.
*/

/*!
    \fn bool QAbstractCallPolicyManager::supportsPresence() const

    Returns \c true if the policy manager supports presence.
*/

/*!
    \fn bool QAbstractCallPolicyManager::isAvailable(const QString& number)

    If the call policy manager supports presence, determine if the party
    at \a number is currently available.  If the call policy manager does
    not support presence, this function should always return true.
*/

/*!
    \fn QString QAbstractCallPolicyManager::registrationMessage() const

    Returns a human-readable message to be displayed to indicate the
    current registration state of the telephony service being used
    by this call policy manager.  This function is called whenever
    the registration state changes.

    Returns an empty QString if the call policy manager does not wish
    to have a message displayed.  Messages are normally only displayed
    if the registration is not present, or the telephony service is
    searching for a new network to register to.  Once successfully
    registered, the returned message should be empty.

    \sa registrationChanged(), registrationState(), registrationIcon()
*/

/*!
    \fn QString QAbstractCallPolicyManager::registrationIcon() const

    Returns the name of an icon to display next to the
    registrationMessage(), or an empty QString for no icon.

    \sa registrationChanged(), registrationState(), registrationMessage()
*/

/*!
    \fn void QAbstractCallPolicyManager::registrationChanged(QTelephony::RegistrationState state)

    Signal that is emitted when the registration \a state of the telephony
    service being used by this call policy manager changes.

    \sa registrationState(), registrationMessage(), registrationIcon()
*/

/*!
    \fn void QAbstractCallPolicyManager::setCellLocation(const QString &location)

    Sets the current cell location to \a location.
*/

/*!
  Returns a reference to the call policy manager that handles call of \a type; 
  otherwise returns 0.

  \code
    QString t = "Voice";
    QAbstractCallPolicyManager* m = QAbstractCallPolicyManager::managerforCallType( t );
    if (m) {
        Q_ASSERT(m->calltype() == "Voice");
    }
  \endcode
  */
QAbstractCallPolicyManager* QAbstractCallPolicyManager::managerForCallType( const QString &type )
{
    QList<QAbstractCallPolicyManager *> managers = qtopiaTasks<QAbstractCallPolicyManager>();
    for (int i = 0; i < managers.count(); i++) {
        if ( managers.at(i)->callType() == type )
            return managers.at(i);
    }

    return 0;
}
