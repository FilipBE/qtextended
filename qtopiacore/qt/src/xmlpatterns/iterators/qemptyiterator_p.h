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

#ifndef Patternist_EmptyIterator_H
#define Patternist_EmptyIterator_H

#include "qabstractxmlforwarditerator_p.h"
#include "qprimitives_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{

    /**
     * @short An QAbstractXmlForwardIterator which always is empty.
     *
     * EmptyIterator is an QAbstractXmlForwardIterator over the type @c T, which always is empty. Other
     * iterators can also be empty(or, at least behave as they are empty), but this
     * class is special designed for this purpose and is therefore fast.
     *
     * EmptyIterator's constructor is protected, instances is retrieved from CommonValues.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_iterators
     */
    template<typename T> class EmptyIterator : public QAbstractXmlForwardIterator<T>
    {
    public:
        /**
         * @returns always a default constructed value, T().
         */
        virtual T next()
        {
            return T();
        }

        /**
         * @returns always a default constructed value, T().
         */
        virtual T current() const
        {
            return T();
        }

        /**
         * @returns always 0.
         */
        virtual xsInteger position() const
        {
            return 0;
        }

        /**
         * @returns always @c this, the reverse of <tt>()</tt> is <tt>()</tt>.
         */
        virtual typename QAbstractXmlForwardIterator<T>::Ptr toReversed()
        {
            return typename QAbstractXmlForwardIterator<T>::Ptr(const_cast<EmptyIterator<T> *>(this));
        }

        /**
         * @returns always 0
         */
        virtual xsInteger count()
        {
            return 0;
        }

        /**
         * @returns @c this
         */
        virtual typename QAbstractXmlForwardIterator<T>::Ptr copy() const
        {
            return typename QAbstractXmlForwardIterator<T>::Ptr(const_cast<EmptyIterator *>(this));
        }

    protected:
        friend class CommonValues;
    };

    template<typename T>
    static inline
    typename QAbstractXmlForwardIterator<T>::Ptr
    makeEmptyIterator()
    {
        return typename QAbstractXmlForwardIterator<T>::Ptr(new EmptyIterator<T>());
    }

}

QT_END_NAMESPACE

QT_END_HEADER

#endif
