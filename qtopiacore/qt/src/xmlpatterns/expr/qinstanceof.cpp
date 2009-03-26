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

#include "qboolean_p.h"
#include "qcommonsequencetypes_p.h"
#include "qcommonvalues_p.h"
#include "qliteral_p.h"

#include "qinstanceof_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

InstanceOf::InstanceOf(const Expression::Ptr &operand,
                       const SequenceType::Ptr &tType) : SingleContainer(operand),
                                                         m_targetType(tType)
{
    Q_ASSERT(operand);
    Q_ASSERT(m_targetType);
}

bool InstanceOf::evaluateEBV(const DynamicContext::Ptr &context) const
{
    const Item::Iterator::Ptr it(m_operand->evaluateSequence(context));
    Item item(it->next());
    unsigned int count = 1;

    if(!item)
        return m_targetType->cardinality().allowsEmpty();

    do
    {
        if(!m_targetType->itemType()->itemMatches(item))
            return false;

        if(count == 2 && !m_targetType->cardinality().allowsMany())
            return false;

        item = it->next();
        ++count;
    } while(item);

    return true;
}

Expression::Ptr InstanceOf::compress(const StaticContext::Ptr &context)
{
    const Expression::Ptr me(SingleContainer::compress(context));

    if(me.data() != this || m_operand->has(DisableTypingDeduction))
        return me;

    const SequenceType::Ptr opType(m_operand->staticType());
    const ItemType::Ptr itType(m_targetType->itemType());
    const ItemType::Ptr ioType(opType->itemType());

    if(m_targetType->cardinality().isMatch(opType->cardinality()))
    {
        if(itType->xdtTypeMatches(ioType))
            return wrapLiteral(CommonValues::BooleanTrue, context, this);
        else if(!ioType->xdtTypeMatches(itType))
        {
            return wrapLiteral(CommonValues::BooleanFalse, context, this);
        }
    }
    /* The cardinality is not guaranteed to match; it will need testing. */
    else if(!ioType->xdtTypeMatches(itType))
    {
        /* There's no way it's gonna match. The cardinality is not only
         * wrong, but the item type as well. */
        return wrapLiteral(CommonValues::BooleanFalse, context, this);
    }

    return me;
}

SequenceType::Ptr InstanceOf::targetType() const
{
    return m_targetType;
}

SequenceType::Ptr InstanceOf::staticType() const
{
    return CommonSequenceTypes::ExactlyOneBoolean;
}

SequenceType::List InstanceOf::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

ExpressionVisitorResult::Ptr InstanceOf::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

QT_END_NAMESPACE
