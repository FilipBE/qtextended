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

#include "qbuiltintypes_p.h"
#include "qcommonnamespaces_p.h"
#include "qcommonsequencetypes_p.h"
#include "qpatternistlocale_p.h"
#include "qqnamevalue_p.h"
#include "qatomicstring_p.h"

#include "qattributenamevalidator_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

AttributeNameValidator::AttributeNameValidator(const Expression::Ptr &source) : SingleContainer(source)
{
}

Item AttributeNameValidator::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item name(m_operand->evaluateSingleton(context));
    const QXmlName qName(name.as<QNameValue>()->qName());

    if(qName.namespaceURI() == StandardNamespaces::xmlns)
    {
        context->error(QtXmlPatterns::tr("The namespace URI in the name for a "
                                         "computed attribute cannot be %1.")
                       .arg(formatURI(CommonNamespaces::XMLNS)),
                       ReportContext::XQDY0044, this);
        return Item(); /* Silence warning. */
    }
    else if(qName.namespaceURI() == StandardNamespaces::empty &&
            qName.localName() == StandardLocalNames::xmlns)
    {
        context->error(QtXmlPatterns::tr("The name for a computed attribute "
                                         "cannot have the namespace URI %1 "
                                         "with the local name %2.")
                          .arg(formatURI(CommonNamespaces::XMLNS))
                          .arg(formatKeyword("xmlns")),
                       ReportContext::XQDY0044, this);
        return Item(); /* Silence warning. */
    }
    else if(!qName.hasPrefix() && qName.hasNamespace())
    {
        return Item(QNameValue::fromValue(context->namePool(),
                                          QXmlName(qName.namespaceURI(), qName.localName(), StandardPrefixes::ns0)));
    }
    else
        return name;
}

SequenceType::Ptr AttributeNameValidator::staticType() const
{
    return m_operand->staticType();
}

SequenceType::List AttributeNameValidator::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ExactlyOneQName);
    return result;
}

ExpressionVisitorResult::Ptr AttributeNameValidator::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

QT_END_NAMESPACE
