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

#ifndef Patternist_ExpressionDispatch_H
#define Patternist_ExpressionDispatch_H

#include <QSharedData>


QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    class AndExpression;
    class ArgumentConverter;
    class ArgumentReference;
    class ArithmeticExpression;
    class Atomizer;
    class AttributeConstructor;
    class AttributeNameValidator;
    class CardinalityVerifier;
    class CastAs;
    class CastableAs;
    class CollationChecker;
    class CombineNodes;
    class CommentConstructor;
    class CopyOf;
    class ContextItem;
    class DocumentConstructor;
    class DynamicContextStore;
    class ElementConstructor;
    class EmptySequence;
    class ExpressionSequence;
    template<bool IsForGlobal> class EvaluationCache;
    class ExpressionVariableReference;
    class ExternalVariableReference;
    class FirstItemPredicate;
    class ForClause;
    class FunctionCall;
    class GeneralComparison;
    class GenericPredicate;
    class IfThenClause;
    class InstanceOf;
    class ItemVerifier;
    class LetClause;
    class Literal;
    class LiteralSequence;
    class NamespaceConstructor;
    class NCNameConstructor;
    class NodeComparison;
    class NodeSortExpression;
    class OrderBy;
    class OrExpression;
    class ParentNodeAxis;
    class Path;
    class PositionalVariableReference;
    class ProcessingInstructionConstructor;
    class QNameConstructor;
    class QuantifiedExpression;
    class RangeExpression;
    class ReturnOrderBy;
    class RangeVariableReference;
    class SimpleContentConstructor;
    class AxisStep;
    class TextNodeConstructor;
    class TreatAs;
    class TruthPredicate;
    class UntypedAtomicConverter;
    class UserFunctionCallsite;
    class ValidationError;
    class ValueComparison;

    /**
     * @todo Documentation's missing
     *
     * @defgroup Patternist_expr_dispatch Expression Dispatching
     */

    /**
     * @ingroup Patternist_expr_dispatch
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ExpressionVisitorResult : public QSharedData
    {
    public:
        typedef QExplicitlySharedDataPointer<ExpressionVisitorResult> Ptr;
        ExpressionVisitorResult() {}
        virtual ~ExpressionVisitorResult() {}
    };

    /**
     * @ingroup Patternist_expr_dispatch
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ExpressionVisitor : public QSharedData
    {
    public:
        typedef QExplicitlySharedDataPointer<ExpressionVisitor> Ptr;
        virtual ~ExpressionVisitor() {}

        virtual ExpressionVisitorResult::Ptr visit(const AndExpression *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ArithmeticExpression *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ArgumentConverter *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ArgumentReference *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const Atomizer *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const AttributeConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const AttributeNameValidator *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const CardinalityVerifier *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const CastableAs *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const CastAs *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const CollationChecker *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const CombineNodes *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const CommentConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ContextItem *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const CopyOf *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const DocumentConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const DynamicContextStore *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ElementConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const EmptySequence *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const EvaluationCache<false> *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const EvaluationCache<true> *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ExpressionSequence *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ExpressionVariableReference *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ExternalVariableReference *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const FirstItemPredicate *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ForClause *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const FunctionCall *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const GeneralComparison *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const GenericPredicate *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const IfThenClause *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const AxisStep *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const InstanceOf *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ItemVerifier *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const LetClause *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const Literal *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const LiteralSequence *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const NamespaceConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const NCNameConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const NodeComparison *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const NodeSortExpression *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const OrderBy *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const OrExpression *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ParentNodeAxis *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const Path *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const PositionalVariableReference *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ProcessingInstructionConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const QNameConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const QuantifiedExpression *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const RangeExpression *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const RangeVariableReference *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ReturnOrderBy *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const SimpleContentConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const TextNodeConstructor *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const TreatAs *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const TruthPredicate *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const UntypedAtomicConverter *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const UserFunctionCallsite *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ValidationError *) const = 0;
        virtual ExpressionVisitorResult::Ptr visit(const ValueComparison *) const = 0;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
