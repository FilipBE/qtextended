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

#ifndef Patternist_ParserContext_H
#define Patternist_ParserContext_H

#include <QFlags>
#include <QSharedData>
#include <QStack>
#include <QStringList>
#include <QtGlobal>

#include "qexpressionfactory_p.h"
#include "quserfunction_p.h"
#include "quserfunctioncallsite_p.h"
#include "qvariabledeclaration_p.h"
#include "qfunctionsignature_p.h"
#include "qtokenizer_p.h"
#include "qbuiltintypes_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Contains data used when parsing and tokenizing.
     *
     * When ExpressionFactory::create() is called, an instance of this class
     * is passed to the scanner and parser. It holds all information that is
     * needed to create the expression.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ParserContext : public QSharedData
    {
    public:
        typedef QExplicitlySharedDataPointer<ParserContext> Ptr;

        enum PrologDeclaration
        {
            BoundarySpaceDecl               = 1,
            DefaultCollationDecl            = 2,
            BaseURIDecl                     = 4,
            ConstructionDecl                = 8,
            OrderingModeDecl                = 16,
            EmptyOrderDecl                  = 32,
            CopyNamespacesDecl              = 64,
            DeclareDefaultElementNamespace  = 128,
            DeclareDefaultFunctionNamespace = 256
        };

        typedef QFlags<PrologDeclaration> PrologDeclarations;

        /**
         * Constructs a ParserContext instance.
         *
         * @param context the static context as defined in XPath. This contain
         * namespace bindings, error handler, and other information necessary
         * for creating an XPath expression.
         * @param lang the particular XPath language sub-set that should be parsed
         * @param tokenizer the Tokenizer to use.
         * @see ExpressionFactory::LanguageAccent
         */
        ParserContext(const StaticContext::Ptr &context,
                      const ExpressionFactory::LanguageAccent lang,
                      const Tokenizer::Ptr &tokenizer);

        inline VariableSlotID allocateRangeSlot()
        {
            ++m_rangeSlot;
            return m_rangeSlot;
        }

        inline VariableSlotID allocatePositionalSlot()
        {
            ++m_positionSlot;
            return m_positionSlot;
        }

        inline VariableSlotID allocateExpressionSlot()
        {
            // TODO simplify this crap.
            const VariableSlotID retval = m_expressionSlot;
            ++m_expressionSlot;
            return retval;
        }

        inline VariableSlotID allocateGlobalVariableSlot()
        {
            ++m_globalVariableSlot;
            return m_globalVariableSlot;
        }

        inline bool hasDeclaration(const PrologDeclaration decl) const
        {
            return (m_prologDeclarations & decl) == decl;
        }

        inline void registerDeclaration(const PrologDeclaration decl)
        {
            m_prologDeclarations |= decl;
        }

        /**
         * The namespaces declared with <tt>declare namespace</tt>.
         */
        QStringList declaredPrefixes;

        /**
         * This is a temporary stack, used for keeping variables in scope,
         * such as for function arguments & let clauses.
         */
        VariableDeclaration::Stack variables;

        const StaticContext::Ptr staticContext;
        const Tokenizer::Ptr tokenizer;
        const ExpressionFactory::LanguageAccent languageAccent;

        /**
         * Used when parsing direct element constructors. It is used
         * for ensuring tags are well-balanced.
         */
        QStack<QXmlName> tagStack;

        /**
         * The actual expression, the Query. This member may be @c null,
         * such as in the case of an XQuery library module.
         */
        Expression::Ptr queryBody;

        /**
         * The user functions declared in the prolog.
         */
        UserFunction::List userFunctions;

        /**
         * Contains all calls to user defined functions.
         */
        UserFunctionCallsite::List userFunctionCallsites;

        /**
         * All variables declared with <tt>declare variable</tt>.
         */
        VariableDeclaration::List declaredVariables;

        inline VariableSlotID currentRangeSlot() const
        {
            return m_rangeSlot;
        }

        inline VariableSlotID currentPositionSlot() const
        {
            return m_positionSlot;
        }

        inline VariableSlotID currentExpressionSlot() const
        {
            return m_expressionSlot;
        }

        inline void restoreNodeTestSource()
        {
            nodeTestSource = BuiltinTypes::element;
        }

        inline VariableSlotID allocateCacheSlot()
        {
            return ++m_evaluationCacheSlot;
        }

        inline VariableSlotID allocateCacheSlots(const int count)
        {
            const VariableSlotID retval = m_evaluationCacheSlot + 1;
            m_evaluationCacheSlot += count + 1;
            return retval;
        }

        ItemType::Ptr nodeTestSource;

        QStack<Expression::Ptr> typeswitchSource;

        /**
         * The library module namespace set with <tt>declare module</tt>.
         */
        QXmlName::NamespaceCode moduleNamespace;

        /**
         * When a direct element constructor is processed, resolvers are
         * created in order to carry the namespace declarations. In such case,
         * the old resolver is pushed here.
         */
        QStack<NamespaceResolver::Ptr> resolvers;

        /**
         * This is used for handling the following obscene case:
         *
         * - <tt>\<e\>{1}{1}\<\/e\></tt> produce <tt>\<e\>11\</e\></tt>
         * - <tt>\<e\>{1, 1}\<\/e\></tt> produce <tt>\<e\>1 1\</e\></tt>
         *
         * This boolean tracks whether the previous reduction inside element
         * content was done with an enclosed expression.
         */
        bool isPreviousEnclosedExpr;

        int elementConstructorDepth;

        QStack<bool> scanOnlyStack;

        QStack<OrderBy::Stability> orderStability;

        /**
         * Whether any prolog declaration that must occur after the first
         * group has been encountered.
         */
        bool hasSecondPrologPart;

        bool preserveNamespacesMode;
        bool inheritNamespacesMode;
    private:
        VariableSlotID      m_evaluationCacheSlot;
        VariableSlotID      m_rangeSlot;
        VariableSlotID      m_expressionSlot;
        VariableSlotID      m_positionSlot;
        PrologDeclarations  m_prologDeclarations;
        VariableSlotID      m_globalVariableSlot;
        Q_DISABLE_COPY(ParserContext)
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
