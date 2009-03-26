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
#include "qobexserversession.h"
#include "qobexserversession_p.h"
#include <qobexheader.h>
#include "qobexheader_p.h"
#include "qobexauthenticationchallenge_p.h"
#include "qobexauthenticationresponse_p.h"

#include <qtopialog.h>

#include <QBuffer>
#include <QMetaMethod>
#include <QSet>
#include <QApplication>


QObexServerSessionPrivate::QObexServerSessionPrivate(QIODevice *device, QObexServerSession *parent)
    : QObject(parent),
      m_socket(new QObexSocket(device, this)),
      m_parent(parent),
      m_closed(false),
      m_socketDisconnected(false)
{
    if (!m_socket || !m_socket->isValid()) {
        qWarning("QObexServerSession: cannot initialise OBEX connection");
        return;
    }

    m_socket->setObexServer(this);

    QObject::connect(m_socket->device(), SIGNAL(aboutToClose()),
                        this, SLOT(socketDisconnected()));
    QObject::connect(m_socket->device(), SIGNAL(readChannelFinished()),
                        this, SLOT(socketDisconnected()));
    QObject::connect(m_socket->device(), SIGNAL(destroyed()),
                        this, SLOT(socketDisconnected()));
    QObject::connect(m_socket, SIGNAL(destroyed()),
                        this, SLOT(socketDisconnected()));
}

QObexServerSessionPrivate::~QObexServerSessionPrivate()
{
    close();
}

void QObexServerSessionPrivate::close()
{
    if (m_closed)
        return;

    // Don't receive any more events from the obex socket
    if (m_socket)
        m_socket->setObexServer(0);

    m_closed = true;
}

void QObexServerSessionPrivate::setNextResponseHeader(const QObexHeader &header)
{
    m_nextResponseHeader = header;

    // Need to keep the nonce if sending an auth challenge, in order to match
    // password when auth response is received.
    if (header.contains(QObexHeader::AuthChallenge)) {
        m_challengeNonce = QObexHeaderPrivate::getChallengeNonce(header);
    }
}

//--------------------------------------------------

void QObexServerSessionPrivate::resetOpData()
{
    m_invokedRequestSlot = false;

    // Don't clear m_challengeNonce, otherwise can't match the password
    // when a new request comes with an Auth Response. Can only clear
    // the challenge nonce when a request finishes with Success.
}

/*
    Called when a new request has been detected.
*/
QObex::ResponseCode QObexServerSessionPrivate::acceptIncomingRequest(QObex::Request request)
{
    qLog(Obex) << "QObexServerSession: incoming request" << request;

    // reset data for the new request
    resetOpData();

    if (m_implementedCallbacks.empty())
        initAvailableCallbacks();

    if (m_implementedCallbacks.contains(getRequestSlot(request)))
        return QObex::Success;
        
    qLog(Obex) << "QObexServerSession: request" << request 
        << "callback not implemented, denying request";   
    return QObex::NotImplemented;
}

/*
    Called when the first packet of a request has been received and there
    is more than 1 packet in the operation. This is only likely to be true
    for Put requests.
*/
QObex::ResponseCode QObexServerSessionPrivate::receivedRequestFirstPacket(QObex::Request request, QObexHeader &header, QObexHeader *responseHeader)
{
    qLog(Obex) << "QObexServerSession: got 1st packet for request" 
        << request << "with" << header.size() << "headers"; 

    QObex::ResponseCode response =
            processRequestHeader(request, header, QByteArray());

    if (m_closed)
        return QObex::InternalServerError;

    // Subclass may have set some response headers.
    *responseHeader = m_nextResponseHeader;
    m_nextResponseHeader.clear();
    return response;
}

/*
    Called when final server response has been sent.
    Not called for Aborts.
*/
void QObexServerSessionPrivate::requestDone(QObex::Request request)
{
    qLog(Obex) << "QObexServerSession: done request" << request;

    if (request != QObex::NoRequest) {
        if (m_implementedCallbacks.contains(getRequestSlot(request)))
            emit m_parent->finalResponseSent(request);
    }
}

