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
#include "qobexclientsession.h"
#include "qobexclientsession_p.h"
#include <qobexheader.h>
#include "qobexcommand_p.h"
#include "qobexheader_p.h"
#include "qobexauthenticationchallenge_p.h"
#include "qobexauthenticationresponse_p.h"

#include <qtopialog.h>

#include <QTimer>
#include <QQueue>
#include <QBuffer>
#include <QApplication>

#define OBEX_STREAM_BUF_SIZE 4096


QObexClientSessionPrivate::QObexClientSessionPrivate(QIODevice *device, QObexClientSession *parent)
    : QObject(parent),
      m_socket(new QObexSocket(device, this)),
      m_parent(parent),
      m_socketDisconnected(false),
      m_busyWithRequest(false)
{
    m_lastResponseCode = QObex::Success;
    m_error = QObexClientSession::NoError;
    m_errorString = qApp->translate("QObexClientSession", "Unknown error");

    m_connId = 0;
    m_haveConnId = false;

    m_doneSocketSetup = false;

    m_buf = 0;
    m_aborting = false;

    m_emittedReqFinishedId = -1;
}

QObexClientSessionPrivate::~QObexClientSessionPrivate()
{
    if (m_socket)
        m_socket->setObexClient(0);

    if (!m_cmdQueue.empty()) {
        cleanUpDataTransfer();
        clearAllRequests();
    }
}

int QObexClientSessionPrivate::connect(const QObexHeader &header)
{
    return queueCommand(new QObexCommand(QObex::Connect, header));
}

int QObexClientSessionPrivate::disconnect(const QObexHeader &header)
{
    return queueCommand(new QObexCommand(QObex::Disconnect, header));
}

int QObexClientSessionPrivate::put(const QObexHeader &header, QIODevice *dev)
{
    return queueCommand(new QObexCommand(QObex::Put, header, dev));
}

int QObexClientSessionPrivate::put(const QObexHeader &header, const QByteArray &data)
{
    return queueCommand(new QObexCommand(QObex::Put, header, data));
}

int QObexClientSessionPrivate::putDelete(const QObexHeader &header)
{
    QObexCommand *cmd = new QObexCommand(QObex::PutDelete, header);
    return queueCommand(cmd);
}

int QObexClientSessionPrivate::get(const QObexHeader &header, QIODevice *dev)
{
    return queueCommand(new QObexCommand(QObex::Get, header, dev));
}

int QObexClientSessionPrivate::setPath(const QObexHeader &header, QObex::SetPathFlags flags)
{
    QObexCommand *cmd = new QObexCommand(QObex::SetPath, header);
    cmd->m_setPathFlags = flags;
    return queueCommand(cmd);
}

/*
    Sends an Abort request. All pending requests will be deleted, since the
    operation will be finished with an error of QObexClientSession::Aborted.
    Does nothing if not connected, or there are no operations in progress.
*/
void QObexClientSessionPrivate::abort()
{
    if (m_cmdQueue.empty() || m_aborting)
        return;

    m_aborting = true;

    // Can't send Abort request straight away, it will be done in
    // performAbort(). We need to wait our turn to send a request (i.e. when
    // we get OBEX_EV_PROGRESS, since that means we got the next server
    // response) otherwise we're breaking the obex request-response model.
}

int QObexClientSessionPrivate::currentId() const
{
    if (m_cmdQueue.empty())
        return 0;

    return m_cmdQueue.head()->m_id;
}

QObex::Request QObexClientSessionPrivate::currentRequest() const
{
    if (m_cmdQueue.empty())
        return QObex::NoRequest;

    return m_cmdQueue.head()->m_req;
}

QIODevice *QObexClientSessionPrivate::currentDevice() const
{
    if (m_cmdQueue.empty())
        return 0;

    QObexCommand *cmd = m_cmdQueue.head();
    if (cmd->m_req != QObex::Put && cmd->m_req != QObex::Get)
        return 0;

    if (cmd->m_isba)
        return 0;

    return cmd->m_data.device;
}

void QObexClientSessionPrivate::clearPendingRequests()
{
    // leave the first item alone - do not delete the current request!
    while (m_cmdQueue.size() > 1)
        delete m_cmdQueue.takeLast();
}

bool QObexClientSessionPrivate::hasPendingRequests() const
{
    // > 1 because op at head of queue is current op, so it's not
    // considered pending/scheduled
    return m_cmdQueue.size() > 1;
}

qint64 QObexClientSessionPrivate::read(char *data, qint64 maxlen)
{
    if (m_readSoFar == m_recvBuffer.size())
        return 0;

    const char *stored = m_recvBuffer.data().constData();
    int bytesToRead = qMin<qint64>(maxlen, bytesAvailable());
    memcpy(data, stored + m_readSoFar, bytesToRead);
    m_readSoFar += bytesToRead;

    return bytesToRead;
}

QByteArray QObexClientSessionPrivate::readAll()
{
    qint64 avail = bytesAvailable();
    QByteArray tmp;
    tmp.resize(int(avail));
    qint64 got = read(tmp.data(), int(avail));
    tmp.resize(got);
    return tmp;
}

qint64 QObexClientSessionPrivate::bytesAvailable() const
{
    return m_recvBuffer.size() - m_readSoFar;
}

//------------------------------------------------------------------


int QObexClientSessionPrivate::queueCommand(QObexCommand *cmd)
{
    m_cmdQueue.enqueue(cmd);
    if (m_cmdQueue.size() == 1)
        QTimer::singleShot(0, this, SLOT(doPending()));

    return cmd->m_id;
}

