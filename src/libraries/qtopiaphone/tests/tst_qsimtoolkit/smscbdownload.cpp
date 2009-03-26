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
// Test encoding and decoding of SMS-CB DOWNLOAD envelopes based on the
// GCF test strings in GSM 51.010, section 27.22.5.2 - SMS-CB DOWNLOAD.
void tst_QSimToolkit::testEncodeSmsCBDownload_data()
{
    QSimToolkitData::populateDataSmsCBDownload();
}
void tst_QSimToolkit::testEncodeSmsCBDownload()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, page );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the envelope PDU can be parsed correctly.
    QSimEnvelope decodedEnv = QSimEnvelope::fromPdu(data);
    QVERIFY( decodedEnv.type() == QSimEnvelope::CellBroadcastDownload );
    QVERIFY( decodedEnv.sourceDevice() == QSimCommand::Network );
    QCOMPARE( decodedEnv.extensionField(0x8C), page );

    // Check that the original envelope PDU can be reconstructed correctly.
    QCOMPARE( decodedEnv.toPdu(), data );
}

// Test that SMS-CB DOWNLOAD envelopes can be properly sent from client applications.
void tst_QSimToolkit::testDeliverSmsCBDownload_data()
{
    QSimToolkitData::populateDataSmsCBDownload();
}
void tst_QSimToolkit::testDeliverSmsCBDownload()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, page );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the envelope.
    QSimEnvelope env;
    env.setType( QSimEnvelope::CellBroadcastDownload );
    env.setSourceDevice( QSimCommand::Network );
    env.addExtensionField( 0x8C, page );
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

// Populate data-driven tests for SMS-CB DOWNLOAD from the GCF test cases
// in GSM 51.010, section 27.22.5.2.
void QSimToolkitData::populateDataSmsCBDownload()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("page");

    static unsigned char const data_1_1[] =
        {0xD2, 0x5E, 0x82, 0x02, 0x83, 0x81, 0x8C, 0x58, 0xC0, 0x11, 0x10, 0x01,
         0x01, 0x11, 0xC3, 0x32, 0x9B, 0x0D, 0x12, 0xCA, 0xDF, 0x61, 0xF2, 0x38,
         0x3C, 0xA7, 0x83, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x81, 0x40, 0x20,
         0x10, 0x08, 0x04, 0x02, 0x81, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x81,
         0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x81, 0x40, 0x20, 0x10, 0x08, 0x04,
         0x02, 0x81, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x81, 0x40, 0x20, 0x10,
         0x08, 0x04, 0x02, 0x81, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x81, 0x40,
         0x20, 0x10, 0x08, 0x04, 0x02, 0x81, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02};
    static unsigned char const page_1_1[] =
        {0xC0, 0x11, 0x10, 0x01, 0x01, 0x11, 0xC3, 0x32, 0x9B, 0x0D, 0x12, 0xCA,
         0xDF, 0x61, 0xF2, 0x38, 0x3C, 0xA7, 0x83, 0x40, 0x20, 0x10, 0x08, 0x04,
         0x02, 0x81, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x81, 0x40, 0x20, 0x10,
         0x08, 0x04, 0x02, 0x81, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x81, 0x40,
         0x20, 0x10, 0x08, 0x04, 0x02, 0x81, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02,
         0x81, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x81, 0x40, 0x20, 0x10, 0x08,
         0x04, 0x02, 0x81, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x81, 0x40, 0x20,
         0x10, 0x08, 0x04, 0x02};
    QTest::newRow( "SMS-CB DOWNLOAD 1.1 - GCF 27.22.5.2" )
        << QByteArray( (char *)data_1_1, sizeof(data_1_1) )
        << QByteArray( (char *)page_1_1, sizeof(page_1_1) );
}
