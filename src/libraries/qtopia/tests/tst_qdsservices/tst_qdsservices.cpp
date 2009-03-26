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

#include <QDSServices>
#include <QDSServiceInfo>

#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QDebug>
#include <QtopiaApplication>

#include "../qdsunittest.h"

// --------------------------- Constants --------------------------------------

static const QString    SERVICES_DIR               = Qtopia::qtopiaDir() + "etc";
static const QString    QDS_SERVICES_DIR           = "qds";

static const QString    QDS_SERVICES_SERVICE_A     = "Service1";
static const QString    QDS_SERVICES_NAME_A        = "ActionA";
static const QString    QDS_SERVICES_REQDATATYPE_A = "text/plain";
static const QString    QDS_SERVICES_RESDATATYPE_A  = "image/jpeg";
static const QStringList QDS_SERVICES_ATTRIBUTES_A = QStringList( "get" );
static const QString    QDS_SERVICES_DESCRIPTION_A = "Test action";
static const QString    QDS_SERVICES_ICON_A        = "testIcon";

static const QString    QDS_SERVICES_SERVICE_B     = "Service1";
static const QString    QDS_SERVICES_NAME_B        = "ActionB";
static const QString    QDS_SERVICES_REQDATATYPE_B = "text/html";
static const QString    QDS_SERVICES_RESDATATYPE_B = "image/bmp";
static const QStringList QDS_SERVICES_ATTRIBUTES_B = QStringList( "GET" );
static const QString    QDS_SERVICES_DESCRIPTION_B = "Test action";
static const QString    QDS_SERVICES_ICON_B        = "testIcon";

static const QString    QDS_SERVICES_SERVICE_C     = "Service1";
static const QString    QDS_SERVICES_NAME_C        = "ActionC";
static const QString    QDS_SERVICES_REQDATATYPE_C = "image/jpeg";
static const QString    QDS_SERVICES_RESDATATYPE_C = "text/plain";
static const QStringList QDS_SERVICES_ATTRIBUTES_C = QStringList( "Get" );
static const QString    QDS_SERVICES_DESCRIPTION_C = "Test action";
static const QString    QDS_SERVICES_ICON_C        = "testIcon";

static const QString    QDS_SERVICES_SERVICE_D     = "Service1";
static const QString    QDS_SERVICES_NAME_D        = "ActionD";
static const QString    QDS_SERVICES_REQDATATYPE_D = "image/bmp";
static const QString    QDS_SERVICES_RESDATATYPE_D = "text/html";
static const QStringList QDS_SERVICES_ATTRIBUTES_D = QStringList( "geT" );
static const QString    QDS_SERVICES_DESCRIPTION_D = "Test action";
static const QString    QDS_SERVICES_ICON_D        = "testIcon";

static const QString    QDS_SERVICES_SERVICE_E     = "Service2";
static const QString    QDS_SERVICES_NAME_E        = "ActionE";
static const QString    QDS_SERVICES_REQDATATYPE_E = "text/plain";
static const QString    QDS_SERVICES_RESDATATYPE_E = "text/html";
static const QStringList QDS_SERVICES_ATTRIBUTES_E = QStringList( "seT" );
static const QString    QDS_SERVICES_DESCRIPTION_E = "Test action";
static const QString    QDS_SERVICES_ICON_E        = "testIcon";

static const QString    QDS_SERVICES_SERVICE_F     = "Service2";
static const QString    QDS_SERVICES_NAME_F        = "ActionF";
static const QString    QDS_SERVICES_REQDATATYPE_F = "text/html";
static const QString    QDS_SERVICES_RESDATATYPE_F = "text/plain";
static const QStringList QDS_SERVICES_ATTRIBUTES_F = QStringList( "SET" );
static const QString    QDS_SERVICES_DESCRIPTION_F = "Test action";
static const QString    QDS_SERVICES_ICON_F        = "testIcon";

