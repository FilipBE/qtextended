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

#include "emailservice.h"
#include "qtopialog.h"

#include <QMailMessageId>
#include <QString>
#include <QStringList>

/*!
    \service EmailService Email
    \brief Provides the Qt Extended Email service.

    The \i Email service enables applications to access features of
    the system's e-mail application.
*/

/*! \internal */
EmailService::EmailService(QObject* parent)
    : QtopiaAbstractService("Email", parent)
{
    publishAll();
}

/*!
    \internal
*/
EmailService::~EmailService()
{
}

/*!
    \fn EmailService::write(const QString&, const QString&, const QStringList&, const QStringList&)
    \internal 
*/

/*!
    \fn EmailService::write(const QString&, const QString&)
    \internal 
*/

/*!
    Direct the \i Email service to interact with the user to compose a new
    e-mail message, and then, if confirmed by the user, send the message.

    This slot corresponds to the QCop service message
    \c{Email::writeMail()}.
*/
void EmailService::writeMail()
{
    qLog(Messaging) << "EmailService::writeMail()";

    emit write(QString(), QString());
}

/*!
    Direct the \i Email service to interact with the user to compose a new
    e-mail message, and then, if confirmed by the user, send the message.
    The message is sent to \a name at \a address.

    This slot corresponds to the QCop service message
    \c{Email::writeMail(QString,QString)}.
*/
void EmailService::writeMail( const QString& name, const QString& address )
{
    qLog(Messaging) << "EmailService::writeMail(" << name << "," << address << ")";

    emit write(name, address);
}

/*!
    \deprecated

    Clients of this service should instead use 
    \c{Messages::composeMessage(QMailMessage::MessageType, QMailAddressList, QString, QString, QContentList, QMailMessage::AttachmentsAction)}.

    Direct the \i Email service to interact with the user to compose a new
    e-mail message, and then, if confirmed by the user, send the message.
    The message is sent to \a name at \a email.  The initial body of
    the message will be based on \a docAttachments and \a fileAttachments.
    The resulting message will contains links to the files passed as
    attachments, unless the attached files are under the path returned by
    \c Qtopia::tempDir(). In this case, the data of the files is copied
    into the resulting message. Linked attachment files must remain accessible
    to qtmail until the message is transmitted.

    This message will choose the best message transport for the message,
    which may be e-mail, SMS, MMS, etc.  This is unlike writeMail(),
    which will always use e-mail.

    This slot corresponds to the QCop service message
    \c{Email::writeMessage(QString,QString,QStringList,QStringList)}.
*/
void EmailService::writeMessage( const QString& name, const QString& email,
                                 const QStringList& docAttachments,
                                 const QStringList& fileAttachments )
{
    qLog(Messaging) << "EmailService::writeMessage(" << name << "," << email << ", ... )";

    emit write(name, email, docAttachments, fileAttachments);
}

/*!
    \fn EmailService::viewInbox()
    \internal 
*/

/*!
    Direct the \i Email service to display the user's message boxes.

    This slot corresponds to the QCop service message
    \c{Email::viewMail()}.
*/
void EmailService::viewMail()
{
    qLog(Messaging) << "EmailService::viewMail()";

    emit viewInbox();
}

/*!
    \fn EmailService::message(const QMailMessageId&)
    \internal 
*/

/*!
    \deprecated

    Direct the \i Email service to display the message identified by
    \a id.

    This slot corresponds to the QCop service message
    \c{Email::viewMail(QMailMessageId)}.
*/
void EmailService::viewMail( const QMailMessageId& id )
{
    qLog(Messaging) << "EmailService::viewMail(" << id << ")";

    emit message(id);
}

/*!
    Direct the \i Email service to interact with the user to compose a new
    e-mail message for sending the vcard data in \a filename.  The
    \a description argument provides an optional descriptive text message.

    This slot corresponds to the QCop service message
    \c{Email::emailVCard(QString,QString)}.
*/
void EmailService::emailVCard( const QString& filename, const QString& description)
{
    qLog(Messaging) << "EmailService::emailVCard(" << filename << "," << description << ")";

    emit vcard(filename, description);
}

/*!
    \fn EmailService::vcard(const QString&, const QString&);
    \internal 
*/

/*!
    Direct the \i Email service to interact with the user to compose a new
    e-mail message for sending the vcard data in \a request.

    This slot corresponds to a QDS service with a request data type of
    "text/x-vcard" and no response data.

    This slot corresponds to the QCop service message
    \c{Email::emailVCard(QDSActionRequest)}.
*/
void EmailService::emailVCard( const QDSActionRequest& request )
{
    qLog(Messaging) << "EmailService::emailVCard( QDSActionRequest )";

    emit vcard(request);
}

/*!
    \fn EmailService::vcard(const QDSActionRequest&);
    \internal 
*/

/*! \internal */
void EmailService::emailVCard( const QString&, const QMailMessageId&, const QString& filename, const QString& description )
{
    qLog(Messaging) << "EmailService::emailVCard( , ," << filename << "," << description << ")";

    emit vcard(filename, description);
}

/*!
    \fn EmailService::cleanup(const QDate&, int)
    \internal 
*/

/*!
    \deprecated

    Direct the \i Email service to purge all messages which
    are older than the given \a date and exceed the minimal mail \a size.
    This is typically called by the cleanup wizard.

    This slot corresponds to the QCop service message
    \c{Email::cleanupMessages(QDate,int)}.
*/
void EmailService::cleanupMessages( const QDate& date, int size )
{
    qLog(Messaging) << "EmailService::cleanupMessages(" << date << "," << size << ")";

    emit cleanup(date, size);
}

