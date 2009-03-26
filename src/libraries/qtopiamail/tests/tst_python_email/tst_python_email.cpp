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
#include <QMailMessage>
#include <QMimeType>
#include "../../../../../src/libraries/qtopiamail/longstring_p.h"

/* 
Note: Any email addresses appearing in this test data must be example addresses,
as defined by RFC 2606.  Therefore, they should use one of the following domains:
    *.example.{com|org|net}
    *.test
    *.example
*/

// RFC 2822 messages use CRLF as the newline indicator
#define CRLF "\015\012"

//TESTED_CLASS=
//TESTED_FILES=

/*
    This class tests that we can handle the mail objects used by the python email library test suite

    Where possible, tests are ported as directly as possible. Tests demonstrating interface
    choices made in the python library are not ported.  Tests duplicating functions tested 
    in other QMailMessage* tests are not ported, unless they exercise the test data files
    we have imported.
*/
class tst_python_email : public QObject
{
    Q_OBJECT

public:
    tst_python_email();
    virtual ~tst_python_email();

    QString path(const QString& filename);
    QMailMessage fromFile(const QString& filename);
    QByteArray fileData(const QString& filename);

private slots:
    void test_get_all();
    void test_get_charsets();
    void test_get_filename();
    void test_get_filename_with_name_parameter();
    void test_get_boundary();
    void test_set_boundary();
    void test_get_decoded_payload();
    void test_decoded_generator();
    void test_as_string();
    void test_get_params();
    void test_get_param_liberal();
    void test_get_param();
    void test_get_param_funky_continuation_lines();
    void test_get_param_with_semis_in_quotes();
    void test_has_key();
    void test_del_param();
    void test_get_content_type_from_message_implicit();
    void test_get_content_type_from_message_explicit();
    void test_get_content_type_from_message_text_plain_implicit();
    void test_get_content_type_from_message_text_plain_explicit();
    void test_get_content_maintype_from_message_implicit();
    void test_get_content_maintype_from_message_explicit();
    void test_get_content_maintype_from_message_text_plain_implicit();
    void test_get_content_maintype_from_message_text_plain_explicit();
    void test_get_content_subtype_from_message_implicit();
    void test_get_content_subtype_from_message_explicit();
    void test_get_content_subtype_from_message_text_plain_implicit();
    void test_get_content_subtype_from_message_text_plain_explicit();
    void test_replace_header();
    void test_broken_base64_payload();
    void test_default_cte();
    void test_long_nonstring();
    void test_long_header_encode();
    void test_no_semis_header_splitter();
    void test_no_split_long_header();
    void test_splitting_multiple_long_lines();
    void test_splitting_first_line_only_is_long();
    void test_long_8bit_header();
    void test_long_to_header();
    void test_long_field_name();
    void test_string_headerinst_eq();
    void test_another_long_multiline_header();
    void test_long_lines_with_different_header();
    void TestMIMEAudio();
    void TestMIMEImage();
    void test_hierarchy();
    void test_empty_multipart_idempotent();
    void test_no_parts_in_a_multipart_with_none_epilogue();
    void test_no_parts_in_a_multipart_with_empty_epilogue();
    void test_one_part_in_a_multipart();
    void test_message_external_body();
    void test_double_boundary();
    void test_nested_inner_contains_outer_boundary();
    void test_nested_with_same_boundary();
    void test_boundary_in_non_multipart();
    void test_boundary_with_leading_space();
    void test_boundary_without_trailing_newline();
    void test_parse_missing_minor_type();
    void test_same_boundary_inner_outer();
    void test_multipart_no_boundary();
    void test_invalid_content_type();
    void test_no_start_boundary();
    void test_no_separating_blank_line();
    void test_lying_multipart();
    void test_missing_start_boundary();
    void test_whitespace_eater_unicode();
    void test_whitespace_eater_unicode_2();
    void test_rfc2047_without_whitespace();
    void test_rfc2047_with_whitespace();
    void test_generate();
    void test_parse_message_rfc822();
    void test_dsn();
    void test_epilogue();
    void test_default_type();
    void test_default_type_with_explicit_container_type();
    void TestIdempotent_data();
    void TestIdempotent();
    void test_crlf_separation();
    void test_rfc2231_get_param();
    void test_rfc2231_set_param();
    void test_rfc2231_no_language_or_charset();
    void test_rfc2231_no_language_or_charset_in_filename();
    void test_rfc2231_partly_encoded();
    void test_rfc2231_no_language_or_charset_in_boundary();
    void test_rfc2231_no_language_or_charset_in_charset();
    void test_rfc2231_bad_encoding_in_filename();
    void test_rfc2231_bad_encoding_in_charset();
    void test_rfc2231_bad_character_in_charset();
    void test_rfc2231_bad_character_in_filename();
    void test_rfc2231_single_tick_in_filename_extended();
    void test_rfc2231_tick_attack_extended();
    void test_rfc2231_no_extended_values();
    void test_rfc2231_encoded_then_unencoded_segments();
    void test_rfc2231_unencoded_then_encoded_segments();
};

QTEST_APP_MAIN( tst_python_email, QtopiaApplication )
#include "tst_python_email.moc"


tst_python_email::tst_python_email()
{
}

tst_python_email::~tst_python_email()
{
}

QString tst_python_email::path(const QString& filename)
{
    QString path(QtopiaUnitTest::baseDataPath());

    if (!path.endsWith('/'))
        path.append('/');
    path.append(filename);

    return path;
}

QMailMessage tst_python_email::fromFile(const QString& filename)
{
    return QMailMessage::fromRfc2822File(path(filename));
}

QByteArray tst_python_email::fileData(const QString& filename)
{
    LongString ls(path(filename));
    QByteArray ba(ls.toQByteArray());
    return QByteArray(ba.constData(), ba.length());
}

void tst_python_email::test_get_all()
{
    QMailMessage msg = fromFile("msg_20.txt");

    QStringList to;
    to << "ccc@zzz.test" << "ddd@zzz.test" << "eee@zzz.test";
    QCOMPARE( msg.headerFieldsText("cc"), to );
    QCOMPARE( msg.headerFieldsText("xx"), QStringList() );

    // Note that our cc() function will return the content of the first encountered 
    // field; it is illegal to have more than one CC field, so this seems acceptable...
    QCOMPARE( msg.cc(), ( QList<QMailAddress>() << QMailAddress("ccc@zzz.test") ) );
}

void tst_python_email::test_get_charsets()
{
    QMailMessage msg = fromFile("msg_08.txt");
    QCOMPARE( msg.multipartType(), QMailMessagePartContainer::MultipartMixed );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/mixed") );
    QVERIFY( msg.hasBody() == false );

    QMailMessageHeaderField field(msg.headerField("Content-Type"));
    QCOMPARE( field.parameter("charset"), QByteArray() );

    QList<QByteArray> charsets;
    charsets << "us-ascii" << "iso-8859-1" << "iso-8859-2" << "koi8-r";
    for (uint i = 0; i < msg.partCount(); ++i)
        QCOMPARE( msg.partAt(i).contentType().charset(), charsets.at(i) );

    msg = fromFile("msg_09.txt");
    QCOMPARE( msg.multipartType(), QMailMessagePartContainer::MultipartMixed );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/mixed") );
    QVERIFY( msg.hasBody() == false );

    field = msg.headerField("Content-Type");
    QCOMPARE( field.parameter("charset"), QByteArray() );

    charsets.clear();
    charsets << "us-ascii" << "iso-8859-1" << QByteArray() << "koi8-r";
    for (uint i = 0; i < msg.partCount(); ++i)
        QCOMPARE( msg.partAt(i).contentType().charset(), charsets.at(i) );

    msg = fromFile("msg_12.txt");
    QCOMPARE( msg.multipartType(), QMailMessagePartContainer::MultipartMixed );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/mixed") );
    QVERIFY( msg.hasBody() == false );

    field = msg.headerField("Content-Type");
    QCOMPARE( field.parameter("charset"), QByteArray() );

    charsets.clear();
    charsets << "us-ascii" << "iso-8859-1" << QByteArray() << "us-ascii" << "koi8-r";
    for (uint i = 0; i < msg.partCount(); ++i)
        QCOMPARE( msg.partAt(i).contentType().charset(), charsets.at(i) );

    const QMailMessagePart& part = msg.partAt(2);
    QCOMPARE( part.multipartType(), QMailMessagePartContainer::MultipartMixed );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/mixed") );
    QVERIFY( part.hasBody() == false );
    QVERIFY( part.partNumber() == 2 );

    charsets.clear();
    charsets << "iso-8859-2" << "iso-8859-3";
    for (uint i = 0; i < part.partCount(); ++i)
        QCOMPARE( part.partAt(i).contentType().charset(), charsets.at(i) );
}

void tst_python_email::test_get_filename()
{
    QMailMessage msg = fromFile("msg_04.txt");
    QCOMPARE( msg.multipartType(), QMailMessagePartContainer::MultipartMixed );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/mixed") );
    QVERIFY( msg.hasBody() == false );

    QList<QByteArray> filenames;
    filenames << "msg.txt" << "msg.txt";
    for (uint i = 0; i < msg.partCount(); ++i)
        QCOMPARE( msg.partAt(i).contentDisposition().filename(), filenames.at(i) );

    msg = fromFile("msg_07.txt");
    QCOMPARE( msg.multipartType(), QMailMessagePartContainer::MultipartMixed );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/mixed") );
    QVERIFY( msg.hasBody() == false );
    QCOMPARE( msg.partAt(1).contentDisposition().filename(), QByteArray("dingusfish.gif") );
}

void tst_python_email::test_get_filename_with_name_parameter()
{
    QMailMessage msg = fromFile("msg_44.txt");
    QCOMPARE( msg.multipartType(), QMailMessagePartContainer::MultipartMixed );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/mixed") );
    QVERIFY( msg.hasBody() == false );

    // Note: the python get_filename() function will return the value of the 'name'
    // parameter, if that exists but the 'filename' parameter does not.  We currently don't...

    QList<QByteArray> filenames;
    filenames << "msg.txt" << "msg.txt";
    for (uint i = 0; i < msg.partCount(); ++i)
        QCOMPARE( msg.partAt(i).contentDisposition().parameter("name"), filenames.at(i) );
}

void tst_python_email::test_get_boundary()
{
    QMailMessage msg = fromFile("msg_07.txt");
    QCOMPARE( msg.boundary(), QByteArray("BOUNDARY") );
}

void tst_python_email::test_set_boundary()
{
    QMailMessage msg = fromFile("msg_01.txt");
    QCOMPARE( msg.boundary(), QByteArray() );

    msg.setBoundary("BOUNDARY");
    QMailMessageContentType ct(msg.headerField("Content-Type"));
    QCOMPARE( ct.toString(), QByteArray("Content-Type: text/plain; charset=us-ascii; boundary=BOUNDARY") );

    msg = fromFile("msg_04.txt");
    QCOMPARE( msg.boundary(), QByteArray("h90VIIIKmx") );

    msg.setBoundary("BOUNDARY");
    ct = msg.headerField("Content-Type");
    QCOMPARE( ct.toString(), QByteArray("Content-Type: multipart/mixed; boundary=BOUNDARY") );

    msg = fromFile("msg_03.txt");
    QCOMPARE( msg.boundary(), QByteArray() );
    QCOMPARE( msg.headerFieldText("Content-Type"), QString("text/plain; charset=us-ascii") );
}

