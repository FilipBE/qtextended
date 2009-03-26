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

//QTEST_SKIP_TEST_DOC

#include "openobexheaderprocessor.h"
#include <QTcpServer>
#include <QTcpSocket>

#include "qobexsocket_p.h"
#include <qtopialog.h>

#include <QList>
#include <QTest>
#include <shared/qtopiaunittest.h>

static const int OBEX_HI_MASK = 0xc0;
static const int OBEX_INT = 0xc0;
static const int OBEX_BYTE = 0x80;
static const int OBEX_BYTE_STREAM = 0x40;
static const int OBEX_UNICODE = 0x00;


//===============================================================

/*?
    The OpenObexClient class is used to send OBEX requests.
 */
class OpenObexClient : public QObject
{
    friend void openobex_client_callback(obex_t *handle, obex_object_t *obj, int mode,
                                          int event, int obex_cmd, int obex_rsp);
    Q_OBJECT

public:
    OpenObexClient(QObexSocket *socket, QObject *parent = 0);
    ~OpenObexClient();

    void startNewRequest();
    void sendRequest();
    void addHeader(uint8_t hi, obex_headerdata_t hv, uint32_t hv_size);

    obex_t *m_handle;
    QObexSocket *m_socket;
    obex_object_t *m_obj;

signals:
    void done();

private:
    bool m_needRequestCleanup;
};

void openobex_client_callback(obex_t *handle, obex_object_t */*obj*/, int /*mode*/,
                              int event, int /*obex_cmd*/, int /*obex_rsp*/)
{
    //qLog(Obex) << "openobex_client_callback()" << event;
    switch (event) {
        case OBEX_EV_REQDONE:
            OpenObexClient *client =
                    static_cast<OpenObexClient*>(OBEX_GetUserData(handle));
            emit client->done();
            break;
    }
}

OpenObexClient::OpenObexClient(QObexSocket *socket, QObject *parent)
    : QObject(parent),
      m_socket(socket),
      m_obj(0),
      m_needRequestCleanup(false)
{
}

OpenObexClient::~OpenObexClient()
{
    if (m_needRequestCleanup && m_handle && m_obj) {
        //qLog(Obex) << "Calling OBEX_ObjectDelete()";
        OBEX_ObjectDelete(m_handle, m_obj);
    }
    m_socket->resetObexHandle();
}

/*
    Creates a new SETPATH request.
    (SETPATH is used since it's the simplest single-packet operation that
    can be used repeatedly.)
*/
void OpenObexClient::startNewRequest()
{
    bool b = false;

    QTcpSocket *tcpSocket = qobject_cast<QTcpSocket*>(m_socket->device());
    Q_ASSERT(tcpSocket != 0);
    Q_ASSERT(tcpSocket->state() == QTcpSocket::ConnectedState);

    // client set-up
    m_handle = static_cast<obex_t *>(m_socket->handle());
    Q_ASSERT(m_handle != 0);

    OBEX_SetUserCallBack(m_handle, openobex_client_callback, 0);
    OBEX_SetUserData(m_handle, this);

    if (m_needRequestCleanup && m_handle && m_obj) {
        //qLog(Obex) << "Calling OBEX_ObjectDelete()";
        OBEX_ObjectDelete(m_handle, m_obj);
    }

    // create a new client request
    m_obj = OBEX_ObjectNew(m_handle, OBEX_CMD_SETPATH);
    Q_ASSERT(m_obj != 0);

    // must set the SETPATH flags for the request, else it will fail
    uint8_t nonHeaderData[2] = { 0, 0 };

    b = OBEX_ObjectSetNonHdrData(m_obj, nonHeaderData, 2);
    Q_ASSERT(b >= 0);

    m_needRequestCleanup = true;
}

void OpenObexClient::sendRequest()
{
    bool b = OBEX_Request(m_handle, m_obj);
    Q_ASSERT(b >= 0);
    m_needRequestCleanup = false;
}

void OpenObexClient::addHeader(uint8_t hi, obex_headerdata_t hv, uint32_t hv_size)
{
    Q_ASSERT(m_obj != 0);
    bool b = OBEX_ObjectAddHeader(m_handle, m_obj, hi, hv, hv_size, OBEX_FL_FIT_ONE_PACKET);
    Q_ASSERT(b >= 0);
}


//======================================================================

HeaderValue::HeaderValue(int id, unsigned int size)
    : m_id(id), m_size(size), m_bs(0)
{
}

HeaderValue::~HeaderValue()
{
    if (m_bs)
        free(m_bs);
}

void HeaderValue::setBuf(const uint8_t *bs)
{
    if (m_size > 0) {
        if (m_bs)
            free(m_bs);
        m_bs = (uint8_t*)malloc(sizeof(uint8_t) * m_size);
        memcpy(m_bs, bs, m_size);
    }
}

void HeaderValue::readBuf(uint8_t **buf)
{
    *buf = m_bs;
}

void HeaderValue::clearList(QList<HeaderValue *> &headerValues)
{
    while (headerValues.size() > 0)
        delete headerValues.takeLast();
}


//======================================================================

/*?
    The OpenObexService class is used to receive OBEX requests.
    It emits the receivedHeader() signal when a request has been received.
 */
class OpenObexService : public QObject
{
    friend void openobex_service_callback(obex_t *handle, obex_object_t *obj, int mode,
                                  int event, int obex_cmd, int obex_rsp);
    Q_OBJECT

public:
    OpenObexService(QObexSocket *socket, OpenObexHeaderProcessor *parent = 0);
    ~OpenObexService();

    QObexSocket *m_socket;
    obex_t *m_handle;
    OpenObexHeaderProcessor *m_parent;
};

