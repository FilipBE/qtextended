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

#ifndef Patternist_Base64Binary_H
#define Patternist_Base64Binary_H

#include "qitem_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{

    /**
     * @short Implements the value instance of the @c xs:base64Binary type.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     */
    class Base64Binary : public AtomicValue
    {
    public:
        friend class CommonValues;

        typedef AtomicValue::Ptr Ptr;

        /**
         * Creates an instance representing @p value.
         */
        static AtomicValue::Ptr fromLexical(const QString &value);

        static Base64Binary::Ptr fromValue(const QByteArray &data);

        virtual QString stringValue() const;
        virtual ItemType::Ptr type() const;
        inline const QByteArray &asByteArray() const
        {
            return m_value;
        }

    protected:
        Base64Binary(const QByteArray &val);

        const QByteArray m_value;

    private:
        /**
         * @short Assumes @p in is a lexical representation of @c xs:base64Binary, and
         * converts it to the binary data set in @p out.
         *
         * If @p instr is invalid Base64 content, @p ok is set to
         * false, and the returned QByteArray has an undefined value.
         *
         *  We cannot use QByteArray::fromBase64() because it doesn't do the
         *  necessary validation that we need to properly implement W3C XML
         *  Schema's xs:base64Binary type.
         */
        static void base64Decode(const QByteArray &in, QByteArray &out, bool &ok);

        static const char Base64DecMap[128];
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
