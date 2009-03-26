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

#include <QApplication>
#include <QLinearGradient>
#include <QPainterPath>
#include <QRadialGradient>
#include <QLabel>
#include <QListWidget>
#include <QFont>
#include <QFontMetrics>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QDebug>
#include <QTimeLine>
#include <QPixmap>
#include <QPainter>
#include <QPaintEvent>
#include <QTextEdit>
#include <QStringList>
#include <qtopialog.h>

//#include <QtopiaInputMethod>
#include "predictivekeyboardimpl.h"
//#include "predictivekeyboardwidget.h"
#include "predictivekeyboard.h"
#include "keyboard.h"

/* XPM */
static const char * kb_xpm[] = {
"28 13 4 1",
"       c None",
".      c #4C4C4C",
"+      c #FFF7DD",
"@      c #D6CFBA",
" .......................... ",
" .+++.+++.+++.+++.+++.++++. ",
" .+@@.+@@.+@@.+@@.+@@.+@@@. ",
" .......................... ",
" .+++++.+++.+++.+++.++++++. ",
" .+@@@@.+@@.+@@.+@@.+@@@@@. ",
" .......................... ",
" .++++++.+++.+++.+++.+++++. ",
" .+@@@@@.+@@.+@@.+@@.+@@@@. ",
" .......................... ",
" .++++.++++++++++++++.++++. ",
" .+@@@.+@@@@@@@@@@@@@.+@@@. ",
" .......................... "};

PredictiveKeyboardInputMethod::PredictiveKeyboardInputMethod(QObject *parent)
    : QtopiaInputMethod(parent), input(0)
{
}

PredictiveKeyboardInputMethod::~PredictiveKeyboardInputMethod()
{
    if(input) {
        delete input;
        input = 0;
    }
}

QWidget *PredictiveKeyboardInputMethod::inputWidget( QWidget* parent)
{
    PredictiveKeyboard* tmp = qobject_cast<PredictiveKeyboard*>(inputModifier());
    if (tmp) 
        return tmp->widget( parent );
    else
        return 0;
}


void PredictiveKeyboardInputMethod::reset()
{
    if ( input )
        input->resetState();
}

QIcon PredictiveKeyboardInputMethod::icon() const
{
    QIcon i;
    i.addPixmap(QPixmap((const char **)kb_xpm));
    return i;
}

QString PredictiveKeyboardInputMethod::name() const
{
    return qApp->translate( "InputMethods", "PredictiveKeyboard" );
//    return qApp->translate( "InputMethods", "Opti" );
}

QString PredictiveKeyboardInputMethod::identifier() const
{
    return "SmartKeyboard";
}

QString PredictiveKeyboardInputMethod::version() const
{
    return "1.0.0";
}

QtopiaInputMethod::State PredictiveKeyboardInputMethod::state() const
{
    if (input && input->widget() && input->widget()->isVisible())
        return Ready;
    else
        return Sleeping;
}


void PredictiveKeyboardInputMethod::menuActionActivated(int v)
{
    Q_UNUSED(v);
}

QWSInputMethod* PredictiveKeyboardInputMethod::inputModifier(){
    if ( !input ) {
        input = new PredictiveKeyboard();
        //connect(input->widget(), SIGNAL(showing()), this, SIGNAL(stateChanged()));
        //connect(input->widget(), SIGNAL(hiding()), this, SIGNAL(stateChanged()));

#ifdef XXX
        PredictiveKeyboardWidget* pw = qobject_cast<PredictiveKeyboardWidget*>(input->widget());
        if(pw) {
            pw->setPlugin(this);
        }
#endif

        //mainWindow->setLayout(layout);
        //   QHBoxLayout *hlayout = new QHBoxLayout();
        //layout->addLayout(hlayout);
        //    hlayout->setGeometry(QRect(0, 0, 240, 40));

    }
    return input;
}

QList<QIMActionDescription*> PredictiveKeyboardInputMethod::menuDescription()
{
    return QList<QIMActionDescription*>();
}

void PredictiveKeyboardInputMethod::setHint(QString const& hint, bool)
{
    input->mKeyboard->setHint(hint);
}

QTOPIA_EXPORT_PLUGIN(PredictiveKeyboardInputMethod);
