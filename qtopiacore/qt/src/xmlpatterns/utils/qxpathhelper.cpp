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

#include <QStringList>

#include "private/qxmlutils_p.h"
#include "qbuiltintypes_p.h"
#include "qcommonvalues_p.h"
#include "qnamepool_p.h"

#include "qxpathhelper_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

bool XPathHelper::isReservedNamespace(const QXmlName::NamespaceCode ns)
{
    /* The order is because of that XFN and WXS are the most common. */
    return ns == StandardNamespaces::fn     ||
           ns == StandardNamespaces::xs     ||
           ns == StandardNamespaces::xml    ||
           ns == StandardNamespaces::xsi;
}

bool XPathHelper::isQName(const QString &qName)
{
    const QStringList result(qName.split(QLatin1Char(':')));
    const int c = result.count();

    if(c == 2)
    {
        return QXmlUtils::isNCName(result.first()) &&
               QXmlUtils::isNCName(result.last());
    }
    else if(c == 1)
        return QXmlUtils::isNCName(result.first());
    else
        return false;
}

void XPathHelper::splitQName(const QString &qName, QString &prefix, QString &ncName)
{
    Q_ASSERT_X(isQName(qName), Q_FUNC_INFO,
               "qName must be a valid QName.");

    const QStringList result(qName.split(QLatin1Char(':')));

    if(result.count() == 1)
    {
        Q_ASSERT(QXmlUtils::isNCName(result.first()));
        ncName = result.first();
    }
    else
    {
        Q_ASSERT(result.count() == 2);
        Q_ASSERT(QXmlUtils::isNCName(result.first()));
        Q_ASSERT(QXmlUtils::isNCName(result.last()));

        prefix = result.first();
        ncName = result.last();
    }
}

ItemType::Ptr XPathHelper::typeFromKind(const QXmlNodeModelIndex::NodeKind nodeKind)
{
    switch(nodeKind)
    {
        case QXmlNodeModelIndex::Element:
            return BuiltinTypes::element;
        case QXmlNodeModelIndex::Attribute:
            return BuiltinTypes::attribute;
        case QXmlNodeModelIndex::Text:
            return BuiltinTypes::text;
        case QXmlNodeModelIndex::ProcessingInstruction:
            return BuiltinTypes::pi;
        case QXmlNodeModelIndex::Comment:
            return BuiltinTypes::comment;
        case QXmlNodeModelIndex::Document:
            return BuiltinTypes::document;
        default:
        {
            Q_ASSERT_X(false, Q_FUNC_INFO,
                       "A node type that doesn't exist in the XPath Data Model was encountered.");
            return ItemType::Ptr(); /* Dummy, silence compiler warning. */
        }
    }
}

QT_END_NAMESPACE
