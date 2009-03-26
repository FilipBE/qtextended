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
#include <unistd.h>

#ifndef SYSTEMTEST

// Test encoding and decoding of DISPLAY TEXT commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.1 - DISPLAY TEXT.
void tst_QSimToolkit::testEncodeDisplayText_data()
{
    QSimToolkitData::populateDataDisplayText();
}
void tst_QSimToolkit::testEncodeDisplayText()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( bool, clearAfterDelay );
    QFETCH( bool, highPriority );
    QFETCH( bool, immediateResponse );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( QByteArray, textAttribute );
    QFETCH( QString, html );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::DisplayText );
    QVERIFY( decoded.destinationDevice() == QSimCommand::Display );
    QCOMPARE( decoded.text(), text );
    QCOMPARE( decoded.clearAfterDelay(), clearAfterDelay );
    QCOMPARE( decoded.highPriority(), highPriority );
    QCOMPARE( decoded.immediateResponse(), immediateResponse );
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

// Test that DISPLAY TEXT commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverDisplayText_data()
{
    QSimToolkitData::populateDataDisplayText();
}
void tst_QSimToolkit::testDeliverDisplayText()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( bool, clearAfterDelay );
    QFETCH( bool, highPriority );
    QFETCH( bool, immediateResponse );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( QByteArray, textAttribute );
    QFETCH( QString, html );
    QFETCH( int, options );

    Q_UNUSED( html );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::DisplayText );
    cmd.setDestinationDevice( QSimCommand::Display );
    cmd.setText( text );
    cmd.setClearAfterDelay( clearAfterDelay );
    cmd.setHighPriority( highPriority );
    cmd.setImmediateResponse( immediateResponse );
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );
    cmd.setTextAttribute( textAttribute );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QVERIFY( deliveredCommand.text() == cmd.text() );
    QVERIFY( deliveredCommand.clearAfterDelay() == cmd.clearAfterDelay() );
    QVERIFY( deliveredCommand.highPriority() == cmd.highPriority() );
    QVERIFY( deliveredCommand.immediateResponse() == cmd.immediateResponse() );
    QVERIFY( deliveredCommand.iconId() == cmd.iconId() );
    QVERIFY( deliveredCommand.iconSelfExplanatory() == cmd.iconSelfExplanatory() );
    QVERIFY( deliveredCommand.textAttribute() == cmd.textAttribute() );
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

// Test the user interface in "simapp" for DISPLAY TEXT.
void tst_QSimToolkit::testUIDisplayText_data()
{
    QSimToolkitData::populateDataDisplayText();
}
void tst_QSimToolkit::testUIDisplayText()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( bool, clearAfterDelay );
    QFETCH( bool, highPriority );
    QFETCH( bool, immediateResponse );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( QByteArray, textAttribute );
    QFETCH( QString, html );
    QFETCH( int, options );

    Q_UNUSED(options);

    // Skip tests that we cannot test using the "simapp" UI.
    if ( resptype == 0x2001 ||      // Screen busy
         resptype == 0x0004 ||      // Icon not displayed
         resptype == 0x0032 ) {     // Command data not understood
        QSKIP( "", SkipSingle );
    }

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Create the command to be tested.
    QSimCommand cmd;
    cmd.setType( QSimCommand::DisplayText );
    cmd.setDestinationDevice( QSimCommand::Display );
    cmd.setText( text );
    cmd.setClearAfterDelay( clearAfterDelay );
    cmd.setHighPriority( highPriority );
    cmd.setImmediateResponse( immediateResponse );
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );
    cmd.setTextAttribute( textAttribute );

    // Set up the server with the command, ready to be selected
    // from the "Run Test" menu item on the test menu.
    server->startUsingTestMenu( cmd );
    QVERIFY( waitForView( SimMenu::staticMetaObject ) );

    // Clear the server state just before we request the actual command under test.
    server->clear();

    // Select the first menu item.
    //select();         // FIXME - nomenu mode doesn't work for 5.1.1 yet.
    keyClick( Qt::Key_Select );

    // Wait for the text to display.
    QVERIFY( waitForView( SimText::staticMetaObject ) );

    // Wait for icons to load.  Should be fast because they are in-process.
    if ( iconId != 0 )
        msleep(1000);

    // Determine what needs to be done next.
    if ( resptype == 0x0010 ) {

        // Proactive SIM session terminated.
        QFutureSignal fs( server, SIGNAL(responseSeen()) );
        simapp->terminateSession();
        QVERIFY( fs.wait(100) );

    } else if ( resptype == 0x0012 ) {

        // No response from user test.  This will normally take 2 minutes,
        // but that's a very long time to wait.  We change it to 5 seconds.
        qDebug() << "Waiting 5 seconds for no-response indication";
        qobject_cast<SimText *>( currentView )->setNoResponseTimeout( 5000 );
        QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 5100 ) );

    } else if ( !clearAfterDelay && !immediateResponse ) {

        if ( resptype == 0x0011 )
            keyClick( Qt::Key_Back );       // Backward move
        else
            keyClick( Qt::Key_Select );
        QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 100 ) );

    } else if ( !clearAfterDelay && immediateResponse ) {

        // The response should arrive immediately without sending a click first.
        if ( server->responseCount() != 1 )
            QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 100 ) );
        QCOMPARE( server->responseCount(), 1 );
        QCOMPARE( server->envelopeCount(), 0 );
        QCOMPARE( server->lastResponse(), resp );

    } else if ( clearAfterDelay && !immediateResponse ) {

        // The response should be automatically sent after three seconds.
        qDebug() << "Waiting 3 seconds for auto-clear";
        QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 3100 ) );

    } else if ( clearAfterDelay && immediateResponse ) {

        // The response should arrive immediately without sending a click first.
        if ( server->responseCount() != 1 )
            QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 100 ) );
        QCOMPARE( server->responseCount(), 1 );
        QCOMPARE( server->envelopeCount(), 0 );
        QCOMPARE( server->lastResponse(), resp );

        // 3 seconds timeout
        QTest::qWait(3100);

        // Now the app is hidden
        QVERIFY( simapp->isVisible() == false );

        // Show the hidden app for the next test case
        simapp->show();

    } else {

        // Waiting for user to clear.  Send the "Select" key and then wait for
        // the terminal response to be sent to the server.
        keyClick( Qt::Key_Select );
        QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 100 ) );

    }

    // Check that the response is what we expected.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    QCOMPARE( server->lastResponse(), resp );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for DISPLAY TEXT from the GCF test cases
// in GSM 51.010, section 27.22.4.1.
void QSimToolkitData::populateDataDisplayText()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<QString>("text");
    QTest::addColumn<bool>("clearAfterDelay");
    QTest::addColumn<bool>("highPriority");
    QTest::addColumn<bool>("immediateResponse");
    QTest::addColumn<int>("iconId");
    QTest::addColumn<bool>("iconSelfExplanatory");
    QTest::addColumn<QByteArray>("textAttribute");
    QTest::addColumn<QString>("html");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x1A, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x0F, 0x04, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x54, 0x65,
         0x73, 0x74, 0x20, 0x31};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "DISPLAY TEXT 1.1.1 - GCF 27.22.4.1.1" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Test 1" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x1A, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x0F, 0x04, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x54, 0x65,
         0x73, 0x74, 0x20, 0x31};
    static unsigned char const resp_1_2_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x20,
         0x01};
    QTest::newRow( "DISPLAY TEXT 1.2.1 - GCF 27.22.4.1.1" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << 0x2001       // Unable to process, screen busy
        << QString( "Toolkit Test 1" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1[] =
        {0xD0, 0x1A, 0x81, 0x03, 0x01, 0x21, 0x81, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x0F, 0x04, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x54, 0x65,
         0x73, 0x74, 0x20, 0x32};
    static unsigned char const resp_1_3_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x81, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "DISPLAY TEXT 1.3.1 - GCF 27.22.4.1.1" )
        << QByteArray( (char *)data_1_3_1, sizeof(data_1_3_1) )
        << QByteArray( (char *)resp_1_3_1, sizeof(resp_1_3_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Test 2" )
        << false        // Wait for user to clear
        << true         // High priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_1[] =
        {0xD0, 0x19, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x0E, 0x00, 0xD4, 0xF7, 0x9B, 0xBD, 0x4E, 0xD3, 0x41, 0xD4, 0xF2, 0x9C,
         0x0E, 0x9A, 0x01};
    static unsigned char const resp_1_4_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "DISPLAY TEXT 1.4.1 - GCF 27.22.4.1.1" )
        << QByteArray( (char *)data_1_4_1, sizeof(data_1_4_1) )
        << QByteArray( (char *)resp_1_4_1, sizeof(resp_1_4_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Test 3" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::PackedStrings );

    static unsigned char const data_1_5_1[] =
        {0xD0, 0x1A, 0x81, 0x03, 0x01, 0x21, 0x00, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x0F, 0x04, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x54, 0x65,
         0x73, 0x74, 0x20, 0x34};
    static unsigned char const resp_1_5_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "DISPLAY TEXT 1.5.1 - GCF 27.22.4.1.1" )
        << QByteArray( (char *)data_1_5_1, sizeof(data_1_5_1) )
        << QByteArray( (char *)resp_1_5_1, sizeof(resp_1_5_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Test 4" )
        << true         // Clear after delay
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_6_1[] =
        {0xD0, 0x81, 0xAD, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02,
         0x8D, 0x81, 0xA1, 0x04, 0x54, 0x68, 0x69, 0x73, 0x20, 0x63, 0x6F, 0x6D,
         0x6D, 0x61, 0x6E, 0x64, 0x20, 0x69, 0x6E, 0x73, 0x74, 0x72, 0x75, 0x63,
         0x74, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x4D, 0x45, 0x20, 0x74, 0x6F,
         0x20, 0x64, 0x69, 0x73, 0x70, 0x6C, 0x61, 0x79, 0x20, 0x61, 0x20, 0x74,
         0x65, 0x78, 0x74, 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x2E,
         0x20, 0x49, 0x74, 0x20, 0x61, 0x6C, 0x6C, 0x6F, 0x77, 0x73, 0x20, 0x74,
         0x68, 0x65, 0x20, 0x53, 0x49, 0x4D, 0x20, 0x74, 0x6F, 0x20, 0x64, 0x65,
         0x66, 0x69, 0x6E, 0x65, 0x20, 0x74, 0x68, 0x65, 0x20, 0x70, 0x72, 0x69,
         0x6F, 0x72, 0x69, 0x74, 0x79, 0x20, 0x6F, 0x66, 0x20, 0x74, 0x68, 0x61,
         0x74, 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x2C, 0x20, 0x61,
         0x6E, 0x64, 0x20, 0x74, 0x68, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74, 0x20,
         0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x66, 0x6F, 0x72, 0x6D, 0x61,
         0x74, 0x2E, 0x20, 0x54, 0x77, 0x6F, 0x20, 0x74, 0x79, 0x70, 0x65, 0x73,
         0x20, 0x6F, 0x66, 0x20, 0x70, 0x72, 0x69, 0x6F};
    static unsigned char const resp_1_6_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "DISPLAY TEXT 1.6.1 - GCF 27.22.4.1.1" )
        << QByteArray( (char *)data_1_6_1, sizeof(data_1_6_1) )
        << QByteArray( (char *)resp_1_6_1, sizeof(resp_1_6_1) )
        << 0x0000       // Command performed successfully
        << QString( "This command instructs the ME to display a text message. It "
                    "allows the SIM to define the priority of that message, and "
                    "the text string format. Two types of prio" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_7_1[] =
        {0xD0, 0x1A, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x0F, 0x04, 0x3C, 0x47, 0x4F, 0x2D, 0x42, 0x41, 0x43, 0x4B, 0x57, 0x41,
         0x52, 0x44, 0x53, 0x3E};
    static unsigned char const resp_1_7_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x11};
    QTest::newRow( "DISPLAY TEXT 1.7.1 - GCF 27.22.4.1.1" )
        << QByteArray( (char *)data_1_7_1, sizeof(data_1_7_1) )
        << QByteArray( (char *)resp_1_7_1, sizeof(resp_1_7_1) )
        << 0x0011       // Backward move
        << QString( "<GO-BACKWARDS>" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_8_1[] =
        {0xD0, 0x13, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x08, 0x04, 0x3C, 0x41, 0x42, 0x4F, 0x52, 0x54, 0x3E};
    static unsigned char const resp_1_8_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x10};
    QTest::newRow( "DISPLAY TEXT 1.8.1 - GCF 27.22.4.1.1" )
        << QByteArray( (char *)data_1_8_1, sizeof(data_1_8_1) )
        << QByteArray( (char *)resp_1_8_1, sizeof(resp_1_8_1) )
        << 0x0010       // Proactive SIM session terminated
        << QString( "<ABORT>" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_9_1[] =
        {0xD0, 0x0F, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x00, 0x9E, 0x02, 0x00, 0x01};
    static unsigned char const resp_1_9_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x32};
    QTest::newRow( "DISPLAY TEXT 1.9.1 - GCF 27.22.4.1.1" )
        << QByteArray( (char *)data_1_9_1, sizeof(data_1_9_1) )
        << QByteArray( (char *)resp_1_9_1, sizeof(resp_1_9_1) )
        << 0x0032       // Command data not understood
        << QString( "" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 1 << true    // Icon details
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1[] =
        {0xD0, 0x16, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x0B, 0x04, 0x3C, 0x54, 0x49, 0x4D, 0x45, 0x2D, 0x4F, 0x55, 0x54, 0x3E};
    static unsigned char const resp_2_1_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x12};
    QTest::newRow( "DISPLAY TEXT 2.1.1 - GCF 27.22.4.1.2" )
        << QByteArray( (char *)data_2_1_1, sizeof(data_2_1_1) )
        << QByteArray( (char *)resp_2_1_1, sizeof(resp_2_1_1) )
        << 0x0012       // No response from user
        << QString( "<TIME-OUT>" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_3_1_1[] =
        {0xD0, 0x81, 0xFD, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02,
         0x8D, 0x81, 0xF1, 0x04, 0x54, 0x68, 0x69, 0x73, 0x20, 0x63, 0x6F, 0x6D,
         0x6D, 0x61, 0x6E, 0x64, 0x20, 0x69, 0x6E, 0x73, 0x74, 0x72, 0x75, 0x63,
         0x74, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x4D, 0x45, 0x20, 0x74, 0x6F,
         0x20, 0x64, 0x69, 0x73, 0x70, 0x6C, 0x61, 0x79, 0x20, 0x61, 0x20, 0x74,
         0x65, 0x78, 0x74, 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x2C,
         0x20, 0x61, 0x6E, 0x64, 0x2F, 0x6F, 0x72, 0x20, 0x61, 0x6E, 0x20, 0x69,
         0x63, 0x6F, 0x6E, 0x20, 0x28, 0x73, 0x65, 0x65, 0x20, 0x36, 0x2E, 0x35,
         0x2E, 0x34, 0x29, 0x2E, 0x20, 0x49, 0x74, 0x20, 0x61, 0x6C, 0x6C, 0x6F,
         0x77, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x53, 0x49, 0x4D, 0x20, 0x74,
         0x6F, 0x20, 0x64, 0x65, 0x66, 0x69, 0x6E, 0x65, 0x20, 0x74, 0x68, 0x65,
         0x20, 0x70, 0x72, 0x69, 0x6f, 0x72, 0x69, 0x74, 0x79, 0x20, 0x6F, 0x66,
         0x20, 0x74, 0x68, 0x61, 0x74, 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 0x67,
         0x65, 0x2C, 0x20, 0x61, 0x6E, 0x64, 0x20, 0x74, 0x68, 0x65, 0x20, 0x74,
         0x65, 0x78, 0x74, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x66,
         0x6F, 0x72, 0x6D, 0x61, 0x74, 0x2E, 0x20, 0x54, 0x77, 0x6F, 0x20, 0x74,
         0x79, 0x70, 0x65, 0x73, 0x20, 0x6F, 0x66, 0x20, 0x70, 0x72, 0x69, 0x6F,
         0x72, 0x69, 0x74, 0x79, 0x20, 0x61, 0x72, 0x65, 0x20, 0x64, 0x65, 0x66,
         0x69, 0x6E, 0x65, 0x64, 0x3A, 0x2D, 0x20, 0x64, 0x69, 0x73, 0x70, 0x6C,
         0x61, 0x79, 0x20, 0x6E, 0x6F, 0x72, 0x6D, 0x61, 0x6C, 0x20, 0x70, 0x72,
         0x69, 0x6F, 0x72, 0x69, 0x74, 0x79, 0x20, 0x74, 0x65, 0x78, 0x74, 0x20,
         0x61, 0x6E, 0x64, 0x2F};
    static unsigned char const resp_3_1_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "DISPLAY TEXT 3.1.1 - GCF 27.22.4.1.3" )
        << QByteArray( (char *)data_3_1_1, sizeof(data_3_1_1) )
        << QByteArray( (char *)resp_3_1_1, sizeof(resp_3_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "This command instructs the ME to display a text message, and/or "
                    "an icon (see 6.5.4). It allows the SIM to define the priority "
                    "of that message, and the text string format. Two types of priority are "
                    "defined:- display normal priority text and/" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_4_1_1[] =
        {0xD0, 0x1C, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x0F, 0x04, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x54, 0x65,
         0x73, 0x74, 0x20, 0x31, 0xAB, 0x00};
    static unsigned char const resp_4_1_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "DISPLAY TEXT 4.1.1 - GCF 27.22.4.1.4" )
        << QByteArray( (char *)data_4_1_1, sizeof(data_4_1_1) )
        << QByteArray( (char *)resp_4_1_1, sizeof(resp_4_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Test 1" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << true         // Immediate response
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_4_2_1[] =
        {0xD0, 0x1C, 0x81, 0x03, 0x01, 0x21, 0x00, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x0F, 0x04, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x54, 0x65,
         0x73, 0x74, 0x20, 0x32, 0xAB, 0x00};
    static unsigned char const resp_4_2_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "DISPLAY TEXT 4.2.1 - GCF 27.22.4.1.4" )
        << QByteArray( (char *)data_4_2_1, sizeof(data_4_2_1) )
        << QByteArray( (char *)resp_4_2_1, sizeof(resp_4_2_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Test 2" )
        << true         // Clear after delay
        << false        // Normal priority
        << true         // Immediate response
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_4_3_1[] =
        {0xD0, 0x1C, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x0F, 0x04, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x54, 0x65,
         0x73, 0x74, 0x20, 0x33, 0xAB, 0x00};
    static unsigned char const resp_4_3_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "DISPLAY TEXT 4.3.1 - GCF 27.22.4.1.4" )
        << QByteArray( (char *)data_4_3_1, sizeof(data_4_3_1) )
        << QByteArray( (char *)resp_4_3_1, sizeof(resp_4_3_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Test 3" )
        << false        // Clear after delay
        << false        // Normal priority
        << true         // Immediate response
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_4_4_1[] =
        {0xD0, 0x1C, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x0F, 0x04, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x54, 0x65,
         0x73, 0x74, 0x20, 0x34, 0xAB, 0x00};
    static unsigned char const resp_4_4_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "DISPLAY TEXT 4.4.1 - GCF 27.22.4.1.4" )
        << QByteArray( (char *)data_4_4_1, sizeof(data_4_4_1) )
        << QByteArray( (char *)resp_4_4_1, sizeof(resp_4_4_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Test 4" )
        << false        // Clear after delay
        << false        // Normal priority
        << true         // Immediate response
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_5_1_1a[] =
        {0xD0, 0x1A, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x0B, 0x04, 0x42, 0x61, 0x73, 0x69, 0x63, 0x20, 0x49, 0x63, 0x6F, 0x6E,
         0x9E, 0x02, 0x00, 0x01};
    static unsigned char const resp_5_1_1a[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "DISPLAY TEXT 5.1.1A - GCF 27.22.4.1.5" )
        << QByteArray( (char *)data_5_1_1a, sizeof(data_5_1_1a) )
        << QByteArray( (char *)resp_5_1_1a, sizeof(resp_5_1_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Basic Icon" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 1 << true    // Icon details
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_5_1_1b[] =
        {0xD0, 0x1A, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x0B, 0x04, 0x42, 0x61, 0x73, 0x69, 0x63, 0x20, 0x49, 0x63, 0x6F, 0x6E,
         0x9E, 0x02, 0x00, 0x01};
    static unsigned char const resp_5_1_1b[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04};
    QTest::newRow( "DISPLAY TEXT 5.1.1B - GCF 27.22.4.1.5" )
        << QByteArray( (char *)data_5_1_1b, sizeof(data_5_1_1b) )
        << QByteArray( (char *)resp_5_1_1b, sizeof(resp_5_1_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Basic Icon" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 1 << true    // Icon details
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_5_2_1a[] =
        {0xD0, 0x1B, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x0C, 0x04, 0x43, 0x6F, 0x6C, 0x6F, 0x75, 0x72, 0x20, 0x49, 0x63, 0x6F,
         0x6E, 0x9E, 0x02, 0x00, 0x02};
    static unsigned char const resp_5_2_1a[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "DISPLAY TEXT 5.2.1A - GCF 27.22.4.1.5" )
        << QByteArray( (char *)data_5_2_1a, sizeof(data_5_2_1a) )
        << QByteArray( (char *)resp_5_2_1a, sizeof(resp_5_2_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Colour Icon" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 2 << true    // Icon details
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_5_2_1b[] =
        {0xD0, 0x1B, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x0C, 0x04, 0x43, 0x6F, 0x6C, 0x6F, 0x75, 0x72, 0x20, 0x49, 0x63, 0x6F,
         0x6E, 0x9E, 0x02, 0x00, 0x02};
    static unsigned char const resp_5_2_1b[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04};
    QTest::newRow( "DISPLAY TEXT 5.2.1B - GCF 27.22.4.1.5" )
        << QByteArray( (char *)data_5_2_1b, sizeof(data_5_2_1b) )
        << QByteArray( (char *)resp_5_2_1b, sizeof(resp_5_2_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Colour Icon" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 2 << true    // Icon details
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_5_3_1a[] =
        {0xD0, 0x1A, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x0B, 0x04, 0x42, 0x61, 0x73, 0x69, 0x63, 0x20, 0x49, 0x63, 0x6F, 0x6E,
         0x9E, 0x02, 0x01, 0x01};
    static unsigned char const resp_5_3_1a[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "DISPLAY TEXT 5.3.1A - GCF 27.22.4.1.5" )
        << QByteArray( (char *)data_5_3_1a, sizeof(data_5_3_1a) )
        << QByteArray( (char *)resp_5_3_1a, sizeof(resp_5_3_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Basic Icon" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 1 << false   // Icon details
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_5_3_1b[] =
        {0xD0, 0x1A, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x0B, 0x04, 0x42, 0x61, 0x73, 0x69, 0x63, 0x20, 0x49, 0x63, 0x6F, 0x6E,
         0x9E, 0x02, 0x01, 0x01};
    static unsigned char const resp_5_3_1b[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04};
    QTest::newRow( "DISPLAY TEXT 5.3.1B - GCF 27.22.4.1.5" )
        << QByteArray( (char *)data_5_3_1b, sizeof(data_5_3_1b) )
        << QByteArray( (char *)resp_5_3_1b, sizeof(resp_5_3_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Basic Icon" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 1 << false   // Icon details
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_6_1_1[] =
        {0xD0, 0x24, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x19, 0x08, 0x04, 0x17, 0x04, 0x14, 0x04, 0x20, 0x04, 0x10, 0x04, 0x12,
         0x04, 0x21, 0x04, 0x22, 0x04, 0x12, 0x04, 0x23, 0x04, 0x19, 0x04, 0x22,
         0x04, 0x15};
    static unsigned char const resp_6_1_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static ushort const str_6_1_1[] =
        {0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423,
         0x0419, 0x0422, 0x0415};
    QTest::newRow( "DISPLAY TEXT 6.1.1 - GCF 27.22.4.1.6" )
        << QByteArray( (char *)data_6_1_1, sizeof(data_6_1_1) )
        << QByteArray( (char *)resp_6_1_1, sizeof(resp_6_1_1) )
        << 0x0000       // Command performed successfully
        << QString::fromUtf16( str_6_1_1, sizeof(str_6_1_1) / sizeof(ushort) )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attributes
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_8_1_1[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x11, 0x04, 0x54, 0x65, 0x78, 0x74, 0x20, 0x41, 0x74, 0x74, 0x72, 0x69,
         0x62, 0x75, 0x74, 0x65, 0x20, 0x31, 0xD0, 0x04, 0x00, 0x10, 0x00, 0xB4};
    static unsigned char const resp_8_1_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const attr_8_1_1[] =
        {0x00, 0x10, 0x00, 0xB4};
    QTest::newRow( "DISPLAY TEXT 8.1.1 - GCF 27.22.4.1.8" )
        << QByteArray( (char *)data_8_1_1, sizeof(data_8_1_1) )
        << QByteArray( (char *)resp_8_1_1, sizeof(resp_8_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Text Attribute 1" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray( (char *)attr_8_1_1, sizeof(attr_8_1_1) )
        << QString( "<body bgcolor=\"#FFFF00\"><div align=\"left\"><font color=\"#008000\">Text Attribute 1</font></div></body>" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_8_2_1[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x11, 0x04, 0x54, 0x65, 0x78, 0x74, 0x20, 0x41, 0x74, 0x74, 0x72, 0x69,
         0x62, 0x75, 0x74, 0x65, 0x20, 0x31, 0xD0, 0x04, 0x00, 0x10, 0x01, 0xB4};
    static unsigned char const resp_8_2_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const attr_8_2_1[] =
        {0x00, 0x10, 0x01, 0xB4};
    QTest::newRow( "DISPLAY TEXT 8.2.1 - GCF 27.22.4.1.8" )
        << QByteArray( (char *)data_8_2_1, sizeof(data_8_2_1) )
        << QByteArray( (char *)resp_8_2_1, sizeof(resp_8_2_1) )
        << 0x0000       // Command performed successfully
        << QString( "Text Attribute 1" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray( (char *)attr_8_2_1, sizeof(attr_8_2_1) )
        << QString( "<body bgcolor=\"#FFFF00\"><center><font color=\"#008000\">Text Attribute 1</font></center></body>" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_8_3_1[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x11, 0x04, 0x54, 0x65, 0x78, 0x74, 0x20, 0x41, 0x74, 0x74, 0x72, 0x69,
         0x62, 0x75, 0x74, 0x65, 0x20, 0x31, 0xD0, 0x04, 0x00, 0x10, 0x02, 0xB4};
    static unsigned char const resp_8_3_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const attr_8_3_1[] =
        {0x00, 0x10, 0x02, 0xB4};
    QTest::newRow( "DISPLAY TEXT 8.3.1 - GCF 27.22.4.1.8" )
        << QByteArray( (char *)data_8_3_1, sizeof(data_8_3_1) )
        << QByteArray( (char *)resp_8_3_1, sizeof(resp_8_3_1) )
        << 0x0000       // Command performed successfully
        << QString( "Text Attribute 1" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray( (char *)attr_8_3_1, sizeof(attr_8_3_1) )
        << QString( "<body bgcolor=\"#FFFF00\"><div align=\"right\"><font color=\"#008000\">Text Attribute 1</font></div></body>" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_8_4_1[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x11, 0x04, 0x54, 0x65, 0x78, 0x74, 0x20, 0x41, 0x74, 0x74, 0x72, 0x69,
         0x62, 0x75, 0x74, 0x65, 0x20, 0x31, 0xD0, 0x04, 0x00, 0x10, 0x04, 0xB4};
    static unsigned char const resp_8_4_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const attr_8_4_1[] =
        {0x00, 0x10, 0x04, 0xB4};
    QTest::newRow( "DISPLAY TEXT 8.4.1 - GCF 27.22.4.1.8" )
        << QByteArray( (char *)data_8_4_1, sizeof(data_8_4_1) )
        << QByteArray( (char *)resp_8_4_1, sizeof(resp_8_4_1) )
        << 0x0000       // Command performed successfully
        << QString( "Text Attribute 1" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray( (char *)attr_8_4_1, sizeof(attr_8_4_1) )
        << QString( "<body bgcolor=\"#FFFF00\"><div align=\"left\"><font color=\"#008000\"><big>Text Attribute 1</big></font></div></body>" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_8_5_1[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x11, 0x04, 0x54, 0x65, 0x78, 0x74, 0x20, 0x41, 0x74, 0x74, 0x72, 0x69,
         0x62, 0x75, 0x74, 0x65, 0x20, 0x31, 0xD0, 0x04, 0x00, 0x10, 0x08, 0xB4};
    static unsigned char const resp_8_5_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const attr_8_5_1[] =
        {0x00, 0x10, 0x08, 0xB4};
    QTest::newRow( "DISPLAY TEXT 8.5.1 - GCF 27.22.4.1.8" )
        << QByteArray( (char *)data_8_5_1, sizeof(data_8_5_1) )
        << QByteArray( (char *)resp_8_5_1, sizeof(resp_8_5_1) )
        << 0x0000       // Command performed successfully
        << QString( "Text Attribute 1" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray( (char *)attr_8_5_1, sizeof(attr_8_5_1) )
        << QString( "<body bgcolor=\"#FFFF00\"><div align=\"left\"><font color=\"#008000\"><small>Text Attribute 1</small></font></div></body>" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_8_6_1[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x11, 0x04, 0x54, 0x65, 0x78, 0x74, 0x20, 0x41, 0x74, 0x74, 0x72, 0x69,
         0x62, 0x75, 0x74, 0x65, 0x20, 0x31, 0xD0, 0x04, 0x00, 0x10, 0x10, 0xB4};
    static unsigned char const resp_8_6_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const attr_8_6_1[] =
        {0x00, 0x10, 0x10, 0xB4};
    QTest::newRow( "DISPLAY TEXT 8.6.1 - GCF 27.22.4.1.8" )
        << QByteArray( (char *)data_8_6_1, sizeof(data_8_6_1) )
        << QByteArray( (char *)resp_8_6_1, sizeof(resp_8_6_1) )
        << 0x0000       // Command performed successfully
        << QString( "Text Attribute 1" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray( (char *)attr_8_6_1, sizeof(attr_8_6_1) )
        << QString( "<body bgcolor=\"#FFFF00\"><div align=\"left\"><font color=\"#008000\"><b>Text Attribute 1</b></font></div></body>" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_8_7_1[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x11, 0x04, 0x54, 0x65, 0x78, 0x74, 0x20, 0x41, 0x74, 0x74, 0x72, 0x69,
         0x62, 0x75, 0x74, 0x65, 0x20, 0x31, 0xD0, 0x04, 0x00, 0x10, 0x20, 0xB4};
    static unsigned char const resp_8_7_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const attr_8_7_1[] =
        {0x00, 0x10, 0x20, 0xB4};
    QTest::newRow( "DISPLAY TEXT 8.7.1 - GCF 27.22.4.1.8" )
        << QByteArray( (char *)data_8_7_1, sizeof(data_8_7_1) )
        << QByteArray( (char *)resp_8_7_1, sizeof(resp_8_7_1) )
        << 0x0000       // Command performed successfully
        << QString( "Text Attribute 1" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray( (char *)attr_8_7_1, sizeof(attr_8_7_1) )
        << QString( "<body bgcolor=\"#FFFF00\"><div align=\"left\"><font color=\"#008000\"><i>Text Attribute 1</i></font></div></body>" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_8_8_1[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x11, 0x04, 0x54, 0x65, 0x78, 0x74, 0x20, 0x41, 0x74, 0x74, 0x72, 0x69,
         0x62, 0x75, 0x74, 0x65, 0x20, 0x31, 0xD0, 0x04, 0x00, 0x10, 0x40, 0xB4};
    static unsigned char const resp_8_8_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const attr_8_8_1[] =
        {0x00, 0x10, 0x40, 0xB4};
    QTest::newRow( "DISPLAY TEXT 8.8.1 - GCF 27.22.4.1.8" )
        << QByteArray( (char *)data_8_8_1, sizeof(data_8_8_1) )
        << QByteArray( (char *)resp_8_8_1, sizeof(resp_8_8_1) )
        << 0x0000       // Command performed successfully
        << QString( "Text Attribute 1" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray( (char *)attr_8_8_1, sizeof(attr_8_8_1) )
        << QString( "<body bgcolor=\"#FFFF00\"><div align=\"left\"><font color=\"#008000\"><u>Text Attribute 1</u></font></div></body>" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_8_9_1[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x11, 0x04, 0x54, 0x65, 0x78, 0x74, 0x20, 0x41, 0x74, 0x74, 0x72, 0x69,
         0x62, 0x75, 0x74, 0x65, 0x20, 0x31, 0xD0, 0x04, 0x00, 0x10, 0x80, 0xB4};
    static unsigned char const resp_8_9_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const attr_8_9_1[] =
        {0x00, 0x10, 0x80, 0xB4};
    QTest::newRow( "DISPLAY TEXT 8.9.1 - GCF 27.22.4.1.8" )
        << QByteArray( (char *)data_8_9_1, sizeof(data_8_9_1) )
        << QByteArray( (char *)resp_8_9_1, sizeof(resp_8_9_1) )
        << 0x0000       // Command performed successfully
        << QString( "Text Attribute 1" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray( (char *)attr_8_9_1, sizeof(attr_8_9_1) )
        << QString( "<body bgcolor=\"#FFFF00\"><div align=\"left\"><font color=\"#008000\"><s>Text Attribute 1</s></font></div></body>" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_8_10_1[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x81, 0x02, 0x8D,
         0x11, 0x04, 0x54, 0x65, 0x78, 0x74, 0x20, 0x41, 0x74, 0x74, 0x72, 0x69,
         0x62, 0x75, 0x74, 0x65, 0x20, 0x31, 0xD0, 0x04, 0x00, 0x10, 0x00, 0xB4};
    static unsigned char const resp_8_10_1[] =
        {0x81, 0x03, 0x01, 0x21, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const attr_8_10_1[] =
        {0x00, 0x10, 0x00, 0xB4};
    QTest::newRow( "DISPLAY TEXT 8.10.1 - GCF 27.22.4.1.8" )
        << QByteArray( (char *)data_8_10_1, sizeof(data_8_10_1) )
        << QByteArray( (char *)resp_8_10_1, sizeof(resp_8_10_1) )
        << 0x0000       // Command performed successfully
        << QString( "Text Attribute 1" )
        << false        // Wait for user to clear
        << false        // Normal priority
        << false        // Immediate response
        << 0 << false   // No icon
        << QByteArray( (char *)attr_8_10_1, sizeof(attr_8_10_1) )
        << QString( "<body bgcolor=\"#FFFF00\"><div align=\"left\"><font color=\"#008000\">Text Attribute 1</font></div></body>" )
        << (int)( QSimCommand::NoPduOptions );
}
