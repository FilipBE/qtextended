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

#ifndef Patternist_AbstractFloatCasters_H
#define Patternist_AbstractFloatCasters_H

#include "qabstractfloat_p.h"
#include "qatomiccaster_p.h"
#include "qnumeric_p.h"

/**
 * @file
 * @short Contains classes sub-classing AtomicCaster and which
 * are responsible of casting an atomic value to AbstractFloat.
 */

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{

    /**
     * @short Casts a @c numeric value, such as @c xs:integer or @c xs:float, to @c xs:double or xs:float.
     *
     * castFrom() uses Numeric::toDouble() for doing the actual casting.
     *
     * @ingroup Patternist_xdm
     * @author Vincent Ricard <magic@magicninja.org>
     */
    template <const bool isDouble>
    class NumericToAbstractFloatCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                              const QExplicitlySharedDataPointer<DynamicContext> &context) const;
    };

    /**
     * @short Casts a string value, @c xs:string or @c xs:untypedAtomic, to @c xs:double or xs:float.
     *
     * @ingroup Patternist_xdm
     * @author Vincent Ricard <magic@magicninja.org>
     */
    template <const bool isDouble>
    class StringToAbstractFloatCaster : public AtomicCaster
    {
    public:
        virtual Item castFrom(const Item &from,
                              const QExplicitlySharedDataPointer<DynamicContext> &context) const;
    };

    /**
     * @short Casts a value of type @c xs:boolean to @c xs:double or xs:float.
     *
     * @ingroup Patternist_xdm
     * @author Vincent Ricard <magic@magicninja.org>
     */
    template <const bool isDouble>
    class BooleanToAbstractFloatCaster : public AtomicCaster
    {
        public:
            virtual Item castFrom(const Item &from,
                                  const QExplicitlySharedDataPointer<DynamicContext> &context) const;
    };

#include "qabstractfloatcasters.cpp"

   /* * no doxygen
    * @short Casts a @c numeric value, such as @c xs:integer or @c xs:float, to @c xs:double.
    *
    * castFrom() uses Numeric::toDouble() for doing the actual casting.
    *
    * @ingroup Patternist_xdm
    * @author Frans Englich <fenglich@trolltech.com>
    */
    typedef NumericToAbstractFloatCaster<true> NumericToDoubleCaster;

   /* * no doxygen
    * @short Casts a @c numeric value, such as @c xs:double or @c xs:decimal, to @c xs:float.
    *
    * castFrom() uses Numeric::toFloat() for doing the actual casting.
    *
    * @ingroup Patternist_xdm
    * @author Frans Englich <fenglich@trolltech.com>
    */
    typedef NumericToAbstractFloatCaster<false> NumericToFloatCaster;

    /* * no doxygen
     * @short Casts a string value, @c xs:string or @c xs:untypedAtomic, to @c xs:double.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    typedef StringToAbstractFloatCaster<true> StringToDoubleCaster;

    /* * no doxygen
     * @short Casts a string value, @c xs:string or @c xs:untypedAtomic, to @c xs:float.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    typedef StringToAbstractFloatCaster<false> StringToFloatCaster;

   /* * no doxygen
    * @short Casts a value of type @c xs:boolean to @c xs:double.
    *
    * @ingroup Patternist_xdm
    * @author Frans Englich <fenglich@trolltech.com>
    */
    typedef BooleanToAbstractFloatCaster<true> BooleanToDoubleCaster;

    /* * no doxygen
     * @short Casts a value of type @c xs:boolean to @c xs:float.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    typedef BooleanToAbstractFloatCaster<false> BooleanToFloatCaster;
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