static const QString    QDS_SERVICES_SERVICE_G     = "Service2";
static const QString    QDS_SERVICES_NAME_G        = "ActionG";
static const QString    QDS_SERVICES_REQDATATYPE_G = "image/bmp";
static const QString    QDS_SERVICES_RESDATATYPE_G = "image/jpeg";
static const QStringList QDS_SERVICES_ATTRIBUTES_G = QStringList( "Set" );
static const QString    QDS_SERVICES_DESCRIPTION_G = "Test action";
static const QString    QDS_SERVICES_ICON_G        = "testIcon";

static const QString    QDS_SERVICES_SERVICE_H     = "Service2";
static const QString    QDS_SERVICES_NAME_H        = "ActionH";
static const QString    QDS_SERVICES_REQDATATYPE_H = "image/jpeg";
static const QString    QDS_SERVICES_RESDATATYPE_H = "image/bmp";
static const QStringList QDS_SERVICES_ATTRIBUTES_H = QStringList( "set" );
static const QString    QDS_SERVICES_DESCRIPTION_H = "Test action";
static const QString    QDS_SERVICES_ICON_H        = "testIcon";

static const QString    QDS_SERVICES_SERVICE_I     = "Service3";
static const QString    QDS_SERVICES_NAME_I        = "ActionD";
static const QString    QDS_SERVICES_REQDATATYPE_I = "";
static const QString    QDS_SERVICES_RESDATATYPE_I = "";
static const QStringList QDS_SERVICES_ATTRIBUTES_I = QStringList( "inform" );
static const QString    QDS_SERVICES_DESCRIPTION_I = "Test action";
static const QString    QDS_SERVICES_ICON_I        = "testIcon";

static const QString    QDS_SERVICES_SERVICE_J     = "Service3";
static const QString    QDS_SERVICES_NAME_J        = "ActionJ";
static const QString    QDS_SERVICES_REQDATATYPE_J = "x-test*";
static const QString    QDS_SERVICES_RESDATATYPE_J = "x-test/plain;x-test/html";
static const QStringList QDS_SERVICES_ATTRIBUTES_J = QStringList( "inform" );
static const QString    QDS_SERVICES_DESCRIPTION_J = "Test action";
static const QString    QDS_SERVICES_ICON_J        = "testIcon";

namespace QTest
{
    template<>
    inline bool qCompare(QDSServiceInfo const &a, QDSServiceInfo const &b,
            const char *actual, const char *expected, const char *file, int line)
    {
#define COMPARE_METHODS(method) \
        qCompare(a.method(), b.method(), \
            QString("%1.%2()").arg(actual).arg(#method).toLatin1(), \
            QString("%1.%2()").arg(expected).arg(#method).toLatin1(), file, line)

        return COMPARE_METHODS(attributes)
            && COMPARE_METHODS(description)
            && COMPARE_METHODS(icon)
            && COMPARE_METHODS(isAvailable)
            && COMPARE_METHODS(isValid)
            && COMPARE_METHODS(name)
            && COMPARE_METHODS(requestDataTypes)
            && COMPARE_METHODS(responseDataTypes)
            && COMPARE_METHODS(serviceId)
            && COMPARE_METHODS(serviceName);
    }
}

//TESTED_CLASS=QDSServices
//TESTED_FILES=src/libraries/qtopia/qdsservices.cpp

/*
    The tst_QDSServices class is a unit test for the QDSServices class.
*/
class tst_QDSServices : public QObject
{
    Q_OBJECT
protected slots:
    void initTestCase();
    void cleanupTestCase();

private slots:
    // QDSServices class test cases
    void requestDataTypeFilterTestCase();
    void responseDataTypeFilterTestCase();
    void attributesFilterTestCase();
    void serviceFilterTestCase();
    void combinationFiltersTestCase();
    void findFirstTestCase();
    void multipleWildcardTestCase();

private:
    void createTestServiceFile( const QString& service,
                                const QString& name,
                                const QString& requestDataType,
                                const QString& responseDataType,
                                const QStringList& attributes,
                                const QString& description,
                                const QString& icon );

    void removeTestServiceFile( const QString& service );

    void createStubServiceFile(const QString& service, const QStringList& actions);
    void removeStubServiceFile(const QString& service);
};

QTEST_APP_MAIN( tst_QDSServices, QtopiaApplication )
#include "tst_qdsservices.moc"

/*?
    Initialisation before all test functions.  Creates service files.
*/
void tst_QDSServices::initTestCase()
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

    createTestServiceFile( QDS_SERVICES_SERVICE_D,
                           QDS_SERVICES_NAME_D,
                           QDS_SERVICES_REQDATATYPE_D,
                           QDS_SERVICES_RESDATATYPE_D,
                           QDS_SERVICES_ATTRIBUTES_D,
                           QDS_SERVICES_DESCRIPTION_D,
                           QDS_SERVICES_ICON_D );

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

    createTestServiceFile( QDS_SERVICES_SERVICE_I,
                           QDS_SERVICES_NAME_I,
                           QDS_SERVICES_REQDATATYPE_I,
                           QDS_SERVICES_RESDATATYPE_I,
                           QDS_SERVICES_ATTRIBUTES_I,
                           QDS_SERVICES_DESCRIPTION_I,
                           QDS_SERVICES_ICON_I );

    createTestServiceFile( QDS_SERVICES_SERVICE_J,
                           QDS_SERVICES_NAME_J,
                           QDS_SERVICES_REQDATATYPE_J,
                           QDS_SERVICES_RESDATATYPE_J,
                           QDS_SERVICES_ATTRIBUTES_J,
                           QDS_SERVICES_DESCRIPTION_J,
                           QDS_SERVICES_ICON_J );

    createStubServiceFile( QDS_SERVICES_SERVICE_A, QStringList()
        << QDS_SERVICES_NAME_A << QDS_SERVICES_NAME_B << QDS_SERVICES_NAME_C
        << QDS_SERVICES_NAME_D );

    createStubServiceFile( QDS_SERVICES_SERVICE_E, QStringList()
        << QDS_SERVICES_NAME_E << QDS_SERVICES_NAME_F << QDS_SERVICES_NAME_G
        << QDS_SERVICES_NAME_H );

    createStubServiceFile( QDS_SERVICES_SERVICE_I, QStringList()
        << QDS_SERVICES_NAME_I << QDS_SERVICES_NAME_J );
}

/*?
    Cleanup after all test functions.  Removes service files.
*/
void tst_QDSServices::cleanupTestCase()
{
    // Remove service files
    removeTestServiceFile( QDS_SERVICES_SERVICE_A );
    removeTestServiceFile( QDS_SERVICES_SERVICE_B );
    removeTestServiceFile( QDS_SERVICES_SERVICE_C );
    removeTestServiceFile( QDS_SERVICES_SERVICE_D );
    removeTestServiceFile( QDS_SERVICES_SERVICE_E );
    removeTestServiceFile( QDS_SERVICES_SERVICE_F );
    removeTestServiceFile( QDS_SERVICES_SERVICE_G );
    removeTestServiceFile( QDS_SERVICES_SERVICE_H );
    removeTestServiceFile( QDS_SERVICES_SERVICE_I );
    removeTestServiceFile( QDS_SERVICES_SERVICE_J );

    removeStubServiceFile( QDS_SERVICES_SERVICE_A );
    removeStubServiceFile( QDS_SERVICES_SERVICE_E );
    removeStubServiceFile( QDS_SERVICES_SERVICE_I );
}

/*?
    Create a service file for a specific service.
*/
void tst_QDSServices::createTestServiceFile(
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
    serviceFile.setValue( "Description[]", description );
    serviceFile.setValue( "Icon", icon );
    serviceFile.endGroup();
}

/*?
    Remove a service file for a specific service.
*/
void tst_QDSServices::removeTestServiceFile( const QString& service )
{
    // Remove the service file
    QString filename = SERVICES_DIR + "/" + QDS_SERVICES_DIR + "/" + service;
    QFile serviceFile( filename );
    serviceFile.remove();
}

/*? Create the stub service file. */
void tst_QDSServices::createStubServiceFile(const QString& service, const QStringList& actions)
{
    {
        QString filename = Qtopia::qtopiaDir() +
                           QString("services/%1.service").arg(service);
        QFile serviceFile( filename );
        if ( !serviceFile.open( QIODevice::WriteOnly ) ) {
            qWarning( QString("Couldn't create the stub service file %1").arg(filename).toLatin1() );
            return;
        }

        QTextStream ts( &serviceFile );
        ts << "[Translation]\n";
        ts << "File=QtopiaServices\n";
        ts << QString("Context=%1\n").arg(service);
        ts << "[Service]\n";
        ts << "Actions = \"" << actions[0] << "(QDSActionRequest)";
        foreach (QString a, actions.mid(1)) {
            ts << ";" << a << "(QDSActionRequest)";
        }
        ts << "\"\n";
        ts << "Icon = \n";
        ts << QString("Name=%1\n").arg(service);

        serviceFile.close();
    }

    {
        // Create the QDSProviderStub service directory
        QDir servicesDir( Qtopia::qtopiaDir() + "services" );
        if ( !servicesDir.exists( service ) && !servicesDir.mkdir( service ) ) {
            QFAIL( QString("Couldn't create QDSProviderStub service directory %1/%1").arg(Qtopia::qtopiaDir() + "services").arg(service).toLatin1() );
        }

        QString filename = Qtopia::qtopiaDir() +
                           QString("services/%1/%2").arg(service).arg(service.toLower());
        QFile serviceFile( filename );
        if ( !serviceFile.open( QIODevice::WriteOnly ) ) {
            qWarning( QString("Couldn't create the stub service file %1").arg(filename).toLatin1() );
            return;
        }

        QTextStream ts( &serviceFile );
        ts << "[Standard]\nVersion=100\n";
        serviceFile.close();
    }
}

/*? Remove the QDSProviderStub service file. */
void tst_QDSServices::removeStubServiceFile(const QString& service)
{
    QFile::remove( Qtopia::qtopiaDir() + QString("services/%1.service").arg(service) );
    QFile::remove( Qtopia::qtopiaDir() + QString("services/%1/%2").arg(service).arg(service.toLower()) );

    // Remove the QDSProviderStub service directory
    QDir servicesDir( Qtopia::qtopiaDir() + "services" );
    if ( !servicesDir.rmdir( service ) ) {
        qWarning( "Couldn't remove stub service directory" );
    }
}

/*?
    Test filtering by request data type.
*/
void tst_QDSServices::requestDataTypeFilterTestCase()
{
    // Search for all services with an request type of text/*
    {
        QDSServices services( "text/*" );
        QCOMPARE( services.count() , 4 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_A,
                            QDS_SERVICES_SERVICE_A ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_B,
                            QDS_SERVICES_SERVICE_B ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_E,
                            QDS_SERVICES_SERVICE_E ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_F,
                            QDS_SERVICES_SERVICE_F ) ) );
    }

