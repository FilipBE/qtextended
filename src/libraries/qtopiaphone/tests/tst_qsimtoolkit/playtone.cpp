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
// Test encoding and decoding of PLAY TONE commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.5 - PLAY TONE.
void tst_QSimToolkit::testEncodePlayTone_data()
{
    QSimToolkitData::populateDataPlayTone();
}
void tst_QSimToolkit::testEncodePlayTone()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( int, tone );
    QFETCH( int, duration );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( QByteArray, textAttribute );
    QFETCH( QString, html );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::PlayTone );
    QVERIFY( decoded.destinationDevice() == QSimCommand::Earpiece );
    QCOMPARE( decoded.text(), text );
    if ( text.isEmpty() ) {
        if ( ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
            QVERIFY( decoded.suppressUserFeedback() );
        else
            QVERIFY( !decoded.suppressUserFeedback() );
    } else {
        QVERIFY( !decoded.suppressUserFeedback() );
    }
    QCOMPARE( (int)decoded.tone(), tone );
    QCOMPARE( (int)decoded.toneTime(), duration );
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

// Test that PLAY TONE commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverPlayTone_data()
{
    QSimToolkitData::populateDataPlayTone();
}
void tst_QSimToolkit::testDeliverPlayTone()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( int, tone );
    QFETCH( int, duration );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
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
    cmd.setType( QSimCommand::PlayTone );
    cmd.setDestinationDevice( QSimCommand::Earpiece );
    cmd.setText( text );
    if ( text.isEmpty() && ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
        cmd.setSuppressUserFeedback( true );
    cmd.setTone( (QSimCommand::Tone)tone );
    cmd.setDuration( duration );
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );
    cmd.setTextAttribute( textAttribute );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QVERIFY( deliveredCommand.text() == cmd.text() );
    QVERIFY( deliveredCommand.suppressUserFeedback() == cmd.suppressUserFeedback() );
    QVERIFY( deliveredCommand.tone() == cmd.tone() );
    QVERIFY( deliveredCommand.duration() == cmd.duration() );
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

// Test the user interface in "simapp" for PLAY TONE.
void tst_QSimToolkit::testUIPlayTone_data()
{
    QSimToolkitData::populateDataPlayTone();
}
void tst_QSimToolkit::testUIPlayTone()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( int, tone );
    QFETCH( int, duration );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( QByteArray, textAttribute );
    QFETCH( QString, html );
    QFETCH( int, options );

    Q_UNUSED(html);
    Q_UNUSED(options);

    // Skip tests that we cannot test using the "simapp" UI.
    if ( resptype == 0x0030 ) {     // Command beyond ME capabilities.
        QSKIP( "", SkipSingle );
    }

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Reduce duration to prevent tests taking too long.
    duration /= 10;

    // Create the command to be tested.
    QSimCommand cmd;
    cmd.setType( QSimCommand::PlayTone );
    cmd.setDestinationDevice( QSimCommand::Earpiece );
    cmd.setText( text );
    if ( text.isEmpty() && ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
        cmd.setSuppressUserFeedback( true );
    cmd.setTone( (QSimCommand::Tone)tone );
    cmd.setDuration( duration );
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
    select();

    // Wait for the tone view to display.
    QVERIFY( waitForView( SimTone::staticMetaObject ) );

    // Wait for the tone to play for the requested duration.
    if ( resptype != 0x0010 ) {
        if ( duration )
            QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), duration + 100 ) );
        else
            QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 600 ) );
    } else {
        // Proactive SIM session terminated.
        QFutureSignal fs( server, SIGNAL(responseSeen()) );
        simapp->terminateSession();
        QVERIFY( fs.wait(100) );
    }

    // Check that the response is what we expected.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    QCOMPARE( server->lastResponse(), resp );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for PLAY TONE from the GCF test cases
