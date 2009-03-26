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

#define private public
#define protected public
#include <QVibrateAccessory>
#undef private
#undef protected

#include <QHardwareManager>
#include <QValueSpaceObject>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QEventLoop>
#include <QSignalSpy>
#include <shared/util.h>


//TESTED_CLASS=QVibrateAccessory,QVibrateAccessoryProvider
//TESTED_FILES=src/libraries/qtopia/qvibrateaccessory.h

/*
    The tst_QVibrateAccessory class provides unit tests for the QVibrateAccessory class.
*/
class tst_QVibrateAccessory : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanup();

    // QVibrateAccessory class test cases
    void accessoryTests();

    // QVibrateAccessoryProvider class test cases
    void providerTests();
};

QTEST_APP_MAIN( tst_QVibrateAccessory, QtopiaApplication )
#include "tst_qvibrateaccessory.moc"

/*?
    Initialisation prior to all test functions.
    Initialises the value space manager.
*/
void tst_QVibrateAccessory::initTestCase()
{
    QValueSpace::initValuespaceManager();
}

/*?
    Cleanup after every test function.
    Syncs value space.
*/
void tst_QVibrateAccessory::cleanup()
{
    QValueSpaceObject::sync();
}

/*?
    Test function for QVibrateAccessoryProvider.
    This test:
        * Creates a QVibrateAccessoryProvider (provider) and ensures it is correctly
          added to QHardwareManager.
        * Does simple set-get tests on provider attributes
          and ensures signals are emitted at the appropriate times.
        * Deletes the provider object and ensures it has
          been removed from QHardwareManager.
*/
void tst_QVibrateAccessory::providerTests()
{
    QHardwareManager* manager = new QHardwareManager( "QVibrateAccessory", this );

    // Test that the provider works as expected
    {
        QVibrateAccessoryProvider* provider = 0;
        {
            QSignalSpy spy(manager,SIGNAL(providerAdded(QString)));
            // Test construction
            provider = new QVibrateAccessoryProvider( "Qt Extended" );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << "Qt Extended" );
        }

        // Check that it was discovered by the accessories manager
        QVERIFY( manager->providers().contains( "Qt Extended" ) );
        QVERIFY( QHardwareManager::providers<QVibrateAccessory>().contains( "Qt Extended" ) );

        // Set and test various attributes
        provider->setSupportsVibrateNow( true );
        QVERIFY( provider->supportsVibrateNow() );
        provider->setSupportsVibrateNow( false );
        QVERIFY( !provider->supportsVibrateNow() );

        provider->setSupportsVibrateOnRing( true );
        QVERIFY( provider->supportsVibrateOnRing() );
        provider->setSupportsVibrateOnRing( false );
        QVERIFY( !provider->supportsVibrateOnRing() );

        {
            QSignalSpy spy(provider,SIGNAL(vibrateNowModified()));
            provider->setVibrateNow( true );
            QVERIFY( provider->vibrateNow() );
            provider->setVibrateNow( false );
            QTRY_VERIFY( spy.count() >= 1 ); spy.clear();
            QVERIFY( !provider->vibrateNow() );
            provider->setVibrateNow( true );
            QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
            QVERIFY( provider->vibrateNow() );
            provider->setVibrateNow( false );
            QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
            QVERIFY( !provider->vibrateNow() );
        }

        {
            QSignalSpy spy(provider,SIGNAL(vibrateOnRingModified()));
            provider->setVibrateOnRing( true );
            QVERIFY( provider->vibrateOnRing() );
            provider->setVibrateOnRing( false );
            QTRY_VERIFY( spy.count() >= 1 ); spy.clear();
            QVERIFY( !provider->vibrateOnRing() );
            provider->setVibrateOnRing( true );
            QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
            QVERIFY( provider->vibrateOnRing() );
            provider->setVibrateOnRing( false );
            QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
            QVERIFY( !provider->vibrateOnRing() );
        }

        {
            QSignalSpy spy(manager,SIGNAL(providerRemoved(QString)));
            delete provider;
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << "Qt Extended" );
        }
    }

    QValueSpaceObject::sync();
    QVERIFY( !manager->providers().contains( "Qt Extended" ) );
    QVERIFY( !QHardwareManager::providers<QVibrateAccessory>().contains( "Qt Extended" ) );
}

