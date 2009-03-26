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
// Maths libraries
#include <math.h>

#include "doubleinstruction.h"
#ifdef ENABLE_INTEGER
#include "integerdata.h"
#endif
#ifdef ENABLE_FRACTION
#include "fractiondata.h"
#endif

#include "engine.h"

int isOddInt(double value) {
    double temp;
    return ( modf(value, &temp) == 0.0 && modf(value/2, &temp) == 0.5 );
}

// Base class
BaseDoubleInstruction::BaseDoubleInstruction() {
    retType = type = "Double"; // No tr
}

BaseDoubleInstruction::~BaseDoubleInstruction(){};

void BaseDoubleInstruction::eval() {
    DoubleData *acc = (DoubleData *)systemEngine->getData();
    if (argCount == 1)
        doEvalI(acc);
    else {
        DoubleData *num = (DoubleData *)systemEngine->getData();
        doEval(num,acc);
        delete num;
    }
    delete acc;
}

void BaseDoubleInstruction::doEval(DoubleData *acc, DoubleData *num) {
    systemEngine->putData(num);
    systemEngine->putData(acc);
}

void BaseDoubleInstruction::doEvalI(DoubleData *acc) {
    systemEngine->putData(acc);
}

// Factory
iDoubleFactory::iDoubleFactory():Instruction() {
    retType = type = "Double"; // No tr
    name = "Factory"; // No tr
}

void iDoubleFactory::eval() {
    DoubleData *newData = new DoubleData();
    newData->clear();
    systemEngine->putData(newData);
}

// Copy
iDoubleCopy::iDoubleCopy():Instruction() {
    name = "Copy"; // No tr
    retType = type = "Double"; // No tr
    argCount = 1;
}

void iDoubleCopy::eval() {
    DoubleData *src = (DoubleData *)systemEngine->getData();
    DoubleData *tgt = new DoubleData();
    tgt->clear();
    tgt->set(((DoubleData *)src)->get());
    systemEngine->putData(src);
    systemEngine->putData(tgt);
};

#ifdef ENABLE_INTEGER
iConvertIntDouble::iConvertIntDouble():Instruction() {
    name = "Convert"; // No tr
    typeOne = "Int"; // No tr
    typeTwo = "Double"; // No tr
}
void iConvertIntDouble::eval(Data *d) {
    DoubleData *ret = new DoubleData();;
    IntegerData *i = (IntegerData *)d;
    ret->set((double)i->get());
    return ret;
}
#endif

#ifdef ENABLE_FRACTION
iConvertFractionDouble::iConvertFractionDouble():Instruction() {
    name = "Convert"; // No tr
    type = "Fraction"; // No tr
    retType = "Double"; // No tr
}

void iConvertFractionDouble::eval() {
    DoubleData *ret = new DoubleData();
    FractionData *f = (FractionData *)systemEngine->getData();
    if (!f->getDenominator()) {
        systemEngine->setError(eDivZero);
        ret->set(0);
    } else {
        double num = f->getNumerator();
        double den = f->getDenominator();
        double val = num / den;
        ret->set(val);
    }
    delete f;
    systemEngine->putData(ret);
}
#endif
// Mathematical functions

iAddDoubleDouble::iAddDoubleDouble():BaseDoubleInstruction() {
    name = "Add"; // No tr
    precedence = 10;
    displayString = "+";
    argCount = 2;
}

iSubtractDoubleDouble::iSubtractDoubleDouble():BaseDoubleInstruction() {
    name = "Subtract"; // No tr
    precedence = 10;
    displayString = "-";
    argCount = 2;
}

iDivideDoubleDouble::iDivideDoubleDouble():BaseDoubleInstruction() {
    name = "Divide"; // No tr
    precedence = 15;
    displayString = "/";
    argCount = 2;
}

iMultiplyDoubleDouble::iMultiplyDoubleDouble():BaseDoubleInstruction() {
    name = "Multiply"; // No tr
    precedence = 15;
    displayString = "x";
    argCount = 2;
}