void QObexClientSessionPrivate::doPending()
{
    if (m_cmdQueue.empty()) {
        finishedAllOps();
        return;
    }

    if (m_busyWithRequest)
        return;

    m_busyWithRequest = true;

    // reset for next command
    cleanUpDataTransfer();
    m_error = QObexClientSession::NoError;
    m_lastResponseCode = QObex::Success;
    m_lastResponseHeader.clear();
    m_challengeNonce.clear();
    m_nextAuthResponseBytes.clear();
    m_sentAuthResponse = false;
    m_receivedAuthResponse = false;
    m_emittedReqFinishedId = -1;
    m_errorString = qApp->translate("QObexClientSession", "Unknown error");

    QObexCommand *cmd = m_cmdQueue.head();
    emit m_parent->requestStarted(cmd->m_id);
    performCommand(cmd);
}

void QObexClientSessionPrivate::performCommand(QObexCommand *cmd)
{
    qLog(Obex) << "QObexClientSession: preparing to send request, type ="
        << cmd->m_req;

    if (m_aborting) {
        // "abort" the current command by not sending it at all
        finishedAbort(true);
        return;
    }

    if ( !m_socket || !m_socket->isValid() ||
                !m_socket->device() || !m_socket->device()->isOpen() ) {
        finishedCurrentOp(QObexClientSession::ConnectionError,
            qApp->translate("QObexClientSession", "Not connected to server"));
        return;
    }

    if (!m_doneSocketSetup) {
        m_socket->setObexClient(this);
        if (m_socket->device()) {
            QObject::connect(m_socket->device(), SIGNAL(aboutToClose()),
                             this, SLOT(socketDisconnected()));
            QObject::connect(m_socket->device(), SIGNAL(readChannelFinished()),
                             this, SLOT(socketDisconnected()));
            QObject::connect(m_socket->device(), SIGNAL(destroyed()),
                             this, SLOT(socketDisconnected()));
            QObject::connect(m_socket, SIGNAL(destroyed()),
                             this, SLOT(socketDisconnected()));
        }
    }

    // add connection id (as long as header doesn't already have a Target)
    if ( (cmd->m_req != QObex::Connect) && (m_haveConnId) &&
            (!cmd->m_header.contains(QObexHeader::Target)) ) {
        cmd->m_header.setConnectionId(m_connId);
    }

    // if there's a challenge, note the nonce so we can authentication the
    // server later
    if (cmd->m_header.contains(QObexHeader::AuthChallenge)) {
        m_challengeNonce = QObexHeaderPrivate::getChallengeNonce(cmd->m_header);
    }

    if ( (cmd->m_req == QObex::Put && !preparePut(cmd)) ||
            (cmd->m_req == QObex::Get && !prepareGet(cmd)) ) {
        return;
    }

    bool result;
    if (cmd->m_req == QObex::SetPath) {
        const char nonHeaderData[2] = { int(cmd->m_setPathFlags), 0 };
        result = m_socket->sendRequest(cmd->m_req, cmd->m_header, nonHeaderData, 2);
    } else {
        result = m_socket->sendRequest(cmd->m_req, cmd->m_header);
    }

    if (result) {
        qLog(Obex) << "QObexClientSession: sending new request, type ="
            << cmd->m_req << ", number of headers =" << cmd->m_header.size();
    }

    if (!result) {
        finishedCurrentOp(QObexClientSession::UnknownError,
            qApp->translate("QObexClientSession", "Error starting OBEX request"));
    }
}

bool QObexClientSessionPrivate::preparePut(QObexCommand *cmd)
{
    if (m_buf)
        delete[] m_buf;

    m_buf = 0;
    m_currBytes = 0;
    m_lastEmittedDone = 0;

    if (!cmd->m_isba && !cmd->m_data.device) {
        finishedCurrentOp(QObexClientSession::InvalidRequest,
                qApp->translate("QObexClientSession", "No source data provided"));
        return false;
    }

    // prepare if source is QIODevice
    if (!cmd->m_isba && cmd->m_data.device) {
        // open QIODevice if necessary
        if (!cmd->m_data.device->isOpen() &&
                    !cmd->m_data.device->open(QIODevice::ReadOnly)) {
            qWarning("QObexClientSession: Cannot open QIODevice for OBEX Put request");
            finishedCurrentOp(QObexClientSession::UnknownError,
                    qApp->translate("QObexClientSession", "Cannot open source I/O device"));
            return false;
        }

        // Create the buffer (only needed in QIODevice case)
        m_buf = new char[OBEX_STREAM_BUF_SIZE];
    }

    // get total length
    if (cmd->m_header.contains(QObexHeader::Length)) {
        m_totalBytes = cmd->m_header.length();
    } else {
        if (cmd->m_isba) 
            m_totalBytes = cmd->m_data.data->size();
        else
            m_totalBytes = cmd->m_data.device->size();
    }

    return true;
}

/*
    Called when more body data should be provided for the next Put request
    packet
 */
void QObexClientSessionPrivate::bodyDataRequired(const char **data, qint64 *size)
{
    const char *streambuf = NULL;
    int bytesToWrite = 0;

    if (m_cmdQueue.isEmpty())
        return;
    QObexCommand *cmd = m_cmdQueue.head();

    // Handle Byte Array case
    if (cmd->m_isba) {
        streambuf = &cmd->m_data.data->data()[m_currBytes];
        bytesToWrite = qMin((m_totalBytes - m_currBytes), OBEX_STREAM_BUF_SIZE);
    } else if (cmd->m_data.device) {
        streambuf = m_buf;
        bytesToWrite = cmd->m_data.device->read(m_buf, OBEX_STREAM_BUF_SIZE);
        if (bytesToWrite < 0) {
            // Signify end of request
            qWarning("QObexClientSession: could not read QIODevice for PUT request");
            *data = 0;
            *size = 0;
            return;
        }
    }

    m_currBytes += bytesToWrite;

    if ( bytesToWrite > 0 ) {
        *data = streambuf;
        *size = bytesToWrite;
    } else {
        /* EOF */
        *data = 0;
        *size = 0;

        // dataTransferProgress() is emitted at OBEX_EV_PROGRESS; if we get to
        // here and it hasn't been emitted, means there's only 1 packet (since
        // user has signalled EOF before OBEX_EV_PROGRESS), so emit it here.
        if (m_lastEmittedDone == 0 && m_currBytes > 0) {
            emit m_parent->dataTransferProgress( m_currBytes, m_totalBytes );
            m_lastEmittedDone = m_currBytes;
        }
    }
}

bool QObexClientSessionPrivate::prepareGet(QObexCommand *cmd)
{
    if (cmd->m_data.device) {
        if (!cmd->m_data.device->isOpen() &&
             !cmd->m_data.device->open(QIODevice::WriteOnly)) {
            finishedCurrentOp(QObexClientSession::UnknownError,
                    qApp->translate("QObexClientSession", "Cannot open I/O device for received data"));
            return false;
        }
    } else {
        if (!m_recvBuffer.isOpen() && !m_recvBuffer.open(QBuffer::ReadWrite)) {
            finishedCurrentOp(QObexClientSession::UnknownError,
                    qApp->translate("QObexClientSession", "Cannot open data buffer for received data"));
            return false;
        }
    }

    m_totalBytes = 0;
    m_currBytes = 0;
    m_readSoFar = 0;

    return true;
}

/*
    Called when data is available for a Get request.
 */
void QObexClientSessionPrivate::bodyDataAvailable(const char *data, qint64 size)
{
    if (size == 0)   // reached end of stream
        return;

    if (m_cmdQueue.isEmpty())
        return;
    QObexCommand *cmd = m_cmdQueue.head();

    if (cmd->m_data.device) {
        // write to the QIODevice
        if ( (cmd->m_data.device->write(data, size)) < 0 ) {
            qWarning("QObexClientSession: could not write to QIODevice for GET request");
        } else {
            m_currBytes += size;
            emit m_parent->dataTransferProgress(m_currBytes, m_totalBytes);
        }
    } else {
        // write to internal buffer, signal readyRead()
        if ( (m_recvBuffer.write(data, size)) < 0 ) {
            qWarning("QObexClientSession: could not write to buffer for GET request");
        } else {
            m_currBytes += size;
            emit m_parent->dataTransferProgress(m_currBytes, m_totalBytes);
            emit m_parent->readyRead();
        }
    }
}

void QObexClientSessionPrivate::requestDone(QObex::ResponseCode response)
{
    // An error may have been set when processing any authentication headers.
    // The m_lastResponseCode shouldn't be set in this case, since it
    // might be dependent on a successful request.
    if (m_error != QObexClientSession::NoError) {
        finishedCurrentOp(m_error, m_errorString);
        return;
    }

    if (response == QObex::Success) {
        // Did the client challenge and the server didn't send an auth response?
        if (!m_challengeNonce.isEmpty() && !m_receivedAuthResponse) {
            finishedCurrentOp(QObexClientSession::AuthenticationFailed,
                    qApp->translate("QObexClientSession", "Did not receive authentication response"));
            return;
        }
    } else {
        // If need to send an Auth Response in response to a Challenge, resend
        // the request with an added Auth Response and don't emit
        // requestFinished() (i.e. pretend the request hasn't finished yet).
        // Have to do the resend at the end of the event loop, otherwise
        // openobex thinks the current request hasn't finished yet.
        if (!m_nextAuthResponseBytes.isEmpty()) {
            QTimer::singleShot(0, this, SLOT(resendRequestWithAuthResponse()));
            return;
        }
    }

    // note the response for this, the most recently completed request
    m_lastResponseCode = response;

    // error for this op
    QObexClientSession::Error opResult = QObexClientSession::NoError;
    QString errorString = QLatin1String("");
    if (m_lastResponseCode == QObex::Unauthorized && m_sentAuthResponse) {
        opResult = QObexClientSession::AuthenticationFailed;
        errorString = qApp->translate("QObexClientSession", "Authentication failed");
    } else if (m_lastResponseCode != QObex::Success) {
        opResult = QObexClientSession::RequestFailed;
        errorString = qApp->translate("QObexClientSession", "Request failed");
    }

    finishedCurrentOp(opResult, errorString);
}

