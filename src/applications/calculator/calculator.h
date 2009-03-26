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

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QStackedWidget>

#ifndef QT_NO_CLIPBOARD
#include <QClipboard>
#endif

#include "engine.h"
#include "interfaces/stdinputwidgets.h"

class QComboBox;

class Calculator:public QWidget
{
    Q_OBJECT
public:
    Calculator ( QWidget * p = 0, Qt::WFlags fl=0);
    virtual ~Calculator ();

protected:
    void keyPressEvent(QKeyEvent *e);
    void showEvent(QShowEvent *);

private:
    QWidget *si;
#ifdef QTOPIA_UNPORTED
    QStackedWidget *pluginStackedWidget;
    QComboBox *modeBox;
    QString lastView;
#endif

    MyLcdDisplay *LCD;
#ifndef QT_NO_CLIPBOARD
    QClipboard *cb;
    QAction* a_paste;
    void cut();
private slots:
    void paste();
    void copy();
    void clipboardChanged();
#endif
private slots:
    void negate();
};

#endif
