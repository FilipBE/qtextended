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

#include "../obextestsglobal.h"
#include "putclientwithnofirstpacketbody.h"
#include <QObexServerSession>
#include <QObexClientSession>
#include <QObexHeader>

#include <QTcpServer>
#include <QTcpSocket>
#include <qobexnamespace.h>

#include "qobexheader_p.h"
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QSignalSpy>
#include <shared/util.h>
#include <qtopiaapplication.h>

#include <QBuffer>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>
#include <QSlotInvoker>
#include <QPointer>

//TESTED_CLASS=QObexServerSession
//TESTED_FILES=src/libraries/qtopiacomm/obex/qobexserversession.cpp

static QByteArray get_some_data(int size)
{
    QByteArray bytes;
    QByteArray dt = QDateTime::currentDateTime().toString().toLatin1();
    while (bytes.size() < size)
        bytes.append(dt);
    if (bytes.size() > size)
        bytes.resize(size);
    return bytes;
}



class SingleOpServerSession : public QObexServerSession
{
    Q_OBJECT

signals:
    void slotCalled();
    void errorCalled();

protected:
    QObex::ResponseCode m_responseCode;
    QObexHeader m_responseHeader;
    int m_slotCalls;
    QObexHeader m_lastRequestHeader;
    QObexServerSession::Error m_lastError;
    int m_errorCount;

    void slotWasCalled(const QObexHeader &receivedRequestHeader)
    {
        m_slotCalls++;
        m_lastRequestHeader = receivedRequestHeader;
        setNextResponseHeader(m_responseHeader);
        emit slotCalled();
    }

public:
    SingleOpServerSession(QIODevice *device, QObject *parent = 0)
        : QObexServerSession(device, parent),
          m_responseCode(QObex::Success),
          m_slotCalls(0)
    {
        if (!device)
            qLog(Autotest) << "(Deliberately created with unconnected socket for testing, ignore warnings)";

        QObject::connect(this, SIGNAL(finalResponseSent(QObex::Request)),
                         SLOT(sentFinalResponse(QObex::Request)));
    }

    virtual void doDefaultClientRequest(QObexClientSession *client, const QObexHeader &header) = 0;

    /*
        See how many times the protected slot has been called (i.e. how many
        client requests have been received).
    */
    int slotCallCount() const { return m_slotCalls; }

    virtual void reset()
    {
        m_slotCalls = 0;
        m_lastError = QObexServerSession::Error(-1);
        m_errorCount = 0;
    }

    /*
        Returns the last client request that was received.
    */
    QObexHeader lastRequestHeader() const { return m_lastRequestHeader; }


    virtual void error(QObexServerSession::Error error, const QString &errorString)
    {
        m_lastError = error;
        m_errorCount++;
        qLog(Autotest) << "SingleOpServerSession error:" << errorString;
        emit errorCalled();
    }
    QObexServerSession::Error lastError() const { return m_lastError; }
    int errorCount() const { return m_errorCount; }

    /*
        Sets the response code & response headers that should be sent back
        to a client when a request is received.
        The default response is QObex::Success.
    */
    void setTestResponse(QObex::ResponseCode code, const QObexHeader &header)
    {
        m_responseCode = code;
        m_responseHeader = header;
    }

protected slots:
    virtual void sentFinalResponse(QObex::Request request) = 0;
};

/*
    A session that doesn't support any types of client requests.
*/
class NoOpServerSession : public SingleOpServerSession
{
    Q_OBJECT
public:
    NoOpServerSession(QIODevice *device, QObject *parent = 0)
    : SingleOpServerSession(device, parent)
    {
        reset();
    }
    virtual void doDefaultClientRequest(QObexClientSession *, const QObexHeader &) {}

protected slots:
    virtual void sentFinalResponse(QObex::Request) { QVERIFY(false); }
};


class ConnectableServerSession : public SingleOpServerSession
{
    Q_OBJECT
public:
    ConnectableServerSession(QIODevice *device, QObject *parent = 0)
    : SingleOpServerSession(device, parent)
    {
        reset();
    }

    virtual void doDefaultClientRequest(QObexClientSession *client, const QObexHeader &header)
    {
        client->connect(header);
    }

protected slots:
    QObex::ResponseCode connect(const QObexHeader &header)
    {
        qLog(Autotest) << "ConnectableServerSession::connect()";
        slotWasCalled(header);
        return m_responseCode;
    }

    virtual void sentFinalResponse(QObex::Request request)
    {
        QCOMPARE(request, QObex::Connect);
    }
};

class DisconnectableServerSession : public SingleOpServerSession
{
    Q_OBJECT
public:
    DisconnectableServerSession(QIODevice *device, QObject *parent = 0)
    : SingleOpServerSession(device, parent)
    {
        reset();
    }

