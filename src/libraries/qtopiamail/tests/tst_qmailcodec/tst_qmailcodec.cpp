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

// To detail string differences on comparison failure:
#include "shared/string_difference.h"

#include <QtopiaApplication>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QMailCodec>
#include <QTextCodec>


//TESTED_CLASS=QMailCodec
//TESTED_FILES=src/libraries/qtopiamail/qmailcodec.cpp

/*
    This class primarily tests that QMailBase64Codec correctly handles encoding/decoding
*/
class tst_QMailCodec : public QObject
{
    Q_OBJECT

public:
    tst_QMailCodec();
    virtual ~tst_QMailCodec();

private slots:
    virtual void initTestCase();
    virtual void cleanupTestCase();
    virtual void init();
    virtual void cleanup();

    void encode_data();
    void encode();
    void decode_data();
    void decode();
    void line_lengths_data();
    void line_lengths();
    void buffer_sizes_data();
    void buffer_sizes();
    void embedded_newlines_data();
    void embedded_newlines();
};

QTEST_APP_MAIN( tst_QMailCodec, QtopiaApplication )

#include "tst_qmailcodec.moc"


tst_QMailCodec::tst_QMailCodec()
{
}

tst_QMailCodec::~tst_QMailCodec()
{
}

void tst_QMailCodec::initTestCase()
{
}

void tst_QMailCodec::cleanupTestCase()
{
}

void tst_QMailCodec::init()
{
}

/*?
    Cleanup after each test case.
*/
void tst_QMailCodec::cleanup()
{
}

void tst_QMailCodec::encode_data()
{
    QTest::addColumn<QString>("plaintext");
    QTest::addColumn<QString>("charset");
    QTest::addColumn<QByteArray>("base64_encoded");
    QTest::addColumn<QByteArray>("qp2045_encoded");
    QTest::addColumn<QByteArray>("qp2047_encoded");

    QTest::newRow("no padding") 
        << "abc xyz ABC XYZ 01 89"
        << "UTF-8"
        << QByteArray("YWJjIHh5eiBBQkMgWFlaIDAxIDg5")
        << QByteArray("abc xyz ABC XYZ 01 89")
        << QByteArray("abc_xyz_ABC_XYZ_01_89");

    QTest::newRow("one padding byte") 
        << "|#abc xyz ABC XYZ 01 89"
        << "UTF-8"
        << QByteArray("fCNhYmMgeHl6IEFCQyBYWVogMDEgODk=")
        << QByteArray("|#abc xyz ABC XYZ 01 89")
        << QByteArray("=7C=23abc_xyz_ABC_XYZ_01_89");

    QTest::newRow("two padding bytes") 
        << "#abc xyz ABC XYZ 01 89"
        << "UTF-8"
        << QByteArray("I2FiYyB4eXogQUJDIFhZWiAwMSA4OQ==")
        << QByteArray("#abc xyz ABC XYZ 01 89")
        << QByteArray("=23abc_xyz_ABC_XYZ_01_89");

    // Test with some arabic characters, as per http://en.wikipedia.org/wiki/List_of_Unicode_characters
    const QChar chars[] = { 0x0636, 0x0020, 0x0669, 0x0009, 0x06a5, 0x0020, 0x06b4 };
    QTest::newRow("unicode characters") 
        << QString(chars, 7)
        << "UTF-8"
        << QByteArray("2LYg2akJ2qUg2rQ=")
        << QByteArray("=D8=B6 =D9=A9\t=DA=A5 =DA=B4")
        << QByteArray("=D8=B6_=D9=A9=09=DA=A5_=DA=B4");
}

void tst_QMailCodec::encode()
{
    QFETCH(QString, plaintext); 
    QFETCH(QString, charset); 
    QFETCH(QByteArray, base64_encoded);
    QFETCH(QByteArray, qp2045_encoded);
    QFETCH(QByteArray, qp2047_encoded);

    QByteArray encoded;
    QString reversed;

    QTextCodec* codec = QTextCodec::codecForName(charset.toLatin1());

    // Test that the base64 encoding is correct
    {
        QMailBase64Codec b64Codec(QMailBase64Codec::Binary);
        encoded = b64Codec.encode(plaintext, charset);
    }
    QCOMPARE(encoded, base64_encoded);

    // Test that the reverse base64 decoding is correct
    {
        QMailBase64Codec b64Codec(QMailBase64Codec::Binary);
        reversed = b64Codec.decode(encoded, charset);
    }
    QCOMPARE(reversed, plaintext);

    // Ensure that the byte-array-to-byte-array conversion matches the QString-to-byte-array conversion
    if (codec)
    {
        QByteArray octet_data = codec->fromUnicode(plaintext);
        QByteArray base64_data;
        {
            QDataStream octet_stream(&octet_data, QIODevice::ReadOnly);
            QDataStream base64_stream(&base64_data, QIODevice::WriteOnly);

            QMailBase64Codec b64Codec(QMailBase64Codec::Binary);
            b64Codec.encode(base64_stream, octet_stream);
        }
        QCOMPARE(base64_data, base64_encoded);

        QByteArray reversed;
        {
            QDataStream base64_stream(&base64_data, QIODevice::ReadOnly);
            QDataStream reversed_stream(&reversed, QIODevice::WriteOnly);

            QMailBase64Codec b64Codec(QMailBase64Codec::Binary);
            b64Codec.decode(reversed_stream, base64_stream);
        }
        QCOMPARE(reversed, octet_data);
    }

    // Test the the quoted-printable encoding works, conforming to RFC 2045
    {
        QMailQuotedPrintableCodec qpCodec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2045);
        encoded = qpCodec.encode(plaintext, charset);
    }
    QCOMPARE(encoded, qp2045_encoded);

    {
        QMailQuotedPrintableCodec qpCodec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2045);
        reversed = qpCodec.decode(encoded, charset);
    }
    QCOMPARE(reversed, plaintext);

    if (codec)
    {
        // Ensure that the byte-array-to-byte-array conversion matches
        QByteArray octet_data = codec->fromUnicode(plaintext);
        QByteArray qp_data;
        {
            QDataStream octet_stream(&octet_data, QIODevice::ReadOnly);
            QDataStream qp_stream(&qp_data, QIODevice::WriteOnly);

            QMailQuotedPrintableCodec qpCodec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2045);
            qpCodec.encode(qp_stream, octet_stream);
        }
        QCOMPARE(qp_data, qp2045_encoded);

        QByteArray reversed;
        {
            QDataStream qp_stream(&qp_data, QIODevice::ReadOnly);
            QDataStream reversed_stream(&reversed, QIODevice::WriteOnly);

            QMailQuotedPrintableCodec qpCodec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2045);
            qpCodec.decode(reversed_stream, qp_stream);
        }
        QCOMPARE(reversed, octet_data);
    }

    if (!qp2047_encoded.isEmpty())
    {
        // Test the the quoted-printable encoding works, conforming to RFC 2047
        {
            QMailQuotedPrintableCodec qpCodec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2047);
            encoded = qpCodec.encode(plaintext, charset);
        }
        QCOMPARE(encoded, qp2047_encoded);

        {
            QMailQuotedPrintableCodec qpCodec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2047);
            reversed = qpCodec.decode(encoded, charset);
        }
        QCOMPARE(reversed, plaintext);
    }

    // Test that the pass-through codec maintains the existing data
    {
        QMailPassThroughCodec ptCodec;
        encoded = ptCodec.encode(plaintext, charset);
    }

    if (codec)
    {
        QByteArray octet_data = codec->fromUnicode(plaintext);
        QCOMPARE(encoded, octet_data);
    }

    {
        QMailPassThroughCodec ptCodec;
        reversed = ptCodec.decode(encoded, charset);
    }
    QCOMPARE(reversed, plaintext);
}

void tst_QMailCodec::decode_data()
{
    /*
    // I can't find any way to create printable strings to test here...
    QTest::addColumn<QByteArray>("base64_encoded");
    QTest::addColumn<QString>("charset");
    QTest::addColumn<QString>("plaintext");

    QTest::newRow("") 
        << QByteArray("...")
        << "UTF-8"
        << QString("...");
    */
}

void tst_QMailCodec::decode()
{
    /*
    QFETCH(QByteArray, base64_encoded); 
    QFETCH(QString, charset); 
    QFETCH(QString, plaintext); 

    QMailBase64Codec b64Codec(QMailBase64Codec::Binary);
    QString decoded = b64Codec.decode(base64_encoded, charset);
    QCOMPARE(decoded, plaintext);

    QByteArray reversed = b64Codec.encode(decoded, charset);
    QCOMPARE(reversed, base64_encoded);
    */
}