// in GSM 51.010, section 27.22.4.5.
void QSimToolkitData::populateDataPlayTone()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<QString>("text");
    QTest::addColumn<int>("tone");
    QTest::addColumn<int>("duration");
    QTest::addColumn<int>("iconId");
    QTest::addColumn<bool>("iconSelfExplanatory");
    QTest::addColumn<QByteArray>("textAttribute");
    QTest::addColumn<QString>("html");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x1B, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x09, 0x44, 0x69, 0x61, 0x6C, 0x20, 0x54, 0x6F, 0x6E, 0x65, 0x8E, 0x01,
         0x01, 0x84, 0x02, 0x01, 0x05};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "PLAY TONE 1.1.1 - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Dial Tone" )
        << (int)( QSimCommand::ToneDial )
        << 5000         // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_2[] =
        {0xD0, 0x1B, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x09, 0x53, 0x75, 0x62, 0x2E, 0x20, 0x42, 0x75, 0x73, 0x79, 0x8E, 0x01,
         0x02, 0x84, 0x02, 0x01, 0x05};
    static unsigned char const resp_1_1_2[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "PLAY TONE 1.1.2 - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_2, sizeof(data_1_1_2) )
        << QByteArray( (char *)resp_1_1_2, sizeof(resp_1_1_2) )
        << 0x0000       // Command performed successfully
        << QString( "Sub. Busy" )
        << (int)( QSimCommand::ToneBusy )
        << 5000         // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_3[] =
        {0xD0, 0x1C, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x0A, 0x43, 0x6F, 0x6E, 0x67, 0x65, 0x73, 0x74, 0x69, 0x6F, 0x6E, 0x8E,
         0x01, 0x03, 0x84, 0x02, 0x01, 0x05};
    static unsigned char const resp_1_1_3[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "PLAY TONE 1.1.3 - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_3, sizeof(data_1_1_3) )
        << QByteArray( (char *)resp_1_1_3, sizeof(resp_1_1_3) )
        << 0x0000       // Command performed successfully
        << QString( "Congestion" )
        << (int)( QSimCommand::ToneCongestion )
        << 5000         // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_4[] =
        {0xD0, 0x18, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x06, 0x52, 0x50, 0x20, 0x41, 0x63, 0x6B, 0x8E, 0x01, 0x04, 0x84, 0x02,
         0x01, 0x05};
    static unsigned char const resp_1_1_4[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "PLAY TONE 1.1.4 - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_4, sizeof(data_1_1_4) )
        << QByteArray( (char *)resp_1_1_4, sizeof(resp_1_1_4) )
        << 0x0000       // Command performed successfully
        << QString( "RP Ack" )
        << (int)( QSimCommand::ToneRadioAck )
        << 5000         // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_5[] =
        {0xD0, 0x17, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x05, 0x4E, 0x6F, 0x20, 0x52, 0x50, 0x8E, 0x01, 0x05, 0x84, 0x02, 0x01,
         0x05};
    static unsigned char const resp_1_1_5[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "PLAY TONE 1.1.5 - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_5, sizeof(data_1_1_5) )
        << QByteArray( (char *)resp_1_1_5, sizeof(resp_1_1_5) )
        << 0x0000       // Command performed successfully
        << QString( "No RP" )
        << (int)( QSimCommand::ToneDropped )
        << 5000         // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_6[] =
        {0xD0, 0x1B, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x09, 0x53, 0x70, 0x65, 0x63, 0x20, 0x49, 0x6E, 0x66, 0x6F, 0x8E, 0x01,
         0x06, 0x84, 0x02, 0x01, 0x05};
    static unsigned char const resp_1_1_6[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "PLAY TONE 1.1.6 - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_6, sizeof(data_1_1_6) )
        << QByteArray( (char *)resp_1_1_6, sizeof(resp_1_1_6) )
        << 0x0000       // Command performed successfully
        << QString( "Spec Info" )
        << (int)( QSimCommand::ToneError )
        << 5000         // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_7[] =
        {0xD0, 0x1B, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x09, 0x43, 0x61, 0x6C, 0x6C, 0x20, 0x57, 0x61, 0x69, 0x74, 0x8E, 0x01,
         0x07, 0x84, 0x02, 0x01, 0x05};
    static unsigned char const resp_1_1_7[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "PLAY TONE 1.1.7 - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_7, sizeof(data_1_1_7) )
        << QByteArray( (char *)resp_1_1_7, sizeof(resp_1_1_7) )
        << 0x0000       // Command performed successfully
        << QString( "Call Wait" )
        << (int)( QSimCommand::ToneCallWaiting )
        << 5000         // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_8[] =
        {0xD0, 0x1B, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x09, 0x52, 0x69, 0x6E, 0x67, 0x20, 0x54, 0x6F, 0x6E, 0x65, 0x8E, 0x01,
         0x08, 0x84, 0x02, 0x01, 0x05};
    static unsigned char const resp_1_1_8[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "PLAY TONE 1.1.8 - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_8, sizeof(data_1_1_8) )
        << QByteArray( (char *)resp_1_1_8, sizeof(resp_1_1_8) )
        << 0x0000       // Command performed successfully
        << QString( "Ring Tone" )
        << (int)( QSimCommand::ToneRinging )
        << 5000         // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_9[] =
        {0xD0, 0x1B, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x09, 0x44, 0x69, 0x61, 0x6C, 0x20, 0x54, 0x6F, 0x6E, 0x65, 0x8E, 0x01,
         0x01, 0x84, 0x02, 0x01, 0x05};
    static unsigned char const resp_1_1_9[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "PLAY TONE 1.1.9 - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_9, sizeof(data_1_1_9) )
        << QByteArray( (char *)resp_1_1_9, sizeof(resp_1_1_9) )
        << 0x0000       // Command performed successfully
        << QString( "Dial Tone" )
        << (int)( QSimCommand::ToneDial )
        << 5000         // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_10a[] =
        {0xD0, 0x81, 0xFD, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03,
         0x85, 0x81, 0xF1, 0x54, 0x68, 0x69, 0x73, 0x20, 0x63, 0x6F, 0x6D, 0x6D,
         0x61, 0x6E, 0x64, 0x20, 0x69, 0x6E, 0x73, 0x74, 0x72, 0x75, 0x63, 0x74,
         0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x4D, 0x45, 0x20, 0x74, 0x6F, 0x20,
         0x70, 0x6C, 0x61, 0x79, 0x20, 0x61, 0x6E, 0x20, 0x61, 0x75, 0x64, 0x69,
         0x6F, 0x20, 0x74, 0x6F, 0x6E, 0x65, 0x2E, 0x20, 0x55, 0x70, 0x6F, 0x6E,
         0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x69, 0x6E, 0x67, 0x20, 0x74,
         0x68, 0x69, 0x73, 0x20, 0x63, 0x6F, 0x6D, 0x6D, 0x61, 0x6E, 0x64, 0x2C,
         0x20, 0x74, 0x68, 0x65, 0x20, 0x4D, 0x45, 0x20, 0x73, 0x68, 0x61, 0x6C,
         0x6C, 0x20, 0x63, 0x68, 0x65, 0x63, 0x6B, 0x20, 0x69, 0x66, 0x20, 0x69,
         0x74, 0x20, 0x69, 0x73, 0x20, 0x63, 0x75, 0x72, 0x72, 0x65, 0x6E, 0x74,
         0x6C, 0x79, 0x20, 0x69, 0x6E, 0x2C, 0x20, 0x6F, 0x72, 0x20, 0x69, 0x6E,
         0x20, 0x74, 0x68, 0x65, 0x20, 0x70, 0x72, 0x6F, 0x63, 0x65, 0x73, 0x73,
         0x20, 0x6F, 0x66, 0x20, 0x73, 0x65, 0x74, 0x74, 0x69, 0x6E, 0x67, 0x20,
         0x75, 0x70, 0x20, 0x28, 0x53, 0x45, 0x54, 0x2D, 0x55, 0x50, 0x20, 0x6D,
         0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x20, 0x73, 0x65, 0x6E, 0x74, 0x20,
         0x74, 0x6F, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6E, 0x65, 0x74, 0x77, 0x6F,
         0x72, 0x6B, 0x2C, 0x20, 0x73, 0x65, 0x65, 0x20, 0x47, 0x53, 0x4D, 0x22,
         0x30, 0x34, 0x2E, 0x30, 0x38, 0x22, 0x28, 0x38, 0x29, 0x29, 0x2C, 0x20,
         0x61, 0x20, 0x73, 0x70, 0x65, 0x65, 0x63, 0x68, 0x20, 0x63, 0x61, 0x6C,
         0x6C, 0x2E, 0x20, 0x2D, 0x20, 0x49, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20,
         0x4D, 0x45, 0x20, 0x49};
    static unsigned char const resp_1_1_10a[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "PLAY TONE 1.1.10A - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_10a, sizeof(data_1_1_10a) )
        << QByteArray( (char *)resp_1_1_10a, sizeof(resp_1_1_10a) )
        << 0x0000       // Command performed successfully
        << QString( "This command instructs the ME to play an audio tone. Upon receiving "
                    "this command, the ME shall check if it is currently in, or in the "
                    "process of setting up (SET-UP message sent to the network, see "
                    "GSM\"04.08\"(8)), a speech call. - If the ME I" )
        << (int)( QSimCommand::ToneNone )
        << 0            // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_10b[] =
        {0xD0, 0x81, 0xFD, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03,
         0x85, 0x81, 0xF1, 0x54, 0x68, 0x69, 0x73, 0x20, 0x63, 0x6F, 0x6D, 0x6D,
         0x61, 0x6E, 0x64, 0x20, 0x69, 0x6E, 0x73, 0x74, 0x72, 0x75, 0x63, 0x74,
         0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x4D, 0x45, 0x20, 0x74, 0x6F, 0x20,
         0x70, 0x6C, 0x61, 0x79, 0x20, 0x61, 0x6E, 0x20, 0x61, 0x75, 0x64, 0x69,
         0x6F, 0x20, 0x74, 0x6F, 0x6E, 0x65, 0x2E, 0x20, 0x55, 0x70, 0x6F, 0x6E,
         0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x69, 0x6E, 0x67, 0x20, 0x74,
         0x68, 0x69, 0x73, 0x20, 0x63, 0x6F, 0x6D, 0x6D, 0x61, 0x6E, 0x64, 0x2C,
         0x20, 0x74, 0x68, 0x65, 0x20, 0x4D, 0x45, 0x20, 0x73, 0x68, 0x61, 0x6C,
         0x6C, 0x20, 0x63, 0x68, 0x65, 0x63, 0x6B, 0x20, 0x69, 0x66, 0x20, 0x69,
         0x74, 0x20, 0x69, 0x73, 0x20, 0x63, 0x75, 0x72, 0x72, 0x65, 0x6E, 0x74,
         0x6C, 0x79, 0x20, 0x69, 0x6E, 0x2C, 0x20, 0x6F, 0x72, 0x20, 0x69, 0x6E,
         0x20, 0x74, 0x68, 0x65, 0x20, 0x70, 0x72, 0x6F, 0x63, 0x65, 0x73, 0x73,
         0x20, 0x6F, 0x66, 0x20, 0x73, 0x65, 0x74, 0x74, 0x69, 0x6E, 0x67, 0x20,
         0x75, 0x70, 0x20, 0x28, 0x53, 0x45, 0x54, 0x2D, 0x55, 0x50, 0x20, 0x6D,
         0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x20, 0x73, 0x65, 0x6E, 0x74, 0x20,
         0x74, 0x6F, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6E, 0x65, 0x74, 0x77, 0x6F,
         0x72, 0x6B, 0x2C, 0x20, 0x73, 0x65, 0x65, 0x20, 0x47, 0x53, 0x4D, 0x22,
         0x30, 0x34, 0x2E, 0x30, 0x38, 0x22, 0x28, 0x38, 0x29, 0x29, 0x2C, 0x20,
         0x61, 0x20, 0x73, 0x70, 0x65, 0x65, 0x63, 0x68, 0x20, 0x63, 0x61, 0x6C,
         0x6C, 0x2E, 0x20, 0x2D, 0x20, 0x49, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20,
         0x4D, 0x45, 0x20, 0x49};
    static unsigned char const resp_1_1_10b[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x30};
    QTest::newRow( "PLAY TONE 1.1.10B - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_10b, sizeof(data_1_1_10b) )
        << QByteArray( (char *)resp_1_1_10b, sizeof(resp_1_1_10b) )
        << 0x0030       // Command beyond ME capabilities
        << QString( "This command instructs the ME to play an audio tone. Upon receiving "
                    "this command, the ME shall check if it is currently in, or in the "
                    "process of setting up (SET-UP message sent to the network, see "
                    "GSM\"04.08\"(8)), a speech call. - If the ME I" )
        << (int)( QSimCommand::ToneNone )
        << 0            // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_11a[] =
        {0xD0, 0x16, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x04, 0x42, 0x65, 0x65, 0x70, 0x8E, 0x01, 0x10, 0x84, 0x02, 0x01, 0x01};
    static unsigned char const resp_1_1_11a[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "PLAY TONE 1.1.11A - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_11a, sizeof(data_1_1_11a) )
        << QByteArray( (char *)resp_1_1_11a, sizeof(resp_1_1_11a) )
        << 0x0000       // Command performed successfully
        << QString( "Beep" )
        << (int)( QSimCommand::ToneGeneralBeep )
        << 1000         // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_11b[] =
        {0xD0, 0x16, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x04, 0x42, 0x65, 0x65, 0x70, 0x8E, 0x01, 0x10, 0x84, 0x02, 0x01, 0x01};
    static unsigned char const resp_1_1_11b[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x30};
    QTest::newRow( "PLAY TONE 1.1.11B - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_11b, sizeof(data_1_1_11b) )
        << QByteArray( (char *)resp_1_1_11b, sizeof(resp_1_1_11b) )
        << 0x0030       // Command beyond ME capabilities
        << QString( "Beep" )
        << (int)( QSimCommand::ToneGeneralBeep )
        << 1000         // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_12a[] =
        {0xD0, 0x1A, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x08, 0x50, 0x6F, 0x73, 0x69, 0x74, 0x69, 0x76, 0x65, 0x8E, 0x01, 0x11,
         0x84, 0x02, 0x01, 0x01};
    static unsigned char const resp_1_1_12a[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "PLAY TONE 1.1.12A - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_12a, sizeof(data_1_1_12a) )
        << QByteArray( (char *)resp_1_1_12a, sizeof(resp_1_1_12a) )
        << 0x0000       // Command performed successfully
        << QString( "Positive" )
        << (int)( QSimCommand::TonePositiveBeep )
        << 1000         // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_12b[] =
        {0xD0, 0x1A, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x08, 0x50, 0x6F, 0x73, 0x69, 0x74, 0x69, 0x76, 0x65, 0x8E, 0x01, 0x11,
         0x84, 0x02, 0x01, 0x01};
    static unsigned char const resp_1_1_12b[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x30};
    QTest::newRow( "PLAY TONE 1.1.12B - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_12b, sizeof(data_1_1_12b) )
        << QByteArray( (char *)resp_1_1_12b, sizeof(resp_1_1_12b) )
        << 0x0030       // Command beyond ME capabilities
        << QString( "Positive" )
        << (int)( QSimCommand::TonePositiveBeep )
        << 1000         // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_13a[] =
        {0xD0, 0x1A, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x08, 0x4E, 0x65, 0x67, 0x61, 0x74, 0x69, 0x76, 0x65, 0x8E, 0x01, 0x12,
         0x84, 0x02, 0x01, 0x01};
    static unsigned char const resp_1_1_13a[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "PLAY TONE 1.1.13A - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_13a, sizeof(data_1_1_13a) )
        << QByteArray( (char *)resp_1_1_13a, sizeof(resp_1_1_13a) )
        << 0x0000       // Command performed successfully
        << QString( "Negative" )
        << (int)( QSimCommand::ToneNegativeBeep )
        << 1000         // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_13b[] =
        {0xD0, 0x1A, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x08, 0x4E, 0x65, 0x67, 0x61, 0x74, 0x69, 0x76, 0x65, 0x8E, 0x01, 0x12,
         0x84, 0x02, 0x01, 0x01};
    static unsigned char const resp_1_1_13b[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x30};
    QTest::newRow( "PLAY TONE 1.1.13B - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_13b, sizeof(data_1_1_13b) )
        << QByteArray( (char *)resp_1_1_13b, sizeof(resp_1_1_13b) )
        << 0x0030       // Command beyond ME capabilities
        << QString( "Negative" )
        << (int)( QSimCommand::ToneNegativeBeep )
        << 1000         // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_14a[] =
        {0xD0, 0x17, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x05, 0x51, 0x75, 0x69, 0x63, 0x6B, 0x8E, 0x01, 0x10, 0x84, 0x02, 0x02,
         0x02};
    static unsigned char const resp_1_1_14a[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "PLAY TONE 1.1.14A - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_14a, sizeof(data_1_1_14a) )
        << QByteArray( (char *)resp_1_1_14a, sizeof(resp_1_1_14a) )
        << 0x0000       // Command performed successfully
        << QString( "Quick" )
        << (int)( QSimCommand::ToneGeneralBeep )
        << 200          // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_14b[] =
        {0xD0, 0x17, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x05, 0x51, 0x75, 0x69, 0x63, 0x6B, 0x8E, 0x01, 0x10, 0x84, 0x02, 0x02,
         0x02};
    static unsigned char const resp_1_1_14b[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x30};
    QTest::newRow( "PLAY TONE 1.1.14B - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_14b, sizeof(data_1_1_14b) )
        << QByteArray( (char *)resp_1_1_14b, sizeof(resp_1_1_14b) )
        << 0x0030       // Command beyond ME capabilities
        << QString( "Quick" )
        << (int)( QSimCommand::ToneGeneralBeep )
        << 200          // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_15[] =
        {0xD0, 0x19, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x07, 0x3C, 0x41, 0x42, 0x4F, 0x52, 0x54, 0x3E, 0x8E, 0x01, 0x06, 0x84,
         0x02, 0x00, 0x01};
    static unsigned char const resp_1_1_15[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x10};
    QTest::newRow( "PLAY TONE 1.1.15 - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_15, sizeof(data_1_1_15) )
        << QByteArray( (char *)resp_1_1_15, sizeof(resp_1_1_15) )
        << 0x0010       // Proactive SIM session terminated
        << QString( "<ABORT>" )
        << (int)( QSimCommand::ToneError )
        << 60000        // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_16[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03};
    static unsigned char const resp_1_1_16[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "PLAY TONE 1.1.16 - GCF 27.22.4.5" )
        << QByteArray( (char *)data_1_1_16, sizeof(data_1_1_16) )
        << QByteArray( (char *)resp_1_1_16, sizeof(resp_1_1_16) )
        << 0x0000       // Command performed successfully
        << QString( "" )
        << (int)( QSimCommand::ToneNone )
        << 0            // Duration
        << 0 << false   // No icon
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_4_1_1[] =
        {0xD0, 0x28, 0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x81, 0x03, 0x85,
         0x10, 0x54, 0x65, 0x78, 0x74, 0x20, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62,
         0x75, 0x74, 0x65, 0x20, 0x31, 0x8E, 0x01, 0x11, 0x84, 0x02, 0x01, 0x01,
         0xD0, 0x04, 0x00, 0x10, 0x00, 0xB4};
    static unsigned char const resp_4_1_1[] =
        {0x81, 0x03, 0x01, 0x20, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const attr_4_1_1[] =
        {0x00, 0x10, 0x00, 0xB4};
    QTest::newRow( "PLAY TONE 4.1.1 - GCF 27.22.4.5.4" )
        << QByteArray( (char *)data_4_1_1, sizeof(data_4_1_1) )
        << QByteArray( (char *)resp_4_1_1, sizeof(resp_4_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Text Attribute 1" )
        << (int)( QSimCommand::TonePositiveBeep )
        << 1000         // Duration
        << 0 << false   // No icon
        << QByteArray( (char *)attr_4_1_1, sizeof(attr_4_1_1) )
        << QString( "<body bgcolor=\"#FFFF00\"><div align=\"left\"><font color=\"#008000\">Text Attribute 1</font></div></body>" )
        << (int)( QSimCommand::NoPduOptions );
}
