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

#include "qscriptextvariant_p.h"

#ifndef QT_NO_SCRIPT

#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

#include <QtCore/QtDebug>

#include <QtCore/QStringList>

#include <limits.h>

QT_BEGIN_NAMESPACE

namespace QScript { namespace Ext {

Variant::Variant(QScriptEnginePrivate *eng):
    Ecma::Core(eng, QLatin1String("QVariant"), QScriptClassInfo::VariantType)
{
    newVariant(&publicPrototype, QVariant());

    eng->newConstructor(&ctor, this, publicPrototype);

    addPrototypeFunction(QLatin1String("toString"), method_toString, 0);
    addPrototypeFunction(QLatin1String("valueOf"), method_valueOf, 0);
}

Variant::~Variant()
{
}

Variant::Instance *Variant::Instance::get(const QScriptValueImpl &object, QScriptClassInfo *klass)
{
    if (! klass || klass == object.classInfo())
        return static_cast<Instance*> (object.objectData());

    return 0;
}

void Variant::execute(QScriptContextPrivate *context)
{
    QScriptValueImpl tmp;
    newVariant(&tmp, QVariant());
    context->setReturnValue(tmp);
}

void Variant::newVariant(QScriptValueImpl *result, const QVariant &value)
{
    Instance *instance;
    if (!result->isValid()) {
        engine()->newObject(result, publicPrototype, classInfo());
        instance = new Instance();
        result->setObjectData(instance);
    } else {
        Q_ASSERT(result->isObject());
        if (result->classInfo() != classInfo()) {
            result->destroyObjectData();
            result->setClassInfo(classInfo());
            instance = new Instance();
            result->setObjectData(instance);
        } else {
            instance = Instance::get(*result, classInfo());
        }
    }
    instance->value = value;
}

QScriptValueImpl Variant::method_toString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        QString result;
        QScriptValueImpl value = method_valueOf(context, eng, classInfo);
        if (value.isObject()) {
            result = instance->value.toString();
            if (result.isEmpty()) {
                result = QString::fromLatin1("QVariant(%0)")
                         .arg(QLatin1String(instance->value.typeName()));
            }
        } else {
            result = value.toString();
        }
        return QScriptValueImpl(eng, result);
    }
    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("QVariant.prototype.toString"));
}

QScriptValueImpl Variant::method_valueOf(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        QVariant v = instance->value;
        switch (v.type ()) {
        case QVariant::String:
            return (QScriptValueImpl(eng, v.toString()));

        case QVariant::Int:
            return (QScriptValueImpl(eng, v.toInt()));

        case QVariant::Bool:
            return (QScriptValueImpl(eng, v.toBool()));

        case QVariant::Double:
            return (QScriptValueImpl(eng, v.toDouble())); // ### hmmm

        case QVariant::Char:
            return (QScriptValueImpl(eng, v.toChar().unicode()));

        case QVariant::UInt:
            return (QScriptValueImpl(eng, v.toUInt()));

        default:
            return context->thisObject();
        } // switch
    }
    return context->thisObject();
}

} } // namespace QScript::Ecma

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT
