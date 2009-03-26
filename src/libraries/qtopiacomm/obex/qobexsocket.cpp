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
#include "qobexsocket_p.h"
#include <qobexnamespace.h>
#include <qobexheader.h>
#include "qobexheader_p.h"
#include "qobexclientsession_p.h"
#include "qobexserversession_p.h"

#ifdef QTOPIA_INFRARED
#include <qirsocket.h>
#endif

#ifdef QTOPIA_BLUETOOTH
#include <qbluetoothrfcommsocket.h>
#endif

#include <QIODevice>
#include <QApplication>

#include <openobex/obex.h>


static int qobexsocket_getCmd(QObex::Request request)
{
    switch (request) {
        case QObex::Connect:
            return OBEX_CMD_CONNECT;
        case QObex::Disconnect:
            return OBEX_CMD_DISCONNECT;
        case QObex::Put:
        case QObex::PutDelete:
            return OBEX_CMD_PUT;
        case QObex::Get:
            return OBEX_CMD_GET;
        case QObex::SetPath:
            return OBEX_CMD_SETPATH;
        default:
            return int(request);
    }
}

struct q_custom_context
{
    QIODevice *device;
};

static int _q_cust_obex_connect(obex_t *, void *)
{
    return 0;
}

static int _q_cust_obex_disconnect(obex_t *, void *)
{
    return 0;
}

static int _q_cust_obex_listen(obex_t *, void *)
{
    return 0;
}

static int _q_cust_obex_write(obex_t *, void * customdata, uint8_t *buf, int buflen)
{
    q_custom_context *context = reinterpret_cast<q_custom_context *>(customdata);
    if (!context->device)
        return -1;

    int len = context->device->write(reinterpret_cast<char *>(buf), buflen);

    if (len < 0) {
        qWarning("QObexSocket: error writing to socket");
        return -1;
    }

    return len;
}

static int _q_cust_obex_handleinput(obex_t *handle, void * customdata, int /*secs*/)
{
    q_custom_context *context = reinterpret_cast<q_custom_context *>(customdata);
    if (!context->device)
        return -1;

    char buf[1024];
    qint64 len = context->device->read(buf, sizeof(buf));

    if (len <= 0)
        return len;

    qint64 numFed = 0;
    int r = 0;

    while (numFed < len) {
        if (!context->device || !context->device->isOpen())
            return -1;

        r = OBEX_CustomDataFeed(handle,
                                reinterpret_cast<uint8_t *>(&buf[numFed]),
                                len - numFed);
        if (r < 0) {
            qWarning("QObexSocket: error processing data");
            return -1;
        }
        numFed += r;
    }

    if (numFed != len) {
        qWarning("QObexSocket: error reading from socket");
    }

    return len;
}

static void openobex_callback(obex_t *handle, obex_object_t *obj, int mode,
                              int event, int obex_cmd, int obex_rsp)
{
    QObexSocket *conn =
            static_cast<QObexSocket *>(OBEX_GetUserData(handle));
    conn->connectionEvent(obj, mode, event, obex_cmd, obex_rsp);
}

//==============================================================


QObexSocket::QObexSocket(QIODevice *device, QObject *parent)
    : QObject(parent),
      m_device(0),
      m_handle(0),
      m_cleanedUp(false),
      m_client(0),
      m_server(0)
{
    if (!device) {
        qWarning("OBEX: invalid QIODevice");
        return;
    }

    obex_t *self = OBEX_Init(OBEX_TRANS_CUSTOM, openobex_callback, OBEX_FL_KEEPSERVER);
    if (!self) {
        qWarning("OBEX: cannot initiate OBEX socket transport");
        return;
    }

    OBEX_SetUserData(self, this);

    // tweak the MTU for common transports
#ifdef QTOPIA_INFRARED
    if (qobject_cast<QIrSocket *>(device))
        OBEX_SetTransportMTU(self, OBEX_IRDA_OPT_MTU, OBEX_IRDA_OPT_MTU);
#endif

#ifdef QTOPIA_BLUETOOTH
    if (qobject_cast<QBluetoothRfcommSocket *>(device))
        OBEX_SetTransportMTU(self, OBEX_MAXIMUM_MTU, OBEX_MAXIMUM_MTU);
#endif

    obex_ctrans_t cust;
    q_custom_context *context = new q_custom_context;
    context->device = device;

    cust.connect = _q_cust_obex_connect;
    cust.disconnect = _q_cust_obex_disconnect;
    cust.write = _q_cust_obex_write;
    cust.handleinput = _q_cust_obex_handleinput;
    cust.listen = _q_cust_obex_listen;
    cust.customdata = context;

    if(OBEX_RegisterCTransport(self, &cust) < 0) {
        OBEX_FreeInterfaces(self);
        return;
    }

    m_device = device;
    m_handle = self;

    QObject::connect(device, SIGNAL(readyRead()), this, SLOT(processInput()));

    if (device->bytesAvailable() > 0)
        processInput();

    // This socket shouldn't be valid if the QIODevice becomes closed.
    QObject::connect(device, SIGNAL(aboutToClose()), this, SLOT(prepareToClose()));
}

