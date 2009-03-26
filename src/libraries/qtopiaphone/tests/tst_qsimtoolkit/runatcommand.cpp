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
// Test encoding and decoding of RUN AT COMMAND commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.23 - RUN AT COMMAND.
void tst_QSimToolkit::testEncodeRunATCommand_data()
{
    QSimToolkitData::populateDataRunATCommand();
}
void tst_QSimToolkit::testEncodeRunATCommand()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( QByteArray, command );
    QFETCH( QByteArray, response );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::RunATCommand );
    QVERIFY( decoded.destinationDevice() == QSimCommand::ME );
    QCOMPARE( decoded.text(), text );
    QCOMPARE( decoded.extensionField(0xA8), command );
    QCOMPARE( (int)decoded.iconId(), iconId );
    QCOMPARE( decoded.iconSelfExplanatory(), iconSelfExplanatory );

    // Check that the original command PDU can be reconstructed correctly.
    QByteArray encoded = decoded.toPdu( (QSimCommand::ToPduOptions)options );
    QCOMPARE( encoded, data );

    // Check that the terminal response PDU can be parsed correctly.
    QSimTerminalResponse decodedResp = QSimTerminalResponse::fromPdu(resp);
    QVERIFY( data.contains( decodedResp.commandPdu() ) );
    QVERIFY( decodedResp.result() == (QSimTerminalResponse::Result)resptype );
    if ( resptype < 0x0100 ) {
        QVERIFY( decodedResp.result() == (QSimTerminalResponse::Result)resptype );
        QVERIFY( decodedResp.causeData().isEmpty() );
        QVERIFY( decodedResp.cause() == QSimTerminalResponse::NoSpecificCause );
    } else {
        QVERIFY( decodedResp.result() == (QSimTerminalResponse::Result)(resptype >> 8) );
        QVERIFY( decodedResp.causeData().size() == 1 );
        QVERIFY( decodedResp.cause() == (QSimTerminalResponse::Cause)(resptype & 0xFF) );
    }
    if ( resptype != 0x0032 )
        QCOMPARE( decodedResp.extensionField(0xA9), response );

    // Check that the original terminal response PDU can be reconstructed correctly.
    QCOMPARE( decodedResp.toPdu(), resp );
}

// Test that RUN AT COMMAND commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverRunATCommand_data()
{
    QSimToolkitData::populateDataRunATCommand();
}
void tst_QSimToolkit::testDeliverRunATCommand()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( QByteArray, command );
    QFETCH( QByteArray, response );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( int, options );

    Q_UNUSED(resptype);

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::RunATCommand );
    cmd.setText( text );
    cmd.addExtensionField( 0xA8, command );
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QVERIFY( deliveredCommand.text() == cmd.text() );
    QVERIFY( deliveredCommand.extensionData() == cmd.extensionData() );
    QVERIFY( deliveredCommand.iconId() == cmd.iconId() );
    QVERIFY( deliveredCommand.iconSelfExplanatory() == cmd.iconSelfExplanatory() );
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

