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
#include <QKeypadLightAccessory>
#include <QHardwareManager>
#include <QValueSpaceObject>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QEventLoop>
#include <QSignalSpy>
#include <shared/util.h>


//TESTED_CLASS=QKeypadLightAccessory,QKeypadLightAccessoryProvider
//TESTED_FILES=src/libraries/qtopia/qkeypadlightaccessory.h

/*
    The tst_QKeypadLightAccessory class provides unit tests for the QKeypadLightAccessory class.
*/
class tst_QKeypadLightAccessory : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanup();

    // QKeypadLightAccessory class test cases
    void accessoryTests();

    // QKeypadLightAccessoryProvider class test cases
    void providerTests();
};

QTEST_APP_MAIN( tst_QKeypadLightAccessory, QtopiaApplication )
#include "tst_qkeypadlightaccessory.moc"

/*?
    Initialisation prior to all test functions.
    Initialises the value space manager.
*/
void tst_QKeypadLightAccessory::initTestCase()
{
    QValueSpace::initValuespaceManager();
}

/*?
    Cleanup after every test function.
    Syncs value space.
*/
void tst_QKeypadLightAccessory::cleanup()
{
    QValueSpaceObject::sync();
}

/*?
    Test function for QKeypadLightAccessoryProvider.
    This test:
        * Creates a QKeypadLightAccessoryProvider and ensures it is correctly
          added to QHardwareManager.
        * Does simple set-get tests on QKeypadLightAccessoryProvider attributes,
          and ensures signals are emitted at the appropriate times.
        * Deletes the QKeypadLightAccessoryProvider object and ensures it has
          been removed from QHardwareManager.
*/
void tst_QKeypadLightAccessory::providerTests()
{
    QHardwareManager* manager = new QHardwareManager( "QKeypadLightAccessory", this );
    // Test that the provider works as expected
    {
        QKeypadLightAccessoryProvider* provider = 0;
        {
            // Test construction
            QSignalSpy spy(manager,SIGNAL(providerAdded(QString)));
            provider = new QKeypadLightAccessoryProvider( "Qt Extended" );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << "Qt Extended" );
        }

        // Check that it was discovered by the accessories manager
        QVERIFY( manager->providers().contains( "Qt Extended" ) );
        QVERIFY( QHardwareManager::providers<QKeypadLightAccessory>().contains( "Qt Extended" ) );

        {
            // Set and test various attributes
            QSignalSpy spy(provider,SIGNAL(onModified()));
            provider->setOn( true );
            QVERIFY( provider->on() );
            provider->setOn( false );
            QTRY_VERIFY( spy.count() >= 1 ); spy.clear();
            QVERIFY( !provider->on() );
            provider->setOn( true );
            QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
            QVERIFY( provider->on() );
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
    QVERIFY( !QHardwareManager::providers<QKeypadLightAccessory>().contains( "Qt Extended" ) );
}

/*?
    Test function for the interaction between QKeypadLightAccessoryProvider
    and QKeypadLightAccessory.
    This test:
        * Creates a QKeypadLightAccessory without a Provider and ensures it is correctly
          unavailable.
        * Creates a QKeypadLightAccessoryProvider and ensures it is correctly
          added to QHardwareManager.
        * Creates a QKeypadLightAccessory with the same ID as the provider.
        * Does simple set-get tests on QKeypadLightAccessoryProvider attributes,
          ensuring all changes to the provider also take effect on the accessory
          and signals are emitted at the appropriate times.
        * Deletes the provider and accessory and ensures it has
          been removed from QHardwareManager.
*/
void tst_QKeypadLightAccessory::accessoryTests()
{
    QHardwareManager* manager = new QHardwareManager( "QKeypadLightAccessory", this );

    // Create the accessory
    {
        QKeypadLightAccessory* accessory = new QKeypadLightAccessory( "Qt Extended" );
        QVERIFY( !accessory->available() );
        delete accessory;
    }

    // Create the provider and try setting and getting attributes through
    // the accessory
    {
        QKeypadLightAccessoryProvider* provider = 0;

        {
            QSignalSpy spy(manager,SIGNAL(providerAdded(QString)));
            provider = new QKeypadLightAccessoryProvider( "Qt Extended" );
            QTRY_VERIFY( spy.count() == 1 );
            QCOMPARE( spy.takeAt(0), QVariantList() << "Qt Extended" );
        }

        QKeypadLightAccessory* accessory = new QKeypadLightAccessory( "Qt Extended" );
        QVERIFY( accessory->available() );

        {
            // Set and test various attributes
            QSignalSpy spy(provider,SIGNAL(onModified()));
            provider->setOn( true );
            QVERIFY( accessory->on() );
            provider->setOn( false );
            QTRY_VERIFY( spy.count() >= 1 ); spy.clear();
            QVERIFY( !accessory->on() );
            provider->setOn( true );
            QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
            QVERIFY( accessory->on() );
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
        QKeypadLightAccessory* accessory = new QKeypadLightAccessory( "Qt Extended" );
        QVERIFY( !accessory->available() );
        delete accessory;
    }

}
