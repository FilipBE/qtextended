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

#include <QDSServiceInfo>
#include <QMimeType>
#include <QDir>
#include <QtopiaApplication>

#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>

#include "../qdsunittest.h"

// --------------------------- Constants --------------------------------------

static const QString    QDS_SERVICE_INFO_DIR           = "etc/qds/";
static const QString    QDS_SERVICE_INFO_SERVICE_A     = "QDSProviderStub";
static const QString    QDS_SERVICE_INFO_TRSERVICE_A   = "trQDSProviderStub";
static const QString    QDS_SERVICE_INFO_NAME_A        = "ActionA";
static const QString    QDS_SERVICE_INFO_REQDATATYPE_A = "text/plain";
static const QString    QDS_SERVICE_INFO_RESDATATYPE_A = "image/jpeg";
static const QStringList QDS_SERVICE_INFO_ATTRIBUTES_A = QStringList( "AttributeA" );
static const QString    QDS_SERVICE_INFO_DESCRIPTION_A = "Test action";
static const QString    QDS_SERVICE_INFO_ICON_A        = "testIcon";

static const QString    QDS_SERVICE_INFO_SERVICE_B     = "QDSProviderStub";
static const QString    QDS_SERVICE_INFO_TRSERVICE_B   = "trQDSProviderStub";
static const QString    QDS_SERVICE_INFO_NAME_B        = "ActionB";
static const QString    QDS_SERVICE_INFO_REQDATATYPE_B = "text/plain";
static const QString    QDS_SERVICE_INFO_RESDATATYPE_B = "image/jpeg";
static const QStringList QDS_SERVICE_INFO_ATTRIBUTES_B = QStringList( "AttributeB" );
static const QString    QDS_SERVICE_INFO_DESCRIPTION_B = "Test action";
static const QString    QDS_SERVICE_INFO_ICON_B        = "testIcon";

static const QString    QDS_SERVICE_INFO_SERVICE_C     = "QDSProviderStub";
static const QString    QDS_SERVICE_INFO_TRSERVICE_C   = "trQDSProviderStub";
static const QString    QDS_SERVICE_INFO_NAME_C        = "ActionC";
static const QString    QDS_SERVICE_INFO_REQDATATYPE_C = "Text/Plain";
static const QString    QDS_SERVICE_INFO_RESDATATYPE_C = "Image/JPEG";
static const QStringList QDS_SERVICE_INFO_ATTRIBUTES_C = QStringList( "AttributeC" );
static const QString    QDS_SERVICE_INFO_DESCRIPTION_C = "Test action";
static const QString    QDS_SERVICE_INFO_ICON_C        = "testIcon";

static const QString    QDS_SERVICE_INFO_SERVICE_E     = "QDSProviderStub";
static const QString    QDS_SERVICE_INFO_TRSERVICE_E   = "trQDSProviderStub";
static const QString    QDS_SERVICE_INFO_NAME_E        = "ActionE";
static const QString    QDS_SERVICE_INFO_REQDATATYPE_E = "text/plain;image/jpeg";
static const QString    QDS_SERVICE_INFO_RESDATATYPE_E = "data/octet;data/x-application";
static const QStringList QDS_SERVICE_INFO_ATTRIBUTES_E = QStringList( "AttributeE" );
static const QString    QDS_SERVICE_INFO_DESCRIPTION_E = "Test action";
static const QString    QDS_SERVICE_INFO_ICON_E        = "testIcon";



//TESTED_CLASS=QDSServiceInfo
//TESTED_FILES=src/libraries/qtopia/qdsserviceinfo.cpp

/*
    The tst_QDSServiceInfo class is a unit test for the QDSServiceInfo class.
*/
class tst_QDSServiceInfo : public QObject
{
    Q_OBJECT
protected slots:
    void initTestCase();
    void cleanupTestCase();

private slots:
    // QDSServiceInfo class test cases
    void defaultConstructorTestCase();
    void constructorTestCase();
    void copyConstructorTestCase();
    void mimeTypeCaseSensitivityTestCase();
    void multipleDataTypesTestCase();

private:
    void createTestServiceFile( const QString& provider,
                                const QString& trService,
                                const QString& name,
                                const QString& argumentType,
                                const QString& returnType,
                                const QStringList& attributes,
                                const QString& description,
                                const QString& icon );