    // Search for all services with an request type of text/html
    {
        QDSServices services( "text/html" );
        QCOMPARE( services.count() , 2 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_B,
                            QDS_SERVICES_SERVICE_B ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_F,
                            QDS_SERVICES_SERVICE_F ) ) );
    }

    // Search for all services with an request type of text/PLAIN, should be
    // case insensitive
    {
        QDSServices services( "text/PLAIN" );
        QCOMPARE( services.count(), 2 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_A,
                            QDS_SERVICES_SERVICE_A ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_E,
                            QDS_SERVICES_SERVICE_E ) ) );
    }

    // Search for all services with an request type of te*
    {
        QDSServices services( "te*" );
        QCOMPARE( services.count(), 4 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_A,
                            QDS_SERVICES_SERVICE_A ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_B,
                            QDS_SERVICES_SERVICE_B ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_E,
                            QDS_SERVICES_SERVICE_E ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_F,
                            QDS_SERVICES_SERVICE_F ) ) );
    }

    // Search for all services with an request type of TEXT/pl*, should be
    // case insensitive
    {
        QDSServices services( "TEXT/pl*" );
        QCOMPARE( services.count() , 2 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_A,
                            QDS_SERVICES_SERVICE_A ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_E,
                            QDS_SERVICES_SERVICE_E ) ) );
    }

    // Search for all services with no request type
    {
        QDSServices services( "" );
        QCOMPARE( services.count() , 1 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_I,
                            QDS_SERVICES_SERVICE_I ) ) );
    }

    // Search for all services with any request type, should find all services
    {
        QDSServices services( "*" );
        QCOMPARE( services.count() , 10 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_A,
                            QDS_SERVICES_SERVICE_A ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_B,
                            QDS_SERVICES_SERVICE_B ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_C,
                            QDS_SERVICES_SERVICE_C ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_D,
                            QDS_SERVICES_SERVICE_D ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_E,
                            QDS_SERVICES_SERVICE_E ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_F,
                            QDS_SERVICES_SERVICE_F ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_G,
                            QDS_SERVICES_SERVICE_G ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_H,
                            QDS_SERVICES_SERVICE_H ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_I,
                            QDS_SERVICES_SERVICE_I ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_J,
                            QDS_SERVICES_SERVICE_J ) ) );
    }
}

