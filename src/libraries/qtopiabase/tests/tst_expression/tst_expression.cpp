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

// unit tests for Qtopia's expression evaluator

// test all operators
// test operators complex case
// test expression class reports right things (returns values, signals)
// test terms/valuespace stuff

#include <QObject>
#include <QTest>
#include <QtTest/qsignalspy.h>
#include <QApplication>
#include <qexpressionevaluator.h>
#include <QVariant>
#include <qvaluespace.h>

//TESTED_CLASS=QExpressionEvaluator
//TESTED_FILES=src/libraries/qtopiail/framework/qexpressionevaluator.cpp

/*
        Multiply = '*',
        Divide = '/',
        Plus = '+',
        Minus = '-',
        Mod = '%',
        LParen =  '(',
        RParen = ')',
        Gt = '>',
        Lt = '<',
        Assign = '=',
        Not = '!',
        Cat = '.',
        GtEq = 200, // ensure unambiguous
        LtEq,
        Equal,
        NotEqual,
        String,
        Bool,
        LogicalOr,
        LogicalAnd,
        Integer,
        Double,
        Unknown,
        Term,
        Null
*/

#define DOINT(expression) expectResult(#expression, QVariant(expression), QVariant::Int);
#define DOSTR(expression,result) expectResult(#expression, QVariant(result), QVariant::String);
#define DODBL(expression) expectResult(#expression, QVariant(expression), QVariant::Double);
#define DOBOOL(expression) expectResult(#expression, QVariant(expression), QVariant::Bool);
#define DOFIXED(expression,expectedvalue) expectResult(#expression, QVariant(expectedvalue), QVariant::String, true);
#define EI(expression) expectInvalid(#expression);
#define ExpandExpr(s) #s

class tst_Expression : public QObject
{
    Q_OBJECT
public:
    tst_Expression() {}
    //void testOperatorsInteger();

private slots:
    /* All operators possible for integer operands. */
    void IntegerAdd() { DOINT(10+2) }
    void IntegerSub() { DOINT(10-2) }
    void IntegerMul() { DOINT(10*2) }
    void IntegerDiv() { DODBL(10/2) }
    void IntegerMod() { DOINT(10 % 2) }
    void IntegerLT() { DOBOOL(10<2) }
    void IntegerGT() { DOBOOL(10>2) }
    void IntegerGTEQ() { DOBOOL(10>=2) }
    void IntegerLTEQ() { DOBOOL(10<=2) }
    void IntegerEQ() { DOBOOL(10==2) }
    void IntegerNEQ() { DOBOOL(10!=2) }
    void IntegerOr() { DOBOOL(0||10) }
    void IntegerAnd() { DOBOOL(10&&0) }
    void IntegerNot() { DOINT(!0) }
    void IntegerUMinus() { DOINT(-19) }
    void IntegerCat() { DOSTR(10 . 2, QString::number(10)+QString::number(2)) }

    /* All operators possible for double operands. */
    void DoubleAdd() { DODBL(10.3+2.16) }
    void DoubleSub() { DODBL(10.3-2.16) }
    void DoubleMul() { DODBL(10.3*2.16) }
    void DoubleDiv() { DODBL(10.3/2.16) }
    //void DoubleMod() { DODBL(10.3 % 2.16) }
    void DoubleLT() { DOBOOL(10.3<2.16) }
    void DoubleGT() { DOBOOL(10.3>2.16) }
    void DoubleGTEQ() { DOBOOL(10.3>=2.16) }
    void DoubleLTEQ() { DOBOOL(10.3<=2.16) }
    void DoubleEQ() { DOBOOL(10.3==2.16) }
    void DoubleNEQ() { DOBOOL(10.3!=2.16) }
    void DoubleOr() { DOBOOL(0.0||10.3) }
    void DoubleAnd() { DOBOOL(10.3&&0.0) }
    void DoubleNot() { DOBOOL(!0.0) }
    void DoubleUMinus() { DODBL(-19.5534) }
    // compare actual DoubleCat result with a QString concatenation.
    // the double numbers are never actually converted to doubles for the cat operator by the expression evaluator.
    // so even if precision loss would occur in representing the given numbers, the expression evaluator
    // concatenates the 2 given numbers exactly into a string
    // comparing to QString::number(19.5534)+QString::number(16.45311) would fail because the calculation has precision loss
    void DoubleCat() { DOSTR(19.5534 . 16.45311, QString("19.5534")+QString("16.45311")) }

