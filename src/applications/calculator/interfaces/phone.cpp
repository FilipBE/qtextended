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

#include "../engine.h"
#include "../phoneinstruction.h"
#include "ui_helperpanel.h"
#include "phone.h"


#include <qsoftmenubar.h>
#include <qtopiaapplication.h>

#include <QSignalMapper>
#include <QShortcut>
#include <QKeyEvent>

static const int KEY_HOLD_TIME = 300;

FormPhone::FormPhone(QWidget *parent)
    : CalcUserInterface(parent), negate_action(0)
{
    setupUi(this);
    if ( layoutDirection() == Qt::LeftToRight ) {
        helper_dec->setText(tr("*\ndecimal"));
        helper_cycle->setText(tr("#\n+ - x /"));
        connect(helper_times,SIGNAL(clicked()),this,SLOT(times()));
        connect(helper_div,SIGNAL(clicked()),this,SLOT(div()));
    } else {
        //the layout swaps the widgets around. However our keymapping remains the same.
        //therefore we have to swap the labels as well.
        helper_cycle->setText(tr("*\ndecimal"));
        helper_dec->setText(tr("#\n+ - x /"));
        QString tmp = helper_times->text();
        helper_times->setText( helper_div->text() );
        helper_div->setText( tmp );
        connect(helper_times,SIGNAL(clicked()),this,SLOT(div()));
        connect(helper_div,SIGNAL(clicked()),this,SLOT(times()));
    }
    connect(helper_dec,SIGNAL(clicked()),this,SLOT(dec()));
    connect(helper_plus,SIGNAL(clicked()),this,SLOT(plus()));
    connect(helper_minus,SIGNAL(clicked()),this,SLOT(minus()));
    connect(helper_eval,SIGNAL(clicked()),this,SLOT(eval()));
    connect(helper_cycle,SIGNAL(clicked()),this,SLOT(nextInstruction()));

    lastInstruction = 0;

    displayedState = drNone;
    connect(systemEngine,SIGNAL(dualResetStateChanged(ResetState)),
        this,SLOT(changeResetButtonText(ResetState)));

    lockEvaluation = firstNumber = true;
    backpressed = false;
    setFocusPolicy(Qt::NoFocus);
    helper_dec->setFocusPolicy(Qt::NoFocus);
    helper_plus->setFocusPolicy(Qt::NoFocus);
    helper_minus->setFocusPolicy(Qt::NoFocus);
    helper_times->setFocusPolicy(Qt::NoFocus);
    helper_div->setFocusPolicy(Qt::NoFocus);
    helper_eval->setFocusPolicy(Qt::NoFocus);
    helper_cycle->setFocusPolicy(Qt::NoFocus);

    QtopiaApplication::setInputMethodHint( this, QtopiaApplication::Number );
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);

    // DPad shortcuts
    QVector<int> keyIdents(13);
    keyIdents.append(Qt::Key_Up);
    keyIdents.append(Qt::Key_Plus);
    keyIdents.append(Qt::Key_Down);
    keyIdents.append(Qt::Key_Minus);
    keyIdents.append(Qt::Key_Right);
    keyIdents.append(Qt::Key_Left);
    keyIdents.append(Qt::Key_Slash);
    keyIdents.append(Qt::Key_Select);
    keyIdents.append(Qt::Key_NumberSign);
    keyIdents.append(Qt::Key_Asterisk);
    keyIdents.append(Qt::Key_Enter);
    keyIdents.append(Qt::Key_Return);
    keyIdents.append(Qt::Key_Equal);

    signalMapper = new QSignalMapper(this);
    QShortcut * shortcut = 0;
    foreach (int ident, keyIdents) {
        shortcut = new QShortcut(QKeySequence(ident), this);
        connect(shortcut, SIGNAL(activated()), signalMapper, SLOT(map()));
        signalMapper->setMapping(shortcut, ident);
    }

    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(shortcutClicked(int)));

    Instruction *da = new iPhoneDoubleFactory();
    systemEngine->registerInstruction(da);
    da = new iEvaluateLine();
    systemEngine->registerInstruction(da);
    da = new iPhoneDoubleCopy();
    systemEngine->registerInstruction(da);
    da = new iPhoneAddDoubleDouble();
    systemEngine->registerInstruction(da);
    da = new iPhoneMultiplyDoubleDouble();
    systemEngine->registerInstruction(da);
    da = new iPhoneSubtractDoubleDouble();
    systemEngine->registerInstruction(da);
    da = new iPhoneDivideDoubleDouble();
    systemEngine->registerInstruction(da);

}

void FormPhone::shortcutClicked(int keyIdent)
{
    switch(keyIdent) {
        case Qt::Key_Up :
        case Qt::Key_Plus:
            plus();
            break;
        case Qt::Key_Down:
        case Qt::Key_Minus:
            minus();
            break;
        case Qt::Key_Right:
            times();
            break;
        case Qt::Key_Left:
        case Qt::Key_Slash:
            div();
            break;
        case Qt::Key_Select:
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Equal:
            eval(); break;
        case Qt::Key_NumberSign:
            nextInstruction(); break;
        case Qt::Key_Asterisk:
            dec();
            break;
    }
}

void FormPhone::showEvent ( QShowEvent *e ) {
    systemEngine->setAccType("Double"); // No tr

    QWidget::showEvent(e);
}

void FormPhone::plus(){
    systemEngine->pushInstruction("Add");
    if (negate_action) negate_action->setVisible(false);
    lastInstruction=1;
    firstNumber = false;
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
    if ( systemEngine->error() )
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
    lockEvaluation = true;
}
void FormPhone::minus(){
    systemEngine->pushInstruction("Subtract");
    if (negate_action) negate_action->setVisible(false);
    lastInstruction=2;
    firstNumber = false;
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
    if ( systemEngine->error() )
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
    lockEvaluation = true;
}
void FormPhone::times(){
    systemEngine->pushInstruction("Multiply");
    if (negate_action) negate_action->setVisible(false);
    lastInstruction=3;
    firstNumber = false;
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
    if ( systemEngine->error() )
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
    lockEvaluation = true;
}
void FormPhone::div(){
    systemEngine->pushInstruction("Divide");
    if (negate_action) negate_action->setVisible(false);
    lastInstruction=0;
    firstNumber = false;
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
    if ( systemEngine->error() )
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
    lockEvaluation = true;
}
void FormPhone::eval(){
    if (!lockEvaluation) {
        systemEngine->evaluate();
        if (negate_action) negate_action->setVisible(false);
        firstNumber = true;
        QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
        if ( systemEngine->error() )
            QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
        lockEvaluation = true;
    }
}

void FormPhone::dec() {
    systemEngine->push('.');
}
void FormPhone::nextInstruction(){
    switch (lastInstruction){
        case 0:
            plus();
            break;
        case 1:
            minus();
            break;
        case 2:
            times();
            break;
        case 3:
            div();
            break;
    }
}

void FormPhone::changeResetButtonText ( ResetState drs ) {
    displayedState = drs;
    if (drs == drNone || drs == drHard) {
        firstNumber = true;
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);
    } else {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
    }
    if (negate_action) negate_action->setVisible(true);
}

void FormPhone::keyReleaseEvent(QKeyEvent *e){
    if ((e->key() == Qt::Key_Back || e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace )
            && !e->isAutoRepeat()
            && backpressed) {
        backpressed = false;
        if ( tid_hold ) {
            killTimer(tid_hold);
            tid_hold = 0;
            systemEngine->delChar();
            int numDataOps = systemEngine->numOps();
            if ((numDataOps % 2 == 1)) {
                QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
                lockEvaluation = true;
                if (numDataOps == 1)
                    firstNumber = true;
            } else {
                QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::Ok);
                lockEvaluation = false;
            }
        } else {
            if (e->key() == Qt::Key_Back)
            close();
        }
    }
    switch (e->key())
    {
        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
        case Qt::Key_Asterisk:
            if (systemEngine->numOps()%2 == 0)
                firstNumber = false;
            if (!firstNumber) {
                QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::Ok);
                lockEvaluation = false;
            }
    }

    e->accept();

}

void FormPhone::keyPressEvent(QKeyEvent *e) {
    if (e->key() != Qt::Key_NumberSign){
        lastInstruction = 0;
    }
    switch(e->key()) {
        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
            {
                QChar qc = e->text().at(0);
                if ( qc.isPrint() && !qc.isSpace() )
                    systemEngine->push(qc.toLatin1());
            }
            break;
        case Qt::Key_Back:
        case Qt::Key_No:
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            if ( displayedState == drNone || displayedState == drHard ) {
                if ( systemEngine->error() )
                    clearAll();
                else
                    if (e->key() == Qt::Key_Back || e->key() == Qt::Key_No)
                        close();
            } else if (!e->isAutoRepeat()){
                tid_hold = startTimer(KEY_HOLD_TIME);
                backpressed = true;
            }
            break;
        default:
            e->ignore();
            break;
    }
}

void FormPhone::clearAll(){
    if (displayedState == drSoft) {
        systemEngine->softReset();
        systemEngine->hardReset();
    }
    else
        systemEngine->hardReset();
    if( !Qtopia::mousePreferred() )
        setEditFocus(true);

    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
    lockEvaluation = true;
}

void FormPhone::timerEvent(QTimerEvent *e){
   if (e->timerId() == tid_hold) {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);
        killTimer(tid_hold);
        tid_hold = 0;
   }
}

void FormPhone::negateAction(QAction* action)
{
    negate_action = action;
}
