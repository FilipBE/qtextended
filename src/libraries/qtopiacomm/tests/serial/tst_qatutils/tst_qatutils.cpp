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
#include <qatutils.h>
#include <qtextcodec.h>

//TESTED_CLASS=QAtUtils
//TESTED_FILES=src/libraries/qtopiacomm/serial/qatutils.cpp

class tst_QAtUtils : public QObject
{
    Q_OBJECT
private slots:
    void testQuote();
    void testToHex();
    void testFromHex();
    void testDecodeNumber();
    void testEncodeNumber();
    void testNextString_data();
    void testNextString();
    void testParseNumber_data();
    void testParseNumber();
    void testSkipField_data();
    void testSkipField();
    void testPccp437Codec_data();
    void testPccp437Codec();
    void testPccp850Codec_data();
    void testPccp850Codec();
    void testUcs2HexCodec();
    void testStripNumber_data();
    void testStripNumber();
    void testAuxCodecs();

private:
    bool seen[256];
};

void tst_QAtUtils::testQuote()
{
    // Check default quoting behaviour for strict ASCII strings.
    QCOMPARE( QAtUtils::quote(""), QString("") );
    QCOMPARE( QAtUtils::quote("a"), QString("a") );
    QCOMPARE( QAtUtils::quote("lazy dog"), QString("lazy dog") );
    QCOMPARE( QAtUtils::quote("lazy \"dog"), QString("lazy \\22dog") );
    QCOMPARE( QAtUtils::quote("\\"), QString("\\5C") );
    QCOMPARE( QAtUtils::quote("\r\n"), QString("\\0D\\0A") );

    // Check that codec is applied before quoting.
    QTextCodec *gsm = QAtUtils::codec( "gsm" );
    QCOMPARE( QAtUtils::quote("", gsm), QString("") );
    QCOMPARE( QAtUtils::quote("a", gsm), QString("a") );
    QCOMPARE( QAtUtils::quote("lazy dog", gsm), QString("lazy dog") );
    QCOMPARE( QAtUtils::quote("lazy \"dog", gsm), QString("lazy \\22dog") );
    QCOMPARE( QAtUtils::quote("\\", gsm), QString("\x1B\x2F") );
    QCOMPARE( QAtUtils::quote("\xD6", gsm), QString("\\5C") );
    QCOMPARE( QAtUtils::quote("\r\n", gsm), QString("\\0D\\0A") );
}

void tst_QAtUtils::testToHex()
{
    QCOMPARE( QAtUtils::toHex(QByteArray()), QString("") );
    QCOMPARE( QAtUtils::toHex("ABC\""), QString("41424322") );
    QCOMPARE( QAtUtils::toHex("\r\n"), QString("0D0A") );
}

void tst_QAtUtils::testFromHex()
{
    QCOMPARE( QAtUtils::fromHex(""), QByteArray() );
    QCOMPARE( QAtUtils::fromHex("41424322"), QByteArray("ABC\"") );
    QCOMPARE( QAtUtils::fromHex("0D0A"), QByteArray("\r\n") );
    QCOMPARE( QAtUtils::fromHex("0D\n0A"), QByteArray("\r\n") );
    QCOMPARE( QAtUtils::fromHex(" 0,D0A:"), QByteArray("\r\n") );
}

void tst_QAtUtils::testDecodeNumber()
{
    QCOMPARE( QAtUtils::decodeNumber("1194", 129), QString("1194") );
    QCOMPARE( QAtUtils::decodeNumber("1194", 145), QString("+1194") );
    QCOMPARE( QAtUtils::decodeNumber("+1194", 145), QString("+1194") );
    QCOMPARE( QAtUtils::decodeNumber("", 129), QString("") );
    QCOMPARE( QAtUtils::decodeNumber("", 145), QString("") );
    QCOMPARE( QAtUtils::decodeNumber("1194", 34), QString("1194") );
}

void tst_QAtUtils::testEncodeNumber()
{
    QCOMPARE( QAtUtils::encodeNumber("1194", true), QString("\"1194\",129") );
    QCOMPARE( QAtUtils::encodeNumber("1194", false), QString("\"1194\",129") );
    QCOMPARE( QAtUtils::encodeNumber("+1194", true), QString("\"+1194\",145") );
    QCOMPARE( QAtUtils::encodeNumber("+1194", false), QString("\"1194\",145") );
    QCOMPARE( QAtUtils::encodeNumber("", true), QString("\"\",129") );
    QCOMPARE( QAtUtils::encodeNumber("", false), QString("\"\",129") );
}

void tst_QAtUtils::testNextString_data()
{
    QTest::addColumn<QString>("str");
    QTest::addColumn<QString>("result");
    QTest::addColumn<int>("posn");
    QTest::addColumn<int>("endPosn");

    QTest::newRow( "empty 1" )
        << "" << "" << 0 << 0;
    QTest::newRow( "empty 2" )
        << "   " << "" << 0 << 3;
    QTest::newRow( "position" )
        << "" << "" << 100 << 100;
    QTest::newRow( "no string" )
        << "abcdef" << "" << 0 << 6;
    QTest::newRow( "one string" )
        << "\"abcdef\"" << "abcdef" << 0 << 8;
    QTest::newRow( "find string" )
        << "12,\"abcdef\"" << "abcdef" << 0 << 11;
    QTest::newRow( "non-term string" )
        << "12,\"abcdef" << "abcdef" << 0 << 10;
    QTest::newRow( "parse till comma" )
        << "\"abcdef\",\"123\"" << "abcdef" << 0 << 8;
    QTest::newRow( "parse from comma" )
        << "\"abcdef\",\"123\"" << "123" << 8 << 14;
    QTest::newRow( "good escapes" )
        << "\"\\22\\5C\\0D\\0A\"" << "\"\\\r\n" << 0 << 14;
    QTest::newRow( "odd escapes" )
        << "\"\\2\\Y\\045\\" << "\002\\Y\0045" << 0 << 10;
}

void tst_QAtUtils::testNextString()
{
    QFETCH(QString, str);
    QFETCH(QString, result);
    QFETCH(int, posn);
    QFETCH(int, endPosn);

    uint tmp = (uint)posn;
    QCOMPARE( QAtUtils::nextString(str, tmp), result );
    QCOMPARE( tmp, (uint)endPosn );
}