void openobex_service_callback(obex_t *handle, obex_object_t *obj, int /*mode*/,
                           int event, int obex_cmd, int /*obex_rsp*/)
{
    //qLog(Obex) << "openobex_service_callback()" << event << obex_cmd;

    OpenObexService *service =
            static_cast<OpenObexService *>(OBEX_GetUserData(handle));
    switch (event) {
        case OBEX_EV_REQHINT:
            OBEX_ObjectSetRsp(obj, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
            break;
        case OBEX_EV_REQ:
        {
            // the OpenObexClient client sends SetPath requests
            if (obex_cmd == OBEX_CMD_SETPATH)
                service->m_parent->serviceReceivedRequest(handle, obj);
            break;
        }
    }
}

OpenObexService::OpenObexService(QObexSocket *socket, OpenObexHeaderProcessor *parent)
    : QObject(parent),
	  m_socket(socket),
	  m_parent(parent)
{
    m_handle = static_cast<obex_t *>(socket->handle());
    OBEX_SetUserCallBack(m_handle, openobex_service_callback, 0);
    OBEX_SetUserData(m_handle, this);
}

OpenObexService::~OpenObexService()
{
    m_socket->resetObexHandle();
}

//====================================================================

OpenObexHeaderProcessor::OpenObexHeaderProcessor(QObject *parent)
    : QObject(parent),
      m_client(0),
      m_server(0),
      m_userResultsStorage(0)
{
}

OpenObexHeaderProcessor::~OpenObexHeaderProcessor()
{
}

void OpenObexHeaderProcessor::startNewRequest()
{
    if (!m_server) {
        qLog(Obex) << "Starting test server...";

        m_server = new QTcpServer(this);
        connect(m_server, SIGNAL(newConnection()), SLOT(newConnection()));

        bool b = m_server->listen(QHostAddress(QHostAddress::Any), 0);
        Q_ASSERT(b);
        qLog(Obex) << "Started test server on" << m_server->serverAddress() << m_server->serverPort() << "...";

        Q_ASSERT(m_client == 0);
        QTcpSocket *tcpSocket = new QTcpSocket;
        tcpSocket->connectToHost(m_server->serverAddress(), m_server->serverPort());
        b = tcpSocket->waitForConnected();
        Q_ASSERT(b);

        QObexSocket *socket = new QObexSocket(tcpSocket);
        tcpSocket->setParent(socket);
        m_client = new OpenObexClient(socket, this);
        socket->setParent(m_client);
        connect(m_client, SIGNAL(done()), SIGNAL(done()));
    }

    m_client->startNewRequest();
}

void OpenObexHeaderProcessor::add4ByteHeader(uint8_t hi, uint32_t value)
{
    obex_headerdata_t hv;
    hv.bq4 = value;
    m_client->addHeader(hi, hv, 4);
}

void OpenObexHeaderProcessor::add1ByteHeader(uint8_t hi, uint8_t value)
{
    obex_headerdata_t hv;
    hv.bq1 = value;
    m_client->addHeader(hi, hv, 1);
}

void OpenObexHeaderProcessor::addBytesHeader(uint8_t hi, const uint8_t *bytes, uint size)
{
    obex_headerdata_t hv;
    hv.bs = bytes;
    m_client->addHeader(hi, hv, size);
}

void OpenObexHeaderProcessor::run(QList<HeaderValue *> *results)
{
    m_userResultsStorage = results;

    m_client->sendRequest();
}

obex_object_t *OpenObexHeaderProcessor::currentRequest() const
{
    return m_client->m_obj;
}

obex_t *OpenObexHeaderProcessor::clientHandle() const
{
    return m_client->m_handle;
}

void OpenObexHeaderProcessor::newConnection()
{
    QIODevice *device = m_server->nextPendingConnection();
    Q_ASSERT(device != 0);

    QObexSocket *socket = new QObexSocket(device);
    device->setParent(socket);
    OpenObexService *service = new OpenObexService(socket, this);
    socket->setParent(service);
}

// The obex object will be invalid once this functions returns, so it
// must be parsed here.
void OpenObexHeaderProcessor::serviceReceivedRequest(obex_t *handle, obex_object_t *obj)
{
    bool shouldEmit = (receivers(SIGNAL(receivedHeaders(obex_t*,obex_object_t*))) > 0);

    if (m_userResultsStorage) {
        *m_userResultsStorage = listFromObject(handle, obj);

        // if the signal is going to be emitted, need to call ReParseHeaders() first
        if (shouldEmit)
            OBEX_ObjectReParseHeaders(handle, obj);
    }

    if (shouldEmit)
        emit receivedHeaders(handle, obj);
}

QList<HeaderValue *> OpenObexHeaderProcessor::listFromObject(obex_t* handle, obex_object_t *obj)
{
    QList<HeaderValue *> headers;

    uchar hi;
    unsigned int hv_size;
    obex_headerdata_t hv;
    while (OBEX_ObjectGetNextHeader(handle, obj, &hi, &hv, &hv_size)) {
        //qLog(Obex) << "\tOBEX_ObjectGetNextHeader():" << hi << hv_size;
        HeaderValue *h = new HeaderValue(hi, hv_size);

        switch (hi & OBEX_HI_MASK) {
            case OBEX_INT:
                h->m_bq4 = hv.bq4;
                break;
            case OBEX_BYTE:
                h->m_bq1 = hv.bq1;
                break;
            case OBEX_BYTE_STREAM:
            case OBEX_UNICODE:
                h->setBuf(hv.bs);
                break;
            default:
                qLog(Obex) << "Unknown encoding:" << (hi & OBEX_HI_MASK);
                Q_ASSERT(false);
        }

        headers.append(h);
    }
    return headers;
}


#include "openobexheaderprocessor.moc"
