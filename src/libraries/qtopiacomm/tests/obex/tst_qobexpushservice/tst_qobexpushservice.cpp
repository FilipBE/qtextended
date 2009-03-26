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
#include <qobexpushservice.h>
#include <qobexclientsession.h>

#include <qtopiaapplication.h>

#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QSignalSpy>
#include <shared/util.h>
#include <qmetatype.h>

#include <QBuffer>
#include <QDebug>
#include <QList>
#include <QTemporaryFile>
#include <QStringList>
#include <QMetaObject>
#include <QTimer>
#include <QPointer>
#include <QMessageBox>

//TESTED_CLASS=QObexPushService
//TESTED_FILES=src/libraries/qtopiacomm/obex/qobexpushservice.cpp

Q_DECLARE_USER_METATYPE_ENUM(QObexPushService::State);
Q_DECLARE_USER_METATYPE_ENUM(QObexPushService::Error);
Q_IMPLEMENT_USER_METATYPE_ENUM(QObexPushService::State);
Q_IMPLEMENT_USER_METATYPE_ENUM(QObexPushService::Error);
Q_DECLARE_METATYPE(QObexHeader);

const QString VALID_GET_TYPE   = "text/x-vCard";

// Default name used for transfers with no name
const QString DEFAULT_FILENAME = "received_file";

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

//=============================================================================

class CustomPushService : public QObexPushService
{
    Q_OBJECT
public:
    CustomPushService(QIODevice *device, QObject *parent = 0);

    QIODevice *m_device;
    QStringList m_names;
    QStringList m_types;
    QList<qint64> m_sizes;
    QStringList m_descriptions;

protected:
    virtual QIODevice *acceptFile(const QString &filename, const QString &mimetype, qint64 size, const QString &description);

};

CustomPushService::CustomPushService(QIODevice *device, QObject *parent)
    : QObexPushService(device, parent),
      m_device(0)
{
}

QIODevice *CustomPushService::acceptFile(const QString &filename, const QString &mimetype, qint64 size, const QString &description)
{
    qLog(Autotest) << "CustomPushService::acceptFile()" << filename << mimetype << size;
    m_names << filename;
    m_types << mimetype;
    m_sizes << size;
    m_descriptions << description;
    return m_device;
}

//=============================================================================

class PushServiceWithUI : public QObexPushService
{
    Q_OBJECT
public:
    PushServiceWithUI(QIODevice *device, QObject *parent = 0);
    ~PushServiceWithUI();

    QMessageBox *m_msgBox;

signals:
    void pausedForUI();

protected:
    virtual QIODevice *acceptFile(const QString &filename, const QString &mimetype, qint64 size, const QString &description);

};

PushServiceWithUI::PushServiceWithUI(QIODevice *device, QObject *parent)
    : QObexPushService(device, parent),
      m_msgBox(new QMessageBox(0))
{
}

PushServiceWithUI::~PushServiceWithUI()
{
    delete m_msgBox;
}

QIODevice *PushServiceWithUI::acceptFile(const QString &, const QString &, qint64, const QString &)
{
    qLog(Autotest) << "PushServiceWithUI::acceptFile()";

    QTimer::singleShot(0, this, SIGNAL(pausedForUI()));
    //emit pausedForUI();
    int result = m_msgBox->exec();
    if (result == QMessageBox::Accepted)
        return new QFile(this);
    return 0;
}


//=============================================================================

class PushServiceSignalReceiver : public QObject
{
    Q_OBJECT
public:
    PushServiceSignalReceiver(QObexPushService *service, QObject *parent = 0);

    QObexPushService *m_push;
    QStringList m_emittedSignals;

    QStringList m_putNames;
    QStringList m_putTypes;
    QList<qint64> m_putSizes;
    QStringList m_putDescriptions;

    QList<QVariant> m_states;
    QList<QObexPushService::Error> m_requestErrors;
    QList<QObexPushService::Error> m_doneErrors;

    QList<qint64> m_doneValues;
    QList<qint64> m_totalValues;
    bool m_receivingPut;
    QIODevice *m_putDevice;

private slots:
    void putRequested(const QString &filename, const QString &mimetype, qint64 size, const QString &description);
    void businessCardRequested();
    void requestFinished(bool error);
    void done(bool error);
    void dataTransferProgress(qint64, qint64);
    void stateChanged(QObexPushService::State);
};

PushServiceSignalReceiver::PushServiceSignalReceiver(QObexPushService *service, QObject *parent)
    : QObject(parent),
      m_push(service)
{
    connect(m_push, SIGNAL(putRequested(QString,QString,qint64,QString)),
            SLOT(putRequested(QString,QString,qint64,QString)));
    connect(m_push, SIGNAL(businessCardRequested()),
            SLOT(businessCardRequested()));
    connect(m_push, SIGNAL(requestFinished(bool)),
            SLOT(requestFinished(bool)));
    connect(m_push, SIGNAL(dataTransferProgress(qint64,qint64)),
            SLOT(dataTransferProgress(qint64,qint64)));
    connect(m_push, SIGNAL(stateChanged(QObexPushService::State)),
            SLOT(stateChanged(QObexPushService::State)));
    connect(m_push, SIGNAL(done(bool)),
            SLOT(done(bool)));

    m_receivingPut = false;
    m_putDevice = 0;
}

