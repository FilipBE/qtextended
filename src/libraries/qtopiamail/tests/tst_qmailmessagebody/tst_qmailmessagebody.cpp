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
#include <QMailBase64Codec>
#include <QMailQuotedPrintableCodec>
#include <QMailMessageBody>
#include <QTemporaryFile>

Q_DECLARE_METATYPE(QMailMessageBody::EncodingStatus)
Q_DECLARE_METATYPE(QMailMessageBody::EncodingFormat)

/* Need to enforce en_US.UTF-8 locale for this test.
 * Unfortunately Qt reads the LANG env var in some code called from
 * a static constructor, so we can't just set it in initTestCase().
 * Need to hijack qgetenv to make sure we set LANG before it can
 * possibly be read. */
QByteArray qgetenv(const char *varName)
{
    static char firstInvoke = 1;
    if (firstInvoke) {
        firstInvoke = 0;
        setenv("LANG", "en_US.UTF-8", 1);
    }
    return QByteArray(getenv(varName));
}


//TESTED_CLASS=QMailMessageBody
//TESTED_FILES=src/libraries/qtopiamail/qmailmessage.cpp

/*
    This class primarily tests that QMailMessageBody correctly handles I/O.
*/
class tst_QMailMessageBody : public QObject
{
    Q_OBJECT

public:
    tst_QMailMessageBody();
    virtual ~tst_QMailMessageBody();

private slots:
    virtual void initTestCase();
    virtual void cleanupTestCase();
    virtual void init();
    virtual void cleanup();

    void fromQByteArray_data();
    void fromQByteArray();
    void fromQString_data();
    void fromQString();
    void fromFile_data();
    void fromFile();
    void toFile_data();
    void toFile();
};

QTEST_APP_MAIN(tst_QMailMessageBody,QtopiaApplication)
#include "tst_qmailmessagebody.moc"

static QByteArray encode(const QByteArray& input, QMailMessageBody::TransferEncoding encoding)
{
    if (encoding == QMailMessageBody::Base64)
    {
        QMailBase64Codec codec(QMailBase64Codec::Text);
        return codec.encode(input);
    }
    else if (encoding == QMailMessageBody::QuotedPrintable)
    {
        QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2045);
        return codec.encode(input);
    }

    return input;
}

static QByteArray encode(const QString& input, const QString& charset, QMailMessageBody::TransferEncoding encoding)
{
    if (encoding == QMailMessageBody::Base64)
    {
        QMailBase64Codec codec(QMailBase64Codec::Text);
        return codec.encode(input, charset);
    }
    else if (encoding == QMailMessageBody::QuotedPrintable)
    {
        QMailQuotedPrintableCodec codec(QMailQuotedPrintableCodec::Text, QMailQuotedPrintableCodec::Rfc2045);
        return codec.encode(input, charset);
    }
    else
    {
        QMailPassThroughCodec codec;
        return codec.encode(input, charset);
    }
}


tst_QMailMessageBody::tst_QMailMessageBody()
{
}

tst_QMailMessageBody::~tst_QMailMessageBody()
{
}

void tst_QMailMessageBody::initTestCase()
{
}

void tst_QMailMessageBody::cleanupTestCase()
{
}

void tst_QMailMessageBody::init()
{
}

/*?
    Cleanup after each test case.
*/
void tst_QMailMessageBody::cleanup()
{
}

