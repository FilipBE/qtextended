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

#include <QTcpSocket>
#include <QTcpServer>
#include <QObexClientSession>

#include <QObexServerSession>

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <shared/util.h>
#include <shared/qtopiaunittest.h>
#include <qtopiaapplication.h>

#include <QBuffer>
#include <QDebug>
#include <QQueue>
#include <QPointer>

//TESTED_CLASS=QObexClientSession
//TESTED_FILES=src/libraries/qtopiacomm/obex/qobexclientsession.cpp

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

static QList<QObex::Request> getRequestList()
{
    QList<QObex::Request> ops;
    ops << QObex::Connect << QObex::Disconnect << QObex::Put
            << QObex::PutDelete << QObex::Get << QObex::SetPath;
    return ops;
}


class MyObexServerSession : public QObexServerSession
{
    Q_OBJECT
public:
    QObexHeader m_receivedHeaders;
    QBuffer m_sendBuffer;
    QBuffer m_recvBuffer;
    QByteArray m_lastSentBytes;
    qint64 m_bytesSoFar;

    QObex::ResponseCode m_responseCode;
    QObexHeader m_responseHeaders;

    QObexServerSession::Error m_lastError;


    MyObexServerSession(QIODevice *device, QObject *parent = 0)
        : QObexServerSession(device, parent)
    {
    }

    static QObexServerSession::Error noError()
    {
        return static_cast<QObexServerSession::Error>(-1);
    }

    void reset()
    {
        m_responseCode = QObex::Success;
        m_lastError = noError();
        m_bytesSoFar = 0;
        m_recvBuffer.close();
        m_recvBuffer.setData(NULL, 0);
        m_sendBuffer.close();
        m_sendBuffer.setData(NULL, 0);

        // set response headers to some random data
        m_responseHeaders = QObexHeader();
        m_responseHeaders.setName("response headers");
        m_responseHeaders.setType("text/plain");
    }

protected:
    virtual void error(QObexServerSession::Error error)
    {
        m_lastError = error;
    }

    virtual QObex::ResponseCode dataAvailable(const char *data, qint64 size)
    {
        m_bytesSoFar += size;
        if (size > 0) {
            if (m_recvBuffer.write(data, size) == -1) {
                qLog(Autotest) << "MyObexServerSession dataAvailable() error writing to recvBuffer";
                Q_ASSERT(false);
            }
        }
        return m_responseCode;
    }

    virtual QObex::ResponseCode provideData(const char **data, qint64 *size)
    {
        if (!m_sendBuffer.atEnd()) {
            m_lastSentBytes = m_sendBuffer.read(1024);
            if (m_lastSentBytes.size() > 0) {
                *data = m_lastSentBytes.constData();
                *size = m_lastSentBytes.size();
                m_bytesSoFar += *size;
            }
        }
        qLog(Autotest) << "MyObexServerSession::provideData()" << *size;
        return m_responseCode;
    }

private:
    QObex::ResponseCode respond(const QObexHeader &header)
    {
        m_receivedHeaders = header;
        setNextResponseHeader(m_responseHeaders);
        return m_responseCode;
    }

protected slots:
    QObex::ResponseCode connect(const QObexHeader &header) { return respond(header); }
    QObex::ResponseCode disconnect(const QObexHeader &header) { return respond(header); }
    QObex::ResponseCode put(const QObexHeader &header)
    {
        m_bytesSoFar = 0;
        m_recvBuffer.open(QIODevice::WriteOnly);
        return respond(header);
    }
    QObex::ResponseCode putDelete(const QObexHeader &header) { return respond(header); }
    QObex::ResponseCode get(const QObexHeader &header)
    {
        m_bytesSoFar = 0;
        m_sendBuffer.open(QIODevice::ReadOnly);
        m_responseHeaders.setLength(m_sendBuffer.size());
        return respond(header);
    }
    QObex::ResponseCode setPath(const QObexHeader &header, QObex::SetPathFlags) { return respond(header); }
};



//=========================================================

class Request
{
public:
    int id;
    QObex::Request op;
    QObexClientSession::Error expectedError;

    bool operator==(const Request &other) const
    {
        return (id == other.id && op == other.op && expectedError == other.expectedError);
    }
};
Q_DECLARE_METATYPE(Request);