void PushServiceSignalReceiver::putRequested(const QString &filename, const QString &mimetype, qint64 size, const QString &description)
{
    qLog(Autotest) << "PushServiceSignalReceiver::putRequested()" << filename << mimetype << size << description;
    m_emittedSignals << "putRequested";
    m_putNames << filename;
    m_putTypes << mimetype;
    m_putSizes << size;
    m_putDescriptions << description;

    m_receivingPut = true;
    QVERIFY(m_push->currentDevice() != 0);
    m_putDevice = m_push->currentDevice();
}

void PushServiceSignalReceiver::businessCardRequested()
{
    qLog(Autotest) << "PushServiceSignalReceiver::businessCardRequested()";
    m_emittedSignals << "businessCardRequested";
}

void PushServiceSignalReceiver::requestFinished(bool error)
{
    qLog(Autotest) << "PushServiceSignalReceiver::requestFinished()" << error << m_push->error();

    m_emittedSignals << "requestFinished";
    QCOMPARE(error, m_push->error() != QObexPushService::NoError);
    m_requestErrors << m_push->error();

    // the currentDevice() should still be valid at requestFinished()
    if (m_receivingPut) {
        QVERIFY(m_push->currentDevice() != 0);
        QCOMPARE(m_push->currentDevice(), m_putDevice);
    }

    m_receivingPut = false;
}

void PushServiceSignalReceiver::done(bool error)
{
    qLog(Autotest) << "PushServiceSignalReceiver::done()" << error;

    QVERIFY(m_push->currentDevice() == 0);

    QCOMPARE(error, m_push->error() != QObexPushService::NoError);
    m_emittedSignals << "done";
    m_doneErrors << m_push->error();
}

void PushServiceSignalReceiver::dataTransferProgress(qint64 done, qint64 total)
{
    qLog(Autotest) << "PushServiceSignalReceiver::dataTransferProgress()" << done << total;
    m_emittedSignals << "dataTransferProgress"; 

    m_doneValues << done;
    m_totalValues << total;

    if (m_receivingPut) {
        QVERIFY(m_push->currentDevice() != 0);
        QCOMPARE(m_push->currentDevice(), m_putDevice);
    }
}

void PushServiceSignalReceiver::stateChanged(QObexPushService::State state)
{
    qLog(Autotest) << "PushServiceSignalReceiver::stateChanged()" << state;
    m_emittedSignals << "stateChanged";
    m_states << state;
}

//=============================================================================


class tst_QObexPushService : public QObject
{
    Q_OBJECT

private:
    QTcpServer *m_tcpServer;
    QTcpSocket *m_clientSocket;
    QTcpSocket *m_serverSocket;
    QPointer<QObexPushService> m_push;  // m_push is deleted in some unit tests
    PushServiceSignalReceiver *m_pushHandler;

    QStringList m_filesToCleanUp;


private slots:

    void initTestCase()
    {
        m_tcpServer = new QTcpServer(this);
        bool b = m_tcpServer->listen(QHostAddress(QHostAddress::Any), 0);
        Q_ASSERT(b);

        m_clientSocket = 0;
        m_serverSocket = 0;
        m_push = 0;
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

        m_push = new QObexPushService(m_serverSocket);
        m_pushHandler = new PushServiceSignalReceiver(m_push);
    }

    void cleanup()
    {
        delete m_push;
        m_push = 0;

        delete m_pushHandler;
        m_pushHandler = 0;

        delete m_clientSocket;
        m_clientSocket = 0;
        delete m_serverSocket;
        m_serverSocket = 0;

        for (int i=0; i<m_filesToCleanUp.size(); i++) {
            qLog(Autotest) << "Deleting file:" << m_filesToCleanUp;
            Q_ASSERT(QFile(m_filesToCleanUp[i]).remove());
        }
        m_filesToCleanUp.clear();
    }

    void initialState()
    {
        QCOMPARE(m_push->error(), QObexPushService::NoError);
        QCOMPARE(m_push->state(), QObexPushService::Ready);
        QCOMPARE(m_push->businessCard(), QByteArray());
    }

    void acceptFile_args_data()
    {
        QTest::addColumn<QString>("name");
        QTest::addColumn<QString>("type");
        QTest::addColumn<qint64>("size");
        QTest::addColumn<QString>("description");

        QTest::newRow("empty args") << QString() << QString() << qint64(0) << QString();
        QTest::newRow("size > 0") << QString() << QString() << qint64(555) << QString();
        QTest::newRow("name not empty") << QString("a name") << QString() << qint64(0) << QString();
        QTest::newRow("type not empty") << QString() << QString("a type") << qint64(0) << QString();
        QTest::newRow("description not empty") << QString() << QString() << qint64(0) << QString("a description");
    }

    void acceptFile_args()
    {
        QFETCH(QString, name);
        QFETCH(QString, type);
        QFETCH(qint64, size);
        QFETCH(QString, description);

        delete m_push;
        m_push = 0;
        delete m_pushHandler;
        m_pushHandler = 0;
        CustomPushService *cpush = new CustomPushService(m_serverSocket, this);

        QObexHeader h;
        h.setName(name);
        h.setType(type);
        h.setLength(size);
        h.setDescription(description);

        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        QSignalSpy spy(client,SIGNAL(done(bool)));
        client->put(h, get_some_data(size));
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << true );  // put will be rejected, we're just checking the acceptFile() args

        QCOMPARE(cpush->m_names, QStringList(name));
        QCOMPARE(cpush->m_types, QStringList(type));
        QCOMPARE(cpush->m_sizes, (QList<qint64>() << size));
        QCOMPARE(cpush->m_descriptions, QStringList(description));

