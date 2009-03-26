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
#include <QDataStream>
#include <qmailmessage.h>
#include <QMailTimeStamp>

/* 
Note: Any email addresses appearing in this test data must be example addresses,
as defined by RFC 2606.  Therefore, they should use one of the following domains:
    *.example.{com|org|net}
    *.test
    *.example
*/

// RFC 2822 messages use CRLF as the newline indicator
#define CRLF "\015\012"

class QMailMessageHeaderPrivate
{
public:
    // This class is a friend
    static QByteArray extract(const QMailMessageHeaderField& field)
    {
        QByteArray result;
        {
            QDataStream out(&result, QIODevice::WriteOnly);
            field.output(out);
        }

        return result;
    }
};

static QByteArray asRfc2822(const QByteArray& id, const QByteArray& content)
{
    // Only unstructured fields can be 'output', whether they contain structured
    // or unstructured content
    QMailMessageHeaderField unstructured(id, content, QMailMessageHeaderField::UnstructuredField);
    return QMailMessageHeaderPrivate::extract(unstructured);
}

static QByteArray asRfc2822(const QMailMessageHeaderField& field)
{
    return asRfc2822(field.id(), field.toString(false, false));
}


//TESTED_CLASS=QMailMessageHeaderField
//TESTED_FILES=src/libraries/qtopiamail/qmailmessage.cpp

/*
    Dummy test matching test filename.
*/
class tst_QMailMessageHeader : public QObject
{
    Q_OBJECT
};

/*
    This class primarily tests that QMailMessageHeaderField correctly parses
    and constructs header field strings.
*/
class tst_QMailMessageHeaderField : public QObject
{
    Q_OBJECT

public:
    tst_QMailMessageHeaderField();
    virtual ~tst_QMailMessageHeaderField();

private slots:
    virtual void initTestCase();
    virtual void cleanupTestCase();
    virtual void init();
    virtual void cleanup();

    void constructor1_data();
    void constructor1();
    void constructor2_data();
    void constructor2();
    void id();
    void setId();
    void content();
    void setContent();
    void parameter();
    void setParameter();
    void isParameterEncoded();
    void setParameterEncoded();
    void parameters();
    void toString();
    void decodedContent_data();
    void decodedContent();
    void encodeWord_data();
    void encodeWord();
    void decodeWord();
    void encodeParameter_data();
    void encodeParameter();
    void decodeParameter();
    void encodeContent_data();
    void encodeContent();
    void decodeContent();
    void removeComments_data();
    void removeComments();
    void removeWhitespace_data();
    void removeWhitespace();
    void output_data();
    void output();
};

class tst_QMailMessageContentType : public QObject
{
    Q_OBJECT

public:
    tst_QMailMessageContentType();
    virtual ~tst_QMailMessageContentType();

private slots:
    void constructor_data();
    void constructor();
    void type();
    void setType();
    void subType();
    void setSubType();
    void name();
    void setName();
};

class tst_QMailMessageContentDisposition : public QObject
{
    Q_OBJECT

public:
    tst_QMailMessageContentDisposition();
    virtual ~tst_QMailMessageContentDisposition();

private slots:
    void constructor_data();
    void constructor();
    void type();
    void setType();
    void filename();
    void setFilename();
    void creationDate();
    void setCreationDate();
    void modificationDate();
    void setModificationDate();
    void readDate();
    void setReadDate();
    void size();
    void setSize();
};

int main(int argc, char *argv[])
{
    QtopiaApplication app(argc, argv);

    tst_QMailMessageHeader dummy;
    tst_QMailMessageHeaderField messageHeader;
    tst_QMailMessageContentType contentType;
    tst_QMailMessageContentDisposition contentDisposition;

    QTest::qExec(&dummy, argc, argv);
    QTest::qExec(&messageHeader, argc, argv);
    QTest::qExec(&contentType, argc, argv);
    QTest::qExec(&contentDisposition, argc, argv);

    return 0;
}

#include "tst_qmailmessageheader.moc"

Q_DECLARE_METATYPE(QMailMessageHeaderField::ParameterType)
Q_DECLARE_METATYPE(QList<QMailMessageHeaderField::ParameterType>)
Q_DECLARE_METATYPE(QMailMessageHeaderField::FieldType)

Q_DECLARE_METATYPE(QMailMessageContentDisposition::DispositionType)

Q_DECLARE_METATYPE(QMailTimeStamp)

Q_DECLARE_METATYPE(QList<QByteArray>)


tst_QMailMessageHeaderField::tst_QMailMessageHeaderField()
{
}

tst_QMailMessageHeaderField::~tst_QMailMessageHeaderField()
{
}

void tst_QMailMessageHeaderField::initTestCase()
{
}

void tst_QMailMessageHeaderField::cleanupTestCase()
{
}

void tst_QMailMessageHeaderField::init()
{
}

void tst_QMailMessageHeaderField::cleanup()
{
}

// QMailMessageHeaderField::QMailMessageHeaderField(const QByteArray& text, FieldType)
void tst_QMailMessageHeaderField::constructor1_data()
{
    QTest::addColumn<QByteArray>("text");
    QTest::addColumn<QMailMessageHeaderField::FieldType>("field_type");
    QTest::addColumn<QByteArray>("id");
    QTest::addColumn<QByteArray>("content");
    QTest::addColumn<QList<QMailMessageHeaderField::ParameterType> >("parameters");
    QTest::addColumn<QByteArray>("to_string");

    QTest::newRow("Empty") 
        << QByteArray()
        << QMailMessageHeaderField::StructuredField
        << QByteArray()
        << QByteArray()
        << QList<QMailMessageHeaderField::ParameterType>()
        << QByteArray();

    QTest::newRow("Id only") 
        << QByteArray("Content-Type")
        << QMailMessageHeaderField::StructuredField
        << QByteArray("Content-Type")
        << QByteArray()
        << QList<QMailMessageHeaderField::ParameterType>()
        << QByteArray("Content-Type:");

    QTest::newRow("Id and content") 
        << QByteArray("Content-Type: text/html")
        << QMailMessageHeaderField::StructuredField
        << QByteArray("Content-Type")
        << QByteArray("text/html")
        << QList<QMailMessageHeaderField::ParameterType>()
        << QByteArray("Content-Type: text/html");

    QTest::newRow("With parameter") 
        << QByteArray("Content-Type: text/html; charset=UTF-8")
        << QMailMessageHeaderField::StructuredField
        << QByteArray("Content-Type")
        << QByteArray("text/html")
        << ( QList<QMailMessageHeaderField::ParameterType>() 
                << qMakePair(QByteArray("charset"), QByteArray("UTF-8")) )
        << QByteArray("Content-Type: text/html; charset=UTF-8");

    QTest::newRow("With parameter - unstructured") 
        << QByteArray("Content-Type: text/html; charset=UTF-8")
        << QMailMessageHeaderField::UnstructuredField
        << QByteArray("Content-Type")
        << QByteArray("text/html; charset=UTF-8")
        << QList<QMailMessageHeaderField::ParameterType>()
        << QByteArray("Content-Type: text/html; charset=UTF-8");

    QTest::newRow("With multiple parameters") 
        << QByteArray("Content-Type: text/html; charset=UTF-8; sample=simple")
        << QMailMessageHeaderField::StructuredField
        << QByteArray("Content-Type")
        << QByteArray("text/html")
        << ( QList<QMailMessageHeaderField::ParameterType>() 
                << qMakePair(QByteArray("charset"), QByteArray("UTF-8")) 
                << qMakePair(QByteArray("sample"), QByteArray("simple")) )
        << QByteArray("Content-Type: text/html; charset=UTF-8; sample=simple");

    QTest::newRow("With quoted parameters") 
        << QByteArray("Content-Type: text/html; quoted=\"needs quotes\"; unquoted=\"doesn't-need-quotes\"")
        << QMailMessageHeaderField::StructuredField
        << QByteArray("Content-Type")
        << QByteArray("text/html")
        << ( QList<QMailMessageHeaderField::ParameterType>() 
                << qMakePair(QByteArray("quoted"), QByteArray("needs quotes"))
                << qMakePair(QByteArray("unquoted"), QByteArray("doesn't-need-quotes")) )
        << QByteArray("Content-Type: text/html; quoted=\"needs quotes\"; unquoted=doesn't-need-quotes");

    QTest::newRow("With quoted parameters - unstructured") 
        << QByteArray("Content-Type: text/html; quoted=\"needs quotes\"; unquoted=\"doesn't-need-quotes\"")
        << QMailMessageHeaderField::UnstructuredField
        << QByteArray("Content-Type")
        << QByteArray("text/html; quoted=\"needs quotes\"; unquoted=\"doesn't-need-quotes\"")
        << QList<QMailMessageHeaderField::ParameterType>()
        << QByteArray("Content-Type: text/html; quoted=\"needs quotes\"; unquoted=\"doesn't-need-quotes\"");

    QTest::newRow("With multiple parameters - whitespace") 
        << QByteArray(" \tContent-Type:text/html;charset=UTF-8  ;   sample=simple;\n\tsimple=\r\r sample\t  ")
        << QMailMessageHeaderField::StructuredField
        << QByteArray("Content-Type")
        << QByteArray("text/html")
        << ( QList<QMailMessageHeaderField::ParameterType>() 
                << qMakePair(QByteArray("charset"), QByteArray("UTF-8"))
                << qMakePair(QByteArray("sample"), QByteArray("simple"))
                << qMakePair(QByteArray("simple"), QByteArray("sample")) )
        << QByteArray("Content-Type: text/html; charset=UTF-8; sample=simple; simple=sample");

    QTest::newRow("With multiple parameters - whitespace unstructured") 
        << QByteArray(" \tContent-Type:text/html;charset=UTF-8  ;   sample=simple;\n\tsimple=\r\r sample\t  ")
        << QMailMessageHeaderField::UnstructuredField
        << QByteArray("Content-Type")
        << QByteArray("text/html;charset=UTF-8  ;   sample=simple;\n\tsimple=\r\r sample")
        << QList<QMailMessageHeaderField::ParameterType>()
        << QByteArray("Content-Type: text/html;charset=UTF-8  ;   sample=simple;\n\tsimple=\r\r sample");
}

