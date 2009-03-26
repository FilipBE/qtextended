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
// Test encoding and decoding of LANGUAGE NOTIFICATION commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.25 - LANGUAGE NOTIFICATION.
void tst_QSimToolkit::testEncodeLanguageNotification_data()
{
    QSimToolkitData::populateDataLanguageNotification();
}
void tst_QSimToolkit::testEncodeLanguageNotification()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QByteArray, language );
    QFETCH( bool, specific );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::LanguageNotification );
    QVERIFY( decoded.destinationDevice() == QSimCommand::ME );
    QVERIFY( decoded.qualifier() == (int)specific );
    QCOMPARE( decoded.extensionField(0xAD), language );

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

// Test that LANGUAGE NOTIFICATION commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverLanguageNotification_data()
{
    QSimToolkitData::populateDataLanguageNotification();
}
void tst_QSimToolkit::testDeliverLanguageNotification()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QByteArray, language );
    QFETCH( bool, specific );
    QFETCH( int, options );

    Q_UNUSED(resptype);

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::LanguageNotification );
    cmd.setQualifier( (int)specific );
    if ( !language.isEmpty() )
        cmd.addExtensionField( 0xAD, language );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QCOMPARE( deliveredCommand.extensionData(), cmd.extensionData() );
    QCOMPARE( deliveredCommand.toPdu( (QSimCommand::ToPduOptions)options ), data );

    // The response should have been sent immediately, and should always be success.
    // We cannot test the actual behaviour with this interface.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    static unsigned char const ok_resp[] =
        {0x81, 0x03, 0x01, 0x35, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QByteArray resp2 = QByteArray( (char *)ok_resp, sizeof( ok_resp ) );
    resp2[4] = (char)specific;
    QCOMPARE( server->lastResponse(), resp2 );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for LANGUAGE NOTIFICATION from the GCF test cases
// in GSM 51.010, section 27.22.4.25.
void QSimToolkitData::populateDataLanguageNotification()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<QByteArray>("language");
    QTest::addColumn<bool>("specific");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x0D, 0x81, 0x03, 0x01, 0x35, 0x01, 0x82, 0x02, 0x81, 0x82, 0xAD,
         0x02, 0x73, 0x65};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x35, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "LANGUAGE NOTIFICATION 1.1.1 - GCF 27.22.4.25" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << 0x0000       // Command performed successfully
        << QByteArray( "se" )
        << true         // Specific language notification
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x35, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_2_1[] =
        {0x81, 0x03, 0x01, 0x35, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "LANGUAGE NOTIFICATION 1.2.1 - GCF 27.22.4.25" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << 0x0000       // Command performed successfully
        << QByteArray( "" )
        << false        // Specific language notification
        << (int)( QSimCommand::NoPduOptions );
}