        delete client;
        delete cpush;
    }

    // If the Name header is empty, or looks like a directory, DEFAULT_FILENAME should
    // be used as the file name by the default acceptFile() implementation.
    // Otherwise, the Name header's value should be used as the file name.
    void acceptFile_defaultImpl_data()
    {
        QTest::addColumn<QString>("name");
        QTest::addColumn<QString>("expectedFileName");

        QTest::newRow("name ok")
                << QString("some_name")
                << QString("some_name");

        QTest::newRow("name is empty")
                << QString()
                << DEFAULT_FILENAME;

        QTest::newRow("name with path")
                << QString("/name/othername")
                << QString("othername");

        QTest::newRow("name with path separator at end")
                << QString("/name/othername/")
                << DEFAULT_FILENAME; // cos there's no base name
    }

    // Verify the default acceptFile() implementation correctly saves an incoming file.
    void acceptFile_defaultImpl()
    {
        QFETCH(QString, name);
        QFETCH(QString, expectedFileName);

        QString expectedFilePath = QDir::homePath() + QDir::separator() + expectedFileName;
        m_filesToCleanUp << expectedFilePath;
        if (QFile::exists(expectedFilePath)) {
            QFile f(expectedFilePath);
            bool b = f.remove();
            Q_ASSERT(b);
        }

        QByteArray bytes = get_some_data(50);

        QObexHeader h;
        h.setName(name);
        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        QSignalSpy spy(client,SIGNAL(done(bool)));
        client->put(h, bytes);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        QVERIFY(QFile::exists(expectedFilePath));
        QVERIFY(m_pushHandler->m_emittedSignals.size() > 0);
        QCOMPARE(m_pushHandler->m_requestErrors,
                (QList<QObexPushService::Error>() << QObexPushService::NoError));

        // check received file
        QFile f(expectedFilePath);
        bool b = f.open(QIODevice::ReadOnly);
        QVERIFY(b);
        QCOMPARE(f.size(), qint64(bytes.size()));
        QCOMPARE(f.readAll(), bytes);
        f.close();

        QCOMPARE(client->error(), QObexClientSession::NoError);

        delete client;
    }

    // Test the "*_X" numbering works if no filename is provided in an
    // incoming Put request.
    void acceptFile_defaultImpl_filenames()
    {
        int fileCount = 5;

        QDir dir(QDir::homePath());
        QStringList existingUnnamedFiles = dir.entryList();
        for (int i=0; i<existingUnnamedFiles.size(); i++) {
            if (existingUnnamedFiles[i].startsWith(DEFAULT_FILENAME)) {
                bool b = QFile(dir.path() + QDir::separator() + existingUnnamedFiles[i]).remove();
                QVERIFY(b);
            }
        }

        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        QSignalSpy spy(client,SIGNAL(done(bool)));
        for (int i=0; i<fileCount; i++) {
            client->put(QObexHeader(), QByteArray(QTest::toString(i)));
        }
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );

        for (int i=0; i<fileCount; i++) {
            QString filename = DEFAULT_FILENAME;
            if (i > 0)
                filename += QString("_%1").arg(i);
            QString path = QDir::homePath() + QDir::separator() + filename;

            QFile f(path);
            QVERIFY(f.exists());
            bool b = f.open(QIODevice::ReadOnly);
            QVERIFY(b);
            QCOMPARE(f.readAll().constData(), QTest::toString(i));
            f.close();
            b = f.remove();
            QVERIFY(b);
        }

        QCOMPARE(client->error(), QObexClientSession::NoError);
        delete client;
    }

    void acceptFile_custom_data()
    {
        QTest::addColumn<bool>("acceptPutRequest");
        QTest::newRow("accept") << true;
        QTest::newRow("don't accept") << false;
    }

    void acceptFile_custom()
    {
        QFETCH(bool, acceptPutRequest);

        delete m_push;
        m_push = 0;
        delete m_pushHandler;
        m_pushHandler = 0;

        CustomPushService *cpush = new CustomPushService(m_serverSocket, this);
        m_pushHandler = new PushServiceSignalReceiver(cpush);

        QByteArray bytes = get_some_data(100);
        if (acceptPutRequest) {
            cpush->m_device = new QBuffer;
            cpush->m_device->open(QIODevice::ReadWrite);
        } else {
            cpush->m_device = 0;
        }
        m_pushHandler->m_putDevice = cpush->m_device;

        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        QSignalSpy spy(client,SIGNAL(done(bool)));
        client->put(QObexHeader(), bytes);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << !acceptPutRequest );

        if (acceptPutRequest) {
            QVERIFY(m_pushHandler->m_emittedSignals.size() > 0);
            QCOMPARE(m_pushHandler->m_requestErrors,
                    (QList<QObexPushService::Error>() << QObexPushService::NoError));

            cpush->m_device->seek(0);
            QCOMPARE(cpush->m_device->readAll(), bytes);

            QCOMPARE(client->error(), QObexClientSession::NoError);
        } else {
            QCOMPARE(m_pushHandler->m_emittedSignals.size(), 0);

            QCOMPARE(client->error(), QObexClientSession::RequestFailed);
            QCOMPARE(client->lastResponseCode(), QObex::Forbidden);
        }

        // service should not touch the custom QIODevice
        if (cpush->m_device)
            QVERIFY(cpush->m_device->isOpen());

        delete cpush->m_device;
        delete cpush;
        delete client;
    }

    void putRequested_data()
    {
        QTest::addColumn<bool>("acceptPutRequest");
        QTest::addColumn<QString>("name");
        QTest::addColumn<QString>("type");
        QTest::addColumn<qint64>("size");
        QTest::addColumn<QString>("description");

        for (int i=0; i<2; i++) {
            bool b = (i % 2 == 0);
            QTest::newRow(QTest::toString(QString("empty args, %1").arg(b)))
                    << b << QString() << QString() << qint64(0) << QString();
            QTest::newRow(QTest::toString(QString("name not empty, %1").arg(b)))
                    << b << QString("a name") << QString() << qint64(0) << QString();
            QTest::newRow(QTest::toString(QString("type not empty, %1").arg(b)))
                    << b << QString() << QString("a type") << qint64(0) << QString();
            QTest::newRow(QTest::toString(QString("size not zero, %1").arg(b)))
                    << b << QString() << QString("a type") << qint64(100) << QString();
            QTest::newRow(QTest::toString(QString("description not empty, %1").arg(b)))
                    << b << QString() << QString() << qint64(0) << QString("a description");
        }
    }

    void putRequested()
    {
        QFETCH(bool, acceptPutRequest);
        QFETCH(QString, name);
        QFETCH(QString, type);
        QFETCH(qint64, size);
        QFETCH(QString, description);

        delete m_push;
        m_push = 0;
        delete m_pushHandler;
        m_pushHandler = 0;

        CustomPushService *cpush = new CustomPushService(m_serverSocket, this);
        m_pushHandler = new PushServiceSignalReceiver(cpush);
        if (acceptPutRequest) {
            cpush->m_device = new QBuffer;
            cpush->m_device->open(QIODevice::ReadWrite);
        }

        QObexHeader h;
        h.setName(name);
        h.setType(type);
        h.setLength(size);
        h.setDescription(description);
        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        QSignalSpy spy(client,SIGNAL(done(bool)));
        client->put(h, QByteArray());
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << !acceptPutRequest );

        // If put is accepted, putRequested() should be emitted, otherwise
        // it should not be emitted.
        if (acceptPutRequest) {
            QCOMPARE(m_pushHandler->m_emittedSignals.count("putRequested"), 1);
            QCOMPARE(m_pushHandler->m_emittedSignals[0], QString("putRequested"));
            QCOMPARE(m_pushHandler->m_putNames, QStringList(name));
            QCOMPARE(m_pushHandler->m_putTypes, QStringList(type));
            QCOMPARE(m_pushHandler->m_putSizes, (QList<qint64>() << size));
            QCOMPARE(m_pushHandler->m_putDescriptions, QStringList(description));
        } else {
            QCOMPARE(m_pushHandler->m_emittedSignals.count("putRequested"), 0);
            QVERIFY(m_pushHandler->m_putNames.isEmpty());
            QVERIFY(m_pushHandler->m_putTypes.isEmpty());
            QVERIFY(m_pushHandler->m_putSizes.isEmpty());
            QVERIFY(m_pushHandler->m_putDescriptions.isEmpty());
        }

        delete cpush->m_device;
        delete client;
        delete cpush;
    }

    void businessCardRequested_data()
    {
        QTest::addColumn<bool>("expectAccepted");
        QTest::addColumn<QObexHeader>("header");
        QTest::addColumn<QByteArray>("businessCard");
        QTest::addColumn<QObex::ResponseCode>("serverResponse");

        // OPP spec 5.6:
        // 1. If server has no default GET object, respond "Not found"
        // 2. Push client must send a type header with "text/x-vCard"
        // 3. Push client must not send name header (or, if exists must be empty?)
        //    otherwise server should respond "Forbidden"

        QObexHeader h;
        QByteArray someBytes = get_some_data(20);

        h.clear();
        h.setType(VALID_GET_TYPE);
        QTest::newRow("headers ok, business card ok")
                << true << h << someBytes << QObex::Success;

        h.clear();
        h.setType(VALID_GET_TYPE);
        QTest::newRow("headers ok, empty business card")
                << false << h << QByteArray() << QObex::NotFound;

        // Specs don't say what should happen if type header is not sent.
        // Probably should respond with Forbidden, since that's what should
        // happen if a Name header is given, and that's a similar case
        // because the service should only accept v-card requests.
        h.clear();
        QTest::newRow("no headers, business card ok")
                << false << h << someBytes << QObex::Forbidden;

        h.clear();
        h.setName("a name that shouldn't be here");
        h.setType(VALID_GET_TYPE);
        QTest::newRow("bad header (has name), business card ok")
                << false << h << someBytes << QObex::Forbidden;

        // Probably accept a Get if name *does* exists, but it's empty.
        h.clear();
        h.setName("");
        h.setType(VALID_GET_TYPE);
        QTest::newRow("passable header (has empty name string), business card ok")
                << true << h << someBytes << QObex::Success;
    }

    void businessCardRequested()
    {
        QFETCH(bool, expectAccepted);
        QFETCH(QObexHeader, header);
        QFETCH(QByteArray, businessCard);
        QFETCH(QObex::ResponseCode, serverResponse);

        m_push->setBusinessCard(businessCard);

        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        QSignalSpy spy(client,SIGNAL(done(bool)));
        QBuffer buffer;
        client->get(header, &buffer);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << !expectAccepted );

        if (expectAccepted) {
            QCOMPARE(m_pushHandler->m_emittedSignals.count("businessCardRequested"), 1);
            QCOMPARE(m_pushHandler->m_emittedSignals[0], QString("businessCardRequested"));
            QCOMPARE(m_pushHandler->m_emittedSignals.count("requestFinished"), 1);
        } else {
            QCOMPARE(m_pushHandler->m_emittedSignals.count("businessCardRequested"), 0);
            QCOMPARE(m_pushHandler->m_emittedSignals.count("requestFinished"), 0);
        }

        QCOMPARE(client->lastResponseCode(), serverResponse);

        delete client;
    }

    void setBusinessCard_data()
    {
        QTest::addColumn<QByteArray>("bytes");
        QTest::newRow("empty bytes") << QByteArray();
        QTest::newRow("small bytes") << get_some_data(200);
        QTest::newRow("large bytes") << get_some_data(5000);
    }

    void setBusinessCard()
    {
        QFETCH(QByteArray, bytes);
        m_push->setBusinessCard(bytes);
        QCOMPARE(m_push->businessCard(), bytes);
    }

    void testBusinessCardRequest_data()
    {
        QTest::addColumn<QByteArray>("bytes");
        QTest::newRow("empty bytes") << QByteArray();
        QTest::newRow("small bytes") << get_some_data(200);
        QTest::newRow("large bytes") << get_some_data(5000);
    }

    void testBusinessCardRequest()
    {
        QFETCH(QByteArray, bytes);
        m_push->setBusinessCard(bytes);

        bool expectSuccess = (!bytes.isEmpty());

        QObexHeader h;
        h.setType(VALID_GET_TYPE);

        QBuffer buffer;
        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        QSignalSpy spy(client,SIGNAL(done(bool)));
        client->get(h, &buffer);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << !expectSuccess );

        if (expectSuccess) {
            QCOMPARE(buffer.data(), bytes);
        } else {
            QCOMPARE(buffer.data(), QByteArray());
        }
    }

    void genericSignals_data()
    {
        QTest::addColumn<bool>("finishWithError");
        QTest::addColumn< QList<QVariant> >("requests");
        QTest::addColumn<QStringList>("expectedSignals");
        QTest::addColumn< QList<QVariant> >("expectedStates");

        // Each Put and Get request test assumes that only a small number of
        // bytes will be sent (i.e. fits in one packet).

        QStringList connectSignals;
        connectSignals << "stateChanged" << "stateChanged";

        QStringList disconnectSignals;
        disconnectSignals << "stateChanged" << "stateChanged" << "done";

        QStringList putSignals;
        putSignals << "putRequested" << "stateChanged"
                   << "dataTransferProgress" << "stateChanged" << "requestFinished";

        QStringList getSignals;
        getSignals << "businessCardRequested" << "stateChanged"
                   << "dataTransferProgress" << "stateChanged" << "requestFinished";


        QTest::newRow("just connect")
                << false
                << (QList<QVariant>() << QObex::Connect)
                << connectSignals
                << (QList<QVariant>() << QObexPushService::Connecting << QObexPushService::Ready);

        QTest::newRow("just disconnect")
                << false
                << (QList<QVariant>() << QObex::Disconnect)
                << disconnectSignals
                << (QList<QVariant>() << QObexPushService::Disconnecting << QObexPushService::Closed);

        QTest::newRow("just put")
                << false
                << (QList<QVariant>() << QObex::Put)
                << putSignals
                << (QList<QVariant>() << QObexPushService::Streaming << QObexPushService::Ready);

        QTest::newRow("just get")
                << false
                << (QList<QVariant>() << QObex::Get)
                << getSignals
                << (QList<QVariant>() << QObexPushService::Streaming << QObexPushService::Ready);

        QTest::newRow("Connect, put, disconnect")
                << false
                << (QList<QVariant>() << QObex::Connect << QObex::Put << QObex::Disconnect)
                << (QStringList() << connectSignals << putSignals << disconnectSignals)
                << (QList<QVariant>() << QObexPushService::Connecting << QObexPushService::Ready
                                      << QObexPushService::Streaming << QObexPushService::Ready
                                      << QObexPushService::Disconnecting << QObexPushService::Closed);

        QTest::newRow("Connect, put * 2, disconnect")
                << false
                << (QList<QVariant>() << QObex::Connect << QObex::Put << QObex::Put << QObex::Disconnect)
                << (QStringList() << connectSignals << putSignals
                                  << putSignals << disconnectSignals)
                << (QList<QVariant>() << QObexPushService::Connecting << QObexPushService::Ready
                                      << QObexPushService::Streaming << QObexPushService::Ready
                                      << QObexPushService::Streaming << QObexPushService::Ready
                                      << QObexPushService::Disconnecting << QObexPushService::Closed);

        QTest::newRow("Connect, get, disconnect")
                << false
                << (QList<QVariant>() << QObex::Connect << QObex::Get << QObex::Disconnect)
                << (QStringList() << connectSignals << getSignals << disconnectSignals)
                << (QList<QVariant>() << QObexPushService::Connecting << QObexPushService::Ready
                                      << QObexPushService::Streaming << QObexPushService::Ready
                                      << QObexPushService::Disconnecting << QObexPushService::Closed);

        QTest::newRow("Connect, get * 2, disconnect")
                << false
                << (QList<QVariant>() << QObex::Connect << QObex::Get << QObex::Get  << QObex::Disconnect)
                << (QStringList() << connectSignals << getSignals
                                  << getSignals << disconnectSignals)
                << (QList<QVariant>() << QObexPushService::Connecting << QObexPushService::Ready
                                      << QObexPushService::Streaming << QObexPushService::Ready
                                      << QObexPushService::Streaming << QObexPushService::Ready
                                      << QObexPushService::Disconnecting << QObexPushService::Closed);

        QTest::newRow("Connect, put, get, disconnect")
                << false
                << (QList<QVariant>() << QObex::Connect << QObex::Put << QObex::Get  << QObex::Disconnect)
                << (QStringList() << connectSignals << putSignals
                                  << getSignals << disconnectSignals)
                << (QList<QVariant>() << QObexPushService::Connecting << QObexPushService::Ready
                                      << QObexPushService::Streaming << QObexPushService::Ready
                                      << QObexPushService::Streaming << QObexPushService::Ready
                                      << QObexPushService::Disconnecting << QObexPushService::Closed);

        QTest::newRow("Connect, get, put, disconnect")
                << false
                << (QList<QVariant>() << QObex::Connect << QObex::Get << QObex::Put  << QObex::Disconnect)
                << (QStringList() << connectSignals << getSignals
                                  << putSignals << disconnectSignals)
                << (QList<QVariant>() << QObexPushService::Connecting << QObexPushService::Ready
                                      << QObexPushService::Streaming << QObexPushService::Ready
                                      << QObexPushService::Streaming << QObexPushService::Ready
                                      << QObexPushService::Disconnecting << QObexPushService::Closed);

        // doesn't make sense, but service should handle it as normal
        QTest::newRow("Connect, connect")
                << false
                << (QList<QVariant>() << QObex::Connect << QObex::Connect)
                << (QStringList() << connectSignals << connectSignals)
                << (QList<QVariant>() << QObexPushService::Connecting << QObexPushService::Ready
                                      << QObexPushService::Connecting << QObexPushService::Ready);

        QTest::newRow("Disconnect, disconnect")
                << true     // client's 2nd disconnect request will fail
                << (QList<QVariant>() << QObex::Disconnect << QObex::Disconnect)
                << disconnectSignals
                << (QList<QVariant>() << QObexPushService::Disconnecting << QObexPushService::Closed);
    }

    void genericSignals()
    {
        QFETCH(bool, finishWithError);
        QFETCH(QStringList, expectedSignals);
        QFETCH(QList<QVariant>, expectedStates);
        QFETCH(QList<QVariant>, requests);

        QBuffer buffer;

        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        QSignalSpy spy(client,SIGNAL(done(bool)));
        for (int i=0; i<requests.size(); i++) {
            // send a valid request
            switch (requests[i].toInt()) {
                case QObex::Connect:
                    client->connect();
                    break;
                case QObex::Disconnect:
                    client->disconnect();
                    break;
                case QObex::Put:
                    client->put(QObexHeader(), QByteArray("abcde"));
                    break;
                case QObex::Get:
                {
                    m_push->setBusinessCard(QByteArray("abcde"));
                    QObexHeader h;
                    h.setType(VALID_GET_TYPE);
                    client->get(h, &buffer);
                    break;
                }
                default:
                    Q_ASSERT(false);
            }
        }
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << finishWithError );

        QCOMPARE(m_pushHandler->m_emittedSignals, expectedSignals);
        QCOMPARE(m_pushHandler->m_states, expectedStates);

        delete client;
    }

    void dataTransferProgress_data()
    {
        QTest::addColumn<QObex::Request>("request");
        QTest::addColumn<QObexHeader>("header");
        QTest::addColumn<QByteArray>("bytes");
        QTest::addColumn<int>("minProgressSignalCount");
        QTest::addColumn<int>("maxProgressSignalCount");
        QTest::addColumn<int>("totalValue");

        QObexHeader h;
        QByteArray smallBytes = get_some_data(50);
        QByteArray largeBytes = get_some_data(5000);

        // if no bytes are sent, there shouldn't be any dataTransferProgress signals
        h.clear();
        QTest::newRow("put empty bytes")
                << QObex::Put << h << QByteArray() << 0 << 0 << 0;

        h.clear();
        QTest::newRow("put small bytes, no size given")
                << QObex::Put << h << smallBytes << 1 << 1 << 0;

        h.clear();
        h.setLength(smallBytes.size());
        QTest::newRow("put small bytes, size given")
                << QObex::Put << h << smallBytes << 1 << 1 << smallBytes.size();

        // don't know exactly how many packets will be sent
        h.clear();
        QTest::newRow("put large bytes, no size given")
                << QObex::Put << h << largeBytes << 2 << 50 << 0;

        h.clear();
        h.setLength(largeBytes.size());
        QTest::newRow("put large bytes, size given")
                << QObex::Put << h << largeBytes << 2 << 50 << largeBytes.size();

        h.clear();
        h.setType(VALID_GET_TYPE);
        QTest::newRow("bad get, empty bytes")
                << QObex::Get << h << QByteArray() << 0 << 0 << 0;

        h.clear();
        h.setType(VALID_GET_TYPE);
        QTest::newRow("get small bytes")
                << QObex::Get << h << smallBytes << 1 << 1 << smallBytes.size();

        h.clear();
        h.setType(VALID_GET_TYPE);
        QTest::newRow("get large bytes")
                << QObex::Get << h << largeBytes << 2 << 10 << largeBytes.size();
    }

    void dataTransferProgress()
    {
        QFETCH(QObex::Request, request);
        QFETCH(QObexHeader, header);
        QFETCH(QByteArray, bytes);
        QFETCH(int, minProgressSignalCount);
        QFETCH(int, maxProgressSignalCount);
        QFETCH(int, totalValue);

        QBuffer buffer;

        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        QSignalSpy spy(client,SIGNAL(done(bool)));
        switch (request) {
            case QObex::Put:
                client->put(header, bytes);
                break;
            case QObex::Get:
            {
                m_push->setBusinessCard(bytes);
                client->get(header, &buffer);
                break;
            }
            default:
                Q_ASSERT(false);
        }
        if (request == QObex::Get && bytes.isEmpty()) {
            QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << true );
        } else {
            QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << false );
        }

        int progressSignalCount = m_pushHandler->m_emittedSignals.count("dataTransferProgress");
        QVERIFY(progressSignalCount >= minProgressSignalCount);
        QVERIFY(progressSignalCount <= maxProgressSignalCount);

        QCOMPARE(m_pushHandler->m_doneValues.size(), progressSignalCount);
        if (m_pushHandler->m_doneValues.size() > 0) {
            QVERIFY(m_pushHandler->m_doneValues[0] > 0);
            QCOMPARE(m_pushHandler->m_doneValues.last(), qint64(bytes.size()));
        }

        // all total values should have been the same
        if (m_pushHandler->m_totalValues.size() > 0) {
            QCOMPARE(m_pushHandler->m_totalValues.toSet().count(), 1);
            QCOMPARE(m_pushHandler->m_totalValues[0], qint64(totalValue));
        }

        // check there are no other signals in between dataTransferProgress signals
        if (progressSignalCount > 0) {
            int index = m_pushHandler->m_emittedSignals.indexOf("dataTransferProgress");
            QStringList progressSignals = m_pushHandler->m_emittedSignals.mid(
                    index, progressSignalCount);
            QCOMPARE(progressSignals.toSet().size(), 1);
        }

        delete client;
    }

    void requestFinished_abortPut_data()
    {
        QTest::addColumn<bool>("useDefaultAcceptFile");
        QTest::newRow("use default acceptFile") << true;
        QTest::newRow("use custom acceptFile") << false;
    }

    // if client aborts put, should finish with Aborted error
    void requestFinished_abortPut()
    {
        QFETCH(bool, useDefaultAcceptFile);

        CustomPushService *cpush = 0;
        if (!useDefaultAcceptFile) {
            delete m_push;
            m_push = 0;
            delete m_pushHandler;
            m_pushHandler = 0;
            cpush = new CustomPushService(m_serverSocket, this);
            m_pushHandler = new PushServiceSignalReceiver(cpush);

            QBuffer *buffer = new QBuffer(cpush);
            cpush->m_device = buffer;
            cpush->m_device->open(QIODevice::WriteOnly);
        }

        QObexHeader h;
        h.setName("some_file_for_qobexpushservice_test");

        QString filePath = QDir::homePath() + QDir::separator() + h.name();
        if (QFile::exists(filePath)) {
            QVERIFY(QFile(filePath).remove());
        }

        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        connect(client, SIGNAL(dataTransferProgress(qint64,qint64)),
                client, SLOT(abort()));
        QSignalSpy spy(client,SIGNAL(done(bool)));
        client->put(h, get_some_data(5000));
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << true );

        QCOMPARE(client->error(), QObexClientSession::Aborted);

        // double-check the put really did start
        QCOMPARE(m_pushHandler->m_emittedSignals.count("putRequested"), 1);
        QCOMPARE(m_pushHandler->m_putNames, QStringList(h.name()));

        // should get requestFinished() with error
        QCOMPARE(m_pushHandler->m_emittedSignals.count("requestFinished"), 1);
        QCOMPARE(m_pushHandler->m_requestErrors,
                 (QList<QObexPushService::Error>() << QObexPushService::Aborted));

        // if using the default acceptFile() implementation, the file should
        // have been deleted. Otherwise, the file shouldn't be touched.
        if (useDefaultAcceptFile) {
            QVERIFY(!QFile::exists(filePath));
        } else {
            Q_ASSERT(cpush != 0);
            QVERIFY(cpush->m_device != 0);
            QVERIFY(cpush->m_device->isOpen());
        }

        delete client;
    }

    void requestFinished_abortGet()
    {
        QObexHeader h;
        h.setType(VALID_GET_TYPE);
        QBuffer buffer;

        m_push->setBusinessCard(get_some_data(5000));

        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        connect(client, SIGNAL(dataTransferProgress(qint64,qint64)),
                client, SLOT(abort()));
        QSignalSpy spy(client,SIGNAL(done(bool)));
        client->get(h, &buffer);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << true );

        QCOMPARE(client->error(), QObexClientSession::Aborted);

        // double-check the put really did start
        QCOMPARE(m_pushHandler->m_emittedSignals.count("businessCardRequested"), 1);

        // should get requestFinished() with error
        QCOMPARE(m_pushHandler->m_emittedSignals.count("requestFinished"), 1);
        QCOMPARE(m_pushHandler->m_requestErrors,
                 (QList<QObexPushService::Error>() << QObexPushService::Aborted));

        // bytes should not have been received
        buffer.close();
        QVERIFY(buffer.data() != m_push->businessCard());

        delete client;
    }

    void requestFinished_deleteSelf_data()
    {
        QTest::addColumn<QObex::Request>("request");

        QHash<QObex::Request, QString> requests;
        requests.insert(QObex::Put, "put");
        requests.insert(QObex::Get, "get");

        foreach(QObex::Request req, requests.keys()) {
            QTest::newRow(QTest::toString(requests[req])) << req;
        }
    }

    // Test if you delete the service as soon as requestFinished() is done
    // for a request, the client should still get the response.
    void requestFinished_deleteSelf()
    {
        QFETCH(QObex::Request, request);

        connect(m_push, SIGNAL(requestFinished(bool)),
                m_push, SLOT(deleteLater()));

        QBuffer buffer;
        QByteArray bytes("abcde");

        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        switch (request) {
            case QObex::Put:
                client->put(QObexHeader(), bytes);
                break;
            case QObex::Get:
            {
                m_push->setBusinessCard(bytes);
                QObexHeader h;
                h.setType(VALID_GET_TYPE);
                client->get(h, &buffer);
                break;
            }
            default:
                Q_ASSERT(false);
        }

        // wait for m_push to be destroyed
        // Unfortunately we have to take special measures here;
        // QTest::qWait (and therefore QTRY_*) call
        // QCoreApplication::processEvents(), which won't post deferred
        // deletion events.
        QSignalSpy destroyedSpy(m_push,SIGNAL(destroyed()));
        QEventLoop loop;
        connect( m_push, SIGNAL(destroyed()), &loop, SLOT(quit()) );
        QTimer::singleShot(5000, &loop, SLOT(quit()));
        loop.exec();
        QVERIFY( destroyedSpy.count() );

        // wait for client to process the response data
        {
            QSignalSpy spy(client,SIGNAL(done(bool)));
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << false );
        }

        QCOMPARE(client->lastResponseCode(), QObex::Success);
        if (request == QObex::Get) {
            QCOMPARE(buffer.data(), bytes);
        }

        delete client;
    }

