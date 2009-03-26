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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#ifndef TESTTEXTEDIT_H
#define TESTTEXTEDIT_H

#include "testwidget.h"

class QTextEdit;

class TestTextEdit : public TestWidget,
    public QtUiTest::TextWidget
{
    Q_OBJECT
    Q_INTERFACES(QtUiTest::TextWidget)


public:
    TestTextEdit(QObject*);

    virtual QString text() const;
    virtual QString selectedText() const;

    virtual bool canEnter(QVariant const&) const;
    virtual bool enter(QVariant const&,bool);

    static bool canWrap(QObject*);

    int cursorPosition() const;

signals:
    void cursorPositionChanged(int,int);
    void textChanged();
    void entered(QVariant const&);

private slots:
    void onTextChanged();
    void onCursorPositionChanged();

private:
    QString lastEntered;
    int     lastCursorPosition;
    bool    committed;
    QTextEdit *q;
};

#endif

