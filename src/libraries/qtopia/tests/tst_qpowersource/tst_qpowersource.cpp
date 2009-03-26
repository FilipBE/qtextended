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

#include <QtDebug>
#include <QtopiaApplication>
#include <QPowerSourceProvider>
#include <QHardwareManager>
#include <QValueSpaceObject>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QEventLoop>
#include <QSignalSpy>
#include <shared/util.h>

namespace QTest
{
    template<>
    char *toString( const QList<QVariant> &value ) {
        QByteArray b = "(";
        QByteArray sep;
        foreach (QVariant v, value) {
            b.append(sep);
            b.append(v.typeName());
            b.append(":'");
            b.append(v.toString().toLatin1());
            b.append("'");
            sep = ",";
        }
        b.append(")");
        return qstrdup(b.constData());
    }
}


//TESTED_CLASS=QPowerSource,QPowerSourceProvider
//TESTED_FILES=src/libraries/qtopia/qpowersource.h

/*
    The tst_QPowerSource class is a unit test for the QPowerSource class.
*/
class tst_QPowerSource : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanup();

    // QPowerSourceProvider class test cases
    void providerTests();

    // QPowerSource class test cases
    void accessoryTests();

private:
    QHardwareManager* m_manager;
    QPowerSource* m_accessory;
    QPowerSourceProvider* m_provider;
};

QTEST_APP_MAIN( tst_QPowerSource, QtopiaApplication )
#include "tst_qpowersource.moc"

/*?
    Initialisation prior to all test functions.
    Initialises the valuespace manager.
*/
void tst_QPowerSource::initTestCase()
{
    QValueSpace::initValuespaceManager();
    m_accessory = 0;
    m_provider = 0;
    qRegisterMetaType<QPowerSource::Availability>();
}

/*?
    Cleanup after every test function.
    Syncs valuespace and deletes members (if necessary).
*/
void tst_QPowerSource::cleanup()
{
    QValueSpaceObject::sync();
    if (m_accessory) {
        delete m_accessory;
        m_accessory = 0;
    }
    if (m_provider) {
        delete m_provider;
        m_provider = 0;
    }
    if (m_manager) {
        delete m_manager;
        m_manager = 0;
    }
}