/*
    Called when all request packets for an operation have been received.
*/
QObex::ResponseCode QObexServerSessionPrivate::receivedRequest(QObex::Request request, QObexHeader &requestHeader, const QByteArray &nonHeaderData, QObexHeader *responseHeader)
{
    qLog(Obex) << "QObexServerSession: got all packets for request" << request;

    // Invoke the appropriate subclass slot, unless it has already been called.
    // (for a multi-packet Put, the slot would have been called at
    // receivedRequestFirstPacket())
    QObex::ResponseCode nextResponse = ( m_invokedRequestSlot ?
            QObex::Success : processRequestHeader(request,
                    requestHeader, nonHeaderData) );

    if (m_closed)
        return QObex::InternalServerError;

    // Subclass may have set some response headers, either in the most recent
    // callback or in a previous call (e.g. in dataAvailable()).
    *responseHeader = m_nextResponseHeader;
    m_nextResponseHeader.clear();
    return nextResponse;
}

QObex::ResponseCode QObexServerSessionPrivate::bodyDataAvailable(const char *data, qint64 size)
{
    return m_parent->dataAvailable(data, size);
}

QObex::ResponseCode QObexServerSessionPrivate::bodyDataRequired(const char **data, qint64 *size)
{
    return m_parent->provideData(data, size);
}


//----------------------------------------------------------------

QObex::ResponseCode QObexServerSessionPrivate::processRequestHeader(QObex::Request request, QObexHeader &requestHeader, const QByteArray &nonHeaderData)
{
    QObex::ResponseCode response = QObex::Success;

    // Check for Authentication Response
    if (requestHeader.contains(QObexHeader::AuthResponse)) {
        // Emit authenticationResponse() with the received response.
        response = readAuthenticationResponse(requestHeader.value(
                QObexHeader::AuthResponse).toByteArray());
        if (response != QObex::Success)
            return response;

        // don't pass raw auth response onto client
        requestHeader.remove(QObexHeader::AuthResponse);
    } else {
        if (!m_challengeNonce.isEmpty()) {
            errorOccurred(QObexServerSession::AuthenticationFailed,
                    qApp->translate("QObexServerSession",
                                  "Did not receive authentication response"));
            return QObex::Unauthorized;
        }
    }

    // If headers have an Authentication Challenge, parse it and remove it
    // before calling the slot.
    QObexAuthenticationChallenge challenge;
    bool gotChallenge = false;
    if (requestHeader.contains(QObexHeader::AuthChallenge)) {
        gotChallenge = QObexAuthenticationChallengePrivate::parseRawChallenge(
                requestHeader.value(QObexHeader::AuthChallenge).toByteArray(),
                challenge);
        if (!gotChallenge) {
            errorOccurred(QObexServerSession::AuthenticationFailed,
                    qApp->translate("QObexServerSession", "Cannot read authentication challenge"));
            return QObex::BadRequest;
        }
        requestHeader.remove(QObexHeader::AuthChallenge);
    }

    bool invoked = false;
    QString callback = getRequestSlot(request);
    switch (request) {
        // the callbacks for these slots only take a single QObexHeader argument
        case QObex::Connect:
        case QObex::Disconnect:
        case QObex::Get:
        case QObex::Put:
        case QObex::PutDelete:
            invoked = invokeSlot(callback, &response,
                                        Q_ARG(QObexHeader, requestHeader));
            break;
        case QObex::SetPath:
            invoked = invokeSlot(callback, &response,
                    Q_ARG(QObexHeader, requestHeader),
                    Q_ARG(QObex::SetPathFlags, getSetPathFlags(nonHeaderData)));
            break;
        default:
            errorOccurred(QObexServerSession::InvalidRequest,
                    qApp->translate("QObexServerSession",
                                  "Received request of unknown type"));
    }

    // close() was called during invocation?
    if (m_closed)
        return QObex::InternalServerError;

    if (invoked) {
        if (gotChallenge && response == QObex::Success) {
            // Emit authenticationRequired() with the received challenge.
            response = processAuthenticationChallenge(challenge);
        }
    } else {
        response = QObex::InternalServerError;
    }

    m_invokedRequestSlot = true;
    return response;
}

