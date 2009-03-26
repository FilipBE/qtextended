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

#include <qsupplementaryservices.h>

/*!
    \class QSupplementaryServices
    \inpublicgroup QtTelephonyModule

    \brief The QSupplementaryServices class provides access to structured and unstructured supplementary services within GSM networks.
    \ingroup telephony

    Structured supplementary service data can be sent to the network with
    sendSupplementaryServiceData() and unstructured supplementary service
    data can be sent with sendUnstructuredData().

    As structured and unstructured supplementary service data is received
    from the network, the outgoingNotification(), incomingNotification(),
    and unstructuredNotification() signals will be emitted.

    If there is an ongoing unstructured supplementary service session in
    progress, it can be canceled with cancelUnstructuredSession().

    \sa QCommInterface
*/

/*!
    Construct a new supplementary services object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports supplementary services.  If there is more
    than one service that supports supplementary services, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QSupplementaryServices objects for each.

    \sa QCommServiceManager::supports()
*/
QSupplementaryServices::QSupplementaryServices
        ( const QString& service, QObject *parent, QCommInterface::Mode mode )
    : QCommInterface( "QSupplementaryServices", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this supplementary services object.
*/
QSupplementaryServices::~QSupplementaryServices()
{
}

/*!
    \enum QSupplementaryServices::OutgoingNotification
    This enum defines notification types for outgoing mobile-originated (MO) calls according to 3GPP TS 27.007.

    \value MO_UnconditionalForwardingActive Unconditional call forwarding is active.
    \value MO_ConditionalForwardingActive Some of the conditional call forwardings are active.
    \value MO_Forwarded Call has been forwarded.
    \value MO_Waiting Call is waiting.
    \value MO_ClosedUserGroup Closed user group call.
    \value MO_OutgoingCallsBarred Outgoing calls are barred.
    \value MO_IncomingCallsBarred Incoming calls are barred.
    \value MO_CallerIdSuppressionRejected Caller-Id suppression request has been rejected.
    \value MO_Deflected Call has been deflected.
*/

/*!
    \enum QSupplementaryServices::IncomingNotification
    This enum defines notification types for incoming mobile-terminated (MT) calls according to 3GPP TS 27.007.

    \value MT_Forwarded Call has been forwarded.
    \value MT_ClosedUserGroup Closed user group call.
    \value MT_Hold Call has been put on hold during a voice call.
    \value MT_Retrieved Call has been retrieved during a voice call.
    \value MT_MultipartyEntered Multiparty call entered during a voice call.
    \value MT_HoldReleased Call on hold has been released during a voice call.
    \value MT_ForwardCheck Forward check message received.
    \value MT_Alerting Call is being connected (alerting) with the remote
           party in alerting state in explicit call transfer operation
           (during a voice call).
    \value MT_ExplicitTransfer Call has been connected with the other remote
           party in explicit call transfer operation (during a voice call
           or MT call setup).
    \value MT_Deflected Call has been deflected.
    \value MT_AdditionalIncomingForwarded Additional incoming call forwarded.
*/

/*!
    \enum QSupplementaryServices::UnstructuredAction
    This enum defines the action that should be taken for an incoming unstructured supplementary service notification according to 3GPP TS 27.007.

    \value NoFurtherActionRequired No further user action required.
    \value FurtherActionRequired Further user action required.
    \value TerminatedByNetwork Terminated by network.
    \value OtherLocalClientResponded Other local client has responded.
    \value OperationNotSupported Operation not supported.
    \value NetworkTimeout Network time out.
*/

/*!
    Cancel the current USSD session.  The unstructuredResult() signal will be
    emitted to indicate the result of canceling the current USSD session.

    \sa sendUnstructuredData(), unstructuredResult()
*/
void QSupplementaryServices::cancelUnstructuredSession()
{
    invoke( SLOT(cancelUnstructuredSession()) );
}

/*!
    Sends \a data within the current USSD session.  On GSM modems,
    the \c{AT+CUSD} command is used for this purpose.  The unstructuredResult()
    signal will be emitted to indicate the result of sending \a data.

    \sa cancelUnstructuredSession(), sendSupplementaryServiceData()
    \sa unstructuredResult()
*/
void QSupplementaryServices::sendUnstructuredData( const QString& data )
{
    invoke( SLOT(sendUnstructuredData(QString)), data );
}

/*!
    Sends \a data as a supplementary service command.  On GSM modems,
    the \c{ATD} command is used for this purpose.  The \a data should
    start with \c{*} or \c{#}.  The sendSupplementaryServiceDataResult()
    signal will be emitted to indicate the result of sending \a data.

    \sa supplementaryServiceResult(), sendUnstructuredData()
*/
void QSupplementaryServices::sendSupplementaryServiceData( const QString& data )
{
    invoke( SLOT(sendSupplementaryServiceData(QString)), data );
}

/*!
    \fn void QSupplementaryServices::outgoingNotification( QSupplementaryServices::OutgoingNotification type, int groupIndex )

    Signal that is emitted when a supplementary service notification \a type
    arrives for an outgoing mobile-originated (MO) call.  If \a type is
    \c MO_ClosedUserGroup, then \a groupIndex will be the index of the closed
    user group.  Otherwise \a groupIndex will be zero.
*/

/*!
    \fn void QSupplementaryServices::incomingNotification( QSupplementaryServices::IncomingNotification type, int groupIndex, const QString& number )

    Signal that is emitted when a supplementary service notification \a type
    arrives for an incoming mobile-terminated (MT) call.  If \a type is
    \c MT_ClosedUserGroup, then \a groupIndex will be the index of the closed
    user group.  Otherwise \a groupIndex will be zero.  If the call was
    transferred (\a type equal to \c MT_ExplicitTransfer), \a number will
    be present.
*/

/*!
    \fn void QSupplementaryServices::unstructuredNotification( QSupplementaryServices::UnstructuredAction action, const QString& data )

    Signal that is emitted when an unstructured supplementary service
    notification arrives.  The \a action parameter specifies the action
    to be taken next in the current session, and \a data specifies the
    data provided by the network.

    \sa cancelUnstructuredSession(), sendUnstructuredData()
*/

/*!
    \fn void QSupplementaryServices::unstructuredResult( QTelephony::Result result )

    Signal that is emitted to report the \a result of a call to
    cancelUnstructuredSession() or sendUnstructuredData().

    \sa cancelUnstructuredSession(), sendUnstructuredData()
*/

/*!
    \fn void QSupplementaryServices::supplementaryServiceResult( QTelephony::Result result )

    Signal that is emitted to report the \a result of a call to
    sendSupplementaryServiceData().

    \sa sendSupplementaryServiceData()
*/

Q_IMPLEMENT_USER_METATYPE_ENUM(QSupplementaryServices::OutgoingNotification)
Q_IMPLEMENT_USER_METATYPE_ENUM(QSupplementaryServices::IncomingNotification)
Q_IMPLEMENT_USER_METATYPE_ENUM(QSupplementaryServices::UnstructuredAction)
