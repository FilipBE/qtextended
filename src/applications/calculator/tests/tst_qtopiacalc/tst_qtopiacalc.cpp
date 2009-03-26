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

#define protected public
#include <engine.h>
#undef protected

//TESTED_COMPONENT=PIM: Productivity: Calculator (18572)
//TESTED_CLASS=Engine
//TESTED_FILES=src/applications/calculator/engine.cpp

class tst_QtopiaCalc: public QObject
{
    Q_OBJECT

protected slots:
    void initTestCase();
    void cleanupTestCase();

private slots:
    void double_instruction();
    void double_instruction_data();
};

QTEST_MAIN( tst_QtopiaCalc )
#include "tst_qtopiacalc.moc"

Engine *systemEngine;

void tst_QtopiaCalc::initTestCase()
{
    systemEngine = new Engine();
}

void tst_QtopiaCalc::cleanupTestCase()
{
    delete systemEngine;
}

void tst_QtopiaCalc::double_instruction_data()
{
    QTest::addColumn<QStringList>( "list" );
    QTest::addColumn<QString>( "result" );

    /*
    double layout (using 53 bit precision)

    S EEEEEEEEEEE FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    0 1        11 12                                                63

    If 0<E<2047 then V=(-1)^S * 2 ^ (E-1023) * (1.F) where F is in binary

    2047-1023 = approx 2^1024 max = 1+e1024 (thats way to much surely only ~ 1+e20?)

    */

    QTest::newRow("Add-1") << QString("0;Add;0").split(';') << QString("0");
    QTest::newRow("Add-2") << QString("1;Add;0").split(';') << QString("1");
    QTest::newRow("Add-3") << QString("0;Add;1").split(';') << QString("1");
    QTest::newRow("Add-4") << QString("4;Add;33").split(';') << QString("37");
    QTest::newRow("Add-5") << QString("9999999999999999;Add;1").split(';') << QString("Calc surpasses limit"); //10000000000000000
    QTest::newRow("Add-6") << QString("0.9999999999;Add;0.0000000001").split(';') << QString("1");
    QTest::newRow("Add-7") << QString("4;Add;33;Add;12").split(';') << QString("49");

    QTest::newRow("Subtract-1") << QString("0;Subtract;0").split(';') << QString("0");
    QTest::newRow("Subtract-2") << QString("1;Subtract;0").split(';') << QString("1");
    QTest::newRow("Subtract-3") << QString("0;Subtract;1").split(';') << QString("-1");
    QTest::newRow("Subtract-4") << QString("1;Subtract;5").split(';') << QString("-4");
    QTest::newRow("Subtract-5") << QString("255;Subtract;1").split(';') << QString("254");
    QTest::newRow("Subtract-6") << QString("0.3456;Subtract;1.345").split(';') << QString("-0.9994");
    QTest::newRow("Subtract-7") << QString("1;Subtract;0.0000001").split(';') << QString("0.9999999");
    QTest::newRow("Subtract-8") << QString("1;Subtract;0.00000001").split(';') << QString("0.99999999");
    QTest::newRow("Subtract-9") << QString("0.000000000000001;Subtract;0.000000000000001").split(';') << QString("0"); //gets rounded down
    QTest::newRow("Subtract-10") << QString("3.01;Subtract;1.99;Subtract;1").split(';') << QString("0.02");

    QTest::newRow("Divide-1") << QString("5;Divide;0").split(';') << QString("Divide by zero error");
    QTest::newRow("Divide-2") << QString("0;Divide;3").split(';') << QString("0");
    QTest::newRow("Divide-3") << QString("10;Divide;1").split(';') << QString("10");
    QTest::newRow("Divide-4") << QString("8;Divide;0.1").split(';') << QString("80");
    QTest::newRow("Divide-5") << QString("1024;Divide;0.25").split(';') << QString("4096");
    QTest::newRow("Divide-6") << QString("1;Divide;3").split(';') << QString("0.3333333333");
    QTest::newRow("Divide-7") << QString("0.000000000000001;Divide;10").split(';') << QString("0"); //error expected
    QTest::newRow("Divide-8") << QString("1;Divide;2;Divide;3").split(';') << QString("0.1666666667");

    QTest::newRow("Multiply-1") << QString("482;Multiply;5.67").split(';') << QString("2732.94");
    QTest::newRow("Multiply-2") << QString("10;Multiply;0").split(';') << QString("0");
    QTest::newRow("Multiply-3") << QString("0;Multiply;1").split(';') << QString("0");
    QTest::newRow("Multiply-4") << QString("0;Multiply;0").split(';') << QString("0");
    QTest::newRow("Multiply-5") << QString("9999999999999999;Multiply;2").split(';') << QString("Calc surpasses limit");
    QTest::newRow("Multiply-6") << QString("1.5;Multiply;2;Multiply;3").split(';') << QString("9");

    QTest::newRow("Precedence-1") << QString("1;Add;2;Subtract;3;Divide;4;Multiply;5").split(';') << QString("-0.75");          // + - / *
    QTest::newRow("Precedence-2") << QString("1;Multiply;2;Add;3;Subtract;4;Divide;5").split(';') << QString("4.2");            // * + - /
    QTest::newRow("Precedence-3") << QString("1;Divide;2;Multiply;3;Add;4;Subtract;5").split(';') << QString("0.5");            // / * + -
    QTest::newRow("Precedence-4") << QString("1;Subtract;2;Divide;3;Multiply;4;Add;5").split(';') << QString("3.3333333333");   // - / * +

    // misc precedence tests
    QTest::newRow("Precedence-5") << QString("1;Multiply;2;Add;3;Divide;4").split(';') << QString("2.75");
    QTest::newRow("Precedence-6") << QString("1;Subtract;2;Add;3").split(';') << QString("2");
    QTest::newRow("Precedence-7") << QString("1;Divide;2;Multiply;5").split(';') << QString("2.5");
    QTest::newRow("Precedence-8") << QString("0;Multiply;4;Divide;10").split(';') << QString("0"); //error expected
    QTest::newRow("Precedence-9") << QString("1;Multiply;2;Divide;3;Multiply;4;Divide;1.3333333333").split(';') << QString("2.0000000001"); //approx

    // testing if scientific notation is acceptable
    //QTest::newRow("ScientificNotation-1") << QString("1.1234e6;Subtract;1.123e6").split(';') << QString("4000");

#ifdef CHECK_NEGATION
    QTest::newRow("Add-8") << QString("-2;Add;1").split(';') << QString("-1");
    QTest::newRow("Add-9") << QString("6;Add;-3").split(';') << QString("3");
    QTest::newRow("Add-10") << QString("-1.2;Add;-0.812345").split(';') << QString("-2.012345");
    QTest::newRow("Add-11") << QString("-0.56789;Add;7").split(';') << QString("6.43211");
    QTest::newRow("Add-12") << QString("-9999999999999999;Add;-1").split(';') << QString("0"); //error expected

    QTest::newRow("Subtract-11") << QString("9;Subtract;-9").split(';') << QString("18");
    QTest::newRow("Subtract-12") << QString("-1;Subtract;-1").split(';') << QString("0");

    QTest::newRow("Divide-9") << QString("3.43758;Divide;-2.45").split(';') << QString("-1.40309387755102");
    QTest::newRow("Divide-10") << QString("-1.23456789;Divide;-1").split(';') << QString("1.23456789");

    QTest::newRow("Multiply-7") << QString("65598;Multiply;-0.1").split(';') << QString("-6559.8");
    QTest::newRow("Multiply-8") << QString("-2;Multiply;-5.25").split(';') << QString("10.5");
    QTest::newRow("Multiply-9") << QString("-834759;Multiply;2984205").split(';') << QString("-2491091981595");

    QTest::newRow("Precedence-7") << QString("1.0000000001;Subtract;-0.9999999999;Divide;2;Multiply;0.5;Add;0.3").split(';') << QString("2.3");
#endif
}

/*
    \req QTOPIA-231

    \groups
*/
void tst_QtopiaCalc::double_instruction()
{
    QFETCH( QStringList, list );
    QFETCH( QString, result );

    //QEXPECT_FAIL("Divide-8", "Task 177589 - calculator doesn't round properly", Continue);

    systemEngine->hardReset();
    systemEngine->setAccType("Double");

    bool instruction = false;
    for (int i = 0; i < list.count(); ++i) {
        if (instruction) {
            systemEngine->pushInstruction(list[i]);
            instruction = false;
        } else {
            systemEngine->push(list[i]);
            instruction = true;
        }
    }

    systemEngine->evaluate();
    if (!systemEngine->checkState()) {
        QTEST( systemEngine->errorString, "result" );
    } else {
        QTEST( systemEngine->getDisplay(), "result" );
    }
}

