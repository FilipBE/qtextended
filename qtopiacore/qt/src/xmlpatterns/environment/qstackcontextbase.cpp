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

/**
 * @file
 * @short This file is included by qstackcontextbase_p.h.
 * If you need includes in this file, put them in qstackcontextbase_p.h, outside of the namespace.
 */

template<typename TSuperClass>
StackContextBase<TSuperClass>::StackContextBase() : m_rangeVariables(10),
                                                    m_expressionVariables(10),
                                                    m_positionIterators(5),
                                                    m_itemCacheCells(5),
                                                    m_itemSequenceCacheCells(5)
{
    /* The m_* containers are initialized with default sizes. Estimated guesses on usage patterns. */
}

template<typename TSuperClass>
StackContextBase<TSuperClass>::StackContextBase(const DynamicContext::Ptr &prevContext)
                                                : TSuperClass(prevContext),
                                                  m_rangeVariables(10),
                                                  m_expressionVariables(10),
                                                  m_positionIterators(5),
                                                  m_itemCacheCells(5),
                                                  m_itemSequenceCacheCells(5)
{
    Q_ASSERT(prevContext);
}

template<typename TSuperClass>
ItemCacheCell &StackContextBase<TSuperClass>::itemCacheCell(const VariableSlotID slot)
{
    if(slot >= m_itemCacheCells.size())
        m_itemCacheCells.resize(qMax(slot + 1, m_itemCacheCells.size()));

    return m_itemCacheCells[slot];
}

template<typename TSuperClass>
ItemSequenceCacheCell::Vector &StackContextBase<TSuperClass>::itemSequenceCacheCells(const VariableSlotID slot)
{
    if(slot >= m_itemSequenceCacheCells.size())
        m_itemSequenceCacheCells.resize(qMax(slot + 1, m_itemSequenceCacheCells.size()));

    return m_itemSequenceCacheCells;
}

template<typename TSuperClass>
Item StackContextBase<TSuperClass>::rangeVariable(const VariableSlotID slot) const
{
    Q_ASSERT(slot < m_rangeVariables.size());
    Q_ASSERT(m_rangeVariables.at(slot));
    return m_rangeVariables.at(slot);
}

template<typename TSuperClass>
Expression::Ptr StackContextBase<TSuperClass>::expressionVariable(const VariableSlotID slot) const
{
    Q_ASSERT(slot < m_expressionVariables.size());
    return m_expressionVariables.at(slot);
}

template<typename TSuperClass>
Item::Iterator::Ptr StackContextBase<TSuperClass>::positionIterator(const VariableSlotID slot) const
{
    Q_ASSERT(slot < m_positionIterators.size());
    return m_positionIterators.at(slot);
}

template<typename TSuperClass>
template<typename VectorType, typename UnitType>
inline
void StackContextBase<TSuperClass>::setSlotVariable(const VariableSlotID slot,
                                                    const UnitType &newValue,
                                                    VectorType &container) const
{
    if(slot < container.size())
        container.replace(slot, newValue);
    else
    {
        container.resize(slot + 1);
        container.replace(slot, newValue);
    }
}

template<typename TSuperClass>
void StackContextBase<TSuperClass>::setRangeVariable(const VariableSlotID slot,
                                                     const Item &newValue)
{
    setSlotVariable(slot, newValue, m_rangeVariables);
}

template<typename TSuperClass>
void StackContextBase<TSuperClass>::setExpressionVariable(const VariableSlotID slot,
                                                          const Expression::Ptr &newValue)
{
    setSlotVariable(slot, newValue, m_expressionVariables);
}

template<typename TSuperClass>
void StackContextBase<TSuperClass>::setPositionIterator(const VariableSlotID slot,
                                                        const Item::Iterator::Ptr &newValue)
{
    setSlotVariable(slot, newValue, m_positionIterators);
}

