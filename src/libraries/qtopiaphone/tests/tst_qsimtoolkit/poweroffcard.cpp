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
// Test encoding and decoding of POWER OFF CARD commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.18 - POWER OFF CARD.
void tst_QSimToolkit::testEncodePowerOffCard_data()
{
    QSimToolkitData::populateDataPowerOffCard();
}
void tst_QSimToolkit::testEncodePowerOffCard()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( int, device );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::PowerOffCard );
    QVERIFY( decoded.destinationDevice() == (QSimCommand::Device)( device + 0x10 ) );

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

// Test that POWER OFF CARD commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverPowerOffCard_data()
{
    QSimToolkitData::populateDataPowerOffCard();
}
void tst_QSimToolkit::testDeliverPowerOffCard()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( int, device );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::PowerOffCard );
    cmd.setDestinationDevice( (QSimCommand::Device)( device + 0x10 ) );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QCOMPARE( deliveredCommand.toPdu( (QSimCommand::ToPduOptions)options ), data );

    // The response should have been sent immediately.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    if ( resptype == 0x0000 ) {
        QCOMPARE( server->lastResponse(), resp );
    } else {
        // We cannot test the "no card inserted" case because the qtopiaphone
        // library will always respond with "command performed successfully".
        static unsigned char const ok_resp[] =
            {0x81, 0x03, 0x01, 0x32, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
        QByteArray resp2 = QByteArray( (char *)ok_resp, sizeof( ok_resp ) );
        QCOMPARE( server->lastResponse(), resp2 );
    }
}

#endif // !SYSTEMTEST

// Populate data-driven tests for POWER OFF CARD from the GCF test cases
// in GSM 51.010, section 27.22.4.18.
void QSimToolkitData::populateDataPowerOffCard()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<int>("device");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x32, 0x00, 0x82, 0x02, 0x81, 0x11};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x32, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "POWER OFF CARD 1.1.1 - GCF 27.22.4.18.1" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << 0x0000       // Command performed successfully
        << 1            // Card reader device number
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x32, 0x00, 0x82, 0x02, 0x81, 0x11};
    static unsigned char const resp_1_2_1[] =
        {0x81, 0x03, 0x01, 0x32, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x38,
         0x02};
    QTest::newRow( "POWER OFF CARD 1.2.1 - GCF 27.22.4.18.1" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << 0x3802       // Card removed or not present
        << 1            // Card reader device number
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x32, 0x00, 0x82, 0x02, 0x81, 0x11};
    static unsigned char const resp_2_1_1[] =
        {0x81, 0x03, 0x01, 0x32, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x38,
         0x01};
    QTest::newRow( "POWER OFF CARD 2.1.1 - GCF 27.22.4.18.2" )
        << QByteArray( (char *)data_2_1_1, sizeof(data_2_1_1) )
        << QByteArray( (char *)resp_2_1_1, sizeof(resp_2_1_1) )
        << 0x3801       // Card reader removed or not present
        << 1            // Card reader device number
        << (int)( QSimCommand::NoPduOptions );
}
