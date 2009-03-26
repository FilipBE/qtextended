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

#include "smsservice.h"
#include "qtopialog.h"

#include <QDSActionRequest>

#ifndef QTOPIA_NO_SMS

/*!
    \service SMSService SMS
    \brief Provides the Qt Extended SMS service.

    The \i SMS service enables applications to access features of
    the system's SMS application.
*/

/*! \internal */
SMSService::SMSService(QObject* parent)
    : QtopiaAbstractService("SMS", parent)
{
    publishAll();
}

/*!  \internal */
SMSService::~SMSService()
{
}

/*!
    \fn SMSService::write(const QString&, const QString&, const QString&);
    \internal 
*/

/*!
    Direct the \i SMS service to interact with the user to compose a new
    SMS message, and then, if confirmed by the user, send the message.

    This slot corresponds to the QCop service message
    \c{SMS::writeSms()}.
*/
void SMSService::writeSms()
{
    qLog(Messaging) << "SMSService::writeSms()";

    emit write(QString(), QString(), QString());
}

/*!
    Direct the \i SMS service to interact with the user to compose a new
    SMS message, and then, if confirmed by the user, send the message.
    The message is sent to \a name at \a number.

    This slot corresponds to the QCop service message
    \c{SMS::writeSms(QString,QString)}.
*/
void SMSService::writeSms( const QString& name, const QString& number )
{
    qLog(Messaging) << "SMSService::writeSms(" << name << "," << number << ")";

    emit write(name, number, QString());
}

/*!
    Direct the \i SMS service to interact with the user to compose a new
    SMS message, and then, if confirmed by the user, send the message.
    The message is sent to \a name at \a number.  The initial body of
    the message will be read from \a filename.  After the file is
    read, it will be removed.

    This slot corresponds to the QCop service message
    \c{SMS::writeSms(QString,QString,QString)}.
*/
void SMSService::writeSms( const QString& name, const QString& number,
                           const QString& filename )
{
    qLog(Messaging) << "SMSService::writeSms(" << name << "," << number << "," << filename << ")";

    emit write(name, number, filename);
}

/*!
    \fn SMSService::newMessages(bool)
    \internal 
*/

/*!
    Show the most recently received SMS message.
    \deprecated

    Clients of this service should instead use \c{Messages::viewNewMessages()}.

    This slot corresponds to the QCop service message
    \c{SMS::viewSms()}.
*/
void SMSService::viewSms()
{
    qLog(Messaging) << "SMSService::viewSms()";

    // Although this requests SMS specifically, we currently have only the 
    // facility to show the newest incoming message, or the combined inbox
    emit newMessages(false);
}

/*!
    \fn SMSService::viewInbox()
    \internal 
*/

/*!
    Show the list of all received SMS messages.
    \deprecated

    This slot corresponds to the QCop service message
    \c{SMS::viewSmsList()}.
*/
void SMSService::viewSmsList()
{
    qLog(Messaging) << "SMSService::viewSmsList";

    // Although this requests SMS specifically, we currently have only the 
    // facility to show the combined inbox
    emit viewInbox();
}

/*!
    \fn SMSService::vcard(const QString&, const QString&);
    \internal 
*/

/*!
    Direct the \i SMS service to interact with the user to compose a new
    SMS message for sending the vcard data in \a filename.  The
    \a description argument provides an optional descriptive text message.

    This slot corresponds to the QCop service message
    \c{SMS::smsVCard(QString,QString)}.
*/
void SMSService::smsVCard( const QString& filename, const QString& description)
{
    qLog(Messaging) << "SMSService::smsVCard(" << filename << "," << description << ")";

    emit vcard(filename, description);
}

/*!
    \fn SMSService::vcard(const QDSActionRequest&);
    \internal 
*/

/*!
    Direct the \i SMS service to interact with the user to compose a new
    SMS message for sending the vcard data in \a request.

    This slot corresponds to a QDS service with a request data type of
    "text/x-vcard" and no response data.

    This slot corresponds to the QCop service message
    \c{SMS::smsVCard(QDSActionRequest)}.

*/
void SMSService::smsVCard( const QDSActionRequest& request )
{
    qLog(Messaging) << "SMSService::smsVCard( QDSActionRequest )";

    emit vcard(request);
}

/*!
    Direct the \i SMS service to interact with the user to compose a new
    SMS message for sending the vcard data in \a filename.  The
    \a description argument provides an optional descriptive text message.
    The \a channel and \a id are not used.

    \deprecated

    This slot corresponds to the QCop service message
    \c{SMS::smsVCard(QString,QString,QString,QString)}.
*/
void SMSService::smsVCard( const QString& channel, const QString& id, const QString& filename, const QString& description )
{
    qLog(Messaging) << "SMSService::smsVCard( , ," << filename << "," << description << ")";

    emit vcard(filename, description);

    Q_UNUSED(channel)
    Q_UNUSED(id)
}

#endif // QTOPIA_NO_SMS

