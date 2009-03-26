/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt3Support module of the Qt Toolkit.
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

#ifndef Q3VALUEVECTOR_H
#define Q3VALUEVECTOR_H

#include <QtCore/qvector.h>

#ifndef QT_NO_STL
#include <vector>
#endif

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Qt3SupportLight)

template <typename T>
class Q3ValueVector : public QVector<T>
{
public:
    inline Q3ValueVector() : QVector<T>() {}
    inline Q3ValueVector(const Q3ValueVector<T>& v) : QVector<T>(v) {}
    inline Q3ValueVector(typename QVector<T>::size_type n,
                         const T& val = T()) : QVector<T>(n, val) {}

#ifndef QT_NO_STL
    inline Q3ValueVector(const std::vector<T>& v) : QVector<T>()
        { this->resize(v.size()); qCopy(v.begin(), v.end(), this->begin()); }
#endif

    Q3ValueVector<T>& operator= (const Q3ValueVector<T>& v)
        { QVector<T>::operator=(v); return *this; }

#ifndef QT_NO_STL
    Q3ValueVector<T>& operator= (const std::vector<T>& v)
    {
        this->clear();
        this->resize(v.size());
        qCopy(v.begin(), v.end(), this->begin());
        return *this;
    }
#endif

    void resize(int n, const T& val = T())
    {
        if (n < this->size())
            erase(this->begin() + n, this->end());
        else
            insert(this->end(), n - this->size(), val);
    }


    T& at(int i, bool* ok = 0)
    {
        this->detach();
        if (ok)
            *ok = (i >= 0 && i < this->size());
        return *(this->begin() + i);
    }

    const T&at(int i, bool* ok = 0) const
    {
        if (ok)
            *ok = (i >= 0 && i < this->size());
        return *(this->begin() + i);
    }
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // Q3VALUEVECTOR_H
