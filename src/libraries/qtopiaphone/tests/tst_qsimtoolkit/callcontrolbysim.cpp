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

// Test encoding and decoding of CALL CONTROL BY SIM envelopes based on the
// GCF test strings in GSM 51.010, section 27.22.6 - CALL CONTROL BY SIM.
void tst_QSimToolkit::testEncodeCallControlBySim_data()
{
    QSimToolkitData::populateDataCallControlBySim();
}
void tst_QSimToolkit::testEncodeCallControlBySim()
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
    QVERIFY( decodedEnv.type() == QSimEnvelope::CallControl );
    QCOMPARE( decodedEnv.extensionData(), payload );

    // Check that the original envelope PDU can be reconstructed correctly.
    QByteArray pdu = decodedEnv.toPdu();
    pdu[2] = data[2];   // Deal with 0x02 vs 0x82 discrepancies in the test data.
    QCOMPARE( pdu, data );

    // Bail out if no response PDU to be processed.
    if ( resp.isEmpty() )
        return;

    // Check that the response PDU can be parsed correctly.
    QSimControlEvent decodedEvent = QSimControlEvent::fromPdu(QSimControlEvent::Sms, resp);
    QVERIFY( decodedEvent.type() == QSimControlEvent::Sms );
    QCOMPARE( (int)decodedEvent.result(), result );
    QCOMPARE( decodedEvent.extensionData(), respPayload );

    // Check that the response PDU can be reconstructed correctly.
    QCOMPARE( decodedEvent.toPdu(), resp );
}

// Test that CALL CONTROL BY SIM envelopes can be properly sent from client applications.
void tst_QSimToolkit::testDeliverCallControlBySim_data()
{
    QSimToolkitData::populateDataCallControlBySim();
}
void tst_QSimToolkit::testDeliverCallControlBySim()
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
    env.setType( QSimEnvelope::CallControl );
    env.setExtensionData( payload );
    client->sendEnvelope( env );

    // Wait for the envelope to be received.
    QVERIFY( QFutureSignal::wait( server, SIGNAL(envelopeSeen()), 100 ) );

    // Check that the envelope is what we expected to get and that we didn't
    // get any terminal responses yet.
    QCOMPARE( server->responseCount(), 0 );
    QCOMPARE( server->envelopeCount(), 1 );
    QByteArray pdu = server->lastEnvelope();
    pdu[2] = data[2];   // Deal with 0x02 vs 0x82 discrepancies in the test data.
    QCOMPARE( pdu, data );

    // Bail out if no response to be checked.
    if ( resp.isEmpty() )
        return;

    // Compose and send the SIM control event.
    QSimControlEvent ev;
    ev.setType( QSimControlEvent::Call );
    ev.setResult( (QSimControlEvent::Result)result );
    ev.setExtensionData( respPayload );
    server->emitControlEvent( ev );

    // Wait for the event to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(eventSeen()), 100 ) );

    // Check that the event PDU was correct.
    QCOMPARE( deliveredEvent.toPdu(), resp );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for CALL CONTROL BY SIM from the GCF test cases