void QObexClientSessionPrivate::requestResponseHeaderReceived(QObexHeader &header)
{
    if ( (currentRequest() == QObex::Connect) &&
            header.contains(QObexHeader::ConnectionId) ) {
        m_connId = header.connectionId();
        m_haveConnId = true;

    } else if (currentRequest() == QObex::Get && m_totalBytes == 0 &&
            header.contains(QObexHeader::Length)) {
        // see if server indicated the total file length
        m_totalBytes = header.length();
    }

    if (header.size() > 0) {
        if (header.contains(QObexHeader::AuthChallenge)) {
            readAuthenticationChallenge(header.value(
                    QObexHeader::AuthChallenge).toByteArray());
            // If there's a challenge, there shouldn't be any other headers
            // to process or pass onto the user.
            // An Auth Response will be sent in requestDone().
            return;
        }

        if (header.contains(QObexHeader::AuthResponse)) {
            m_receivedAuthResponse = true;
            if ( !readAuthenticationResponse(header.value(
                    QObexHeader::AuthResponse).toByteArray()) ) {
                // Don't set m_lastResponseHeader if authentication failed.
                return;
            }

            // Don't pass raw auth response onto user.
            m_lastResponseHeader = header;
            m_lastResponseHeader.remove(QObexHeader::AuthResponse);
        } else {
            m_lastResponseHeader = header;
        }
        emit m_parent->responseHeaderReceived(m_lastResponseHeader);
    }
}

void QObexClientSessionPrivate::requestProgressed()
{
    if (currentRequest() == QObex::Put && m_lastEmittedDone != m_currBytes) {
        emit m_parent->dataTransferProgress(m_currBytes, m_totalBytes);
        m_lastEmittedDone = m_currBytes;
    }

    // If user called abort(), send an Abort request here.
    // Must call performAbort() at end of event loop. Otherwise openobex
    // obex_client changes to STATE_SEND immediately after this event
    // and then gets confused by getting server's Abort response
    // during the STATE_SEND. This ensures obex_client gets to STATE_REC.
    if (m_aborting)
        QTimer::singleShot(0, this, SLOT(performAbort()));
}

void QObexClientSessionPrivate::requestAborted()
{
    finishedAbort(true);
}

void QObexClientSessionPrivate::errorOccurred(QObexClientSession::Error error, const QString &msg)
{
    if (currentRequest() == QObex::NoRequest) {
        qLog(Obex) << "QObexClientSession: ignoring error" << error
                << ", no request in progress";
        return;
    }

    switch (error) {
        case QObexClientSession::ConnectionError:
            if (m_aborting)    // get linkerr if got non-success from abort
                finishedAbort(false);
            else
                finishedCurrentOp(QObexClientSession::ConnectionError, msg);
            break;
        default:
            finishedCurrentOp(error, msg);
    }
}

void QObexClientSessionPrivate::performAbort()
{
    if (m_cmdQueue.isEmpty())
        return;

    qLog(Obex) << "QObexClientSession: sending Abort";
    if (!m_socket->abortCurrentRequest())
        finishedAbort(false);
}

void QObexClientSessionPrivate::finishedAbort(bool success)
{
    qLog(Obex) << "QObexClientSession: finished Abort, successful?" << success;

    m_aborting = false;

    // The current request should finish with an error, and all
    // pending requests and the current request will be cleared.
    if (success) {
        finishedCurrentOp(QObexClientSession::Aborted,
                qApp->translate("QObexClientSession", "Request aborted"));
    } else {
        // If abort fails, we are out of sync with the server.
        finishedCurrentOp(QObexClientSession::ConnectionError,
                qApp->translate("QObexClientSession", "Error aborting request"));
    }
}

void QObexClientSessionPrivate::finishedCurrentOp(QObexClientSession::Error error, const QString &msg)
{
    qLog(Obex) << "QObexClientSession: finished request, type =" 
        << currentRequest() << ", error =" << error << "errormsg =" << msg;

    if (m_aborting) {
        // This is not an error - see abort() docs.
        qLog(Obex) << "QObexClientSession: request finished before it could be aborted";
    }

    if (m_cmdQueue.empty())
        return;

    // set last error
    m_error = error;
    if (!msg.isEmpty())
        m_errorString = msg;

    // reset flags
    m_aborting = false;

    if (currentRequest() == QObex::Disconnect) {
        m_connId = 0;
        m_haveConnId = false;
    }

    // emit requestFinished() before dequeue-ing command
    QObexCommand *cmd = m_cmdQueue.head();

    m_emittedReqFinishedId = cmd->m_id;
    QPointer<QObexClientSession> that = m_parent;
    emit m_parent->requestFinished(cmd->m_id,
            m_error != QObexClientSession::NoError);

    if (!that)
        return;

    if (!m_cmdQueue.isEmpty())
        delete m_cmdQueue.dequeue();

    m_busyWithRequest = false;

    // last op failed, don't do any more ops
    if (m_error != QObexClientSession::NoError)
        clearAllRequests();

    // Take care of the case where we get a command
    // scheduled as a result of the requestFinished
    // being emitted
    QTimer::singleShot(0, this, SLOT(doPending()));
}

void QObexClientSessionPrivate::finishedAllOps()
{
    emit m_parent->done(m_error != QObexClientSession::NoError);
}

//----------------------------------------------------------------

bool QObexClientSessionPrivate::readAuthenticationResponse(const QByteArray &responseBytes)
{
    QObexAuthenticationResponse response =
            QObexAuthenticationResponsePrivate::createResponse(m_challengeNonce);
    if (!QObexAuthenticationResponsePrivate::parseRawResponse(
            responseBytes, response)) {
        m_error = QObexClientSession::AuthenticationFailed;
        m_errorString = qApp->translate( "QObexClientSession", "Invalid server authentication response");
        return false;
    }

    bool accept = false;
    emit m_parent->authenticationResponse(response, &accept);
    if (!accept) {
        m_error = QObexClientSession::AuthenticationFailed;
        m_errorString = qApp->translate("QObexClientSession", "Authentication failed");
        return false;
    }

    return true;
}

