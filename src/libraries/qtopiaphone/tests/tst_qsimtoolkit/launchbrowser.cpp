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
// Test encoding and decoding of LAUNCH BROWSER commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.26 - LAUNCH BROWSER.
void tst_QSimToolkit::testEncodeLaunchBrowser_data()
{
    QSimToolkitData::populateDataLaunchBrowser();
}
void tst_QSimToolkit::testEncodeLaunchBrowser()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, parameters );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( QString, url );
    QFETCH( int, mode );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::LaunchBrowser );
    QVERIFY( decoded.destinationDevice() == QSimCommand::ME );
    QCOMPARE( decoded.text(), text );
    QCOMPARE( decoded.url(), url );
    QCOMPARE( (int)decoded.browserLaunchMode(), mode );
    QCOMPARE( decoded.extensionData(), parameters );
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

// Test that LAUNCH BROWSER commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverLaunchBrowser_data()
{
    QSimToolkitData::populateDataLaunchBrowser();
}
void tst_QSimToolkit::testDeliverLaunchBrowser()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, parameters );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( QString, url );
    QFETCH( int, mode );
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
    cmd.setType( QSimCommand::LaunchBrowser );
    cmd.setText( text );
    cmd.setUrl( url );
    cmd.setBrowserLaunchMode( (QSimCommand::BrowserLaunchMode)mode );
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );
    cmd.setExtensionData( parameters );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QVERIFY( deliveredCommand.text() == cmd.text() );
    QVERIFY( deliveredCommand.url() == cmd.url() );
    QVERIFY( deliveredCommand.iconId() == cmd.iconId() );
    QVERIFY( deliveredCommand.iconSelfExplanatory() == cmd.iconSelfExplanatory() );
    QVERIFY( deliveredCommand.extensionData() == cmd.extensionData() );
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

