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

#include "qnamepool_p.h"

#include "qgenericnamespaceresolver_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

GenericNamespaceResolver::GenericNamespaceResolver(const Bindings &list) : m_bindings(list)
{
}

void GenericNamespaceResolver::addBinding(const QXmlName nb)
{
    if(nb.namespaceURI() == StandardNamespaces::UndeclarePrefix)
        m_bindings.remove(nb.prefix());
    else
        m_bindings.insert(nb.prefix(), nb.namespaceURI());
}

QXmlName::NamespaceCode GenericNamespaceResolver::lookupNamespaceURI(const QXmlName::PrefixCode prefix) const
{
    return m_bindings.value(prefix, NoBinding);
}

NamespaceResolver::Ptr GenericNamespaceResolver::defaultXQueryBindings()
{
    Bindings list;

    list.insert(StandardPrefixes::xml,    StandardNamespaces::xml);
    list.insert(StandardPrefixes::xs,     StandardNamespaces::xs);
    list.insert(StandardPrefixes::xsi,    StandardNamespaces::xsi);
    list.insert(StandardPrefixes::fn,     StandardNamespaces::fn);
    list.insert(StandardPrefixes::local,  StandardNamespaces::local);
    list.insert(StandardPrefixes::empty,  StandardNamespaces::empty);

    return NamespaceResolver::Ptr(new GenericNamespaceResolver(list));
}

NamespaceResolver::Bindings GenericNamespaceResolver::bindings() const
{
    return m_bindings;
}

QT_END_NAMESPACE
