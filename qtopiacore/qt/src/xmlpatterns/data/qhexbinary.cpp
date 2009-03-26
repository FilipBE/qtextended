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

#include <QtGlobal>

#include "qbase64binary_p.h"
#include "qbuiltintypes_p.h"
#include "qpatternistlocale_p.h"
#include "qvalidationerror_p.h"

#include "qhexbinary_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

HexBinary::HexBinary(const QByteArray &val) : Base64Binary(val)
{
}

qint8 HexBinary::fromHex(const QChar &c)
{
    // TODO static data
    static const QString g_fromHex(QString::fromLatin1("0123456789ABCDEFabcdef"));

    int digit = g_fromHex.indexOf(c);

    if(digit > 15)
        digit -= 6;

    return digit; /* may be negative if no match is found. */
}

AtomicValue::Ptr HexBinary::fromLexical(const NamePool::Ptr &np, const QString &str)
{
    const QString lexical(str.trimmed());
    const int len = lexical.length();

    if(len == 0)
        return AtomicValue::Ptr(new HexBinary(QByteArray()));

    if((len & 1) != 0)
    {
        /* Catch a common case. */
        return ValidationError::createError(QtXmlPatterns::tr(
                  "A value of type %1 must contain an even number of "
                  "digits. The value %2 does not.")
                  .arg(formatType(np, BuiltinTypes::xsHexBinary),
                       formatData(QString::number(len))));
    }

    QByteArray val;
    val.resize(len / 2);

    for(int i = 0; i < len / 2; ++i)
    {
        qint8 p1 = fromHex(lexical[i * 2]);
        qint8 p2 = fromHex(lexical[i * 2 + 1]);

        if(p1 == -1 || p2 == -1)
        {
            const QString hex(QString::fromLatin1("%1%2").arg(lexical[i * 2], lexical[i * 2 + 1]));

            return ValidationError::createError(QtXmlPatterns::tr(
                             "%1 is not valid as a value of type %2.")
                             .arg(formatData(hex),
                                  formatType(np, BuiltinTypes::xsHexBinary)));
        }

        val[i] = static_cast<char>(p1 * 16 + p2);
    }
    Q_ASSERT(!val.isEmpty());

    return AtomicValue::Ptr(new HexBinary(val));
}

HexBinary::Ptr HexBinary::fromValue(const QByteArray &data)
{
    return HexBinary::Ptr(new HexBinary(data));
}

QString HexBinary::stringValue() const
{
    static const char s_toHex[] = "0123456789ABCDEF";
    const int len = m_value.count();
    QString result;
    result.reserve(len * 2);

    for(int i = 0; i < len; ++i)
    {
        // This cast is significant.
        const unsigned char val = static_cast<unsigned char>(m_value.at(i));
        result += QLatin1Char(s_toHex[val >> 4]);
        result += QLatin1Char(s_toHex[val & 0x0F]);
    }

    return result;
}

ItemType::Ptr HexBinary::type() const
{
    return BuiltinTypes::xsHexBinary;
}

QT_END_NAMESPACE
