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

#include <QPhoneCallManager>

#ifndef SYSTEMTEST

// Test encoding and decoding of EVENT DOWNLOAD envelopes based on the
// Test encoding and decoding of SET UP CALL commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.13 - SET UP CALL.
void tst_QSimToolkit::testEncodeSetupCall_data()
{
    QSimToolkitData::populateDataSetupCall();
}
void tst_QSimToolkit::testEncodeSetupCall()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, capabilityConfig );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( QString, secondAlpha );
    QFETCH( QString, number );
    QFETCH( QString, subAddress );
    QFETCH( int, disposition );
    QFETCH( int, callClass );
    QFETCH( bool, withRedial );
    QFETCH( int, duration );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( int, iconId2 );
    QFETCH( bool, iconSelfExplanatory2 );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::SetupCall );
    QVERIFY( decoded.destinationDevice() == QSimCommand::Network );
    QCOMPARE( decoded.text(), text );
    QCOMPARE( decoded.otherText(), secondAlpha );
    QCOMPARE( decoded.number(), number );
    QCOMPARE( decoded.subAddress(), subAddress );
    QCOMPARE( (int)decoded.disposition(), disposition );
    QCOMPARE( (int)decoded.callClass(), callClass );
    QCOMPARE( decoded.withRedial(), withRedial );
    QCOMPARE( (int)decoded.duration(), duration );
    QCOMPARE( (int)decoded.iconId(), iconId );
    QCOMPARE( decoded.iconSelfExplanatory(), iconSelfExplanatory );
    QCOMPARE( (int)decoded.otherIconId(), iconId2 );
    QCOMPARE( decoded.otherIconSelfExplanatory(), iconSelfExplanatory2 );
    QCOMPARE( decoded.extensionField(0x87), capabilityConfig );

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

// Test that SET UP CALL commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverSetupCall_data()
{
    QSimToolkitData::populateDataSetupCall();
}
void tst_QSimToolkit::testDeliverSetupCall()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, capabilityConfig );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( QString, secondAlpha );
    QFETCH( QString, number );
    QFETCH( QString, subAddress );
    QFETCH( int, disposition );
    QFETCH( int, callClass );
    QFETCH( bool, withRedial );
    QFETCH( int, duration );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( int, iconId2 );
    QFETCH( bool, iconSelfExplanatory2 );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::SetupCall );
    cmd.setDestinationDevice( QSimCommand::Network );
    cmd.setText( text );
    cmd.setOtherText( secondAlpha );
    cmd.setNumber( number );
    cmd.setSubAddress( subAddress );
    cmd.setDisposition( (QSimCommand::Disposition)disposition );
    cmd.setCallClass( (QSimCommand::CallClass)callClass );
    cmd.setWithRedial( withRedial );
    cmd.setDuration( duration );
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );
    cmd.setOtherIconId( (uint)iconId2 );
    cmd.setOtherIconSelfExplanatory( iconSelfExplanatory2 );
    if ( !capabilityConfig.isEmpty() )
        cmd.addExtensionField( 0x87, capabilityConfig );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QVERIFY( deliveredCommand.text() == cmd.text() );
    QVERIFY( deliveredCommand.otherText() == cmd.otherText() );
    QVERIFY( deliveredCommand.number() == cmd.number() );
    QVERIFY( deliveredCommand.subAddress() == cmd.subAddress() );
    QVERIFY( deliveredCommand.disposition() == cmd.disposition() );
    QVERIFY( deliveredCommand.callClass() == cmd.callClass() );
    QVERIFY( deliveredCommand.withRedial() == cmd.withRedial() );
    QVERIFY( deliveredCommand.duration() == cmd.duration() );
    QVERIFY( deliveredCommand.iconId() == cmd.iconId() );
    QVERIFY( deliveredCommand.iconSelfExplanatory() == cmd.iconSelfExplanatory() );
    QVERIFY( deliveredCommand.otherIconId() == cmd.otherIconId() );
    QVERIFY( deliveredCommand.otherIconSelfExplanatory() == cmd.otherIconSelfExplanatory() );
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

