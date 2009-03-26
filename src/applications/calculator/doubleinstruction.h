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

#ifndef DOUBLEINSTRUCTION_H
#define DOUBLEINSTRUCTION_H

#include "instruction.h"
#include "doubledata.h"

// Double instruction base
class BaseDoubleInstruction:public Instruction {
public:
    BaseDoubleInstruction();
    ~BaseDoubleInstruction();

    virtual void eval();
    virtual void doEval(DoubleData *,DoubleData *);
    virtual void doEvalI(DoubleData *);
};

// Factory
class iDoubleFactory:public Instruction {
public:
    iDoubleFactory();
    ~iDoubleFactory(){};
    void eval();
};

// Copy
class iDoubleCopy:public Instruction {
public:
    iDoubleCopy();
    ~iDoubleCopy(){};
    void eval();
};

#ifdef ENABLE_INTEGER
class iConvertIntDouble:public Instruction {
public:
    iConvertIntDouble();
    ~iConvertIntDouble(){};
    Data *eval(Data *);
};
#endif
#ifdef ENABLE_FRACTION
class iConvertFractionDouble:public Instruction {
public:
    iConvertFractionDouble();
    ~iConvertFractionDouble(){};
    void eval();
};
#endif

// Mathematical functions
class iAddDoubleDouble:public BaseDoubleInstruction {
public:
    iAddDoubleDouble();
    ~iAddDoubleDouble(){};
    void doEval(DoubleData *,DoubleData *);
};

class iSubtractDoubleDouble:public BaseDoubleInstruction {
public:
    iSubtractDoubleDouble();
    ~iSubtractDoubleDouble(){};
    void doEval(DoubleData *,DoubleData *);
};

class iMultiplyDoubleDouble:public BaseDoubleInstruction {
public:
    iMultiplyDoubleDouble();
    ~iMultiplyDoubleDouble(){};
    void doEval(DoubleData *,DoubleData *);
};

class iDivideDoubleDouble:public BaseDoubleInstruction {
public:
    iDivideDoubleDouble();
    ~iDivideDoubleDouble(){};
    void doEval(DoubleData *,DoubleData *);
};

#ifdef ENABLE_SCIENCE
// Normal functions
class iDoublePow:public BaseDoubleInstruction {
public:
    iDoublePow();
    ~iDoublePow(){};
    void doEval(DoubleData *,DoubleData *);
};

class iDoubleXRootY:public BaseDoubleInstruction {
public:
    iDoubleXRootY();
    ~iDoubleXRootY(){};
    void doEval(DoubleData *,DoubleData *);
};

// Immediate
class iDoubleSinRad:public BaseDoubleInstruction {
public:
    iDoubleSinRad();
    ~iDoubleSinRad(){};
    void doEvalI(DoubleData *);
};

class iDoubleSinDeg:public BaseDoubleInstruction {
public:
    iDoubleSinDeg();
    ~iDoubleSinDeg(){};
    void doEvalI(DoubleData *);
};
class iDoubleSinGra:public BaseDoubleInstruction {
public:
    iDoubleSinGra();
    ~iDoubleSinGra(){};
    void doEvalI(DoubleData *);
};

class iDoubleCosRad:public BaseDoubleInstruction {
public:
    iDoubleCosRad();
    ~iDoubleCosRad(){};
    void doEvalI(DoubleData *);
};
class iDoubleCosDeg:public BaseDoubleInstruction {
public:
    iDoubleCosDeg();
    ~iDoubleCosDeg(){};
    void doEvalI(DoubleData *);
};
class iDoubleCosGra:public BaseDoubleInstruction {
public:
    iDoubleCosGra();
    ~iDoubleCosGra(){};
    void doEvalI(DoubleData *);
};

class iDoubleTanRad:public BaseDoubleInstruction {
public:
    iDoubleTanRad();
    ~iDoubleTanRad(){};
    void doEvalI(DoubleData *);
};
class iDoubleTanDeg:public BaseDoubleInstruction {
public:
    iDoubleTanDeg();
    ~iDoubleTanDeg(){};
    void doEvalI(DoubleData *);
};
class iDoubleTanGra:public BaseDoubleInstruction {
public:
    iDoubleTanGra();
    ~iDoubleTanGra(){};
    void doEvalI(DoubleData *);
};

class iDoubleASinRad:public BaseDoubleInstruction {
public:
    iDoubleASinRad();
    ~iDoubleASinRad(){};
    void doEvalI(DoubleData *);
};
class iDoubleASinDeg:public BaseDoubleInstruction {
public:
    iDoubleASinDeg();
    ~iDoubleASinDeg(){};
    void doEvalI(DoubleData *);
};
class iDoubleASinGra:public BaseDoubleInstruction {
public:
    iDoubleASinGra();
    ~iDoubleASinGra(){};
    void doEvalI(DoubleData *);
};

class iDoubleACosRad:public BaseDoubleInstruction {
public:
    iDoubleACosRad();
    ~iDoubleACosRad(){};
    void doEvalI(DoubleData *);
};
class iDoubleACosDeg:public BaseDoubleInstruction {
public:
    iDoubleACosDeg();
    ~iDoubleACosDeg(){};
    void doEvalI(DoubleData *);
};
class iDoubleACosGra:public BaseDoubleInstruction {
public:
    iDoubleACosGra();
    ~iDoubleACosGra(){};
    void doEvalI(DoubleData *);
};

class iDoubleATanRad:public BaseDoubleInstruction {
public:
    iDoubleATanRad();
    ~iDoubleATanRad(){};
    void doEvalI(DoubleData *);
};
class iDoubleATanDeg:public BaseDoubleInstruction {
public:
    iDoubleATanDeg();
    ~iDoubleATanDeg(){};
    void doEvalI(DoubleData *);
};
class iDoubleATanGra:public BaseDoubleInstruction {
public:
    iDoubleATanGra();
    ~iDoubleATanGra(){};
    void doEvalI(DoubleData *);
};

class iDoubleLog:public BaseDoubleInstruction {
public:
    iDoubleLog();
    ~iDoubleLog(){};
    void doEvalI(DoubleData *);
};

class iDoubleLn:public BaseDoubleInstruction {
public:
    iDoubleLn();
    ~iDoubleLn(){};
    void doEvalI(DoubleData *);
};

class iDoubleExp:public BaseDoubleInstruction {
public:
    iDoubleExp();
    ~iDoubleExp(){};
    void doEvalI(DoubleData *);
};

class iDoubleOneOverX:public BaseDoubleInstruction {
public:
    iDoubleOneOverX();
    ~iDoubleOneOverX(){};
    void doEvalI(DoubleData *);
};

class iDoubleFactorial:public BaseDoubleInstruction {
public:
    iDoubleFactorial();
    ~iDoubleFactorial(){};
    void doEvalI(DoubleData *);
};

class iDoubleSquareRoot:public BaseDoubleInstruction {
public:
    iDoubleSquareRoot();
    ~iDoubleSquareRoot(){};
    void doEvalI(DoubleData *);
};

class iDoubleCubeRoot:public BaseDoubleInstruction {
public:
    iDoubleCubeRoot();
    ~iDoubleCubeRoot(){};
    void doEvalI(DoubleData *);
};

class iDoubleSquare:public BaseDoubleInstruction {
public:
    iDoubleSquare();
    ~iDoubleSquare(){};
    void doEvalI(DoubleData *);
};
#endif //ENABLE_SCIENCE

class iDoubleNegate:public BaseDoubleInstruction {
public:
    iDoubleNegate();
    ~iDoubleNegate(){};
    void doEvalI(DoubleData *);
};

#endif