QObexSocket::~QObexSocket()
{
    prepareToClose();

    if (m_handle) {
        OBEX_Cleanup(m_handle);
        m_handle = 0;
    }
}

QIODevice *QObexSocket::device() const
{
    return m_device;
}

void QObexSocket::setObexClient(QObexClientSessionPrivate *client)
{
    m_client = client;
}

void QObexSocket::setObexServer(QObexServerSessionPrivate *server)
{
    m_server = server;
}

bool QObexSocket::sendRequest(QObex::Request request, const QObexHeader &header, const char *nonHeaderData, int nonHeaderDataSize)
{
    if (!isValid())
        return false;

    obex_object_t *obj = OBEX_ObjectNew(m_handle, qobexsocket_getCmd(request));
    if (!obj)
        return false;

    if (!QObexHeaderPrivate::writeOpenObexHeaders(m_handle, obj,
            QObexHeaderPrivate::requestShouldFitOnePacket(request), header)) {
        return false;
    }

    if (nonHeaderData && nonHeaderDataSize > 0) {
        if (OBEX_ObjectSetNonHdrData(obj, reinterpret_cast<const uint8_t*>(nonHeaderData),
                nonHeaderDataSize) < 0) {
            return false;
        }
    }

    if (request == QObex::Put) {
        obex_headerdata_t hv;
        hv.bs = 0;
        if (OBEX_ObjectAddHeader(m_handle, obj, OBEX_HDR_BODY, hv, 0,
                            OBEX_FL_STREAM_START) < 0) {
            return false;
        }
    } else if (request == QObex::Get) {
        if (OBEX_ObjectReadStream(m_handle, obj, 0) < 0) {
            return false;
        }
    }

    if (OBEX_Request(m_handle, obj) < 0)
        return false;

    return true;
}

bool QObexSocket::abortCurrentRequest()
{
    if (!isValid())
        return false;
    return (OBEX_CancelRequest(m_handle, 1) >= 0);
}

//--------------------------------------------------------------------

/*
    Called by OpenOBEX when an event has occurred.
 */
