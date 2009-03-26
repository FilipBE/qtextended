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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef Patternist_AtomicType_H
#define Patternist_AtomicType_H

#include "qanysimpletype_p.h"
#include "qatomiccasterlocator_p.h"
#include "qatomiccomparatorlocator_p.h"
#include "qatomicmathematicianlocator_p.h"
#include "qatomictypedispatch_p.h"
#include "qitemtype_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    class Item;
    class SourceLocationReflection;

    /**
     * @short Base class for all classes that implements atomic types.
     *
     * AtomicType does not implement @c xs:anyAtomicType, it is the C++
     * base class for classes that implement atomic types, such as @c xs:anyAtomicType.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AtomicType : public ItemType,
                       public AnySimpleType
    {
    public:

        typedef QExplicitlySharedDataPointer<AtomicType> Ptr;

        virtual ~AtomicType();

        /**
         * Implements a generic algorithm which relies on wxsTypeMatches().
         *
         * @returns @c true depending on if @p item is an atomic type, and that
         * AtomicValue::itemType()'s SequenceType::itemType() matches this type.
         */
        virtual bool itemMatches(const Item &item) const;

        /**
         * @returns the result of SharedQXmlName::displayName(), of the SharedQName
         * object returned from the name() function.
         */
        virtual QString displayName(const NamePool::Ptr &np) const;

        /**
         * returns always @c false
         */
        virtual bool isNodeType() const;

        /**
         * returns always @c true
         */
        virtual bool isAtomicType() const;

        /**
         * Determines whether @p other is equal to this type, or is a
         * sub-type of this type.
         *
         * The implementation is generic, relying on operator==()
         * and xdtSuperType().
         */
        virtual bool xdtTypeMatches(const ItemType::Ptr &other) const;

        /**
         * @returns always 'this'
         */
        virtual ItemType::Ptr atomizedType() const;

        /**
         * @returns always SchemaType::SimpleTypeAtomic
         */
        virtual TypeCategory category() const;

        /**
         * @returns DerivationRestriction
         */
        virtual DerivationMethod derivationMethod() const;

        virtual AtomicTypeVisitorResult::Ptr
        accept(const QExplicitlySharedDataPointer<AtomicTypeVisitor> &visitor,
               const SourceLocationReflection *const) const = 0;

        virtual AtomicTypeVisitorResult::Ptr
        accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
               const qint16 param,
               const SourceLocationReflection *const) const = 0;

        virtual AtomicComparatorLocator::Ptr comparatorLocator() const = 0;
        virtual AtomicMathematicianLocator::Ptr mathematicianLocator() const = 0;
        virtual AtomicCasterLocator::Ptr casterLocator() const = 0;

    protected:
        AtomicType();

    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
