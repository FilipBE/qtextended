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

#include "tst_qsimtoolkit.h"

#ifndef SYSTEMTEST

// Test encoding and decoding of EVENT DOWNLOAD envelopes based on the
// Test encoding and decoding of MO SHORT MESSAGE CONTROL envelopes based on the
// GCF test strings in GSM 51.010, section 27.22.8 - MO SHORT MESSAGE CONTROL.
void tst_QSimToolkit::testEncodeMoSmsControl_data()
{
    QSimToolkitData::populateDataMoSmsControl();
}
void tst_QSimToolkit::testEncodeMoSmsControl()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, payload );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, respPayload );
    QFETCH( int, result );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the envelope PDU can be parsed correctly.
    QSimEnvelope decodedEnv = QSimEnvelope::fromPdu(data);
    QVERIFY( decodedEnv.type() == QSimEnvelope::MOSMSControl );
    QCOMPARE( decodedEnv.extensionData(), payload );

    // Check that the original envelope PDU can be reconstructed correctly.
    QCOMPARE( decodedEnv.toPdu(), data );

    // Check that the response PDU can be parsed correctly.
    QSimControlEvent decodedEvent = QSimControlEvent::fromPdu(QSimControlEvent::Sms, resp);
    QVERIFY( decodedEvent.type() == QSimControlEvent::Sms );
    QCOMPARE( (int)decodedEvent.result(), result );
    QCOMPARE( decodedEvent.extensionData(), respPayload );

    // Check that the response PDU can be reconstructed correctly.
    QCOMPARE( decodedEvent.toPdu(), resp );
}

// Test that MO SHORT MESSAGE CONTROL envelopes can be properly sent from client applications.
void tst_QSimToolkit::testDeliverMoSmsControl_data()
{
    QSimToolkitData::populateDataMoSmsControl();
}
void tst_QSimToolkit::testDeliverMoSmsControl()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, payload );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, respPayload );
    QFETCH( int, result );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();
    deliveredEvent = QSimControlEvent();

    // Compose and send the envelope.
    QSimEnvelope env;
    env.setType( QSimEnvelope::MOSMSControl );
    env.setExtensionData( payload );
    client->sendEnvelope( env );

    // Wait for the envelope to be received.
    QVERIFY( QFutureSignal::wait( server, SIGNAL(envelopeSeen()), 100 ) );

    // Check that the envelope is what we expected to get and that we didn't
    // get any terminal responses yet.
    QCOMPARE( server->responseCount(), 0 );
    QCOMPARE( server->envelopeCount(), 1 );
    QCOMPARE( server->lastEnvelope(), data );

    // Compose and send the SIM control event.
    QSimControlEvent ev;
    ev.setType( QSimControlEvent::Sms );
    ev.setResult( (QSimControlEvent::Result)result );
    ev.setExtensionData( respPayload );
    server->emitControlEvent( ev );

    // Wait for the event to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(eventSeen()), 100 ) );

    // Check that the event PDU was correct.
    QCOMPARE( deliveredEvent.toPdu(), resp );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for MO SHORT MESSAGE CONTROL from the GCF test cases