class tst_QObexClientSession : public QObject
{
    Q_OBJECT

private:
    QTcpSocket *m_clientSessionSocket;
    MyObexServerSession *m_serverSession;
    QObexClientSession *m_client;

    QBuffer *m_buffer;
    qint64 m_lastDone;
    qint64 m_lastTotal;
    bool m_usingIODevice;

    QStringList m_emittedSignals;

    QQueue<Request> m_requests;

    bool m_calledAbort;

    QTcpServer *m_server;
    QTcpSocket *m_serverSessionSocket;


    int doDefaultRequest(QObexClientSession *client, QObex::Request op, const QObexHeader &header)
    {
        // do some kind of "default" request for each type of request
        int id = 0;
        switch (op) {
            case QObex::Connect:
                id = client->connect(header);
                break;
            case QObex::Disconnect:
                id = client->disconnect(header);
                break;
            case QObex::Put:
                id = client->put(header, QByteArray());
                break;
            case QObex::PutDelete:
                id = client->putDelete(header);
                break;
            case QObex::Get:
                id = client->get(header);
                break;
            case QObex::SetPath:
                id = client->setPath(header);
                break;
            default:
                Q_ASSERT(false);
        }
        return id;
    }

    void addRequest(int id, QObex::Request op, QObexClientSession::Error expectedError)
    {
        Request req;
        req.id = id;
        req.op = op;
        req.expectedError = expectedError;
        m_requests << req;
    }

    void checkCurrentData(bool opInProgress)
    {
        if (opInProgress) {
            if (m_requests.size() > 0) {
                QCOMPARE(m_client->currentId(), m_requests.head().id);
                QCOMPARE(m_client->currentRequest(), m_requests.head().op);
                QVERIFY(m_client->currentDevice() == (m_usingIODevice ? m_buffer : 0));
            }
        } else {
            QCOMPARE(m_client->currentId(), 0);
            QCOMPARE(m_client->currentRequest(), QObex::NoRequest);
            QVERIFY(m_client->currentDevice() == 0);
        }

        if (m_requests.size() > 0) {
            if (opInProgress)
                QCOMPARE((m_requests.count() > 1), m_client->hasPendingRequests());
            else 
                QVERIFY(!m_client->hasPendingRequests());
        }
    }

    // Checks for the signals that are always emitted during an request.
    void checkStandardEmittedSignals()
    {
        int opCount = m_requests.size();

        QCOMPARE(m_emittedSignals.first(), QString("requestStarted"));
        QCOMPARE(m_emittedSignals.count("requestStarted"), 1*opCount);

        QCOMPARE(m_emittedSignals[m_emittedSignals.size()-2], QString("requestFinished"));
        QCOMPARE(m_emittedSignals.count("requestFinished"), 1*opCount);

        QCOMPARE(m_emittedSignals.last(), QString("done"));
        QCOMPARE(m_emittedSignals.count("done"), 1);
    }


protected slots:
    void requestStarted(int id)
    {
        // if this is not the 1st request, move along to the next request
        if (m_emittedSignals.size() > 0)
            m_requests.dequeue();

        m_lastDone = 0;
        m_lastTotal = 0;

        m_emittedSignals << "requestStarted";

        QCOMPARE(m_client->currentId(), id);
        QCOMPARE(m_client->error(), QObexClientSession::NoError);
        checkCurrentData(true);
    }

    void requestFinished(int id, bool error)
    {
        m_emittedSignals << "requestFinished";

        if (m_requests.size() > 0) {
            QObexClientSession::Error expectedError = m_requests.head().expectedError;
            QCOMPARE(m_client->error(), expectedError);

            // the test server session always sends back headers
            if (expectedError == QObexClientSession::NoError || expectedError == QObexClientSession::RequestFailed)
                QVERIFY(m_emittedSignals.contains("responseHeaderReceived"));
        }

        QCOMPARE(m_client->currentId(), id);
        QCOMPARE(m_client->error() != QObexClientSession::NoError, error);

        // "current" data should still be valid at requestFinished() signal
        checkCurrentData(true);
    }

