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

#include "qcommonsequencetypes_p.h"
#include "qgenericsequencetype_p.h"
#include "qitemmappingiterator_p.h"

#include "qquantifiedexpression_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

QuantifiedExpression::QuantifiedExpression(const VariableSlotID varSlot,
                                           const Operator quantifier,
                                           const Expression::Ptr &inClause,
                                           const Expression::Ptr &testExpression)
                                           : PairContainer(inClause, testExpression),
                                             m_varSlot(varSlot),
                                             m_quantifier(quantifier)
{
    Q_ASSERT(quantifier == Some || quantifier == Every);
}

Item QuantifiedExpression::mapToItem(const Item &item,
                                          const DynamicContext::Ptr &context) const
{
    context->setRangeVariable(m_varSlot, item);
    return item;
}

bool QuantifiedExpression::evaluateEBV(const DynamicContext::Ptr &context) const
{
    const Item::Iterator::Ptr it(makeItemMappingIterator<Item>(ConstPtr(this),
                                                               m_operand1->evaluateSequence(context),
                                                               context));

    Item item(it->next());

    if(m_quantifier == Some)
    {
        while(item)
        {
            if(m_operand2->evaluateEBV(context))
                return true;
            else
                item = it->next();
        };

        return false;
    }
    else
    {
        Q_ASSERT(m_quantifier == Every);

        while(item)
        {
            if(m_operand2->evaluateEBV(context))
                item = it->next();
            else
                return false;
        }

        return true;
    }
}

QString QuantifiedExpression::displayName(const Operator quantifier)
{
    if(quantifier == Some)
        return QLatin1String("some");
    else
    {
        Q_ASSERT(quantifier == Every);
        return QLatin1String("every");
    }
}

SequenceType::Ptr QuantifiedExpression::staticType() const
{
    return CommonSequenceTypes::ExactlyOneBoolean;
}

SequenceType::List QuantifiedExpression::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    result.append(CommonSequenceTypes::EBV);
    return result;
}

QuantifiedExpression::Operator QuantifiedExpression::operatorID() const
{
    return m_quantifier;
}

ExpressionVisitorResult::Ptr QuantifiedExpression::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

QT_END_NAMESPACE
