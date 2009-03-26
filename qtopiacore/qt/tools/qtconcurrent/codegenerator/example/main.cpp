/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/
#include <QDebug>
#include "codegenerator.h"
using namespace CodeGenerator;

int main()
{
    // The code generator works on items. Each item has a generate() function:
    Item item("");
    qDebug() << item.generate(); // produces "".
    
    // There are several Item subclasses. Text items contains a text string which they
    // reproduce when generate is called:
    Text text(" Hi there");
    qDebug() << text.generate(); // produces " Hi there".
    
    // Items can be concatenated:
    Item sentence = text + Text(" Bye there") ;
    qDebug() << sentence.generate(); // produces "Hi there Bye there".
    // (Internally, this creates a tree of items, and generate is called recursively 
    // for items that have children.)

    // Repeater items repeat their content when generate is called:
    Repeater repeater = text;
    repeater.setRepeatCount(3);
    qDebug() << repeater.generate(); // produces "Hi there Hi there Hi there".
    
    // Counters evaluate to the current repeat index.
    Repeater repeater2 = text + Counter();
    repeater2.setRepeatCount(3);
    qDebug() << repeater2.generate(); // produces "Hi there0 Hi there1 Hi there2".

    // Groups provide sub-groups which are repeated according to the current repeat index.
    // Counters inside Groups evaluate to the local repeat index for the Group.
    Group arguments("Arg" + Counter()  + " arg" + Counter());
    Repeater function("void foo(" + arguments + ");\n");
    function.setRepeatCount(3);
    qDebug() << function.generate();

    // Produces:
    // void foo(Arg1 arg1);
    // void foo(Arg1 arg1, Arg2 arg2);
    // void foo(Arg1 arg1, Arg2 arg2, Arg3 arg3);
}
