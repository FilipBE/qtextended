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
#ifndef STDINPUTWIDGETS_H
#define STDINPUTWIDGETS_H

#include <QLayout>
#include <QPushButton>

#include "../doubleinstruction.h"
#include "../engine.h"
#ifdef ENABLE_FRACTION
#include "../fractioninstruction.h"
#endif

class CalcUserInterface : public QWidget
{
    public:
        CalcUserInterface( QWidget* parent = 0, Qt::WFlags fl = 0)
        : QWidget(parent, fl) {};

        virtual QString interfaceName() = 0;
    protected:
        QGridLayout * InputWidgetLayout;
};

class InputWidget : public CalcUserInterface
{
    Q_OBJECT

public:
    InputWidget( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~InputWidget(){};

public slots:
    void val0Clicked ();
    void val1Clicked ();
    void val2Clicked ();
    void val3Clicked ();
    void val4Clicked ();
    void val5Clicked ();
    void val6Clicked ();
    void val7Clicked ();
    void val8Clicked ();
    void val9Clicked ();
    void evalClicked();
    void addClicked ();
    void subClicked ();
    void mulClicked ();
    void divClicked ();
    void negClicked ();
    void bsClicked ();

    QString interfaceName() { return QString(tr("Standard")); };


protected:
    virtual void init(int fromRow, int fromCol);

    QGridLayout* InputWidgetLayout;

    QPushButton* PB0,*PB1,*PB2,*PB3,*PB4,*PB5,*PB6,*PB7,*PB8,*PB9;
    QPushButton* PBPlus;
    QPushButton* PBMinus;
    QPushButton* PBTimes;
    QPushButton* PBDiv;
    QPushButton* PBEval;
    QPushButton* PBNegate;
    QPushButton* PBBS;
};

class DecimalInputWidget : public InputWidget
{
    Q_OBJECT

public:
    DecimalInputWidget( QWidget* parent = 0, Qt::WFlags fl = 0 );
    virtual ~DecimalInputWidget(){};

public slots:
    void decimalClicked ();
protected:
    virtual void init(int fromRow, int fromCol);
private:
    QPushButton* PBDecimal;
};


class FractionInputWidget : public InputWidget
{
    Q_OBJECT

public:
    FractionInputWidget( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~FractionInputWidget(){};

public slots:
    void fractionClicked ();
protected:
    virtual void init(int fromRow, int fromCol);
private:
    QPushButton* PBFraction;
};
#endif
