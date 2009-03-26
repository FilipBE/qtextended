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

#include "qpatternistlocale_p.h"

#include "qoutputvalidator_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

OutputValidator::OutputValidator(QAbstractXmlReceiver *const receiver,
                                 const DynamicContext::Ptr &context,
                                 const SourceLocationReflection *const r) : DelegatingSourceLocationReflection(r)
                                                                          , m_hasReceivedChildren(false)
                                                                          , m_receiver(receiver)
                                                                          , m_context(context)
{
    Q_ASSERT(receiver);
    Q_ASSERT(context);
}

void OutputValidator::namespaceBinding(const QXmlName &nb)
{
    m_receiver->namespaceBinding(nb);
}

void OutputValidator::startElement(const QXmlName &name)
{
    m_hasReceivedChildren = false;
    m_receiver->startElement(name);
    m_attributes.clear();
}

void OutputValidator::endElement()
{
    m_hasReceivedChildren = true;
    m_receiver->endElement();
}

void OutputValidator::attribute(const QXmlName &name,
                                const QStringRef &value)
{
    if(m_hasReceivedChildren)
    {
        m_context->error(QtXmlPatterns::tr("It's not possible to add attributes after any other kind of node."),
                            ReportContext::XQTY0024, this);
    }
    else
    {
        if(m_attributes.contains(name))
        {
            m_context->error(QtXmlPatterns::tr("An attribute by name %1 has already been created.").arg(formatKeyword(m_context->namePool(), name)),
                                ReportContext::XQDY0025, this);
        }
        else
        {
            m_attributes.insert(name);
            m_receiver->attribute(name, value);
        }
    }
}

void OutputValidator::comment(const QString &value)
{
    m_hasReceivedChildren = true;
    m_receiver->comment(value);
}

void OutputValidator::characters(const QStringRef &value)
{
    m_hasReceivedChildren = true;
    m_receiver->characters(value);
}

void OutputValidator::processingInstruction(const QXmlName &name,
                                            const QString &value)
{
    m_hasReceivedChildren = true;
    m_receiver->processingInstruction(name, value);
}

void OutputValidator::item(const Item &outputItem)
{
    /* We can't send outputItem directly to m_receiver since its item() function
     * won't dispatch to this OutputValidator, but to itself. We're not sub-classing here,
     * we're delegating. */

    if(outputItem.isNode())
        sendAsNode(outputItem);
    else
    {
        m_hasReceivedChildren = true;
        m_receiver->item(outputItem);
    }
}

void OutputValidator::startDocument()
{
    m_receiver->startDocument();
}

void OutputValidator::endDocument()
{
    m_receiver->endDocument();
}

void OutputValidator::atomicValue(const QVariant &value)
{
    Q_UNUSED(value);
    // TODO
}

void OutputValidator::endOfSequence()
{
}

void OutputValidator::startOfSequence()
{
}

QT_END_NAMESPACE