void tst_QMailMessageBody::fromQByteArray_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QByteArray>("type");
    QTest::addColumn<QMailMessageBody::TransferEncoding>("encoding");
    QTest::addColumn<QMailMessageBody::EncodingStatus>("status");
    QTest::addColumn<QByteArray>("encoded");
    QTest::addColumn<QByteArray>("decoded");

    QByteArray source, input;

    source = "This is a simple test";
    QTest::newRow("simple") 
        << source
        << QByteArray("text/plain")
        << QMailMessageBody::EightBit
        << QMailMessageBody::AlreadyEncoded
        << source
        << source;

    source = "This is a proper test with encoded input";
    input = encode(source, "ISO-8859-1",  QMailMessageBody::Base64);
    QTest::newRow("encoded - base64") 
        << input
        << QByteArray("text/plain; charset=ISO-8859-1")
        << QMailMessageBody::Base64
        << QMailMessageBody::AlreadyEncoded
        << input
        << source;

    input = encode(source, "ISO-8859-1",  QMailMessageBody::QuotedPrintable);
    QTest::newRow("encoded - QP") 
        << input
        << QByteArray("text/plain; charset=ISO-8859-1")
        << QMailMessageBody::QuotedPrintable
        << QMailMessageBody::AlreadyEncoded
        << input
        << source;

    source = "This is a proper test with unencoded input";
    QTest::newRow("decoded - base64") 
        << source
        << QByteArray("text/plain; charset=ISO-8859-1")
        << QMailMessageBody::Base64
        << QMailMessageBody::RequiresEncoding
        << encode(source, QMailMessageBody::Base64)
        << source;

    QTest::newRow("decoded - QP") 
        << source
        << QByteArray("text/plain; charset=ISO-8859-1")
        << QMailMessageBody::QuotedPrintable
        << QMailMessageBody::RequiresEncoding
        << encode(source, QMailMessageBody::QuotedPrintable)
        << source;
}

void tst_QMailMessageBody::fromQByteArray()
{
    QFETCH( QByteArray, input ); 
    QFETCH( QByteArray, type );
    QFETCH( QMailMessageBody::TransferEncoding, encoding );
    QFETCH( QMailMessageBody::EncodingStatus, status );

    QMailMessageBody body = QMailMessageBody::fromData( input, QMailMessageContentType( type ), encoding, status );
    QTEST( body.data( QMailMessageBody::Encoded ), "encoded" );
    QTEST( body.data( QMailMessageBody::Decoded ), "decoded" );
}

void tst_QMailMessageBody::fromQString_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QByteArray>("type");
    QTest::addColumn<QMailMessageBody::TransferEncoding>("encoding");
    QTest::addColumn<QByteArray>("encoded");
    QTest::addColumn<QString>("decoded");

    QString source;

    source = "This is a simple test";
    QTest::newRow("simple") 
        << source
        << QByteArray("text/plain; charset=UTF-8")
        << QMailMessageBody::EightBit
        << encode(source, "UTF-8", QMailMessageBody::EightBit)
        << source;

    source = "This is a proper test"; 
    QTest::newRow("basic - base64") 
        << source
        << QByteArray("text/plain; charset=UTF-8")
        << QMailMessageBody::Base64
        << encode(source, "UTF-8", QMailMessageBody::Base64)
        << source;

    QTest::newRow("basic - QP") 
        << source
        << QByteArray("text/plain; charset=UTF-8")
        << QMailMessageBody::QuotedPrintable
        << encode(source, "UTF-8", QMailMessageBody::QuotedPrintable)
        << source;

    // Latin-1 Characters
    source = QString("Joh\361 D\366e");
    QTest::newRow("Latin-1 - base64") 
        << source
        << QByteArray("text/plain; charset=ISO-8859-1")
        << QMailMessageBody::Base64
        << encode(source, "ISO-8859-1", QMailMessageBody::Base64)
        << source;

    QTest::newRow("Latin-1 - QP") 
        << source
        << QByteArray("text/plain; charset=ISO-8859-1")
        << QMailMessageBody::QuotedPrintable
        << encode(source, "ISO-8859-1", QMailMessageBody::QuotedPrintable)
        << source;

    // Unicode Characters
    const QChar chars[] = { 0x0636, 0x0669, 0x06a5, 0x06b4 };
    source = QString(chars, 4);
    QTest::newRow("unicode - base64") 
        << source
        << QByteArray("text/plain; charset=UTF-8")
        << QMailMessageBody::Base64
        << encode(source, "UTF-8", QMailMessageBody::Base64)
        << source;

    QTest::newRow("unicode - QP") 
        << source
        << QByteArray("text/plain; charset=UTF-8")
        << QMailMessageBody::QuotedPrintable
        << encode(source, "UTF-8", QMailMessageBody::QuotedPrintable)
        << source;
}

