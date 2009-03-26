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
#include <qatutils.h>

#ifndef SYSTEMTEST

// Test encoding and decoding of EVENT DOWNLOAD envelopes based on the
// Test encoding and decoding of REFRESH commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.7.1 - REFRESH.
void tst_QSimToolkit::testEncodeRefresh_data()
{
    QSimToolkitData::populateDataRefresh();
}
void tst_QSimToolkit::testEncodeRefresh()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( int, refreshtype );
    QFETCH( int, numfiles );
    QFETCH( QString, fileids );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::Refresh );
    QVERIFY( decoded.destinationDevice() == QSimCommand::ME );
    QCOMPARE( (int)decoded.refreshType(), refreshtype );
    if ( numfiles > 0 ) {
        QCOMPARE( decoded.extensionField(0x92)[0] & 0xFF, numfiles );
        QCOMPARE( decoded.extensionField(0x92).mid(1), QAtUtils::fromHex(fileids) );
    } else {
        QCOMPARE( decoded.extensionField(0x92), QByteArray() );
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

    // Check that the original terminal response PDU can be reconstructed correctly.
    QCOMPARE( decodedResp.toPdu(), resp );
}

// Test that REFRESH commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverRefresh_data()
{
    QSimToolkitData::populateDataRefresh();
}
void tst_QSimToolkit::testDeliverRefresh()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( int, refreshtype );
    QFETCH( int, numfiles );
    QFETCH( QString, fileids );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::Refresh );
    cmd.setRefreshType( (QSimCommand::RefreshType)refreshtype );
    if ( numfiles > 0 ) {
        QByteArray extData;
        extData += (char)numfiles;
        extData += QAtUtils::fromHex( fileids );
        cmd.addExtensionField( 0x92, extData );
    }
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QVERIFY( deliveredCommand.refreshType() == cmd.refreshType() );
    QVERIFY( deliveredCommand.extensionData() == cmd.extensionData() );
    QCOMPARE( deliveredCommand.toPdu( (QSimCommand::ToPduOptions)options ), data );

    // The response should have been sent immediately.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    if ( resptype != 0x0003 ) {
        QCOMPARE( server->lastResponse(), resp );
    } else {
        // We cannot test the "additional EF's read" case because the qtopiaphone
        // library will always respond with "command performed successfully".
        QByteArray resp2 = resp;
        resp2[resp2.size() - 1] = 0x00;
        QCOMPARE( server->lastResponse(), resp2 );
    }
}

#endif // !SYSTEMTEST

