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
// Test encoding and decoding of GET INPUT commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.3 - GET INPUT.
void tst_QSimToolkit::testEncodeGetInput_data()
{
    QSimToolkitData::populateDataGetInput();
}
void tst_QSimToolkit::testEncodeGetInput()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( QString, defaultText );
    QFETCH( int, qualifiers );
    QFETCH( int, minimumLength );
    QFETCH( int, maximumLength );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( QString, resptext );
    QFETCH( QByteArray, textAttribute );
    QFETCH( QString, html );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::GetInput );
    QVERIFY( decoded.destinationDevice() == QSimCommand::ME );
    QCOMPARE( decoded.text(), text );
    QCOMPARE( decoded.defaultText(), defaultText );
    QCOMPARE( decoded.wantDigits(), (qualifiers & InputQualifierWantDigits) != 0 );
    QCOMPARE( decoded.ucs2Input(), (qualifiers & InputQualifierUcs2Input) != 0 );
    QCOMPARE( decoded.packedInput(), (qualifiers & InputQualifierPackedInput) != 0 );
    QCOMPARE( decoded.hasHelp(), (qualifiers & InputQualifierHasHelp) != 0 );
    QCOMPARE( decoded.echo(), (qualifiers & InputQualifierEcho) != 0 );
    QCOMPARE( (int)decoded.minimumLength(), minimumLength );
    QCOMPARE( (int)decoded.maximumLength(), maximumLength );
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

// Test that GET INPUT commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverGetInputText_data()
{
    QSimToolkitData::populateDataGetInput();
}
void tst_QSimToolkit::testDeliverGetInputText()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( QString, defaultText );
    QFETCH( int, qualifiers );
    QFETCH( int, minimumLength );
    QFETCH( int, maximumLength );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( QString, resptext );
    QFETCH( QByteArray, textAttribute );
    QFETCH( QString, html );
    QFETCH( int, options );

    Q_UNUSED(html);

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::GetInput );
    cmd.setText( text );
    cmd.setDefaultText( defaultText );
    cmd.setWantDigits( (qualifiers & InputQualifierWantDigits) != 0 );
    cmd.setUcs2Input( (qualifiers & InputQualifierUcs2Input) != 0 );
    cmd.setPackedInput( (qualifiers & InputQualifierPackedInput) != 0 );
    cmd.setEcho( (qualifiers & InputQualifierEcho) != 0 );
    cmd.setMinimumLength( (uint)minimumLength );
    cmd.setMaximumLength( (uint)maximumLength );
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
    QVERIFY( deliveredCommand.defaultText() == cmd.defaultText() );
    QVERIFY( deliveredCommand.wantDigits() == cmd.wantDigits() );
    QVERIFY( deliveredCommand.ucs2Input() == cmd.ucs2Input() );
    QVERIFY( deliveredCommand.packedInput() == cmd.packedInput() );
    QVERIFY( deliveredCommand.echo() == cmd.echo() );
    QVERIFY( deliveredCommand.minimumLength() == cmd.minimumLength() );
    QVERIFY( deliveredCommand.maximumLength() == cmd.maximumLength() );
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

// Test the user interface in "simapp" for GET INPUT.
void tst_QSimToolkit::testUIGetInput_data()
{
    QSimToolkitData::populateDataGetInput();
}