void tst_QMailMessageBody::fromQString()
{
    QFETCH( QString, input ); 
    QFETCH( QByteArray, type );
    QFETCH( QMailMessageBody::TransferEncoding, encoding );

    QMailMessageBody body = QMailMessageBody::fromData( input, QMailMessageContentType( type ), encoding );
    QTEST( body.data( QMailMessageBody::Encoded ), "encoded" );
    QTEST( body.data(), "decoded" );
}

void tst_QMailMessageBody::fromFile_data()
{
    QTest::addColumn<QString>("string_input");
    QTest::addColumn<QByteArray>("bytearray_input");
    QTest::addColumn<QByteArray>("type");
    QTest::addColumn<QMailMessageBody::TransferEncoding>("encoding");
    QTest::addColumn<QMailMessageBody::EncodingStatus>("status");
    QTest::addColumn<QByteArray>("encoded");
    QTest::addColumn<QByteArray>("bytearray_decoded");
    QTest::addColumn<QString>("string_decoded");
    QTest::addColumn<QStringList>("content_properties");

    QString string_source;
    QByteArray bytearray_source;

    // Non-encoding tests
    string_source = "This is a simple test";
    QTest::newRow("simple - QString") 
        << string_source
        << QByteArray()
        << QByteArray("text/plain; charset=UTF-8")
        << QMailMessageBody::EightBit
        << QMailMessageBody::RequiresEncoding
        << encode(string_source, "UTF-8", QMailMessageBody::EightBit)
        << QByteArray()
        << string_source
        << ( QStringList() << "text" << "plain" << "UTF-8" );

    bytearray_source = "This is a simple test";
    QTest::newRow("simple - QByteArray") 
        << QString()
        << bytearray_source
        << QByteArray("text/plain")
        << QMailMessageBody::EightBit
        << QMailMessageBody::RequiresEncoding
        << encode(bytearray_source, QMailMessageBody::EightBit)
        << bytearray_source
        << QString()
        << ( QStringList() << "text" << "plain" << QString() );

    // Encode with B64 and QP
    string_source = "This is a proper test"; 
    QTest::newRow("basic - string - base64") 
        << string_source
        << QByteArray()
        << QByteArray("text/plain; charset=UTF-8")
        << QMailMessageBody::Base64
        << QMailMessageBody::RequiresEncoding
        << encode(string_source, "UTF-8", QMailMessageBody::Base64)
        << QByteArray()
        << string_source
        << ( QStringList() << "text" << "plain" << "UTF-8" );

    QTest::newRow("basic - string - QP") 
        << string_source
        << QByteArray()
        << QByteArray("text/plain; charset=UTF-8")
        << QMailMessageBody::QuotedPrintable
        << QMailMessageBody::RequiresEncoding
        << encode(string_source, "UTF-8", QMailMessageBody::QuotedPrintable)
        << QByteArray()
        << string_source
        << ( QStringList() << "text" << "plain" << "UTF-8" );

    bytearray_source = "This is a proper test"; 
    QTest::newRow("basic - bytearray - base64") 
        << QString()
        << bytearray_source
        << QByteArray("text/plain; charset=ASCII")
        << QMailMessageBody::Base64
        << QMailMessageBody::RequiresEncoding
        << encode(bytearray_source, QMailMessageBody::Base64)
        << bytearray_source
        << QString()
        << ( QStringList() << "text" << "plain" << "ASCII" );

    QTest::newRow("basic - bytearray - base64 - encoded") 
        << QString()
        << encode(bytearray_source, QMailMessageBody::Base64)
        << QByteArray("text/plain; charset=ASCII")
        << QMailMessageBody::Base64
        << QMailMessageBody::AlreadyEncoded
        << encode(bytearray_source, QMailMessageBody::Base64)
        << bytearray_source
        << QString()
        << ( QStringList() << "text" << "plain" << "ASCII" );

    QTest::newRow("basic - bytearray - QP") 
        << QString()
        << bytearray_source
        << QByteArray("text/plain; charset=ASCII")
        << QMailMessageBody::QuotedPrintable
        << QMailMessageBody::RequiresEncoding
        << encode(bytearray_source, QMailMessageBody::QuotedPrintable)
        << bytearray_source
        << QString()
        << ( QStringList() << "text" << "plain" << "ASCII" );

    QTest::newRow("basic - bytearray - QP - encoded") 
        << QString()
        << encode(bytearray_source, QMailMessageBody::QuotedPrintable)
        << QByteArray("text/plain; charset=ASCII")
        << QMailMessageBody::QuotedPrintable
        << QMailMessageBody::AlreadyEncoded
        << encode(bytearray_source, QMailMessageBody::QuotedPrintable)
        << bytearray_source
        << QString()
        << ( QStringList() << "text" << "plain" << "ASCII" );

    // Latin-1 Characters
    string_source = QString("Joh\361 D\366e");
    QTest::newRow("Latin-1 - string - base64") 
        << string_source
        << QByteArray()
        << QByteArray("text/other; charset=UTF-8")
        << QMailMessageBody::Base64
        << QMailMessageBody::RequiresEncoding
        << encode(string_source, "UTF-8", QMailMessageBody::Base64)
        << QByteArray()
        << string_source
        << ( QStringList() << "text" << "other" << "UTF-8" );

    QTest::newRow("Latin-1 - string - QP") 
        << string_source
        << QByteArray()
        << QByteArray("text/other; charset=UTF-8")
        << QMailMessageBody::QuotedPrintable
        << QMailMessageBody::RequiresEncoding
        << encode(string_source, "UTF-8", QMailMessageBody::QuotedPrintable)
        << QByteArray()
        << string_source
        << ( QStringList() << "text" << "other" << "UTF-8" );

    bytearray_source = QByteArray("Joh\361 D\366e");
    QTest::newRow("Latin-1 - bytearray - base64") 
        << QString()
        << bytearray_source
        << QByteArray("text/plain; charset=ISO-8859-1")
        << QMailMessageBody::Base64
        << QMailMessageBody::RequiresEncoding
        << encode(bytearray_source, QMailMessageBody::Base64)
        << bytearray_source
        << QString()
        << ( QStringList() << "text" << "plain" << "ISO-8859-1" );

    QTest::newRow("Latin-1 - bytearray - base64 - encoded") 
        << QString()
        << encode(bytearray_source, QMailMessageBody::Base64)
        << QByteArray("text/plain; charset=ISO-8859-1")
        << QMailMessageBody::Base64
        << QMailMessageBody::AlreadyEncoded
        << encode(bytearray_source, QMailMessageBody::Base64)
        << bytearray_source
        << QString()
        << ( QStringList() << "text" << "plain" << "ISO-8859-1" );

    QTest::newRow("Latin-1 - bytearray - QP") 
        << QString()
        << bytearray_source
        << QByteArray("text/plain; charset=ISO-8859-1")
        << QMailMessageBody::QuotedPrintable
        << QMailMessageBody::RequiresEncoding
        << encode(bytearray_source, QMailMessageBody::QuotedPrintable)
        << bytearray_source
        << QString()
        << ( QStringList() << "text" << "plain" << "ISO-8859-1" );

    QTest::newRow("Latin-1 - bytearray - QP - encoded") 
        << QString()
        << encode(bytearray_source, QMailMessageBody::QuotedPrintable)
        << QByteArray("text/plain; charset=ISO-8859-1")
        << QMailMessageBody::QuotedPrintable
        << QMailMessageBody::AlreadyEncoded
        << encode(bytearray_source, QMailMessageBody::QuotedPrintable)
        << bytearray_source
        << QString()
        << ( QStringList() << "text" << "plain" << "ISO-8859-1" );

    // Unicode Characters
    const QChar chars[] = { 0x0636, 0x0669, 0x06a5, 0x06b4 };
    string_source = QString(chars, 4);
    QTest::newRow("unicode - string - base64") 
        << string_source
        << QByteArray()
        << QByteArray("text/plain; charset=UTF-8")
        << QMailMessageBody::Base64
        << QMailMessageBody::RequiresEncoding
        << encode(string_source, "UTF-8", QMailMessageBody::Base64)
        << QByteArray()
        << string_source
        << ( QStringList() << "text" << "plain" << "UTF-8" );

    QTest::newRow("unicode - string - QP") 
        << string_source
        << QByteArray()
        << QByteArray("text/plain; charset=UTF-8")
        << QMailMessageBody::QuotedPrintable
        << QMailMessageBody::RequiresEncoding
        << encode(string_source, "UTF-8", QMailMessageBody::QuotedPrintable)
        << QByteArray()
        << string_source
        << ( QStringList() << "text" << "plain" << "UTF-8" );
}

