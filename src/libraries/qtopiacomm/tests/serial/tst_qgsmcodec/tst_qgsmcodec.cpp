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
#include <qgsmcodec.h>
#include <qatutils.h>
#include <QDebug>

//TESTED_CLASS=QGsmCodec
//TESTED_FILES=src/libraries/qtopiacomm/serial/qgsmcodec.cpp

class tst_QGsmCodec : public QObject
{
    Q_OBJECT
private slots:
    void testFromUnicode_data();
    void testFromUnicode();
    void testToUnicode_data();
    void testToUnicode();
    void testAllUnicode();

private:
    static const int Main = 0x01;
    static const int Loss = 0x02;
    static const int Reverse = 0x04;
    static const int Encodable = 0x08;

    void populateData( int types );

    bool encodable[65536];
};

// Input data that defines the contents of the tables in GSM 03.38.
void tst_QGsmCodec::populateData( int types )
{
    struct codepair { int unicode, gsm; };

    // Define the main table for mapping unicode characters to GSM sequences.
    static struct codepair const mainTable[] = {
        {'@',           0x00},
        {0x00A3,        0x01},      // pound sign
        {'$',           0x02},
        {0x00A5,        0x03},      // yen sign
        {0x00E8,        0x04},      // e grave
        {0x00E9,        0x05},      // e acute
        {0x00F9,        0x06},      // u grave
        {0x00EC,        0x07},      // i grave
        {0x00F2,        0x08},      // o grave
        {0x00C7,        0x09},      // C cedilla
        {0x000A,        0x0A},      // line feed
        {0x00D8,        0x0B},      // O stroke
        {0x00F8,        0x0C},      // o stroke
        {0x000D,        0x0D},      // carriage return
        {0x00C5,        0x0E},      // A ring
        {0x00E5,        0x0F},      // a ring
        {0x0394,        0x10},      // capital delta
        {'_',           0x11},
        {0x03A6,        0x12},      // capital phi
        {0x0393,        0x13},      // capital gamma
        {0x039B,        0x14},      // capital lambda
        {0x03A9,        0x15},      // capital omega
        {0x03A0,        0x16},      // capital pi
        {0x03A8,        0x17},      // capital psi
        {0x03A3,        0x18},      // capital sigma
        {0x0398,        0x19},      // capital theta
        {0x039E,        0x1A},      // capital xi
        // 0x1B is an escape for two-byte encodings
        {0x00C6,        0x1C},      // AE
        {0x00E6,        0x1D},      // ae
        {0x00DF,        0x1E},      // sharp s
        {0x00C9,        0x1F},      // e acute
        {' ',           0x20},
        {'!',           0x21},
        {'"',           0x22},
        {'#',           0x23},
        {0x00A4,        0x24},      // international currency symbol
        {'%',           0x25},
        {'&',           0x26},
        {'\'',          0x27},
        {'(',           0x28},
        {')',           0x29},
        {'*',           0x2A},
        {'+',           0x2B},
        {',',           0x2C},
        {'-',           0x2D},
        {'.',           0x2E},
        {'/',           0x2F},
        {'0',           0x30},
        {'1',           0x31},
        {'2',           0x32},
        {'3',           0x33},
        {'4',           0x34},
        {'5',           0x35},
        {'6',           0x36},
        {'7',           0x37},
        {'8',           0x38},
        {'9',           0x39},
        {':',           0x3A},
        {';',           0x3B},
        {'<',           0x3C},
        {'=',           0x3D},
        {'>',           0x3E},
        {'?',           0x3F},
        {0x00A1,        0x40},      // inverted exclaimation mark
        {'A',           0x41},
        {'B',           0x42},
        {'C',           0x43},
        {'D',           0x44},
        {'E',           0x45},
        {'F',           0x46},
        {'G',           0x47},
        {'H',           0x48},
        {'I',           0x49},
        {'J',           0x4A},
        {'K',           0x4B},
        {'L',           0x4C},
        {'M',           0x4D},
        {'N',           0x4E},
        {'O',           0x4F},
        {'P',           0x50},
        {'Q',           0x51},
        {'R',           0x52},
        {'S',           0x53},
        {'T',           0x54},
        {'U',           0x55},
        {'V',           0x56},
        {'W',           0x57},
        {'X',           0x58},
        {'Y',           0x59},
        {'Z',           0x5A},
        {0x00C4,        0x5B},      // A diaeresis
        {0x00D6,        0x5C},      // O diaeresis
        {0x00D1,        0x5D},      // N tilde
        {0x00DC,        0x5E},      // U diaeresis
        {0x00A7,        0x5F},      // section sign
        {0x00BF,        0x60},      // inverted question mark
        {'a',           0x61},
        {'b',           0x62},
        {'c',           0x63},
        {'d',           0x64},
        {'e',           0x65},
        {'f',           0x66},
        {'g',           0x67},
        {'h',           0x68},
        {'i',           0x69},
        {'j',           0x6A},
        {'k',           0x6B},
        {'l',           0x6C},
        {'m',           0x6D},
        {'n',           0x6E},
        {'o',           0x6F},
        {'p',           0x70},
        {'q',           0x71},
        {'r',           0x72},
        {'s',           0x73},
        {'t',           0x74},
        {'u',           0x75},
        {'v',           0x76},
        {'w',           0x77},
        {'x',           0x78},
        {'y',           0x79},
        {'z',           0x7A},
        {0x00E4,        0x7B},      // a diaeresis
        {0x00F6,        0x7C},      // o diaeresis
        {0x00F1,        0x7D},      // n tilde
        {0x00FC,        0x7E},      // u diaeresis
        {0x00E0,        0x7F},      // a grave
        {'^',           0x1B14},
        {'{',           0x1B28},
        {'}',           0x1B29},
        {'\\',          0x1B2F},
        {'[',           0x1B3C},
        {'~',           0x1B3D},
        {']',           0x1B3E},
        {'|',           0x1B40},
        {0x20AC,        0x1B65},    // euro sign
        {-1,            -1}
    };

    // Mappings that cause loss of information.  From GSM 27.005.
    static struct codepair const lossTable[] = {
        {0x00C0,        'A'},       // A grave
        {0x00C1,        'A'},       // A acute
        {0x00C2,        'A'},       // A circumflex
        {0x00C3,        'A'},       // A tilde
        {0x00C8,        'E'},       // E grave
        {0x00CA,        'E'},       // E circumflex
        {0x00CB,        'E'},       // E diaeresis
        {0x00CC,        'I'},       // I grave
        {0x00CD,        'I'},       // I acute
        {0x00CE,        'I'},       // I circumflex
        {0x00CF,        'I'},       // I diaeresis
        {0x00D2,        'O'},       // O grave
        {0x00D3,        'O'},       // O acute
        {0x00D4,        'O'},       // O circumflex
        {0x00D5,        'O'},       // O tilde
        {0x00D9,        'U'},       // U grave
        {0x00DA,        'U'},       // U acute
        {0x00DB,        'U'},       // U circumflex
        {0x00DD,        'Y'},       // Y acute
        {0x00E1,        'a'},       // a acute
        {0x00E2,        'a'},       // a circumflex
        {0x00E3,        'a'},       // a tilde
        {0x00E7,        0x09},      // c cedilla
        {0x00EA,        'e'},       // e circumflex
        {0x00EB,        'e'},       // e diaeresis
        {0x00ED,        'i'},       // i acute
        {0x00EE,        'i'},       // i circumflex
        {0x00EF,        'i'},       // i diaeresis
        {0x00F3,        'o'},       // o acute
        {0x00F4,        'o'},       // o circumflex
        {0x00F5,        'o'},       // o tilde
        {0x00FA,        'u'},       // u acute
        {0x00FB,        'u'},       // u circumflex
        {0x00FD,        'y'},       // y acute
        {0x00FF,        'y'},       // y diaresis
        {-1,            -1}
    };

    // Define the reverse mappings for codepoints that are not assigned.
    static struct codepair const reverseTable[] = {
        {'@',           0x1B00},
        {0x00A3,        0x1B01},      // pound sign
        {'$',           0x1B02},
        {0x00A5,        0x1B03},      // yen sign
        {0x00E8,        0x1B04},      // e grave
        {0x00E9,        0x1B05},      // e acute
        {0x00F9,        0x1B06},      // u grave
        {0x00EC,        0x1B07},      // i grave
        {0x00F2,        0x1B08},      // o grave
        {0x00C7,        0x1B09},      // C cedilla
        {0x000A,        0x1B0A},      // line feed
        {0x00D8,        0x1B0B},      // O stroke
        {0x00F8,        0x1B0C},      // o stroke
        {0x000D,        0x1B0D},      // carriage return
        {0x00C5,        0x1B0E},      // A ring
        {0x00E5,        0x1B0F},      // a ring
        {0x0394,        0x1B10},      // capital delta
        {'_',           0x1B11},
        {0x03A6,        0x1B12},      // capital phi
        {0x0393,        0x1B13},      // capital gamma
        // 0x1B14 is normal
        {0x03A9,        0x1B15},      // capital omega
        {0x03A0,        0x1B16},      // capital pi
        {0x03A8,        0x1B17},      // capital psi
        {0x03A3,        0x1B18},      // capital sigma
        {0x0398,        0x1B19},      // capital theta
        {0x039E,        0x1B1A},      // capital xi
        {' ',           0x1B1B},    // double extension
        {0x00C6,        0x1B1C},      // AE
        {0x00E6,        0x1B1D},      // ae
        {0x00DF,        0x1B1E},      // sharp s
        {0x00C9,        0x1B1F},      // e acute
        {' ',           0x1B20},
        {'!',           0x1B21},
        {'"',           0x1B22},
        {'#',           0x1B23},
        {0x00A4,        0x1B24},      // international currency symbol
        {'%',           0x1B25},
        {'&',           0x1B26},
        {'\'',          0x1B27},
        // 0x1B28 is normal
        // 0x1B29 is normal
        {'*',           0x1B2A},
        {'+',           0x1B2B},
        {',',           0x1B2C},
        {'-',           0x1B2D},
        {'.',           0x1B2E},
        // 0x1B2F is normal
        {'0',           0x1B30},
        {'1',           0x1B31},
        {'2',           0x1B32},
        {'3',           0x1B33},
        {'4',           0x1B34},
        {'5',           0x1B35},
        {'6',           0x1B36},
        {'7',           0x1B37},
        {'8',           0x1B38},
        {'9',           0x1B39},
        {':',           0x1B3A},
        {';',           0x1B3B},
        // 0x1B3C is normal
        // 0x1B3D is normal
        // 0x1B3E is normal
        {'?',           0x1B3F},
        // 0x1B40 is normal
        {'A',           0x1B41},
        {'B',           0x1B42},
        {'C',           0x1B43},
        {'D',           0x1B44},
        {'E',           0x1B45},
        {'F',           0x1B46},
        {'G',           0x1B47},
        {'H',           0x1B48},
        {'I',           0x1B49},
        {'J',           0x1B4A},
        {'K',           0x1B4B},
        {'L',           0x1B4C},
        {'M',           0x1B4D},
        {'N',           0x1B4E},
        {'O',           0x1B4F},
        {'P',           0x1B50},
        {'Q',           0x1B51},
        {'R',           0x1B52},
        {'S',           0x1B53},
        {'T',           0x1B54},
        {'U',           0x1B55},
        {'V',           0x1B56},
        {'W',           0x1B57},
        {'X',           0x1B58},
        {'Y',           0x1B59},
        {'Z',           0x1B5A},
        {0x00C4,        0x1B5B},      // A diaeresis
        {0x00D6,        0x1B5C},      // O diaeresis
        {0x00D1,        0x1B5D},      // N tilde
        {0x00DC,        0x1B5E},      // U diaeresis
        {0x00A7,        0x1B5F},      // section sign
        {0x00BF,        0x1B60},      // inverted question mark
        {'a',           0x1B61},
        {'b',           0x1B62},
        {'c',           0x1B63},
        {'d',           0x1B64},
        // 0x1B65 is normal
        {'f',           0x1B66},
        {'g',           0x1B67},
        {'h',           0x1B68},
        {'i',           0x1B69},
        {'j',           0x1B6A},
        {'k',           0x1B6B},
        {'l',           0x1B6C},
        {'m',           0x1B6D},
        {'n',           0x1B6E},
        {'o',           0x1B6F},
        {'p',           0x1B70},
        {'q',           0x1B71},
        {'r',           0x1B72},
        {'s',           0x1B73},
        {'t',           0x1B74},
        {'u',           0x1B75},
        {'v',           0x1B76},
        {'w',           0x1B77},
        {'x',           0x1B78},
        {'y',           0x1B79},
        {'z',           0x1B7A},
        {0x00E4,        0x1B7B},      // a diaeresis
        {0x00F6,        0x1B7C},      // o diaeresis
        {0x00F1,        0x1B7D},      // n tilde
        {0x00FC,        0x1B7E},      // u diaeresis
        {0x00E0,        0x1B7F},      // a grave
        {-1,            -1}
    };

    if ( ( types & Encodable ) == 0 ) {
        QTest::addColumn<int>("unicode");
        QTest::addColumn<int>("gsm");
        QTest::addColumn<int>("type");
    }

    int index;
    QString name;

    if ( ( types & Main ) != 0 ) {
        for ( index = 0; mainTable[index].unicode != -1; ++index ) {
            name = "0x" + QString::number(mainTable[index].unicode, 16);
            QTest::newRow( name.toLatin1().constData() )
                    << mainTable[index].unicode
                    << mainTable[index].gsm
                    << (int)Main;
        }
    }

    if ( ( types & Loss ) != 0 ) {
        for ( index = 0; lossTable[index].unicode != -1; ++index ) {
            name = "0x" + QString::number(lossTable[index].unicode, 16);
            QTest::newRow( name.toLatin1().constData() )
                    << lossTable[index].unicode
                    << lossTable[index].gsm
                    << (int)Loss;
        }
    }

    if ( ( types & Reverse ) != 0 ) {
        for ( index = 0; reverseTable[index].unicode != -1; ++index ) {
            name = "0x" + QString::number(reverseTable[index].unicode, 16);
            QTest::newRow( name.toLatin1().constData() )
                    << reverseTable[index].unicode
                    << reverseTable[index].gsm
                    << (int)Reverse;
        }
    }

    if ( ( types & Encodable ) != 0 ) {
        memset( encodable, 0, sizeof(encodable) );
        for ( index = 0; mainTable[index].unicode != -1; ++index ) {
            encodable[mainTable[index].unicode] = true;
        }
    }
}

