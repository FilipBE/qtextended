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

#ifndef Patternist_BuiltinAtomicType_H
#define Patternist_BuiltinAtomicType_H

#include "qatomictype_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{

    /**
     * @short Instances of this class represents types that are sub-classes
     * of @c xs:anyAtomicType.
     *
     * Retrieving instances of builtin types is done
     * via BuiltinTypesFactory::createSchemaType(), not by instantiating this
     * class manually.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class BuiltinAtomicType : public AtomicType
    {
    public:

        typedef QExplicitlySharedDataPointer<BuiltinAtomicType> Ptr;

        /**
         * @returns always @c false
         */
        virtual bool isAbstract() const;

        /**
         * @returns the base type as specified in the constructors baseType argument.
         */
        virtual SchemaType::Ptr wxsSuperType() const;

        /**
         * @returns the same type as wxsSuperType(), except for the type @c xs:anyAtomicType, which
         * returns item()
         */
        virtual ItemType::Ptr xdtSuperType() const;

        virtual AtomicComparatorLocator::Ptr comparatorLocator() const;
        virtual AtomicMathematicianLocator::Ptr mathematicianLocator() const;
        virtual AtomicCasterLocator::Ptr casterLocator() const;

    protected:
        friend class BuiltinTypes;

        /**
         * @param baseType the type that is the super type of the constructed
         * atomic type. In the case of AnyAtomicType, @c null is passed.
         * @param comp the AtomicComparatorLocator this type should return. May be @c null.
         * @param mather similar to @p comp, this is the AtomicMathematicianLocator
         * that's appropriate for this type May be @c null.
         * @param casterLocator the CasterLocator that locates classes performing
         * casting with this type. May be @c null.
         */
        BuiltinAtomicType(const AtomicType::Ptr &baseType,
                          const AtomicComparatorLocator::Ptr &comp,
                          const AtomicMathematicianLocator::Ptr &mather,
                          const AtomicCasterLocator::Ptr &casterLocator);

    private:
        const AtomicType::Ptr                   m_superType;
        const AtomicComparatorLocator::Ptr      m_comparatorLocator;
        const AtomicMathematicianLocator::Ptr   m_mathematicianLocator;
        const AtomicCasterLocator::Ptr          m_casterLocator;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
