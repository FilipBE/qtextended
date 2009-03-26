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
// GCF test strings in GSM 51.010, section 27.22.7 - EVENT DOWNLOAD.
void tst_QSimToolkit::testEncodeEventDownload_data()
{
    QSimToolkitData::populateDataEventDownload();
}
void tst_QSimToolkit::testEncodeEventDownload()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, payload );
    QFETCH( int, event );
    QFETCH( int, sourceDevice );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the envelope PDU can be parsed correctly.
    QSimEnvelope decodedEnv = QSimEnvelope::fromPdu(data);
    QVERIFY( decodedEnv.type() == QSimEnvelope::EventDownload );
    QVERIFY( decodedEnv.sourceDevice() == (QSimCommand::Device)sourceDevice );
    QCOMPARE( (int)decodedEnv.event(), event );
    QCOMPARE( decodedEnv.extensionData(), payload );

    // Check that the original envelope PDU can be reconstructed correctly.
    QByteArray pdu = decodedEnv.toPdu();
    pdu[2] = data[2];      // Handle 0x19 vs 0x99 discrepancy.
    QCOMPARE( pdu, data );
}

// Test that EVENT DOWNLOAD envelopes can be properly sent from client applications.
void tst_QSimToolkit::testDeliverEventDownload_data()
{
    QSimToolkitData::populateDataEventDownload();
}
void tst_QSimToolkit::testDeliverEventDownload()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, payload );
    QFETCH( int, event );
    QFETCH( int, sourceDevice );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the envelope.
    QSimEnvelope env;
    env.setType( QSimEnvelope::EventDownload );
    env.setSourceDevice( (QSimCommand::Device)sourceDevice );
    env.setEvent( (QSimEnvelope::Event)event );
    env.setExtensionData( payload );
    client->sendEnvelope( env );

    // Wait for the envelope to be received.
    QVERIFY( QFutureSignal::wait( server, SIGNAL(envelopeSeen()), 100 ) );

    // Check that the envelope is what we expected to get and that we didn't
    // get any terminal responses yet.
    QCOMPARE( server->responseCount(), 0 );
    QCOMPARE( server->envelopeCount(), 1 );
    QByteArray lastenv = server->lastEnvelope();
    lastenv[2] = data[2];      // Handle 0x19 vs 0x99 discrepancy.
    QCOMPARE( lastenv, data );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for EVENT DOWNLOAD from the GCF test cases