void tst_python_email::test_get_decoded_payload()
{
    QMailMessage msg = fromFile("msg_10.txt");
    QCOMPARE( msg.multipartType(), QMailMessagePartContainer::MultipartMixed );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/mixed") );
    QVERIFY( msg.hasBody() == false );
    QVERIFY( msg.partCount() == 4 );

    QCOMPARE( msg.partAt(0).body().data(), QString("This is a 7bit encoded message.\n") );
    QCOMPARE( msg.partAt(1).body().data(), QString("\241This is a Quoted Printable encoded message!\n") );
    QCOMPARE( msg.partAt(2).body().data(), QString("This is a Base64 encoded message.") );
    QCOMPARE( msg.partAt(3).body().data(), QString("This has no Content-Transfer-Encoding: header.\n") );
}

void tst_python_email::test_decoded_generator()
{
    // This test tests for 'flattening' a message (msg_07.txt), by removing an image
    // file attachment.  The result (msg_17.txt) appears ill-formed, to me...
}

void tst_python_email::test_as_string()
{
    QMailMessage msg = fromFile("msg_01.txt");
    // Note: our standard version differs slightly from python's, due to a header folding policy variation
    QCOMPARE( msg.toRfc2822(), fileData("msg_01a.txt") );
}

void tst_python_email::test_get_params()
{
    QMailMessage msg = QMailMessage::fromRfc2822(QByteArray("X-Header: foo=one; bar=two; baz=three" CRLF));

    // I think the python interface incorrectly returns the 'foo' part as a parameter...
    QList<QMailMessageHeaderField::ParameterType> parameters;
    //parameters << qMakePair(QByteArray("foo"), QByteArray("one"));
    parameters << qMakePair(QByteArray("bar"), QByteArray("two"));
    parameters << qMakePair(QByteArray("baz"), QByteArray("three"));

    QMailMessageHeaderField headerField(msg.headerField("x-header"));
    QCOMPARE( headerField.parameters(), parameters );

    msg = QMailMessage::fromRfc2822(QByteArray("X-Header: foo; bar=one; baz=two" CRLF));

    // I think the python interface incorrectly returns the 'foo' part as a parameter...
    parameters.clear();
    //parameters << qMakePair(QByteArray("foo"), QByteArray());
    parameters << qMakePair(QByteArray("bar"), QByteArray("one"));
    parameters << qMakePair(QByteArray("baz"), QByteArray("two"));

    headerField = msg.headerField("x-header");
    QCOMPARE( headerField.parameters(), parameters );

    msg = QMailMessage::fromRfc2822(QByteArray("X-Header: foo; bar=\"one\"; baz=two" CRLF));

    // I think the python interface incorrectly returns the 'foo' part as a parameter...
    parameters.clear();
    //parameters << qMakePair(QByteArray("foo"), QByteArray());
    parameters << qMakePair(QByteArray("bar"), QByteArray("one"));
    parameters << qMakePair(QByteArray("baz"), QByteArray("two"));

    headerField = msg.headerField("x-header");
    QCOMPARE( headerField.parameters(), parameters );
}

void tst_python_email::test_get_param_liberal()
{
    QMailMessageContentType type("Content-Type: Multipart/mixed; boundary = \"CPIMSSMTPC06p5f3tG\"");
    QCOMPARE( type.boundary(), QByteArray("CPIMSSMTPC06p5f3tG") );
}

void tst_python_email::test_get_param()
{
    QMailMessage msg = QMailMessage::fromRfc2822(QByteArray("X-Header: foo=one; bar=two; baz=three" CRLF));

    QMailMessageHeaderField headerField(msg.headerField("x-header"));
    QCOMPARE( headerField.parameter("bar"), QByteArray("two") );
    QCOMPARE( headerField.parameter("quuz"), QByteArray() );

    msg = QMailMessage::fromRfc2822(QByteArray("X-Header: foo; bar=\"one\"; baz=two" CRLF));

    headerField = msg.headerField("x-header");
    QCOMPARE( headerField.parameter("foo"), QByteArray() );
    QCOMPARE( headerField.parameter("bar"), QByteArray("one") );
    QCOMPARE( headerField.parameter("baz"), QByteArray("two") );

    QMailMessageContentType type("text/plain; weird=\"hey; dolly? [you] @ <\\\\\"home\\\\\">?\"");
    QCOMPARE( type.parameter("weird"), QByteArray("hey; dolly? [you] @ <\\\\\"home\\\\\">?") );
}

void tst_python_email::test_get_param_funky_continuation_lines()
{
    QMailMessage msg = fromFile("msg_22.txt");
    QCOMPARE( msg.partAt(1).contentType().name(), QByteArray("wibble.JPG") );
}

void tst_python_email::test_get_param_with_semis_in_quotes()
{
    QMailMessageContentType type("Content-Type: image/pjpeg; name=\"Jim&amp;&amp;Jill\"" CRLF);
    QCOMPARE( type.name(), QByteArray("Jim&amp;&amp;Jill") );
    QCOMPARE( QMail::quoteString(type.name()), QByteArray("\"Jim&amp;&amp;Jill\"") );
}

void tst_python_email::test_has_key()
{
    QMailMessage msg = QMailMessage::fromRfc2822(QByteArray("Header: exists"));
    QCOMPARE( msg.headerFieldText("header"), QString("exists") );
    QCOMPARE( msg.headerFieldText("Header"), QString("exists") );
    QCOMPARE( msg.headerFieldText("HEADER"), QString("exists") );
    QCOMPARE( msg.headerFieldText("headeri"), QString() );
}

void tst_python_email::test_del_param()
{
    QMailMessage msg = fromFile("msg_05.txt");

    QMailMessageContentType type(msg.headerField("Content-Type"));
    QCOMPARE( type.content(), QByteArray("multipart/report") );

    QList<QMailMessageHeaderField::ParameterType> parameters;
    parameters << qMakePair(QByteArray("report-type"), QByteArray("delivery-status"));
    parameters << qMakePair(QByteArray("boundary"), QByteArray("D1690A7AC1.996856090/mail.example.com"));
    QCOMPARE( type.parameters(), parameters );

    // We don't support the deletion of parameters as tested following the above...
}

void tst_python_email::test_get_content_type_from_message_implicit()
{
    QMailMessage msg = fromFile("msg_30.txt");
    QCOMPARE( msg.partAt(0).contentType().content(), QByteArray("message/rfc822") );
}

void tst_python_email::test_get_content_type_from_message_explicit()
{
    QMailMessage msg = fromFile("msg_28.txt");
    QCOMPARE( msg.partAt(0).contentType().content(), QByteArray("message/rfc822") );
}

void tst_python_email::test_get_content_type_from_message_text_plain_implicit()
{
    QMailMessage msg = fromFile("msg_03.txt");
    QCOMPARE( msg.contentType().content(), QByteArray("text/plain") );
}

void tst_python_email::test_get_content_type_from_message_text_plain_explicit()
{
    QMailMessage msg = fromFile("msg_01.txt");
    QCOMPARE( msg.contentType().content(), QByteArray("text/plain") );
}

void tst_python_email::test_get_content_maintype_from_message_implicit()
{
    QMailMessage msg = fromFile("msg_30.txt");
    QCOMPARE( msg.partAt(0).contentType().type(), QByteArray("message") );
}

void tst_python_email::test_get_content_maintype_from_message_explicit()
{
    QMailMessage msg = fromFile("msg_28.txt");
    QCOMPARE( msg.partAt(0).contentType().type(), QByteArray("message") );
}

void tst_python_email::test_get_content_maintype_from_message_text_plain_implicit()
{
    QMailMessage msg = fromFile("msg_03.txt");
    QCOMPARE( msg.contentType().type(), QByteArray("text") );
}

void tst_python_email::test_get_content_maintype_from_message_text_plain_explicit()
{
    QMailMessage msg = fromFile("msg_01.txt");
    QCOMPARE( msg.contentType().type(), QByteArray("text") );
}

void tst_python_email::test_get_content_subtype_from_message_implicit()
{
    QMailMessage msg = fromFile("msg_30.txt");
    QCOMPARE( msg.partAt(0).contentType().subType(), QByteArray("rfc822") );
}

void tst_python_email::test_get_content_subtype_from_message_explicit()
{
    QMailMessage msg = fromFile("msg_28.txt");
    QCOMPARE( msg.partAt(0).contentType().subType(), QByteArray("rfc822") );
}

void tst_python_email::test_get_content_subtype_from_message_text_plain_implicit()
{
    QMailMessage msg = fromFile("msg_03.txt");
    QCOMPARE( msg.contentType().subType(), QByteArray("plain") );
}

void tst_python_email::test_get_content_subtype_from_message_text_plain_explicit()
{
    QMailMessage msg = fromFile("msg_01.txt");
    QCOMPARE( msg.contentType().subType(), QByteArray("plain") );
}

void tst_python_email::test_replace_header()
{
    QMailMessage msg;
    msg.appendHeaderField("First", "One");
    msg.appendHeaderField("Second", "Two");
    msg.appendHeaderField("Third", "Three");

    QCOMPARE( msg.headerFields(), ( QList<QMailMessageHeaderField>()
                                        << QMailMessageHeaderField("First", "One")
                                        << QMailMessageHeaderField("Second", "Two")
                                        << QMailMessageHeaderField("Third", "Three") ) );

    msg.setHeaderField("Second", "Twenty");
    QCOMPARE( msg.headerFields(), ( QList<QMailMessageHeaderField>()
                                        << QMailMessageHeaderField("First", "One")
                                        << QMailMessageHeaderField("Second", "Twenty")
                                        << QMailMessageHeaderField("Third", "Three") ) );

    msg.appendHeaderField("First", "Eleven");
    msg.setHeaderField("First", "One Hundred");
    QCOMPARE( msg.headerFields(), ( QList<QMailMessageHeaderField>()
                                        << QMailMessageHeaderField("First", "One Hundred")
                                        << QMailMessageHeaderField("Second", "Twenty")
                                        << QMailMessageHeaderField("Third", "Three")
                                        << QMailMessageHeaderField("First", "Eleven") ) );
}

void tst_python_email::test_broken_base64_payload()
{
    QByteArray data("AwDp0P7//y6LwKEAcPa/6Q=9");

    QMailMessageContentType type("audio/x-midi");
    QMailMessageBody body = QMailMessageBody::fromData(data, type, QMailMessageBody::Base64, QMailMessageBody::AlreadyEncoded);

    // It appears that python will return the un-decoded data when the decoding fails - we don't...
    //QCOMPARE( body.data(QMailMessageBody::Decoded), data );
}

void tst_python_email::test_default_cte()
{
    // We don't support this interface directly, but we do have similar logic that needs testing...

    //def test_default_cte(self):
    //  eq = self.assertEqual
    //  # With no explicit _charset its us-ascii, and all are 7-bit
    //  msg = MIMEText('hello world')
    //  eq(msg['content-transfer-encoding'], '7bit')
    //  # Similar, but with 8-bit data
    //  msg = MIMEText('hello \xf8 world')
    //  eq(msg['content-transfer-encoding'], '8bit')
    //  # And now with a different charset
    //  msg = MIMEText('hello \xf8 world', _charset='iso-8859-1')
    //  eq(msg['content-transfer-encoding'], 'quoted-printable')
}

void tst_python_email::test_long_nonstring()
{
    // Note, this is not the same as the python test; it does test the same functions,
    // and I believe the output is conforming...

    QByteArray input;

    QString original = "Die Mieter treten hier ein werden mit einem Foerderband komfortabel den Korridor entlang, an s" "\xfc" "dl" "\xfc" "ndischen Wandgem" "\xe4" "lden vorbei, gegen die rotierenden Klingen bef" "\xf6" "rdert. ";
    input.append(QMailMessageHeaderField::encodeWord(original, "ISO-8859-1"));

    // We don't have a ISO-8859-2 codec...
    //original = "Finan" "\xe8" "ni metropole se hroutily pod tlakem jejich d" "\xf9" "vtipu.. ";
    //input.append(' ').append(QMailMessageHeaderField::encodeWord(original, "ISO-8859-2"));

    // Python appears to identify runs of single-byte characters within unicode strings, and 
    // output them in quoted-printable encoded-words.  We don't do that.
    QChar chars[] = { 0x6b63, 0x78ba, 0x306b, 0x8a00, 0x3046, 0x3068, 0x7ffb, 0x8a33, 0x306f, 0x3055, 0x308c, 0x3066, 0x3044, 0x307e, 0x305b, 0x3093, 0x3002, 0x4e00, 0x90e8, 0x306f, 0x30c9, 0x30a4, 0x30c4, 0x8a9e, 0x3067, 0x3059, 0x304c, 0x3001, 0x3042, 0x3068, 0x306f, 0x3067, 0x305f, 0x3089, 0x3081, 0x3067, 0x3059, 0x3002, 0x5b9f, 0x969b, 0x306b, 0x306f, 0x300c, 'W', 'e', 'n', 'n', ' ', 'i', 's', 't', ' ', 'd', 'a', 's', ' ', 'N', 'u', 'n', 's', 't', 'u', 'c', 'k', ' ', 'g', 'i', 't', ' ', 'u', 'n', 'd', ' ', 'S', 'l', 'o', 't', 'e', 'r', 'm', 'e', 'y', 'e', 'r', '?', ' ', 'J', 'a', '!', ' ', 'B', 'e', 'i', 'h', 'e', 'r', 'h', 'u', 'n', 'd', ' ', 'd', 'a', 's', ' ', 'O', 'd', 'e', 'r', ' ', 'd', 'i', 'e', ' ', 'F', 'l', 'i', 'p', 'p', 'e', 'r', 'w', 'a', 'l', 'd', 't', ' ', 'g', 'e', 'r', 's', 'p', 'u', 't', '.', 0x300d, 0x3068, 0x8a00, 0x3063, 0x3066, 0x3044, 0x307e, 0x3059, 0x3002 };
    original = QString(chars, sizeof(chars) / sizeof(chars[0]));
    input.append(' ').append(QMailMessageHeaderField::encodeWord(original, "UTF-8"));

    QByteArray output = "\
Subject: =?ISO-8859-1?Q?Die_Mieter_treten_hier_ein_werden_mit_einem_Foerderband=20?=\
 =?ISO-8859-1?Q?komfortabel_den_Korridor_entlang=2C_an_s=FCdl=FCndischen?=\
 =?ISO-8859-1?Q?_Wandgem=E4lden_vorbei=2C_gegen_die_rotierenden_Klingen=20?=\
 =?ISO-8859-1?Q?bef=F6rdert=2E_?="
/* We don't support ISO-8859-2, so this part is not relevant:
 =?iso-8859-2?q?Finan=E8ni_met?=\
 =?iso-8859-2?q?ropole_se_hroutily_pod_tlakem_jejich_d=F9vtipu=2E=2E_?=\
*/
/* We don't do mixed-mode encoding of unicode, so our output differs:
 =?utf-8?b?5q2j56K644Gr6KiA44GG44Go57+76Kiz44Gv44GV44KM44Gm44GE?=\
 =?utf-8?b?44G+44Gb44KT44CC5LiA6YOo44Gv44OJ44Kk44OE6Kqe44Gn44GZ44GM44CB?=\
 =?utf-8?b?44GC44Go44Gv44Gn44Gf44KJ44KB44Gn44GZ44CC5a6f6Zqb44Gr44Gv44CM?=\
 =?utf-8?q?Wenn_ist_das_Nunstuck_git_und_Slotermeyer=3F_Ja!_Beiherhund_das?=\
 =?utf-8?b?IE9kZXIgZGllIEZsaXBwZXJ3YWxkdCBnZXJzcHV0LuOAjeOBqOiogOOBow==?=\
 =?utf-8?b?44Gm44GE44G+44GZ44CC?=";
*/
"\
 =?UTF-8?B?5q2j56K644Gr6KiA44GG44Go57+76Kiz44Gv44GV44KM44Gm44GE44G+44Gb?=\
 =?UTF-8?B?44KT44CC5LiA6YOo44Gv44OJ44Kk44OE6Kqe44Gn44GZ44GM44CB44GC44Go?=\
 =?UTF-8?B?44Gv44Gn44Gf44KJ44KB44Gn44GZ44CC5a6f6Zqb44Gr44Gv44CMV2VubiBp?=\
 =?UTF-8?B?c3QgZGFzIE51bnN0dWNrIGdpdCB1bmQgU2xvdGVybWV5ZXI/IEphISBCZWlo?=\
 =?UTF-8?B?ZXJodW5kIGRhcyBPZGVyIGRpZSBGbGlwcGVyd2FsZHQgZ2Vyc3B1dC7jgI3j?=\
 =?UTF-8?B?gajoqIDjgaPjgabjgYTjgb7jgZnjgII=?=";

    QMailMessageHeaderField subject("Subject", input);
    QCOMPARE( subject.toString(), output );
}

static QByteArray testHeaderOutput(const QMailMessageHeaderField& field)
{
    QMailMessage msg;
    msg.appendHeaderField(field);

    QByteArray output = msg.toRfc2822();

    // Find the end of the first header output
    int index = output.indexOf(CRLF);
    while (isspace(output[index + 2]))
        index = output.indexOf(CRLF, index + 1);

    return output.left(index);
}

void tst_python_email::test_long_header_encode()
{
    // NOte: python will preserve unnecessary quotes in header fields; we currently don't
    QMailMessageHeaderField field("X-Foobar-Spoink-Defrobnit", "wasnipoop; giraffes=\"very-long-necked-animals\"; spooge=\"yummy\"; hippos=\"gargantuan\"; marshmallows=\"gooey\"");
    QCOMPARE( field.toString(), QByteArray("X-Foobar-Spoink-Defrobnit: wasnipoop; giraffes=very-long-necked-animals; spooge=yummy; hippos=gargantuan; marshmallows=gooey") );

    QByteArray output =
"X-Foobar-Spoink-Defrobnit: wasnipoop; giraffes=very-long-necked-animals;" CRLF
" spooge=yummy; hippos=gargantuan; marshmallows=gooey";

    QByteArray result = testHeaderOutput(field);
    QCOMPARE( result, output );
}

void tst_python_email::test_no_semis_header_splitter()
{
    QByteArray refs;
    for (int i = 0; i < 10; ++i)
        refs.append(QString("%1<%2@example>").arg(QString(i == 0 ? "" : " ")).arg(i).toLatin1());

    QByteArray output =
"References: <0@example> <1@example> <2@example> <3@example> <4@example>" CRLF
" <5@example> <6@example> <7@example> <8@example> <9@example>";

    QMailMessageHeaderField field("References", refs, QMailMessageHeaderField::UnstructuredField);
    QByteArray result = testHeaderOutput(field);
    QCOMPARE( result, output );
}

void tst_python_email::test_no_split_long_header()
{
    // Note: we differ from python here - python will not split up a token which exceeds the line length...
    QByteArray output =
"References:" CRLF
" xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" CRLF
"\txxxxxxxxxxxxxxxxxxxxxxx";

    QMailMessageHeaderField field("References", QByteArray(100, 'x'), QMailMessageHeaderField::UnstructuredField);
    QByteArray result = testHeaderOutput(field);
    QCOMPARE( result, output );
}

void tst_python_email::test_splitting_multiple_long_lines()
{
    QByteArray input = 
"from babylon.scr.example.org (localhost [127.0.0.1]); by babylon.scr.example.org (Postfix) with ESMTP id B570E51B81; for <mailman-admin@babylon.scr.example.org>; Sat, 2 Feb 2002 17:00:06 -0800 (PST)"
"\tfrom babylon.scr.example.org (localhost [127.0.0.1]); by babylon.scr.example.org (Postfix) with ESMTP id B570E51B81; for <mailman-admin@babylon.scr.example.org>; Sat, 2 Feb 2002 17:00:06 -0800 (PST)"
"\tfrom babylon.scr.example.org (localhost [127.0.0.1]); by babylon.scr.example.org (Postfix) with ESMTP id B570E51B81; for <mailman-admin@babylon.scr.example.org>; Sat, 2 Feb 2002 17:00:06 -0800 (PST)";

    QByteArray output =
"X-Data: from babylon.scr.example.org (localhost [127.0.0.1]);" CRLF
" by babylon.scr.example.org (Postfix) with ESMTP id B570E51B81;" CRLF
" for <mailman-admin@babylon.scr.example.org>;" CRLF
" Sat, 2 Feb 2002 17:00:06 -0800 (PST)\tfrom babylon.scr.example.org (localhost" CRLF
" [127.0.0.1]); by babylon.scr.example.org (Postfix) with ESMTP id B570E51B81;" CRLF
" for <mailman-admin@babylon.scr.example.org>;" CRLF
" Sat, 2 Feb 2002 17:00:06 -0800 (PST)\tfrom babylon.scr.example.org (localhost" CRLF
" [127.0.0.1]); by babylon.scr.example.org (Postfix) with ESMTP id B570E51B81;" CRLF
" for <mailman-admin@babylon.scr.example.org>;" CRLF
" Sat, 2 Feb 2002 17:00:06 -0800 (PST)";

    QMailMessageHeaderField field("X-Data", input, QMailMessageHeaderField::UnstructuredField);
    QByteArray result = testHeaderOutput(field);
    QCOMPARE( result, output );
}

void tst_python_email::test_splitting_first_line_only_is_long()
{
    QByteArray input = 
"from modemcable093.139-201-24.que.mc.vidtron.test ([24.201.139.93] helo=cthulhu.gg.test)"
"\tby kronos.mems-exchange.test with esmtp (Exim 4.05)"
"\tid 17k4h5-00034i-00"
"\tfor test@mems-exchange.test; Wed, 28 Aug 2002 11:25:20 -0400";

    QByteArray output =
"X-Data: from modemcable093.139-201-24.que.mc.vidtron.test ([24.201.139.93]" CRLF
" helo=cthulhu.gg.test)\tby kronos.mems-exchange.test with esmtp (Exim 4.05)\tid" CRLF
" 17k4h5-00034i-00\tfor test@mems-exchange.test; Wed, 28 Aug 2002 11:25:20 -0400";

    QMailMessageHeaderField field("X-Data", input, QMailMessageHeaderField::UnstructuredField);
    QByteArray result = testHeaderOutput(field);
    QCOMPARE( result, output );
}

