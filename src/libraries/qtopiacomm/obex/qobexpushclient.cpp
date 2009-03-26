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
#include "qobexpushclient.h"
#include <qobexheader.h>
#include <qobexclientsession.h>

#include <QString>
#include <QFile>
#include <QHash>


class QObexPushClientPrivate : public QObject
{
    Q_OBJECT

public:
    QObexPushClientPrivate(QIODevice *device, QObexPushClient *parent);
    ~QObexPushClientPrivate();

    QIODevice *sessionDevice() const;

    QObexPushClient::Error error() const;
    QObexPushClient::Command currentCommand() const;

    int connect();
    int disconnect();
    int send(QIODevice *device, const QString &filename,
             const QString &type, const QString &description);
    int send(const QByteArray &array, const QString &filename,
             const QString &type, const QString &description);

    int sendBusinessCard(QIODevice *vcard);
    int requestBusinessCard(QIODevice *vcard);
    void exchangeBusinessCard(QIODevice *mine, QIODevice *theirs,
                              int *putId = 0, int *getId = 0);

    void abort();

public slots:
    void requestStarted(int id);
    void requestFinished(int id, bool error);
    void done(bool error);

public:
    QObexPushClient *m_parent;
    QObexClientSession *m_client;

private:
    void fillPutHeader(QObexHeader &, const QString &, const QString &, const QString &);

    QHash<int, QObexPushClient::Command> m_requestCommands;
};

QObexPushClientPrivate::QObexPushClientPrivate(QIODevice *device, QObexPushClient *parent) :
        QObject(parent),
        m_client(new QObexClientSession(device, this))
{
    m_parent = parent;

    // connect signals from client to self
    QObject::connect(m_client, SIGNAL(done(bool)),
                     SLOT(done(bool)));
    QObject::connect(m_client, SIGNAL(requestStarted(int)),
                     SLOT(requestStarted(int)));
    QObject::connect(m_client, SIGNAL(requestFinished(int,bool)),
                     SLOT(requestFinished(int,bool)));

    // connect these client signals directly to parent signals
    QObject::connect(m_client, SIGNAL(dataTransferProgress(qint64,qint64)),
                     m_parent, SIGNAL(dataTransferProgress(qint64,qint64)));
}

QObexPushClientPrivate::~QObexPushClientPrivate()
{
}

QIODevice *QObexPushClientPrivate::sessionDevice() const
{
    return m_client->sessionDevice();
}

QObexPushClient::Error QObexPushClientPrivate::error() const
{
    switch (m_client->error()) {
        case QObexClientSession::NoError:
            return QObexPushClient::NoError;
        case QObexClientSession::ConnectionError:
            return QObexPushClient::ConnectionError;
        case QObexClientSession::RequestFailed:
            return QObexPushClient::RequestFailed;
        case QObexClientSession::Aborted:
            return QObexPushClient::Aborted;

        default:    // including UnknownError
            return QObexPushClient::UnknownError;
    }
}

QObexPushClient::Command QObexPushClientPrivate::currentCommand() const
{
    return m_requestCommands.value(m_client->currentId(), QObexPushClient::None);
}

int QObexPushClientPrivate::connect()
{
    int id = m_client->connect();
    m_requestCommands.insert(id, QObexPushClient::Connect);
    return id;
}

int QObexPushClientPrivate::disconnect()
{
    int id = m_client->disconnect();
    m_requestCommands.insert(id, QObexPushClient::Disconnect);
    return id;
}

int QObexPushClientPrivate::send(QIODevice *device, const QString &filename,
const QString &type, const QString &description)
{
    QObexHeader header;
    fillPutHeader(header, filename, type, description);
    if ( device && (device->isOpen() || device->open(QIODevice::ReadOnly)) ) {
        if (!device->isSequential())
            header.setLength(device->size());
    }
    int id = m_client->put(header, device);
    m_requestCommands.insert(id, QObexPushClient::Send);
    return id;
}

int QObexPushClientPrivate::send(const QByteArray &array, const QString &filename, const QString &type, const QString &description)
{
    QObexHeader header;
    fillPutHeader(header, filename, type, description);
    header.setLength(array.size());

    int id = m_client->put(header, array);
    m_requestCommands.insert(id, QObexPushClient::Send);
    return id;
}