// in GSM 51.010, section 27.22.7.
void QSimToolkitData::populateDataEventDownload()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<int>("event");
    QTest::addColumn<int>("sourceDevice");

    // Note: many of these test cases use the 0x19 tag to indicate the event
    // list, whereas others here and in the SET UP EVENT LIST tests use 0x99.
    // It isn't clear how to determine which is the correct tag to use under
    // what circumstances.  We therefore always generate 0x99 in qsimenvelope.cpp,
    // and then normalize the test cases to match.

    static unsigned char const mtcall_data_1_1_1[] =
        {0xD6, 0x0A, 0x19, 0x01, 0x00, 0x82, 0x02, 0x83, 0x81, 0x1C, 0x01, 0x00};
    static unsigned char const mtcall_payl_1_1_1[] =
        {0x1C, 0x01, 0x00};
    QTest::newRow( "MT CALL 1.1.1 - GCF 27.22.7.1" )
        << QByteArray( (char *)mtcall_data_1_1_1, sizeof(mtcall_data_1_1_1) )
        << QByteArray( (char *)mtcall_payl_1_1_1, sizeof(mtcall_payl_1_1_1) )
        << (int)( QSimEnvelope::MTCall )
        << (int)( QSimCommand::Network );

    static unsigned char const mtcall_data_1_1_2[] =
        {0xD6, 0x0F, 0x19, 0x01, 0x00, 0x82, 0x02, 0x83, 0x81, 0x1C, 0x01, 0x00,
         0x86, 0x03, 0x81, 0x89, 0x67};
    static unsigned char const mtcall_payl_1_1_2[] =
        {0x1C, 0x01, 0x00, 0x86, 0x03, 0x81, 0x89, 0x67};
    QTest::newRow( "MT CALL 1.1.2 - GCF 27.22.7.1" )
        << QByteArray( (char *)mtcall_data_1_1_2, sizeof(mtcall_data_1_1_2) )
        << QByteArray( (char *)mtcall_payl_1_1_2, sizeof(mtcall_payl_1_1_2) )
        << (int)( QSimEnvelope::MTCall )
        << (int)( QSimCommand::Network );

    static unsigned char const conn_data_1_1_1[] =
        {0xD6, 0x0A, 0x19, 0x01, 0x01, 0x82, 0x02, 0x82, 0x81, 0x1C, 0x01, 0x80};
    static unsigned char const conn_payl_1_1_1[] =
        {0x1C, 0x01, 0x80};
    QTest::newRow( "CALL CONNECTED 1.1.1 - GCF 27.22.7.2.1" )
        << QByteArray( (char *)conn_data_1_1_1, sizeof(conn_data_1_1_1) )
        << QByteArray( (char *)conn_payl_1_1_1, sizeof(conn_payl_1_1_1) )
        << (int)( QSimEnvelope::CallConnected )
        << (int)( QSimCommand::ME );

    static unsigned char const conn_data_1_1_2[] =
        {0xD6, 0x0A, 0x19, 0x01, 0x01, 0x82, 0x02, 0x83, 0x81, 0x1C, 0x01, 0x80};
    static unsigned char const conn_payl_1_1_2[] =
        {0x1C, 0x01, 0x80};
    QTest::newRow( "CALL CONNECTED 1.1.2 - GCF 27.22.7.2.1" )
        << QByteArray( (char *)conn_data_1_1_2, sizeof(conn_data_1_1_2) )
        << QByteArray( (char *)conn_payl_1_1_2, sizeof(conn_payl_1_1_2) )
        << (int)( QSimEnvelope::CallConnected )
        << (int)( QSimCommand::Network );

    static unsigned char const conn_data_2_1_1[] =
        {0xD6, 0x0A, 0x19, 0x01, 0x01, 0x82, 0x02, 0x83, 0x81, 0x1C, 0x01, 0x80};
    static unsigned char const conn_payl_2_1_1[] =
        {0x1C, 0x01, 0x80};
    QTest::newRow( "CALL CONNECTED 2.1.1 - GCF 27.22.7.2.2" )
        << QByteArray( (char *)conn_data_2_1_1, sizeof(conn_data_2_1_1) )
        << QByteArray( (char *)conn_payl_2_1_1, sizeof(conn_payl_2_1_1) )
        << (int)( QSimEnvelope::CallConnected )
        << (int)( QSimCommand::Network );

    static unsigned char const disc_data_1_1_1[] =
        {0xD6, 0x0A, 0x19, 0x01, 0x02, 0x82, 0x02, 0x83, 0x81, 0x1C, 0x01, 0x00};
    static unsigned char const disc_payl_1_1_1[] =
        {0x1C, 0x01, 0x00};
    QTest::newRow( "CALL DISCONNECTED 1.1.1 - GCF 27.22.7.3" )
        << QByteArray( (char *)disc_data_1_1_1, sizeof(disc_data_1_1_1) )
        << QByteArray( (char *)disc_payl_1_1_1, sizeof(disc_payl_1_1_1) )
        << (int)( QSimEnvelope::CallDisconnected )
        << (int)( QSimCommand::Network );

    static unsigned char const disc_data_1_1_2a[] =
        {0xD6, 0x0A, 0x19, 0x01, 0x02, 0x82, 0x02, 0x82, 0x81, 0x1C, 0x01, 0x80};
    static unsigned char const disc_payl_1_1_2a[] =
        {0x1C, 0x01, 0x80};
    QTest::newRow( "CALL DISCONNECTED 1.1.2A - GCF 27.22.7.3" )
        << QByteArray( (char *)disc_data_1_1_2a, sizeof(disc_data_1_1_2a) )
        << QByteArray( (char *)disc_payl_1_1_2a, sizeof(disc_payl_1_1_2a) )
        << (int)( QSimEnvelope::CallDisconnected )
        << (int)( QSimCommand::ME );

    static unsigned char const disc_data_1_1_2b[] =
        {0xD6, 0x0E, 0x19, 0x01, 0x02, 0x82, 0x02, 0x82, 0x81, 0x1C, 0x01, 0x80,
         0x9A, 0x02, 0x60, 0x90};
    static unsigned char const disc_payl_1_1_2b[] =
        {0x1C, 0x01, 0x80, 0x9A, 0x02, 0x60, 0x90};
    QTest::newRow( "CALL DISCONNECTED 1.1.2B - GCF 27.22.7.3" )
        << QByteArray( (char *)disc_data_1_1_2b, sizeof(disc_data_1_1_2b) )
        << QByteArray( (char *)disc_payl_1_1_2b, sizeof(disc_payl_1_1_2b) )
        << (int)( QSimEnvelope::CallDisconnected )
        << (int)( QSimCommand::ME );

    static unsigned char const disc_data_1_1_2c[] =
        {0xD6, 0x0E, 0x19, 0x01, 0x02, 0x82, 0x02, 0x82, 0x81, 0x1C, 0x01, 0x80,
         0x9A, 0x02, 0xE0, 0x90};
    static unsigned char const disc_payl_1_1_2c[] =
        {0x1C, 0x01, 0x80, 0x9A, 0x02, 0xE0, 0x90};
    QTest::newRow( "CALL DISCONNECTED 1.1.2C - GCF 27.22.7.3" )
        << QByteArray( (char *)disc_data_1_1_2c, sizeof(disc_data_1_1_2c) )
        << QByteArray( (char *)disc_payl_1_1_2c, sizeof(disc_payl_1_1_2c) )
        << (int)( QSimEnvelope::CallDisconnected )
        << (int)( QSimCommand::ME );

    static unsigned char const disc_data_1_1_3a[] =
        {0xD6, 0x0E, 0x19, 0x01, 0x02, 0x82, 0x02, 0x83, 0x81, 0x1C, 0x01, 0x00,
         0x9A, 0x02, 0x60, 0x90};
    static unsigned char const disc_payl_1_1_3a[] =
        {0x1C, 0x01, 0x00, 0x9A, 0x02, 0x60, 0x90};
    QTest::newRow( "CALL DISCONNECTED 1.1.3A - GCF 27.22.7.3" )
        << QByteArray( (char *)disc_data_1_1_3a, sizeof(disc_data_1_1_3a) )
        << QByteArray( (char *)disc_payl_1_1_3a, sizeof(disc_payl_1_1_3a) )
        << (int)( QSimEnvelope::CallDisconnected )
        << (int)( QSimCommand::Network );

    static unsigned char const disc_data_1_1_3b[] =
        {0xD6, 0x0E, 0x19, 0x01, 0x02, 0x82, 0x02, 0x83, 0x81, 0x1C, 0x01, 0x00,
         0x9A, 0x02, 0xE0, 0x90};
    static unsigned char const disc_payl_1_1_3b[] =
        {0x1C, 0x01, 0x00, 0x9A, 0x02, 0xE0, 0x90};
    QTest::newRow( "CALL DISCONNECTED 1.1.3B - GCF 27.22.7.3" )
        << QByteArray( (char *)disc_data_1_1_3b, sizeof(disc_data_1_1_3b) )
        << QByteArray( (char *)disc_payl_1_1_3b, sizeof(disc_payl_1_1_3b) )
        << (int)( QSimEnvelope::CallDisconnected )
        << (int)( QSimCommand::Network );

    static unsigned char const disc_data_1_1_4a[] =
        {0xD6, 0x0C, 0x19, 0x01, 0x02, 0x82, 0x02, 0x82, 0x81, 0x1C, 0x01, 0x80,
         0x9A, 0x00};
    static unsigned char const disc_payl_1_1_4a[] =
        {0x1C, 0x01, 0x80, 0x9A, 0x00};
    QTest::newRow( "CALL DISCONNECTED 1.1.4A - GCF 27.22.7.3" )
        << QByteArray( (char *)disc_data_1_1_4a, sizeof(disc_data_1_1_4a) )
        << QByteArray( (char *)disc_payl_1_1_4a, sizeof(disc_payl_1_1_4a) )
        << (int)( QSimEnvelope::CallDisconnected )
        << (int)( QSimCommand::ME );

    static unsigned char const disc_data_1_1_4b[] =
        {0xD6, 0x0C, 0x19, 0x01, 0x02, 0x82, 0x02, 0x82, 0x81, 0x1C, 0x01, 0x00,
         0x9A, 0x00};
    static unsigned char const disc_payl_1_1_4b[] =
        {0x1C, 0x01, 0x00, 0x9A, 0x00};
    QTest::newRow( "CALL DISCONNECTED 1.1.4B - GCF 27.22.7.3" )
        << QByteArray( (char *)disc_data_1_1_4b, sizeof(disc_data_1_1_4b) )
        << QByteArray( (char *)disc_payl_1_1_4b, sizeof(disc_payl_1_1_4b) )
        << (int)( QSimEnvelope::CallDisconnected )
        << (int)( QSimCommand::ME );

    static unsigned char const locs_data_1_1_1[] =
        {0xD6, 0x0A, 0x19, 0x01, 0x03, 0x82, 0x02, 0x82, 0x81, 0x1B, 0x01, 0x02};
    static unsigned char const locs_payl_1_1_1[] =
        {0x1B, 0x01, 0x02};
    QTest::newRow( "LOCATION STATUS 1.1.1 - GCF 27.22.7.4" )
        << QByteArray( (char *)locs_data_1_1_1, sizeof(locs_data_1_1_1) )
        << QByteArray( (char *)locs_payl_1_1_1, sizeof(locs_payl_1_1_1) )
        << (int)( QSimEnvelope::LocationStatus )
        << (int)( QSimCommand::ME );

    static unsigned char const locs_data_1_1_2a[] =
        {0xD6, 0x13, 0x19, 0x01, 0x03, 0x82, 0x02, 0x82, 0x81, 0x1B, 0x01, 0x00,
         0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x02, 0x00, 0x02};
    static unsigned char const locs_payl_1_1_2a[] =
        {0x1B, 0x01, 0x00, 0x13, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x02, 0x00, 0x02};
    QTest::newRow( "LOCATION STATUS 1.1.2A - GCF 27.22.7.4" )
        << QByteArray( (char *)locs_data_1_1_2a, sizeof(locs_data_1_1_2a) )
        << QByteArray( (char *)locs_payl_1_1_2a, sizeof(locs_payl_1_1_2a) )
        << (int)( QSimEnvelope::LocationStatus )
        << (int)( QSimCommand::ME );

    static unsigned char const locs_data_1_1_2b[] =
        {0xD6, 0x13, 0x19, 0x01, 0x03, 0x82, 0x02, 0x82, 0x81, 0x1B, 0x01, 0x00,
         0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x02, 0x00, 0x02};
    static unsigned char const locs_payl_1_1_2b[] =
        {0x1B, 0x01, 0x00, 0x13, 0x07, 0x00, 0x11, 0x10, 0x00, 0x02, 0x00, 0x02};
    QTest::newRow( "LOCATION STATUS 1.1.2B - GCF 27.22.7.4" )
        << QByteArray( (char *)locs_data_1_1_2b, sizeof(locs_data_1_1_2b) )
        << QByteArray( (char *)locs_payl_1_1_2b, sizeof(locs_payl_1_1_2b) )
        << (int)( QSimEnvelope::LocationStatus )
        << (int)( QSimCommand::ME );

    static unsigned char const user_data_1_1_1[] =
        {0xD6, 0x07, 0x19, 0x01, 0x04, 0x82, 0x02, 0x82, 0x81};
    QTest::newRow( "USER ACTIVITY 1.1.1 - GCF 27.22.7.5" )
        << QByteArray( (char *)user_data_1_1_1, sizeof(user_data_1_1_1) )
        << QByteArray()
        << (int)( QSimEnvelope::UserActivity )
        << (int)( QSimCommand::ME );

    static unsigned char const idle_data_1_1_1[] =
        {0xD6, 0x07, 0x19, 0x01, 0x05, 0x82, 0x02, 0x02, 0x81};
    QTest::newRow( "IDLE SCREEN 1.1.1 - GCF 27.22.7.6" )
        << QByteArray( (char *)idle_data_1_1_1, sizeof(idle_data_1_1_1) )
        << QByteArray()
        << (int)( QSimEnvelope::IdleScreenAvailable )
        << (int)( QSimCommand::Display );

    static unsigned char const card_data_1_1_1a[] =
        {0xD6, 0x0A, 0x99, 0x01, 0x06, 0x82, 0x02, 0x82, 0x81, 0xA0, 0x01, 0x79};
    static unsigned char const card_payl_1_1_1a[] =
        {0xA0, 0x01, 0x79};
    QTest::newRow( "CARD READER STATUS 1.1.1A - GCF 27.22.7.7.1" )
        << QByteArray( (char *)card_data_1_1_1a, sizeof(card_data_1_1_1a) )
        << QByteArray( (char *)card_payl_1_1_1a, sizeof(card_payl_1_1_1a) )
        << (int)( QSimEnvelope::CardReaderStatus )
        << (int)( QSimCommand::ME );

    static unsigned char const card_data_1_1_1b[] =
        {0xD6, 0x0A, 0x99, 0x01, 0x06, 0x82, 0x02, 0x82, 0x81, 0xA0, 0x01, 0x59};
    static unsigned char const card_payl_1_1_1b[] =
        {0xA0, 0x01, 0x59};
    QTest::newRow( "CARD READER STATUS 1.1.1B - GCF 27.22.7.7.1" )
        << QByteArray( (char *)card_data_1_1_1b, sizeof(card_data_1_1_1b) )
        << QByteArray( (char *)card_payl_1_1_1b, sizeof(card_payl_1_1_1b) )
        << (int)( QSimEnvelope::CardReaderStatus )
        << (int)( QSimCommand::ME );

    static unsigned char const card_data_1_1_1c[] =
        {0xD6, 0x0A, 0x99, 0x01, 0x06, 0x82, 0x02, 0x82, 0x81, 0xA0, 0x01, 0x71};
    static unsigned char const card_payl_1_1_1c[] =
        {0xA0, 0x01, 0x71};
    QTest::newRow( "CARD READER STATUS 1.1.1C - GCF 27.22.7.7.1" )
        << QByteArray( (char *)card_data_1_1_1c, sizeof(card_data_1_1_1c) )
        << QByteArray( (char *)card_payl_1_1_1c, sizeof(card_payl_1_1_1c) )
        << (int)( QSimEnvelope::CardReaderStatus )
        << (int)( QSimCommand::ME );

    static unsigned char const card_data_1_1_1d[] =
        {0xD6, 0x0A, 0x99, 0x01, 0x06, 0x82, 0x02, 0x82, 0x81, 0xA0, 0x01, 0x51};
    static unsigned char const card_payl_1_1_1d[] =
        {0xA0, 0x01, 0x51};
    QTest::newRow( "CARD READER STATUS 1.1.1D - GCF 27.22.7.7.1" )
        << QByteArray( (char *)card_data_1_1_1d, sizeof(card_data_1_1_1d) )
        << QByteArray( (char *)card_payl_1_1_1d, sizeof(card_payl_1_1_1d) )
        << (int)( QSimEnvelope::CardReaderStatus )
        << (int)( QSimCommand::ME );

    static unsigned char const card_data_1_1_2a[] =
        {0xD6, 0x0A, 0x99, 0x01, 0x06, 0x82, 0x02, 0x82, 0x81, 0xA0, 0x01, 0x39};
    static unsigned char const card_payl_1_1_2a[] =
        {0xA0, 0x01, 0x39};
    QTest::newRow( "CARD READER STATUS 1.1.2A - GCF 27.22.7.7.1" )
        << QByteArray( (char *)card_data_1_1_2a, sizeof(card_data_1_1_2a) )
        << QByteArray( (char *)card_payl_1_1_2a, sizeof(card_payl_1_1_2a) )
        << (int)( QSimEnvelope::CardReaderStatus )
        << (int)( QSimCommand::ME );

    static unsigned char const card_data_1_1_2b[] =
        {0xD6, 0x0A, 0x99, 0x01, 0x06, 0x82, 0x02, 0x82, 0x81, 0xA0, 0x01, 0x19};
    static unsigned char const card_payl_1_1_2b[] =
        {0xA0, 0x01, 0x19};
    QTest::newRow( "CARD READER STATUS 1.1.2B - GCF 27.22.7.7.1" )
        << QByteArray( (char *)card_data_1_1_2b, sizeof(card_data_1_1_2b) )
        << QByteArray( (char *)card_payl_1_1_2b, sizeof(card_payl_1_1_2b) )
        << (int)( QSimEnvelope::CardReaderStatus )
        << (int)( QSimCommand::ME );

    static unsigned char const card_data_1_1_2c[] =
        {0xD6, 0x0A, 0x99, 0x01, 0x06, 0x82, 0x02, 0x82, 0x81, 0xA0, 0x01, 0x31};
    static unsigned char const card_payl_1_1_2c[] =
        {0xA0, 0x01, 0x31};
    QTest::newRow( "CARD READER STATUS 1.1.2C - GCF 27.22.7.7.1" )
        << QByteArray( (char *)card_data_1_1_2c, sizeof(card_data_1_1_2c) )
        << QByteArray( (char *)card_payl_1_1_2c, sizeof(card_payl_1_1_2c) )
        << (int)( QSimEnvelope::CardReaderStatus )
        << (int)( QSimCommand::ME );

    static unsigned char const card_data_1_1_2d[] =
        {0xD6, 0x0A, 0x99, 0x01, 0x06, 0x82, 0x02, 0x82, 0x81, 0xA0, 0x01, 0x11};
    static unsigned char const card_payl_1_1_2d[] =
        {0xA0, 0x01, 0x11};
    QTest::newRow( "CARD READER STATUS 1.1.2D - GCF 27.22.7.7.1" )
        << QByteArray( (char *)card_data_1_1_2d, sizeof(card_data_1_1_2d) )
        << QByteArray( (char *)card_payl_1_1_2d, sizeof(card_payl_1_1_2d) )
        << (int)( QSimEnvelope::CardReaderStatus )
        << (int)( QSimCommand::ME );

    static unsigned char const card_data_2_1_1a[] =
        {0xD6, 0x0A, 0x99, 0x01, 0x06, 0x82, 0x02, 0x82, 0x81, 0xA0, 0x01, 0x39};
    static unsigned char const card_payl_2_1_1a[] =
        {0xA0, 0x01, 0x39};
    QTest::newRow( "CARD READER STATUS 2.1.1A - GCF 27.22.7.7.2" )
        << QByteArray( (char *)card_data_2_1_1a, sizeof(card_data_2_1_1a) )
        << QByteArray( (char *)card_payl_2_1_1a, sizeof(card_payl_2_1_1a) )
        << (int)( QSimEnvelope::CardReaderStatus )
        << (int)( QSimCommand::ME );

    static unsigned char const card_data_2_1_1b[] =
        {0xD6, 0x0A, 0x99, 0x01, 0x06, 0x82, 0x02, 0x82, 0x81, 0xA0, 0x01, 0x19};
    static unsigned char const card_payl_2_1_1b[] =
        {0xA0, 0x01, 0x19};
    QTest::newRow( "CARD READER STATUS 2.1.1B - GCF 27.22.7.7.2" )
        << QByteArray( (char *)card_data_2_1_1b, sizeof(card_data_2_1_1b) )
        << QByteArray( (char *)card_payl_2_1_1b, sizeof(card_payl_2_1_1b) )
        << (int)( QSimEnvelope::CardReaderStatus )
        << (int)( QSimCommand::ME );

    static unsigned char const card_data_2_1_2a[] =
        {0xD6, 0x0A, 0x99, 0x01, 0x06, 0x82, 0x02, 0x82, 0x81, 0xA0, 0x01, 0x29};
    static unsigned char const card_payl_2_1_2a[] =
        {0xA0, 0x01, 0x29};
    QTest::newRow( "CARD READER STATUS 2.1.2A - GCF 27.22.7.7.2" )
        << QByteArray( (char *)card_data_2_1_2a, sizeof(card_data_2_1_2a) )
        << QByteArray( (char *)card_payl_2_1_2a, sizeof(card_payl_2_1_2a) )
        << (int)( QSimEnvelope::CardReaderStatus )
        << (int)( QSimCommand::ME );

    static unsigned char const card_data_2_1_2b[] =
        {0xD6, 0x0A, 0x99, 0x01, 0x06, 0x82, 0x02, 0x82, 0x81, 0xA0, 0x01, 0x09};
    static unsigned char const card_payl_2_1_2b[] =
        {0xA0, 0x01, 0x09};
    QTest::newRow( "CARD READER STATUS 2.1.2B - GCF 27.22.7.7.2" )
        << QByteArray( (char *)card_data_2_1_2b, sizeof(card_data_2_1_2b) )
        << QByteArray( (char *)card_payl_2_1_2b, sizeof(card_payl_2_1_2b) )
        << (int)( QSimEnvelope::CardReaderStatus )
        << (int)( QSimCommand::ME );

    static unsigned char const lang_data_1_1_1[] =
        {0xD6, 0x0B, 0x19, 0x01, 0x07, 0x82, 0x02, 0x82, 0x81, 0x2D, 0x02, 0x64,
         0x65};
    static unsigned char const lang_payl_1_1_1[] =
        {0x2D, 0x02, 0x64, 0x65};
    QTest::newRow( "LANGUAGE SELECTION 1.1.1 - GCF 27.22.7.8" )
        << QByteArray( (char *)lang_data_1_1_1, sizeof(lang_data_1_1_1) )
        << QByteArray( (char *)lang_payl_1_1_1, sizeof(lang_payl_1_1_1) )
        << (int)( QSimEnvelope::LanguageSelection )
        << (int)( QSimCommand::ME );

    static unsigned char const brow_data_1_1_1[] =
        {0xD6, 0x0A, 0x99, 0x01, 0x08, 0x82, 0x02, 0x82, 0x81, 0xB4, 0x01, 0x00};
    static unsigned char const brow_payl_1_1_1[] =
        {0xB4, 0x01, 0x00};
    QTest::newRow( "BROWSER TERMINATION 1.1.1 - GCF 27.22.7.9" )
        << QByteArray( (char *)brow_data_1_1_1, sizeof(brow_data_1_1_1) )
        << QByteArray( (char *)brow_payl_1_1_1, sizeof(brow_payl_1_1_1) )
        << (int)( QSimEnvelope::BrowserTermination )
        << (int)( QSimCommand::ME );

    static unsigned char const aval_data_1_1_1[] =
        {0xD6, 0x0E, 0x99, 0x01, 0x09, 0x82, 0x02, 0x82, 0x81, 0xB8, 0x02, 0x81,
         0x00, 0xB7, 0x01, 0x08};
    static unsigned char const aval_payl_1_1_1[] =
        {0xB8, 0x02, 0x81, 0x00, 0xB7, 0x01, 0x08};
    QTest::newRow( "DATA AVAILABLE 1.1.1 - GCF 27.22.7.10" )
        << QByteArray( (char *)aval_data_1_1_1, sizeof(aval_data_1_1_1) )
        << QByteArray( (char *)aval_payl_1_1_1, sizeof(aval_payl_1_1_1) )
        << (int)( QSimEnvelope::DataAvailable )
        << (int)( QSimCommand::ME );

    static unsigned char const chst_data_1_1_1[] =
        {0xD6, 0x0B, 0x99, 0x01, 0x0A, 0x82, 0x02, 0x82, 0x81, 0xB8, 0x02, 0x01,
         0x05};
    static unsigned char const chst_payl_1_1_1[] =
        {0xB8, 0x02, 0x01, 0x05};
    QTest::newRow( "CHANNEL STATUS 1.1.1 - GCF 27.22.7.11" )
        << QByteArray( (char *)chst_data_1_1_1, sizeof(chst_data_1_1_1) )
        << QByteArray( (char *)chst_payl_1_1_1, sizeof(chst_payl_1_1_1) )
        << (int)( QSimEnvelope::ChannelStatus )
        << (int)( QSimCommand::ME );
}