void tst_python_email::test_long_8bit_header()
{
    QByteArray original = "Britische Regierung gibt";
    QByteArray input(QMailMessageHeaderField::encodeWord(original, "ISO-8859-1"));

    original = "gr" "\xfc" "nes Licht f" "\xfc" "r Offshore-Windkraftprojekte";
    input.append(' ').append(QMailMessageHeaderField::encodeWord(original, "ISO-8859-1"));

    // Note the same as the equivalent python formulation, but again, conforming
    QByteArray output = 
"Subject: =?ISO-8859-1?Q?Britische_Regierung_gibt?=" CRLF
" =?ISO-8859-1?Q?gr=FCnes_Licht_f=FCr_Offshore-Windkraftprojekte?=";

    QMailMessageHeaderField field("Subject", input);
    QByteArray result = testHeaderOutput(field);
    QCOMPARE( result, output );
}

void tst_python_email::test_long_to_header()
{
    QByteArray input = 
"\"Someone Test #A\" <someone@eecs.umich.test>,<someone@eecs.umich.test>,\"Someone Test #B\" <someone@umich.test>, \"Someone Test #C\" <someone@eecs.umich.test>, \"Someone Test #D\" <someone@eecs.umich.test>";

    // Note the same as the equivalent python formulation, but again, conforming
    QByteArray output =
"To: \"Someone Test #A\"" CRLF
" <someone@eecs.umich.test>,<someone@eecs.umich.test>,\"Someone Test #B\"" CRLF
" <someone@umich.test>, \"Someone Test #C\" <someone@eecs.umich.test>, \"Someone" CRLF
" Test #D\" <someone@eecs.umich.test>";

    QMailMessageHeaderField field("To", input, QMailMessageHeaderField::UnstructuredField);
    QByteArray result = testHeaderOutput(field);
    QCOMPARE( result, output );
}

void tst_python_email::test_long_field_name()
{
    QString original = 
"Die Mieter treten hier ein werden mit einem Foerderband komfortabel den Korridor entlang, an s" "\xfc" "dl" "\xf" "cndischen Wandgem" "\xe4" "lden vorbei, gegen die rotierenden Klingen bef" "\xf6" "rdert. ";
    QByteArray input = QMailMessageHeaderField::encodeContent(original);

    // Note the same as the equivalent python formulation, but again, conforming
    QByteArray output =
"X-Very-Very-Very-Long-Header-Name: Die Mieter treten hier ein werden mit" CRLF
" einem Foerderband komfortabel den Korridor entlang, an" CRLF
" =?ISO-8859-1?Q?s=FCdl=0Fcndischen?= =?ISO-8859-1?Q?_Wandgem=E4lden?= vorbei," CRLF
" gegen die rotierenden Klingen =?ISO-8859-1?Q?bef=F6rdert=2E?=";

    QMailMessageHeaderField field("X-Very-Very-Very-Long-Header-Name", input, QMailMessageHeaderField::UnstructuredField);
    QByteArray result = testHeaderOutput(field);
    QCOMPARE( result, output );

    // And, lets try with encodeWord instead
    input = QMailMessageHeaderField::encodeWord(original);

    output =
"X-Very-Very-Very-Long-Header-Name:" CRLF
" =?ISO-8859-1?Q?Die_Mieter_treten_hier_ein_werden_mit_einem_Foerderband=20?=" CRLF
" =?ISO-8859-1?Q?komfortabel_den_Korridor_entlang=2C_an_s=FCdl=0Fcndischen?=" CRLF
" =?ISO-8859-1?Q?_Wandgem=E4lden_vorbei=2C_gegen_die_rotierenden_Klingen=20?=" CRLF
" =?ISO-8859-1?Q?bef=F6rdert=2E_?=";

    field = QMailMessageHeaderField("X-Very-Very-Very-Long-Header-Name", input, QMailMessageHeaderField::UnstructuredField);
    result = testHeaderOutput(field);
    QCOMPARE( result, output );
}

void tst_python_email::test_string_headerinst_eq()
{
    // This test doesn't really achieve anything except use of realistic-looking header string...
    QByteArray input = 
"<15975.17901.207240.414604@sgritzmann1.mathematik.tu-muenchen.test> (David Bremner's message of \"Thu, 6 Mar 2003 13:58:21 +0100\")";

    // Note the same as the equivalent python formulation, but again, conforming
    QByteArray output =
"Received: <15975.17901.207240.414604@sgritzmann1.mathematik.tu-muenchen.test>" CRLF
" (David Bremner's message of \"Thu, 6 Mar 2003 13:58:21 +0100\")";

    QMailMessageHeaderField field("Received", input, QMailMessageHeaderField::UnstructuredField);
    QByteArray result = testHeaderOutput(field);
    QCOMPARE( result, output );
}

void tst_python_email::test_another_long_multiline_header()
{
    QByteArray input = 
"Received: from siimage.test ([172.25.1.3]) by zima.siliconimage.test with Microsoft SMTPSVC(5.0.2195.4905);" CRLF
"\tWed, 16 Oct 2002 07:41:11 -0700";

    QMailMessage msg = QMailMessage::fromRfc2822(input);
    QMailMessageHeaderField field = msg.headerField("Received", QMailMessageHeaderField::UnstructuredField);

    QByteArray output =
"Received: from siimage.test ([172.25.1.3]) by zima.siliconimage.test with" CRLF
" Microsoft SMTPSVC(5.0.2195.4905);\tWed, 16 Oct 2002 07:41:11 -0700";

    QByteArray result = testHeaderOutput(field);
    QCOMPARE( result, output );
}

void tst_python_email::test_long_lines_with_different_header()
{
    // This test doesn't really achieve anything except use of realistic-looking header string...
    QByteArray input = 
"List: List-Unsubscribe: <https://lists.sourceforge.test/lists/listinfo/spamassassin-talk>," CRLF
"\t<mailto:spamassassin-talk-request@lists.sourceforge.test?subject=unsubscribe>";

    QMailMessage msg = QMailMessage::fromRfc2822(input);
    QMailMessageHeaderField field = msg.headerField("List", QMailMessageHeaderField::UnstructuredField);

    // Note the same as the equivalent python formulation, but again, conforming
    QByteArray output =
"List: List-Unsubscribe:" CRLF
" <https://lists.sourceforge.test/lists/listinfo/spamassassin-talk>," CRLF
"\t<mailto:spamassassin-talk-request@lists.sourceforge.test?subject=unsubscribe>";

    QByteArray result = testHeaderOutput(field);
    QCOMPARE( result, output );
}

void tst_python_email::TestMIMEAudio()
{
    const QString filename("audiotest.au");

    // A sprinkling of the tests from this python class...
    QString p(path(filename));
    QMimeType mimeType(p);
    QCOMPARE( mimeType.id(), QString("audio/basic") );

    QMailMessageContentType type(mimeType.id().toLatin1());
    QMailMessageBody body = QMailMessageBody::fromFile(p, type, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding);
    QCOMPARE( body.data(QMailMessageBody::Decoded), fileData(filename) );

    QByteArray encoded = body.data(QMailMessageBody::Encoded);
    QMailMessageBody copy = QMailMessageBody::fromData(encoded, type, QMailMessageBody::Base64, QMailMessageBody::AlreadyEncoded);
    QCOMPARE( copy.data(QMailMessageBody::Decoded), fileData(filename) );

    QMailMessageContentDisposition disposition(QMailMessageContentDisposition::Attachment);
    disposition.setFilename(filename.toLatin1());
    QCOMPARE( disposition.type(), QMailMessageContentDisposition::Attachment );
    QCOMPARE( disposition.filename(), filename.toLatin1() );
    QCOMPARE( disposition.toString(), QByteArray("Content-Disposition: attachment; filename=audiotest.au") );
}

void tst_python_email::TestMIMEImage()
{
    const QString filename("PyBanner048.gif");

    // A sprinkling of the tests from this python class...
    QString p(path(filename));
    QMimeType mimeType(p);
    QCOMPARE( mimeType.id(), QString("image/gif") );

    QMailMessageContentType type(mimeType.id().toLatin1());
    QMailMessageBody body = QMailMessageBody::fromFile(p, type, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding);
    QCOMPARE( body.data(QMailMessageBody::Decoded), fileData(filename) );

    QByteArray encoded = body.data(QMailMessageBody::Encoded);
    QMailMessageBody copy = QMailMessageBody::fromData(encoded, type, QMailMessageBody::Base64, QMailMessageBody::AlreadyEncoded);
    QCOMPARE( copy.data(QMailMessageBody::Decoded), fileData(filename) );

    QByteArray description("dingusfish.gif");
    QMailMessageContentDisposition disposition(QMailMessageContentDisposition::Attachment);
    disposition.setFilename(description);
    QCOMPARE( disposition.type(), QMailMessageContentDisposition::Attachment );
    QCOMPARE( disposition.filename(), description );
    QCOMPARE( disposition.toString(), QByteArray("Content-Disposition: attachment; filename=dingusfish.gif") );
}

/*
void tst_python_email::TestMIMEText()
{
    // This python test doesn't really map to any of our functionality...
    // What we should test instead, perhaps, is loading text files containing
    // variously encoded text data...
}
*/

void tst_python_email::test_hierarchy()
{
    QString p(path("PyBanner048.gif"));

    QByteArray input = 
"Hi there," CRLF
CRLF
"This is the dingus fish." CRLF;

    QMailMessageContentType imageType("image/gif");
    QMailMessageBody image = QMailMessageBody::fromFile(p, imageType, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding);

    QMailMessageContentType textType("text/plain");
    QMailMessageBody text = QMailMessageBody::fromData(input, textType, QMailMessageBody::EightBit, QMailMessageBody::AlreadyEncoded);

    QMailMessageContentDisposition disposition(QMailMessageContentDisposition::Attachment);
    disposition.setFilename("dingusfish.gif");

    QMailMessagePart imagePart;
    imagePart.setBody(image);
    imagePart.setContentDisposition(disposition);

    QMailMessagePart textPart;
    textPart.setBody(text);

    QMailMessage msg;
    msg.setMultipartType(QMailMessage::MultipartMixed);
    msg.setBoundary("BOUNDARY");
    msg.appendPart(textPart);
    msg.appendPart(imagePart);

    msg.appendHeaderField("From", "Barry <barry@example.com>");
    msg.appendHeaderField("To", "Dingus Lovers <cravindogs@cravindogs.test>");
    msg.appendHeaderField("Subject", "Here is your dingus fish");

    QCOMPARE( msg.multipartType(), QMailMessage::MultipartMixed );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/mixed") );
    QVERIFY( msg.hasBody() == false );
    QVERIFY( msg.partCount() == 2 );

    QCOMPARE( msg.partAt(0).multipartType(), QMailMessagePart::MultipartNone );
    QVERIFY( msg.partAt(0).hasBody() );
    QCOMPARE( msg.partAt(0).contentType().content().toLower(), QByteArray("text/plain") );
    QCOMPARE( msg.partAt(0).body().data(QMailMessageBody::Decoded), textPart.body().data(QMailMessageBody::Decoded) );
    QVERIFY( msg.partAt(0).partNumber() == 0 );

    QCOMPARE( msg.partAt(1).multipartType(), QMailMessagePart::MultipartNone );
    QVERIFY( msg.partAt(1).hasBody() );
    QCOMPARE( msg.partAt(1).contentType().content().toLower(), QByteArray("image/gif") );
    QCOMPARE( msg.partAt(1).body().data(QMailMessageBody::Decoded), imagePart.body().data(QMailMessageBody::Decoded) );
    QVERIFY( msg.partAt(1).partNumber() == 1 );
}

