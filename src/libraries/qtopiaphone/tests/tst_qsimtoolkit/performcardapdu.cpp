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
// Test encoding and decoding of PERFORM CARD APDU commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.17 - PERFORM CARD APDU.
void tst_QSimToolkit::testEncodePerformCardAPDU_data()
{
    QSimToolkitData::populateDataPerformCardAPDU();
}
void tst_QSimToolkit::testEncodePerformCardAPDU()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, apdu );
    QFETCH( QByteArray, result );
    QFETCH( int, resptype );
    QFETCH( int, device );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::PerformCardAPDU );
    QVERIFY( decoded.destinationDevice() == (QSimCommand::Device)( device + 0x10 ) );
    QCOMPARE( decoded.extensionField(0xA2), apdu );

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
    QCOMPARE( decodedResp.extensionField(0xA3), result );

    // Check that the original terminal response PDU can be reconstructed correctly.
    QCOMPARE( decodedResp.toPdu(), resp );
}

// Test that PERFORM CARD APDU commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverPerformCardAPDU_data()
{
    QSimToolkitData::populateDataPerformCardAPDU();
}
void tst_QSimToolkit::testDeliverPerformCardAPDU()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, apdu );
    QFETCH( QByteArray, result );
    QFETCH( int, resptype );
    QFETCH( int, device );
    QFETCH( int, options );

    Q_UNUSED(resptype);

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::PerformCardAPDU );
    cmd.setDestinationDevice( (QSimCommand::Device)( device + 0x10 ) );
    cmd.addExtensionField( 0xA2, apdu );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QCOMPARE( deliveredCommand.toPdu( (QSimCommand::ToPduOptions)options ), data );

    // The response should have been sent immediately, but won't have a payload
    // because the qtopiaphone library will assume the modem has sent the APDU.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    static unsigned char const ok_resp[] =
        {0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QByteArray resp2 = QByteArray( (char *)ok_resp, sizeof( ok_resp ) );
    QCOMPARE( server->lastResponse(), resp2 );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for PERFORM CARD APDU from the GCF test cases
// in GSM 51.010, section 27.22.4.17.
void QSimToolkitData::populateDataPerformCardAPDU()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<QByteArray>("apdu");
    QTest::addColumn<QByteArray>("result");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<int>("device");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x12, 0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x81, 0x11, 0xA2,
         0x07, 0xA0, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA3, 0x02, 0x9F, 0x1B};
    static unsigned char const apdu_1_1_1[] =
        {0xA0, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00};
    static unsigned char const resu_1_1_1[] =
        {0x9F, 0x1B};
    QTest::newRow( "PERFORM CARD APDU 1.1.1 - GCF 27.22.4.17.1" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << QByteArray( (char *)apdu_1_1_1, sizeof(apdu_1_1_1) )
        << QByteArray( (char *)resu_1_1_1, sizeof(resu_1_1_1) )
        << 0x0000       // Command performed successfully
        << 1            // Card reader device number
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_2[] =
        {0xD0, 0x10, 0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x81, 0x11, 0xA2,
         0x05, 0xA0, 0xC0, 0x00, 0x00, 0x1B};
    static unsigned char const resp_1_1_2[] =
        {0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA3, 0x0F, 0x00, 0x00, 0x02, 0x8D, 0x3F, 0x00, 0x01, 0x00, 0x00, 0x22,
         0xFF, 0x01, 0x0E, 0x90, 0x00};
    static unsigned char const apdu_1_1_2[] =
        {0xA0, 0xC0, 0x00, 0x00, 0x1B};
    static unsigned char const resu_1_1_2[] =
        {0x00, 0x00, 0x02, 0x8D, 0x3F, 0x00, 0x01, 0x00, 0x00, 0x22, 0xFF, 0x01,
         0x0E, 0x90, 0x00};
    QTest::newRow( "PERFORM CARD APDU 1.1.2 - GCF 27.22.4.17.1" )
        << QByteArray( (char *)data_1_1_2, sizeof(data_1_1_2) )
        << QByteArray( (char *)resp_1_1_2, sizeof(resp_1_1_2) )
        << QByteArray( (char *)apdu_1_1_2, sizeof(apdu_1_1_2) )
        << QByteArray( (char *)resu_1_1_2, sizeof(resu_1_1_2) )
        << 0x0000       // Command performed successfully
        << 1            // Card reader device number
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x12, 0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x81, 0x11, 0xA2,
         0x07, 0xA0, 0xA4, 0x00, 0x00, 0x02, 0x7F, 0x20};
    static unsigned char const resp_1_2_1[] =
        {0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA3, 0x02, 0x9F, 0x1B};
    static unsigned char const apdu_1_2_1[] =
        {0xA0, 0xA4, 0x00, 0x00, 0x02, 0x7F, 0x20};
    static unsigned char const resu_1_2_1[] =
        {0x9F, 0x1B};
    QTest::newRow( "PERFORM CARD APDU 1.2.1 - GCF 27.22.4.17.1" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << QByteArray( (char *)apdu_1_2_1, sizeof(apdu_1_2_1) )
        << QByteArray( (char *)resu_1_2_1, sizeof(resu_1_2_1) )
        << 0x0000       // Command performed successfully
        << 1            // Card reader device number
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_2[] =
        {0xD0, 0x12, 0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x81, 0x11, 0xA2,
         0x07, 0xA0, 0xA4, 0x00, 0x00, 0x02, 0x6F, 0x30};
    static unsigned char const resp_1_2_2[] =
        {0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA3, 0x02, 0x9F, 0x0F};
    static unsigned char const apdu_1_2_2[] =
        {0xA0, 0xA4, 0x00, 0x00, 0x02, 0x6F, 0x30};
    static unsigned char const resu_1_2_2[] =
        {0x9F, 0x0F};
    QTest::newRow( "PERFORM CARD APDU 1.2.2 - GCF 27.22.4.17.1" )
        << QByteArray( (char *)data_1_2_2, sizeof(data_1_2_2) )
        << QByteArray( (char *)resp_1_2_2, sizeof(resp_1_2_2) )
        << QByteArray( (char *)apdu_1_2_2, sizeof(apdu_1_2_2) )
        << QByteArray( (char *)resu_1_2_2, sizeof(resu_1_2_2) )
        << 0x0000       // Command performed successfully
        << 1            // Card reader device number
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_3[] =
        {0xD0, 0x28, 0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x81, 0x11, 0xA2,
         0x1D, 0xA0, 0xD6, 0x00, 0x00, 0x18, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
         0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
         0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
    static unsigned char const resp_1_2_3[] =
        {0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA3, 0x02, 0x90, 0x00};
    static unsigned char const apdu_1_2_3[] =
        {0xA0, 0xD6, 0x00, 0x00, 0x18, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
         0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12,
         0x13, 0x14, 0x15, 0x16, 0x17};
    static unsigned char const resu_1_2_3[] =
        {0x90, 0x00};
    QTest::newRow( "PERFORM CARD APDU 1.2.3 - GCF 27.22.4.17.1" )
        << QByteArray( (char *)data_1_2_3, sizeof(data_1_2_3) )
        << QByteArray( (char *)resp_1_2_3, sizeof(resp_1_2_3) )
        << QByteArray( (char *)apdu_1_2_3, sizeof(apdu_1_2_3) )
        << QByteArray( (char *)resu_1_2_3, sizeof(resu_1_2_3) )
        << 0x0000       // Command performed successfully
        << 1            // Card reader device number
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_4[] =
        {0xD0, 0x10, 0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x81, 0x11, 0xA2,
         0x05, 0xA0, 0xB0, 0x00, 0x00, 0x18};
    static unsigned char const resp_1_2_4[] =
        {0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA3, 0x1A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
         0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
         0x16, 0x17, 0x90, 0x00};
    static unsigned char const apdu_1_2_4[] =
        {0xA0, 0xB0, 0x00, 0x00, 0x18};
    static unsigned char const resu_1_2_4[] =
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
         0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
         0x90, 0x00};
    QTest::newRow( "PERFORM CARD APDU 1.2.4 - GCF 27.22.4.17.1" )
        << QByteArray( (char *)data_1_2_4, sizeof(data_1_2_4) )
        << QByteArray( (char *)resp_1_2_4, sizeof(resp_1_2_4) )
        << QByteArray( (char *)apdu_1_2_4, sizeof(apdu_1_2_4) )
        << QByteArray( (char *)resu_1_2_4, sizeof(resu_1_2_4) )
        << 0x0000       // Command performed successfully
        << 1            // Card reader device number
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_5[] =
        {0xD0, 0x28, 0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x81, 0x11, 0xA2,
         0x1D, 0xA0, 0xD6, 0x00, 0x00, 0x18, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    static unsigned char const resp_1_2_5[] =
        {0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA3, 0x02, 0x90, 0x00};
    static unsigned char const apdu_1_2_5[] =
        {0xA0, 0xD6, 0x00, 0x00, 0x18, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    static unsigned char const resu_1_2_5[] =
        {0x90, 0x00};
    QTest::newRow( "PERFORM CARD APDU 1.2.5 - GCF 27.22.4.17.1" )
        << QByteArray( (char *)data_1_2_5, sizeof(data_1_2_5) )
        << QByteArray( (char *)resp_1_2_5, sizeof(resp_1_2_5) )
        << QByteArray( (char *)apdu_1_2_5, sizeof(apdu_1_2_5) )
        << QByteArray( (char *)resu_1_2_5, sizeof(resu_1_2_5) )
        << 0x0000       // Command performed successfully
        << 1            // Card reader device number
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1[] =
        {0xD0, 0x12, 0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x81, 0x11, 0xA2,
         0x07, 0xA0, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00};
    static unsigned char const resp_1_3_1[] =
        {0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x38,
         0x04};
    static unsigned char const apdu_1_3_1[] =
        {0xA0, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00};
    QTest::newRow( "PERFORM CARD APDU 1.3.1 - GCF 27.22.4.17.1" )
        << QByteArray( (char *)data_1_3_1, sizeof(data_1_3_1) )
        << QByteArray( (char *)resp_1_3_1, sizeof(resp_1_3_1) )
        << QByteArray( (char *)apdu_1_3_1, sizeof(apdu_1_3_1) )
        << QByteArray()
        << 0x3804       // Card powered off
        << 1            // Card reader device number
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_1[] =
        {0xD0, 0x12, 0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x81, 0x11, 0xA2,
         0x07, 0xA0, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00};
    static unsigned char const resp_1_4_1[] =
        {0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x38,
         0x02};
    static unsigned char const apdu_1_4_1[] =
        {0xA0, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00};
    QTest::newRow( "PERFORM CARD APDU 1.4.1 - GCF 27.22.4.17.1" )
        << QByteArray( (char *)data_1_4_1, sizeof(data_1_4_1) )
        << QByteArray( (char *)resp_1_4_1, sizeof(resp_1_4_1) )
        << QByteArray( (char *)apdu_1_4_1, sizeof(apdu_1_4_1) )
        << QByteArray()
        << 0x3802       // Card powered off
        << 1            // Card reader device number
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_1[] =
        {0xD0, 0x12, 0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x81, 0x17, 0xA2,
         0x07, 0xA0, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00};
    static unsigned char const resp_1_5_1[] =
        {0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x38,
         0x09};
    static unsigned char const apdu_1_5_1[] =
        {0xA0, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00};
    QTest::newRow( "PERFORM CARD APDU 1.5.1 - GCF 27.22.4.17.1" )
        << QByteArray( (char *)data_1_5_1, sizeof(data_1_5_1) )
        << QByteArray( (char *)resp_1_5_1, sizeof(resp_1_5_1) )
        << QByteArray( (char *)apdu_1_5_1, sizeof(apdu_1_5_1) )
        << QByteArray()
        << 0x3809       // Specified reader not valid
        << 7            // Card reader device number
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1[] =
        {0xD0, 0x12, 0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x81, 0x11, 0xA2,
         0x07, 0xA0, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00};
    static unsigned char const resp_2_1_1[] =
        {0x81, 0x03, 0x01, 0x30, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x38,
         0x01};
    static unsigned char const apdu_2_1_1[] =
        {0xA0, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00};
    QTest::newRow( "PERFORM CARD APDU 2.1.1 - GCF 27.22.4.17.2" )
        << QByteArray( (char *)data_2_1_1, sizeof(data_2_1_1) )
        << QByteArray( (char *)resp_2_1_1, sizeof(resp_2_1_1) )
        << QByteArray( (char *)apdu_2_1_1, sizeof(apdu_2_1_1) )
        << QByteArray()
        << 0x3801       // Card reader removed or not present
        << 1            // Card reader device number
        << (int)( QSimCommand::NoPduOptions );
}
