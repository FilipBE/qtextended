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
#include <QUsbStorageGadget>
#include <QUsbManager>
#include <QValueSpaceObject>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QEventLoop>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QProcess>


//TESTED_CLASS=QUsbStorageGadget,QUsbStorageGadgetProvider
//TESTED_FILES=src/libraries/qtopiacomm/usb/qusbstoragegadget.h

/*
    The tst_QUsbStorageGadget class provides unit tests for the QUsbStorageGadget class.
*/
class tst_QUsbStorageGadget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    // QUsbStorageGadget class test cases
    void gadgetTests();

    // QUsbStorageGadgetProvider class test cases
    void providerTests();
};

QTEST_APP_MAIN(tst_QUsbStorageGadget, QtopiaApplication)
#include "tst_qusbstoragegadget.moc"

/*?
    Initialisation prior to all test functions.
    Initialises the value space manager.
*/
void tst_QUsbStorageGadget::initTestCase()
{
    QValueSpace::initValuespaceManager();
}

/*?
    Cleanup after every test function.
    Syncs value space.
*/
void tst_QUsbStorageGadget::cleanup()
{
    QValueSpaceObject::sync();
}

/*?
    Test function for QUsbStorageGadgetProvider.
    This test:
        * Creates a QUsbStorageGadgetProvider.
        * Does simple set-get tests on QUsbStorageGadgetProvider attributes.
        * Activates and deactivates the gadget and ensures that the
          required signals are emitted.
*/
void tst_QUsbStorageGadget::providerTests()
{
    QUsbManager *manager = new QUsbManager(this);

    // Test that there is no current usb gadget
    QList<QUsbGadget *> activeGadgets = manager->activeGadgets();
    QVERIFY(activeGadgets.count() == 0);
    foreach (QUsbGadget *gadget, activeGadgets)
        delete gadget;

    // Test that the provider works as expected
    {
        QUsbStorageGadgetProvider *provider = new QUsbStorageGadgetProvider("Qt Extended");

        // Set and test various attributes
        {
            if (provider->supportsProduct()) {
                provider->setProduct("Qt Extended Product");
                QVERIFY(provider->product() == "Qt Extended Product");
            }

            if (provider->supportsProductId()) {
                provider->setProductId(0x5454);
                QVERIFY(provider->productId() == 0x5454);
            }

            if (provider->supportsVendor()) {
                provider->setVendor("Qt Extended Vendor");
                QVERIFY(provider->vendor() == "Qt Extended Vendor");
            }

            if (provider->supportsVendorId()) {
                provider->setVendorId(0x5454);
                QVERIFY(provider->vendorId() == 0x5454);
            }

            QVERIFY(provider->backingStore().isEmpty());

            provider->addBackingStore("/dev/partition1");
            QVERIFY(provider->backingStore().count() == 1);
            QVERIFY(provider->backingStore().contains("/dev/partition1"));

            provider->addBackingStore("/dev/partition2");
            QVERIFY(provider->backingStore().count() == 2);
            QVERIFY(provider->backingStore().contains("/dev/partition1"));
            QVERIFY(provider->backingStore().contains("/dev/partition2"));

            provider->removeBackingStore("/dev/partition1");
            QVERIFY(provider->backingStore().count() == 1);
            QVERIFY(!provider->backingStore().contains("/dev/partition1"));
            QVERIFY(provider->backingStore().contains("/dev/partition2"));

            QStringList partitions;
            partitions << "/dev/partition3" << "/dev/partition4";
            provider->setBackingStore(partitions);
            QVERIFY(provider->backingStore() == partitions);

            provider->setBackingStore(QStringList());
            QVERIFY(provider->backingStore().isEmpty());
        }

        // Activate and deactivate gadget
        if (QProcess::execute("/sbin/modinfo g_file_storage")) {
            QSKIP("This test requires the g_file_storage usb gadget driver", SkipSingle);
        } else {
            QVERIFY(manager->canActivate("Storage"));

            QSignalSpy activateSpy(provider, SIGNAL(activated()));
            QSignalSpy deactivateSpy(provider, SIGNAL(deactivated()));

            QTemporaryFile backingStore;
            backingStore.open();
            backingStore.resize(5*1024*1024);

            provider->addBackingStore(backingStore.fileName());

            provider->activate();

            //QTRY_VERIFY(activateSpy.count() + deactivateSpy.count() == 1);
            activateSpy.clear();
            deactivateSpy.clear();

            QVERIFY(provider->active());

            QList<QUsbGadget *> activeGadgets = manager->activeGadgets();
            bool activated = false;
            foreach (QUsbGadget *gadget, activeGadgets) {
                if (gadget->gadget() == provider->gadget())
                    activated = true;

                delete gadget;
            }
            QVERIFY(activated);

            provider->deactivate();
            //QTRY_VERIFY(activateSpy.count() + deactivateSpy.count() == 1);

            QVERIFY(!provider->active());

            activeGadgets = manager->activeGadgets();
            bool deactivated = true;
            foreach (QUsbGadget *gadget, activeGadgets) {
                if (gadget->gadget() == provider->gadget())
                    deactivated = false;

                delete gadget;
            }
            QVERIFY(deactivated);
        }

        delete provider;
    }

    delete manager;

    QValueSpaceObject::sync();
}