    void removeTestServiceFile( const QString& service );

    void createProviderStubServiceFiles();
    void removeProviderStubServiceFiles();
};

QTEST_APP_MAIN( tst_QDSServiceInfo, QtopiaApplication )
#include "tst_qdsserviceinfo.moc"

/*?
    Initialisation before all test cases.
    Creates all needed test service files.
*/
void tst_QDSServiceInfo::initTestCase()
{
    // Create service files
    createTestServiceFile( QDS_SERVICE_INFO_SERVICE_A,
                           QDS_SERVICE_INFO_TRSERVICE_A,
                           QDS_SERVICE_INFO_NAME_A,
                           QDS_SERVICE_INFO_REQDATATYPE_A,
                           QDS_SERVICE_INFO_RESDATATYPE_A,
                           QDS_SERVICE_INFO_ATTRIBUTES_A,
                           QDS_SERVICE_INFO_DESCRIPTION_A,
                           QDS_SERVICE_INFO_ICON_A );

    createTestServiceFile( QDS_SERVICE_INFO_SERVICE_C,
                           QDS_SERVICE_INFO_TRSERVICE_C,
                           QDS_SERVICE_INFO_NAME_C,
                           QDS_SERVICE_INFO_REQDATATYPE_C,
                           QDS_SERVICE_INFO_RESDATATYPE_C,
                           QDS_SERVICE_INFO_ATTRIBUTES_C,
                           QDS_SERVICE_INFO_DESCRIPTION_C,
                           QDS_SERVICE_INFO_ICON_C );

    createTestServiceFile( QDS_SERVICE_INFO_SERVICE_E,
                           QDS_SERVICE_INFO_TRSERVICE_E,
                           QDS_SERVICE_INFO_NAME_E,
                           QDS_SERVICE_INFO_REQDATATYPE_E,
                           QDS_SERVICE_INFO_RESDATATYPE_E,
                           QDS_SERVICE_INFO_ATTRIBUTES_E,
                           QDS_SERVICE_INFO_DESCRIPTION_E,
                           QDS_SERVICE_INFO_ICON_E );

    createProviderStubServiceFiles();
}

/*?
    Cleanup after all test functions.
    Removes test service files.
*/
void tst_QDSServiceInfo::cleanupTestCase()
{
    // Remove service files
    removeTestServiceFile( QDS_SERVICE_INFO_SERVICE_A );
    removeTestServiceFile( QDS_SERVICE_INFO_SERVICE_C );
    removeTestServiceFile( QDS_SERVICE_INFO_SERVICE_E );

    removeProviderStubServiceFiles();
}

/*?
    Create a service file for a specified service.
*/
void tst_QDSServiceInfo::createTestServiceFile(
    const QString& service,
    const QString& trService,
    const QString& name,
    const QString& argumentType,
    const QString& returnType,
    const QStringList& attributes,
    const QString& description,
    const QString& icon )
{
    // Open the service file
    QString filename = Qtopia::qtopiaDir() +
                       QDS_SERVICE_INFO_DIR +
                       service;

    QSettings serviceFile( filename, QSettings::IniFormat );

    serviceFile.beginGroup( "QDSInformation" );
    serviceFile.setValue( "Name", trService );
    serviceFile.endGroup();

    // Write the service file
    serviceFile.beginGroup( name );
    serviceFile.setValue( "RequestDataType", argumentType );
    serviceFile.setValue( "ResponseDataType", returnType );
    serviceFile.setValue( "Attributes", attributes );
    serviceFile.setValue( "Description", description );
    serviceFile.setValue( "Icon", icon );
    serviceFile.endGroup();
}