    virtual void doDefaultClientRequest(QObexClientSession *client, const QObexHeader &header)
    {
        client->disconnect(header);
    }

protected slots:
    QObex::ResponseCode disconnect(const QObexHeader &header)
    {
        qLog(Autotest) << "DisconnectableServerSession::disconnect()";
        slotWasCalled(header);
        return m_responseCode;
    }

    virtual void sentFinalResponse(QObex::Request request)
    {
        QCOMPARE(request, QObex::Disconnect);
    }
};


class PuttableServerSession : public SingleOpServerSession
{
    Q_OBJECT

public:
    QList<QByteArray> m_receivedData;
    QObex::ResponseCode m_dataAvailableResponse;
    bool m_putCalledBeforeDataAvailable;

    int m_minPacketsBeforeUsingResponse;

    PuttableServerSession(QIODevice *device, QObject *parent = 0)
    : SingleOpServerSession(device, parent)
    {
        reset();
    }

    virtual void reset()
    {
        SingleOpServerSession::reset();
        m_dataAvailableResponse = QObex::Success;
        m_minPacketsBeforeUsingResponse = -1;
        m_receivedData.clear();
    }

    virtual QObex::ResponseCode dataAvailable(const char *data, qint64 size)
    {
        qLog(Autotest) << "PuttableServerSession:dataAvailable()" << size;
        m_receivedData.append(QByteArray(data, size));
        if (m_minPacketsBeforeUsingResponse != 1) {
            if (m_receivedData.size() >= m_minPacketsBeforeUsingResponse) {
                return m_dataAvailableResponse;
            } else {
                return QObex::Success;
            }
        } else {
            return m_dataAvailableResponse;
        }
    }

    virtual void doDefaultClientRequest(QObexClientSession *client, const QObexHeader &header)
    {
        client->put(header, QByteArray("test data"));
    }

protected slots:
    QObex::ResponseCode put(const QObexHeader &header)
    {
        qLog(Autotest) << "PuttableServerSession::put()";

        // dataAvailable() should not have been called yet
        m_putCalledBeforeDataAvailable = (m_receivedData.size() == 0);

        slotWasCalled(header);
        return m_responseCode;
    }

    virtual void sentFinalResponse(QObex::Request request)
    {
        QCOMPARE(request, QObex::Put);
    }
};


class PutDeletableServerSession : public SingleOpServerSession
{
    Q_OBJECT
public:
    bool m_wrongFunctionsCalled;

    PutDeletableServerSession(QIODevice *device, QObject *parent = 0)
    : SingleOpServerSession(device, parent)
    {
        reset();
        m_wrongFunctionsCalled = false;
    }

    virtual void doDefaultClientRequest(QObexClientSession *client, const QObexHeader &header)
    {
        client->putDelete(header);
    }

    virtual QObex::ResponseCode dataAvailable(const char *, qint64)
    {
        m_wrongFunctionsCalled = true;
        return m_responseCode;
    }

protected slots:
    QObex::ResponseCode putDelete(const QObexHeader &header)
    {
        qLog(Autotest) << "PutDeletableServerSession::putDelete()";
        slotWasCalled(header);
        return m_responseCode;
    }

    QObex::ResponseCode put(const QObexHeader &)
    {
        m_wrongFunctionsCalled = true;
        return m_responseCode;
    }

    virtual void sentFinalResponse(QObex::Request request)
    {
        QCOMPARE(request, QObex::PutDelete);
    }
};


class GettableServerSession : public SingleOpServerSession
{
    Q_OBJECT
public:
    QBuffer m_sendBuffer;
    int m_packetSize;
    QObex::ResponseCode m_provideDataResponse;
    int m_provideDataCalls;
    QByteArray m_bytes;

    int m_sentBytes;


    GettableServerSession(QIODevice *device, QObject *parent = 0)
    : SingleOpServerSession(device, parent)
    {
        reset();
    }

    virtual void reset()
    {
        SingleOpServerSession::reset();
        m_packetSize = 255;
        m_provideDataResponse = QObex::Success;
        m_provideDataCalls = 0;
        m_sentBytes = 0;
    }

    ~GettableServerSession()
    {
        m_sendBuffer.close();
        m_sendBuffer.setData(NULL, 0);
    }

    virtual QObex::ResponseCode provideData(const char **data, qint64 *size)
    {
        qLog(Autotest) << "GettableServerSession:provideData() reading from total" << m_sendBuffer.size();
        m_provideDataCalls++;

        if (m_provideDataResponse != QObex::Success)
            return m_provideDataResponse;

        if (m_sendBuffer.atEnd()) {
            m_sendBuffer.close();
        } else {

            m_bytes = m_sendBuffer.read(m_packetSize);
            if (m_bytes.size() > 0) {
                *data = m_bytes.constData();
                *size = m_bytes.size();
            }
        }
        return QObex::Success;
    }

