/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtScript module of the Qt Toolkit.
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

#include <QtCore/QTextStream>

#ifndef QT_NO_SCRIPT

#include "qscriptasm_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptengine_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

QT_BEGIN_NAMESPACE

const char *QScriptInstruction::opcode[] = {
#define STR(a) #a
#define Q_SCRIPT_DEFINE_OPERATOR(op) STR(i##op) ,
#include "instruction.table"
#undef Q_SCRIPT_DEFINE_OPERATOR
#undef STR
};

void QScriptInstruction::print(QTextStream &out) const
{
    out << opcode[op];

    if (! operand[0].isValid())
        return;

    out << '(' << operand[0].toString();

    if (operand[1].isValid())
        out << ", " << operand[1].toString();

    out << ')';
}

namespace QScript {

Code::Code():
    optimized(false),
    firstInstruction(0),
    lastInstruction(0)
{
}

Code::~Code()
{
    delete[] firstInstruction;
}

void Code::init(const CompilationUnit &compilation, NodePool *pool)
{
    optimized = false;
    const QVector<QScriptInstruction> ilist = compilation.instructions();
    firstInstruction = new QScriptInstruction[ilist.count()];
    lastInstruction = firstInstruction + ilist.count();
    qCopy(ilist.begin(), ilist.end(), firstInstruction);
    exceptionHandlers = compilation.exceptionHandlers();
    astPool = pool;
}

} // namespace QScript

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT
