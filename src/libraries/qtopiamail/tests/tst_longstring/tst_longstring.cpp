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

#include <QtopiaApplication>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QMailAddress>
#include "../../../../../src/libraries/qtopiamail/longstring_p.h"

#include <ctype.h>


//TESTED_CLASS=LongString
//TESTED_FILES=src/libraries/qtopiamail/qmailmessage.cpp

/*
    This class primarily tests that LongString class correctly replaces a QByteArray.
    LongString is not for external use; it is an implementation detail of QMailMessage.
*/
class tst_LongString : public QObject
{
    Q_OBJECT

public:
    tst_LongString();
    virtual ~tst_LongString();

private slots:
    virtual void initTestCase();
    virtual void cleanupTestCase();
    virtual void init();
    virtual void cleanup();

    void indexOf_data();
    void indexOf();
    void left_data();
    void left();
    void right_data();
    void right();
    void mid_data();
    void mid();

    void length();
    void isEmpty();
    void toQByteArray();
    void dataStream();
};

QTEST_APP_MAIN( tst_LongString, QtopiaApplication )
#include "tst_longstring.moc"

static void verifyIndexOf( const LongString& container, const QByteArray& content )
{
    if (content.length() > 1)
    {
        QByteArray target = content.mid(1, -1);

        // Invert the case of the first character
        target[0] = (isupper(target[0]) ? tolower(target[0]) : toupper(target[0]));
        QCOMPARE( container.indexOf( target ), 1 );
    }
}


tst_LongString::tst_LongString()
{
}

tst_LongString::~tst_LongString()
{
}

void tst_LongString::initTestCase()
{
}

void tst_LongString::cleanupTestCase()
{
}

void tst_LongString::init()
{
}

void tst_LongString::cleanup()
{
}

void tst_LongString::indexOf_data()
{
    QTest::addColumn<QByteArray>("source");
    QTest::addColumn<QByteArray>("target");
    QTest::addColumn<int>("from");
    QTest::addColumn<int>("index");

    QTest::newRow("No match") 
        << QByteArray("Hello hello")
        << QByteArray("goodbye")
        << 0
        << -1;

    QTest::newRow("No match with from") 
        << QByteArray("Hello hello")
        << QByteArray("goodbye")
        << 3
        << -1;

    QTest::newRow("No match with invalid from") 
        << QByteArray("Hello hello")
        << QByteArray("goodbye")
        << 33
        << -1;

    QTest::newRow("Case-correct match") 
        << QByteArray("Hello hello")
        << QByteArray("lo h")
        << 0
        << 3;

    QTest::newRow("Case-correct match with from") 
        << QByteArray("Hello hello")
        << QByteArray("lo h")
        << 2
        << 3;

    QTest::newRow("Case-correct subsequent match with from") 
        << QByteArray("Hello hello")
        << QByteArray("ell")
        << 4
        << 7;

    QTest::newRow("Case-insensitive match") 
        << QByteArray("Hello hello")
        << QByteArray("Lo H")
        << 0
        << 3;

    QTest::newRow("Case-insensitive match with from") 
        << QByteArray("Hello hello")
        << QByteArray("lO H")
        << 2
        << 3;

    QTest::newRow("Case-insensitive subsequent match with from") 
        << QByteArray("Hello hello")
        << QByteArray("ElL")
        << 4
        << 7;

    QTest::newRow("Match first character")
        << QByteArray("Hello hello")
        << QByteArray("HelLO ")
        << 0
        << 0;

    QTest::newRow("Match last character")
        << QByteArray("Hello hello")
        << QByteArray(" HELlo")
        << 0
        << 5;

    QTest::newRow("Non-match first character")
        << QByteArray("Hello hello")
        << QByteArray("xHelLO ")
        << 0
        << -1;

    QTest::newRow("Non-match last character")
        << QByteArray("Hello hello")
        << QByteArray(" HELlox")
        << 0
        << -1;

    QTest::newRow("Match entirety")
        << QByteArray("Hello hello")
        << QByteArray("HElLo HelLO")
        << 0
        << 0;

    QTest::newRow("Non-match entirety")
        << QByteArray("Hell")
        << QByteArray("hello")
        << 0
        << -1;
}