void tst_QMailMessageHeaderField::constructor1()
{
    QFETCH( QByteArray, text ); 
    QFETCH( QMailMessageHeaderField::FieldType, field_type ); 

    QMailMessageHeaderField header(text, field_type);
    QTEST( header.id(), "id" );
    QTEST( header.content(), "content" );
    QTEST( header.parameters(), "parameters" );
    QTEST( header.toString(), "to_string" );
}

// QMailMessageHeaderField::QMailMessageHeaderField(const QByteArray& name, const QByteArray& text, FieldType)
void tst_QMailMessageHeaderField::constructor2_data()
{
    QTest::addColumn<QByteArray>("text");
    QTest::addColumn<QByteArray>("id");
    QTest::addColumn<QByteArray>("content");
    QTest::addColumn<QList<QMailMessageHeaderField::ParameterType> >("parameters");
    QTest::addColumn<QByteArray>("to_string");

    QTest::newRow("Empty") 
        << QByteArray()
        << QByteArray()
        << QByteArray()
        << QList<QMailMessageHeaderField::ParameterType>()
        << QByteArray();

    QTest::newRow("Id only") 
        << QByteArray()
        << QByteArray("Content-Type")
        << QByteArray()
        << QList<QMailMessageHeaderField::ParameterType>()
        << QByteArray("Content-Type:");

    QTest::newRow("Differing Id") 
        << QByteArray("Context-Hype:")
        << QByteArray("Content-Type")
        << QByteArray()
        << QList<QMailMessageHeaderField::ParameterType>()
        << QByteArray("Content-Type:");

    QTest::newRow("Id and content") 
        << QByteArray("text/html")
        << QByteArray("Content-Type")
        << QByteArray("text/html")
        << QList<QMailMessageHeaderField::ParameterType>()
        << QByteArray("Content-Type: text/html");

    QTest::newRow("Duplicated Id and content") 
        << QByteArray("Content-Type: text/html")
        << QByteArray("Content-Type")
        << QByteArray("text/html")
        << QList<QMailMessageHeaderField::ParameterType>()
        << QByteArray("Content-Type: text/html");

    QTest::newRow("Differing Id and content") 
        << QByteArray("Context-Hype: text/html")
        << QByteArray("Content-Type")
        << QByteArray("text/html")
        << QList<QMailMessageHeaderField::ParameterType>()
        << QByteArray("Content-Type: text/html");

    QTest::newRow("With parameter") 
        << QByteArray("text/html; charset=UTF-8")
        << QByteArray("Content-Type")
        << QByteArray("text/html")
        << ( QList<QMailMessageHeaderField::ParameterType>() 
                << qMakePair(QByteArray("charset"), QByteArray("UTF-8")) )
        << QByteArray("Content-Type: text/html; charset=UTF-8");

    QTest::newRow("With multiple parameters") 
        << QByteArray("text/html; charset=UTF-8; sample=simple")
        << QByteArray("Content-Type")
        << QByteArray("text/html")
        << ( QList<QMailMessageHeaderField::ParameterType>() 
                << qMakePair(QByteArray("charset"), QByteArray("UTF-8")) 
                << qMakePair(QByteArray("sample"), QByteArray("simple")) )
        << QByteArray("Content-Type: text/html; charset=UTF-8; sample=simple");
}

void tst_QMailMessageHeaderField::constructor2()
{
    QFETCH( QByteArray, text ); 
    QFETCH( QByteArray, id ); 

    QMailMessageHeaderField header(id, text);

    QCOMPARE( header.id(), id );
    QTEST( header.content(), "content" );
    QTEST( header.parameters(), "parameters" );
    QTEST( header.toString(), "to_string" );
}

void tst_QMailMessageHeaderField::id()
{
    // Tested-by: setId
}

void tst_QMailMessageHeaderField::setId()
{
    QMailMessageHeaderField header1;
    QMailMessageHeaderField header2("Content-Type: text/plain; charset=UTF-8; sample=simple");
    
    QCOMPARE( header1.id(), QByteArray() );
    QCOMPARE( header2.id(), QByteArray("Content-Type") );

    QByteArray id("Albert");

    header1.setId(id);
    QCOMPARE( header1.id(), id );

    header2.setId(id);
    QCOMPARE( header2.id(), id );
}

void tst_QMailMessageHeaderField::content()
{
    // Tested-by: setContent
}

