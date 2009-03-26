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
// Test encoding and decoding of GET INKEY commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.2 - GET INKEY.
void tst_QSimToolkit::testEncodeGetInkey_data()
{
    QSimToolkitData::populateDataGetInkey();
}
void tst_QSimToolkit::testEncodeGetInkey()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( int, qualifiers );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( QString, resptext );
    QFETCH( int, options );
    QFETCH( QByteArray, textAttribute );
    QFETCH( QString, html );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::GetInkey );
    QVERIFY( decoded.destinationDevice() == QSimCommand::ME );
    QCOMPARE( decoded.text(), text );
    if ( (qualifiers & InputQualifierWantYesNo) != 0 ) {
        QCOMPARE( decoded.wantYesNo(), true );
    } else {
        QCOMPARE( decoded.wantDigits(), (qualifiers & InputQualifierWantDigits) != 0 );
        QCOMPARE( decoded.ucs2Input(), (qualifiers & InputQualifierUcs2Input) != 0 );
        QCOMPARE( decoded.wantYesNo(), false );
    }
    QCOMPARE( decoded.hasHelp(), (qualifiers & InputQualifierHasHelp) != 0 );
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
    QCOMPARE( decodedResp.text(), resptext );
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

// Test that GET INKEY commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverGetInkey_data()
{
    QSimToolkitData::populateDataGetInkey();
}
void tst_QSimToolkit::testDeliverGetInkey()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( int, qualifiers );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( QString, resptext );
    QFETCH( int, options );
    QFETCH( QByteArray, textAttribute );
    QFETCH( QString, html );

    Q_UNUSED(html);

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::GetInkey );
    cmd.setText( text );
    if ( (qualifiers & InputQualifierWantYesNo) != 0 ) {
        cmd.setWantYesNo( true );
    } else {
        cmd.setWantDigits( (qualifiers & InputQualifierWantDigits) != 0 );
        cmd.setUcs2Input( (qualifiers & InputQualifierUcs2Input) != 0 );
    }
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );
    cmd.setHasHelp( (qualifiers & InputQualifierHasHelp) != 0 );
    cmd.setTextAttribute( textAttribute );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QVERIFY( deliveredCommand.text() == cmd.text() );
    QVERIFY( deliveredCommand.wantYesNo() == cmd.wantYesNo() );
    QVERIFY( deliveredCommand.wantDigits() == cmd.wantDigits() );
    QVERIFY( deliveredCommand.ucs2Input() == cmd.ucs2Input() );
    QVERIFY( deliveredCommand.iconId() == cmd.iconId() );
    QVERIFY( deliveredCommand.iconSelfExplanatory() == cmd.iconSelfExplanatory() );
    QVERIFY( deliveredCommand.hasHelp() == cmd.hasHelp() );
    QVERIFY( deliveredCommand.textAttribute() == cmd.textAttribute() );
    QCOMPARE( deliveredCommand.toPdu( (QSimCommand::ToPduOptions)options ), data );

    // There should be no responses or envelopes in the reverse direction yet.
    QCOMPARE( server->responseCount(), 0 );
    QCOMPARE( server->envelopeCount(), 0 );

    // Compose and send the response.
    QSimTerminalResponse response;
    response.setCommand( deliveredCommand );
    response.setText( resptext );
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

// Test the user interface in "simapp" for GET INKEY.
void tst_QSimToolkit::testUIGetInkey_data()
{
    QSimToolkitData::populateDataGetInkey();
}