/*?
    Test function for the interaction between QVibrateAccessoryProvider
    and QVibrateAccessory.
    This test:
        * Creates a QVibrateAccessory without a Provider and ensures it is correctly
          unavailable.
        * Creates a QVibrateAccessoryProvider (provider) and ensures it is correctly
          added to QHardwareManager.
        * Creates a QVibrateAccessory with the same ID as the provider.
        * Does simple set-get tests on provider attributes,
          ensuring all changes to the provider also take effect on the accessory
          and signals are emitted at the appropriate times.
        * Deletes the provider and accessory and ensures it has
          been removed from QHardwareManager.
*/
void tst_QVibrateAccessory::accessoryTests()
{
    QHardwareManager* manager = new QHardwareManager( "QVibrateAccessory", this ) ;

    // Create the accessory
    {
        QVibrateAccessory* accessory = new QVibrateAccessory( "Qt Extended" );
        QVERIFY( !accessory->available() );
        delete accessory;
    }

    // Create the provider and try setting and getting attributes through
    // the accessory
    {
        QVibrateAccessoryProvider* provider = 0;
        {
            QSignalSpy spy(manager,SIGNAL(providerAdded(QString)));
            provider = new QVibrateAccessoryProvider( "Qt Extended" );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << "Qt Extended" );
        }

        QVibrateAccessory* accessory = new QVibrateAccessory( "Qt Extended" );
        QVERIFY( accessory->available() );

        // Set and test various attributes
        provider->setSupportsVibrateNow( true );
        QVERIFY( accessory->supportsVibrateNow() );
        provider->setSupportsVibrateNow( false );
        QVERIFY( !accessory->supportsVibrateNow() );

        provider->setSupportsVibrateOnRing( true );
        QVERIFY( accessory->supportsVibrateOnRing() );
        provider->setSupportsVibrateOnRing( false );
        QVERIFY( !accessory->supportsVibrateOnRing() );

        {
            QSignalSpy spy(provider,SIGNAL(vibrateNowModified()));
            provider->setVibrateNow( true );
            QVERIFY( accessory->vibrateNow() );
            provider->setVibrateNow( false );
            QTRY_VERIFY( spy.count() >= 1 ); spy.clear();
            QVERIFY( !accessory->vibrateNow() );
            provider->setVibrateNow( true );
            QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
            QVERIFY( accessory->vibrateNow() );
            provider->setVibrateNow( false );
            QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
            QVERIFY( !accessory->vibrateNow() );
        }

        {
            QSignalSpy spy(provider,SIGNAL(vibrateOnRingModified()));
            provider->setVibrateOnRing( true );
            QVERIFY( accessory->vibrateOnRing() );
            provider->setVibrateOnRing( false );
            QTRY_VERIFY( spy.count() >= 1 ); spy.clear();
            QVERIFY( !accessory->vibrateOnRing() );
            provider->setVibrateOnRing( true );
            QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
            QVERIFY( accessory->vibrateOnRing() );
            provider->setVibrateOnRing( false );
            QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
            QVERIFY( !accessory->vibrateOnRing() );
        }

        {
            QSignalSpy spy(manager,SIGNAL(providerRemoved(QString)));
            delete provider;
            delete accessory;
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << "Qt Extended" );
        }
    }

    // Create another accessory and check that it is not available
    {
        QVibrateAccessory* accessory = new QVibrateAccessory( "Qt Extended" );
        QVERIFY( !accessory->available() );
        delete accessory;
    }
}