void tst_QMailMessageHeaderField::setContent()
{
    QMailMessageHeaderField header1;
    QMailMessageHeaderField header2("Content-Type: text/plain; charset=UTF-8; sample=simple");
    
    QCOMPARE( header1.content(), QByteArray() );
    QCOMPARE( header2.content(), QByteArray("text/plain") );

    QByteArray content("image/jpeg");

    header1.setContent(content);
    QCOMPARE( header1.content(), content );

    header2.setContent(content);
    QCOMPARE( header2.content(), content );

    // This test caused a failure where a CRLF-pair was encoded into the output
    QByteArray longContent =
"There are quite a lot of places in the code that use qApp->desktop->width() \n"
"and height() to calculate the size of widgets.\n"
"\n"
"Some devices will have two QScreens, one for the LCD and the other for the TV \n"
"output.  The \"desktop\" is the union of both screens, not just the first \n"
"screen, which causes widgets to get resized incorrectly on the LCD.  To see a \n"
"demo, try running World Time in a multi screen skin in qvfb.\n"
"\n"
"The correct value to use is that from QDesktopWidget::availableGeometry() or \n"
"QDesktopWidget::screenGeometry() for the primary screen, not the desktop \n"
"geometry.\n";

    header1.setId("Subject");
    header1.setContent(longContent);
    QCOMPARE( asRfc2822(header1), QByteArray(
"Subject: There are quite a lot of places in the code that use" CRLF
" qApp->desktop->width() \nand height() to calculate the size of widgets.\n\nSome" CRLF
" devices will have two QScreens, one for the LCD and the other for the TV " CRLF
"\noutput.  The \"desktop\" is the union of both screens, not just the first " CRLF
"\nscreen, which causes widgets to get resized incorrectly on the LCD.  To see" CRLF
" a \ndemo, try running World Time in a multi screen skin in qvfb.\n\nThe correct" CRLF
" value to use is that from QDesktopWidget::availableGeometry() or " CRLF
"\nQDesktopWidget::screenGeometry() for the primary screen, not the desktop " CRLF
"\ngeometry." CRLF) );
}

void tst_QMailMessageHeaderField::parameter()
{
    // Tested-by: setParameter
}

void tst_QMailMessageHeaderField::setParameter()
{
    QByteArray oversize = 
"This is a really long parameter value that will need to be partitioned into"
" multiple pieces for transmission, using the RFC 2231 encoding marking"
" and not exceeding 78 characters per line.";

    QMailMessageHeaderField header1;
    QMailMessageHeaderField header2("Content-Type: text/plain; charset=UTF-8; sample=simple");
    QMailMessageHeaderField header3(QByteArray("Content-Type: application/octet-stream; charset=") + oversize);
    
    QByteArray name("charset");

    QCOMPARE( header1.parameter(name), QByteArray() );
    QVERIFY( header1.isParameterEncoded(name) == false );
    QCOMPARE( header2.parameter(name), QByteArray("UTF-8") );
    QVERIFY( header2.isParameterEncoded(name) == false );
    QCOMPARE( header3.parameter(name), oversize );
    QVERIFY( header3.isParameterEncoded(name) == false );

    QByteArray parameter("ISO-8859-1");

    header1.setParameter(name, parameter);
    header2.setParameter(name, parameter);
    header3.setParameter(name, parameter);
    QCOMPARE( header1.parameter(name), parameter );
    QVERIFY( header1.isParameterEncoded(name) == false );
    QCOMPARE( header2.parameter(name), parameter );
    QVERIFY( header2.isParameterEncoded(name) == false );
    QCOMPARE( header3.parameter(name), parameter );
    QVERIFY( header3.isParameterEncoded(name) == false );

    name = "sample";
    parameter = "wimple";

    header1.setParameter(name + '*', parameter);
    header2.setParameter(name + '*', parameter);
    header3.setParameter(name + '*', oversize);
    QCOMPARE( header1.parameter(name), parameter );
    QVERIFY( header1.isParameterEncoded(name) == true );
    QCOMPARE( header2.parameter(name), parameter );
    QVERIFY( header2.isParameterEncoded(name) == true );
    QCOMPARE( header3.parameter(name), oversize );
    QVERIFY( header3.isParameterEncoded(name) == true );

    name = "quoted";
    parameter = "needs quotes";

    header1.setParameter(name, parameter);
    header2.setParameter(name, parameter);
    header3.setParameter(name, oversize);
    QCOMPARE( header1.parameter(name), parameter );
    QVERIFY( header1.isParameterEncoded(name) == false );
    QCOMPARE( header2.parameter(name), parameter );
    QVERIFY( header2.isParameterEncoded(name) == false );
    QCOMPARE( header3.parameter(name), oversize );
    QVERIFY( header3.isParameterEncoded(name) == false );
}

void tst_QMailMessageHeaderField::isParameterEncoded()
{
    // Tested-by: setParameterEncoded()
}

void tst_QMailMessageHeaderField::setParameterEncoded()
{
    QMailMessageHeaderField header1;
    QMailMessageHeaderField header2("Content-Type: text/plain; charset=UTF-8; sample=simple");
    
    QByteArray name("charset");

    QVERIFY( header1.isParameterEncoded(name) == false );

    header1.setParameterEncoded(name);
    QVERIFY( header1.isParameterEncoded(name) == false );

    QVERIFY( header2.isParameterEncoded(name) == false );
    QCOMPARE( header2.toString(), QByteArray("Content-Type: text/plain; charset=UTF-8; sample=simple") );
    QCOMPARE( asRfc2822(header2), QByteArray(
"Content-Type: text/plain; charset=UTF-8; sample=simple" CRLF) );

    header2.setParameterEncoded(name);
    QVERIFY( header2.isParameterEncoded(name) == true );
    QCOMPARE( header2.toString(), QByteArray("Content-Type: text/plain; charset=UTF-8; sample=simple") );
    QCOMPARE( asRfc2822(header2), QByteArray(
"Content-Type: text/plain; charset*=UTF-8; sample=simple" CRLF) );

    header2.setParameterEncoded(name);
    QVERIFY( header2.isParameterEncoded(name) == true );
    QCOMPARE( header2.toString(), QByteArray("Content-Type: text/plain; charset=UTF-8; sample=simple") );
    QCOMPARE( asRfc2822(header2), QByteArray(
"Content-Type: text/plain; charset*=UTF-8; sample=simple" CRLF) );

    header2.setParameter(name, "Very long charset value which will take up multiple lines in the RFC 2822 output");
    QVERIFY( header2.isParameterEncoded(name) == false );
    QCOMPARE( header2.toString(), QByteArray("Content-Type: text/plain; charset=\"Very long charset value which will take up multiple lines in the RFC 2822 output\"; sample=simple") );
    QCOMPARE( asRfc2822(header2), QByteArray(
"Content-Type: text/plain;" CRLF
" charset*0=\"Very long charset value which will take up multiple lines in t\";" CRLF
" charset*1=\"he RFC 2822 output\"; sample=simple" CRLF) );

    header2.setParameterEncoded(name);
    QVERIFY( header2.isParameterEncoded(name) == true );
    QCOMPARE( header2.toString(), QByteArray("Content-Type: text/plain; charset=\"Very long charset value which will take up multiple lines in the RFC 2822 output\"; sample=simple") );
    QCOMPARE( asRfc2822(header2), QByteArray(
"Content-Type: text/plain;" CRLF
" charset*0*=\"Very long charset value which will take up multiple lines in t\";" CRLF
" charset*1*=\"he RFC 2822 output\"; sample=simple" CRLF) );
}

void tst_QMailMessageHeaderField::parameters()
{
    QMailMessageHeaderField header1;
    QMailMessageHeaderField header2("Content-Type: text/plain; charset=UTF-8; sample=simple");
    
    QList<QMailMessageHeaderField::ParameterType> result1, result2;

    QByteArray name("charset");
    QByteArray parameter("ISO-8859-1");
    result1.append(qMakePair(name, parameter));
    result2.append(qMakePair(name, parameter));
    result2.append(qMakePair(QByteArray("sample"), QByteArray("simple")));

    header1.setParameter(name, parameter);
    header2.setParameter(name, parameter);
    QCOMPARE( header1.parameters(), result1 );
    QCOMPARE( header2.parameters(), result2 );

    // Ensure parameters are appended rather than prepended
    name = "quoted";
    parameter = "needs quotes";
    result1.append(qMakePair(name, parameter));
    result2.append(qMakePair(name, parameter));

    header1.setParameter(name, parameter);
    header2.setParameter(name, parameter);
    QCOMPARE( header1.parameters(), result1 );
    QCOMPARE( header2.parameters(), result2 );

    // Ensure parameters are quoted when necessary
    header2.setParameter("sample", QByteArray("needs\tquotes"));
    QCOMPARE( header2.toString(), QByteArray("Content-Type: text/plain; charset=ISO-8859-1; sample=\"needs\tquotes\"; quoted=\"needs quotes\"") );

    header2.setParameter("sample", QByteArray("needs(quotes"));
    header2.setParameter("quoted", QByteArray("needs)quotes"));
    QCOMPARE( header2.toString(), QByteArray("Content-Type: text/plain; charset=ISO-8859-1; sample=\"needs(quotes\"; quoted=\"needs)quotes\"") );

    header2.setParameter("sample", QByteArray("needs<quotes"));
    header2.setParameter("quoted", QByteArray("needs>quotes"));
    QCOMPARE( header2.toString(), QByteArray("Content-Type: text/plain; charset=ISO-8859-1; sample=\"needs<quotes\"; quoted=\"needs>quotes\"") );

    header2.setParameter("sample", QByteArray("needs[quotes"));
    header2.setParameter("quoted", QByteArray("needs]quotes"));
    QCOMPARE( header2.toString(), QByteArray("Content-Type: text/plain; charset=ISO-8859-1; sample=\"needs[quotes\"; quoted=\"needs]quotes\"") );

    header2.setParameter("sample", QByteArray("needs:quotes"));
    header2.setParameter("quoted", QByteArray("needs;quotes"));
    QCOMPARE( header2.toString(), QByteArray("Content-Type: text/plain; charset=ISO-8859-1; sample=\"needs:quotes\"; quoted=\"needs;quotes\"") );

    header2.setParameter("sample", QByteArray("needs@quotes"));
    header2.setParameter("quoted", QByteArray("needs?quotes"));
    QCOMPARE( header2.toString(), QByteArray("Content-Type: text/plain; charset=ISO-8859-1; sample=\"needs@quotes\"; quoted=\"needs?quotes\"") );

    header2.setParameter("sample", QByteArray("needs=quotes"));
    header2.setParameter("quoted", QByteArray("needs,quotes"));
    QCOMPARE( header2.toString(), QByteArray("Content-Type: text/plain; charset=ISO-8859-1; sample=\"needs=quotes\"; quoted=\"needs,quotes\"") );

    header2.setParameter("sample", QByteArray("doesnt.need.quotes"));
    header2.setParameter("quoted", QByteArray("needs\\\\quotes"));
    QCOMPARE( header2.toString(), QByteArray("Content-Type: text/plain; charset=ISO-8859-1; sample=doesnt.need.quotes; quoted=\"needs\\\\quotes\"") );

    // Ensure parameters are coalesced where necessary
    QByteArray input =
"Content-Type: text/plain;" CRLF
" charset*0*=\"Very long charset value which will take up multiple lines in t\";" CRLF
" charset*1*=\"he RFC 2822 output\"; sample=simple" CRLF
CRLF;

    result1.clear();
    result1.append(qMakePair(QByteArray("charset"), QByteArray("Very long charset value which will take up multiple lines in the RFC 2822 output")));
    result1.append(qMakePair(QByteArray("sample"), QByteArray("simple")));
                
    QMailMessage msg = QMailMessage::fromRfc2822(input);
    QCOMPARE( msg.contentType().parameters(), result1 );
}

void tst_QMailMessageHeaderField::toString()
{
    // Tested by: constructor1, constructor2, setParameter
}

void tst_QMailMessageHeaderField::decodedContent_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QList<QByteArray> >("encoded");
    QTest::addColumn<QString>("result");

    // Test with some arabic characters, as per http://en.wikipedia.org/wiki/List_of_Unicode_characters
    const QChar arabicChars[] = { 0x0636, 0x0669, 0x06a5, 0x06b4 };
    QString unicodeAddress(arabicChars, 4);
    unicodeAddress.append(" <address@example>");

    QTest::newRow("Empty")
        << QByteArray()
        << QList<QByteArray>()
        << QString();

    QTest::newRow("No encoding")
        << QByteArray("hello")
        << QList<QByteArray>()
        << QString("hello");

    // Minimal testing - lets go straight to UTF-8 content and parameter
    QByteArray input;
    QString result;

    input = "=?UTF-8?B?2LYg2akJ2qUg2rQ=?=; param=UTF-8'ar'%D8%B6%D9%A9%DA%A5%DA%B4%20%3Caddress%40example%3E";

    const QChar whitespaceChars[] = { 0x0636, 0x0020, 0x0669, 0x0009, 0x06a5, 0x0020, 0x06b4 };
    result = QString(whitespaceChars, 7);
    result += "; ";
    result += "param*=";
    result += unicodeAddress;

    QTest::newRow("unicode characters") 
        << input
        << ( QList<QByteArray>() << QByteArray("param") )
        << result;
}

void tst_QMailMessageHeaderField::decodedContent()
{
    QFETCH( QByteArray, input );
    QFETCH( QList<QByteArray>, encoded );

    QMailMessageHeaderField field("X-Type", input);
    foreach (const QByteArray& param, encoded)
        field.setParameterEncoded(param);
        
    QTEST( field.decodedContent(), "result" );
}

void tst_QMailMessageHeaderField::encodeWord_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QByteArray>("charset");
    QTest::addColumn<QByteArray>("result");

    QTest::newRow("Empty")
        << QString()
        << QByteArray()
        << QByteArray();

    QString simpleAddress("mary@example.net");

    QTest::newRow("No encoding")
        << simpleAddress
        << QByteArray()
        << simpleAddress.toLatin1();

    QTest::newRow("ISO-8859-1 without encoding")
        << simpleAddress
        << QByteArray("ISO-8859-1")
        << QByteArray("=?ISO-8859-1?Q?mary=40example=2Enet?=");

    /* Qt Extended seems to be configured without iso-8859-[^1]
    QTest::newRow("ISO-8859-2 encoding")
        << QString("hello")
        << QByteArray("ISO-8859-2")
        << QByteArray("=?ISO-8859-2?Q?hello?=");
    */

    QString latin1Address("Joh\361 D\366e <jdoe@machine.test>");

    QTest::newRow("ISO-8859-1 with encoding")
        << latin1Address
        << QByteArray("ISO-8859-1")
        << QByteArray("=?ISO-8859-1?Q?Joh=F1_D=F6e_=3Cjdoe=40machine=2Etest=3E?=");

    QTest::newRow("Deduced ISO-8859-1")
        << latin1Address
        << QByteArray()
        << QByteArray("=?ISO-8859-1?Q?Joh=F1_D=F6e_=3Cjdoe=40machine=2Etest=3E?=");

    latin1Address = QString("\"Joh\361 D\366e\" <jdoe@machine.test>");

    QTest::newRow("ISO-8859-1 with quoted encoding")
        << latin1Address
        << QByteArray("ISO-8859-1")
        << QByteArray("=?ISO-8859-1?Q?=22Joh=F1_D=F6e=22_=3Cjdoe=40machine=2Etest=3E?=");

    const QChar chars[] = { 0x0636, 0x0669, 0x06a5, 0x06b4 };
    QString unicodeAddress(chars, 4);
    unicodeAddress.append(" <address@example>");

    QTest::newRow("UTF-8 with encoding")
        << unicodeAddress
        << QByteArray("UTF-8")
        << QByteArray("=?UTF-8?B?2LbZqdql2rQgPGFkZHJlc3NAZXhhbXBsZT4=?=");

    QTest::newRow("Deduced UTF-8")
        << unicodeAddress
        << QByteArray()
        << QByteArray("=?UTF-8?B?2LbZqdql2rQgPGFkZHJlc3NAZXhhbXBsZT4=?=");

    QTest::newRow("UTF-8 with language")
        << unicodeAddress
        << QByteArray("UTF-8*ar")
        << QByteArray("=?UTF-8*ar?B?2LbZqdql2rQgPGFkZHJlc3NAZXhhbXBsZT4=?=");

    unicodeAddress.append(unicodeAddress);
    QTest::newRow("Excessive length requires splitting")
        << unicodeAddress
        << QByteArray()
        << QByteArray("=?UTF-8?B?2LbZqdql2rQgPGFkZHJlc3NAZXhhbXBsZT7Yttmp2qXatCA8YWRkcmVzc0Bl?= =?UTF-8?B?eGFtcGxlPg==?=");
}

void tst_QMailMessageHeaderField::encodeWord()
{
    QFETCH( QString, text );
    QFETCH( QByteArray, charset );

    QByteArray encoded = QMailMessageHeaderField::encodeWord(text, charset);
    QTEST( encoded, "result" );

    QString decoded = QMailMessageHeaderField::decodeWord(encoded);
    QCOMPARE( decoded, text );
}

void tst_QMailMessageHeaderField::decodeWord()
{
    // Tested-by: encodeWord
}

void tst_QMailMessageHeaderField::encodeParameter_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QByteArray>("charset");
    QTest::addColumn<QByteArray>("language");
    QTest::addColumn<QByteArray>("result");

    // Note 1 - there's no particular reason why these test strings are addresses
    QString simpleAddress("mary@example.net");
    QString latin1Address("\"Joh\361 D\366e\" <jdoe@machine.test>");

    // Test with some arabic characters, as per http://en.wikipedia.org/wiki/List_of_Unicode_characters
    const QChar chars[] = { 0x0636, 0x0669, 0x06a5, 0x06b4 };
    QString unicodeAddress(chars, 4);
    unicodeAddress.append(" <address@example>");

    QTest::newRow("Empty")
        << QString()
        << QByteArray()
        << QByteArray()
        << QByteArray("''");

    QTest::newRow("No encoding")
        << QString("hello")
        << QByteArray()
        << QByteArray()
        << QByteArray("''hello");

    // Note 2 - due to our easy-way-out selection of encodable characters, some characters not
    // strictly requiring encoding will be encoded anyway (which is conforming :)
    QTest::newRow("No conversion")
        << simpleAddress
        << QByteArray()
        << QByteArray()
        << QByteArray("''mary%40example%2Enet");

    QTest::newRow("ASCII")
        << simpleAddress
        << QByteArray("us-ascii")
        << QByteArray("")
        << QByteArray("us-ascii''mary%40example%2Enet");

    QTest::newRow("Latin1 without encoding")
        << simpleAddress
        << QByteArray("ISO-8859-1")
        << QByteArray("en")
        << QByteArray("ISO-8859-1'en'mary%40example%2Enet");

    QTest::newRow("Latin1 with encoding")
        << latin1Address
        << QByteArray("ISO-8859-1")
        << QByteArray("de")
        << QByteArray("ISO-8859-1'de'%22Joh%F1%20D%F6e%22%20%3Cjdoe%40machine%2Etest%3E");

    QTest::newRow("Latin1 with encoding incorporating language")
        << latin1Address
        << QByteArray("ISO-8859-1*de")
        << QByteArray()
        << QByteArray("ISO-8859-1'de'%22Joh%F1%20D%F6e%22%20%3Cjdoe%40machine%2Etest%3E");

    QTest::newRow("Latin1 with encoding overridden by language")
        << latin1Address
        << QByteArray("ISO-8859-1*en")
        << QByteArray("en_US")
        << QByteArray("ISO-8859-1'en_US'%22Joh%F1%20D%F6e%22%20%3Cjdoe%40machine%2Etest%3E");

    QTest::newRow("Latin1 without context")
        << latin1Address
        << QByteArray()
        << QByteArray()
        << QByteArray("ISO-8859-1''%22Joh%F1%20D%F6e%22%20%3Cjdoe%40machine%2Etest%3E");

    QTest::newRow("UTF-8")
        << unicodeAddress
        << QByteArray("UTF-8")
        << QByteArray("ar")
        << QByteArray("UTF-8'ar'%D8%B6%D9%A9%DA%A5%DA%B4%20%3Caddress%40example%3E");

    QTest::newRow("UTF-8 without context")
        << unicodeAddress
        << QByteArray()
        << QByteArray()
        << QByteArray("UTF-8''%D8%B6%D9%A9%DA%A5%DA%B4%20%3Caddress%40example%3E");
}

void tst_QMailMessageHeaderField::encodeParameter()
{
    QFETCH( QString, text );
    QFETCH( QByteArray, charset );
    QFETCH( QByteArray, language );

    QByteArray encoded = QMailMessageHeaderField::encodeParameter(text, charset, language);
    QTEST( encoded, "result" );

    QString decoded = QMailMessageHeaderField::decodeParameter(encoded);
    QCOMPARE( decoded, text );
}

void tst_QMailMessageHeaderField::decodeParameter()
{
    // Mostly Tested-by: encodeParameter

    // Verify that we will decode an illegally-formatted ASCII parameter:
    QString simpleAddress("mary@example.net");

    QByteArray validlyEncoded("us-ascii''mary%40example%2Enet");
    QByteArray invalidlyEncoded("mary%40example%2Enet");

    QCOMPARE( QMailMessageHeaderField::decodeParameter(validlyEncoded), simpleAddress );
    QCOMPARE( QMailMessageHeaderField::decodeParameter(invalidlyEncoded), simpleAddress );

    // Verify that non-ASCII characters will not be present after decoding:
    invalidlyEncoded = QByteArray("%EAmary%40example%2Enet%FE");
    QCOMPARE( QMailMessageHeaderField::decodeParameter(invalidlyEncoded), simpleAddress );

    invalidlyEncoded = QByteArray("us-ascii''%EAmary%40example%2Enet%FE");
    QCOMPARE( QMailMessageHeaderField::decodeParameter(invalidlyEncoded), simpleAddress );
}

void tst_QMailMessageHeaderField::encodeContent_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QByteArray>("charset");
    QTest::addColumn<QByteArray>("result");

    QTest::newRow("Empty")
        << QString()
        << QByteArray()
        << QByteArray();

    QString simpleAddress("mary@example.net");

    QTest::newRow("No encoding")
        << simpleAddress
        << QByteArray()
        << simpleAddress.toLatin1();

    QTest::newRow("ISO-8859-1 without encoding")
        << simpleAddress
        << QByteArray("ISO-8859-1")
        << QByteArray("=?ISO-8859-1?Q?mary=40example=2Enet?=");

    /* Qt Extended seems to be configured without iso-8859-[^1]
    QTest::newRow("ISO-8859-2 encoding")
        << QString("hello")
        << QByteArray("ISO-8859-2")
        << QByteArray("=?ISO-8859-2?Q?hello?=");
    */

    QString latin1Address("Joh\361 D\366e <jdoe@machine.test>");

    QTest::newRow("ISO-8859-1 with encoding")
        << latin1Address
        << QByteArray("ISO-8859-1")
        << QByteArray("=?ISO-8859-1?Q?Joh=F1?= =?ISO-8859-1?Q?_D=F6e?= =?ISO-8859-1?Q?_=3Cjdoe=40machine=2Etest=3E?=");

    QTest::newRow("Deduced ISO-8859-1")
        << latin1Address
        << QByteArray()
        << QByteArray("=?ISO-8859-1?Q?Joh=F1?= =?ISO-8859-1?Q?_D=F6e?= <jdoe@machine.test>");

    latin1Address = QString("\"Joh\361 D\366e\" <jdoe@machine.test>");

    QTest::newRow("ISO-8859-1 with quoted encoding")
        << latin1Address
        << QByteArray("ISO-8859-1")
        << QByteArray("=?ISO-8859-1?Q?=22Joh=F1_D=F6e=22?= =?ISO-8859-1?Q?_=3Cjdoe=40machine=2Etest=3E?=");

    const QChar chars[] = { 0x0636, 0x0669, 0x06a5, 0x06b4 };
    QString unicodeAddress(chars, 4);
    unicodeAddress.append(" <address@example>");

    QTest::newRow("UTF-8 with encoding")
        << unicodeAddress
        << QByteArray("UTF-8")
        << QByteArray("=?UTF-8?B?2LbZqdql2rQ=?= =?UTF-8?B?IDxhZGRyZXNzQGV4YW1wbGU+?=");

    QTest::newRow("Deduced UTF-8")
        << unicodeAddress
        << QByteArray()
        << QByteArray("=?UTF-8?B?2LbZqdql2rQ=?= <address@example>");

    QTest::newRow("UTF-8 with language")
        << unicodeAddress
        << QByteArray("UTF-8*ar")
        << QByteArray("=?UTF-8*ar?B?2LbZqdql2rQ=?= =?UTF-8*ar?B?IDxhZGRyZXNzQGV4YW1wbGU+?=");
}