QObex::ResponseCode QObexServerSessionPrivate::processAuthenticationChallenge(QObexAuthenticationChallenge &challenge)
{
    emit m_parent->authenticationRequired(&challenge);
    const QObexAuthenticationChallengePrivate *priv =
            QObexAuthenticationChallengePrivate::getPrivate(challenge);

    if (!priv->m_modified) {
        errorOccurred(QObexServerSession::AuthenticationFailed,
                qApp->translate("QObexServerSession",
                              "Server did not provide username or password for authentication"));
        // any headers that have been set should be rendered invalid, since
        // they are probably dependent on a successful operation
        m_nextResponseHeader.clear();
        return QObex::InternalServerError;
    }

    QByteArray bytes;
    if (!priv->toRawResponse(bytes)) {
        errorOccurred(QObexServerSession::AuthenticationFailed,
                qApp->translate("QObexServerSession",
                              "Error responding to authentication challenge"));
        // any headers that have been set should be rendered invalid, since
        // they are probably dependent on a successful operation
        m_nextResponseHeader.clear();
        return QObex::InternalServerError;
    }

    // add the Auth Response to the response headers
    m_nextResponseHeader.setValue(QObexHeader::AuthResponse, bytes);
    return QObex::Success;
}

QObex::ResponseCode QObexServerSessionPrivate::readAuthenticationResponse(const QByteArray &responseBytes)
{
    QObexAuthenticationResponse response =
            QObexAuthenticationResponsePrivate::createResponse(m_challengeNonce);
    if (!QObexAuthenticationResponsePrivate::parseRawResponse(
            responseBytes, response)) {
        errorOccurred(QObexServerSession::AuthenticationFailed,
                qApp->translate("QObexServerSession",
                              "Invalid client authentication response"));
        return QObex::BadRequest;
    }

    bool accept = false;
    emit m_parent->authenticationResponse(response, &accept);
    if (!accept) {
        errorOccurred(QObexServerSession::AuthenticationFailed,
                qApp->translate("QObexServerSession", "Authentication failed"));
        return QObex::Unauthorized;
    }
    return QObex::Success;
}

/*
    Invoke a user callback to notify about a request.
    Need to call the callbacks through invokeMethod() through Qt meta system
    instead of calling the callbacks directly as methods, because the slots
    are not virtual. (This is to be consistent within Qt/Qt Extended APIs.)

    (The request callbacks are slots because this allows us to reject an
    request when a REQHINT is received if we can see that the subclass doesn't
    implement the corresponding slot and thus is not interested in that request.)
 */
bool QObexServerSessionPrivate::invokeSlot(const QString &methodName, QObex::ResponseCode *responseCode, QGenericArgument arg1, QGenericArgument arg2)
{
    bool invoked;
    QObex::ResponseCode tempResponse;
    QPointer<QObexServerSessionPrivate> ptr(this);
    if (responseCode) {
        invoked = QMetaObject::invokeMethod(m_parent,
                        methodName.toAscii().constData(),
                        Q_RETURN_ARG(QObex::ResponseCode, tempResponse),
                        arg1, arg2);
    } else {
        invoked = QMetaObject::invokeMethod(m_parent,
                        methodName.toAscii().constData(),
                        arg1, arg2);
    }

    // self deleted during invocation?
    if (ptr.isNull())
        return false;

    if (m_closed)
        return false;

    if (invoked) {
        if (responseCode)
            *responseCode = tempResponse;
        return true;
    } else {
        errorOccurred(QObexServerSession::UnknownError,
                qApp->translate("QObexServerSession","Error invoking callback for %1").arg(methodName));
        return false;
    }
}

void QObexServerSessionPrivate::errorOccurred(QObexServerSession::Error error, const QString &errorString)
{
    if (m_closed) {
        qLog(Obex) << "QObexServerSession: closed, ignoring error:" << error
                << errorString;
        return;
    }

    m_parent->error(error, errorString);
}

void QObexServerSessionPrivate::socketDisconnected()
{
    if (m_socketDisconnected)
        return;

    qLog(Obex) << "QObexServerSession: my socket was disconnected!";    

    // Disconnect these signals so that we don't get any more calls
    // to socketDisconnected()
    if (m_socket) {
        QObject::disconnect(m_socket->device(), SIGNAL(aboutToClose()),
                            this, SLOT(socketDisconnected()));
        QObject::disconnect(m_socket->device(), SIGNAL(readChannelFinished()),
                            this, SLOT(socketDisconnected()));
        QObject::disconnect(m_socket->device(), SIGNAL(destroyed()),
                            this, SLOT(socketDisconnected()));
        QObject::disconnect(m_socket, SIGNAL(destroyed()),
                            this, SLOT(socketDisconnected()));
    }

    m_socketDisconnected = true;

    errorOccurred(QObexServerSession::ConnectionError, qApp->translate(
                "QObexServerSession", "Connection error"));
}

