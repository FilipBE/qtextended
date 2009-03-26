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

#include "qmailmessageserver.h"

#include <QtopiaIpcAdaptor>


class QTOPIAMAIL_EXPORT QMailMessageServerPrivate : public QObject
{
    Q_OBJECT

    friend class QMailMessageServer;

public:
    QMailMessageServerPrivate(QMailMessageServer* parent);
    ~QMailMessageServerPrivate();

signals:
    void initialise();

    void send(const QMailMessageIdList& mailList);

    void retrieve(const QMailAccountId& account, bool foldersOnly);
    void completeRetrieval(const QMailMessageIdList& mailList);

    void cancelTransfer();

    void acknowledgeNewMessages(const QMailMessageTypeList&);

    void deleteMessages(const QMailMessageIdList& id);

    void searchMessages(const QMailMessageKey& filter, const QString& bodyText);

    void cancelSearch();

private:
    QtopiaIpcAdaptor* adaptor;
};

QMailMessageServerPrivate::QMailMessageServerPrivate(QMailMessageServer* parent)
    : QObject(parent),
      adaptor(new QtopiaIpcAdaptor("QPE/QMailMessageServer", this))
{
    // Forward signals to the message server
    QtopiaIpcAdaptor::connect(this, SIGNAL(initialise()),
                              adaptor, MESSAGE(initialise()));
    QtopiaIpcAdaptor::connect(this, SIGNAL(send(QMailMessageIdList)),
                              adaptor, MESSAGE(send(QMailMessageIdList)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(retrieve(QMailAccountId, bool)),
                              adaptor, MESSAGE(retrieve(QMailAccountId, bool)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(completeRetrieval(QMailMessageIdList)),
                              adaptor, MESSAGE(completeRetrieval(QMailMessageIdList)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(cancelTransfer()),
                              adaptor, MESSAGE(cancelTransfer()));
    QtopiaIpcAdaptor::connect(this, SIGNAL(acknowledgeNewMessages(QMailMessageTypeList)),
                              adaptor, MESSAGE(acknowledgeNewMessages(QMailMessageTypeList)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(deleteMessages(QMailMessageIdList)),
                              adaptor, MESSAGE(deleteMessages(QMailMessageIdList)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(searchMessages(QMailMessageKey, QString)),
                              adaptor, MESSAGE(searchMessages(QMailMessageKey, QString)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(cancelSearch()),
                              adaptor, MESSAGE(cancelSearch()));

    // Propagate received events as exposed signals
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(statusChanged(QMailMessageServer::Operation, QString, QString)),
                              parent, SIGNAL(statusChanged(QMailMessageServer::Operation,QString,QString)));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(errorOccurred(QMailAccountId, QString, int)),
                              parent, SIGNAL(errorOccurred(QMailAccountId,QString,int)));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(newCountChanged(QMailMessageCountMap)),
                              parent, SIGNAL(newCountChanged(QMailMessageCountMap)));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(partialMessageRetrieved(QMailMessageMetaData)),
                              parent, SIGNAL(partialMessageRetrieved(QMailMessageMetaData)));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(partialRetrievalCompleted()),
                              parent, SIGNAL(partialRetrievalCompleted()));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(retrievalTotal(uint)),
                              parent, SIGNAL(retrievalTotal(uint)));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(retrievalProgress(uint)),
                              parent, SIGNAL(retrievalProgress(uint)));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(messageRetrieved(QMailMessageMetaData)),
                              parent, SIGNAL(messageRetrieved(QMailMessageMetaData)));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(retrievalCompleted()),
                              parent, SIGNAL(retrievalCompleted()));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(sendTotal(uint)),
                              parent, SIGNAL(sendTotal(uint)));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(sendProgress(uint)),
                              parent, SIGNAL(sendProgress(uint)));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(messageSent(QMailMessageId)),
                              parent, SIGNAL(messageSent(QMailMessageId)));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(sendCompleted()),
                              parent, SIGNAL(sendCompleted()));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(searchTotal(uint)),
                              parent, SIGNAL(searchTotal(uint)));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(searchProgress(uint)),
                              parent, SIGNAL(searchProgress(uint)));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(matchingMessageIds(QMailMessageIdList)),
                              parent, SIGNAL(matchingMessageIds(QMailMessageIdList)));
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(searchCompleted()),
                              parent, SIGNAL(searchCompleted()));
}

QMailMessageServerPrivate::~QMailMessageServerPrivate()
{
}


/*!
    \class QMailMessageServer
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \preliminary
    \brief The QMailMessageServer class provides signals and slots which implement a convenient
    interface for communicating with the MessageServer process via IPC.

    \ingroup messaginglibrary

    Qt Extended messaging applications can send and receive messages of various types by
    communicating with the external MessageServer application.  The MessageServer application
    is a separate process, communicating with clients via inter-process messages.  
    QMailMessageServer acts as a proxy object for the server process, providing an
    interface for communicating with the MessageServer by the use of signals and slots 
    in the client process.  It provides Qt signals corresponding to messages received from 
    the MessageServer application, and Qt slots which send messages to the MessageServer 
    when invoked.

    For most messaging client applications, the QMailServiceAction objects offer a simpler
    interface for requesting actions from the messageserver, and assessing their results.

    Note: the functions performed by the message server and the interface to them provided
    by the QMailMessageServer class are expected to change in a future release of Qt Extended,
    to provide a finer-grained interface for communicating with external services.

    \section1 New Messages

    When a client initiates communication with the MessageServer, the server informs the
    client of the number and type of 'new' messages, via the newCountChanged() signal.
    'New' messages are those that arrive without the client having first requested their
    retrieval.  The client may choose to invalidate the 'new' status of these messages;
    if the acknowledgeNewMessages() slot is invoked, the count of 'new' messages is reset
    to zero for the nominated message types.  If the count of 'new' messages changes while
    a client is active, the newCountChanged() signal is emitted with the updated information.

    \section1 Sending Messages

    To send messages, the client should construct instances of the QMailMessage class
    formulated to contain the desired content.  These messages should be stored to the
    mail store, then submitted for transmission by their QMailMessageId values, via the send()
    slot.  The MessageServer will determine how to transmit each message by inspecting
    the account associated with the message.

    If the MessageServer application succeeds in sending a message, the messageSent()
    signal is emitted to notify the client.  After transmission has been attempted for
    each message in the list, the sendCompleted() signal is emitted; any messages
    for which a messageSent() signal was not emitted were not successfully transmitted.

    After a send operation is initiated, the MessageServer emits the sendTotal() signal
    to indicate the approximate extent of the transmission effort.  As progress is made
    in performing the transmission operation, the sendProgress() signal is repeatedly
    emitted to report the progress toward completion of the operation.

    \section1 Retrieving Messages

    To retrieve messages, the client should invoke the retrieve() slot with the identifier
    for the account to retrieve new messages for.  If new messages are discovered, the
    messageRetrieved() signal is emitted for each message.  After retrieval has been
    attempted for all new messages, the retrievalCompleted() signal is emitted.

    After a retrieval operation is initiated, the MessageServer emits the retrievalTotal()
    signal to indicate the approximate extent of the retrieval effort.  As progress is made
    in performing the retrieval operation, the retrievalProgress() signal is repeatedly
    emitted to report the progress toward completion of the operation.

    \section2 Partial Retrieval

    If the message type supports partial retrieval (such as with POP or IMAP email
    messages), the partialMessageRetrieved() signal is emitted for each message once it
    has been partially retrieved.  After all new messages have been partially retrieved,
    the partialRetrievalCompleted() signal is emitted.

    \section2 Completing Partial Retrievals

    The client must invoke the completeRetrieval() operation in response to receiving the
    partialRetrievalCompleted() signal, to complete the retrieval operation.  A client may
    also invoke the completeRetrieval() slot without first performing a retrieval operation.
    If the list of messages to complete is empty, no further retrieval action will be taken.
    For any messages contained in the list, the MessageServer will attempt to retrieve the
    remainder of the message.  If the message whose retrieval is requested cannot be found on
    the external server, the mail store message record is updated to contain the \c Removed 
    status flag.  After retrieval completion has been attempted for all requested messages, 
    the retrievalCompleted() signal is emitted.

    \section2 Folder Retrieval

    It is possible to retrieve only the folder structure of an account rather than the
    messages contained by the folders, by invoking retrieve() retrieve slot with the 
    \c foldersOnly option set to true.  When the folder retrieval operation is complete,
    it is not necessary to invoke the completeRetrieval() operation.

    \section1 Cancellation

    In order to cancel an operation previously initiated, the client can invoke the
    cancelTransfer() slot.

    \section1 Server Status

    As the MessageServer application performs the operations requested of it, it indicates
    the tasks it is performing by emitting the statusChanged() signal.  If it encounters
    an error that prevents an operation from being performed, the errorOccurred() signal
    is emitted.

    \sa QMailServiceAction, QMailMessage, QtopiaIpcAdaptor
*/

