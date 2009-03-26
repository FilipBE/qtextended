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
#include "qcommonvalues_p.h"
#include "qgenericsequencetype_p.h"
#include "qinsertioniterator_p.h"
#include "qpatternistlocale_p.h"

#include "qcardinalityverifier_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

QString CardinalityVerifier::wrongCardinality(const Cardinality &req,
                                              const Cardinality &got)
{
    return QtXmlPatterns::tr("Required cardinality is %1; got cardinality %2.")
                .arg(formatType(req), formatType(got));
}

Expression::Ptr CardinalityVerifier::verifyCardinality(const Expression::Ptr &operand,
                                                       const Cardinality &requiredCard,
                                                       const ReportContext::Ptr &context,
                                                       const ReportContext::ErrorCode code)
{
    const Cardinality opCard(operand->staticType()->cardinality());

    if(requiredCard.isMatch(opCard))
        return operand;
    else if(requiredCard.canMatch(opCard))
        return Expression::Ptr(new CardinalityVerifier(operand, requiredCard, code));
    else
    {
        /* Sequences within this cardinality can never match. */
        context->error(wrongCardinality(requiredCard, opCard), code, operand.data());
        return operand;
    }
}

CardinalityVerifier::CardinalityVerifier(const Expression::Ptr &operand,
                                         const Cardinality &card,
                                         const ReportContext::ErrorCode code)
                                         : SingleContainer(operand),
                                           m_reqCard(card),
                                           m_allowsMany(operand->staticType()->cardinality().allowsMany()),
                                           m_errorCode(code)
{
    Q_ASSERT_X(m_reqCard != Cardinality::zeroOrMore(), Q_FUNC_INFO,
               "It makes no sense to use CardinalityVerifier for cardinality zero-or-more.");
}

Item::Iterator::Ptr CardinalityVerifier::evaluateSequence(const DynamicContext::Ptr &context) const
{
    const Item::Iterator::Ptr it(m_operand->evaluateSequence(context));
    const Item next(it->next());

    if(next)
    {
        const Item next2(it->next());

        if(next2)
        {
            if(m_reqCard.allowsMany())
            {
                Item::List start;
                start.append(next);
                start.append(next2);

                return Item::Iterator::Ptr(new InsertionIterator(it, 1, makeListIterator(start)));
            }
            else
            {
                context->error(wrongCardinality(m_reqCard, Cardinality::twoOrMore()), m_errorCode, this);
                return CommonValues::emptyIterator;
            }
        }
        else
            return makeSingletonIterator(next);
    }
    else
    {
        if(m_reqCard.allowsEmpty())
            return CommonValues::emptyIterator;
        else
        {
            context->error(wrongCardinality(m_reqCard, Cardinality::twoOrMore()), m_errorCode, this);
            return CommonValues::emptyIterator;
        }
    }
}

Item CardinalityVerifier::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    if(m_allowsMany)
    {
        const Item::Iterator::Ptr it(m_operand->evaluateSequence(context));
        const Item item(it->next());

        if(item)
        {
            if(it->next())
            {
                context->error(wrongCardinality(m_reqCard, Cardinality::twoOrMore()),
                               m_errorCode, this);
                return Item();
            }
            else
                return item;
        }
        else if(m_reqCard.allowsEmpty())
            return Item();
        else
        {
            context->error(wrongCardinality(m_reqCard), m_errorCode, this);
            return Item();
        }
    }
    else
    {
        const Item item(m_operand->evaluateSingleton(context));

        if(item)
            return item;
        else if(m_reqCard.allowsEmpty())
            return Item();
        else
        {
            context->error(wrongCardinality(m_reqCard), m_errorCode, this);
            return Item();
        }
    }
}

const SourceLocationReflection *CardinalityVerifier::actualReflection() const
{
    return m_operand->actualReflection();
}

Expression::Ptr CardinalityVerifier::typeCheck(const StaticContext::Ptr &context,
                                               const SequenceType::Ptr &reqType)
{
    if(m_reqCard.isMatch(m_operand->staticType()->cardinality()))
        return m_operand->typeCheck(context, reqType);
    else
        return SingleContainer::typeCheck(context, reqType);
}

SequenceType::List CardinalityVerifier::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

SequenceType::Ptr CardinalityVerifier::staticType() const
{
    return makeGenericSequenceType(m_operand->staticType()->itemType(), m_reqCard);
}

ExpressionVisitorResult::Ptr CardinalityVerifier::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

QT_END_NAMESPACE