    void dataTransferProgress(qint64 done, qint64 total)
    {
        m_emittedSignals << "dataTransferProgress";
        Q_ASSERT( m_requests.head().op == QObex::Put || m_requests.head().op == QObex::Get);

        QVERIFY(done > 0);
        QVERIFY(done <= total); // test server always provides length header for total, won't be 0
        QVERIFY(done > m_lastDone);
        if (m_lastTotal > 0)
            QCOMPARE(total, m_lastTotal);

        if (m_requests.head().op == QObex::Put) {
            if (m_usingIODevice)
                QCOMPARE(done, m_buffer->pos());

        } else {
            // can't check exact pos() cos of mtu differences
            QVERIFY(done <= m_serverSession->m_bytesSoFar);
            QCOMPARE(total, m_serverSession->m_sendBuffer.size());

            // should get responseHeaderReceived() before the first dataTransferProgress()
            QVERIFY(m_emittedSignals.contains("responseHeaderReceived"));
        }

        m_lastDone = done;
        m_lastTotal = total;
        checkCurrentData(true);
    }

    void responseHeaderReceived(const QObexHeader &header)
    {
        m_emittedSignals << "responseHeaderReceived";
        QCOMPARE(header, m_serverSession->m_responseHeaders);
        checkCurrentData(true);
    }

    void readyRead()
    {
        m_emittedSignals << "readyRead";
        QVERIFY(!m_usingIODevice);

        qint64 bytesAvailable = m_client->bytesAvailable();
        QVERIFY(bytesAvailable > 0);

        if (m_buffer) { // may be 0 for certain tests
            int bufsize = m_buffer->buffer().size();
            m_buffer->buffer().resize(bufsize + bytesAvailable);
            qint64 size = m_client->read(m_buffer->buffer().data() + bufsize, bytesAvailable);
            QCOMPARE(size, bytesAvailable);

            QCOMPARE(m_client->bytesAvailable(), qint64(0));    // should now be cleared
        }

        // should get dataTransferProgress() before the first readyRead()
        QVERIFY(m_emittedSignals.contains("dataTransferProgress"));

        checkCurrentData(true);
    }

    void done(bool error)
    {
        m_emittedSignals << "done";
        QVERIFY(m_emittedSignals.contains("requestFinished"));
        checkCurrentData(false);

        QCOMPARE(m_client->error() != QObexClientSession::NoError, error);

        if (m_requests.size() > 0) {
            QCOMPARE(m_client->error(), m_requests.head().expectedError);

            // only check these if we actually got through to the server
            if (error == QObexClientSession::NoError || error == QObexClientSession::RequestFailed) {
                QCOMPARE(m_client->lastResponseCode(), m_serverSession->m_responseCode);
                QCOMPARE(m_client->lastResponseHeader(), m_serverSession->m_responseHeaders);
            }
        }

        // this should be the last request
        QVERIFY(!m_client->hasPendingRequests());
    }

private slots:

    void initTestCase()
    {
        m_server = new QTcpServer(this);
        bool b = m_server->listen(QHostAddress(QHostAddress::Any), 0);
        Q_ASSERT(b);

        m_clientSessionSocket = 0;
        m_serverSessionSocket = 0;
        m_client = 0;
        m_serverSession = 0;
    }

    void init()
    {
        QSignalSpy spy(m_server,SIGNAL(newConnection()));
        m_clientSessionSocket = new QTcpSocket;
        m_clientSessionSocket->connectToHost(m_server->serverAddress(),
                                             m_server->serverPort());
        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();

        m_serverSessionSocket = m_server->nextPendingConnection();
        m_serverSession = new MyObexServerSession(m_serverSessionSocket, this);

        m_requests.clear();
        m_emittedSignals.clear();

        m_buffer = 0;
        m_lastDone = 0;
        m_lastTotal = 0;
        m_usingIODevice = false;

        m_serverSession->reset();

        m_client = new QObexClientSession(m_clientSessionSocket, this);
        connect(m_client, SIGNAL(requestStarted(int)),
                SLOT(requestStarted(int)));
        connect(m_client, SIGNAL(requestFinished(int,bool)),
                SLOT(requestFinished(int,bool)));
        connect(m_client, SIGNAL(dataTransferProgress(qint64,qint64)),
                SLOT(dataTransferProgress(qint64,qint64)));
        connect(m_client, SIGNAL(responseHeaderReceived(QObexHeader)),
                SLOT(responseHeaderReceived(QObexHeader)));
        connect(m_client, SIGNAL(readyRead()),
                SLOT(readyRead()));
        connect(m_client, SIGNAL(done(bool)), SLOT(done(bool)));
    }

