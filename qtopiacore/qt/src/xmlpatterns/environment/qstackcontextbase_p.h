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

#ifndef Patternist_StackContextBase_H
#define Patternist_StackContextBase_H

#include <QVector>

#include "qdaytimeduration_p.h"
#include "qdelegatingdynamiccontext_p.h"
#include "qexpression_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Base class for all DynamicContext classes that needs to
     * supply variables. It has a new frame for local caches, position iterators,
     * expressions, range variables but notably continues to delegate global caches.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<typename TSuperClass>
    class StackContextBase : public TSuperClass
    {
    public:
        StackContextBase();
        /**
         * Construct a StackContextBase and passes @p prevContext to its super class. This
         * constructor is typically used when the super class is DelegatingDynamicContext.
         */
        StackContextBase(const DynamicContext::Ptr &prevContext);

        virtual void setRangeVariable(const VariableSlotID slotNumber,
                                      const Item &newValue);
        virtual Item rangeVariable(const VariableSlotID slotNumber) const;

        virtual void setExpressionVariable(const VariableSlotID slotNumber,
                                           const Expression::Ptr &newValue);
        virtual Expression::Ptr expressionVariable(const VariableSlotID slotNumber) const;

        virtual Item::Iterator::Ptr positionIterator(const VariableSlotID slot) const;
        virtual void setPositionIterator(const VariableSlotID slot,
                                         const Item::Iterator::Ptr &newValue);
        virtual ItemCacheCell &itemCacheCell(const VariableSlotID slot);
        virtual ItemSequenceCacheCell::Vector &itemSequenceCacheCells(const VariableSlotID slot);

    protected:
        /**
         * This function is protected, although it only is used in this class. I don't
         * know why it has to be, but it won't compile when private.
         */
        template<typename VectorType, typename UnitType>
        inline
        void setSlotVariable(const VariableSlotID slot,
                             const UnitType &newValue,
                             VectorType &container) const;

    private:
        Item::Vector                    m_rangeVariables;
        Expression::Vector              m_expressionVariables;
        Item::Iterator::Vector          m_positionIterators;
        ItemCacheCell::Vector           m_itemCacheCells;
        ItemSequenceCacheCell::Vector   m_itemSequenceCacheCells;
    };

    #include "qstackcontextbase.cpp"

    /**
     * @short A DynamicContext that creates a new scope for variables.
     *
     * This DynamicContext is used for recursive user function calls, for example.
     */
    typedef StackContextBase<DelegatingDynamicContext> StackContext;
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