void tst_QAtUtils::testParseNumber_data()
{
    QTest::addColumn<QString>("str");
    QTest::addColumn<int>("result");
    QTest::addColumn<int>("posn");
    QTest::addColumn<int>("endPosn");

    QTest::newRow( "empty 1" )
        << "" << 0 << 0 << 0;
    QTest::newRow( "empty 2" )
        << "   " << 0 << 0 << 3;
    QTest::newRow( "simple 1" )
        << "0" << 0 << 0 << 1;
    QTest::newRow( "simple 2" )
        << "5" << 5 << 0 << 1;
    QTest::newRow( "simple 3" )
        << "123456" << 123456 << 0 << 6;
    QTest::newRow( "simple 4" )
        << "123,456" << 123 << 0 << 3;
    QTest::newRow( "simple 5" )
        << "123,456" << 456 << 3 << 7;
    QTest::newRow( "large 1" )
        << "4294967295" << (int)0xFFFFFFFF << 0 << 10;
    QTest::newRow( "large 2" )
        << "4294967296" << 0 << 0 << 10;
    QTest::newRow( "skip 1" )
        << "   5" << 5 << 0 << 4;
    QTest::newRow( "skip 2" )
        << ",5" << 5 << 0 << 2;
    QTest::newRow( "stop 1" )
        << "5," << 5 << 0 << 1;
    QTest::newRow( "stop 2" )
        << "5y" << 5 << 0 << 1;
    QTest::newRow( "stop 3" )
        << "   5 " << 5 << 0 << 4;
}

void tst_QAtUtils::testParseNumber()
{
    QFETCH(QString, str);
    QFETCH(int, result);
    QFETCH(int, posn);
    QFETCH(int, endPosn);

    uint tmp = (uint)posn;
    QCOMPARE( QAtUtils::parseNumber(str, tmp), (uint)result );
    QCOMPARE( tmp, (uint)endPosn );
}

void tst_QAtUtils::testSkipField_data()
{
    QTest::addColumn<QString>("str");
    QTest::addColumn<int>("posn");
    QTest::addColumn<int>("endPosn");

    QTest::newRow( "empty 1" )
        << "" << 0 << 0;
    QTest::newRow( "empty 2" )
        << "   " << 0 << 3;
    QTest::newRow( "skip 1" )
        << "a," << 0 << 1;
    QTest::newRow( "skip 2" )
        << ",," << 0 << 1;
    QTest::newRow( "skip 3" )
        << "aaaa," << 0 << 4;
    QTest::newRow( "skip 4" )
        << ",aaaa," << 0 << 5;
}

void tst_QAtUtils::testSkipField()
{
    QFETCH(QString, str);
    QFETCH(int, posn);
    QFETCH(int, endPosn);

    uint tmp = (uint)posn;
    QAtUtils::skipField(str, tmp);
    QCOMPARE( tmp, (uint)endPosn );
}

void tst_QAtUtils::testPccp437Codec_data()
{
    struct codepair { int unicode, pccp437; };

    // Mapping table from the standard ibm-437.ucm file from IBM.
    static struct codepair const table[] = {
        {0x0000, 0x00},
        {0x0001, 0x01},
        {0x0002, 0x02},
        {0x0003, 0x03},
        {0x0004, 0x04},
        {0x0005, 0x05},
        {0x0006, 0x06},
        {0x0007, 0x07},
        {0x0008, 0x08},
        {0x0009, 0x09},
        {0x000A, 0x0A},
        {0x000B, 0x0B},
        {0x000C, 0x0C},
        {0x000D, 0x0D},
        {0x000E, 0x0E},
        {0x000F, 0x0F},
        {0x0010, 0x10},
        {0x0011, 0x11},
        {0x0012, 0x12},
        {0x0013, 0x13},
        {0x0014, 0x14},
        {0x0015, 0x15},
        {0x0016, 0x16},
        {0x0017, 0x17},
        {0x0018, 0x18},
        {0x0019, 0x19},
        {0x001A, 0x7F},
        {0x001B, 0x1B},
        {0x001C, 0x1A},
        {0x001D, 0x1D},
        {0x001E, 0x1E},
        {0x001F, 0x1F},
        {0x0020, 0x20},
        {0x0021, 0x21},
        {0x0022, 0x22},
        {0x0023, 0x23},
        {0x0024, 0x24},
        {0x0025, 0x25},
        {0x0026, 0x26},
        {0x0027, 0x27},
        {0x0028, 0x28},
        {0x0029, 0x29},
        {0x002A, 0x2A},
        {0x002B, 0x2B},
        {0x002C, 0x2C},
        {0x002D, 0x2D},
        {0x002E, 0x2E},
        {0x002F, 0x2F},
        {0x0030, 0x30},
        {0x0031, 0x31},
        {0x0032, 0x32},
        {0x0033, 0x33},
        {0x0034, 0x34},
        {0x0035, 0x35},
        {0x0036, 0x36},
        {0x0037, 0x37},
        {0x0038, 0x38},
        {0x0039, 0x39},
        {0x003A, 0x3A},
        {0x003B, 0x3B},
        {0x003C, 0x3C},
        {0x003D, 0x3D},
        {0x003E, 0x3E},
        {0x003F, 0x3F},
        {0x0040, 0x40},
        {0x0041, 0x41},
        {0x0042, 0x42},
        {0x0043, 0x43},
        {0x0044, 0x44},
        {0x0045, 0x45},
        {0x0046, 0x46},
        {0x0047, 0x47},
        {0x0048, 0x48},
        {0x0049, 0x49},
        {0x004A, 0x4A},
        {0x004B, 0x4B},
        {0x004C, 0x4C},
        {0x004D, 0x4D},
        {0x004E, 0x4E},
        {0x004F, 0x4F},
        {0x0050, 0x50},
        {0x0051, 0x51},
        {0x0052, 0x52},
        {0x0053, 0x53},
        {0x0054, 0x54},
        {0x0055, 0x55},
        {0x0056, 0x56},
        {0x0057, 0x57},
        {0x0058, 0x58},
        {0x0059, 0x59},
        {0x005A, 0x5A},
        {0x005B, 0x5B},
        {0x005C, 0x5C},
        {0x005D, 0x5D},
        {0x005E, 0x5E},
        {0x005F, 0x5F},
        {0x0060, 0x60},
        {0x0061, 0x61},
        {0x0062, 0x62},
        {0x0063, 0x63},
        {0x0064, 0x64},
        {0x0065, 0x65},
        {0x0066, 0x66},
        {0x0067, 0x67},
        {0x0068, 0x68},
        {0x0069, 0x69},
        {0x006A, 0x6A},
        {0x006B, 0x6B},
        {0x006C, 0x6C},
        {0x006D, 0x6D},
        {0x006E, 0x6E},
        {0x006F, 0x6F},
        {0x0070, 0x70},
        {0x0071, 0x71},
        {0x0072, 0x72},
        {0x0073, 0x73},
        {0x0074, 0x74},
        {0x0075, 0x75},
        {0x0076, 0x76},
        {0x0077, 0x77},
        {0x0078, 0x78},
        {0x0079, 0x79},
        {0x007A, 0x7A},
        {0x007B, 0x7B},
        {0x007C, 0x7C},
        {0x007D, 0x7D},
        {0x007E, 0x7E},
        {0x007F, 0x1C},
        {0x00A0, 0xFF},
        {0x00A1, 0xAD},
        {0x00A2, 0x9B},
        {0x00A3, 0x9C},
        {0x00A5, 0x9D},
        {0x00A7, 0x15},
        {0x00AA, 0xA6},
        {0x00AB, 0xAE},
        {0x00AC, 0xAA},
        {0x00B0, 0xF8},
        {0x00B1, 0xF1},
        {0x00B2, 0xFD},
        {0x00B6, 0x14},
        {0x00B7, 0xFA},
        {0x00BA, 0xA7},
        {0x00BB, 0xAF},
        {0x00BC, 0xAC},
        {0x00BD, 0xAB},
        {0x00BF, 0xA8},
        {0x00C4, 0x8E},
        {0x00C5, 0x8F},
        {0x00C6, 0x92},
        {0x00C7, 0x80},
        {0x00C9, 0x90},
        {0x00D1, 0xA5},
        {0x00D6, 0x99},
        {0x00DC, 0x9A},
        {0x00DF, 0xE1},
        {0x00E0, 0x85},
        {0x00E1, 0xA0},
        {0x00E2, 0x83},
        {0x00E4, 0x84},
        {0x00E5, 0x86},
        {0x00E6, 0x91},
        {0x00E7, 0x87},
        {0x00E8, 0x8A},
        {0x00E9, 0x82},
        {0x00EA, 0x88},
        {0x00EB, 0x89},
        {0x00EC, 0x8D},
        {0x00ED, 0xA1},
        {0x00EE, 0x8C},
        {0x00EF, 0x8B},
        {0x00F1, 0xA4},
        {0x00F2, 0x95},
        {0x00F3, 0xA2},
        {0x00F4, 0x93},
        {0x00F6, 0x94},
        {0x00F7, 0xF6},
        {0x00F9, 0x97},
        {0x00FA, 0xA3},
        {0x00FB, 0x96},
        {0x00FC, 0x81},
        {0x00FF, 0x98},
        {0x0192, 0x9F},
        {0x0393, 0xE2},
        {0x0398, 0xE9},
        {0x03A3, 0xE4},
        {0x03A6, 0xE8},
        {0x03A9, 0xEA},
        {0x03B1, 0xE0},
        {0x03B4, 0xEB},
        {0x03B5, 0xEE},
        {0x03BC, 0xE6},
        {0x03C0, 0xE3},
        {0x03C3, 0xE5},
        {0x03C4, 0xE7},
        {0x03C6, 0xED},
        {0x2022, 0x07},
        {0x203C, 0x13},
        {0x207F, 0xFC},
        {0x20A7, 0x9E},
        {0x2190, 0x1B},
        {0x2191, 0x18},
        {0x2192, 0x1A},
        {0x2193, 0x19},
        {0x2194, 0x1D},
        {0x2195, 0x12},
        {0x21A8, 0x17},
        {0x2219, 0xF9},
        {0x221A, 0xFB},
        {0x221E, 0xEC},
        {0x221F, 0x1C},
        {0x2229, 0xEF},
        {0x2248, 0xF7},
        {0x2261, 0xF0},
        {0x2264, 0xF3},
        {0x2265, 0xF2},
        {0x2302, 0x7F},
        {0x2310, 0xA9},
        {0x2320, 0xF4},
        {0x2321, 0xF5},
        {0x2500, 0xC4},
        {0x2502, 0xB3},
        {0x250C, 0xDA},
        {0x2510, 0xBF},
        {0x2514, 0xC0},
        {0x2518, 0xD9},
        {0x251C, 0xC3},
        {0x2524, 0xB4},
        {0x252C, 0xC2},
        {0x2534, 0xC1},
        {0x253C, 0xC5},
        {0x2550, 0xCD},
        {0x2551, 0xBA},
        {0x2552, 0xD5},
        {0x2553, 0xD6},
        {0x2554, 0xC9},
        {0x2555, 0xB8},
        {0x2556, 0xB7},
        {0x2557, 0xBB},
        {0x2558, 0xD4},
        {0x2559, 0xD3},
        {0x255A, 0xC8},
        {0x255B, 0xBE},
        {0x255C, 0xBD},
        {0x255D, 0xBC},
        {0x255E, 0xC6},
        {0x255F, 0xC7},
        {0x2560, 0xCC},
        {0x2561, 0xB5},
        {0x2562, 0xB6},
        {0x2563, 0xB9},
        {0x2564, 0xD1},
        {0x2565, 0xD2},
        {0x2566, 0xCB},
        {0x2567, 0xCF},
        {0x2568, 0xD0},
        {0x2569, 0xCA},
        {0x256A, 0xD8},
        {0x256B, 0xD7},
        {0x256C, 0xCE},
        {0x2580, 0xDF},
        {0x2584, 0xDC},
        {0x2588, 0xDB},
        {0x258C, 0xDD},
        {0x2590, 0xDE},
        {0x2591, 0xB0},
        {0x2592, 0xB1},
        {0x2593, 0xB2},
        {0x25A0, 0xFE},
        {0x25AC, 0x16},
        {0x25B2, 0x1E},
        {0x25BA, 0x10},
        {0x25BC, 0x1F},
        {0x25C4, 0x11},
        {0x25CB, 0x09},
        {0x25D8, 0x08},
        {0x25D9, 0x0A},
        {0x263A, 0x01},
        {0x263B, 0x02},
        {0x263C, 0x0F},
        {0x2640, 0x0C},
        {0x2642, 0x0B},
        {0x2660, 0x06},
        {0x2663, 0x05},
        {0x2665, 0x03},
        {0x2666, 0x04},
        {0x266A, 0x0D},
        {0x266B, 0x0E},
        {0xFF01, 0x21},
        {0xFF02, 0x22},
        {0xFF03, 0x23},
        {0xFF04, 0x24},
        {0xFF05, 0x25},
        {0xFF06, 0x26},
        {0xFF07, 0x27},
        {0xFF08, 0x28},
        {0xFF09, 0x29},
        {0xFF0A, 0x2A},
        {0xFF0B, 0x2B},
        {0xFF0C, 0x2C},
        {0xFF0D, 0x2D},
        {0xFF0E, 0x2E},
        {0xFF0F, 0x2F},
        {0xFF10, 0x30},
        {0xFF11, 0x31},
        {0xFF12, 0x32},
        {0xFF13, 0x33},
        {0xFF14, 0x34},
        {0xFF15, 0x35},
        {0xFF16, 0x36},
        {0xFF17, 0x37},
        {0xFF18, 0x38},
        {0xFF19, 0x39},
        {0xFF1A, 0x3A},
        {0xFF1B, 0x3B},
        {0xFF1C, 0x3C},
        {0xFF1D, 0x3D},
        {0xFF1E, 0x3E},
        {0xFF1F, 0x3F},
        {0xFF20, 0x40},
        {0xFF21, 0x41},
        {0xFF22, 0x42},
        {0xFF23, 0x43},
        {0xFF24, 0x44},
        {0xFF25, 0x45},
        {0xFF26, 0x46},
        {0xFF27, 0x47},
        {0xFF28, 0x48},
        {0xFF29, 0x49},
        {0xFF2A, 0x4A},
        {0xFF2B, 0x4B},
        {0xFF2C, 0x4C},
        {0xFF2D, 0x4D},
        {0xFF2E, 0x4E},
        {0xFF2F, 0x4F},
        {0xFF30, 0x50},
        {0xFF31, 0x51},
        {0xFF32, 0x52},
        {0xFF33, 0x53},
        {0xFF34, 0x54},
        {0xFF35, 0x55},
        {0xFF36, 0x56},
        {0xFF37, 0x57},
        {0xFF38, 0x58},
        {0xFF39, 0x59},
        {0xFF3A, 0x5A},
        {0xFF3B, 0x5B},
        {0xFF3C, 0x5C},
        {0xFF3D, 0x5D},
        {0xFF3E, 0x5E},
        {0xFF3F, 0x5F},
        {0xFF40, 0x60},
        {0xFF41, 0x61},
        {0xFF42, 0x62},
        {0xFF43, 0x63},
        {0xFF44, 0x64},
        {0xFF45, 0x65},
        {0xFF46, 0x66},
        {0xFF47, 0x67},
        {0xFF48, 0x68},
        {0xFF49, 0x69},
        {0xFF4A, 0x6A},
        {0xFF4B, 0x6B},
        {0xFF4C, 0x6C},
        {0xFF4D, 0x6D},
        {0xFF4E, 0x6E},
        {0xFF4F, 0x6F},
        {0xFF50, 0x70},
        {0xFF51, 0x71},
        {0xFF52, 0x72},
        {0xFF53, 0x73},
        {0xFF54, 0x74},
        {0xFF55, 0x75},
        {0xFF56, 0x76},
        {0xFF57, 0x77},
        {0xFF58, 0x78},
        {0xFF59, 0x79},
        {0xFF5A, 0x7A},
        {0xFF5B, 0x7B},
        {0xFF5C, 0x7C},
        {0xFF5D, 0x7D},
        {0xFF5E, 0x7E},
        {0xFFE8, 0xB3},
        {0xFFE9, 0x1B},
        {0xFFEA, 0x18},
        {0xFFEB, 0x1A},
        {0xFFEC, 0x19},
        {0xFFED, 0xFE},
        {0xFFEE, 0x09},
        {-1, -1}
    };

    QTest::addColumn<int>("unicode");
    QTest::addColumn<int>("pccp437");

    for ( int index = 0; table[index].unicode != -1; ++index ) {
        QString name = "0x" + QString::number(table[index].unicode, 16);
        QTest::newRow( name.toLatin1().constData() )
                << table[index].unicode
                << table[index].pccp437;
    }

    memset( seen, 0, sizeof(seen) );
}

void tst_QAtUtils::testPccp437Codec()
{
    static char const hexchars[] = "0123456789ABCDEF";

    QFETCH( int, unicode );
    QFETCH( int, pccp437 );

    QTextCodec *codec = QAtUtils::codec( "pccp437" );
    QString str = QChar(unicode);
    QByteArray pccphex;
    pccphex += (char)pccp437;
    QByteArray ucs2hex;
    ucs2hex += (char)'8';
    ucs2hex += (char)'0';
    ucs2hex += hexchars[(unicode >> 12) & 0x0F];
    ucs2hex += hexchars[(unicode >> 8) & 0x0F];
    ucs2hex += hexchars[(unicode >> 4) & 0x0F];
    ucs2hex += hexchars[unicode & 0x0F];
    ucs2hex += (char)'F';
    ucs2hex += (char)'F';
    ucs2hex += (char)'F';
    ucs2hex += (char)'F';
    if ( unicode >= 0x0100 ) {
        // We expect this to be encoded with the Wavecom UCS-2 hack,
        // even if there may be a direct PCCP437 mapping, because the
        // UCS-2 version is more efficient than using a lookup table.
        QCOMPARE( codec->fromUnicode( str ), ucs2hex );
    } else {
        // Direct conversion to PCCP437.
        QCOMPARE( codec->fromUnicode( str ), pccphex );
    }

    // Test converting PCCP437 back to Unicode, even for code points
    // that may use the UCS-2 hack in the forward direction.
    if ( !seen[pccp437] ) {
        seen[pccp437] = true;
        QCOMPARE( codec->toUnicode( pccphex ), str );
    } else {
        QCOMPARE( codec->toUnicode( ucs2hex ), str );
    }
}