void tst_LongString::indexOf()
{
    QFETCH( QByteArray, source );
    QFETCH( QByteArray, target );
    QFETCH( int, from );
    QFETCH( int, index );

    LongString ls( source );
    QCOMPARE( ls.indexOf( target, from ), index );
}

void tst_LongString::left_data()
{
    QTest::addColumn<QByteArray>("source");
    QTest::addColumn<int>("size");
    QTest::addColumn<int>("nested_size");
    QTest::addColumn<QByteArray>("result");

    QTest::newRow("Size - zero") 
        << QByteArray("Supercalifragilistic")
        << 0
        << -1
        << QByteArray();

    QTest::newRow("Size - one") 
        << QByteArray("Supercalifragilistic")
        << 1
        << -1
        << QByteArray("S");

    QTest::newRow("Size - greater than one") 
        << QByteArray("Supercalifragilistic")
        << 5
        << -1
        << QByteArray("Super");

    QTest::newRow("Size - entire") 
        << QByteArray("Supercalifragilistic")
        << 20
        << -1
        << QByteArray("Supercalifragilistic");

    QTest::newRow("Size - entire plus one") 
        << QByteArray("Supercalifragilistic")
        << 21
        << -1
        << QByteArray("Supercalifragilistic");

    QTest::newRow("Size - negative") 
        << QByteArray("Supercalifragilistic")
        << -5
        << -1
        << QByteArray("Supercalifragilistic");

    QTest::newRow("Nested - size - zero") 
        << QByteArray("Supercalifragilistic")
        << 0
        << 0
        << QByteArray();

    QTest::newRow("Nested - size - one") 
        << QByteArray("Supercalifragilistic")
        << 1
        << 1
        << QByteArray("S");

    QTest::newRow("Nested - size - greater than one") 
        << QByteArray("Supercalifragilistic")
        << 3
        << 5
        << QByteArray("Sup");

    QTest::newRow("Nested - size - entire") 
        << QByteArray("Supercalifragilistic")
        << 5
        << 5
        << QByteArray("Super");

    QTest::newRow("Nested - size - entire plus one") 
        << QByteArray("Supercalifragilistic")
        << 6
        << 5
        << QByteArray("Super");
}

void tst_LongString::left()
{
    QFETCH( QByteArray, source );
    QFETCH( int, size );
    QFETCH( int, nested_size );
    QFETCH( QByteArray, result );

    LongString ls( source );
    LongString comparator;

    if ( nested_size == -1 )
    {
        comparator = ls.left( size );
    }
    else
    {
        LongString nested = ls.left( nested_size );
        comparator = nested.left( size );
    }

    QCOMPARE( comparator.toQByteArray(), result );
    verifyIndexOf( comparator, comparator.toQByteArray() );
}

void tst_LongString::right_data()
{
    QTest::addColumn<QByteArray>("source");
    QTest::addColumn<int>("size");
    QTest::addColumn<int>("nested_size");
    QTest::addColumn<QByteArray>("result");

    QTest::newRow("Size - zero") 
        << QByteArray("Supercalifragilistic")
        << 0
        << -1
        << QByteArray();

    QTest::newRow("Size - one") 
        << QByteArray("Supercalifragilistic")
        << 1
        << -1
        << QByteArray("c");

    QTest::newRow("Size - greater than one") 
        << QByteArray("Supercalifragilistic")
        << 5
        << -1
        << QByteArray("istic");

    QTest::newRow("Size - entire") 
        << QByteArray("Supercalifragilistic")
        << 20
        << -1
        << QByteArray("Supercalifragilistic");

    QTest::newRow("Size - entire plus one") 
        << QByteArray("Supercalifragilistic")
        << 21
        << -1
        << QByteArray("Supercalifragilistic");

    QTest::newRow("Size - negative") 
        << QByteArray("Supercalifragilistic")
        << -5
        << -1
        << QByteArray("Supercalifragilistic");

    QTest::newRow("Nested - size - zero") 
        << QByteArray("Supercalifragilistic")
        << 0
        << 0
        << QByteArray();

    QTest::newRow("Nested - size - one") 
        << QByteArray("Supercalifragilistic")
        << 1
        << 1
        << QByteArray("c");

    QTest::newRow("Nested - size - greater than one") 
        << QByteArray("Supercalifragilistic")
        << 3
        << 5
        << QByteArray("tic");

    QTest::newRow("Nested - size - entire") 
        << QByteArray("Supercalifragilistic")
        << 5
        << 5
        << QByteArray("istic");

    QTest::newRow("Nested - size - entire plus one") 
        << QByteArray("Supercalifragilistic")
        << 6
        << 5
        << QByteArray("istic");
}

