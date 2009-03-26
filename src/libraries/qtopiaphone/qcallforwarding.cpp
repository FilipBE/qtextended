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

#include <qcallforwarding.h>

/*!
    \class QCallForwarding
    \inpublicgroup QtTelephonyModule

    \brief The QCallForwarding class provides access to the call forwarding settings on a GSM phone.

    The functionality in this class is implemented in accordance with
    the Call Forwarding supplementary service described in 3GPP TS 02.82.

    This service permits a called mobile subscriber to have the network send all
    incoming calls, or just those associated with a specific call type, addressed
    to the called mobile subscriber's directory number to another directory number.
    The ability of the served mobile subscriber to originate calls is unaffected.

    The current state of call forwarding can be queried using the
    requestForwardingStatus() function, and changed using the setForwarding()
    function.  The reasons for forwarding (unconditional, mobile busy, no reply, etc)
    are specified by the QCallForwarding::Reason enumeration.

    \sa QCommInterface
    \ingroup telephony
*/

/*!
    \enum QCallForwarding::Reason
    Specifies the reason for a forwarding request.

    \value Unconditional Forward all calls unconditionally.
    \value MobileBusy Forward calls when the mobile is busy on another call.
    \value NoReply Forward calls when no reply within a certain time period.
    \value NotReachable Forward calls when the mobile is not reachable.
    \value All Special value to modify all forwarding rules.  This is typically
           used to turn off all forwarding.
    \value AllConditional Special value to modify all conditional
           forwarding rules.  This is typically used to turn off all
           conditional forwarding.
*/

/*!
    \class QCallForwarding::Status
    \inpublicgroup QtTelephonyModule

    \brief The Status class provides information about a single call forwarding rule.

    The Status class contains information about a single
    call forwarding rule.  The \c cls member is the class of call (or calls)
    that the rule applies to.  The \c number member is the phone number
    that calls will be redirected to,  The \c time member is the amount
    of time to wait for QCallForwarding::NoReply before forwarding the
    call.  The \c time member will be zero if the rule is not for
    QCallForwarding::NoReply, or the timeout is unknown.

*/

/*!
    Construct a new call forwarding object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports call forwarding.  If there is more than one
    service that supports call forwarding, the caller should enumerate
    them with QCommServiceManager::supports() and create separate
    QCallForwarding objects for each.

    \sa QCommServiceManager::supports()
*/
QCallForwarding::QCallForwarding( const QString& service, QObject *parent,
                                  QCommInterface::Mode mode )
    : QCommInterface( "QCallForwarding", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this call forwarding object.
*/
QCallForwarding::~QCallForwarding()
{
}

/*!
    Request the status of the forwarding rules associated with \a reason.
    The service responds by emitting the forwardingStatus() signal.

    \sa forwardingStatus()
*/
void QCallForwarding::requestForwardingStatus( QCallForwarding::Reason reason )
{
    invoke( SLOT(requestForwardingStatus(QCallForwarding::Reason)),
            qVariantFromValue( reason ) );
}

/*!
    Sets the forwarding state associated with \a reason to \a enable,
    with the number information in \a status if forwarding is being
    enabled.  The service responds by emitting the setForwardingResult()
    signal.

    \sa setForwardingResult()
*/
void QCallForwarding::setForwarding
        ( QCallForwarding::Reason reason,
          const QCallForwarding::Status& status, bool enable )
{
    invoke( SLOT(setForwarding(QCallForwarding::Reason,QCallForwarding::Status,bool)),
            qVariantFromValue( reason ),
            qVariantFromValue( status ), enable );
}

/*!
    \fn void QCallForwarding::forwardingStatus( QCallForwarding::Reason reason, const QList<QCallForwarding::Status>& status )

    Signal that is emitted in response to requestForwardingStatus() call
    for \a reason.  The \a status parameter contains a list of forwarding
    rules for the reason code.  The list only contains entries for rules
    where forwarding is enabled for a call class.  If no forwarding is
    not enabled for any call class, the list will be empty.

    \sa requestForwardingStatus()
*/

/*!
    \fn void QCallForwarding::setForwardingResult( QCallForwarding::Reason reason, QTelephony::Result result )

    Signal that is emitted to report the \a result of calling setForwarding()
    for \a reason.

    \sa setForwarding()
*/

/*!
    \internal
    \fn void QCallForwarding::Status::serialize(Stream &stream) const
*/
template <typename Stream>
    void QCallForwarding::Status::serialize(Stream &stream) const
{
    stream << cls;
    stream << number;
    stream << time;
}

/*!
    \internal
    \fn void QCallForwarding::Status::deserialize(Stream &stream)
*/
template <typename Stream>
    void QCallForwarding::Status::deserialize(Stream &stream)
{
    stream >> cls;
    stream >> number;
    stream >> time;
}

Q_IMPLEMENT_USER_METATYPE_ENUM(QCallForwarding::Reason)
Q_IMPLEMENT_USER_METATYPE(QCallForwarding::Status)
Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(QList<QCallForwarding::Status>)