/*?
    Test function for QPowerSourceProvider.
    This test:
        * Creates a QPowerSourceProvider and ensures it is correctly
          added to QHardwareManager.
        * Does simple set-get tests on QPowerSourceProvider attributes,
          ensuring signals are emitted at the appropriate times.
        * Deletes the QPowerSourceProvider object and ensures it has
          been removed from QHardwareManager.
*/
void tst_QPowerSource::providerTests()
{
    QHardwareManager* manager = new QHardwareManager( "QPowerSource", this );
    m_manager = manager;
    // Test that the provider works as expected
    {
        QPowerSourceProvider* provider = 0;
        // Test construction
        {
            QSignalSpy spy(manager,SIGNAL(providerAdded(QString)));
            provider
                = new QPowerSourceProvider( QPowerSource::Virtual, "Qt Extended" );
            m_provider = provider;
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << "Qt Extended" );
        }

        // Check that it was discovered by the accessories manager
        QVERIFY( manager->providers().contains( "Qt Extended" ) );
        QVERIFY( QHardwareManager::providers<QPowerSource>().contains("Qt Extended") );

        // Set and get various attributes, and make sure signals are
        // appropriately produced
        provider->setCharging( false );
        {
            QSignalSpy spy(provider,SIGNAL(chargingChanged(bool)));
            provider->setCharging( true );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeLast(), QVariantList() << true );
            QVERIFY( provider->charging() );
            provider->setCharging( false );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeLast(), QVariantList() << false );
            QVERIFY( !provider->charging() );
        }

        provider->setAvailability( QPowerSource::Available );
        {
            QSignalSpy spy(provider,SIGNAL(availabilityChanged(QPowerSource::Availability)));
            provider->setAvailability( QPowerSource::NotAvailable );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeLast().at(0).value<QPowerSource::Availability>(), QPowerSource::NotAvailable );
            QVERIFY( provider->availability() == QPowerSource::NotAvailable );
            provider->setAvailability( QPowerSource::Available );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeLast().at(0).value<QPowerSource::Availability>(), QPowerSource::Available );
            QVERIFY( provider->availability() == QPowerSource::Available );
            provider->setAvailability( QPowerSource::Failed );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeLast().at(0).value<QPowerSource::Availability>(), QPowerSource::Failed );
            QVERIFY( provider->availability() == QPowerSource::Failed );
        }

        provider->setCharge( 14 );
        {
            QSignalSpy spy(provider,SIGNAL(chargeChanged(int)));
            provider->setCharge( 15 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << 15 );
            QCOMPARE( provider->charge(), 15 );
            provider->setCharge( 0 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << 0 );
            QCOMPARE( provider->charge(), 0 );
            /* Causes assertion failure. */
            /*provider->setCharge( -30 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << -30 );
            QCOMPARE( provider->charge(), -30 );*/
        }

        provider->setCapacity( 14 );
        {
            QSignalSpy spy(provider,SIGNAL(capacityChanged(int)));
            provider->setCapacity( 15 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << 15 );
            QCOMPARE( provider->capacity(), 15 );
            provider->setCapacity( 0 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << 0 );
            QCOMPARE( provider->capacity(), 0 );
            provider->setCapacity( -30 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << -30 );
            QCOMPARE( provider->capacity(), -30 );
        }

        provider->setTimeRemaining( 1499 );
        {
            QSignalSpy spy(provider,SIGNAL(timeRemainingChanged(int)));
            provider->setTimeRemaining( 1500 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << 1500 );
            QCOMPARE( provider->timeRemaining(), 1500 );
            provider->setTimeRemaining( 0 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << 0 );
            QCOMPARE( provider->timeRemaining(), 0 );
            provider->setTimeRemaining( -500 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << -500 );
            QCOMPARE( provider->timeRemaining(), -500 );
        }

        {
            QSignalSpy spy(manager,SIGNAL(providerRemoved(QString)));
            delete provider;
            m_provider = 0;
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << "Qt Extended" );
        }
    }

    QValueSpaceObject::sync();
    QVERIFY( !manager->providers().contains( "Qt Extended" ) );
    QVERIFY( !QHardwareManager::providers<QPowerSource>().contains("Qt Extended") );
    delete manager;
    m_manager = 0;
}

