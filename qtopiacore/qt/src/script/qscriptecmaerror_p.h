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

#ifndef QSCRIPTECMAERROR_P_H
#define QSCRIPTECMAERROR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qscriptecmacore_p.h"

#ifndef QT_NO_SCRIPT

QT_BEGIN_NAMESPACE

namespace QScript { namespace Ecma {

class Error: public Core
{
public:
    Error(QScriptEnginePrivate *engine);
    virtual ~Error();

    virtual void execute(QScriptContextPrivate *context);

    virtual void mark(QScriptEnginePrivate *eng, int generation);

    void newError(QScriptValueImpl *result, const QString &message = QString());
    void newEvalError(QScriptValueImpl *result, const QString &message = QString());
    void newRangeError(QScriptValueImpl *result, const QString &message = QString());
    void newReferenceError(QScriptValueImpl *result, const QString &message = QString());
    void newSyntaxError(QScriptValueImpl *result, const QString &message = QString());
    void newTypeError(QScriptValueImpl *result, const QString &message = QString());
    void newURIError(QScriptValueImpl *result, const QString &message = QString());

    bool isEvalError(const QScriptValueImpl &value) const;
    bool isRangeError(const QScriptValueImpl &value) const;
    bool isReferenceError(const QScriptValueImpl &value) const;
    bool isSyntaxError(const QScriptValueImpl &value) const;
    bool isTypeError(const QScriptValueImpl &value) const;
    bool isURIError(const QScriptValueImpl &value) const;

    static QStringList backtrace(const QScriptValueImpl &error);

    QScriptValueImpl evalErrorCtor;
    QScriptValueImpl rangeErrorCtor;
    QScriptValueImpl referenceErrorCtor;
    QScriptValueImpl syntaxErrorCtor;
    QScriptValueImpl typeErrorCtor;
    QScriptValueImpl uriErrorCtor;

    QScriptValueImpl evalErrorPrototype;
    QScriptValueImpl rangeErrorPrototype;
    QScriptValueImpl referenceErrorPrototype;
    QScriptValueImpl syntaxErrorPrototype;
    QScriptValueImpl typeErrorPrototype;
    QScriptValueImpl uriErrorPrototype;

protected:
    static QScriptValueImpl method_toString(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo);
    static QScriptValueImpl method_backtrace(QScriptContextPrivate *context, QScriptEnginePrivate *eng, QScriptClassInfo *classInfo);

private:
    void newError(QScriptValueImpl *result, const QScriptValueImpl &proto,
                  const QString &message = QString());
    void newErrorPrototype(QScriptValueImpl *result, const QScriptValueImpl &proto,
                           QScriptValueImpl &ztor, const QString &name);
};

} } // namespace QScript::Ecma

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT
#endif // QSCRIPTECMAERROR_P_H
