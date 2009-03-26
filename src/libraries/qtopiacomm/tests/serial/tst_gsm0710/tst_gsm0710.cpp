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

#include <QObject>
#include <QTest>
#include <QDebug>
#include <string.h>
#include <stdio.h>
#include "gsm0710_p.h"

//TESTED_CLASS=
//TESTED_FILES=src/libraries/qtopiacomm/serial/gsm0710.c

class tst_Gsm0710 : public QObject
{
    Q_OBJECT
private slots:
    void testCrc();
    void testCreate();
    void testCMUX_data();
    void testCMUX();
    void testStartup();
    void testWriteFrame_data();
    void testWriteFrame();
    void testOpenClose();
    void testReadFrame_data();
    void testReadFrame();
    void testReadJoinedFrames();

public:
    int channel;
    int type;
    QByteArray data;
};

void tst_Gsm0710::testCrc()
{
    // Use the example test string given in Appendix B of GSM 27.010
    // to verify that the CRC is being computed correctly.
    static char const testbuf[2] = {(char)0x07, (char)0x3F};
    int crc = gsm0710_compute_crc( testbuf, 2 );
    QVERIFY(crc == 0x89);

    // A more extensive test of crc lookup table coverage.
    char testbuf2[256];
    for ( int index = 0; index < 256; ++index ) {
        testbuf2[index] = (char)(~index);
    }
    crc = gsm0710_compute_crc( testbuf2, 256 );
    QVERIFY(crc == 0x55);
}

void tst_Gsm0710::testCreate()
{
    // Test that gsm0710_initialize puts the context into a good known state.
    struct gsm0710_context ctx;
    gsm0710_initialize( &ctx );
    QVERIFY( ctx.mode == GSM0710_MODE_BASIC );
    QVERIFY( ctx.frame_size == 31 );
    QVERIFY( ctx.port_speed == 115200 );
    QVERIFY( ctx.server == 0 );
    QVERIFY( ctx.buffer_used == 0 );
    QVERIFY( ctx.used_channels[0] == 0 );
    QVERIFY( ctx.used_channels[1] == 0 );
    QVERIFY( ctx.reinit_detect == 0 );
    QVERIFY( ctx.reinit_detect_len == 0 );
    QVERIFY( ctx.user_data == 0 );
    QVERIFY( ctx.fd == -1 );
    QVERIFY( ctx.at_command == 0 );
    QVERIFY( ctx.read == 0 );
    QVERIFY( ctx.write == 0 );
    QVERIFY( ctx.deliver_data == 0 );
    QVERIFY( ctx.deliver_status == 0 );
    QVERIFY( ctx.debug_message == 0 );
    QVERIFY( ctx.open_channel == 0 );
    QVERIFY( ctx.close_channel == 0 );
    QVERIFY( ctx.terminate == 0 );
    QVERIFY( ctx.packet_filter == 0 );
}

// Copy test frame data into the context buffer, where it can be checked later.
static int writeTestFrame
    ( struct gsm0710_context *ctx, const void *data, int len )
{
    if ( !ctx->user_data ) {
        // Replace the buffer.
        memcpy( ctx->buffer, data, len );
        ctx->fd = len;
    } else {
        // Append to the buffer.
        memcpy( ctx->buffer + ctx->fd, data, len );
        ctx->fd += len;
    }
    return len;
}

// Copy AT command data into the context buffer.
static int writeAtCommand( struct gsm0710_context *ctx, const char *cmd )
{
    strcpy( ctx->buffer, cmd );
    return 1;
}

static bool checkBuffer( struct gsm0710_context *ctx, const char *expected,
                         int expectedLen )
{
    int index;
    if ( ctx->fd == expectedLen ) {
        for ( index = 0; index < expectedLen; ++index ) {
            if ( ctx->buffer[index] != expected[index] )
                break;
        }
        if ( index >= expectedLen )
            return true;
    }
    printf( "Actual: " );
    for ( index = 0; index < ctx->fd; ++index ) {
        printf( "0x%02X, ", ctx->buffer[index] & 0xFF );
    }
    printf( "\n" );
    printf( "Expected: " );
    for ( index = 0; index < expectedLen; ++index ) {
        printf( "0x%02X, ", expected[index] & 0xFF );
    }
    printf( "\n" );
    return false;
}

