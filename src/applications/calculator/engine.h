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

#ifndef ENGINE_H
#define ENGINE_H

#include <qtopiaglobal.h>
#include <QStack>
#include <QList>
#include <QLabel>

#include "instruction.h"
#include "display.h"

class iBraceOpen;
enum State { sStart, sAppend, sError };
enum Error { eError, eOutOfRange, eDivZero, eNotSolvable, eNonPositive,
    eNonInteger,eNotANumber,eInf,eNoDataFactory,eNegInf, eSurpassLimits };
enum ResetState { drNone, drSoft, drHard };

class Engine:public QObject {
    Q_OBJECT
public:
    Engine();
    ~Engine();

    void registerInstruction(Instruction*);
    void pushInstruction(QString);
    void evaluate();

    Data *getData();
    void putData(Data *);

    void dualReset();
    void softReset();
    void hardReset();
    void push(char);
    void push(QString);
    void delChar();
    void memoryRecall();
    void memorySave();
    void memoryReset();

    void openBrace();
    void closeBrace();

    void setError(Error = eError, bool resetStack = true);
    void setError(QString, bool resetStack = true);
    void setDisplay(MyLcdDisplay *);
    void setAccType(QString);
    QString getDisplay();

    int numOps();
    bool error();

signals:
    void stackChanged();
    void dualResetStateChanged(ResetState);

protected:
    friend class iBraceOpen;
    friend class iBraceOpenImpl;
    friend class MyLcdDisplay;

    void incBraceCount();
    bool checkState();
    QString errorString;
    Instruction *resolve(QString);

private:
    void doEvalStack(int=0,bool=false);
    void evalStack(int=0,bool=false);
    void executeInstructionOnStack(QString);
    void executeInstructionOnStack(Instruction *);

    void changeState(State);
    void changeResetState(ResetState);

    int previousInstructionsPrecedence;


    State state;
    ResetState drs;
public:
    QStack<QString*> iStack;
    QStack<Data*> dStack;
private:
    QStack <Data*> tmpDStack;

    QString currentType,wantedType;
    int braceCount;

    QLabel *memMark;
    Data *mem;
    Data *recoveredDStack;
    Instruction *kDesc;

    QList<Instruction*> list;
    MyLcdDisplay *lcd;
};

extern Engine *systemEngine;
#endif