    virtual void doDefaultClientRequest(QObexClientSession *client, const QObexHeader &header)
    {
        client->get(header);
    }

protected slots:
    QObex::ResponseCode get(const QObexHeader &header)
    {
        qLog(Autotest) << "GettableServerSession::get()";
        slotWasCalled(header);

        m_sendBuffer.open(QIODevice::ReadOnly);

        return m_responseCode;
    }

    virtual void sentFinalResponse(QObex::Request request)
    {
        QCOMPARE(request, QObex::Get);
    }
};


class GetClient : public QObject
{
    Q_OBJECT
public:
    QObexClientSession *m_client;
    QBuffer m_recvBuffer;

    GetClient(QIODevice *device, QObject *parent = 0)
    : QObject(parent),
      m_client(new QObexClientSession(device, this))
    {
        m_recvBuffer.open(QIODevice::WriteOnly);
    }

    void get(const QObexHeader &header)
    {
        m_client->get(header, &m_recvBuffer);
    }
};


class SetPathableServerSession : public SingleOpServerSession
{
    Q_OBJECT
public:
    QObex::SetPathFlags m_recvdFlags;

    SetPathableServerSession(QIODevice *device, QObject *parent = 0)
    : SingleOpServerSession(device, parent)
    {
        reset();
        m_recvdFlags = 0;
    }

    virtual void doDefaultClientRequest(QObexClientSession *client, const QObexHeader &header)
    {
        client->setPath(header, 0);
    }

protected slots:
    QObex::ResponseCode setPath(const QObexHeader &header, QObex::SetPathFlags flags)
    {
        qLog(Autotest) << "SetPathableServerSession::setPath()";
        slotWasCalled(header);

        m_recvdFlags = flags;
        return m_responseCode;
    }

    virtual void sentFinalResponse(QObex::Request request)
    {
        QCOMPARE(request, QObex::SetPath);
    }
};

class AbortableServerSession : public SingleOpServerSession
{
    Q_OBJECT

public:
    AbortableServerSession(QIODevice *device, QObject *parent = 0)
    : SingleOpServerSession(device, parent)
    {
        reset();
    }

    virtual QObex::ResponseCode dataAvailable(const char *, qint64)
    {
        qLog(Autotest) << "AbortableServerSession:dataAvailable()";
        return QObex::Success;
    }

    virtual void doDefaultClientRequest(QObexClientSession *client, const QObexHeader &header)
    {
        QObject::connect(client, SIGNAL(dataTransferProgress(qint64,qint64)),
                           client, SLOT(abort()));

        // send lots of data to ensure there are multiple put packets
        client->put(header, get_some_data(10000));
    }

protected slots:
    QObex::ResponseCode put(const QObexHeader &)
    {
        qLog(Autotest) << "AbortableServerSession::put()";
        return QObex::Success;
    }

    virtual void sentFinalResponse(QObex::Request) { QVERIFY(false); }
};


class ReentrantServerSession : public SingleOpServerSession
{
    Q_OBJECT

public:
    ReentrantServerSession(QIODevice *device, QObject *parent = 0)
    : SingleOpServerSession(device, parent)
    {
        reset();
    }

    virtual QObex::ResponseCode dataAvailable(const char *, qint64)
    {
        qLog(Autotest) << "ReentrantServerSession:dataAvailable()";
        return QObex::Success;
    }

    virtual void doDefaultClientRequest(QObexClientSession *client, const QObexHeader &header)
    {
        client->put(header, QByteArray());
    }

protected slots:
    QObex::ResponseCode put(const QObexHeader &)
    {
        qLog(Autotest) << "ReentrantServerSession::put()";

        QDialog msg;
        QTimer::singleShot(1000, &msg, SLOT(accept()));
        msg.exec();

        return QObex::Success;
    }

    virtual void sentFinalResponse(QObex::Request request)
    {
        QCOMPARE(request, QObex::Put);
    }
};


//=================================================================