extern int Base64MaxLineLength;
extern int QuotedPrintableMaxLineLength;

void tst_QMailCodec::line_lengths_data()
{
    QTest::addColumn<int>("base64_line_length");
    QTest::addColumn<int>("qp_line_length");
    QTest::addColumn<QString>("plaintext");
    QTest::addColumn<QString>("charset");
    QTest::addColumn<QByteArray>("base64_encoded");
    QTest::addColumn<QByteArray>("qp2045_encoded");
    QTest::addColumn<QByteArray>("qp2047_encoded");

    // Base-64 lengths must be a multiple of 4...
    QTest::newRow("default line length") 
        << Base64MaxLineLength 
        << QuotedPrintableMaxLineLength
        << "The quick brown fox jumps over the lazy dog"
        << "UTF-8"
        << QByteArray("VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZw==")
        << QByteArray("The quick brown fox jumps over the lazy dog")
        << QByteArray("The_quick_brown_fox_jumps_over_the_lazy_dog");

    QTest::newRow("line length 32") 
        << 32
        << 32
        << "The quick brown fox jumps over the lazy dog"
        << "UTF-8"
        << QByteArray("VGhlIHF1aWNrIGJyb3duIGZveCBqdW1w\r\ncyBvdmVyIHRoZSBsYXp5IGRvZw==")
        << QByteArray("The quick brown fox jumps over=\n the lazy dog")
        << QByteArray("The_quick_brown_fox_jumps_over=\n_the_lazy_dog");

    QTest::newRow("line length 16") 
        << 16
        << 16
        << "The quick brown fox jumps over the lazy dog"
        << "UTF-8"
        << QByteArray("VGhlIHF1aWNrIGJy\r\nb3duIGZveCBqdW1w\r\ncyBvdmVyIHRoZSBs\r\nYXp5IGRvZw==")
        << QByteArray("The quick brown=\n fox jumps over=\n the lazy dog")
        << QByteArray("The_quick_brown=\n_fox_jumps_over=\n_the_lazy_dog");

    QTest::newRow("line length 8") 
        << 8
        << 8
        << "The quick brown fox jumps over the lazy dog"
        << "UTF-8"
        << QByteArray("VGhlIHF1\r\naWNrIGJy\r\nb3duIGZv\r\neCBqdW1w\r\ncyBvdmVy\r\nIHRoZSBs\r\nYXp5IGRv\r\nZw==")
        << QByteArray("The quic=\nk brown=\n fox jum=\nps over=\n the laz=\ny dog")
        << QByteArray("The_quic=\nk_brown=\n_fox_jum=\nps_over=\n_the_laz=\ny_dog");

    QTest::newRow("whitespace") 
        << 8
        << 8
        << "The    quick\t\t  brown\t     \tfox"
        << "UTF-8"
        << QByteArray("VGhlICAg\r\nIHF1aWNr\r\nCQkgIGJy\r\nb3duCSAg\r\nICAgCWZv\r\neA==")
        << QByteArray("The  =20=\n quick=\n\t\t  brow=\nn\t   =20=\n \tfox")
        << QByteArray("The__=20=\n_quick=\n=09=09=\n__brown=\n=09__=20=\n__=09fox");

    // Restore normality
    QTest::newRow("restore default line length") 
        << Base64MaxLineLength
        << QuotedPrintableMaxLineLength
        << QString()
        << "UTF-8"
        << QByteArray()
        << QByteArray()
        << QByteArray();
}