bool QObexClientSessionPrivate::readAuthenticationChallenge(const QByteArray &challengeBytes)
{
    QObexAuthenticationChallenge challenge;
    bool result = QObexAuthenticationChallengePrivate::parseRawChallenge(
            challengeBytes, challenge);
    if (!result) {
        m_error = QObexClientSession::AuthenticationFailed;
        m_errorString = qApp->translate("QObexClientSession",
                "Invalid server authentication challenge");
        return false;
    }

    emit m_parent->authenticationRequired(&challenge);
    const QObexAuthenticationChallengePrivate *priv =
            QObexAuthenticationChallengePrivate::getPrivate(challenge);
    if (!priv->m_modified) {
        m_error = QObexClientSession::AuthenticationFailed;
        m_errorString = qApp->translate("QObexClientSession","Client did not provide username or password for authentication");
        return false;
    }

    m_nextAuthResponseBytes.clear();
    if (!priv->toRawResponse(m_nextAuthResponseBytes)) {
        m_nextAuthResponseBytes.clear();
        m_error = QObexClientSession::AuthenticationFailed;
        m_errorString = qApp->translate("QObexClientSession",
                "Error responding to authentication challenge");
        return false;
    }

    return true;
}

void QObexClientSessionPrivate::resendRequestWithAuthResponse()
{
    if (m_cmdQueue.size() > 0) {
        QObexCommand *cmd = m_cmdQueue.head();
        cmd->m_header.remove(QObexHeader::AuthResponse); // in case added one before
        cmd->m_header.setValue(QObexHeader::AuthResponse, m_nextAuthResponseBytes);
        performCommand(cmd);

        m_sentAuthResponse = true;
    }

    // must reset this, is checked in requestDone()
    m_nextAuthResponseBytes.clear();
}

//-----------------------------------------------------------------

void QObexClientSessionPrivate::socketDisconnected()
{
    if (m_socketDisconnected)
        return;

    qLog(Obex) << "QObexClientSession: my socket was disconnected!";

    // don't react to any more socket disconnections
    if (m_socket) {
        QObject::disconnect(m_socket, SIGNAL(destroyed()),
                            this, SLOT(socketDisconnected()));
        if (m_socket->device()) {
            QObject::disconnect(m_socket->device(), SIGNAL(aboutToClose()),
                                this, SLOT(socketDisconnected()));
            QObject::disconnect(m_socket->device(), SIGNAL(readChannelFinished()),
                                this, SLOT(socketDisconnected()));
            QObject::disconnect(m_socket->device(), SIGNAL(destroyed()),
                                this, SLOT(socketDisconnected()));
        }
    }

    m_socketDisconnected = true;

    // check m_emittedReqFinishedId in case the socket disconnection
    // is triggered from a requestFinished() signal, in which case
    // you don't want to emit requestFinished() twice for the same
    // request
    if (currentRequest() != QObex::NoRequest && m_emittedReqFinishedId != currentId()) {
        finishedCurrentOp(QObexClientSession::ConnectionError,
                qApp->translate("QObexClientSession",
                              "Connection error"));
    }
}

void QObexClientSessionPrivate::cleanUpDataTransfer()
{
    // get - reset buffer
    m_recvBuffer.close();
    m_recvBuffer.setData(NULL, 0);

    // put - clear send buffer
    if (m_buf) {
        delete[] m_buf;
        m_buf = 0;
    }
}

void QObexClientSessionPrivate::clearAllRequests()
{
    for (int i=0; i<m_cmdQueue.size(); i++)
        delete m_cmdQueue[i];
    m_cmdQueue.clear();
}

//=======================================================================

