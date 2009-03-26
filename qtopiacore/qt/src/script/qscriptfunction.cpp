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

#include "qscriptfunction_p.h"

#ifndef QT_NO_SCRIPT

#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

QT_BEGIN_NAMESPACE

QScriptFunction::~QScriptFunction()
{
}

QString QScriptFunction::toString(QScriptContextPrivate *) const
{
    QString result;
    result += QLatin1String("function () { [native] }");
    return result;
}

QString QScriptFunction::fileName() const
{
    return QString();
}

QString QScriptFunction::functionName() const
{
    return QString();
}

int QScriptFunction::startLineNumber() const
{
    return -1;
}

int QScriptFunction::endLineNumber() const
{
    return -1;
}

void QScriptFunction::mark(QScriptEnginePrivate *engine, int generation)
{
    for (int i = 0; i < formals.count(); ++i)
        engine->markString(formals.at(i), generation);
}

// public API function
void QScript::CFunction::execute(QScriptContextPrivate *context)
{
    QScriptEngine *eng = context->engine();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);

    eng_p->newUndefined(&context->m_result);

#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    eng_p->notifyFunctionEntry(context);
#endif

    QScriptValueImpl result = QScriptValuePrivate::valueOf((*m_funPtr)(eng->currentContext(), eng));
    if (result.isValid() && !eng_p->shouldAbort())
        context->m_result = result;

#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    eng_p->notifyFunctionExit(context);
#endif
}

QString QScript::CFunction::functionName() const
{
    return QString();
}

// internal API function
void QScript::C2Function::execute(QScriptContextPrivate *context)
{
    QScriptEngine *eng = context->engine();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);

    bool blocked = eng_p->blockGC(true);

#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    eng_p->notifyFunctionEntry(context);
#endif

    context->m_result = (*m_funPtr)(context, eng_p, m_classInfo);
    Q_ASSERT(context->m_result.isValid());

#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    eng_p->notifyFunctionExit(context);
#endif

    eng_p->blockGC(blocked);
}

QString QScript::C2Function::functionName() const
{
    if (!m_name.isEmpty())
        return m_name;
    return QString();
}

void QScript::C3Function::execute(QScriptContextPrivate *context)
{
    QScriptEngine *eng = context->engine();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);

    eng_p->newUndefined(&context->m_result);

#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    eng_p->notifyFunctionEntry(context);
#endif

    QScriptValueImpl result = QScriptValuePrivate::valueOf((*m_funPtr)(eng->currentContext(), eng, m_arg));
    if (result.isValid() && !eng_p->shouldAbort())
        context->m_result = result;

#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    eng_p->notifyFunctionExit(context);
#endif
}

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT
