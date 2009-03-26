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

// Ensure we don't try to instantiate types defined in qmailmessage.h
#define SUPPRESS_REGISTER_QMAILMESSAGE_METATYPES
#include "qprivateimplementationdef.h"

#include "qmailserviceaction.h"

#include <QtopiaIpcAdaptor>
#include <QMailMessageKey>
#include <QMailMessageServer>
#include <QMailStore>
#include <QTimer>

#include "qtopialog.h"


// Note: this implementation is still using the QMailMessageServer interface, which will 
// probably be deprecated.  The entire implementation is likely to change; this is a 
// transitional version, allowing qtmail conversion but not providing true mapping
// of events to actions, nor unicasting of event messages.

class QMailServiceActionPrivate 
    : public QObject,
      public QPrivateNoncopyableBase
{
    Q_OBJECT

public:
    template<typename Subclass>
    QMailServiceActionPrivate(Subclass *p, QMailServiceAction *i);

    ~QMailServiceActionPrivate();

    void cancelOperation();

protected slots:
    void errorOccurred(const QMailAccountId&, const QString&, int);

protected:
    friend class QMailServiceAction;

    virtual void init();

    void setActivity(QMailServiceAction::Activity newActivity);
    void setConnectivity(QMailServiceAction::Connectivity newConnectivity);

    void setStatus(QMailServiceAction::Status::ErrorCode code, const QString &text);
    void setStatus(QMailServiceAction::Status::ErrorCode code, const QString &text, const QMailAccountId &accountId, const QMailFolderId &folderId, const QMailMessageId &messageId);

    void setTotal(uint newTotal);
    void setProgress(uint newProgress);

    void emitChanges();

    QMailServiceAction *_interface;
    QMailMessageServer *_server;

    QMailServiceAction::Connectivity _connectivity;
    QMailServiceAction::Activity _activity;
    QMailServiceAction::Status _status;

    uint _total;
    uint _progress;

    bool _active;

    bool _connectivityChanged;
    bool _activityChanged;
    bool _progressChanged;
    bool _statusChanged;
};

template<typename Subclass>
QMailServiceActionPrivate::QMailServiceActionPrivate(Subclass *p, QMailServiceAction *i)
    : QPrivateNoncopyableBase(p),
      _interface(i),
      _server(new QMailMessageServer(this)),
      _connectivity(QMailServiceAction::Offline),
      _activity(QMailServiceAction::Pending),
      _status(QMailServiceAction::Status::ErrNoError, QString(), QMailAccountId(), QMailFolderId(), QMailMessageId()),
      _total(0),
      _progress(0),
      _active(false)
{
    connect(_server, SIGNAL(errorOccurred(QMailAccountId, QString, int)),
            this, SLOT(errorOccurred(QMailAccountId, QString, int)));
}

QMailServiceActionPrivate::~QMailServiceActionPrivate()
{
}

void QMailServiceActionPrivate::cancelOperation()
{
    if (_active) {
        _server->cancelTransfer();
    }
}

void QMailServiceActionPrivate::errorOccurred(const QMailAccountId &id, const QString &text, int code)
{
    if (_active) {
        setStatus(static_cast<QMailServiceAction::Status::ErrorCode>(code), text, id, QMailFolderId(), QMailMessageId());
        setConnectivity(QMailServiceAction::Disconnected);
        setActivity(QMailServiceAction::Failed);

        emitChanges();
    }
}

void QMailServiceActionPrivate::init()
{
    _connectivity = QMailServiceAction::Offline;
    _activity = QMailServiceAction::Pending;
    _status = QMailServiceAction::Status(QMailServiceAction::Status::ErrNoError, QString(), QMailAccountId(), QMailFolderId(), QMailMessageId());
    _total = 0;
    _progress = 0;
    _active = false;
    _connectivityChanged = false;
    _activityChanged = false;
    _progressChanged = false;
    _statusChanged = false;
}

void QMailServiceActionPrivate::setConnectivity(QMailServiceAction::Connectivity newConnectivity)
{
    if (_active && (_connectivity != newConnectivity)) {
        _connectivity = newConnectivity;
        _connectivityChanged = true;
    }
}

void QMailServiceActionPrivate::setActivity(QMailServiceAction::Activity newActivity)
{
    if (_active && (_activity != newActivity)) {
        _activity = newActivity;

        if (_activity == QMailServiceAction::Failed || _activity == QMailServiceAction::Successful) {
            // Reset any progress we've indicated
            _total = 0;
            _progress = 0;
            _progressChanged = true;

            // We're finished
            _active = false;
        }

        _activityChanged = true;
    }
}

void QMailServiceActionPrivate::setStatus(QMailServiceAction::Status::ErrorCode code, const QString &text)
{
    setStatus(code, text, QMailAccountId(), QMailFolderId(), QMailMessageId());
}

void QMailServiceActionPrivate::setStatus(QMailServiceAction::Status::ErrorCode code, const QString &text, const QMailAccountId &accountId, const QMailFolderId &folderId, const QMailMessageId &messageId)
{
    if (_active) {
        _status = QMailServiceAction::Status(code, text, accountId, folderId, messageId);
        _statusChanged = true;
    }
}

void QMailServiceActionPrivate::setTotal(uint newTotal)
{
    if (_active) {
        if (newTotal != _total) {
            _total = newTotal;
            _progress = 0;
            _progressChanged = true;
        }

        setActivity(QMailServiceAction::InProgress);
    }
}

void QMailServiceActionPrivate::setProgress(uint newProgress)
{
    if (_active) {
        newProgress = qMin(newProgress, _total);
        if (newProgress != _progress) {
            _progress = newProgress;
            _progressChanged = true;
        }
    }
}

void QMailServiceActionPrivate::emitChanges()
{
    if (_connectivityChanged) {
        _connectivityChanged = false;
        emit _interface->connectivityChanged(_connectivity);
    }
    if (_activityChanged) {
        _activityChanged = false;
        emit _interface->activityChanged(_activity);
    }
    if (_progressChanged) {
        _progressChanged = false;
        emit _interface->progressChanged(_progress, _total);
    }
    if (_statusChanged) {
        _statusChanged = false;
        emit _interface->statusChanged(_status);
    }
}

template class QPrivatelyNoncopyable<QMailServiceActionPrivate>;


/*!
    \class QMailServiceAction::Status
    \inpublicgroup QtMessagingModule

    \preliminary
    \ingroup messaginglibrary

    \brief The Status class encapsulates the instantaneous state of a QMailServiceAction.

    QMailServiceAction::Status contains the pieces of information that describe the state of a
    requested action.  The \l errorCode reflects the overall state, and may be supplemented
    by a description in \l text.

    If \l errorCode is not equal to \l ErrNoError, then each of \l accountId, \l folderId and 
    \l messageId may have been set to a valid identifier, if pertinent to the situation.
*/

/*!
    \enum QMailServiceAction::Status::ErrorCode

    This enum type is used to identify common error conditions encountered when implementing service actions.

    \value ErrNoError               No error was encountered.
    \value ErrCancel                The operation was canceled by user intervention.
    \value ErrConfiguration         The configuration needed for the requested action is invalid.
    \value ErrNoConnection          A connection could not be established to the external service.
    \value ErrConnectionInUse       The connection to the external service is exclusively held by another user.
    \value ErrConnectionNotReady    The connection to the external service is not ready for operation.
    \value ErrUnknownResponse       The response from the external service could not be handled.
    \value ErrLoginFailed           The external service did not accept the supplied authentication details.
    \value ErrFileSystemFull        The action could not be performed due to insufficient storage space.
    \value ErrNonexistentMessage    The specified message does not exist.
    \value ErrEnqueueFailed         The specified message could not be enqueued for transmission.
    \value ErrInvalidAddress        The specified address is invalid for the requested action.
*/

/*! \variable QMailServiceAction::Status::errorCode
    
    Describes the error condition encountered by the action.
*/

/*! \variable QMailServiceAction::Status::text
    
    Provides a human-readable description of the error condition in \l errorCode.
*/

/*! \variable QMailServiceAction::Status::accountId
    
    If relevant to the \l errorCode, contains the ID of the associated account.
*/

/*! \variable QMailServiceAction::Status::folderId
    
    If relevant to the \l errorCode, contains the ID of the associated folder.
*/

/*! \variable QMailServiceAction::Status::messageId
    
    If relevant to the \l errorCode, contains the ID of the associated message.
*/

/*! 
    \fn QMailServiceAction::Status::Status(ErrorCode, const QString&, const QMailAccountId&, const QMailFolderId&, const QMailMessageId&)

    Constructs a status object with 
    \l errorCode set to \a c, 
    \l text set to \a t, 
    \l accountId set to \a a, 
    \l folderId set to \a f and 
    \l messageId set to \a m.
*/
QMailServiceAction::Status::Status(ErrorCode c, const QString &t, const QMailAccountId &a, const QMailFolderId &f, const QMailMessageId &m)
    : errorCode(c),
      text(t),
      accountId(a),
      folderId(f),
      messageId(m)
{
}

/*! 
    \fn QMailServiceAction::Status::Status(const Status&)

    Constructs a status object which is a copy of \a other.
*/
QMailServiceAction::Status::Status(const QMailServiceAction::Status &other)
    : errorCode(other.errorCode),
      text(other.text),
      accountId(other.accountId),
      folderId(other.folderId),
      messageId(other.messageId)
{
}


/*!
    \class QMailServiceAction
    \inpublicgroup QtMessagingModule

    \preliminary
    \ingroup messaginglibrary

    \brief The QMailServiceAction class provides the interface for requesting actions from external message services.

    QMailServiceAction provides the mechanisms for messaging clients to request actions from 
    external services, and to receive information in response.  The details of requests
    differ for each derived action, and the specific data returned is also variable.  However,
    all actions present the same interface for communicating status, connectivity and 
    progress information.

    All actions communicate changes in their operational state by emitting the activityChanged()
    signal.  If the execution of the action requires connectivity to an external service, then
    changes in connectivity state are emitted via the connectivityChanged() signal.  Some actions
    are able to provide progress information as they execute; these changes are reported via the 
    progressChanged() signal.  If error conditions are encountered, the statusChanged() signal is
    emitted to report the new status.

    A user may attempt to cancel an operation after it has been initiated.  The cancelOperation()
    slot is provided for this purpose.
*/

/*!
    \enum QMailServiceAction::Connectivity

    This enum type is used to describe the connectivity state of the service implementing an action.

    \value Offline          The service is offline.
    \value Connecting       The service is currently attempting to establish a connection.
    \value Connected        The service is connected.
    \value Disconnected     The service has been disconnected.
*/

/*!
    \enum QMailServiceAction::Activity

    This enum type is used to describe the activity state of the requested action.

    \value Pending          The action has not yet begun execution.
    \value InProgress       The action is currently executing.
    \value Successful       The action has completed successfully.
    \value Failed           The action could not be completed successfully, and has finished execution.
*/

/*!
    \fn QMailServiceAction::connectivityChanged(QMailServiceAction::Connectivity c)

    This signal is emitted when the connectivity status of the service performing 
    the action changes, with the new state described by \a c.

    \sa connectivity()
*/

/*!
    \fn QMailServiceAction::activityChanged(QMailServiceAction::Activity a)

    This signal is emitted when the activity status of the action changes,
    with the new state sdescribed by \a a.

    \sa activity()
*/

/*!
    \fn QMailServiceAction::statusChanged(const QMailServiceAction::Status &s)

    This signal is emitted when the error status of the action changes, with
    the new status described by \a s.

    \sa status()
*/

/*!
    \fn QMailServiceAction::progressChanged(uint value, uint total)

    This signal is emitted when the progress of the action changes, with
    the new state described by \a value and \a total.

    \sa progress()
*/

/*!
    \typedef QMailServiceAction::ImplementationType
    \internal
*/

/*!
    \fn QMailServiceAction::QMailServiceAction(Subclass*, QObject*)
    \internal
*/
template<typename Subclass>
QMailServiceAction::QMailServiceAction(Subclass* p, QObject *parent)
    : QObject(parent),
      QPrivatelyNoncopyable<QMailServiceActionPrivate>(p)
{
}

/*! \internal */
QMailServiceAction::~QMailServiceAction()
{
}

/*!
    Returns the current connectivity state of the service implementing this action.

    \sa connectivityChanged()
*/
QMailServiceAction::Connectivity QMailServiceAction::connectivity() const
{
    return impl(this)->_connectivity;
}

/*!
    Returns the current activity state of the action.

    \sa activityChanged()
*/
QMailServiceAction::Activity QMailServiceAction::activity() const
{
    return impl(this)->_activity;
}

/*!
    Returns the current status of the service action.

    \sa statusChanged()
*/
const QMailServiceAction::Status QMailServiceAction::status() const
{
    return impl(this)->_status;
}

/*!
    Returns the current progress of the service action.

    \sa progressChanged()
*/
QPair<uint, uint> QMailServiceAction::progress() const
{
    return qMakePair(impl(this)->_progress, impl(this)->_total);
}

/*!
    Attempts to cancel the last requested operation.
*/
void QMailServiceAction::cancelOperation()
{
    impl(this)->cancelOperation();
}

/*!
    Set the current status so that 
    \l{QMailServiceAction::Status::errorCode} errorCode is set to \a c and 
    \l{QMailServiceAction::Status::text} text is set to \a t.
*/
void QMailServiceAction::setStatus(QMailServiceAction::Status::ErrorCode c, const QString &t)
{
    impl(this)->setStatus(c, t, QMailAccountId(), QMailFolderId(), QMailMessageId());
}

/*!
    Set the current status so that 
    \l{QMailServiceAction::Status::errorCode} errorCode is set to \a c, 
    \l{QMailServiceAction::Status::text} text is set to \a t,
    \l{QMailServiceAction::Status::accountId} accountId is set to \a a, 
    \l{QMailServiceAction::Status::folderId} folderId is set to \a f and 
    \l{QMailServiceAction::Status::messageId} messageId is set to \a m.
*/
void QMailServiceAction::setStatus(QMailServiceAction::Status::ErrorCode c, const QString &t, const QMailAccountId &a, const QMailFolderId &f, const QMailMessageId &m)
{
    impl(this)->setStatus(c, t, a, f, m);
}


class QMailRetrievalActionPrivate : public QMailServiceActionPrivate
{
    Q_OBJECT

public:
    QMailRetrievalActionPrivate(QMailRetrievalAction *);

    void retrieve(const QMailAccountId &accountId, bool foldersOnly);
    void completeRetrieval(const QMailMessageIdList &ids);

protected slots:
    void statusChanged(QMailMessageServer::Operation, const QString&, const QString&);
    void retrievalTotal(uint);
    void retrievalProgress(uint);
    void retrievalCompleted();

private:
    friend class QMailRetrievalAction;
};

QMailRetrievalActionPrivate::QMailRetrievalActionPrivate(QMailRetrievalAction *i)
    : QMailServiceActionPrivate(this, i)
{
    connect(_server, SIGNAL(statusChanged(QMailMessageServer::Operation, QString, QString)),
            this, SLOT(statusChanged(QMailMessageServer::Operation, QString, QString)));
    connect(_server, SIGNAL(retrievalTotal(uint)),
            this, SLOT(retrievalTotal(uint)));
    connect(_server, SIGNAL(retrievalProgress(uint)),
            this, SLOT(retrievalProgress(uint)));
    connect(_server, SIGNAL(partialRetrievalCompleted()),
            this, SLOT(retrievalCompleted()));
    connect(_server, SIGNAL(retrievalCompleted()),
            this, SLOT(retrievalCompleted()));

    init();
}

void QMailRetrievalActionPrivate::retrieve(const QMailAccountId &accountId, bool foldersOnly)
{
    init();

    _active = true;
    _server->retrieve(accountId, foldersOnly);
}

void QMailRetrievalActionPrivate::completeRetrieval(const QMailMessageIdList &ids)
{
    init();

    _active = true;
    _server->completeRetrieval(ids);
}

void QMailRetrievalActionPrivate::statusChanged(QMailMessageServer::Operation op, const QString &, const QString &text)
{
    if (op != QMailMessageServer::Send) {
        setStatus(QMailServiceAction::Status::ErrNoError, text);
        emitChanges();
    }
}

void QMailRetrievalActionPrivate::retrievalTotal(uint newTotal)
{
    setTotal(newTotal);
    emitChanges();
}

void QMailRetrievalActionPrivate::retrievalProgress(uint newProgress)
{
    setProgress(newProgress);
    emitChanges();
}

void QMailRetrievalActionPrivate::retrievalCompleted()
{
    QMailServiceAction::Activity result(true ? QMailServiceAction::Successful : QMailServiceAction::Failed);
    setActivity(result);
    emitChanges();
}

template class QPrivatelyNoncopyable<QMailRetrievalActionPrivate>;


/*!
    \class QMailRetrievalAction
    \inpublicgroup QtMessagingModule

    \preliminary
    \ingroup messaginglibrary

    \brief The QMailRetrievalAction class provides the interface for retrieving messages from external message services.

    QMailRetrievalAction provides the mechanism for messaging clients to request that the message
    server retrieve messages from external services.  The retrieval action object reports on
    the progress and outcome of its activities via the signals inherited from QMailServiceAction.

    The retrieve() slot requests that the message server synchronize the set of known message
    identifiers with those available for the nominated account.  The completeRetrieval() slot
    requests that the message server retrieve the content of each identified message.  A client
    that requests the retrieve() action should subsequently request the completeRetrieval()
    action after the first operation is successful (possibly with an empty message list) to 
    terminate any remaining connection.

    The retrieveFolders() slot requests that the message server synchronize the folder
    structure of the account with that available from the external server.

    Note: the slots exported by QMailRetrievalAction are expected to change in future releases, as 
    the message server is extended to provide a finer-grained interface for message discovery and retrieval.
*/

/*!
    \typedef QMailRetrievalAction::ImplementationType
    \internal
*/

/*! 
    Constructs a new retrieval action object with the supplied \a parent.
*/
QMailRetrievalAction::QMailRetrievalAction(QObject *parent)
    : QMailServiceAction(new QMailRetrievalActionPrivate(this), parent)
{
}

/*! \internal */
QMailRetrievalAction::~QMailRetrievalAction()
{
}

/*
void QMailRetrievalAction::retrieveFolderList(const QMailFolderId &folderId)
{
}

void QMailRetrievalAction::retrieveMessageList(const QMailFolderId &folderId)
{
}

void QMailRetrievalAction::retrieveMessages(const QMailMessageIdList &messageIds, RetrievalSpecification spec)
{
}

void QMailRetrievalAction::retrieveMessageElement(const QMailMessageId &messageId, const QString &elementIdentifier)
{
}

void QMailRetrievalAction::retrieveAll(const QMailAccountId &accountId, RetrievalSpecification spec)
{
}

void QMailRetrievalAction::exportUpdates(const QMailAccountId &accountId)
{
}

void QMailRetrievalAction::synchronize(const QMailAccountId &accountId, RetrievalSpecification spec)
{
}
*/

/*!
    Requests that the message server synchronize the set of known message identifiers 
    with those currently available for the account identified by \a accountId.

    New messages will be added to the mail store as they are discovered, and 
    marked with the \l QMailMessage::New status flag.  Messages that are no longer 
    available will be marked with the \l QMailMessage::Removed status flag.  
    The folder structure of the account will also be synchronized with that
    available from the external service.

    After a retrieve operation is successfully completed, a client should request the
    completeRetrieval() operation to terminate any remaining connection.  
*/
void QMailRetrievalAction::retrieve(const QMailAccountId &accountId)
{
    impl(this)->retrieve(accountId, false);
}

/*!
    Requests that the message server retrieve the full content of each message listed in \a ids
    and terminate the connection.  The message list may be empty.

    It is possible to request completion for a message list without having previously requested
    the message server to perform a retrieve().
*/
void QMailRetrievalAction::completeRetrieval(const QMailMessageIdList &ids)
{
    impl(this)->completeRetrieval(ids);
}

/*!
    Requests that the message server synchronize the folder structure of the account 
    identified by \a accountId with that available from the external service. 
    
    It is not necessary to request the completeRetrieval() operation after retrieveFolders()
    is successfully completed.
*/
void QMailRetrievalAction::retrieveFolders(const QMailAccountId &accountId)
{
    impl(this)->retrieve(accountId, true);
}


class QMailTransmitActionPrivate : public QMailServiceActionPrivate
{
    Q_OBJECT

public:
    QMailTransmitActionPrivate(QMailTransmitAction *i);

    //void transmitMessages(const QMailAccountId &accountId);
    void send(const QMailMessageIdList &);

protected:
    virtual void init();

protected slots:
    void statusChanged(QMailMessageServer::Operation, const QString&, const QString&);
    void sendTotal(uint);
    void sendProgress(uint);
    void messageSent(const QMailMessageId &id);
    void sendCompleted();

private:
    friend class QMailTransmitAction;

    QMailMessageIdList _ids;
};

QMailTransmitActionPrivate::QMailTransmitActionPrivate(QMailTransmitAction *i)
    : QMailServiceActionPrivate(this, i)
{
    connect(_server, SIGNAL(statusChanged(QMailMessageServer::Operation, QString, QString)),
            this, SLOT(statusChanged(QMailMessageServer::Operation, QString, QString)));
    connect(_server, SIGNAL(sendTotal(uint)),
            this, SLOT(sendTotal(uint)));
    connect(_server, SIGNAL(sendProgress(uint)),
            this, SLOT(sendProgress(uint)));
    connect(_server, SIGNAL(messageSent(QMailMessageId)),
            this, SLOT(messageSent(QMailMessageId)));
    connect(_server, SIGNAL(sendCompleted()),
            this, SLOT(sendCompleted()));

    init();
}