// Populate data-driven tests for REFRESH from the GCF test cases
// in GSM 51.010, section 27.22.4.7.1.
void QSimToolkitData::populateDataRefresh()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<int>("refreshtype");
    QTest::addColumn<int>("numfiles");
    QTest::addColumn<QString>("fileids");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1a[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x01, 0x03, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_1_1a[] =
        {0x81, 0x03, 0x01, 0x01, 0x03, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "REFRESH 1.1.1A - GCF 27.22.4.7.1" )
        << QByteArray( (char *)data_1_1_1a, sizeof(data_1_1_1a) )
        << QByteArray( (char *)resp_1_1_1a, sizeof(resp_1_1_1a) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::Initialization )
        << 0            // Number of files
        << QString( "" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_1b[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x01, 0x03, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_1_1b[] =
        {0x81, 0x03, 0x01, 0x01, 0x03, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x03};
    QTest::newRow( "REFRESH 1.1.1B - GCF 27.22.4.7.1" )
        << QByteArray( (char *)data_1_1_1b, sizeof(data_1_1_1b) )
        << QByteArray( (char *)resp_1_1_1b, sizeof(resp_1_1_1b) )
        << 0x0003       // Command performed with additional EFs read
        << (int)( QSimCommand::Initialization )
        << 0            // Number of files
        << QString( "" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1a[] =
        {0xD0, 0x12, 0x81, 0x03, 0x01, 0x01, 0x01, 0x82, 0x02, 0x81, 0x82, 0x92,
         0x07, 0x01, 0x3F, 0x00, 0x7F, 0x10, 0x6F, 0x3B};
    static unsigned char const resp_1_2_1a[] =
        {0x81, 0x03, 0x01, 0x01, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "REFRESH 1.2.1A - GCF 27.22.4.7.1" )
        << QByteArray( (char *)data_1_2_1a, sizeof(data_1_2_1a) )
        << QByteArray( (char *)resp_1_2_1a, sizeof(resp_1_2_1a) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::FileChange )
        << 1            // Number of files
        << QString( "3F007F106F3B" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1b[] =
        {0xD0, 0x12, 0x81, 0x03, 0x01, 0x01, 0x01, 0x82, 0x02, 0x81, 0x82, 0x92,
         0x07, 0x01, 0x3F, 0x00, 0x7F, 0x10, 0x6F, 0x3B};
    static unsigned char const resp_1_2_1b[] =
        {0x81, 0x03, 0x01, 0x01, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x03};
    QTest::newRow( "REFRESH 1.2.1B - GCF 27.22.4.7.1" )
        << QByteArray( (char *)data_1_2_1b, sizeof(data_1_2_1b) )
        << QByteArray( (char *)resp_1_2_1b, sizeof(resp_1_2_1b) )
        << 0x0003       // Command performed with additional EFs read
        << (int)( QSimCommand::FileChange )
        << 1            // Number of files
        << QString( "3F007F106F3B" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1a[] =
        {0xD0, 0x12, 0x81, 0x03, 0x01, 0x01, 0x02, 0x82, 0x02, 0x81, 0x82, 0x92,
         0x07, 0x01, 0x3F, 0x00, 0x7F, 0x20, 0x6F, 0x30};
    static unsigned char const resp_1_3_1a[] =
        {0x81, 0x03, 0x01, 0x01, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "REFRESH 1.3.1A - GCF 27.22.4.7.1" )
        << QByteArray( (char *)data_1_3_1a, sizeof(data_1_3_1a) )
        << QByteArray( (char *)resp_1_3_1a, sizeof(resp_1_3_1a) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::InitAndFileChange )
        << 1            // Number of files
        << QString( "3F007F206F30" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1b[] =
        {0xD0, 0x12, 0x81, 0x03, 0x01, 0x01, 0x02, 0x82, 0x02, 0x81, 0x82, 0x92,
         0x07, 0x01, 0x3F, 0x00, 0x7F, 0x20, 0x6F, 0x30};
    static unsigned char const resp_1_3_1b[] =
        {0x81, 0x03, 0x01, 0x01, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x03};
    QTest::newRow( "REFRESH 1.3.1B - GCF 27.22.4.7.1" )
        << QByteArray( (char *)data_1_3_1b, sizeof(data_1_3_1b) )
        << QByteArray( (char *)resp_1_3_1b, sizeof(resp_1_3_1b) )
        << 0x0003       // Command performed with additional EFs read
        << (int)( QSimCommand::InitAndFileChange )
        << 1            // Number of files
        << QString( "3F007F206F30" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_1a[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x01, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_4_1a[] =
        {0x81, 0x03, 0x01, 0x01, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "REFRESH 1.4.1A - GCF 27.22.4.7.1" )
        << QByteArray( (char *)data_1_4_1a, sizeof(data_1_4_1a) )
        << QByteArray( (char *)resp_1_4_1a, sizeof(resp_1_4_1a) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::InitAndFullFileChange )
        << 0            // Number of files
        << QString( "" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_1b[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x01, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_4_1b[] =
        {0x81, 0x03, 0x01, 0x01, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x03};
    QTest::newRow( "REFRESH 1.4.1B - GCF 27.22.4.7.1" )
        << QByteArray( (char *)data_1_4_1b, sizeof(data_1_4_1b) )
        << QByteArray( (char *)resp_1_4_1b, sizeof(resp_1_4_1b) )
        << 0x0003       // Command performed with additional EFs read
        << (int)( QSimCommand::InitAndFullFileChange )
        << 0            // Number of files
        << QString( "" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_1[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x01, 0x04, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_1_5_1[] =
        {0x81, 0x03, 0x01, 0x01, 0x04, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "REFRESH 1.5.1 - GCF 27.22.4.7.1" )
        << QByteArray( (char *)data_1_5_1, sizeof(data_1_5_1) )
        << QByteArray( (char *)resp_1_5_1, sizeof(resp_1_5_1) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::Reset )
        << 0            // Number of files
        << QString( "" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1a[] =
        {0xD0, 0x1E, 0x81, 0x03, 0x01, 0x01, 0x02, 0x82, 0x02, 0x81, 0x82, 0x92,
         0x13, 0x03, 0x3F, 0x00, 0x7F, 0x20, 0x6F, 0x07, 0x3F, 0x00, 0x7F, 0x20,
         0x6F, 0x7E, 0x3F, 0x00, 0x7F, 0x20, 0x6F, 0x20};
    static unsigned char const resp_2_1_1a[] =
        {0x81, 0x03, 0x01, 0x01, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "REFRESH 2.1.1A - GCF 27.22.4.7.2" )
        << QByteArray( (char *)data_2_1_1a, sizeof(data_2_1_1a) )
        << QByteArray( (char *)resp_2_1_1a, sizeof(resp_2_1_1a) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::InitAndFileChange )
        << 3            // Number of files
        << QString( "3F007F206F07,3F007F206F7E,3F007F206F20" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1b[] =
        {0xD0, 0x1E, 0x81, 0x03, 0x01, 0x01, 0x02, 0x82, 0x02, 0x81, 0x82, 0x92,
         0x13, 0x03, 0x3F, 0x00, 0x7F, 0x20, 0x6F, 0x07, 0x3F, 0x00, 0x7F, 0x20,
         0x6F, 0x7E, 0x3F, 0x00, 0x7F, 0x20, 0x6F, 0x20};
    static unsigned char const resp_2_1_1b[] =
        {0x81, 0x03, 0x01, 0x01, 0x02, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x03};
    QTest::newRow( "REFRESH 2.1.1B - GCF 27.22.4.7.2" )
        << QByteArray( (char *)data_2_1_1b, sizeof(data_2_1_1b) )
        << QByteArray( (char *)resp_2_1_1b, sizeof(resp_2_1_1b) )
        << 0x0003       // Command performed with additional EFs read
        << (int)( QSimCommand::InitAndFileChange )
        << 3            // Number of files
        << QString( "3F007F206F07,3F007F206F7E,3F007F206F20" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_2_1a[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x01, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_2_2_1a[] =
        {0x81, 0x03, 0x01, 0x01, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "REFRESH 2.2.1A - GCF 27.22.4.7.2" )
        << QByteArray( (char *)data_2_2_1a, sizeof(data_2_2_1a) )
        << QByteArray( (char *)resp_2_2_1a, sizeof(resp_2_2_1a) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::InitAndFullFileChange )
        << 0            // Number of files
        << QString( "" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_2_1b[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x01, 0x00, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_2_2_1b[] =
        {0x81, 0x03, 0x01, 0x01, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x03};
    QTest::newRow( "REFRESH 2.2.1B - GCF 27.22.4.7.2" )
        << QByteArray( (char *)data_2_2_1b, sizeof(data_2_2_1b) )
        << QByteArray( (char *)resp_2_2_1b, sizeof(resp_2_2_1b) )
        << 0x0003       // Command performed with additional EFs read
        << (int)( QSimCommand::InitAndFullFileChange )
        << 0            // Number of files
        << QString( "" )
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_3_1[] =
        {0xD0, 0x09, 0x81, 0x03, 0x01, 0x01, 0x04, 0x82, 0x02, 0x81, 0x82};
    static unsigned char const resp_2_3_1[] =
        {0x81, 0x03, 0x01, 0x01, 0x04, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "REFRESH 2.3.1 - GCF 27.22.4.7.2" )
        << QByteArray( (char *)data_2_3_1, sizeof(data_2_3_1) )
        << QByteArray( (char *)resp_2_3_1, sizeof(resp_2_3_1) )
        << 0x0000       // Command performed successfully
        << (int)( QSimCommand::Reset )
        << 0            // Number of files
        << QString( "" )
        << (int)( QSimCommand::NoPduOptions );
}
