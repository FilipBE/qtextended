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

#include "qcollectivemessenger.h"
#include "qcollectivesimplemessage.h"

#include <qtopialog.h>

/*!
    \class QCollectiveMessenger
    \inpublicgroup QtBaseModule

    \brief The QCollectiveMessenger class defines a generic interface to P2P instant messaging.

    The QCollectiveMessenger class inherits QAbstractIpcInterface which provides
    client/server communications framework.

    The QCollectiveMessenger in Server mode parses the received instant
    messages from the network and informs the rest of the system.  It is also
    responsible for sending instant messages to the remote peers.

    For example, to implement the server mode provider that provides
    presence interface in "voip" service, create a class that inherits
    QCollectiveMessenger class and override public slots.

    \code
    class IMProvider : public QCollectiveMessenger
    {
        Q_OBJECT
    public:
        IMProvider( QObject *parent=0 )
            : QCollectiveMessenger( "voip", parent, Server )
        {
            ...
        }
    public slots:
        virtual void sendMessage(const QCollectiveSimpleMessage &msg)
        {
            ...
        }
        ...
    };
    \endcode

    The QCollectiveMessenger in Client mode is used by client applications to
    access the incoming and outgoing instant message data.

    \code
    QCollectiveMessenger *provider = new QCollectiveMessenger( "voip" );
    provider->registerIncomingHandler("MyIncomingMessageService");
    \endcode

    \ingroup collective

    \sa QCollectiveSimpleMessage
*/

class QCollectiveMessengerPrivate
{
};

/*!
    Constructs a QCollectiveMessenger object for \a service and attaches it to \a parent.
    The object will be created in client mode if \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports the QCollectiveMessenger interface.
    If there is more than one service that supports this interface,
    the caller should enumerate them with QCommServiceManager::supports()
    and create separate QCollectiveMessenger objects for each.

    \sa QCommServiceManager::supports()
*/
QCollectiveMessenger::QCollectiveMessenger(const QString &service, QObject *parent,
                                         QAbstractIpcInterface::Mode mode)
                : QAbstractIpcInterface( "/Communications", "QCollectiveMessenger",
                                 service, parent, mode )
{
    proxyAll(staticMetaObject);
}

/*!
    Destroys the QCollectiveMessenger object.
*/
QCollectiveMessenger::~QCollectiveMessenger()
{
}

/*!
    \fn void QCollectiveMessenger::messageSent(const QCollectiveSimpleMessage &message)

    This signal is emitted whenever a \a message has been successfully
    sent.

    \sa sendMessage()
*/

/*!
    \fn void QCollectiveMessenger::messageFailed(const QCollectiveSimpleMessage &message)

    This signal is emitted whenever \a message could not be delivered
    successfully.

    \sa sendMessage()
*/

/*!
    Attempts to send the \a message to the remote peer specified
    in the message's to field.  The messageSent() signal will
    be emitted once the message has been sent, and the messageError()
    signal will be emitted if the message could not be delivered.
*/
void QCollectiveMessenger::sendMessage(const QCollectiveSimpleMessage &message)
{
    invoke(SLOT(sendMessage(QCollectiveSimpleMessage)), QVariant::fromValue(message));
}

/*!
    Registers an incoming message handler \a service.

    The service must implement a service method with the following
    signature:
    \code
        void incomingMessages(const QList<QCollectiveSimpleMessage> &);
    \endcode

    If multiple handlers are registered, then all registered handlers
    will be called with the incoming text messages.

    If no handlers are registered, then the messages will queue
    up to a certain implementation defined limit.  If that limit is
    exceeded, then further messages can be lost.
*/
void QCollectiveMessenger::registerIncomingHandler(const QString &service)
{
    invoke(SLOT(registerIncomingHandler(QString)), QVariant::fromValue(service));
}

/*!
    Unregisters an incoming message handler \a service.
*/
void QCollectiveMessenger::unregisterIncomingHandler(const QString &service)
{
    invoke(SLOT(unregisterIncomingHandle(QString)), QVariant::fromValue(service));
}
