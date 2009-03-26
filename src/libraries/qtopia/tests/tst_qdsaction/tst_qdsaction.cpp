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

#define private public
#define protected public
#include "qdsaction_p.h"
#include <QDSAction>
#undef private
#undef protected

#include <QDSActionRequest>
#include <QDSData>
#include <QDSServiceInfo>
#include <QTextStream>

#include <QEventLoop>
#include <QUniqueId>
#include <QtopiaApplication>
#include <QtopiaIpcEnvelope>
#include <QThread>

#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QObjectCleanupHandler>
#include <QSignalSpy>
#include <shared/util.h>

#include "../qdsunittest.h"

// --------------------------- Constants --------------------------------------

static const QString    SERVICES_DIR               = Qtopia::qtopiaDir() + "etc";
static const QString    QDS_SERVICES_DIR           = "qds";
static const QString    QDS_SERVICES_DIR_BACKUP    = "qdsbackup";
static const QString    QDS_RESPONSE_BASE          = "QPE/QDSResponse/";

static const QString    QDS_SERVICES_SERVICE_A     = "QDSProviderStub";
static const QString    QDS_SERVICES_NAME_A        = "respondTestA";
static const QString    QDS_SERVICES_REQDATATYPE_A = "";
static const QString    QDS_SERVICES_RESDATATYPE_A = "";
static const QStringList QDS_SERVICES_ATTRIBUTES_A = QStringList( "respond" );
static const QString    QDS_SERVICES_DESCRIPTION_A = "Check QDS connection";
static const QString    QDS_SERVICES_ICON_A        = "";

static const QString    QDS_SERVICES_SERVICE_B     = "QDSProviderStub";
static const QString    QDS_SERVICES_NAME_B        = "respondDataTestA";
static const QString    QDS_SERVICES_REQDATATYPE_B = "";
static const QString    QDS_SERVICES_RESDATATYPE_B = "text/plain";
static const QStringList QDS_SERVICES_ATTRIBUTES_B = QStringList( "respond" );
static const QString    QDS_SERVICES_DESCRIPTION_B = "Check QDS connection";
static const QString    QDS_SERVICES_ICON_B        = "";

static const QString    QDS_SERVICES_SERVICE_C     = "QDSProviderStub";
static const QString    QDS_SERVICES_NAME_C        = "timeoutTestA";
static const QString    QDS_SERVICES_REQDATATYPE_C = "";
static const QString    QDS_SERVICES_RESDATATYPE_C = "";
static const QStringList QDS_SERVICES_ATTRIBUTES_C = QStringList( "timeout" );
static const QString    QDS_SERVICES_DESCRIPTION_C = "Check QDS connection";
static const QString    QDS_SERVICES_ICON_C        = "";

static const QString    QDS_SERVICES_SERVICE_D     = "QDSProviderStub";
static const QString    QDS_SERVICES_NAME_D        = "errorTestA";
static const QString    QDS_SERVICES_REQDATATYPE_D = "";
static const QString    QDS_SERVICES_RESDATATYPE_D = "";
static const QStringList QDS_SERVICES_ATTRIBUTES_D = QStringList( "error" );
static const QString    QDS_SERVICES_DESCRIPTION_D = "Check QDS connection";
static const QString    QDS_SERVICES_ICON_D        = "";

static const QString    QDS_SERVICES_SERVICE_E     = "QDSProviderStub";
static const QString    QDS_SERVICES_NAME_E        = "respondTestB";
static const QString    QDS_SERVICES_REQDATATYPE_E = "text/plain";
static const QString    QDS_SERVICES_RESDATATYPE_E = "";
static const QStringList QDS_SERVICES_ATTRIBUTES_E = QStringList( "respondWait" );
static const QString    QDS_SERVICES_DESCRIPTION_E = "Check QDS connection";
static const QString    QDS_SERVICES_ICON_E        = "";

