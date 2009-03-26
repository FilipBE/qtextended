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

#include "phoneinstruction.h"
#include "engine.h"

#include <QApplication>
#include <QPalette>
#include <QPainter>

// Factory
iPhoneDoubleFactory::iPhoneDoubleFactory():Instruction() {
    retType = type = "Double"; // No tr
    name = "Factory"; // No tr
}
void iPhoneDoubleFactory::eval() {
    DoubleData *newData = new DoubleData();
    newData->clear();
    systemEngine->dStack.push(newData);
}

// Copy
iPhoneDoubleCopy::iPhoneDoubleCopy():Instruction() {
    name = "Copy"; // No tr
    retType = type = "Double"; // No tr
    argCount = 1;
}
void iPhoneDoubleCopy::eval() {
    DoubleData *src = (DoubleData *)systemEngine->dStack.pop();
    DoubleData *tgt = new DoubleData();
    tgt->clear();
    tgt->set(((DoubleData *)src)->get());
    systemEngine->dStack.push(src);
    systemEngine->dStack.push(tgt);
};

iEvaluateLine::iEvaluateLine():Instruction() {
    name = "EvaluateLine"; // No tr
    retType = type = "Double"; // No tr
    argCount = 2;
    precedence = 1;
}
QPixmap *iEvaluateLine::draw() {
    if (!cache) {
        cache = new QPixmap(60,3);
        cache->fill(Qt::transparent);
        QPainter p(cache);
        p.setPen(QApplication::palette().color(QPalette::Text));
        p.drawLine(0,0,60,0);
        p.drawLine(0,1,60,1);
    }
    return cache;
}

// Basic functions without precedence
iPhoneAddDoubleDouble::iPhoneAddDoubleDouble():Instruction() {
    name = "Add"; // No tr
    precedence = 10;
    displayString = "+";
    argCount = 2;
}
void iPhoneAddDoubleDouble::eval() {
    DoubleData *acc = (DoubleData *)systemEngine->getData();
    DoubleData *num = (DoubleData *)systemEngine->getData();
    DoubleData *result = new DoubleData();
    result->set(num->get() + acc->get());
    systemEngine->putData(result);
    delete num;
    delete acc;
}

iPhoneSubtractDoubleDouble::iPhoneSubtractDoubleDouble():Instruction() {
    name = "Subtract"; // No tr
    precedence = 10;
    displayString = "-";
    argCount = 2;
}
void iPhoneSubtractDoubleDouble::eval() {
    DoubleData *acc = (DoubleData *)systemEngine->getData();
    DoubleData *num = (DoubleData *)systemEngine->getData();
    DoubleData *result = new DoubleData();
    result->set(num->get() - acc->get());
    systemEngine->putData(result);
    delete num;
    delete acc;
}

iPhoneMultiplyDoubleDouble::iPhoneMultiplyDoubleDouble():Instruction() {
    name = "Multiply"; // No tr
    precedence = 10;
    displayString = "x";
    argCount = 2;
}
void iPhoneMultiplyDoubleDouble::eval() {
    DoubleData *acc = (DoubleData *)systemEngine->getData();
    DoubleData *num = (DoubleData *)systemEngine->getData();
    DoubleData *result = new DoubleData();
    result->set(num->get() * acc->get());
    systemEngine->putData(result);
    delete num;
    delete acc;
}

iPhoneDivideDoubleDouble::iPhoneDivideDoubleDouble():Instruction() {
    name = "Divide"; // No tr
    precedence = 10;
    displayString = "/";
    argCount = 2;
}
void iPhoneDivideDoubleDouble::eval() {
    DoubleData *acc = (DoubleData *)systemEngine->getData();
    DoubleData *num = (DoubleData *)systemEngine->getData();
    if (acc->get() == 0) {
        systemEngine->setError(eDivZero);
    } else {
        DoubleData *result = new DoubleData();
        result->set(num->get() / acc->get());
        systemEngine->putData(result);
    }
    delete num;
    delete acc;
}