void tst_QSimToolkit::testUIGetInkey()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( int, qualifiers );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( QString, resptext );
    QFETCH( int, options );
    QFETCH( QByteArray, textAttribute );
    QFETCH( QString, html );

    Q_UNUSED(html);
    Q_UNUSED(options);

    // Skip tests that we cannot test using the "simapp" UI.
    if ( resptype == 0x0004 ||      // Icon not displayed
         resptype == 0x0010 ||      // Proactive SIM session terminated
         resptype == 0x0011 ) {     // Backward move
        QSKIP( "", SkipSingle );
    }

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Create the command to be tested.
    QSimCommand cmd;
    cmd.setType( QSimCommand::GetInkey );
    cmd.setText( text );
    if ( (qualifiers & InputQualifierWantYesNo) != 0 ) {
        cmd.setWantYesNo( true );
    } else {
        cmd.setWantDigits( (qualifiers & InputQualifierWantDigits) != 0 );
        cmd.setUcs2Input( (qualifiers & InputQualifierUcs2Input) != 0 );
    }
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );
    cmd.setHasHelp( (qualifiers & InputQualifierHasHelp) != 0 );
    cmd.setTextAttribute( textAttribute );

    // Set up the server with the command, ready to be selected
    // from the "Run Test" menu item on the test menu.
    server->startUsingTestMenu( cmd );
    QVERIFY( waitForView( SimMenu::staticMetaObject ) );

    // Clear the server state just before we request the actual command under test.
    server->clear();

    // Select the first menu item.
    select();

    // Wait for the text to display.
    QVERIFY( waitForView( SimInKey::staticMetaObject ) );

    // Wait for icons to load.  Should be fast because they are in-process.
    if ( iconId != 0 )
        msleep(1000);

    // Determine what needs to be done next.
    if ( resptype == 0x0012 ) {
        // No response from user test.  This will normally take 2 minutes,
        // but that's a very long time to wait.  We change it to 5 seconds.
        qDebug() << "Waiting 5 seconds for no-response indication";
        qobject_cast<SimInKey *>( currentView )->setNoResponseTimeout( 5000 );
        QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 5100 ) );
    } else if ( resptype == 0x0013 ) { // Help information requested by user
        keyClick( Qt::Key_Context1 );
        QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 5100 ) );
    } else { // success
        if ( resptext == "+" ) {
            keyClick( Qt::Key_Plus, "+" );
        } else if ( resptext == "0" ) {
            keyClick( Qt::Key_0, "0" );
        } else if ( resptext == "q" ) {
            keyClick( Qt::Key_Q, resptext );
        } else if ( resptext == "x" ) {
            keyClick( Qt::Key_X, resptext );
        } else if ( resptext == QString( QChar(0x0414) ) ) {
            keyClick( Qt::Key_unknown, resptext );
        } else if ( ( qualifiers & InputQualifierWantYesNo ) != 0 ) {
            if ( resptext == "Yes" )
                keyClick( Qt::Key_Context1 );
            else
                keyClick( Qt::Key_Back );
        }
        QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 100 ) );
    }

    // Check that the response is what we expected.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    QCOMPARE( server->lastResponse(), resp );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for GET INKEY from the GCF test cases
// in GSM 51.010, section 27.22.4.2.
void QSimToolkitData::populateDataGetInkey()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<QString>("text");
    QTest::addColumn<int>("qualifiers");
    QTest::addColumn<int>("iconId");
    QTest::addColumn<bool>("iconSelfExplanatory");
    QTest::addColumn<QString>("resptext");
    QTest::addColumn<int>("options");
    QTest::addColumn<QByteArray>("textAttribute");
    QTest::addColumn<QString>("html");