// Populate data-driven tests for RUN AT COMMAND from the GCF test cases
// in GSM 51.010, section 27.22.4.23.
void QSimToolkitData::populateDataRunATCommand()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<QString>("text");
    QTest::addColumn<QByteArray>("command");
    QTest::addColumn<QByteArray>("response");
    QTest::addColumn<int>("iconId");
    QTest::addColumn<bool>("iconSelfExplanatory");
    QTest::addColumn<int>("options");

    // Compose the return value for the commands that follow.
    static unsigned char const imsi_1_1_1[] =
        {0x09, 0x10, 0x10, 0x10, 0x32, 0x54, 0x76, 0x98};
    QByteArray imsi = QByteArray( (char *)imsi_1_1_1, sizeof(imsi_1_1_1) );

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x12, 0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA8,
         0x07, 0x41, 0x54, 0x2B, 0x43, 0x49, 0x4D, 0x49};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA9, 0x08, 0x09, 0x10, 0x10, 0x10, 0x32, 0x54, 0x76, 0x98};
    QTest::newRow( "RUN AT COMMAND 1.1.1 - GCF 27.22.4.23.1" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "" )
        << QByteArray( "AT+CIMI" )
        << imsi
        << 0 << false   // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x14, 0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x00, 0xA8, 0x07, 0x41, 0x54, 0x2B, 0x43, 0x49, 0x4D, 0x49};
    static unsigned char const resp_1_2_1[] =
        {0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA9, 0x08, 0x09, 0x10, 0x10, 0x10, 0x32, 0x54, 0x76, 0x98};
    QTest::newRow( "RUN AT COMMAND 1.2.1 - GCF 27.22.4.23.1" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << 0x0000       // Command performed successfully
        << QString( "" )
        << QByteArray( "AT+CIMI" )
        << imsi
        << 0 << false   // Icon details
        << (int)( QSimCommand::EncodeEmptyStrings );

    static unsigned char const data_1_3_1[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0E, 0x52, 0x75, 0x6E, 0x20, 0x41, 0x54, 0x20, 0x43, 0x6F, 0x6D, 0x6D,
         0x61, 0x6E, 0x64, 0xA8, 0x07, 0x41, 0x54, 0x2B, 0x43, 0x49, 0x4D, 0x49};
    static unsigned char const resp_1_3_1[] =
        {0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA9, 0x08, 0x09, 0x10, 0x10, 0x10, 0x32, 0x54, 0x76, 0x98};
    QTest::newRow( "RUN AT COMMAND 1.3.1 - GCF 27.22.4.23.1" )
        << QByteArray( (char *)data_1_3_1, sizeof(data_1_3_1) )
        << QByteArray( (char *)resp_1_3_1, sizeof(resp_1_3_1) )
        << 0x0000       // Command performed successfully
        << QString( "Run AT Command" )
        << QByteArray( "AT+CIMI" )
        << imsi
        << 0 << false   // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1a[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0A, 0x42, 0x61, 0x73, 0x69, 0x63, 0x20, 0x49, 0x63, 0x6F, 0x6E, 0xA8,
         0x07, 0x41, 0x54, 0x2B, 0x43, 0x49, 0x4D, 0x49, 0x9E, 0x02, 0x00, 0x01};
    static unsigned char const resp_2_1_1a[] =
        {0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA9, 0x08, 0x09, 0x10, 0x10, 0x10, 0x32, 0x54, 0x76, 0x98};
    QTest::newRow( "RUN AT COMMAND 2.1.1A - GCF 27.22.4.23.2" )
        << QByteArray( (char *)data_2_1_1a, sizeof(data_2_1_1a) )
        << QByteArray( (char *)resp_2_1_1a, sizeof(resp_2_1_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Basic Icon" )
        << QByteArray( "AT+CIMI" )
        << imsi
        << 1 << true    // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1b[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0A, 0x42, 0x61, 0x73, 0x69, 0x63, 0x20, 0x49, 0x63, 0x6F, 0x6E, 0xA8,
         0x07, 0x41, 0x54, 0x2B, 0x43, 0x49, 0x4D, 0x49, 0x9E, 0x02, 0x00, 0x01};
    static unsigned char const resp_2_1_1b[] =
        {0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04,
         0xA9, 0x08, 0x09, 0x10, 0x10, 0x10, 0x32, 0x54, 0x76, 0x98};
    QTest::newRow( "RUN AT COMMAND 2.1.1B - GCF 27.22.4.23.2" )
        << QByteArray( (char *)data_2_1_1b, sizeof(data_2_1_1b) )
        << QByteArray( (char *)resp_2_1_1b, sizeof(resp_2_1_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Basic Icon" )
        << QByteArray( "AT+CIMI" )
        << imsi
        << 1 << true    // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_2_1a[] =
        {0xD0, 0x23, 0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0B, 0x43, 0x6F, 0x6C, 0x6F, 0x75, 0x72, 0x20, 0x49, 0x63, 0x6F, 0x6E,
         0xA8, 0x07, 0x41, 0x54, 0x2B, 0x43, 0x49, 0x4D, 0x49, 0x9E, 0x02, 0x00,
         0x02};
    static unsigned char const resp_2_2_1a[] =
        {0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA9, 0x08, 0x09, 0x10, 0x10, 0x10, 0x32, 0x54, 0x76, 0x98};
    QTest::newRow( "RUN AT COMMAND 2.2.1A - GCF 27.22.4.23.2" )
        << QByteArray( (char *)data_2_2_1a, sizeof(data_2_2_1a) )
        << QByteArray( (char *)resp_2_2_1a, sizeof(resp_2_2_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Colour Icon" )
        << QByteArray( "AT+CIMI" )
        << imsi
        << 2 << true    // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_2_1b[] =
        {0xD0, 0x23, 0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0B, 0x43, 0x6F, 0x6C, 0x6F, 0x75, 0x72, 0x20, 0x49, 0x63, 0x6F, 0x6E,
         0xA8, 0x07, 0x41, 0x54, 0x2B, 0x43, 0x49, 0x4D, 0x49, 0x9E, 0x02, 0x00,
         0x02};
    static unsigned char const resp_2_2_1b[] =
        {0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04,
         0xA9, 0x08, 0x09, 0x10, 0x10, 0x10, 0x32, 0x54, 0x76, 0x98};
    QTest::newRow( "RUN AT COMMAND 2.2.1A - GCF 27.22.4.23.2" )
        << QByteArray( (char *)data_2_2_1b, sizeof(data_2_2_1b) )
        << QByteArray( (char *)resp_2_2_1b, sizeof(resp_2_2_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Colour Icon" )
        << QByteArray( "AT+CIMI" )
        << imsi
        << 2 << true    // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_3_1a[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0A, 0x42, 0x61, 0x73, 0x69, 0x63, 0x20, 0x49, 0x63, 0x6F, 0x6E, 0xA8,
         0x07, 0x41, 0x54, 0x2B, 0x43, 0x49, 0x4D, 0x49, 0x9E, 0x02, 0x01, 0x01};
    static unsigned char const resp_2_3_1a[] =
        {0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA9, 0x08, 0x09, 0x10, 0x10, 0x10, 0x32, 0x54, 0x76, 0x98};
    QTest::newRow( "RUN AT COMMAND 2.3.1A - GCF 27.22.4.23.2" )
        << QByteArray( (char *)data_2_3_1a, sizeof(data_2_3_1a) )
        << QByteArray( (char *)resp_2_3_1a, sizeof(resp_2_3_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Basic Icon" )
        << QByteArray( "AT+CIMI" )
        << imsi
        << 1 << false   // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_3_1b[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0A, 0x42, 0x61, 0x73, 0x69, 0x63, 0x20, 0x49, 0x63, 0x6F, 0x6E, 0xA8,
         0x07, 0x41, 0x54, 0x2B, 0x43, 0x49, 0x4D, 0x49, 0x9E, 0x02, 0x01, 0x01};
    static unsigned char const resp_2_3_1b[] =
        {0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04,
         0xA9, 0x08, 0x09, 0x10, 0x10, 0x10, 0x32, 0x54, 0x76, 0x98};
    QTest::newRow( "RUN AT COMMAND 2.3.1B - GCF 27.22.4.23.2" )
        << QByteArray( (char *)data_2_3_1b, sizeof(data_2_3_1b) )
        << QByteArray( (char *)resp_2_3_1b, sizeof(resp_2_3_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Basic Icon" )
        << QByteArray( "AT+CIMI" )
        << imsi
        << 1 << false   // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_4_1a[] =
        {0xD0, 0x23, 0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0B, 0x43, 0x6F, 0x6C, 0x6F, 0x75, 0x72, 0x20, 0x49, 0x63, 0x6F, 0x6E,
         0xA8, 0x07, 0x41, 0x54, 0x2B, 0x43, 0x49, 0x4D, 0x49, 0x9E, 0x02, 0x01,
         0x02};
    static unsigned char const resp_2_4_1a[] =
        {0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA9, 0x08, 0x09, 0x10, 0x10, 0x10, 0x32, 0x54, 0x76, 0x98};
    QTest::newRow( "RUN AT COMMAND 2.4.1A - GCF 27.22.4.23.2" )
        << QByteArray( (char *)data_2_4_1a, sizeof(data_2_4_1a) )
        << QByteArray( (char *)resp_2_4_1a, sizeof(resp_2_4_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Colour Icon" )
        << QByteArray( "AT+CIMI" )
        << imsi
        << 2 << false   // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_4_1b[] =
        {0xD0, 0x23, 0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0B, 0x43, 0x6F, 0x6C, 0x6F, 0x75, 0x72, 0x20, 0x49, 0x63, 0x6F, 0x6E,
         0xA8, 0x07, 0x41, 0x54, 0x2B, 0x43, 0x49, 0x4D, 0x49, 0x9E, 0x02, 0x01,
         0x02};
    static unsigned char const resp_2_4_1b[] =
        {0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04,
         0xA9, 0x08, 0x09, 0x10, 0x10, 0x10, 0x32, 0x54, 0x76, 0x98};
    QTest::newRow( "RUN AT COMMAND 2.4.1B - GCF 27.22.4.23.2" )
        << QByteArray( (char *)data_2_4_1b, sizeof(data_2_4_1b) )
        << QByteArray( (char *)resp_2_4_1b, sizeof(resp_2_4_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Colour Icon" )
        << QByteArray( "AT+CIMI" )
        << imsi
        << 2 << false   // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_5_1[] =
        {0xD0, 0x16, 0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA8,
         0x07, 0x41, 0x54, 0x2B, 0x43, 0x49, 0x4D, 0x49, 0x9E, 0x02, 0x01, 0x01};
    static unsigned char const resp_2_5_1[] =
        {0x81, 0x03, 0x01, 0x34, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x32};
    QTest::newRow( "RUN AT COMMAND 2.5.1 - GCF 27.22.4.23.2" )
        << QByteArray( (char *)data_2_5_1, sizeof(data_2_5_1) )
        << QByteArray( (char *)resp_2_5_1, sizeof(resp_2_5_1) )
        << 0x0032       // Command data not understood by ME
        << QString( "" )
        << QByteArray( "AT+CIMI" )
        << imsi
        << 1 << false   // Icon details
        << (int)( QSimCommand::NoPduOptions );
}
