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
#include <qtcpsocket.h>
#include <qtcpserver.h>
#include <qobexheader.h>
#include <qobexpushclient.h>
#include <qobexserversession.h>

#include <qtopiaapplication.h>

#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QSignalSpy>
#include <shared/util.h>
#include <qmetatype.h>

#include <QBuffer>
#include <QDebug>
#include <QQueue>
#include <QList>
#include <QTimer>

//TESTED_CLASS=QObexPushClient
//TESTED_FILES=src/libraries/qtopiacomm/obex/qobexpushclient.cpp

Q_DECLARE_USER_METATYPE_ENUM(QObexPushClient::Error);
Q_IMPLEMENT_USER_METATYPE_ENUM(QObexPushClient::Error);
Q_DECLARE_METATYPE(QObexHeader);

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


class MyObexServerSession : public QObexServerSession
{
    Q_OBJECT
public:
    QList<QVariant> m_responseCodes;
    QList<QObexServerSession::Error> m_errors;
    QList<QObexHeader> m_requestHeaders;
    QBuffer m_sendBuffer;
    QBuffer m_recvBuffer;
    QByteArray m_lastSentBytes;

    MyObexServerSession(QIODevice *device, QObject *parent = 0)
        : QObexServerSession(device, parent)
    {
    }

protected:
    virtual void error(QObexServerSession::Error error)
    {
        m_errors << error;
    }

    virtual QObex::ResponseCode dataAvailable(const char *data, qint64 size)
    {
        qLog(Autotest) << "MyObexServerSession::dataAvailable()" << size;
        Q_ASSERT(m_responseCodes.size() > 0);
        if (size == 0) {
            m_recvBuffer.close();
            return m_responseCodes.takeFirst().value<QObex::ResponseCode>();
        } else {
            m_recvBuffer.write(data, size);
            return m_responseCodes.first().value<QObex::ResponseCode>();
        }
    }

    virtual QObex::ResponseCode provideData(const char **data, qint64 *size)
    {
        qLog(Autotest) << "MyObexServerSession::provideData()";
        Q_ASSERT(m_responseCodes.size() > 0);
        if (m_sendBuffer.atEnd()) {
            *size = 0;
            m_lastSentBytes.clear();
            m_sendBuffer.close();
            return m_responseCodes.takeFirst().value<QObex::ResponseCode>();
        } else {
            m_lastSentBytes = m_sendBuffer.read(1024);
            if (m_lastSentBytes.size() > 0) {
                *data = m_lastSentBytes.constData();
                *size = m_lastSentBytes.size();
            }
            return m_responseCodes.first().value<QObex::ResponseCode>();
        }
    }

protected slots:
    QObex::ResponseCode connect(const QObexHeader &header)
    {
        qLog(Autotest) << "MyObexServerSession::connect()";
        m_requestHeaders << header;
        Q_ASSERT(m_responseCodes.size() > 0);
        return m_responseCodes.takeFirst().value<QObex::ResponseCode>();
    }

    QObex::ResponseCode disconnect(const QObexHeader &header)
    {
        qLog(Autotest) << "MyObexServerSession::disconnect()";
        m_requestHeaders << header;
        Q_ASSERT(m_responseCodes.size() > 0);
        return m_responseCodes.takeFirst().value<QObex::ResponseCode>();
    }

    QObex::ResponseCode put(const QObexHeader &header)
    {
        qLog(Autotest) << "MyObexServerSession::put()";
        m_requestHeaders << header;
        m_recvBuffer.open(QIODevice::WriteOnly);
        Q_ASSERT(m_responseCodes.size() > 0);
        return m_responseCodes.first().value<QObex::ResponseCode>();
    }

    QObex::ResponseCode get(const QObexHeader &header)
    {
        qLog(Autotest) << "MyObexServerSession::get()";
        m_requestHeaders << header;
        m_sendBuffer.open(QIODevice::ReadOnly);
        Q_ASSERT(m_responseCodes.size() > 0);
        return m_responseCodes.first().value<QObex::ResponseCode>();
    }
};

