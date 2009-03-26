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

#include <qcallbarring.h>

/*!
    \class QCallBarring
    \inpublicgroup QtTelephonyModule

    \brief The QCallBarring class provides access to the call barring settings on a GSM phone.

    The functionality in this class is implemented in accordance with the
    Call Barring supplementary service described in 3GPP TS 02.88.

    This service makes it possible for a mobile subscriber to have barring of
    certain categories of outgoing calls according to a barring type which
    is selected from a set of one or more barring types chosen at provision time
    and is valid for all outgoing calls, or just those associated with a specific
    call class (voice, data, etc).

    The current state of the call barring service is retrieved using the
    requestBarringStatus() function.  The state of the call barring service
    is changed using the setBarringStatus() function.  An entire group of
    barred calls can be unlocked with unlockAll(), unlockAllIncoming(),
    and unlockAllOutgoing().

    \sa QCommInterface
    \ingroup telephony
*/

/*!
    \enum QCallBarring::BarringType
    The type of GSM barring information to be requested or modified.

    \value OutgoingAll All outgoing calls.  3GPP TS 27.007 type \c{AO}.
    \value OutgoingInternational All outgoing international calls.  3GPP TS 27.007 type \c{OI}.
    \value OutgoingInternationalExceptHome All outgoing international calls except to home country.  3GPP TS 27.007 type \c{OX}.
    \value IncomingAll All incoming calls.  3GPP TS 27.007 type \c{AI}.
    \value IncomingWhenRoaming All incoming calls when roaming.  3GPP TS 27.007 type \c{IR}.
    \value IncomingNonTA All incoming calls not stored to TA memory.  3GPP TS 27.007 type \c{NT}.
    \value IncomingNonMT All incoming calls not stored to MT memory.  3GPP TS 27.007 type \c{NM}.
    \value IncomingNonSIM All incoming calls not stored to SIM/UICC memory.  3GPP TS 27.007 type \c{NS}.
    \value IncomingNonMemory All incoming calls not stored in any memory.  3GPP TS 27.007 type \c{NA}.
    \value AllBarringServices All barring services.  3GPP TS 27.007 type \c{AB}.  This should only be used with QCallBarring::changeBarringPassword().
    \value AllOutgoingBarringServices All outgoing barring services.  3GPP TS 27.007 type \c{AG}.  This should only be used with QCallBarring::changeBarringPassword().
    \value AllIncomingBarringServices All incoming barring services.  3GPP TS 27.007 type \c{AC}.  This should only be used with QCallBarring::changeBarringPassword().
*/

/*!
    Construct a new call barring object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports call barring.  If there is more than one
    service that supports call barring, the caller should enumerate
    them with QCommServiceManager::supports() and create separate
    QCallBarring objects for each.

    \sa QCommServiceManager::supports()
*/
QCallBarring::QCallBarring( const QString& service, QObject *parent,
                            QCommInterface::Mode mode )
    : QCommInterface( "QCallBarring", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this call barring object.
*/
QCallBarring::~QCallBarring()
{
}

/*!
    Request the current call barring status of \a type.  The service
    responds with the barringStatus() signal.

    \sa barringStatus(), setBarringStatus()
*/
void QCallBarring::requestBarringStatus( QCallBarring::BarringType type )
{
    invoke( SLOT(requestBarringStatus(QCallBarring::BarringType)),
            qVariantFromValue( type ) );
}

/*!
    Sets the call barring status of \a type.  The \a cls parameter will
    contain flags for all of the call types that should be either locked
    or unlocked according to the \a lock flag.  The \a password parameter
    should be the call barring password.  The service responds with the
    setBarringStatusResult() signal.

    \sa setBarringStatusResult(), requestBarringStatus()
    \sa changeBarringPassword()
*/
void QCallBarring::setBarringStatus( QCallBarring::BarringType type,
                                     const QString& password,
                                     QTelephony::CallClass cls,
                                     bool lock )
{
    invoke( SLOT(setBarringStatus(QCallBarring::BarringType,QString,QTelephony::CallClass,bool)) )
        << qVariantFromValue( type )
        << password
        << qVariantFromValue( cls )
        << lock;
}

/*!
    Unlock all barring services with the supplied call barring \a password.
    The service responds with the unlockResult() signal. On AT-based modems,
    this uses 3GPP TS 27.007 call barring type \c{AB}.

    \sa unlockResult(), unlockAllIncoming(), unlockAllOutgoing()
    \sa changeBarringPassword()
*/
void QCallBarring::unlockAll( const QString& password )
{
    invoke( SLOT(unlockAll(QString)), password );
}

/*!
    Unlock all incoming barring services with the supplied call barring
    \a password.  The service responds with the unlockResult() signal.
    On AT-based modems, this uses 3GPP TS 27.007 call barring type \c{AB}.

    \sa unlockResult(), unlockAll(), unlockAllOutgoing()
    \sa changeBarringPassword()
*/
void QCallBarring::unlockAllIncoming( const QString& password )
{
    invoke( SLOT(unlockAllIncoming(QString)), password );
}

/*!
    Unlock all outgoing barring services with the supplied call barring
    \a password.  The service responds with the unlockResult() signal.
    On AT-based modems, this uses 3GPP TS 27.007 call barring type \c{AB}.

    \sa unlockResult(), unlockAll(), unlockAllIncoming()
    \sa changeBarringPassword()
*/
void QCallBarring::unlockAllOutgoing( const QString& password )
{
    invoke( SLOT(unlockAllOutgoing(QString)), password );
}

/*!
    Change the password for the call barring service \a type from
    \a oldPassword to \a newPassword.  The result is reported
    via the changeBarringPasswordResult() signal.

    \sa changeBarringPasswordResult()
*/
void QCallBarring::changeBarringPassword( QCallBarring::BarringType type,
                                          const QString& oldPassword,
                                          const QString& newPassword )
{
    invoke( SLOT(changeBarringPassword(QCallBarring::BarringType,QString,QString)),
            qVariantFromValue( type ), oldPassword, newPassword );
}

/*!
    \fn void QCallBarring::barringStatus( QCallBarring::BarringType type, QTelephony::CallClass cls )

    Signal that is emitted in response to a requestBarring() call for \a type.
    The \a cls parameter contains a set of flags indicating which call types
    have been barred.

    \sa requestBarringStatus()
*/

/*!
    \fn void QCallBarring::setBarringStatusResult( QTelephony::Result result )

    Signal that is emitted in response to a setBarringStatus() call.
    If \a result is QTelephony::OK, then the request was successful.
    Otherwise the request failed (usually because the call barring password
    was incorrect).

    \sa setBarringStatus()
*/

/*!
    \fn void QCallBarring::unlockResult( QTelephony::Result result )

    Signal that is emitted in response to an unlockAll(), unlockAllIncoming(),
    or unlockAllOutgoing() call.  If \a result is QTelephony::OK, then the
    request was successful.  Otherwise the request failed (usually because
    the call barring password was incorrect).

    \sa unlockAll(), unlockAllIncoming(), unlockAllOutgoing()
*/

/*!
    \fn void QCallBarring::changeBarringPasswordResult( QTelephony::Result result )

    Signal that is emitted to report the \a result of calling
    changeBarringPassword().

    \sa changeBarringPassword()
*/

Q_IMPLEMENT_USER_METATYPE_ENUM(QCallBarring::BarringType)
