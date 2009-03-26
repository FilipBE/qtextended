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

#include "qfirstitempredicate_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

FirstItemPredicate::FirstItemPredicate(const Expression::Ptr &source) : SingleContainer(source)
{
}

Item FirstItemPredicate::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    /* If our operand allows the empty sequence, this function can return Item(), otherwise
     * it returns the first item. As simple as that. */
    return m_operand->evaluateSequence(context)->next();
}

SequenceType::Ptr FirstItemPredicate::staticType() const
{
    const SequenceType::Ptr t(m_operand->staticType());
    return makeGenericSequenceType(t->itemType(), t->cardinality().toWithoutMany());
}

Expression::Ptr FirstItemPredicate::compress(const StaticContext::Ptr &context)
{
    const Expression::Ptr me(SingleContainer::compress(context));
    if(me.data() != this)
        return me;

    if(m_operand->is(IDFirstItemPredicate))
        m_operand = m_operand->operands().first();

    return me;
}

SequenceType::List FirstItemPredicate::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

ExpressionVisitorResult::Ptr FirstItemPredicate::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

Expression::ID FirstItemPredicate::id() const
{
    return IDFirstItemPredicate;
}

QT_END_NAMESPACE