QObex::SetPathFlags QObexServerSessionPrivate::getSetPathFlags(const QByteArray &nonHeaderData)
{
    QObex::SetPathFlags flags = 0;

    if (nonHeaderData.size() < 2)
        return flags;

    // 1st byte has flags, 2nd byte has constants (not used)
    if (nonHeaderData.constData()[0] & 1)
        flags |= QObex::BackUpOneLevel;
    if (nonHeaderData.constData()[0] & 2)
        flags |= QObex::NoPathCreation;
    return flags;
}

/*
    Looks at which protected slots have been reimplemented by the subclass.
    Adds the name of each implemented slot to m_implementedCallbacks.
*/
void QObexServerSessionPrivate::initAvailableCallbacks()
{
    const QMetaObject superMetaObj = QObexServerSession::staticMetaObject;

    // methodOffset is the index where the superclass methods end and
    // this class's methods begin
    const QMetaObject *metaObj = m_parent->metaObject();
    int methodCount = metaObj->methodCount();
    for (int i=metaObj->methodOffset(); i<methodCount; i++) {
        const char *sig = metaObj->method(i).signature();

        if (superMetaObj.indexOfMethod(sig) != -1) {
            // superclass also has this method, therefore subclass is
            // overriding/re-implementing this method
            QString sigStr(sig);
            QString methodName = sigStr.mid(0, sigStr.indexOf("("));
            m_implementedCallbacks.insert(methodName);

            qLog(Obex) << "QObexServerSession: subclass implements calbacks:"
                << m_implementedCallbacks;
        }
    }
}

QLatin1String QObexServerSessionPrivate::getRequestSlot(QObex::Request request)
{
    switch (request) {
        case QObex::Connect:
            return QLatin1String("connect");
        case QObex::Disconnect:
            return QLatin1String("disconnect");
        case QObex::Put:
            return QLatin1String("put");
        case QObex::PutDelete:
            return QLatin1String("putDelete");
        case QObex::Get:
            return QLatin1String("get");
        case QObex::SetPath:
            return QLatin1String("setPath");
        default:
            return QLatin1String("");
    }
}


//======================================================================