void tst_QMailMessageHeaderField::encodeContent()
{
    QFETCH( QString, text );
    QFETCH( QByteArray, charset );

    QByteArray encoded = QMailMessageHeaderField::encodeContent(text, charset);
    QTEST( encoded, "result" );

    QString decoded = QMailMessageHeaderField::decodeContent(encoded);
    QCOMPARE( decoded, text );
}

void tst_QMailMessageHeaderField::decodeContent()
{
    // Tested-by: encodeContent
}

void tst_QMailMessageHeaderField::removeComments_data()
{
    QTest::addColumn<QByteArray>("before");
    QTest::addColumn<QByteArray>("after");

    QTest::newRow("Empty")
        << QByteArray()
        << QByteArray();

    QTest::newRow("None")
        << QByteArray("no \\(comments\\)")
        << QByteArray("no (comments)");

    QTest::newRow("Preceding")
        << QByteArray("(This is a comment) preserved")
        << QByteArray(" preserved");

    QTest::newRow("Succeeding")
        << QByteArray("Preserved(This is a comment)")
        << QByteArray("Preserved");

    QTest::newRow("Interspersed")
        << QByteArray("Preserved(comment)preserved(comment)preserved")
        << QByteArray("Preservedpreservedpreserved");

    QTest::newRow("Nested comments")
        << QByteArray("P(comment(nested))res(comment)er((()))ved")
        << QByteArray("Preserved");
}

void tst_QMailMessageHeaderField::removeComments()
{
    QFETCH( QByteArray, before );
    QFETCH( QByteArray, after );

    QCOMPARE( QMailMessageHeaderField::removeComments(before), after );
}

void tst_QMailMessageHeaderField::removeWhitespace_data()
{
    QTest::addColumn<QByteArray>("before");
    QTest::addColumn<QByteArray>("after");

    QTest::newRow("Empty")
        << QByteArray()
        << QByteArray();

    QTest::newRow("None")
        << QByteArray("no-whitespace")
        << QByteArray("no-whitespace");

    QTest::newRow("Preceding")
        << QByteArray("\t preserved")
        << QByteArray("preserved");

    QTest::newRow("Succeeding")
        << QByteArray("Preserved\r\r   \t")
        << QByteArray("Preserved");

    QTest::newRow("Interspersed")
        << QByteArray("Preserved\r\n  preserved \tpreserved")
        << QByteArray("Preservedpreservedpreserved");

    QTest::newRow("Single characters")
        << QByteArray("Pre s\ter\nve d")
        << QByteArray("Preserved");

    QTest::newRow("Quoted whitespace")
        << QByteArray("Preserved \" \" Preserved \"\n\t\" ")
        << QByteArray("Preserved\" \"Preserved\"\n\t\"");
}

void tst_QMailMessageHeaderField::removeWhitespace()
{
    QFETCH( QByteArray, before );
    QFETCH( QByteArray, after );

    QCOMPARE( QMailMessageHeaderField::removeWhitespace(before), after );
}

void tst_QMailMessageHeaderField::output_data()
{
    QTest::addColumn<QByteArray>("id");
    QTest::addColumn<QByteArray>("content");
    QTest::addColumn<QByteArray>("expected");

    QTest::newRow("Empty")
        << QByteArray()
        << QByteArray()
        << QByteArray();

    QTest::newRow("Simple")
        << QByteArray("X-Header")
        << QByteArray("something")
        << QByteArray("X-Header: something" CRLF);

    // A possible improvement would be to preserve quoted strings from unnecessary breaks?
    QTest::newRow("Prefer wrapping after header")
        << QByteArray("X-Very-Long-Identifier-For-A-Header")
        << QByteArray("\"quoted text string, should follow the soft line-break, maybe?\"")
        << QByteArray(
"X-Very-Long-Identifier-For-A-Header: \"quoted text string, should follow the" CRLF
" soft line-break, maybe?\"" CRLF);

    QTest::newRow("Requires wrapping after content")
        << QByteArray("X-Very-Long-Identifier-For-A-Header")
        << QByteArray("\"quoted text string\"; this=first-parameter")
        << QByteArray(
"X-Very-Long-Identifier-For-A-Header: \"quoted text string\";" CRLF
" this=first-parameter" CRLF);

    QTest::newRow("Requires wrapping after parameter")
        << QByteArray("X-Very-Long-Identifier-For-A-Header")
        << QByteArray("unquoted_text; this=first-parameter; this=second-parameter")
        << QByteArray(
"X-Very-Long-Identifier-For-A-Header: unquoted_text; this=first-parameter;" CRLF
" this=second-parameter" CRLF);

    QTest::newRow("Requires wrapping inside content")
        << QByteArray("X-Very-Long-Identifier-For-A-Header")
        << QByteArray("This text string is too long to fit entirely into a single line of a message header field...")
        << QByteArray(
"X-Very-Long-Identifier-For-A-Header: This text string is too long to fit" CRLF
" entirely into a single line of a message header field..." CRLF);

    QTest::newRow("Requires wrapping without whitespace")
        << QByteArray("X-Very-Long-Identifier-For-A-Header")
        << QByteArray("This_text_string_is_too_long_to_fit_entirely_into_a_single_line_of_a_message_header_field...")
        << QByteArray(
"X-Very-Long-Identifier-For-A-Header:" CRLF
" This_text_string_is_too_long_to_fit_entirely_into_a_single_line_of_a_message_" CRLF
"\theader_field..." CRLF);

    QTest::newRow("Unbreakable token after breakable whitespace")
        << QByteArray("Content-Disposition")
        << QByteArray("attachment; filename=/home/a_user_with_a_long_username/qtopia/home/Documents/channell1.jpg")
        << QByteArray(
"Content-Disposition: attachment;" CRLF
" filename=/home/a_user_with_a_long_username/qtopia/home/Documents/channell1.jp" CRLF
"\tg" CRLF);
}

void tst_QMailMessageHeaderField::output()
{
    QFETCH( QByteArray, id );
    QFETCH( QByteArray, content );
    QFETCH( QByteArray, expected );

    QByteArray output = asRfc2822(id, content);
    QCOMPARE( output, expected );
}


tst_QMailMessageContentType::tst_QMailMessageContentType()
{
}

tst_QMailMessageContentType::~tst_QMailMessageContentType()
{
}

