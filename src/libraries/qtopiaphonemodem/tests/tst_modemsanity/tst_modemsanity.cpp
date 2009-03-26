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

#define protected public
#define private public
#include <QNetworkRegistration>
#undef protected
#undef private

#include <QTcpServer>
#include <QSettings>
#include <QtopiaApplication>
#include <QValueSpaceObject>
#include <QServiceChecker>
#include <QSignalSpy>
#include <shared/util.h>
#include <stdlib.h>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QDebug>
#include <QModemService>
#include <QPinManager>
#include <QSettings>

class tst_ModemSanity : public QObject
{
    Q_OBJECT
public:
    tst_ModemSanity();

private slots:
    void initTestCase();

    void sendInitialPin();
    void waitForNetwork();

signals:
    void pinStatusSeen();
    void dummySignal();

public slots:
    void pinStatus( const QString& type, QPinManager::Status status,
                    const QPinOptions& options );

private:
    QModemService *service;
    QString requestedPin;

    QVariant config( const QString& section, const QString& name );
};

QTEST_APP_MAIN( tst_ModemSanity, QtopiaApplication )
#include "tst_modemsanity.moc"

tst_ModemSanity::tst_ModemSanity()
{
    service = 0;
}

void tst_ModemSanity::initTestCase()
{
    QSettings config( "Trolltech", "ModemSanity" );
    config.beginGroup( "Modem" );

    // Determine what modem device to use for testing.
    QString device = config.value
        ( "Device", QString( "sim:localhost" ) ).toString();
    QString vendor = config.value( "Vendor", QString( "" ) ).toString();
    bool multiplexing = config.value( "Multiplexing", true ).toBool();

    // Skip if there is no phone simulator running on the local host.
    if ( device == "sim:localhost" ) {
        QTcpServer server;
        if ( server.listen( QHostAddress::Any, 12345 ) ) {
            // We could listen on the phone simulator port, so there
            // mustn't be an actual phone simulator running on that port!
            server.close();
            QSKIP( "Cannot run modem sanity tests: phone simulator is not running", SkipAll );
        }
    }

    // Modify the environment so that QModemService will create what we want.
    setenv( "QTOPIA_PHONE_DEVICE", device.toUtf8().constData(), 1 );
    setenv( "QTOPIA_PHONE_VENDOR", vendor.toUtf8().constData(), 1 );
    if ( multiplexing )
        setenv( "QTOPIA_PHONE_MUX", "yes", 1 );
    else
        setenv( "QTOPIA_PHONE_MUX", "no", 1 );

    // Start up the "modem" telephony service.
    QValueSpace::initValuespaceManager();
    service = QModemService::createVendorSpecific( "modem", device, this );
    service->initialize();
    QValueSpaceObject::sync();

    // Determine if we could connect to the modem.
    QServiceChecker checker( "modem" );
    QVERIFY( checker.isValid() );
}

void tst_ModemSanity::sendInitialPin()
{
    QString pin = config( "Security", "Pin" ).toString();

    QPinManager *pinManager = new QPinManager( QString(), this );
    QVERIFY(pinManager->available());
    connect( pinManager, SIGNAL(pinStatus(QString,QPinManager::Status,QPinOptions)),
             this, SLOT(pinStatus(QString,QPinManager::Status,QPinOptions)) );

    // Query the initial PIN status.
    {
        QSignalSpy spy(this,SIGNAL(pinStatusSeen()));
        pinManager->querySimPinStatus();
        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
    }

    // Is the modem requesting a SIM PIN?
    if ( requestedPin == "SIM PIN" ) {
        QVERIFY( !pin.isEmpty() );
        QSignalSpy spy(this,SIGNAL(pinStatusSeen()));
        pinManager->enterPin( requestedPin, pin );
        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
    } else {
        QVERIFY( pin.isEmpty() );
    }

    // The type should be "READY" at this point.
    QCOMPARE( requestedPin, QString( "READY" ) );

    // Clean up.
    delete pinManager;
}

void tst_ModemSanity::pinStatus
        ( const QString& type, QPinManager::Status status, const QPinOptions& options )
{
    Q_UNUSED(options);
    if ( status == QPinManager::NeedPin ) {
        requestedPin = type;
        emit pinStatusSeen();
    } else if ( type == "READY" && status == QPinManager::Valid ) {
        requestedPin = type;
        emit pinStatusSeen();
    }
}

void tst_ModemSanity::waitForNetwork()
{
    QNetworkRegistration *netReg = new QNetworkRegistration( QString(), this );
    QVERIFY(netReg->available());

    // Loop until network registration changes to Home, Unknown, or Roaming.
    qLog(Autotest) << "waiting for network registration";
    for (;;) {
        // Are we registered yet?
        QTelephony::RegistrationState state = netReg->registrationState();
        if ( state == QTelephony::RegistrationHome ||
             state == QTelephony::RegistrationUnknown ||
             state == QTelephony::RegistrationRoaming ) {
            break;
        }

        // Wait for a registration change for up to 30 seconds.
        QSignalSpy spy(netReg,SIGNAL(registrationStateChanged()));
        netReg->connectNotify(SIGNAL(registrationStateChanged()));
        for (int i = 0; i < 30000; i += 50) {
            if (spy.count()) break;
            QTest::qWait(50);
        }
        QVERIFY2(spy.count(), qPrintable(QString("Registration state: %1").arg(netReg->registrationState())));
    }

    // If no operator name yet, then wait for one.
    if ( netReg->currentOperatorName().isEmpty() ) {
        qLog(Autotest) << "waiting for operator name";
        QSignalSpy spy(netReg,SIGNAL(currentOperatorChanged()));
        netReg->connectNotify(SIGNAL(currentOperatorChanged()));
        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
    }

    // Check the operator name against the value in the config file.
    QString expectedOperatorName = config( "Operator", "Name" ).toString();
    if ( !expectedOperatorName.isEmpty() )
        QCOMPARE( netReg->currentOperatorName(), expectedOperatorName );
    qLog(Autotest) << "network operator is" << netReg->currentOperatorName();

    // Clean up.
    delete netReg;
}

QVariant tst_ModemSanity::config( const QString& section, const QString& name )
{
    QSettings config( "Trolltech", "ModemSanity" );
    config.beginGroup( section );
    return config.value( name );
}
