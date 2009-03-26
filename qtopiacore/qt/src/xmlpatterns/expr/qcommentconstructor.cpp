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
#include "qpatternistlocale_p.h"
#include "qnodebuilder_p.h"

#include "qcommentconstructor_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

CommentConstructor::CommentConstructor(const Expression::Ptr &op) : SingleContainer(op)
{
}

QString CommentConstructor::evaluateContent(const DynamicContext::Ptr &context) const
{
    const Item item(m_operand->evaluateSingleton(context));

    if(!item)
        return QString();

    const QString content(item.stringValue());

    if(content.contains(QLatin1String("--")))
    {
        context->error(QtXmlPatterns::tr("A comment cannot contain %1")
                       .arg(formatData("--")),
                       ReportContext::XQDY0072, this);
    }
    else if(content.endsWith(QLatin1Char('-')))
    {
        context->error(QtXmlPatterns::tr("A comment cannot end with a %1.")
                       .arg(formatData(QLatin1Char('-'))),
                       ReportContext::XQDY0072, this);
    }

    return content;
}

Item CommentConstructor::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const QString content(evaluateContent(context));
    const NodeBuilder::Ptr nodeBuilder(context->nodeBuilder(QUrl()));
    nodeBuilder->comment(content);

    const QAbstractXmlNodeModel::Ptr nm(nodeBuilder->builtDocument());
    context->addNodeModel(nm);

    return nm->root(QXmlNodeModelIndex());
}

void CommentConstructor::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
    const QString content(evaluateContent(context));
    QAbstractXmlReceiver *const receiver = context->outputReceiver();

    receiver->comment(content);
}

SequenceType::Ptr CommentConstructor::staticType() const
{
    return CommonSequenceTypes::ExactlyOneComment;
}

SequenceType::List CommentConstructor::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrOneString);
    return result;
}

Expression::Properties CommentConstructor::properties() const
{
    return DisableElimination | IsNodeConstructor;
}

ExpressionVisitorResult::Ptr
CommentConstructor::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

QT_END_NAMESPACE
