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

#ifndef PHONE_H
#define PHONE_H

#include "stdinputwidgets.h"

#include <QLabel>
#include <QLayout>

#include <qtopiaglobal.h>
#include <qsoftmenubar.h>
#include <QAction>

#include "../engine.h"
#include "../doubledata.h"

#include "ui_helperpanel.h"

class QTimer;
class QSignalMapper;

class FormPhone:public CalcUserInterface, Ui::HelperPanel {
Q_OBJECT
public:
    FormPhone(QWidget *parent = 0);
    ~FormPhone(){};

    QString interfaceName() { return QString(tr("Phone")); };

public slots:
    void showEvent ( QShowEvent * );

    void plus();
    void minus();
    void times();
    void div();
    void eval();
    void dec();
    void nextInstruction();
    void clearAll();

    void changeResetButtonText(ResetState);

    void negateAction(QAction* action);

signals:
    void close();

protected:
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void timerEvent(QTimerEvent*);
private slots:
    void shortcutClicked(int keyIdent);
private:
    int tid_hold;
    bool waitForRelease;
    ResetState displayedState;

    QVBoxLayout* Type1BaseInputWidgetLayout;
    int lastInstruction;
    bool firstNumber;
    bool backpressed;
    bool lockEvaluation;

    QSignalMapper *signalMapper;
    QAction* negate_action;
};

#endif