/*?
    Remove a service file for a specific service.
*/
void tst_QDSServiceInfo::removeTestServiceFile( const QString& service )
{
    // Remove the service file
    QString filename = Qtopia::qtopiaDir() +
                       QDS_SERVICE_INFO_DIR +
                       service;

    QFile serviceFile( filename );
    serviceFile.remove();
}

/*? Create the QDSProviderStub service file. */
void tst_QDSServiceInfo::createProviderStubServiceFiles()
{
    {
        QString filename = Qtopia::qtopiaDir() +
                           "services/QDSProviderStub.service";
        QFile serviceFile( filename );
        if ( !serviceFile.open( QIODevice::WriteOnly ) ) {
            qWarning( QString("Couldn't create the QDSProviderStub service file %1").arg(filename).toLatin1() );
            return;
        }

        QTextStream ts( &serviceFile );
        ts << "[Translation]\n";
        ts << "File=QtopiaServices\n";
        ts << "Context=QDSProviderStub\n";
        ts << "[Service]\n";
        ts << "Actions = \"" << QDS_SERVICE_INFO_NAME_A << "(QDSActionRequest)";
        QStringList actions;
        actions << QDS_SERVICE_INFO_NAME_C << QDS_SERVICE_INFO_NAME_E;
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
            QFAIL( QString("Couldn't create QDSProviderStub service directory %1/QDSProviderStub").arg(Qtopia::qtopiaDir() + "services").toLatin1() );
        }

        QString filename = Qtopia::qtopiaDir() +
                           "services/QDSProviderStub/qdsproviderstub";
        QFile serviceFile( filename );
        if ( !serviceFile.open( QIODevice::WriteOnly ) ) {
            qWarning( QString("Couldn't create the QDSProviderStub service file %1").arg(filename).toLatin1() );
            return;
        }

        QTextStream ts( &serviceFile );
        ts << "[Standard]\nVersion=100\n";
        serviceFile.close();
    }
}

/*? Remove the QDSProviderStub service file. */
void tst_QDSServiceInfo::removeProviderStubServiceFiles()
{
    QFile::remove( Qtopia::qtopiaDir() + "services/QDSProviderStub.service" );
    QFile::remove( Qtopia::qtopiaDir() + "services/QDSProviderStub/qdsproviderstub" );

    // Remove the QDSProviderStub service directory
    QDir servicesDir( Qtopia::qtopiaDir() + "services" );
    if ( !servicesDir.rmdir( "QDSProviderStub" ) ) {
        qWarning( "Couldn't remove QDSProviderStub service directory" );
    }
}

/*?
    Test that the default constructor constructs an empty object.
*/
void tst_QDSServiceInfo::defaultConstructorTestCase()
{
    QDSServiceInfo defaultInfo;

    QVERIFY( !defaultInfo.isValid() );
    QVERIFY( defaultInfo.serviceId() == QString() );
    QVERIFY( defaultInfo.serviceName() == QString() );
    QVERIFY( defaultInfo.name() == QString() );
    QVERIFY( defaultInfo.supportsRequestDataType() );
    QVERIFY( defaultInfo.supportsResponseDataType() );
    QVERIFY( defaultInfo.attributes() == QStringList() );
    QVERIFY( defaultInfo.description() == QString() );
    QVERIFY( defaultInfo.icon() == QString() );
}

