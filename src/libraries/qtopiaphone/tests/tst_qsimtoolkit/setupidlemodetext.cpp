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
#include <qvaluespace.h>

#ifndef SYSTEMTEST

// Test encoding and decoding of SET UP IDLE MODE TEXT commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.22 - SET UP IDLE MODE TEXT.
void tst_QSimToolkit::testEncodeSetupIdleModeText_data()
{
    QSimToolkitData::populateDataSetupIdleModeText();
}
void tst_QSimToolkit::testEncodeSetupIdleModeText()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::SetupIdleModeText );
    QVERIFY( decoded.destinationDevice() == QSimCommand::ME );
    QCOMPARE( decoded.text(), text );
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

// Test that SET UP IDLE MODE TEXT commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverSetupIdleModeText_data()
{
    QSimToolkitData::populateDataSetupIdleModeText();
}
void tst_QSimToolkit::testDeliverSetupIdleModeText()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
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
    cmd.setType( QSimCommand::SetupIdleModeText );
    cmd.setText( text );
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QVERIFY( deliveredCommand.text() == cmd.text() );
    QVERIFY( deliveredCommand.iconId() == cmd.iconId() );
    QVERIFY( deliveredCommand.iconSelfExplanatory() == cmd.iconSelfExplanatory() );
    QCOMPARE( deliveredCommand.toPdu( (QSimCommand::ToPduOptions)options ), data );

    // The terminal response should have been sent immediately to ack reception of the command.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    if ( resptype == 0x0000 ) {
        QCOMPARE( server->lastResponse(), resp );
    } else {
        // We cannot test the "icon not displayed" or "command data not understood" cases
        // because the qtopiaphone library will always respond with "command performed
        // successfully".  Presumably the Qtopia user interface can always display icons
        // and can handle badly-formatted SetupIdleModeText parameters.
        QByteArray resp2 = resp;
        resp2[resp2.size() - 1] = 0x00;
        QCOMPARE( server->lastResponse(), resp2 );
    }
}

// Test the user interface in "simapp" for SETUP IDLE MODE TEXT.
void tst_QSimToolkit::testUISetupIdleModeText_data()
{
    QSimToolkitData::populateDataSetupIdleModeText();
}
void tst_QSimToolkit::testUISetupIdleModeText()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( int, options );

    Q_UNUSED(options);

    // Skip tests that we cannot test using the "simapp" UI.
    if ( resptype == 0x0004 ||      // Icon not displayed
         resptype == 0x0032 ) {     // Command data not understood
        QSKIP( "", SkipSingle );
    }

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Create the command to be tested.
    QSimCommand cmd;
    cmd.setType( QSimCommand::SetupIdleModeText );
    cmd.setText( text );
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );

    // Set up the server with the command, ready to be selected
    // from the "Run Test" menu item on the test menu.
    server->startUsingTestMenu( cmd );
    QVERIFY( waitForView( SimMenu::staticMetaObject ) );

    // Clear the server state just before we request the actual command under test.
    server->clear();

    // Select the first menu item.
    QFutureSignal fs( server, SIGNAL(responseSeen()) );
    select();

    // The terminal response should have been sent immediately to ack reception of the command.
    QVERIFY( fs.wait(100) );
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    QCOMPARE( server->lastResponse(), resp );

    // Allow icon loading and value space updates to settle down.
    msleep( 1000 );

    // Check that the text was set correctly in the value space.
    // We can't check the icon because it is a QIcon in the value space
    // and we cannot easily load it at this point to check.  Assume
    // that it is OK.
    QValueSpaceItem item( "/Telephony/Status/SimToolkit" );
    QCOMPARE( item.value("IdleModeText").toString(), text );
    QCOMPARE( item.value("IdleModeIconSelfExplanatory").toBool(), iconSelfExplanatory );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for SET UP IDLE MODE TEXT from the GCF test cases