void tst_QMailCodec::line_lengths()
{
    QFETCH(int, base64_line_length); 
    QFETCH(int, qp_line_length); 
    QFETCH(QString, plaintext); 
    QFETCH(QString, charset); 
    QFETCH(QByteArray, base64_encoded);
    QFETCH(QByteArray, qp2045_encoded);
    QFETCH(QByteArray, qp2047_encoded);

    QByteArray encoded;
    QString reversed;

    {
        // First, test by specifying the line length
        {
            QMailBase64Codec codec(QMailBase64Codec::Binary, base64_line_length);
            encoded = codec.encode(plaintext, charset);
            QCOMPARE(encoded, base64_encoded);
        }
        {
            QMailBase64Codec codec(QMailBase64Codec::Binary, base64_line_length);
            reversed = codec.decode(encoded, charset);
            QCOMPARE(reversed, plaintext);
        }

        {
            QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2045, qp_line_length);
            encoded = codec.encode(plaintext, charset);
            QCOMPARE(encoded, qp2045_encoded);
        }
        {
            QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2045, qp_line_length);
            reversed = codec.decode(encoded, charset);
            QCOMPARE(reversed, plaintext);
        }

        if (!qp2047_encoded.isEmpty())
        {
            {
                QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2047, qp_line_length);
                encoded = codec.encode(plaintext, charset);
                QCOMPARE(encoded, qp2047_encoded);
            }
            {
                QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2047, qp_line_length);
                reversed = codec.decode(encoded, charset);
                QCOMPARE(reversed, plaintext);
            }
        }
    }

    // Second, test with default line length overridden
    Base64MaxLineLength = base64_line_length;
    QuotedPrintableMaxLineLength = qp_line_length;

    {
        {
            QMailBase64Codec codec(QMailBase64Codec::Binary);
            encoded = codec.encode(plaintext, charset);
            QCOMPARE(encoded, base64_encoded);
        }
        {
            QMailBase64Codec codec(QMailBase64Codec::Binary);
            reversed = codec.decode(encoded, charset);
            QCOMPARE(reversed, plaintext);
        }

        {
            QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2045);
            encoded = codec.encode(plaintext, charset);
            QCOMPARE(encoded, qp2045_encoded);
        }
        {
            QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2045);
            reversed = codec.decode(encoded, charset);
            QCOMPARE(reversed, plaintext);
        }

        if (!qp2047_encoded.isEmpty())
        {
            {
                QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2047);
                encoded = codec.encode(plaintext, charset);
                QCOMPARE(encoded, qp2047_encoded);
            }
            {
                QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2047);
                reversed = codec.decode(encoded, charset);
                QCOMPARE(reversed, plaintext);
            }
        }
    }
}

extern int MaxCharacters;

void tst_QMailCodec::buffer_sizes_data()
{
    QTest::addColumn<int>("buffer_size");

    QTest::newRow("default buffer size") 
        << MaxCharacters;
        
    QTest::newRow("buffer size 19") 
        << 19;

    QTest::newRow("buffer size 5") 
        << 5;

    QTest::newRow("buffer size 1") 
        << 1;

    // Restore normality
    QTest::newRow("restore default buffer size") 
        << MaxCharacters;
}

void tst_QMailCodec::buffer_sizes()
{
    QFETCH(int, buffer_size);

    int originalMaxCharacters = MaxCharacters;
    int originalBase64MaxLineLength = Base64MaxLineLength;
    int originalQuotedPrintableMaxLineLength = QuotedPrintableMaxLineLength;

    MaxCharacters = buffer_size;
    Base64MaxLineLength = 8;
    QuotedPrintableMaxLineLength = 8;

    QString plaintext("The    quick\t\t  brown\t     \tfox");
    QString charset("UTF-8");
    QByteArray base64_encoded("VGhlICAg\r\nIHF1aWNr\r\nCQkgIGJy\r\nb3duCSAg\r\nICAgCWZv\r\neA==");
    QByteArray qp2045_encoded("The  =20=\n quick=\n\t\t  brow=\nn\t   =20=\n \tfox");
    QByteArray qp2047_encoded("The__=20=\n_quick=\n=09=09=\n__brown=\n=09__=20=\n__=09fox");

    QByteArray encoded;
    QString reversed;

    {
        QMailBase64Codec codec(QMailBase64Codec::Binary);
        encoded = codec.encode(plaintext, charset);
        QCOMPARE(encoded, base64_encoded);
    }
    {
        QMailBase64Codec codec(QMailBase64Codec::Binary);
        reversed = codec.decode(encoded, charset);
        QCOMPARE(reversed, plaintext);
    }
    {
        QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2045);
        encoded = codec.encode(plaintext, charset);
        QCOMPARE(encoded, qp2045_encoded);
    }
    {
        QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2045);
        reversed = codec.decode(encoded, charset);
        QCOMPARE(reversed, plaintext);
    }
    {
        QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2047);
        encoded = codec.encode(plaintext, charset);
        QCOMPARE(encoded, qp2047_encoded);
    }
    {
        QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2047);
        reversed = codec.decode(encoded, charset);
        QCOMPARE(reversed, plaintext);
    }

    MaxCharacters = originalMaxCharacters;
    Base64MaxLineLength = originalBase64MaxLineLength;
    QuotedPrintableMaxLineLength = originalQuotedPrintableMaxLineLength;
}

