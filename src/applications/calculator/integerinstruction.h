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

#ifndef INTEGERINSTRUCTION_H
#define INTEGERINSTRUCTION_H

#include "instruction.h"
#include "integerdata.h"

// Integer instruction base
class BaseIntegerInstruction : public Instruction {
public:
    BaseIntegerInstruction():Instruction(){};
    ~BaseIntegerInstruction(){};

    Data *eval(Data*);
    Data *doEval(IntegerData *i){return i;};
protected:
    IntegerData *integerNum;
};

// Mathematical functions
class IntegerAdd : public BaseIntegerInstruction {
public:
    IntegerAdd():BaseIntegerInstruction(){};
    ~IntegerAdd(){};
    Data *doEval(IntegerData *);
};
class IntegerSub : public BaseIntegerInstruction {
public:
    IntegerSub():BaseIntegerInstruction(){};
    ~IntegerSub(){};
    Data *doEval(IntegerData *);
};
class IntegerMul : public BaseIntegerInstruction {
public:
    IntegerMul():BaseIntegerInstruction(){};
    ~IntegerMul(){};
    Data *doEval(IntegerData *);
};
class IntegerDiv : public BaseIntegerInstruction {
public:
    IntegerDiv():BaseIntegerInstruction(){};
    ~IntegerDiv(){};
    Data *doEval(IntegerData *);
};

#endif