/*!
    \class QObexServerSession
    \inpublicgroup QtBaseModule
    \brief The QObexServerSession class provides an abstract base class for implementing an OBEX server.

    To implement an OBEX server, subclass QObexServerSession and override
    the protected slots that correspond to the requests to be handled by your
    server, as shown below.

    QObexServerSession can accept OBEX client requests over any transport
    that is provided through a subclass of QIODevice. For example,
    QBluetoothRfcommSocket, QIrSocket and QTcpSocket are all subclasses of
    QIODevice, and objects of these subclasses can passed to the
    QObexServerSession constructor to run OBEX servers over RFCOMM, IrDA
    or TCP, respectively.


    \tableofcontents

    \section1 Receiving client requests

    In order to receive requests from an OBEX client, your QObexServerSession
    subclass must override the protected slots for the requests that it wants
    to receive. For example, if you want to receive \c Connect requests,
    you must override the corresponding connect() slot, which will be called
    each time a \c Connect request is received:

    \code
    class SimpleObexServer : public QObexServerSession
    {
        Q_OBJECT
    public:
        SimpleObexServer(QIODevice *device, QObject *parent = 0)
            : QObexServerSession(device, parent)
        {
        }

    protected slots:
        QObex::ResponseCode connect(const QObexHeader &header)
        {
            qDebug() << "Received a CONNECT request";

            // Return QObex::Success to allow the request, or return
            // some other response code to deny the request.
            return QObex::Success;
        }
    };
    \endcode

    Other slots are similarly named according to their request types. Here
    are the types of requests that can be received by a subclass of
    QObexServerSession, and the corresponding slots that must be overridden
    to receive them:

    \table
    \header
        \o Request type
        \o Protected slot
    \row
        \o \c Connect
        \o connect()
    \row
        \o \c Disconnect
        \o disconnect()
    \row
        \o \c Put
        \o put()
    \row
        \o \c Put-Delete
        \o putDelete()
    \row
        \o \c Get
        \o get()
    \row
        \o \c SetPath
        \o setPath()
    \endtable

    If a client sends a request but the server subclass has not overridden the slot for
    that particular type of request, the request will automatically be denied
    by returning a QObex::NotImplemented response to the client.

    There is no corresponding slot for \c Abort requests. When an \c Abort
    request is received, error() will be called with an error value of
    QObexServerSession::Aborted. \c Abort requests cannot be denied.


    \section1 Handling \c Put and \c Get requests

    When a \c Put request is received, the put() slot is called with the
    initial request headers. This allows you to refuse the request before
    any body data is received. Then, the virtual dataAvailable() function
    is called each time body data is received for the \c Put
    request. You can return a non-success response from dataAvaiable()
    to cancel the \c Put operaton. Here is an example subclass implementation:

    \code
    private:
        QFile *m_incomingFile;

    protected slots:
        QObex::ResponseCode put(const QObexHeader &header)
        {
            // Reject the PUT request if the requested file cannot be opened
            QString incomingFilename = header.filename();
            m_incomingFile = new QFile(incomingFilename);

            if (!m_incomingFile->open(QIODevice::WriteOnly)) {
                delete m_incomingFile;
                return QObex::InternalServerError;
            }

            return QObex::Success;
        }

    protected:
        QObex::ResponseCode dataAvailable(const char *data, qint64 size)
        {
            if (size > 0) {
                // Write the new data to file. If the data cannot be written, reject
                // the request.
                if (m_incomingFile->write(data, size) < 0) {
                    m_incomingFile->close();
                    return QObex::InternalServerError;
                }
            } else {
                qDebug() << "Received all PUT data";
                m_incomingFile->close();
            }

            return QObex::Success;
        }
    \endcode


    For \c Get requests, override the get() slot and override the
    provideData() function. The get() slot is called when the \c Get
    request is received, and provideData() is called each time
    the service is required to send more body data to the client. You can
    return a non-success response from provideData() to cancel the \c Get
    request.

    Note that when implementing provideData(), the subclass must ensure that
    the data remains valid until the next call to provideData(), or until
    the request is finished.

    Here is an example implementation:

    \code
    private:
        QFile *m_requestedFile;
        QByteArray m_lastSentBytes;

    protected slots:
        QObex::ResponseCode get(const QObexHeader &header)
        {
            QString requestedFilename = header.name();
            if (!QFile::exists(requestedFilename))
                return QObex::NotFound;

            m_requestedFile = new QFile(requestedFilename);
            if (!m_requestedFile->open(QIODevice::ReadOnly)) {
                delete m_requestedFile;
                return QObex::InternalServerError;
            }

            qDebug() << "Prepared for GET request for" << requestedFile;
            return QObex::Success;
        }

    protected:
        QObex::ResponseCode provideData(const char **data, qint64 *size)
        {
            // Read another data chunk from the file.
            // The read data is stored in m_lastSentBytes so that it remains
            // valid until the next call to provideData(), or until the
            // request is finished.
            m_lastSentBytes = m_requestedFile.read(1024);

            if (m_lastSentBytes.size() > 0) {
                *data = m_lastSentBytes.constData();
                *size = m_lastSentBytes.size();
                qDebug() << "Sending another" << size << "bytes";
            } else {
                *size = 0;
                m_requestedFile->close();
                m_lastSentBytes.clear();
                qDebug() << "Finished reading file";
            }

            return QObex::Success;
        }
    \endcode


    \section1 Handling errors and \c Abort requests

    If an error occurs while running the server, error() will be called
    with the corresponding error. The server should override this function
    to perform any clean-up actions that might be necessary.

    The error() function is also called when an OBEX client sends an
    \c Abort request to cancel a multi-packet request such as \c Put or
    \c Get. When an \c Abort request is received, the server will
    automatically respond with QObex::Success and call error() with
    the error argument set to QObexServerSession::Aborted. Therefore, if
    a server supports the \c Put or \c Get requests, it should override
    error() so that it can be notified of \c Abort requests and perform
    any clean-up actions that might be necessary.

    \ingroup qtopiaobex
    \sa QObexClientSession, {Simple OBEX Demo}
*/

