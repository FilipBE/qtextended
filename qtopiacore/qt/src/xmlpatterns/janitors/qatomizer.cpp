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

#include "qatomictype_p.h"
#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qgenericsequencetype_p.h"
#include "qsequencemappingiterator_p.h"

#include "qatomizer_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

Atomizer::Atomizer(const Expression::Ptr &operand) : SingleContainer(operand)
{
}

Item::Iterator::Ptr Atomizer::mapToSequence(const Item &item, const DynamicContext::Ptr &) const
{
    /* Function & Operators, 2.4.2 fn:data, says "If the node does not have a
     * typed value an error is raised [err:FOTY0012]."
     * When does a node not have a typed value? */
    Q_ASSERT(item);
    return item.sequencedTypedValue();
}

Item::Iterator::Ptr Atomizer::evaluateSequence(const DynamicContext::Ptr &context) const
{
    return makeSequenceMappingIterator<Item>(ConstPtr(this),
                                             m_operand->evaluateSequence(context),
                                             context);
}

Item Atomizer::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item item(m_operand->evaluateSingleton(context));

    if(!item) /* Empty is allowed, cardinality is considered '?' */
        return Item();

    const Item::Iterator::Ptr it(mapToSequence(item, context));
    Q_ASSERT_X(it, Q_FUNC_INFO, "A valid QAbstractXmlForwardIterator must always be returned.");

    Item result(it->next());
    Q_ASSERT_X(!it->next(), Q_FUNC_INFO,
               "evaluateSingleton should never be used if the cardinality is two or more");

    return result;
}

Expression::Ptr Atomizer::typeCheck(const StaticContext::Ptr &context,
                                    const SequenceType::Ptr &reqType)
{
    /* Compress -- the earlier the better. */
    if(BuiltinTypes::xsAnyAtomicType->xdtTypeMatches(m_operand->staticType()->itemType()))
        return m_operand->typeCheck(context, reqType);

    return SingleContainer::typeCheck(context, reqType);
}

SequenceType::Ptr Atomizer::staticType() const
{
    const SequenceType::Ptr opt(m_operand->staticType());
    return makeGenericSequenceType(opt->itemType()->atomizedType(),
                                   opt->cardinality());
}

SequenceType::List Atomizer::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

ExpressionVisitorResult::Ptr Atomizer::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

const SourceLocationReflection *Atomizer::actualReflection() const
{
    return m_operand->actualReflection();
}

QT_END_NAMESPACE