    void cleanup()
    {
        delete m_buffer;
        QObject::disconnect(m_client, 0, this, 0);
        delete m_client;
        delete m_serverSession;

        delete m_clientSessionSocket;
        delete m_serverSessionSocket;
    }

    void genericOpTests_data()
    {
        QTest::addColumn<QObex::Request>("op");
        QTest::addColumn<QObex::ResponseCode>("serverResponse");
        QTest::addColumn<QObexClientSession::Error>("expectedError");

        QList<QObex::Request> ops = getRequestList();
        for (int i=0; i<ops.size(); i++) {
            QString desc = QString("op %1 success").arg(ops[i]);
            QTest::newRow(QTest::toString(desc)) << ops[i]
                    << QObex::Success << QObexClientSession::NoError;
            desc = QString("op %1 non-success").arg(ops[i]);
            QTest::newRow(QTest::toString(desc)) << ops[i]
                    << QObex::Forbidden << QObexClientSession::RequestFailed;
        }
    }

    void genericOpTests()
    {
        QFETCH(QObex::Request, op);
        QFETCH(QObex::ResponseCode, serverResponse);
        QFETCH(QObexClientSession::Error, expectedError);

        m_serverSession->m_responseCode = serverResponse;

        // check attributes before the test
        QCOMPARE(m_client->currentId(), 0);
        QCOMPARE(m_client->currentRequest(), QObex::NoRequest);
        QCOMPARE(m_client->error(), QObexClientSession::NoError);
        QCOMPARE(m_client->lastResponseCode(), QObex::Success);
        QCOMPARE(m_client->lastResponseHeader(), QObexHeader());

        // send any old test header
        QObexHeader header;
        header.setName("a name");
        header.setTime(QDateTime::currentDateTime());

        // wait for request to finish
        QSignalSpy spy(m_client,SIGNAL(done(bool)));
        int id = doDefaultRequest(m_client, op, header);
        addRequest(id, op, expectedError);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << (serverResponse!=QObex::Success) );

        // check server received the right headers
        QCOMPARE(m_serverSession->m_receivedHeaders, header);

        // shouldn't have gotten dataTransferProgress() signal
        QStringList expectedSignals;
        expectedSignals << "requestStarted"
                << "responseHeaderReceived"
                 << "requestFinished" << "done";
        QCOMPARE(m_emittedSignals, expectedSignals);
    }


    // If you try to perform requests when the socket is not connected, the 
    // request should finish with TransportNotConnected.
    void genericOpTests_unconnected()
    {
        delete m_client;
        m_client = 0;

        QTcpSocket *unconnectedSocket = new QTcpSocket;
        m_client = new QObexClientSession(unconnectedSocket);

        QList<QObex::Request> ops = getRequestList();

        for (int i=0; i<ops.size(); i++) {
            // wait for request to finish
            QSignalSpy spy(m_client,SIGNAL(done(bool)));
            int id = doDefaultRequest(m_client, ops[i], QObexHeader());
            addRequest(id, ops[i], QObexClientSession::ConnectionError);
            QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << true );
        }

        delete unconnectedSocket;
        unconnectedSocket = 0;
    }

//----------------------- Put and Get tests follow ------------------------------

private:
    void fillTransferTestData()
    {
        QTest::addColumn<QByteArray>("bytes");
        QTest::addColumn<bool>("useIODevice");

        QTest::newRow("empty byte array") << QByteArray() << false;
        QTest::newRow("empty io device") << QByteArray() << true;
        QTest::newRow("small byte array") << QByteArray("abcdefg123456789") << false;
        QTest::newRow("small io device") << QByteArray("abcdefg123456789") << true;

        QByteArray b = get_some_data(10000);
        QTest::newRow("large byte array") << b << false;
        QTest::newRow("large io device") << b << true;
    }