/*!
    \enum QObexServerSession::Error
    \brief These are the errors that may be passed to QObexServerSession::error().

    \value ConnectionError The client is unable to send data, or the client-server communication process is otherwise disrupted. In this case, the client and server is no longer synchronized with each other, so the server session should be closed and the underlying OBEX socket should not be used any longer.
    \value InvalidRequest A client request was malformed or otherwise invalid.
    \value Aborted The client sent an \c Abort request.
    \value AuthenticationFailed The current request failed because the client could not be authenticated, or the server ignored an authentication request from the client.
    \value UnknownError An error other than those above has occurred.
*/


/*!
    Constructs an OBEX server session that uses \a device for the transport
    connection. The \a parent is the QObject parent.

    The \a device must be opened, or else the service will be unable
    to receive any client requests.
 */
QObexServerSession::QObexServerSession(QIODevice *device, QObject *parent)
    : QObject(parent),
      m_data(new QObexServerSessionPrivate(device, this))
{
}

/*!
    Destroys the server session.
 */
QObexServerSession::~QObexServerSession()
{
}

/*!
    Returns the device used by this server session, as provided in the
    constructor.
 */
QIODevice *QObexServerSession::sessionDevice()
{
    if (m_data->m_socket)
        return m_data->m_socket->device();
    return 0;
}

/*!
    Closes the service. The service will no longer receive client requests.
 */
void QObexServerSession::close()
{
    m_data->close();
}

/*!
    Sets the session to send \a header in the next server response for the
    current request.

    This might be useful, for example, to give the client some information about the file
    that is about to be sent back in response to a \c Get request:

    \code
    protected slots:
        QObex::ResponseCode get(const QObexHeader &header)
        {
            QString requestedFilename = header.name();
            QFile file(requestedFilename);
            if (file.open(QIODevice::ReadOnly)) {
                QObexHeader header;
                header.setLength(file.size());
                setNextResponseHeader(header);
            }

            return QObex::Success;
        }
    \endcode
*/
void QObexServerSession::setNextResponseHeader(const QObexHeader &header)
{
    m_data->setNextResponseHeader(header);
}


/*!
    Called each time the server receives body data for a \c Put request. The
    \a data is the received data and \a size is the data length. If \a size
    is 0, then all body data has been received.

    If your server supports \c Put requests, override this function to read
    and store the data. The data will not be buffered and made available
    elsewhere, so it must be read at this point. Return QObex::Success to
    allow the request to continue, or any other response code to stop the
    request.

    The default implementation returns QObex::InternalServerError.

    \sa put()
*/
QObex::ResponseCode QObexServerSession::dataAvailable(const char *, qint64)
{
    return QObex::InternalServerError;
}

/*!
    Called each time the server is required to send body data back to the
    client for a \c Get request.

    If your server supports \c Get requests, override this function and set
    \a data to the body data to be sent and \a size to the length of the
    data. If all body data has been sent, set \a size to zero. Return
    QObex::Success to allow the request to continue, or any other
    response code to stop the request.

    The default implementation returns QObex::InternalServerError.

    \bold {Note:} The subclass is responsible for ensuring that \a data
    remains valid until the next call to provideData(), or until all data has
    been sent.

    \sa get()
*/
QObex::ResponseCode QObexServerSession::provideData(const char **, qint64*)
{
    return QObex::InternalServerError;
}


/*!
    Called when an error occurs while running the server.
    The \a error is the error that has occurred, and \a errorString is a
    human-readable description of the error.

    Override this to provide custom error handling functionality. The default
    implementation prints the warning to the console, and also calls close()
    if the error is QObexServerSession::ConnectionError.
*/
void QObexServerSession::error(QObexServerSession::Error error, const QString &errorString)
{
    qWarning("QObexServerSession error: %s", errorString.toLatin1().constData());
    if (error == QObexServerSession::ConnectionError)
        close();
}

/*!
    Called when a \c Connect request is received with \a header.

    If your server supports \c Connect requests, override this slot and
    return QObex::Success to allow the request to continue. If you return 
    any other response code, the request will be denied. The default 
    implementation returns QObex::NotImplemented.
*/
QObex::ResponseCode QObexServerSession::connect(const QObexHeader &)
{
    return QObex::NotImplemented;
}

/*!
    Called when a \c Disconnect request is received with \a header.

    If your server supports \c Disconnect requests, override this slot and
    return QObex::Success to allow the request to continue. If you return 
    any other response code, the request will be denied. The default 
    implementation returns QObex::NotImplemented.

    \bold {Note:} Clients may terminate the transport connection without
    sending a \c Disconnect request, so you should not necessarily expect to
    receive this request.
*/
QObex::ResponseCode QObexServerSession::disconnect(const QObexHeader &)
{
    return QObex::NotImplemented;
}

