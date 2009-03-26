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

#include <QDSActionRequest>
#include <QDSServiceInfo>
#include <QMimeType>
#include <QDSData>
#include <QtopiaApplication>

#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <shared/util.h>
#include <QSignalSpy>
#include <QtopiaChannel>

// --------------------------- Constants --------------------------------------

static const QString    SERVICES_DIR               = Qtopia::qtopiaDir() + "etc";
static const QString    QDS_SERVICES_DIR           = "qds";
static const QString    QDS_SERVICES_DIR_BACKUP    = "qdsbackup";
static const QString    QDS_ACTION_REQUEST_CHANNEL =
                            "QPE/QDSResponse/tst_qdsactionrequest";

static const QString    QDS_SERVICES_SERVICE_A     = "testApp";
static const QString    QDS_SERVICES_NAME_A        = "respondTestA";
static const QString    QDS_SERVICES_REQDATATYPE_A = "";
static const QString    QDS_SERVICES_RESDATATYPE_A = "";
static const QStringList QDS_SERVICES_ATTRIBUTES_A = QStringList();
static const QString    QDS_SERVICES_DESCRIPTION_A = "Check QDS connection";
static const QString    QDS_SERVICES_ICON_A        = "";

static const QString    QDS_SERVICES_SERVICE_B     = "testApp";
static const QString    QDS_SERVICES_NAME_B        = "respondDataTestB";
static const QString    QDS_SERVICES_REQDATATYPE_B = "";
static const QString    QDS_SERVICES_RESDATATYPE_B = "text/plain";
static const QStringList QDS_SERVICES_ATTRIBUTES_B = QStringList();
static const QString    QDS_SERVICES_DESCRIPTION_B = "Check QDS connection";
static const QString    QDS_SERVICES_ICON_B        = "";

static const QString    QDS_SERVICES_SERVICE_C     = "testApp";
static const QString    QDS_SERVICES_NAME_C        = "respondTestC";
static const QString    QDS_SERVICES_REQDATATYPE_C = "text/plain";
static const QString    QDS_SERVICES_RESDATATYPE_C = "";
static const QStringList QDS_SERVICES_ATTRIBUTES_C = QStringList();
static const QString    QDS_SERVICES_DESCRIPTION_C = "Check QDS connection";
static const QString    QDS_SERVICES_ICON_C        = "";

static const QString    QDS_SERVICES_SERVICE_D     = "testApp";
static const QString    QDS_SERVICES_NAME_D        = "respondDataTestD";
static const QString    QDS_SERVICES_REQDATATYPE_D = "text/plain";
static const QString    QDS_SERVICES_RESDATATYPE_D = "text/plain";
static const QStringList QDS_SERVICES_ATTRIBUTES_D = QStringList();
static const QString    QDS_SERVICES_DESCRIPTION_D = "Check QDS connection";
static const QString    QDS_SERVICES_ICON_D        = "";

static const QString    QDS_SERVICES_SERVICE_INVALID_B     = "half-existent1";
static const QString    QDS_SERVICES_NAME_INVALID_B        = "etherC";
static const QString    QDS_SERVICES_REQDATATYPE_INVALID_B = "";

static const QString    QDS_SERVICES_SERVICE_INVALID_C     = "half-existent2";
static const QString    QDS_SERVICES_NAME_INVALID_C        = "etherD";
static const QString    QDS_SERVICES_REQDATATYPE_INVALID_C = "";
static const QString    QDS_SERVICES_RESDATATYPE_INVALID_C = "";
static const QStringList QDS_SERVICES_ATTRIBUTES_INVALID_C = QStringList();
static const QString    QDS_SERVICES_DESCRIPTION_INVALID_C = "foo";
static const QString    QDS_SERVICES_ICON_INVALID_C = "";

static const QString    QDS_SERVICES_SERVICE_INVALID_A     = "non-existent";
static const QString    QDS_SERVICES_NAME_INVALID_A        = "etherA";
static const QString    QDS_SERVICES_REQDATATYPE_INVALID_A = "";

static const QString    QDS_SERVICES_SERVICE_INVALID_DATA       = "non-existent";
static const QString    QDS_SERVICES_NAME_INVALID_DATA          = "etherB";
static const QString    QDS_SERVICES_REQDATATYPE_INVALID_DATA   = "text/plain";

