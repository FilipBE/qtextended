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
// Test encoding and decoding of GET CHANNEL STATUS commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.31 - GET CHANNEL STATUS.
void tst_QSimToolkit::testEncodeGetChannelStatus_data()
{
    QSimToolkitData::populateDataGetChannelStatus();
}
void tst_QSimToolkit::testEncodeGetChannelStatus()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, status );
    QFETCH( QByteArray, envelope );
    QFETCH( int, resptype );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::GetChannelStatus );
    QVERIFY( decoded.destinationDevice() == QSimCommand::ME );

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
    QCOMPARE( decodedResp.extensionData(), status );

    // Check that the original terminal response PDU can be reconstructed correctly.
    QCOMPARE( decodedResp.toPdu(), resp );

    // Bail out if no envelope to be tested.
    if ( envelope.isEmpty() )
        return;

    // Check that the envelope PDU can be parsed correctly.
    QSimEnvelope decodedEnv = QSimEnvelope::fromPdu(envelope);
    QVERIFY( decodedEnv.type() == QSimEnvelope::EventDownload );
    QVERIFY( decodedEnv.event() == QSimEnvelope::ChannelStatus );
    QCOMPARE( decodedEnv.extensionData(), status );

    // Check that the original envelope PDU can be reconstructed correctly.
    QCOMPARE( decodedEnv.toPdu(), envelope );
}

// Test that GET CHANNEL STATUS commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverGetChannelStatus_data()
{
    QSimToolkitData::populateDataGetChannelStatus();
}
void tst_QSimToolkit::testDeliverGetChannelStatus()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, status );
    QFETCH( QByteArray, envelope );
    QFETCH( int, resptype );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Send the envelope to notify the SIM that there is data available.
    if ( !envelope.isEmpty() ) {
        // Compose and send the envelope.
        QSimEnvelope env;
        env.setType( QSimEnvelope::EventDownload );
        env.setEvent( QSimEnvelope::ChannelStatus );
        env.setExtensionData( status );
        client->sendEnvelope( env );

        // Wait for the envelope to be received.
        QVERIFY( QFutureSignal::wait( server, SIGNAL(envelopeSeen()), 100 ) );

        // Check that the envelope is what we expected to get and that we didn't
        // get any terminal responses yet.
        QCOMPARE( server->responseCount(), 0 );
        QCOMPARE( server->envelopeCount(), 1 );
        QCOMPARE( server->lastEnvelope(), envelope );
    }

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::GetChannelStatus );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QCOMPARE( deliveredCommand.toPdu( (QSimCommand::ToPduOptions)options ), data );

    // There should be no responses or envelopes in the reverse direction yet.
    QCOMPARE( server->responseCount(), 0 );
    if ( envelope.isEmpty() )
        QCOMPARE( server->envelopeCount(), 0 );
    else
        QCOMPARE( server->envelopeCount(), 1 );

    // Compose and send the response.
    QSimTerminalResponse response;
    response.setCommand( deliveredCommand );
    if ( resptype < 0x0100 ) {
        response.setResult( (QSimTerminalResponse::Result)resptype );
    } else {
        response.setResult( (QSimTerminalResponse::Result)(resptype >> 8) );
        response.setCause( (QSimTerminalResponse::Cause)(resptype & 0xFF) );
    }
    response.setExtensionData( status );
    client->sendResponse( response );

    // Wait for the response to be received.
    QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 100 ) );

    // Check that the response is what we expected to get.
    QCOMPARE( server->responseCount(), 1 );
    if ( envelope.isEmpty() )
        QCOMPARE( server->envelopeCount(), 0 );
    else
        QCOMPARE( server->envelopeCount(), 1 );
    QCOMPARE( server->lastResponse(), resp );
}

#endif // !SYSTEMTEST

