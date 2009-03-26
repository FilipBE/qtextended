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

#include "qitem_p.h"
#include "qbuiltintypes_p.h"
#include "qitem_p.h"
#include "qschematypefactory_p.h"
#include "qxmlname.h"

#include "qatomictype_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

AtomicType::AtomicType()
{
}

AtomicType::~AtomicType()
{
}

bool AtomicType::xdtTypeMatches(const ItemType::Ptr &other) const
{
    if(other->isAtomicType())
    {
        if(*other == *this)
            return true;
        else
            return xdtTypeMatches(other->xdtSuperType());
    }
    else
        return false;
}

bool AtomicType::itemMatches(const Item &item) const
{
    Q_ASSERT(item);
    if(item.isNode())
        return false;
    else
    {
        const SchemaType::Ptr t(static_cast<AtomicType *>(item.type().data()));
        return wxsTypeMatches(t);
    }
}

ItemType::Ptr AtomicType::atomizedType() const
{
    return AtomicType::Ptr(const_cast<AtomicType *>(this));
}

QString AtomicType::displayName(const NamePool::Ptr &) const
{
    /* A bit faster than calling name()->displayName() */
    return QLatin1String("xs:anyAtomicType");
}

bool AtomicType::isNodeType() const
{
    return false;
}

bool AtomicType::isAtomicType() const
{
    return true;
}

SchemaType::TypeCategory AtomicType::category() const
{
    return SimpleTypeAtomic;
}

SchemaType::DerivationMethod AtomicType::derivationMethod() const
{
    return DerivationRestriction;
}

QT_END_NAMESPACE