void tst_QSimToolkit::testUIGetInput()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( QString, defaultText );
    QFETCH( int, qualifiers );
    QFETCH( int, minimumLength );
    QFETCH( int, maximumLength );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( QString, resptext );
    QFETCH( QByteArray, textAttribute );
    QFETCH( QString, html );
    QFETCH( int, options );

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
    cmd.setType( QSimCommand::GetInput );
    cmd.setText( text );
    cmd.setDefaultText( defaultText );
    cmd.setWantDigits( (qualifiers & InputQualifierWantDigits) != 0 );
    cmd.setUcs2Input( (qualifiers & InputQualifierUcs2Input) != 0 );
    cmd.setPackedInput( (qualifiers & InputQualifierPackedInput) != 0 );
    cmd.setEcho( (qualifiers & InputQualifierEcho) != 0 );
    cmd.setMinimumLength( (uint)minimumLength );
    cmd.setMaximumLength( (uint)maximumLength );
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );
    cmd.setHasHelp( (qualifiers & InputQualifierHasHelp) != 0 );
    cmd.setTextAttribute( textAttribute );
    server->emitCommand( cmd );

    // Enter the event loop to flush pending key events.
    msleep(0);

    // Set up the server with the command, ready to be selected
    // from the "Run Test" menu item on the test menu.
    server->startUsingTestMenu( cmd );
    QVERIFY( waitForView( SimMenu::staticMetaObject ) );

    // Clear the server state just before we request the actual command under test.
    server->clear();

    // Select the first menu item.
    select();

    // Wait for the input widget to display.
    QVERIFY( waitForView( SimInput::staticMetaObject ) );

    // Wait for icons to load.  Should be fast because they are in-process.
    if ( iconId != 0 )
        msleep(1000);

    // Determine what needs to be done next.
    if ( resptype == 0x0012 ) {
        // No response from user test.  This will normally take 2 minutes,
        // but that's a very long time to wait.  We change it to 5 seconds.
        qDebug() << "Waiting 5 seconds for no-response indication";
        qobject_cast<SimInput *>( currentView )->setNoResponseTimeout( 5000 );
        QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 5100 ) );
    } else if ( resptype == 0x0013 ) { // Help information requested by user
        keyClick( Qt::Key_Context1 );
        QTest::qWait(0);
        keyClick( Qt::Key_Select );
        QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 100 ) );
    } else { // success
        if ( !resptext.isEmpty() )
            qobject_cast<SimInput *>( currentView )->setInput( resptext );
        keyClick( Qt::Key_Select );
        QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 100 ) );
    }

    // Check that the response is what we expected.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    QCOMPARE( server->lastResponse(), resp );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for GET INPUT from the GCF test cases
