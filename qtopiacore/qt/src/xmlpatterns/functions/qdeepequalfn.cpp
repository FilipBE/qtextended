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
#include "qcommonsequencetypes_p.h"
#include "qcommonvalues_p.h"
#include "qliteral_p.h"
#include "qnumeric_p.h"

#include "qdeepequalfn_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

bool DeepEqualFN::evaluateEBV(const DynamicContext::Ptr &context) const
{
    const Item::Iterator::Ptr it1(m_operands.first()->evaluateSequence(context));
    const Item::Iterator::Ptr it2(m_operands.at(1)->evaluateSequence(context));

    while(true)
    {
        const Item item1(it1->next());
        const Item item2(it2->next());

        if(!item1)
        {
            if(item2)
                return false;
            else
                return true;
        }
        else if(!item2)
        {
            if(item1)
                return false;
            else
                return true;
        }
        else if(item1.isNode())
        {
            if(item2.isNode())
            {
                if(item1.asNode().isDeepEqual(item2.asNode()))
                    continue;
                else
                    return false;
            }
            else
                return false;
        }
        else if(item2.isNode())
        {
            /* We know that item1 is not a node due to the check above. */
            return false;
        }
        else if(flexibleCompare(item1, item2, context))
            continue;
        else if(BuiltinTypes::numeric->itemMatches(item1) &&
                item1.as<Numeric>()->isNaN() &&
                item2.as<Numeric>()->isNaN())
        {
            // TODO
            /* Handle the specific NaN circumstances. item2 isn't checked whether it's of
             * type numeric, since the AtomicComparator lookup would have failed if both weren't
             * numeric. */
            continue;
        }
        else
            return false;
    };
}

Expression::Ptr DeepEqualFN::typeCheck(const StaticContext::Ptr &context,
                                       const SequenceType::Ptr &reqType)
{
    const Expression::Ptr me(FunctionCall::typeCheck(context, reqType));
    const ItemType::Ptr t1(m_operands.first()->staticType()->itemType());
    const ItemType::Ptr t2(m_operands.at(1)->staticType()->itemType());
    /* TODO This can be much more improved, and the optimizations should be moved
     * to compress(). */

    if(*CommonSequenceTypes::Empty == *t1)
    {
        if(*CommonSequenceTypes::Empty == *t2)
            return wrapLiteral(CommonValues::BooleanTrue, context, this);
        else
            return me;
    }
    else if(*CommonSequenceTypes::Empty == *t2)
    {
        if(*CommonSequenceTypes::Empty == *t1)
            return wrapLiteral(CommonValues::BooleanTrue, context, this);
        else
            return me;
    }
    else if(BuiltinTypes::node->xdtTypeMatches(t1) &&
            BuiltinTypes::node->xdtTypeMatches(t2))
        return me; /* We're comparing nodes. */
    else if(BuiltinTypes::xsAnyAtomicType->xdtTypeMatches(t1) &&
            BuiltinTypes::xsAnyAtomicType->xdtTypeMatches(t2))
    {
        prepareComparison(fetchComparator(t1, t2, context));
        return me;
    }
    else
    {
        if ((BuiltinTypes::node->xdtTypeMatches(t1) && BuiltinTypes::xsAnyAtomicType->xdtTypeMatches(t2))
            || (BuiltinTypes::node->xdtTypeMatches(t2) && BuiltinTypes::xsAnyAtomicType->xdtTypeMatches(t1)))
        {
            /* One operand contains nodes and the other atomic values, or vice versa. They can never
             * be identical. */
        // TODO warn?
            return wrapLiteral(CommonValues::BooleanFalse, context, this);
        }
        else
        {
            // TODO Warn?
            return me;
        }
    }
}

QT_END_NAMESPACE
