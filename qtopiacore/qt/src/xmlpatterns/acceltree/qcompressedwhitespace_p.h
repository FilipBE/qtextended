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

#ifndef Patternist_CompressedWhitespace_H
#define Patternist_CompressedWhitespace_H

#include <QtGlobal>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QChar;
class QString;
class QStringRef;

namespace QPatternist
{
    /**
     * @short A compression facility for whitespace nodes.
     *
     * CompressedWhitespace compresses and decompresses strings that consists of
     * whitespace only, and do so with a scheme that is designed to do this
     * specialized task in an efficient way. The approach is simple: each
     * sequence of equal whitespace in the input gets coded into one byte,
     * where the first two bits signals the type, CharIdentifier, and the
     * remininding six bits is the count.
     *
     * For instance, this scheme manages to compress a sequence of spaces
     * followed by a new line into 16 bits(one QChar), and QString stores
     * strings of one QChar quite efficiently, by avoiding a heap allocation.
     *
     * There is no way to tell whether a QString is compressed or not.
     *
     * The compression scheme originates from Saxon, by Michael Kay.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class CompressedWhitespace
    {
        public:
            /**
             * @short Compresses @p input into a compressed format, returned
             * as a QString.
             *
             * The caller guarantees that input is not empty
             * and consists only of whitespace.
             *
             * The returned format is opaque. There is no way to find out
             * whether a QString contains compressed data or not.
             *
             * @see decompress()
             */
            static QString compress(const QStringRef &input);

            /**
             * @short Decompresses @p input into a usual QString.
             *
             * @p input must be a QString as per returned from compress().
             *
             * @see compress()
             */
            static QString decompress(const QString &input);

        private:
            /**
             * We use the two upper bits for communicating what space it is.
             */
            enum CharIdentifier
            {
                Space   = 0x0,

                /**
                 * 0xA, \\r
                 *
                 * Binary: 10000000
                 */
                CR      = 0x80,

                /**
                 * 0xD, \\n
                 *
                 * Binary: 01000000
                 */
                LF      = 0x40,

                /**
                 * Binary: 11000000
                 */
                Tab     = 0xC0
            };

            enum Constants
            {
                /* We can at maximum store this many consecutive characters
                 * of one type. We use 6 bits for the count. */
                MaxCharCount = (1 << 6) - 1,

                /**
                 * Binary: 11111111
                 */
                Lower8Bits = (1 << 8) - 1,

                /**
                 * Binary: 111111
                 */
                Lower6Bits = (1 << 6) - 1,

                /*
                 * Binary: 11000000
                 */
                UpperTwoBits = 3 << 6
            };

            static inline CharIdentifier toIdentifier(const QChar ch);

            static inline quint8 toCompressedChar(const QChar ch, const int len);
            static inline QChar toChar(const CharIdentifier id);

            /**
             * @short Returns @c true if @p number is an even number, otherwise
             * @c false.
             */
            static inline bool isEven(const int number);

            /**
             * @short This class can only be used via its static members.
             */
            inline CompressedWhitespace();
            Q_DISABLE_COPY(CompressedWhitespace)
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
