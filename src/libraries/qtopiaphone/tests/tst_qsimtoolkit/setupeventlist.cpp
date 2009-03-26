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
// Test encoding and decoding of SET UP EVENT LIST commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.16 - SET UP EVENT LIST.
void tst_QSimToolkit::testEncodeSetupEventList_data()
{
    QSimToolkitData::populateDataSetupEventList();
}
void tst_QSimToolkit::testEncodeSetupEventList()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, events );
    QFETCH( QByteArray, envelope );
    QFETCH( QByteArray, extData );
    QFETCH( int, resptype );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::SetupEventList );
    QVERIFY( decoded.destinationDevice() == QSimCommand::ME );
    QCOMPARE( decoded.extensionField(0x99), events );

    // Check that the original command PDU can be reconstructed correctly.
    QByteArray encoded = decoded.toPdu( (QSimCommand::ToPduOptions)options );
    QCOMPARE( encoded, data );

    // Check that the terminal response PDU can be parsed correctly.
    QSimTerminalResponse decodedResp = QSimTerminalResponse::fromPdu(resp);
    QVERIFY( data.contains( decodedResp.commandPdu() ) );
    if ( resptype < 0x0100 ) {
        QVERIFY( decodedResp.result() == (QSimTerminalResponse::Result)resptype );
        QVERIFY( decodedResp.causeData().isEmpty() );
        QVERIFY( decodedResp.cause() == QSimTerminalResponse::NoSpecificCause );
    } else {
        QVERIFY( decodedResp.result() == (QSimTerminalResponse::Result)(resptype >> 8) );
        QVERIFY( decodedResp.causeData().size() == 1 );
        QVERIFY( decodedResp.cause() == (QSimTerminalResponse::Cause)(resptype & 0xFF) );
    }

    // Check that the original terminal response PDU can be reconstructed correctly.
    QCOMPARE( decodedResp.toPdu(), resp );

    // Bail out if no envelope tests to perform.
    if ( envelope.isEmpty() )
        return;

    // Check that the envelope PDU can be parsed correctly.
    QSimEnvelope env = QSimEnvelope::fromPdu(envelope);
    QVERIFY( env.type() == QSimEnvelope::EventDownload );
    if ( events.contains( (char)0x02 ) )
        QVERIFY( env.sourceDevice() == QSimCommand::Network );
    else
        QVERIFY( env.sourceDevice() == QSimCommand::ME );
    QCOMPARE( (int)env.event(), events[0] & 0xFF );
    QCOMPARE( env.extensionData(), extData );

    // Check that the original envelope PDU can be reconstructed correctly.
    QCOMPARE( env.toPdu(), envelope );
}

// Test that SET UP EVENT LIST commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverSetupEventList_data()
{
    QSimToolkitData::populateDataSetupEventList();
}
void tst_QSimToolkit::testDeliverSetupEventList()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, events );
    QFETCH( QByteArray, envelope );
    QFETCH( QByteArray, extData );
    QFETCH( int, resptype );
    QFETCH( int, options );

    Q_UNUSED(resptype);

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::SetupEventList );
    cmd.addExtensionField( 0x99, events );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QVERIFY( deliveredCommand.extensionData() == cmd.extensionData() );
    QCOMPARE( deliveredCommand.toPdu( (QSimCommand::ToPduOptions)options ), data );

    // The response should have been sent immediately.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    QCOMPARE( server->lastResponse(), resp );

    // Bail out if no envelope in the test data.
    if ( envelope.isEmpty() )
        return;

    // Compose and send the envelope.
    QSimEnvelope env;
    env.setType( QSimEnvelope::EventDownload );
    if ( events.contains( (char)0x02 ) )
        env.setSourceDevice( QSimCommand::Network );
    env.setEvent( (QSimEnvelope::Event)(events[0] & 0xFF) );
    env.setExtensionData( extData );
    client->sendEnvelope( env );

    // Wait for the envelope to be received.
    QVERIFY( QFutureSignal::wait( server, SIGNAL(envelopeSeen()), 100 ) );

    // Check that the envelope is what we expected to get and that we didn't
    // get any further terminal responses.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 1 );
    QCOMPARE( server->lastEnvelope(), envelope );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for SET UP EVENT LIST from the GCF test cases
// in GSM 51.010, section 27.22.4.16.
void QSimToolkitData::populateDataSetupEventList()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<QByteArray>("events");
    QTest::addColumn<QByteArray>("envelope");
    QTest::addColumn<QByteArray>("extData");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x05, 0x00, 0x82, 0x02, 0x81, 0x82, 0x99,
         0x01, 0x01};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x05, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const evnt_1_1_1[] =
        {0x01};
    static unsigned char const enve_1_1_1[] =
        {0xD6, 0x0A, 0x99, 0x01, 0x01, 0x82, 0x02, 0x82, 0x81, 0x9C, 0x01, 0x80};
    static unsigned char const extd_1_1_1[] =
        {0x9C, 0x01, 0x80};
    QTest::newRow( "SET UP EVENT LIST 1.1.1 - GCF 27.22.4.16" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << QByteArray( (char *)evnt_1_1_1, sizeof(evnt_1_1_1) )
        << QByteArray( (char *)enve_1_1_1, sizeof(enve_1_1_1) )
        << QByteArray( (char *)extd_1_1_1, sizeof(extd_1_1_1) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x0D, 0x81, 0x03, 0x01, 0x05, 0x00, 0x82, 0x02, 0x81, 0x82, 0x99,
         0x02, 0x01, 0x02};
    static unsigned char const resp_1_2_1[] =
        {0x81, 0x03, 0x01, 0x05, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const evnt_1_2_1[] =
        {0x01, 0x02};
    QTest::newRow( "SET UP EVENT LIST 1.2.1 - GCF 27.22.4.16" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << QByteArray( (char *)evnt_1_2_1, sizeof(evnt_1_2_1) )
        << QByteArray()
        << QByteArray()
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_2a[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x05, 0x00, 0x82, 0x02, 0x81, 0x82, 0x99,
         0x01, 0x02};
    static unsigned char const resp_1_2_2a[] =
        {0x81, 0x03, 0x01, 0x05, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const evnt_1_2_2a[] =
        {0x02};
    static unsigned char const enve_1_2_2a[] =
        {0xD6, 0x0E, 0x99, 0x01, 0x02, 0x82, 0x02, 0x83, 0x81, 0x9C, 0x01, 0x00,
         0x9A, 0x02, 0x60, 0x90};
    static unsigned char const extd_1_2_2a[] =
        {0x9C, 0x01, 0x00, 0x9A, 0x02, 0x60, 0x90};
    QTest::newRow( "SET UP EVENT LIST 1.2.2A - GCF 27.22.4.16" )
        << QByteArray( (char *)data_1_2_2a, sizeof(data_1_2_2a) )
        << QByteArray( (char *)resp_1_2_2a, sizeof(resp_1_2_2a) )
        << QByteArray( (char *)evnt_1_2_2a, sizeof(evnt_1_2_2a) )
        << QByteArray( (char *)enve_1_2_2a, sizeof(enve_1_2_2a) )
        << QByteArray( (char *)extd_1_2_2a, sizeof(extd_1_2_2a) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_2b[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x05, 0x00, 0x82, 0x02, 0x81, 0x82, 0x99,
         0x01, 0x02};
    static unsigned char const resp_1_2_2b[] =
        {0x81, 0x03, 0x01, 0x05, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const evnt_1_2_2b[] =
        {0x02};
    static unsigned char const enve_1_2_2b[] =
        {0xD6, 0x0E, 0x99, 0x01, 0x02, 0x82, 0x02, 0x83, 0x81, 0x9C, 0x01, 0x00,
         0x9A, 0x02, 0xE0, 0x90};
    static unsigned char const extd_1_2_2b[] =
        {0x9C, 0x01, 0x00, 0x9A, 0x02, 0xE0, 0x90};
    QTest::newRow( "SET UP EVENT LIST 1.2.2B - GCF 27.22.4.16" )
        << QByteArray( (char *)data_1_2_2b, sizeof(data_1_2_2b) )
        << QByteArray( (char *)resp_1_2_2b, sizeof(resp_1_2_2b) )
        << QByteArray( (char *)evnt_1_2_2b, sizeof(evnt_1_2_2b) )
        << QByteArray( (char *)enve_1_2_2b, sizeof(enve_1_2_2b) )
        << QByteArray( (char *)extd_1_2_2b, sizeof(extd_1_2_2b) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x05, 0x00, 0x82, 0x02, 0x81, 0x82, 0x99,
         0x01, 0x01};
    static unsigned char const resp_1_3_1[] =
        {0x81, 0x03, 0x01, 0x05, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const evnt_1_3_1[] =
        {0x01};
    QTest::newRow( "SET UP EVENT LIST 1.3.1 - GCF 27.22.4.16" )
        << QByteArray( (char *)data_1_3_1, sizeof(data_1_3_1) )
        << QByteArray( (char *)resp_1_3_1, sizeof(resp_1_3_1) )
        << QByteArray( (char *)evnt_1_3_1, sizeof(evnt_1_3_1) )
        << QByteArray()
        << QByteArray()
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_2[] =
        {0xD0, 0x0B, 0x81, 0x03, 0x01, 0x05, 0x00, 0x82, 0x02, 0x81, 0x82, 0x99,
         0x00};
    static unsigned char const resp_1_3_2[] =
        {0x81, 0x03, 0x01, 0x05, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SET UP EVENT LIST 1.3.2 - GCF 27.22.4.16" )
        << QByteArray( (char *)data_1_3_2, sizeof(data_1_3_2) )
        << QByteArray( (char *)resp_1_3_2, sizeof(resp_1_3_2) )
        << QByteArray()
        << QByteArray()
        << QByteArray()
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );
}
