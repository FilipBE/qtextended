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
#include "qitem_p.h"
#include "qboolean_p.h"
#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qcommonvalues_p.h"
#include "qliteral_p.h"

#include "qcastableas_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

CastableAs::CastableAs(const Expression::Ptr &operand,
                       const SequenceType::Ptr &tType) : SingleContainer(operand),
                                                         m_targetType(tType)
{
    Q_ASSERT(tType);
    Q_ASSERT(!tType->cardinality().allowsMany());
    Q_ASSERT(tType->itemType()->isAtomicType());
}

bool CastableAs::evaluateEBV(const DynamicContext::Ptr &context) const
{
    Item item;

    if(m_operand->staticType()->cardinality().allowsMany())
    {
        const Item::Iterator::Ptr it(m_operand->evaluateSequence(context));
        item = it->next();

        if(it->next())
            return false;
    }
    else
        item = m_operand->evaluateSingleton(context);

    if(item)
        return !cast(item, context).as<AtomicValue>()->hasError();
    else
        return m_targetType->cardinality().allowsEmpty();
}

Expression::Ptr CastableAs::compress(const StaticContext::Ptr &context)
{
    const Expression::Ptr me(SingleContainer::compress(context));

    if(me.data() != this) /* We already managed to const fold, how convenient. */
        return me;

    const AtomicType::Ptr t(m_targetType->itemType());

    const SequenceType::Ptr opType(m_operand->staticType());

    /* Casting to these always succeeds. */
    if(*t == *BuiltinTypes::xsString ||
       *t == *BuiltinTypes::xsUntypedAtomic ||
       (*t == *opType->itemType() &&
       (m_targetType->cardinality().isMatch(opType->cardinality()))))
    {
        return wrapLiteral(CommonValues::BooleanTrue, context, this);
    }
    else
        return me;
}

Expression::Ptr CastableAs::typeCheck(const StaticContext::Ptr &context,
                                      const SequenceType::Ptr &reqType)
{
    checkTargetType(context);
    const Expression::Ptr me(SingleContainer::typeCheck(context, reqType));

    return me;
    if(BuiltinTypes::xsQName->xdtTypeMatches(m_targetType->itemType()))
    {
        const SequenceType::Ptr seqt(m_operand->staticType());
        /* We can cast a string literal, an xs:QName value, and an
         * empty sequence(if empty is allowed), to xs:QName. */
        if(m_operand->is(IDStringValue) ||
           BuiltinTypes::xsQName->xdtTypeMatches(seqt->itemType()) ||
           (*seqt->itemType() == *CommonSequenceTypes::Empty && m_targetType->cardinality().allowsEmpty()))
        {
            return wrapLiteral(CommonValues::BooleanTrue, context, this)->typeCheck(context, reqType);
        }
        else
            return wrapLiteral(CommonValues::BooleanFalse, context, this)->typeCheck(context, reqType);
    }
    else
    {
        /* Let the CastingPlatform look up its AtomicCaster. */
        prepareCasting(context, m_operand->staticType()->itemType());

        return me;
    }
}

SequenceType::List CastableAs::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreAtomicTypes);
    return result;
}

SequenceType::Ptr CastableAs::staticType() const
{
    return CommonSequenceTypes::ExactlyOneBoolean;
}

ExpressionVisitorResult::Ptr CastableAs::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

QT_END_NAMESPACE