void tst_QMailMessageBody::fromFile()
{
    QFETCH( QString, string_input ); 
    QFETCH( QByteArray, bytearray_input ); 
    QFETCH( QByteArray, type );
    QFETCH( QMailMessageBody::TransferEncoding, encoding );
    QFETCH( QMailMessageBody::EncodingStatus, status );

    QTemporaryFile file(QString("%1/%2").arg(QDir::tempPath()).arg(metaObject()->className()));
    QVERIFY( file.open() );
    QString name = file.fileName();
    if ( !string_input.isEmpty() )
    {
        {
            QTextStream out( &file );
            out << string_input;
        }
        file.close();

        QMailMessageBody body = QMailMessageBody::fromFile( name, QMailMessageContentType( type ), encoding, status );
        QCOMPARE( body.transferEncoding(), encoding );
        QTEST( body.data( QMailMessageBody::Encoded ), "encoded" );
        QTEST( body.data(), "string_decoded" );

        QFETCH( QStringList, content_properties );
        QCOMPARE( body.contentType().type(), content_properties[0].toLatin1() );
        QCOMPARE( body.contentType().subType(), content_properties[1].toLatin1() );
        QCOMPARE( body.contentType().charset(), content_properties[2].toLatin1() );
    }
    else
    {
        {
            QDataStream out( &file );
            out.writeRawData( bytearray_input.constData(), bytearray_input.length() );
        }
        file.close();

        QMailMessageBody body = QMailMessageBody::fromFile( name, QMailMessageContentType( type ), encoding, status );
        QCOMPARE( body.transferEncoding(), encoding );
        QTEST( body.data( QMailMessageBody::Encoded ), "encoded" );
        QTEST( body.data( QMailMessageBody::Decoded ), "bytearray_decoded" );

        QFETCH( QStringList, content_properties );
        QCOMPARE( body.contentType().type(), content_properties[0].toLatin1() );
        QCOMPARE( body.contentType().subType(), content_properties[1].toLatin1() );
        QCOMPARE( body.contentType().charset(), content_properties[2].toLatin1() );
    }
}

