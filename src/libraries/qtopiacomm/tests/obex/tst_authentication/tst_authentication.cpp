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
#include <QObexClientSession>
#include <QObexServerSession>
#include <QTcpSocket>
#include <QTcpServer>

#include <QObexAuthenticationChallenge>
#include <QObexAuthenticationResponse>

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <shared/util.h>
#include <QDebug>
#include <QList>
#include <QStringList>
#include <QTimer>

const QString RIGHT_PASSWORD = "open sesame";
const QString WRONG_PASSWORD = "just plain wrong";
const QString DONT_SEND_THIS_PASSWORD = "";

//TESTED_CLASS=
//TESTED_FILES=

class MyClientProxy : public QObject
{
    Q_OBJECT

public:
    QObexClientSession *m_client;
    QStringList m_emittedSignals;
    QString m_password;

    MyClientProxy(QObexClientSession *client)
        : m_client(client)
    {
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

        connect(m_client, SIGNAL(authenticationRequired(QObexAuthenticationChallenge*)),
                SLOT(authenticationRequired(QObexAuthenticationChallenge*)));
        connect(m_client, SIGNAL(authenticationResponse(const QObexAuthenticationResponse &,
                                                        bool*)),
                SLOT(authenticationResponse(QObexAuthenticationResponse,bool*)));
    }

private slots:
    void requestStarted(int) { m_emittedSignals << "requestStarted"; }
    void requestFinished(int, bool) { m_emittedSignals << "requestFinished"; }
    void responseHeaderReceived(const QObexHeader &) { m_emittedSignals << "responseHeaderReceived"; }
    void dataTransferProgress(qint64, qint64) { m_emittedSignals << "dataTransferProgress"; }
    void readyRead() { m_emittedSignals << "readyRead"; }
    void done(bool) { m_emittedSignals << "done"; }

    void authenticationRequired(QObexAuthenticationChallenge *challenge)
    {
        m_emittedSignals << "authenticationRequired";
        if (m_password != DONT_SEND_THIS_PASSWORD)
            challenge->setPassword(m_password);
    }

    void authenticationResponse(const QObexAuthenticationResponse &response, bool *accept)
    {
        m_emittedSignals << "authenticationResponse";
        *accept = (response.match(RIGHT_PASSWORD));
    }
};


class MyServerSession : public QObexServerSession
{
    Q_OBJECT
public:
    QObex::ResponseCode m_serverResponse;
    QObexHeader m_responseHeader;
    QObexHeader m_receivedRequestHeader;
    QStringList m_serverCalls;
    QObexServerSession::Error m_error;

    QString m_password;
    bool m_authenticateClients;
    bool m_clientIsAuthenticated;

    static const QObexServerSession::Error NoError;

    MyServerSession(QIODevice *device, QObject *parent = 0)
        : QObexServerSession(device, parent)
    {
        QObject::connect(this, SIGNAL(authenticationRequired(QObexAuthenticationChallenge*)),
                SLOT(authRequired(QObexAuthenticationChallenge*)));
        QObject::connect(this, SIGNAL(authenticationResponse(QObexAuthenticationResponse,bool*)),
                SLOT(authResponse(QObexAuthenticationResponse,bool*)));

        m_error = NoError;
        m_authenticateClients = false;
        m_clientIsAuthenticated = false;
    }

protected:
    virtual void error(QObexServerSession::Error error, const QString &errorString)
    {
        m_error = error;
        qWarning("OBEX server error (%d): %s", error, errorString.toLatin1().constData());
    }

private slots:
    void authRequired(QObexAuthenticationChallenge *challenge)
    {
        m_serverCalls << "authenticationRequired";
        if (m_password != DONT_SEND_THIS_PASSWORD)
            challenge->setPassword(m_password);
    }

