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

#ifndef Patternist_PatternPlatform_H
#define Patternist_PatternPlatform_H

#include <QFlags>
#include <QRegExp>

#include "qfunctioncall_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Contains functionality for functions and expressions that
     * uses regular expressions.
     *
     * @ingroup Patternist_utils
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class PatternPlatform : public FunctionCall
    {
    public:
        /**
         * @see <a href="http://www.w3.org/TR/xpath-functions/#flags">XQuery 1.0 and
         * XPath 2.0 Functions and Operators, 7.6.1.1 Flags</a>
         */
        enum Flag
        {
            /**
             * No flags are set. Default behavior is used.
             */
            NoFlags             = 0,

            /**
             * Flag @c s
             */
            DotAllMode          = 1,

            /**
             * Flag @c m
             */
            MultiLineMode       = 2,

            /**
             * Flag @c i
             */
            CaseInsensitive     = 4,

            /**
             * Flag @c x
             */
            SimplifyWhitespace  = 8
        };
        typedef QFlags<Flag> Flags;

        /**
         * @short This constructor is protected, because this class is supposed to be sub-classed.
         *
         * @param flagsPosition an index position specifying the operand containing the pattern
         * flags.
         */
        PatternPlatform(const qint8 flagsPosition);

        virtual Expression::Ptr compress(const StaticContext::Ptr &context);

        /**
         * Retrieves the pattern supplied in the arguments, taking care of compiling it,
         * settings its flags, and everything else required for getting it ready to use. If an error
         * occurs, an appropriate error is raised via @p context.
         */
        const QRegExp pattern(const DynamicContext::Ptr &context) const;

        /**
         * @returns the number of captures, also called parenthesized sub-expressions, the pattern has.
         *
         * If the pattern isn't precompiled, -1 is returned.
         */
        inline int captureCount() const;

    private:
        /**
         * Enum telling whether the flags, pattern, or both
         * have been compiled at compile time.
         */
        enum PreCompiledPart
        {
            NoPart          = 0,
            PatternPrecompiled     = 1,
            FlagsPrecompiled       = 2,
            FlagsAndPattern = PatternPrecompiled | FlagsPrecompiled

        };
        typedef QFlags<PreCompiledPart> PreCompiledParts;

        Q_DISABLE_COPY(PatternPlatform)

        Flags parseFlags(const QString &flags,
                         const DynamicContext::Ptr &context) const;

        QRegExp parsePattern(const QString &pattern,
                             const DynamicContext::Ptr &context) const;

        static void applyFlags(const Flags flags, QRegExp &pattern);

        /**
         * The parts that have been pre-compiled at compile time.
         */
        PreCompiledParts    m_compiledParts;
        Flags               m_flags;
        QRegExp             m_pattern;
        const qint8         m_flagsPosition;
    };

    inline int PatternPlatform::captureCount() const
    {
        if(m_compiledParts.testFlag(PatternPrecompiled))
            return m_pattern.numCaptures();
        else
            return -1;
    }

    Q_DECLARE_OPERATORS_FOR_FLAGS(PatternPlatform::Flags)
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
