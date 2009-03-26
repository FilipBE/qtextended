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

#include "messagearrivalservice.h"
#include "qtopialog.h"

#include <QDSActionRequest>

#ifndef QTOPIA_NO_SMS

/*!
    \service MessageArrivalService MessageArrival
    \inpublicgroup QtMessagingModule
    \brief The MessageArrivalService class provides the service handling the arrival of new messages.

    The \i MessageArrival service enables QPE to delegate the handling of new messages as they are received.
*/

/*! \internal */
MessageArrivalService::MessageArrivalService(QObject* parent)
    : QtopiaAbstractService("MessageArrival", parent)
{
    publishAll();
}

/*!  \internal */
MessageArrivalService::~MessageArrivalService()
{
}

/*!
    \fn MessageArrivalService::mmsMessage(const QDSActionRequest&)
    \internal 
*/

/*!
    Direct the \i MessageArrival service to handle the MMS push notification \a request.

    This slot corresponds to a QDS service with request data containing
    the serialization of an \c MMSMessage, and no response data.

    This slot corresponds to the QCop service message
    \c{MessageArrival::pushMmsMessage(QDSActionRequest)}.
*/
void MessageArrivalService::pushMmsMessage( const QDSActionRequest& request )
{
    qLog(Messaging) << "MessageArrivalService::pushMmsMessage( QDSActionRequest )";

#ifndef QTOPIA_NO_MMS
    emit mmsMessage(request);
#endif
}

/*!
    \fn MessageArrivalService::smsFlash(const QDSActionRequest&)
    \internal 
*/

/*!
    Direct the \i MessageArrival service to process the flash SMS message
    within \a request.

    This slot corresponds to a QDS service with request data containing
    the serialization of a QSMSMessage, and no response data.

    This slot corresponds to the QCop service message
    \c{MessageArrival::flashSms(QDSActionRequest)}.
*/
void MessageArrivalService::flashSms( const QDSActionRequest& request )
{
    qLog(Messaging) << "MessageArrivalService::flashSms( QDSActionRequest )";

    emit smsFlash( request );
}

#endif // QTOPIA_NO_SMS