    void authResponse(const QObexAuthenticationResponse &response, bool *accept)
    {
        m_serverCalls << "authenticationResponse";
        *accept = (response.match(RIGHT_PASSWORD));
        if (*accept)
            m_clientIsAuthenticated = true;
    }

protected slots:
    QObex::ResponseCode connect(const QObexHeader &header)
    {
        m_serverCalls << "connect";
        m_receivedRequestHeader = header;

        if (m_authenticateClients && !m_clientIsAuthenticated) {
            QObexHeader h = m_responseHeader;
            h.setAuthenticationChallenge();
            setNextResponseHeader(h);
            return QObex::Unauthorized;
        }

        setNextResponseHeader(m_responseHeader);
        return m_serverResponse;
    }
};
const QObexServerSession::Error MyServerSession::NoError = QObexServerSession::Error(-1);



class tst_authentication : public QObject
{
    Q_OBJECT

private:
    QTcpSocket *m_clientSessionSocket, *m_serverSessionSocket;
    MyClientProxy *m_clientProxy;
    MyServerSession *m_serverSession;

    // Put any old data into a header.
    void fillTestHeader(QObexHeader &header)
    {
        header.setName("a test header");
        header.setTime(QDateTime::currentDateTime());
    }

private slots:

    void initTestCase()
    {
        QTcpServer *server = new QTcpServer(this);
        bool b = server->listen(QHostAddress(QHostAddress::Any), 0);
        Q_ASSERT(b);

        QSignalSpy spy(server,SIGNAL(newConnection()));
        m_clientSessionSocket = new QTcpSocket;
        m_clientSessionSocket->connectToHost(server->serverAddress(), server->serverPort());
        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();

        m_serverSessionSocket = server->nextPendingConnection();
        Q_ASSERT(m_serverSessionSocket != 0);
    }

    void init()
    {
        QObexClientSession *client = new QObexClientSession(m_clientSessionSocket);
        m_clientProxy = new MyClientProxy(client);
        client->setParent(m_clientProxy);

        m_serverSession = new MyServerSession(m_serverSessionSocket);
    }

    void cleanup()
    {
        delete m_clientProxy;
        delete m_serverSession;
    }


private:

    /*
    Creates test data for the scenario where the client sends a request with 
    an Authentication Challenge. What should happen if:

    1. Everything goes ok - server responds with correct password
    2. Server responds with wrong password, fails authentication
    3. Server rejects the request without sending an Authentication Response
       (e.g. just responds with NotFound, Forbidden, etc.) This is possible
        because authenticationRequired() is emitted *after* the request slot
        invocation.
    4. Server accepts the request but ignores the Authentication Challenge.
    */
    void fillData_clientSendsChallenge()
    {
        QStringList changedClientSignals;

        // Expected signals if everything goes ok.
        // This expects that the server *will* send some response
        // headers back to the client (not just a response code).
        QStringList clientSignals;
        clientSignals << "requestStarted"
                << "authenticationResponse" << "responseHeaderReceived"
                << "requestFinished" << "done";

        // Expected server function calls/signals if everything goes ok.
        QStringList serverCalls;
        serverCalls << "connect" << "authenticationRequired";

        // Test if everything goes ok.
        QTest::newRow("server authenticates OK")
                << true << false
                << RIGHT_PASSWORD
                << clientSignals << serverCalls
                << QObexClientSession::NoError << MyServerSession::NoError
                << QObex::Success << QObex::Success;

        // If server fails authentication, the client should error with
        // AuthenticationFailed, and it shouldn't receive the
        // response headers.
        changedClientSignals = clientSignals;
        changedClientSignals.removeAll("responseHeaderReceived");
        QTest::newRow("server fails authentication")
                << true << false
                << WRONG_PASSWORD
                << changedClientSignals << serverCalls
                << QObexClientSession::AuthenticationFailed << MyServerSession::NoError
                << QObex::Success << QObex::Success;

        // If server rejects the request at connect() callback, before it
        // gets the authenticationRequired() signal, client should just fail
        // normally with RequestFailed error (not AuthenticationFailed), and
        // still receive server response headers.
        changedClientSignals = clientSignals;
        changedClientSignals.removeAll("authenticationResponse");
        QTest::newRow("server rejects request early")
                << true << false
                << ""
                << changedClientSignals << QStringList("connect")
                << QObexClientSession::RequestFailed << MyServerSession::NoError
                << QObex::Forbidden << QObex::Forbidden;

        // If server ignores Authentication Challenge, it should send back
        // InternalServerError and its error() should get called with
        // AuthenticationFailed, and the client should not receive the response
        // headers (since the server slot would have returned QObex::Success,
        // and the headers probably depended on the response being Success).
        changedClientSignals = clientSignals;
        changedClientSignals.removeAll("authenticationResponse");
        changedClientSignals.removeAll("responseHeaderReceived");
        QTest::newRow("server ignores authentication challenge")
                << true << false
                << DONT_SEND_THIS_PASSWORD
                << changedClientSignals << serverCalls
                << QObexClientSession::RequestFailed << MyServerSession::AuthenticationFailed
                << QObex::Success << QObex::InternalServerError;
    }