void QObexSocket::connectionEvent(obex_object_t *obj, int mode, int event, int obex_cmd, int obex_rsp)
{
    /*
    A server receiving a Put request with body data in the first packet
    will get events in this order:
        OBEX_EV_REQHINT, OBEX_EV_STREAMAVAIL, OBEX_EV_REQCHECK,
        OBEX_EV_STREAMAVAIL, ..., OBEX_EV_REQ, OBEX_EV_REQDONE.

    A server receiving a Put request with no body data in the first packet
    (as the Sony Ericsson P800 seems to do even if there is room for the body
    in the first packet) will get events in this order:
        OBEX_EV_REQHINT, OBEX_EV_REQCHECK, OBEX_EV_STREAMAVAIL, ...,
        OBEX_EV_REQ, OBEX_EV_REQDONE.

    A server receiving a Get request will get events in this order:
        OBEX_EV_REQHINT, OBEX_EV_REQ, OBEX_EV_STREAMEMPTY, ..., OBEX_EV_REQDONE.

    The QObexSocket will differentiate between a Put and a Put-Delete so that
    the QObexServerSession doesn't have to. It delays notifying the server
    session of an OBEX_CMD_PUT until it knows whether it's a Put or a
    Put-Delete. That is, when body data is received, it must be a Put, so
    notifies the server session of the Put request; otherwise, it waits for
    the OBEX_EV_REQ (when the whole packet is received) at which point if it
    hasn't received body data, it must be a Put-Delete, and the socket
    notifies the server session of the Put-Delete then.

    The OBEX_EV_REQCHECK is ignored because if the server hasn't received any
    body data at that point (i.e. there's no body data in the first packet)
    it has no idea whether it's a Put or Put-Delete anyway. And if there is
    body data in the first packet, the OBEX_EV_STREAMAVAIL event is received
    before the OBEX_EV_REQCHECK, so the body data would have to be buffered
    if the server was to be notified of the request before it is notified
    of the body data.

    A QObexServerSession is able to set response headers when the full
    request has been received, i.e. at OBEX_EV_REQ, and also when the first
    packet is received if it is receiving a Put request.
    */

    /*
    This has to check m_serverRefusedRequest when doing callbacks to a
    QObexServerSession because we'll continue to get events for a request even
    if a non-success response has been set, if there is only one packet in the
    request.
    */

    if (m_cleanedUp)
        return;

    switch (event) {
    case OBEX_EV_PROGRESS:
        // (A middle packet in a multi-packet op has been sent; won't
        // get this event for final or 1st-and-final packet.)
        if (mode == OBEX_CLIENT && m_client) {
            m_client->requestProgressed();
        }
        break;
    case OBEX_EV_REQHINT:
        // (Received first request packet, haven't parsed headers yet.)
        if (mode == OBEX_SERVER) {
            handleReqHint(obex_cmd, obj);
        }
        break;
    case OBEX_EV_REQ:
        // (Received and parsed final request packet.)
        if (mode == OBEX_SERVER) {
            handleReq(obex_cmd, obj);
        }
        break;
    case OBEX_EV_REQDONE:
        // (Request is finished - client received the final server response,
        // or server has sent the final response.)
        if (mode == OBEX_CLIENT && m_client) {
            QObexHeader header;
            if (!QObexHeaderPrivate::readOpenObexHeaders(header, m_handle, obj)) {
                m_client->errorOccurred(QObexClientSession::UnknownError,
                        qApp->translate("QObexClientSession", "Unknown error"));
                return;
            }
            if (header.size() > 0)
                m_client->requestResponseHeaderReceived(header);
            m_client->requestDone(static_cast<QObex::ResponseCode>(obex_rsp));
        } else if (mode == OBEX_SERVER && m_server) {
            m_server->requestDone(getCurrentRequest(obex_cmd));
        }
        break;
    case OBEX_EV_LINKERR:
        // (Error sending or receiving at transport level, or client and
        // server have somehow become out-of-sync with each other.)
        if (mode == OBEX_CLIENT && m_client) {
            m_client->errorOccurred(QObexClientSession::ConnectionError,
                    qApp->translate("QObexClientSession",
                                  "Connection error"));
        } else if (mode == OBEX_SERVER && m_server) {
            m_server->errorOccurred(QObexServerSession::ConnectionError,
                    qApp->translate("QObexServerSession",
                                  "Connection error"));
        }
        break;
    case OBEX_EV_PARSEERR:
        // (Received bad packet.)
        if (mode == OBEX_CLIENT && m_client) {
            m_client->errorOccurred(QObexClientSession::InvalidResponse,
                    qApp->translate("QObexClientSession",
                                  "Invalid server response"));
        } else if (mode == OBEX_SERVER && m_server) {
            m_server->errorOccurred(QObexServerSession::InvalidRequest,
                    qApp->translate("QObexServerSession",
                                  "Invalid client request"));
        }
        break;
    case OBEX_EV_ABORT:
        // (Client received a Success response for an Abort request, or
        // server received an Abort request.)
        if (mode == OBEX_CLIENT && m_client) {
            m_client->requestAborted();
        } else if (mode == OBEX_SERVER && m_server) {
            m_server->errorOccurred(QObexServerSession::Aborted,
                    qApp->translate("QObexServerSession",
                                "Request aborted"));
        }
        break;
    case OBEX_EV_STREAMEMPTY:
        // (Client needs to feed more data for a Put request, or server
        // needs to feed more data for a Get request.)
        handleStreamEmpty(mode, obj);
        break;
    case OBEX_EV_STREAMAVAIL:
        // (Client needs to read data received from a Get request, or
        // server needs to read data received from a Put request.)
        handleStreamAvailable(mode, obj);
        break;
    default:
        // includes OBEX_EV_UNEXPECTED and OBEX_EV_REQCHECK
        break;
    }
}

void QObexSocket::handleReqHint(int obex_cmd, obex_object_t *obj)
{
    if (!m_server)
        return;

    // reset state for new request
    m_serverRefusedRequest = false;
    m_checkedServerAcceptsRequest = false;
    m_gotPutBodyData = false;

    // Notify server of incoming request.
    if (obex_cmd == OBEX_CMD_PUT) {
        // Don't notify server of request until it's certain whether
        // request is a Put or Put-Delete. Just accept the request and
        // tell openobex to stream the Put body data.
        setNextResponse(obj, QObex::Success);
        OBEX_ObjectReadStream(m_handle, obj, NULL);

    } else {
        // Notify server of incoming request.
        serverAcceptsRequest(obj, getCurrentRequest(obex_cmd));
    }
}

