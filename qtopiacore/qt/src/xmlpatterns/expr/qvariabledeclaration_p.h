/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtXMLPatterns module of the Qt Toolkit.
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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef Patternist_VariableDeclaration_H
#define Patternist_VariableDeclaration_H

#include <QSharedData>

#include "qexpression_p.h"
#include "qpatternistlocale_p.h"
#include "qvariablereference_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

template<typename T> class QStack;

namespace QPatternist
{
    /**
     * @short Represents a declared variable. Only used at
     * the compilation stage.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class VariableDeclaration : public QSharedData
    {
    public:
        typedef QExplicitlySharedDataPointer<VariableDeclaration> Ptr;
        typedef QStack<VariableDeclaration::Ptr> Stack;
        typedef QList<VariableDeclaration::Ptr> List;

        enum Type
        {
            RangeVariable,
            ExpressionVariable,
            FunctionArgument,
            PositionalVariable,

            /**
             * A global variable is always an external variable, but it is
             * cached differently.
             *
             * @see DynamicContext::globalItemCacheCell()
             */
            GlobalVariable,

            /**
             * External variables doesn't use slots, that's a big difference
             * compared to the other types.
             */
            ExternalVariable
        };

        /**
         * Creates a VariableDeclaration.
         *
         * @p sourceExpr and @p seqType may be @c null.
         */
        VariableDeclaration(const QXmlName n,
                            const VariableSlotID varSlot,
                            const Type t,
                            const SequenceType::Ptr &seqType) : name(n)
                                                              , slot(varSlot)
                                                              , type(t)
                                                              , sequenceType(seqType)
                                                              , canSourceRewrite(true)
        {
            Q_ASSERT(!name.isNull());
            Q_ASSERT(t == ExternalVariable || varSlot > -1);
        }

        inline bool isUsed() const
        {
            return !references.isEmpty();
        }

        inline const Expression::Ptr &expression() const
        {
            return m_expression;
        }

        inline void setExpression(const Expression::Ptr &expr)
        {
            m_expression = expr;
        }

        /**
         * @short Returns how many times this variable is used.
         */
        inline bool usedByMany() const
        {
            return references.count() > 1;
        }

        const QXmlName                  name;
        const VariableSlotID            slot;
        const Type                      type;
        const SequenceType::Ptr         sequenceType;
        VariableReference::List         references;

        /**
         * @short Whether a reference can rewrite itself to expression().
         *
         * The default value is @c true.
         */
        bool canSourceRewrite;

    private:
        Expression::Ptr                 m_expression;
        Q_DISABLE_COPY(VariableDeclaration)
    };

    /**
     * @short Formats @p var appropriately for display.
     *
     * @relates VariableDeclaration
     */
    static inline QString formatKeyword(const VariableDeclaration::Ptr &var,
                                        const NamePool::Ptr &np)
    {
        Q_ASSERT(var);
        Q_ASSERT(np);
        return formatKeyword(np->displayName(var->name));
    }

}

QT_END_NAMESPACE

QT_END_HEADER

#endif