    /*
    Creates test data for the scenario where the client sends a request and 
    the server responds with an Authentication Challenge. What should happen if:

    1. Everything goes ok - client sends another request with correct password
    2. Client sends another request with wrong password, fails authentication
    3. Client sends another request with correct password, but server still 
       rejects the request (e.g. just responds with NotFound, Forbidden, etc.)
    4. Client ignores Authentication Challenge (doesn't set user/password).
    */
    void fillData_serverRespondsWithChallenge()
    {
        // Expected signals if everything goes ok.
        // This expects that the server *will* send some response
        // headers back to the client (not just a response code).
        QStringList clientSignals;
        clientSignals << "requestStarted" 
                << "authenticationRequired" << "responseHeaderReceived"
                << "requestFinished" << "done";

        // Expected server function calls/signals if everything goes ok.
        QStringList serverCalls;
        serverCalls << "connect" << "authenticationResponse" << "connect";

        // Test if everything ok.
        QTest::newRow("client authenticates ok")
                << false << true
                << RIGHT_PASSWORD
                << clientSignals << serverCalls
                << QObexClientSession::NoError << MyServerSession::NoError
                << QObex::Success << QObex::Success;

        // If client fails authentication, it should realise this from the
        // server's response, and fail with AuthenticationFailed. Client
        // will not receive the server response headers since it failed.
        // Server's connect() shouldn't get called a 2nd time because
        // authentication failed, and server error() should get notified of
        // the authentication failure.
        QStringList changedClientSignals = clientSignals;
        changedClientSignals.removeAll("responseHeaderReceived");
        QStringList changedServerCalls = serverCalls;
        changedServerCalls.removeLast();
        QTest::newRow("client fails authentication")
                << false << true
                << WRONG_PASSWORD
                << changedClientSignals << changedServerCalls
                << QObexClientSession::AuthenticationFailed << QObexServerSession::AuthenticationFailed
                << QObex::Success << QObex::Unauthorized;

        // If client passes authentication but server still rejects the
        // request (e.g. with NotFound, Forbidden, etc.) then the request
        // finishes with just the RequestFailed error, and client should
        // receive the server response headers.
        QTest::newRow("client authenticates ok but server still rejects")
                << false << true
                << RIGHT_PASSWORD
                << clientSignals << serverCalls
                << QObexClientSession::RequestFailed << MyServerSession::NoError
                << QObex::NotFound << QObex::NotFound;

        // If client ignores auth challenge, it shouldn't send the
        // Authentication Response, and should error with
        // AuthenticationFailed. It shouldn't receive the server response
        // headers in case these are dependent on a successful request
        // (although the server shouldn't send any other headers when it
        // sends a challenge anyway).
        changedClientSignals = clientSignals;
        changedClientSignals.removeAll("responseHeaderReceived");
        QTest::newRow("client ignores authentication challenge")
                << false << true
                << DONT_SEND_THIS_PASSWORD
                << changedClientSignals << QStringList("connect")
                << QObexClientSession::AuthenticationFailed << MyServerSession::NoError
                << QObex::Success << QObex::Success;
    }

