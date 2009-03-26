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

#include "qabstractduration_p.h"
#include "qabstractdatetime_p.h"
#include "qbase64binary_p.h"
#include "qboolean_p.h"
#include "qdynamiccontext_p.h"
#include "qqnamevalue_p.h"

#include "qatomiccomparators_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

/* -------------------------------------------------- */
AtomicComparator::ComparisonResult
StringComparator::compare(const Item &o1,
                          const AtomicComparator::Operator,
                          const Item &o2) const
{
    const int result = QString::compare(o1.stringValue(), o2.stringValue());

    if(result > 0)
        return GreaterThan;
    else if(result < 0)
        return LessThan;
    else
    {
        Q_ASSERT(result == 0);
        return Equal;
    }
}

bool StringComparator::equals(const Item &o1,
                              const Item &o2) const
{
    return o1.stringValue() == o2.stringValue();
}
/* -------------------------------------------------- */

/* -------------------------------------------------- */
AtomicComparator::ComparisonResult
CaseInsensitiveStringComparator::compare(const Item &o1,
                                           const AtomicComparator::Operator,
                                           const Item &o2) const
{
    const QString i1(o1.stringValue().toLower());
    const QString i2(o2.stringValue().toLower());
    const int result = QString::compare(i1, i2);

    if(result > 0)
        return GreaterThan;
    else if(result < 0)
        return LessThan;
    else
    {
        Q_ASSERT(result == 0);
        return Equal;
    }
}

bool CaseInsensitiveStringComparator::equals(const Item &o1,
                                             const Item &o2) const
{
    const QString s1(o1.stringValue());
    const QString s2(o2.stringValue());

    return s1.length() == s2.length() &&
           s1.startsWith(s2, Qt::CaseInsensitive);
}
/* -------------------------------------------------- */

/* -------------------------------------------------- */
bool BinaryDataComparator::equals(const Item &o1,
                                  const Item &o2) const
{
    return o1.as<Base64Binary>()->asByteArray() ==
           o2.as<Base64Binary>()->asByteArray();
}
/* -------------------------------------------------- */

/* -------------------------------------------------- */
AtomicComparator::ComparisonResult
BooleanComparator::compare(const Item &o1,
                           const AtomicComparator::Operator,
                           const Item &o2) const
{
    /* We know Boolean::evaluateEBV doesn't use the DynamicContext. */
    const bool v1 = o1.as<AtomicValue>()->evaluateEBV(QExplicitlySharedDataPointer<DynamicContext>());
    const bool v2 = o2.as<AtomicValue>()->evaluateEBV(QExplicitlySharedDataPointer<DynamicContext>());

    if(v1 == v2)
        return Equal;
    else if(v1 == false)
    {
        Q_ASSERT(v2 == true);
        return LessThan;
    }
    else
    {
        Q_ASSERT(v1 == true && v2 == false);
        return GreaterThan;
    }
}

bool BooleanComparator::equals(const Item &o1,
                               const Item &o2) const
{
    /* Boolean is an atomic class. */
    return o1.as<AtomicValue>() == o2.as<AtomicValue>();
}
/* -------------------------------------------------- */

/* -------------------------------------------------- */
AtomicComparator::ComparisonResult
AbstractFloatComparator::compare(const Item &o1,
                                 const AtomicComparator::Operator op,
                                 const Item &o2) const
{
    const xsDouble v1 = o1.as<Numeric>()->toDouble();
    const xsDouble v2 = o2.as<Numeric>()->toDouble();

    if(Double::isEqual(v1, v2))
        return Equal;
    else if(v1 < v2)
        return LessThan;
    else if(v1 > v2)
        return GreaterThan;
    else
    {
        /* We have NaN values. Make sure we don't return a result which would
         * signify success for the operator in question. */
        if((op & OperatorGreaterThan) == OperatorGreaterThan)
            return LessThan;
        else
        {
            Q_ASSERT((op & OperatorLessThan) == OperatorLessThan);
            return GreaterThan;
        }
    }
}

