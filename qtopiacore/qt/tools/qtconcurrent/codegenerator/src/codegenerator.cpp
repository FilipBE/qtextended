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
#include "codegenerator.h"
#include <qdebug.h>
namespace CodeGenerator
{

//Convenience constructor so you can say Item("foo")
Item::Item(const char * const text) : generator(Text(QByteArray(text)).generator) {}

int BaseGenerator::currentCount(GeneratorStack * const stack) const
{
    const int stackSize = stack->count();
    for (int i = stackSize - 1; i >= 0; --i) {
        BaseGenerator const * const generator = stack->at(i);
        switch (generator->type) {
            case RepeaterType: {
                RepeaterGenerator const * const repeater = static_cast<RepeaterGenerator const * const>(generator);
                return repeater->currentRepeat;
            } break;
            case GroupType: {
                GroupGenerator const * const group = static_cast<GroupGenerator const * const>(generator);
                return group->currentRepeat;
            } break;
            default:
            break;
        }
    }
    return -1;
}

int BaseGenerator::repeatCount(GeneratorStack * const stack) const
{
    const int stackSize = stack->count();
    for (int i = stackSize - 1; i >= 0; --i) {
        BaseGenerator const * const generator = stack->at(i);
        switch (generator->type) {
            case RepeaterType: {
                RepeaterGenerator const * const repeater = static_cast<RepeaterGenerator const * const>(generator);
                return repeater->currentRepeat;
            } break;
/*            case GroupType: {
                GroupGenerator const * const group = static_cast<GroupGenerator const * const>(generator);
                return group->currentRepeat;
            } break;
*/  
            default:
            break;
        }
    }
    return -1;
}

QByteArray RepeaterGenerator::generate(GeneratorStack * const stack)
{ 
    GeneratorStacker stacker(stack, this);
    QByteArray generated;
    for (int i = repeatOffset; i < repeatCount + repeatOffset; ++i) {
        currentRepeat = i;
        generated += childGenerator->generate(stack);
    }
    return generated;
};

QByteArray GroupGenerator::generate(GeneratorStack * const stack)
{ 
    const int repeatCount = currentCount(stack);
    GeneratorStacker stacker(stack, this);
    QByteArray generated;

    if (repeatCount > 0)
        generated += prefix->generate(stack);

    for (int i = 1; i <= repeatCount; ++i) {
        currentRepeat = i;
        generated += childGenerator->generate(stack);
        if (i != repeatCount)
            generated += separator->generate(stack);
    }

    if (repeatCount > 0)
        generated += postfix->generate(stack);

    return generated;
};

const Compound operator+(const Item &a, const Item &b)
{
    return Compound(a, b);
}

const Compound operator+(const Item &a, const char * const text)
{
    return Compound(a, Text(text));
}

const Compound operator+(const char * const text, const Item &b)
{
    return Compound(Text(text), b);    
}

}