/*?
    Test filtering by response data type.
*/
void tst_QDSServices::responseDataTypeFilterTestCase()
{
    // Search for all services with an response type of text/*
    {
        QDSServices services( "*", "text/*" );
        QCOMPARE( services.count() , 4 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_C,
                            QDS_SERVICES_SERVICE_C ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_D,
                            QDS_SERVICES_SERVICE_D ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_E,
                            QDS_SERVICES_SERVICE_E ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_F,
                            QDS_SERVICES_SERVICE_F ) ) );
    }

    // Search for all services with a response type of text/html
    {
        QDSServices services( "*", "text/html" );
        QCOMPARE( services.count() , 2 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_D,
                            QDS_SERVICES_SERVICE_D ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_E,
                            QDS_SERVICES_SERVICE_E ) ) );
    }

    // Search for all services with a response type of text/PLAIN, should be
    // case insensitive
    {
        QDSServices services( "*", "text/PLAIN" );
        QCOMPARE( services.count() , 2 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_C,
                            QDS_SERVICES_SERVICE_C ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_F,
                            QDS_SERVICES_SERVICE_F ) ) );
    }

    // Search for all services with a response type of te*
    {
        QDSServices services( "*", "te*" );
        QCOMPARE( services.count() , 4 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_C,
                            QDS_SERVICES_SERVICE_C ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_D,
                            QDS_SERVICES_SERVICE_D ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_E,
                            QDS_SERVICES_SERVICE_E ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_F,
                            QDS_SERVICES_SERVICE_F ) ) );
    }

    // Search for all services with a response type of TEXT/pl*, should be
    // case insensitive
    {
        QDSServices services( "*", "TEXT/pl*" );
        QCOMPARE( services.count() , 2 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_C,
                            QDS_SERVICES_SERVICE_C ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_F,
                            QDS_SERVICES_SERVICE_F ) ) );
    }

    // Search for all services with no response type
    {
        QDSServices services( "*", "" );
        QCOMPARE( services.count() , 1 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_I,
                            QDS_SERVICES_SERVICE_I ) ) );
    }

    // Search for all services with any response type, should find all services
    {
        QDSServices services( "*", "*" );
        QCOMPARE( services.count() , 10 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_A,
                            QDS_SERVICES_SERVICE_A ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_B,
                            QDS_SERVICES_SERVICE_B ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_C,
                            QDS_SERVICES_SERVICE_C ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_D,
                            QDS_SERVICES_SERVICE_D ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_E,
                            QDS_SERVICES_SERVICE_E ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_F,
                            QDS_SERVICES_SERVICE_F ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_G,
                            QDS_SERVICES_SERVICE_G ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_H,
                            QDS_SERVICES_SERVICE_H ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_I,
                            QDS_SERVICES_SERVICE_I ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_J,
                            QDS_SERVICES_SERVICE_J ) ) );
    }
}