void tst_QMailCodec::embedded_newlines_data()
{
    QTest::addColumn<QString>("plaintext");
    QTest::addColumn<QString>("charset");
    QTest::addColumn<QByteArray>("text_encoded");
    QTest::addColumn<QString>("text_decoded");
    QTest::addColumn<QByteArray>("binary_encoded");
    QTest::addColumn<QString>("binary_decoded");
    QTest::addColumn<QByteArray>("b64_encoded");

    // In these test cases we use the following sequences:
    //   CR - 0x0D - \015
    //   LF - 0x0A - \012
    QTest::newRow("new lines") 
        << "The\012quick\015\012\015brown\015fox"
        << "UTF-8"
        << QByteArray("The\015\012quick\015\012\015\012brown\015\012fox")
        << QString("The\nquick\n\nbrown\nfox")
        << QByteArray("The=0Aquick=0D=0A=0Dbrown=0Dfox")
        << QString("The\012quick\015\012\015brown\015fox")
        << QByteArray("VGhlDQpxdWljaw0KDQpicm93bg0KZm94");
}

void tst_QMailCodec::embedded_newlines()
{
    QFETCH(QString, plaintext); 
    QFETCH(QString, charset); 
    QFETCH(QByteArray, text_encoded);
    QFETCH(QString, text_decoded);
    QFETCH(QByteArray, binary_encoded);
    QFETCH(QString, binary_decoded);
    QFETCH(QByteArray, b64_encoded);

    QByteArray encoded;
    QString reversed;

    // QP encoding should convert line feeds, if the data is textual
    {
        QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2045);
        encoded = codec.encode(plaintext, charset);
        QCOMPARE(encoded, text_encoded);
    }
    {
        QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2045);
        reversed = codec.decode(encoded, charset);
        QCOMPARE(reversed, text_decoded);
    }

    // The lineEnding codec should encode CRLFs 
    {
        QMailLineEndingCodec codec;
        encoded = codec.encode(plaintext, charset);
        QCOMPARE(encoded, text_encoded);
    }
    {
        QMailLineEndingCodec codec;
        reversed = codec.decode(encoded, charset);
        QCOMPARE(reversed, text_decoded);
    }

    // QP encoding should preserve the binary sequence for non-textual data
    {
        QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Binary, QMailQuotedPrintableCodec::Rfc2045);
        encoded = codec.encode(plaintext, charset);
        QCOMPARE(encoded, binary_encoded);
    }
    {
        QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Binary, QMailQuotedPrintableCodec::Rfc2045);
        reversed = codec.decode(encoded, charset);
        QCOMPARE(reversed, binary_decoded);
    }

    // Base64 should produce the same output for these scenarios:
    // 1. Text-mode encoding of plaintext
    // 2. Text-mode encoding of text_encoded
    // 3. Binary-mode encoding of text_encoded
    {
        QMailBase64Codec codec(QMailBase64Codec::Text);
        encoded = codec.encode(plaintext, charset);
        QCOMPARE( encoded, b64_encoded );
    }
    {
        QMailBase64Codec codec(QMailBase64Codec::Text);
        encoded = codec.encode(text_encoded, charset);
        QCOMPARE( encoded, b64_encoded );
    }
    {
        QMailBase64Codec codec(QMailBase64Codec::Binary);
        encoded = codec.encode(text_encoded, charset);
        QCOMPARE( encoded, b64_encoded );
    }

    // Text-mode decoding will produce only the text_decoded form
    {
        QMailBase64Codec codec(QMailBase64Codec::Text);
        reversed = codec.decode(b64_encoded, charset);
        QCOMPARE( reversed, text_decoded );
    }

    // Binary-mode decoding should reproduce the binary input
    {
        QMailBase64Codec codec(QMailBase64Codec::Binary);
        reversed = codec.decode(b64_encoded, charset);
        QCOMPARE( encoded, b64_encoded );
    }
    {
        QMailBase64Codec codec(QMailBase64Codec::Binary);
        encoded = codec.encode(plaintext, charset);
    }
    {
        QMailBase64Codec codec(QMailBase64Codec::Binary);
        reversed = codec.decode(encoded, charset);
        QCOMPARE( reversed, plaintext );
    }
}