void tst_LongString::right()
{
    QFETCH( QByteArray, source );
    QFETCH( int, size );
    QFETCH( int, nested_size );
    QFETCH( QByteArray, result );

    LongString ls( source );
    LongString comparator;

    if ( nested_size == -1 )
    {
        comparator = ls.right( size );
    }
    else
    {
        LongString nested = ls.right( nested_size );
        comparator = nested.right( size );
    }

    QCOMPARE( comparator.toQByteArray(), result );
    verifyIndexOf( comparator, comparator.toQByteArray() );
}

void tst_LongString::mid_data()
{
    QTest::addColumn<QByteArray>("source");
    QTest::addColumn<int>("size");
    QTest::addColumn<int>("from");
    QTest::addColumn<int>("nested_size");
    QTest::addColumn<int>("nested_from");
    QTest::addColumn<QByteArray>("result");

    QTest::newRow("Size zero - offset zero") 
        << QByteArray("Supercalifragilistic")
        << 0
        << 0
        << -1
        << -1
        << QByteArray();

    QTest::newRow("Size zero - offset non-zero") 
        << QByteArray("Supercalifragilistic")
        << 0
        << 5
        << -1
        << -1
        << QByteArray();

    QTest::newRow("Size zero - offset negative") 
        << QByteArray("Supercalifragilistic")
        << 0
        << -5
        << -1
        << -1
        << QByteArray();

    QTest::newRow("Size one - offset zero") 
        << QByteArray("Supercalifragilistic")
        << 1
        << 0
        << -1
        << -1
        << QByteArray("S");

    QTest::newRow("Size one - offset negative") 
        << QByteArray("Supercalifragilistic")
        << 1
        << -1
        << -1
        << -1
        << QByteArray("S");

    QTest::newRow("Size one - offset non-zero") 
        << QByteArray("Supercalifragilistic")
        << 1
        << 9
        << -1
        << -1
        << QByteArray("f");

    QTest::newRow("Size one - offset last") 
        << QByteArray("Supercalifragilistic")
        << 1
        << 19
        << -1
        << -1
        << QByteArray("c");

    QTest::newRow("Size one - offset beyond last") 
        << QByteArray("Supercalifragilistic")
        << 1
        << 20
        << -1
        << -1
        << QByteArray();

    QTest::newRow("Size greater than one - offset zero") 
        << QByteArray("Supercalifragilistic")
        << 5
        << 0
        << -1
        << -1
        << QByteArray("Super");

    QTest::newRow("Size greater than one - offset negative") 
        << QByteArray("Supercalifragilistic")
        << 5
        << -1
        << -1
        << -1
        << QByteArray("Super");

    QTest::newRow("Size greater than one - offset non-zero") 
        << QByteArray("Supercalifragilistic")
        << 5
        << 9
        << -1
        << -1
        << QByteArray("fragi");

    QTest::newRow("Size greater than one - offset last") 
        << QByteArray("Supercalifragilistic")
        << 5
        << 15
        << -1
        << -1
        << QByteArray("istic");

    QTest::newRow("Size greater than one - offset beyond last") 
        << QByteArray("Supercalifragilistic")
        << 5
        << 17
        << -1
        << -1
        << QByteArray("tic");

    QTest::newRow("Size greater than one - offset exceeds size") 
        << QByteArray("Supercalifragilistic")
        << 5
        << 20
        << -1
        << -1
        << QByteArray();

    QTest::newRow("Size entire - offset zero") 
        << QByteArray("Supercalifragilistic")
        << 20
        << 0
        << -1
        << -1
        << QByteArray("Supercalifragilistic");

    QTest::newRow("Size entire - offset negative") 
        << QByteArray("Supercalifragilistic")
        << 20
        << -1
        << -1
        << -1
        << QByteArray("Supercalifragilistic");

    QTest::newRow("Size entire - offset non-zero") 
        << QByteArray("Supercalifragilistic")
        << 20
        << 9
        << -1
        << -1
        << QByteArray("fragilistic");

    QTest::newRow("Size entire - offset beyond last") 
        << QByteArray("Supercalifragilistic")
        << 20
        << 17
        << -1
        << -1
        << QByteArray("tic");

    QTest::newRow("Size entire - offset exceeds size") 
        << QByteArray("Supercalifragilistic")
        << 20
        << 20
        << -1
        << -1
        << QByteArray();

    QTest::newRow("Size entire plus one - offset zero") 
        << QByteArray("Supercalifragilistic")
        << 21
        << 0
        << -1
        << -1
        << QByteArray("Supercalifragilistic");

    QTest::newRow("Size entire plus one - offset negative") 
        << QByteArray("Supercalifragilistic")
        << 21
        << -1
        << -1
        << -1
        << QByteArray("Supercalifragilistic");

    QTest::newRow("Size entire plus one - offset non-zero") 
        << QByteArray("Supercalifragilistic")
        << 21
        << 9
        << -1
        << -1
        << QByteArray("fragilistic");

    QTest::newRow("Size entire plus one - offset beyond last") 
        << QByteArray("Supercalifragilistic")
        << 21
        << 17
        << -1
        << -1
        << QByteArray("tic");

    QTest::newRow("Size entire plus one - offset exceeds size") 
        << QByteArray("Supercalifragilistic")
        << 21
        << 20
        << -1
        << -1
        << QByteArray();

    QTest::newRow("Size negative - offset zero") 
        << QByteArray("Supercalifragilistic")
        << -1
        << 0
        << -1
        << -1
        << QByteArray("Supercalifragilistic");

    QTest::newRow("Size negative - offset negative") 
        << QByteArray("Supercalifragilistic")
        << -1
        << -1
        << -1
        << -1
        << QByteArray("Supercalifragilistic");

    QTest::newRow("Size negative - offset non-zero") 
        << QByteArray("Supercalifragilistic")
        << -1
        << 9
        << -1
        << -1
        << QByteArray("fragilistic");

    QTest::newRow("Size negative - offset beyond last") 
        << QByteArray("Supercalifragilistic")
        << -1
        << 17
        << -1
        << -1
        << QByteArray("tic");

    QTest::newRow("Size negative - offset exceeds size") 
        << QByteArray("Supercalifragilistic")
        << -1
        << 20
        << -1
        << -1
        << QByteArray();

    QTest::newRow("Nested size zero - offset zero") 
        << QByteArray("Supercalifragilistic")
        << 0
        << 0
        << 8
        << 5
        << QByteArray();

    QTest::newRow("Nested size zero - offset non-zero") 
        << QByteArray("Supercalifragilistic")
        << 0
        << 5
        << 8
        << 5
        << QByteArray();

    QTest::newRow("Nested size zero - offset negative") 
        << QByteArray("Supercalifragilistic")
        << 0
        << -5
        << 8
        << 5
        << QByteArray();

    QTest::newRow("Nested size one - offset zero") 
        << QByteArray("Supercalifragilistic")
        << 1
        << 0
        << 8
        << 5
        << QByteArray("c");

    QTest::newRow("Nested size one - offset negative") 
        << QByteArray("Supercalifragilistic")
        << 1
        << -1
        << 8
        << 5
        << QByteArray("c");

    QTest::newRow("Nested size one - offset non-zero") 
        << QByteArray("Supercalifragilistic")
        << 1
        << 4
        << 8
        << 5
        << QByteArray("f");

    QTest::newRow("Nested size one - offset last") 
        << QByteArray("Supercalifragilistic")
        << 1
        << 7
        << 8
        << 5
        << QByteArray("g");

    QTest::newRow("Nested size one - offset beyond last") 
        << QByteArray("Supercalifragilistic")
        << 1
        << 8
        << 8
        << 5
        << QByteArray();

    QTest::newRow("Nested size greater than one - offset zero") 
        << QByteArray("Supercalifragilistic")
        << 5
        << 0
        << 8
        << 5
        << QByteArray("calif");

    QTest::newRow("Nested size greater than one - offset negative") 
        << QByteArray("Supercalifragilistic")
        << 5
        << -1
        << 8
        << 5
        << QByteArray("calif");

    QTest::newRow("Nested size greater than one - offset non-zero") 
        << QByteArray("Supercalifragilistic")
        << 5
        << 2
        << 8
        << 5
        << QByteArray("lifra");

    QTest::newRow("Nested size greater than one - offset last") 
        << QByteArray("Supercalifragilistic")
        << 5
        << 3
        << 8
        << 5
        << QByteArray("ifrag");

    QTest::newRow("Nested size greater than one - offset beyond last") 
        << QByteArray("Supercalifragilistic")
        << 5
        << 5
        << 8
        << 5
        << QByteArray("rag");

    QTest::newRow("Nested size greater than one - offset exceeds size") 
        << QByteArray("Supercalifragilistic")
        << 5
        << 8
        << 8
        << 5
        << QByteArray();

    QTest::newRow("Nested size entire - offset zero") 
        << QByteArray("Supercalifragilistic")
        << 8
        << 0
        << 8
        << 5
        << QByteArray("califrag");

    QTest::newRow("Nested size entire - offset negative") 
        << QByteArray("Supercalifragilistic")
        << 8
        << -1
        << 8
        << 5
        << QByteArray("califrag");

    QTest::newRow("Nested size entire - offset beyond last") 
        << QByteArray("Supercalifragilistic")
        << 8
        << 2
        << 8
        << 5
        << QByteArray("lifrag");

    QTest::newRow("Nested size entire - offset exceeds size") 
        << QByteArray("Supercalifragilistic")
        << 8
        << 8
        << 8
        << 5
        << QByteArray();

    QTest::newRow("Nested size entire plus one - offset zero") 
        << QByteArray("Supercalifragilistic")
        << 9
        << 0
        << 8
        << 5
        << QByteArray("califrag");

    QTest::newRow("Nested size entire plus one - offset negative") 
        << QByteArray("Supercalifragilistic")
        << 9
        << -1
        << 8
        << 5
        << QByteArray("califrag");

    QTest::newRow("Nested size entire plus one - offset beyond last") 
        << QByteArray("Supercalifragilistic")
        << 9
        << 2
        << 8
        << 5
        << QByteArray("lifrag");

    QTest::newRow("Nested size entire plus one - offset exceeds size") 
        << QByteArray("Supercalifragilistic")
        << 9
        << 8
        << 8
        << 5
        << QByteArray();

    QTest::newRow("Nested size negative - offset zero") 
        << QByteArray("Supercalifragilistic")
        << -1
        << 0
        << 8
        << 5
        << QByteArray("califrag");

    QTest::newRow("Nested size negative - offset negative") 
        << QByteArray("Supercalifragilistic")
        << -1
        << -1
        << 8
        << 5
        << QByteArray("califrag");

    QTest::newRow("Nested size negative - offset beyond last") 
        << QByteArray("Supercalifragilistic")
        << -1
        << 2
        << 8
        << 5
        << QByteArray("lifrag");

    QTest::newRow("Nested size negative - offset exceeds size") 
        << QByteArray("Supercalifragilistic")
        << -1
        << 8
        << 8
        << 5
        << QByteArray();
}