/*!
    \class QObexClientSession
    \inpublicgroup QtBaseModule

    \brief The QObexClientSession class provides an implementation of the client side of the OBEX protocol.

    A QObexClientSession can be used over any type of transport that is
    accessible through a subclass of QIODevice. For example,
    QBluetoothRfcommSocket, QIrSocket and QTcpSocket are all subclasses of
    QIODevice, and objects of these subclasses can passed to the
    QObexClientSession constructor to run an OBEX client over RFCOMM,
    IrDA or TCP, respectively.

    QObexClientSession performs OBEX requests asynchronously and depends on
    the presence of a running event loop.


    \tableofcontents

    \section1 Executing OBEX client requests

    The QObexClientSession class can be used to execute the standard OBEX requests:
    \c Connect, \c Disconnect, \c Put, \c Put-Delete, \c Get and \c SetPath.
    These requests are available through the connect(), disconnect(), put(),
    putDelete(), get() and setPath() functions, respectively.

    All requests are performed asynchronously. These functions do not block;
    instead, they schedule requests for later execution, and return
    immediately. Each function returns a unique identifier for the scheduled
    request that can be used to track the request's progress by connecting
    to signals of interest such as requestStarted() and requestFinished().
    The currentId() function can also be used to determine the request that
    is currently executed.

    The use of scheduled requests allows the execution of a sequence of
    commands. For example, this code will connect to an OBEX server
    running on a particular Bluetooth device, download a file, and then
    disconnect:

    \code
    QBluetoothRfcommSocket *rfcommSocket = new QBluetoothRfcommSocket;
    rfcommSocket->connect("11:22:33:aa:bb:cc", 10);

    QObexClientSession *client = new QObexClientSession(rfcommSocket);
    client->connect();

    QObexHeader header;
    header.setName("SomeFile.txt");
    client->get(header);
    client->disconnect();
    \endcode

    When the last scheduled request has finished, a done() signal is
    emitted with a \c bool argument that indicates whether the sequence
    of requests finished with an error.

    As an example, the code example above will produce a sequence of signals
    similar to this:

    \code
    requestStarted(1)
    responseHeaderReceived(responseHeader)
    requestFinished(1, false)

    requestStarted(2)
    responseHeaderReceived(responseHeader)
    dataTransferProgress(962, 2415)
    readyRead()
    dataTransferProgress(1980, 2415)
    readyRead()
    dataTransferProgress(2415, 2415)
    readyRead()
    requestFinished(2, false)

    requestStarted(3)
    responseHeaderReceived(responseHeader)
    requestFinished(3, false)

    done(false)
    \endcode

    The readyRead() signal tells you that there is data ready to be
    read following a \c Get request. The amount of data can then be queried
    with the bytesAvailable() function and it can be read with the read()
    or readAll() functions.

    If an error occurs during the execution of one of the commands in
    a sequence of commands, all the pending commands (i.e. scheduled,
    but not yet executed commands) are cleared and no signals are
    emitted for them.

    For example, if the \c Get request in the above example code fails
    because the server responded with a response code other than QObex::Success,
    the \c Disconnect request would not be executed, and the sequence of
    signals would look like this instead:

    \code
    requestStarted(1)
    responseHeaderReceived(responseHeader)
    requestFinished(1, false)

    requestStarted(2)
    requestFinished(2, true)

    done(true)
    \endcode

    You can then get details about the error with the error() function.


    \section1 Multiplexing and Connection IDs

    The \c {Connection Id} header is used when multiplexing OBEX connections
    over a single transport connection. If an OBEX server includes a
    \c {Connection Id} header in a response to a \c Connect request, the ID
    will be retained and the client will automatically include it in the
    OBEX headers for all future requests. You do not need to track and send
    the Connection Id yourself. The ID can be retrieved using connectionId().

    Note that the Connection Id will not be automatically added if a request
    already contains a \c Target header, as a request cannot have both of
    these headers at the same time. (If it did, the server would be unsure as
    to whether the client was establishing a new directed connection or using
    an existing connection.)

    \sa QObexServerSession, {Simple OBEX Demo}
    \ingroup qtopiaobex
 */

/*!
    \enum QObexClientSession::Error
    \brief The errors that may occur for an OBEX client.

    \value NoError No error has occurred.
    \value ConnectionError The client is unable to send data, or the client-server communication process is otherwise disrupted. In this case, the client and server are no longer synchronized with each other, so the QIODevice provided in the constructor should not be used for any more OBEX requests.
    \value RequestFailed The request was refused by the server (i.e. the server responded with a response code other than QObex::Success).
    \value InvalidRequest The client request is invalid.
    \value InvalidResponse The server sent an invalid or unreadable response.
    \value Aborted The request was aborted by a call to abort().
    \value AuthenticationFailed The request failed because the client or server could not be authenticated.
    \value UnknownError An error other than those specified above occurred.
*/


/*!
    Constructs an OBEX client session that uses \a device for the transport
    connection. The \a parent is the QObject parent.

    The \a device must be opened in order to perform client requests.
    Otherwise, requests will fail with the QObexClientSession::ConnectionError
    error.
 */
QObexClientSession::QObexClientSession(QIODevice *device, QObject *parent)
    : QObject(parent),
      m_data(new QObexClientSessionPrivate(device, this))
{
}

/*!
    Destroys the client.
 */
QObexClientSession::~QObexClientSession()
{
    delete m_data;
}

/*!
    Returns the device used for this client session, as provided in the
    constructor.
 */
QIODevice *QObexClientSession::sessionDevice() const
{
    return m_data->m_socket->device();
}

/*!
    Initiates the OBEX session by sending a \c Connect request with
    the given \a header.

    The function does not block and returns immediately. The request
    is scheduled, and executed asynchronously.

    This function returns a unique identifier for the request. This
    identifier is passed by the requestStarted() signal when the request
    starts, and by the requestFinished() signal when the request is
    finished.

    \sa done()
 */
int QObexClientSession::connect(const QObexHeader &header)
{
    return m_data->connect(header);
}

/*!
    Signals the end of the OBEX session by sending a \c Disconnect request with
    the given \a header.

    The function does not block and returns immediately. The request
    is scheduled, and executed asynchronously.

    This function returns a unique identifier for the request. This
    identifier is passed by the requestStarted() signal when the request
    starts, and by the requestFinished() signal when the request is
    finished.

    \sa done()
 */
int QObexClientSession::disconnect(const QObexHeader &header)
{
    return m_data->disconnect(header);
}

/*!
    Sends a data object to the server through a \c Put request with the data
    from \a dev and the given \a header. The data is read in chunks
    from the QIODevice object, so this allows you to transmit large chunks
    of data without the need to read all the data into memory at once.

    Make sure that the \a dev pointer is valid for the duration of the
    request; it is safe to delete it when the requestFinished() signal
    is emitted.

    The progress of the data transfer is reported via the
    dataTransferProgress() signal.

    The function does not block and returns immediately. The request
    is scheduled, and executed asynchronously.

    This function returns a unique identifier for the request. This
    identifier is passed by the requestStarted() signal when the request
    starts, and by the requestFinished() signal when the request is
    finished.

    \sa dataTransferProgress(), putDelete(), done()
 */
int QObexClientSession::put(const QObexHeader &header, QIODevice *dev)
{
    return m_data->put(header, dev);
}

