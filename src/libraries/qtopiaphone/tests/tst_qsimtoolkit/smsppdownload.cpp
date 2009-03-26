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
// Convert a service center address into its binary form.
static QByteArray convertAddress( const QString& strin )
{
    QString str = strin;
    QByteArray result;
    if ( str[0] == (QChar)'+' ) {
        result += (char)0x91;
        str = str.mid(1);
    } else {
        result += (char)0x81;
    }
    int byte = 0;
    bool nibble = false;
    foreach ( QChar ch, str ) {
        int digit;
        switch ( ch.unicode() ) {
            case '0':           digit = 0; break;
            case '1':           digit = 1; break;
            case '2':           digit = 2; break;
            case '3':           digit = 3; break;
            case '4':           digit = 4; break;
            case '5':           digit = 5; break;
            case '6':           digit = 6; break;
            case '7':           digit = 7; break;
            case '8':           digit = 8; break;
            case '9':           digit = 9; break;
            case '*':           digit = 10; break;
            case '#':           digit = 11; break;
            case 'A': case 'a': digit = 12; break;
            case 'B': case 'b': digit = 13; break;
            case 'C': case 'c': digit = 14; break;
            default:            digit = -1; break;
        }
        if ( digit != -1 ) {
            if ( !nibble ) {
                byte = digit;
            } else {
                byte |= (digit << 4);
                result += (char)byte;
            }
            nibble = !nibble;
        }
    }
    if ( nibble )
        result += (char)(byte | 0xF0);
    return result;
}

// Test encoding and decoding of SMS-PP DOWNLOAD envelopes based on the
// GCF test strings in GSM 51.010, section 27.22.5.1 - SMS-PP DOWNLOAD.
void tst_QSimToolkit::testEncodeSmsPPDownload_data()
{
    QSimToolkitData::populateDataSmsPPDownload();
}
void tst_QSimToolkit::testEncodeSmsPPDownload()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, tpdu );
    QFETCH( QString, address );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the envelope PDU can be parsed correctly.
    QSimEnvelope decodedEnv = QSimEnvelope::fromPdu(data);
    QVERIFY( decodedEnv.type() == QSimEnvelope::SMSPPDownload );
    QVERIFY( decodedEnv.sourceDevice() == QSimCommand::Network );
    QCOMPARE( decodedEnv.extensionField(0x06), convertAddress( address ) );
    QCOMPARE( decodedEnv.extensionField(0x8B), tpdu );

    // Check that the original envelope PDU can be reconstructed correctly.
    QCOMPARE( decodedEnv.toPdu(), data );
}

// Test that SMS-PP DOWNLOAD envelopes can be properly sent from client applications.
void tst_QSimToolkit::testDeliverSmsPPDownload_data()
{
    QSimToolkitData::populateDataSmsPPDownload();
}
void tst_QSimToolkit::testDeliverSmsPPDownload()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, tpdu );
    QFETCH( QString, address );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the envelope.
    QSimEnvelope env;
    env.setType( QSimEnvelope::SMSPPDownload );
    env.setSourceDevice( QSimCommand::Network );
    env.addExtensionField( 0x06, convertAddress( address ) );
    env.addExtensionField( 0x8B, tpdu );
    client->sendEnvelope( env );

    // Wait for the envelope to be received.
    QVERIFY( QFutureSignal::wait( server, SIGNAL(envelopeSeen()), 100 ) );

    // Check that the envelope is what we expected to get and that we didn't
    // get any terminal responses yet.
    QCOMPARE( server->responseCount(), 0 );
    QCOMPARE( server->envelopeCount(), 1 );
    QCOMPARE( server->lastEnvelope(), data );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for SMS-PP DOWNLOAD from the GCF test cases
// in GSM 51.010, section 27.22.5.1.
void QSimToolkitData::populateDataSmsPPDownload()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("tpdu");
    QTest::addColumn<QString>("address");

    static unsigned char const data_1_2_2[] =
        {0xD1, 0x2D, 0x82, 0x02, 0x83, 0x81, 0x06, 0x09, 0x91, 0x11, 0x22, 0x33,
         0x44, 0x55, 0x66, 0x77, 0xF8, 0x8B, 0x1C, 0x04, 0x04, 0x91, 0x21, 0x43,
         0x7F, 0x16, 0x89, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x53, 0x68,
         0x6F, 0x72, 0x74, 0x20, 0x4D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65};
    static unsigned char const tpdu_1_2_2[] =
        {0x04, 0x04, 0x91, 0x21, 0x43, 0x7F, 0x16, 0x89, 0x10, 0x10, 0x00, 0x00,
         0x00, 0x00, 0x0D, 0x53, 0x68, 0x6F, 0x72, 0x74, 0x20, 0x4D, 0x65, 0x73,
         0x73, 0x61, 0x67, 0x65};
    QTest::newRow( "SMS-PP DOWNLOAD 1.2.2 - GCF 27.22.5.1" )
        << QByteArray( (char *)data_1_2_2, sizeof(data_1_2_2) )
        << QByteArray( (char *)tpdu_1_2_2, sizeof(tpdu_1_2_2) )
        << QString( "+112233445566778" );

    static unsigned char const data_1_6_2[] =
        {0xD1, 0x2D, 0x82, 0x02, 0x83, 0x81, 0x06, 0x09, 0x91, 0x11, 0x22, 0x33,
         0x44, 0x55, 0x66, 0x77, 0xF8, 0x8B, 0x1C, 0x04, 0x04, 0x91, 0x21, 0x43,
         0x7F, 0xF6, 0x89, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x53, 0x68,
         0x6F, 0x72, 0x74, 0x20, 0x4D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65};
    static unsigned char const tpdu_1_6_2[] =
        {0x04, 0x04, 0x91, 0x21, 0x43, 0x7F, 0xF6, 0x89, 0x10, 0x10, 0x00, 0x00,
         0x00, 0x00, 0x0D, 0x53, 0x68, 0x6F, 0x72, 0x74, 0x20, 0x4D, 0x65, 0x73,
         0x73, 0x61, 0x67, 0x65};
    QTest::newRow( "SMS-PP DOWNLOAD 1.6.2 - GCF 27.22.5.1" )
        << QByteArray( (char *)data_1_6_2, sizeof(data_1_6_2) )
        << QByteArray( (char *)tpdu_1_6_2, sizeof(tpdu_1_6_2) )
        << QString( "+112233445566778" );
}