class tst_QObexServerSession : public QObject
{
    Q_OBJECT

protected:
    /*
    Test that if a server session implements one of the protected slots, and
    it returns a certain response code and sets a certain set of response
    headers, the client should get back the correct response data after a request.
    */
    void testServerResponseIsSent(SingleOpServerSession *session, const QObexHeader &responseHeader, QObex::ResponseCode responseCode)
    {
        session->setTestResponse(responseCode, responseHeader);
        session->reset();

        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        QSignalSpy spy(client,SIGNAL(done(bool)));
        session->doDefaultClientRequest(client, QObexHeader());
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0).at(0).value<bool>(), responseCode!=QObex::Success );

        // If it's a Put or Get, check dataAvailable() and provideData() weren't called
        if (responseCode != QObex::Success) {
            PuttableServerSession *putServer =
                    qobject_cast<PuttableServerSession*>(session);
            if (putServer) {
                QCOMPARE(putServer->m_receivedData.size(), 0);
            }

            GettableServerSession *getServer =
                    qobject_cast<GettableServerSession*>(session);
            if (getServer)
                QCOMPARE(getServer->m_provideDataCalls, 0);
        }

        QCOMPARE(session->slotCallCount(), 1);
        QCOMPARE(client->lastResponseCode(), responseCode);
        QCOMPARE(client->lastResponseHeader(), responseHeader);
        delete client;
    }

    /*
    Tests a server session receives the correct client headers in the
    implemented protected slot.
    */
    void testSlotReceivesRequestHeaders(SingleOpServerSession *session, const QObexHeader &requestHeader)
    {
        // set server session to accept client requests
        session->setTestResponse(QObex::Success, QObexHeader());
        session->reset();

        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        QSignalSpy spy(client,SIGNAL(done(bool)));
        session->doDefaultClientRequest(client, requestHeader);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        QCOMPARE(session->slotCallCount(), 1);
        QCOMPARE(session->lastRequestHeader(), requestHeader);
        delete client;
    }

    /*
    Tests that if a slot is not implemented, the client should get a 
    NotImplemented response and empty headers.
    */
    void testUnimplementedSlot(SingleOpServerSession *referenceSession)
    {
        SingleOpServerSession *noOpSession = new NoOpServerSession(m_serverSessionSocket);

        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        QSignalSpy spy(client,SIGNAL(done(bool)));
        referenceSession->doDefaultClientRequest(client, QObexHeader());
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << true ); // request fails because not implemented

        QCOMPARE(noOpSession->slotCallCount(), 0);
        QCOMPARE(client->lastResponseCode(), QObex::NotImplemented);
        QCOMPARE(client->lastResponseHeader(), QObexHeader()); // no response header was sent

        delete client;
        delete noOpSession;
    }

    void runGenericSlotTests(SingleOpServerSession *session)
    {
        QObexHeader h;
        h.setCount(10);
        h.setName("test test");
        h.setTime(QDateTime::currentDateTime());

        // Check that the server receives any request headers sent by the client.
        testSlotReceivesRequestHeaders(session, QObexHeader());
        testSlotReceivesRequestHeaders(session, h);

        // Check that the response code returned in server's protected slot is
        // received by the client.
        testServerResponseIsSent(session, QObexHeader(), QObex::Success);
        testServerResponseIsSent(session, QObexHeader(), QObex::BadRequest);
        testServerResponseIsSent(session, QObexHeader(), QObex::Forbidden);

        // Check that the response headers specified in server protected slot are
        // received by the client.
        // Empty headers:
        testServerResponseIsSent(session, QObexHeader(), QObex::Success);
        // Non-empty headers:
        testServerResponseIsSent(session, h, QObex::Success);
    }

private:
    QTcpSocket *m_serverSessionSocket;
    QTcpSocket *m_clientSocket;
    QTcpServer *m_server;

