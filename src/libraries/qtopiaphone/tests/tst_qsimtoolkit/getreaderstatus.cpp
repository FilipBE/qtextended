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
// Test encoding and decoding of GET READER STATUS commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.20 - GET READER STATUS.
void tst_QSimToolkit::testEncodeGetReaderStatus_data()
{
    QSimToolkitData::populateDataGetReaderStatus();
}
void tst_QSimToolkit::testEncodeGetReaderStatus()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, result );
    QFETCH( int, resptype );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::GetReaderStatus );
    QVERIFY( decoded.destinationDevice() == QSimCommand::ME );

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
    QCOMPARE( decodedResp.extensionField(0xA0), result );

    // Check that the original terminal response PDU can be reconstructed correctly.
    QCOMPARE( decodedResp.toPdu(), resp );
}

// Test that GET READER STATUS commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverGetReaderStatus_data()
{
    QSimToolkitData::populateDataGetReaderStatus();
}
void tst_QSimToolkit::testDeliverGetReaderStatus()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, result );
    QFETCH( int, resptype );
    QFETCH( int, options );

    Q_UNUSED(resptype);

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::GetReaderStatus );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QCOMPARE( deliveredCommand.toPdu( (QSimCommand::ToPduOptions)options ), data );

    // The response should have been sent immediately, but won't have a payload
    // because the qtopiaphone library will assume the modem has processed the request.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    static unsigned char const ok_resp[] =
        {0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QByteArray resp2 = QByteArray( (char *)ok_resp, sizeof( ok_resp ) );
    QCOMPARE( server->lastResponse(), resp2 );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for GET READER STATUS from the GCF test cases
// in GSM 51.010, section 27.22.4.20.
void QSimToolkitData::populateDataGetReaderStatus()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<QByteArray>("result");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1a[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_1_1a[] =
        {0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA0, 0x01, 0xF1};
    static unsigned char const resu_1_1_1a[] =
        {0xF1};
    QTest::newRow( "GET READER STATUS 1.1.1A - GCF 27.22.4.20.1" )
        << QByteArray( (char *)data_1_1_1a, sizeof(data_1_1_1a) )
        << QByteArray( (char *)resp_1_1_1a, sizeof(resp_1_1_1a) )
        << QByteArray( (char *)resu_1_1_1a, sizeof(resu_1_1_1a) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_1b[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_1_1b[] =
        {0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA0, 0x01, 0xD1};
    static unsigned char const resu_1_1_1b[] =
        {0xD1};
    QTest::newRow( "GET READER STATUS 1.1.1B - GCF 27.22.4.20.1" )
        << QByteArray( (char *)data_1_1_1b, sizeof(data_1_1_1b) )
        << QByteArray( (char *)resp_1_1_1b, sizeof(resp_1_1_1b) )
        << QByteArray( (char *)resu_1_1_1b, sizeof(resu_1_1_1b) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_1c[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_1_1c[] =
        {0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA0, 0x01, 0xF9};
    static unsigned char const resu_1_1_1c[] =
        {0xF9};
    QTest::newRow( "GET READER STATUS 1.1.1C - GCF 27.22.4.20.1" )
        << QByteArray( (char *)data_1_1_1c, sizeof(data_1_1_1c) )
        << QByteArray( (char *)resp_1_1_1c, sizeof(resp_1_1_1c) )
        << QByteArray( (char *)resu_1_1_1c, sizeof(resu_1_1_1c) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_1d[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_1_1d[] =
        {0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA0, 0x01, 0xD9};
    static unsigned char const resu_1_1_1d[] =
        {0xD9};
    QTest::newRow( "GET READER STATUS 1.1.1D - GCF 27.22.4.20.1" )
        << QByteArray( (char *)data_1_1_1d, sizeof(data_1_1_1d) )
        << QByteArray( (char *)resp_1_1_1d, sizeof(resp_1_1_1d) )
        << QByteArray( (char *)resu_1_1_1d, sizeof(resu_1_1_1d) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1a[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_2_1a[] =
        {0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA0, 0x01, 0x71};
    static unsigned char const resu_1_2_1a[] =
        {0x71};
    QTest::newRow( "GET READER STATUS 1.2.1A - GCF 27.22.4.20.1" )
        << QByteArray( (char *)data_1_2_1a, sizeof(data_1_2_1a) )
        << QByteArray( (char *)resp_1_2_1a, sizeof(resp_1_2_1a) )
        << QByteArray( (char *)resu_1_2_1a, sizeof(resu_1_2_1a) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1b[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_2_1b[] =
        {0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA0, 0x01, 0x51};
    static unsigned char const resu_1_2_1b[] =
        {0x51};
    QTest::newRow( "GET READER STATUS 1.2.1B - GCF 27.22.4.20.1" )
        << QByteArray( (char *)data_1_2_1b, sizeof(data_1_2_1b) )
        << QByteArray( (char *)resp_1_2_1b, sizeof(resp_1_2_1b) )
        << QByteArray( (char *)resu_1_2_1b, sizeof(resu_1_2_1b) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1c[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_2_1c[] =
        {0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA0, 0x01, 0x79};
    static unsigned char const resu_1_2_1c[] =
        {0x79};
    QTest::newRow( "GET READER STATUS 1.2.1C - GCF 27.22.4.20.1" )
        << QByteArray( (char *)data_1_2_1c, sizeof(data_1_2_1c) )
        << QByteArray( (char *)resp_1_2_1c, sizeof(resp_1_2_1c) )
        << QByteArray( (char *)resu_1_2_1c, sizeof(resu_1_2_1c) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1d[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_2_1d[] =
        {0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA0, 0x01, 0x59};
    static unsigned char const resu_1_2_1d[] =
        {0x59};
    QTest::newRow( "GET READER STATUS 1.2.1D - GCF 27.22.4.20.1" )
        << QByteArray( (char *)data_1_2_1d, sizeof(data_1_2_1d) )
        << QByteArray( (char *)resp_1_2_1d, sizeof(resp_1_2_1d) )
        << QByteArray( (char *)resu_1_2_1d, sizeof(resu_1_2_1d) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1a[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_3_1a[] =
        {0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA0, 0x01, 0x31};
    static unsigned char const resu_1_3_1a[] =
        {0x31};
    QTest::newRow( "GET READER STATUS 1.3.1A - GCF 27.22.4.20.1" )
        << QByteArray( (char *)data_1_3_1a, sizeof(data_1_3_1a) )
        << QByteArray( (char *)resp_1_3_1a, sizeof(resp_1_3_1a) )
        << QByteArray( (char *)resu_1_3_1a, sizeof(resu_1_3_1a) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1b[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_3_1b[] =
        {0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA0, 0x01, 0x11};
    static unsigned char const resu_1_3_1b[] =
        {0x11};
    QTest::newRow( "GET READER STATUS 1.3.1B - GCF 27.22.4.20.1" )
        << QByteArray( (char *)data_1_3_1b, sizeof(data_1_3_1b) )
        << QByteArray( (char *)resp_1_3_1b, sizeof(resp_1_3_1b) )
        << QByteArray( (char *)resu_1_3_1b, sizeof(resu_1_3_1b) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1c[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_3_1c[] =
        {0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA0, 0x01, 0x39};
    static unsigned char const resu_1_3_1c[] =
        {0x39};
    QTest::newRow( "GET READER STATUS 1.3.1C - GCF 27.22.4.20.1" )
        << QByteArray( (char *)data_1_3_1c, sizeof(data_1_3_1c) )
        << QByteArray( (char *)resp_1_3_1c, sizeof(resp_1_3_1c) )
        << QByteArray( (char *)resu_1_3_1c, sizeof(resu_1_3_1c) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1d[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_3_1d[] =
        {0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA0, 0x01, 0x19};
    static unsigned char const resu_1_3_1d[] =
        {0x19};
    QTest::newRow( "GET READER STATUS 1.3.1D - GCF 27.22.4.20.1" )
        << QByteArray( (char *)data_1_3_1d, sizeof(data_1_3_1d) )
        << QByteArray( (char *)resp_1_3_1d, sizeof(resp_1_3_1d) )
        << QByteArray( (char *)resu_1_3_1d, sizeof(resu_1_3_1d) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1a[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_2_1_1a[] =
        {0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA0, 0x01, 0x29};
    static unsigned char const resu_2_1_1a[] =
        {0x29};
    QTest::newRow( "GET READER STATUS 2.1.1A - GCF 27.22.4.20.2" )
        << QByteArray( (char *)data_2_1_1a, sizeof(data_2_1_1a) )
        << QByteArray( (char *)resp_2_1_1a, sizeof(resp_2_1_1a) )
        << QByteArray( (char *)resu_2_1_1a, sizeof(resu_2_1_1a) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1b[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_2_1_1b[] =
        {0x81, 0x03, 0x01, 0x33, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0xA0, 0x01, 0x09};
    static unsigned char const resu_2_1_1b[] =
        {0x09};
    QTest::newRow( "GET READER STATUS 2.1.1B - GCF 27.22.4.20.2" )
        << QByteArray( (char *)data_2_1_1b, sizeof(data_2_1_1b) )
        << QByteArray( (char *)resp_2_1_1b, sizeof(resp_2_1_1b) )
        << QByteArray( (char *)resu_2_1_1b, sizeof(resu_2_1_1b) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::NoPduOptions );
}