private slots:

    void put_transfer_data()
    {
        fillTransferTestData();
    }

    void put_transfer()
    {
        QFETCH(QByteArray, bytes);
        QFETCH(bool, useIODevice);
        m_usingIODevice = useIODevice;

        QObexHeader header;
        header.setName("put file transfer test name");
        header.setType("put file transfer test type");

        int id = 0;
        m_buffer = new QBuffer(&bytes);
        if (useIODevice) {
            id = m_client->put(header, m_buffer);
        } else {
            id = m_client->put(header, bytes);
        }
        addRequest(id, QObex::Put, QObexClientSession::NoError);

        QSignalSpy spy(m_client,SIGNAL(done(bool)));
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        QCOMPARE(m_lastDone, qint64(bytes.size()));
        QCOMPARE(m_lastTotal, qint64(bytes.size()));
        QCOMPARE(m_serverSession->m_recvBuffer.buffer(), bytes);

        QCOMPARE(m_serverSession->m_receivedHeaders, header);

        // I don't think it should emit dataTransferProgress() if the bytes
        // size is zero...
        if (bytes.size() > 0)
            QVERIFY(m_emittedSignals.count("dataTransferProgress") >= 1);
        else
            QVERIFY(m_emittedSignals.count("dataTransferProgress") == 0);

        QCOMPARE(m_emittedSignals.count("responseHeaderReceived"), 1);
        checkStandardEmittedSignals();
    }

    // If you call put() with a null QIODevice, it should fail with InvalidRequest.
    void put_error()
    {
        QSignalSpy spy(m_client,SIGNAL(done(bool)));
        int id = m_client->put(QObexHeader(), 0);
        addRequest(id, QObex::Put, QObexClientSession::InvalidRequest);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << true );

        QCOMPARE(m_emittedSignals.count("dataTransferProgress"), 0);
        QCOMPARE(m_emittedSignals.count("responseHeaderReceived"), 0);  // didn't get to server
        checkStandardEmittedSignals();
    }

    void get_transfer_data()
    {
        fillTransferTestData();
    }

    void get_transfer()
    {
        QFETCH(QByteArray, bytes);
        QFETCH(bool, useIODevice);
        m_usingIODevice = useIODevice;

        m_serverSession->m_sendBuffer.setData(bytes);
        m_buffer = new QBuffer;

        QSignalSpy spy(m_client,SIGNAL(done(bool)));
        int id = m_client->get(QObexHeader(), ( m_usingIODevice ? m_buffer : 0 ));
        addRequest(id, QObex::Get, QObexClientSession::NoError);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        QCOMPARE(m_lastDone, qint64(bytes.size()));
        QCOMPARE(m_lastTotal, qint64(bytes.size()));
        QCOMPARE(m_buffer->data(), bytes);

        if (bytes.size() > 0) {
            // readyRead() only emitted if no IO device given.
            if (m_usingIODevice)
                QCOMPARE(m_emittedSignals.count("readyRead"), 0);
            else
                QVERIFY(m_emittedSignals.count("readyRead") >= 1);
            QVERIFY(m_emittedSignals.count("dataTransferProgress") >= 1);
        } else {
            // should not emit signals if not data is received.
            QCOMPARE(m_emittedSignals.count("readyRead"), 0);
            QCOMPARE(m_emittedSignals.count("dataTransferProgress"), 0);
        }

        QCOMPARE(m_emittedSignals.count("responseHeaderReceived"), 1);
        checkStandardEmittedSignals();
    }

protected:
    void get_transfer_readDataAtEnd(const QByteArray &bytes, bool readAll)
    {
        m_serverSession->m_sendBuffer.setData(bytes);
        QSignalSpy spy(m_client,SIGNAL(done(bool)));
        int id = m_client->get(QObexHeader());
        addRequest(id, QObex::Get, QObexClientSession::NoError);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        qint64 bytesAvailable = m_client->bytesAvailable();
        QCOMPARE(bytesAvailable, qint64(bytes.size()));

        if (readAll) {
            // test readAll()
            QCOMPARE(m_client->readAll(), bytes);
        } else {
            // test read()
            QByteArray b;
            b.resize(bytesAvailable);
            QCOMPARE(m_client->read(b.data(), bytesAvailable), bytesAvailable);
            QCOMPARE(b, bytes);
        }
    }


private slots:

    // Call read or readAll() at the end of the op instead of when readyRead() is emitted,
    // to check the data is queued correctly.

    void get_transfer_readAllAtEnd_data() { fillTransferTestData(); }
    void get_transfer_readAllAtEnd()
    {
        QFETCH(QByteArray, bytes);
        get_transfer_readDataAtEnd(bytes, true);
    }

    void get_transfer_readAtEnd_data() { fillTransferTestData(); }
    void get_transfer_readAtEnd()
    {
        QFETCH(QByteArray, bytes);
        get_transfer_readDataAtEnd(bytes, false);
    }

