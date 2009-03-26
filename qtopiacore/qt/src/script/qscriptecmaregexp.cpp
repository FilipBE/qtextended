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

#include "qscriptecmaregexp_p.h"

#ifndef QT_NO_SCRIPT

#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

#include <QtCore/QStringList>
#include <QtCore/QRegExp>
#include <QtCore/QtDebug>

QT_BEGIN_NAMESPACE

namespace QScript { namespace Ecma {

RegExp::RegExp(QScriptEnginePrivate *eng):
    Core(eng, QLatin1String("RegExp"), QScriptClassInfo::RegExpType)
{
    newRegExp(&publicPrototype, QString(), QString());

    eng->newConstructor(&ctor, this, publicPrototype);

    addPrototypeFunction(QLatin1String("exec"), method_exec, 1);
    addPrototypeFunction(QLatin1String("test"), method_test, 1);
    addPrototypeFunction(QLatin1String("toString"), method_toString, 1);
}

RegExp::~RegExp()
{
}

RegExp::Instance *RegExp::Instance::get(const QScriptValueImpl &object, QScriptClassInfo *klass)
{
    if (! klass || klass == object.classInfo())
        return static_cast<Instance*> (object.objectData());

    return 0;
}

void RegExp::execute(QScriptContextPrivate *context)
{
#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    engine()->notifyFunctionEntry(context);
#endif
    QString source;
    QString flags;
    QScriptValueImpl error;

    if (context->argumentCount() > 0)
        source = context->argument(0).toString();

    if (context->argumentCount() > 1) {
        flags = context->argument(1).toString();
        if (!flags.isEmpty()) {
            // check that the flags are OK
            const QString legalFlags = QLatin1String("gim");
            for (int i = 0; i < flags.length(); ++i) {
                if (legalFlags.indexOf(flags.at(i)) == -1) {
                    error = context->throwError(
                        QScriptContext::SyntaxError,
                        QString::fromUtf8("invalid regular expression flag '%0'")
                        .arg(flags.at(i)));
                    break;
                }
            }
        }
    }
    if (!error.isValid())
        newRegExp(&context->m_result, source, flags);
#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
    engine()->notifyFunctionExit(context);
#endif
}

void RegExp::newRegExp(QScriptValueImpl *result, const QString &pattern, const QString &flags)
{
#ifndef QT_NO_REGEXP
    bool ignoreCase = flags.contains(QLatin1Char('i'));

    QRegExp rx(pattern,
        (ignoreCase ? Qt::CaseInsensitive: Qt::CaseSensitive), QRegExp::RegExp2);

    newRegExp_helper(result, rx, flags);
#else
    Instance *instance = new Instance();
    instance->pattern = pattern;
    instance->flags = flags;

    QScriptEnginePrivate *eng = engine();
    eng->newObject(result, publicPrototype, classInfo());
    result->setObjectData(instance);
    result->setProperty(QLatin1String("source"), QScriptValueImpl(eng, pattern), QScriptValue::ReadOnly);
#endif // QT_NO_REGEXP
}

#ifndef QT_NO_REGEXP
void RegExp::newRegExp(QScriptValueImpl *result, const QRegExp &rx, const QString &flags)
{
    Q_ASSERT(!flags.contains(QLatin1Char('i'))
             || (rx.caseSensitivity() == Qt::CaseInsensitive));
    newRegExp_helper(result, rx, flags);
}

void RegExp::newRegExp_helper(QScriptValueImpl *result, const QRegExp &rx,
                              const QString &flags)
{
    bool global = flags.contains(QLatin1Char('g'));
    bool ignoreCase = flags.contains(QLatin1Char('i'));
    bool multiline = flags.contains(QLatin1Char('m'));

    Instance *instance = new Instance();
    instance->value = rx;
    instance->flags = flags;

    QScriptEnginePrivate *eng = engine();
    eng->newObject(result, publicPrototype, classInfo());
    result->setObjectData(instance);

    QScriptValue::PropertyFlags propertyFlags = QScriptValue::SkipInEnumeration
                                                | QScriptValue::Undeletable
                                                | QScriptValue::ReadOnly;

    result->setProperty(QLatin1String("global"), QScriptValueImpl(eng, global),
                        propertyFlags);
    result->setProperty(QLatin1String("ignoreCase"), QScriptValueImpl(eng, ignoreCase),
                        propertyFlags);
    result->setProperty(QLatin1String("multiline"), QScriptValueImpl(eng, multiline),
                        propertyFlags);
    result->setProperty(QLatin1String("source"), QScriptValueImpl(eng, rx.pattern()),
                        propertyFlags);
    result->setProperty(QLatin1String("lastIndex"), QScriptValueImpl(eng, 0),
                        propertyFlags & ~QScriptValue::ReadOnly);
}

QRegExp RegExp::toRegExp(const QScriptValueImpl &value) const
{
    Instance *rx_data = Instance::get(value, classInfo());
    Q_ASSERT(rx_data != 0);
    return rx_data->value;
}
#endif // QT_NO_REGEXP

QScriptValueImpl RegExp::method_exec(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl self = context->thisObject();
    if (self.classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("RegExp.prototype.exec"));

    Instance *rx_data = Instance::get(self, classInfo);
    Q_ASSERT(rx_data != 0);

    QString S = context->argument(0).toString();
    int length = S.length();
    QScriptValue lastIndex = self.property(QLatin1String("lastIndex"));

    int i = lastIndex.isValid() ? int (lastIndex.toInteger()) : 0;
    bool global = self.property(QLatin1String("global")).toBoolean();

    if (! global)
        i = 0;

    if (i < 0 || i >= length)
        return (eng->nullValue());

#ifndef QT_NO_REGEXP
    int index = rx_data->value.indexIn(S, i);
    if (index == -1)
#endif // QT_NO_REGEXP
        return eng->nullValue();

#ifndef QT_NO_REGEXP
    int e = index + rx_data->value.matchedLength();

    if (global)
        self.setProperty(QLatin1String("lastIndex"), QScriptValueImpl(eng, e));

    QScript::Array elts;
    QStringList capturedTexts = rx_data->value.capturedTexts();
    for (int i = 0; i < capturedTexts.count(); ++i)
        elts.assign(i, QScriptValueImpl(eng, capturedTexts.at(i)));

    QScriptValueImpl r = eng->newArray(elts);

    r.setProperty(QLatin1String("index"), QScriptValueImpl(eng, index));
    r.setProperty(QLatin1String("input"), QScriptValueImpl(eng, S));

    return r;
#endif // QT_NO_REGEXP
}

QScriptValueImpl RegExp::method_test(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    QScriptValueImpl r = method_exec(context, eng, classInfo);
    return QScriptValueImpl(eng, !r.isNull());
}

QScriptValueImpl RegExp::method_toString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo)
{
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        QString pattern;
        pattern += QLatin1Char('/');
#ifndef QT_NO_REGEXP
        pattern += instance->value.pattern(); // ### quote
#else
        pattern += instance->pattern;
#endif
        pattern += QLatin1Char('/');
        pattern += instance->flags;
        return (QScriptValueImpl(eng, pattern));
    }

    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("RegExp.prototype.toString"));
}

} } // namespace QScript::Ecma

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT
