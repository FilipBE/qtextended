/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#include "qplatformdefs.h"

#include "qbytearray.h"
#include "qstring.h"

#include "string.h"

QT_BEGIN_NAMESPACE

#ifndef QT_VSNPRINTF

/*!
    \relates QByteArray

    A portable \c vsnprintf() function. Will call \c ::vsnprintf(), \c
    ::_vsnprintf(), or \c ::vsnprintf_s depending on the system, or
    fall back to an internal version.

    \a fmt is the \c printf() format string. The result is put into
    \a str, which is a buffer of at least \a n bytes.

    The caller is responsible to call \c va_end() on \a ap.

    \warning Since vsnprintf() shows different behavior on certain
    platforms, you should not rely on the return value or on the fact
    that you will always get a 0 terminated string back.

    Ideally, you should never call this function but use QString::sprintf()
    instead.

    \sa qsnprintf(), QString::sprintf()
*/

int qvsnprintf(char *str, size_t n, const char *fmt, va_list ap)
{
    if (!str || !fmt)
        return -1;

    QString buf;
    buf.vsprintf(fmt, ap);

    QByteArray ba = buf.toLocal8Bit();

    if (n > 0) {
        size_t blen = qMin(size_t(ba.length()), size_t(n - 1));
        memcpy(str, ba.constData(), blen);
        str[blen] = '\0'; // make sure str is always 0 terminated
    }

    return ba.length();
}

#else

QT_BEGIN_INCLUDE_NAMESPACE
#include <stdio.h>
QT_END_INCLUDE_NAMESPACE

int qvsnprintf(char *str, size_t n, const char *fmt, va_list ap)
{
    return QT_VSNPRINTF(str, n, fmt, ap);
}

#endif

/*!
    \relates QByteArray

    A portable snprintf() function, calls qvsnprintf.

    \a fmt is the \c printf() format string. The result is put into
    \a str, which is a buffer of at least \a n bytes.

    \warning Call this function only when you know what you are doing
    since it shows different behavior on certain platforms.
    Use QString::sprintf() to format a string instead.

    \sa qvsnprintf(), QString::sprintf()
*/

int qsnprintf(char *str, size_t n, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int ret = qvsnprintf(str, n, fmt, ap);
    va_end(ap);

    return ret;
}

QT_END_NAMESPACE