void tst_LongString::mid()
{
    QFETCH( QByteArray, source );
    QFETCH( int, size );
    QFETCH( int, from );
    QFETCH( int, nested_size );
    QFETCH( int, nested_from );
    QFETCH( QByteArray, result );

    LongString ls( source );
    LongString comparator;

    if ( nested_size == -1 )
    {
        comparator = ls.mid( from, size );
    }
    else
    {
        LongString nested = ls.mid( nested_from, nested_size );
        comparator = nested.mid( from, size );
    }

    QCOMPARE( comparator.toQByteArray(), result );
    QCOMPARE( comparator.length(), result.length() ); 
    QCOMPARE( comparator.isEmpty(), result.isEmpty() ); 

    QDataStream* in = comparator.dataStream();
    char buffer[256] = { 0 };
    int len = in->readRawData(buffer, 256);
    delete in;

    QByteArray streamOutput = QByteArray::fromRawData(buffer, len);
    QCOMPARE( streamOutput, result);
    
    // Ensure that indexOf works correctly on result of mid
    verifyIndexOf( comparator, streamOutput );
}

void tst_LongString::length()
{
    DEPENDS_ON(mid());
}

void tst_LongString::isEmpty()
{
    DEPENDS_ON(mid());
}

void tst_LongString::toQByteArray()
{
    DEPENDS_ON(mid());
}

void tst_LongString::dataStream()
{
    DEPENDS_ON(mid());
}

