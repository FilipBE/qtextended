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
#include <QUsbSerialGadget>
#include <QUsbManager>
#include <QValueSpaceObject>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QEventLoop>
#include <QSignalSpy>
#include <QProcess>

//TESTED_CLASS=QUsbSerialGadget,QUsbSerialGadgetProvider
//TESTED_FILES=src/libraries/qtopiacomm/usb/qusbserialgadget.h

/*
    The tst_QUsbSerialGadget class provides unit tests for the QUsbSerialGadget class.
*/
class tst_QUsbSerialGadget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    // QUsbSerialGadget class test cases
    void gadgetTests();

    // QUsbSerialGadgetProvider class test cases
    void providerTests();
};

QTEST_APP_MAIN(tst_QUsbSerialGadget, QtopiaApplication)
#include "tst_qusbserialgadget.moc"

/*?
    Initialisation prior to all test functions.
    Initialises the value space manager.
*/
void tst_QUsbSerialGadget::initTestCase()
{
    QValueSpace::initValuespaceManager();
}

/*?
    Cleanup after every test function.
    Syncs value space.
*/
void tst_QUsbSerialGadget::cleanup()
{
    QValueSpaceObject::sync();
}

/*?
    Test function for QUsbSerialGadgetProvider.
    This test:
        * Creates a QUsbSerialGadgetProvider.
        * Does simple set-get tests on QUsbSerialGadgetProvider attributes.
        * Activates and deactivates the gadget and ensures that the
          required signals are emitted.
*/
void tst_QUsbSerialGadget::providerTests()
{
    QUsbManager *manager = new QUsbManager(this);

    // Test that there is no current usb gadget
    QList<QUsbGadget *> activeGadgets = manager->activeGadgets();
    QVERIFY(activeGadgets.count() == 0);
    foreach (QUsbGadget *gadget, activeGadgets)
        delete gadget;

    // Test that the provider works as expected
    {
        QUsbSerialGadgetProvider *provider = new QUsbSerialGadgetProvider("Qt Extended");

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

            if (provider->supportsCdcAcm()) {
                provider->setCdcAcm(true);
                QVERIFY(provider->cdcAcm());
                provider->setCdcAcm(false);
                QVERIFY(!provider->cdcAcm());
            }
        }

        // Activate and deactivate gadget
        if (QProcess::execute("/sbin/modinfo g_serial")) {
            QSKIP("This test requires the g_serial usb gadget driver", SkipSingle);
        } else {
            QVERIFY(manager->canActivate("Serial"));

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
    Test function for the interaction between QUsbSerialGadgetProvider
    and QUsbSerialGadget.
    This test:
        * Creates a QUsbSerialGadget without a Provider and ensures it is correctly
          unavailable.
        * Creates a QUsbSerialGadgetProvider.
        * Creates a QUsbSerialGadget with the same ID as the provider.
        * Does simple set-get tests on QUsbSerialGadget attributes,
          ensuring all changes also take effect on the provider.
*/
void tst_QUsbSerialGadget::gadgetTests()
{
    QUsbManager *manager = new QUsbManager(this);

    // Create the gadget
    {
        QUsbSerialGadget *gadget = new QUsbSerialGadget("Qt Extended");
        QVERIFY(!gadget->available());
        delete gadget;
    }

    // Create the provider and try setting and getting attributes through
    // the gadget
    {
        QUsbSerialGadgetProvider *provider = new QUsbSerialGadgetProvider("Qt Extended");

        QUsbSerialGadget *gadget = new QUsbSerialGadget("Qt Extended");
        QVERIFY(gadget->available());

        // Set and test various attributes
        {
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

            QVERIFY(provider->supportsCdcAcm() == gadget->supportsCdcAcm());
            if (provider->supportsCdcAcm()) {
                gadget->setCdcAcm(true);
                QCoreApplication::processEvents();
                QVERIFY(gadget->cdcAcm());
                QVERIFY(provider->cdcAcm());
            }
        }

        delete gadget;
        delete provider;
    }

    delete manager;
}