class PushClientProxy : public QObject
{
    Q_OBJECT

public:
    QObexPushClient *m_client;
    QStringList m_emittedSignals;
    QList<QObexPushClient::Error> m_errors;
    QQueue<int> m_ids;
    QQueue<QObexPushClient::Command> m_cmds;

    PushClientProxy(QIODevice *device, QObject *parent = 0)
        : QObject(parent),
          m_client(new QObexPushClient(device))
    {
        connect(m_client, SIGNAL(commandStarted(int)),
                SLOT(commandStarted(int)));
        connect(m_client, SIGNAL(commandFinished(int,bool)),
                SLOT(commandFinished(int,bool)));
        connect(m_client, SIGNAL(dataTransferProgress(qint64,qint64)),
                SLOT(dataTransferProgress(qint64,qint64)));
        connect(m_client, SIGNAL(done(bool)), SLOT(done(bool)));
    }

    void addTestCommand(int id, QObexPushClient::Command cmd)
    {
        m_ids << id;
        m_cmds << cmd;
    }

private:
    void checkCurrentId(int expected, int actual)
    {
        QCOMPARE(expected, actual);
        QCOMPARE(m_client->currentId(), actual);
    }

private slots:
    void commandStarted(int id)
    {
        m_emittedSignals << "commandStarted";
        if (m_ids.size() > 0)
            checkCurrentId(m_ids.head(), id);
        if (m_cmds.size() > 0)
            QCOMPARE(m_cmds.head(), m_client->currentCommand());
    }
    void commandFinished(int id, bool)
    {
        m_emittedSignals << "commandFinished";
        m_errors << m_client->error();
        if (m_ids.size() > 0)
            checkCurrentId(m_ids.takeFirst(), id);
        if (m_cmds.size() > 0)
            QCOMPARE(m_cmds.takeFirst(), m_client->currentCommand());
    }
    void dataTransferProgress(qint64, qint64)
    {
        m_emittedSignals << "dataTransferProgress";
    }
    void done(bool error)
    {
        m_emittedSignals << "done";
        qLog(Autotest) << "PushClientProxy::done()" << error << m_client->error();

        QCOMPARE(m_client->currentId(), 0);
        QCOMPARE(m_client->currentCommand(), QObexPushClient::None);
    }
};

enum PushCommand
{
    Connect,
    Disconnect,
    SendIODevice,
    SendByteArray,
    SendBusinessCard,
    RequestBusinessCard,
    ExchangeBusinessCard
};

Q_DECLARE_USER_METATYPE_ENUM(PushCommand);
Q_IMPLEMENT_USER_METATYPE_ENUM(PushCommand);

class tst_QObexPushClient : public QObject
{
    Q_OBJECT

private:
    QTcpServer *m_tcpServer;
    QTcpSocket *m_clientSocket;
    QTcpSocket *m_serverSocket;

    PushClientProxy *m_proxy;
    MyObexServerSession *m_serverSession;

    QBuffer m_sendBuffer;

private slots:

    void initTestCase()
    {
        m_tcpServer = new QTcpServer(this);
        bool b = m_tcpServer->listen(QHostAddress(QHostAddress::Any), 0);
        Q_ASSERT(b);

        m_clientSocket = 0;
        m_serverSocket = 0;
        m_proxy = 0;
        m_serverSession = 0;
    }

    void init()
    {
        QSignalSpy spy(m_tcpServer,SIGNAL(newConnection()));
        m_clientSocket = new QTcpSocket;
        m_clientSocket->connectToHost(m_tcpServer->serverAddress(),
                                      m_tcpServer->serverPort());
        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();

        m_serverSocket = m_tcpServer->nextPendingConnection();
        Q_ASSERT(m_serverSocket != 0);

        m_proxy = new PushClientProxy(m_clientSocket, this);
        m_serverSession = new MyObexServerSession(m_serverSocket, this);
        m_sendBuffer.open(QIODevice::ReadWrite);
    }