// Populate data-driven tests for LAUNCH BROWSER from the GCF test cases
// in GSM 51.010, section 27.22.4.26.
void QSimToolkitData::populateDataLaunchBrowser()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<QByteArray>("parameters");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("url");
    QTest::addColumn<int>("mode");
    QTest::addColumn<int>("iconId");
    QTest::addColumn<bool>("iconSelfExplanatory");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x18, 0x81, 0x03, 0x01, 0x15, 0x00, 0x82, 0x02, 0x81, 0x82, 0x31,
         0x00, 0x05, 0x0B, 0x44, 0x65, 0x66, 0x61, 0x75, 0x6C, 0x74, 0x20, 0x55,
         0x52, 0x4C};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x15, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "LAUNCH BROWSER 1.1.1 - GCF 27.22.4.26.1" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "Default URL" )
        << QString( "" )
        << (int)( QSimCommand::IfNotAlreadyLaunched )
        << 0 << false   // No icon
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x1F, 0x81, 0x03, 0x01, 0x15, 0x00, 0x82, 0x02, 0x81, 0x82, 0x31,
         0x12, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x78, 0x78, 0x78, 0x2E,
         0x79, 0x79, 0x79, 0x2E, 0x7A, 0x7A, 0x7A, 0x05, 0x00};
    static unsigned char const resp_1_2_1[] =
        {0x81, 0x03, 0x01, 0x15, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "LAUNCH BROWSER 1.2.1 - GCF 27.22.4.26.1" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "" )
        << QString( "http://xxx.yyy.zzz" )
        << (int)( QSimCommand::IfNotAlreadyLaunched )
        << 0 << false   // No icon
        << (int)( QSimCommand::EncodeEmptyStrings );

    static unsigned char const data_1_3_1[] =
        {0xD0, 0x0E, 0x81, 0x03, 0x01, 0x15, 0x00, 0x82, 0x02, 0x81, 0x82, 0x30,
         0x01, 0x00, 0x31, 0x00};
    static unsigned char const resp_1_3_1[] =
        {0x81, 0x03, 0x01, 0x15, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const parm_1_3_1[] =
        {0x30, 0x01, 0x00};
    QTest::newRow( "LAUNCH BROWSER 1.3.1 - GCF 27.22.4.26.1" )
        << QByteArray( (char *)data_1_3_1, sizeof(data_1_3_1) )
        << QByteArray( (char *)resp_1_3_1, sizeof(resp_1_3_1) )
        << QByteArray( (char *)parm_1_3_1, sizeof(parm_1_3_1) )
        << 0x0000       // Command performed successfully
        << QString( "" )
        << QString( "" )
        << (int)( QSimCommand::IfNotAlreadyLaunched )
        << 0 << false   // No icon
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_1[] =
        {0xD0, 0x1C, 0x81, 0x03, 0x01, 0x15, 0x00, 0x82, 0x02, 0x81, 0x82, 0x31,
         0x00, 0x32, 0x01, 0x03, 0x0D, 0x0C, 0x04, 0x61, 0x62, 0x63, 0x2E, 0x64,
         0x65, 0x66, 0x2E, 0x67, 0x68, 0x69};
    static unsigned char const resp_1_4_1[] =
        {0x81, 0x03, 0x01, 0x15, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const parm_1_4_1[] =
        {0x32, 0x01, 0x03, 0x0D, 0x0C, 0x04, 0x61, 0x62, 0x63, 0x2E, 0x64, 0x65,
         0x66, 0x2E, 0x67, 0x68, 0x69};
    QTest::newRow( "LAUNCH BROWSER 1.4.1 - GCF 27.22.4.26.1" )
        << QByteArray( (char *)data_1_4_1, sizeof(data_1_4_1) )
        << QByteArray( (char *)resp_1_4_1, sizeof(resp_1_4_1) )
        << QByteArray( (char *)parm_1_4_1, sizeof(parm_1_4_1) )
        << 0x0000       // Command performed successfully
        << QString( "" )
        << QString( "" )
        << (int)( QSimCommand::IfNotAlreadyLaunched )
        << 0 << false   // No icon
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1[] =
        {0xD0, 0x18, 0x81, 0x03, 0x01, 0x15, 0x02, 0x82, 0x02, 0x81, 0x82, 0x31,
         0x00, 0x05, 0x0B, 0x44, 0x65, 0x66, 0x61, 0x75, 0x6C, 0x74, 0x20, 0x55,
         0x52, 0x4C};
    static unsigned char const resp_2_1_1[] =
        {0x81, 0x03, 0x01, 0x15, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "LAUNCH BROWSER 2.1.1 - GCF 27.22.4.26.2" )
        << QByteArray( (char *)data_2_1_1, sizeof(data_2_1_1) )
        << QByteArray( (char *)resp_2_1_1, sizeof(resp_2_1_1) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "Default URL" )
        << QString( "" )
        << (int)( QSimCommand::UseExisting )
        << 0 << false   // No icon
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_2_1[] =
        {0xD0, 0x18, 0x81, 0x03, 0x01, 0x15, 0x03, 0x82, 0x02, 0x81, 0x82, 0x31,
         0x00, 0x05, 0x0B, 0x44, 0x65, 0x66, 0x61, 0x75, 0x6C, 0x74, 0x20, 0x55,
         0x52, 0x4C};
    static unsigned char const resp_2_2_1[] =
        {0x81, 0x03, 0x01, 0x15, 0x03, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "LAUNCH BROWSER 2.2.1 - GCF 27.22.4.26.2" )
        << QByteArray( (char *)data_2_2_1, sizeof(data_2_2_1) )
        << QByteArray( (char *)resp_2_2_1, sizeof(resp_2_2_1) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "Default URL" )
        << QString( "" )
        << (int)( QSimCommand::CloseExistingAndLaunch )
        << 0 << false   // No icon
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_3_1[] =
        {0xD0, 0x0B, 0x81, 0x03, 0x01, 0x15, 0x00, 0x82, 0x02, 0x81, 0x82, 0x31,
         0x00};
    static unsigned char const resp_2_3_1[] =
        {0x81, 0x03, 0x01, 0x15, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x26,
         0x02};
    QTest::newRow( "LAUNCH BROWSER 2.3.1 - GCF 27.22.4.26.2" )
        << QByteArray( (char *)data_2_3_1, sizeof(data_2_3_1) )
        << QByteArray( (char *)resp_2_3_1, sizeof(resp_2_3_1) )
        << QByteArray()
        << 0x2602       // Browser not available
        << QString( "" )
        << QString( "" )
        << (int)( QSimCommand::IfNotAlreadyLaunched )
        << 0 << false   // No icon
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_3_1_1[] =
        {0xD0, 0x26, 0x81, 0x03, 0x01, 0x15, 0x02, 0x82, 0x02, 0x81, 0x82, 0x31,
         0x00, 0x05, 0x19, 0x80, 0x04, 0x17, 0x04, 0x14, 0x04, 0x20, 0x04, 0x10,
         0x04, 0x12, 0x04, 0x21, 0x04, 0x22, 0x04, 0x12, 0x04, 0x23, 0x04, 0x19,
         0x04, 0x22, 0x04, 0x15};
    static unsigned char const resp_3_1_1[] =
        {0x81, 0x03, 0x01, 0x15, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static ushort const str_3_1_1[] =
        {0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423,
         0x0419, 0x0422, 0x0415};
    QTest::newRow( "LAUNCH BROWSER 3.1.1 - GCF 27.22.4.26.3" )
        << QByteArray( (char *)data_3_1_1, sizeof(data_3_1_1) )
        << QByteArray( (char *)resp_3_1_1, sizeof(resp_3_1_1) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString::fromUtf16( str_3_1_1, sizeof(str_3_1_1) / sizeof(ushort) )
        << QString( "" )
        << (int)( QSimCommand::UseExisting )
        << 0 << false   // No icon
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_4_1_1a[] =
        {0xD0, 0x21, 0x81, 0x03, 0x01, 0x15, 0x02, 0x82, 0x02, 0x81, 0x82, 0x31,
         0x00, 0x05, 0x10, 0x4E, 0x6F, 0x74, 0x20, 0x73, 0x65, 0x6C, 0x66, 0x20,
         0x65, 0x78, 0x70, 0x6C, 0x61, 0x6E, 0x2E, 0x1E, 0x02, 0x01, 0x01};
    static unsigned char const resp_4_1_1a[] =
        {0x81, 0x03, 0x01, 0x15, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "LAUNCH BROWSER 4.1.1A - GCF 27.22.4.26.4" )
        << QByteArray( (char *)data_4_1_1a, sizeof(data_4_1_1a) )
        << QByteArray( (char *)resp_4_1_1a, sizeof(resp_4_1_1a) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "Not self explan." )
        << QString( "" )
        << (int)( QSimCommand::UseExisting )
        << 1 << false   // No icon
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_4_1_1b[] =
        {0xD0, 0x21, 0x81, 0x03, 0x01, 0x15, 0x02, 0x82, 0x02, 0x81, 0x82, 0x31,
         0x00, 0x05, 0x10, 0x4E, 0x6F, 0x74, 0x20, 0x73, 0x65, 0x6C, 0x66, 0x20,
         0x65, 0x78, 0x70, 0x6C, 0x61, 0x6E, 0x2E, 0x1E, 0x02, 0x01, 0x01};
    static unsigned char const resp_4_1_1b[] =
        {0x81, 0x03, 0x01, 0x15, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04};
    QTest::newRow( "LAUNCH BROWSER 4.1.1B - GCF 27.22.4.26.4" )
        << QByteArray( (char *)data_4_1_1b, sizeof(data_4_1_1b) )
        << QByteArray( (char *)resp_4_1_1b, sizeof(resp_4_1_1b) )
        << QByteArray()
        << 0x0004       // Icon not displayed
        << QString( "Not self explan." )
        << QString( "" )
        << (int)( QSimCommand::UseExisting )
        << 1 << false   // No icon
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_4_2_1a[] =
        {0xD0, 0x1D, 0x81, 0x03, 0x01, 0x15, 0x02, 0x82, 0x02, 0x81, 0x82, 0x31,
         0x00, 0x05, 0x0C, 0x53, 0x65, 0x6C, 0x66, 0x20, 0x65, 0x78, 0x70, 0x6C,
         0x61, 0x6E, 0x2E, 0x1E, 0x02, 0x00, 0x01};
    static unsigned char const resp_4_2_1a[] =
        {0x81, 0x03, 0x01, 0x15, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "LAUNCH BROWSER 4.2.1A - GCF 27.22.4.26.4" )
        << QByteArray( (char *)data_4_2_1a, sizeof(data_4_2_1a) )
        << QByteArray( (char *)resp_4_2_1a, sizeof(resp_4_2_1a) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "Self explan." )
        << QString( "" )
        << (int)( QSimCommand::UseExisting )
        << 1 << true    // No icon
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_4_2_1b[] =
        {0xD0, 0x1D, 0x81, 0x03, 0x01, 0x15, 0x02, 0x82, 0x02, 0x81, 0x82, 0x31,
         0x00, 0x05, 0x0C, 0x53, 0x65, 0x6C, 0x66, 0x20, 0x65, 0x78, 0x70, 0x6C,
         0x61, 0x6E, 0x2E, 0x1E, 0x02, 0x00, 0x01};
    static unsigned char const resp_4_2_1b[] =
        {0x81, 0x03, 0x01, 0x15, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04};
    QTest::newRow( "LAUNCH BROWSER 4.2.1B - GCF 27.22.4.26.4" )
        << QByteArray( (char *)data_4_2_1b, sizeof(data_4_2_1b) )
        << QByteArray( (char *)resp_4_2_1b, sizeof(resp_4_2_1b) )
        << QByteArray()
        << 0x0004       // Icon not displayed
        << QString( "Self explan." )
        << QString( "" )
        << (int)( QSimCommand::UseExisting )
        << 1 << true    // No icon
        << (int)( QSimCommand::NoPduOptions );
}