    /* All operators possible for string operands */
    void StringCat() { DOSTR("10" . "2", QString("10")+QString("2")) }
    void StringEQ() { DOSTR("10" == "2", QString("10") == QString("2")) };
    void StringNEQ() { DOSTR("10" != "2", QString("10") != QString("2")) };
    void StringOr() { DOSTR("" || "2", !QString("").isEmpty() || !QString("2").isEmpty()) }
    void StringAnd() { DOSTR("2" && "", !QString("2").isEmpty() && !QString("").isEmpty()) }
    void StringNot() { DOSTR(!"", QString("").isEmpty()) }

    /* All operators possible for bool operands */
    void BoolLT() { DOBOOL(false < true) }
    void BoolGT() { DOBOOL(false > true) }
    void BoolLTEQ() { DOBOOL(false <= true) }
    void BoolGTEQ() { DOBOOL(false >= true) }
    void BoolEQ() { DOBOOL(false == false) }
    void BoolNEQ() { DOBOOL(false != false) }
    void BoolOr() { DOBOOL(false || true) }
    void BoolAnd() { DOBOOL(true && false) }
    void BoolNot() { DOBOOL(!false) }
    void BoolCat() { DOSTR(true . false, QString::number(true)+QString::number(false)) }

    /* Successful Coercion/Mixed operands */
    /*Integer  & String */
    /* arithmetic */
    void IntegerStringAdd() { expectResult(ExpandExpr("10" + 2), QString("10").toInt() + 2, QVariant::Int); }
    void IntegerStringSub() { expectResult(ExpandExpr("10" - 2), QString("10").toInt() - 2, QVariant::Int); }
    void IntegerStringMul() { expectResult(ExpandExpr("10" * 2), QString("10").toInt() * 2, QVariant::Int); }
    void IntegerStringDiv() { expectResult(ExpandExpr("10" / 2), QString("10").toInt() / 2, QVariant::Int); }
    void IntegerStringMod() { expectResult(ExpandExpr("10" % 2), QString("10").toInt() % 2, QVariant::Int); }
    /* comparison */
    void IntegerStringLT() { expectResult(ExpandExpr("10" < 2), QString("10").toInt() < 2, QVariant::Bool); }
    void IntegerStringGT() { expectResult(ExpandExpr("10" > 2), QString("10").toInt() > 2, QVariant::Bool); }
    void IntegerStringGTEQ() { expectResult(ExpandExpr("10" >= 2),QString("10").toInt() >= 2,QVariant::Bool); }
    void IntegerStringLTEQ() { expectResult(ExpandExpr("10" <= 2),QString("10").toInt() <= 2,QVariant::Bool); }
    void IntegerStringEQ() { expectResult(ExpandExpr("10" == 2), QString("10").toInt() == 2, QVariant::Bool); }
    void IntegerStringNEQ() { expectResult(ExpandExpr("10" != 2), QString("10").toInt() != 2, QVariant::Bool); }
    void IntegerStringOr() { expectResult(ExpandExpr("0" || 2), QString("0").toInt() || 2, QVariant::Bool); }
    void IntegerStringAnd() { expectResult(ExpandExpr("2" && 0), QString("2").toInt() && 0, QVariant::Bool); }
    void IntegerStringNotAdd() { expectResult(ExpandExpr(0 + !""), 0 + QString("").isEmpty(), QVariant::Int); }
    void IntegerStringCat() { expectResult(ExpandExpr("0101" . 1010), QString("0101") + QString::number(1010), QVariant::String); }

