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

#include <qtopialog.h>
#include <qmailnewmessagehandler.h>

#include <QDataStream>
#include <QDSData>
#include <QMimeType>


/*!
    \class QMailNewMessageHandler
    \inpublicgroup QtMessagingModule

    \preliminary
    \ingroup messaginglibrary

    \brief The QMailNewMessageHandler class provides a convenient interface for responding to
    new message arrival notifications emitted by the message server.

    Clients should instantiate handlers derived from QMailNewMessageHandler for each type of 
    new message notification that they wish to handle.

    When a new message of some type is reported by the message server, each process that has 
    registered to handle the 'New<MessageType>Arrival' service will be raised to handle the
    event.  If the raised process instantiates a handler derived from QMailNewMessageHandler,
    the handler object will process the service request and emit the newCountChanged() signal.

    In response to the newCountChanged() signal, the handler process should invoke the
    setHandled() slot, to communicate whether or not they have handled the event to the message
    server.  If any handler process reports the event as handled, then the count of new messages 
    of the specified type will be reset, and those messages will have the QMailMessage::New status 
    flag unset.

    The process may invoke setHandled() asynchronously, so it is possible to wait for a user
    response before responding to the event.
*/

/*!
    Constructs a new handler for the service \a service, with the specified \a parent.
*/
QMailNewMessageHandler::QMailNewMessageHandler(const QString &service, QObject* parent)
    : QtopiaAbstractService(service, parent),
      pending(false)
{
    publishAll();
}

/*! \internal */
QMailNewMessageHandler::~QMailNewMessageHandler()
{
    if (pending) {
        // Inform the server that we have not been handled
        setHandled(false);
    }
}

/*! \internal */
void QMailNewMessageHandler::arrived(const QDSActionRequest &req)
{
    if (pending) {
        // We haven't handled the previous invocation yet - consider it unhandled
        setHandled(false);
    }

    request = req;

    uint newCount;
    {
        QByteArray data(request.requestData().data());
        QDataStream extractor(data);
        extractor >> newCount;
    }

    pending = true;
    emit newCountChanged(newCount);
}

/*!
    If \a b is true, responds to the message server that the new message arrival event
    has been handled.  If the event is reported as handled, then the count of new messages 
    of the type returned by messageType() will be reset, and those messages will have the 
    QMailMessage::New status flag unset.
*/
void QMailNewMessageHandler::setHandled(bool b)
{
    QByteArray response;
    {
        QDataStream insertor(&response, QIODevice::WriteOnly);
        insertor << b;
    }

    request.respond(QDSData(response, QMimeType()));
    pending = false;
}

/*!
    \fn QMailMessage::MessageType QMailNewMessageHandler::messageType() const

    Returns the type of message handled by this event handler.
*/

/*!
    \fn void QMailNewMessageHandler::newCountChanged(uint newCount)

    Emitted when the arrival of new messages of the type returned by messageType()
    causes the count of new messages to increase.  The increased count of 
    new messages is \a newCount.
*/


/*!
    \class QMailNewSmsHandler
    \inpublicgroup QtMessagingModule

    \preliminary
    \ingroup messaginglibrary

    \brief The QMailNewSmsHandler class provides a convenient interface for responding to
    new SMS arrival notifications emitted by the message server.

    When a new SMS is retrieved by the message server, the \c{NewSmsArrival::arrived(QDSActionRequest)} 
    service request is emitted.  If a process registers to handle this request and instantiates 
    a QMailNewSmsHandler object, the handler object will process the service request and emit 
    the newCountChanged() signal.

    In response to the newCountChanged() signal, the handler process should invoke the
    setHandled() slot, to communicate whether or not they have handled the event to the message
    server.  If any handler process reports the event as handled, then the count of new SMS
    messages will be reset, and those messages will have the QMailMessage::New status flag unset.

    Note: if a process instantiates a QMailNewSmsHandler, it will not do anything unless the 
    process is also registered to handle the \c NewSmsArrival service.
*/

/*!
    Constructs a new handler for the \c NewSmsArrival service, with the specified \a parent.
*/
QMailNewSmsHandler::QMailNewSmsHandler(QObject* parent)
    : QMailNewMessageHandler("NewSmsArrival", parent)
{
}

/*! 
    Returns QMailMessage::Sms, the type of messages handled by QMailNewSmsHandler.
*/
QMailMessage::MessageType QMailNewSmsHandler::messageType() const
{
    return QMailMessage::Sms;
}


/*!
    \class QMailNewMmsHandler
    \inpublicgroup QtMessagingModule

    \preliminary
    \ingroup messaginglibrary

    \brief The QMailNewMmsHandler class provides a convenient interface for responding to
    new MMS arrival notifications emitted by the message server.

    When a new MMS is retrieved by the message server, the \c{NewMmsArrival::arrived(QDSActionRequest)} 
    service request is emitted.  If a process registers to handle this request and instantiates 
    a QMailNewMmsHandler object, the handler object will process the service request and emit 
    the newCountChanged() signal.

    In response to the newCountChanged() signal, the handler process should invoke the
    setHandled() slot, to communicate whether or not they have handled the event to the message
    server.  If any handler process reports the event as handled, then the count of new MMS
    messages will be reset, and those messages will have the QMailMessage::New status flag unset.

    Note: if a process instantiates a QMailNewMmsHandler, it will not do anything unless the 
    process is also registered to handle the \c NewMmsArrival service.
*/

