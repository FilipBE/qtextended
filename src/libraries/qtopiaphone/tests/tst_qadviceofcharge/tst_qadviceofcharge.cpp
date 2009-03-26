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

#include <QtopiaApplication>
#include <QValueSpaceObject>
#include <QSignalSpy>
#include <QtopiaChannel>

#include <shared/qtopiaunittest.h>
#include <shared/util.h>
#include <qadviceofcharge.h>


//TESTED_CLASS=QAdviceOfCharge,QCommInterface,QAbstractIpcInterface
//TESTED_FILES=src/libraries/qtopiaphone/qadviceofcharge.cpp,src/libraries/qtopiacomm/qcomminterface.cpp,src/libraries/qtopiabase/qabstractipcinterface.cpp

/*
    Unit test for QAdviceOfCharge.

    This class is a unit test for QAdviceOfCharge.
    The class is tested for the correct emission of IPC messages as
    a client only; a QAdviceOfCharge server must be implemented in
    a subclass such as \l {QModemAdviceOfCharge}, hence server
    functionality cannot be tested here.
*/
class tst_QAdviceOfCharge : public QObject
{
Q_OBJECT
private slots:
    void testRequestSignals();
    void initTestCase();
};

QTEST_APP_MAIN( tst_QAdviceOfCharge, QtopiaApplication )
#include "tst_qadviceofcharge.moc"

/*?
    Initialisation before all test functions.
    Initialises the valuespace.
*/
void tst_QAdviceOfCharge::initTestCase()
{
    QValueSpace::initValuespaceManager();
}

/*?
    Test that a client QAdviceOfCharge sends signals to the correct IPC channel
    when certain methods are called.  Specifically, this test ensures that messages
    are sent with the correct parameters for the following QAdviceOfCharge methods:
        * requestAccumulatedCallMeter()
        * requestAccumulatedCallMeterMaximum()
        * requestCurrentCallMeter()
        * requestPricePerUnit()
        * resetAccumulatedCallMeter()
        * setAccumulatedCallMeterMaximum()
        * setPricePerUnit()
*/
void tst_QAdviceOfCharge::testRequestSignals()
{
    for (int j = 0; j < 3; ++j) {
        /* To ensure that having multiple QAdviceOfCharge objects doesn't harm anything,
        * we create 5 servers and clients stored in arrays. */
        QAdviceOfCharge *srv[5];
        QAdviceOfCharge *client[5];
        for (int i = 0; i < 5; ++i) {
            /* Create a server. */
            QString service = QString("qtextended%1").arg(i);
            srv[i] = new QAdviceOfCharge(service, 0, QCommInterface::Server);
            /* Retrieve the service name, as we are not guaranteed to get the service requested. */
            service = srv[i]->service();

            /* Construct client. */
            QString channel = "QPE/Communications/QAdviceOfCharge/Request/" + service;
            client[i] = new QAdviceOfCharge(service);

            QtopiaChannel ch(channel);
            QSignalSpy spy(&ch, SIGNAL(received(QString,QByteArray)));

            /* Test messages. */
            client[i]->requestAccumulatedCallMeter();
            QTRY_COMPARE(spy.count(), 1);
            QCOMPARE(qPrintable(spy.takeFirst().at(0).toString()), "requestAccumulatedCallMeter()");

            client[i]->requestAccumulatedCallMeterMaximum();
            QTRY_COMPARE(spy.count(), 1);
            QCOMPARE(qPrintable(spy.takeFirst().at(0).toString()), "requestAccumulatedCallMeterMaximum()");

            client[i]->requestCurrentCallMeter();
            QTRY_COMPARE(spy.count(), 1);
            QCOMPARE(qPrintable(spy.takeFirst().at(0).toString()), "requestCurrentCallMeter()");

            client[i]->requestPricePerUnit();
            QTRY_COMPARE(spy.count(), 1);
            QCOMPARE(qPrintable(spy.takeFirst().at(0).toString()), "requestPricePerUnit()");

            client[i]->resetAccumulatedCallMeter("foobar");
            QTRY_COMPARE(spy.count(), 1);
            QCOMPARE(qPrintable(spy.takeFirst().at(0).toString()), "resetAccumulatedCallMeter(QString)");

            client[i]->setAccumulatedCallMeterMaximum(100, "foobar");
            QTRY_COMPARE(spy.count(), 1);
            QCOMPARE(qPrintable(spy.takeFirst().at(0).toString()), "setAccumulatedCallMeterMaximum(int,QString)");

            client[i]->setPricePerUnit("AUD", "1", "foobar");
            QTRY_COMPARE(spy.count(), 1);
            QCOMPARE(qPrintable(spy.takeFirst().at(0).toString()), "setPricePerUnit(QString,QString,QString)");
        }
        for (int i = 0; i < 5; ++i) {
            delete client[i];
            delete srv[i];
        }
    }
}
