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
#include <QUsbEthernetGadget>
#include <QUsbManager>
#include <QValueSpaceObject>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QEventLoop>
#include <QSignalSpy>
#include <QProcess>


//TESTED_CLASS=QUsbEthernetGadget,QUsbEthernetGadgetProvider
//TESTED_FILES=src/libraries/qtopiacomm/usb/qusbethernetgadget.h

/*
    The tst_QUsbEthernetGadget class provides unit tests for the QUsbEthernetGadget class.
*/
class tst_QUsbEthernetGadget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    // QUsbEthernetGadget class test cases
    void gadgetTests();

    // QUsbEthernetGadgetProvider class test cases
    void providerTests();
};

QTEST_APP_MAIN(tst_QUsbEthernetGadget, QtopiaApplication)
#include "tst_qusbethernetgadget.moc"

/*?
    Initialisation prior to all test functions.
    Initialises the value space manager.
*/
void tst_QUsbEthernetGadget::initTestCase()
{
    QValueSpace::initValuespaceManager();
}

/*?
    Cleanup after every test function.
    Syncs value space.
*/
void tst_QUsbEthernetGadget::cleanup()
{
    QValueSpaceObject::sync();
}

/*?
    Test function for QUsbEthernetGadgetProvider.
    This test:
        * Creates a QUsbEthernetGadgetProvider.
        * Does simple set-get tests on QUsbEthernetGadgetProvider attributes.
        * Activates and deactivates the gadget and ensures that the
          required signals are emitted.
*/
void tst_QUsbEthernetGadget::providerTests()
{
    QUsbManager *manager = new QUsbManager(this);

    // Test that there is no current usb gadget
    QList<QUsbGadget *> activeGadgets = manager->activeGadgets();
    QVERIFY(activeGadgets.count() == 0);
    foreach (QUsbGadget *gadget, activeGadgets)
        delete gadget;

    // Test that the provider works as expected
    {
        QUsbEthernetGadgetProvider *provider = new QUsbEthernetGadgetProvider("Qt Extended");

        // Set and test various attributes
        {
            const QByteArray mac = "00:11:22:33:44:55";

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

            provider->setLocalMac(mac);
            QVERIFY(provider->localMac() == mac);

            provider->setRemoteMac(mac);
            QVERIFY(provider->remoteMac() == mac);
        }

        // Activate and deactivate gadget
        if (QProcess::execute("/sbin/modinfo g_ether")) {
            QSKIP("This test requires the g_ether usb gadget driver", SkipSingle);
        } else {
            QVERIFY(manager->canActivate("Ethernet"));

            QSignalSpy activateSpy(provider, SIGNAL(activated()));
            QSignalSpy deactivateSpy(provider, SIGNAL(deactivated()));

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
    Test function for the interaction between QUsbEthernetGadgetProvider
    and QUsbEthernetGadget.
    This test:
        * Creates a QUsbEthernetGadget without a Provider and ensures it is correctly
          unavailable.
        * Creates a QUsbEthernetGadgetProvider.
        * Creates a QUsbEthernetGadget with the same ID as the provider.
        * Does simple set-get tests on QUsbEthernetGadget attributes,
          ensuring all changes also take effect on the provider.
*/
void tst_QUsbEthernetGadget::gadgetTests()
{
    QUsbManager *manager = new QUsbManager(this);

    // Create the gadget
    {
        QUsbEthernetGadget *gadget = new QUsbEthernetGadget("Qt Extended");
        QVERIFY(!gadget->available());
        delete gadget;
    }

    // Create the provider and try setting and getting attributes through
    // the gadget
    {
        QUsbEthernetGadgetProvider *provider = new QUsbEthernetGadgetProvider("Qt Extended");

        QUsbEthernetGadget *gadget = new QUsbEthernetGadget("Qt Extended");
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

            gadget->setLocalMac(mac);
            QCoreApplication::processEvents();
            QVERIFY(gadget->localMac() == mac);
            QVERIFY(provider->localMac() == mac);

            gadget->setRemoteMac(mac);
            QCoreApplication::processEvents();
            QVERIFY(gadget->remoteMac() == mac);
            QVERIFY(provider->remoteMac() == mac);
        }

        delete gadget;
        delete provider;
    }

    delete manager;
}

