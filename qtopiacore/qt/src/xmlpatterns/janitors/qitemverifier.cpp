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
#include "qpatternistlocale_p.h"

#include "qitemverifier_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

ItemVerifier::ItemVerifier(const Expression::Ptr &operand,
                           const ItemType::Ptr &reqType,
                           const ReportContext::ErrorCode errorCode) : SingleContainer(operand),
                                                                       m_reqType(reqType),
                                                                       m_errorCode(errorCode)
{
    Q_ASSERT(reqType);
}

void ItemVerifier::verifyItem(const Item &item, const DynamicContext::Ptr &context) const
{
    if(m_reqType->itemMatches(item))
        return;

    context->error(QtXmlPatterns::tr("The item %1 did not match the required type %2.")
                                    .arg(formatData(item.stringValue()),
                                         formatType(context->namePool(), m_reqType)),
                   m_errorCode,
                   this);
}

const SourceLocationReflection *ItemVerifier::actualReflection() const
{
    return m_operand->actualReflection();
}

Item ItemVerifier::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item item(m_operand->evaluateSingleton(context));

    if(item)
    {
        verifyItem(item, context);
        return item;
    }
    else
        return Item();
}

Item ItemVerifier::mapToItem(const Item &item, const DynamicContext::Ptr &context) const
{
    verifyItem(item, context);
    return item;
}

Item::Iterator::Ptr ItemVerifier::evaluateSequence(const DynamicContext::Ptr &context) const
{
    return makeItemMappingIterator<Item>(ConstPtr(this),
                                         m_operand->evaluateSequence(context),
                                         context);
}

SequenceType::Ptr ItemVerifier::staticType() const
{
    return makeGenericSequenceType(m_reqType, m_operand->staticType()->cardinality());
}

SequenceType::List ItemVerifier::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

ExpressionVisitorResult::Ptr ItemVerifier::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

QT_END_NAMESPACE