    void cleanup()
    {
        delete m_clientSocket;
        delete m_serverSocket;
        delete m_proxy;
        delete m_serverSession;
        m_sendBuffer.buffer().clear();
    }

    void initialState()
    {
        QCOMPARE(m_proxy->m_client->error(), QObexPushClient::NoError);
        QCOMPARE(m_proxy->m_client->currentId(), 0);
        QCOMPARE(m_proxy->m_client->currentCommand(), QObexPushClient::None);
    }

    void testGenericSignals_data()
    {
        QTest::addColumn< QList<QVariant> >("commands");
        QTest::addColumn< QList<QVariant> >("serverResponses");
        QTest::addColumn<QObexPushClient::Error>("expectedError");
        QTest::addColumn<QStringList>("expectedSignals");

        QStringList singleCommandSignals;
        singleCommandSignals << "commandStarted" << "commandFinished" << "done";

        QTest::newRow("connect, successful")
                << (QList<QVariant>() << Connect)
                << (QList<QVariant>() << qVariantFromValue(QObex::Success))
                << QObexPushClient::NoError
                << singleCommandSignals;

        QTest::newRow("connect, unsuccessful")
                << (QList<QVariant>() << Connect)
                << (QList<QVariant>() << qVariantFromValue(QObex::Forbidden))
                << QObexPushClient::RequestFailed
                << singleCommandSignals;

        QTest::newRow("disconnect, successful")
                << (QList<QVariant>() << Disconnect)
                << (QList<QVariant>() << qVariantFromValue(QObex::Success))
                << QObexPushClient::NoError
                << singleCommandSignals;

        // Most streaming commands will behave the same way.
        QList<PushCommand> streamCmds;
        streamCmds << SendIODevice << SendByteArray << SendBusinessCard << RequestBusinessCard;
        for (int i=0; i<streamCmds.size(); i++) {
            QTest::newRow(QTest::toString(QString("send %1, successful").arg(streamCmds[i])))
                    << (QList<QVariant>() << streamCmds[i])
                    << (QList<QVariant>() << qVariantFromValue(QObex::Success))
                    << QObexPushClient::NoError
                    << singleCommandSignals;

            QTest::newRow(QTest::toString(QString("send %1, unsuccessful").arg(streamCmds[i])))
                    << (QList<QVariant>() << streamCmds[i])
                    << (QList<QVariant>() << qVariantFromValue(QObex::Forbidden))
                    << QObexPushClient::RequestFailed
                    << singleCommandSignals;
        }

        QTest::newRow("exchangeBusinessCard, successful")
                << (QList<QVariant>() << ExchangeBusinessCard)
                << (QList<QVariant>() << qVariantFromValue(QObex::Success) << qVariantFromValue(QObex::Success))
                << QObexPushClient::NoError
                << (QStringList() << "commandStarted" << "commandFinished"
                                  << "commandStarted" << "commandFinished" << "done");

        // The Get part of the exchange doesn't start if the Put part fails.
        QTest::newRow("exchangeBusinessCard, unsuccessful")
                << (QList<QVariant>() << ExchangeBusinessCard)
                << (QList<QVariant>() << qVariantFromValue(QObex::Forbidden))
                << QObexPushClient::RequestFailed
                << (QStringList() << "commandStarted" << "commandFinished" << "done");


        // Run some tests for when multiple commands are queued:

        // Connect -> Disconnect
        // Should get signals in this order:
        //  commandStarted(), commandFinished(),
        //  commandStarted(), commandFinished(), done()
        QTest::newRow("connect+disconnect, successful")
                << (QList<QVariant>() << Connect << Disconnect)
                << (QList<QVariant>() << qVariantFromValue(QObex::Success) << qVariantFromValue(QObex::Success))
                << QObexPushClient::NoError
                << (QStringList() << "commandStarted" << "commandFinished"
                                  << "commandStarted" << "commandFinished" << "done");

        // Connect -> Disconnect, but Connect is refused, so Disconnect isn't performed
        QTest::newRow("connect+disconnect, disconnect successful")
                << (QList<QVariant>() << Connect << Disconnect)
                << (QList<QVariant>() << qVariantFromValue(QObex::Forbidden) << qVariantFromValue(QObex::Forbidden))
                << QObexPushClient::RequestFailed
                << (QStringList() << "commandStarted" << "commandFinished" << "done");

        // Note the dataTransferProgress() signal hasn't been tested here. It is tested
        // by the QObexClientSession tests, but should probably add a
        // separate test in this file.
    }