void QObexSocket::handleReq(int obex_cmd, obex_object_t *obj)
{
    if (!m_server || m_serverRefusedRequest)
        return;

    QObex::Request request = getCurrentRequest(obex_cmd);

    // Server won't be notified of Put-Delete requests until this
    // point, so see if the request should be allowed.
    if (!m_checkedServerAcceptsRequest && !serverAcceptsRequest(obj, request))
        return;

    // Notify server that request has finished.
    QObexHeader requestHeader;
    if (!QObexHeaderPrivate::readOpenObexHeaders(requestHeader, m_handle, obj)) {
        m_server->errorOccurred(QObexServerSession::UnknownError,
                qApp->translate("QObexClientSession", "Unknown error"));
        setNextResponse(obj, QObex::InternalServerError);
        return;
    }
    QObexHeader responseHeader;
    QByteArray bytes;

    QObex::ResponseCode response =
            m_server->receivedRequest(request,
                requestHeader, readNonHeaderData(obj, bytes), &responseHeader);

    if (!isValid())
         return;

    // Write the response headers.
    if (!QObexHeaderPrivate::writeOpenObexHeaders(m_handle, obj,
            (obex_cmd != QObex::Get), responseHeader))  {
        m_server->errorOccurred(QObexServerSession::UnknownError,
                qApp->translate("QObexClientSession", "Unknown error"));
        setNextResponse(obj, QObex::InternalServerError);
        return;
    }

    setNextResponse(obj, response);

    // Now start streaming the body in response to a Get.
    if (response == QObex::Success && obex_cmd == OBEX_CMD_GET) {
        // now we've received all the client Get request packets, tell
        // openobex to stream the data for the Get
        obex_headerdata_t hv;
        OBEX_ObjectAddHeader(m_handle, obj, OBEX_HDR_BODY, hv, 0,
                    OBEX_FL_STREAM_START);
    }
}

void QObexSocket::handleStreamAvailable(int mode, obex_object_t *obj)
{
    if (mode == OBEX_CLIENT && m_client) {
        // Pass on last received headers e.g. for initial Get response headers
        QObexHeader header;
        if (!QObexHeaderPrivate::readOpenObexHeaders(header, m_handle, obj)) {
            m_client->errorOccurred(QObexClientSession::UnknownError,
                    qApp->translate("QObexClientSession", "Unknown error"));
            return;
        }
        if (header.size() > 0) {
            m_client->requestResponseHeaderReceived(header);
        }

        const uint8_t* buf;
        int bufSize = OBEX_ObjectReadStream(m_handle, obj, &buf);
        if (bufSize < 0) {
            qWarning("Unable to read body data for OBEX Get request");
            return;
        }

        m_client->bodyDataAvailable(reinterpret_cast<const char*>(buf), bufSize);

    } else if (mode == OBEX_SERVER && m_server && !m_serverRefusedRequest) {
        m_gotPutBodyData = true;

        if (!m_checkedServerAcceptsRequest) {
            // Notify the server of the Put request.
            if (!serverAcceptsRequest(obj, QObex::Put))
                return;

            // Pass on the Put request headers (should be before the body
            // headers in the packet).
            QObexHeader header;
            if (!QObexHeaderPrivate::readOpenObexHeaders(header, m_handle, obj)) {
                m_server->errorOccurred(QObexServerSession::UnknownError,
                        qApp->translate("QObexClientSession", "Unknown error"));
                setNextResponse(obj, QObex::InternalServerError);
                return;
            }

            QPointer<QObexSocket> ptr(this);

            QObexHeader responseHeader;
            QObex::ResponseCode response =
                    m_server->receivedRequestFirstPacket(QObex::Put, header, &responseHeader);

            if (ptr.isNull() || !isValid())
                return;

            // Write the response headers.
            if (!QObexHeaderPrivate::writeOpenObexHeaders(m_handle, obj,
                   true, responseHeader)) {
                m_server->errorOccurred(QObexServerSession::UnknownError,
                        qApp->translate("QObexClientSession", "Unknown error"));
                setNextResponse(obj, QObex::InternalServerError);
                return;
            }
            if (response != QObex::Success) {
                setNextResponse(obj, response);
                return;
            }
        }

        const uint8_t* buf;
        int bufSize = OBEX_ObjectReadStream(m_handle, obj, &buf);
        if (bufSize < 0) {
            qWarning("Unable to read body data for OBEX Put request");
            return;
        }

        setNextResponse(obj, m_server->bodyDataAvailable(
                reinterpret_cast<const char*>(buf), bufSize));
    }
}