void tst_python_email::test_empty_multipart_idempotent()
{
    QByteArray input = 
"Content-Type: multipart/mixed; boundary=\"BOUNDARY\"" CRLF
"MIME-Version: 1.0" CRLF
"Subject: A subject" CRLF
"To: aperson@domain.example" CRLF
"From: bperson@domain.example" CRLF
CRLF
CRLF
"--BOUNDARY" CRLF
CRLF
CRLF
"--BOUNDARY--" CRLF;

    QByteArray output(input);

    // Unlike python, we don't actually produce idempotent output

    // It's certainly arguable that we should not produce these values, since
    // they are the defaults...
    QByteArray partHeader =
"Content-Type: text/plain; charset=us-ascii" CRLF
"Content-Transfer-Encoding: 7bit" CRLF;

    int index = output.indexOf("--BOUNDARY");
    output.insert(index + 12, partHeader);

    QMailMessage msg = QMailMessage::fromRfc2822(input);
    QCOMPARE( msg.toRfc2822(QMailMessage::IdentityFormat), output );
}

void tst_python_email::test_no_parts_in_a_multipart_with_none_epilogue()
{
    QMailMessage msg;
    msg.setMultipartType(QMailMessage::MultipartMixed);
    msg.setBoundary("BOUNDARY");

    msg.appendHeaderField("Subject", "A subject");
    msg.appendHeaderField("To", "aperson@domain.example");
    msg.appendHeaderField("From", "bperson@domain.example");

    // We include more standard header fields than python:

    // Note - python produces an empty, one-part formulation for this test.  I think
    // neither formulation is well-formed, so it probably doesn't matter that we differ...
    QByteArray output = 
"Content-Type: multipart/mixed; boundary=BOUNDARY" CRLF
"Subject: A subject" CRLF
"To: aperson@domain.example" CRLF
"From: bperson@domain.example" CRLF
CRLF
CRLF
"--BOUNDARY--" CRLF;

    QCOMPARE( msg.toRfc2822(QMailMessage::IdentityFormat), output );
}

void tst_python_email::test_no_parts_in_a_multipart_with_empty_epilogue()
{
    // Note: this test demonstrates python's ability to set the preamble
    // and epilogue of a multipart message. We don't expose this functionality
}

void tst_python_email::test_one_part_in_a_multipart()
{
    QMailMessage msg;
    msg.setMultipartType(QMailMessage::MultipartMixed);
    msg.setBoundary("BOUNDARY");

    msg.appendHeaderField("Subject", "A subject");
    msg.appendHeaderField("To", "aperson@domain.example");
    msg.appendHeaderField("From", "bperson@domain.example");

    QMailMessagePart textPart;
    QMailMessageContentType type("text/plain");
    textPart.setBody(QMailMessageBody::fromData(QByteArray("hello world"), type, QMailMessageBody::SevenBit, QMailMessageBody::AlreadyEncoded));

    msg.appendPart(textPart);

    QByteArray output = 
"Content-Type: multipart/mixed; boundary=BOUNDARY" CRLF
"Subject: A subject" CRLF
"To: aperson@domain.example" CRLF
"From: bperson@domain.example" CRLF
CRLF
CRLF
"--BOUNDARY" CRLF
"Content-Type: text/plain" CRLF
"Content-Transfer-Encoding: 7bit" CRLF
CRLF
"hello world" CRLF
"--BOUNDARY--" CRLF;

    QCOMPARE( msg.toRfc2822(QMailMessage::IdentityFormat), output );
}

void tst_python_email::test_message_external_body()
{
    QMailMessage msg = fromFile("msg_36.txt");
    QVERIFY( msg.partCount() == 2 );

    QCOMPARE( msg.partAt(1).multipartType(), QMailMessagePart::MultipartAlternative );
    QCOMPARE( msg.partAt(1).contentType().content().toLower(), QByteArray("multipart/alternative") );
    QVERIFY( msg.partAt(1).hasBody() == false );
    QVERIFY( msg.partAt(1).partCount() == 2 );
    QVERIFY( msg.partAt(1).partNumber() == 1 );

    for (int i = 0; i < 2; ++i) {
        const QMailMessagePart& part = msg.partAt(1).partAt(i);

        QCOMPARE( part.contentType().content().toLower(), QByteArray("message/external-body") );

        /* Note: we don't have built-in support for 'message' parts.
           Python prsents the following interface:
        QVERIFY( part.partCount() == 1 );
        QCOMPARE( part.partAt(0).contentType().content().toLower(), QByteArray("text/plain") );
        QVERIFY( part.partAt(0).partCount() == 0 );
        */

        // Parse the message header field explicitly
        QVERIFY( part.hasBody() == true );
        QVERIFY( part.partCount() == 0 );
        QByteArray messageData = part.body().data(QMailMessageBody::Decoded);
        QMailMessage subMessage = QMailMessage::fromRfc2822(messageData);

        QCOMPARE( subMessage.contentType().content().toLower(), QByteArray("text/plain") );
        QVERIFY( subMessage.partCount() == 0 );
    }
}

void tst_python_email::test_double_boundary()
{
    /* From python:
        # msg_37.txt is a multipart that contains two dash-boundary's in a
        # row.  Our interpretation of RFC 2046 calls for ignoring the second
        # and subsequent boundaries.

    Note: unlike python, we parse this into 7 parts, of which three are conforming,
    and the remainder do not contain the minimum requirement of a CRLF-pair. The
    non-conforming parts are ignored.
    */
    QMailMessage msg = fromFile("msg_37.txt");
    QVERIFY( msg.partCount() == 3 );
}

void tst_python_email::test_nested_inner_contains_outer_boundary()
{
    /* From python:
        # msg_38.txt has an inner part that contains outer boundaries.  My
        # interpretation of RFC 2046 (based on sections 5.1 and 5.1.2) say
        # these are illegal and should be interpreted as unterminated inner
        # parts.

    Note: AFAICT, this is illegal, due to the following stipulation: 
    "Boundary delimiters must not appear within the encapsulated material"
    And we don't support it, apart from accepting the input.
    */
    QMailMessage msg = fromFile("msg_38.txt");
    QVERIFY( msg.partCount() == 2 );
}

void tst_python_email::test_nested_with_same_boundary()
{
    /* From python:
        # msg 39.txt is similarly evil in that it's got inner parts that use
        # the same boundary as outer parts.  Again, I believe the way this is
        # parsed is closest to the spirit of RFC 2046

    Note: Again this is illegal, and we don't support it.
    */
    QMailMessage msg = fromFile("msg_39.txt");
    QVERIFY( msg.partCount() == 2 );
}

void tst_python_email::test_boundary_in_non_multipart()
{
    QMailMessage msg = fromFile("msg_40.txt");

    QVERIFY( msg.multipartType() == QMailMessage::MultipartNone );
    QVERIFY( msg.hasBody() );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("text/html") );

    QByteArray partText = "\
----961284236552522269\n\
Content-Type: text/html;\n\
Content-Transfer-Encoding: 7Bit\n\
\n\
<html></html>\n\
\n\
----961284236552522269--\n\
";

    QCOMPARE( msg.body().data(QMailMessageBody::Decoded), partText );
}

void tst_python_email::test_boundary_with_leading_space()
{
    QByteArray input = 
"MIME-Version: 1.0" CRLF
"Content-Type: multipart/mixed; boundary=\"    XXXX\"" CRLF
CRLF
"--    XXXX" CRLF
"Content-Type: text/plain" CRLF
CRLF
CRLF
"--    XXXX" CRLF
"Content-Type: text/plain" CRLF
CRLF
"--    XXXX--" CRLF;

    QMailMessage msg = QMailMessage::fromRfc2822(input);
    QVERIFY( msg.multipartType() == QMailMessage::MultipartMixed );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/mixed") );
    QVERIFY( msg.hasBody() == false );
    QCOMPARE( msg.contentType().boundary(), QByteArray("    XXXX") );

    QVERIFY( msg.hasBody() == false );
    // Note: we correctly parse this as only a single valid part, because the second has no
    // terminator for the header; the last CRLF is part of the boundary terminator.
    QVERIFY( msg.partCount() == 1 );
}

void tst_python_email::test_boundary_without_trailing_newline()
{
    QByteArray input = 
"Content-Type: multipart/mixed; boundary=\"===============0012394164==\"" CRLF
"MIME-Version: 1.0" CRLF
CRLF
"--===============0012394164==" CRLF
"Content-Type: image/file1.jpg" CRLF
"MIME-Version: 1.0" CRLF
"Content-Transfer-Encoding: base64" CRLF
CRLF
"YXNkZg==" CRLF
"--===============0012394164==--";

    QMailMessage msg = QMailMessage::fromRfc2822(input);

    QVERIFY( msg.multipartType() == QMailMessage::MultipartMixed );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/mixed") );
    QVERIFY( msg.hasBody() == false );
    QCOMPARE( msg.contentType().boundary(), QByteArray("===============0012394164==") );

    QVERIFY( msg.hasBody() == false );
    QVERIFY( msg.partCount() == 1 );
}

void tst_python_email::test_parse_missing_minor_type()
{
    QMailMessage msg = fromFile("msg_14.txt");

    QCOMPARE( msg.contentType().type(), QByteArray("text") );
    QCOMPARE( msg.contentType().subType(), QByteArray("plain") );
}

void tst_python_email::test_same_boundary_inner_outer()
{
    QMailMessage msg = fromFile("msg_15.txt");

    // Note: we can't parse this broken message correctly, but at least we shouldn't choke
    QVERIFY( msg.multipartType() == QMailMessage::MultipartMixed );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/mixed") );
    QVERIFY( msg.hasBody() == false );
    QVERIFY( msg.partCount() == 2 );
}

void tst_python_email::test_multipart_no_boundary()
{
    QMailMessage msg = fromFile("msg_25.txt");

    // Note: we can't parse this broken message correctly, but at least we shouldn't choke
    QVERIFY( msg.multipartType() == QMailMessage::MultipartReport );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/report") );
    QVERIFY( msg.hasBody() == false );
    QVERIFY( msg.partCount() == 0 );
}

void tst_python_email::test_invalid_content_type()
{
    QByteArray input = 
"Content-Type: text";

    QMailMessage msg = QMailMessage::fromRfc2822(input);
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("text/plain") );
    QCOMPARE( msg.contentType().type().toLower(), QByteArray("text") );
    QCOMPARE( msg.contentType().subType().toLower(), QByteArray("plain") );

    input = "foo";

    msg = QMailMessage::fromRfc2822(input);
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("text/plain") );
    QCOMPARE( msg.contentType().type().toLower(), QByteArray("text") );
    QCOMPARE( msg.contentType().subType().toLower(), QByteArray("plain") );
}