    void testGenericSignals()
    {
        QFETCH(QList<QVariant>, commands);
        QFETCH(QList<QVariant>, serverResponses);
        QFETCH(QObexPushClient::Error, expectedError);
        QFETCH(QStringList, expectedSignals);

        m_serverSession->m_responseCodes = serverResponses;

        Q_ASSERT(serverResponses.size() > 0);
        QObex::ResponseCode lastResponse = serverResponses.last().value<QObex::ResponseCode>();

        QSignalSpy spy(m_proxy->m_client,SIGNAL(done(bool)));
        for (int i=0; i<commands.size(); i++) {
            int id;
            switch (commands[i].toInt()) {
                case Connect:
                    id = m_proxy->m_client->connect();
                    m_proxy->addTestCommand(id, QObexPushClient::Connect);
                    break;
                case Disconnect:
                    id = m_proxy->m_client->disconnect();
                    m_proxy->addTestCommand(id, QObexPushClient::Disconnect);
                    break;
                case SendIODevice:
                    id = m_proxy->m_client->send(&m_sendBuffer, QString(), QString());
                    m_proxy->addTestCommand(id, QObexPushClient::Send);
                    break;
                case SendByteArray:
                    id = m_proxy->m_client->send(QByteArray(), QString(), QString());
                    m_proxy->addTestCommand(id, QObexPushClient::Send);
                    break;
                case SendBusinessCard:
                    id = m_proxy->m_client->sendBusinessCard(&m_sendBuffer);
                    m_proxy->addTestCommand(id, QObexPushClient::SendBusinessCard);
                    break;
                case RequestBusinessCard:
                    id = m_proxy->m_client->requestBusinessCard(&m_sendBuffer);
                    m_proxy->addTestCommand(id, QObexPushClient::RequestBusinessCard);
                    break;
                case ExchangeBusinessCard:
                    m_proxy->m_client->exchangeBusinessCard(&m_sendBuffer, &m_sendBuffer, &id, 0);
                    break;
                default:
                    Q_ASSERT(false);
            }
        }

        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << (expectedError!=QObexPushClient::NoError) );