private slots:
    void initTestCase()
    {
        m_server = new QTcpServer(this);
        bool b = m_server->listen(QHostAddress(QHostAddress::Any), 0);
        Q_ASSERT(b);
    }

    void cleanupTestCase()
    {
    }

    void init()
    {
        QSignalSpy spy(m_server,SIGNAL(newConnection()));
        m_clientSocket = new QTcpSocket;
        m_clientSocket->connectToHost(m_server->serverAddress(),
                                      m_server->serverPort());
        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();

        m_serverSessionSocket = m_server->nextPendingConnection();
        Q_ASSERT(m_serverSessionSocket != 0);
    }

    void cleanup()
    {
        delete m_clientSocket;
        delete m_serverSessionSocket;
    }

    void close()
    {
        QObexServerSession *server;

        // close should not close or touch the underlying socket
        server = new NoOpServerSession(m_serverSessionSocket);
        QIODevice *device = server->sessionDevice();
        Q_ASSERT(device->isOpen());
        server->close();
        Q_ASSERT(device->isOpen());
        delete server;

        // close should not crash on null socket
        server = new NoOpServerSession(0);
        QVERIFY(0 == server->sessionDevice());
        server->close();
        QVERIFY(0 == server->sessionDevice());
        delete server;
    }

   void connect_genericTest()
    {
        ConnectableServerSession *session = new ConnectableServerSession(m_serverSessionSocket);
        runGenericSlotTests(session);
        delete session;

        ConnectableServerSession *dummySession = new ConnectableServerSession(0);
        testUnimplementedSlot(dummySession);
        delete dummySession;
    }

    void disconnect_genericTest()
    {
        DisconnectableServerSession *session = new DisconnectableServerSession(m_serverSessionSocket);
        runGenericSlotTests(session);
        delete session;

        DisconnectableServerSession *dummySession = new DisconnectableServerSession(0);
        testUnimplementedSlot(dummySession);
        delete dummySession;
    }

    void put_genericTest()
    {
        PuttableServerSession *session = new PuttableServerSession(m_serverSessionSocket);
        runGenericSlotTests(session);
        delete session;

        PuttableServerSession *dummySession = new PuttableServerSession(0);
        testUnimplementedSlot(dummySession);
        delete dummySession;
    }

    void put_dataAvailable_1packet()
    {
        PuttableServerSession *session = new PuttableServerSession(m_serverSessionSocket);
        QObexClientSession *client = new QObexClientSession(m_clientSocket);

        // some data that fits in one packet
        QByteArray originalData("asdfijawoeifjaosifjoi1j2312390172");

        QSignalSpy spy(client,SIGNAL(done(bool)));
        client->put(QObexHeader(), originalData);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        // put() should have been called before dataAvailable()
        QVERIFY(session->m_putCalledBeforeDataAvailable);

        // put() should only be called once
        QCOMPARE(session->slotCallCount(), 1);

        // check dataAvailable() was called twice (second time to say all data is received)
        QCOMPARE(session->m_receivedData.size(), 2);

        // check the received packets
        QCOMPARE(session->m_receivedData[0], originalData);
        QCOMPARE(session->m_receivedData[1], QByteArray());  // zero-length indicated end-of-data

        delete client;
        delete session;
    }

    void put_dataAvailable_manyPackets()
    {
        PuttableServerSession *session = new PuttableServerSession(m_serverSessionSocket);
        QObexClientSession *client = new QObexClientSession(m_clientSocket);

        // something big enough to take a few packets to send
        QByteArray originalData = get_some_data(5000);

        QSignalSpy spy(client,SIGNAL(done(bool)));
        client->put(QObexHeader(), originalData);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        // put() should have been called before dataAvailable()
        QVERIFY(session->m_putCalledBeforeDataAvailable);

        // put() should only be called once
        QCOMPARE(session->slotCallCount(), 1);

        // check dataAvailable() was called at least three times
        // (... unless the obex buf size is really big?)
        // Can't know the exact buffer size, which is why this just checks
        // the minimum size here.
        QVERIFY(session->m_receivedData.size() >= 3);

        // last packet should be empty (to indicate end of data)
        QCOMPARE(session->m_receivedData[session->m_receivedData.size()-1], QByteArray());

        // other packets should add up to original body data
        QByteArray recvdBytes;
        for (int i=0; i<session->m_receivedData.size()-1; i++) {    // up to 2nd-last packet
            recvdBytes.append(session->m_receivedData[i]);
            QVERIFY(session->m_receivedData[i].size() != 0);    // shouldn't get empty packets
        }
        QCOMPARE(recvdBytes.size(), originalData.size());
        QCOMPARE(recvdBytes, originalData);

        delete client;
        delete session;
    }

    // If the client sends an empty body, dataAvailable() should just be called 
    // once, with zero length.
    void put_dataAvailable_zeroData()
    {
        PuttableServerSession *session = new PuttableServerSession(m_serverSessionSocket);
        QObexClientSession *client = new QObexClientSession(m_clientSocket);

        QByteArray originalData("");

        QSignalSpy spy(client,SIGNAL(done(bool)));
        client->put(QObexHeader(), originalData);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        // put() should only be called once
        QCOMPARE(session->slotCallCount(), 1);

        // dataAvailable() should be called once, to signal all data was received
        QCOMPARE(session->m_receivedData.size(), 1);
        QCOMPARE(session->m_receivedData[0], QByteArray());

        delete client;
        delete session;
    }

    // Test the subclass can refuse an operation by returning a non-success
    // response in dataAvailable().
    void put_dataAvailable_response()
    {
        PuttableServerSession *session = new PuttableServerSession(m_serverSessionSocket);
        QObexClientSession *client = new QObexClientSession(m_clientSocket);

        // some non-success response
        session->m_dataAvailableResponse = QObex::Forbidden;

        // check client before the test
        QCOMPARE(client->lastResponseCode(), QObex::Success);

        QByteArray originalData("asdfijawoeifjaosifjoi1j2312390172");

        QSignalSpy spy(client,SIGNAL(done(bool)));
        client->put(QObexHeader(), originalData);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << true );  // fails

        // put() should only be called once
        QCOMPARE(session->slotCallCount(), 1);

        // dataAvailable() should only have been called once 
        QCOMPARE(session->m_receivedData.size(), 1);

        // check the received data
        QCOMPARE(session->m_receivedData[0], originalData.left(session->m_receivedData[0].size()));

        // check the response received by the client 
        QCOMPARE(client->lastResponseCode(), QObex::Forbidden);

        delete client;
        delete session;
    }

    // Same as before, but don't return the non-success response until a few packets
    // have been sent.
    void put_dataAvailable_response_cancelMidway()
    {
        PuttableServerSession *session = new PuttableServerSession(m_serverSessionSocket);
        QObexClientSession *client = new QObexClientSession(m_clientSocket);

        QByteArray originalData = get_some_data(2000);

        int numPacketsToAllow = 2;

        // some non-success response
        session->m_dataAvailableResponse = QObex::Forbidden;
        session->m_minPacketsBeforeUsingResponse = numPacketsToAllow;

        // check client before the test
        QCOMPARE(client->lastResponseCode(), QObex::Success);

        QSignalSpy spy(client,SIGNAL(done(bool)));
        client->put(QObexHeader(), originalData);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << true );  // fails

        // put() should only be called once
        QCOMPARE(session->slotCallCount(), 1);

        // should only have received the packets sent before the Forbidden response
        QByteArray recvdBytes;
        for (int i=0; i<session->m_receivedData.size(); i++)
            recvdBytes.append(session->m_receivedData[i]);
        QVERIFY(recvdBytes.size() < originalData.size());
        QCOMPARE(recvdBytes, originalData.mid(0, recvdBytes.size()));

        QCOMPARE(session->m_receivedData.size(), numPacketsToAllow);

        // check the response received by the client 
        QCOMPARE(client->lastResponseCode(), QObex::Forbidden);

        delete client;
        delete session;
    }

    /*
        We need to make sure we can distinguish a Put from a Put-Delete if
        it is not obvious from the very first packet (i.e. when we get an
        OBEX_EV_REQCHECK). That is, a Put request may be made in multiple
        ways:

        a) The first packet has a Body or End-of-Body header
        b) The first packet doesn't have the Body nor End-of-Body headers.
           These headers are sent in the following packets instead.

        The first is easy to recognise because we can see the Body or
        End-of-Body header in the OBEX_EV_REQCHECK. However, if there is
        *no* body data in the first packet, the server should not immediately
        think that it is a Put-Delete because the body data might be coming
        in the following packets, in which case it is a Put, not a Put-Delete.

        This test checks that we recognise method b) as a Put request, not
        a Put-Delete.
    */
    void put_unusual_data()
    {
        QTest::addColumn<QByteArray>("originalData");
        QTest::addColumn<int>("dummyDataLength");
        QTest::addColumn<int>("expectedPacketCount");

        QByteArray data = get_some_data(100);

        // Test that it works if Body is not sent until second packet.
        QTest::newRow("forceBodyIntoSecondPacket")
                << data
                << PutClientWithNoFirstPacketBody::MAX_HEADER_SIZE
                << 2;

        // Test that it works if Body is partially sent in first packet
        // (just like the way put() is normally tested).
        // This is to test that the PutClientWithNoFirstPacketBody is 
        // testing it correctly if we slightly changed the parameters.
        // Need to use MAX_HEADER_SIZE - 3 and not just MAX_HEADER_SIZE - 1
        // because openobex needs to fit the 3 bytes of header info before
        // the body; otherwise it will place the body into the next packet.
        QTest::newRow("normalPut") 
                << data
                << PutClientWithNoFirstPacketBody::MAX_HEADER_SIZE - 3
                << 2;
    }

    void put_unusual()
    {
        QFETCH(QByteArray, originalData);
        QFETCH(int, dummyDataLength);
        QFETCH(int, expectedPacketCount);

        PuttableServerSession *session = new PuttableServerSession(m_serverSessionSocket);
        PutClientWithNoFirstPacketBody *client =
                new PutClientWithNoFirstPacketBody(m_clientSocket);

        QBuffer *buffer = new QBuffer(&originalData);

        QObexHeader header;
        QString s;
        s.fill('a', dummyDataLength);
        header.setType(s);

        QSignalSpy spy(client,SIGNAL(done(bool)));
        client->sendPutRequest(header, buffer);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        // dataAvailable() should be called at least twice (once or more
        // for data, once to signal done)
        QVERIFY(session->m_receivedData.size() > 1);

        // verify data was received by server
        QByteArray recvdBytes;
        for (int i=0; i<session->m_receivedData.size(); i++)
            recvdBytes.append(session->m_receivedData[i]);
        QCOMPARE(recvdBytes.size(), originalData.size());
        QCOMPARE(recvdBytes, originalData);

        QCOMPARE(client->sentPacketsCount(), expectedPacketCount);

        // check put() was called
        QCOMPARE(session->slotCallCount(), 1);

        // verify header was received by server
        QCOMPARE(session->lastRequestHeader(), header);

        delete buffer;
        delete client;
        delete session;
    }

    void putDelete_genericTest()
    {
        PutDeletableServerSession *session = new PutDeletableServerSession(m_serverSessionSocket);
        runGenericSlotTests(session);
        delete session;

        PutDeletableServerSession *dummySession = new PutDeletableServerSession(0);
        testUnimplementedSlot(dummySession);
        delete dummySession;
    }

    void putDelete_checkCalls()
    {
        PutDeletableServerSession *session = new PutDeletableServerSession(m_serverSessionSocket);
        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        QSignalSpy spy(client,SIGNAL(done(bool)));
        client->putDelete(QObexHeader());
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        QCOMPARE(session->m_wrongFunctionsCalled, false);

        delete client;
        delete session;
    }

    void get_genericTest()
    {
        GettableServerSession *session = new GettableServerSession(m_serverSessionSocket);
        runGenericSlotTests(session);
        delete session;

        GettableServerSession *dummySession = new GettableServerSession(0);
        testUnimplementedSlot(dummySession);
        delete dummySession;
    }

    void get_provideData_data()
    {
        QTest::addColumn<QByteArray>("testBytes");
        QTest::addColumn<int>("minProvideDataCalls");
        QTest::addColumn<int>("maxProvideDataCalls");

        QTest::newRow("zero data") << QByteArray("") << 1 << 1;
        QTest::newRow("1-packet data") << QByteArray("alweoiwflaskfjoij") << 2 << 2;
        QTest::newRow("many-packet data") << get_some_data(5000) << 1 << 1000;
                                            // max unknown, don't know packet size
    }

    void get_provideData()
    {
        QFETCH(QByteArray, testBytes);
        QFETCH(int, minProvideDataCalls);
        QFETCH(int, maxProvideDataCalls);

        GettableServerSession *session = new GettableServerSession(m_serverSessionSocket);
        session->m_sendBuffer.setData(testBytes);

        GetClient *getClient = new GetClient(m_clientSocket);
        QSignalSpy spy(getClient->m_client,SIGNAL(done(bool)));
        getClient->get(QObexHeader());
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        // get() should have been called just once
        QCOMPARE(session->slotCallCount(), 1);

        // see how many times provideData() was called
        QVERIFY(session->m_provideDataCalls >= minProvideDataCalls);
        QVERIFY(session->m_provideDataCalls <= maxProvideDataCalls);

        // check the client got the correct data
        QCOMPARE(getClient->m_recvBuffer.data(), testBytes);

        delete getClient;
        delete session;
    }


    void get_provideData_response_data()
    {
        QTest::addColumn<QObex::ResponseCode>("testResponse");
        QTest::newRow("ok") << QObex::Success;
        QTest::newRow("forbidden") << QObex::Forbidden;
        QTest::newRow("internal server error") << QObex::InternalServerError;
        QTest::newRow("not found") << QObex::NotFound;
    }

    void get_provideData_response()
    {
        QFETCH(QObex::ResponseCode, testResponse);

        GettableServerSession *session = new GettableServerSession(m_serverSessionSocket);
        session->m_sendBuffer.setData(QByteArray(""));
        session->m_provideDataResponse = testResponse;

        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        QSignalSpy spy(client,SIGNAL(done(bool)));
        client->get(QObexHeader());
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0).at(0).value<bool>(), testResponse!=QObex::Success );

        // check the response received by the client
        QCOMPARE(client->lastResponseCode(), testResponse);

        delete client;
        delete session;
    }

    // Check that if you respond with body + headers, the client gets both of them.
    void get_withResponseHeader()
    {
        GettableServerSession *session = new GettableServerSession(m_serverSessionSocket);
        QObexHeader header;
        header.setName("test name");
        header.setType("test type");
        QByteArray testBytes("abcde");
        session->m_sendBuffer.setData(testBytes);
        session->setTestResponse(QObex::Success, header);

        GetClient *getClient = new GetClient(m_clientSocket);
        QSignalSpy spy(getClient->m_client,SIGNAL(done(bool)));
        getClient->get(QObexHeader());
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        // check the headers and the data received by the client
        QCOMPARE(getClient->m_recvBuffer.data(), testBytes);
        QCOMPARE(getClient->m_client->lastResponseHeader(), header);

        delete getClient;
        delete session;
    }

    void setPath_genericTest()
    {
        SetPathableServerSession *session = new SetPathableServerSession(m_serverSessionSocket);
        runGenericSlotTests(session);
        delete session;

        SetPathableServerSession *dummySession = new SetPathableServerSession(0);
        testUnimplementedSlot(dummySession);
        delete dummySession;
    }

    void setPath_flags_data()
    {
        QTest::addColumn<int>("testFlags");
        QTest::newRow("no flags") << 0;
        QTest::newRow("back up") << int(QObex::BackUpOneLevel);
        QTest::newRow("no path making") << int(QObex::NoPathCreation);
        QTest::newRow("back up + no path making")
                << int(QObex::BackUpOneLevel | QObex::NoPathCreation);
    }

    void setPath_flags()
    {
        QFETCH(int, testFlags);
        SetPathableServerSession *session = new SetPathableServerSession(m_serverSessionSocket);

        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        QSignalSpy spy(client,SIGNAL(done(bool)));
        client->setPath(QObexHeader(), static_cast<QObex::SetPathFlags>(testFlags));
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        QCOMPARE(session->m_recvdFlags, static_cast<QObex::SetPathFlags>(testFlags));

        delete client;
        delete session;
    }

    void abort()
    {
        AbortableServerSession *session = new AbortableServerSession(m_serverSessionSocket);
        QObexClientSession *client = new QObexClientSession(m_clientSocket);

        QSignalSpy spy(client,SIGNAL(done(bool)));
        session->doDefaultClientRequest(client, QObexHeader());
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << true );

        QCOMPARE(client->error(), QObexClientSession::Aborted);
        QCOMPARE(session->lastError(), QObexServerSession::Aborted);
        QCOMPARE(session->errorCount(), 1);

        delete client;
        delete session;
    }

    //-----------------------------------------------------

    void reentrant_callback()
    {
        ReentrantServerSession *session = new ReentrantServerSession(m_serverSessionSocket);
        QObexClientSession *client = new QObexClientSession(m_clientSocket);

        QSignalSpy spy(client,SIGNAL(done(bool)));
        session->doDefaultClientRequest(client, QObexHeader());
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        QCOMPARE(client->error(), QObexClientSession::NoError);
        QCOMPARE(session->lastError(), QObexServerSession::Error(-1));

        delete client;
        delete session;
    }

    //-----------------------------------------------------