//----------------------- test abort() ------------------------------

protected slots:

    void fillAbortTestData()
    {
        QTest::addColumn<QByteArray>("bytes");
        QTest::addColumn<bool>("enoughDataToAbort");

        //QTest::newRow("empty byte array") << QByteArray() << false;
        //QTest::newRow("small byte array") << QByteArray("abcdefg123456789") << false;

        QTest::newRow("large byte array") << get_some_data(5000) << true;
        //QTest::newRow("even larger byte array") << get_some_data(10000) << true;
    }

    void abortCurrentOp()
    {
        if (!m_calledAbort)
            m_client->abort();
        m_calledAbort = true;
    }

    // The enoughDataToAbort value should be true if given bytes is big enough
    // to be split over multiple packets, since you can't send an Abort unless
    // you are sending multiple packets.
    void runAbortTest(QObex::Request op, const QByteArray &bytes, bool enoughDataToAbort)
    {
        Q_ASSERT(op == QObex::Put || op == QObex::Get);
        m_buffer = new QBuffer;
        m_buffer->setData(bytes);
        m_usingIODevice = true;

        // send abort as soon as we get first dataTransferProgress() signal
        m_calledAbort = false;
        connect(m_client, SIGNAL(dataTransferProgress(qint64,qint64)),
                SLOT(abortCurrentOp()));

        QObexClientSession::Error expectedError = (enoughDataToAbort ?
                QObexClientSession::Aborted : QObexClientSession::NoError );

        QSignalSpy spy(m_client,SIGNAL(done(bool)));
        int id;
        if (op == QObex::Put) {
            id = m_client->put(QObexHeader(), m_buffer);
            addRequest(id, QObex::Put, expectedError);
        } else {
            m_serverSession->m_sendBuffer.setData(bytes);
            id = m_client->get(QObexHeader(), m_buffer);
            addRequest(id, QObex::Get, expectedError);
        }
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << (expectedError!=QObexClientSession::NoError) );

        // Client should get Aborted error if the abort was possible.
        // Otherwise, the op should finish normally.
        if (enoughDataToAbort) {
            QVERIFY(m_serverSession->m_lastError = QObexServerSession::Aborted);
        } else {
            QVERIFY(m_serverSession->m_lastError = MyObexServerSession::noError());
        }
        QCOMPARE(m_client->error(), expectedError);

        // Data should have been sent successfully, even if there was only
        // one data packet and no Abort was sent.
        QBuffer *sendBuffer = (op == QObex::Put ? m_buffer : &m_serverSession->m_sendBuffer);
        QBuffer *recvBuffer = (op == QObex::Put ? &m_serverSession->m_recvBuffer : m_buffer);
        if (bytes.size() > 0) {
            QCOMPARE(m_emittedSignals.count("dataTransferProgress"), 1);
            QVERIFY(m_lastDone > 0);
            QVERIFY(sendBuffer->pos() >= m_lastDone);
            QVERIFY(recvBuffer->pos() > 0);
            QVERIFY(recvBuffer->pos() <= m_lastDone);
        } else {
            QCOMPARE(m_emittedSignals.count("dataTransferProgress"), 0);
            QCOMPARE(sendBuffer->pos(), qint64(0));
            QCOMPARE(recvBuffer->pos(), qint64(0));
        }

        // should still have emitted requestStarted, finished etc.
        checkStandardEmittedSignals();
    }

private slots:

    void abort_put_data() { fillAbortTestData(); }
    void abort_put()
    {
        QFETCH(QByteArray, bytes);
        QFETCH(bool, enoughDataToAbort);
        runAbortTest(QObex::Put, bytes, enoughDataToAbort);
    }

    void abort_get_data() { fillAbortTestData(); }
    void abort_get()
    {
        QFETCH(QByteArray, bytes);
        QFETCH(bool, enoughDataToAbort);
        runAbortTest(QObex::Get, bytes, enoughDataToAbort);
    }

    void abort_put_early()
    {
        // See what happens if you call abort() when it's not in the middle
        // of a multi-packet Put or Get.

        QSignalSpy spy(m_client,SIGNAL(done(bool)));
        m_client->connect();
        m_client->disconnect();
        m_client->abort();
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << true );

        // the Connect request should have started but not finished
        // successfully, and the Disconnect shouldn't have been done at all
        QVERIFY(m_emittedSignals.size() > 0);
        QCOMPARE(m_emittedSignals.count("requestStarted"), 1);
        QCOMPARE(m_emittedSignals.count("requestFinished"), 1);

        // should have finished with Aborted error
        QCOMPARE(m_client->error(), QObexClientSession::Aborted);
    }