/*!
    Called when a \c Put request is received with \a header.

    If your server supports \c Put requests, override this slot and
    return QObex::Success to allow the request to continue. If you return 
    any other response code, the request will be denied. The default 
    implementation returns QObex::NotImplemented.

    If you override this slot, you should also override dataAvailable()
    to read the body data that is received during the request, and also
    override error() so you can be notified if the client aborts the request.

    \sa dataAvailable(), error()
*/

QObex::ResponseCode QObexServerSession::put(const QObexHeader &)
{
    return QObex::NotImplemented;
}

/*!
    Called when a \c Put-Delete request is received with \a header.

    If your server supports \c Put-Delete requests, override this slot and
    return QObex::Success to allow the request to continue. If you return 
    any other response code, the request will be denied. The default 
    implementation returns QObex::NotImplemented.
*/
QObex::ResponseCode QObexServerSession::putDelete(const QObexHeader &)
{
    return QObex::NotImplemented;
}

/*!
    Called when a \c Get request is received with \a header.

    If your server supports \c Get requests, override this slot and
    return QObex::Success to allow the request to continue. If you return 
    any other response code, the request will be denied. The default 
    implementation returns QObex::NotImplemented.

    If you override this slot, you should also override provideData() to
    provide the body data that will be sent back to the client, and also
    override error() so you can be notified if the client aborts the request.

    \sa provideData(), error()
*/
QObex::ResponseCode QObexServerSession::get(const QObexHeader &)
{
    return QObex::NotImplemented;
}

/*!
    Called when a \c SetPath request is received with \a header and \a flags.

    If your server supports \c SetPath requests, override this slot and
    return QObex::Success to allow the request to continue. If you return 
    any other response code, the request will be denied. The default 
    implementation returns QObex::NotImplemented.
*/
QObex::ResponseCode QObexServerSession::setPath(const QObexHeader &, QObex::SetPathFlags)
{
    return QObex::NotImplemented;
}

/*!
    \fn void QObexServerSession::finalResponseSent(QObex::Request request)

    This signal is emitted when the server has sent the final response for 
    the specified \a request.

    Note this signal not emitted if the request was aborted, or if the
    server responded with QObex::NotImplemented because the corresponding
    request slot was not implemented by this server session.
*/

/*!
    \fn void QObexServerSession::authenticationRequired(QObexAuthenticationChallenge *challenge)

    This signal is emitted when a client requires the server to authenticate
    itself before proceeding with the current request.

    The \a challenge provides the details of the authentication challenge sent
    by the client. The \a challenge object can then be filled in with the
    username and password that should be sent back to the client in order to
    authenticate this server.

    The server will not be notified if the client rejects the authentication
    details provided in \a challenge; in this case, it is likely that the
    client will simply disconnect from the server.

    This signal is not emitted if the protected slot for the current request
    has denied the request by returning a response code other than
    QObex::Success.

    \bold {Note:} It is not possible to use a QueuedConnection to connect to
    this signal, as the server will not send back a username and password
    (and so authentication will fail) if the \a challenge has not been filled 
    in with new information when the signal returns.
*/

/*!
    \fn void QObexServerSession::authenticationResponse(const QObexAuthenticationResponse &response, bool *accept)

    This signal is emitted if the server has previously issued an authentication
    challenge to indicate that the client must authenticate itself before
    proceeding with the current request, and the client has now responded with
    an authentication \a response containing a username and password for
    authentication.

    Set \a accept to \c true if the authentication details in \a response are
    correct. If \a accept is set to \c true, the corresponding slot for the
    current request will be called after this signal is emitted. Otherwise,
    the request will automatically be denied with a QObex::Unauthorized
    response, and error() will be called with QObexServerSession::AuthenticationFailed.

    You can issue an authentication challenge to a client by responding to a
    request with QObex::Unauthorized and a QObexHeader object that includes an
    authentication challenge (by calling QObexHeader::setAuthenticationChallenge()).

    \bold {Note:} It is not possible to use a QueuedConnection to connect to
    this signal, as \a accept will automatically be set to \c false if its
    value has not been set when the signal returns.
*/
