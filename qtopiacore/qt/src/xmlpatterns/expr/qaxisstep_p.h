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

#ifndef Patternist_AxisStep_H
#define Patternist_AxisStep_H

#include "qitem_p.h"
#include "qemptycontainer_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short A step in a path expression that with an axis and a node test evaluates
     * to a sequence of nodes from the context item.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class Q_AUTOTEST_EXPORT AxisStep : public EmptyContainer
    {
    public:
        AxisStep(const QXmlNodeModelIndex::Axis axis,
                 const ItemType::Ptr &nodeTest);

        virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const;
        virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;

        /**
         * Returns @p node if it matches the node test this step is using, otherwise @c null.
         */
        inline Item mapToItem(const QXmlNodeModelIndex &node,
                              const DynamicContext::Ptr &context) const;

        virtual SequenceType::List expectedOperandTypes() const;
        virtual SequenceType::Ptr staticType() const;

        /**
         * Rewrites to ParentNodeAxis, if possible.
         */
        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

        /**
         * @returns always BuiltinTypes::node;
         */
        virtual ItemType::Ptr expectedContextItemType() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        virtual Properties properties() const;

        /**
         * @returns the axis this step is using.
         */
        QXmlNodeModelIndex::Axis axis() const;

        /**
         * @returns the node test this step is using.
         */
        inline ItemType::Ptr nodeTest() const
        {
            return m_nodeTest;
        }

        /**
         * @short Prints the EBNF name corresponding to @p axis.
         *
         * For instance, for QXmlNodeModelIndex::Child, "child" is returned.
         *
         * Apart from being used in this class, it is used in the SDK.
         */
        static QString axisName(const QXmlNodeModelIndex::Axis axis);

    private:
        typedef QExplicitlySharedDataPointer<const AxisStep> ConstPtr;

        static const QXmlNodeModelIndex::NodeKind s_whenAxisNodeKindEmpty[];

        /**
         * @returns @c true when the axis @p axis and a node test testing node of
         * type @p nodeKind always produces an empty sequence. One such example
         * is <tt>attribute::comment()</tt>.
         */
        static bool isAlwaysEmpty(const QXmlNodeModelIndex::Axis axis, const QXmlNodeModelIndex::NodeKind nodeKind);

        const QXmlNodeModelIndex::Axis  m_axis;
        const ItemType::Ptr             m_nodeTest;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