void tst_QAtUtils::testPccp850Codec_data()
{
    struct codepair { int unicode, pccp850; };

    // Mapping table from the standard ibm-850.ucm file from IBM.
    static struct codepair const table[] = {
        {0x0000, 0x00},
        {0x0001, 0x01},
        {0x0002, 0x02},
        {0x0003, 0x03},
        {0x0004, 0x04},
        {0x0005, 0x05},
        {0x0006, 0x06},
        {0x0007, 0x07},
        {0x0008, 0x08},
        {0x0009, 0x09},
        {0x000A, 0x0A},
        {0x000B, 0x0B},
        {0x000C, 0x0C},
        {0x000D, 0x0D},
        {0x000E, 0x0E},
        {0x000F, 0x0F},
        {0x0010, 0x10},
        {0x0011, 0x11},
        {0x0012, 0x12},
        {0x0013, 0x13},
        {0x0014, 0x14},
        {0x0015, 0x15},
        {0x0016, 0x16},
        {0x0017, 0x17},
        {0x0018, 0x18},
        {0x0019, 0x19},
        {0x001A, 0x7F},
        {0x001B, 0x1B},
        {0x001C, 0x1A},
        {0x001D, 0x1D},
        {0x001E, 0x1E},
        {0x001F, 0x1F},
        {0x0020, 0x20},
        {0x0021, 0x21},
        {0x0022, 0x22},
        {0x0023, 0x23},
        {0x0024, 0x24},
        {0x0025, 0x25},
        {0x0026, 0x26},
        {0x0027, 0x27},
        {0x0028, 0x28},
        {0x0029, 0x29},
        {0x002A, 0x2A},
        {0x002B, 0x2B},
        {0x002C, 0x2C},
        {0x002D, 0x2D},
        {0x002E, 0x2E},
        {0x002F, 0x2F},
        {0x0030, 0x30},
        {0x0031, 0x31},
        {0x0032, 0x32},
        {0x0033, 0x33},
        {0x0034, 0x34},
        {0x0035, 0x35},
        {0x0036, 0x36},
        {0x0037, 0x37},
        {0x0038, 0x38},
        {0x0039, 0x39},
        {0x003A, 0x3A},
        {0x003B, 0x3B},
        {0x003C, 0x3C},
        {0x003D, 0x3D},
        {0x003E, 0x3E},
        {0x003F, 0x3F},
        {0x0040, 0x40},
        {0x0041, 0x41},
        {0x0042, 0x42},
        {0x0043, 0x43},
        {0x0044, 0x44},
        {0x0045, 0x45},
        {0x0046, 0x46},
        {0x0047, 0x47},
        {0x0048, 0x48},
        {0x0049, 0x49},
        {0x004A, 0x4A},
        {0x004B, 0x4B},
        {0x004C, 0x4C},
        {0x004D, 0x4D},
        {0x004E, 0x4E},
        {0x004F, 0x4F},
        {0x0050, 0x50},
        {0x0051, 0x51},
        {0x0052, 0x52},
        {0x0053, 0x53},
        {0x0054, 0x54},
        {0x0055, 0x55},
        {0x0056, 0x56},
        {0x0057, 0x57},
        {0x0058, 0x58},
        {0x0059, 0x59},
        {0x005A, 0x5A},
        {0x005B, 0x5B},
        {0x005C, 0x5C},
        {0x005D, 0x5D},
        {0x005E, 0x5E},
        {0x005F, 0x5F},
        {0x0060, 0x60},
        {0x0061, 0x61},
        {0x0062, 0x62},
        {0x0063, 0x63},
        {0x0064, 0x64},
        {0x0065, 0x65},
        {0x0066, 0x66},
        {0x0067, 0x67},
        {0x0068, 0x68},
        {0x0069, 0x69},
        {0x006A, 0x6A},
        {0x006B, 0x6B},
        {0x006C, 0x6C},
        {0x006D, 0x6D},
        {0x006E, 0x6E},
        {0x006F, 0x6F},
        {0x0070, 0x70},
        {0x0071, 0x71},
        {0x0072, 0x72},
        {0x0073, 0x73},
        {0x0074, 0x74},
        {0x0075, 0x75},
        {0x0076, 0x76},
        {0x0077, 0x77},
        {0x0078, 0x78},
        {0x0079, 0x79},
        {0x007A, 0x7A},
        {0x007B, 0x7B},
        {0x007C, 0x7C},
        {0x007D, 0x7D},
        {0x007E, 0x7E},
        {0x007F, 0x1C},
        {0x00A0, 0xFF},
        {0x00A1, 0xAD},
        {0x00A2, 0xBD},
        {0x00A3, 0x9C},
        {0x00A4, 0xCF},
        {0x00A5, 0xBE},
        {0x00A6, 0xDD},
        {0x00A7, 0xF5},
        {0x00A8, 0xF9},
        {0x00A9, 0xB8},
        {0x00AA, 0xA6},
        {0x00AB, 0xAE},
        {0x00AC, 0xAA},
        {0x00AD, 0xF0},
        {0x00AE, 0xA9},
        {0x00AF, 0xEE},
        {0x00B0, 0xF8},
        {0x00B1, 0xF1},
        {0x00B2, 0xFD},
        {0x00B3, 0xFC},
        {0x00B4, 0xEF},
        {0x00B5, 0xE6},
        {0x00B6, 0xF4},
        {0x00B7, 0xFA},
        {0x00B8, 0xF7},
        {0x00B9, 0xFB},
        {0x00BA, 0xA7},
        {0x00BB, 0xAF},
        {0x00BC, 0xAC},
        {0x00BD, 0xAB},
        {0x00BE, 0xF3},
        {0x00BF, 0xA8},
        {0x00C0, 0xB7},
        {0x00C1, 0xB5},
        {0x00C2, 0xB6},
        {0x00C3, 0xC7},
        {0x00C4, 0x8E},
        {0x00C5, 0x8F},
        {0x00C6, 0x92},
        {0x00C7, 0x80},
        {0x00C8, 0xD4},
        {0x00C9, 0x90},
        {0x00CA, 0xD2},
        {0x00CB, 0xD3},
        {0x00CC, 0xDE},
        {0x00CD, 0xD6},
        {0x00CE, 0xD7},
        {0x00CF, 0xD8},
        {0x00D0, 0xD1},
        {0x00D1, 0xA5},
        {0x00D2, 0xE3},
        {0x00D3, 0xE0},
        {0x00D4, 0xE2},
        {0x00D5, 0xE5},
        {0x00D6, 0x99},
        {0x00D7, 0x9E},
        {0x00D8, 0x9D},
        {0x00D9, 0xEB},
        {0x00DA, 0xE9},
        {0x00DB, 0xEA},
        {0x00DC, 0x9A},
        {0x00DD, 0xED},
        {0x00DE, 0xE8},
        {0x00DF, 0xE1},
        {0x00E0, 0x85},
        {0x00E1, 0xA0},
        {0x00E2, 0x83},
        {0x00E3, 0xC6},
        {0x00E4, 0x84},
        {0x00E5, 0x86},
        {0x00E6, 0x91},
        {0x00E7, 0x87},
        {0x00E8, 0x8A},
        {0x00E9, 0x82},
        {0x00EA, 0x88},
        {0x00EB, 0x89},
        {0x00EC, 0x8D},
        {0x00ED, 0xA1},
        {0x00EE, 0x8C},
        {0x00EF, 0x8B},
        {0x00F0, 0xD0},
        {0x00F1, 0xA4},
        {0x00F2, 0x95},
        {0x00F3, 0xA2},
        {0x00F4, 0x93},
        {0x00F5, 0xE4},
        {0x00F6, 0x94},
        {0x00F7, 0xF6},
        {0x00F8, 0x9B},
        {0x00F9, 0x97},
        {0x00FA, 0xA3},
        {0x00FB, 0x96},
        {0x00FC, 0x81},
        {0x00FD, 0xEC},
        {0x00FE, 0xE7},
        {0x00FF, 0x98},
        {0x0110, 0xD1},
        {0x0131, 0xD5},
        {0x0192, 0x9F},
        {0x2017, 0xF2},
        {0x2022, 0x07},
        {0x203C, 0x13},
        {0x203E, 0xEE},
        {0x2190, 0x1B},
        {0x2191, 0x18},
        {0x2192, 0x1A},
        {0x2193, 0x19},
        {0x2194, 0x1D},
        {0x2195, 0x12},
        {0x21A8, 0x17},
        {0x221F, 0x1C},
        {0x2302, 0x7F},
        {0x2500, 0xC4},
        {0x2502, 0xB3},
        {0x250C, 0xDA},
        {0x2510, 0xBF},
        {0x2514, 0xC0},
        {0x2518, 0xD9},
        {0x251C, 0xC3},
        {0x2524, 0xB4},
        {0x252C, 0xC2},
        {0x2534, 0xC1},
        {0x253C, 0xC5},
        {0x2550, 0xCD},
        {0x2551, 0xBA},
        {0x2554, 0xC9},
        {0x2557, 0xBB},
        {0x255A, 0xC8},
        {0x255D, 0xBC},
        {0x2560, 0xCC},
        {0x2563, 0xB9},
        {0x2566, 0xCB},
        {0x2569, 0xCA},
        {0x256C, 0xCE},
        {0x2580, 0xDF},
        {0x2584, 0xDC},
        {0x2588, 0xDB},
        {0x2591, 0xB0},
        {0x2592, 0xB1},
        {0x2593, 0xB2},
        {0x25A0, 0xFE},
        {0x25AC, 0x16},
        {0x25B2, 0x1E},
        {0x25BA, 0x10},
        {0x25BC, 0x1F},
        {0x25C4, 0x11},
        {0x25CB, 0x09},
        {0x25D8, 0x08},
        {0x25D9, 0x0A},
        {0x263A, 0x01},
        {0x263B, 0x02},
        {0x263C, 0x0F},
        {0x2640, 0x0C},
        {0x2642, 0x0B},
        {0x2660, 0x06},
        {0x2663, 0x05},
        {0x2665, 0x03},
        {0x2666, 0x04},
        {0x266A, 0x0D},
        {0x266C, 0x0E},
        {0xFF01, 0x21},
        {0xFF02, 0x22},
        {0xFF03, 0x23},
        {0xFF04, 0x24},
        {0xFF05, 0x25},
        {0xFF06, 0x26},
        {0xFF07, 0x27},
        {0xFF08, 0x28},
        {0xFF09, 0x29},
        {0xFF0A, 0x2A},
        {0xFF0B, 0x2B},
        {0xFF0C, 0x2C},
        {0xFF0D, 0x2D},
        {0xFF0E, 0x2E},
        {0xFF0F, 0x2F},
        {0xFF10, 0x30},
        {0xFF11, 0x31},
        {0xFF12, 0x32},
        {0xFF13, 0x33},
        {0xFF14, 0x34},
        {0xFF15, 0x35},
        {0xFF16, 0x36},
        {0xFF17, 0x37},
        {0xFF18, 0x38},
        {0xFF19, 0x39},
        {0xFF1A, 0x3A},
        {0xFF1B, 0x3B},
        {0xFF1C, 0x3C},
        {0xFF1D, 0x3D},
        {0xFF1E, 0x3E},
        {0xFF1F, 0x3F},
        {0xFF20, 0x40},
        {0xFF21, 0x41},
        {0xFF22, 0x42},
        {0xFF23, 0x43},
        {0xFF24, 0x44},
        {0xFF25, 0x45},
        {0xFF26, 0x46},
        {0xFF27, 0x47},
        {0xFF28, 0x48},
        {0xFF29, 0x49},
        {0xFF2A, 0x4A},
        {0xFF2B, 0x4B},
        {0xFF2C, 0x4C},
        {0xFF2D, 0x4D},
        {0xFF2E, 0x4E},
        {0xFF2F, 0x4F},
        {0xFF30, 0x50},
        {0xFF31, 0x51},
        {0xFF32, 0x52},
        {0xFF33, 0x53},
        {0xFF34, 0x54},
        {0xFF35, 0x55},
        {0xFF36, 0x56},
        {0xFF37, 0x57},
        {0xFF38, 0x58},
        {0xFF39, 0x59},
        {0xFF3A, 0x5A},
        {0xFF3B, 0x5B},
        {0xFF3C, 0x5C},
        {0xFF3D, 0x5D},
        {0xFF3E, 0x5E},
        {0xFF3F, 0x5F},
        {0xFF40, 0x60},
        {0xFF41, 0x61},
        {0xFF42, 0x62},
        {0xFF43, 0x63},
        {0xFF44, 0x64},
        {0xFF45, 0x65},
        {0xFF46, 0x66},
        {0xFF47, 0x67},
        {0xFF48, 0x68},
        {0xFF49, 0x69},
        {0xFF4A, 0x6A},
        {0xFF4B, 0x6B},
        {0xFF4C, 0x6C},
        {0xFF4D, 0x6D},
        {0xFF4E, 0x6E},
        {0xFF4F, 0x6F},
        {0xFF50, 0x70},
        {0xFF51, 0x71},
        {0xFF52, 0x72},
        {0xFF53, 0x73},
        {0xFF54, 0x74},
        {0xFF55, 0x75},
        {0xFF56, 0x76},
        {0xFF57, 0x77},
        {0xFF58, 0x78},
        {0xFF59, 0x79},
        {0xFF5A, 0x7A},
        {0xFF5B, 0x7B},
        {0xFF5C, 0x7C},
        {0xFF5D, 0x7D},
        {0xFF5E, 0x7E},
        {0xFFE8, 0xB3},
        {0xFFE9, 0x1B},
        {0xFFEA, 0x18},
        {0xFFEB, 0x1A},
        {0xFFEC, 0x19},
        {0xFFED, 0xFE},
        {0xFFEE, 0x09},
        {-1, -1}
    };

    QTest::addColumn<int>("unicode");
    QTest::addColumn<int>("pccp850");

    for ( int index = 0; table[index].unicode != -1; ++index ) {
        QString name = "0x" + QString::number(table[index].unicode, 16);
        QTest::newRow( name.toLatin1().constData() )
                << table[index].unicode
                << table[index].pccp850;
    }

    memset( seen, 0, sizeof(seen) );
}

void tst_QAtUtils::testPccp850Codec()
{
    QFETCH( int, unicode );
    QFETCH( int, pccp850 );

#define BUG218873(data) \
    QEXPECT_FAIL(#data, "Qt bug 218873: CP850 implementation is incomplete", Abort)

    // These are the unicode characters which cannot be converted from when using
    // Qt's CP850 implementation.
    BUG218873(0x1a);
    BUG218873(0x1c);
    BUG218873(0x7f);
    BUG218873(0x110);
    BUG218873(0x2022);
    BUG218873(0x203c);
    BUG218873(0x203e);
    BUG218873(0x2190);
    BUG218873(0x2191);
    BUG218873(0x2192);
    BUG218873(0x2193);
    BUG218873(0x2194);
    BUG218873(0x2195);
    BUG218873(0x21a8);
    BUG218873(0x221f);
    BUG218873(0x2302);
    BUG218873(0x25ac);
    BUG218873(0x25b2);
    BUG218873(0x25ba);
    BUG218873(0x25bc);
    BUG218873(0x25c4);
    BUG218873(0x25cb);
    BUG218873(0x25d8);
    BUG218873(0x25d9);
    BUG218873(0x263a);
    BUG218873(0x263b);
    BUG218873(0x263c);
    BUG218873(0x2640);
    BUG218873(0x2642);
    BUG218873(0x2660);
    BUG218873(0x2663);
    BUG218873(0x2665);
    BUG218873(0x2666);
    BUG218873(0x266a);
    BUG218873(0x266c);
    BUG218873(0xff01);
    BUG218873(0xff02);
    BUG218873(0xff03);
    BUG218873(0xff04);
    BUG218873(0xff05);
    BUG218873(0xff06);
    BUG218873(0xff07);
    BUG218873(0xff08);
    BUG218873(0xff09);
    BUG218873(0xff0a);
    BUG218873(0xff0b);
    BUG218873(0xff0c);
    BUG218873(0xff0d);
    BUG218873(0xff0e);
    BUG218873(0xff0f);
    BUG218873(0xff10);
    BUG218873(0xff11);
    BUG218873(0xff12);
    BUG218873(0xff13);
    BUG218873(0xff14);
    BUG218873(0xff15);
    BUG218873(0xff16);
    BUG218873(0xff17);
    BUG218873(0xff18);
    BUG218873(0xff19);
    BUG218873(0xff1a);
    BUG218873(0xff1b);
    BUG218873(0xff1c);
    BUG218873(0xff1d);
    BUG218873(0xff1e);
    BUG218873(0xff20);
    BUG218873(0xff21);
    BUG218873(0xff22);
    BUG218873(0xff23);
    BUG218873(0xff24);
    BUG218873(0xff25);
    BUG218873(0xff26);
    BUG218873(0xff27);
    BUG218873(0xff28);
    BUG218873(0xff29);
    BUG218873(0xff2a);
    BUG218873(0xff2b);
    BUG218873(0xff2c);
    BUG218873(0xff2d);
    BUG218873(0xff2e);
    BUG218873(0xff2f);
    BUG218873(0xff30);
    BUG218873(0xff31);
    BUG218873(0xff32);
    BUG218873(0xff33);
    BUG218873(0xff34);
    BUG218873(0xff35);
    BUG218873(0xff36);
    BUG218873(0xff37);
    BUG218873(0xff38);
    BUG218873(0xff39);
    BUG218873(0xff3a);
    BUG218873(0xff3b);
    BUG218873(0xff3c);
    BUG218873(0xff3d);
    BUG218873(0xff3e);
    BUG218873(0xff3f);
    BUG218873(0xff40);
    BUG218873(0xff41);
    BUG218873(0xff42);
    BUG218873(0xff43);
    BUG218873(0xff44);
    BUG218873(0xff45);
    BUG218873(0xff46);
    BUG218873(0xff47);
    BUG218873(0xff48);
    BUG218873(0xff49);
    BUG218873(0xff4a);
    BUG218873(0xff4b);
    BUG218873(0xff4c);
    BUG218873(0xff4d);
    BUG218873(0xff4e);
    BUG218873(0xff4f);
    BUG218873(0xff50);
    BUG218873(0xff51);
    BUG218873(0xff52);
    BUG218873(0xff53);
    BUG218873(0xff54);
    BUG218873(0xff55);
    BUG218873(0xff56);
    BUG218873(0xff57);
    BUG218873(0xff58);
    BUG218873(0xff59);
    BUG218873(0xff5a);
    BUG218873(0xff5b);
    BUG218873(0xff5c);
    BUG218873(0xff5d);
    BUG218873(0xff5e);
    BUG218873(0xffe8);
    BUG218873(0xffe9);
    BUG218873(0xffea);
    BUG218873(0xffeb);
    BUG218873(0xffec);
    BUG218873(0xffed);
    BUG218873(0xffee);

    QTextCodec *codec = QAtUtils::codec( "pccp850" );
    QString str = QChar(unicode);
    QCOMPARE( codec->fromUnicode( str ), QByteArray( 1, (char)pccp850 ) );
    if ( !seen[pccp850] ) {
        seen[pccp850] = true;
        QCOMPARE( codec->toUnicode( QByteArray(1, (char)pccp850 ) ), str );
    }
}

void tst_QAtUtils::testUcs2HexCodec()
{
    QTextCodec *codec = QAtUtils::codec( "ucs2" );
    QCOMPARE( codec->fromUnicode( QString("") ), QByteArray() );
    QCOMPARE( codec->fromUnicode( QString("ABC") ),
              QByteArray( "004100420043" ) );
    QCOMPARE( codec->fromUnicode( QString(QChar(0x1234)) + QChar('\r') ),
              QByteArray( "1234000D" ) );
    QCOMPARE( codec->toUnicode( QByteArray("") ), QString() );
    QCOMPARE( codec->toUnicode( QByteArray("004100420043") ),
              QString("ABC") );
    QCOMPARE( codec->toUnicode( QByteArray("1234000D") ),
              QString(QChar(0x1234)) + QChar('\r') );
    QCOMPARE( codec->toUnicode( QByteArray("  004100420.043 --") ), // bad chars
              QString("ABC") );
    QCOMPARE( codec->toUnicode( QByteArray("00410042004") ), // truncated
              QString("AB") );
}

void tst_QAtUtils::testStripNumber_data()
{
    QTest::addColumn<QString>("number");
    QTest::addColumn<QString>("result");

    QTest::newRow( "empty 1" ) << "" << "";
    QTest::newRow( "empty 2" ) << " -&" << "";
    QTest::newRow( "digits 1" ) << "1194" << "1194";
    QTest::newRow( "digits 2" ) << "1800-123-4567" << "18001234567";
    QTest::newRow( "digits 3" ) << "+1800-123-4567 " << "+18001234567";
    QTest::newRow( "specials 1" ) << "*#ABCDabcd" << "*#ABCDABCD";
    QTest::newRow( "specials 2" ) << ",pPxXwW!@" << ",,,,,WW!@";
}

void tst_QAtUtils::testStripNumber()
{
    QFETCH( QString, number );
    QFETCH( QString, result );

    QCOMPARE( QAtUtils::stripNumber( number ), result );
}

void tst_QAtUtils::testAuxCodecs()
{
    // Test that the codecs that GSM 27.007 specifies are available.
    QVERIFY( QAtUtils::codec( "IRA" ) != 0 );
    QCOMPARE( QAtUtils::codec( "IRA" ),
              QTextCodec::codecForName( "ISO-8859-1" ) );
    QVERIFY( QAtUtils::codec( "GSM" ) != 0 );
    QCOMPARE( QAtUtils::codec( "GSM" ),
              QAtUtils::codec( "gsm" ) );  // check case conversion
    QVERIFY( QAtUtils::codec( "HEX" ) != 0 );
    QVERIFY( QAtUtils::codec( "UCS2" ) != 0 );
    QVERIFY( QAtUtils::codec( "PCDN" ) != 0 );
    QVERIFY( QAtUtils::codec( "UTF-8" ) != 0 );
    QVERIFY( QAtUtils::codec( "8859-1" ) != 0 );
    QVERIFY( QAtUtils::codec( "8859-2" ) != 0 );
    QVERIFY( QAtUtils::codec( "8859-3" ) != 0 );
    QVERIFY( QAtUtils::codec( "8859-4" ) != 0 );
    QVERIFY( QAtUtils::codec( "8859-5" ) != 0 );
    QVERIFY( QAtUtils::codec( "8859-6" ) != 0 );
    QVERIFY( QAtUtils::codec( "8859-C" ) != 0 );
    QVERIFY( QAtUtils::codec( "8859-A" ) != 0 );
    QVERIFY( QAtUtils::codec( "8859-G" ) != 0 );
    QVERIFY( QAtUtils::codec( "8859-H" ) != 0 );

    // Check for code pages that we know some modems require.
    QVERIFY( QAtUtils::codec( "PCCP437" ) != 0 );

    // The specialist codecs should now be registered with QTextCodec.
    QVERIFY( QTextCodec::codecForName( "gsm" ) != 0 );         // GSM
    QVERIFY( QTextCodec::codecForName( "gsm-hex" ) != 0 );     // HEX
    QVERIFY( QTextCodec::codecForName( "ucs2-hex" ) != 0 );    // UCS2
    QVERIFY( QTextCodec::codecForName( "CP850" ) != 0 );       // PCDN
    QVERIFY( QTextCodec::codecForName( "CP437" ) != 0 );       // PCCP437
}

QTEST_MAIN( tst_QAtUtils )

#include "tst_qatutils.moc"
