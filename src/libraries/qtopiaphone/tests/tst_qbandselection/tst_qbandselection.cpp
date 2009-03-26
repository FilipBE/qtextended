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

#include <QtopiaChannel>
#include <QtopiaApplication>
#include <QValueSpaceObject>
#include <QTest>

#include <qbandselection.h>

#include <QSignalSpy>

#include <shared/util.h>
#include <shared/qtopiaunittest.h>


//TESTED_CLASS=QBandSelection,QCommInterface,QAbstractIpcInterface
//TESTED_FILES=src/libraries/qtopiaphone/qbandselection.cpp,src/libraries/qtopiacomm/qcomminterface.cpp,src/libraries/qtopiabase/qabstractipcinterface.cpp

/*
    Unit test for QBandSelection

    This class is a unit test for QBandSelection.
    The class is tested for the correct emission of IPC messages as
    a client only; a QBandSelection server must be implemented in
    a subclass, hence server functionality cannot be tested here.
*/
class tst_QBandSelection : public QObject
{
Q_OBJECT
private slots:
    void testRequestSignals();
    void initTestCase();
};

QTEST_APP_MAIN( tst_QBandSelection, QtopiaApplication )

/*?
    Initialisation before all test functions.
    Initialises the valuespace.
*/
void tst_QBandSelection::initTestCase()
{
    QValueSpace::initValuespaceManager();
}

/*?
    Test that a client QBandSelection sends signals to the correct IPC channel
    when certain methods are called.  Specifically, this test ensures that messages
    are sent with the correct parameters for the following QBandSelection methods:
        * requestBand()
        * requestBands()
        * setBand()
*/
void tst_QBandSelection::testRequestSignals()
{
    for (int j = 0; j < 3; ++j) {
        /* To ensure that having multiple QBandSelection objects doesn't harm anything,
        * we create 5 servers and clients stored in arrays. */
        QBandSelection *srv[5];
        QBandSelection *client[5];
        for (int i = 0; i < 5; ++i) {
            /* Create a server. */
            QString service = QString("qtextended%1").arg(i);
            srv[i] = new QBandSelection(service, 0, QCommInterface::Server);
            /* Retrieve the service name, as we are not guaranteed to get the service requested. */
            service = srv[i]->service();

            /* Construct client. */
            QString channel = "QPE/Communications/QBandSelection/Request/" + service;
            client[i] = new QBandSelection(service);

            /* Test messages. */
            QtopiaChannel ch(channel);
            QSignalSpy spy(&ch, SIGNAL(received(QString,QByteArray)));

            client[i]->requestBand();
            QTRY_COMPARE( spy.count(), 1 );
            QCOMPARE( spy.takeFirst(), QVariantList() << "requestBand()" << QByteArray() );

            client[i]->requestBands();
            QTRY_COMPARE( spy.count(), 1 );
            QCOMPARE( spy.takeFirst(), QVariantList() << "requestBands()" << QByteArray() );

            client[i]->setBand(QBandSelection::Automatic, "ahoy");
            QTRY_COMPARE( spy.count(), 1 );
            QCOMPARE( spy.takeFirst().at(0).toString().toLatin1().constData(), "setBand(QBandSelection::BandMode,QString)" );
        }
        for (int i = 0; i < 5; ++i) {
            delete client[i];
            delete srv[i];
        }
    }
}

#include "tst_qbandselection.moc"

