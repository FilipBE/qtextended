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

#ifndef Patternist_EmptySequence_H
#define Patternist_EmptySequence_H

#include "qemptycontainer_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Implements the value instance of empty sequence: <tt>()</tt>.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     */
    class EmptySequence : public EmptyContainer
    {
    public:
        /**
         * @short Creates an EmptySequence that is a replacement for @p
         * replacementFor.
         *
         * @see EmptySequence()
         */
        static Expression::Ptr create(const Expression *const replacementFor,
                                      const StaticContext::Ptr &context);


        /**
         * @short Creates an instance of EmptySequence.
         *
         * @note In most cases create() should be used, since it takes care of
         * adjusting source location annotations.
         *
         * @see create()
         */
        inline EmptySequence()
        {
        }

        virtual QString stringValue() const;

        /**
         * @returns always an empty iterator, an instance of EmptyIterator.
         */
        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const;

        /**
         * @returns always @c null.
         */
        virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;

        /**
         * Does nothing.
         */
        virtual void evaluateToSequenceReceiver(const DynamicContext::Ptr &) const;

        /**
         * @returns always @c false.
         */
        virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;

        /**
         * @returns always CommonSequenceTypes::Empty
         */
        virtual ItemType::Ptr type() const;

        /**
         * @returns always CommonSequenceTypes::Empty
         */
        virtual SequenceType::Ptr staticType() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
        virtual ID id() const;
        virtual Properties properties() const;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