// Test encoding and decoding of EVENT DOWNLOAD envelopes based on the
// Populate data-driven tests for GET CHANNEL STATUS from the GCF test cases
// in GSM 51.010, section 27.22.4.31.
void QSimToolkitData::populateDataGetChannelStatus()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<QByteArray>("status");
    QTest::addColumn<QByteArray>("envelope");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1a[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_1_1a[] =
        {0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "GET CHANNEL STATUS 1.1.1A - GCF 27.22.4.31" )
        << QByteArray( (char *)data_1_1_1a, sizeof(data_1_1_1a) )
        << QByteArray( (char *)resp_1_1_1a, sizeof(resp_1_1_1a) )
        << QByteArray()
        << QByteArray()
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_1b[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_1_1b[] =
        {0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xB8, 0x02, 0x00, 0x00};
    static unsigned char const stat_1_1_1b[] =
        {0xB8, 0x02, 0x00, 0x00};
    QTest::newRow( "GET CHANNEL STATUS 1.1.1B - GCF 27.22.4.31" )
        << QByteArray( (char *)data_1_1_1b, sizeof(data_1_1_1b) )
        << QByteArray( (char *)resp_1_1_1b, sizeof(resp_1_1_1b) )
        << QByteArray( (char *)stat_1_1_1b, sizeof(stat_1_1_1b) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_1c[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_1_1c[] =
        {0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xB8, 0x02, 0x01, 0x00, 0xB8, 0x02, 0x02, 0x00, 0xB8, 0x02, 0x03, 0x00,
         0xB8, 0x02, 0x04, 0x00, 0xB8, 0x02, 0x05, 0x00, 0xB8, 0x02, 0x06, 0x00,
         0xB8, 0x02, 0x07, 0x00};
    static unsigned char const stat_1_1_1c[] =
        {0xB8, 0x02, 0x01, 0x00, 0xB8, 0x02, 0x02, 0x00, 0xB8, 0x02, 0x03, 0x00,
         0xB8, 0x02, 0x04, 0x00, 0xB8, 0x02, 0x05, 0x00, 0xB8, 0x02, 0x06, 0x00,
         0xB8, 0x02, 0x07, 0x00};
    QTest::newRow( "GET CHANNEL STATUS 1.1.1C - GCF 27.22.4.31" )
        << QByteArray( (char *)data_1_1_1c, sizeof(data_1_1_1c) )
        << QByteArray( (char *)resp_1_1_1c, sizeof(resp_1_1_1c) )
        << QByteArray( (char *)stat_1_1_1c, sizeof(stat_1_1_1c) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1a[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_2_1a[] =
        {0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xB8, 0x02, 0x81, 0x00};
    static unsigned char const stat_1_2_1a[] =
        {0xB8, 0x02, 0x81, 0x00};
    QTest::newRow( "GET CHANNEL STATUS 1.2.1A - GCF 27.22.4.31" )
        << QByteArray( (char *)data_1_2_1a, sizeof(data_1_2_1a) )
        << QByteArray( (char *)resp_1_2_1a, sizeof(resp_1_2_1a) )
        << QByteArray( (char *)stat_1_2_1a, sizeof(stat_1_2_1a) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1b[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_2_1b[] =
        {0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xB8, 0x02, 0x81, 0x00, 0xB8, 0x02, 0x02, 0x00, 0xB8, 0x02, 0x03, 0x00,
         0xB8, 0x02, 0x04, 0x00, 0xB8, 0x02, 0x05, 0x00, 0xB8, 0x02, 0x06, 0x00,
         0xB8, 0x02, 0x07, 0x00};
    static unsigned char const stat_1_2_1b[] =
        {0xB8, 0x02, 0x81, 0x00, 0xB8, 0x02, 0x02, 0x00, 0xB8, 0x02, 0x03, 0x00,
         0xB8, 0x02, 0x04, 0x00, 0xB8, 0x02, 0x05, 0x00, 0xB8, 0x02, 0x06, 0x00,
         0xB8, 0x02, 0x07, 0x00};
    QTest::newRow( "GET CHANNEL STATUS 1.2.1B - GCF 27.22.4.31" )
        << QByteArray( (char *)data_1_2_1b, sizeof(data_1_2_1b) )
        << QByteArray( (char *)resp_1_2_1b, sizeof(resp_1_2_1b) )
        << QByteArray( (char *)stat_1_2_1b, sizeof(stat_1_2_1b) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1a[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_3_1a[] =
        {0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "GET CHANNEL STATUS 1.3.1A - GCF 27.22.4.31" )
        << QByteArray( (char *)data_1_3_1a, sizeof(data_1_3_1a) )
        << QByteArray( (char *)resp_1_3_1a, sizeof(resp_1_3_1a) )
        << QByteArray()
        << QByteArray()
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1b[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_3_1b[] =
        {0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xB8, 0x02, 0x00, 0x00};
    static unsigned char const stat_1_3_1b[] =
        {0xB8, 0x02, 0x00, 0x00};
    QTest::newRow( "GET CHANNEL STATUS 1.3.1B - GCF 27.22.4.31" )
        << QByteArray( (char *)data_1_3_1b, sizeof(data_1_3_1b) )
        << QByteArray( (char *)resp_1_3_1b, sizeof(resp_1_3_1b) )
        << QByteArray( (char *)stat_1_3_1b, sizeof(stat_1_3_1b) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1c[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_3_1c[] =
        {0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xB8, 0x02, 0x01, 0x00, 0xB8, 0x02, 0x02, 0x00, 0xB8, 0x02, 0x03, 0x00,
         0xB8, 0x02, 0x04, 0x00, 0xB8, 0x02, 0x05, 0x00, 0xB8, 0x02, 0x06, 0x00,
         0xB8, 0x02, 0x07, 0x00};
    static unsigned char const stat_1_3_1c[] =
        {0xB8, 0x02, 0x01, 0x00, 0xB8, 0x02, 0x02, 0x00, 0xB8, 0x02, 0x03, 0x00,
         0xB8, 0x02, 0x04, 0x00, 0xB8, 0x02, 0x05, 0x00, 0xB8, 0x02, 0x06, 0x00,
         0xB8, 0x02, 0x07, 0x00};
    QTest::newRow( "GET CHANNEL STATUS 1.3.1C - GCF 27.22.4.31" )
        << QByteArray( (char *)data_1_3_1c, sizeof(data_1_3_1c) )
        << QByteArray( (char *)resp_1_3_1c, sizeof(resp_1_3_1c) )
        << QByteArray( (char *)stat_1_3_1c, sizeof(stat_1_3_1c) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1d[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_3_1d[] =
        {0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xB8, 0x02, 0x01, 0x05};
    static unsigned char const stat_1_3_1d[] =
        {0xB8, 0x02, 0x01, 0x05};
    static unsigned char const enve_1_3_1d[] =
        {0xD6, 0x0B, 0x99, 0x01, 0x0A, 0x82, 0x02, 0x82, 0x81, 0xB8, 0x02, 0x01,
         0x05};
    QTest::newRow( "GET CHANNEL STATUS 1.3.1D - GCF 27.22.4.31" )
        << QByteArray( (char *)data_1_3_1d, sizeof(data_1_3_1d) )
        << QByteArray( (char *)resp_1_3_1d, sizeof(resp_1_3_1d) )
        << QByteArray( (char *)stat_1_3_1d, sizeof(stat_1_3_1d) )
        << QByteArray( (char *)enve_1_3_1d, sizeof(enve_1_3_1d) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1e[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_3_1e[] =
        {0x81, 0x03, 0x01, 0x44, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xB8, 0x02, 0x01, 0x05, 0xB8, 0x02, 0x02, 0x00, 0xB8, 0x02, 0x03, 0x00,
         0xB8, 0x02, 0x04, 0x00, 0xB8, 0x02, 0x05, 0x00, 0xB8, 0x02, 0x06, 0x00,
         0xB8, 0x02, 0x07, 0x00};
    static unsigned char const stat_1_3_1e[] =
        {0xB8, 0x02, 0x01, 0x05, 0xB8, 0x02, 0x02, 0x00, 0xB8, 0x02, 0x03, 0x00,
         0xB8, 0x02, 0x04, 0x00, 0xB8, 0x02, 0x05, 0x00, 0xB8, 0x02, 0x06, 0x00,
         0xB8, 0x02, 0x07, 0x00};
    QTest::newRow( "GET CHANNEL STATUS 1.3.1E - GCF 27.22.4.31" )
        << QByteArray( (char *)data_1_3_1e, sizeof(data_1_3_1e) )
        << QByteArray( (char *)resp_1_3_1e, sizeof(resp_1_3_1e) )
        << QByteArray( (char *)stat_1_3_1e, sizeof(stat_1_3_1e) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );
}