// in GSM 51.010, section 27.22.8.
void QSimToolkitData::populateDataMoSmsControl()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<QByteArray>("respPayload");
    QTest::addColumn<int>("result");

    static unsigned char const data_1_1_1a[] =
        {0xD5, 0x20, 0x02, 0x02, 0x82, 0x81, 0x06, 0x09, 0x91, 0x11, 0x22, 0x33,
         0x44, 0x55, 0x66, 0x77, 0xF8, 0x06, 0x06, 0x91, 0x10, 0x32, 0x54, 0x76,
         0xF8, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_1_1a[] =
        {0x06, 0x09, 0x91, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0xF8, 0x06,
         0x06, 0x91, 0x10, 0x32, 0x54, 0x76, 0xF8, 0x13, 0x07, 0x00, 0xF1, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_1_1a[] =
        {0x00, 0x00};
    QTest::newRow( "MO SHORT MESSAGE CONTROL 1.1.1A - GCF 27.22.8" )
        << QByteArray( (char *)data_1_1_1a, sizeof(data_1_1_1a) )
        << QByteArray( (char *)payl_1_1_1a, sizeof(payl_1_1_1a) )
        << QByteArray( (char *)resp_1_1_1a, sizeof(resp_1_1_1a) )
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    static unsigned char const data_1_1_1b[] =
        {0xD5, 0x20, 0x02, 0x02, 0x82, 0x81, 0x06, 0x09, 0x91, 0x11, 0x22, 0x33,
         0x44, 0x55, 0x66, 0x77, 0xF8, 0x06, 0x06, 0x91, 0x10, 0x32, 0x54, 0x76,
         0xF8, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_1_1b[] =
        {0x06, 0x09, 0x91, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0xF8, 0x06,
         0x06, 0x91, 0x10, 0x32, 0x54, 0x76, 0xF8, 0x13, 0x07, 0x00, 0x11, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_1_1b[] =
        {0x00, 0x00};
    QTest::newRow( "MO SHORT MESSAGE CONTROL 1.1.1B - GCF 27.22.8" )
        << QByteArray( (char *)data_1_1_1b, sizeof(data_1_1_1b) )
        << QByteArray( (char *)payl_1_1_1b, sizeof(payl_1_1_1b) )
        << QByteArray( (char *)resp_1_1_1b, sizeof(resp_1_1_1b) )
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    // 1.2.1 is essentially a duplicate of 1.1.1.  The main difference is that
    // 1.1.1 deals with an SMS message from the SIM toolkit application, and 1.2.1
    // deals with an SMS message composed manually by the user.  For our purposes,
    // the two test cases are indistinguishable.

    static unsigned char const data_1_3_1a[] =
        {0xD5, 0x20, 0x02, 0x02, 0x82, 0x81, 0x06, 0x09, 0x91, 0x11, 0x22, 0x33,
         0x44, 0x55, 0x66, 0x77, 0xF8, 0x06, 0x06, 0x91, 0x10, 0x32, 0x54, 0x76,
         0xF8, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_3_1a[] =
        {0x06, 0x09, 0x91, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0xF8, 0x06,
         0x06, 0x91, 0x10, 0x32, 0x54, 0x76, 0xF8, 0x13, 0x07, 0x00, 0xF1, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_3_1a[] =
        {0x01, 0x00};
    QTest::newRow( "MO SHORT MESSAGE CONTROL 1.3.1A - GCF 27.22.8" )
        << QByteArray( (char *)data_1_3_1a, sizeof(data_1_3_1a) )
        << QByteArray( (char *)payl_1_3_1a, sizeof(payl_1_3_1a) )
        << QByteArray( (char *)resp_1_3_1a, sizeof(resp_1_3_1a) )
        << QByteArray()
        << (int)( QSimControlEvent::NotAllowed );

    static unsigned char const data_1_3_1b[] =
        {0xD5, 0x20, 0x02, 0x02, 0x82, 0x81, 0x06, 0x09, 0x91, 0x11, 0x22, 0x33,
         0x44, 0x55, 0x66, 0x77, 0xF8, 0x06, 0x06, 0x91, 0x10, 0x32, 0x54, 0x76,
         0xF8, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_3_1b[] =
        {0x06, 0x09, 0x91, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0xF8, 0x06,
         0x06, 0x91, 0x10, 0x32, 0x54, 0x76, 0xF8, 0x13, 0x07, 0x00, 0x11, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_3_1b[] =
        {0x01, 0x00};
    QTest::newRow( "MO SHORT MESSAGE CONTROL 1.3.1B - GCF 27.22.8" )
        << QByteArray( (char *)data_1_3_1b, sizeof(data_1_3_1b) )
        << QByteArray( (char *)payl_1_3_1b, sizeof(payl_1_3_1b) )
        << QByteArray( (char *)resp_1_3_1b, sizeof(resp_1_3_1b) )
        << QByteArray()
        << (int)( QSimControlEvent::NotAllowed );

    // 1.4.1 is similar to 1.2.1 - a duplicate of 1.3.1, with a different way
    // of generating the SMS message.

    static unsigned char const data_1_5_1a[] =
        {0xD5, 0x20, 0x02, 0x02, 0x82, 0x81, 0x06, 0x09, 0x91, 0x11, 0x22, 0x33,
         0x44, 0x55, 0x66, 0x77, 0xF8, 0x06, 0x06, 0x91, 0x10, 0x32, 0x54, 0x76,
         0xF8, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_5_1a[] =
        {0x06, 0x09, 0x91, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0xF8, 0x06,
         0x06, 0x91, 0x10, 0x32, 0x54, 0x76, 0xF8, 0x13, 0x07, 0x00, 0xF1, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_5_1a[] =
        {0x02, 0x13, 0x86, 0x09, 0x91, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
         0xF9, 0x86, 0x06, 0x91, 0x10, 0x32, 0x54, 0x76, 0xF9};
    static unsigned char const rpay_1_5_1a[] =
        {0x86, 0x09, 0x91, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0xF9, 0x86,
         0x06, 0x91, 0x10, 0x32, 0x54, 0x76, 0xF9};
    QTest::newRow( "MO SHORT MESSAGE CONTROL 1.5.1A - GCF 27.22.8" )
        << QByteArray( (char *)data_1_5_1a, sizeof(data_1_5_1a) )
        << QByteArray( (char *)payl_1_5_1a, sizeof(payl_1_5_1a) )
        << QByteArray( (char *)resp_1_5_1a, sizeof(resp_1_5_1a) )
        << QByteArray( (char *)rpay_1_5_1a, sizeof(rpay_1_5_1a) )
        << (int)( QSimControlEvent::AllowedWithModifications );

    static unsigned char const data_1_5_1b[] =
        {0xD5, 0x20, 0x02, 0x02, 0x82, 0x81, 0x06, 0x09, 0x91, 0x11, 0x22, 0x33,
         0x44, 0x55, 0x66, 0x77, 0xF8, 0x06, 0x06, 0x91, 0x10, 0x32, 0x54, 0x76,
         0xF8, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_5_1b[] =
        {0x06, 0x09, 0x91, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0xF8, 0x06,
         0x06, 0x91, 0x10, 0x32, 0x54, 0x76, 0xF8, 0x13, 0x07, 0x00, 0x11, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_5_1b[] =
        {0x02, 0x13, 0x86, 0x09, 0x91, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
         0xF9, 0x86, 0x06, 0x91, 0x10, 0x32, 0x54, 0x76, 0xF9};
    static unsigned char const rpay_1_5_1b[] =
        {0x86, 0x09, 0x91, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0xF9, 0x86,
         0x06, 0x91, 0x10, 0x32, 0x54, 0x76, 0xF9};
    QTest::newRow( "MO SHORT MESSAGE CONTROL 1.5.1B - GCF 27.22.8" )
        << QByteArray( (char *)data_1_5_1b, sizeof(data_1_5_1b) )
        << QByteArray( (char *)payl_1_5_1b, sizeof(payl_1_5_1b) )
        << QByteArray( (char *)resp_1_5_1b, sizeof(resp_1_5_1b) )
        << QByteArray( (char *)rpay_1_5_1b, sizeof(rpay_1_5_1b) )
        << (int)( QSimControlEvent::AllowedWithModifications );

    // 1.6.1 is similar to 1.2.1 - a duplicate of 1.3.1, with a different way
    // of generating the SMS message.

    // 1.7.1 is identical to 1.1.1, but returns the result in a different way
    // from the SIM which will be converted by the modem vendor plugin into a
    // regular response and hence invisible to us.
}