/*?
    Test that the constructor with a service name and provider correctly
    reads information from the relevant service file(s).
*/
void tst_QDSServiceInfo::constructorTestCase()
{
    // Valid service info
    QDSServiceInfo validInfo( QDS_SERVICE_INFO_NAME_A,
                              QDS_SERVICE_INFO_SERVICE_A );

    QVERIFY( validInfo.isValid() );
    QCOMPARE( validInfo.serviceId(), QDS_SERVICE_INFO_SERVICE_A );
    QCOMPARE( validInfo.name(), QDS_SERVICE_INFO_NAME_A );
    QCOMPARE( validInfo.serviceName(), QDS_SERVICE_INFO_TRSERVICE_A );
    foreach(QString t, QDS_SERVICE_INFO_REQDATATYPE_A.split(';')) {
        QVERIFY( validInfo.supportsRequestDataType( QMimeType(t) ) );
    }
    foreach(QString t, QDS_SERVICE_INFO_RESDATATYPE_A.split(';')) {
        QVERIFY( validInfo.supportsResponseDataType( QMimeType(t) ) );
    }
    QVERIFY( validInfo.attributes() == QDS_SERVICE_INFO_ATTRIBUTES_A );
    QVERIFY( validInfo.description() == QDS_SERVICE_INFO_DESCRIPTION_A );
    QVERIFY( validInfo.icon() == QDS_SERVICE_INFO_ICON_A );

    // Invalid service info
    /* Needs bug 147711 to be fixed.
    QDSServiceInfo invalidInfo( QDS_SERVICE_INFO_NAME_B,
                                QDS_SERVICE_INFO_SERVICE_B );

    QVERIFY( !invalidInfo.isValid() );
    QVERIFY( invalidInfo.serviceId() == QDS_SERVICE_INFO_SERVICE_B );
    QVERIFY( invalidInfo.name() == QDS_SERVICE_INFO_NAME_B );
    QCOMPARE( invalidInfo.serviceName(), QDS_SERVICE_INFO_TRSERVICE_B );
    QVERIFY( invalidInfo.supportsRequestDataType() );
    QVERIFY( invalidInfo.supportsResponseDataType() );
    QVERIFY( invalidInfo.attributes() == QStringList() );
    QVERIFY( invalidInfo.description() == QString() );
    QVERIFY( invalidInfo.icon() == QString() );
    */
}

/*?
    Test that the copy constructor correctly creates a copy of a QDSServiceInfo.
*/
void tst_QDSServiceInfo::copyConstructorTestCase()
{
    // Check invalid service file
    QDSServiceInfo defaultInfo;
    QDSServiceInfo defaultInfoCopy( defaultInfo );

    QVERIFY( defaultInfo.isValid() == defaultInfoCopy.isValid() );
    QVERIFY( defaultInfo.serviceId() == defaultInfoCopy.serviceId() );
    QVERIFY( defaultInfo.serviceName() == defaultInfoCopy.serviceName() );
    QVERIFY( defaultInfo.name() == defaultInfoCopy.name() );

    foreach( QString type, defaultInfoCopy.requestDataTypes() ) {
        QVERIFY( defaultInfo.supportsRequestDataType( QMimeType(type) ) );
    }

    foreach ( QString type, defaultInfoCopy.responseDataTypes() ) {
        QVERIFY( defaultInfo.supportsResponseDataType( QMimeType(type) ) );
    }

    QVERIFY( defaultInfo.attributes() == defaultInfoCopy.attributes() );
    QVERIFY( defaultInfo.description() == defaultInfoCopy.description() );
    QVERIFY( defaultInfo.icon() == defaultInfoCopy.icon() );
    QVERIFY( defaultInfo == defaultInfoCopy );

    // Check valid service file
    QDSServiceInfo validInfo( QDS_SERVICE_INFO_NAME_A,
                              QDS_SERVICE_INFO_SERVICE_A );
    QDSServiceInfo validInfoCopy( validInfo );

    QVERIFY( validInfo.isValid() == validInfoCopy.isValid() );
    QVERIFY( validInfo.serviceId() == validInfoCopy.serviceId() );
    QVERIFY( validInfo.serviceName() == validInfoCopy.serviceName() );
    QVERIFY( validInfo.name() == validInfoCopy.name() );

    foreach( QString type, validInfoCopy.requestDataTypes() ) {
        QVERIFY( validInfo.supportsRequestDataType( QMimeType(type) ) );
    }

    foreach ( QString type, validInfoCopy.responseDataTypes() ) {
        QVERIFY( validInfo.supportsResponseDataType( QMimeType(type) ) );
    }

    QVERIFY( validInfo.attributes() == validInfoCopy.attributes() );
    QVERIFY( validInfo.description() == validInfoCopy.description() );
    QVERIFY( validInfo.icon() == validInfoCopy.icon() );
    QVERIFY( validInfo == validInfoCopy );
}