/*?
    Test function for the interaction between QUsbStorageGadgetProvider
    and QUsbStorageGadget.
    This test:
        * Creates a QUsbStorageGadget without a Provider and ensures it is correctly
          unavailable.
        * Creates a QUsbStorageGadgetProvider.
        * Creates a QUsbStorageGadget with the same ID as the provider.
        * Does simple set-get tests on QUsbStorageGadget attributes,
          ensuring all changes also take effect on the provider.
*/
void tst_QUsbStorageGadget::gadgetTests()
{
    QUsbManager *manager = new QUsbManager(this);

    // Create the gadget
    {
        QUsbStorageGadget *gadget = new QUsbStorageGadget("Qt Extended");
        QVERIFY(!gadget->available());
        delete gadget;
    }

    // Create the provider and try setting and getting attributes through
    // the gadget
    {
        QUsbStorageGadgetProvider *provider = new QUsbStorageGadgetProvider("Qt Extended");

        QUsbStorageGadget *gadget = new QUsbStorageGadget("Qt Extended");
        QVERIFY(gadget->available());

        // Set and test various attributes
        {
            const QByteArray mac = "00:11:22:33:44:55";

            QVERIFY(provider->supportsProduct() == gadget->supportsProduct());
            if (provider->supportsProduct()) {
                gadget->setProduct("Qt Extended Product");
                QCoreApplication::processEvents();
                QVERIFY(gadget->product() == "Qt Extended Product");
                QVERIFY(provider->product() == "Qt Extended Product");
            }

            QVERIFY(provider->supportsProductId() == gadget->supportsProductId());
            if (provider->supportsProductId()) {
                gadget->setProductId(0x5454);
                QCoreApplication::processEvents();
                QVERIFY(gadget->productId() == 0x5454);
                QVERIFY(provider->productId() == 0x5454);
            }

            QVERIFY(provider->supportsVendor() == gadget->supportsVendor());
            if (provider->supportsVendor()) {
                gadget->setVendor("Qt Extended Vendor");
                QCoreApplication::processEvents();
                QVERIFY(gadget->vendor() == "Qt Extended Vendor");
                QVERIFY(provider->vendor() == "Qt Extended Vendor");
            }

            QVERIFY(provider->supportsVendorId() == gadget->supportsVendorId());
            if (provider->supportsVendorId()) {
                gadget->setVendorId(0x5454);
                QCoreApplication::processEvents();
                QVERIFY(gadget->vendorId() == 0x5454);
                QVERIFY(provider->vendorId() == 0x5454);
            }

            QVERIFY(gadget->backingStore().isEmpty());

            gadget->addBackingStore("/dev/partition1");
            QCoreApplication::processEvents();
            QVERIFY(gadget->backingStore().count() == 1);
            QVERIFY(gadget->backingStore().contains("/dev/partition1"));
            QVERIFY(provider->backingStore().count() == 1);
            QVERIFY(provider->backingStore().contains("/dev/partition1"));

            gadget->addBackingStore("/dev/partition2");
            QCoreApplication::processEvents();
            QVERIFY(gadget->backingStore().count() == 2);
            QVERIFY(gadget->backingStore().contains("/dev/partition1"));
            QVERIFY(gadget->backingStore().contains("/dev/partition2"));
            QVERIFY(provider->backingStore().count() == 2);
            QVERIFY(provider->backingStore().contains("/dev/partition1"));
            QVERIFY(provider->backingStore().contains("/dev/partition2"));

            gadget->removeBackingStore("/dev/partition1");
            QCoreApplication::processEvents();
            QVERIFY(gadget->backingStore().count() == 1);
            QVERIFY(!gadget->backingStore().contains("/dev/partition1"));
            QVERIFY(gadget->backingStore().contains("/dev/partition2"));
            QVERIFY(provider->backingStore().count() == 1);
            QVERIFY(!provider->backingStore().contains("/dev/partition1"));
            QVERIFY(provider->backingStore().contains("/dev/partition2"));

            QStringList partitions;
            partitions << "/dev/partition3" << "/dev/partition4";
            gadget->setBackingStore(partitions);
            QCoreApplication::processEvents();
            QVERIFY(gadget->backingStore() == partitions);
            QVERIFY(provider->backingStore() == partitions);
        }

        delete gadget;
        delete provider;
    }

    delete manager;
}