// Test the user interface in "simapp" for SET UP CALL.
void tst_QSimToolkit::testUISetupCall_data()
{
    QSimToolkitData::populateDataSetupCall();
}

void tst_QSimToolkit::testUISetupCall()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, capabilityConfig );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( QString, secondAlpha );
    QFETCH( QString, number );
    QFETCH( QString, subAddress );
    QFETCH( int, disposition );
    QFETCH( int, callClass );
    QFETCH( bool, withRedial );
    QFETCH( int, duration );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( int, iconId2 );
    QFETCH( bool, iconSelfExplanatory2 );
    QFETCH( int, options );

    Q_UNUSED(options)

    // Skip tests that we cannot test using the "simapp" UI.
    if ( resptype == 0x0004 ||      // Icon not displayed
         resptype == 0x2191 ||      // User busy
         resptype == 0x0030 ||      // Beyond ME's capabilities
         resptype == 0x2100 ) {     // Could not put call on hold
        QSKIP( "", SkipSingle );
    }

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Create the command to be tested.
    QSimCommand cmd;
    cmd.setType( QSimCommand::SetupCall );
    cmd.setDestinationDevice( QSimCommand::Network );
    cmd.setText( text );
    cmd.setOtherText( secondAlpha );
    cmd.setNumber( number );
    cmd.setSubAddress( subAddress );
    cmd.setDisposition( (QSimCommand::Disposition)disposition );
    cmd.setCallClass( (QSimCommand::CallClass)callClass );
    cmd.setWithRedial( withRedial );
    cmd.setDuration( duration );
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );
    cmd.setOtherIconId( (uint)iconId2 );
    cmd.setOtherIconSelfExplanatory( iconSelfExplanatory2 );
    if ( !capabilityConfig.isEmpty() )
        cmd.addExtensionField( 0x87, capabilityConfig );

    // Set up some dummy calls if needed before sending the command
    QPhoneCallManager callManager;
    if ( cmd.disposition() != QSimCommand::IfNoOtherCalls
            || ( cmd.disposition() == QSimCommand::IfNoOtherCalls
                && resptype == 0x2002 ) ) { // ME busy on another call
        QDialOptions dialopts;
        dialopts.setNumber( "3333" );
        QPhoneCall call = callManager.create( "Voice" );

        if ( call.isNull() )
            qDebug() << "Failed to create a dummy call";

        call.dial( dialopts );

        msleep( 3000 ); // wait for 3 seconds for the call connected.
    }

    // Set up the server with the command, ready to be selected
    // from the "Run Test" menu item on the test menu.
    server->startUsingTestMenu( cmd );
    QVERIFY( waitForView( SimMenu::staticMetaObject ) );

    // Clear the server state just before we request the actual command under test.
    server->clear();

    // Select the first menu item.
    select();

    // Wait for the text to display.
    QVERIFY( waitForView( SimSetupCall::staticMetaObject ) );

    // Wait for icons to load.  Should be fast because they are in-process.
    if ( iconId != 0 || iconId2 != 0 )
        msleep(1000);

    // Determine what needs to be done next.
    if ( resptype == 0x0022 ) { // User did not accept the request
        keyClick( Qt::Key_Back );

        QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 5000 ) );
    } else if ( resptype == 0x2002 ) { // ME busy on another call
        keyClick( Qt::Key_Back ); // click OK to information

        QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 5000 ) );
    } else { // success
        // place a call by clicking contex 1 button
        keyClick( Qt::Key_Context1 );

        QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 5000 ) );
    }

    // Check that the response is what we expected.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    QCOMPARE( server->lastResponse(), resp );

    // close second alpha indentifer message box if any
    if ( !secondAlpha.isEmpty() || (uint)iconId2 )
        keyClick( Qt::Key_Back );

    // hangup all calls
    foreach ( QPhoneCall call, callManager.calls() ) {
        call.hangup();
    }

    msleep( 2000 );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for SET UP CALL from the GCF test cases