void QObexSocket::handleStreamEmpty(int mode, obex_object_t *obj)
{
    const char *buf = NULL;
    qint64 bufSize = 0;

    if (mode == OBEX_CLIENT && m_client) {
        m_client->bodyDataRequired(&buf, &bufSize);

    } else if (mode == OBEX_SERVER && m_server) {
        if (!m_serverRefusedRequest) {
            QObex::ResponseCode response = m_server->bodyDataRequired(&buf, &bufSize);
            if (response != QObex::Success) {
                setNextResponse(obj, response);
            }
        }
        // If the request was rejected, continue on anyway so that the
        // OBEX_FL_STREAM_DATAEND is set, otherwise openobex is confused if
        // the streaming terminates early.
    }

    if (!isValid())   // if closed during callback
        return;

    // add the header with the body data
    obex_headerdata_t hv;
    int flags;
    if (bufSize > 0) {
        hv.bs = reinterpret_cast<const uint8_t *>(buf);
        flags = OBEX_FL_STREAM_DATA;
    } else {
        flags = OBEX_FL_STREAM_DATAEND;
    }

    OBEX_ObjectAddHeader(m_handle, obj, OBEX_HDR_BODY, hv, bufSize, flags);
}

/*
    Ask the obex server session whether it accepts a request.
*/
bool QObexSocket::serverAcceptsRequest(obex_object_t *obj, QObex::Request request)
{
    m_checkedServerAcceptsRequest = true;

    if (!m_server) {
        setNextResponse(obj, QObex::InternalServerError);
        return false;
}

    QObex::ResponseCode response = m_server->acceptIncomingRequest(request);
    if (response != QObex::Success) {
        setNextResponse(obj, response);
        return false;
    }
    return true;
}

/*
    This should only be called when the whole packet has been received
    and we know for sure whether it's a Put or Put-Delete.
*/
QObex::Request QObexSocket::getCurrentRequest(int obex_cmd)
{
    switch (obex_cmd) {
        case OBEX_CMD_CONNECT:
            return QObex::Connect;
        case OBEX_CMD_DISCONNECT:
            return QObex::Disconnect;
        case OBEX_CMD_GET:
            return QObex::Get;
        case OBEX_CMD_PUT:
            return (m_gotPutBodyData ? QObex::Put : QObex::PutDelete);
        case OBEX_CMD_SETPATH:
            return QObex::SetPath;
        default:
            return QObex::NoRequest;
    }
}

QByteArray &QObexSocket::readNonHeaderData(obex_object_t *obj, QByteArray &bytes)
{
    uint8_t *nonHeaderData;
    int len = OBEX_ObjectGetNonHdrData(obj, &nonHeaderData);
    if (len < 0)
        return bytes;
    bytes.resize(len);
    memcpy(bytes.data(), nonHeaderData, len);
    return bytes;
}

void QObexSocket::setNextResponse(obex_object_t *obj, QObex::ResponseCode response)
{
    if (!isValid())
        return;

    if (response == QObex::Success || response == OBEX_RSP_CONTINUE) {
        OBEX_ObjectSetRsp(obj, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
    } else {
        m_serverRefusedRequest = true;
        OBEX_ObjectSetRsp(obj, int(response), int(response));
    }
}

void QObexSocket::processInput()
{
    // Need while loop because of how custom transport is implemented -
    // otherwise it will only process the first part.
    while (OBEX_HandleInput(m_handle, 0) > 0);
}

void QObexSocket::prepareToClose()
{
    if (m_cleanedUp)
        return;

    setObexClient(0);
    setObexServer(0);

    if (m_device) {
        QObexSocket *s = qobject_cast<QObexSocket *>(parent());
        if (s) {
            QObject::disconnect(m_device, SIGNAL(readyRead()),
                                this, SIGNAL(processInput()));
        }
    }

    if (m_handle) {
        // set the custom data object to 0 so that _q_cust_obex_handleinput()
        // won't try to do anything
        q_custom_context *context =
                reinterpret_cast<q_custom_context *>(OBEX_GetCustomData(m_handle));
        if (context)
            context->device = 0;
    }

    m_cleanedUp = true;
}

QLatin1String QObexSocket::responseToString(QObex::ResponseCode response)
{
    return QLatin1String(OBEX_ResponseToString(int(response)));
}

/*
    Used from unit tests.
*/
void QObexSocket::resetObexHandle()
{
    OBEX_SetUserCallBack(m_handle, openobex_callback, 0);
    OBEX_SetUserData(m_handle, this);
}