int QObexPushClientPrivate::sendBusinessCard(QIODevice *vcard)
{
    int id = send(vcard, "BusinessCard.vcf", "text/x-vCard", QString());
    m_requestCommands.insert(id, QObexPushClient::SendBusinessCard);
    return id;
}

int QObexPushClientPrivate::requestBusinessCard(QIODevice *vcard)
{
    QObexHeader header;
    header.setType("text/x-vCard");
    if (vcard && !vcard->isOpen())
        vcard->open(QIODevice::ReadWrite);

    int id = m_client->get(header, vcard);
    m_requestCommands.insert(id, QObexPushClient::RequestBusinessCard);
    return id;
}

void QObexPushClientPrivate::exchangeBusinessCard(QIODevice *mine, QIODevice *theirs,
                                                  int *putId, int *getId )
{
    int ret;

    ret = sendBusinessCard(mine);
    if (putId)
        *putId = ret;

    ret = requestBusinessCard(theirs);
    if (getId)
        *getId = ret;
}

void QObexPushClientPrivate::abort()
{
    m_client->abort();
}

void QObexPushClientPrivate::fillPutHeader(QObexHeader &header, const QString &filename, const QString &type, const QString &description)
{
    if (!filename.isNull())
        header.setName(filename);

    if (!type.isNull())
        header.setType(type);

    if (!description.isNull())
        header.setDescription(description);
}

void QObexPushClientPrivate::requestStarted(int id)
{
    emit m_parent->commandStarted(id);
}

void QObexPushClientPrivate::requestFinished(int id, bool error)
{
    emit m_parent->commandFinished(id, error);
    m_requestCommands.remove(id);
}

void QObexPushClientPrivate::done(bool error)
{
    emit m_parent->done(error);
}



//================================================================

/*!
    \class QObexPushClient
    \inpublicgroup QtBaseModule

    \brief The QObexPushClient class encapsulates an OBEX PUSH client.

    The QObexPushClient class can be used to send files to an OBEX Push
    server.  The file sent can either be a business card (vCard), calendar (vCal),
    or any other file type.  This class can also be used to request a business card,
    or perform a business card exchange.

    Here is an example of using QObexPushClient to send a file over a
    Bluetooth connection:

    \code
    // Connect to an RFCOMM server
    QBluetoothRfcommSocket *rfcommSocket = new QBluetoothRfcommSocket;
    if (rfcommSocket->connect("11:22:33:aa:bb:cc", 9)) {

        QObexPushClient *sender = new QObexPushClient(rfcommSocket);
        sender->connect();
        QByteArray data = getSomeData();
        sender->send(data, "MyData.txt");
        sender->disconnect();
    }
    \endcode

    The functions connect(), disconnect(), send(), sendBusinessCard(),
    requestBusinessCard() and exchangeBusinessCard() are all asynchronous.
    When called, they return immediately, returning a unique identifier
    for that particular operation. If the command cannot be performed
    immediately because another command is in progress, the command is
    scheduled for later execution.

    When the execution of a command starts, the commandStarted() signal is
    emitted with the identifier of the command. When it is finished, the
    commandFinished() signal is emitted with the identifier and also a bool to
    indicate whether the command finished with an error.

    The done() signal is emitted when all pending commands have finished. This
    can be used to automatically delete the client object when it has finished
    its operations, by connecting the client's done() signal to the client's
    QObject::deleteLater() slot. (The QIODevice object can similarly be connected
    to automatically delete it when the client has finished.)

    If an error occurs during the execution of one of the commands in a
    sequence of commands, all the pending commands (i.e. scheduled, but not
    yet executed commands) are cleared and no signals are emitted for them.
    In this case, the done() signal is emitted with the error argument set to
    \c true.


    \section1 Handling socket disconnections

    You should ensure that the QIODevice provided in the constructor emits
    QIODevice::aboutToClose() or QObject::destroyed() when the associated
    transport connection is disconnected. If one of these signals
    are emitted while a command is in progress, QObexPushClient will
    know the transport connection has been lost, and will emit
    commandFinished() with \c error set to \c true, and error() will return
    ConnectionError.

    This is particularly an issue for socket classes such as QTcpSocket that
    do not emit QIODevice::aboutToClose() when a \c disconnected() signal is
    emitted. In these cases, QObexPushClient will not know that the
    transport has been disconnected. To avoid this, you can make the socket
    emit QIODevice::aboutToClose() when it is disconnected:

    \code
    // make the socket emit aboutToClose() when disconnected() is emitted
    QObject::connect(socket, SIGNAL(disconnected()), socket, SIGNAL(aboutToClose()));
    \endcode

    Or, if the socket can be discarded as soon as it is disconnected:

    \code
    // delete the socket when the transport is disconnected
    QObject::connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
    \endcode


    \sa QObexPushService, QObexClientSession

    \ingroup qtopiaobex
 */