//----------------------- test running multiple requests ------------------------------


protected slots:
    void sendclearPendingRequests()
    {
        m_client->clearPendingRequests();

        // indicate we don't expect further requests, otherwise tests fail
        Request r = m_requests.head();
        int index = m_requests.indexOf(r);
        while (m_requests.size() > index + 1)
            m_requests.takeLast();
    }

    void sendAbort()
    {
        m_client->abort();
/*
        // indicate we don't expect further requests, otherwise tests fail
        Request r = m_requests.head();
        int index = m_requests.indexOf(r);
        while (m_requests.size() > index + 1)
            m_requests.takeLast();
*/
    }

private slots:

    void multipleOps_normal()
    {
        QByteArray putBytes = get_some_data(10000);
        QByteArray getBytes = get_some_data(10000);
        m_serverSession->m_sendBuffer.setData(getBytes);

        int id;

        QSignalSpy spy(m_client,SIGNAL(done(bool)));

        id = m_client->connect();
        addRequest(id, QObex::Connect, QObexClientSession::NoError);

        id = m_client->put(QObexHeader(), putBytes);
        addRequest(id, QObex::Put, QObexClientSession::NoError);

        id = m_client->put(QObexHeader(), get_some_data(15000));
        addRequest(id, QObex::Put, QObexClientSession::NoError);

        id = m_client->get(QObexHeader());
        addRequest(id, QObex::Get, QObexClientSession::NoError);

        id = m_client->disconnect();
        addRequest(id, QObex::Disconnect, QObexClientSession::NoError);

        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );
    }

    void multipleOps_cancelPending()
    {
        QByteArray putBytes = get_some_data(10000);
        QByteArray getBytes = get_some_data(10000);
        m_serverSession->m_sendBuffer.setData(getBytes);

        // call clearPendingRequests() when the Put request gets going
        connect(m_client, SIGNAL(dataTransferProgress(qint64,qint64)),
                SLOT(sendclearPendingRequests()));

        int id;

        QSignalSpy spy(m_client,SIGNAL(done(bool)));

        id = m_client->connect();
        addRequest(id, QObex::Connect, QObexClientSession::NoError);

        id = m_client->put(QObexHeader(), putBytes);
        addRequest(id, QObex::Put, QObexClientSession::NoError);

        id = m_client->get(QObexHeader());
        addRequest(id, QObex::Get, QObexClientSession::NoError);

        id = m_client->disconnect();
        addRequest(id, QObex::Disconnect, QObexClientSession::NoError);

        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        // The Get and Disconnect requests should have been cleared, but the
        // Put should have kept going.
        QVERIFY(m_emittedSignals.count("dataTransferProgress") > 1);
        QCOMPARE(m_serverSession->m_recvBuffer.data(), putBytes);

        QCOMPARE(m_emittedSignals.count("requestStarted"), 2);
        QCOMPARE(m_emittedSignals.count("requestFinished"), 2);
    }

    void multipleOps_abort()
    {
        QByteArray putBytes = get_some_data(10000);
        QByteArray getBytes = get_some_data(10000);
        m_serverSession->m_sendBuffer.setData(getBytes);

        // call abort() when the Put request gets going
        connect(m_client, SIGNAL(dataTransferProgress(qint64,qint64)),
                SLOT(sendAbort()));

        int id;

        QSignalSpy spy(m_client,SIGNAL(done(bool)));

        id = m_client->connect();
        addRequest(id, QObex::Connect, QObexClientSession::NoError);

        id = m_client->put(QObexHeader(), putBytes);
        addRequest(id, QObex::Put, QObexClientSession::Aborted);

        id = m_client->get(QObexHeader());
        addRequest(id, QObex::Get, QObexClientSession::NoError);

        id = m_client->disconnect();
        addRequest(id, QObex::Disconnect, QObexClientSession::NoError);

        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << true );

        // Put should have been aborted after first data packet.
        QCOMPARE(m_emittedSignals.count("dataTransferProgress"), 1);
        QVERIFY(m_serverSession->m_recvBuffer.pos() > 0);
        QVERIFY(m_serverSession->m_recvBuffer.pos() <= m_lastDone);

        // Get and Disconnect should have been cancelled.
        QCOMPARE(m_emittedSignals.count("requestStarted"), 2);
        QCOMPARE(m_emittedSignals.count("requestFinished"), 2);

        QCOMPARE(m_client->error(), QObexClientSession::Aborted);
    }


