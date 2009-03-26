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

#ifndef Patternist_GenericSequenceType_H
#define Patternist_GenericSequenceType_H

#include "qsequencetype_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @todo Documentation is missing.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class GenericSequenceType : public SequenceType
    {
    public:
        GenericSequenceType(const ItemType::Ptr &itemType, const Cardinality &card);

        /**
         * Generates a name for the sequence type for display purposes. The
         * prefix used for the QName identifying the schema type is conventional.
         * An example of a display name for a GenericSequenceType is "xs:integer?".
         */
        virtual QString displayName(const NamePool::Ptr &np) const;

        virtual Cardinality cardinality() const;

        virtual ItemType::Ptr itemType() const;

    private:
        const ItemType::Ptr m_itemType;
        const Cardinality m_cardinality;
    };

    /**
     * @short An object generator for GenericSequenceType.
     *
     * makeGenericSequenceType() is a convenience function for avoiding invoking
     * the @c new operator, and wrapping the result in GenericSequenceType::Ptr.
     *
     * @returns a smart pointer to to a GenericSequenceType instaniated from @p itemType and @p cardinality.
     * @relates GenericSequenceType
     */
    static inline SequenceType::Ptr
    makeGenericSequenceType(const ItemType::Ptr &itemType, const Cardinality &cardinality)
    {
        return SequenceType::Ptr(new GenericSequenceType(itemType, cardinality));
    }
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
