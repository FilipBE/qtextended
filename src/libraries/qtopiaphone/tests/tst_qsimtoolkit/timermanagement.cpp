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
// Test encoding and decoding of TIMER MANAGEMENT commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.21 - TIMER MANAGEMENT.
void tst_QSimToolkit::testEncodeTimerManagement_data()
{
    QSimToolkitData::populateDataTimerManagement();
}
void tst_QSimToolkit::testEncodeTimerManagement()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, envelope );
    QFETCH( int, resptype );
    QFETCH( int, timernum );
    QFETCH( int, action );
    QFETCH( int, hours );
    QFETCH( int, minutes );
    QFETCH( int, seconds );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Compose a byte array representing the timer timer.
    QByteArray timer;
    timer += (char)timernum;

    // Compose a time value from the hours, minutes, and seconds.
    // The format is reversed BCD.
    QByteArray time;
    time += (char)((hours / 10) + ((hours % 10) << 4));
    time += (char)((minutes / 10) + ((minutes % 10) << 4));
    time += (char)((seconds / 10) + ((seconds % 10) << 4));

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::TimerManagement );
    QVERIFY( decoded.destinationDevice() == QSimCommand::ME );
    QCOMPARE( decoded.qualifier(), action );
    QCOMPARE( decoded.extensionField(0xA4), timer );
    if ( action == Timer_Start ) {
        QCOMPARE( decoded.extensionField(0xA5), time );
    }

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
    if ( hours != -2 )
        QCOMPARE( decodedResp.extensionField(0xA4), timer );
    if ( action != Timer_Start && hours >= 0 )
        QCOMPARE( decodedResp.extensionField(0xA5), time );

    // Check that the original terminal response PDU can be reconstructed correctly.
    QCOMPARE( decodedResp.toPdu(), resp );

    // Bail out if no envelope to test.
    if ( envelope.isEmpty() )
        return;

    // Check that the envelope PDU can be parsed correctly.
    QSimEnvelope decodedEnv = QSimEnvelope::fromPdu(envelope);
    QVERIFY( decodedEnv.type() == QSimEnvelope::TimerExpiration );
    QVERIFY( decodedEnv.sourceDevice() == QSimCommand::ME );
    QCOMPARE( decoded.extensionField(0xA4), timer );
    QCOMPARE( decoded.extensionField(0xA5), time );

    // Check that the original envelope PDU can be reconstructed correctly.
    QCOMPARE( decodedEnv.toPdu(), envelope );
}

// Test that TIMER MANAGEMENT commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverTimerManagement_data()
{
    QSimToolkitData::populateDataTimerManagement();
}
void tst_QSimToolkit::testDeliverTimerManagement()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, envelope );
    QFETCH( int, resptype );
    QFETCH( int, timernum );
    QFETCH( int, action );
    QFETCH( int, hours );
    QFETCH( int, minutes );
    QFETCH( int, seconds );
    QFETCH( int, options );

    Q_UNUSED(resptype);

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose a byte array representing the timer timer.
    QByteArray timer;
    timer += (char)timernum;

    // Compose a time value from the hours, minutes, and seconds.
    // The format is reversed BCD.
    QByteArray time;
    time += (char)((hours / 10) + ((hours % 10) << 4));
    time += (char)((minutes / 10) + ((minutes % 10) << 4));
    time += (char)((seconds / 10) + ((seconds % 10) << 4));

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::TimerManagement );
    cmd.setQualifier( action );
    cmd.addExtensionField( 0xA4, timer );
    if ( action == Timer_Start )
        cmd.addExtensionField( 0xA5, time );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QCOMPARE( deliveredCommand.qualifier(), action );
    QCOMPARE( deliveredCommand.extensionField(0xA4), timer );
    if ( action == Timer_Start )
        QCOMPARE( deliveredCommand.extensionField(0xA5), time );
    QCOMPARE( deliveredCommand.toPdu( (QSimCommand::ToPduOptions)options ), data );

    // The response should have been sent immediately, and should always be success.
    // We cannot test the actual behaviour with this interface.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    static unsigned char const ok_resp[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QByteArray resp2 = QByteArray( (char *)ok_resp, sizeof( ok_resp ) );
    resp2[4] = (char)action;
    QCOMPARE( server->lastResponse(), resp2 );

    // Bail out if no envelope in the test data.
    if ( envelope.isEmpty() )
        return;

    // Compose and send the envelope.
    QSimEnvelope env;
    env.setType( QSimEnvelope::TimerExpiration );
    env.addExtensionField( 0xA4, timer );
    env.addExtensionField( 0xA5, time );
    client->sendEnvelope( env );

    // Wait for the envelope to be received.
    QVERIFY( QFutureSignal::wait( server, SIGNAL(envelopeSeen()), 100 ) );

    // Check that the envelope is what we expected to get and that we didn't
    // get any further terminal responses.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 1 );
    QCOMPARE( server->lastEnvelope(), envelope );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for TIMER MANAGEMENT from the GCF test cases