/*
    QTest::addColumn<QByteArray>("helpdata");
    QTest::addColumn<QByteArray>("helpresp");
*/

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x15, 0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x22, 0x2B, 0x22};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INKEY 1.1.1 - GCF 27.22.4.2.1" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Enter \"+\"" )
        << InputQualifierWantDigits
        << 0 << false   // No icon
        << QString( "+" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x14, 0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x09, 0x00, 0x45, 0x37, 0xBD, 0x2C, 0x07, 0x89, 0x60, 0x22};
    static unsigned char const resp_1_2_1[] =
        {0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x30};
    QTest::newRow( "GET INKEY 1.2.1 - GCF 27.22.4.2.1" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << 0x0000       // Command performed successfully
        << QString( "Enter \"0\"" )
        << InputQualifierWantDigits
        << 0 << false   // No icon
        << QString( "0" )
        << (int)( QSimCommand::PackedStrings )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_1_3_1[] =
        {0xD0, 0x1A, 0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0F, 0x04, 0x3C, 0x47, 0x4F, 0x2D, 0x42, 0x41, 0x43, 0x4B, 0x57, 0x41,
         0x52, 0x44, 0x53, 0x3E};
    static unsigned char const resp_1_3_1[] =
        {0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x11};
    QTest::newRow( "GET INKEY 1.3.1 - GCF 27.22.4.2.1" )
        << QByteArray( (char *)data_1_3_1, sizeof(data_1_3_1) )
        << QByteArray( (char *)resp_1_3_1, sizeof(resp_1_3_1) )
        << 0x0011       // Backward move
        << QString( "<GO-BACKWARDS>" )
        << InputQualifierWantDigits
        << 0 << false   // No icon
        << QString( "" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_1_4_1[] =
        {0xD0, 0x13, 0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x08, 0x04, 0x3C, 0x41, 0x42, 0x4F, 0x52, 0x54, 0x3E};
    static unsigned char const resp_1_4_1[] =
        {0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x10};
    QTest::newRow( "GET INKEY 1.4.1 - GCF 27.22.4.2.1" )
        << QByteArray( (char *)data_1_4_1, sizeof(data_1_4_1) )
        << QByteArray( (char *)resp_1_4_1, sizeof(resp_1_4_1) )
        << 0x0010       // Proactive SIM session terminated
        << QString( "<ABORT>" )
        << InputQualifierWantDigits
        << 0 << false   // No icon
        << QString( "" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_1_5_1[] =
        {0xD0, 0x15, 0x81, 0x03, 0x01, 0x22, 0x01, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x22, 0x71, 0x22};
    static unsigned char const resp_1_5_1[] =
        {0x81, 0x03, 0x01, 0x22, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x71};
    QTest::newRow( "GET INKEY 1.5.1 - GCF 27.22.4.2.1" )
        << QByteArray( (char *)data_1_5_1, sizeof(data_1_5_1) )
        << QByteArray( (char *)resp_1_5_1, sizeof(resp_1_5_1) )
        << 0x0000       // Command performed successfully
        << QString( "Enter \"q\"" )
        << InputQualifierNone
        << 0 << false   // No icon
        << QString( "q" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_1_6_1[] =
        {0xD0, 0x81, 0xAD, 0x81, 0x03, 0x01, 0x22, 0x01, 0x82, 0x02, 0x81, 0x82,
         0x8D, 0x81, 0xA1, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x22, 0x78,
         0x22, 0x2E, 0x20, 0x54, 0x68, 0x69, 0x73, 0x20, 0x63, 0x6F, 0x6D, 0x6D,
         0x61, 0x6E, 0x64, 0x20, 0x69, 0x6E, 0x73, 0x74, 0x72, 0x75, 0x63, 0x74,
         0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x4D, 0x45, 0x20, 0x74, 0x6F, 0x20,
         0x64, 0x69, 0x73, 0x70, 0x6C, 0x61, 0x79, 0x20, 0x74, 0x65, 0x78, 0x74,
         0x2C, 0x20, 0x61, 0x6E, 0x64, 0x20, 0x74, 0x6F, 0x20, 0x65, 0x78, 0x70,
         0x65, 0x63, 0x74, 0x20, 0x74, 0x68, 0x65, 0x20, 0x75, 0x73, 0x65, 0x72,
         0x20, 0x74, 0x6F, 0x20, 0x65, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x61, 0x20,
         0x73, 0x69, 0x6E, 0x67, 0x6C, 0x65, 0x20, 0x63, 0x68, 0x61, 0x72, 0x61,
         0x63, 0x74, 0x65, 0x72, 0x2E, 0x20, 0x41, 0x6E, 0x79, 0x20, 0x72, 0x65,
         0x73, 0x70, 0x6F, 0x6E, 0x73, 0x65, 0x20, 0x65, 0x6E, 0x74, 0x65, 0x72,
         0x65, 0x64, 0x20, 0x62, 0x79, 0x20, 0x74, 0x68, 0x65, 0x20, 0x75, 0x73,
         0x65, 0x72, 0x20, 0x73, 0x68, 0x61, 0x6C, 0x6C, 0x20, 0x62, 0x65, 0x20,
         0x70, 0x61, 0x73, 0x73, 0x65, 0x64, 0x20, 0x74};
    static unsigned char const resp_1_6_1[] =
        {0x81, 0x03, 0x01, 0x22, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x78};
    QTest::newRow( "GET INKEY 1.6.1 - GCF 27.22.4.2.1" )
        << QByteArray( (char *)data_1_6_1, sizeof(data_1_6_1) )
        << QByteArray( (char *)resp_1_6_1, sizeof(resp_1_6_1) )
        << 0x0000       // Command performed successfully
        << QString( "Enter \"x\". This command instructs the ME to display text, "
                    "and to expect the user to enter a single character. Any response "
                    "entered by the user shall be passed t" )
        << InputQualifierNone
        << 0 << false   // No icon
        << QString( "x" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_2_1_1[] =
        {0xD0, 0x16, 0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0B, 0x04, 0x3C, 0x54, 0x49, 0x4D, 0x45, 0x2D, 0x4F, 0x55, 0x54, 0x3E};
    static unsigned char const resp_2_1_1[] =
        {0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x12};
    QTest::newRow( "GET INKEY 2.1.1 - GCF 27.22.4.2.2" )
        << QByteArray( (char *)data_2_1_1, sizeof(data_2_1_1) )
        << QByteArray( (char *)resp_2_1_1, sizeof(resp_2_1_1) )
        << 0x0012       // No response from user
        << QString( "<TIME-OUT>" )
        << InputQualifierWantDigits
        << 0 << false   // No icon
        << QString( "" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_3_1_1[] =
        {0xD0, 0x24, 0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x19, 0x08, 0x04, 0x17, 0x04, 0x14, 0x04, 0x20, 0x04, 0x10, 0x04, 0x12,
         0x04, 0x21, 0x04, 0x22, 0x04, 0x12, 0x04, 0x23, 0x04, 0x19, 0x04, 0x22,
         0x04, 0x15};
    static unsigned char const resp_3_1_1[] =
        {0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x2B};
    static ushort const str_3_1_1[] =
        {0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423,
         0x0419, 0x0422, 0x0415};
    QTest::newRow( "GET INKEY 3.1.1 - GCF 27.22.4.2.3" )
        << QByteArray( (char *)data_3_1_1, sizeof(data_3_1_1) )
        << QByteArray( (char *)resp_3_1_1, sizeof(resp_3_1_1) )
        << 0x0000       // Command performed successfully
        << QString::fromUtf16( str_3_1_1, sizeof(str_3_1_1) / sizeof(ushort) )
        << InputQualifierWantDigits
        << 0 << false   // No icon
        << QString( "+" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_3_2_1[] =
        {0xD0, 0x81, 0x99, 0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x81, 0x82,
         0x8D, 0x81, 0x8D, 0x08, 0x04, 0x17, 0x04, 0x14, 0x04, 0x20, 0x04, 0x10,
         0x04, 0x12, 0x04, 0x21, 0x04, 0x22, 0x04, 0x12, 0x04, 0x23, 0x04, 0x19,
         0x04, 0x22, 0x04, 0x15, 0x04, 0x17, 0x04, 0x14, 0x04, 0x20, 0x04, 0x10,
         0x04, 0x12, 0x04, 0x21, 0x04, 0x22, 0x04, 0x12, 0x04, 0x23, 0x04, 0x19,
         0x04, 0x22, 0x04, 0x15, 0x04, 0x17, 0x04, 0x14, 0x04, 0x20, 0x04, 0x10,
         0x04, 0x12, 0x04, 0x21, 0x04, 0x22, 0x04, 0x12, 0x04, 0x23, 0x04, 0x19,
         0x04, 0x22, 0x04, 0x15, 0x04, 0x17, 0x04, 0x14, 0x04, 0x20, 0x04, 0x10,
         0x04, 0x12, 0x04, 0x21, 0x04, 0x22, 0x04, 0x12, 0x04, 0x23, 0x04, 0x19,
         0x04, 0x22, 0x04, 0x15, 0x04, 0x17, 0x04, 0x14, 0x04, 0x20, 0x04, 0x10,
         0x04, 0x12, 0x04, 0x21, 0x04, 0x22, 0x04, 0x12, 0x04, 0x23, 0x04, 0x19,
         0x04, 0x22, 0x04, 0x15, 0x04, 0x17, 0x04, 0x14, 0x04, 0x20, 0x04, 0x10,
         0x04, 0x12, 0x04, 0x21, 0x04, 0x22, 0x04, 0x12, 0x04, 0x23, 0x04, 0x19};
    static unsigned char const resp_3_2_1[] =
        {0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x2B};
    static ushort const str_3_2_1[] =
        {0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423,
         0x0419, 0x0422, 0x0415, 0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421,
         0x0422, 0x0412, 0x0423, 0x0419, 0x0422, 0x0415, 0x0417, 0x0414, 0x0420,
         0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423, 0x0419, 0x0422, 0x0415,
         0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423,
         0x0419, 0x0422, 0x0415, 0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421,
         0x0422, 0x0412, 0x0423, 0x0419, 0x0422, 0x0415, 0x0417, 0x0414, 0x0420,
         0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423, 0x0419};
    QTest::newRow( "GET INKEY 3.2.1 - GCF 27.22.4.2.3" )
        << QByteArray( (char *)data_3_2_1, sizeof(data_3_2_1) )
        << QByteArray( (char *)resp_3_2_1, sizeof(resp_3_2_1) )
        << 0x0000       // Command performed successfully
        << QString::fromUtf16( str_3_2_1, sizeof(str_3_2_1) / sizeof(ushort) )
        << InputQualifierWantDigits
        << 0 << false   // No icon
        << QString( "+" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_4_1_1[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x22, 0x03, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x06, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72};
    static unsigned char const resp_4_1_1[] =
        {0x81, 0x03, 0x01, 0x22, 0x03, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x03, 0x08, 0x04, 0x14};
    QTest::newRow( "GET INKEY 4.1.1 - GCF 27.22.4.2.4" )
        << QByteArray( (char *)data_4_1_1, sizeof(data_4_1_1) )
        << QByteArray( (char *)resp_4_1_1, sizeof(resp_4_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Enter" )
        << InputQualifierUcs2Input
        << 0 << false   // No icon
        << QString( QChar(0x0414) )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_5_1_1[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x22, 0x04, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x06, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72};
    static unsigned char const resp_5_1_1[] =
        {0x81, 0x03, 0x01, 0x22, 0x04, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x01};
    QTest::newRow( "GET INKEY 5.1.1 - GCF 27.22.4.2.5" )
        << QByteArray( (char *)data_5_1_1, sizeof(data_5_1_1) )
        << QByteArray( (char *)resp_5_1_1, sizeof(resp_5_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Enter" )
        << InputQualifierWantYesNo
        << 0 << false   // No icon
        << QString( "Yes" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_5_1_2[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x22, 0x04, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x06, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72};
    static unsigned char const resp_5_1_2[] =
        {0x81, 0x03, 0x01, 0x22, 0x04, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x00};
    QTest::newRow( "GET INKEY 5.1.2 - GCF 27.22.4.2.5" )
        << QByteArray( (char *)data_5_1_2, sizeof(data_5_1_2) )
        << QByteArray( (char *)resp_5_1_2, sizeof(resp_5_1_2) )
        << 0x0000       // Command performed successfully
        << QString( "Enter" )
        << InputQualifierWantYesNo
        << 0 << false   // No icon
        << QString( "No" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_6_1_1a[] =
        {0xD0, 0x19, 0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x3C, 0x4E, 0x4F, 0x2D, 0x49, 0x43, 0x4F, 0x4E, 0x3E, 0x1E,
         0x02, 0x00, 0x01};
    static unsigned char const resp_6_1_1a[] =
        {0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INKEY 6.1.1A - GCF 27.22.4.2.6" )
        << QByteArray( (char *)data_6_1_1a, sizeof(data_6_1_1a) )
        << QByteArray( (char *)resp_6_1_1a, sizeof(resp_6_1_1a) )
        << 0x0000       // Command performed successfully
        << QString( "<NO-ICON>" )
        << InputQualifierWantDigits
        << 1 << true    // Icon details
        << QString( "+" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_6_1_1b[] =
        {0xD0, 0x19, 0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x3C, 0x4E, 0x4F, 0x2D, 0x49, 0x43, 0x4F, 0x4E, 0x3E, 0x1E,
         0x02, 0x00, 0x01};
    static unsigned char const resp_6_1_1b[] =
        {0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INKEY 6.1.1B - GCF 27.22.4.2.6" )
        << QByteArray( (char *)data_6_1_1b, sizeof(data_6_1_1b) )
        << QByteArray( (char *)resp_6_1_1b, sizeof(resp_6_1_1b) )
        << 0x0004       // Icon not displayed
        << QString( "<NO-ICON>" )
        << InputQualifierWantDigits
        << 1 << true    // Icon details
        << QString( "+" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_6_2_1a[] =
        {0xD0, 0x1C, 0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0D, 0x04, 0x3C, 0x42, 0x41, 0x53, 0x49, 0x43, 0x2D, 0x49, 0x43, 0x4F,
         0x4E, 0x3E, 0x1E, 0x02, 0x01, 0x01};
    static unsigned char const resp_6_2_1a[] =
        {0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INKEY 6.2.1A - GCF 27.22.4.2.6" )
        << QByteArray( (char *)data_6_2_1a, sizeof(data_6_2_1a) )
        << QByteArray( (char *)resp_6_2_1a, sizeof(resp_6_2_1a) )
        << 0x0000       // Command performed successfully
        << QString( "<BASIC-ICON>" )
        << InputQualifierWantDigits
        << 1 << false   // Icon details
        << QString( "+" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_6_2_1b[] =
        {0xD0, 0x1C, 0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0D, 0x04, 0x3C, 0x42, 0x41, 0x53, 0x49, 0x43, 0x2D, 0x49, 0x43, 0x4F,
         0x4E, 0x3E, 0x1E, 0x02, 0x01, 0x01};
    static unsigned char const resp_6_2_1b[] =
        {0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INKEY 6.2.1B - GCF 27.22.4.2.6" )
        << QByteArray( (char *)data_6_2_1b, sizeof(data_6_2_1b) )
        << QByteArray( (char *)resp_6_2_1b, sizeof(resp_6_2_1b) )
        << 0x0004       // Icon not displayed
        << QString( "<BASIC-ICON>" )
        << InputQualifierWantDigits
        << 1 << false   // Icon details
        << QString( "+" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_6_3_1a[] =
        {0xD0, 0x19, 0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x3C, 0x4E, 0x4F, 0x2D, 0x49, 0x43, 0x4F, 0x4E, 0x3E, 0x1E,
         0x02, 0x00, 0x02};
    static unsigned char const resp_6_3_1a[] =
        {0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INKEY 6.3.1A - GCF 27.22.4.2.6" )
        << QByteArray( (char *)data_6_3_1a, sizeof(data_6_3_1a) )
        << QByteArray( (char *)resp_6_3_1a, sizeof(resp_6_3_1a) )
        << 0x0000       // Command performed successfully
        << QString( "<NO-ICON>" )
        << InputQualifierWantDigits
        << 2 << true    // Icon details
        << QString( "+" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_6_3_1b[] =
        {0xD0, 0x19, 0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x3C, 0x4E, 0x4F, 0x2D, 0x49, 0x43, 0x4F, 0x4E, 0x3E, 0x1E,
         0x02, 0x00, 0x02};
    static unsigned char const resp_6_3_1b[] =
        {0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INKEY 6.3.1B - GCF 27.22.4.2.6" )
        << QByteArray( (char *)data_6_3_1b, sizeof(data_6_3_1b) )
        << QByteArray( (char *)resp_6_3_1b, sizeof(resp_6_3_1b) )
        << 0x0004       // Icon not displayed
        << QString( "<NO-ICON>" )
        << InputQualifierWantDigits
        << 2 << true    // Icon details
        << QString( "+" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_6_4_1a[] =
        {0xD0, 0x1D, 0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0E, 0x04, 0x3C, 0x43, 0x4F, 0x4C, 0x4F, 0x55, 0x52, 0x2D, 0x49, 0x43,
         0x4F, 0x4E, 0x3E, 0x1E, 0x02, 0x01, 0x02};
    static unsigned char const resp_6_4_1a[] =
        {0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INKEY 6.4.1A - GCF 27.22.4.2.6" )
        << QByteArray( (char *)data_6_4_1a, sizeof(data_6_4_1a) )
        << QByteArray( (char *)resp_6_4_1a, sizeof(resp_6_4_1a) )
        << 0x0000       // Command performed successfully
        << QString( "<COLOUR-ICON>" )
        << InputQualifierWantDigits
        << 2 << false   // Icon details
        << QString( "+" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_6_4_1b[] =
        {0xD0, 0x1D, 0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0E, 0x04, 0x3C, 0x43, 0x4F, 0x4C, 0x4F, 0x55, 0x52, 0x2D, 0x49, 0x43,
         0x4F, 0x4E, 0x3E, 0x1E, 0x02, 0x01, 0x02};
    static unsigned char const resp_6_4_1b[] =
        {0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INKEY 6.4.1B - GCF 27.22.4.2.6" )
        << QByteArray( (char *)data_6_4_1b, sizeof(data_6_4_1b) )
        << QByteArray( (char *)resp_6_4_1b, sizeof(resp_6_4_1b) )
        << 0x0004       // Icon not displayed
        << QString( "<COLOUR-ICON>" )
        << InputQualifierWantDigits
        << 2 << false   // Icon details
        << QString( "+" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    static unsigned char const data_7_1_1[] =
        {0xD0, 0x15, 0x81, 0x03, 0x01, 0x22, 0x80, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x22, 0x2B, 0x22};
    static unsigned char const resp_7_1_1[] =
        {0x81, 0x03, 0x01, 0x22, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x13};
/*
    static unsigned char const help_7_1_1[] =
        {0xD0, 0x1C, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x11, 0x04, 0x48, 0x65, 0x6C, 0x70, 0x20, 0x69, 0x6E, 0x66, 0x6F, 0x72,
         0x6D, 0x61, 0x74, 0x69, 0x6F, 0x6E};
    static unsigned char const hrsp_7_1_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
*/
    QTest::newRow( "GET INKEY 7.1.1 - GCF 27.22.4.2.7" )
        << QByteArray( (char *)data_7_1_1, sizeof(data_7_1_1) )
        << QByteArray( (char *)resp_7_1_1, sizeof(resp_7_1_1) )
        << 0x0013       // Help information requested by user
        << QString( "Enter \"+\"" )
        << (InputQualifierWantDigits | InputQualifierHasHelp)
        << 0 << false   // No icon
        << QString( "" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information
/*
        << QByteArray( (char *)help_7_1_1, sizeof(help_7_1_1) )
        << QByteArray( (char *)hrsp_7_1_1, sizeof(hrsp_7_1_1) )
*/

    static unsigned char const data_7_1_2[] =
        {0xD0, 0x15, 0x81, 0x03, 0x01, 0x22, 0x80, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x22, 0x2B, 0x22};
    static unsigned char const resp_7_1_2[] =
        {0x81, 0x03, 0x01, 0x22, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INKEY 7.1.2 - GCF 27.22.4.2.7" )
        << QByteArray( (char *)data_7_1_2, sizeof(data_7_1_2) )
        << QByteArray( (char *)resp_7_1_2, sizeof(resp_7_1_2) )
        << 0x0000       // Command performed successfully
        << QString( "Enter \"+\"" )
        << (InputQualifierWantDigits | InputQualifierHasHelp)
        << 0 << false   // No icon
        << QString( "+" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray() << QString();       // No text attribute information

    // Test only one of the text attribute test cases.  We assume that if
    // one works, then they will all work.  The DISPLAY TEXT command tests
    // the formatting rules.
    static unsigned char const data_9_1_1[] =
        {0xD0, 0x1B, 0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x22, 0x2B, 0x22, 0xD0,
         0x04, 0x00, 0x09, 0x00, 0xB4};
    static unsigned char const resp_9_1_1[] =
        {0x81, 0x03, 0x01, 0x22, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x2B};
    static unsigned char const attr_9_1_1[] =
        {0x00, 0x09, 0x00, 0xB4};
    QTest::newRow( "GET INKEY 9.1.1 - GCF 27.22.4.2.9" )
        << QByteArray( (char *)data_9_1_1, sizeof(data_9_1_1) )
        << QByteArray( (char *)resp_9_1_1, sizeof(resp_9_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Enter \"+\"" )
        << InputQualifierWantDigits
        << 0 << false   // No icon
        << QString( "+" )
        << (int)( QSimCommand::NoPduOptions )
        << QByteArray( (char *)attr_9_1_1, sizeof(attr_9_1_1) )
        << QString( "<body bgcolor=\"#FFFF00\"><div align=\"left\"><font color=\"#008000\">Enter \"+\"</font></div></body>" );
}