public slots:
    void closeClientTransport()
    {
        m_clientSocket->close();
    }

    void closeServerTransport()
    {
       m_serverSessionSocket->close();
    }

    void deleteClientTransport()
    {
        delete m_clientSocket;
        m_clientSocket = 0;
    }

    void deleteServerTransport()
    {
        delete m_serverSessionSocket;
        m_serverSessionSocket = 0;
    }

private slots:

    void interruptedConnection_data()
    {
        QTest::addColumn<QString>("slotName");
        QTest::addColumn<bool>("interruptDuringRequest");

        /*
        // Can't test this, we won't be told at the QIODevice level if the
        // other side of a QTcpSocket connection closes.
        QTest::newRow("close client transport during request")
                << SLOT(closeClientTransport()) << true ;
        QTest::newRow("close client transport")
                << SLOT(closeClientTransport()) << false;
        */

        QTest::newRow("close server transport during request")
                << SLOT(closeServerTransport()) << true;
        QTest::newRow("close server transport")
                << SLOT(closeServerTransport()) << false;

        /*
        // Can't test this, we won't be told at the QIODevice level if the
        // other side of a QTcpSocket connection closes.
        QTest::newRow("delete client transport during request")
                << SLOT(deleteClientTransport()) << true;
        QTest::newRow("delete client transport")
                << SLOT(deleteClientTransport()) << false;
        */

        QTest::newRow("delete server transport during request")
                << SLOT(deleteServerTransport()) << true;
        QTest::newRow("delete server transport")
                << SLOT(deleteServerTransport()) << false;

    }

    // if transport socket is suddenly disconnected, we should know it
    // (whether a request is in progress or not)
    void interruptedConnection()
    {
        QFETCH(QString, slotName);
        QFETCH(bool, interruptDuringRequest);

        QPointer<QIODevice> pointer;
        if (slotName.contains("delete")) {
            if (slotName.contains("ServerTransport"))
                pointer = m_serverSessionSocket;
            else if (slotName.contains("ClientTransport"))
                pointer = m_clientSocket;
            else
                Q_ASSERT(false);

            QVERIFY(pointer != 0);
        }

        PuttableServerSession *session = new PuttableServerSession(m_serverSessionSocket);
        QSignalSpy spy(session,SIGNAL(errorCalled()));

        QObexClientSession *client = 0;
        if (interruptDuringRequest) {
            // start a multi-packet request, then break the connection during the request
            client = new QObexClientSession(m_clientSocket);
            QObject::connect(client, SIGNAL(dataTransferProgress(qint64,qint64)),
                             slotName.toLatin1().constData());
            client->put(QObexHeader(), get_some_data(5000));
        } else {
            // break the connection immediately
            QSlotInvoker inv(this, slotName.toLatin1().constData());
            QVERIFY(inv.canInvoke(0));
            inv.invoke(QList<QVariant>());
        }

        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();

        // see if the QIODevice or obex socket really was closed/deleted
        if (slotName.contains("delete")) {
            QVERIFY(pointer == 0);
        } else {
            QVERIFY(!m_serverSessionSocket->isOpen());
        }

        QCOMPARE(session->lastError(), QObexServerSession::ConnectionError);
        QCOMPARE(session->errorCount(), 1);

        delete client;
        delete session;
        delete pointer;
    }
};


QTEST_MAIN(tst_QObexServerSession)
#include "tst_qobexserversession.moc"
