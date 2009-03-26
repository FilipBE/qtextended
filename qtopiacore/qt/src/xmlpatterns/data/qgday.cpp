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

#include "qbuiltintypes_p.h"

#include "qgday_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

GDay::GDay(const QDateTime &dateTime) : AbstractDateTime(dateTime)
{
}

GDay::Ptr GDay::fromLexical(const QString &lexical)
{
    static const CaptureTable captureTable( // STATIC DATA
        /* The extra paranthesis is a build fix for GCC 3.3. */
        (QRegExp(QLatin1String(
                "^\\s*"                                 /* Any preceding whitespace. */
                "---"                                   /* Delimiter. */
                "(\\d{2})"                              /* The day part, "03". */
                "(?:(?:(\\+|-))(\\d{2}):(\\d{2})|(Z))?" /* Timezone, "+08:24". */
                "\\s*$"                                 /* Any whitespace at the end. */))),
        /*zoneOffsetSignP*/         2,
        /*zoneOffsetHourP*/         3,
        /*zoneOffsetMinuteP*/       4,
        /*zoneOffsetUTCSymbolP*/    5,
        /*yearP*/                   -1,
        /*monthP*/                  -1,
        /*dayP*/                    1);

    AtomicValue::Ptr err;
    const QDateTime retval(create(err, lexical, captureTable));

    return err ? err : GDay::Ptr(new GDay(retval));
}

GDay::Ptr GDay::fromDateTime(const QDateTime &dt)
{
    QDateTime result(QDate(DefaultYear, DefaultMonth, dt.date().day()));
    copyTimeSpec(dt, result);

    return GDay::Ptr(new GDay(result));
}

QString GDay::stringValue() const
{
    return m_dateTime.toString(QLatin1String("---dd")) + zoneOffsetToString();
}

ItemType::Ptr GDay::type() const
{
    return BuiltinTypes::xsGDay;
}

QT_END_NAMESPACE