    /* Integer & Double */
    /* arithmetic */
    void IntegerDoubleAdd() { DODBL(10+2.5) }
    void IntegerDoubleSub() { DODBL(10-2.5) }
    void IntegerDoubleMul() { DODBL(10*2.5) }
    void IntegerDoubleDiv() { DODBL(10/2.5) }
    /* comparison */
    void IntegerDoubleLT() { DOBOOL(10<2.5) }
    void IntegerDoubleGT() { DOBOOL(10>2.5) }
    void IntegerDoubleGTEQ() { DOBOOL(10>=2.5) }
    void IntegerDoubleLTEQ() { DOBOOL(10<=2.5) }
    void IntegerDoubleEQ() { DOBOOL(10==2.5) }
    void IntegerDoubleNEQ() { DOBOOL(10!=2.5) }
    void IntegerDoubleOr() { DOBOOL(0||10.3) }
    void IntegerDoubleAnd() { DOBOOL(10.3&&0) }

    /* Integer & Bool */
    void IntegerBoolAdd() { DOINT(10 + true); }
    void IntegerBoolSub() { DOINT(10 - false); }
    void IntegerBoolMul() { DOINT(10 * true); }
    void IntegerBoolDiv() { DODBL(10 / true); }
    void IntegerBoolMod() { DOINT(10 % true); }
    void IntegerBoolLT() { DOBOOL(10 < false); }
    void IntegerBoolGT() { DOBOOL(10 > false); }
    void IntegerBoolGTEQ() { DOBOOL(10 >= true); }
    void IntegerBoolLTEQ() { DOBOOL(10 <= true); }
    void IntegerBoolEQ() { DOBOOL(10 == true); }
    void IntegerBoolNEQ() { DOBOOL(10 != false); }
    void IntegerBoolOr() { DOBOOL(0 || true); }
    void IntegerBoolAnd() { DOBOOL(true && 0); }
    void IntegerBoolNotAdd() { DOBOOL(0 + !true); }

    /* Double & String */
    void DoubleStringAdd() { expectResult(ExpandExpr("10.3" + 2.5), QString("10.3").toDouble() + 2.5, QVariant::Double); }
    void DoubleStringSub() { expectResult(ExpandExpr("10.3" - 2.5), QString("10.3").toDouble() - 2.5, QVariant::Double); }
    void DoubleStringMul() { expectResult(ExpandExpr("10.3" * 2.5), QString("10.3").toDouble() * 2.5, QVariant::Double); }
    void DoubleStringDiv() { expectResult(ExpandExpr("10.3" / 2.5), QString("10.3").toDouble() / 2.5, QVariant::Double); }
    void DoubleStringLT() { expectResult(ExpandExpr("10.3" < 2.5), QString("10.3").toDouble() < 2.5, QVariant::Bool); }
    void DoubleStringGT() { expectResult(ExpandExpr("10.3" > 2.5), QString("10.3").toDouble() > 2.5, QVariant::Bool); }
    void DoubleStringGTEQ() { expectResult(ExpandExpr("10.3" >= 2.5),QString("10.3").toDouble() >= 2.5,QVariant::Bool); }
    void DoubleStringLTEQ() { expectResult(ExpandExpr("10.3" <= 2.5),QString("10.3").toDouble() <= 2.5,QVariant::Bool); }
    void DoubleStringEQ() { expectResult(ExpandExpr("10.3" == 2.5), QString("10.3").toDouble() == 2.5, QVariant::Bool); }
    void DoubleStringNEQ() { expectResult(ExpandExpr("10.3" != 2.5), QString("10.3").toDouble() != 2.5, QVariant::Bool); }
    void DoubleStringOr() { expectResult(ExpandExpr("0.000" || 2.5), QString("0.000").toDouble() || 2.5, QVariant::Bool); }
    void DoubleStringAnd() { expectResult(ExpandExpr("2.5" && 0.00005), QString("2.5").toDouble() && 0.00005, QVariant::Bool); }
    void DoubleStringNotAdd() { expectResult(ExpandExpr(0.00032 + !"5"), 0.00032 + !QString("5").toDouble(), QVariant::Int); }
    void DoubleStringCat() { expectResult(ExpandExpr("758" . 38.7), QString("758") + QString::number(38.7), QVariant::String); }

    /* Double & Bool */
    void DoubleBoolAdd() { DOINT(10.3 + true); }
    void DoubleBoolSub() { DOINT(10.3 - false); }
    void DoubleBoolMul() { DOINT(10.3 * true); }
    void DoubleBoolDiv() { DODBL(10.3 / true); }
    void DoubleBoolLT() { DOBOOL(10.3 < false); }
    void DoubleBoolGT() { DOBOOL(10.3 > false); }
    void DoubleBoolGTEQ() { DOBOOL(10.3 >= true); }
    void DoubleBoolLTEQ() { DOBOOL(10.3 <= true); }
    void DoubleBoolEQ() { DOBOOL(10.3 == true); }
    void DoubleBoolNEQ() { DOBOOL(10.3 != true); }
    void DoubleBoolOr() { DOBOOL(0.3 || true); }
    void DoubleBoolAnd() { DOBOOL(true && 0); }
    void DoubleBoolNotAdd() { DOBOOL(0.005 + !true); }

    /*? Bool & String */
    void BoolStringGT() { expectResult(ExpandExpr(true > ""), true > !QString("").isEmpty(), QVariant::Bool); }
    void BoolStringLT() { expectResult(ExpandExpr(true < ""), true < !QString("").isEmpty(), QVariant::Bool); }
    void BoolStringGTEQ() { expectResult(ExpandExpr(true >= ""), true >= !QString("").isEmpty(), QVariant::Bool); }
    void BoolStringLTEQ() { expectResult(ExpandExpr(true <= ""), true <= !QString("").isEmpty(), QVariant::Bool); }

    void BoolStringEQ() { expectResult(ExpandExpr("1010" == true), !QString("1010").isEmpty(), QVariant::Bool); }
    void BoolStringNEQ() { expectResult(ExpandExpr("" == false), QString("").isEmpty(), QVariant::Bool); }
    void BoolStringCat() { expectResult(ExpandExpr("number" . true), QString("number1"), QVariant::String); }
    void BoolStringOr() { expectResult(ExpandExpr("" || true), !QString("").isEmpty() || true, QVariant::Bool); }

    /* Failures */
    /* Cannot coerce string operands for arithmetric operators. */
    void StringAdd() { EI("10" + "2"); }
    void StringSub() { EI("2" - "3"); }
    void StringMul() { EI("3" * "4"); }
    void StringDiv() { EI("4" / "2"); }
    void StringMod() { EI("9" % "3"); }
    void StringUMinus() { EI(-"16.5"); }
    /* Cannot coerce string operands for comparison operators. */
    void StringLT() { EI("10" < "5"); }
    void StringGT() { EI("10" > "5"); }
    void StringLTEQ() { EI("10" <= "5"); }
    void StringGTEQ() { EI("10" >= "5"); }

    /* Cannot coerce string operand with bool for arithmetric operators */
    void StringBoolAdd() { EI("10" + "true"); }
    void StringBoolSub() { EI(false - "2"); }
    void StringBoolMul() { EI(true * "2"); }
    void StringBoolDiv() { EI("10" / false); }
    void StringBoolMod() { EI("10" % false); }

    /* Cannot coerce all bool operands with arithmetic operators. */
    void BoolAdd() { EI(true + true); }
    void BoolSub() { EI(true - false) }
    void BoolMul() { EI(false * false); }
    void BoolDiv() { EI(false/true); }
    void BoolMod() { EI(true % false); }

    /* Doubles don't work with mod */
    void DoubleMod() { EI(53.23 % 16.321); }

    /* Complex expressions */
    void MixedOperands() { DODBL("5.5 * 6 + \"87.355\" - 16 / .42"); } 
    void ComplexBrackets() { DOINT(1 + (2 + (3 + (4 + (5 + 6) + (7 + 8) + 9)))); }
    void BracketsUMinus() { DODBL(45.3 - -(6.5+(7.33323+5))); }
    void IntegerMinusUMinus() { DOINT(30 - -5); }
    void ArithmeticWithCat() { expectResult(ExpandExpr(2*(3 * 6-(5.1) . "7")), 24, QVariant::Int); }
    void RelationalMixed() { expectResult(ExpandExpr("2.5 > 3.0 || 50 <= 49.9 || true == \"notempty\""), true, QVariant::Bool); }
    // TODO : more cases
    