void tst_QMailMessageContentType::constructor_data()
{
    QTest::addColumn<QByteArray>("text");
    QTest::addColumn<QByteArray>("type");
    QTest::addColumn<QByteArray>("subtype");

    QTest::newRow("Empty")
        << QByteArray()
        << QByteArray()
        << QByteArray();

    QTest::newRow("Name only")
        << QByteArray("Content-Type:")
        << QByteArray()
        << QByteArray();

    QTest::newRow("No subtype")
        << QByteArray("text")
        << QByteArray("text")
        << QByteArray();

    QTest::newRow("No subtype with separator")
        << QByteArray("text/")
        << QByteArray("text")
        << QByteArray();

    QTest::newRow("With subtype")
        << QByteArray("text/html")
        << QByteArray("text")
        << QByteArray("html");

    QTest::newRow("With subtype and name")
        << QByteArray("Content-Type:text/html")
        << QByteArray("text")
        << QByteArray("html");

    QTest::newRow("With whitespace")
        << QByteArray(" \t text\n /  html \r\t ")
        << QByteArray("text")
        << QByteArray("html");
}

void tst_QMailMessageContentType::constructor()
{
    QFETCH( QByteArray, text ); 
    QMailMessageContentType ct(text);

    QCOMPARE( ct.id(), QByteArray("Content-Type") );
    QTEST( ct.type(), "type" );
    QTEST( ct.subType(), "subtype" );

    QByteArray content = ct.type();
    if (!content.isEmpty() && (!ct.subType().isEmpty()))
        content.append('/').append(ct.subType());
    QCOMPARE( ct.content(), content );
}

void tst_QMailMessageContentType::type() 
{
    // Tested-by: constructor, setType
}

void tst_QMailMessageContentType::setType() 
{
    QMailMessageContentType type1;
    QMailMessageContentType type2("text/html; charset=us-ascii");

    QCOMPARE( type1.type(), QByteArray() );
    QCOMPARE( type2.type(), QByteArray("text") );

    QByteArray type("application");

    // Set the type
    type1.setType(type);
    QCOMPARE( type1.type(), type );
    QCOMPARE( type1.toString(), QByteArray("Content-Type: application") );

    // Set the type to null
    type1.setType(QByteArray());
    QCOMPARE( type1.type(), QByteArray() );
    QCOMPARE( type1.toString(), QByteArray("Content-Type:") );

    // Set the type with existing
    type2.setType(type);
    QCOMPARE( type2.type(), type );
    QCOMPARE( type2.toString(), QByteArray("Content-Type: application/html; charset=us-ascii") );

    // Set the type to null with existing sub-type
    type2.setType(QByteArray());
    QCOMPARE( type2.type(), QByteArray() );
    QCOMPARE( type2.subType(), QByteArray() );
    QCOMPARE( type2.toString(), QByteArray("Content-Type:; charset=us-ascii") );
}

void tst_QMailMessageContentType::subType() 
{
    // Tested-by: constructor, setSubType
}

void tst_QMailMessageContentType::setSubType() 
{
    QMailMessageContentType type1;
    QMailMessageContentType type2("text/html; charset=us-ascii");

    QCOMPARE( type1.subType(), QByteArray() );
    QCOMPARE( type2.subType(), QByteArray("html") );

    QByteArray subType("plain");

    // Set the sub-type with no type
    type1.setSubType(subType);
    QCOMPARE( type1.subType(), QByteArray() );
    QCOMPARE( type1.toString(), QByteArray("Content-Type:") );

    // Set the sub-type to null with no type
    type1.setSubType(QByteArray());
    QCOMPARE( type1.subType(), QByteArray() );
    QCOMPARE( type1.toString(), QByteArray("Content-Type:") );

    // Set the sub-type with existing type
    type1.setType("text");
    type1.setSubType(subType);
    QCOMPARE( type1.subType(), subType );
    QCOMPARE( type1.toString(), QByteArray("Content-Type: text/plain") );

    // Set the sub-type to null with existing type
    type1.setSubType(QByteArray());
    QCOMPARE( type1.type(), QByteArray("text") );
    QCOMPARE( type1.subType(), QByteArray() );
    QCOMPARE( type1.toString(), QByteArray("Content-Type: text") );

    // Set the sub-type with existing sub-type
    type2.setSubType(subType);
    QCOMPARE( type2.subType(), subType );
    QCOMPARE( type2.toString(), QByteArray("Content-Type: text/plain; charset=us-ascii") );

    // Set the sub-type to null with existing sub-type
    type2.setSubType(QByteArray());
    QCOMPARE( type2.type(), QByteArray("text") );
    QCOMPARE( type2.subType(), QByteArray() );
    QCOMPARE( type2.toString(), QByteArray("Content-Type: text; charset=us-ascii") );
}

void tst_QMailMessageContentType::name()
{
    // Tested-by: setFilename
}

void tst_QMailMessageContentType::setName()
{
    QMailMessageContentType type1;
    QMailMessageContentType type2("text/html; charset=us-ascii; name=Albert");

    QCOMPARE( type1.name(), QByteArray() );
    QCOMPARE( type2.name(), QByteArray("Albert") );

    QByteArray name("Brian");

    type1.setName(name);
    QCOMPARE( type1.name(), name );
    // This is invalid output, but we can't enforce correct usage:
    QCOMPARE( type1.toString(), QByteArray("Content-Type:; name=Brian") );

    type2.setName(name);
    QCOMPARE( type2.name(), name );
    QCOMPARE( type2.toString(), QByteArray("Content-Type: text/html; charset=us-ascii; name=Brian") );
}


tst_QMailMessageContentDisposition::tst_QMailMessageContentDisposition()
{
}

tst_QMailMessageContentDisposition::~tst_QMailMessageContentDisposition()
{
}

void tst_QMailMessageContentDisposition::constructor_data()
{
    QTest::addColumn<QByteArray>("text");
    QTest::addColumn<QMailMessageContentDisposition::DispositionType>("type");
    QTest::addColumn<QByteArray>("content");
    QTest::addColumn<QByteArray>("filename");
    QTest::addColumn<QMailTimeStamp>("creation_date");
    QTest::addColumn<QMailTimeStamp>("modification_date");
    QTest::addColumn<QMailTimeStamp>("read_date");
    QTest::addColumn<int>("size");

    QTest::newRow("Empty")
        << QByteArray()
        << QMailMessageContentDisposition::None
        << QByteArray()
        << QByteArray()
        << QMailTimeStamp()
        << QMailTimeStamp()
        << QMailTimeStamp()
        << -1;

    QTest::newRow("Inline")
        << QByteArray("inLINe")
        << QMailMessageContentDisposition::Inline
        << QByteArray("inLINe")
        << QByteArray()
        << QMailTimeStamp()
        << QMailTimeStamp()
        << QMailTimeStamp()
        << -1;

    QTest::newRow("Attachment")
        << QByteArray("aTTachMENt")
        << QMailMessageContentDisposition::Attachment
        << QByteArray("aTTachMENt")
        << QByteArray()
        << QMailTimeStamp()
        << QMailTimeStamp()
        << QMailTimeStamp()
        << -1;

    QTest::newRow("Invalid")
        << QByteArray("inlined")
        << QMailMessageContentDisposition::None
        << QByteArray("inlined")
        << QByteArray()
        << QMailTimeStamp()
        << QMailTimeStamp()
        << QMailTimeStamp()
        << -1;

    QTest::newRow("With parameters")
        << QByteArray("attachment; filename=att.tar.gz; creation-date=\"Wed, 12 Feb 1997 16:29:51 -0500\"; "
                      "size=12345; modification-date=\"Wed, 12 Feb 1997 16:29:51 -0500\"")
        << QMailMessageContentDisposition::Attachment
        << QByteArray("attachment")
        << QByteArray("att.tar.gz")
        << QMailTimeStamp("Wed, 12 Feb 1997 16:29:51 -0500")
        << QMailTimeStamp("Wed, 12 Feb 1997 16:29:51 -0500")
        << QMailTimeStamp()
        << 12345;
}