static const QString    QDS_SERVICES_SERVICE_F     = "QDSProviderStub";
static const QString    QDS_SERVICES_NAME_F        = "respondDataTestB";
static const QString    QDS_SERVICES_REQDATATYPE_F = "text/plain;text/html";
static const QString    QDS_SERVICES_RESDATATYPE_F = "text/plain;text/html*";
static const QStringList QDS_SERVICES_ATTRIBUTES_F = QStringList( "respond" );
static const QString    QDS_SERVICES_DESCRIPTION_F = "Check QDS connection";
static const QString    QDS_SERVICES_ICON_F        = "";

static const QString    QDS_SERVICES_SERVICE_G     = "QDSProviderStub";
static const QString    QDS_SERVICES_NAME_G        = "timeoutTestB";
static const QString    QDS_SERVICES_REQDATATYPE_G = "text/plain";
static const QString    QDS_SERVICES_RESDATATYPE_G = "";
static const QStringList QDS_SERVICES_ATTRIBUTES_G = QStringList( "timeout" );
static const QString    QDS_SERVICES_DESCRIPTION_G = "Check QDS connection";
static const QString    QDS_SERVICES_ICON_G        = "";

static const QString    QDS_SERVICES_SERVICE_H     = "QDSProviderStub";
static const QString    QDS_SERVICES_NAME_H        = "errorTestB";
static const QString    QDS_SERVICES_REQDATATYPE_H = "text/plain";
static const QString    QDS_SERVICES_RESDATATYPE_H = "";
static const QStringList QDS_SERVICES_ATTRIBUTES_H = QStringList( "error" );
static const QString    QDS_SERVICES_DESCRIPTION_H = "Check QDS connection";
static const QString    QDS_SERVICES_ICON_H        = "";

#define QCHECKED(code) \
do {\
    code;\
    if (QTest::currentTestFailed()) return;\
} while(0)

namespace QTest {
template<> inline
bool qCompare(QDSData const &a, QDSData const &b,
    const char *actual, const char *expected, const char *file, int line) {
    return qCompare(a.type(), b.type(),
        QString("%1.type()").arg(actual).toLatin1().constData(),
        QString("%1.type()").arg(expected).toLatin1().constData(),
        file, line) && qCompare(a.data(), b.data(),
        QString("%1.data()").arg(actual).toLatin1().constData(),
        QString("%1.data()").arg(expected).toLatin1().constData(),
        file, line);
}
}


//TESTED_CLASS=QDSAction
//TESTED_FILES=src/libraries/qtopia/qdsaction.cpp

/*
    The tst_QDSAction class is a unit test for the QDSAction class.
*/
class tst_QDSAction : public QObject
{
    Q_OBJECT
protected slots:
    void initTestCase();
    void cleanupTestCase();

private slots:

    void execRespondTestCase();
    void execRespondDataTestCase();
    void execTimeoutTestCase();
    void execDataRespondTestCase();
    void execDataRespondDataTestCase();
    void execDataTimeoutTestCase();
    void execSendInvalidDataTestCase();

    void invokeRespondTestCase();
    void invokeRespondDataTestCase();
    void invokeTimeoutTestCase();
    void invokeDataRespondTestCase();
    void invokeDataRespondDataTestCase();
    void invokeDataTimeoutTestCase();
    void invokeSendInvalidDataTestCase();

private:
    /* These two tests disabled until bug 147711 is fixed */
    void execInvalidTestCase();
    void invokeInvalidTestCase();

    void createTestServiceFile( const QString& service,
                                const QString& name,
                                const QString& requestDataType,
                                const QString& responseDataType,
                                const QStringList& attributes,
                                const QString& description,
                                const QString& icon );

    void removeTestServiceFile( const QString& service );

    void createProviderStubServiceFiles();
    void removeProviderStubServiceFiles();

    void execTest(int, QString const&, QDSAction&, QDSServiceInfo&, QUniqueId&, QString&, QString&,
                  QString&, QDSData&, QDSData&);
    void invokeTest(QString const&, QDSAction&, QDSServiceInfo&, QUniqueId&, QString&, QString&, QString&,
                    QDSData&, QDSData&);
};