void rmdir(QString const& path)
{
    QFileInfo fi(path);
    if (!fi.exists()) return;
    if (fi.isDir() && !fi.isSymLink()) {
        QDir dir(path);
        foreach (QString const& sub, dir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot))
            rmdir(path + "/" + sub);
        dir.cdUp();
        if (!dir.rmdir(fi.fileName()))
            qFatal("Could not remove directory '%s'", qPrintable(path));
    } else {
        if (!QFile::remove(path))
            qFatal("Could not remove file '%s'", qPrintable(path));
    }
}

//TESTED_CLASS=QDSActionRequest
//TESTED_FILES=src/libraries/qtopia/qdsactionrequest.cpp

/*
    The tst_QDSActionRequest class is a unit test for the QDSActionReque class.
*/
class tst_QDSActionRequest : public QObject
{
    Q_OBJECT
public:
    tst_QDSActionRequest();
    ~tst_QDSActionRequest();

protected slots:
    void initTestCase();
    void cleanupTestCase();

private slots:
    void noDataTestCase();
    void requestDataTestCase();
    void responseDataTestCase();
    void requestAndResponseDataTestCase();

private:
    /* FIXME disabled pending fix of bug 147711 */
    void invalidServiceTestCase();

    void createTestServiceFile( const QString& service,
                                const QString& name,
                                const QMimeType& requestDataType,
                                const QMimeType& responseDataType,
                                const QStringList& attributes,
                                const QString& description,
                                const QString& icon );

    void removeTestServiceFile( const QString& service );

    void backupExistingServices();
    void restoreExistingServices();

    void createProviderStubServiceFiles();
    void removeProviderStubServiceFiles();

    QtopiaChannel* ipc_ch;
    QSignalSpy* ipc_spy;
    QString ipc_msg;
};

tst_QDSActionRequest::tst_QDSActionRequest()
    : ipc_ch(0), ipc_spy(0)
{}

tst_QDSActionRequest::~tst_QDSActionRequest()
{ delete ipc_ch; delete ipc_spy; }

QTEST_APP_MAIN( tst_QDSActionRequest, QtopiaApplication )
#include "tst_qdsactionrequest.moc"

/*? Initialisation before first test function.  Creates service files. */
void tst_QDSActionRequest::initTestCase()
{
    // Move the existing services to another position during the tests
    backupExistingServices();

    // Create service files
    createTestServiceFile( QDS_SERVICES_SERVICE_A,
                           QDS_SERVICES_NAME_A,
                           QMimeType( QDS_SERVICES_REQDATATYPE_A ),
                           QMimeType( QDS_SERVICES_RESDATATYPE_A ),
                           QDS_SERVICES_ATTRIBUTES_A,
                           QDS_SERVICES_DESCRIPTION_A,
                           QDS_SERVICES_ICON_A );

    createTestServiceFile( QDS_SERVICES_SERVICE_B,
                           QDS_SERVICES_NAME_B,
                           QMimeType( QDS_SERVICES_REQDATATYPE_B ),
                           QMimeType( QDS_SERVICES_RESDATATYPE_B ),
                           QDS_SERVICES_ATTRIBUTES_B,
                           QDS_SERVICES_DESCRIPTION_B,
                           QDS_SERVICES_ICON_B );

    createTestServiceFile( QDS_SERVICES_SERVICE_C,
                           QDS_SERVICES_NAME_C,
                           QMimeType( QDS_SERVICES_REQDATATYPE_C ),
                           QMimeType( QDS_SERVICES_RESDATATYPE_C ),
                           QDS_SERVICES_ATTRIBUTES_C,
                           QDS_SERVICES_DESCRIPTION_C,
                           QDS_SERVICES_ICON_C );

    createTestServiceFile( QDS_SERVICES_SERVICE_D,
                           QDS_SERVICES_NAME_D,
                           QMimeType( QDS_SERVICES_REQDATATYPE_D ),
                           QMimeType( QDS_SERVICES_RESDATATYPE_D ),
                           QDS_SERVICES_ATTRIBUTES_D,
                           QDS_SERVICES_DESCRIPTION_D,
                           QDS_SERVICES_ICON_D );

    createTestServiceFile( QDS_SERVICES_SERVICE_INVALID_C,
                           QDS_SERVICES_NAME_INVALID_C,
                           QMimeType( QDS_SERVICES_REQDATATYPE_INVALID_C ),
                           QMimeType( QDS_SERVICES_RESDATATYPE_INVALID_C ),
                           QDS_SERVICES_ATTRIBUTES_INVALID_C,
                           QDS_SERVICES_DESCRIPTION_INVALID_C,
                           QDS_SERVICES_ICON_INVALID_C );

    createProviderStubServiceFiles();
}

/*? Create the stub service file. */
void tst_QDSActionRequest::createProviderStubServiceFiles()
{
    {
        QString filename = Qtopia::qtopiaDir() +
                           "services/testApp.service";
        QFile serviceFile( filename );
        if ( !serviceFile.open( QIODevice::WriteOnly ) ) {
            QSKIP( QString("Couldn't create the testApp service file %1").arg(filename).toLatin1(), SkipAll );
        }

        QTextStream ts( &serviceFile );
        ts << "[Translation]\n";
        ts << "File=QtopiaServices\n";
        ts << "Context=testApp\n";
        ts << "[Service]\n";
        ts << "Actions = \"" << QDS_SERVICES_NAME_A << "(QDSActionRequest)";
        QStringList actions;
        actions << QDS_SERVICES_NAME_B << QDS_SERVICES_NAME_C << QDS_SERVICES_NAME_D << QDS_SERVICES_NAME_INVALID_B;
        foreach (QString a, actions) {
            ts << ";" << a << "(QDSActionRequest)";
        }
        ts << "\"\n";
        ts << "Icon = \n";
        ts << "Name=testApp\n";

        serviceFile.close();
    }

    {
        // Create the QDSProviderStub service directory
        QDir servicesDir( Qtopia::qtopiaDir() + "services" );
        if ( !servicesDir.exists( "testApp" ) && !servicesDir.mkdir( "testApp" ) ) {
            QSKIP( QString("Couldn't create testApp service directory %1/testApp").arg(Qtopia::qtopiaDir() + "services").toLatin1(), SkipAll );
        }

        QString filename = Qtopia::qtopiaDir() +
                           "services/testApp/testapp";
        QFile serviceFile( filename );
        if ( !serviceFile.open( QIODevice::WriteOnly ) ) {
            QSKIP( QString("Couldn't create the testApp service file %1").arg(filename).toLatin1(), SkipAll );
            return;
        }

        QTextStream ts( &serviceFile );
        ts << "[Standard]\nVersion=100\n";
        serviceFile.close();
    }
}

/*? Remove the stub service files. */
void tst_QDSActionRequest::removeProviderStubServiceFiles()
{
    QFile::remove( Qtopia::qtopiaDir() + "services/testApp.service" );
    QFile::remove( Qtopia::qtopiaDir() + "services/testApp/testapp" );

    // Remove the testApp service directory
    QDir servicesDir( Qtopia::qtopiaDir() + "services" );
    if ( !servicesDir.rmdir( "testApp" ) ) {
        qWarning( "Couldn't remove testApp service directory" );
    }
}

/*? Cleanup after last test case.  Removes service files. */
void tst_QDSActionRequest::cleanupTestCase()
{
    // Remove service files
    removeTestServiceFile( QDS_SERVICES_SERVICE_A );
    removeTestServiceFile( QDS_SERVICES_SERVICE_B );
    removeTestServiceFile( QDS_SERVICES_SERVICE_C );
    removeTestServiceFile( QDS_SERVICES_SERVICE_D );
    removeTestServiceFile( QDS_SERVICES_SERVICE_INVALID_C );

    removeProviderStubServiceFiles();

    restoreExistingServices();
}

/*? Back up existing QDS service files. */
void tst_QDSActionRequest::backupExistingServices()
{
    rmdir(SERVICES_DIR + "/" + QDS_SERVICES_DIR_BACKUP);
    QDir dir( SERVICES_DIR );
    if (!dir.rename( QDS_SERVICES_DIR, QDS_SERVICES_DIR_BACKUP ))
        qFatal("Could not back up QDS directory!");
}

/*? Restore backed up QDS service files. */
void tst_QDSActionRequest::restoreExistingServices()
{
    rmdir(SERVICES_DIR + "/" + QDS_SERVICES_DIR);
    QDir dir( SERVICES_DIR );
    if (!dir.rename( QDS_SERVICES_DIR_BACKUP, QDS_SERVICES_DIR ))
        qFatal("Could not restore backup of QDS directory!");
}

/*? Create service file for a specific service. */
void tst_QDSActionRequest::createTestServiceFile(
    const QString& service,
    const QString& name,
    const QMimeType& requestDataType,
    const QMimeType& responseDataType,
    const QStringList& attributes,
    const QString& description,
    const QString& icon )
{
    // Open the service file
    QString filename = SERVICES_DIR + "/" + QDS_SERVICES_DIR + "/" + service;

    QSettings serviceFile( filename, QSettings::IniFormat );

    // Write the service file
    serviceFile.beginGroup( name );
    serviceFile.setValue( "RequestDataType", requestDataType.id() );
    serviceFile.setValue( "ResponseDataType", responseDataType.id() );
    serviceFile.setValue( "Attributes", attributes );
    serviceFile.setValue( "Description", description );
    serviceFile.setValue( "Icon", icon );
    serviceFile.endGroup();
}

/*? Remove service file for a specific service. */
void tst_QDSActionRequest::removeTestServiceFile( const QString& service )
{
    // Remove the service file
    QString filename = SERVICES_DIR + "/" + QDS_SERVICES_DIR + "/" + service;
    QFile serviceFile( filename );
    serviceFile.remove();
}

/*? Test that a QDSActionRequest is invalid in all cases when it should be. */
void tst_QDSActionRequest::invalidServiceTestCase()
{
    // Test a completely nonexistent service is invalid.
    QDSActionRequest request( QDSServiceInfo( QDS_SERVICES_NAME_INVALID_A,
                                              QDS_SERVICES_SERVICE_INVALID_A ),
                              QDS_ACTION_REQUEST_CHANNEL );

    QVERIFY( !request.isValid() );
    QVERIFY( !request.isComplete() );
    QVERIFY( request.requestData() == QDSData() );
    QVERIFY( request.responseData() == QDSData() );
    QVERIFY( request.serviceInfo() ==
             QDSServiceInfo( QDS_SERVICES_NAME_INVALID_A,
                             QDS_SERVICES_SERVICE_INVALID_A ) );

    // Test that a copy of an invalid service is invalid.
    QDSActionRequest requestCopy( request );

    QVERIFY( !requestCopy.isValid() );
    QVERIFY( !requestCopy.isComplete() );
    QVERIFY( requestCopy.requestData() == QDSData() );
    QVERIFY( requestCopy.responseData() == QDSData() );
    QVERIFY( requestCopy.serviceInfo() ==
             QDSServiceInfo( QDS_SERVICES_NAME_INVALID_A,
                             QDS_SERVICES_SERVICE_INVALID_A ) );

    // Test that a completely nonexistent service with data is invalid.
    QDSActionRequest requestData(
        QDSServiceInfo( QDS_SERVICES_NAME_INVALID_DATA,
                        QDS_SERVICES_SERVICE_INVALID_DATA ),
        QDSData( QByteArray( "Some data" ),
                 QMimeType( QDS_SERVICES_REQDATATYPE_INVALID_DATA ) ),
        QDS_ACTION_REQUEST_CHANNEL );

    QVERIFY( !requestData.isValid() );
    QVERIFY( requestData.isComplete() );
    QVERIFY( requestData.requestData() != QDSData() );
    QVERIFY( requestData.responseData() == QDSData() );
    QVERIFY( requestData.serviceInfo() ==
             QDSServiceInfo( QDS_SERVICES_NAME_INVALID_DATA,
                             QDS_SERVICES_SERVICE_INVALID_DATA ) );

    // Test that a service appearing in the .service file but without its own descriptor file
    // is invalid.
    QDSActionRequest requestB( QDSServiceInfo( QDS_SERVICES_NAME_INVALID_B,
                                               QDS_SERVICES_SERVICE_INVALID_B ),
                               QDS_ACTION_REQUEST_CHANNEL );

    QVERIFY( !requestB.isValid() );
    QVERIFY( !requestB.isComplete() );
    QVERIFY( requestB.requestData() == QDSData() );
    QVERIFY( requestB.responseData() == QDSData() );
    QVERIFY( requestB.serviceInfo() ==
             QDSServiceInfo( QDS_SERVICES_NAME_INVALID_B,
                             QDS_SERVICES_SERVICE_INVALID_B ) );

    // Test that a service not appearing in the .service file but with its own descriptor file
    // is invalid.
    QDSActionRequest requestC( QDSServiceInfo( QDS_SERVICES_NAME_INVALID_C,
                                               QDS_SERVICES_SERVICE_INVALID_C ),
                               QDS_ACTION_REQUEST_CHANNEL );

    QVERIFY( !requestB.isValid() );
    QVERIFY( !requestB.isComplete() );
    QVERIFY( requestB.requestData() == QDSData() );
    QVERIFY( requestB.responseData() == QDSData() );
    QVERIFY( requestB.serviceInfo() ==
             QDSServiceInfo( QDS_SERVICES_NAME_INVALID_C,
                             QDS_SERVICES_SERVICE_INVALID_C ) );
}

#define IPC_EXPECT(channel, message)                                         \
    delete ipc_ch;                                                           \
    ipc_ch = new QtopiaChannel(channel);                                     \
    delete ipc_spy;                                                          \
    ipc_spy = new QSignalSpy(ipc_ch, SIGNAL(received(QString,QByteArray))); \
    ipc_msg = QString(message);

#define IPC_WAIT(...)                                                          \
    for (int _ii = 0; _ii < 5000; _ii += 100, QTest::qWait(100)) {             \
        while (ipc_spy->count() && ipc_spy->at(0).at(0).toString() != ipc_msg) \
            ipc_spy->removeFirst();                                            \
        if (ipc_spy->count() >= 1) break;                                      \
    }                                                                          \
    QVERIFY(ipc_spy->count() >= 1);                                            \
    QCOMPARE(ipc_spy->takeAt(0).at(0).toString(), ipc_msg);                    \
    ipc_spy->clear();

/*? Test requesting and responding on a service with no data. */
void tst_QDSActionRequest::noDataTestCase()
{
    // Attempt to create with data when we shouldn't.
    {
        // ignore heartbeat()
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "error(QString)");
        QDSActionRequest request( QDSServiceInfo( QDS_SERVICES_NAME_A,
                                                  QDS_SERVICES_SERVICE_A ),
                                  QDSData( QByteArray( "Some data" ),
                                           QMimeType( "text/plain" ) ),
                                  QDS_ACTION_REQUEST_CHANNEL );
        IPC_WAIT( tr( "request contained unexpected data" ) );

        QVERIFY( !request.isValid() );
        QVERIFY( request.isComplete() );
        QVERIFY( request.requestData() != QDSData() );
        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.serviceInfo() ==
                 QDSServiceInfo( QDS_SERVICES_NAME_A,
                                 QDS_SERVICES_SERVICE_A ) );

    }

    // Attempt to respond with data when we shouldn't.
    {
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "heartbeat()");
        QDSActionRequest request( QDSServiceInfo( QDS_SERVICES_NAME_A,
                                                  QDS_SERVICES_SERVICE_A ),
                                  QDS_ACTION_REQUEST_CHANNEL );
        IPC_WAIT();

        QVERIFY( request.isValid() );
        QVERIFY( !request.isComplete() );
        QVERIFY( request.requestData() == QDSData() );
        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.serviceInfo() ==
                 QDSServiceInfo( QDS_SERVICES_NAME_A,
                                 QDS_SERVICES_SERVICE_A ) );

    
        QVERIFY( !request.respond( QDSData( QByteArray( "Some data" ),
                                            QMimeType( "text/plain" ) ) ) );

        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( !request.isComplete() );
    }

    // Attempt to respond successfully
    {
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "heartbeat()");
        QDSActionRequest request( QDSServiceInfo( QDS_SERVICES_NAME_A,
                                                  QDS_SERVICES_SERVICE_A ),
                                  QDS_ACTION_REQUEST_CHANNEL );
        IPC_WAIT();

        QVERIFY( request.isValid() );
        QVERIFY( !request.isComplete() );
        QVERIFY( request.requestData() == QDSData() );
        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.serviceInfo() ==
                 QDSServiceInfo( QDS_SERVICES_NAME_A,
                                 QDS_SERVICES_SERVICE_A ) );

        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "response()");
        QVERIFY( request.respond() );
        IPC_WAIT();

        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.isComplete() );
    }

    // Attempt to respond with error message
    {
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "heartbeat()");
        QDSActionRequest request( QDSServiceInfo( QDS_SERVICES_NAME_A,
                                                  QDS_SERVICES_SERVICE_A ),
                                  QDS_ACTION_REQUEST_CHANNEL );
        IPC_WAIT();

        QVERIFY( request.isValid() );
        QVERIFY( !request.isComplete() );
        QVERIFY( request.requestData() == QDSData() );
        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.serviceInfo() ==
                 QDSServiceInfo( QDS_SERVICES_NAME_A,
                                 QDS_SERVICES_SERVICE_A ) );

        QString errorMessage = "testing";
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "error(QString)");
        QVERIFY( request.respond( errorMessage ) );
        IPC_WAIT(errorMessage);
    }
}

/*? Test requesting and responding on a service with request data and no response data. */
void tst_QDSActionRequest::requestDataTestCase()
{
    // Attempt to create with no data when we should have data.
    {
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "error(QString)");
        QDSActionRequest request( QDSServiceInfo( QDS_SERVICES_NAME_C,
                                                  QDS_SERVICES_SERVICE_C ),
                                  QDS_ACTION_REQUEST_CHANNEL );
        IPC_WAIT(tr("request didn't contain data"));

        QVERIFY( !request.isValid() );
        QVERIFY( request.isComplete() );
        QVERIFY( request.requestData() == QDSData() );
        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.serviceInfo() ==
                 QDSServiceInfo( QDS_SERVICES_NAME_C,
                                 QDS_SERVICES_SERVICE_C ) );

    }

    // Attempt to respond with data when we shouldn't.
    {
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "heartbeat()");
        QDSActionRequest request( QDSServiceInfo( QDS_SERVICES_NAME_C,
                                                  QDS_SERVICES_SERVICE_C ),
                                  QDSData( QByteArray( "Some data" ),
                                           QMimeType( QDS_SERVICES_REQDATATYPE_C ) ),
                                  QDS_ACTION_REQUEST_CHANNEL,
                                  QByteArray( "Auxiliary data" ) );
        IPC_WAIT();

        QVERIFY( request.isValid() );
        QVERIFY( !request.isComplete() );
        QVERIFY( request.requestData() != QDSData() );
        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.auxiliaryData() == QByteArray( "Auxiliary data" ) );
        QVERIFY( request.serviceInfo() ==
                 QDSServiceInfo( QDS_SERVICES_NAME_C,
                                 QDS_SERVICES_SERVICE_C ) );

        QVERIFY( !request.respond( QDSData( QByteArray( "Some data" ),
                                            QMimeType( QDS_SERVICES_REQDATATYPE_C ) ) ) );
        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( !request.isComplete() );
    }

    // Attempt to respond correctly
    {
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "heartbeat()");
        QDSActionRequest request( QDSServiceInfo( QDS_SERVICES_NAME_C,
                                                  QDS_SERVICES_SERVICE_C ),
                                  QDSData( QByteArray( "Some data" ),
                                           QMimeType( QDS_SERVICES_REQDATATYPE_C ) ),
                                  QDS_ACTION_REQUEST_CHANNEL );
        IPC_WAIT();

        QVERIFY( request.isValid() );
        QVERIFY( !request.isComplete() );
        QVERIFY( request.requestData() != QDSData() );
        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.serviceInfo() ==
                 QDSServiceInfo( QDS_SERVICES_NAME_C,
                                 QDS_SERVICES_SERVICE_C ) );

        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "response()");
        QVERIFY( request.respond() );
        IPC_WAIT();

        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.isComplete() );
    }

    // Attempt to respond with error
    {
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "heartbeat()");
        QDSActionRequest request( QDSServiceInfo( QDS_SERVICES_NAME_C,
                                                  QDS_SERVICES_SERVICE_C ),
                                  QDSData( QByteArray( "Some data" ),
                                           QMimeType( QDS_SERVICES_REQDATATYPE_C ) ),
                                  QDS_ACTION_REQUEST_CHANNEL );
        IPC_WAIT();

        QVERIFY( request.isValid() );
        QVERIFY( !request.isComplete() );
        QVERIFY( request.requestData() != QDSData() );
        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.serviceInfo() ==
                 QDSServiceInfo( QDS_SERVICES_NAME_C,
                                 QDS_SERVICES_SERVICE_C ) );

        QString errorMessage = "testing";
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "error(QString)");
        QVERIFY( request.respond( errorMessage ) );
        IPC_WAIT( errorMessage );

        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.isComplete() );
    }
}

/*? Test requesting and responding on a service with no request data and response data. */
void tst_QDSActionRequest::responseDataTestCase()
{
    // Attempt to create with data when we shouldn't.
    {
        // ignore heartbeat()
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "error(QString)");
        QDSActionRequest request( QDSServiceInfo( QDS_SERVICES_NAME_B,
                                                  QDS_SERVICES_SERVICE_B ),
                                  QDSData( QByteArray( "Some data" ),
                                           QMimeType( "text/plain" ) ),
                                  QDS_ACTION_REQUEST_CHANNEL );
        IPC_WAIT(tr("request contained unexpected data"));

        QVERIFY( !request.isValid() );
        QVERIFY( request.isComplete() );
        QVERIFY( request.requestData() != QDSData() );
        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.serviceInfo() ==
                 QDSServiceInfo( QDS_SERVICES_NAME_B,
                                 QDS_SERVICES_SERVICE_B ) );
    }

    // Attempt to respond with data correctly
    {
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "heartbeat()");
        QDSActionRequest request( QDSServiceInfo( QDS_SERVICES_NAME_B,
                                                  QDS_SERVICES_SERVICE_B ),
                                  QDS_ACTION_REQUEST_CHANNEL );
        IPC_WAIT();

        QVERIFY( request.isValid() );
        QVERIFY( !request.isComplete() );
        QVERIFY( request.requestData() == QDSData() );
        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.serviceInfo() ==
                 QDSServiceInfo( QDS_SERVICES_NAME_B,
                                 QDS_SERVICES_SERVICE_B ) );

        QDSData testData( QByteArray( "some string" ),
                          QMimeType( QDS_SERVICES_RESDATATYPE_B ) );

        IPC_EXPECT( QDS_ACTION_REQUEST_CHANNEL, "response(QDSData)" );
        QVERIFY( request.respond( testData ) );
        IPC_WAIT( qVariantFromValue(testData) );

        QVERIFY( request.responseData() == testData );
        QVERIFY( request.isComplete() );
    }

    // Attempt to respond without data (incorrect)
    {
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "heartbeat()");
        QDSActionRequest request( QDSServiceInfo( QDS_SERVICES_NAME_B,
                                                  QDS_SERVICES_SERVICE_B ),
                                  QDS_ACTION_REQUEST_CHANNEL );
        IPC_WAIT();

        QVERIFY( request.isValid() );
        QVERIFY( !request.isComplete() );
        QVERIFY( request.requestData() == QDSData() );
        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.serviceInfo() ==
                 QDSServiceInfo( QDS_SERVICES_NAME_B,
                                 QDS_SERVICES_SERVICE_B ) );

        QVERIFY( !request.respond() );

        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( !request.isComplete() );
    }

    // Attempt to reply with error message
    {
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "heartbeat()");
        QDSActionRequest request( QDSServiceInfo( QDS_SERVICES_NAME_B,
                                                  QDS_SERVICES_SERVICE_B ),
                                  QDS_ACTION_REQUEST_CHANNEL );
        IPC_WAIT();

        QVERIFY( request.isValid() );
        QVERIFY( !request.isComplete() );
        QVERIFY( request.requestData() == QDSData() );
        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.serviceInfo() ==
                 QDSServiceInfo( QDS_SERVICES_NAME_B,
                                 QDS_SERVICES_SERVICE_B ) );

        QString errorMessage = "testing";
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "error(QString)");
        QVERIFY( request.respond( errorMessage ) );
        IPC_WAIT( errorMessage );

        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.isComplete() );
    }
}

