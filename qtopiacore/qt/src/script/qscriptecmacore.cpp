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

#include "qscriptecmacore_p.h"

#ifndef QT_NO_SCRIPT

#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

QT_BEGIN_NAMESPACE

namespace QScript { namespace Ecma {

Core::Core(QScriptEnginePrivate *engine, const QString &className,
           QScriptClassInfo::Type type)
    : m_engine(engine)
{
    m_classInfo = engine->registerClass(className, type);
    this->length = 1;
}

Core::Core(QScriptEnginePrivate *engine, QScriptClassInfo *classInfo)
    : m_engine(engine), m_classInfo(classInfo)
{
    this->length = 1;
}

Core::~Core()
{
}

void Core::addPrototypeFunction(const QString &name, QScriptInternalFunctionSignature fun,
                                int length, const QScriptValue::PropertyFlags flags)
{
    addFunction(publicPrototype, name, fun, length, flags);
}

void Core::addConstructorFunction(const QString &name, QScriptInternalFunctionSignature fun,
                                  int length, const QScriptValue::PropertyFlags flags)
{
    addFunction(ctor, name, fun, length, flags);
}

void Core::addFunction(QScriptValueImpl &object, const QString &name,
                       QScriptInternalFunctionSignature fun, int length,
                       const QScriptValue::PropertyFlags flags)
{
    QScriptValueImpl val = engine()->createFunction(fun, length, m_classInfo, name);
    object.setProperty(name, val, flags);
}

QString Core::functionName() const
{
    return m_classInfo->name();
}

void Core::mark(QScriptEnginePrivate *eng, int generation)
{
    QScriptFunction::mark(eng, generation);
    eng->markObject(ctor, generation);
    eng->markObject(publicPrototype, generation);
}

} // namespace Ecma


} // namespace QScript

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT
