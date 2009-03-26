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
#include "stdinputwidgets.h"
#include <Qtopia>

extern Engine *systemEngine;

InputWidget::InputWidget( QWidget* parent, Qt::WFlags fl  )
: CalcUserInterface( parent, fl)
{
    setWhatsThis( tr("Click to enter digit or operator") );
    QSizePolicy mySizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    setSizePolicy(mySizePolicy);
}

void InputWidget::evalClicked() {
        systemEngine->evaluate();
}

void InputWidget::val1Clicked ()
{
        systemEngine->push('1');
}

void InputWidget::val2Clicked ()
{
        systemEngine->push('2');
}
void InputWidget::val3Clicked ()
{
        systemEngine->push('3');
}

void InputWidget::val4Clicked ()
{
        systemEngine->push('4');
}

void InputWidget::val5Clicked ()
{
        systemEngine->push('5');
}

void InputWidget::val6Clicked ()
{
        systemEngine->push('6');
}

void InputWidget::val7Clicked ()
{
        systemEngine->push('7');
}

void InputWidget::val8Clicked ()
{
        systemEngine->push('8');
}

void InputWidget::val9Clicked ()
{
        systemEngine->push('9');
}

void InputWidget::val0Clicked ()
{
        systemEngine->push('0');
}

void InputWidget::addClicked () {
        systemEngine->pushInstruction("Add"); // No tr
}

void InputWidget::subClicked () {
        systemEngine->pushInstruction("Subtract"); // No tr
}

void InputWidget::mulClicked () {
        systemEngine->pushInstruction("Multiply"); // No tr
}

void InputWidget::divClicked () {
        systemEngine->pushInstruction("Divide"); // No tr
}

void InputWidget::negClicked () {
        systemEngine->pushInstruction("Negate"); // No tr
}

void InputWidget::bsClicked () {
        systemEngine->delChar();
}