//----------------------- test connection ID handling ------------------------------

private slots:

    void connectionId()
    {
        quint32 connectionId = 5008;
        m_serverSession->m_responseHeaders.setConnectionId(connectionId);

        QSignalSpy spy(m_client,SIGNAL(done(bool)));
        int id = m_client->connect(QObexHeader());
        addRequest(id, QObex::Connect, QObexClientSession::NoError);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        QCOMPARE(m_client->connectionId(), connectionId);
        QVERIFY(m_client->hasConnectionId());
        QVERIFY(m_client->lastResponseHeader().contains(QObexHeader::ConnectionId));
        QCOMPARE(m_client->lastResponseHeader().connectionId(), connectionId);
    }

    void connectionId_none()
    {
        QSignalSpy spy(m_client,SIGNAL(done(bool)));
        int id = m_client->connect(QObexHeader());
        addRequest(id, QObex::Connect, QObexClientSession::NoError);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        QCOMPARE(m_client->connectionId(), quint32(0));
        QVERIFY(!m_client->hasConnectionId());
        QVERIFY(!m_client->lastResponseHeader().contains(QObexHeader::ConnectionId));
    }

//---------------------------------------------------------------------------

protected slots:
    void closeClientTransport()
    {
        m_clientSessionSocket->close();
    }

    void closeServerTransport()
    {
        m_serverSessionSocket->close();
    }

    void deleteClientTransport()
    {
        delete m_clientSessionSocket;
        m_clientSessionSocket = 0;
    }

    void deleteServerTransport()
    {
        delete m_serverSessionSocket;
        m_serverSessionSocket = 0;
    }

private slots:

    void interruptedTransport_data()
    {
        QTest::addColumn<QString>("slotName");

        QTest::newRow("close client transport") << QString(SLOT(closeClientTransport()));

        // Can't test this, we won't be told at the QIODevice level if the
        // other side of a QTcpSocket connection closes.
        //QTest::newRow("close server transport") << QString(SLOT(closeServerTransport()));

        // these might not be necessary, since we know that QTcpSocket calls
        // QIODevice::close() if an operation is in progress when it is deleted.
        QTest::newRow("delete client transport") << QString(SLOT(deleteClientTransport()));

        // Can't test this, we won't be told at the QIODevice level if the
        // other side of a QTcpSocket connection closes.
        //QTest::newRow("delete server transport") << QString(SLOT(deleteServerTransport()));
    }

    void interruptedTransport()
    {
        QFETCH(QString, slotName);

        QPointer<QIODevice> pointer;
        if (slotName.contains("delete")) {
            if (slotName.contains("Client"))
                pointer = m_client->sessionDevice();
            else if (slotName.contains("Server"))
                pointer = m_serverSession->sessionDevice();
            else
                Q_ASSERT(false);
            QVERIFY(pointer != 0);
        }

        // send a multi-packet request and close the server or client TCP socket halfway
        QSignalSpy spy(m_client,SIGNAL(done(bool)));
        int id = m_client->put(QObexHeader(), get_some_data(5000));
        connect(m_client, SIGNAL(dataTransferProgress(qint64,qint64)),
                slotName.toLatin1().constData());
        addRequest(id, QObex::Put, QObexClientSession::ConnectionError);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << true );

        // see if the QIODevice or obex socket really was closed/deleted
        if (slotName.contains("close")) {
            QVERIFY(!m_client->sessionDevice()->isOpen());
        } else if (slotName.contains("delete")) {
            QVERIFY(pointer == 0);
        } else {
            Q_ASSERT(false);
        }

        QCOMPARE(m_client->error(), QObexClientSession::ConnectionError);
    }

};

QTEST_MAIN(tst_QObexClientSession)
#include "tst_qobexclientsession.moc"