void tst_QMailMessageContentDisposition::constructor()
{
    QFETCH( QByteArray, text );

    QMailMessageContentDisposition cd(text);

    QCOMPARE( cd.id(), QByteArray("Content-Disposition") );
    QTEST( cd.type(), "type" );
    QTEST( cd.content(), "content" );
    QTEST( cd.filename(), "filename" );
    QTEST( cd.creationDate(), "creation_date" );
    QTEST( cd.modificationDate(), "modification_date" );
    QTEST( cd.readDate(), "read_date" );
    QTEST( cd.size(), "size" );
}

void tst_QMailMessageContentDisposition::type()
{
    // Tested-by: setType
}

void tst_QMailMessageContentDisposition::setType()
{
    QMailMessageContentDisposition disposition1;
    QMailMessageContentDisposition disposition2("inline");

    QCOMPARE( disposition1.type(), QMailMessageContentDisposition::None );
    QCOMPARE( disposition2.type(), QMailMessageContentDisposition::Inline );

    QMailMessageContentDisposition::DispositionType type = QMailMessageContentDisposition::Attachment;

    disposition1.setType(type);
    QCOMPARE( disposition1.type(), type );
    QCOMPARE( disposition1.toString(), QByteArray("Content-Disposition: attachment") );

    disposition2.setType(type);
    QCOMPARE( disposition2.type(), type );
    QCOMPARE( disposition2.toString(), QByteArray("Content-Disposition: attachment") );
}

void tst_QMailMessageContentDisposition::filename()
{
    // Tested-by: setFilename
}

void tst_QMailMessageContentDisposition::setFilename()
{
    QMailMessageContentDisposition disposition1;
    QMailMessageContentDisposition disposition2("attachment; filename=sample.txt; size=12345");

    QCOMPARE( disposition1.filename(), QByteArray() );
    QCOMPARE( disposition2.filename(), QByteArray("sample.txt") );

    QByteArray filename("att.tar.gz");

    disposition1.setFilename(filename);
    QCOMPARE( disposition1.filename(), filename );
    // This is invalid output, but we can't enforce correct usage:
    QCOMPARE( disposition1.toString(), QByteArray("Content-Disposition:; filename=att.tar.gz") );

    disposition2.setFilename(filename);
    QCOMPARE( disposition2.filename(), filename );
    QCOMPARE( disposition2.toString(), QByteArray("Content-Disposition: attachment; filename=att.tar.gz; size=12345") );
}

void tst_QMailMessageContentDisposition::creationDate()
{
    // Tested-by: setCreationDate
}

void tst_QMailMessageContentDisposition::setCreationDate()
{
    QMailMessageContentDisposition disposition1;
    QMailMessageContentDisposition disposition2("attachment; filename=sample.txt; creation-date=\"Wed, 12 Feb 1997 16:29:51 -0500\"");

    QCOMPARE( disposition1.creationDate(), QMailTimeStamp() );
    QCOMPARE( disposition2.creationDate(), QMailTimeStamp("Wed, 12 Feb 1997 16:29:51 -0500") );

    QMailTimeStamp timeStamp = QMailTimeStamp::currentDateTime();

    disposition1.setCreationDate(timeStamp);
    QCOMPARE( disposition1.creationDate(), timeStamp );
    // This is invalid output, but we can't enforce correct usage:
    QCOMPARE( disposition1.toString(), QByteArray("Content-Disposition:; "
                                                  "creation-date=") + QMail::quoteString(timeStamp.toString()).toLatin1() );

    disposition2.setCreationDate(timeStamp);
    QCOMPARE( disposition2.creationDate(), timeStamp );
    QCOMPARE( disposition2.toString(), QByteArray("Content-Disposition: attachment; filename=sample.txt; "
                                                  "creation-date=") + QMail::quoteString(timeStamp.toString()).toLatin1() );
}

void tst_QMailMessageContentDisposition::modificationDate()
{
    // Tested-by: setModificationDate
}

void tst_QMailMessageContentDisposition::setModificationDate()
{
    QMailMessageContentDisposition disposition1;
    QMailMessageContentDisposition disposition2("attachment; filename=sample.txt; modification-date=\"Wed, 12 Feb 1997 16:29:51 -0500\"");

    QCOMPARE( disposition1.modificationDate(), QMailTimeStamp() );
    QCOMPARE( disposition2.modificationDate(), QMailTimeStamp("Wed, 12 Feb 1997 16:29:51 -0500") );

    QMailTimeStamp timeStamp = QMailTimeStamp::currentDateTime();

    disposition1.setModificationDate(timeStamp);
    QCOMPARE( timeStamp, timeStamp );
    QCOMPARE( disposition1.modificationDate(), timeStamp );
    // This is invalid output, but we can't enforce correct usage:
    QCOMPARE( disposition1.toString(), QByteArray("Content-Disposition:; "
                                                  "modification-date=") + QMail::quoteString(timeStamp.toString()).toLatin1() );

    disposition2.setModificationDate(timeStamp);
    QCOMPARE( disposition2.modificationDate(), timeStamp );
    QCOMPARE( disposition2.toString(), QByteArray("Content-Disposition: attachment; filename=sample.txt; "
                                                  "modification-date=") + QMail::quoteString(timeStamp.toString()).toLatin1() );
}

void tst_QMailMessageContentDisposition::readDate()
{
    // Tested-by: setReadDate
}

void tst_QMailMessageContentDisposition::setReadDate()
{
    QMailMessageContentDisposition disposition1;
    QMailMessageContentDisposition disposition2("attachment; filename=sample.txt; read-date=\"Wed, 12 Feb 1997 16:29:51 -0500\"");

    QCOMPARE( disposition1.readDate(), QMailTimeStamp() );
    QCOMPARE( disposition2.readDate(), QMailTimeStamp("Wed, 12 Feb 1997 16:29:51 -0500") );

    QMailTimeStamp timeStamp = QMailTimeStamp::currentDateTime();

    disposition1.setReadDate(timeStamp);
    QCOMPARE( timeStamp, timeStamp );
    QCOMPARE( disposition1.readDate(), timeStamp );
    // This is invalid output, but we can't enforce correct usage:
    QCOMPARE( disposition1.toString(), QByteArray("Content-Disposition:; "
                                                  "read-date=") + QMail::quoteString(timeStamp.toString()).toLatin1() );

    disposition2.setReadDate(timeStamp);
    QCOMPARE( disposition2.readDate(), timeStamp );
    QCOMPARE( disposition2.toString(), QByteArray("Content-Disposition: attachment; filename=sample.txt; "
                                                  "read-date=") + QMail::quoteString(timeStamp.toString()).toLatin1() );
}

void tst_QMailMessageContentDisposition::size()
{
    // Tested-by: setSize
}

void tst_QMailMessageContentDisposition::setSize()
{
    QMailMessageContentDisposition disposition1;
    QMailMessageContentDisposition disposition2("attachment; filename=sample.txt; size=12345");

    QCOMPARE( disposition1.size(), -1 );
    QCOMPARE( disposition2.size(), 12345 );

    int size = 54321;

    disposition1.setSize(size);
    QCOMPARE( disposition1.size(), size );
    // This is invalid output, but we can't enforce correct usage:
    QCOMPARE( disposition1.toString(), QByteArray("Content-Disposition:; size=54321") );

    disposition2.setSize(size);
    QCOMPARE( disposition2.size(), size );
    QCOMPARE( disposition2.toString(), QByteArray("Content-Disposition: attachment; filename=sample.txt; size=54321") );
}