/*!
    \enum QMailMessageServer::Operation

    This enum type is used to indicate the type of server operation to which a status notification relates.

    \value None     The notification is not specifically relating to a send or retrieve operation.
    \value Send     The notification relates to a send operation.
    \value Retrieve The notification relates to a retrieve operation.
*/

/*!
    \enum QMailMessageServer::ErrorCode

    This enum type defines error codes emitted by the MessageServer application.

    \value ErrNoError               No error has occurred.
    \value ErrUnknownResponse       The response from an external server could not be processed correctly.
    \value ErrLoginFailed           The login attempt was refused by an external server.
    \value ErrCancel                The operation was cancelled by a user action.
    \value ErrFileSystemFull        The file system does not have enough space to support the requested operation.
    \value ErrNonexistentMessage    The specified message does not exist.
    \value ErrEnqueueFailed         The supplied message could not be enqueued for transmission.
    \value ErrNoConnection          No external network connection could be selected to use for transmission.
    \value ErrConnectionInUse       The external network connection is already in use.
    \value ErrConnectionNotReady    The external network connection could not be prepared for transmission.
    \value ErrConfiguration         The account in use does not have a valid configuration.
    \value ErrInvalidAddress        The supplied message does not have a conforming address specification.
*/

/*!
    \fn void QMailMessageServer::statusChanged(QMailMessageServer::Operation operation, const QString& accountName, const QString& text);

    Emitted whenever the MessageServer performs an action that may be of interest to the client.
    The server status is described by \a text, and relates to \a operation performed on the account named \a accountName.

    \sa errorOccurred()
*/

/*!
    \fn void QMailMessageServer::errorOccurred(const QMailAccountId& accountId, const QString& text, int errorCode);

    Emitted when the MessageServer encounters an error while performing an action relating to the account with id \a accountId.
    The error is described by \a text and \a errorCode.

    \sa statusChanged()
*/

/*!
    \fn void QMailMessageServer::newCountChanged(const QMailMessageCountMap& counts);

    Emitted when the count of 'new' messages changes; the new count is described by \a counts.

    \sa acknowledgeNewMessages()
*/

/*!
    \fn void QMailMessageServer::partialMessageRetrieved(const QMailMessageMetaData& message);

    Emitted when \a message has been partially retrieved from the external server.
    \a message is delivered in meta-data-only form, since the remainder of the message
    has not yet been retrieved from the server.

    \sa retrieve(), completeRetrieval()
*/

/*!
    \fn void QMailMessageServer::partialRetrievalCompleted();

    Emitted when all new messages have been partially retrieved.

    \sa retrieve()
*/

/*!
    \fn void QMailMessageServer::retrievalTotal(uint total);

    Emitted when a retrieval operation is initiated; \a total indicates the extent of the retrieval operation to be performed.

    \sa retrievalProgress(), retrieve()
*/

/*!
    \fn void QMailMessageServer::retrievalProgress(uint value);

    Emitted during a retrieval operation; \a value reports the progress of the current retrieval operation.

    \sa retrievalTotal(), retrieve(),
*/

/*!
    \fn void QMailMessageServer::messageRetrieved(const QMailMessageMetaData& message);

    Emitted when \a message has been retrieved from the external server.
    \a message is delivered in meta-data-only form, since the size of the entire message
    may be prohibitive.

    \sa retrieve()
*/

/*!
    \fn void QMailMessageServer::retrievalCompleted();

    Emitted when a retrieval operation is completed.

    \sa retrieve()
*/

/*!
    \fn void QMailMessageServer::sendTotal(uint total);

    Emitted when a send operation is initiated; \a total indicates the extent of the send operation to be performed.

    \sa sendProgress(), send()
*/

/*!
    \fn void QMailMessageServer::sendProgress(uint value);

    Emitted during a send operation; \a value reports the progress of the current send operation.

    \sa sendTotal(), send()
*/

/*!
    \fn void QMailMessageServer::messageSent(const QMailMessageId& id);

    Emitted when the message identified by \a id has been transmitted to the external server.

    \sa send()
*/

/*!
    \fn void QMailMessageServer::sendCompleted();

    Emitted when a send operation is completed.

    \sa send()
*/

