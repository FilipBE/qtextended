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

#ifndef COMMON_H
#define COMMON_H

#include <QtTest>
#include <private/qtestresult_p.h>
#include <trace.h>
#include <QSignalSpy>
#include <qcopchannel_qd.h>
#include <qcopenvelope_qd.h>
#include <QXmlStreamReader>
#include "qtopia4sync.h"
#include "qpimsyncstorage.h"
#include <QtopiaApplication>
#include <qvaluespace.h>

bool &qdTraceEnabled();

class ChannelObject : public QObject
{
    Q_OBJECT
public:
    ChannelObject();
    ~ChannelObject();

    void send_later( const QString &channel, const QString &message, const QByteArray &data = QByteArray() );
    void send();
    int expect( const QString &channel, const QString &message, const QByteArray &data = QByteArray() );
    int expectedCount();
    int pendingCount();

    struct Message
    {
        int id;
        QString channel;
        QString message;
        QByteArray data;
        bool operator==( const Message &other ) const
        {
            return ( channel == other.channel && message == other.message && (data.isEmpty() || data == other.data) );
        }
    };

    QList<Message> pending()
    {
        return pendingMessages;
    }

signals:
    void gotExpected( int id, const QByteArray &data );
    void gotUnexpected( const QString &channel, const QString &message, const QByteArray &data );

private slots:
    void qpeMessage( const QString &message, const QByteArray &data );

private:
    void checkExpected();

    int lastId;
    QList<Message> expectedMessages;
    QList<Message> pendingMessages;
    QList<Message> sendingMessages;
};

QString dateToString( const QDate &date );
QDate stringToDate( const QString &string );
QString dateTimeToString( const QDateTime &datetime, bool utc );
QDateTime stringToDateTime( const QString &string, bool utc );
QString escape( const QString &string );
QString unescape( const QString &string );
bool getIdentifier( const QByteArray &record, QString &id, bool &local );
bool isEquivalent( const QByteArray &record1, const QByteArray &record2 );

class CommonMethods
{
protected:
    ChannelObject *chanobj;
    Qtopia4Sync *protocol;
    QMap<QString,QString> idMap;
    QString dataset;

    bool startSync();
    bool doSync();
    bool endSync();
    bool cleanOutTable();
    bool checkForEmptyTable();
    bool addClientRecord( const QByteArray &record );
    bool checkForAddedItem( const QByteArray &expected );
    bool editClientRecord( const QByteArray &record );
};

extern void (*messagehandler)(QtMsgType type, const char *msg);
void aBetterMessageHandler(QtMsgType type, const char *msg);

#define INIT_BODY(enable_trace,ds)\
    /* disable TRACE/LOG output */\
    bool &trace = qdTraceEnabled();\
    trace = enable_trace;\
    if ( trace ) {\
        QSettings settings( "Trolltech", "Log" );\
        settings.setValue("Synchronization/Enabled", 1);\
        settings.setValue("QDSync_/Enabled", 1);\
        settings.setValue("QDSync_Modem/Enabled", 1);\
        settings.setValue("QDSync_Modem_Status/Enabled", 1);\
        settings.setValue("QDSync_QCopBridge/Enabled", 1);\
        settings.setValue("QDSync_QDLinkHelper/Enabled", 1);\
        settings.setValue("QDSync_QDSync/Enabled", 1);\
        settings.setValue("QDSync_QDThread/Enabled", 1);\
        settings.setValue("QDSync_Qtopia4Sync/Enabled", 1);\
        settings.setValue("QDSync_SyncAuthentication/Enabled", 1);\
    }\
    dataset = ds;\
    messagehandler = qInstallMsgHandler(aBetterMessageHandler);\
    chanobj = new ChannelObject;\
    protocol = Qtopia4Sync::instance();\
    Qtopia4SyncPluginFactory *pf = new QPimSyncStorageFactory( 0 );\
    protocol->registerPlugin(pf->plugin(dataset))

#define CLEANUP_BODY()\
    delete protocol;\
    delete chanobj

#define SEND_QCOP(c,m,d)\
do {\
    QByteArray data;\
    {\
        QDataStream stream( &data, QIODevice::WriteOnly );\
        stream d;\
    }\
    chanobj->send_later(c, m, data);\
} while ( 0 )

#define EXPECT_QCOP(c,m,d)\
do {\
    QByteArray data;\
    {\
        QDataStream stream( &data, QIODevice::WriteOnly );\
        stream d;\
    }\
    chanobj->expect(c, m, data);\
} while ( 0 )

#define WAIT(timeout,x)\
do {\
    QSignalSpy spy( chanobj, SIGNAL(gotUnexpected(QString,QString,QByteArray)) );\
    chanobj->send();\
    QTime now;\
    now.start();\
    do {\
        qApp->processEvents();\
        x\
        QVERIFY2(now.elapsed() < timeout*1000, "WAIT exceeded timeout " #timeout);\
        if (!ok) break;\
    } while ( chanobj->expectedCount() );\
    foreach ( const ChannelObject::Message &m, chanobj->pending() )\
        qWarning() << "Pending message" << m.channel << m.message;\
    QVERIFY(chanobj->pendingCount() == 0);\
    bool unexpected_messages = false;\
    while ( spy.count() ) {\
        QList<QVariant> args = spy.takeFirst();\
        QString channel = args.at(0).toString();\
        QString message = args.at(1).toString();\
        QByteArray data = args.at(2).toByteArray();\
        qWarning() << "Unexpected message" << channel << message << data;\
        unexpected_messages = true;\
    }\
    QVERIFY(!unexpected_messages);\
} while ( 0 )

#define SANITIZE(array) QString(array).replace("\n","").replace(QRegExp("> +<"), "><").trimmed().toLocal8Bit()

// Redefine QTEST_MAIN so that it returns non-zero on failure!
#undef QTEST_MAIN
#define QTEST_MAIN(TestObject)\
int main(int argc, char *argv[])\
{\
    QtopiaApplication app(argc, argv);\
    QValueSpace::initValuespaceManager();\
    TestObject tc;\
    QTest::qExec(&tc, argc, argv);\
    int ret = QTestResult::failCount();\
    return ret;\
}

#endif
