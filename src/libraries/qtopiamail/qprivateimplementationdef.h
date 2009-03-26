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

#ifndef QPRIVATEIMPLEMENTATIONDEF_H
#define QPRIVATEIMPLEMENTATIONDEF_H

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

#include "qprivateimplementation.h"


template<typename ImplementationType>
QTOPIAMAIL_EXPORT QPrivatelyImplemented<ImplementationType>::QPrivatelyImplemented(ImplementationType* p)
    : d(p)
{
}

template<typename ImplementationType>
QTOPIAMAIL_EXPORT QPrivatelyImplemented<ImplementationType>::QPrivatelyImplemented(const QPrivatelyImplemented& other)
    : d(other.d)
{
}

template<typename ImplementationType>
template<typename A1>
QTOPIAMAIL_EXPORT QPrivatelyImplemented<ImplementationType>::QPrivatelyImplemented(ImplementationType* p, A1 a1)
    : d(p, a1)
{
}

template<typename ImplementationType>
QTOPIAMAIL_EXPORT QPrivatelyImplemented<ImplementationType>::~QPrivatelyImplemented()
{
}

template<typename ImplementationType>
QTOPIAMAIL_EXPORT const QPrivatelyImplemented<ImplementationType>& QPrivatelyImplemented<ImplementationType>::operator=(const QPrivatelyImplemented<ImplementationType>& other)
{
    d = other.d;
    return *this;
}

template<typename ImplementationType>
template<typename ImplementationSubclass>
QTOPIAMAIL_EXPORT ImplementationSubclass* QPrivatelyImplemented<ImplementationType>::impl()
{
    return static_cast<ImplementationSubclass*>(static_cast<ImplementationType*>(d));
}

template<typename ImplementationType>
template<typename InterfaceType>
QTOPIAMAIL_EXPORT typename InterfaceType::ImplementationType* QPrivatelyImplemented<ImplementationType>::impl(InterfaceType*)
{
    return impl<typename InterfaceType::ImplementationType>();
}

template<typename ImplementationType>
template<typename ImplementationSubclass>
QTOPIAMAIL_EXPORT const ImplementationSubclass* QPrivatelyImplemented<ImplementationType>::impl() const
{
    return static_cast<const ImplementationSubclass*>(static_cast<const ImplementationType*>(d));
}

template<typename ImplementationType>
template<typename InterfaceType>
QTOPIAMAIL_EXPORT const typename InterfaceType::ImplementationType* QPrivatelyImplemented<ImplementationType>::impl(const InterfaceType*) const
{
    return impl<const typename InterfaceType::ImplementationType>();
}

/* We could probably use SFINAE to make these work, but I won't try now...
template<typename ImplementationType>
QTOPIAMAIL_EXPORT bool QPrivatelyImplemented<ImplementationType>::operator== (const QPrivatelyImplemented<ImplementationType>& other) const
{
    return ((d == other.d) ||
            (*(impl<ImplementationType>()) == *(other.impl<ImplementationType>())));
}

template<typename ImplementationType>
QTOPIAMAIL_EXPORT bool QPrivatelyImplemented<ImplementationType>::operator!= (const QPrivatelyImplemented<ImplementationType>& other) const
{
    return ((d != other.d) &&
            !(*(impl<ImplementationType>()) == *(other.impl<ImplementationType>())));
}

template<typename ImplementationType>
QTOPIAMAIL_EXPORT bool QPrivatelyImplemented<ImplementationType>::operator< (const QPrivatelyImplemented<ImplementationType>& other) const
{
    return ((d != other.d) &&
            (*impl<ImplementationType>() < *other.impl<ImplementationType>()));
}
*/


template<typename ImplementationType>
QTOPIAMAIL_EXPORT QPrivatelyNoncopyable<ImplementationType>::QPrivatelyNoncopyable(ImplementationType* p)
    : d(p)
{
}

template<typename ImplementationType>
template<typename A1>
QTOPIAMAIL_EXPORT QPrivatelyNoncopyable<ImplementationType>::QPrivatelyNoncopyable(ImplementationType* p, A1 a1)
    : d(p, a1)
{
}

template<typename ImplementationType>
QTOPIAMAIL_EXPORT QPrivatelyNoncopyable<ImplementationType>::~QPrivatelyNoncopyable()
{
}

template<typename ImplementationType>
template<typename ImplementationSubclass>
QTOPIAMAIL_EXPORT ImplementationSubclass* QPrivatelyNoncopyable<ImplementationType>::impl()
{
    return static_cast<ImplementationSubclass*>(static_cast<ImplementationType*>(d));
}

template<typename ImplementationType>
template<typename InterfaceType>
QTOPIAMAIL_EXPORT typename InterfaceType::ImplementationType* QPrivatelyNoncopyable<ImplementationType>::impl(InterfaceType*)
{
    return impl<typename InterfaceType::ImplementationType>();
}

template<typename ImplementationType>
template<typename ImplementationSubclass>
QTOPIAMAIL_EXPORT const ImplementationSubclass* QPrivatelyNoncopyable<ImplementationType>::impl() const
{
    return static_cast<const ImplementationSubclass*>(static_cast<const ImplementationType*>(d));
}

template<typename ImplementationType>
template<typename InterfaceType>
QTOPIAMAIL_EXPORT const typename InterfaceType::ImplementationType* QPrivatelyNoncopyable<ImplementationType>::impl(const InterfaceType*) const
{
    return impl<const typename InterfaceType::ImplementationType>();
}

/* We could probably use SFINAE to make these work, but I won't try now...
template<typename ImplementationType>
QTOPIAMAIL_EXPORT bool QPrivatelyNoncopyable<ImplementationType>::operator== (const QPrivatelyNoncopyable<ImplementationType>& other) const
{
    return ((d == other.d) ||
            (*(impl<ImplementationType>()) == *(other.impl<ImplementationType>())));
}

template<typename ImplementationType>
QTOPIAMAIL_EXPORT bool QPrivatelyNoncopyable<ImplementationType>::operator!= (const QPrivatelyNoncopyable<ImplementationType>& other) const
{
    return ((d != other.d) &&
            !(*(impl<ImplementationType>()) == *(other.impl<ImplementationType>())));
}

template<typename ImplementationType>
QTOPIAMAIL_EXPORT bool QPrivatelyNoncopyable<ImplementationType>::operator< (const QPrivatelyNoncopyable<ImplementationType>& other) const
{
    return ((d != other.d) &&
            (*impl<ImplementationType>() < *other.impl<ImplementationType>()));
}
*/


#endif