    /*
    Creates test data for the scenario where both the client and server
    authenticate each other.
    This only runs the one test since the other tests cover other situations
    where authentication fails, etc.
    */
    void fillData_bothPartiesChallenge()
    {
        // Expected signals if everything goes ok.
        // This expects that the server *will* send some response
        // headers back to the client (not just a response code).
        QStringList clientSignals;
        clientSignals << "requestStarted"  
                << "authenticationRequired" << "authenticationResponse"
                << "responseHeaderReceived"
                << "requestFinished" << "done";

        // Expected server function calls/signals if everything goes ok.
        QStringList serverCalls;
        serverCalls << "connect" << "authenticationResponse"
                << "connect" << "authenticationRequired";

        // Test if everything ok.
        QTest::newRow("client and server both authenticate ok")
                << true << true
                << RIGHT_PASSWORD
                << clientSignals << serverCalls
                << QObexClientSession::NoError << MyServerSession::NoError
                << QObex::Success << QObex::Success;
    }

private slots:

    void testAuthenticationScenarios_data()
    {
        QTest::addColumn<bool>("clientSendsChallenge");
        QTest::addColumn<bool>("serverSendsChallenge");
        QTest::addColumn<QString>("testPassword");
        QTest::addColumn<QStringList>("expectedClientSignals");
        QTest::addColumn<QStringList>("expectedServerCalls");
        QTest::addColumn<QObexClientSession::Error>("expectedClientError");
        QTest::addColumn<QObexServerSession::Error>("expectedServerError");
        QTest::addColumn<QObex::ResponseCode>("initialServerResponse");
        QTest::addColumn<QObex::ResponseCode>("lastServerResponse");

        fillData_clientSendsChallenge();
        fillData_serverRespondsWithChallenge();
        fillData_bothPartiesChallenge();
    }

    void testAuthenticationScenarios()
    {
        QFETCH(bool, clientSendsChallenge);
        QFETCH(bool, serverSendsChallenge);
        QFETCH(QString, testPassword);
        QFETCH(QStringList, expectedClientSignals);
        QFETCH(QStringList, expectedServerCalls);
        QFETCH(QObexClientSession::Error, expectedClientError);
        QFETCH(QObexServerSession::Error, expectedServerError);
        QFETCH(QObex::ResponseCode, initialServerResponse);
        QFETCH(QObex::ResponseCode, lastServerResponse);

        QObexHeader requestHeader = QObexHeader();
        fillTestHeader(requestHeader);
        if (clientSendsChallenge)
            requestHeader.setAuthenticationChallenge();

        m_serverSession->m_authenticateClients = serverSendsChallenge;
        m_serverSession->m_responseHeader = QObexHeader();
        fillTestHeader(m_serverSession->m_responseHeader);

        m_serverSession->m_password = testPassword;
        m_clientProxy->m_password = testPassword;

        m_serverSession->m_serverResponse = initialServerResponse;

        QSignalSpy spy(m_clientProxy->m_client,SIGNAL(done(bool)));
        m_clientProxy->m_client->connect(requestHeader);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << (expectedClientError!=QObexClientSession::NoError) );

        QCOMPARE(m_clientProxy->m_client->error(), expectedClientError);
        QCOMPARE(m_clientProxy->m_client->lastResponseCode(), lastServerResponse);
        QCOMPARE(m_clientProxy->m_emittedSignals, expectedClientSignals);
        QCOMPARE(m_serverSession->m_serverCalls, expectedServerCalls);
        QCOMPARE(m_serverSession->m_error, expectedServerError);

        // client should not see any Auth Challenge or Auth Response headers
        if (expectedClientSignals.contains("responseHeaderReceived")) {
            m_serverSession->m_responseHeader.remove(QObexHeader::AuthChallenge);
            m_serverSession->m_responseHeader.remove(QObexHeader::AuthResponse);
            QCOMPARE(m_clientProxy->m_client->lastResponseHeader(),
                     m_serverSession->m_responseHeader);
        }

        // server should not see any Auth Challenge or Auth Response headers
        requestHeader.remove(QObexHeader::AuthChallenge);
        requestHeader.remove(QObexHeader::AuthResponse);
        QCOMPARE(m_serverSession->m_receivedRequestHeader, requestHeader);
    }
};

QTEST_MAIN(tst_authentication)
#include "tst_authentication.moc"
