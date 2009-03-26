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

#ifndef Patternist_CommonValues_H
#define Patternist_CommonValues_H

#include "qdaytimeduration_p.h"
#include "qyearmonthduration_p.h"
#include "qemptyiterator_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{

    /**
     * @short A collection of common values.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     * @todo Documentation is missing/incomplete
     */
    class CommonValues
    {
    public:
        /**
         * An empty, zero-length string.
         *
         * @note It is not @c null, but empty.
         */
        static const AtomicValue::Ptr EmptyString;

        /**
         * The string "true", the lexical representation of
         * @c xs:boolean's value @c true.
         */
        static const AtomicValue::Ptr TrueString;

        /**
         * The string "false", the lexical representation of
         * @c xs:boolean's value @c false.
         */
        static const AtomicValue::Ptr UntypedAtomicFalse;

        /**
         * The string "true", the lexical representation of
         * @c xs:boolean's value @c true.
         */
        static const AtomicValue::Ptr UntypedAtomicTrue;

        /**
         * The string "false", the lexical representation of
         * @c xs:boolean's value @c false.
         */
        static const AtomicValue::Ptr FalseString;

        /**
         * @returns a Boolean instance carrying the boolean value @c true.
         * Use this value instead of Boolean::fromValue() if you
         * know what boolean value you need.
         */
        static const AtomicValue::Ptr BooleanTrue;

        /**
         * @returns a Boolean instance carrying the boolean value @c true.
         * Use this value instead of Boolean::fromValue() if you
         * know what boolean value you need.
         */
        static const AtomicValue::Ptr BooleanFalse;

        /**
         * Not-a-Numeric typed as @c xs:double.
         */
        static const AtomicValue::Ptr DoubleNaN;

        /**
         * Not-a-Number typed as @c xs:float, <tt>xs:float("NaN")</tt>.
         */
        static const AtomicValue::Ptr FloatNaN;

        /**
         * Zero(0) typed as @c xs:integer, <tt>xs:integer("0")</tt>.
         */
        static const Item IntegerZero;

        /**
         * An empty, "", @c xs:anyURI.
         */
        static const AtomicValue::Ptr EmptyAnyURI;

        /**
         * @see emptySequence
         */
        static const EmptyIterator<Item>::Ptr emptyIterator;

        /**
         * <tt>xs:float("-INF")</tt>
         */
        static const AtomicValue::Ptr NegativeInfFloat;

        /**
         * <tt>xs:float("INF")</tt>
         */
        static const AtomicValue::Ptr InfFloat;

        /**
         * <tt>xs:double("-INF")</tt>
         */
        static const AtomicValue::Ptr NegativeInfDouble;

        /**
         * <tt>xs:double("INF")</tt>
         */
        static const AtomicValue::Ptr InfDouble;

        /**
         * <tt>xs:float("1")</tt>
         */
        static const AtomicValue::Ptr FloatOne;
        /**
         * <tt>xs:double("1")</tt>
         */
        static const AtomicValue::Ptr DoubleOne;
        /**
         * <tt>xs:decimal("1")</tt>
         */
        static const AtomicValue::Ptr DecimalOne;

        /**
         * <tt>xs:integer("1")</tt>
         */
        static const Item IntegerOne;

        /**
         * <tt>xs:integer("-1")</tt>
         */
        static const Item IntegerOneNegative;

        /**
         * <tt>xs:double("0")</tt>
         */
        static const AtomicValue::Ptr DoubleZero;

        /**
         * <tt>xs:float("0")</tt>
         */
        static const AtomicValue::Ptr FloatZero;
        /**
         * <tt>xs:integer("0")</tt>
         */
        static const AtomicValue::Ptr DecimalZero;

        /**
         * The @c xs:dayTimeDuration value PT0S
         */
        static const DayTimeDuration::Ptr DayTimeDurationZero;

        /**
         * The @c xs:yearMonthDuration value P0M
         */
        static const DayTimeDuration::Ptr YearMonthDurationZero;

    private:
        /**
         * The constructor is private because this class is not meant to be instantiated,
         * but should only be used via its static const members.
         */
        inline CommonValues();

        Q_DISABLE_COPY(CommonValues)
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