void tst_QMailMessageBody::toFile_data()
{
    QTest::addColumn<QString>("string_input");
    QTest::addColumn<QByteArray>("bytearray_input");
    QTest::addColumn<QByteArray>("type");
    QTest::addColumn<QMailMessageBody::EncodingFormat>("format");
    QTest::addColumn<QByteArray>("bytearray_output");

    QString string_source = "This is a simple test";

    QTest::newRow("string - decoded") 
        << string_source
        << QByteArray()
        << QByteArray("text/plain; charset=UTF-8")
        << QMailMessageBody::Decoded
        << QByteArray();

    QTest::newRow("string - encoded") 
        << string_source
        << QByteArray()
        << QByteArray("text/plain; charset=UTF-8")
        << QMailMessageBody::Encoded
        << encode(string_source, "UTF-8", QMailMessageBody::Base64);

    QByteArray bytearray_source = "This is a simple test";

    QTest::newRow("bytearray - decoded") 
        << QString()
        << bytearray_source
        << QByteArray("text/plain")
        << QMailMessageBody::Decoded
        << QByteArray();

    QTest::newRow("bytearray - encoded") 
        << QString()
        << bytearray_source
        << QByteArray("text/plain")
        << QMailMessageBody::Encoded
        << encode(bytearray_source, QMailMessageBody::Base64);

    // Latin-1 Characters
    string_source = QString("Joh\361 D\366e");

    QTest::newRow("Latin-1 - string - decoded") 
        << string_source
        << QByteArray()
        << QByteArray("text/other; charset=UTF-8")
        << QMailMessageBody::Decoded
        << QByteArray();

    QTest::newRow("Latin-1 - string - encoded") 
        << string_source
        << QByteArray()
        << QByteArray("text/other; charset=UTF-8")
        << QMailMessageBody::Encoded
        << encode(string_source, "UTF-8", QMailMessageBody::Base64);

    bytearray_source = QByteArray("Joh\361 D\366e");

    QTest::newRow("Latin-1 - bytearray - decoded") 
        << QString()
        << bytearray_source
        << QByteArray("text/other; charset=ISO-8859-1")
        << QMailMessageBody::Decoded
        << QByteArray();

    QTest::newRow("Latin-1 - bytearray - encoded") 
        << QString()
        << bytearray_source
        << QByteArray("text/other; charset=ISO-8859-1")
        << QMailMessageBody::Encoded
        << encode(bytearray_source, QMailMessageBody::Base64);

    // Unicode Characters
    const QChar chars[] = { 0x0636, 0x0669, 0x06a5, 0x06b4 };
    string_source = QString(chars, 4);

    QTest::newRow("unicode - string - decoded") 
        << string_source
        << QByteArray()
        << QByteArray("text/plain; charset=UTF-8")
        << QMailMessageBody::Decoded
        << QByteArray();

    QTest::newRow("unicode - string - encoded") 
        << string_source
        << QByteArray()
        << QByteArray("text/plain; charset=UTF-8")
        << QMailMessageBody::Encoded
        << encode(string_source, "UTF-8", QMailMessageBody::Base64);
}