/*?
    Test supportsRequestDataType and supportsResponseDataType with mime types
    with mixed case.
*/
void tst_QDSServiceInfo::mimeTypeCaseSensitivityTestCase()
{
    // Test without lower conversions in service file path
    QDSServiceInfo validInfo( QDS_SERVICE_INFO_NAME_C,
                              QDS_SERVICE_INFO_SERVICE_C );

    QVERIFY( validInfo.isValid() );
    QVERIFY( validInfo.serviceId() == QDS_SERVICE_INFO_SERVICE_C );
    QVERIFY( validInfo.name() == QDS_SERVICE_INFO_NAME_C );
    QVERIFY( validInfo.serviceName() == QDS_SERVICE_INFO_TRSERVICE_C );
    foreach(QString t, QDS_SERVICE_INFO_REQDATATYPE_C.split(';')) {
        QVERIFY2( validInfo.supportsRequestDataType( QMimeType(t) ), QString("t = %1").arg(t).toLatin1() );
    }
    foreach(QString t, QDS_SERVICE_INFO_RESDATATYPE_C.split(';')) {
        QVERIFY2( validInfo.supportsResponseDataType( QMimeType(t) ), QString("t = %1").arg(t).toLatin1() );
    }
    QVERIFY( validInfo.attributes() == QDS_SERVICE_INFO_ATTRIBUTES_C );
    QVERIFY( validInfo.description() == QDS_SERVICE_INFO_DESCRIPTION_C );
    QVERIFY( validInfo.icon() == QDS_SERVICE_INFO_ICON_C );
}

/*?
    Test services which support multiple data types for requests and responses
    work correctly.
*/
void tst_QDSServiceInfo::multipleDataTypesTestCase()
{
    // Test without lower conversions in service file path
    QDSServiceInfo validInfo( QDS_SERVICE_INFO_NAME_E,
                              QDS_SERVICE_INFO_SERVICE_E );

    QVERIFY( validInfo.isValid() );
    QVERIFY( validInfo.serviceId() == QDS_SERVICE_INFO_SERVICE_E );
    QVERIFY( validInfo.name() == QDS_SERVICE_INFO_NAME_E );
    QVERIFY( validInfo.serviceName() == QDS_SERVICE_INFO_TRSERVICE_E );

    QVERIFY( !validInfo.supportsRequestDataType() );
    QVERIFY( validInfo.supportsRequestDataType( QMimeType( "text/plain" ) ) );
    QVERIFY( validInfo.supportsRequestDataType( QMimeType( "image/jpeg" ) ) );
    QVERIFY( validInfo.supportsRequestDataType( QMimeType( "teXt/plAin" ) ) );
    QVERIFY( validInfo.supportsRequestDataType( QMimeType( "image/jpEG" ) ) );
    QVERIFY( !validInfo.supportsRequestDataType( QMimeType( "image/jpE" ) ) );
    QVERIFY( !validInfo.supportsRequestDataType( QMimeType( "mage/jpeg" ) ) );

    QVERIFY( validInfo.supportsResponseDataType( QMimeType( "data/octet" ) ) );
    QVERIFY( validInfo.supportsResponseDataType( QMimeType( "datA/ocTet" ) ) );
    QVERIFY( validInfo.supportsResponseDataType( QMimeType( "data/x-application" ) ) );
    QVERIFY( validInfo.supportsResponseDataType( QMimeType( "DATA/X-apPlication" ) ) );
    QVERIFY( !validInfo.supportsResponseDataType( QMimeType( "datA/ocTe" ) ) );
    QVERIFY( !validInfo.supportsResponseDataType( QMimeType( "ata/x-application" ) ) );

    QVERIFY( validInfo.attributes() == QDS_SERVICE_INFO_ATTRIBUTES_E );
    QVERIFY( validInfo.description() == QDS_SERVICE_INFO_DESCRIPTION_E );
    QVERIFY( validInfo.icon() == QDS_SERVICE_INFO_ICON_E );
}
