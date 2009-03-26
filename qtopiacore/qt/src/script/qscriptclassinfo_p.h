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

#ifndef QSCRIPTCLASSINFO_P_H
#define QSCRIPTCLASSINFO_P_H

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

#include "qscriptclassdata_p.h"

#ifndef QT_NO_SCRIPT

#include <QtCore/qstring.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QScriptClassInfo
{
public:
    enum Type {
        FunctionBased   = 0x40000000,

        ObjectType      = 1,
        FunctionType    = 2 | FunctionBased,
        ArrayType       = 3,
        StringType      = 4,
        BooleanType     = 5,
        NumberType      = 6,
        DateType        = 7,
        RegExpType      = 8,
        ErrorType       = 9,

        VariantType     = 10,
        QObjectType     = 11 | FunctionBased,
        QMetaObjectType = 12 | FunctionBased,

        // Types used by the runtime
        ActivationType  = 100,
        EnumerationType = 101,

        CustomType      = 1000,

        TypeMask        = 0x0000FFFF
    };

    inline QScriptClassInfo(Type type, const QString &name)
        : m_type(type), m_name(name), m_data(0) { }
    inline ~QScriptClassInfo() { }

    inline Type type() const
        { return m_type; }
    inline QString name() const
        { return m_name; }

    inline void setData(QExplicitlySharedDataPointer<QScriptClassData> data)
        { m_data = data; }
    QExplicitlySharedDataPointer<QScriptClassData> data() const
        { return m_data; }

private:
    Type m_type;
    QString m_name;
    QExplicitlySharedDataPointer<QScriptClassData> m_data;

private:
    Q_DISABLE_COPY(QScriptClassInfo)
};

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT
#endif // QSCRIPTCLASSINFO_P_H