/*!
    \overload

    Sends a data object to the server through a \c Put request with a copy of
    the data from \a data and the given \a header.
 */
int QObexClientSession::put(const QObexHeader &header, const QByteArray &data)
{
    return m_data->put(header, data);
}

/*!
    Deletes a file on the server through a \c Put-Delete request with
    the given \a header.

    The function does not block and returns immediately. The request
    is scheduled, and executed asynchronously.

    This function returns a unique identifier for the request. This
    identifier is passed by the requestStarted() signal when the request
    starts, and by the requestFinished() signal when the request is
    finished.

    \sa done()
 */
int QObexClientSession::putDelete(const QObexHeader &header)
{
    return m_data->putDelete(header);
}

/*!
    Retrieves a data object through a \c Get request with the given
    \a header.

    If \a dev is not 0, the received data is written to \a dev.
    Make sure that the \a dev pointer is valid for the duration of the
    request; it is safe to delete it when the requestFinished() signal
    is emitted.

    If \a dev is 0, the readyRead() signal is emitted when there is data
    available to be read. You can then read the data with read() or readAll().

    The function does not block and returns immediately. The request
    is scheduled, and executed asynchronously.

    This function returns a unique identifier for the request. This
    identifier is passed by the requestStarted() signal when the request
    starts, and by the requestFinished() signal when the request is
    finished.

    \sa done()
 */
int QObexClientSession::get(const QObexHeader &header, QIODevice *dev)
{
    return m_data->get(header, dev);
}

/*!
    Sets the remote path on the server through a \c SetPath reqest with
    the specified \a flags and the given \a header.

    The function does not block and returns immediately. The request
    is scheduled, and executed asynchronously.

    This function returns a unique identifier for the request. This
    identifier is passed by the requestStarted() signal when the request
    starts, and by the requestFinished() signal when the request is
    finished.

    \sa done()
 */
int QObexClientSession::setPath(const QObexHeader &header, QObex::SetPathFlags flags)
{
    return m_data->setPath(header, flags);
}

/*!
    Aborts the current request and deletes all scheduled requests.

    If there is an unfinished request, the client will send an \c Abort
    request to the server. Once the server replies to the request, the
    requestFinished() signal will be emitted with the \c error argument set
    to \c true, and the error() function will return QObexClientSession::Aborted
    if the server accepted the \c Abort request. If the request was refused,
    error() will return QObexClientSession::ConnectionError and the client should
    disconnect as it is no longer synchronized with the server.

    Due to timing issues, the client may not be able to send the \c Abort
    immediately. If the request finishes before it can be aborted, the
    request will be completed normally and the requestFinished() \c error
    argument will be \c false.

    If no other requests are started after the call to abort(), there will be
    no scheduled requests and the done() signal will be emitted.
 */
void QObexClientSession::abort()
{
    m_data->abort();
}

/*!
    Returns the identifier of the request that is being executed, or 0 if
    there is no request being executed.

    \sa currentRequest()
 */
int QObexClientSession::currentId() const
{
    return m_data->currentId();
}

/*!
    Returns the request that is being executed, or QObex::NoRequest if
    there is no request being executed.

    \sa currentId()
*/
QObex::Request QObexClientSession::currentRequest() const
{
    return m_data->currentRequest();
}

/*!
    Returns the QIODevice pointer that is used as the data source or data
    target of the request currently in progress. Returns 0 if the current
    request does not use an IO device, or if there is no request in
    progress.

    This function can be used to delete the QIODevice in a slot connected to
    the requestFinished() signal.

    \sa put(), get()
 */
QIODevice *QObexClientSession::currentDevice() const
{
    return m_data->currentDevice();
}

/*!
    Deletes all pending requests from the list of scheduled requests. This
    does not affect the request that is being executed. If you want to stop
    this request as well, use abort().

    \sa hasPendingRequests(), currentId()
 */
void QObexClientSession::clearPendingRequests()
{
    m_data->clearPendingRequests();
}

/*!
    Returns true if there are any requests scheduled that have not yet been
    executed; otherwise returns false.

    The request that is being executed is \i not considered as a scheduled
    request.

    \sa clearPendingRequests(), currentId()
 */
bool QObexClientSession::hasPendingRequests() const
{
    return m_data->hasPendingRequests();
}

/*!
    Reads \a maxlen bytes from the response content into \a data and
    returns the number of bytes read. Returns -1 if an error occurred.

    \sa readAll(), bytesAvailable(), readyRead(), get()
 */
qint64 QObexClientSession::read(char *data, qint64 maxlen)
{
    return m_data->read(data, maxlen);
}

/*!
    Reads all the bytes available from the data buffer and returns
    them.

    \sa read(), readyRead(), get()
 */
QByteArray QObexClientSession::readAll()
{
    return m_data->readAll();
}

/*!
    Returns the number of bytes that can be read from read() or readAll()
    at the moment.

    \sa read(), readyRead(), readAll(), get()
 */
qint64 QObexClientSession::bytesAvailable() const
{
    return m_data->bytesAvailable();
}

/*!
    Returns the server response code for the most recently completed request.

    This value is reset to QObex::Success when a new request is started.
 */
QObex::ResponseCode QObexClientSession::lastResponseCode() const
{
    return m_data->m_lastResponseCode;
}

/*!
    Returns the last response headers received from the server.

    This value is reset when a new request is started.

    \sa responseHeaderReceived()
 */
QObexHeader QObexClientSession::lastResponseHeader() const
{
    return m_data->m_lastResponseHeader;
}