/*? Test requesting and responding on a service with request data and response data. */
void tst_QDSActionRequest::requestAndResponseDataTestCase()
{
    // Attempt to incorrectly create with no data
    {
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "error(QString)");
        QDSActionRequest request( QDSServiceInfo( QDS_SERVICES_NAME_D,
                                                  QDS_SERVICES_SERVICE_D ),
                                  QDS_ACTION_REQUEST_CHANNEL );
        IPC_WAIT(tr("request didn't contain data"));

        QVERIFY( !request.isValid() );
        QVERIFY( request.isComplete() );
        QVERIFY( request.requestData() == QDSData() );
        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.serviceInfo() ==
                 QDSServiceInfo( QDS_SERVICES_NAME_D,
                                 QDS_SERVICES_SERVICE_D ) );
    }

    // Attempt to respond with data successfully
    {
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "heartbeat()");
        QDSActionRequest request( QDSServiceInfo( QDS_SERVICES_NAME_D,
                                                  QDS_SERVICES_SERVICE_D ),
                                  QDSData( QByteArray( "Some data" ),
                                           QMimeType( QDS_SERVICES_REQDATATYPE_D ) ),
                                  QDS_ACTION_REQUEST_CHANNEL,
                                  QByteArray( "Auxiliary data" ) );
        IPC_WAIT();

        QVERIFY( request.isValid() );
        QVERIFY( !request.isComplete() );
        QVERIFY( request.requestData() != QDSData() );
        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.auxiliaryData() == QByteArray( "Auxiliary data" ) );
        QVERIFY( request.serviceInfo() ==
                 QDSServiceInfo( QDS_SERVICES_NAME_D,
                                 QDS_SERVICES_SERVICE_D ) );

        QDSData testData( QByteArray( "another string" ),
                          QMimeType( QDS_SERVICES_RESDATATYPE_D ) );
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "response(QDSData)");
        QVERIFY( request.respond( testData ) );
        IPC_WAIT( qVariantFromValue(testData) );

        QVERIFY( request.responseData() == testData );
        QVERIFY( request.isComplete() );
    }

    // Attempt to incorrectly respond without data
    {
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "heartbeat()");
        QDSActionRequest request( QDSServiceInfo( QDS_SERVICES_NAME_D,
                                                  QDS_SERVICES_SERVICE_D ),
                                  QDSData( QByteArray( "Some data" ),
                                           QMimeType( QDS_SERVICES_REQDATATYPE_D ) ),
                                  QDS_ACTION_REQUEST_CHANNEL );
        IPC_WAIT();

        QVERIFY( request.isValid() );
        QVERIFY( !request.isComplete() );
        QVERIFY( request.requestData() != QDSData() );
        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.serviceInfo() ==
                 QDSServiceInfo( QDS_SERVICES_NAME_D,
                                 QDS_SERVICES_SERVICE_D ) );

        QVERIFY( !request.respond() );

        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( !request.isComplete() );
    }

    // Attempt to respond with error message
    {
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "heartbeat()");
        QDSActionRequest request( QDSServiceInfo( QDS_SERVICES_NAME_D,
                                                  QDS_SERVICES_SERVICE_D ),
                                  QDSData( QByteArray( "Some data" ),
                                           QMimeType( QDS_SERVICES_REQDATATYPE_D ) ),
                                  QDS_ACTION_REQUEST_CHANNEL );
        IPC_WAIT();

        QVERIFY( request.isValid() );
        QVERIFY( !request.isComplete() );
        QVERIFY( request.requestData() != QDSData() );
        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.serviceInfo() ==
                 QDSServiceInfo( QDS_SERVICES_NAME_D,
                                 QDS_SERVICES_SERVICE_D ) );

        QString errorMessage = "testing";
        IPC_EXPECT(QDS_ACTION_REQUEST_CHANNEL, "error(QString)");        
        QVERIFY( request.respond( errorMessage ) );
        IPC_WAIT( errorMessage );

        QVERIFY( request.responseData() == QDSData() );
        QVERIFY( request.isComplete() );
    }
}
