/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef QCIRCULARBUFFER_P_H
#define QCIRCULARBUFFER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QVector>

template <typename T>
struct QCircularBuffer
{
    QCircularBuffer(int = 1000);

    void clear();

    void append(T const&);
    T const& at(int) const;
    int  count() const;

    int  limit() const;
    void setLimit(int);

private:
    int        m_beginIndex;
    int        m_limit;
    QVector<T> m_vector;
};

template <typename T>
inline
QCircularBuffer<T>::QCircularBuffer(int limit)
    : m_beginIndex(0)
    , m_limit(limit)
    , m_vector()
{}

template <typename T>
inline
int QCircularBuffer<T>::count() const
{ return m_vector.count(); }

template <typename T>
inline
void QCircularBuffer<T>::clear()
{
    m_vector.clear();
    m_beginIndex = 0;
}

template <typename T>
inline
void QCircularBuffer<T>::append(T const& value)
{
    if (m_limit <= 0 || m_vector.count() < m_limit)
        m_vector.append(value);
    else {
        m_vector[m_beginIndex] = value;
        m_beginIndex = (m_beginIndex+1) % m_limit;
    }
}

template <typename T>
inline
T const& QCircularBuffer<T>::at(int i) const
{
    Q_ASSERT(i < count());
    if (m_limit <= 0 || m_vector.count() < m_limit)
        return m_vector.at(i);
    else
        return m_vector.at((i+m_beginIndex) % m_limit);
}

template <typename T>
inline
int  QCircularBuffer<T>::limit() const
{ return m_limit; }

// FIXME handle decreasing the limit
template <typename T>
inline
void QCircularBuffer<T>::setLimit(int limit)
{ m_limit = limit; }

#endif