QTEST_APP_MAIN( tst_QDSAction, QtopiaApplication )

class QtopiaIpcEnvelopeHolder : public QObject
{
Q_OBJECT
public:
    ~QtopiaIpcEnvelopeHolder()
    { delete env; }

private:
    QtopiaIpcEnvelopeHolder() : env(0) {}
    QtopiaIpcEnvelope* env;
    friend class QtopiaDelayedIpcEnvelope;
};

/* Works like QtopiaIpcEnvelope, but only sends when the event loop runs. */
struct QtopiaDelayedIpcEnvelope
{
    QtopiaDelayedIpcEnvelope(const QString & channel, const QString & message)
    {
        holder = new QtopiaIpcEnvelopeHolder;
        holder->env = new QtopiaIpcEnvelope(channel, message);
    }

    virtual ~QtopiaDelayedIpcEnvelope()
    { holder->deleteLater(); }

    template <typename T>
    QtopiaDelayedIpcEnvelope& operator<<(T const& value)
    {
        (*holder->env) << value;
        return *this;
    }

private:
    QtopiaIpcEnvelopeHolder* holder;
};

/* Initialise all variables in current test function.
 * A is one of A..H, B is request data, C is response data.
 */
#define SETUP(A,B,C) \
    QDSAction action(QDS_SERVICES_NAME_##A, \
                     QDS_SERVICES_SERVICE_##A );\
\
    QDSServiceInfo info(QDS_SERVICES_NAME_##A, QDS_SERVICES_SERVICE_##A); \
    QVERIFY( action.serviceInfo() == info ); \
\
    QUniqueId id = action.id(); \
    QVERIFY( id != QUniqueId() ); \
    QVERIFY( !action.isActive() ); \
\
    QString requestChannel("QPE/Application/" + QDS_SERVICES_SERVICE_##A.toLower()); \
    QString requestMessage(QDS_SERVICES_SERVICE_##A + "::" + info.name() + "(QDSActionRequest)"); \
    QString responseChannel(action.d->responseChannel()); \
    QDSData requestData = B; \
    QDSData responseData = C;
// END SETUP

/* Initialise all variables in current test function, testing a valid QDS service */
#define SETUP_VALID(A,B,C) \
    SETUP(A,B,C); \
    QVERIFY( action.isValid() );

/* Initialise all variables in current test function, testing an invalid QDS service */
#define SETUP_INVALID(A,B,C) \
    SETUP(A,B,C); \
    QVERIFY( !action.isValid() );

#define RUN_EXEC_TEST(A,B) \
    QCHECKED( execTest(A, B, action, info, id, requestChannel, requestMessage, responseChannel,\
                       requestData, responseData) );

#define RUN_INVOKE_TEST(A) \
    QCHECKED( invokeTest(A, action, info, id, requestChannel, requestMessage, responseChannel,\
                         requestData, responseData) );

/* Run a test on QDSAction::exec() using data set up by SETUP.
 * Expected return value is ret, expected error message is err. */
void tst_QDSAction::execTest(int ret, QString const &err, QDSAction &action, QDSServiceInfo &info, QUniqueId &id,
                             QString &requestChannel, QString &requestMessage, QString &responseChannel,
                             QDSData &requestData, QDSData &responseData) {
    QSignalSpy *spy = 0;
    QObjectCleanupHandler cleanup;
    if (ret == QDSAction::CompleteData || ret == QDSAction::Complete ||
        ((ret == QDSAction::Error) && (err == tr("timeout")) )) {

        if (ret != QDSAction::Error) {
            /* When a response is received, we should get this signal. */
            if (responseData != QDSData()) {
                spy = new QSignalSpy(&action,SIGNAL(response(QUniqueId,QDSData)));
                cleanup.add(spy);
                QtopiaDelayedIpcEnvelope env(responseChannel, "response(QDSData)"); env << responseData;
            } else {
                spy = new QSignalSpy(&action,SIGNAL(response(QUniqueId)));
                cleanup.add(spy);
                QtopiaDelayedIpcEnvelope env(responseChannel, "response()");
            }
        }
    }

    if (requestData != QDSData()) {
        QVERIFY2( action.exec(requestData) == ret, action.errorMessage().toLatin1() );
    } else {
        QVERIFY2( action.exec() == ret, action.errorMessage().toLatin1() );
    }

    if (ret == QDSAction::CompleteData || ret == QDSAction::Complete ||
        ((ret == QDSAction::Error) && (err == tr("timeout")) )) {

        if (ret != QDSAction::Error) {
            if (responseData != QDSData()) {
                QTRY_VERIFY( spy->count() == 1 );
                QCOMPARE( spy->last().at(0).value<QUniqueId>(), id );
                QCOMPARE( spy->last().at(1).value<QDSData>(), responseData );
                spy->takeLast();
            } else {
                QTRY_VERIFY( spy->count() == 1 );
                QCOMPARE( spy->takeLast().at(0).value<QUniqueId>(), id );
            }
        }
    }

    QCOMPARE( action.responseData(), responseData );
    QCOMPARE( action.errorMessage(), err );

    QVERIFY( !action.isActive() );
    QVERIFY( action.id() == id );
}

/* Run a test on QDSAction::invoke() using data set up by SETUP.
 * Expected error message is err. */
void tst_QDSAction::invokeTest(QString const &err, QDSAction &action, QDSServiceInfo &info, QUniqueId &id,
                               QString &requestChannel, QString &requestMessage, QString &responseChannel,
                               QDSData &requestData, QDSData &responseData) {
    bool expect_success = (err == QString());
    bool expect_invoke_success = (expect_success || err == tr("timeout"));

    QSignalSpy errorSpy(&action,SIGNAL(error(QUniqueId,QString)));

    if (requestData != QDSData()) {
        QVERIFY2( action.invoke(requestData) == expect_invoke_success, action.errorMessage().toLatin1() );
    } else {
        QVERIFY2( action.invoke() == expect_invoke_success, action.errorMessage().toLatin1() );
    }

    if (expect_invoke_success) {
        QVERIFY( action.isActive() );
        QVERIFY( action.id() == id );
        QCOMPARE( action.errorMessage(), QString() );
        QCOMPARE( action.responseData(), QDSData() );

        if (expect_success) {
            if (responseData != QDSData()) {
                QSignalSpy spy(&action,SIGNAL(response(QUniqueId,QDSData)));
                /* Respond to the request. */
                { QtopiaIpcEnvelope env(responseChannel, "response(QDSData)"); env << responseData; }
                QTRY_COMPARE(spy.count(), 1);
                QCOMPARE( spy.last().at(0).value<QUniqueId>(), id );
                QCOMPARE( spy.last().at(1).value<QDSData>(), responseData );
            } else {
                QSignalSpy spy(&action,SIGNAL(response(QUniqueId)));
                /* Respond to the request. */
                { QtopiaIpcEnvelope env(responseChannel, "response()"); }
                QTRY_COMPARE(spy.count(), 1);
                QCOMPARE( spy.takeLast().at(0).value<QUniqueId>(), id );
            }
        } else {
            QTest::qWait(30000);
            QTRY_COMPARE(errorSpy.count(), 1);
            QCOMPARE( errorSpy.last().at(0).value<QUniqueId>(), id );
            QCOMPARE( errorSpy.last().at(1).value<QString>(), err );
        }
    }

    QCOMPARE( action.responseData(), responseData );
    QCOMPARE( action.errorMessage(), err );

    QVERIFY( !action.isActive() );
    QVERIFY( action.id() == id );
}

/*? Initialisation before first test case; create all service files. */
void tst_QDSAction::initTestCase()
{
    // Create service files
    createTestServiceFile( QDS_SERVICES_SERVICE_A,
                           QDS_SERVICES_NAME_A,
                           QDS_SERVICES_REQDATATYPE_A,
                           QDS_SERVICES_RESDATATYPE_A,
                           QDS_SERVICES_ATTRIBUTES_A,
                           QDS_SERVICES_DESCRIPTION_A,
                           QDS_SERVICES_ICON_A );

    createTestServiceFile( QDS_SERVICES_SERVICE_B,
                           QDS_SERVICES_NAME_B,
                           QDS_SERVICES_REQDATATYPE_B,
                           QDS_SERVICES_RESDATATYPE_B,
                           QDS_SERVICES_ATTRIBUTES_B,
                           QDS_SERVICES_DESCRIPTION_B,
                           QDS_SERVICES_ICON_B );

    createTestServiceFile( QDS_SERVICES_SERVICE_C,
                           QDS_SERVICES_NAME_C,
                           QDS_SERVICES_REQDATATYPE_C,
                           QDS_SERVICES_RESDATATYPE_C,
                           QDS_SERVICES_ATTRIBUTES_C,
                           QDS_SERVICES_DESCRIPTION_C,
                           QDS_SERVICES_ICON_C );

    createTestServiceFile( QDS_SERVICES_SERVICE_E,
                           QDS_SERVICES_NAME_E,
                           QDS_SERVICES_REQDATATYPE_E,
                           QDS_SERVICES_RESDATATYPE_E,
                           QDS_SERVICES_ATTRIBUTES_E,
                           QDS_SERVICES_DESCRIPTION_E,
                           QDS_SERVICES_ICON_E );

    createTestServiceFile( QDS_SERVICES_SERVICE_F,
                           QDS_SERVICES_NAME_F,
                           QDS_SERVICES_REQDATATYPE_F,
                           QDS_SERVICES_RESDATATYPE_F,
                           QDS_SERVICES_ATTRIBUTES_F,
                           QDS_SERVICES_DESCRIPTION_F,
                           QDS_SERVICES_ICON_F );

    createTestServiceFile( QDS_SERVICES_SERVICE_G,
                           QDS_SERVICES_NAME_G,
                           QDS_SERVICES_REQDATATYPE_G,
                           QDS_SERVICES_RESDATATYPE_G,
                           QDS_SERVICES_ATTRIBUTES_G,
                           QDS_SERVICES_DESCRIPTION_G,
                           QDS_SERVICES_ICON_G );

    createTestServiceFile( QDS_SERVICES_SERVICE_H,
                           QDS_SERVICES_NAME_H,
                           QDS_SERVICES_REQDATATYPE_H,
                           QDS_SERVICES_RESDATATYPE_H,
                           QDS_SERVICES_ATTRIBUTES_H,
                           QDS_SERVICES_DESCRIPTION_H,
                           QDS_SERVICES_ICON_H );

    // Create the provider stub service files
    createProviderStubServiceFiles();
}

/*? Cleanup after last test case; remove all created files. */
void tst_QDSAction::cleanupTestCase()
{
    // Create the provider stub service files
    removeProviderStubServiceFiles();

    // Remove service files
    removeTestServiceFile( QDS_SERVICES_SERVICE_A );
    removeTestServiceFile( QDS_SERVICES_SERVICE_B );
    removeTestServiceFile( QDS_SERVICES_SERVICE_C );
    removeTestServiceFile( QDS_SERVICES_SERVICE_E );
    removeTestServiceFile( QDS_SERVICES_SERVICE_F );
    removeTestServiceFile( QDS_SERVICES_SERVICE_G );
    removeTestServiceFile( QDS_SERVICES_SERVICE_H );
}

/*? Create the QDSProviderStub service file. */
void tst_QDSAction::createProviderStubServiceFiles()
{
    {
        QString filename = Qtopia::qtopiaDir() +
                           "services/QDSProviderStub.service";
        QFile serviceFile( filename );
        if ( !serviceFile.open( QIODevice::WriteOnly ) ) {
            qFatal( QString("Couldn't create the QDSProviderStub service file %1").arg(filename).toLatin1() );
            return;
        }

        QTextStream ts( &serviceFile );
        ts << "[Translation]\n";
        ts << "File=QtopiaServices\n";
        ts << "Context=QDSProviderStub\n";
        ts << "[Service]\n";
        ts << "Actions = \"" << QDS_SERVICES_NAME_A << "(QDSActionRequest)";
        QStringList actions;
        actions << QDS_SERVICES_NAME_B << QDS_SERVICES_NAME_C << /*QDS_SERVICES_NAME_D <<*/ QDS_SERVICES_NAME_E
                << QDS_SERVICES_NAME_F << QDS_SERVICES_NAME_G << QDS_SERVICES_NAME_H;
        foreach (QString a, actions) {
            ts << ";" << a << "(QDSActionRequest)";
        }
        ts << "\"\n";
        ts << "Icon = \n";
        ts << "Name=QDSProviderStub\n";

        serviceFile.close();
    }

    {
        // Create the QDSProviderStub service directory
        QDir servicesDir( Qtopia::qtopiaDir() + "services" );
        if ( !servicesDir.exists( "QDSProviderStub" ) && !servicesDir.mkdir( "QDSProviderStub" ) ) {
            qFatal( QString("Couldn't create QDSProviderStub service directory %1/QDSProviderStub").arg(Qtopia::qtopiaDir() + "services").toLatin1() );
        }

        QString filename = Qtopia::qtopiaDir() +
                           "services/QDSProviderStub/qdsproviderstub";
        QFile serviceFile( filename );
        if ( !serviceFile.open( QIODevice::WriteOnly ) ) {
            qFatal( QString("Couldn't create the QDSProviderStub service file %1").arg(filename).toLatin1() );
            return;
        }

        QTextStream ts( &serviceFile );
        ts << "[Standard]\nVersion=100\n";
        serviceFile.close();
    }
}

/*? Remove the QDSProviderStub service file. */
void tst_QDSAction::removeProviderStubServiceFiles()
{
    QFile::remove( Qtopia::qtopiaDir() + "services/QDSProviderStub.service" );
    QFile::remove( Qtopia::qtopiaDir() + "services/QDSProviderStub/qdsproviderstub" );

    // Remove the QDSProviderStub service directory
    QDir servicesDir( Qtopia::qtopiaDir() + "services" );
    if ( !servicesDir.rmdir( "QDSProviderStub" ) ) {
        qWarning( "Couldn't remove QDSProviderStub service directory" );
    }
}

/*? Create a service file for a specific test service. */
void tst_QDSAction::createTestServiceFile(
    const QString& service,
    const QString& name,
    const QString& requestDataType,
    const QString& responseDataType,
    const QStringList& attributes,
    const QString& description,
    const QString& icon )
{
    // Open the service file
    QString filename = SERVICES_DIR + "/" + QDS_SERVICES_DIR + "/" + service;

    QSettings serviceFile( filename, QSettings::IniFormat );

    // Write the service file
    serviceFile.beginGroup( name );
    serviceFile.setValue( "RequestDataType", requestDataType );
    serviceFile.setValue( "ResponseDataType", responseDataType );
    serviceFile.setValue( "Attributes", attributes );
    serviceFile.setValue( "Description", description );
    serviceFile.setValue( "Icon", icon );
    serviceFile.endGroup();
}

/*? Remove a service file for a specific test service. */
void tst_QDSAction::removeTestServiceFile( const QString& service )
{
    // Remove the service file
    QString filename = SERVICES_DIR + "/" + QDS_SERVICES_DIR + "/" + service;
    QFile serviceFile( filename );
    serviceFile.remove();
}

/*? Test exec with no data. */
void tst_QDSAction::execRespondTestCase()
{
    SETUP_VALID(A, QDSData(), QDSData());
    RUN_EXEC_TEST( QDSAction::Complete, QString() );
}

/*? Test exec with no request data and response data. */
void tst_QDSAction::execRespondDataTestCase()
{
    SETUP_VALID(B, QDSData(), QDSData( QString("hello"), QMimeType("text/plain") ) );
    RUN_EXEC_TEST( QDSAction::CompleteData, QString() );
}

/*? Test exec times out with no request data when no response is given. */
void tst_QDSAction::execTimeoutTestCase()
{
    SETUP_VALID(C, QDSData(), QDSData());
    RUN_EXEC_TEST( QDSAction::Error, QString(tr("timeout")) );
}

/*? Test exec fails when run on an invalid service. */
void tst_QDSAction::execInvalidTestCase()
{
    SETUP_INVALID(D, QDSData(), QDSData());
    RUN_EXEC_TEST( QDSAction::Error, QString(tr("invalid service")) );
}

/*? Test exec with request data and no response data. */
void tst_QDSAction::execDataRespondTestCase()
{
    SETUP_VALID(E, QDSData(QString("hi"), QMimeType("text/plain")), QDSData() );
    RUN_EXEC_TEST( QDSAction::Complete, QString() );
}

/*? Test exec with request data and response data. */
void tst_QDSAction::execDataRespondDataTestCase()
{
    SETUP_VALID(F, QDSData(QString("req"), QMimeType("text/plain")), QDSData(QString("resp"), QMimeType("text/plain")) );
    RUN_EXEC_TEST(QDSAction::CompleteData, QString() );
}

/*? Test exec times out with request data when no response is given. */
void tst_QDSAction::execDataTimeoutTestCase()
{
    SETUP_VALID(G, QDSData(QString("req"), QMimeType("text/plain")), QDSData());
    RUN_EXEC_TEST(QDSAction::Error, QString(tr("timeout")) );
}

/*? Test exec fails when attempting to send data of invalid type. */
void tst_QDSAction::execSendInvalidDataTestCase()
{
    SETUP_VALID(E, QDSData(QString("req"), QMimeType("image/jpeg")), QDSData() );
    RUN_EXEC_TEST( QDSAction::Error, QString(tr("incorrect data type")) );
}

/*? Test invoke with no data. */
void tst_QDSAction::invokeRespondTestCase()
{
    SETUP_VALID(A, QDSData(), QDSData() );
    RUN_INVOKE_TEST( QString() );
}

/*? Test invoke with no request data and response data. */
void tst_QDSAction::invokeRespondDataTestCase()
{
    SETUP_VALID(B, QDSData(), QDSData(QString("some data"), QMimeType("text/plain")) );
    RUN_INVOKE_TEST( QString() );
}

/*? Test invoke times out when not responded to. */
void tst_QDSAction::invokeTimeoutTestCase()
{
    SETUP_VALID(C, QDSData(), QDSData() );
    RUN_INVOKE_TEST( tr("timeout") );
}

/*? Test invoke fails on an invalid service. */
void tst_QDSAction::invokeInvalidTestCase()
{
    SETUP_VALID(D, QDSData(QString("foo"), QMimeType("text/plain")), QDSData());
    RUN_INVOKE_TEST( tr("invalid service") );
}

/*? Test invoke with request data and no response data. */
void tst_QDSAction::invokeDataRespondTestCase()
{
    SETUP_VALID(E, QDSData(QString("foo"), QMimeType("text/plain")), QDSData());
    RUN_INVOKE_TEST(QString());
}

/*? Test invoke with request data and response data. */
void tst_QDSAction::invokeDataRespondDataTestCase()
{
    SETUP_VALID(F, QDSData(QString("foo"), QMimeType("text/plain")), QDSData(QString("bar"), QMimeType("text/plain")));
    RUN_INVOKE_TEST( QString() );
}

/*? Test invoke times out with request data when no response is sent. */
void tst_QDSAction::invokeDataTimeoutTestCase()
{
    SETUP_VALID(G, QDSData(QString("req"), QMimeType("text/plain")), QDSData() );
    RUN_INVOKE_TEST( tr("timeout") );
}

/*? Test invoke fails when attempting to send data of incorrect type. */
void tst_QDSAction::invokeSendInvalidDataTestCase()
{
    SETUP_VALID(E, QDSData( QString("invalid"), QMimeType("image/jpeg")), QDSData() );
    RUN_INVOKE_TEST( tr("incorrect data type") );
}

#include "tst_qdsaction.moc"