void iAddDoubleDouble::doEval(DoubleData *acc, DoubleData *num) {
    DoubleData *result = new DoubleData();
    result->set(acc->get() + num->get());
    systemEngine->putData(result);
}

void iSubtractDoubleDouble::doEval(DoubleData *acc, DoubleData *num) {
    DoubleData *result = new DoubleData();
    result->set(acc->get() - num->get());
    systemEngine->putData(result);
}

void iMultiplyDoubleDouble::doEval(DoubleData *acc, DoubleData *num) {
    DoubleData *result = new DoubleData();
    result->set(num->get() * acc->get());
    systemEngine->putData(result);
}

void iDivideDoubleDouble::doEval(DoubleData *acc, DoubleData *num) {
    if (num->get() == 0) {
        systemEngine->setError(eDivZero);
        return;
    }
    DoubleData *result = new DoubleData();
    result->set( acc->get() / num->get() );
    systemEngine->putData(result);
}

iDoubleNegate::iDoubleNegate():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "Negate"; // No tr
}

void iDoubleNegate::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( 0 - acc->get() );
    result->setEdited(true);
    systemEngine->putData(result);
}

#ifdef ENABLE_SCIENCE
void iDoublePow::doEval(DoubleData *acc, DoubleData *num) {
    DoubleData *result = new DoubleData();
    result->set( pow(acc->get(),num->get()) );
    systemEngine->putData(result);
}
void iDoubleXRootY::doEval(DoubleData *acc,DoubleData *num) {
    DoubleData *result = new DoubleData();
    if (acc->get() < 0 && isOddInt(num->get()))
        result->set( -1 * pow(-1*acc->get(), 1 / num->get()) );
    else
        result->set( pow(acc->get(), 1 / num->get()) );
    systemEngine->putData(result);
}

// Immediate
//trigonometric functions radians mode
void iDoubleSinRad::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( sin(acc->get()) );
    systemEngine->putData(result);
}

void iDoubleCosRad::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( cos(acc->get()) );
    systemEngine->putData(result);
}

void iDoubleTanRad::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( tan(acc->get()) );
    systemEngine->putData(result);
}

void iDoubleASinRad::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( asin(acc->get()) );
    systemEngine->putData(result);
}

void iDoubleACosRad::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( acos(acc->get()) );
    systemEngine->putData(result);
}

void iDoubleATanRad::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( atan(acc->get()) );
    systemEngine->putData(result);
}

//trigonometric functions - degree mode
void iDoubleSinDeg::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( sin(acc->get()*M_PI/180) );
    systemEngine->putData(result);
}

void iDoubleCosDeg::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( cos(acc->get()*M_PI/180) );
    systemEngine->putData(result);
}

void iDoubleTanDeg::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( tan(acc->get()*M_PI/180) );
    systemEngine->putData(result);
}

void iDoubleASinDeg::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( asin(acc->get())*180/M_PI );
    systemEngine->putData(result);
}

void iDoubleACosDeg::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( acos(acc->get())*180/M_PI );
    systemEngine->putData(result);
}

void iDoubleATanDeg::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( atan(acc->get())*180/M_PI );
    systemEngine->putData(result);
}

//trigonometric functions - gradians mode
void iDoubleSinGra::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( sin(acc->get()*M_PI/200) );
    systemEngine->putData(result);
}

void iDoubleCosGra::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( cos(acc->get()*M_PI/200) );
    systemEngine->putData(result);
}

void iDoubleTanGra::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( tan(acc->get()*M_PI/200) );
    systemEngine->putData(result);
}

void iDoubleASinGra::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( asin(acc->get())*200/M_PI );
    systemEngine->putData(result);
}

void iDoubleACosGra::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( acos(acc->get())*200/M_PI );
    systemEngine->putData(result);
}

void iDoubleATanGra::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( atan(acc->get())*200/M_PI );
    systemEngine->putData(result);
}

void iDoubleLog::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( log10(acc->get()) );
    systemEngine->putData(result);
}

void iDoubleLn::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( log(acc->get()) );
    systemEngine->putData(result);
}

void iDoubleExp::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( exp(acc->get()) );
    systemEngine->putData(result);
}

void iDoubleOneOverX::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( 1 / acc->get() );
    systemEngine->putData(result);
}

void iDoubleFactorial::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    if (acc->get() < 0) {
        systemEngine->setError(eNonPositive);
    } else if ( acc->get() > 180 ) {
        systemEngine->setError(eOutOfRange);
    } else if ( acc->get() != int(acc->get()) ) {
        systemEngine->setError(eNonInteger);
    } else {
        int count = int(acc->get());
        result->set(1);
        while (count) {
            result->set(result->get()*count);
            count--;
        }
    }
    systemEngine->putData(result);
}

void iDoubleSquareRoot::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( sqrt(acc->get()) );
    systemEngine->putData(result);
}

void iDoubleCubeRoot::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( cbrt(acc->get()) );
    systemEngine->putData(result);
}

void iDoubleSquare::doEvalI(DoubleData *acc) {
    DoubleData *result = new DoubleData();
    result->set( acc->get() * acc->get() );
    systemEngine->putData(result);
}

// Normal instructions with full precedence
iDoublePow::iDoublePow():BaseDoubleInstruction() {
    name = "Pow"; // No tr
    precedence = 20;
    displayString = "^";
    argCount = 2;
}

iDoubleSinRad::iDoubleSinRad():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "SinRad"; // No tr
}

iDoubleCosRad::iDoubleCosRad():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "CosRad"; // No tr
}

iDoubleTanRad::iDoubleTanRad():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "TanRad"; // No tr
}

iDoubleASinRad::iDoubleASinRad():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "aSinRad"; // No tr
}

iDoubleACosRad::iDoubleACosRad():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "aCosRad"; // No tr
}

iDoubleATanRad::iDoubleATanRad():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "aTanRad"; // No tr
}

iDoubleSinDeg::iDoubleSinDeg():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "SinDeg"; // No tr
}

iDoubleCosDeg::iDoubleCosDeg():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "CosDeg"; // No tr
}

iDoubleTanDeg::iDoubleTanDeg():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "TanDeg"; // No tr
}

iDoubleASinDeg::iDoubleASinDeg():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "aSinDeg"; // No tr
}

iDoubleACosDeg::iDoubleACosDeg():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "aCosDeg"; // No tr
}

iDoubleATanDeg::iDoubleATanDeg():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "aTanDeg"; // No tr
}

iDoubleSinGra::iDoubleSinGra():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "SinGra"; // No tr
}

iDoubleCosGra::iDoubleCosGra():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "CosGra"; // No tr
}

iDoubleTanGra::iDoubleTanGra():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "TanGra"; // No tr
}

iDoubleASinGra::iDoubleASinGra():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "aSinGra"; // No tr
}

iDoubleACosGra::iDoubleACosGra():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "aCosGra"; // No tr
}

iDoubleATanGra::iDoubleATanGra():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "aTanGra"; // No tr
}

iDoubleLog::iDoubleLog():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "Log"; // No tr
}

iDoubleLn::iDoubleLn():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "Ln"; // No tr
}

iDoubleExp::iDoubleExp():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "Exp"; // No tr
}

iDoubleOneOverX::iDoubleOneOverX():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "One over x"; // No tr
}

iDoubleFactorial::iDoubleFactorial():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "Factorial"; // No tr
}

iDoubleSquareRoot::iDoubleSquareRoot():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "Square root"; // No tr
}

iDoubleCubeRoot::iDoubleCubeRoot():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "Cube root"; // No tr
}

iDoubleXRootY::iDoubleXRootY():BaseDoubleInstruction() {
    name = "X root Y"; // No tr
    precedence = 20;
    displayString = "to the root";
    argCount = 2;
}

iDoubleSquare::iDoubleSquare():BaseDoubleInstruction() {
    precedence = 0;
    argCount = 1;
    name = "Square"; // No tr
}

#endif //ENABLE_SCIENCE