void InputWidget::init( int fromRow, int fromCol )
{
    // First column
    PBBS = new QPushButton;
    PBBS->setSizePolicy(sizePolicy());
    /* This is a directional marker - not to be translated */
    if (QApplication::layoutDirection() == Qt::LeftToRight)
        PBBS->setText("<-");
    else
        PBBS->setText("-<");
    InputWidgetLayout->addWidget( PBBS, fromRow, fromCol );

    PB7 = new QPushButton;
    PB7->setSizePolicy(sizePolicy());
    PB7->setText( tr( "7" ) );
    InputWidgetLayout->addWidget( PB7, fromRow+1, fromCol );

    PB4 = new QPushButton( this );
    PB4->setSizePolicy(sizePolicy());
    PB4->setText( tr( "4" ) );
    InputWidgetLayout->addWidget( PB4, fromRow+2, fromCol );

    PB1 = new QPushButton( this );
    PB1->setSizePolicy(sizePolicy());
    PB1->setText( tr( "1" ) );
    InputWidgetLayout->addWidget( PB1, fromRow+3, fromCol );

    PB0 = new QPushButton( this );
    PB0->setSizePolicy(sizePolicy());
    PB0->setText( tr( "0" ) );
    InputWidgetLayout->addWidget( PB0, fromRow+4, fromCol );

    // Second column
    PBDiv = new QPushButton( this );
    PBDiv->setSizePolicy(sizePolicy());
    PBDiv->setText(QString(QChar(0xF7)));
    InputWidgetLayout->addWidget( PBDiv, fromRow, fromCol+1 );

    PB8 = new QPushButton( this );
    PB8->setSizePolicy(sizePolicy());
    PB8->setText( tr( "8" ) );
    InputWidgetLayout->addWidget( PB8, fromRow+1, fromCol+1 );

    PB5 = new QPushButton( this );
    PB5->setSizePolicy(sizePolicy());
    PB5->setText( tr( "5" ) );
    InputWidgetLayout->addWidget( PB5, fromRow+2, fromCol+1 );

    PB2 = new QPushButton( this );
    PB2->setSizePolicy(sizePolicy());
    PB2->setText( tr( "2" ) );
    InputWidgetLayout->addWidget( PB2, fromRow+3, fromCol+1 );

    // Thrid column
    PBTimes = new QPushButton( this );
    PBTimes->setSizePolicy(sizePolicy());
    PBTimes->setText( tr( "x", "times" ) );
    InputWidgetLayout->addWidget( PBTimes, fromRow, fromCol+2 );

    PB9 = new QPushButton( this );
    PB9->setSizePolicy(sizePolicy());
    PB9->setText( tr( "9" ) );
    InputWidgetLayout->addWidget( PB9, fromRow+1, fromCol+2 );

    PB6 = new QPushButton( this );
    PB6->setSizePolicy(sizePolicy());
    PB6->setText( tr( "6" ) );
    InputWidgetLayout->addWidget( PB6, fromRow+2, fromCol+2 );

    PB3 = new QPushButton( this );
    PB3->setSizePolicy(sizePolicy());
    PB3->setText( tr( "3" ) );
    InputWidgetLayout->addWidget( PB3, fromRow+3, fromCol+2 );

    PBNegate = new QPushButton( this );
    PBNegate->setSizePolicy(sizePolicy());
    PBNegate->setText( tr("+/-") );
    InputWidgetLayout->addWidget( PBNegate, fromRow+4, fromCol+2 );

    // Forth column
    PBMinus = new QPushButton( this );
    PBMinus->setSizePolicy(sizePolicy());
    PBMinus->setText( tr( "-" ) );
    InputWidgetLayout->addWidget( PBMinus, fromRow, fromCol+3 );

    PBPlus = new QPushButton( this );
    PBPlus->setSizePolicy(sizePolicy());
    PBPlus->setText( tr( "+" ) );
    InputWidgetLayout->addWidget( PBPlus, fromRow+1, fromCol+3, 2, 1 );

    PBEval = new QPushButton( this );
    PBEval->setSizePolicy(sizePolicy());
    PBEval->setText( tr( "=" ) );
    InputWidgetLayout->addWidget( PBEval, fromRow+3, fromCol+3, 2, 1 );

    if (Qtopia::mousePreferred()) {
        PB0->setFocusPolicy(Qt::NoFocus);
        PB1->setFocusPolicy(Qt::NoFocus);
        PB2->setFocusPolicy(Qt::NoFocus);
        PB3->setFocusPolicy(Qt::NoFocus);
        PB4->setFocusPolicy(Qt::NoFocus);
        PB5->setFocusPolicy(Qt::NoFocus);
        PB6->setFocusPolicy(Qt::NoFocus);
        PB7->setFocusPolicy(Qt::NoFocus);
        PB8->setFocusPolicy(Qt::NoFocus);
        PB9->setFocusPolicy(Qt::NoFocus);
        PBEval->setFocusPolicy(Qt::NoFocus);
        PBDiv->setFocusPolicy(Qt::NoFocus);
        PBPlus->setFocusPolicy(Qt::NoFocus);
        PBMinus->setFocusPolicy(Qt::NoFocus);
        PBTimes->setFocusPolicy(Qt::NoFocus);
        PBNegate->setFocusPolicy(Qt::NoFocus);
        PBBS->setFocusPolicy(Qt::NoFocus);
    }

    // Connect
    connect (PB0, SIGNAL(clicked()), this, SLOT(val0Clicked()));
    connect (PB1, SIGNAL(clicked()), this, SLOT(val1Clicked()));
    connect (PB2, SIGNAL(clicked()), this, SLOT(val2Clicked()));
    connect (PB3, SIGNAL(clicked()), this, SLOT(val3Clicked()));
    connect (PB4, SIGNAL(clicked()), this, SLOT(val4Clicked()));
    connect (PB5, SIGNAL(clicked()), this, SLOT(val5Clicked()));
    connect (PB6, SIGNAL(clicked()), this, SLOT(val6Clicked()));
    connect (PB7, SIGNAL(clicked()), this, SLOT(val7Clicked()));
    connect (PB8, SIGNAL(clicked()), this, SLOT(val8Clicked()));
    connect (PB9, SIGNAL(clicked()), this, SLOT(val9Clicked()));
    connect (PBEval, SIGNAL(clicked()), this, SLOT(evalClicked()));
    connect (PBPlus, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect (PBMinus, SIGNAL(clicked()), this, SLOT(subClicked()));
    connect (PBTimes, SIGNAL(clicked()), this, SLOT(mulClicked()));
    connect (PBDiv, SIGNAL(clicked()), this, SLOT(divClicked()));
    connect (PBNegate, SIGNAL(clicked()), this, SLOT(negClicked()));
    connect (PBBS, SIGNAL(clicked()), this, SLOT(bsClicked()));

}

// DECIMAL input widget

DecimalInputWidget::DecimalInputWidget( QWidget* parent, Qt::WFlags fl )
: InputWidget( parent, fl )
{

}

void DecimalInputWidget::init(int fromRow, int fromCol) {
    InputWidget::init(fromRow, fromCol);

    PBDecimal = new QPushButton( this );
    PBDecimal->setSizePolicy(sizePolicy());
    PBDecimal->setText( tr( ".", "decimal point" ) );
    if (Qtopia::mousePreferred())
        PBDecimal->setFocusPolicy(Qt::NoFocus);
    InputWidgetLayout->addWidget( PBDecimal, fromRow + 4, fromCol + 1);
    connect (PBDecimal, SIGNAL(clicked()), this, SLOT(decimalClicked()));
}

void DecimalInputWidget::decimalClicked () {
    systemEngine->push('.');
}

// FRACTION input widget
FractionInputWidget::FractionInputWidget( QWidget* parent, Qt::WFlags fl )
: InputWidget( parent, fl )
{
}

void FractionInputWidget::fractionClicked () {
    systemEngine->push('/');
}

void FractionInputWidget::init(int fromRow, int fromCol) {
    InputWidget::init(fromRow, fromCol);
    PBFraction = new QPushButton( this );
    PBFraction->setSizePolicy(sizePolicy());
    PBFraction->setFocusPolicy(Qt::TabFocus);
    PBFraction->setText( tr( "/" , "separator numerator - denominator") );
    InputWidgetLayout->addWidget( PBFraction, fromRow+4, fromCol+1);
    connect (PBFraction, SIGNAL(clicked()), this, SLOT(fractionClicked()));
}