/*!
    Constructs a new handler for the \c NewMmsArrival service, with the specified \a parent.
*/
QMailNewMmsHandler::QMailNewMmsHandler(QObject* parent)
    : QMailNewMessageHandler("NewMmsArrival", parent)
{
}

/*! 
    Returns QMailMessage::Mms, the type of messages handled by QMailNewMmsHandler.
*/
QMailMessage::MessageType QMailNewMmsHandler::messageType() const
{
    return QMailMessage::Mms;
}


/*!
    \class QMailNewEmailHandler
    \inpublicgroup QtMessagingModule

    \preliminary
    \ingroup messaginglibrary

    \brief The QMailNewEmailHandler class provides a convenient interface for responding to
    new email arrival notifications emitted by the message server.

    When a new email is retrieved by the message server, the \c{NewEmailArrival::arrived(QDSActionRequest)} 
    service request is emitted.  If a process registers to handle this request and instantiates 
    a QMailNewEmailHandler object, the handler object will process the service request and emit 
    the newCountChanged() signal.

    In response to the newCountChanged() signal, the handler process should invoke the
    setHandled() slot, to communicate whether or not they have handled the event to the message
    server.  If any handler process reports the event as handled, then the count of new email
    messages will be reset, and those messages will have the QMailMessage::New status flag unset.

    Note: if a process instantiates a QMailNewEmailHandler, it will not do anything unless the 
    process is also registered to handle the \c NewEmailArrival service.
*/

/*!
    Constructs a new handler for the \c NewEmailArrival service, with the specified \a parent.
*/
QMailNewEmailHandler::QMailNewEmailHandler(QObject* parent)
    : QMailNewMessageHandler("NewEmailArrival", parent)
{
}

/*! 
    Returns QMailMessage::Email, the type of messages handled by QMailNewEmailHandler.
*/
QMailMessage::MessageType QMailNewEmailHandler::messageType() const
{
    return QMailMessage::Email;
}


/*!
    \class QMailNewInstantMessageHandler
    \inpublicgroup QtMessagingModule

    \preliminary
    \ingroup messaginglibrary

    \brief The QMailNewInstantMessageHandler class provides a convenient interface for responding to
    new instant message arrival notifications emitted by the message server.

    When a new instant message is retrieved by the message server, the \c{NewInstantMessageArrival::arrived(QDSActionRequest)}
    service request is emitted.  If a process registers to handle this request and instantiates 
    a QMailNewInstantMessageHandler object, the handler object will process the service request 
    and emit the newCountChanged() signal.

    In response to the newCountChanged() signal, the handler process should invoke the
    setHandled() slot, to communicate whether or not they have handled the event to the message
    server.  If any handler process reports the event as handled, then the count of new instant message
    messages will be reset, and those messages will have the QMailMessage::New status flag unset.

    Note: if a process instantiates a QMailNewInstantMessageHandler, it will not do anything unless the 
    process is also registered to handle the \c NewInstantMessageArrival service.
*/

/*!
    Constructs a new handler for the \c NewInstantMessageArrival service, with the specified \a parent.
*/
QMailNewInstantMessageHandler::QMailNewInstantMessageHandler(QObject* parent)
    : QMailNewMessageHandler("NewInstantMessageArrival", parent)
{
}

/*! 
    Returns QMailMessage::Instant, the type of messages handled by QMailNewInstantMessageHandler.
*/
QMailMessage::MessageType QMailNewInstantMessageHandler::messageType() const
{
    return QMailMessage::Instant;
}


/*!
    \class QMailNewSystemMessageHandler
    \inpublicgroup QtMessagingModule

    \preliminary
    \ingroup messaginglibrary

    \brief The QMailNewSystemMessageHandler class provides a convenient interface for responding to
    new system message arrival notifications emitted by the message server.

    When a new system message is retrieved by the message server, the \c{NewSystemMessageArrival::arrived(QDSActionRequest)} 
    service request is emitted.  If a process registers to handle this request and instantiates 
    a QMailNewSystemMessageHandler object, the handler object will process the service request 
    and emit the newCountChanged() signal.

    In response to the newCountChanged() signal, the handler process should invoke the
    setHandled() slot, to communicate whether or not they have handled the event to the message
    server.  If any handler process reports the event as handled, then the count of new system message
    messages will be reset, and those messages will have the QMailMessage::New status flag unset.

    Note: if a process instantiates a QMailNewSystemMessageHandler, it will not do anything unless the 
    process is also registered to handle the \c NewSystemMessageArrival service.
*/

/*!
    Constructs a new handler for the \c NewSystemMessageArrival service, with the specified \a parent.
*/
QMailNewSystemMessageHandler::QMailNewSystemMessageHandler(QObject* parent)
    : QMailNewMessageHandler("NewSystemMessageArrival", parent)
{
}

/*! 
    Returns QMailMessage::System, the type of messages handled by QMailNewSystemMessageHandler.
*/
QMailMessage::MessageType QMailNewSystemMessageHandler::messageType() const
{
    return QMailMessage::System;
}