// Test forward conversion tables.
void tst_QGsmCodec::testFromUnicode_data()
{
    populateData( Main | Loss );
}
void tst_QGsmCodec::testFromUnicode()
{
    QFETCH( int, unicode );
    QFETCH( int, gsm );
    QFETCH( int, type );

    // Test the direct conversion API.
    QCOMPARE( (int)QGsmCodec::twoByteFromUnicode( QChar(unicode) ), gsm );

    // Old API: only works for single-byte encodings, so ignore two-byte.
    if ( gsm < 256 ) {
        QCOMPARE( ((int)QGsmCodec::singleFromUnicode( QChar(unicode) )) & 0xFF,
                  gsm );
    }

    // Test the QTextCodec-based conversion API.
    QString str = QChar(unicode);
    QTextCodec *codec = QAtUtils::codec( "gsm" );
    if ( gsm < 256 ) {
        QCOMPARE( codec->fromUnicode( str ), QByteArray(1, (char)gsm) );
    } else {
        QCOMPARE( codec->fromUnicode( str ),
                  QByteArray(1, (char)(gsm >> 8)) + QByteArray(1, (char)gsm) );
    }

    // Test the no-loss QTextCodec-based conversion API.
    codec = QAtUtils::codec( "gsm-noloss" );
    if ( type == Loss ) {
        // The string should always convert into the GUC character, 0x10.
        QCOMPARE( codec->fromUnicode( str ), QByteArray(1, (char)0x10) );
    } else {
        if ( gsm < 256 ) {
            QCOMPARE( codec->fromUnicode( str ), QByteArray(1, (char)gsm) );
        } else {
            QCOMPARE( codec->fromUnicode( str ),
                      QByteArray(1, (char)(gsm >> 8)) +
                      QByteArray(1, (char)gsm) );
        }
    }

    // Use the buffer version and check reporting of the error state.
    QTextCodec::ConverterState state;
    QChar buf = QChar(unicode);
    if ( type == Loss ) {
        // The string should always convert into the GUC character, 0x10.
        QCOMPARE( codec->fromUnicode( &buf, 1, &state ),
                  QByteArray(1, (char)0x10) );
        QCOMPARE( state.invalidChars, 1 );
    } else {
        if ( gsm < 256 ) {
            QCOMPARE( codec->fromUnicode( &buf, 1, &state ),
                      QByteArray(1, (char)gsm) );
        } else {
            QCOMPARE( codec->fromUnicode( &buf, 1, &state ),
                      QByteArray(1, (char)(gsm >> 8)) +
                      QByteArray(1, (char)gsm) );
        }
        if ( state.invalidChars != 0 )
            qDebug() << unicode << gsm;
        QCOMPARE( state.invalidChars, 0 );
    }

    // Test the hex codec, which is the same as "gsm", with hex encoding.
    str = QChar(unicode);
    codec = QAtUtils::codec( "hex" );
    if ( gsm < 256 ) {
        QCOMPARE( codec->fromUnicode( str ),
                  QAtUtils::toHex( QByteArray(1, (char)gsm) ).toLatin1() );
    } else {
        QCOMPARE( codec->fromUnicode( str ),
                  QAtUtils::toHex
                        ( QByteArray(1, (char)(gsm >> 8)) +
                          QByteArray(1, (char)gsm) ).toLatin1() );
    }

}

// Test backward conversion tables.
void tst_QGsmCodec::testToUnicode_data()
{
    populateData( Main | Reverse );
}
void tst_QGsmCodec::testToUnicode()
{
    QFETCH( int, unicode );
    QFETCH( int, gsm );
    QFETCH( int, type );

    // Test the direct conversion API.
    QCOMPARE( (int)QGsmCodec::twoByteToUnicode( (unsigned short)gsm ).unicode(),
              unicode );

    // Old API: only works for single-byte encodings, so ignore two-byte.
    if ( gsm < 256 ) {
        QCOMPARE( (int)QGsmCodec::singleToUnicode( (char)gsm ).unicode(),
                  unicode );
    }

    // Test the QTextCodec-based conversion API.
    QString str = QChar(unicode);
    QTextCodec *codec = QAtUtils::codec( "gsm" );
    if ( gsm < 256 ) {
        QCOMPARE( codec->toUnicode( QByteArray(1, (char)gsm) ),
                  QString( QChar(unicode) ) );
    } else {
        QCOMPARE( codec->toUnicode( QByteArray(1, (char)(gsm >> 8)) +
                                    QByteArray(1, (char)gsm) ),
                  QString( QChar(unicode) ) );
    }

    // Use the buffer version and check reporting of the error state.
    QTextCodec::ConverterState state;
    char buf[2];
    if ( gsm < 256 ) {
        buf[0] = (char)gsm;
        QCOMPARE( codec->toUnicode( buf, 1, &state ),
                  QString( QChar(unicode) ) );
    } else {
        buf[0] = (char)(gsm >> 8);
        buf[1] = (char)gsm;
        QCOMPARE( codec->toUnicode( buf, 2, &state ),
                  QString( QChar(unicode) ) );
    }
    if ( type != Main && gsm != 0x1B1B ) {
        QCOMPARE( state.invalidChars, 1 );
    } else {
        QCOMPARE( state.invalidChars, 0 );
    }

    // Test the hex codec, which is the same as "gsm", with hex encoding.
    str = QChar(unicode);
    codec = QAtUtils::codec( "hex" );
    if ( gsm < 256 ) {
        QCOMPARE( codec->toUnicode
                    ( QAtUtils::toHex( QByteArray(1, (char)gsm) ).toLatin1() ),
                  QString( QChar(unicode) ) );
    } else {
        QCOMPARE( codec->toUnicode
                    ( QAtUtils::toHex
                        ( QByteArray(1, (char)(gsm >> 8)) +
                          QByteArray(1, (char)gsm) ).toLatin1() ),
                  QString( QChar(unicode) ) );
    }
}

// Run tests on all Unicode code points to make sure things that are
// valid are recognized as valid and things that are invalid are
// recognized as invalid.
void tst_QGsmCodec::testAllUnicode()
{
    populateData( Encodable );

    QTextCodec *codec = QAtUtils::codec( "gsm-noloss" );

    bool ok = true;
    for ( int unicode = 0; unicode < 65536; ++unicode ) {
        QChar buf = QChar(unicode);
        QTextCodec::ConverterState state;
        codec->fromUnicode( &buf, 1, &state );
        if ( encodable[unicode] ) {
            if ( state.invalidChars != 0 ) {
                qDebug() << "0x" + QString::number( unicode, 16 )
                         << "should be encodable but isn't";
                ok = false;
            }
        } else {
            if ( state.invalidChars != 1 ) {
                qDebug() << "0x" + QString::number( unicode, 16 )
                         << "should not be encodable but is";
                ok = false;
            }
        }
    }

    QVERIFY(ok);
}

QTEST_MAIN( tst_QGsmCodec )

#include "tst_qgsmcodec.moc"
