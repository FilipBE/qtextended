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
#include "integerinstruction.h"

// Automatic type casting
Data *BaseIntegerInstruction::eval(Data *i) {
        integerNum = (IntegerData *)num;
        Data *ret = doEval((IntegerData *)i);
        return ret;
}

// Mathematical functions
Data * IntegerAdd::doEval(IntegerData *i) {
        i->set( integerNum->get() + i->get() );
        return i;
}
Data * IntegerSub::doEval(IntegerData *i) {
        i->set( i->get() - integerNum->get() );
        return i;
}
Data * IntegerMul::doEval(IntegerData *i) {
        i->set( i->get() * integerNum->get() );
        return i;
}
Data * IntegerDiv::doEval(IntegerData *i) {
        if (!integerNum->get()) {
                systemEngine->setError(eDivZero);
        } else
                integerNum->set( i->get() / integerNum->get() );
        return i;
}

