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

#include "qmultiitemtype_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

MultiItemType::MultiItemType(const ItemType::List &list) : m_types(list),
                                                           m_end(list.constEnd())
{
    Q_ASSERT_X(list.count() >= 2, Q_FUNC_INFO,
               "It makes no sense to use MultiItemType for types less than two.");
    Q_ASSERT_X(list.count(ItemType::Ptr()) == 0, Q_FUNC_INFO,
               "No member in the list can be null.");
}

QString MultiItemType::displayName(const NamePool::Ptr &np) const
{
    QString result;
    ItemType::List::const_iterator it(m_types.constBegin());

    while(true)
    {
        result += (*it)->displayName(np);
        ++it;

        if(it != m_end)
            result += QLatin1String(" | ");
        else
            break;
    }

    return result;
}

bool MultiItemType::itemMatches(const Item &item) const
{
    for(ItemType::List::const_iterator it(m_types.constBegin()); it != m_end; ++it)
        if((*it)->itemMatches(item))
            return true;

    return false;
}

bool MultiItemType::xdtTypeMatches(const ItemType::Ptr &type) const
{
    for(ItemType::List::const_iterator it(m_types.constBegin()); it != m_end; ++it)
        if((*it)->xdtTypeMatches(type))
            return true;

    return false;
}

bool MultiItemType::isNodeType() const
{
    for(ItemType::List::const_iterator it(m_types.constBegin()); it != m_end; ++it)
        if((*it)->isNodeType())
            return true;

    return false;
}

bool MultiItemType::isAtomicType() const
{
    for(ItemType::List::const_iterator it(m_types.constBegin()); it != m_end; ++it)
        if((*it)->isAtomicType())
            return true;

    return false;
}

ItemType::Ptr MultiItemType::xdtSuperType() const
{
    ItemType::List::const_iterator it(m_types.constBegin());
    /* Load the first one, and jump over it in the loop. */
    ItemType::Ptr result((*it)->xdtSuperType());
    ++it;

    for(; it != m_end; ++it)
        result |= (*it)->xdtSuperType();

    return result;
}

ItemType::Ptr MultiItemType::atomizedType() const
{
    ItemType::List::const_iterator it(m_types.constBegin());
    /* Load the first one, and jump over it in the loop. */
    ItemType::Ptr result((*it)->atomizedType());
    ++it;

    for(; it != m_end; ++it)
        result |= (*it)->atomizedType();

    return result;
}

QT_END_NAMESPACE