/*?
    Test filtering by attributes.
*/
void tst_QDSServices::attributesFilterTestCase()
{
    // Search for all services which have a "get" attribute
    {
        QDSServices services( "*", "*", QStringList( "get" ) );
        QCOMPARE( services.count() , 4 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_A,
                            QDS_SERVICES_SERVICE_A ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_B,
                            QDS_SERVICES_SERVICE_B ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_C,
                            QDS_SERVICES_SERVICE_C ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_D,
                            QDS_SERVICES_SERVICE_D ) ) );
    }

    // Search for all services which have a "GEt" attribute, should be case 
    // insensitive
    {
        QDSServices services( "*", "*", QStringList( "GEt" ) );
        QCOMPARE( services.count() , 4 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_A,
                            QDS_SERVICES_SERVICE_A ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_B,
                            QDS_SERVICES_SERVICE_B ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_C,
                            QDS_SERVICES_SERVICE_C ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_D,
                            QDS_SERVICES_SERVICE_D ) ) );
    }

    // Search for all services which have a "set" attribute
    {
        QDSServices services( "*", "*", QStringList( "set" ) );
        QCOMPARE( services.count() , 4 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_E,
                            QDS_SERVICES_SERVICE_E ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_F,
                            QDS_SERVICES_SERVICE_F ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_G,
                            QDS_SERVICES_SERVICE_G ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_H,
                            QDS_SERVICES_SERVICE_H ) ) );
    }

    // Search for all services which have a "get" and a "set" attribute, 
    // should be none
    {
        QStringList list;
        list.append( "get" );
        list.append( "set" );
        QDSServices services( "*", "*", list );
        QCOMPARE( services.count() , 0 );
    }

    // Search for all services with any attribute, should find all services
    {
        QDSServices services( "*", "*", QStringList() );
        QCOMPARE( services.count() , 10 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_A,
                            QDS_SERVICES_SERVICE_A ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_B,
                            QDS_SERVICES_SERVICE_B ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_C,
                            QDS_SERVICES_SERVICE_C ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_D,
                            QDS_SERVICES_SERVICE_D ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_E,
                            QDS_SERVICES_SERVICE_E ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_F,
                            QDS_SERVICES_SERVICE_F ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_G,
                            QDS_SERVICES_SERVICE_G ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_H,
                            QDS_SERVICES_SERVICE_H ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_I,
                            QDS_SERVICES_SERVICE_I ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_J,
                            QDS_SERVICES_SERVICE_J ) ) );
    }
}

