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
// Test encoding and decoding of SEND DTMF commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.24 - SEND DTMF.
void tst_QSimToolkit::testEncodeSendDTMF_data()
{
    QSimToolkitData::populateDataSendDTMF();
}
void tst_QSimToolkit::testEncodeSendDTMF()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( QString, number );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( QByteArray, textAttribute );
    QFETCH( QString, html );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::SendDTMF );
    QVERIFY( decoded.destinationDevice() == QSimCommand::Network );
    QCOMPARE( decoded.text(), text );
    QCOMPARE( decoded.number(), number );
    QCOMPARE( (int)decoded.iconId(), iconId );
    QCOMPARE( decoded.iconSelfExplanatory(), iconSelfExplanatory );
    QCOMPARE( decoded.textAttribute(), textAttribute );
    if ( !textAttribute.isEmpty() )
        QCOMPARE( decoded.textHtml(), html );

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

// Test that SEND DTMF commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverSendDTMF_data()
{
    QSimToolkitData::populateDataSendDTMF();
}
void tst_QSimToolkit::testDeliverSendDTMF()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( QString, number );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( QByteArray, textAttribute );
    QFETCH( QString, html );
    QFETCH( int, options );

    Q_UNUSED(resptype);
    Q_UNUSED(html);

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::SendDTMF );
    cmd.setDestinationDevice( QSimCommand::Network );
    cmd.setText( text );
    cmd.setNumber( number );
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );
    cmd.setTextAttribute( textAttribute );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QVERIFY( deliveredCommand.text() == cmd.text() );
    QVERIFY( deliveredCommand.number() == cmd.number() );
    QVERIFY( deliveredCommand.iconId() == cmd.iconId() );
    QVERIFY( deliveredCommand.iconSelfExplanatory() == cmd.iconSelfExplanatory() );
    QVERIFY( deliveredCommand.textAttribute() == cmd.textAttribute() );
    QCOMPARE( deliveredCommand.toPdu( (QSimCommand::ToPduOptions)options ), data );

    // The terminal response should have been sent immediately to ack reception of the command.
    // We cannot check the return data explicitly because that will be handled in the modem
    // and will typically be invisible to Qtopia.  We therefore compare against what the
    // response would be without the return data.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    QSimTerminalResponse resp2;
    resp2.setCommand( deliveredCommand );
    resp2.setResult( QSimTerminalResponse::Success );
    QCOMPARE( server->lastResponse(), resp2.toPdu() );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for SEND DTMF from the GCF test cases