// in GSM 51.010, section 27.22.4.21.
void QSimToolkitData::populateDataTimerManagement()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<QByteArray>("envelope");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<int>("timernum");
    QTest::addColumn<int>("action");
    QTest::addColumn<int>("hours");
    QTest::addColumn<int>("minutes");
    QTest::addColumn<int>("seconds");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x01, 0xA5, 0x03, 0x00, 0x50, 0x00};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x01};
    QTest::newRow( "TIMER MANAGEMENT 1.1.1 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 1            // Timer number
        << Timer_Start
        << 0 << 5 << 0  // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_2[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x01};
    static unsigned char const resp_1_1_2[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x01, 0xA5, 0x03, 0x00, 0x50, 0x00};
    QTest::newRow( "TIMER MANAGEMENT 1.1.2 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_1_2, sizeof(data_1_1_2) )
        << QByteArray( (char *)resp_1_1_2, sizeof(resp_1_1_2) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 1            // Timer number
        << Timer_GetCurrentValue
        << 0 << 5 << 0  // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_3[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x01, 0xA5, 0x03, 0x00, 0x10, 0x03};
    static unsigned char const resp_1_1_3[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x01};
    QTest::newRow( "TIMER MANAGEMENT 1.1.3 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_1_3, sizeof(data_1_1_3) )
        << QByteArray( (char *)resp_1_1_3, sizeof(resp_1_1_3) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 1            // Timer number
        << Timer_Start
        << 0 << 1 << 30 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_4[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x01};
    static unsigned char const resp_1_1_4[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x01, 0xA5, 0x03, 0x00, 0x10, 0x03};
    QTest::newRow( "TIMER MANAGEMENT 1.1.4 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_1_4, sizeof(data_1_1_4) )
        << QByteArray( (char *)resp_1_1_4, sizeof(resp_1_1_4) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 1            // Timer number
        << Timer_Deactivate
        << 0 << 1 << 30 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x02, 0xA5, 0x03, 0x32, 0x95, 0x95};
    static unsigned char const resp_1_2_1[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x02};
    QTest::newRow( "TIMER MANAGEMENT 1.2.1 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 2            // Timer number
        << Timer_Start
        << 23<< 59 << 59// Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_2[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x02};
    static unsigned char const resp_1_2_2[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x02, 0xA5, 0x03, 0x32, 0x95, 0x95};
    QTest::newRow( "TIMER MANAGEMENT 1.2.2 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_2_2, sizeof(data_1_2_2) )
        << QByteArray( (char *)resp_1_2_2, sizeof(resp_1_2_2) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 2            // Timer number
        << Timer_GetCurrentValue
        << 23<< 59 << 59// Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_3[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x02, 0xA5, 0x03, 0x00, 0x10, 0x01};
    static unsigned char const resp_1_2_3[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x02};
    QTest::newRow( "TIMER MANAGEMENT 1.2.3 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_2_3, sizeof(data_1_2_3) )
        << QByteArray( (char *)resp_1_2_3, sizeof(resp_1_2_3) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 2            // Timer number
        << Timer_Start
        << 0 << 1 << 10 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_4[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x02};
    static unsigned char const resp_1_2_4[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x02, 0xA5, 0x03, 0x00, 0x10, 0x01};
    QTest::newRow( "TIMER MANAGEMENT 1.2.4 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_2_4, sizeof(data_1_2_4) )
        << QByteArray( (char *)resp_1_2_4, sizeof(resp_1_2_4) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 2            // Timer number
        << Timer_Deactivate
        << 0 << 1 << 10 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x08, 0xA5, 0x03, 0x00, 0x02, 0x00};
    static unsigned char const resp_1_3_1[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x08};
    QTest::newRow( "TIMER MANAGEMENT 1.3.1 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_3_1, sizeof(data_1_3_1) )
        << QByteArray( (char *)resp_1_3_1, sizeof(resp_1_3_1) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 8            // Timer number
        << Timer_Start
        << 0 << 20 << 0 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_2[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x08};
    static unsigned char const resp_1_3_2[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x08, 0xA5, 0x03, 0x00, 0x02, 0x00};
    QTest::newRow( "TIMER MANAGEMENT 1.3.2 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_3_2, sizeof(data_1_3_2) )
        << QByteArray( (char *)resp_1_3_2, sizeof(resp_1_3_2) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 8            // Timer number
        << Timer_GetCurrentValue
        << 0 << 20 << 0 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_3[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x08, 0xA5, 0x03, 0x10, 0x00, 0x00};
    static unsigned char const resp_1_3_3[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x08};
    QTest::newRow( "TIMER MANAGEMENT 1.3.3 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_3_3, sizeof(data_1_3_3) )
        << QByteArray( (char *)resp_1_3_3, sizeof(resp_1_3_3) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 8            // Timer number
        << Timer_Start
        << 1 << 0 << 0  // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_4[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x08};
    static unsigned char const resp_1_3_4[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x08, 0xA5, 0x03, 0x10, 0x00, 0x00};
    QTest::newRow( "TIMER MANAGEMENT 1.3.4 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_3_4, sizeof(data_1_3_4) )
        << QByteArray( (char *)resp_1_3_4, sizeof(resp_1_3_4) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 8            // Timer number
        << Timer_Deactivate
        << 1 << 0 << 0  // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_1a[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x01};
    static unsigned char const resp_1_4_1a[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24,
         0xA4, 0x01, 0x01};
    QTest::newRow( "TIMER MANAGEMENT 1.4.1A - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_4_1a, sizeof(data_1_4_1a) )
        << QByteArray( (char *)resp_1_4_1a, sizeof(resp_1_4_1a) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 1            // Timer number
        << Timer_GetCurrentValue
        << -1 << -1 << -1 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_1b[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x01};
    static unsigned char const resp_1_4_1b[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24};
    QTest::newRow( "TIMER MANAGEMENT 1.4.1B - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_4_1b, sizeof(data_1_4_1b) )
        << QByteArray( (char *)resp_1_4_1b, sizeof(resp_1_4_1b) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 1            // Timer number
        << Timer_GetCurrentValue
        << -2 << -2 << -2 // Timer value (number not present in response)
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_2a[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x02};
    static unsigned char const resp_1_4_2a[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24,
         0xA4, 0x01, 0x02};
    QTest::newRow( "TIMER MANAGEMENT 1.4.2A - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_4_2a, sizeof(data_1_4_2a) )
        << QByteArray( (char *)resp_1_4_2a, sizeof(resp_1_4_2a) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 2            // Timer number
        << Timer_GetCurrentValue
        << -1 << -1 << -1 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_2b[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x02};
    static unsigned char const resp_1_4_2b[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24};
    QTest::newRow( "TIMER MANAGEMENT 1.4.2B - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_4_2b, sizeof(data_1_4_2b) )
        << QByteArray( (char *)resp_1_4_2b, sizeof(resp_1_4_2b) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 2            // Timer number
        << Timer_GetCurrentValue
        << -2 << -2 << -2 // Timer value (number not present in response)
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_3a[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x03};
    static unsigned char const resp_1_4_3a[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24,
         0xA4, 0x01, 0x03};
    QTest::newRow( "TIMER MANAGEMENT 1.4.3A - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_4_3a, sizeof(data_1_4_3a) )
        << QByteArray( (char *)resp_1_4_3a, sizeof(resp_1_4_3a) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 3            // Timer number
        << Timer_GetCurrentValue
        << -1 << -1 << -1 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_3b[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x03};
    static unsigned char const resp_1_4_3b[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24};
    QTest::newRow( "TIMER MANAGEMENT 1.4.3B - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_4_3b, sizeof(data_1_4_3b) )
        << QByteArray( (char *)resp_1_4_3b, sizeof(resp_1_4_3b) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 3            // Timer number
        << Timer_GetCurrentValue
        << -2 << -2 << -2 // Timer value (number not present in response)
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_4a[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x04};
    static unsigned char const resp_1_4_4a[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24,
         0xA4, 0x01, 0x04};
    QTest::newRow( "TIMER MANAGEMENT 1.4.4A - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_4_4a, sizeof(data_1_4_4a) )
        << QByteArray( (char *)resp_1_4_4a, sizeof(resp_1_4_4a) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 4            // Timer number
        << Timer_GetCurrentValue
        << -1 << -1 << -1 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_4b[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x04};
    static unsigned char const resp_1_4_4b[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24};
    QTest::newRow( "TIMER MANAGEMENT 1.4.4B - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_4_4b, sizeof(data_1_4_4b) )
        << QByteArray( (char *)resp_1_4_4b, sizeof(resp_1_4_4b) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 4            // Timer number
        << Timer_GetCurrentValue
        << -2 << -2 << -2 // Timer value (number not present in response)
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_5a[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x05};
    static unsigned char const resp_1_4_5a[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24,
         0xA4, 0x01, 0x05};
    QTest::newRow( "TIMER MANAGEMENT 1.4.5A - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_4_5a, sizeof(data_1_4_5a) )
        << QByteArray( (char *)resp_1_4_5a, sizeof(resp_1_4_5a) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 5            // Timer number
        << Timer_GetCurrentValue
        << -1 << -1 << -1 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_5b[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x05};
    static unsigned char const resp_1_4_5b[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24};
    QTest::newRow( "TIMER MANAGEMENT 1.4.5B - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_4_5b, sizeof(data_1_4_5b) )
        << QByteArray( (char *)resp_1_4_5b, sizeof(resp_1_4_5b) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 5            // Timer number
        << Timer_GetCurrentValue
        << -2 << -2 << -2 // Timer value (number not present in response)
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_6a[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x06};
    static unsigned char const resp_1_4_6a[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24,
         0xA4, 0x01, 0x06};
    QTest::newRow( "TIMER MANAGEMENT 1.4.6A - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_4_6a, sizeof(data_1_4_6a) )
        << QByteArray( (char *)resp_1_4_6a, sizeof(resp_1_4_6a) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 6            // Timer number
        << Timer_GetCurrentValue
        << -1 << -1 << -1 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_6b[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x06};
    static unsigned char const resp_1_4_6b[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24};
    QTest::newRow( "TIMER MANAGEMENT 1.4.6B - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_4_6b, sizeof(data_1_4_6b) )
        << QByteArray( (char *)resp_1_4_6b, sizeof(resp_1_4_6b) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 6            // Timer number
        << Timer_GetCurrentValue
        << -2 << -2 << -2 // Timer value (number not present in response)
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_7a[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x07};
    static unsigned char const resp_1_4_7a[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24,
         0xA4, 0x01, 0x07};
    QTest::newRow( "TIMER MANAGEMENT 1.4.7A - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_4_7a, sizeof(data_1_4_7a) )
        << QByteArray( (char *)resp_1_4_7a, sizeof(resp_1_4_7a) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 7            // Timer number
        << Timer_GetCurrentValue
        << -1 << -1 << -1 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_7b[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x07};
    static unsigned char const resp_1_4_7b[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24};
    QTest::newRow( "TIMER MANAGEMENT 1.4.7B - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_4_7b, sizeof(data_1_4_7b) )
        << QByteArray( (char *)resp_1_4_7b, sizeof(resp_1_4_7b) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 7            // Timer number
        << Timer_GetCurrentValue
        << -2 << -2 << -2 // Timer value (number not present in response)
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_8a[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x08};
    static unsigned char const resp_1_4_8a[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24,
         0xA4, 0x01, 0x08};
    QTest::newRow( "TIMER MANAGEMENT 1.4.8A - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_4_8a, sizeof(data_1_4_8a) )
        << QByteArray( (char *)resp_1_4_8a, sizeof(resp_1_4_8a) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 8            // Timer number
        << Timer_GetCurrentValue
        << -1 << -1 << -1 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_8b[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x08};
    static unsigned char const resp_1_4_8b[] =
        {0x81, 0x03, 0x01, 0x27, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24};
    QTest::newRow( "TIMER MANAGEMENT 1.4.8B - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_4_8b, sizeof(data_1_4_8b) )
        << QByteArray( (char *)resp_1_4_8b, sizeof(resp_1_4_8b) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 8            // Timer number
        << Timer_GetCurrentValue
        << -2 << -2 << -2 // Timer value (number not present in response)
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_1a[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x01};
    static unsigned char const resp_1_5_1a[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24,
         0xA4, 0x01, 0x01};
    QTest::newRow( "TIMER MANAGEMENT 1.5.1A - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_5_1a, sizeof(data_1_5_1a) )
        << QByteArray( (char *)resp_1_5_1a, sizeof(resp_1_5_1a) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 1            // Timer number
        << Timer_Deactivate
        << -1 << -1 << -1 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_1b[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x01};
    static unsigned char const resp_1_5_1b[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24};
    QTest::newRow( "TIMER MANAGEMENT 1.5.1B - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_5_1b, sizeof(data_1_5_1b) )
        << QByteArray( (char *)resp_1_5_1b, sizeof(resp_1_5_1b) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 1            // Timer number
        << Timer_Deactivate
        << -2 << -2 << -2 // Timer value (number not present in response)
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_2a[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x02};
    static unsigned char const resp_1_5_2a[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24,
         0xA4, 0x01, 0x02};
    QTest::newRow( "TIMER MANAGEMENT 1.5.2A - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_5_2a, sizeof(data_1_5_2a) )
        << QByteArray( (char *)resp_1_5_2a, sizeof(resp_1_5_2a) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 2            // Timer number
        << Timer_Deactivate
        << -1 << -1 << -1 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_2b[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x02};
    static unsigned char const resp_1_5_2b[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24};
    QTest::newRow( "TIMER MANAGEMENT 1.5.2B - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_5_2b, sizeof(data_1_5_2b) )
        << QByteArray( (char *)resp_1_5_2b, sizeof(resp_1_5_2b) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 2            // Timer number
        << Timer_Deactivate
        << -2 << -2 << -2 // Timer value (number not present in response)
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_3a[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x03};
    static unsigned char const resp_1_5_3a[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24,
         0xA4, 0x01, 0x03};
    QTest::newRow( "TIMER MANAGEMENT 1.5.3A - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_5_3a, sizeof(data_1_5_3a) )
        << QByteArray( (char *)resp_1_5_3a, sizeof(resp_1_5_3a) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 3            // Timer number
        << Timer_Deactivate
        << -1 << -1 << -1 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_3b[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x03};
    static unsigned char const resp_1_5_3b[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24};
    QTest::newRow( "TIMER MANAGEMENT 1.5.3B - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_5_3b, sizeof(data_1_5_3b) )
        << QByteArray( (char *)resp_1_5_3b, sizeof(resp_1_5_3b) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 3            // Timer number
        << Timer_Deactivate
        << -2 << -2 << -2 // Timer value (number not present in response)
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_4a[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x04};
    static unsigned char const resp_1_5_4a[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24,
         0xA4, 0x01, 0x04};
    QTest::newRow( "TIMER MANAGEMENT 1.5.4A - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_5_4a, sizeof(data_1_5_4a) )
        << QByteArray( (char *)resp_1_5_4a, sizeof(resp_1_5_4a) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 4            // Timer number
        << Timer_Deactivate
        << -1 << -1 << -1 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_4b[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x04};
    static unsigned char const resp_1_5_4b[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24};
    QTest::newRow( "TIMER MANAGEMENT 1.5.4B - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_5_4b, sizeof(data_1_5_4b) )
        << QByteArray( (char *)resp_1_5_4b, sizeof(resp_1_5_4b) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 4            // Timer number
        << Timer_Deactivate
        << -2 << -2 << -2 // Timer value (number not present in response)
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_5a[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x05};
    static unsigned char const resp_1_5_5a[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24,
         0xA4, 0x01, 0x05};
    QTest::newRow( "TIMER MANAGEMENT 1.5.5A - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_5_5a, sizeof(data_1_5_5a) )
        << QByteArray( (char *)resp_1_5_5a, sizeof(resp_1_5_5a) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 5            // Timer number
        << Timer_Deactivate
        << -1 << -1 << -1 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_5b[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x05};
    static unsigned char const resp_1_5_5b[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24};
    QTest::newRow( "TIMER MANAGEMENT 1.5.5B - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_5_5b, sizeof(data_1_5_5b) )
        << QByteArray( (char *)resp_1_5_5b, sizeof(resp_1_5_5b) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 5            // Timer number
        << Timer_Deactivate
        << -2 << -2 << -2 // Timer value (number not present in response)
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_6a[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x06};
    static unsigned char const resp_1_5_6a[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24,
         0xA4, 0x01, 0x06};
    QTest::newRow( "TIMER MANAGEMENT 1.5.6A - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_5_6a, sizeof(data_1_5_6a) )
        << QByteArray( (char *)resp_1_5_6a, sizeof(resp_1_5_6a) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 6            // Timer number
        << Timer_Deactivate
        << -1 << -1 << -1 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_6b[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x06};
    static unsigned char const resp_1_5_6b[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24};
    QTest::newRow( "TIMER MANAGEMENT 1.5.6B - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_5_6b, sizeof(data_1_5_6b) )
        << QByteArray( (char *)resp_1_5_6b, sizeof(resp_1_5_6b) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 6            // Timer number
        << Timer_Deactivate
        << -2 << -2 << -2 // Timer value (number not present in response)
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_7a[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x07};
    static unsigned char const resp_1_5_7a[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24,
         0xA4, 0x01, 0x07};
    QTest::newRow( "TIMER MANAGEMENT 1.5.7A - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_5_7a, sizeof(data_1_5_7a) )
        << QByteArray( (char *)resp_1_5_7a, sizeof(resp_1_5_7a) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 7            // Timer number
        << Timer_Deactivate
        << -1 << -1 << -1 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_7b[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x07};
    static unsigned char const resp_1_5_7b[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24};
    QTest::newRow( "TIMER MANAGEMENT 1.5.7B - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_5_7b, sizeof(data_1_5_7b) )
        << QByteArray( (char *)resp_1_5_7b, sizeof(resp_1_5_7b) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 7            // Timer number
        << Timer_Deactivate
        << -2 << -2 << -2 // Timer value (number not present in response)
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_8a[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x08};
    static unsigned char const resp_1_5_8a[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24,
         0xA4, 0x01, 0x08};
    QTest::newRow( "TIMER MANAGEMENT 1.5.8A - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_5_8a, sizeof(data_1_5_8a) )
        << QByteArray( (char *)resp_1_5_8a, sizeof(resp_1_5_8a) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 8            // Timer number
        << Timer_Deactivate
        << -1 << -1 << -1 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_8b[] =
        {0xD0, 0x0C, 0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x08};
    static unsigned char const resp_1_5_8b[] =
        {0x81, 0x03, 0x01, 0x27, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x24};
    QTest::newRow( "TIMER MANAGEMENT 1.5.8B - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_5_8b, sizeof(data_1_5_8b) )
        << QByteArray( (char *)resp_1_5_8b, sizeof(resp_1_5_8b) )
        << QByteArray()
        << 0x0024       // Action in contradiction with timer state
        << 8            // Timer number
        << Timer_Deactivate
        << -2 << -2 << -2 // Timer value (number not present in response)
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_6_1[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x01, 0xA5, 0x03, 0x00, 0x00, 0x50};
    static unsigned char const resp_1_6_1[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x01};
    QTest::newRow( "TIMER MANAGEMENT 1.6.1 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_6_1, sizeof(data_1_6_1) )
        << QByteArray( (char *)resp_1_6_1, sizeof(resp_1_6_1) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 1            // Timer number
        << Timer_Start
        << 0 << 0 << 5  // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_6_2[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x02, 0xA5, 0x03, 0x00, 0x00, 0x50};
    static unsigned char const resp_1_6_2[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x02};
    QTest::newRow( "TIMER MANAGEMENT 1.6.2 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_6_2, sizeof(data_1_6_2) )
        << QByteArray( (char *)resp_1_6_2, sizeof(resp_1_6_2) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 2            // Timer number
        << Timer_Start
        << 0 << 0 << 5  // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_6_3[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x03, 0xA5, 0x03, 0x00, 0x00, 0x50};
    static unsigned char const resp_1_6_3[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x03};
    QTest::newRow( "TIMER MANAGEMENT 1.6.3 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_6_3, sizeof(data_1_6_3) )
        << QByteArray( (char *)resp_1_6_3, sizeof(resp_1_6_3) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 3            // Timer number
        << Timer_Start
        << 0 << 0 << 5  // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_6_4[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x04, 0xA5, 0x03, 0x00, 0x00, 0x50};
    static unsigned char const resp_1_6_4[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x04};
    QTest::newRow( "TIMER MANAGEMENT 1.6.4 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_6_4, sizeof(data_1_6_4) )
        << QByteArray( (char *)resp_1_6_4, sizeof(resp_1_6_4) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 4            // Timer number
        << Timer_Start
        << 0 << 0 << 5  // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_6_5[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x05, 0xA5, 0x03, 0x00, 0x00, 0x50};
    static unsigned char const resp_1_6_5[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x05};
    QTest::newRow( "TIMER MANAGEMENT 1.6.5 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_6_5, sizeof(data_1_6_5) )
        << QByteArray( (char *)resp_1_6_5, sizeof(resp_1_6_5) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 5            // Timer number
        << Timer_Start
        << 0 << 0 << 5  // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_6_6[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x06, 0xA5, 0x03, 0x00, 0x00, 0x50};
    static unsigned char const resp_1_6_6[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x06};
    QTest::newRow( "TIMER MANAGEMENT 1.6.6 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_6_6, sizeof(data_1_6_6) )
        << QByteArray( (char *)resp_1_6_6, sizeof(resp_1_6_6) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 6            // Timer number
        << Timer_Start
        << 0 << 0 << 5  // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_6_7[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x07, 0xA5, 0x03, 0x00, 0x00, 0x50};
    static unsigned char const resp_1_6_7[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x07};
    QTest::newRow( "TIMER MANAGEMENT 1.6.7 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_6_7, sizeof(data_1_6_7) )
        << QByteArray( (char *)resp_1_6_7, sizeof(resp_1_6_7) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 7            // Timer number
        << Timer_Start
        << 0 << 0 << 5  // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_6_8[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x08, 0xA5, 0x03, 0x00, 0x00, 0x50};
    static unsigned char const resp_1_6_8[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x08};
    QTest::newRow( "TIMER MANAGEMENT 1.6.8 - GCF 27.22.4.21.1" )
        << QByteArray( (char *)data_1_6_8, sizeof(data_1_6_8) )
        << QByteArray( (char *)resp_1_6_8, sizeof(resp_1_6_8) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << 8            // Timer number
        << Timer_Start
        << 0 << 0 << 5  // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x01, 0xA5, 0x03, 0x00, 0x00, 0x01};
    static unsigned char const resp_2_1_1[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x01};
    static unsigned char const enve_2_1_1[] =
        {0xD7, 0x0C, 0x82, 0x02, 0x82, 0x81, 0xA4, 0x01, 0x01, 0xA5, 0x03, 0x00,
         0x00, 0x01};
    QTest::newRow( "TIMER MANAGEMENT 2.1.1 - GCF 27.22.4.21.2" )
        << QByteArray( (char *)data_2_1_1, sizeof(data_2_1_1) )
        << QByteArray( (char *)resp_2_1_1, sizeof(resp_2_1_1) )
        << QByteArray( (char *)enve_2_1_1, sizeof(enve_2_1_1) )
        << 0x0000       // Command performed successfully
        << 1            // Timer number
        << Timer_Start
        << 0 << 0 << 10 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_2_1a[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x01, 0xA5, 0x03, 0x00, 0x00, 0x03};
    static unsigned char const resp_2_2_1a[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x01};
    static unsigned char const enve_2_2_1a[] =
        {0xD7, 0x0C, 0x82, 0x02, 0x82, 0x81, 0xA4, 0x01, 0x01, 0xA5, 0x03, 0x00,
         0x00, 0x03};
    QTest::newRow( "TIMER MANAGEMENT 2.2.1A - GCF 27.22.4.21.2" )
        << QByteArray( (char *)data_2_2_1a, sizeof(data_2_2_1a) )
        << QByteArray( (char *)resp_2_2_1a, sizeof(resp_2_2_1a) )
        << QByteArray( (char *)enve_2_2_1a, sizeof(enve_2_2_1a) )
        << 0x0000       // Command performed successfully
        << 1            // Timer number
        << Timer_Start
        << 0 << 0 << 30 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_2_1b[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x01, 0xA5, 0x03, 0x00, 0x00, 0x03};
    static unsigned char const resp_2_2_1b[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x01};
    static unsigned char const enve_2_2_1b[] =
        {0xD7, 0x0C, 0x82, 0x02, 0x82, 0x81, 0xA4, 0x01, 0x01, 0xA5, 0x03, 0x00,
         0x00, 0x03};
    QTest::newRow( "TIMER MANAGEMENT 2.2.1B - GCF 27.22.4.21.2" )
        << QByteArray( (char *)data_2_2_1b, sizeof(data_2_2_1b) )
        << QByteArray( (char *)resp_2_2_1b, sizeof(resp_2_2_1b) )
        << QByteArray( (char *)enve_2_2_1b, sizeof(enve_2_2_1b) )
        << 0x0000       // Command performed successfully
        << 1            // Timer number
        << Timer_Start
        << 0 << 0 << 30 // Timer value
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_2_1c[] =
        {0xD0, 0x11, 0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x81, 0x82, 0xA4,
         0x01, 0x01, 0xA5, 0x03, 0x00, 0x00, 0x03};
    static unsigned char const resp_2_2_1c[] =
        {0x81, 0x03, 0x01, 0x27, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA4, 0x01, 0x01};
    static unsigned char const enve_2_2_1c[] =
        {0xD7, 0x0C, 0x82, 0x02, 0x82, 0x81, 0xA4, 0x01, 0x01, 0xA5, 0x03, 0x00,
         0x00, 0x03};
    QTest::newRow( "TIMER MANAGEMENT 2.2.1C - GCF 27.22.4.21.2" )
        << QByteArray( (char *)data_2_2_1c, sizeof(data_2_2_1c) )
        << QByteArray( (char *)resp_2_2_1c, sizeof(resp_2_2_1c) )
        << QByteArray( (char *)enve_2_2_1c, sizeof(enve_2_2_1c) )
        << 0x0000       // Command performed successfully
        << 1            // Timer number
        << Timer_Start
        << 0 << 0 << 30 // Timer value
        << (int)( QSimCommand::NoPduOptions );
}
