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
// Test encoding and decoding of PROVIDE LOCAL INFORMATION commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.15 - PROVIDE LOCAL INFORMATION.
void tst_QSimToolkit::testEncodeProvideLocalInformation_data()
{
    QSimToolkitData::populateDataProvideLocalInformation();
}
void tst_QSimToolkit::testEncodeProvideLocalInformation()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, info );
    QFETCH( int, resptype );
    QFETCH( int, localtype );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::ProvideLocalInformation );
    QVERIFY( decoded.destinationDevice() == QSimCommand::ME );
    QCOMPARE( decoded.qualifier(), localtype );

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

// Test that PROVIDE LOCAL INFORMATION commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverProvideLocalInformation_data()
{
    QSimToolkitData::populateDataProvideLocalInformation();
}
void tst_QSimToolkit::testDeliverProvideLocalInformation()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, info );
    QFETCH( int, resptype );
    QFETCH( int, localtype );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::ProvideLocalInformation );
    cmd.setQualifier( localtype );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QVERIFY( deliveredCommand.qualifier() == cmd.qualifier() );
    QCOMPARE( deliveredCommand.toPdu( (QSimCommand::ToPduOptions)options ), data );

    // The response should have been sent immediately.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    QSimTerminalResponse resp2 = QSimTerminalResponse::fromPdu( server->lastResponse() );
    QVERIFY( resp2.result() == (QSimTerminalResponse::Result)resptype );
    QVERIFY( resp2.cause() == QSimTerminalResponse::NoSpecificCause );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for PROVIDE LOCAL INFORMATION from the GCF test cases
// in GSM 51.010, section 27.22.4.15.
void QSimToolkitData::populateDataProvideLocalInformation()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<QByteArray>("info");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<int>("localtype");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1a[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x26, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_1_1a[] =
        {0x81, 0x03, 0x01, 0x26, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x93, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const info_1_1_1a[] =
        {0x93, 0x07, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x01};
    QTest::newRow( "PROVIDE LOCAL INFORMATION 1.1.1A - GCF 27.22.4.15" )
        << QByteArray( (char *)data_1_1_1a, sizeof(data_1_1_1a) )
        << QByteArray( (char *)resp_1_1_1a, sizeof(resp_1_1_1a) )
        << QByteArray( (char *)info_1_1_1a, sizeof(info_1_1_1a) )
        << 0x0000       // Command performed successfully
        << 0            // Location information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_1b[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x26, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_1_1b[] =
        {0x81, 0x03, 0x01, 0x26, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x93, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    static unsigned char const info_1_1_1b[] =
        {0x93, 0x07, 0x00, 0x11, 0x10, 0x00, 0x01, 0x00, 0x01};
    QTest::newRow( "PROVIDE LOCAL INFORMATION 1.1.1B - GCF 27.22.4.15" )
        << QByteArray( (char *)data_1_1_1b, sizeof(data_1_1_1b) )
        << QByteArray( (char *)resp_1_1_1b, sizeof(resp_1_1_1b) )
        << QByteArray( (char *)info_1_1_1b, sizeof(info_1_1_1b) )
        << 0x0000       // Command performed successfully
        << 0            // Location information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x26, 0x01, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_2_1[] =
        {0x81, 0x03, 0x01, 0x26, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x94, 0x08, 0x1A, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0x54};
    static unsigned char const info_1_2_1[] =
        {0x94, 0x08, 0x1A, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0x54};
    QTest::newRow( "PROVIDE LOCAL INFORMATION 1.2.1 - GCF 27.22.4.15" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << QByteArray( (char *)info_1_2_1, sizeof(info_1_2_1) )
        << 0x0000       // Command performed successfully
        << 1            // IMEI
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x26, 0x02, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_3_1[] =
        {0x81, 0x03, 0x01, 0x26, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x96, 0x10, 0x34, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9D, 0x0D, 0x8C, 0x63, 0x58, 0xE2,
         0x39, 0x8F, 0x63, 0xF9, 0x06, 0x45, 0x91, 0xA4, 0x90};
    static unsigned char const info_1_3_1[] =
        {0x96, 0x10, 0x34, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9D, 0x0D, 0x8C, 0x63, 0x58, 0xE2,
         0x39, 0x8F, 0x63, 0xF9, 0x06, 0x45, 0x91, 0xA4, 0x90};
    QTest::newRow( "PROVIDE LOCAL INFORMATION 1.3.1 - GCF 27.22.4.15" )
        << QByteArray( (char *)data_1_3_1, sizeof(data_1_3_1) )
        << QByteArray( (char *)resp_1_3_1, sizeof(resp_1_3_1) )
        << QByteArray( (char *)info_1_3_1, sizeof(info_1_3_1) )
        << 0x0000       // Command performed successfully
        << 2            // Network measurement results
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_1[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x26, 0x03, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_4_1[] =
        {0x81, 0x03, 0x01, 0x26, 0x03, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA6, 0x07, 0x20, 0x50, 0x70, 0x41, 0x80, 0x71, 0xFF};
    static unsigned char const info_1_4_1[] =
        {0xA6, 0x07, 0x20, 0x50, 0x70, 0x41, 0x80, 0x71, 0xFF};
    QTest::newRow( "PROVIDE LOCAL INFORMATION 1.4.1 - GCF 27.22.4.15" )
        << QByteArray( (char *)data_1_4_1, sizeof(data_1_4_1) )
        << QByteArray( (char *)resp_1_4_1, sizeof(resp_1_4_1) )
        << QByteArray( (char *)info_1_4_1, sizeof(info_1_4_1) )
        << 0x0000       // Command performed successfully
        << 3            // Date, time and time zone
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_1[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x26, 0x04, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_5_1[] =
        {0x81, 0x03, 0x01, 0x26, 0x04, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xAD, 0x02, 0x65, 0x6E};
    static unsigned char const info_1_5_1[] =
        {0xAD, 0x02, 0x65, 0x6E};
    QTest::newRow( "PROVIDE LOCAL INFORMATION 1.5.1 - GCF 27.22.4.15" )
        << QByteArray( (char *)data_1_5_1, sizeof(data_1_5_1) )
        << QByteArray( (char *)resp_1_5_1, sizeof(resp_1_5_1) )
        << QByteArray( (char *)info_1_5_1, sizeof(info_1_5_1) )
        << 0x0000       // Command performed successfully
        << 4            // Language setting
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_6_1[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x26, 0x05, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_6_1[] =
        {0x81, 0x03, 0x01, 0x26, 0x05, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xAE, 0x02, 0x00, 0x00};
    static unsigned char const info_1_6_1[] =
        {0xAE, 0x02, 0x00, 0x00};
    QTest::newRow( "PROVIDE LOCAL INFORMATION 1.6.1 - GCF 27.22.4.15" )
        << QByteArray( (char *)data_1_6_1, sizeof(data_1_6_1) )
        << QByteArray( (char *)resp_1_6_1, sizeof(resp_1_6_1) )
        << QByteArray( (char *)info_1_6_1, sizeof(info_1_6_1) )
        << 0x0000       // Command performed successfully
        << 5            // Timeing advance
        << (int)( QSimCommand::NoPduOptions );
}