/*?
    Test filtering by service.
*/
void tst_QDSServices::serviceFilterTestCase()
{
    // Search for all services which have a "GEt" attribute, should be case 
    // insensitive
    {
        QDSServices services( "*", "*", QStringList(), "Service2" );
        QCOMPARE( services.count() , 4 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_E,
                            QDS_SERVICES_SERVICE_E ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_F,
                            QDS_SERVICES_SERVICE_F ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_G,
                            QDS_SERVICES_SERVICE_G ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_H,
                            QDS_SERVICES_SERVICE_H ) ) );
    }

    // Search for all services with a service "Service*", should find all services
    {
        QDSServices services( "*", "*", QStringList(), "Service*" );
        QCOMPARE( services.count() , 10 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_A,
                            QDS_SERVICES_SERVICE_A ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_B,
                            QDS_SERVICES_SERVICE_B ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_C,
                            QDS_SERVICES_SERVICE_C ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_D,
                            QDS_SERVICES_SERVICE_D ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_E,
                            QDS_SERVICES_SERVICE_E ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_F,
                            QDS_SERVICES_SERVICE_F ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_G,
                            QDS_SERVICES_SERVICE_G ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_H,
                            QDS_SERVICES_SERVICE_H ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_I,
                            QDS_SERVICES_SERVICE_I ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_J,
                            QDS_SERVICES_SERVICE_J ) ) );
    }

    // Search for all services with any service, should find all services
    {
        QDSServices services( "*", "*", QStringList(), "*" );
        QCOMPARE( services.count() , 10 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_A,
                            QDS_SERVICES_SERVICE_A ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_B,
                            QDS_SERVICES_SERVICE_B ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_C,
                            QDS_SERVICES_SERVICE_C ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_D,
                            QDS_SERVICES_SERVICE_D ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_E,
                            QDS_SERVICES_SERVICE_E ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_F,
                            QDS_SERVICES_SERVICE_F ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_G,
                            QDS_SERVICES_SERVICE_G ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_H,
                            QDS_SERVICES_SERVICE_H ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_I,
                            QDS_SERVICES_SERVICE_I ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_J,
                            QDS_SERVICES_SERVICE_J ) ) );
    }
}

/*?
    Test filtering by multiple criteria at once.
*/
void tst_QDSServices::combinationFiltersTestCase()
{
    // None which satisfy
    {
        QDSServices services( "image*",
                              "image/*",
                              QStringList( QString( "get" ) ),
                              "Service2" );

        QCOMPARE( services.count() , 0 );
    }

    // Single which satisy
    {
        QDSServices services( "text/html",
                              "te*",
                              QStringList( QString( "SET" ) ), 
                              "Service2" );

        QCOMPARE( services.count() , 1 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_F,
                            QDS_SERVICES_SERVICE_F ) ) );
    }

    // Multiple which satify
    {
        QDSServices services( "image*",
                              "image/*",
                              QStringList( QString( "sEt" ) ),
                              "Service2" );

        QCOMPARE( services.count() , 2 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_G,
                            QDS_SERVICES_SERVICE_G ) ) );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_H,
                            QDS_SERVICES_SERVICE_H ) ) );
    }
}