// in GSM 51.010, section 27.22.4.22.
void QSimToolkitData::populateDataSetupIdleModeText()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<QString>("text");
    QTest::addColumn<int>("iconId");
    QTest::addColumn<bool>("iconSelfExplanatory");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x1A, 0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0F, 0x04, 0x49, 0x64, 0x6C, 0x65, 0x20, 0x4D, 0x6F, 0x64, 0x65, 0x20,
         0x54, 0x65, 0x78, 0x74};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SETUP IDLE MODE TEXT 1.1.1 - GCF 27.22.4.22" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Idle Mode Text" )
        << 0 << false   // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x18, 0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0D, 0x04, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x54, 0x65,
         0x73, 0x74};
    static unsigned char const resp_1_2_1[] =
        {0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SETUP IDLE MODE TEXT 1.2.1 - GCF 27.22.4.22" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Test" )
        << 0 << false   // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1[] =
        {0xD0, 0x0B, 0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x00};
    static unsigned char const resp_1_3_1[] =
        {0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SETUP IDLE MODE TEXT 1.3.1 - GCF 27.22.4.22" )
        << QByteArray( (char *)data_1_3_1, sizeof(data_1_3_1) )
        << QByteArray( (char *)resp_1_3_1, sizeof(resp_1_3_1) )
        << 0x0000       // Command performed successfully
        << QString( "" )
        << 0 << false   // Icon details
        << (int)( QSimCommand::NoPduOptions );

    // SETUP IDLE MODE TEXT 1.4.1 is identical to 1.1.1
    // Expected behaviour is that the idle mode text will be
    // displayed again after other SIM toolkit operations have
    // come and gone.  We can't test that here.

    // SETUP IDLE MODE TEXT 1.5.1 is identical to 1.1.1
    // Expected behaviour is that if the idle mode text is set and
    // when the device is power cycled, the idle mode text will not
    // reappear when the device is powered on again unless explictly
    // set by a new idle mode text command.  We can't test that here.

    // SETUP IDLE MODE TEXT 1.6.1 is identical to 1.1.1
    // Expected behaviour is that a REFRESH with SIM initialization
    // will cause the idle mode text to be removed from the idle screen.
    // We can't test that here.

    static unsigned char const data_1_7_1[] =
        {0xD0, 0x81, 0xFD, 0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x81, 0x82,
         0x8D, 0x81, 0xF1, 0x00, 0x54, 0x74, 0x19, 0x34, 0x4D, 0x36, 0x41, 0x73,
         0x74, 0x98, 0xCD, 0x06, 0xCD, 0xEB, 0x70, 0x38, 0x3B, 0x0F, 0x0A, 0x83,
         0xE8, 0x65, 0x3C, 0x1D, 0x34, 0xA7, 0xCB, 0xD3, 0xEE, 0x33, 0x0B, 0x74,
         0x47, 0xA7, 0xC7, 0x68, 0xD0, 0x1C, 0x1D, 0x66, 0xB3, 0x41, 0xE2, 0x32,
         0x88, 0x9C, 0x9E, 0xC3, 0xD9, 0xE1, 0x7C, 0x99, 0x0C, 0x12, 0xE7, 0x41,
         0x74, 0x74, 0x19, 0xD4, 0x2C, 0x82, 0xC2, 0x73, 0x50, 0xD8, 0x0D, 0x4A,
         0x93, 0xD9, 0x65, 0x50, 0xFB, 0x4D, 0x2E, 0x83, 0xE8, 0x65, 0x3C, 0x1D,
         0x94, 0x36, 0x83, 0xE8, 0xE8, 0x32, 0xA8, 0x59, 0x04, 0xA5, 0xE7, 0xA0,
         0xB0, 0x98, 0x5D, 0x06, 0xD1, 0xDF, 0x20, 0xF2, 0x1B, 0x94, 0xA6, 0xBB,
         0xA8, 0xE8, 0x32, 0x08, 0x2E, 0x2F, 0xCF, 0xCB, 0x6E, 0x7A, 0x98, 0x9E,
         0x7E, 0xBB, 0x41, 0x73, 0x7A, 0x9E, 0x5D, 0x06, 0xA5, 0xE7, 0x20, 0x76,
         0xD9, 0x4C, 0x07, 0x85, 0xE7, 0xA0, 0xB0, 0x1B, 0x94, 0x6E, 0xC3, 0xD9,
         0xE5, 0x76, 0xD9, 0x4D, 0x0F, 0xD3, 0xD3, 0x6F, 0x37, 0x88, 0x5C, 0x1E,
         0xA7, 0xE7, 0xE9, 0xB7, 0x1B, 0x44, 0x7F, 0x83, 0xE8, 0xE8, 0x32, 0xA8,
         0x59, 0x04, 0xB5, 0xC3, 0xEE, 0xBA, 0x39, 0x3C, 0xA6, 0xD7, 0xE5, 0x65,
         0xB9, 0x0B, 0x44, 0x45, 0x97, 0x41, 0x69, 0x32, 0xBB, 0x0C, 0x6A, 0xBF,
         0xC9, 0x65, 0x10, 0xBD, 0x8C, 0xA7, 0x83, 0xE6, 0xE8, 0x30, 0x9B, 0x0D,
         0x12, 0x97, 0x41, 0xE4, 0xF4, 0x1C, 0xCE, 0x0E, 0xE7, 0xCB, 0x64, 0x50,
         0xDA, 0x0D, 0x0A, 0x83, 0xDA, 0x61, 0xB7, 0xBB, 0x2C, 0x07, 0xD1, 0xD1,
         0x61, 0x3A, 0xA8, 0xEC, 0x9E, 0xD7, 0xE5, 0xE5, 0x39, 0x88, 0x8E, 0x0E,
         0xD3, 0x41, 0xEE, 0x32};
    static unsigned char const resp_1_7_1[] =
        {0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SETUP IDLE MODE TEXT 1.7.1 - GCF 27.22.4.22" )
        << QByteArray( (char *)data_1_7_1, sizeof(data_1_7_1) )
        << QByteArray( (char *)resp_1_7_1, sizeof(resp_1_7_1) )
        << 0x0000       // Command performed successfully
        << QString( "The SIM shall supply a text string, which shall be "
                    "displayed by the ME as an idle mode text if the ME is "
                    "able to do it.The presentation style is left as an "
                    "implementation decision to the ME manufacturer. "
                    "The idle mode text shall be displayed in a manner "
                    "that ensures that ne" )
        << 0 << false   // Icon details
        << (int)( QSimCommand::PackedStrings );

    static unsigned char const data_2_1_1a[] =
        {0xD0, 0x19, 0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x49, 0x64, 0x6C, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74, 0x9E,
         0x02, 0x00, 0x01};
    static unsigned char const resp_2_1_1a[] =
        {0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SETUP IDLE MODE TEXT 2.1.1A - GCF 27.22.4.22" )
        << QByteArray( (char *)data_2_1_1a, sizeof(data_2_1_1a) )
        << QByteArray( (char *)resp_2_1_1a, sizeof(resp_2_1_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Idle text" )
        << 1 << true    // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1b[] =
        {0xD0, 0x19, 0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x49, 0x64, 0x6C, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74, 0x9E,
         0x02, 0x00, 0x01};
    static unsigned char const resp_2_1_1b[] =
        {0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04};
    QTest::newRow( "SETUP IDLE MODE TEXT 2.1.1B - GCF 27.22.4.22" )
        << QByteArray( (char *)data_2_1_1b, sizeof(data_2_1_1b) )
        << QByteArray( (char *)resp_2_1_1b, sizeof(resp_2_1_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Idle text" )
        << 1 << true    // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_2_1a[] =
        {0xD0, 0x19, 0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x49, 0x64, 0x6C, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74, 0x9E,
         0x02, 0x01, 0x01};
    static unsigned char const resp_2_2_1a[] =
        {0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SETUP IDLE MODE TEXT 2.2.1A - GCF 27.22.4.22" )
        << QByteArray( (char *)data_2_2_1a, sizeof(data_2_2_1a) )
        << QByteArray( (char *)resp_2_2_1a, sizeof(resp_2_2_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Idle text" )
        << 1 << false   // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_2_1b[] =
        {0xD0, 0x19, 0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x49, 0x64, 0x6C, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74, 0x9E,
         0x02, 0x01, 0x01};
    static unsigned char const resp_2_2_1b[] =
        {0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04};
    QTest::newRow( "SETUP IDLE MODE TEXT 2.2.1B - GCF 27.22.4.22" )
        << QByteArray( (char *)data_2_2_1b, sizeof(data_2_2_1b) )
        << QByteArray( (char *)resp_2_2_1b, sizeof(resp_2_2_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Idle text" )
        << 1 << false   // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_3_1a[] =
        {0xD0, 0x19, 0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x49, 0x64, 0x6C, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74, 0x9E,
         0x02, 0x00, 0x02};
    static unsigned char const resp_2_3_1a[] =
        {0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SETUP IDLE MODE TEXT 2.3.1A - GCF 27.22.4.22" )
        << QByteArray( (char *)data_2_3_1a, sizeof(data_2_3_1a) )
        << QByteArray( (char *)resp_2_3_1a, sizeof(resp_2_3_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Idle text" )
        << 2 << true    // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_3_1b[] =
        {0xD0, 0x19, 0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x49, 0x64, 0x6C, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74, 0x9E,
         0x02, 0x00, 0x02};
    static unsigned char const resp_2_3_1b[] =
        {0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04};
    QTest::newRow( "SETUP IDLE MODE TEXT 2.3.1B - GCF 27.22.4.22" )
        << QByteArray( (char *)data_2_3_1b, sizeof(data_2_3_1b) )
        << QByteArray( (char *)resp_2_3_1b, sizeof(resp_2_3_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Idle text" )
        << 2 << true    // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_4_1[] =
        {0xD0, 0x0F, 0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x00, 0x9E, 0x02, 0x01, 0x01};
    static unsigned char const resp_2_4_1[] =
        {0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x32};
    QTest::newRow( "SETUP IDLE MODE TEXT 2.4.1 - GCF 27.22.4.22" )
        << QByteArray( (char *)data_2_4_1, sizeof(data_2_4_1) )
        << QByteArray( (char *)resp_2_4_1, sizeof(resp_2_4_1) )
        << 0x0032       // Command data not understood
        << QString( "" )
        << 1 << false   // Icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_3_1_1[] =
        {0xD0, 0x24, 0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x19, 0x08, 0x04, 0x17, 0x04, 0x14, 0x04, 0x20, 0x04, 0x10, 0x04, 0x12,
         0x04, 0x21, 0x04, 0x22, 0x04, 0x12, 0x04, 0x23, 0x04, 0x19, 0x04, 0x22,
         0x04, 0x15};
    static unsigned char const resp_3_1_1[] =
        {0x81, 0x03, 0x01, 0x28, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static ushort const str_3_1_1[] =
        {0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423,
         0x0419, 0x0422, 0x0415};
    QTest::newRow( "SETUP IDLE MODE TEXT 3.1.1 - GCF 27.22.4.22" )
        << QByteArray( (char *)data_3_1_1, sizeof(data_3_1_1) )
        << QByteArray( (char *)resp_3_1_1, sizeof(resp_3_1_1) )
        << 0x0000       // Command performed successfully
        << QString::fromUtf16( str_3_1_1, sizeof(str_3_1_1) / sizeof(ushort) )
        << 0 << false   // Icon details
        << (int)( QSimCommand::EncodeEmptyStrings );
}