void tst_python_email::test_no_start_boundary()
{
    QMailMessage msg = fromFile("msg_31.txt");

    /* Note: it seems as though python, when finding no multipart body in the correct
       format, will insert the text as a plain body.  If anything, I think we should
       treat it as a preamble...

    QByteArray output = 
"--BOUNDARY" CRLF
"Content-Type: text/plain" CRLF
CRLF
"message 1" CRLF
CRLF
"--BOUNDARY" CRLF
"Content-Type: text/plain" CRLF
CRLF
"message 2" CRLF
CRLF
"--BOUNDARY--" CRLF;

    QByteArray bodyData = msg.body().data(QMailMessageBody::Decoded);
    QCOMPARE( bodyData, output );
    */
    QVERIFY( msg.multipartType() == QMailMessage::MultipartMixed );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/mixed") );
    QVERIFY( msg.hasBody() == false );
    QVERIFY( msg.partCount() == 0 );
}

void tst_python_email::test_no_separating_blank_line()
{
    QMailMessage msg = fromFile("msg_35.txt");

    QCOMPARE( msg.headerFieldText("From"), QString("aperson@domain.example") );
    QCOMPARE( msg.headerFieldText("To"), QString("bperson@domain.example") );
    QCOMPARE( msg.headerFieldText("Subject"), QString("here's something interesting") );

    // Note: python will add the last line as a body, which seems wrong - it
    // could also be a badly-formatted header field...
    QVERIFY( msg.hasBody() == false );
    QVERIFY( msg.partCount() == 0 );
}

void tst_python_email::test_lying_multipart()
{
    QMailMessage msg = fromFile("msg_41.txt");

    QVERIFY( msg.multipartType() == QMailMessage::MultipartAlternative );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/alternative") );
    QVERIFY( msg.hasBody() == false );
    QVERIFY( msg.partCount() == 0 );
}

void tst_python_email::test_missing_start_boundary()
{
    QMailMessage msg = fromFile("msg_42.txt");

    QVERIFY( msg.multipartType() == QMailMessage::MultipartMixed );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/mixed") );
    QVERIFY( msg.hasBody() == false );
    QVERIFY( msg.partCount() == 2 );

    QVERIFY( msg.partAt(0).multipartType() == QMailMessage::MultipartNone );
    QCOMPARE( msg.partAt(0).contentType().content().toLower(), QByteArray("text/plain") );
    QVERIFY( msg.partAt(0).hasBody() );
    QVERIFY( msg.partAt(0).partCount() == 0 );
    QVERIFY( msg.partAt(0).partNumber() == 0 );

    QVERIFY( msg.partAt(1).multipartType() == QMailMessage::MultipartNone );
    QCOMPARE( msg.partAt(1).contentType().content().toLower(), QByteArray("message/rfc822") );
    QVERIFY( msg.partAt(1).hasBody() == true );
    QVERIFY( msg.partAt(1).partCount() == 0 );
    QVERIFY( msg.partAt(1).partNumber() == 1 );

    QMailMessage subMessage = QMailMessage::fromRfc2822(msg.partAt(1).body().data(QMailMessageBody::Decoded));

    QVERIFY( subMessage.multipartType() == QMailMessage::MultipartMixed );
    QCOMPARE( subMessage.contentType().content().toLower(), QByteArray("multipart/mixed") );
    QVERIFY( subMessage.hasBody() == false );
    QVERIFY( subMessage.partCount() == 0 );
}

void tst_python_email::test_whitespace_eater_unicode()
{
    QByteArray input("=?ISO-8859-1?Q?Andr=E9?= Pirard <pirard@domain.example>");

    QByteArray output("Andr" "\xe9" " Pirard <pirard@domain.example>");

    QMailMessageHeaderField field("From", input);
    QCOMPARE( field.decodedContent(), QString::fromLatin1(output) );
}

void tst_python_email::test_whitespace_eater_unicode_2()
{
    QByteArray input("The =?iso-8859-1?b?cXVpY2sgYnJvd24gZm94?= jumped over the =?iso-8859-1?b?bGF6eSBkb2c=?=");

    QByteArray output("The quick brown fox jumped over the lazy dog");

    QMailMessageHeaderField field("Subject", input);
    QCOMPARE( field.decodedContent(), QString::fromLatin1(output) );
}

void tst_python_email::test_rfc2047_without_whitespace()
{
    QByteArray input("Sm=?ISO-8859-1?B?9g==?=rg=?ISO-8859-1?B?5Q==?=sbord");

    QMailMessageHeaderField field("Subject", input);
    QCOMPARE( field.decodedContent(), QString::fromLatin1(input) );
}

void tst_python_email::test_rfc2047_with_whitespace()
{
    QByteArray input("Sm =?ISO-8859-1?B?9g==?= rg =?ISO-8859-1?B?5Q==?= sbord");

    QByteArray output("Sm " "\xf6" " rg " "\xe5" " sbord");

    QMailMessageHeaderField field("Subject", input);
    QCOMPARE( field.decodedContent(), QString::fromLatin1(output) );
}

void tst_python_email::test_generate()
{
    QMailMessageContentType type("text/plain");

    QByteArray input("Here is the body of the message." CRLF);

    QMailMessage subMessage;
    subMessage.setHeaderField("Subject", "An enclosed message");
    subMessage.setBody(QMailMessageBody::fromData(input, type, QMailMessageBody::SevenBit, QMailMessageBody::AlreadyEncoded));

    type = QMailMessageContentType("message/rfc822");

    QMailMessage msg;
    msg.setHeaderField("Subject", "The enclosing message");
    msg.setHeaderField("Date", "irrelevant");
    msg.setBody(QMailMessageBody::fromData(subMessage.toRfc2822(QMailMessage::IdentityFormat), type, QMailMessageBody::SevenBit, QMailMessageBody::AlreadyEncoded));

    QByteArray output = 
"Subject: The enclosing message" CRLF
"Date: irrelevant" CRLF
"Content-Type: message/rfc822" CRLF
"Content-Transfer-Encoding: 7bit" CRLF
"MIME-Version: 1.0" CRLF
CRLF
"Subject: An enclosed message" CRLF
"Content-Type: text/plain" CRLF
"Content-Transfer-Encoding: 7bit" CRLF
CRLF
"Here is the body of the message." CRLF;

    QCOMPARE( msg.toRfc2822(), output );
}

