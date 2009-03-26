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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include <QTest>

// for m_map
#define protected public
# include "qtestprotocol_p.h"
#undef protected

#include "testprotocol.h"
#include "testprotocolserver.h"
#include <shared/util.h>

//TESTED_COMPONENT=QA: Testing Framework (18707)

class tst_QTestProtocol : public QObject
{
    Q_OBJECT

private slots:
    void postMessage();
    void postMessage_data();
};

QTEST_MAIN(tst_QTestProtocol)

/*
    Comparison for QTestMessage.
*/
namespace QTest {
template<>
inline bool qCompare
    (QTestMessage const &m1, QTestMessage const &m2,
     const char* actual, const char* expected, const char* file, int line)
{
#define COMPARE_MEMBER(a,b,...)                                      \
    qCompare(a.__VA_ARGS__, b.__VA_ARGS__,                            \
        qPrintable(QString("%1.%2").arg(actual).arg(#__VA_ARGS__)),   \
        qPrintable(QString("%1.%2").arg(expected).arg(#__VA_ARGS__)), \
            file, line)

    return COMPARE_MEMBER(m1,m2,event())
        && COMPARE_MEMBER(m1,m2,toString())
        && COMPARE_MEMBER(m1,m2,m_map);

#undef COMPARE_MEMBER
}
}

/*
    \req QTOPIA-78

    \groups
    Simply tests that messages can be sent correctly without becoming corrupt.
*/
void tst_QTestProtocol::postMessage_data()
{
    QTest::addColumn<QTestMessage>("message");

    QTest::newRow("empty")
        << QTestMessage();

    QTest::newRow("simple, no map")
        << QTestMessage("foo");

    {
        QVariantMap vm;
        vm["foo"] = 1;
        vm["bar"] = "Hello.";

        QTest::newRow("simple, small map")
            << QTestMessage("foobar", vm);
    }

    {
        QByteArray ba;
        while (ba.size() < 1024*1024*10)
            ba.append("zoop");

        QVariantMap vm;
        vm["aaa"] = "A";
        vm["bbb"] = ba;
        vm["ccc"] = "C";

        QTest::newRow("10 Mb")
            << QTestMessage("big_test", vm);
    }
}

void tst_QTestProtocol::postMessage()
{
    // Start a server to listen for connections.
    TestProtocolServer server;
    QVERIFY(server.listen());

    // Create a test protocol which connects to the server.
    TestProtocol* clientP = new TestProtocol;
    clientP->setParent(&server);
    clientP->connect( "127.0.0.1", server.serverPort() );

    QVERIFY( clientP->waitForConnected(1000) );
    QVERIFY( server.hasPendingConnections() );

    // Get the protocol which should have been created on the server side
    TestProtocol* serverP = server.nextPendingConnection();
    QVERIFY( serverP );
    // Should be no further connections
    QVERIFY( !server.hasPendingConnections() );

    // OK, now we have two peers.  They should both be connected by now.
    QVERIFY( clientP->isConnected() );
    QVERIFY( serverP->isConnected() );

    QList<QTestMessage>& clientMessages = clientP->messages();
    QList<QTestMessage>& serverMessages = serverP->messages();

    QFETCH(QTestMessage, message);

    // Try sending the message from client to server.
    uint post_id = clientP->postMessage(message);
    QVERIFY(post_id);
    QCOMPARE(serverMessages.count(), 0);
    QTRY_COMPARE(serverMessages.count(), 1);

    // Verify the message is OK.
    QCOMPARE(serverMessages.takeFirst(), message);


    // Try sending the message from server to client.
    post_id = serverP->postMessage(message);
    QVERIFY(post_id);
    QCOMPARE(clientMessages.count(), 0);
    QTRY_COMPARE(clientMessages.count(), 1);

    // Verify the message is OK.
    QCOMPARE(clientMessages.takeFirst(), message);
}

#include "tst_qtestprotocol.moc"

