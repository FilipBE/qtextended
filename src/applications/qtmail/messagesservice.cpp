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

#include "messagesservice.h"
#include "qtopialog.h"

/*!
    \service MessagesService Messages
    \brief Provides the Qt Extended Messages viewing service.

    The \i Messages service enables applications to request the display of messages of various types.
*/

/*! \internal */
MessagesService::MessagesService(QObject* parent)
    : QtopiaAbstractService( "Messages", parent )
{
    publishAll();
}

/*! \internal */
MessagesService::~MessagesService()
{
}

/*!
    \fn MessagesService::view()
    \internal 
*/

/*!
    Show the default selection of messages, equivalent to that displayed when the Messages 
    application is started by a direct user action.

    This slot corresponds to the QCop service message
    \c{Messages::viewMessages()}.
*/
void MessagesService::viewMessages()
{
    qLog(Messaging) << "MessagesService::viewMessages()";

    emit view();
}

/*!
    \fn MessagesService::viewNew(bool)
    \internal 
*/

/*!
    Show the newly arrived messages.  If \a userRequest is true, the request will be treated
    as if arising from a direct user action; otherwise, the user will be requested to confirm 
    the action before proceeding.

    This slot corresponds to the QCop service message
    \c{Messages::viewNewMessages()}.
*/
void MessagesService::viewNewMessages(bool userRequest)
{
    qLog(Messaging) << "MessagesService::viewNewMessages(" << userRequest << ")";

    emit viewNew(userRequest);
}

/*!
    \fn MessagesService::view(const QMailMessageId&)
    \internal 
*/

/*!
    Show the message with the supplied \a id.

    This slot corresponds to the QCop service message
    \c{Messages::viewMessage(QMailMessageId)}.
*/
void MessagesService::viewMessage(QMailMessageId id)
{
    qLog(Messaging) << "MessagesService::viewMessage(" << id << ")";
    if (!id.isValid()) {
        qWarning() << "viewMessage supplied invalid id:" << id;
        return;
    }

    emit view(id);
}

/*!
    \fn MessagesService::replyTo(const QMailMessageId&)
    \internal 
*/

/*!
    Reply to the message with the supplied \a id.

    This slot corresponds to the QCop service message
    \c{Messages::replyToMessage(QMailMessageId)}.
*/
void MessagesService::replyToMessage(QMailMessageId id)
{
    qLog(Messaging) << "MessagesService::replyToMessage(" << id << ")";
    if (!id.isValid()) {
        qWarning() << "replyToMessage supplied invalid id:" << id;
        return;
    }

    emit replyTo(id);
}

/*!
    \fn MessagesService::compose(QMailMessage::MessageType, const QMailAddressList&, const QString&, const QString&, const QContentList&, QMailMessage::AttachmentsAction)
    \internal 
*/

/*!
    Compose a message of type \a type, with the supplied properties. If \a type is
    \c QMailMessage::AnyType, the type will be determined by inspecting the types of 
    the addresses in \a to. The message will be addressed to each of the recipients 
    in \a to, the subject will be preset to \a subject, and the message text will be 
    preset to \a text.

    This slot corresponds to the QCop service message
    \c{Messages::composeMessage(QMailMessage::MessageType, QMailAddressList, QString, QString)}.
*/
void MessagesService::composeMessage(QMailMessage::MessageType type, QMailAddressList to, QString subject, QString text)
{
    qLog(Messaging) << "MessagesService::composeMessage(" << type << ',' << QMailAddress::toStringList(to).join(",") << ", <text> )";

    emit compose(type, to, subject, text, QContentList(), QMailMessage::LinkToAttachments);
}

/*!
    Compose a message of type \a type, with the supplied properties. If \a type is
    \c QMailMessage::AnyType, the type will be determined by inspecting the types of 
    the addresses in \a to, and by the existence of attachments. The message will be 
    addressed to each of the recipients in \a to, the message subject will be set to
    \a subject, and the message text will be preset to \a text.  All the documents 
    listed in \a attachments will be added to the message as attachments. If \a action 
    is \c MessagesService::LinkToAttachments, the attachments will be created as links 
    to the source document; otherwise, the data of the documents will be stored directly 
    in the message parts. If \a action is \c MessagesService::CopyAndDeleteAttachments, 
    the source document will be deleted after the data is copied.

    This slot corresponds to the QCop service message
    \c{Messages::composeMessage(QMailMessage::MessageType, QMailAddressList, QString, QString, QContentList, QMailMessage::AttachmentsAction)}.
*/
void MessagesService::composeMessage(QMailMessage::MessageType type, 
                                     QMailAddressList to, 
                                     QString subject, 
                                     QString text, 
                                     QContentList attachments, 
                                     QMailMessage::AttachmentsAction action)
{
    qLog(Messaging) << "MessagesService::composeMessage(" << type << ", ...)";

    emit compose(type, to, subject, text, attachments, action);
}

/*!
    \fn MessagesService::compose(const QMailMessage&)
    \internal 
*/

/*!
    Compose a message in the appropriate composer, where all composer fields are preset 
    with the data from the matching field of \a message.

    This slot corresponds to the QCop service message
    \c{Messages::composeMessage(QMailMessage)}.
*/
void MessagesService::composeMessage(QMailMessage message)
{
    qLog(Messaging) << "MessagesService::composeMessage(QMailMessage)";

    emit compose(message);
}