/*!
    \enum QObexPushClient::Error
    Defines the possible errors for a push client.

    \value NoError No error has occurred.
    \value ConnectionError The client is unable to send data, or the client-server communication process is otherwise disrupted. In this case, the client and server are no longer synchronized with each other, so the QIODevice provided in the constructor should not be used for any more OBEX requests.
    \value RequestFailed The client's request was refused by the remote service, or an error occurred while sending the request.
    \value Aborted The command was aborted by a call to abort().
    \value UnknownError An error other than those specified above occurred.
 */

/*!
    \enum QObexPushClient::Command
    Defines the commands that may be returned from currentCommand() to
    indicate the command that is being executed.

    \value None No command is being executed.
    \value Connect connect() is being executed.
    \value Disconnect disconnect() is being executed.
    \value Send send() is being executed.
    \value SendBusinessCard sendBusinessCard() is being executed.
    \value RequestBusinessCard requestBusinessCard() is being executed.
 */


/*!
    Constructs an OBEX Push client that uses \a device for the transport
    connection. The \a parent is the QObject parent.

    The \a device should already be opened, or else commands will fail with
    ConnectionError.
*/
QObexPushClient::QObexPushClient(QIODevice *device, QObject *parent)
    : QObject(parent)
{
    m_data = new QObexPushClientPrivate(device, this);
}

/*!
    Destroys the client.
*/
QObexPushClient::~QObexPushClient()
{
    if (m_data)
        delete m_data;
}

/*!
    Sends a \c Connect command to the OBEX server to initiate the OBEX session.

    This function returns immediately. It will be executed asynchronously, and
    the unique identifier that is returned from this function can be used to
    track the the status of the command through the currentId() function and
    the commandStarted() and commandFinished() signals.

    \sa disconnect()
*/
int QObexPushClient::connect()
{
    return m_data->connect();
}

/*!
    Sends a \c Disconnect command to the OBEX server to close the OBEX session.

    This function returns immediately. It will be executed asynchronously, and
    the unique identifier that is returned from this function can be used to
    track the the status of the command through the currentId() function and
    the commandStarted() and commandFinished() signals.

    \bold {Note:} Some OBEX Push services close the transport connection as 
    soon as a file transfer operation is completed. In this case, if an OBEX
    Push client sends a file transfer request and then sends a disconnect()
    request, the disconnect() request will fail as the transport connection
    is no longer available.

    \sa connect()
*/
int QObexPushClient::disconnect()
{
    return m_data->disconnect();
}

/*!
    Sends the contents of \a device to the OBEX server, using the given
    \a name, \a type and \a description to describe the contents to the
    server. If a given \a name, \a type or \a description value is a null
    string, that value will not be used in the request.

    This function returns immediately. It will be executed asynchronously, and
    the unique identifier that is returned from this function can be used to
    track the the status of the command through the currentId() function and
    the commandStarted() and commandFinished() signals.
 */
int QObexPushClient::send(QIODevice *device, const QString &name,
const QString &type, const QString &description)
{
    return m_data->send(device, name, type, description);
}

/*!
    \overload

    This convenience function sends the contents of \a array to the OBEX
    server, using \a name, \a type and \a description to describe the
    contents to the server.  If a given \a name, \a type or \a description
    value is a null string, that value will not be used in the request.
*/
int QObexPushClient::send(const QByteArray &array, const QString &name,
const QString &type, const QString &description)
{
    return m_data->send(array, name, type, description);
}

/*!
    Sends the business card stored in \a vcard to the OBEX server.
*/
int QObexPushClient::sendBusinessCard(QIODevice *vcard)
{
    return m_data->sendBusinessCard(vcard);
}

/*!
    Requests a business card from the OBEX server. The received contents will
    be stored in \a vcard.
 */
int QObexPushClient::requestBusinessCard(QIODevice *vcard)
{
    return m_data->requestBusinessCard(vcard);
}