/*
void QMailTransmitActionPrivate::transmitMessages(const QMailAccountId &accountId)
{
}
*/

void QMailTransmitActionPrivate::send(const QMailMessageIdList &ids)
{
    init();

    _active = true;
    _ids = ids;

    _server->send(_ids);
    emitChanges();
}

void QMailTransmitActionPrivate::init()
{
    QMailServiceActionPrivate::init();

    _ids.clear();
}

void QMailTransmitActionPrivate::statusChanged(QMailMessageServer::Operation op, const QString &, const QString &text)
{
    if (op != QMailMessageServer::Retrieve) {
        setStatus(QMailServiceAction::Status::ErrNoError, text);
        emitChanges();
    }
}

void QMailTransmitActionPrivate::sendTotal(uint newTotal)
{
    setTotal(newTotal);
    emitChanges();
}

void QMailTransmitActionPrivate::sendProgress(uint newProgress)
{
    setProgress(newProgress);
    emitChanges();
}

void QMailTransmitActionPrivate::messageSent(const QMailMessageId &id)
{
    if (_active) {
        _ids.removeAll(id);
    }
}

void QMailTransmitActionPrivate::sendCompleted()
{
    QMailServiceAction::Activity result(_ids.isEmpty() ? QMailServiceAction::Successful : QMailServiceAction::Failed);
    setActivity(result);
    emitChanges();
}

template class QPrivatelyNoncopyable<QMailTransmitActionPrivate>;


/*!
    \class QMailTransmitAction
    \inpublicgroup QtMessagingModule

    \preliminary
    \ingroup messaginglibrary

    \brief The QMailTransmitAction class provides the interface for transmitting messages to external message services.

    QMailSearchAction provides the mechanism for messaging clients to request that the message
    server transmit messages to external services.  The transmit action object reports on
    the progress and outcome of its activities via the signals inherited from QMailServiceAction.

    The send() slot requests that the message server transmit each identified message to the
    external service associated with each messages' account.

    Note: the slots exported by QMailTransmitAction are expected to change in future releases, as 
    the message server is extended to provide a finer-grained interface for message transmission.
*/

/*!
    \typedef QMailTransmitAction::ImplementationType
    \internal
*/

/*!
    Constructs a new transmit action object with the supplied \a parent.
*/
QMailTransmitAction::QMailTransmitAction(QObject *parent)
    : QMailServiceAction(new QMailTransmitActionPrivate(this), parent)
{
}

/*! \internal */
QMailTransmitAction::~QMailTransmitAction()
{
}

/*
void QMailTransmitAction::transmitMessages(const QMailAccountId &accountId)
{
    impl(this)->transmitMessages(accountId);
}
*/

/*!
    Requests that the message server transmit each message listed in \a ids to the
    external service associated with that messages' account.
*/
void QMailTransmitAction::send(const QMailMessageIdList &ids)
{
    impl(this)->send(ids);
}


class QMailSearchActionPrivate : public QMailServiceActionPrivate
{
    Q_OBJECT

public:
    QMailSearchActionPrivate(QMailSearchAction *i);

    void searchMessages(const QMailMessageKey &filter, const QString &bodyText);
    void cancelOperation();

protected:
    virtual void init();

private slots:
    void statusChanged(QMailMessageServer::Operation, const QString&, const QString&);
    void searchTotal(uint);
    void searchProgress(uint);
    void matchingMessageIds(const QMailMessageIdList &ids);
    void searchCompleted();
    void performSearch();

private:
    friend class QMailSearchAction;

    QMailMessageKey _filter;
    QMailMessageIdList _matchingIds;
};

QMailSearchActionPrivate::QMailSearchActionPrivate(QMailSearchAction *i)
    : QMailServiceActionPrivate(this, i)
{
    connect(_server, SIGNAL(statusChanged(QMailMessageServer::Operation, QString, QString)),
            this, SLOT(statusChanged(QMailMessageServer::Operation, QString, QString)));
    connect(_server, SIGNAL(searchTotal(uint)),
            this, SLOT(searchTotal(uint)));
    connect(_server, SIGNAL(searchProgress(uint)),
            this, SLOT(searchProgress(uint)));
    connect(_server, SIGNAL(matchingMessageIds(QMailMessageIdList)),
            this, SLOT(matchingMessageIds(QMailMessageIdList)));
    connect(_server, SIGNAL(searchCompleted()),
            this, SLOT(searchCompleted()));

    init();
}

