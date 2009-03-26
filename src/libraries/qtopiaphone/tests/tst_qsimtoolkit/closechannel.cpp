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

// Test encoding and decoding of CLOSE CHANNEL commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.28 - CLOSE CHANNEL.
void tst_QSimToolkit::testEncodeCloseChannel_data()
{
    QSimToolkitData::populateDataCloseChannel();
}
void tst_QSimToolkit::testEncodeCloseChannel()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( int, channel );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::CloseChannel );
    QCOMPARE( decoded.text(), text );
    QCOMPARE( decoded.destinationDevice(), (QSimCommand::Device)( channel + 0x20 ) );
    QCOMPARE( (int)decoded.iconId(), iconId );
    QCOMPARE( decoded.iconSelfExplanatory(), iconSelfExplanatory );

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
}

// Test that CLOSE CHANNEL commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverCloseChannel_data()
{
    QSimToolkitData::populateDataCloseChannel();
}
void tst_QSimToolkit::testDeliverCloseChannel()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( int, channel );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::CloseChannel );
    cmd.setText( text );
    cmd.setDestinationDevice( (QSimCommand::Device)( channel + 0x20 ) );
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QVERIFY( deliveredCommand.text() == cmd.text() );
    QVERIFY( deliveredCommand.destinationDevice() == cmd.destinationDevice() );
    QVERIFY( deliveredCommand.iconId() == cmd.iconId() );
    QVERIFY( deliveredCommand.iconSelfExplanatory() == cmd.iconSelfExplanatory() );
    QCOMPARE( deliveredCommand.toPdu( (QSimCommand::ToPduOptions)options ), data );

    // There should be no responses or envelopes in the reverse direction yet.
    QCOMPARE( server->responseCount(), 0 );
    QCOMPARE( server->envelopeCount(), 0 );

    // Compose and send the response.
    QSimTerminalResponse response;
    response.setCommand( deliveredCommand );
    if ( resptype < 0x0100 ) {
        response.setResult( (QSimTerminalResponse::Result)resptype );
    } else {
        response.setResult( (QSimTerminalResponse::Result)(resptype >> 8) );
        response.setCause( (QSimTerminalResponse::Cause)(resptype & 0xFF) );
    }
    client->sendResponse( response );

    // Wait for the response to be received.
    QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 100 ) );

    // Check that the response is what we expected to get.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    QCOMPARE( server->lastResponse(), resp );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for CLOSE CHANNEL from the GCF test cases
// in GSM 51.010, section 27.22.4.28.
void QSimToolkitData::populateDataCloseChannel()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<QString>("text");
    QTest::addColumn<int>("channel");
    QTest::addColumn<int>("iconId");
    QTest::addColumn<bool>("iconSelfExplanatory");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x41, 0x00, 0x82, 0x02, 0x81, 0x21};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x41, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "CLOSE CHANNEL 1.1.1 - GCF 27.22.4.28" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "" )
        << 1
        << 0 << false   // No icon
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x41, 0x00, 0x82, 0x02, 0x81, 0x22};
    static unsigned char const resp_1_2_1[] =
        {0x81, 0x03, 0x01, 0x41, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x3A,
         0x03};
    QTest::newRow( "CLOSE CHANNEL 1.2.1 - GCF 27.22.4.28" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << 0x3A03       // Channel identifier not valid
        << QString( "" )
        << 2
        << 0 << false   // No icon
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1a[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x41, 0x00, 0x82, 0x02, 0x81, 0x21};
    static unsigned char const resp_1_3_1a[] =
        {0x81, 0x03, 0x01, 0x41, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x3A,
         0x02};
    QTest::newRow( "CLOSE CHANNEL 1.3.1A - GCF 27.22.4.28" )
        << QByteArray( (char *)data_1_3_1a, sizeof(data_1_3_1a) )
        << QByteArray( (char *)resp_1_3_1a, sizeof(resp_1_3_1a) )
        << 0x3A02       // Channel closed
        << QString( "" )
        << 1
        << 0 << false   // No icon
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1b[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x41, 0x00, 0x82, 0x02, 0x81, 0x21};
    static unsigned char const resp_1_3_1b[] =
        {0x81, 0x03, 0x01, 0x41, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x3A,
         0x03};
    QTest::newRow( "CLOSE CHANNEL 1.3.1B - GCF 27.22.4.28" )
        << QByteArray( (char *)data_1_3_1b, sizeof(data_1_3_1b) )
        << QByteArray( (char *)resp_1_3_1b, sizeof(resp_1_3_1b) )
        << 0x3A03       // Channel identifier not valid
        << QString( "" )
        << 1
        << 0 << false   // No icon
        << (int)( QSimCommand::NoPduOptions );
}
