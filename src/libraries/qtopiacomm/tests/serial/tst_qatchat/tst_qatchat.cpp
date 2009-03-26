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

#include <QObject>
#include <QTest>
#include <QDebug>
#include <QByteArray>
#include <qatchat.h>
#include "testserialiodevice.h"
#include "qfuturesignal.h"

//TESTED_CLASS=QAtChat
//TESTED_FILES=src/libraries/qtopiacomm/serial/qatchat.cpp

class tst_QAtChat : public QObject
{
    Q_OBJECT
public:
    tst_QAtChat()
    {
        device = 0;
        atchat = 0;
    }

private slots:
    void init();
    void cleanup();
    void testSettings();
    void testCPINTerminator();
    void testAbortDial();

signals:
    void cpinDone();
    void atdDone();
    void cimiDone(bool);

public:
    TestSerialIODevice *device;
    QAtChat *atchat;
};

void tst_QAtChat::init()
{
    if ( device )
        delete device;
    device = new TestSerialIODevice( this );
    atchat = device->atchat();
}

void tst_QAtChat::cleanup()
{
    if ( device ) {
        delete device;
        device = 0;
        atchat = 0;
    }
}

// Test the settings that can be changed on a QAtChat object.
void tst_QAtChat::testSettings()
{
    QCOMPARE( atchat->deadTimeout(), -1 );
    QCOMPARE( atchat->retryOnNonEcho(), -1 );

    atchat->setDeadTimeout( 100 );
    QCOMPARE( atchat->deadTimeout(), 100 );

    atchat->setDeadTimeout( -1 );
    QCOMPARE( atchat->deadTimeout(), -1 );

    atchat->setRetryOnNonEcho( 200 );
    QCOMPARE( atchat->retryOnNonEcho(), 200 );

    atchat->setRetryOnNonEcho( -1 );
    QCOMPARE( atchat->retryOnNonEcho(), -1 );
}

// Test that the CPIN hack for broken Wavecom modems works correctly.
void tst_QAtChat::testCPINTerminator()
{
    // Send the command without the CPIN hack enabled.
    atchat->chat( "AT+CPIN?", this, SIGNAL(cpinDone()) );

    // Check that the correct data was sent behind the scenes.
    QCOMPARE( device->readOutgoingData(), QByteArray( "AT+CPIN?\r" ) );

    // Respond to the command, but don't send the OK yet.
    device->addIncomingData( "AT+CPIN?\r\n" );
    device->addIncomingData( "+CPIN: READY\r\n" );

    // Wait for the cpinDone() signal.  It shouldn't arrive.
    QVERIFY( !QFutureSignal::wait( this, SIGNAL(cpinDone()), 100 ) );

    // Now send the OK and check again.  It should arrive this time.
    device->addIncomingData( "OK\r\n" );
    QVERIFY( QFutureSignal::wait( this, SIGNAL(cpinDone()), 100 ) );

    // Send the command with the CPIN hack enabled.
    atchat->setCPINTerminator();
    atchat->chat( "AT+CPIN?", this, SIGNAL(cpinDone()) );

    // Check that the correct data was sent behind the scenes.
    QCOMPARE( device->readOutgoingData(), QByteArray( "AT+CPIN?\r" ) );

    // Respond to the command, but don't send the OK.
    device->addIncomingData( "AT+CPIN?\r\n" );
    device->addIncomingData( "+CPIN: READY\r\n" );

    // Wait for the cpinDone() signal.  It should arrive.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(cpinDone()), 100 ) );
}

// Test that abortDial() works.
void tst_QAtChat::testAbortDial()
{
    // Send the dial command and check it.
    atchat->chat( "ATD1234;", this, SIGNAL(atdDone()) );
    QCOMPARE( device->readOutgoingData(), QByteArray( "ATD1234;\r" ) );

    // Queue up another command, which should be delayed until after the ATD.
    QFutureSignal fs( this, SIGNAL(cimiDone(bool)) );
    atchat->chat( "AT+CIMI", this, SIGNAL(cimiDone(bool)) );

    // Respond with an echo.
    device->addIncomingData( "ATD1234;\r\n" );

    // Wait a bit to see if the commands ends (it shouldn't).
    QVERIFY( !QFutureSignal::wait( this, SIGNAL(atdDone()), 100 ) );

    // Send the abortDial() request, which should be a single CR.
    atchat->abortDial();
    QCOMPARE( device->readOutgoingData(), QByteArray( "\r" ) );

    // It still shouldn't end yet - waiting for the "NO CARRIER".
    QVERIFY( !QFutureSignal::wait( this, SIGNAL(atdDone()), 100 ) );

    // The AT+CIMI should still be in the queue.
    QCOMPARE( fs.resultCount(), 0 );

    // Simulate the "NO CARRIER" and the response to AT+CIMI.
    device->addIncomingData( "NO CARRIER\r\n" );
    device->addIncomingData( "AT+CIMI\r\n" );
    device->addIncomingData( "OK\r\n" );

    // Now it should end.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(atdDone()), 100 ) );

    // The AT+CIMI should have also terminated.  We have to use a full QFutureSignal
    // here because QFutureSignal::wait() will miss the signal.  It has already
    // occurred during the queue flush for the ATD result.
    QCOMPARE( fs.resultCount(), 1 );
    QList<QVariant> args = fs.results()[0];
    QVERIFY( args.at(0).type() == QVariant::Bool );
    QVERIFY( args.at(0).toBool() );
}

QTEST_MAIN( tst_QAtChat )

#include "tst_qatchat.moc"