void tst_python_email::test_parse_message_rfc822()
{
    QMailMessage msg = fromFile("msg_11.txt");
    
    QCOMPARE( msg.multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("message/rfc822") );
    QVERIFY( msg.hasBody() );
    QVERIFY( msg.partCount() == 0 );
    QCOMPARE( msg.subject(), QString("The enclosing message") );

    QMailMessage subMessage = QMailMessage::fromRfc2822(msg.body().data(QMailMessageBody::Decoded));

    QCOMPARE( subMessage.multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( subMessage.contentType().content().toLower(), QByteArray("text/plain") );
    QVERIFY( subMessage.hasBody() );
    QVERIFY( subMessage.partCount() == 0 );
    QCOMPARE( subMessage.subject(), QString("An enclosed message") );
    QCOMPARE( subMessage.body().data(), QString("Here is the body of the message.\n") );
}

void tst_python_email::test_dsn()
{
    QMailMessage msg = fromFile("msg_16.txt");
    
    QCOMPARE( msg.multipartType(), QMailMessagePartContainer::MultipartReport );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/report") );
    QVERIFY( msg.hasBody() == false );
    QVERIFY( msg.partCount() == 3 );
    
    QByteArray output = "\
This report relates to a message you sent with the following header fields:\n\
\n\
  Message-id: <002001c144a6$8752e060$56104586@oxy.test>\n\
  Date: Sun, 23 Sep 2001 20:10:55 -0700\n\
  From: \"Ian T. Henry\" <henryi@oxy.test>\n\
  To: SoCal Raves <scr@scr.example.org>\n\
  Subject: [scr] yeah for Ians!!\n\
\n\
Your message cannot be delivered to the following recipients:\n\
\n\
  Recipient address: jangel1@cougar.noc.ucla.test\n\
  Reason: recipient reached disk quota\n\
\n\
";

    QCOMPARE( msg.partAt(0).multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( msg.partAt(0).contentType().content().toLower(), QByteArray("text/plain") );
    QVERIFY( msg.partAt(0).hasBody() );
    QVERIFY( msg.partAt(0).partCount() == 0 );
    QCOMPARE( msg.partAt(0).body().data(QMailMessageBody::Decoded), output );
    QVERIFY( msg.partAt(0).partNumber() == 0 );

    QCOMPARE( msg.partAt(1).multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( msg.partAt(1).contentType().content().toLower(), QByteArray("message/delivery-status") );
    QVERIFY( msg.partAt(1).hasBody() );
    QVERIFY( msg.partAt(1).partCount() == 0 );
    QVERIFY( msg.partAt(1).partNumber() == 1 );

    // Note: python has built-in support for RFC 1894 delivery-status messages
    // We don't; instead, we can parse it manually by using the bodies as message data
    QMailMessage status = QMailMessage::fromRfc2822(msg.partAt(1).body().data(QMailMessageBody::Decoded));

    QCOMPARE( status.multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( status.contentType().content().toLower(), QByteArray("text/plain") );
    QVERIFY( status.hasBody() );
    QVERIFY( status.partCount() == 0 );
    QCOMPARE( status.headerField("Original-Envelope-ID").content(), QByteArray("0GK500B4HD0888@cougar.noc.ucla.test") );
    QCOMPARE( status.headerField("Reporting-MTA", QMailMessageHeaderField::UnstructuredField).content(), QByteArray("dns; cougar.noc.ucla.test") );

    // We need the data in un-decoded form; decoding will convert the CRLFs to '\n', due
    // to the type being 'text/plain'.  We need the CRLF intact to treat it as a message.
    // This is a hack, however - the delivery status body is not truly a message, it just
    // re-uses the message header formatting grammar.
    QMailMessage statusData = QMailMessage::fromRfc2822(status.body().data(QMailMessageBody::Encoded));

    QCOMPARE( statusData.headerField("Action").content(), QByteArray("failed") );
    QCOMPARE( statusData.headerField("Original-Recipient", QMailMessageHeaderField::UnstructuredField).content(), QByteArray("rfc822;jangel1@cougar.noc.ucla.test") );

    QCOMPARE( msg.partAt(2).multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( msg.partAt(2).contentType().content().toLower(), QByteArray("message/rfc822") );
    QVERIFY( msg.partAt(2).hasBody() );
    QVERIFY( msg.partAt(2).partCount() == 0 );
    QVERIFY( msg.partAt(2).partNumber() == 2 );

    QMailMessage original = QMailMessage::fromRfc2822(msg.partAt(2).body().data(QMailMessageBody::Decoded));

    QCOMPARE( original.multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( original.contentType().content().toLower(), QByteArray("text/plain") );
    QVERIFY( original.hasBody() );
    QVERIFY( original.partCount() == 0 );
    QCOMPARE( original.subject(), QString("[scr] yeah for Ians!!") );
    QCOMPARE( original.headerFieldText("Message-ID"), QString("<002001c144a6$8752e060$56104586@oxy.test>") );
}

void tst_python_email::test_epilogue()
{
    // Python allows the client to set the preamble and epilogue of a 
    // multipart message. Since we don't, we will only test that the message
    // is correctly parsed here
    QMailMessage msg = fromFile("msg_21.txt");
    
    QCOMPARE( msg.multipartType(), QMailMessagePartContainer::MultipartMixed );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/mixed") );
    QVERIFY( msg.hasBody() == false );
    QVERIFY( msg.partCount() == 2 );

    QByteArray output = 
"This report relates to a message you sent with the following header fields:" CRLF
CRLF
"  Message-id: <002001c144a6$8752e060$56104586@oxy.test>" CRLF
"  Date: Sun, 23 Sep 2001 20:10:55 -0700" CRLF
"  From: \"Ian T. Henry\" <henryi@oxy.test>" CRLF
"  To: SoCal Raves <scr@scr.example.org>" CRLF
"  Subject: [scr] yeah for Ians!!" CRLF
CRLF
"Your message cannot be delivered to the following recipients:" CRLF
CRLF
"  Recipient address: jangel1@cougar.noc.ucla.test" CRLF
"  Reason: recipient reached disk quota" CRLF
CRLF;

    QCOMPARE( msg.partAt(0).multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( msg.partAt(0).contentType().content().toLower(), QByteArray("text/plain") );
    QVERIFY( msg.partAt(0).hasBody() );
    QVERIFY( msg.partAt(0).partCount() == 0 );
    QCOMPARE( msg.partAt(0).body().data(QMailMessageBody::Decoded), QByteArray("One") );
    QVERIFY( msg.partAt(0).partNumber() == 0 );

    QCOMPARE( msg.partAt(1).multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( msg.partAt(1).contentType().content().toLower(), QByteArray("text/plain") );
    QVERIFY( msg.partAt(1).hasBody() );
    QVERIFY( msg.partAt(1).partCount() == 0 );
    QCOMPARE( msg.partAt(1).body().data(QMailMessageBody::Decoded), QByteArray("Two") );
    QVERIFY( msg.partAt(1).partNumber() == 1 );
}

void tst_python_email::test_default_type()
{
    QMailMessage msg = fromFile("msg_30.txt");
    
    QCOMPARE( msg.multipartType(), QMailMessagePartContainer::MultipartDigest );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/digest") );
    QVERIFY( msg.hasBody() == false );
    QVERIFY( msg.partCount() == 2 );

    QCOMPARE( msg.partAt(0).multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( msg.partAt(0).contentType().content().toLower(), QByteArray("message/rfc822") );
    QVERIFY( msg.partAt(0).hasBody() );
    QVERIFY( msg.partAt(0).partCount() == 0 );
    QVERIFY( msg.partAt(0).partNumber() == 0 );

    QMailMessage subMessage = QMailMessage::fromRfc2822(msg.partAt(0).body().data(QMailMessageBody::Decoded));

    QCOMPARE( subMessage.multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( subMessage.contentType().content().toLower(), QByteArray("text/plain") );
    QVERIFY( subMessage.hasBody() );
    QVERIFY( subMessage.partCount() == 0 );

    QCOMPARE( msg.partAt(1).multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( msg.partAt(1).contentType().content().toLower(), QByteArray("message/rfc822") );
    QVERIFY( msg.partAt(1).hasBody() );
    QVERIFY( msg.partAt(1).partCount() == 0 );
    QVERIFY( msg.partAt(1).partNumber() == 1 );

    subMessage = QMailMessage::fromRfc2822(msg.partAt(1).body().data(QMailMessageBody::Decoded));

    QCOMPARE( subMessage.multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( subMessage.contentType().content().toLower(), QByteArray("text/plain") );
    QVERIFY( subMessage.hasBody() );
    QVERIFY( subMessage.partCount() == 0 );
}

void tst_python_email::test_default_type_with_explicit_container_type()
{
    QMailMessage msg = fromFile("msg_28.txt");
    
    QCOMPARE( msg.multipartType(), QMailMessagePartContainer::MultipartDigest );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/digest") );
    QVERIFY( msg.hasBody() == false );
    QVERIFY( msg.partCount() == 2 );

    QCOMPARE( msg.partAt(0).multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( msg.partAt(0).contentType().content().toLower(), QByteArray("message/rfc822") );
    QVERIFY( msg.partAt(0).hasBody() );
    QVERIFY( msg.partAt(0).partCount() == 0 );
    QVERIFY( msg.partAt(0).partNumber() == 0 );

    QMailMessage subMessage = QMailMessage::fromRfc2822(msg.partAt(0).body().data(QMailMessageBody::Decoded));

    QCOMPARE( subMessage.multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( subMessage.contentType().content().toLower(), QByteArray("text/plain") );
    QVERIFY( subMessage.hasBody() );
    QVERIFY( subMessage.partCount() == 0 );

    QCOMPARE( msg.partAt(1).multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( msg.partAt(1).contentType().content().toLower(), QByteArray("message/rfc822") );
    QVERIFY( msg.partAt(1).hasBody() );
    QVERIFY( msg.partAt(1).partCount() == 0 );
    QVERIFY( msg.partAt(1).partNumber() == 1 );

    subMessage = QMailMessage::fromRfc2822(msg.partAt(1).body().data(QMailMessageBody::Decoded));

    QCOMPARE( subMessage.multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( subMessage.contentType().content().toLower(), QByteArray("text/plain") );
    QVERIFY( subMessage.hasBody() );
    QVERIFY( subMessage.partCount() == 0 );
}

void tst_python_email::TestIdempotent_data()
{
    QTest::addColumn<QString>( "filename" );

    QTest::newRow("msg_01") << "msg_01.txt";
    QTest::newRow("msg_03") << "msg_03.txt";
    QTest::newRow("msg_04") << "msg_04.txt";
    QTest::newRow("msg_02") << "msg_02.txt";
    QTest::newRow("msg_27") << "msg_27.txt";
    QTest::newRow("msg_28") << "msg_28.txt";
    QTest::newRow("msg_06") << "msg_06.txt";
    QTest::newRow("msg_05") << "msg_05.txt";
    QTest::newRow("msg_16") << "msg_16.txt";
    QTest::newRow("msg_21") << "msg_21.txt";
    QTest::newRow("msg_23") << "msg_23.txt";
    QTest::newRow("msg_24") << "msg_24.txt";
    QTest::newRow("msg_31") << "msg_31.txt";
    QTest::newRow("msg_32") << "msg_32.txt";
    QTest::newRow("msg_33") << "msg_33.txt";
    QTest::newRow("msg_34") << "msg_34.txt";
    QTest::newRow("msg_12a") << "msg_12a.txt";
    QTest::newRow("msg_36") << "msg_36.txt";
    QTest::newRow("msg_05") << "msg_05.txt";
    QTest::newRow("msg_06") << "msg_06.txt";
}

void tst_python_email::TestIdempotent()
{
    // This sequence of python tests ensures that they can read in
    // a message, then write it out without any change.  We don't do this,
    // for various reasons:
    //  1. We add C-T and C-T-E header fields where they need to be defaulted
    //  2. We wrap long header lines differently to python
    //  3. We don't preserve the original state of folded parameters
    // Instead, we'll just load all these messages to ensure we don't choke
    QFETCH( QString, filename );

    QMailMessage msg = fromFile(filename);
    QVERIFY( !msg.contentType().content().isEmpty() );

    // This file is badly formatted:
    if (filename == "msg_31.txt")
        QVERIFY( !msg.hasBody() && msg.partCount() == 0 );
    else
        QVERIFY( msg.hasBody() || msg.partCount() != 0 );
}

void tst_python_email::test_crlf_separation()
{
    QMailMessage msg = fromFile("msg_26.txt");

    QCOMPARE( msg.multipartType(), QMailMessagePartContainer::MultipartMixed );
    QCOMPARE( msg.contentType().content().toLower(), QByteArray("multipart/mixed") );
    QVERIFY( msg.hasBody() == false );
    QVERIFY( msg.partCount() == 2 );

    QByteArray text("Simple email with attachment.\n\n");

    QCOMPARE( msg.partAt(0).multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( msg.partAt(0).contentType().content().toLower(), QByteArray("text/plain") );
    QVERIFY( msg.partAt(0).hasBody() == true );
    QVERIFY( msg.partAt(0).partCount() == 0 );
    QCOMPARE( msg.partAt(0).body().data(QMailMessageBody::Decoded), text );
    QVERIFY( msg.partAt(0).partNumber() == 0 );

    QCOMPARE( msg.partAt(1).multipartType(), QMailMessagePartContainer::MultipartNone );
    QCOMPARE( msg.partAt(1).contentType().content().toLower(), QByteArray("application/riscos") );
    QVERIFY( msg.partAt(1).hasBody() == true );
    QVERIFY( msg.partAt(1).partCount() == 0 );
    QVERIFY( msg.partAt(1).partNumber() == 1 );
}

void tst_python_email::test_rfc2231_get_param()
{
    QMailMessage msg = fromFile("msg_29.txt");

    QMailMessageHeaderField field(msg.headerField("Content-Type"));
    QVERIFY( field.isParameterEncoded("title") == true );

    QString output("This is even more ***fun*** isn't it!");
    QCOMPARE( QMailMessageHeaderField::decodeParameter(field.parameter("title")), output );
}

void tst_python_email::test_rfc2231_set_param()
{
    QMailMessage msg = fromFile("msg_01.txt");

    QMailMessageHeaderField field(msg.headerField("Content-Type"));
    field.setParameter("title", QMailMessageHeaderField::encodeParameter("This is even more ***fun*** isn't it!", "us-ascii", "en"));
    field.setParameterEncoded("title");
    msg.setHeaderField(field);

    QByteArray output = 
"Return-Path: <bbb@zzz.test>" CRLF
"Delivered-To: bbb@zzz.test" CRLF
"Received: by mail.zzz.test (Postfix, from userid 889)\tid 27CEAD38CC;" CRLF
" Fri,  4 May 2001 14:05:44 -0400 (EDT)" CRLF
"Content-Type: text/plain; charset=us-ascii;" CRLF
" title*0*=us-ascii'en'This%20is%20even%20more%20%2A%2A%2Afun%2A%2A%2A%20is;" CRLF
" title*1*=n%27t%20it%21" CRLF
"Content-Transfer-Encoding: 7bit" CRLF
"Message-ID: <15090.61304.110929.45684@aaa.zzz.test>" CRLF
"From: bbb@ddd.example (John X. Doe)" CRLF
"To: bbb@zzz.test" CRLF
"Subject: This is a test message" CRLF
"Date: Fri, 4 May 2001 14:05:44 -0400" CRLF
"MIME-Version: 1.0" CRLF
CRLF
CRLF
"Hi," CRLF
CRLF
"Do you like this message?" CRLF
CRLF
"-Me" CRLF;

    QCOMPARE( msg.toRfc2822(), output );
}

void tst_python_email::test_rfc2231_no_language_or_charset()
{
    QByteArray input =
"Content-Transfer-Encoding: 8bit" CRLF
"Content-Disposition: inline; filename=\"file____C__DOCUMENTS_20AND_20SETTINGS_FABIEN_LOCAL_20SETTINGS_TEMP_nsmail.htm\"" CRLF
"Content-Type: text/html; NAME*0=file____C__DOCUMENTS_20AND_20SETTINGS_FABIEN_LOCAL_20SETTINGS_TEM; NAME*1=P_nsmail.htm" CRLF
CRLF;

    QMailMessage msg = QMailMessage::fromRfc2822(input);
    QCOMPARE( msg.contentType().name(), QByteArray("file____C__DOCUMENTS_20AND_20SETTINGS_FABIEN_LOCAL_20SETTINGS_TEMP_nsmail.htm") );
}

void tst_python_email::test_rfc2231_no_language_or_charset_in_filename()
{
    QByteArray input =
"Content-Disposition: inline;" CRLF
"\tfilename*0*=\"''This%20is%20even%20more%20\";" CRLF
"\tfilename*1*=\"%2A%2A%2Afun%2A%2A%2A%20\";" CRLF
"\tfilename*2=\"is it not.pdf\"" CRLF
CRLF;

    QMailMessage msg = QMailMessage::fromRfc2822(input);
    QMailMessageContentDisposition disposition(msg.headerField("Content-Disposition"));

    QCOMPARE( disposition.filename(), QByteArray("''This%20is%20even%20more%20%2A%2A%2Afun%2A%2A%2A%20is it not.pdf") );
    QVERIFY( disposition.isParameterEncoded("filename") == true );
    QCOMPARE( QMailMessageHeaderField::decodeParameter(disposition.filename()), QString("This is even more ***fun*** is it not.pdf") );
}

void tst_python_email::test_rfc2231_partly_encoded()
{
    QByteArray input =
"Content-Disposition: inline;" CRLF
"\tfilename*0=\"''This%20is%20even%20more%20\";" CRLF
"\tfilename*1*=\"%2A%2A%2Afun%2A%2A%2A%20\";" CRLF
"\tfilename*2=\"is it not.pdf\"" CRLF
CRLF;

    // Note: python treats this differently by decoding only the second part - this appears to
    // be in contravention of RFC 2231 section 4.1.  My interpretation is that we must treat this
    // as not encoded, in entirety.

    QMailMessage msg = QMailMessage::fromRfc2822(input);
    QMailMessageContentDisposition disposition(msg.headerField("Content-Disposition"));

    QCOMPARE( disposition.filename(), QByteArray("''This%20is%20even%20more%20%2A%2A%2Afun%2A%2A%2A%20is it not.pdf") );
    QVERIFY( disposition.isParameterEncoded("filename") == false );

    // Although it is marked as not encoded, we can decode it:
    QCOMPARE( QMailMessageHeaderField::decodeParameter(disposition.filename()), QString("This is even more ***fun*** is it not.pdf") );
}

void tst_python_email::test_rfc2231_no_language_or_charset_in_boundary()
{
    QByteArray input =
"Content-Type: multipart/alternative;" CRLF
"\tboundary*0*=\"''This%20is%20even%20more%20\";" CRLF
"\tboundary*1*=\"%2A%2A%2Afun%2A%2A%2A%20\";" CRLF
"\tboundary*2=\"is it not.pdf\"" CRLF
CRLF;

    QMailMessage msg = QMailMessage::fromRfc2822(input);

    QCOMPARE( msg.contentType().parameter("boundary"), QByteArray("''This%20is%20even%20more%20%2A%2A%2Afun%2A%2A%2A%20is it not.pdf") );
    QVERIFY( msg.contentType().isParameterEncoded("boundary") == true );
    QCOMPARE( msg.contentType().boundary(), QByteArray("This is even more ***fun*** is it not.pdf") );
}

void tst_python_email::test_rfc2231_no_language_or_charset_in_charset()
{
    QByteArray input =
"Content-Type: text/plain;" CRLF
"\tcharset*0*=\"This%20is%20even%20more%20\";" CRLF
"\tcharset*1*=\"%2A%2A%2Afun%2A%2A%2A%20\";" CRLF
"\tcharset*2=\"is it not.pdf\"" CRLF
CRLF;

    QMailMessage msg = QMailMessage::fromRfc2822(input);

    QCOMPARE( msg.contentType().parameter("charset"), QByteArray("This%20is%20even%20more%20%2A%2A%2Afun%2A%2A%2A%20is it not.pdf") );
    QVERIFY( msg.contentType().isParameterEncoded("charset") == true );

    // Charset must contain only ASCII, so when requested, we need to decode it
    QCOMPARE( msg.contentType().charset(), QByteArray("This is even more ***fun*** is it not.pdf") );

    // Note: I'm ambivalent about this one, the RFC specifies that it is illegal.
    // Python will decode it into ascii, which seems ok.  I think we will decode it
    // also, but be on the lookout for consequences!
    QCOMPARE( QMailMessageHeaderField::decodeParameter(msg.contentType().charset()), QString("This is even more ***fun*** is it not.pdf") );
}

void tst_python_email::test_rfc2231_bad_encoding_in_filename()
{
    QByteArray input =
"Content-Disposition: inline;" CRLF
"\tfilename*0*=\"bogus'xx'This%20is%20even%20more%20\";" CRLF
"\tfilename*1*=\"%2A%2A%2Afun%2A%2A%2A%20\";" CRLF
"\tfilename*2=\"is it not.pdf\"" CRLF
CRLF;

    QMailMessage msg = QMailMessage::fromRfc2822(input);
    QMailMessageContentDisposition disposition(msg.headerField("Content-Disposition"));

    // Like python, we will extract this to ASCII when the charset is unknown...
    QCOMPARE( disposition.filename(), QByteArray("bogus'xx'This%20is%20even%20more%20%2A%2A%2Afun%2A%2A%2A%20is it not.pdf") );
    QCOMPARE( QMailMessageHeaderField::decodeParameter(disposition.filename()), QString("This is even more ***fun*** is it not.pdf") );
}

void tst_python_email::test_rfc2231_bad_encoding_in_charset()
{
    QByteArray input =
"Content-Type: text/plain; charset*=bogus''utf-8%E2%80%9D" CRLF
CRLF;

    QMailMessage msg = QMailMessage::fromRfc2822(input);

    // Note: python returns an error here; we return only the valid characters
    QCOMPARE( msg.contentType().parameter("charset"), QByteArray("bogus''utf-8%E2%80%9D") );
    QCOMPARE( QMailMessageHeaderField::decodeParameter(msg.contentType().charset()), QString("utf-8") );
    QCOMPARE( msg.contentType().charset(), QByteArray("utf-8") );
}

void tst_python_email::test_rfc2231_bad_character_in_charset()
{
    QByteArray input =
"Content-Type: text/plain; charset*=ascii''utf-8%E2%80%9D" CRLF
CRLF;

    QMailMessage msg = QMailMessage::fromRfc2822(input);

    // Note: python returns an error here; we return the invalid characters...
    QCOMPARE( msg.contentType().parameter("charset"), QByteArray("ascii''utf-8%E2%80%9D") );
    QCOMPARE( QMailMessageHeaderField::decodeParameter(msg.contentType().charset()), QString("utf-8") );
    QCOMPARE( msg.contentType().charset(), QByteArray("utf-8") );
}

void tst_python_email::test_rfc2231_bad_character_in_filename()
{
    QByteArray input =
"Content-Disposition: inline;" CRLF
"\tfilename*0*=\"ascii'xx'This%20is%20even%20more%20\";" CRLF
"\tfilename*1*=\"%2A%2A%2Afun%2A%2A%2A%20\";" CRLF
"\tfilename*2*=\"is it not.pdf%E2\"" CRLF
CRLF;

    QMailMessage msg = QMailMessage::fromRfc2822(input);
    QMailMessageContentDisposition disposition(msg.headerField("Content-Disposition"));

    // Note: python returns an marker character; we return the invalid characters...
    QCOMPARE( disposition.filename(), QByteArray("ascii'xx'This%20is%20even%20more%20%2A%2A%2Afun%2A%2A%2A%20is it not.pdf%E2") );
    QCOMPARE( QMailMessageHeaderField::decodeParameter(disposition.filename()), QString("This is even more ***fun*** is it not.pdf") );
}

void tst_python_email::test_rfc2231_single_tick_in_filename_extended()
{
    QByteArray input =
"Content-Type: application/x-foo;" CRLF
"\tname*0*=\"Frank's\"; name*1*=\" Document\"" CRLF
CRLF;

    QMailMessage msg = QMailMessage::fromRfc2822(input);

    QCOMPARE( msg.contentType().name(), QByteArray("Frank's Document") );
    QCOMPARE( QMailMessageHeaderField::decodeParameter(msg.contentType().name()), QString("Frank's Document") );
}

void tst_python_email::test_rfc2231_tick_attack_extended()
{
    QByteArray input =
"Content-Type: application/x-foo;" CRLF
"\tname*0*=\"us-ascii'en-us'Frank's\"; name*1*=\" Document\"" CRLF
CRLF;

    QMailMessage msg = QMailMessage::fromRfc2822(input);

    QCOMPARE( msg.contentType().name(), QByteArray("us-ascii'en-us'Frank's Document") );
    QCOMPARE( QMailMessageHeaderField::decodeParameter(msg.contentType().name()), QString("Frank's Document") );
}

void tst_python_email::test_rfc2231_no_extended_values()
{
    QByteArray input =
"Content-Type: application/x-foo; name=\"Frank's Document\"" CRLF
CRLF;

    QMailMessage msg = QMailMessage::fromRfc2822(input);
    QCOMPARE( msg.contentType().name(), QByteArray("Frank's Document") );
}

void tst_python_email::test_rfc2231_encoded_then_unencoded_segments()
{
    QByteArray input =
"Content-Type: application/x-foo;" CRLF
"\tname*0*=\"us-ascii'en-us'My\";" CRLF
"\tname*1=\" Document\";" CRLF
"\tname*2*=\" For You\"" CRLF
CRLF;

    QMailMessage msg = QMailMessage::fromRfc2822(input);

    QCOMPARE( msg.contentType().name(), QByteArray("us-ascii'en-us'My Document For You") );
    QVERIFY( msg.contentType().isParameterEncoded("name") == true );
    QCOMPARE( QMailMessageHeaderField::decodeParameter(msg.contentType().name()), QString("My Document For You") );
}

void tst_python_email::test_rfc2231_unencoded_then_encoded_segments()
{
    QByteArray input =
"Content-Type: application/x-foo;" CRLF
"\tname*0=\"us-ascii'en-us'My\";" CRLF
"\tname*1*=\" Document\";" CRLF
"\tname*2*=\" For You\"" CRLF
CRLF;

    QMailMessage msg = QMailMessage::fromRfc2822(input);

    QCOMPARE( msg.contentType().name(), QByteArray("us-ascii'en-us'My Document For You") );

    // Note: we will report this parameter as unencoded - python will mark it encoded...
    QVERIFY( msg.contentType().isParameterEncoded("name") == false );
    QCOMPARE( QMailMessageHeaderField::decodeParameter(msg.contentType().name()), QString("My Document For You") );
}