/*!
    \fn void QMailMessageServer::searchCompleted();

    Emitted when a search operation is completed.

    \sa send()
*/

/*!
    \fn void QMailMessageServer::searchTotal(uint total);

    Emitted when a search operation is initiated; \a total indicates the extent of the search operation to be performed.

    \sa searchProgress(), searchMessages()
*/

/*!
    \fn void QMailMessageServer::searchProgress(uint value);

    Emitted during a search operation; \a value reports the progress of the current search operation.

    \sa searchTotal(), searchMessages()
*/

/*!
    \fn void QMailMessageServer::matchingMessageIds(const QMailMessageIdList& ids);

    Emitted after the successful completion of a search operation; \a ids contains the list of message identifiers located by the search.

    \sa searchMessages()
*/

/*!
    Constructs a QMailMessageServer object with parent \a parent, and initiates communication with the MessageServer application.
*/
QMailMessageServer::QMailMessageServer(QObject* parent)
    : QObject(parent),
      d(new QMailMessageServerPrivate(this))
{
}

/*!
    Destroys the QMailMessageServer object.
*/
QMailMessageServer::~QMailMessageServer()
{
}

/*!
    Sends the messages in \a mailList to the MessageServer application for transmission.

    \sa sendCompleted()
*/
void QMailMessageServer::send(const QMailMessageIdList& mailList)
{
    emit d->send(mailList);
}

/*!
    Requests that the MessageServer retrieve any new messages from the account with id \a id.
    If \a foldersOnly is true, only the folder structure of the account will be
    retrieved from the external service.

    \sa retrievalCompleted()
*/
void QMailMessageServer::retrieve(const QMailAccountId& id, bool foldersOnly)
{
    emit d->retrieve(id, foldersOnly);
}

/*!
    Requests that the MessageServer complete the retrieval process for the messages in
    \a mailList. If the list contains messages from multiple accounts, only those from
    the account associated with the first message in the list will be retrieved.

    \sa partialRetrievalCompleted(), retrievalCompleted()
*/
void QMailMessageServer::completeRetrieval(const QMailMessageIdList& mailList)
{
    emit d->completeRetrieval(mailList);
}

/*!
    Requests that the MessageServer cancel any pending transfer operations.

    \sa send(), retrieve(), completeRetrieval()
*/
void QMailMessageServer::cancelTransfer()
{
    emit d->cancelTransfer();
}

/*!
    Requests that the MessageServer reset the counts of 'new' messages to zero, for
    each message type listed in \a types.

    \sa newCountChanged()
*/
void QMailMessageServer::acknowledgeNewMessages(const QMailMessageTypeList& types)
{
    emit d->acknowledgeNewMessages(types);
}

/*!
    Requests that the MessageServer delete the messages in \a mailList from the external
    server, if necessary for the relevant message type.

    Deleting messages using this slot does not initiate communication with any external
    server; instead the information needed to delete the messages is recorded.  Deletion
    from the external server will occur when messages are next retrieved from that server.
    Invoking this slot does not remove a message from the mail store.

    \sa QMailStore::removeMessage()
*/
void QMailMessageServer::deleteMessages(const QMailMessageIdList& mailList)
{
    emit d->deleteMessages(mailList);
}

/*!
    Requests that the MessageServer search for messages that meet the criteria encoded
    in \a filter.  If \a bodyText is non-empty, messages must also contain the specified
    text in their content to be considered matching.  If the content of a message is not
    stored locally, the MessageServer may connect to the originating server to request
    a content search, if necessary.

    The identifiers of all matching messages are returned via matchingMessageIds() after 
    the search is completed.

    \sa matchingMessageIds()
*/
void QMailMessageServer::searchMessages(const QMailMessageKey& filter, const QString& bodyText)
{
    emit d->searchMessages(filter, bodyText);
}

/*!
    Requests that the MessageServer cancel any pending search operations.
*/
void QMailMessageServer::cancelSearch()
{
    emit d->cancelSearch();
}

Q_IMPLEMENT_USER_METATYPE_ENUM(QMailMessageServer::Operation)
Q_IMPLEMENT_USER_METATYPE_ENUM(QMailMessageServer::ErrorCode)

Q_IMPLEMENT_USER_METATYPE_TYPEDEF(QMailMessageCountMap, QMailMessageCountMap)

#include "qmailmessageserver.moc"