protected slots:

    void deleteServerSocket()
    {
        delete m_serverSocket;
        m_serverSocket = 0;
    }

private slots:

    void deleteSocketDuringRequest_data()
    {
        QTest::addColumn<QObex::Request>("request");
        QTest::newRow("put") << QObex::Put;
        QTest::newRow("get") << QObex::Get;
    }

    void deleteSocketDuringRequest()
    {
        QFETCH(QObex::Request, request);
        QByteArray bytes = get_some_data(15000);

        QObexHeader h;
        if (request == QObex::Get) {
            h.setType(VALID_GET_TYPE);
            m_push->setBusinessCard(bytes);
        }
        QBuffer buffer;


        QObexClientSession *client = new QObexClientSession(m_clientSocket);

        // delete socket as soon as dataTransferProgress() is emitted
        connect(m_push, SIGNAL(dataTransferProgress(qint64,qint64)),
                this, SLOT(deleteServerSocket()));

        QSignalSpy spy(m_push,SIGNAL(done(bool)));
        if (request == QObex::Put)
            client->put(h, bytes);
        else
            client->get(h, &buffer);
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << true );

        // check the request was started
        if (request == QObex::Put)
            QCOMPARE(m_pushHandler->m_emittedSignals.count("putRequested"), 1);
        else
            QCOMPARE(m_pushHandler->m_emittedSignals.count("businessCardRequested"), 1);

        // should get requestFinished() with error
        QCOMPARE(m_pushHandler->m_emittedSignals.count("requestFinished"), 1);
        QCOMPARE(m_pushHandler->m_requestErrors,
                 (QList<QObexPushService::Error>() << QObexPushService::ConnectionError));

        // should get done() signal since lost connection
        QCOMPARE(m_pushHandler->m_emittedSignals.count("done"), 1);
        QCOMPARE(m_pushHandler->m_doneErrors,
                 (QList<QObexPushService::Error>() << QObexPushService::ConnectionError));

        delete client;
    }



    //-----------------------------------------------------