void tst_QMailMessageBody::toFile()
{
    QFETCH( QString, string_input ); 
    QFETCH( QByteArray, bytearray_input ); 
    QFETCH( QByteArray, type );
    QFETCH( QMailMessageBody::EncodingFormat, format );
    QFETCH( QByteArray, bytearray_output );

    QTemporaryFile file(QString("%1/%2").arg(QDir::tempPath()).arg(metaObject()->className()));
    QVERIFY( file.open() );
    QString name = file.fileName();
    file.close();

    QMailMessageContentType contentType( type );

    // Create a body from whatever data was supplied
    if ( !string_input.isEmpty() )
    {
        QMailMessageBody body = QMailMessageBody::fromData( string_input, contentType, QMailMessageBody::Base64 );
        body.toFile( name, format );
    }
    else
    {
        QMailMessageBody body = QMailMessageBody::fromData( bytearray_input, contentType, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding );
        body.toFile( name, format );
    }

    QVERIFY( file.open() );
    if ( !string_input.isEmpty() && format == QMailMessageBody::Decoded )
    {
        // Read the string from the file and compare
        QTextStream in( &file );
        QString data = in.readAll();
        QCOMPARE( data, string_input );
    }
    else
    {
        // Find the size of the file data
        QFileInfo fi( name );
        QByteArray data( fi.size(), '\0' );

        // Read the file data in for comparison
        QDataStream in( &file );
        in.readRawData( data.data(), data.length() );

        if ( !bytearray_output.isEmpty() )
        {
            QCOMPARE( data, bytearray_output );
        }
        else
        {
            QCOMPARE( data, bytearray_input );
        }
    }
}