/*!
    Performs a business card exchange operation by sending this client's 
    business card, given by \a mine, and then requesting a business card from
    the remote device, which will be stored in \a theirs.

    The \a putId will be set to the unique identifier for the \c Put command 
    that is executed when the client's business card is sent. The \a getId will 
    be set to the unique identifier for \c Get command that is executed when 
    the client requests the business card from the server.

    Note that this method is equivalent to calling the sendBusinessCard()
    method and then calling the requestBusinessCard() method.
*/
void QObexPushClient::exchangeBusinessCard(QIODevice *mine, QIODevice *theirs,
                                           int *putId, int *getId)
{
    m_data->exchangeBusinessCard(mine, theirs, putId, getId);
}

/*!
    Aborts the current file transfer operation and deletes all scheduled
    commands.

    If there is a file transfer operation in progress, an \c Abort command
    will be sent to the server. When the server replies that the command is
    aborted, the commandFinished() signal will be emitted with the \c error
    argument set to \c true, and the error() function will return
    QObexPushClient::Aborted.

    If there is no file transfer operation in progress, or the operation
    finishes before it can be aborted, the operation will be completed
    normally and the \c error argument for the commandFinished() signal will
    be set to \c false.

    If no other commands are started after the call to abort(), there will be
    no scheduled commands and the done() signal will be emitted.
*/
void QObexPushClient::abort()
{
    m_data->abort();
}

/*!
    Returns the device used for this OBEX Push client session, as provided
    in the constructor.
 */
QIODevice *QObexPushClient::sessionDevice() const
{
    return m_data->sessionDevice();
}

/*!
    Returns the last error that occurred. This is useful for finding out
    what happened when receiving a commandFinished() or done()
    signal that has the \c error argument set to \c true.

    If you start a new command, the error status is reset to \c NoError.
*/
QObexPushClient::Error QObexPushClient::error() const
{
    return m_data->error();
}

/*!
    Returns the identifier of the command that is being executed, or 0 if
    there is no command being executed.

    \sa currentCommand(), hasPendingCommands()
 */
int QObexPushClient::currentId() const
{
    return m_data->m_client->currentId();
}

/*!
    Returns the command that is being executed, or QObexPushClient::None if
    there is no command being executed.

    \sa currentId()
 */
QObexPushClient::Command QObexPushClient::currentCommand() const
{
    return m_data->currentCommand();
}

/*!
    Returns true if there are any commands scheduled that have not yet been
    executed; otherwise returns false.

    The command that is being executed is not considered as a scheduled command.

    \sa clearPendingCommands(), currentId(), currentCommand()
 */
bool QObexPushClient::hasPendingCommands() const
{
    return m_data->m_client->hasPendingRequests();
}

/*!
    Deletes all pending commands from the list of scheduled commands. This
    does not affect the command that is being executed. If you want to stop
    this command as well, use abort().

    \sa hasPendingCommands(), currentId(), currentCommand(), abort()
 */
void QObexPushClient::clearPendingCommands()
{
    m_data->m_client->clearPendingRequests();
}

/*!
    Returns the server's response code for the last completed command.
 */
QObex::ResponseCode QObexPushClient::lastCommandResponse() const
{
    return m_data->m_client->lastResponseCode();
}

/*!
    \fn void QObexPushClient::commandFinished(int id, bool error);

    This signal is emitted when the client has finished processing the command
    identified by \a id. The \a error value is \c true if an error occurred
    during the processing of the command; otherwise \a error is \c false.

    \bold {Note:} Some OBEX Push services close the transport connection as 
    soon as a file transfer operation is completed. In this case, if an OBEX
    Push client sends a file transfer request and then sends a disconnect()
    request, the disconnect() request will fail as the transport connection
    is no longer available.

    \sa commandStarted(), currentId()
*/

/*!
    \fn void QObexPushClient::commandStarted(int id);

    This signal is emitted when the client has started processing the command
    identified by \a id.

    \sa commandFinished(), currentId()
*/

/*!
    \fn void QObexPushClient::done(bool error);

    This signal is emitted when all pending commands have finished; it is
    emitted after the commandFinished() signal for the last request. The
    \a error value is \c true if an error occurred during the processing
    of the command; otherwise \a error is \c false.
 */

/*!
    \fn void QObexPushClient::dataTransferProgress(qint64 done, qint64 total);

    This signal is emitted during file transfer operations to
    indicate the progress of the transfer. The \a done value is the
    number of bytes that have been sent or received so far, and \a total
    is the total number of bytes to be sent or received.
 */


#include "qobexpushclient.moc"