/*?
    Test function for the interaction between QPowerSourceProvider
    and QPowerSource.
    This test:
        * Creates a QPowerSource without a Provider and ensures it is correctly
          unavailable.
        * Creates a QPowerSourceProvider and ensures it is correctly
          added to QHardwareManager.
        * Creates a QPowerSource with the same ID as the provider.
        * Does simple set-get tests on QPowerSourceProvider attributes,
          ensuring signals are emitted at the appropriate times and all changes
          to the provider also take effect on the accessory.
        * Deletes the provider and accessory and ensures it has
          been removed from QHardwareManager.
*/
void tst_QPowerSource::accessoryTests()
{
    QHardwareManager* manager = new QHardwareManager( "QPowerSource", this ) ;
    m_manager = manager;

    // Create the accessory first - since no provider has been created,
    // the accessory should not be available.
    {
        QPowerSource* accessory = new QPowerSource( "Qt Extended" );
        m_accessory = accessory;
        QVERIFY( !accessory->available() );
        delete accessory;
        m_accessory = 0;
    }

    // Create the provider and try setting and getting attributes through
    // the accessory
    {
        QPowerSourceProvider* provider = 0;
        {
            QSignalSpy spy(manager,SIGNAL(providerAdded(QString)));
            provider
                = new QPowerSourceProvider( QPowerSource::Virtual, "Qt Extended" );
            m_provider = provider;
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << "Qt Extended" );
        }

        QPowerSource* accessory = new QPowerSource( "Qt Extended" );
        m_accessory = accessory;
        QVERIFY( accessory->available() );

        // Set and get various attributes, and make sure signals are
        // appropriately produced
        provider->setCharging( false );
        {
            QSignalSpy spy(provider,SIGNAL(chargingChanged(bool)));
            provider->setCharging( true );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << true );
            QVERIFY( accessory->charging() );
            QCOMPARE( accessory->charging(), provider->charging() );
            provider->setCharging( false );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << false );
            QVERIFY( !accessory->charging() );
            QCOMPARE( accessory->charging(), provider->charging() );
        }

        provider->setAvailability( QPowerSource::Available );
        {
            QSignalSpy spy(provider,SIGNAL(availabilityChanged(QPowerSource::Availability)));
            provider->setAvailability( QPowerSource::NotAvailable );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeLast().at(0).value<QPowerSource::Availability>(), QPowerSource::NotAvailable );
            QVERIFY( accessory->availability() == QPowerSource::NotAvailable );
            QCOMPARE( accessory->availability(), provider->availability() );
            provider->setAvailability( QPowerSource::Available );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeLast().at(0).value<QPowerSource::Availability>(), QPowerSource::Available );
            QVERIFY( accessory->availability() == QPowerSource::Available );
            QCOMPARE( accessory->availability(), provider->availability() );
            provider->setAvailability( QPowerSource::Failed );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeLast().at(0).value<QPowerSource::Availability>(), QPowerSource::Failed );
            QVERIFY( accessory->availability() == QPowerSource::Failed );
            QCOMPARE( accessory->availability(), provider->availability() );
        }

        provider->setCharge( 14 );
        {
            QSignalSpy spy(provider,SIGNAL(chargeChanged(int)));
            provider->setCharge( 15 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << 15 );
            QCOMPARE( accessory->charge(), 15 );
            QCOMPARE( accessory->charge(), provider->charge() );
            provider->setCharge( 0 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << 0 );
            QCOMPARE( accessory->charge(), 0 );
            QCOMPARE( accessory->charge(), provider->charge() );

            /* Causes assertion failure
            provider->setCharge( -30 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << -30 );
            QCOMPARE( accessory->charge(), -30 );
            QCOMPARE( accessory->charge(), provider->charge() );
            */
        }

        provider->setCapacity( 14 );
        {
            QSignalSpy spy(provider,SIGNAL(capacityChanged(int)));
            provider->setCapacity( 15 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << 15 );
            QCOMPARE( accessory->capacity(), 15 );
            QCOMPARE( accessory->capacity(), provider->capacity() );
            provider->setCapacity( 0 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << 0 );
            QCOMPARE( accessory->capacity(), 0 );
            QCOMPARE( accessory->capacity(), provider->capacity() );
            provider->setCapacity( -30 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << -30 );
            QCOMPARE( accessory->capacity(), -30 );
            QCOMPARE( accessory->capacity(), provider->capacity() );
        }

        provider->setTimeRemaining( 1499 );
        {
            QSignalSpy spy(provider,SIGNAL(timeRemainingChanged(int)));
            provider->setTimeRemaining( 1500 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << 1500 );
            QCOMPARE( accessory->timeRemaining(), 1500 );
            QCOMPARE( accessory->timeRemaining(), provider->timeRemaining() );
            provider->setTimeRemaining( 0 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << 0 );
            QCOMPARE( accessory->timeRemaining(), 0 );
            QCOMPARE( accessory->timeRemaining(), provider->timeRemaining() );
            provider->setTimeRemaining( -500 );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << -500 );
            QCOMPARE( accessory->timeRemaining(), -500 );
            QCOMPARE( accessory->timeRemaining(), provider->timeRemaining() );
        }

        {
            QSignalSpy spy(manager,SIGNAL(providerRemoved(QString)));
            delete provider;
            m_provider = 0;
            delete accessory;
            m_accessory = 0;
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << "Qt Extended" );
        }
    }

    // Create another accessory and check that it is not available
    {
        QPowerSource* accessory = new QPowerSource( "Qt Extended" );
        m_accessory = accessory;
        QVERIFY( !accessory->available() );
        delete accessory;
        m_accessory = 0;
    }

    delete manager;
    m_manager = 0;
}