bool AbstractFloatComparator::equals(const Item &o1,
                                     const Item &o2) const
{
    return Double::isEqual(o1.as<Numeric>()->toDouble(), o2.as<Numeric>()->toDouble());
}
/* -------------------------------------------------- */

/* -------------------------------------------------- */
AtomicComparator::ComparisonResult
DecimalComparator::compare(const Item &o1,
                           const AtomicComparator::Operator,
                           const Item &o2) const
{
    const xsDecimal v1 = o1.as<Numeric>()->toDecimal();
    const xsDecimal v2 = o2.as<Numeric>()->toDecimal();

    if(Double::isEqual(v1, v2))
        return Equal;
    else if(v1 < v2)
        return LessThan;
    else
        return GreaterThan;
}

bool DecimalComparator::equals(const Item &o1,
                               const Item &o2) const
{
    return Double::isEqual(o1.as<Numeric>()->toDecimal(), o2.as<Numeric>()->toDecimal());
}
/* -------------------------------------------------- */

/* -------------------------------------------------- */
AtomicComparator::ComparisonResult
IntegerComparator::compare(const Item &o1,
                           const AtomicComparator::Operator,
                           const Item &o2) const
{
    const xsInteger v1 = o1.as<Numeric>()->toInteger();
    const xsInteger v2 = o2.as<Numeric>()->toInteger();

    if(v1 == v2)
        return Equal;
    else if(v1 < v2)
        return LessThan;
    else
        return GreaterThan;
}

bool IntegerComparator::equals(const Item &o1,
                               const Item &o2) const
{
    return o1.as<Numeric>()->toInteger() == o2.as<Numeric>()->toInteger();
}

/* -------------------------------------------------- */

/* -------------------------------------------------- */
bool QNameComparator::equals(const Item &o1,
                             const Item &o2) const
{
    return o1.as<QNameValue>()->m_qName ==
           o2.as<QNameValue>()->m_qName;
}
/* -------------------------------------------------- */

/* -------------------------------------------------- */
bool AbstractDateTimeComparator::equals(const Item &o1,
                                        const Item &o2) const
{
    const QDateTime dt1(o1.as<AbstractDateTime>()->toDateTime());
    const QDateTime dt2(o2.as<AbstractDateTime>()->toDateTime());

    /*
    pDebug() << "COMPARING:"
        << o1->as<AbstractDateTime>()->toDateTime().toString()
           << o2->as<AbstractDateTime>()->toDateTime().toString();
    pDebug() << "DATE ONLY:"
        << o1->as<AbstractDateTime>()->toDateTime().isDateOnly()
           << o2->as<AbstractDateTime>()->toDateTime().isDateOnly();
           */
    return dt1 == dt2 &&
           dt1.timeSpec() == dt2.timeSpec();
}

AtomicComparator::ComparisonResult
AbstractDateTimeComparator::compare(const Item &operand1,
                                    const AtomicComparator::Operator,
                                    const Item &operand2) const
{
    const QDateTime &dt1 = operand1.as<AbstractDateTime>()->toDateTime();
    const QDateTime &dt2 = operand2.as<AbstractDateTime>()->toDateTime();

    if(dt1 == dt2)
        return Equal;
    else if(dt1 < dt2)
        return LessThan;
    else
        return GreaterThan;
}
/* -------------------------------------------------- */

/* -------------------------------------------------- */
bool AbstractDurationComparator::equals(const Item &o1,
                                        const Item &o2) const
{
    /* We use AbstractDuration::operator==() */
    return *o1.as<AbstractDuration>() ==
           *o2.as<AbstractDuration>();
}

AtomicComparator::ComparisonResult
AbstractDurationComparator::compare(const Item &o1,
                                    const AtomicComparator::Operator,
                                    const Item &o2) const
{
    const AbstractDuration::Value val1 = o1.as<AbstractDuration>()->value();
    const AbstractDuration::Value val2 = o2.as<AbstractDuration>()->value();

    if(val1 > val2)
        return GreaterThan;
    else if(val1 < val2)
        return LessThan;
    else
        return Equal;
}

/* -------------------------------------------------- */
QT_END_NAMESPACE