protected slots:

    void closeServerSocket()
    {
        QIODevice *d = m_serverSocket;
        d->close();
    }

private slots:

    void lostConnectionDuringAcceptPut_data()
    {
        QTest::addColumn<bool>("acceptPut");
        QTest::addColumn<bool>("deleteWhenDone");
        QTest::newRow("accept, delete") << true << true;
        QTest::newRow("accept, don't delete") << true << false;
        QTest::newRow("don't accept, delete") << false << true;
        QTest::newRow("don't accept, don't delete") << false << false;
    }

    // If you pause during put() callback to show the user an "Accept this file?"
    // dialog, and the transport is broken during this point, everything
    // should still be ok when the dialog is accepted or rejected (i.e. whatever is
    // returned from acceptFile() should be ignored, and it shouldn't segfault).
    // This is a scenario that occurs in obexservicemanager.cpp and it seems to
    // be a likely scenario for users of a push service.
    void lostConnectionDuringAcceptPut()
    {
        QFETCH(bool, acceptPut);
        QFETCH(bool, deleteWhenDone);

        delete m_push;
        m_push = 0;
        delete m_pushHandler;
        m_pushHandler = 0;

        QPointer<PushServiceWithUI> cpush = new PushServiceWithUI(m_serverSocket, this);
        if (deleteWhenDone)
            connect(cpush, SIGNAL(done(bool)), cpush, SLOT(deleteLater()));
        connect(cpush, SIGNAL(pausedForUI()), this, SLOT(closeServerSocket()));

        if (acceptPut) {
            connect(m_serverSocket, SIGNAL(aboutToClose()),
                    cpush->m_msgBox, SLOT(accept()));
        } else {
            connect(m_serverSocket, SIGNAL(aboutToClose()),
                    cpush->m_msgBox, SLOT(reject()));
        }

        QSignalSpy spy(cpush,SIGNAL(done(bool)));
        QObexClientSession *client = new QObexClientSession(m_clientSocket);
        client->put(QObexHeader(), get_some_data(500));
        QTRY_VERIFY( spy.count() == 1 );
        QCOMPARE( spy.takeAt(0), QVariantList() << true );

        delete client;
        delete cpush;
    }

};


QTEST_MAIN(tst_QObexPushService)
#include "tst_qobexpushservice.moc"
