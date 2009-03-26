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

#ifndef Patternist_CppCastingHelper_H
#define Patternist_CppCastingHelper_H

#include <QtCore/QtGlobal>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Provides convenience methods for performing static casts between C++ classes.
     *
     * In Patternist, it is very common to do up-casts from Expression or Item, which typically
     * involves writing messy code. Such an old-way cast looks like this:
     *
     * @code
     * static_cast<const MyClass *>(myInstance.data())->myClassMember()
     * @endcode
     *
     * CppCastingHelper provides the convenience method as() for this, which is functionally
     * equivalent to the above code, but simpler:
     *
     * @code
     * myInstance->as<MyClass>()->myClassMember()
     * @endcode
     *
     * The as() function performs a static cast.
     *
     * By using CppCastingHelper, this is achieved:
     *
     * - Const correctness is automatically taken care of
     * - Less code to write
     * - When compiling in debug mode, the as() functions uses a @c dynamic_cast to verify that the
     *   static casts are properly done, such that sensible error messages are given when the casts
     *   are invalid. It also traps invalid casts which nevertheless happen to work on a particular
     *   platform/compiler/hardware architecture.
     *
     * CppCastingHelper is a template class where the TSubClass parameter must be the class
     * inheriting CppCastingHelper. See Item or Expression for demonstration.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<typename TSubClass>
    class CppCastingHelper
    {
    public:

        /**
         * Casts this instance to:
         *
         * @code
         * const TCastTarget *
         * @endcode
         *
         * and returns the result.
         *
         * When compiled in debug mode, this function perform a @c dynamic_cast, in order to
         * check the correctness of the cast.
         */
        template<typename TCastTarget>
        inline const TCastTarget *as() const
        {
#ifndef Q_CC_XLC
/* At least on aix-xlc-64, the compiler cries when it sees dynamic_cast. */
            Q_ASSERT_X(dynamic_cast<const TCastTarget *>(static_cast<const TSubClass *>(this)),
                       Q_FUNC_INFO,
                       "The cast is invalid. This class does not inherit the cast target.");
#endif
            return static_cast<const TCastTarget *>(static_cast<const TSubClass *>(this));
        }

        /**
         * Casts this instance to:
         *
         * @code
         * TCastTarget *
         * @endcode
         *
         * and returns the result.
         *
         * When compiled in debug mode, a @c dynamic_cast is attempted, in order to
         * check the correctness of the cast.
         */
        template<typename TCastTarget>
        inline TCastTarget *as()
        {
#ifndef Q_CC_XLC
/* At least on aix-xlc-64, the compiler cries when it sees dynamic_cast. */
            Q_ASSERT_X(dynamic_cast<TCastTarget *>(static_cast<TSubClass *>(this)),
                       Q_FUNC_INFO,
                       "The cast is invalid. This class does not inherit the cast target.");
#endif
            return static_cast<TCastTarget *>(static_cast<TSubClass *>(this));
        }

    protected:
        /**
         * This constructor is protected because this class must be sub-classed.
         */
        inline CppCastingHelper() {}
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