    /* Fixed point support */
    void FixedPointAdd() { DOFIXED(.5 + .5, "1.0"); }
    void FixedPointSub() { DOFIXED(.2 - .1, "0.1"); }
    void FixedPointMul() { DOFIXED(.2 * 5.0, "1.0"); }
    void FixedPointDiv() { DOFIXED(1 / .1, "10.0"); }
    void FixedPointLT() { DOFIXED(.333332 < .333333, "true"); }
    void FixedPointGT() { DOFIXED(.333332 > .333333, "false"); }
    void FixedPointGTEQ() { DOFIXED(.666665 >= .666666, "false"); }
    void FixedPointLTEQ() { DOFIXED(.666665 <= .666666, "true"); }
    void FixedPointEQ() { DOFIXED(.9999999 == .9999999, "true"); }
    void FixedPointNEQ() { DOFIXED(10.5935423!=10.5935423, "false"); }
    void FixedPointOr() { DOFIXED(0.00000001 || 0.0000, "true"); }
    void FixedPointAnd() { DOFIXED(0.00000001&&0.0, "false"); }
    void FixedPointNot() { DOFIXED(!0.0, "true"); }
    void FixedPointUMinus() { DOFIXED(-19.3333333, "-19.3333333"); }

    /* Valuespace keys */
    void Valuespace() {
	QValueSpace::initValuespaceManager();
	QValueSpaceObject object("");
	object.setAttribute("/Test/Expression/MyKey", QVariant("helloworld"));
	object.sync();
	QValueSpaceItem item("/Test/Expression/MyKey");
	QCOMPARE(item.value().toString(), QString("helloworld"));
	QExpressionEvaluator expr;
	QSignalSpy ess(&expr, SIGNAL(termsChanged()));
	expr.setExpression("@/Test/Expression/MyKey");
	QCOMPARE(ess.count(), 0);
	QVERIFY(expr.isValid());
	QVERIFY(expr.evaluate());
	QVERIFY(expr.result().canConvert(QVariant::String));
	QCOMPARE(expr.result().toString(), QString("helloworld"));
	object.setAttribute("/Test/Expression/MyKey", QVariant("45.3"));
	object.sync();
	QCOMPARE(ess.count(), 1);
	ess.clear();
	expr.setExpression("@/Test/Expression/MyKey * 23.25");
	QVERIFY(expr.isValid());
	QVERIFY(expr.evaluate());
	QVERIFY(expr.result().canConvert(QVariant::Double));
	QVERIFY(expr.result().toDouble() == 45.3*23.25);
	QCOMPARE(ess.count(), 0);
    }

private:
    void expectResult(const QByteArray& data, const QVariant& expectedresult, const QVariant::Type& expectedresulttype, bool useFixedPoint = false ) {
	QExpressionEvaluator _testexpr;
	QVERIFY(_testexpr.floatingPointFormat() == QExpressionEvaluator::Double);
	if(useFixedPoint)
	    _testexpr.setFloatingPointFormat(QExpressionEvaluator::FixedPoint);
	_testexpr.setExpression(data);
	QVERIFY(_testexpr.isValid());
	QVERIFY(_testexpr.evaluate());
	QVariant actualresult = _testexpr.result();
	QVERIFY(!actualresult.isNull());
	switch(expectedresulttype) {
	    case QVariant::Int:
		QCOMPARE(actualresult.toInt(),expectedresult.toInt());
		break;
	    case QVariant::Double:
		QCOMPARE(actualresult.toDouble(), expectedresult.toDouble());
		break;
	    case QVariant::String:
		QCOMPARE(actualresult.toString(), expectedresult.toString());
		break;
	    case QVariant::Bool:
		QCOMPARE(actualresult.toBool(), expectedresult.toBool());
		break;
	    default:
		 QFAIL("Invalid returntype given for third argument of expectResult.");
		 return;
	}
    }

    void expectInvalid(const QByteArray& data) {
	QExpressionEvaluator _testexpr;
	_testexpr.setExpression(data);
	QVERIFY(!_testexpr.isValid());
    }

    bool m_usedFixedPoint;
};

QTEST_MAIN(tst_Expression)
#include "tst_expression.moc"