        QCOMPARE(m_proxy->m_emittedSignals, expectedSignals);
        QCOMPARE(m_proxy->m_client->error(), expectedError);
        QCOMPARE(m_proxy->m_client->lastCommandResponse(), lastResponse);
    }

    void testErrors_data()
    {
        QTest::addColumn<QObexPushClient::Error>("error");
        QTest::newRow("NoError") << QObexPushClient::NoError;
        QTest::newRow("ConnectionError") << QObexPushClient::ConnectionError;
        QTest::newRow("RequestFailed") << QObexPushClient::RequestFailed;
        QTest::newRow("Aborted") << QObexPushClient::Aborted;
        // No UnknownError
    }

    void testErrors()
    {
        QFETCH(QObexPushClient::Error, error);

        QSignalSpy spy(m_proxy->m_client,SIGNAL(done(bool)));
        switch (error) {
            case QObexPushClient::NoError:
                m_serverSession->m_responseCodes << qVariantFromValue(QObex::Success);
                m_proxy->m_client->connect();
                break;
            case QObexPushClient::ConnectionError:
                m_clientSocket->close();
                m_proxy->m_client->connect();
                break;
            case QObexPushClient::RequestFailed:
                m_serverSession->m_responseCodes << qVariantFromValue(QObex::Forbidden);
                m_proxy->m_client->connect();
                break;
            case QObexPushClient::Aborted:
                m_serverSession->m_responseCodes << qVariantFromValue(QObex::Success);
                m_proxy->m_client->send(&m_sendBuffer, QString(), QString());
                m_proxy->m_client->abort();
                break;
            default:
                Q_ASSERT(false);
        }
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << (error!=QObexPushClient::NoError) );

        QCOMPARE(m_proxy->m_client->error(), error);
        QCOMPARE(m_proxy->m_errors.count(error), 1);
        QCOMPARE(m_proxy->m_errors.size(), 1);
    }

    void send_data()
    {
        QTest::addColumn<PushCommand>("requestType");
        QTest::addColumn<QString>("filename");
        QTest::addColumn<QString>("mimeType");

        QTest::newRow("sendIODevice") << SendIODevice << "some name" << "some type";
        QTest::newRow("sendByteArray") << SendByteArray << "some name" << "some type";
    }

    void send()
    {
        QFETCH(PushCommand, requestType);
        QFETCH(QString, filename);
        QFETCH(QString, mimeType);

        QByteArray bytesToSend = get_some_data(5000);

        QSignalSpy spy(m_proxy->m_client,SIGNAL(done(bool)));
        m_serverSession->m_responseCodes << qVariantFromValue(QObex::Success);
        switch (requestType) {
            case SendIODevice:
                m_sendBuffer.buffer().append(bytesToSend);
                m_proxy->m_client->send(&m_sendBuffer, filename, mimeType);
                break;
            case SendByteArray:
                m_proxy->m_client->send(bytesToSend, filename, mimeType);
                break;
            default:
                Q_ASSERT(false);
        }
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        QCOMPARE(m_serverSession->m_recvBuffer.data(),bytesToSend);

        QCOMPARE(m_serverSession->m_requestHeaders.size(), 1);
        QObexHeader header = m_serverSession->m_requestHeaders.first();
        QCOMPARE(header.size(), 3);
        QCOMPARE(header.type(), mimeType);
        QCOMPARE(header.name(), filename);
    }

    // OPP spec 4.4.2
    void exchangeBusinessCard()
    {
        QBuffer myCard;
        myCard.setData(get_some_data(5000));
        QBuffer theirCard;
        theirCard.open(QIODevice::WriteOnly);

        m_serverSession->m_sendBuffer.setData(get_some_data(5000));

        QSignalSpy spy(m_proxy->m_client,SIGNAL(done(bool)));
        m_serverSession->m_responseCodes << qVariantFromValue(QObex::Success)
                                  << qVariantFromValue(QObex::Success);
        int id1, id2;
        m_proxy->m_client->exchangeBusinessCard(&myCard, &theirCard, &id1, &id2);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        // check data
        QCOMPARE(m_serverSession->m_recvBuffer.data(), myCard.data());
        QCOMPARE(m_serverSession->m_sendBuffer.data(), theirCard.data());

        // check headers
        QCOMPARE(m_serverSession->m_requestHeaders.size(), 2);

        // specs don't require certain headers for put, but we should send
        // the vcard mime type since we're sending a business card
        QObexHeader sendVCardHeader = m_serverSession->m_requestHeaders.first();
        QVERIFY(sendVCardHeader.name().endsWith(".vcf"));
        QCOMPARE(sendVCardHeader.type(), QString("text/x-vCard"));

        // specs require text/x-vCard mime type and no name header
        QObexHeader requestVCardHeader = m_serverSession->m_requestHeaders.last();
        QVERIFY(!requestVCardHeader.contains(QObexHeader::Name));
        QCOMPARE(requestVCardHeader.type(), QString("text/x-vCard"));
    }
};

QTEST_MAIN(tst_QObexPushClient)
#include "tst_qobexpushclient.moc"