void tst_Gsm0710::testCMUX_data()
{
    QTest::addColumn<int>("mode");
    QTest::addColumn<int>("port_speed");
    QTest::addColumn<int>("frame_size");
    QTest::addColumn<QString>("expected");

    QTest::newRow( "Basic 9600, 31" )
        << GSM0710_MODE_BASIC << 9600 << 31 << "AT+CMUX=0,0,1,31";
    QTest::newRow( "Basic 19200, 256" )
        << GSM0710_MODE_BASIC << 19200 << 256 << "AT+CMUX=0,0,2,256";
    QTest::newRow( "Basic 38400, 1024" )
        << GSM0710_MODE_BASIC << 38400 << 1024 << "AT+CMUX=0,0,3,1024";
    QTest::newRow( "Advanced 57600, 31" )
        << GSM0710_MODE_ADVANCED << 57600 << 31 << "AT+CMUX=1,0,4,31";
    QTest::newRow( "Advanced 115200, 31" )
        << GSM0710_MODE_ADVANCED << 115200 << 31 << "AT+CMUX=1,0,5,31";
    QTest::newRow( "Advanced 230400, 31" )
        << GSM0710_MODE_ADVANCED << 230400 << 31 << "AT+CMUX=1,0,6,31";

    // Unknown port speeds should convert into 115200.
    QTest::newRow( "Advanced 460800, 31" )
        << GSM0710_MODE_ADVANCED << 460800 << 31 << "AT+CMUX=1,0,5,31";
    QTest::newRow( "Advanced 4, 31" )
        << GSM0710_MODE_ADVANCED << 4 << 31 << "AT+CMUX=1,0,5,31";
}

void tst_Gsm0710::testCMUX()
{
    QFETCH( int, mode );
    QFETCH( int, port_speed );
    QFETCH( int, frame_size );
    QFETCH( QString, expected );

    struct gsm0710_context ctx;
    gsm0710_initialize( &ctx );
    ctx.mode = mode;
    ctx.port_speed = port_speed;
    ctx.frame_size = frame_size;
    ctx.at_command = writeAtCommand;
    memset( ctx.buffer, 0, sizeof(ctx.buffer) );

    QVERIFY( gsm0710_startup( &ctx, 1 ) );
    QCOMPARE( QString(ctx.buffer), expected );
}

void tst_Gsm0710::testStartup()
{
    struct gsm0710_context ctx;
    gsm0710_initialize( &ctx );
    ctx.write = writeTestFrame;

    // Send the initial startup request, which should create channel 0 only.
    gsm0710_startup( &ctx, 0 );
    static char const result1[] = {0xF9, 0x03, 0x3F, 0x01, 0x1C, 0xF9};
    QVERIFY( checkBuffer( &ctx, result1, sizeof(result1) ) );

    // Open channels 1, 2, and 4.
    gsm0710_open_channel( &ctx, 1 );
    gsm0710_open_channel( &ctx, 2 );
    gsm0710_open_channel( &ctx, 4 );

    // Send the startup again, which should recreate 0, 1, 2, and 4.
    ctx.user_data = &ctx;
    ctx.fd = 0;
    gsm0710_startup( &ctx, 0 );
    static char const result2[] =
        {0xF9, 0x03, 0x3F, 0x01, 0x1C, 0xF9,
         0xF9, 0x07, 0x3F, 0x01, 0xDE, 0xF9,
         0xF9, 0x0B, 0x3F, 0x01, 0x59, 0xF9,
         0xF9, 0x13, 0x3F, 0x01, 0x96, 0xF9};
    QVERIFY( checkBuffer( &ctx, result2, sizeof(result2) ) );
}

void tst_Gsm0710::testWriteFrame_data()
{
    QTest::addColumn<int>("mode");
    QTest::addColumn<int>("channel");
    QTest::addColumn<int>("type");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("expected");

    // Test for correct construction of channel open messages.
    static char const bresult1[] = {0xF9, 0x07, 0x3F, 0x01, 0xDE, 0xF9};
    QTest::newRow( "Basic Open" )
            << GSM0710_MODE_BASIC
            << 1 << GSM0710_OPEN_CHANNEL
            << QByteArray()
            << QByteArray( bresult1, sizeof(bresult1) );

    // Test for correct construction of channel close messages.
    static char const bresult2[] = {0xF9, 0x07, 0x53, 0x01, 0x3F, 0xF9};
    QTest::newRow( "Basic Close" )
            << GSM0710_MODE_BASIC
            << 1 << GSM0710_CLOSE_CHANNEL
            << QByteArray()
            << QByteArray( bresult2, sizeof(bresult2) );

    // Test for correct construction of channel data messages.
    static char const bdata3[] = {0x12, 0x34, 0x56};
    static char const bresult3[] =
        {0xF9, 0x07, 0xEF, 0x07, 0x12, 0x34, 0x56, 0xD3, 0xF9};
    QTest::newRow( "Basic Data" )
            << GSM0710_MODE_BASIC
            << 1 << GSM0710_DATA
            << QByteArray( bdata3, sizeof(bdata3) )
            << QByteArray( bresult3, sizeof(bresult3) );

    // Test that gsm0710_write_frame truncates at the frame size.
    static char const bdata4[] =
            {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    static char const bresult4[] =
            {0xF9, 0x07, 0xEF, 0x3F, 0x12, 0x34, 0x56, 0x78,
             0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78,
             0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78,
             0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78,
             0x9A, 0xBC, 0xDE, 0xF9, 0xF9};
    QTest::newRow( "Basic Truncated" )
            << GSM0710_MODE_BASIC
            << 1 << GSM0710_DATA
            << QByteArray( bdata4, sizeof(bdata4) )
            << QByteArray( bresult4, sizeof(bresult4) );

    // Test double-byte length values in the packet header.
    // There is no advanced-mode equivalent for this because
    // advanced mode does not transmit the length: it is implicit.
    static char const bdata5[] =
            {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    static char const bresult5[] =
            {0xF9, 0x07, 0xEF, 0x10, 0x01, 0x12, 0x34, 0x56,
             0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56,
             0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56,
             0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56,
             0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56,
             0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56,
             0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56,
             0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56,
             0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56,
             0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56,
             0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56,
             0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56,
             0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56,
             0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56,
             0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56,
             0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56,
             0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56,
             0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x88, 0xF9};
    QTest::newRow( "Basic Large" )
            << (GSM0710_MODE_BASIC | 0x80)
            << 1 << GSM0710_DATA
            << QByteArray( bdata5, sizeof(bdata5) )
            << QByteArray( bresult5, sizeof(bresult5) );

    // Test for correct construction of channel open messages.
    static char const aresult1[] = {0x7E, 0x07, 0x3F, 0x89, 0x7E};
    QTest::newRow( "Advanced Open" )
            << GSM0710_MODE_ADVANCED
            << 1 << GSM0710_OPEN_CHANNEL
            << QByteArray()
            << QByteArray( aresult1, sizeof(aresult1) );

    // Test for correct construction of channel close messages.
    static char const aresult2[] = {0x7E, 0x07, 0x53, 0xC8, 0x7E};
    QTest::newRow( "Advanced Close" )
            << GSM0710_MODE_ADVANCED
            << 1 << GSM0710_CLOSE_CHANNEL
            << QByteArray()
            << QByteArray( aresult2, sizeof(aresult2) );

    // Test for correct construction of channel data messages.
    static char const adata3[] = {0x12, 0x34, 0x56};
    static char const aresult3[] =
        {0x7E, 0x07, 0xEF, 0x12, 0x34, 0x56, 0x05, 0x7E};
    QTest::newRow( "Advanced Data" )
            << GSM0710_MODE_ADVANCED
            << 1 << GSM0710_DATA
            << QByteArray( adata3, sizeof(adata3) )
            << QByteArray( aresult3, sizeof(aresult3) );

    // Test for correct construction of channel data messages, with quoting.
    static char const adata4[] = {0x12, 0x34, 0x56, 0x7E, 0x78, 0x7D};
    static char const aresult4[] =
        {0x7E, 0x07, 0xEF, 0x12, 0x34, 0x56, 0x7D, 0x5E,
         0x78, 0x7D, 0x5D, 0x05, 0x7E};
    QTest::newRow( "Advanced Quoted Data" )
            << GSM0710_MODE_ADVANCED
            << 1 << GSM0710_DATA
            << QByteArray( adata4, sizeof(adata4) )
            << QByteArray( aresult4, sizeof(aresult4) );

    // Test that gsm0710_write_frame truncates at the frame size.
    static char const adata5[] =
            {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
             0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    static char const aresult5[] =
            {0x7E, 0x07, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x9A,
             0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78, 0x9A,
             0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78, 0x9A,
             0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78, 0x9A,
             0xBC, 0xDE, 0x05, 0x7E};
    QTest::newRow( "Advanced Truncated" )
            << GSM0710_MODE_ADVANCED
            << 1 << GSM0710_DATA
            << QByteArray( adata5, sizeof(adata5) )
            << QByteArray( aresult5, sizeof(aresult5) );

}

void tst_Gsm0710::testWriteFrame()
{
    QFETCH( int, mode );
    QFETCH( int, channel );
    QFETCH( int, type );
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, expected );

    struct gsm0710_context ctx;
    gsm0710_initialize( &ctx );
    if ( ( mode & 0x80 ) != 0 ) {
        ctx.mode = mode & 0x7F;
        ctx.frame_size = 256;
    } else {
        ctx.mode = mode;
    }
    ctx.write = writeTestFrame;

    gsm0710_write_frame( &ctx, channel, type, data.constData(), data.size() );
    QByteArray result( ctx.buffer, ctx.fd );
    if ( result != expected ) {
        int index;
        printf( "Actual: " );
        for ( index = 0; index < ctx.fd; ++index ) {
            printf( "0x%02X, ", ctx.buffer[index] & 0xFF );
        }
        printf( "\n" );
        printf( "Expected: " );
        for ( index = 0; index < expected.size(); ++index ) {
            printf( "0x%02X, ", expected[index] & 0xFF );
        }
        printf( "\n" );
    }
    QCOMPARE( result, expected );
}

void tst_Gsm0710::testOpenClose()
{
    struct gsm0710_context ctx;
    gsm0710_initialize( &ctx );
    ctx.write = writeTestFrame;

    // Check for invalid channel numbers.
    QVERIFY( !gsm0710_open_channel( &ctx, -1 ) );
    QVERIFY( !gsm0710_open_channel( &ctx, 0 ) );
    QVERIFY( !gsm0710_open_channel( &ctx, GSM0710_MAX_CHANNELS + 1 ) );

    // Open channel 1 and check the frame that was sent.
    QVERIFY( !gsm0710_is_channel_open( &ctx, 1 ) );
    QVERIFY( gsm0710_open_channel( &ctx, 1 ) );
    static char const result1[] = {0xF9, 0x07, 0x3F, 0x01, 0xDE, 0xF9};
    QVERIFY( checkBuffer( &ctx, result1, sizeof(result1) ) );
    QVERIFY( gsm0710_is_channel_open( &ctx, 1 ) );

    // Open it again and verify that no further bytes are sent.
    ctx.fd = -1;
    QVERIFY( gsm0710_open_channel( &ctx, 1 ) );
    QVERIFY( ctx.fd == -1 );
    QVERIFY( gsm0710_is_channel_open( &ctx, 1 ) );

    // Close channel 1 and check the frame that was sent.
    gsm0710_close_channel( &ctx, 1 );
    static char const result2[] = {0xF9, 0x07, 0x53, 0x01, 0x3F, 0xF9};
    QVERIFY( checkBuffer( &ctx, result2, sizeof(result2) ) );
    QVERIFY( !gsm0710_is_channel_open( &ctx, 1 ) );

    // Check that nothing is sent when the channel is already closed.
    ctx.fd = -1;
    gsm0710_close_channel( &ctx, 1 );
    QVERIFY( ctx.fd == -1 );

    // Check that all valid channel numbers can be opened.
    int channel;
    for ( channel = 1; channel <= GSM0710_MAX_CHANNELS; ++channel ) {
        QVERIFY( !gsm0710_is_channel_open( &ctx, channel ) );
        QVERIFY( gsm0710_open_channel( &ctx, channel ) );
        QVERIFY( gsm0710_is_channel_open( &ctx, channel ) );
    }

    // Check the shutdown sequence, which should close everything.
    // Close channel 13 first, to check that it will skip closed channels.
    // This test also indirectly checks that GSM0710_MAX_CHANNELS is 63.
    // If the define is changed, then this test will need to be changed too.
    gsm0710_close_channel( &ctx, 13 );
    ctx.user_data = &ctx;
    ctx.fd = 0;
    gsm0710_shutdown( &ctx );
    static char const result3[] =
        {0xF9, 0x07, 0x53, 0x01, 0x3F, 0xF9,
         0xF9, 0x0B, 0x53, 0x01, 0xB8, 0xF9,
         0xF9, 0x0F, 0x53, 0x01, 0x7A, 0xF9,
         0xF9, 0x13, 0x53, 0x01, 0x77, 0xF9,
         0xF9, 0x17, 0x53, 0x01, 0xB5, 0xF9,
         0xF9, 0x1B, 0x53, 0x01, 0x32, 0xF9,
         0xF9, 0x1F, 0x53, 0x01, 0xF0, 0xF9,
         0xF9, 0x23, 0x53, 0x01, 0x28, 0xF9,
         0xF9, 0x27, 0x53, 0x01, 0xEA, 0xF9,
         0xF9, 0x2B, 0x53, 0x01, 0x6D, 0xF9,
         0xF9, 0x2F, 0x53, 0x01, 0xAF, 0xF9,
         0xF9, 0x33, 0x53, 0x01, 0xA2, 0xF9,
         //0xF9, 0x37, 0x53, 0x01, 0x60, 0xF9,  // Closed channel 13
         0xF9, 0x3B, 0x53, 0x01, 0xE7, 0xF9,
         0xF9, 0x3F, 0x53, 0x01, 0x25, 0xF9,
         0xF9, 0x43, 0x53, 0x01, 0x96, 0xF9,
         0xF9, 0x47, 0x53, 0x01, 0x54, 0xF9,
         0xF9, 0x4B, 0x53, 0x01, 0xD3, 0xF9,
         0xF9, 0x4F, 0x53, 0x01, 0x11, 0xF9,
         0xF9, 0x53, 0x53, 0x01, 0x1C, 0xF9,
         0xF9, 0x57, 0x53, 0x01, 0xDE, 0xF9,
         0xF9, 0x5B, 0x53, 0x01, 0x59, 0xF9,
         0xF9, 0x5F, 0x53, 0x01, 0x9B, 0xF9,
         0xF9, 0x63, 0x53, 0x01, 0x43, 0xF9,
         0xF9, 0x67, 0x53, 0x01, 0x81, 0xF9,
         0xF9, 0x6B, 0x53, 0x01, 0x06, 0xF9,
         0xF9, 0x6F, 0x53, 0x01, 0xC4, 0xF9,
         0xF9, 0x73, 0x53, 0x01, 0xC9, 0xF9,
         0xF9, 0x77, 0x53, 0x01, 0x0B, 0xF9,
         0xF9, 0x7B, 0x53, 0x01, 0x8C, 0xF9,
         0xF9, 0x7F, 0x53, 0x01, 0x4E, 0xF9,
         0xF9, 0x83, 0x53, 0x01, 0x2B, 0xF9,
         0xF9, 0x87, 0x53, 0x01, 0xE9, 0xF9,
         0xF9, 0x8B, 0x53, 0x01, 0x6E, 0xF9,
         0xF9, 0x8F, 0x53, 0x01, 0xAC, 0xF9,
         0xF9, 0x93, 0x53, 0x01, 0xA1, 0xF9,
         0xF9, 0x97, 0x53, 0x01, 0x63, 0xF9,
         0xF9, 0x9B, 0x53, 0x01, 0xE4, 0xF9,
         0xF9, 0x9F, 0x53, 0x01, 0x26, 0xF9,
         0xF9, 0xA3, 0x53, 0x01, 0xFE, 0xF9,
         0xF9, 0xA7, 0x53, 0x01, 0x3C, 0xF9,
         0xF9, 0xAB, 0x53, 0x01, 0xBB, 0xF9,
         0xF9, 0xAF, 0x53, 0x01, 0x79, 0xF9,
         0xF9, 0xB3, 0x53, 0x01, 0x74, 0xF9,
         0xF9, 0xB7, 0x53, 0x01, 0xB6, 0xF9,
         0xF9, 0xBB, 0x53, 0x01, 0x31, 0xF9,
         0xF9, 0xBF, 0x53, 0x01, 0xF3, 0xF9,
         0xF9, 0xC3, 0x53, 0x01, 0x40, 0xF9,
         0xF9, 0xC7, 0x53, 0x01, 0x82, 0xF9,
         0xF9, 0xCB, 0x53, 0x01, 0x05, 0xF9,
         0xF9, 0xCF, 0x53, 0x01, 0xC7, 0xF9,
         0xF9, 0xD3, 0x53, 0x01, 0xCA, 0xF9,
         0xF9, 0xD7, 0x53, 0x01, 0x08, 0xF9,
         0xF9, 0xDB, 0x53, 0x01, 0x8F, 0xF9,
         0xF9, 0xDF, 0x53, 0x01, 0x4D, 0xF9,
         0xF9, 0xE3, 0x53, 0x01, 0x95, 0xF9,
         0xF9, 0xE7, 0x53, 0x01, 0x57, 0xF9,
         0xF9, 0xEB, 0x53, 0x01, 0xD0, 0xF9,
         0xF9, 0xEF, 0x53, 0x01, 0x12, 0xF9,
         0xF9, 0xF3, 0x53, 0x01, 0x1F, 0xF9,
         0xF9, 0xF7, 0x53, 0x01, 0xDD, 0xF9,
         0xF9, 0xFB, 0x53, 0x01, 0x5A, 0xF9,
         0xF9, 0xFF, 0x53, 0x01, 0x98, 0xF9,
         0xF9, 0x03, 0xEF, 0x05, 0xC3, 0x01, 0xF2, 0xF9};
    QVERIFY( checkBuffer( &ctx, result3, sizeof(result3) ) );
    for ( channel = 1; channel <= GSM0710_MAX_CHANNELS; ++channel ) {
        QVERIFY( !gsm0710_is_channel_open( &ctx, channel ) );
    }

    // Shutdown one more time to check that close messages are only
    // sent for channels that are open.
    ctx.fd = 0;
    gsm0710_shutdown( &ctx );
    static char const result4[] =
        {0xF9, 0x03, 0xEF, 0x05, 0xC3, 0x01, 0xF2, 0xF9};
    QVERIFY( checkBuffer( &ctx, result4, sizeof(result4) ) );
}

// Read a test frame into the input buffer.
static int readTestFrame(struct gsm0710_context *ctx, void *data, int len)
{
    len = ctx->fd;
    ctx->fd = 0;
    memcpy( data, ctx->buffer + sizeof(ctx->buffer) / 2, len );
    return len;
}

// Set the test frame data to use on the next read call.
static void setTestFrame(struct gsm0710_context *ctx, const char *data, int len)
{
    memcpy( ctx->buffer + sizeof(ctx->buffer) / 2, data, len );
    ctx->fd = len;
}

// Filter incoming packets to check that they were received correctly.
static int packetFilter(struct gsm0710_context *ctx, int channel,
                        int type, const char *data, int len)
{
    tst_Gsm0710 *tst = (tst_Gsm0710 *)(ctx->user_data);
    tst->channel = channel;
    tst->type = type;
    tst->data = QByteArray( data, len );
    return 1;
}

// Filter incoming packets to check that they were received correctly.
// Append the data instead of replacing.
static int packetFilterAppend(struct gsm0710_context *ctx, int channel,
                              int type, const char *data, int len)
{
    tst_Gsm0710 *tst = (tst_Gsm0710 *)(ctx->user_data);
    tst->channel = channel;
    tst->type = type;
    tst->data += QByteArray( data, len );
    return 1;
}

void tst_Gsm0710::testReadFrame_data()
{
    // We use the same data as for frame writes, but run the tests in reverse.
    testWriteFrame_data();
}

void tst_Gsm0710::testReadFrame()
{
    QFETCH( int, mode );
    QFETCH( int, channel );
    QFETCH( int, type );
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, expected );

    struct gsm0710_context ctx;
    gsm0710_initialize( &ctx );
    if ( ( mode & 0x80 ) != 0 ) {
        ctx.mode = mode & 0x7F;
        ctx.frame_size = 256;
    } else {
        ctx.mode = mode;
    }
    ctx.read = readTestFrame;
    ctx.packet_filter = packetFilter;
    ctx.user_data = (void *)this;

    static char const dummy[2] = {(char)0xFF, (char)0x56};
    this->channel = -1;
    this->type = -1;
    this->data = QByteArray( dummy, sizeof(dummy) );

    setTestFrame( &ctx, expected.constData(), expected.size() );
    gsm0710_ready_read( &ctx );

    QCOMPARE( this->channel, channel );
    QCOMPARE( this->type, type & 0xEF );    /* PF bit is stripped internally */
    if ( data.size() <= ctx.frame_size )
        QCOMPARE( this->data, data );
    else
        QCOMPARE( this->data, data.left( ctx.frame_size ) );
}

// In basic mode, the 0xF9 terminator for one frame can be
// the begin marker for the next frame.  Test such "joined" frames.
void tst_Gsm0710::testReadJoinedFrames()
{
    struct gsm0710_context ctx;
    gsm0710_initialize( &ctx );
    ctx.read = readTestFrame;
    ctx.packet_filter = packetFilterAppend;
    ctx.user_data = (void *)this;

    this->channel = -1;
    this->type = -1;
    this->data = QByteArray();

    static char const input[] =
        {0xF9, 0x07, 0xEF, 0x07, 0x12, 0x34, 0x56, 0xD3, 0xF9,
               0x07, 0xEF, 0x07, 0x12, 0x34, 0x56, 0xD3, 0xF9};
    static char const output[] =
        {0x12, 0x34, 0x56, 0x12, 0x34, 0x56};
    setTestFrame( &ctx, input, sizeof(input) );
    gsm0710_ready_read( &ctx );

    QCOMPARE( this->channel, 1 );
    QCOMPARE( this->type, GSM0710_DATA & 0xEF ); // PF bit stripped internally.
    QCOMPARE( this->data, QByteArray(output, sizeof(output)) );
}

QTEST_MAIN( tst_Gsm0710 )

#include "tst_gsm0710.moc"
