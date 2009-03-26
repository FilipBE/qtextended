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

#ifndef Patternist_Debug_H
#define Patternist_Debug_H

#include <QtDebug>
/**
 * @file
 * @short Contains macros for debugging.
 */

QT_BEGIN_NAMESPACE

/**
 * @short Enables detailed parser debug output.
 *
 * If this macro is defined, @em a @em lot of debugging information will be outputted.
 * This is all the state transitions, token shifting, and rule reductions that
 * the parser do.
 *
 * This is automatically disabled if @c QT_NO_DEBUG is defined.
 */
#define Patternist_DEBUG_PARSER

/**
 * @short Enables debug printing statements.
 *
 * Patternist does not use qDebug(), but pDebug() instead. It only output
 * if this define is defined.
 *
 * It is automatically disabled if @c QT_NO_DEBUG is defined.
 */
#define Patternist_DEBUG

#undef Patternist_DEBUG_PARSER // disable it for now
#undef Patternist_DEBUG // disable it for now

#ifdef QT_NO_DEBUG
#   undef Patternist_DEBUG_PARSER
#   undef Patternist_DEBUG
#endif

/**
 * @def Patternist_TOKENIZER_MSG(msg)
 * @short Prints a debug message, used in tokenizer code.
 *
 * Used in tokenizer code and prints the debug message @p msg to @c stderr. Valid
 * types of @p msg is what pDebug() accepts.
 *
 * A no-op is generated if @c QT_NO_DEBUG is defined.
 */

#ifdef Patternist_DEBUG_PARSER
#else
#   define Patternist_TOKENIZER_MSG(msg) do{}while(0)
#endif

namespace QPatternist
{
#ifdef Patternist_DEBUG
    inline QDebug pDebug()
    {
        return qDebug();
    }
#else
    /**
     * @short Provides a receiver for pDebug when Patternist_DEBUG is not defined.
     *
     * Copied from src/corelib/io/qdebug.h.
     */
    class NoDebug
    {
    public:
        inline NoDebug() {}
        inline NoDebug(const QDebug &){}
        inline ~NoDebug(){}
#if !defined( QT_NO_TEXTSTREAM )
        inline NoDebug &operator<<(QTextStreamFunction) { return *this; }
        inline NoDebug &operator<<(QTextStreamManipulator) { return *this; }
#endif
        inline NoDebug &space() { return *this; }
        inline NoDebug &nospace() { return *this; }
        inline NoDebug &maybeSpace() { return *this; }

#ifndef QT_NO_MEMBER_TEMPLATES
        template<typename T>
        inline NoDebug &operator<<(const T &) { return *this; }
#endif
    };

    inline NoDebug pDebug()
    {
        return NoDebug();
    }
#endif
}

QT_END_NAMESPACE
#endif