/*?
    Test the findFirst method.
*/
void tst_QDSServices::findFirstTestCase()
{
    // Find with wildcard (should fail as wild cards aren't supported), and
    // find with empty string ( should fail as should be no empty names )
    {
        QDSServices services( "image*",
                              "image/*",
                              QStringList( QString( "sEt" ) ),
                              "Service2" );

        QCOMPARE( services.count() , 2 );
        QCOMPARE( services.findFirst( "Action*" ) , QDSServiceInfo() );
        QCOMPARE( services.findFirst( "" ) , QDSServiceInfo() );
    }

    // Find with none existing in services
    {
        QDSServices services( "",
                              "image/*",
                              QStringList( QString( "sEt" ) ),
                              "Service2" );

        QCOMPARE( services.count() , 0 );
        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_A ) , QDSServiceInfo() );
        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_B ) , QDSServiceInfo() );
        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_C ) , QDSServiceInfo() );
        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_D ) , QDSServiceInfo() );
        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_E ) , QDSServiceInfo() );
        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_F ) , QDSServiceInfo() );
        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_G ) , QDSServiceInfo() );
        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_H ) , QDSServiceInfo() );
        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_I ) , QDSServiceInfo() );
    }

    // Find with one matching name
    {
        QDSServices services( "image/JPEG",
                              "IMAGE/bmp*",
                              QStringList( QString( "sEt" ) ),
                              "Service2" );

        QCOMPARE( services.count(), 1 );
        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_H ) ,
                 QDSServiceInfo( QDS_SERVICES_NAME_H,
                                 QDS_SERVICES_SERVICE_H ) );
    }

    // Find each service with find first
    {
        QDSServices services( "*", "*", QStringList(), "*" );
        QCOMPARE( services.count() , 10 );

        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_A ) ,
                 QDSServiceInfo( QDS_SERVICES_NAME_A,
                                 QDS_SERVICES_SERVICE_A ) );

        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_B ) ,
                 QDSServiceInfo( QDS_SERVICES_NAME_B,
                                 QDS_SERVICES_SERVICE_B ) );

        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_C ) ,
                 QDSServiceInfo( QDS_SERVICES_NAME_C,
                                 QDS_SERVICES_SERVICE_C ) );

        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_E ) ,
                 QDSServiceInfo( QDS_SERVICES_NAME_E,
                                 QDS_SERVICES_SERVICE_E ) );

        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_F ) ,
                 QDSServiceInfo( QDS_SERVICES_NAME_F,
                                 QDS_SERVICES_SERVICE_F ) );

        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_G ) ,
                 QDSServiceInfo( QDS_SERVICES_NAME_G,
                                 QDS_SERVICES_SERVICE_G ) );

        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_H ) ,
                 QDSServiceInfo( QDS_SERVICES_NAME_H,
                                 QDS_SERVICES_SERVICE_H ) );

        QCOMPARE( services.findFirst( QDS_SERVICES_NAME_J ) ,
                 QDSServiceInfo( QDS_SERVICES_NAME_J,
                                 QDS_SERVICES_SERVICE_J ) );
    }

    // Find with multiple with same name
    {
        QDSServices services;
        QCOMPARE( services.count() , 10 );

        QDSServiceInfo i = services.findFirst( QDS_SERVICES_NAME_D );
        QVERIFY( i == QDSServiceInfo( QDS_SERVICES_NAME_D, QDS_SERVICES_SERVICE_D )
              || i == QDSServiceInfo( QDS_SERVICES_NAME_I, QDS_SERVICES_SERVICE_I ) );

        i = services.findFirst( QDS_SERVICES_NAME_I );
        QVERIFY( i == QDSServiceInfo( QDS_SERVICES_NAME_D, QDS_SERVICES_SERVICE_D )
              || i == QDSServiceInfo( QDS_SERVICES_NAME_I, QDS_SERVICES_SERVICE_I ) );

    }
}

/*?
    Test finding services using wildcards.
*/
void tst_QDSServices::multipleWildcardTestCase()
{
    // Search for all services which have a request type wildcard
    {
        QDSServices services( "x-test*" );
        QCOMPARE( services.count() , 1 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_J,
                            QDS_SERVICES_SERVICE_J ) ) );
    }

    // Search for all services which have a non-statisfying request type wildcard
    {
        QDSServices services( "y-test*" );
        QCOMPARE( services.count() , 0 );
    }

    // Search for all services which have a request type
    {
        QDSServices services( "x-test/x-data" );
        QCOMPARE( services.count() , 1 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_J,
                            QDS_SERVICES_SERVICE_J ) ) );
    }

    // Search for all services which have a request and response type wildcard
    {
        QDSServices services( "x-test*", "x-test*" );
        QCOMPARE( services.count() , 1 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_J,
                            QDS_SERVICES_SERVICE_J ) ) );
    }

    // Search for all services which have a request and response type
    {
        QDSServices services( "x-test/data", "x-test/plain" );
        QCOMPARE( services.count() , 1 );

        QVERIFY( services.contains(
            QDSServiceInfo( QDS_SERVICES_NAME_J,
                            QDS_SERVICES_SERVICE_J ) ) );
    }

    // Search for all services which have a request and response type
    {
        QDSServices services( "x-test/data", "x-test/not-plain" );
        QCOMPARE( services.count() , 0 );
    }
}