// in GSM 51.010, section 27.22.4.13.
void QSimToolkitData::populateDataSetupCall()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<QByteArray>("capabilityConfig");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("secondAlpha");
    QTest::addColumn<QString>("number");
    QTest::addColumn<QString>("subAddress");
    QTest::addColumn<int>("disposition");
    QTest::addColumn<int>("callClass");
    QTest::addColumn<bool>("withRedial");
    QTest::addColumn<int>("duration");
    QTest::addColumn<int>("iconId");
    QTest::addColumn<bool>("iconSelfExplanatory");
    QTest::addColumn<int>("iconId2");
    QTest::addColumn<bool>("iconSelfExplanatory2");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x1E, 0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x08, 0x4E, 0x6F, 0x74, 0x20, 0x62, 0x75, 0x73, 0x79, 0x86, 0x09, 0x91,
         0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SET UP CALL 1.1.1 - GCF 27.22.4.13.1" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "Not busy" )
        << QString( "" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 0 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x1E, 0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x08, 0x4E, 0x6F, 0x74, 0x20, 0x62, 0x75, 0x73, 0x79, 0x86, 0x09, 0x91,
         0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C};
    static unsigned char const resp_1_2_1[] =
        {0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x22};
    QTest::newRow( "SET UP CALL 1.2.1 - GCF 27.22.4.13.1" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << QByteArray()
        << 0x0022       // User did not accept the proactive command
        << QString( "Not busy" )
        << QString( "" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 0 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1[] =
        {0xD0, 0x2A, 0x81, 0x03, 0x01, 0x10, 0x01, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x14, 0x4E, 0x6F, 0x74, 0x20, 0x62, 0x75, 0x73, 0x79, 0x20, 0x77, 0x69,
         0x74, 0x68, 0x20, 0x72, 0x65, 0x64, 0x69, 0x61, 0x6C, 0x86, 0x09, 0x91,
         0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C};
    static unsigned char const resp_1_3_1[] =
        {0x81, 0x03, 0x01, 0x10, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x21,
         0x91};
    QTest::newRow( "SET UP CALL 1.3.1 - GCF 27.22.4.13.1" )
        << QByteArray( (char *)data_1_3_1, sizeof(data_1_3_1) )
        << QByteArray( (char *)resp_1_3_1, sizeof(resp_1_3_1) )
        << QByteArray()
        << 0x2191       // User busy
        << QString( "Not busy with redial" )
        << QString( "" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << true         // With redial
        << 0            // Duration
        << 0 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_1[] =
        {0xD0, 0x1D, 0x81, 0x03, 0x01, 0x10, 0x02, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x07, 0x4F, 0x6E, 0x20, 0x68, 0x6F, 0x6C, 0x64, 0x86, 0x09, 0x91, 0x10,
         0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C};
    static unsigned char const resp_1_4_1[] =
        {0x81, 0x03, 0x01, 0x10, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SET UP CALL 1.4.1 - GCF 27.22.4.13.1" )
        << QByteArray( (char *)data_1_4_1, sizeof(data_1_4_1) )
        << QByteArray( (char *)resp_1_4_1, sizeof(resp_1_4_1) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "On hold" )
        << QString( "" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::PutOnHold )
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 0 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_1[] =
        {0xD0, 0x20, 0x81, 0x03, 0x01, 0x10, 0x04, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0A, 0x44, 0x69, 0x73, 0x63, 0x6F, 0x6E, 0x6E, 0x65, 0x63, 0x74, 0x86,
         0x09, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C};
    static unsigned char const resp_1_5_1[] =
        {0x81, 0x03, 0x01, 0x10, 0x04, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SET UP CALL 1.5.1 - GCF 27.22.4.13.1" )
        << QByteArray( (char *)data_1_5_1, sizeof(data_1_5_1) )
        << QByteArray( (char *)resp_1_5_1, sizeof(resp_1_5_1) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "Disconnect" )
        << QString( "" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::Disconnect)
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 0 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_6_1[] =
        {0xD0, 0x1E, 0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x08, 0x4E, 0x6F, 0x74, 0x20, 0x62, 0x75, 0x73, 0x79, 0x86, 0x09, 0x91,
         0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C};
    static unsigned char const resp_1_6_1[] =
        {0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x20,
         0x02};
    QTest::newRow( "SET UP CALL 1.6.1 - GCF 27.22.4.13.1" )
        << QByteArray( (char *)data_1_6_1, sizeof(data_1_6_1) )
        << QByteArray( (char *)resp_1_6_1, sizeof(resp_1_6_1) )
        << QByteArray()
        << 0x2002       // ME busy on another call
        << QString( "Not busy" )
        << QString( "" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 0 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_7_1[] =
        {0xD0, 0x1D, 0x81, 0x03, 0x01, 0x10, 0x02, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x07, 0x4F, 0x6E, 0x20, 0x68, 0x6F, 0x6C, 0x64, 0x86, 0x09, 0x91, 0x10,
         0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C};
    static unsigned char const resp_1_7_1[] =
        {0x81, 0x03, 0x01, 0x10, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x21,
         0x00};
    QTest::newRow( "SET UP CALL 1.7.1 - GCF 27.22.4.13.1" )
        << QByteArray( (char *)data_1_7_1, sizeof(data_1_7_1) )
        << QByteArray( (char *)resp_1_7_1, sizeof(resp_1_7_1) )
        << QByteArray()
        << 0x2100       // Could not put call on hold
        << QString( "On hold" )
        << QString( "" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::PutOnHold )
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 0 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_8_1[] =
        {0xD0, 0x2B, 0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x11, 0x43, 0x61, 0x70, 0x61, 0x62, 0x69, 0x6C, 0x69, 0x74, 0x79, 0x20,
         0x63, 0x6F, 0x6E, 0x66, 0x69, 0x67, 0x86, 0x09, 0x91, 0x10, 0x32, 0x04,
         0x21, 0x43, 0x65, 0x1C, 0x2C, 0x87, 0x02, 0x01, 0xA0};
    static unsigned char const resp_1_8_1[] =
        {0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const conf_1_8_1[] =
        {0x01, 0xA0};
    QTest::newRow( "SET UP CALL 1.8.1 - GCF 27.22.4.13.1" )
        << QByteArray( (char *)data_1_8_1, sizeof(data_1_8_1) )
        << QByteArray( (char *)resp_1_8_1, sizeof(resp_1_8_1) )
        << QByteArray( (char *)conf_1_8_1, sizeof(conf_1_8_1) )
        << 0x0000       // Command performed successfully
        << QString( "Capability config" )
        << QString( "" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 0 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_9_1a[] =
        {0xD0, 0x34, 0x81, 0x03, 0x01, 0x10, 0x01, 0x82, 0x02, 0x81, 0x83, 0x86,
         0x29, 0x91, 0x10, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0x54, 0x76, 0x98,
         0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0x10, 0x32,
         0x54, 0x76, 0x98, 0x10, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0x54, 0x76,
         0x98, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA};
    static unsigned char const resp_1_9_1a[] =
        {0x81, 0x03, 0x01, 0x10, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SET UP CALL 1.9.1A - GCF 27.22.4.13.1" )
        << QByteArray( (char *)data_1_9_1a, sizeof(data_1_9_1a) )
        << QByteArray( (char *)resp_1_9_1a, sizeof(resp_1_9_1a) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "" )
        << QString( "" )
        << QString( "+012345678901234567890123456789*#*#*#*#*#"
                    "012345678901234567890123456789*#*#*#*#*#" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << true         // With redial
        << 0            // Duration
        << 0 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_9_1b[] =
        {0xD0, 0x1C, 0x81, 0x03, 0x01, 0x10, 0x01, 0x82, 0x02, 0x81, 0x83, 0x86,
         0x11, 0x91, 0x10, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0x54, 0x76, 0xBA,
         0xBA, 0xBA, 0xBA, 0xBA, 0x10, 0x32};
    static unsigned char const resp_1_9_1b[] =
        {0x81, 0x03, 0x01, 0x10, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SET UP CALL 1.9.1B - GCF 27.22.4.13.1" )
        << QByteArray( (char *)data_1_9_1b, sizeof(data_1_9_1b) )
        << QByteArray( (char *)resp_1_9_1b, sizeof(resp_1_9_1b) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "" )
        << QString( "" )
        << QString( "+012345678901234567*#*#*#*#*#0123" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << true         // With redial
        << 0            // Duration
        << 0 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_10_1[] =
        {0xD0, 0x81, 0xFD, 0x81, 0x03, 0x01, 0x10, 0x01, 0x82, 0x02, 0x81, 0x83,
         0x85, 0x81, 0xED, 0x54, 0x68, 0x72, 0x65, 0x65, 0x20, 0x74, 0x79, 0x70,
         0x65, 0x73, 0x20, 0x61, 0x72, 0x65, 0x20, 0x64, 0x65, 0x66, 0x69, 0x6E,
         0x65, 0x64, 0x3A, 0x20, 0x2D, 0x20, 0x73, 0x65, 0x74, 0x20, 0x75, 0x70,
         0x20, 0x61, 0x20, 0x63, 0x61, 0x6C, 0x6C, 0x2C, 0x20, 0x62, 0x75, 0x74,
         0x20, 0x6F, 0x6E, 0x6C, 0x79, 0x20, 0x69, 0x66, 0x20, 0x6E, 0x6F, 0x74,
         0x20, 0x63, 0x75, 0x72, 0x72, 0x65, 0x6E, 0x74, 0x6C, 0x79, 0x20, 0x62,
         0x75, 0x73, 0x79, 0x20, 0x6F, 0x6E, 0x20, 0x61, 0x6E, 0x6F, 0x74, 0x68,
         0x65, 0x72, 0x20, 0x63, 0x61, 0x6C, 0x6C, 0x3B, 0x20, 0x2D, 0x20, 0x73,
         0x65, 0x74, 0x20, 0x75, 0x70, 0x20, 0x61, 0x20, 0x63, 0x61, 0x6C, 0x6C,
         0x2C, 0x20, 0x70, 0x75, 0x74, 0x74, 0x69, 0x6E, 0x67, 0x20, 0x61, 0x6C,
         0x6C, 0x20, 0x6F, 0x74, 0x68, 0x65, 0x72, 0x20, 0x63, 0x61, 0x6C, 0x6C,
         0x73, 0x20, 0x28, 0x69, 0x66, 0x20, 0x61, 0x6E, 0x79, 0x29, 0x20, 0x6F,
         0x6E, 0x20, 0x68, 0x6F, 0x6C, 0x64, 0x3B, 0x20, 0x2D, 0x20, 0x73, 0x65,
         0x74, 0x20, 0x75, 0x70, 0x20, 0x61, 0x20, 0x63, 0x61, 0x6C, 0x6C, 0x2C,
         0x20, 0x64, 0x69, 0x73, 0x63, 0x6F, 0x6E, 0x6E, 0x65, 0x63, 0x74, 0x69,
         0x6E, 0x67, 0x20, 0x61, 0x6C, 0x6C, 0x20, 0x6F, 0x74, 0x68, 0x65, 0x72,
         0x20, 0x63, 0x61, 0x6C, 0x6C, 0x73, 0x20, 0x28, 0x69, 0x66, 0x20, 0x61,
         0x6E, 0x79, 0x29, 0x20, 0x66, 0x69, 0x72, 0x73, 0x74, 0x2E, 0x20, 0x46,
         0x6F, 0x72, 0x20, 0x65, 0x61, 0x63, 0x68, 0x20, 0x6F, 0x66, 0x20, 0x74,
         0x68, 0x65, 0x73, 0x65, 0x20, 0x74, 0x79, 0x70, 0x65, 0x73, 0x2C, 0x20,
         0x86, 0x02, 0x91, 0x10};
    static unsigned char const resp_1_10_1[] =
        {0x81, 0x03, 0x01, 0x10, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SET UP CALL 1.10.1 - GCF 27.22.4.13.1" )
        << QByteArray( (char *)data_1_10_1, sizeof(data_1_10_1) )
        << QByteArray( (char *)resp_1_10_1, sizeof(resp_1_10_1) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "Three types are defined: - set up a call, but only if "
                    "not currently busy on another call; - set up a call, "
                    "putting all other calls (if any) on hold; - set up a call, "
                    "disconnecting all other calls (if any) first. "
                    "For each of these types, " )
        << QString( "" )
        << QString( "+01" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << true         // With redial
        << 0            // Duration
        << 0 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_11_1a[] =
        {0xD0, 0x2B, 0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0C, 0x43, 0x61, 0x6C, 0x6C, 0x65, 0x64, 0x20, 0x70, 0x61, 0x72, 0x74,
         0x79, 0x86, 0x09, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C,
         0x88, 0x07, 0x80, 0x50, 0x95, 0x95, 0x95, 0x95, 0x95};
    static unsigned char const resp_1_11_1a[] =
        {0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static ushort const str_1_11_1a[] =
        {0x007F, 0x007F, 0x007F, 0x007F, 0x007F};
    QTest::newRow( "SET UP CALL 1.11.1A - GCF 27.22.4.13.1" )
        << QByteArray( (char *)data_1_11_1a, sizeof(data_1_11_1a) )
        << QByteArray( (char *)resp_1_11_1a, sizeof(resp_1_11_1a) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "Called party" )
        << QString( "" )
        << QString( "+012340123456p1p2" )
        << QString::fromUtf16( str_1_11_1a, sizeof(str_1_11_1a) / sizeof(ushort) )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 0 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_11_1b[] =
        {0xD0, 0x2B, 0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0C, 0x43, 0x61, 0x6C, 0x6C, 0x65, 0x64, 0x20, 0x70, 0x61, 0x72, 0x74,
         0x79, 0x86, 0x09, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C,
         0x88, 0x07, 0x80, 0x50, 0x95, 0x95, 0x95, 0x95, 0x95};
    static unsigned char const resp_1_11_1b[] =
        {0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x30};
    static ushort const str_1_11_1b[] =
        {0x007F, 0x007F, 0x007F, 0x007F, 0x007F};
    QTest::newRow( "SET UP CALL 1.11.1B - GCF 27.22.4.13.1" )
        << QByteArray( (char *)data_1_11_1b, sizeof(data_1_11_1b) )
        << QByteArray( (char *)resp_1_11_1b, sizeof(resp_1_11_1b) )
        << QByteArray()
        << 0x0030       // Beyond ME's capabilities
        << QString( "Called party" )
        << QString( "" )
        << QString( "+012340123456p1p2" )
        << QString::fromUtf16( str_1_11_1b, sizeof(str_1_11_1b) / sizeof(ushort) )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 0 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_12_1[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x10, 0x01, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x08, 0x44, 0x75, 0x72, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x86, 0x09, 0x91,
         0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C, 0x84, 0x02, 0x01, 0x0A};
    static unsigned char const resp_1_12_1[] =
        {0x81, 0x03, 0x01, 0x10, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x21,
         0x91};
    QTest::newRow( "SET UP CALL 1.12.1 - GCF 27.22.4.13.1" )
        << QByteArray( (char *)data_1_12_1, sizeof(data_1_12_1) )
        << QByteArray( (char *)resp_1_12_1, sizeof(resp_1_12_1) )
        << QByteArray()
        << 0x2191       // User busy
        << QString( "Duration" )
        << QString( "" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << true         // With redial
        << 10000        // Duration
        << 0 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1[] =
        {0xD0, 0x28, 0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0C, 0x43, 0x4F, 0x4E, 0x46, 0x49, 0x52, 0x4D, 0x41, 0x54, 0x49, 0x4F,
         0x4E, 0x86, 0x09, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C,
         0x85, 0x04, 0x43, 0x41, 0x4C, 0x4C};
    static unsigned char const resp_2_1_1[] =
        {0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SET UP CALL 2.1.1 - GCF 27.22.4.13.2" )
        << QByteArray( (char *)data_2_1_1, sizeof(data_2_1_1) )
        << QByteArray( (char *)resp_2_1_1, sizeof(resp_2_1_1) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "CONFIRMATION" )
        << QString( "CALL" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 0 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_3_1_1a[] =
        {0xD0, 0x30, 0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x16, 0x53, 0x65, 0x74, 0x20, 0x75, 0x70, 0x20, 0x63, 0x61, 0x6C, 0x6C,
         0x20, 0x49, 0x63, 0x6F, 0x6E, 0x20, 0x33, 0x2E, 0x31, 0x2E, 0x31, 0x86,
         0x09, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C, 0x9E, 0x02,
         0x01, 0x01};
    static unsigned char const resp_3_1_1a[] =
        {0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SET UP CALL 3.1.1A - GCF 27.22.4.13.3" )
        << QByteArray( (char *)data_3_1_1a, sizeof(data_3_1_1a) )
        << QByteArray( (char *)resp_3_1_1a, sizeof(resp_3_1_1a) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "Set up call Icon 3.1.1" )
        << QString( "" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 1 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_3_1_1b[] =
        {0xD0, 0x30, 0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x16, 0x53, 0x65, 0x74, 0x20, 0x75, 0x70, 0x20, 0x63, 0x61, 0x6C, 0x6C,
         0x20, 0x49, 0x63, 0x6F, 0x6E, 0x20, 0x33, 0x2E, 0x31, 0x2E, 0x31, 0x86,
         0x09, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C, 0x9E, 0x02,
         0x01, 0x01};
    static unsigned char const resp_3_1_1b[] =
        {0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04};
    QTest::newRow( "SET UP CALL 3.1.1B - GCF 27.22.4.13.3" )
        << QByteArray( (char *)data_3_1_1b, sizeof(data_3_1_1b) )
        << QByteArray( (char *)resp_3_1_1b, sizeof(resp_3_1_1b) )
        << QByteArray()
        << 0x0004       // Icon not displayed
        << QString( "Set up call Icon 3.1.1" )
        << QString( "" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 1 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_3_2_1a[] =
        {0xD0, 0x30, 0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x16, 0x53, 0x65, 0x74, 0x20, 0x75, 0x70, 0x20, 0x63, 0x61, 0x6C, 0x6C,
         0x20, 0x49, 0x63, 0x6F, 0x6E, 0x20, 0x33, 0x2E, 0x32, 0x2E, 0x31, 0x86,
         0x09, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C, 0x9E, 0x02,
         0x00, 0x01};
    static unsigned char const resp_3_2_1a[] =
        {0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SET UP CALL 3.2.1A - GCF 27.22.4.13.3" )
        << QByteArray( (char *)data_3_2_1a, sizeof(data_3_2_1a) )
        << QByteArray( (char *)resp_3_2_1a, sizeof(resp_3_2_1a) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "Set up call Icon 3.2.1" )
        << QString( "" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 1 << true    // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_3_2_1b[] =
        {0xD0, 0x30, 0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x16, 0x53, 0x65, 0x74, 0x20, 0x75, 0x70, 0x20, 0x63, 0x61, 0x6C, 0x6C,
         0x20, 0x49, 0x63, 0x6F, 0x6E, 0x20, 0x33, 0x2E, 0x32, 0x2E, 0x31, 0x86,
         0x09, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C, 0x9E, 0x02,
         0x00, 0x01};
    static unsigned char const resp_3_2_1b[] =
        {0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04};
    QTest::newRow( "SET UP CALL 3.2.1B - GCF 27.22.4.13.3" )
        << QByteArray( (char *)data_3_2_1b, sizeof(data_3_2_1b) )
        << QByteArray( (char *)resp_3_2_1b, sizeof(resp_3_2_1b) )
        << QByteArray()
        << 0x0004       // Icon not displayed
        << QString( "Set up call Icon 3.2.1" )
        << QString( "" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 1 << true    // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_3_3_1a[] =
        {0xD0, 0x30, 0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x16, 0x53, 0x65, 0x74, 0x20, 0x75, 0x70, 0x20, 0x63, 0x61, 0x6C, 0x6C,
         0x20, 0x49, 0x63, 0x6F, 0x6E, 0x20, 0x33, 0x2E, 0x33, 0x2E, 0x31, 0x86,
         0x09, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C, 0x9E, 0x02,
         0x01, 0x02};
    static unsigned char const resp_3_3_1a[] =
        {0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SET UP CALL 3.3.1A - GCF 27.22.4.13.3" )
        << QByteArray( (char *)data_3_3_1a, sizeof(data_3_3_1a) )
        << QByteArray( (char *)resp_3_3_1a, sizeof(resp_3_3_1a) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "Set up call Icon 3.3.1" )
        << QString( "" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 2 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_3_3_1b[] =
        {0xD0, 0x30, 0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x16, 0x53, 0x65, 0x74, 0x20, 0x75, 0x70, 0x20, 0x63, 0x61, 0x6C, 0x6C,
         0x20, 0x49, 0x63, 0x6F, 0x6E, 0x20, 0x33, 0x2E, 0x33, 0x2E, 0x31, 0x86,
         0x09, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C, 0x9E, 0x02,
         0x01, 0x02};
    static unsigned char const resp_3_3_1b[] =
        {0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04};
    QTest::newRow( "SET UP CALL 3.3.1B - GCF 27.22.4.13.3" )
        << QByteArray( (char *)data_3_3_1b, sizeof(data_3_3_1b) )
        << QByteArray( (char *)resp_3_3_1b, sizeof(resp_3_3_1b) )
        << QByteArray()
        << 0x0004       // Icon not displayed
        << QString( "Set up call Icon 3.3.1" )
        << QString( "" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 2 << false   // Icon details
        << 0 << false   // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_3_4_1a[] =
        {0xD0, 0x4C, 0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x16, 0x53, 0x65, 0x74, 0x20, 0x75, 0x70, 0x20, 0x63, 0x61, 0x6C, 0x6C,
         0x20, 0x49, 0x63, 0x6F, 0x6E, 0x20, 0x33, 0x2E, 0x34, 0x2E, 0x31, 0x86,
         0x09, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C, 0x9E, 0x02,
         0x00, 0x01, 0x85, 0x16, 0x53, 0x65, 0x74, 0x20, 0x75, 0x70, 0x20, 0x63,
         0x61, 0x6C, 0x6C, 0x20, 0x49, 0x63, 0x6F, 0x6E, 0x20, 0x33, 0x2E, 0x34,
         0x2E, 0x32, 0x9E, 0x02, 0x00, 0x01};
    static unsigned char const resp_3_4_1a[] =
        {0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SET UP CALL 3.4.1A - GCF 27.22.4.13.3" )
        << QByteArray( (char *)data_3_4_1a, sizeof(data_3_4_1a) )
        << QByteArray( (char *)resp_3_4_1a, sizeof(resp_3_4_1a) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "Set up call Icon 3.4.1" )
        << QString( "Set up call Icon 3.4.2" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 1 << true    // Icon details
        << 1 << true    // Second icon details
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_3_4_1b[] =
        {0xD0, 0x4C, 0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x16, 0x53, 0x65, 0x74, 0x20, 0x75, 0x70, 0x20, 0x63, 0x61, 0x6C, 0x6C,
         0x20, 0x49, 0x63, 0x6F, 0x6E, 0x20, 0x33, 0x2E, 0x34, 0x2E, 0x31, 0x86,
         0x09, 0x91, 0x10, 0x32, 0x04, 0x21, 0x43, 0x65, 0x1C, 0x2C, 0x9E, 0x02,
         0x00, 0x01, 0x85, 0x16, 0x53, 0x65, 0x74, 0x20, 0x75, 0x70, 0x20, 0x63,
         0x61, 0x6C, 0x6C, 0x20, 0x49, 0x63, 0x6F, 0x6E, 0x20, 0x33, 0x2E, 0x34,
         0x2E, 0x32, 0x9E, 0x02, 0x00, 0x01};
    static unsigned char const resp_3_4_1b[] =
        {0x81, 0x03, 0x01, 0x10, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04};
    QTest::newRow( "SET UP CALL 3.4.1B - GCF 27.22.4.13.3" )
        << QByteArray( (char *)data_3_4_1b, sizeof(data_3_4_1b) )
        << QByteArray( (char *)resp_3_4_1b, sizeof(resp_3_4_1b) )
        << QByteArray()
        << 0x0004       // Icon not displayed
        << QString( "Set up call Icon 3.4.1" )
        << QString( "Set up call Icon 3.4.2" )
        << QString( "+012340123456p1p2" )
        << QString( "" )
        << (int)( QSimCommand::IfNoOtherCalls )
        << (int)( QSimCommand::Voice )
        << false        // With redial
        << 0            // Duration
        << 1 << true    // Icon details
        << 1 << true    // Second icon details
        << (int)( QSimCommand::NoPduOptions );
}
