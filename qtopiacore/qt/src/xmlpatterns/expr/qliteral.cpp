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
#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qgenericsequencetype_p.h"

#include "qliteral_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

Literal::Literal(const Item &i) : m_item(i)
{
    Q_ASSERT(m_item);
    Q_ASSERT(m_item.isAtomicValue());
}

Item Literal::evaluateSingleton(const DynamicContext::Ptr &) const
{
    return m_item;
}

bool Literal::evaluateEBV(const DynamicContext::Ptr &context) const
{
    return Boolean::evaluateEBV(m_item, context);
}

void Literal::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
    context->outputReceiver()->item(m_item);
}

SequenceType::Ptr Literal::staticType() const
{
    return makeGenericSequenceType(m_item.type(), Cardinality::exactlyOne());
}

ExpressionVisitorResult::Ptr Literal::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

Expression::ID Literal::id() const
{
    Q_ASSERT(m_item);
    Q_ASSERT(m_item.isAtomicValue());
    const ItemType::Ptr t(m_item.type());

    if(BuiltinTypes::xsBoolean->xdtTypeMatches(t))
        return IDBooleanValue;
    else if(BuiltinTypes::xsString->xdtTypeMatches(t) ||
            BuiltinTypes::xsAnyURI->xdtTypeMatches(t) ||
            BuiltinTypes::xsUntypedAtomic->xdtTypeMatches(t))
    {
        return IDStringValue;
    }
    else if(BuiltinTypes::xsInteger->xdtTypeMatches(t))
        return IDIntegerValue;
    else
        return IDIgnorableExpression;
}

Expression::Properties Literal::properties() const
{
    return IsEvaluated;
}

QString Literal::description() const
{
    return m_item.stringValue();
}

QT_END_NAMESPACE