// in GSM 51.010, section 27.22.6.
void QSimToolkitData::populateDataCallControlBySim()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<QByteArray>("respPayload");
    QTest::addColumn<int>("result");

    static unsigned char const data_1_1_1a[] =
        {0xD4, 0x1A, 0x82, 0x02, 0x82, 0x81, 0x86, 0x0B, 0x91, 0x10, 0x32, 0x54,
         0x76, 0x98, 0x10, 0x32, 0x54, 0x76, 0x98, 0x13, 0x07, 0x00, 0xF1, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_1_1a[] =
        {0x86, 0x0B, 0x91, 0x10, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0x54, 0x76,
         0x98, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    QTest::newRow( "CALL CONTROL BY SIM 1.1.1A - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_1_1a, sizeof(data_1_1_1a) )
        << QByteArray( (char *)payl_1_1_1a, sizeof(payl_1_1_1a) )
        << QByteArray()
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    static unsigned char const data_1_1_1b[] =
        {0xD4, 0x1A, 0x82, 0x02, 0x82, 0x81, 0x86, 0x0B, 0x91, 0x10, 0x32, 0x54,
         0x76, 0x98, 0x10, 0x32, 0x54, 0x76, 0x98, 0x13, 0x07, 0x00, 0x11, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_1_1b[] =
        {0x86, 0x0B, 0x91, 0x10, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0x54, 0x76,
         0x98, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    QTest::newRow( "CALL CONTROL BY SIM 1.1.1B - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_1_1b, sizeof(data_1_1_1b) )
        << QByteArray( (char *)payl_1_1_1b, sizeof(payl_1_1_1b) )
        << QByteArray()
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    static unsigned char const data_1_2_1a[] =
        {0xD4, 0x1A, 0x82, 0x02, 0x82, 0x81, 0x86, 0x0B, 0x91, 0x10, 0x32, 0x54,
         0x76, 0x98, 0x10, 0x32, 0x54, 0x76, 0x98, 0x13, 0x07, 0x00, 0xF1, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_2_1a[] =
        {0x86, 0x0B, 0x91, 0x10, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0x54, 0x76,
         0x98, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_2_1a[] =
        {0x00, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 1.2.1A - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_2_1a, sizeof(data_1_2_1a) )
        << QByteArray( (char *)payl_1_2_1a, sizeof(payl_1_2_1a) )
        << QByteArray( (char *)resp_1_2_1a, sizeof(resp_1_2_1a) )
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    static unsigned char const data_1_2_1b[] =
        {0xD4, 0x1A, 0x82, 0x02, 0x82, 0x81, 0x86, 0x0B, 0x91, 0x10, 0x32, 0x54,
         0x76, 0x98, 0x10, 0x32, 0x54, 0x76, 0x98, 0x13, 0x07, 0x00, 0x11, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_2_1b[] =
        {0x86, 0x0B, 0x91, 0x10, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0x54, 0x76,
         0x98, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_2_1b[] =
        {0x00, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 1.2.1B - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_2_1b, sizeof(data_1_2_1b) )
        << QByteArray( (char *)payl_1_2_1b, sizeof(payl_1_2_1b) )
        << QByteArray( (char *)resp_1_2_1b, sizeof(resp_1_2_1b) )
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    static unsigned char const data_1_3_1a[] =
        {0xD4, 0x16, 0x02, 0x02, 0x82, 0x81, 0x06, 0x07, 0x91, 0x10, 0x32, 0x04,
         0x21, 0x43, 0x65, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_3_1a[] =
        {0x06, 0x07, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x13, 0x07, 0x00,
         0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_3_1a[] =
        {0x00, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 1.3.1A - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_3_1a, sizeof(data_1_3_1a) )
        << QByteArray( (char *)payl_1_3_1a, sizeof(payl_1_3_1a) )
        << QByteArray( (char *)resp_1_3_1a, sizeof(resp_1_3_1a) )
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    static unsigned char const data_1_3_1b[] =
        {0xD4, 0x16, 0x02, 0x02, 0x82, 0x81, 0x06, 0x07, 0x91, 0x10, 0x32, 0x04,
         0x21, 0x43, 0x65, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_3_1b[] =
        {0x06, 0x07, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x13, 0x07, 0x00,
         0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_3_1b[] =
        {0x00, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 1.3.1B - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_3_1b, sizeof(data_1_3_1b) )
        << QByteArray( (char *)payl_1_3_1b, sizeof(payl_1_3_1b) )
        << QByteArray( (char *)resp_1_3_1b, sizeof(resp_1_3_1b) )
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    static unsigned char const data_1_4_1a[] =
        {0xD4, 0x1A, 0x82, 0x02, 0x82, 0x81, 0x86, 0x0B, 0x91, 0x10, 0x32, 0x54,
         0x76, 0x98, 0x10, 0x32, 0x54, 0x76, 0x98, 0x13, 0x07, 0x00, 0xF1, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_4_1a[] =
        {0x86, 0x0B, 0x91, 0x10, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0x54, 0x76,
         0x98, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_4_1a[] =
        {0x01, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 1.4.1A - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_4_1a, sizeof(data_1_4_1a) )
        << QByteArray( (char *)payl_1_4_1a, sizeof(payl_1_4_1a) )
        << QByteArray( (char *)resp_1_4_1a, sizeof(resp_1_4_1a) )
        << QByteArray()
        << (int)( QSimControlEvent::NotAllowed );

    static unsigned char const data_1_4_1b[] =
        {0xD4, 0x1A, 0x82, 0x02, 0x82, 0x81, 0x86, 0x0B, 0x91, 0x10, 0x32, 0x54,
         0x76, 0x98, 0x10, 0x32, 0x54, 0x76, 0x98, 0x13, 0x07, 0x00, 0x11, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_4_1b[] =
        {0x86, 0x0B, 0x91, 0x10, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0x54, 0x76,
         0x98, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_4_1b[] =
        {0x01, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 1.4.1B - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_4_1b, sizeof(data_1_4_1b) )
        << QByteArray( (char *)payl_1_4_1b, sizeof(payl_1_4_1b) )
        << QByteArray( (char *)resp_1_4_1b, sizeof(resp_1_4_1b) )
        << QByteArray()
        << (int)( QSimControlEvent::NotAllowed );

    static unsigned char const data_1_5_1a[] =
        {0xD4, 0x16, 0x02, 0x02, 0x82, 0x81, 0x06, 0x07, 0x91, 0x10, 0x32, 0x04,
         0x21, 0x43, 0x65, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_5_1a[] =
        {0x06, 0x07, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x13, 0x07, 0x00,
         0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_5_1a[] =
        {0x01, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 1.5.1A - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_5_1a, sizeof(data_1_5_1a) )
        << QByteArray( (char *)payl_1_5_1a, sizeof(payl_1_5_1a) )
        << QByteArray( (char *)resp_1_5_1a, sizeof(resp_1_5_1a) )
        << QByteArray()
        << (int)( QSimControlEvent::NotAllowed );

    static unsigned char const data_1_5_1b[] =
        {0xD4, 0x16, 0x02, 0x02, 0x82, 0x81, 0x06, 0x07, 0x91, 0x10, 0x32, 0x04,
         0x21, 0x43, 0x65, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_5_1b[] =
        {0x06, 0x07, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x13, 0x07, 0x00,
         0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_5_1b[] =
        {0x01, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 1.5.1B - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_5_1b, sizeof(data_1_5_1b) )
        << QByteArray( (char *)payl_1_5_1b, sizeof(payl_1_5_1b) )
        << QByteArray( (char *)resp_1_5_1b, sizeof(resp_1_5_1b) )
        << QByteArray()
        << (int)( QSimControlEvent::NotAllowed );

    static unsigned char const data_1_6_1a[] =
        {0xD4, 0x1A, 0x82, 0x02, 0x82, 0x81, 0x86, 0x0B, 0x91, 0x10, 0x32, 0x54,
         0x76, 0x98, 0x10, 0x32, 0x54, 0x76, 0x98, 0x13, 0x07, 0x00, 0xF1, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_6_1a[] =
        {0x86, 0x0B, 0x91, 0x10, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0x54, 0x76,
         0x98, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_6_1a[] =
        {0x02, 0x06, 0x86, 0x04, 0x91, 0x10, 0x20, 0x30};
    static unsigned char const rpay_1_6_1a[] =
        {0x86, 0x04, 0x91, 0x10, 0x20, 0x30};
    QTest::newRow( "CALL CONTROL BY SIM 1.6.1A - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_6_1a, sizeof(data_1_6_1a) )
        << QByteArray( (char *)payl_1_6_1a, sizeof(payl_1_6_1a) )
        << QByteArray( (char *)resp_1_6_1a, sizeof(resp_1_6_1a) )
        << QByteArray( (char *)rpay_1_6_1a, sizeof(rpay_1_6_1a) )
        << (int)( QSimControlEvent::AllowedWithModifications );

    static unsigned char const data_1_6_1b[] =
        {0xD4, 0x1A, 0x82, 0x02, 0x82, 0x81, 0x86, 0x0B, 0x91, 0x10, 0x32, 0x54,
         0x76, 0x98, 0x10, 0x32, 0x54, 0x76, 0x98, 0x13, 0x07, 0x00, 0x11, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_6_1b[] =
        {0x86, 0x0B, 0x91, 0x10, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0x54, 0x76,
         0x98, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_6_1b[] =
        {0x02, 0x06, 0x86, 0x04, 0x91, 0x10, 0x20, 0x30};
    static unsigned char const rpay_1_6_1b[] =
        {0x86, 0x04, 0x91, 0x10, 0x20, 0x30};
    QTest::newRow( "CALL CONTROL BY SIM 1.6.1B - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_6_1b, sizeof(data_1_6_1b) )
        << QByteArray( (char *)payl_1_6_1b, sizeof(payl_1_6_1b) )
        << QByteArray( (char *)resp_1_6_1b, sizeof(resp_1_6_1b) )
        << QByteArray( (char *)rpay_1_6_1b, sizeof(rpay_1_6_1b) )
        << (int)( QSimControlEvent::AllowedWithModifications );

    static unsigned char const data_1_7_1a[] =
        {0xD4, 0x16, 0x02, 0x02, 0x82, 0x81, 0x06, 0x07, 0x91, 0x10, 0x32, 0x04,
         0x21, 0x43, 0x65, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_7_1a[] =
        {0x06, 0x07, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x13, 0x07, 0x00,
         0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_7_1a[] =
        {0x02, 0x09, 0x86, 0x07, 0x91, 0x10, 0x11, 0x11, 0x11, 0x11, 0x11};
    static unsigned char const rpay_1_7_1a[] =
        {0x86, 0x07, 0x91, 0x10, 0x11, 0x11, 0x11, 0x11, 0x11};
    QTest::newRow( "CALL CONTROL BY SIM 1.7.1A - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_7_1a, sizeof(data_1_7_1a) )
        << QByteArray( (char *)payl_1_7_1a, sizeof(payl_1_7_1a) )
        << QByteArray( (char *)resp_1_7_1a, sizeof(resp_1_7_1a) )
        << QByteArray( (char *)rpay_1_7_1a, sizeof(rpay_1_7_1a) )
        << (int)( QSimControlEvent::AllowedWithModifications );

    static unsigned char const data_1_7_1b[] =
        {0xD4, 0x16, 0x02, 0x02, 0x82, 0x81, 0x06, 0x07, 0x91, 0x10, 0x32, 0x04,
         0x21, 0x43, 0x65, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_7_1b[] =
        {0x06, 0x07, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x13, 0x07, 0x00,
         0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_7_1b[] =
        {0x02, 0x09, 0x86, 0x07, 0x91, 0x10, 0x11, 0x11, 0x11, 0x11, 0x11};
    static unsigned char const rpay_1_7_1b[] =
        {0x86, 0x07, 0x91, 0x10, 0x11, 0x11, 0x11, 0x11, 0x11};
    QTest::newRow( "CALL CONTROL BY SIM 1.7.1B - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_7_1b, sizeof(data_1_7_1b) )
        << QByteArray( (char *)payl_1_7_1b, sizeof(payl_1_7_1b) )
        << QByteArray( (char *)resp_1_7_1b, sizeof(resp_1_7_1b) )
        << QByteArray( (char *)rpay_1_7_1b, sizeof(rpay_1_7_1b) )
        << (int)( QSimControlEvent::AllowedWithModifications );

    static unsigned char const data_1_8_1a[] =
        {0xD4, 0x1A, 0x82, 0x02, 0x82, 0x81, 0x86, 0x0B, 0x91, 0x10, 0x32, 0x54,
         0x76, 0x98, 0x10, 0x32, 0x54, 0x76, 0x98, 0x13, 0x07, 0x00, 0xF1, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_8_1a[] =
        {0x86, 0x0B, 0x91, 0x10, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0x54, 0x76,
         0x98, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_8_1a[] =
        {0x02, 0x05, 0x86, 0x03, 0x81, 0x11, 0xF2};
    static unsigned char const rpay_1_8_1a[] =
        {0x86, 0x03, 0x81, 0x11, 0xF2};
    QTest::newRow( "CALL CONTROL BY SIM 1.8.1A - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_8_1a, sizeof(data_1_8_1a) )
        << QByteArray( (char *)payl_1_8_1a, sizeof(payl_1_8_1a) )
        << QByteArray( (char *)resp_1_8_1a, sizeof(resp_1_8_1a) )
        << QByteArray( (char *)rpay_1_8_1a, sizeof(rpay_1_8_1a) )
        << (int)( QSimControlEvent::AllowedWithModifications );

    static unsigned char const data_1_8_1b[] =
        {0xD4, 0x1A, 0x82, 0x02, 0x82, 0x81, 0x86, 0x0B, 0x91, 0x10, 0x32, 0x54,
         0x76, 0x98, 0x10, 0x32, 0x54, 0x76, 0x98, 0x13, 0x07, 0x00, 0x11, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_8_1b[] =
        {0x86, 0x0B, 0x91, 0x10, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0x54, 0x76,
         0x98, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_8_1b[] =
        {0x02, 0x05, 0x86, 0x03, 0x81, 0x11, 0xF2};
    static unsigned char const rpay_1_8_1b[] =
        {0x86, 0x03, 0x81, 0x11, 0xF2};
    QTest::newRow( "CALL CONTROL BY SIM 1.8.1B - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_8_1b, sizeof(data_1_8_1b) )
        << QByteArray( (char *)payl_1_8_1b, sizeof(payl_1_8_1b) )
        << QByteArray( (char *)resp_1_8_1b, sizeof(resp_1_8_1b) )
        << QByteArray( (char *)rpay_1_8_1b, sizeof(rpay_1_8_1b) )
        << (int)( QSimControlEvent::AllowedWithModifications );

    static unsigned char const data_1_9_1a[] =
        {0xD4, 0x1A, 0x82, 0x02, 0x82, 0x81, 0x86, 0x0B, 0x91, 0x10, 0x32, 0x54,
         0x76, 0x98, 0x10, 0x32, 0x54, 0x76, 0x98, 0x13, 0x07, 0x00, 0xF1, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_9_1a[] =
        {0x86, 0x0B, 0x91, 0x10, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0x54, 0x76,
         0x98, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_9_1a[] =
        {0x02, 0x05, 0x86, 0x03, 0x81, 0x01, 0x02};
    static unsigned char const rpay_1_9_1a[] =
        {0x86, 0x03, 0x81, 0x01, 0x02};
    QTest::newRow( "CALL CONTROL BY SIM 1.9.1A - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_9_1a, sizeof(data_1_9_1a) )
        << QByteArray( (char *)payl_1_9_1a, sizeof(payl_1_9_1a) )
        << QByteArray( (char *)resp_1_9_1a, sizeof(resp_1_9_1a) )
        << QByteArray( (char *)rpay_1_9_1a, sizeof(rpay_1_9_1a) )
        << (int)( QSimControlEvent::AllowedWithModifications );

    static unsigned char const data_1_9_1b[] =
        {0xD4, 0x1A, 0x82, 0x02, 0x82, 0x81, 0x86, 0x0B, 0x91, 0x10, 0x32, 0x54,
         0x76, 0x98, 0x10, 0x32, 0x54, 0x76, 0x98, 0x13, 0x07, 0x00, 0x11, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_1_9_1b[] =
        {0x86, 0x0B, 0x91, 0x10, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0x54, 0x76,
         0x98, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_1_9_1b[] =
        {0x02, 0x05, 0x86, 0x03, 0x81, 0x01, 0x02};
    static unsigned char const rpay_1_9_1b[] =
        {0x86, 0x03, 0x81, 0x01, 0x02};
    QTest::newRow( "CALL CONTROL BY SIM 1.9.1B - GCF 27.22.6.1" )
        << QByteArray( (char *)data_1_9_1b, sizeof(data_1_9_1b) )
        << QByteArray( (char *)payl_1_9_1b, sizeof(payl_1_9_1b) )
        << QByteArray( (char *)resp_1_9_1b, sizeof(resp_1_9_1b) )
        << QByteArray( (char *)rpay_1_9_1b, sizeof(rpay_1_9_1b) )
        << (int)( QSimControlEvent::AllowedWithModifications );

    // 1.10.1 to 1.14.1 are repetitions of other test cases.

    static unsigned char const data_2_1_1a[] =
        {0xD4, 0x14, 0x82, 0x02, 0x82, 0x81, 0x89, 0x05, 0xFF, 0x2A, 0xA1, 0x1A,
         0xB0, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_2_1_1a[] =
        {0x89, 0x05, 0xFF, 0x2A, 0xA1, 0x1A, 0xB0, 0x13, 0x07, 0x00, 0xF1, 0x10,
         0x00, 0x01, 0x00, 0x01};
    QTest::newRow( "CALL CONTROL BY SIM 2.1.1A - GCF 27.22.6.2" )
        << QByteArray( (char *)data_2_1_1a, sizeof(data_2_1_1a) )
        << QByteArray( (char *)payl_2_1_1a, sizeof(payl_2_1_1a) )
        << QByteArray()
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    static unsigned char const data_2_1_1b[] =
        {0xD4, 0x14, 0x82, 0x02, 0x82, 0x81, 0x89, 0x05, 0xFF, 0x2A, 0xA1, 0x1A,
         0xB0, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_2_1_1b[] =
        {0x89, 0x05, 0xFF, 0x2A, 0xA1, 0x1A, 0xB0, 0x13, 0x07, 0x00, 0x11, 0x10,
         0x00, 0x01, 0x00, 0x01};
    QTest::newRow( "CALL CONTROL BY SIM 2.1.1B - GCF 27.22.6.2" )
        << QByteArray( (char *)data_2_1_1b, sizeof(data_2_1_1b) )
        << QByteArray( (char *)payl_2_1_1b, sizeof(payl_2_1_1b) )
        << QByteArray()
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    static unsigned char const data_2_2_1a[] =
        {0xD4, 0x14, 0x82, 0x02, 0x82, 0x81, 0x89, 0x05, 0xFF, 0x2A, 0xA1, 0x1A,
         0xB0, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_2_2_1a[] =
        {0x89, 0x05, 0xFF, 0x2A, 0xA1, 0x1A, 0xB0, 0x13, 0x07, 0x00, 0xF1, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_2_2_1a[] =
        {0x00, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 2.2.1A - GCF 27.22.6.2" )
        << QByteArray( (char *)data_2_2_1a, sizeof(data_2_2_1a) )
        << QByteArray( (char *)payl_2_2_1a, sizeof(payl_2_2_1a) )
        << QByteArray( (char *)resp_2_2_1a, sizeof(resp_2_2_1a) )
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    static unsigned char const data_2_2_1b[] =
        {0xD4, 0x14, 0x82, 0x02, 0x82, 0x81, 0x89, 0x05, 0xFF, 0x2A, 0xA1, 0x1A,
         0xB0, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_2_2_1b[] =
        {0x89, 0x05, 0xFF, 0x2A, 0xA1, 0x1A, 0xB0, 0x13, 0x07, 0x00, 0x11, 0x10,
         0x00, 0x01, 0x00, 0x01};
    static unsigned char const resp_2_2_1b[] =
        {0x00, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 2.2.1A - GCF 27.22.6.2" )
        << QByteArray( (char *)data_2_2_1b, sizeof(data_2_2_1b) )
        << QByteArray( (char *)payl_2_2_1b, sizeof(payl_2_2_1b) )
        << QByteArray( (char *)resp_2_2_1b, sizeof(resp_2_2_1b) )
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    static unsigned char const data_2_3_1a[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x89, 0x03, 0xFF, 0x2A, 0xB1, 0x13,
         0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_2_3_1a[] =
        {0x89, 0x03, 0xFF, 0x2A, 0xB1, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_2_3_1a[] =
        {0x01, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 2.3.1A - GCF 27.22.6.2" )
        << QByteArray( (char *)data_2_3_1a, sizeof(data_2_3_1a) )
        << QByteArray( (char *)payl_2_3_1a, sizeof(payl_2_3_1a) )
        << QByteArray( (char *)resp_2_3_1a, sizeof(resp_2_3_1a) )
        << QByteArray()
        << (int)( QSimControlEvent::NotAllowed );

    static unsigned char const data_2_3_1b[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x89, 0x03, 0xFF, 0x2A, 0xB1, 0x13,
         0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_2_3_1b[] =
        {0x89, 0x03, 0xFF, 0x2A, 0xB1, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_2_3_1b[] =
        {0x01, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 2.3.1B - GCF 27.22.6.2" )
        << QByteArray( (char *)data_2_3_1b, sizeof(data_2_3_1b) )
        << QByteArray( (char *)payl_2_3_1b, sizeof(payl_2_3_1b) )
        << QByteArray( (char *)resp_2_3_1b, sizeof(resp_2_3_1b) )
        << QByteArray()
        << (int)( QSimControlEvent::NotAllowed );

    static unsigned char const data_2_4_1a[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x89, 0x03, 0xFF, 0x2A, 0xB1, 0x13,
         0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_2_4_1a[] =
        {0x89, 0x03, 0xFF, 0x2A, 0xB1, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_2_4_1a[] =
        {0x02, 0x06, 0x89, 0x04, 0xFF, 0xBA, 0x12, 0xFB};
    static unsigned char const rpay_2_4_1a[] =
        {0x89, 0x04, 0xFF, 0xBA, 0x12, 0xFB};
    QTest::newRow( "CALL CONTROL BY SIM 2.4.1A - GCF 27.22.6.2" )
        << QByteArray( (char *)data_2_4_1a, sizeof(data_2_4_1a) )
        << QByteArray( (char *)payl_2_4_1a, sizeof(payl_2_4_1a) )
        << QByteArray( (char *)resp_2_4_1a, sizeof(resp_2_4_1a) )
        << QByteArray( (char *)rpay_2_4_1a, sizeof(rpay_2_4_1a) )
        << (int)( QSimControlEvent::AllowedWithModifications );

    static unsigned char const data_2_4_1b[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x89, 0x03, 0xFF, 0x2A, 0xB1, 0x13,
         0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_2_4_1b[] =
        {0x89, 0x03, 0xFF, 0x2A, 0xB1, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_2_4_1b[] =
        {0x02, 0x06, 0x89, 0x04, 0xFF, 0xBA, 0x12, 0xFB};
    static unsigned char const rpay_2_4_1b[] =
        {0x89, 0x04, 0xFF, 0xBA, 0x12, 0xFB};
    QTest::newRow( "CALL CONTROL BY SIM 2.4.1B - GCF 27.22.6.2" )
        << QByteArray( (char *)data_2_4_1b, sizeof(data_2_4_1b) )
        << QByteArray( (char *)payl_2_4_1b, sizeof(payl_2_4_1b) )
        << QByteArray( (char *)resp_2_4_1b, sizeof(resp_2_4_1b) )
        << QByteArray( (char *)rpay_2_4_1b, sizeof(rpay_2_4_1b) )
        << (int)( QSimControlEvent::AllowedWithModifications );

    // 3.1.1 tests a call to a number not in the fixed dialing phone book.
    // No envelope is sent for that case, as the call is disallowed before
    // it ever gets to the call control mechanism.

    static unsigned char const data_3_2_1a[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x86, 0x03, 0x81, 0x21, 0xF3, 0x13,
         0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_3_2_1a[] =
        {0x86, 0x03, 0x81, 0x21, 0xF3, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01,
         0x00, 0x01};
    QTest::newRow( "CALL CONTROL BY SIM 3.2.1A - GCF 27.22.6.3" )
        << QByteArray( (char *)data_3_2_1a, sizeof(data_3_2_1a) )
        << QByteArray( (char *)payl_3_2_1a, sizeof(payl_3_2_1a) )
        << QByteArray()
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    static unsigned char const data_3_2_1b[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x86, 0x03, 0x81, 0x21, 0xF3, 0x13,
         0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_3_2_1b[] =
        {0x86, 0x03, 0x81, 0x21, 0xF3, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01,
         0x00, 0x01};
    QTest::newRow( "CALL CONTROL BY SIM 3.2.1B - GCF 27.22.6.3" )
        << QByteArray( (char *)data_3_2_1b, sizeof(data_3_2_1b) )
        << QByteArray( (char *)payl_3_2_1b, sizeof(payl_3_2_1b) )
        << QByteArray()
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    static unsigned char const data_3_3_1a[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x86, 0x03, 0x81, 0x89, 0x67, 0x13,
         0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_3_3_1a[] =
        {0x86, 0x03, 0x81, 0x89, 0x67, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_3_3_1a[] =
        {0x00, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 3.3.1A - GCF 27.22.6.3" )
        << QByteArray( (char *)data_3_3_1a, sizeof(data_3_3_1a) )
        << QByteArray( (char *)payl_3_3_1a, sizeof(payl_3_3_1a) )
        << QByteArray( (char *)resp_3_3_1a, sizeof(resp_3_3_1a) )
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    static unsigned char const data_3_3_1b[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x86, 0x03, 0x81, 0x89, 0x67, 0x13,
         0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_3_3_1b[] =
        {0x86, 0x03, 0x81, 0x89, 0x67, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_3_3_1b[] =
        {0x00, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 3.3.1B - GCF 27.22.6.3" )
        << QByteArray( (char *)data_3_3_1b, sizeof(data_3_3_1b) )
        << QByteArray( (char *)payl_3_3_1b, sizeof(payl_3_3_1b) )
        << QByteArray( (char *)resp_3_3_1b, sizeof(resp_3_3_1b) )
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    static unsigned char const data_3_4_1a[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x86, 0x03, 0x81, 0x89, 0x67, 0x13,
         0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_3_4_1a[] =
        {0x86, 0x03, 0x81, 0x89, 0x67, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_3_4_1a[] =
        {0x01, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 3.4.1A - GCF 27.22.6.3" )
        << QByteArray( (char *)data_3_4_1a, sizeof(data_3_4_1a) )
        << QByteArray( (char *)payl_3_4_1a, sizeof(payl_3_4_1a) )
        << QByteArray( (char *)resp_3_4_1a, sizeof(resp_3_4_1a) )
        << QByteArray()
        << (int)( QSimControlEvent::NotAllowed );

    static unsigned char const data_3_4_1b[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x86, 0x03, 0x81, 0x89, 0x67, 0x13,
         0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_3_4_1b[] =
        {0x86, 0x03, 0x81, 0x89, 0x67, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_3_4_1b[] =
        {0x01, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 3.4.1B - GCF 27.22.6.3" )
        << QByteArray( (char *)data_3_4_1b, sizeof(data_3_4_1b) )
        << QByteArray( (char *)payl_3_4_1b, sizeof(payl_3_4_1b) )
        << QByteArray( (char *)resp_3_4_1b, sizeof(resp_3_4_1b) )
        << QByteArray()
        << (int)( QSimControlEvent::NotAllowed );

    static unsigned char const data_3_5_1a[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x86, 0x03, 0x81, 0x89, 0x67, 0x13,
         0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_3_5_1a[] =
        {0x86, 0x03, 0x81, 0x89, 0x67, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_3_5_1a[] =
        {0x02, 0x05, 0x86, 0x03, 0x81, 0x33, 0x33};
    static unsigned char const rpay_3_5_1a[] =
        {0x86, 0x03, 0x81, 0x33, 0x33};
    QTest::newRow( "CALL CONTROL BY SIM 3.5.1A - GCF 27.22.6.3" )
        << QByteArray( (char *)data_3_5_1a, sizeof(data_3_5_1a) )
        << QByteArray( (char *)payl_3_5_1a, sizeof(payl_3_5_1a) )
        << QByteArray( (char *)resp_3_5_1a, sizeof(resp_3_5_1a) )
        << QByteArray( (char *)rpay_3_5_1a, sizeof(rpay_3_5_1a) )
        << (int)( QSimControlEvent::AllowedWithModifications );

    static unsigned char const data_3_5_1b[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x86, 0x03, 0x81, 0x89, 0x67, 0x13,
         0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_3_5_1b[] =
        {0x86, 0x03, 0x81, 0x89, 0x67, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_3_5_1b[] =
        {0x02, 0x05, 0x86, 0x03, 0x81, 0x33, 0x33};
    static unsigned char const rpay_3_5_1b[] =
        {0x86, 0x03, 0x81, 0x33, 0x33};
    QTest::newRow( "CALL CONTROL BY SIM 3.5.1B - GCF 27.22.6.3" )
        << QByteArray( (char *)data_3_5_1b, sizeof(data_3_5_1b) )
        << QByteArray( (char *)payl_3_5_1b, sizeof(payl_3_5_1b) )
        << QByteArray( (char *)resp_3_5_1b, sizeof(resp_3_5_1b) )
        << QByteArray( (char *)rpay_3_5_1b, sizeof(rpay_3_5_1b) )
        << (int)( QSimControlEvent::AllowedWithModifications );

    static unsigned char const data_4_1_1a[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x86, 0x03, 0x81, 0x23, 0xF1, 0x13,
         0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_4_1_1a[] =
        {0x86, 0x03, 0x81, 0x23, 0xF1, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_4_1_1a[] =
        {0x01, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 4.1.1A - GCF 27.22.6.4" )
        << QByteArray( (char *)data_4_1_1a, sizeof(data_4_1_1a) )
        << QByteArray( (char *)payl_4_1_1a, sizeof(payl_4_1_1a) )
        << QByteArray( (char *)resp_4_1_1a, sizeof(resp_4_1_1a) )
        << QByteArray()
        << (int)( QSimControlEvent::NotAllowed );

    static unsigned char const data_4_1_1b[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x86, 0x03, 0x81, 0x23, 0xF1, 0x13,
         0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_4_1_1b[] =
        {0x86, 0x03, 0x81, 0x23, 0xF1, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_4_1_1b[] =
        {0x01, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 4.1.1B - GCF 27.22.6.4" )
        << QByteArray( (char *)data_4_1_1b, sizeof(data_4_1_1b) )
        << QByteArray( (char *)payl_4_1_1b, sizeof(payl_4_1_1b) )
        << QByteArray( (char *)resp_4_1_1b, sizeof(resp_4_1_1b) )
        << QByteArray()
        << (int)( QSimControlEvent::NotAllowed );

    static unsigned char const data_4_2_1a[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x86, 0x03, 0x81, 0x21, 0x43, 0x13,
         0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_4_2_1a[] =
        {0x86, 0x03, 0x81, 0x21, 0x43, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_4_2_1a[] =
        {0x00, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 4.2.1A - GCF 27.22.6.4" )
        << QByteArray( (char *)data_4_2_1a, sizeof(data_4_2_1a) )
        << QByteArray( (char *)payl_4_2_1a, sizeof(payl_4_2_1a) )
        << QByteArray( (char *)resp_4_2_1a, sizeof(resp_4_2_1a) )
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    static unsigned char const data_4_2_1b[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x86, 0x03, 0x81, 0x21, 0x43, 0x13,
         0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_4_2_1b[] =
        {0x86, 0x03, 0x81, 0x21, 0x43, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_4_2_1b[] =
        {0x00, 0x00};
    QTest::newRow( "CALL CONTROL BY SIM 4.2.1B - GCF 27.22.6.4" )
        << QByteArray( (char *)data_4_2_1b, sizeof(data_4_2_1b) )
        << QByteArray( (char *)payl_4_2_1b, sizeof(payl_4_2_1b) )
        << QByteArray( (char *)resp_4_2_1b, sizeof(resp_4_2_1b) )
        << QByteArray()
        << (int)( QSimControlEvent::Allowed );

    static unsigned char const data_4_3_1a[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x86, 0x03, 0x81, 0x11, 0x11, 0x13,
         0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_4_3_1a[] =
        {0x86, 0x03, 0x81, 0x11, 0x11, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_4_3_1a[] =
        {0x02, 0x05, 0x86, 0x03, 0x81, 0x22, 0x22};
    static unsigned char const rpay_4_3_1a[] =
        {0x86, 0x03, 0x81, 0x22, 0x22};
    QTest::newRow( "CALL CONTROL BY SIM 4.3.1A - GCF 27.22.6.4" )
        << QByteArray( (char *)data_4_3_1a, sizeof(data_4_3_1a) )
        << QByteArray( (char *)payl_4_3_1a, sizeof(payl_4_3_1a) )
        << QByteArray( (char *)resp_4_3_1a, sizeof(resp_4_3_1a) )
        << QByteArray( (char *)rpay_4_3_1a, sizeof(rpay_4_3_1a) )
        << (int)( QSimControlEvent::AllowedWithModifications );

    static unsigned char const data_4_3_1b[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x86, 0x03, 0x81, 0x11, 0x11, 0x13,
         0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_4_3_1b[] =
        {0x86, 0x03, 0x81, 0x11, 0x11, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_4_3_1b[] =
        {0x02, 0x05, 0x86, 0x03, 0x81, 0x22, 0x22};
    static unsigned char const rpay_4_3_1b[] =
        {0x86, 0x03, 0x81, 0x22, 0x22};
    QTest::newRow( "CALL CONTROL BY SIM 4.3.1B - GCF 27.22.6.4" )
        << QByteArray( (char *)data_4_3_1b, sizeof(data_4_3_1b) )
        << QByteArray( (char *)payl_4_3_1b, sizeof(payl_4_3_1b) )
        << QByteArray( (char *)resp_4_3_1b, sizeof(resp_4_3_1b) )
        << QByteArray( (char *)rpay_4_3_1b, sizeof(rpay_4_3_1b) )
        << (int)( QSimControlEvent::AllowedWithModifications );

    static unsigned char const data_4_4_1a[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x86, 0x03, 0x81, 0x21, 0xF3, 0x13,
         0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_4_4_1a[] =
        {0x86, 0x03, 0x81, 0x21, 0xF3, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_4_4_1a[] =
        {0x02, 0x08, 0x86, 0x06, 0x81, 0x89, 0x67, 0x45, 0x23, 0xF1};
    static unsigned char const rpay_4_4_1a[] =
        {0x86, 0x06, 0x81, 0x89, 0x67, 0x45, 0x23, 0xF1};
    QTest::newRow( "CALL CONTROL BY SIM 4.4.1A - GCF 27.22.6.4" )
        << QByteArray( (char *)data_4_4_1a, sizeof(data_4_4_1a) )
        << QByteArray( (char *)payl_4_4_1a, sizeof(payl_4_4_1a) )
        << QByteArray( (char *)resp_4_4_1a, sizeof(resp_4_4_1a) )
        << QByteArray( (char *)rpay_4_4_1a, sizeof(rpay_4_4_1a) )
        << (int)( QSimControlEvent::AllowedWithModifications );

    static unsigned char const data_4_4_1b[] =
        {0xD4, 0x12, 0x82, 0x02, 0x82, 0x81, 0x86, 0x03, 0x81, 0x21, 0xF3, 0x13,
         0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const payl_4_4_1b[] =
        {0x86, 0x03, 0x81, 0x21, 0xF3, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01,
         0x00, 0x01};
    static unsigned char const resp_4_4_1b[] =
        {0x02, 0x08, 0x86, 0x06, 0x81, 0x89, 0x67, 0x45, 0x23, 0xF1};
    static unsigned char const rpay_4_4_1b[] =
        {0x86, 0x06, 0x81, 0x89, 0x67, 0x45, 0x23, 0xF1};
    QTest::newRow( "CALL CONTROL BY SIM 4.4.1B - GCF 27.22.6.4" )
        << QByteArray( (char *)data_4_4_1b, sizeof(data_4_4_1b) )
        << QByteArray( (char *)payl_4_4_1b, sizeof(payl_4_4_1b) )
        << QByteArray( (char *)resp_4_4_1b, sizeof(resp_4_4_1b) )
        << QByteArray( (char *)rpay_4_4_1b, sizeof(rpay_4_4_1b) )
        << (int)( QSimControlEvent::AllowedWithModifications );
}