void QMailSearchActionPrivate::searchMessages(const QMailMessageKey &filter, const QString &bodyText)
{
    init();

    _active = true;

    if (!bodyText.isEmpty()) {
        _server->searchMessages(filter, bodyText);
    } else {
        // This search can be performed locally
        _filter = filter;
        setActivity(QMailServiceAction::InProgress);
        QTimer::singleShot(0, this, SLOT(performSearch()));
    }
}

void QMailSearchActionPrivate::cancelOperation()
{
    if(_active)
        _server->cancelSearch();
}

void QMailSearchActionPrivate::init()
{
    QMailServiceActionPrivate::init();

    _matchingIds.clear();
    _filter = QMailMessageKey();
}

void QMailSearchActionPrivate::statusChanged(QMailMessageServer::Operation op, const QString &, const QString &text)
{
    if (op == QMailMessageServer::None) {
        setStatus(QMailServiceAction::Status::ErrNoError, text);
        emitChanges();
    }
}

void QMailSearchActionPrivate::searchTotal(uint newTotal)
{
    setTotal(newTotal);
    emitChanges();
}

void QMailSearchActionPrivate::searchProgress(uint newProgress)
{
    setProgress(newProgress);
    emitChanges();
}

void QMailSearchActionPrivate::matchingMessageIds(const QMailMessageIdList &ids)
{
    if (_active) {
        _matchingIds += ids;
    }
}

void QMailSearchActionPrivate::searchCompleted()
{
    QMailServiceAction::Activity result(QMailServiceAction::Successful);
    setActivity(result);
    emitChanges();
}

void QMailSearchActionPrivate::performSearch()
{
    _matchingIds = QMailStore::instance()->queryMessages(_filter);
    setActivity(QMailServiceAction::Successful);
    emitChanges();
}

template class QPrivatelyNoncopyable<QMailSearchActionPrivate>;


/*!
    \class QMailSearchAction
    \inpublicgroup QtMessagingModule

    \preliminary
    \ingroup messaginglibrary

    \brief The QMailSearchAction class provides the interface for identifying messages that match a set of search criteria.

    QMailSearchAction provides the mechanism for messaging clients to request that the message
    server perform a search for messages that match the supplied search criteria.  For criteria
    pertaining to message meta data, the message server will search within the locally held
    meta data.  For a textual content criterion, the message server will search locally where
    the message content is held locally, and on the external server for messages whose content
    has not been retrieved (provided the external service provides a search facility).
*/

/*!
    \typedef QMailSearchAction::ImplementationType
    \internal
*/

/*!
    Constructs a new search action object with the supplied \a parent.
*/
QMailSearchAction::QMailSearchAction(QObject *parent)
    : QMailServiceAction(new QMailSearchActionPrivate(this), parent)
{
}

/*! \internal */
QMailSearchAction::~QMailSearchAction()
{
}

/*!
    Requests that the message server identify all messages that match the criteria
    specified by \a filter.  If \a bodyText is non-empty, identified messages must
    also contain the supplied text in their content.  

    For messages whose content is not held locally, the external service will be 
    requested to perform the content search, if that facility is available.
*/
void QMailSearchAction::searchMessages(const QMailMessageKey &filter, const QString &bodyText)
{
    impl(this)->searchMessages(filter, bodyText);
}

/*!
    Attempts to cancel the last requested search operation.
*/
void QMailSearchAction::cancelOperation()
{
    impl(this)->cancelOperation();
}

/*!
    Returns the list of message identifiers found to match the search criteria.
    Unless activity() returns \l Successful, an empty list is returned.
*/
QMailMessageIdList QMailSearchAction::matchingMessageIds() const
{
    return impl(this)->_matchingIds;
}


Q_IMPLEMENT_USER_METATYPE_ENUM(QMailServiceAction::Connectivity)
Q_IMPLEMENT_USER_METATYPE_ENUM(QMailServiceAction::Activity)
Q_IMPLEMENT_USER_METATYPE_ENUM(QMailServiceAction::Status::ErrorCode)


#include "qmailserviceaction.moc"