// in GSM 51.010, section 27.22.4.24.
void QSimToolkitData::populateDataSendDTMF()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("number");
    QTest::addColumn<int>("iconId");
    QTest::addColumn<bool>("iconSelfExplanatory");
    QTest::addColumn<QByteArray>("textAttribute");
    QTest::addColumn<QString>("html");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x0D, 0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x81, 0x83, 0xAC,
         0x02, 0xC1, 0xF2};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SEND DTMF 1.1.1 - GCF 27.22.4.24.1" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "" )
        << QString( "1p2" )
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x1B, 0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x09, 0x53, 0x65, 0x6E, 0x64, 0x20, 0x44, 0x54, 0x4D, 0x46, 0xAC, 0x05,
         0x21, 0x43, 0x65, 0x87, 0x09};
    static unsigned char const resp_1_2_1[] =
        {0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SEND DTMF 1.2.1 - GCF 27.22.4.24.1" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << 0x0000       // Command performed successfully
        << QString( "Send DTMF" )
        << QString( "1234567890" )
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1[] =
        {0xD0, 0x13, 0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x00, 0xAC, 0x06, 0xC1, 0xCC, 0xCC, 0xCC, 0xCC, 0x2C};
    static unsigned char const resp_1_3_1[] =
        {0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SEND DTMF 1.3.1 - GCF 27.22.4.24.1" )
        << QByteArray( (char *)data_1_3_1, sizeof(data_1_3_1) )
        << QByteArray( (char *)resp_1_3_1, sizeof(resp_1_3_1) )
        << 0x0000       // Command performed successfully
        << QString( "" )
        << QString( "1pppppppppp2" )
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::EncodeEmptyStrings );

    static unsigned char const data_1_4_1[] =
        {0xD0, 0x0D, 0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x81, 0x83, 0xAC,
         0x02, 0xC1, 0xF2};
    static unsigned char const resp_1_4_1[] =
        {0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x20,
         0x07};
    QTest::newRow( "SEND DTMF 1.4.1 - GCF 27.22.4.24.1" )
        << QByteArray( (char *)data_1_4_1, sizeof(data_1_4_1) )
        << QByteArray( (char *)resp_1_4_1, sizeof(resp_1_4_1) )
        << 0x2007       // Not in a speech call
        << QString( "" )
        << QString( "1p2" )
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1a[] =
        {0xD0, 0x1D, 0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0A, 0x42, 0x61, 0x73, 0x69, 0x63, 0x20, 0x49, 0x63, 0x6F, 0x6E, 0xAC,
         0x02, 0xC1, 0xF2, 0x9E, 0x02, 0x00, 0x01};
    static unsigned char const resp_2_1_1a[] =
        {0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SEND DTMF 2.1.1A - GCF 27.22.4.24.2" )
        << QByteArray( (char *)data_2_1_1a, sizeof(data_2_1_1a) )
        << QByteArray( (char *)resp_2_1_1a, sizeof(resp_2_1_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Basic Icon" )
        << QString( "1p2" )
        << 1 << true    // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1b[] =
        {0xD0, 0x1D, 0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0A, 0x42, 0x61, 0x73, 0x69, 0x63, 0x20, 0x49, 0x63, 0x6F, 0x6E, 0xAC,
         0x02, 0xC1, 0xF2, 0x9E, 0x02, 0x00, 0x01};
    static unsigned char const resp_2_1_1b[] =
        {0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04};
    QTest::newRow( "SEND DTMF 2.1.1B - GCF 27.22.4.24.2" )
        << QByteArray( (char *)data_2_1_1b, sizeof(data_2_1_1b) )
        << QByteArray( (char *)resp_2_1_1b, sizeof(resp_2_1_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Basic Icon" )
        << QString( "1p2" )
        << 1 << true    // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_2_1a[] =
        {0xD0, 0x1E, 0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0B, 0x43, 0x6F, 0x6C, 0x6F, 0x75, 0x72, 0x20, 0x49, 0x63, 0x6F, 0x6E,
         0xAC, 0x02, 0xC1, 0xF2, 0x9E, 0x02, 0x00, 0x02};
    static unsigned char const resp_2_2_1a[] =
        {0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SEND DTMF 2.2.1A - GCF 27.22.4.24.2" )
        << QByteArray( (char *)data_2_2_1a, sizeof(data_2_2_1a) )
        << QByteArray( (char *)resp_2_2_1a, sizeof(resp_2_2_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Colour Icon" )
        << QString( "1p2" )
        << 2 << true    // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_2_1b[] =
        {0xD0, 0x1E, 0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0B, 0x43, 0x6F, 0x6C, 0x6F, 0x75, 0x72, 0x20, 0x49, 0x63, 0x6F, 0x6E,
         0xAC, 0x02, 0xC1, 0xF2, 0x9E, 0x02, 0x00, 0x02};
    static unsigned char const resp_2_2_1b[] =
        {0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04};
    QTest::newRow( "SEND DTMF 2.2.1B - GCF 27.22.4.24.2" )
        << QByteArray( (char *)data_2_2_1b, sizeof(data_2_2_1b) )
        << QByteArray( (char *)resp_2_2_1b, sizeof(resp_2_2_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Colour Icon" )
        << QString( "1p2" )
        << 2 << true    // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_3_1a[] =
        {0xD0, 0x1C, 0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x09, 0x53, 0x65, 0x6E, 0x64, 0x20, 0x44, 0x54, 0x4D, 0x46, 0xAC, 0x02,
         0xC1, 0xF2, 0x9E, 0x02, 0x01, 0x01};
    static unsigned char const resp_2_3_1a[] =
        {0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SEND DTMF 2.3.1A - GCF 27.22.4.24.2" )
        << QByteArray( (char *)data_2_3_1a, sizeof(data_2_3_1a) )
        << QByteArray( (char *)resp_2_3_1a, sizeof(resp_2_3_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Send DTMF" )
        << QString( "1p2" )
        << 1 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_3_1b[] =
        {0xD0, 0x1C, 0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x09, 0x53, 0x65, 0x6E, 0x64, 0x20, 0x44, 0x54, 0x4D, 0x46, 0xAC, 0x02,
         0xC1, 0xF2, 0x9E, 0x02, 0x01, 0x01};
    static unsigned char const resp_2_3_1b[] =
        {0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04};
    QTest::newRow( "SEND DTMF 2.3.1B - GCF 27.22.4.24.2" )
        << QByteArray( (char *)data_2_3_1b, sizeof(data_2_3_1b) )
        << QByteArray( (char *)resp_2_3_1b, sizeof(resp_2_3_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Send DTMF" )
        << QString( "1p2" )
        << 1 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    // Note: this test case was modified from the GCF original to include the
    // FFFF bytes at the end of the UCS-2 string.  I think the GCF original was wrong.
    static unsigned char const data_3_1_1[] =
        {0xD0, 0x28, 0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x19, 0x80, 0x04, 0x17, 0x04, 0x14, 0x04, 0x20, 0x04, 0x10, 0x04, 0x12,
         0x04, 0x21, 0x04, 0x22, 0x04, 0x12, 0x04, 0x23, 0x04, 0x19, 0x04, 0x22,
         0x04, 0x15, 0xAC, 0x02, 0xC1, 0xF2};
    static unsigned char const resp_3_1_1[] =
        {0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static ushort const str_3_1_1[] =
        {0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423,
         0x0419, 0x0422, 0x0415};
    QTest::newRow( "SEND DTMF 3.1.1 - GCF 27.22.4.24.3" )
        << QByteArray( (char *)data_3_1_1, sizeof(data_3_1_1) )
        << QByteArray( (char *)resp_3_1_1, sizeof(resp_3_1_1) )
        << 0x0000       // Command performed successfully
        << QString::fromUtf16( str_3_1_1, sizeof(str_3_1_1) / sizeof(ushort) )
        << QString( "1p2" )
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    // Test only one of the text attribute test cases.  We assume that if
    // one works, then they will all work.  The DISPLAY TEXT command tests
    // the formatting rules.
    static unsigned char const data_4_1_1[] =
        {0xD0, 0x23, 0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0B, 0x53, 0x65, 0x6E, 0x64, 0x20, 0x44, 0x54, 0x4D, 0x46, 0x20, 0x31,
         0xAC, 0x05, 0x21, 0x43, 0x65, 0x87, 0x09, 0xD0, 0x04, 0x00, 0x0B, 0x00,
         0xB4};
    static unsigned char const resp_4_1_1[] =
        {0x81, 0x03, 0x01, 0x14, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const attr_4_1_1[] =
        {0x00, 0x0B, 0x00, 0xB4};
    QTest::newRow( "SEND DTMF 4.1.1 - GCF 27.22.4.24.4" )
        << QByteArray( (char *)data_4_1_1, sizeof(data_4_1_1) )
        << QByteArray( (char *)resp_4_1_1, sizeof(resp_4_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Send DTMF 1" )
        << QString( "1234567890" )
        << 0 << false   // Icon details
        << QByteArray( (char *)attr_4_1_1, sizeof(attr_4_1_1) )
        << QString( "<body bgcolor=\"#FFFF00\"><div align=\"left\"><font color=\"#008000\">Send DTMF 1</font></div></body>" )
        << (int)( QSimCommand::NoPduOptions );
}