// in GSM 51.010, section 27.22.4.3.
void QSimToolkitData::populateDataGetInput()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("defaultText");
    QTest::addColumn<int>("qualifiers");
    QTest::addColumn<int>("minimumLength");
    QTest::addColumn<int>("maximumLength");
    QTest::addColumn<int>("iconId");
    QTest::addColumn<bool>("iconSelfExplanatory");
    QTest::addColumn<QString>("resptext");
    QTest::addColumn<QByteArray>("textAttribute");
    QTest::addColumn<QString>("html");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x1B, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0C, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x31, 0x32, 0x33, 0x34,
         0x35, 0x91, 0x02, 0x05, 0x05};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x06, 0x04, 0x31, 0x32, 0x33, 0x34, 0x35};
    QTest::newRow( "GET INPUT 1.1.1 - GCF 27.22.4.3.1" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Enter 12345" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 5 << 5       // Min and max length
        << 0 << false   // No icon
        << QString( "12345" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x1A, 0x81, 0x03, 0x01, 0x23, 0x08, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0B, 0x00, 0x45, 0x37, 0xBD, 0x2C, 0x07, 0xD9, 0x6E, 0xAA, 0xD1, 0x0A,
         0x91, 0x02, 0x05, 0x05};
    static unsigned char const resp_1_2_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x08, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x06, 0x00, 0xB6, 0x9B, 0x6A, 0xB4, 0x02};
    QTest::newRow( "GET INPUT 1.2.1 - GCF 27.22.4.3.1" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << 0x0000       // Command performed successfully
        << QString( "Enter 67*#+" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho | InputQualifierPackedInput)
        << 5 << 5       // Min and max length
        << 0 << false   // No icon
        << QString( "67*#+" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::PackedStrings );

    static unsigned char const data_1_3_1[] =
        {0xD0, 0x1B, 0x81, 0x03, 0x01, 0x23, 0x01, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0C, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x41, 0x62, 0x43, 0x64,
         0x45, 0x91, 0x02, 0x05, 0x05};
    static unsigned char const resp_1_3_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x06, 0x04, 0x41, 0x62, 0x43, 0x64, 0x45};
    QTest::newRow( "GET INPUT 1.3.1 - GCF 27.22.4.3.1" )
        << QByteArray( (char *)data_1_3_1, sizeof(data_1_3_1) )
        << QByteArray( (char *)resp_1_3_1, sizeof(resp_1_3_1) )
        << 0x0000       // Command performed successfully
        << QString( "Enter AbCdE" )
        << QString( "" )
        << (InputQualifierEcho)
        << 5 << 5       // Min and max length
        << 0 << false   // No icon
        << QString( "AbCdE" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_1[] =
        {0xD0, 0x27, 0x81, 0x03, 0x01, 0x23, 0x04, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x18, 0x04, 0x50, 0x61, 0x73, 0x73, 0x77, 0x6F, 0x72, 0x64, 0x20, 0x31,
         0x3C, 0x53, 0x45, 0x4E, 0x44, 0x3E, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
         0x38, 0x91, 0x02, 0x04, 0x08};
    static unsigned char const resp_1_4_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x04, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x08, 0x04, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38};
    QTest::newRow( "GET INPUT 1.4.1 - GCF 27.22.4.3.1" )
        << QByteArray( (char *)data_1_4_1, sizeof(data_1_4_1) )
        << QByteArray( (char *)resp_1_4_1, sizeof(resp_1_4_1) )
        << 0x0000       // Command performed successfully
        << QString( "Password 1<SEND>2345678" )
        << QString( "" )
        << InputQualifierWantDigits
        << 4 << 8       // Min and max length
        << 0 << false   // No icon
        << QString( "2345678" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_1[] =
        {0xD0, 0x24, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x15, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x31, 0x2E, 0x2E, 0x39,
         0x2C, 0x30, 0x2E, 0x2E, 0x39, 0x2C, 0x30, 0x28, 0x31, 0x29, 0x91, 0x02,
         0x01, 0x14};
    static unsigned char const resp_1_5_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x15, 0x04, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
         0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30};
    QTest::newRow( "GET INPUT 1.5.1 - GCF 27.22.4.3.1" )
        << QByteArray( (char *)data_1_5_1, sizeof(data_1_5_1) )
        << QByteArray( (char *)resp_1_5_1, sizeof(resp_1_5_1) )
        << 0x0000       // Command performed successfully
        << QString( "Enter 1..9,0..9,0(1)" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 1 << 20      // Min and max length
        << 0 << false   // No icon
        << QString( "12345678901234567890" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_6_1[] =
        {0xD0, 0x1E, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0F, 0x04, 0x3C, 0x47, 0x4F, 0x2D, 0x42, 0x41, 0x43, 0x4B, 0x57, 0x41,
         0x52, 0x44, 0x53, 0x3E, 0x91, 0x02, 0x00, 0x08};
    static unsigned char const resp_1_6_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x11};
    QTest::newRow( "GET INPUT 1.6.1 - GCF 27.22.4.3.1" )
        << QByteArray( (char *)data_1_6_1, sizeof(data_1_6_1) )
        << QByteArray( (char *)resp_1_6_1, sizeof(resp_1_6_1) )
        << 0x0011       // Backward move
        << QString( "<GO-BACKWARDS>" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 0 << 8       // Min and max length
        << 0 << false   // No icon
        << QString( "" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_7_1[] =
        {0xD0, 0x17, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x08, 0x04, 0x3C, 0x41, 0x42, 0x4F, 0x52, 0x54, 0x3E, 0x91, 0x02, 0x00,
         0x08};
    static unsigned char const resp_1_7_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x10};
    QTest::newRow( "GET INPUT 1.7.1 - GCF 27.22.4.3.1" )
        << QByteArray( (char *)data_1_7_1, sizeof(data_1_7_1) )
        << QByteArray( (char *)resp_1_7_1, sizeof(resp_1_7_1) )
        << 0x0010       // Proactive SIM session terminated
        << QString( "<ABORT>" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 0 << 8       // Min and max length
        << 0 << false   // No icon
        << QString( "" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_8_1[] =
        {0xD0, 0x81, 0xB1, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82,
         0x8D, 0x81, 0xA1, 0x04, 0x2A, 0x2A, 0x2A, 0x31, 0x31, 0x31, 0x31, 0x31,
         0x31, 0x31, 0x31, 0x31, 0x31, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x32,
         0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x23, 0x23, 0x23,
         0x2A, 0x2A, 0x2A, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
         0x33, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x34, 0x34, 0x34, 0x34, 0x34,
         0x34, 0x34, 0x34, 0x34, 0x34, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x35,
         0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x23, 0x23, 0x23,
         0x2A, 0x2A, 0x2A, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
         0x36, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x37, 0x37, 0x37, 0x37, 0x37,
         0x37, 0x37, 0x37, 0x37, 0x37, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x38,
         0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x23, 0x23, 0x23,
         0x2A, 0x2A, 0x2A, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39,
         0x39, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x30, 0x30, 0x30, 0x30, 0x30,
         0x30, 0x30, 0x30, 0x30, 0x30, 0x23, 0x23, 0x23, 0x91, 0x02, 0xA0, 0xA0};
    static unsigned char const resp_1_8_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x81, 0xA1, 0x04, 0x2A, 0x2A, 0x2A, 0x31, 0x31, 0x31, 0x31, 0x31,
         0x31, 0x31, 0x31, 0x31, 0x31, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x32,
         0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x23, 0x23, 0x23,
         0x2A, 0x2A, 0x2A, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
         0x33, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x34, 0x34, 0x34, 0x34, 0x34,
         0x34, 0x34, 0x34, 0x34, 0x34, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x35,
         0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x23, 0x23, 0x23,
         0x2A, 0x2A, 0x2A, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
         0x36, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x37, 0x37, 0x37, 0x37, 0x37,
         0x37, 0x37, 0x37, 0x37, 0x37, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x38,
         0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x23, 0x23, 0x23,
         0x2A, 0x2A, 0x2A, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39,
         0x39, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x30, 0x30, 0x30, 0x30, 0x30,
         0x30, 0x30, 0x30, 0x30, 0x30, 0x23, 0x23, 0x23};
    QTest::newRow( "GET INPUT 1.8.1 - GCF 27.22.4.3.1" )
        << QByteArray( (char *)data_1_8_1, sizeof(data_1_8_1) )
        << QByteArray( (char *)resp_1_8_1, sizeof(resp_1_8_1) )
        << 0x0000       // Command performed successfully
        << QString( "***1111111111###***2222222222###***3333333333###"
                    "***4444444444###***5555555555###***6666666666###"
                    "***7777777777###***8888888888###***9999999999###"
                    "***0000000000###" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 160 << 160   // Min and max length
        << 0 << false   // No icon
        << QString( "***1111111111###***2222222222###***3333333333###"
                    "***4444444444###***5555555555###***6666666666###"
                    "***7777777777###***8888888888###***9999999999###"
                    "***0000000000###" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_9_1[] =
        {0xD0, 0x16, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x07, 0x04, 0x3C, 0x53, 0x45, 0x4E, 0x44, 0x3E, 0x91, 0x02, 0x00, 0x01};
    static unsigned char const resp_1_9_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x01, 0x04};
    QTest::newRow( "GET INPUT 1.9.1 - GCF 27.22.4.3.1" )
        << QByteArray( (char *)data_1_9_1, sizeof(data_1_9_1) )
        << QByteArray( (char *)resp_1_9_1, sizeof(resp_1_9_1) )
        << 0x0000       // Command performed successfully
        << QString( "<SEND>" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 0 << 1       // Min and max length
        << 0 << false   // No icon
        << QString( "" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_10_1[] =
        {0xD0, 0x0F, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x00, 0x91, 0x02, 0x01, 0x05};
    static unsigned char const resp_1_10_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x06, 0x04, 0x31, 0x32, 0x33, 0x34, 0x35};
    QTest::newRow( "GET INPUT 1.10.1 - GCF 27.22.4.3.1" )
        << QByteArray( (char *)data_1_10_1, sizeof(data_1_10_1) )
        << QByteArray( (char *)resp_1_10_1, sizeof(resp_1_10_1) )
        << 0x0000       // Command performed successfully
        << QString( "" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 1 << 5       // Min and max length
        << 0 << false   // No icon
        << QString( "12345" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1[] =
        {0xD0, 0x1A, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0B, 0x04, 0x3C, 0x54, 0x49, 0x4D, 0x45, 0x2D, 0x4F, 0x55, 0x54, 0x3E,
         0x91, 0x02, 0x00, 0x0A};
    static unsigned char const resp_2_1_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x12};
    QTest::newRow( "GET INPUT 2.1.1 - GCF 27.22.4.3.2" )
        << QByteArray( (char *)data_2_1_1, sizeof(data_2_1_1) )
        << QByteArray( (char *)resp_2_1_1, sizeof(resp_2_1_1) )
        << 0x0012       // No response from user
        << QString( "<TIME-OUT>" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 0 << 10      // Min and max length
        << 0 << false   // No icon
        << QString( "" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_3_1_1[] =
        {0xD0, 0x28, 0x81, 0x03, 0x01, 0x23, 0x01, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x19, 0x08, 0x04, 0x17, 0x04, 0x14, 0x04, 0x20, 0x04, 0x10, 0x04, 0x12,
         0x04, 0x21, 0x04, 0x22, 0x04, 0x12, 0x04, 0x23, 0x04, 0x19, 0x04, 0x22,
         0x04, 0x15, 0x91, 0x02, 0x05, 0x05};
    static unsigned char const resp_3_1_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x06, 0x04, 0x48, 0x45, 0x4C, 0x4C, 0x4F};
    static ushort const str_3_1_1[] =
        {0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423,
         0x0419, 0x0422, 0x0415};
    QTest::newRow( "GET INPUT 3.1.1 - GCF 27.22.4.3.3" )
        << QByteArray( (char *)data_3_1_1, sizeof(data_3_1_1) )
        << QByteArray( (char *)resp_3_1_1, sizeof(resp_3_1_1) )
        << 0x0000       // Command performed successfully
        << QString::fromUtf16( str_3_1_1, sizeof(str_3_1_1) / sizeof(ushort) )
        << QString( "" )
        << InputQualifierEcho
        << 5 << 5       // Min and max length
        << 0 << false   // No icon
        << QString( "HELLO" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_3_2_1[] =
        {0xD0, 0x81, 0x9D, 0x81, 0x03, 0x01, 0x23, 0x01, 0x82, 0x02, 0x81, 0x82,
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
         0x04, 0x12, 0x04, 0x21, 0x04, 0x22, 0x04, 0x12, 0x04, 0x23, 0x04, 0x19,
         0x91, 0x02, 0x05, 0x05};
    static unsigned char const resp_3_2_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x06, 0x04, 0x48, 0x45, 0x4C, 0x4C, 0x4F};
    static ushort const str_3_2_1[] =
        {0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423,
         0x0419, 0x0422, 0x0415, 0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421,
         0x0422, 0x0412, 0x0423, 0x0419, 0x0422, 0x0415, 0x0417, 0x0414, 0x0420,
         0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423, 0x0419, 0x0422, 0x0415,
         0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423,
         0x0419, 0x0422, 0x0415, 0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421,
         0x0422, 0x0412, 0x0423, 0x0419, 0x0422, 0x0415, 0x0417, 0x0414, 0x0420,
         0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423, 0x0419};
    QTest::newRow( "GET INPUT 3.2.1 - GCF 27.22.4.3.3" )
        << QByteArray( (char *)data_3_2_1, sizeof(data_3_2_1) )
        << QByteArray( (char *)resp_3_2_1, sizeof(resp_3_2_1) )
        << 0x0000       // Command performed successfully
        << QString::fromUtf16( str_3_2_1, sizeof(str_3_2_1) / sizeof(ushort) )
        << QString( "" )
        << InputQualifierEcho
        << 5 << 5       // Min and max length
        << 0 << false   // No icon
        << QString( "HELLO" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_4_1_1[] =
        {0xD0, 0x1B, 0x81, 0x03, 0x01, 0x23, 0x03, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0C, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x48, 0x65, 0x6C, 0x6C,
         0x6F, 0x91, 0x02, 0x0C, 0x0C};
    static unsigned char const resp_4_1_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x03, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x19, 0x08, 0x04, 0x17, 0x04, 0x14, 0x04, 0x20, 0x04, 0x10, 0x04,
         0x12, 0x04, 0x21, 0x04, 0x22, 0x04, 0x12, 0x04, 0x23, 0x04, 0x19, 0x04,
         0x22, 0x04, 0x15};
    static ushort const str_4_1_1[] =
        {0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423,
         0x0419, 0x0422, 0x0415};
    QTest::newRow( "GET INPUT 4.1.1 - GCF 27.22.4.3.4" )
        << QByteArray( (char *)data_4_1_1, sizeof(data_4_1_1) )
        << QByteArray( (char *)resp_4_1_1, sizeof(resp_4_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Enter Hello" )
        << QString( "" )
        << (InputQualifierEcho | InputQualifierUcs2Input)
        << 12 << 12     // Min and max length
        << 0 << false   // No icon
        << QString::fromUtf16( str_4_1_1, sizeof(str_4_1_1) / sizeof(ushort) )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_4_2_1[] =
        {0xD0, 0x1B, 0x81, 0x03, 0x01, 0x23, 0x03, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0C, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x48, 0x65, 0x6C, 0x6C,
         0x6F, 0x91, 0x02, 0x05, 0xFF};
    static unsigned char const resp_4_2_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x03, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
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
    static ushort const str_4_2_1[] =
        {0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423,
         0x0419, 0x0422, 0x0415, 0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421,
         0x0422, 0x0412, 0x0423, 0x0419, 0x0422, 0x0415, 0x0417, 0x0414, 0x0420,
         0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423, 0x0419, 0x0422, 0x0415,
         0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423,
         0x0419, 0x0422, 0x0415, 0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421,
         0x0422, 0x0412, 0x0423, 0x0419, 0x0422, 0x0415, 0x0417, 0x0414, 0x0420,
         0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423, 0x0419};
    QTest::newRow( "GET INPUT 4.2.1 - GCF 27.22.4.3.4" )
        << QByteArray( (char *)data_4_2_1, sizeof(data_4_2_1) )
        << QByteArray( (char *)resp_4_2_1, sizeof(resp_4_2_1) )
        << 0x0000       // Command performed successfully
        << QString( "Enter Hello" )
        << QString( "" )
        << (InputQualifierEcho | InputQualifierUcs2Input)
        << 5 << 255     // Min and max length
        << 0 << false   // No icon
        << QString::fromUtf16( str_4_2_1, sizeof(str_4_2_1) / sizeof(ushort) )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_5_1_1[] =
        {0xD0, 0x23, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0C, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x31, 0x32, 0x33, 0x34,
         0x35, 0x91, 0x02, 0x05, 0x05, 0x17, 0x06, 0x04, 0x31, 0x32, 0x33, 0x34,
         0x35};
    static unsigned char const resp_5_1_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x06, 0x04, 0x31, 0x32, 0x33, 0x34, 0x35};
    QTest::newRow( "GET INPUT 5.1.1 - GCF 27.22.4.3.5" )
        << QByteArray( (char *)data_5_1_1, sizeof(data_5_1_1) )
        << QByteArray( (char *)resp_5_1_1, sizeof(resp_5_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Enter 12345" )
        << QString( "12345" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 5 << 5       // Min and max length
        << 0 << false   // No icon
        << QString( "12345" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_5_2_1[] =
        {0xD0, 0x81, 0xBA, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82,
         0x8D, 0x07, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x3A, 0x91, 0x02, 0xA0,
         0xA0, 0x17, 0x81, 0xA1, 0x04, 0x2A, 0x2A, 0x2A, 0x31, 0x31, 0x31, 0x31,
         0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A,
         0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x23, 0x23,
         0x23, 0x2A, 0x2A, 0x2A, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
         0x33, 0x33, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x34, 0x34, 0x34, 0x34,
         0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A,
         0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x23, 0x23,
         0x23, 0x2A, 0x2A, 0x2A, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
         0x36, 0x36, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x37, 0x37, 0x37, 0x37,
         0x37, 0x37, 0x37, 0x37, 0x37, 0x37, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A,
         0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x23, 0x23,
         0x23, 0x2A, 0x2A, 0x2A, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39,
         0x39, 0x39, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x30, 0x30, 0x30, 0x30,
         0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x23, 0x23, 0x23};
    static unsigned char const resp_5_2_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x81, 0xA1, 0x04, 0x2A, 0x2A, 0x2A, 0x31, 0x31, 0x31, 0x31, 0x31,
         0x31, 0x31, 0x31, 0x31, 0x31, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x32,
         0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x23, 0x23, 0x23,
         0x2A, 0x2A, 0x2A, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
         0x33, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x34, 0x34, 0x34, 0x34, 0x34,
         0x34, 0x34, 0x34, 0x34, 0x34, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x35,
         0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x35, 0x23, 0x23, 0x23,
         0x2A, 0x2A, 0x2A, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
         0x36, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x37, 0x37, 0x37, 0x37, 0x37,
         0x37, 0x37, 0x37, 0x37, 0x37, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x38,
         0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x23, 0x23, 0x23,
         0x2A, 0x2A, 0x2A, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39,
         0x39, 0x23, 0x23, 0x23, 0x2A, 0x2A, 0x2A, 0x30, 0x30, 0x30, 0x30, 0x30,
         0x30, 0x30, 0x30, 0x30, 0x30, 0x23, 0x23, 0x23};
    QTest::newRow( "GET INPUT 5.2.1 - GCF 27.22.4.3.5" )
        << QByteArray( (char *)data_5_2_1, sizeof(data_5_2_1) )
        << QByteArray( (char *)resp_5_2_1, sizeof(resp_5_2_1) )
        << 0x0000       // Command performed successfully
        << QString( "Enter:" )
        << QString( "***1111111111###***2222222222###***3333333333###"
                    "***4444444444###***5555555555###***6666666666###"
                    "***7777777777###***8888888888###***9999999999###"
                    "***0000000000###" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 160 << 160   // Min and max length
        << 0 << false   // No icon
        << QString( "***1111111111###***2222222222###***3333333333###"
                    "***4444444444###***5555555555###***6666666666###"
                    "***7777777777###***8888888888###***9999999999###"
                    "***0000000000###" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_6_1_1a[] =
        {0xD0, 0x1D, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x3C, 0x4E, 0x4F, 0x2D, 0x49, 0x43, 0x4F, 0x4E, 0x3E, 0x91,
         0x02, 0x00, 0x0A, 0x1E, 0x02, 0x00, 0x01};
    static unsigned char const resp_6_1_1a[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INPUT 6.1.1A - GCF 27.22.4.3.6" )
        << QByteArray( (char *)data_6_1_1a, sizeof(data_6_1_1a) )
        << QByteArray( (char *)resp_6_1_1a, sizeof(resp_6_1_1a) )
        << 0x0000       // Command performed successfully
        << QString( "<NO-ICON>" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 0 << 10      // Min and max length
        << 1 << true    // Icon details
        << QString( "+" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_6_1_1b[] =
        {0xD0, 0x1D, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x3C, 0x4E, 0x4F, 0x2D, 0x49, 0x43, 0x4F, 0x4E, 0x3E, 0x91,
         0x02, 0x00, 0x0A, 0x1E, 0x02, 0x00, 0x01};
    static unsigned char const resp_6_1_1b[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INPUT 6.1.1B - GCF 27.22.4.3.6" )
        << QByteArray( (char *)data_6_1_1b, sizeof(data_6_1_1b) )
        << QByteArray( (char *)resp_6_1_1b, sizeof(resp_6_1_1b) )
        << 0x0004       // Icon not displayed
        << QString( "<NO-ICON>" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 0 << 10      // Min and max length
        << 1 << true    // Icon details
        << QString( "+" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_6_2_1a[] =
        {0xD0, 0x20, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0D, 0x04, 0x3C, 0x42, 0x41, 0x53, 0x49, 0x43, 0x2D, 0x49, 0x43, 0x4F,
         0x4E, 0x3E, 0x91, 0x02, 0x00, 0x0A, 0x1E, 0x02, 0x01, 0x01};
    static unsigned char const resp_6_2_1a[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INPUT 6.2.1A - GCF 27.22.4.3.6" )
        << QByteArray( (char *)data_6_2_1a, sizeof(data_6_2_1a) )
        << QByteArray( (char *)resp_6_2_1a, sizeof(resp_6_2_1a) )
        << 0x0000       // Command performed successfully
        << QString( "<BASIC-ICON>" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 0 << 10      // Min and max length
        << 1 << false   // Icon details
        << QString( "+" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_6_2_1b[] =
        {0xD0, 0x20, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0D, 0x04, 0x3C, 0x42, 0x41, 0x53, 0x49, 0x43, 0x2D, 0x49, 0x43, 0x4F,
         0x4E, 0x3E, 0x91, 0x02, 0x00, 0x0A, 0x1E, 0x02, 0x01, 0x01};
    static unsigned char const resp_6_2_1b[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INPUT 6.2.1B - GCF 27.22.4.3.6" )
        << QByteArray( (char *)data_6_2_1b, sizeof(data_6_2_1b) )
        << QByteArray( (char *)resp_6_2_1b, sizeof(resp_6_2_1b) )
        << 0x0004       // Icon not displayed
        << QString( "<BASIC-ICON>" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 0 << 10      // Min and max length
        << 1 << false   // Icon details
        << QString( "+" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_6_3_1a[] =
        {0xD0, 0x1D, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x3C, 0x4E, 0x4F, 0x2D, 0x49, 0x43, 0x4F, 0x4E, 0x3E, 0x91,
         0x02, 0x00, 0x0A, 0x1E, 0x02, 0x00, 0x02};
    static unsigned char const resp_6_3_1a[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INPUT 6.3.1A - GCF 27.22.4.3.6" )
        << QByteArray( (char *)data_6_3_1a, sizeof(data_6_3_1a) )
        << QByteArray( (char *)resp_6_3_1a, sizeof(resp_6_3_1a) )
        << 0x0000       // Command performed successfully
        << QString( "<NO-ICON>" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 0 << 10      // Min and max length
        << 2 << true    // Icon details
        << QString( "+" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_6_3_1b[] =
        {0xD0, 0x1D, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x3C, 0x4E, 0x4F, 0x2D, 0x49, 0x43, 0x4F, 0x4E, 0x3E, 0x91,
         0x02, 0x00, 0x0A, 0x1E, 0x02, 0x00, 0x02};
    static unsigned char const resp_6_3_1b[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INPUT 6.3.1B - GCF 27.22.4.3.6" )
        << QByteArray( (char *)data_6_3_1b, sizeof(data_6_3_1b) )
        << QByteArray( (char *)resp_6_3_1b, sizeof(resp_6_3_1b) )
        << 0x0004       // Icon not displayed
        << QString( "<NO-ICON>" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 0 << 10      // Min and max length
        << 2 << true    // Icon details
        << QString( "+" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_6_4_1a[] =
        {0xD0, 0x1D, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x3C, 0x4E, 0x4F, 0x2D, 0x49, 0x43, 0x4F, 0x4E, 0x3E, 0x91,
         0x02, 0x00, 0x0A, 0x1E, 0x02, 0x01, 0x02};
    static unsigned char const resp_6_4_1a[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INPUT 6.4.1A - GCF 27.22.4.3.6" )
        << QByteArray( (char *)data_6_4_1a, sizeof(data_6_4_1a) )
        << QByteArray( (char *)resp_6_4_1a, sizeof(resp_6_4_1a) )
        << 0x0000       // Command performed successfully
        << QString( "<NO-ICON>" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 0 << 10      // Min and max length
        << 2 << false   // Icon details
        << QString( "+" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_6_4_1b[] =
        {0xD0, 0x1D, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0A, 0x04, 0x3C, 0x4E, 0x4F, 0x2D, 0x49, 0x43, 0x4F, 0x4E, 0x3E, 0x91,
         0x02, 0x00, 0x0A, 0x1E, 0x02, 0x01, 0x02};
    static unsigned char const resp_6_4_1b[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04,
         0x8D, 0x02, 0x04, 0x2B};
    QTest::newRow( "GET INPUT 6.4.1B - GCF 27.22.4.3.6" )
        << QByteArray( (char *)data_6_4_1b, sizeof(data_6_4_1b) )
        << QByteArray( (char *)resp_6_4_1b, sizeof(resp_6_4_1b) )
        << 0x0004       // Icon not displayed
        << QString( "<NO-ICON>" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 0 << 10      // Min and max length
        << 2 << false   // Icon details
        << QString( "+" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_7_1_1[] =
        {0xD0, 0x1B, 0x81, 0x03, 0x01, 0x23, 0x80, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0C, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x31, 0x32, 0x33, 0x34,
         0x35, 0x91, 0x02, 0x05, 0x05};
    static unsigned char const resp_7_1_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x13};
    QTest::newRow( "GET INPUT 7.1.1 - GCF 27.22.4.3.7" )
        << QByteArray( (char *)data_7_1_1, sizeof(data_7_1_1) )
        << QByteArray( (char *)resp_7_1_1, sizeof(resp_7_1_1) )
        << 0x0013       // Help information requested by user
        << QString( "Enter 12345" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho | InputQualifierHasHelp)
        << 5 << 5       // Min and max length
        << 0 << false   // No icon
        << QString( "" )
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    // Test only one of the text attribute test cases.  We assume that if
    // one works, then they will all work.  The DISPLAY TEXT command tests
    // the formatting rules.
    static unsigned char const data_8_1_1[] =
        {0xD0, 0x21, 0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x81, 0x82, 0x8D,
         0x0C, 0x04, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x31, 0x32, 0x33, 0x34,
         0x35, 0x91, 0x02, 0x05, 0x05, 0xD0, 0x04, 0x00, 0x0B, 0x00, 0xB4};
    static unsigned char const resp_8_1_1[] =
        {0x81, 0x03, 0x01, 0x23, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x06, 0x04, 0x31, 0x32, 0x33, 0x34, 0x35};
    static unsigned char const attr_8_1_1[] =
        {0x00, 0x0B, 0x00, 0xB4};
    QTest::newRow( "GET INPUT 8.1.1 - GCF 27.22.4.3.8" )
        << QByteArray( (char *)data_8_1_1, sizeof(data_8_1_1) )
        << QByteArray( (char *)resp_8_1_1, sizeof(resp_8_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Enter 12345" )
        << QString( "" )
        << (InputQualifierWantDigits | InputQualifierEcho)
        << 5 << 5       // Min and max length
        << 0 << false   // No icon
        << QString( "12345" )
        << QByteArray( (char *)attr_8_1_1, sizeof(attr_8_1_1) )
        << QString( "<body bgcolor=\"#FFFF00\"><div align=\"left\"><font color=\"#008000\">Enter 12345</font></div></body>" )
        << (int)( QSimCommand::NoPduOptions );
}