/*!
    Returns the client's Connection Id, or 0 if it does not have a
    connection Id.

    The Connection Id is used for directed OBEX connections. If the client
    sends a \c Connect request with a \c Target header, the OBEX server will 
    include a \c {Connection Id} header in its response. In this case, the
    client will automatically send this Connection Id value in all future
    requests; you do not have to include the \c {Connection Id} header
    yourself.

    The Connection Id will be reset following a \c Disconnect request.

    \sa hasConnectionId()
*/
quint32 QObexClientSession::connectionId() const
{
    return m_data->m_connId;
}

/*!
    Returns whether the client has a Connection Id.

    \sa connectionId()
*/
bool QObexClientSession::hasConnectionId() const
{
    return m_data->m_haveConnId;
}

/*!
    Returns the last error that occurred. This is useful for finding out
    what happened when receiving an requestFinished() or done()
    signal that has the \c error argument set to \c true.

    If you start a new request, the error status is reset to
    QObexClientSession::NoError.

    \sa errorString()
 */
QObexClientSession::Error QObexClientSession::error() const
{
    return m_data->m_error;
}

/*!
    Returns a human-readable description of the last error that occurred.

    The error string is reset when a new request is started.

    \sa error()
 */
QString QObexClientSession::errorString() const
{
    return m_data->m_errorString;
}

/*!
    \fn void QObexClientSession::requestFinished(int id, bool error)

    This signal is emitted when the client has finished processing the request
    identified by \a id. The \a error value is \c true if an error occurred
    during the processing of the request; otherwise \a error is \c false.

    \bold {Note:} \a error is set to \c true if the server responded with a
    response code other than QObex::Success. In this case, error() will return
    QObexClientSession::RequestFailed, and lastResponseCode() will return
    the response code sent by the server.

    \warning Do not delete a QObexClientSession instance while it is emitting
    this signal. If you need to delete it, call QObject::deleteLater() instead 
    of using the \c delete keyword. (If you do this, you may have to store the
    instance as a QPointer if you need to check the validity of the pointer 
    later on.)
    
    \sa requestStarted(), currentId(), currentRequest()
*/

/*!
    \fn void QObexClientSession::requestStarted(int id)

    This signal is emitted when the client has started processing the request
    identified by \a id.

    \sa requestFinished(), currentId(), currentRequest()
*/

/*!
    \fn void QObexClientSession::responseHeaderReceived(const QObexHeader &header)

    This signal is emitted when a response header is received.  The \a header
    contains the header data that was received.
 */

/*!
    \fn void QObexClientSession::readyRead()

    This signal is emitted in response to a get() request when there is
    new data to read.

    If you specify a device as the second argument in the get() request, this
    signal is \i not emitted; instead, the data is written directly to
    the device.

    You can then read the data with the read() or readAll() functions.

    This signal is useful if you want to process the data in chunks as soon as
    it becomes available. If you are only interested in the complete data,
    just connect to the requestFinished() signal and read the data then
    instead.
 */

/*!
    \fn void QObexClientSession::dataTransferProgress(qint64 done, qint64 total)

    This signal is emitted during file transfer requests to
    indicate the progress of the transfer. The \a done value is the
    number of bytes that have been sent or received so far, and \a total
    is the total number of bytes to be sent or received.
 */

/*!
    \fn void QObexClientSession::done(bool error)

    This signal is emitted when all pending requests have finished; it is
    emitted after the requestFinished() signal for the last request. The
    \a error value is \c true if an error occurred during the processing
    of the request; otherwise \a error is \c false.

    \warning Do not delete a QObexClientSession instance while it is emitting
    this signal. If you need to delete it, call QObject::deleteLater() instead
    of using the \c delete keyword. (If you do this, you may have to store the
    instance as a QPointer if you need to check the validity of the pointer
    later on.)
 */

/*!
    \fn void QObexClientSession::authenticationRequired(QObexAuthenticationChallenge *challenge)

    This signal is emitted when the server requires the client to authenticate
    itself before proceeding with the current request.

    The \a challenge provides the details of the authentication challenge sent
    by the server. The \a challenge object can then be filled in with the
    username and password that should be sent back to the server in order to
    authenticate this client.

    If the server rejects the authentication details provided in \a challenge,
    the requestFinished() signal will be emitted with the \c error argument
    set to \c true, and the error() function will return
    QObexClientSession::AuthenticationFailed.

    \bold {Note:} It is not possible to use a QueuedConnection to connect to
    this signal, as the request will fail if the \a challenge has not
    been filled in with new information when the signal returns.
*/

/*!
    \fn void QObexClientSession::authenticationResponse(const QObexAuthenticationResponse &response, bool *accept)

    This signal is emitted when the client has previously issued an authentication
    challenge to indicate that the server must authenticate itself before
    proceeding with the current request, and the server has now responded with
    an authentication \a response containing a username and password for
    authentication.

    Set \a accept to \c true if the authentication details in \a response are
    correct. If \a accept is set to \c true, the request will continue.
    Otherwise, the requestFinished() signal will be emitted with
    the \c error argument set to \c true, and the error() function will return
    QObexClientSession::AuthenticationFailed.

    To issue an authentication challenge to the server, send a request with a 
    QObexHeader object that includes an authentication challenge (by calling 
    QObexHeader::setAuthenticationChallenge()).

    \bold {Note:} It is not possible to use a QueuedConnection to connect to
    this signal, as \a accept will automatically be set to \c false if its
    value has not been set when the signal returns.
*/

#include "qobexclientsession.moc"
