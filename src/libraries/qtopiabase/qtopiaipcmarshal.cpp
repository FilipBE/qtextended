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

#include "qtopiaipcmarshal.h"

/*!
    \macro Q_DECLARE_USER_METATYPE(TYPE)

    This macro declares \a TYPE as a user-defined type within the
    Qt metatype system.  It should be used in header files, just
    after the declaration of \a TYPE.  A corresponding invocation
    of Q_IMPLEMENT_USER_METATYPE should appear in a source file.

    This example declares the class \c{MyClass}:

    \code
        Q_DECLARE_USER_METATYPE(MyClass)
    \endcode

    \sa Q_DECLARE_USER_METATYPE_ENUM(), Q_IMPLEMENT_USER_METATYPE(), Q_IMPLEMENT_USER_METATYPE_ENUM(), Q_REGISTER_USER_METATYPE()
*/

/*!
    \macro Q_DECLARE_USER_METATYPE_TYPEDEF(TAG,TYPE)

    This macro declares \a TYPE as a user-defined type within the
    Qt metatype system, but declares it as a typedef for a pre-existing
    metatype.  The \a TAG is an identifier that will usually be the same
    as \a TYPE, but may be slightly different if \a TYPE is nested.
    For example, if \a TYPE is \c{Foo::Bar}, then \a TAG should be
    \c{Foo_Bar} to make it a valid identifier.

    This macro should be used in header files, just after the declaration
    of \a TYPE.  A corresponding invocation of
    Q_IMPLEMENT_USER_METATYPE_TYPEDEF should appear in a source file.

    This example declares the types \c{Foo} and \c{Bar} as typedef aliases.

    \code
        typedef Foo Bar;
        Q_DECLARE_USER_METATYPE(Foo)
        Q_DECLARE_USER_METATYPE_TYPEDEF(Bar, Bar)
    \endcode

    \sa Q_DECLARE_USER_METATYPE_ENUM(), Q_IMPLEMENT_USER_METATYPE(), Q_IMPLEMENT_USER_METATYPE_ENUM(), Q_REGISTER_USER_METATYPE()
*/

/*!
    \macro Q_DECLARE_USER_METATYPE_ENUM(TYPE)

    This macro declares \a TYPE as a user-defined enumerated type within
    the Qt metatype system.  It should be used in header files, just
    after the declaration of \a TYPE.  A corresponding invocation
    of Q_IMPLEMENT_USER_METATYPE_ENUM should appear in a source file.

    This example declares the enumerated type \c{MyEnum}:

    \code
        Q_DECLARE_USER_METATYPE_ENUM(MyEnum)
    \endcode

    This macro differs from Q_DECLARE_USER_METATYPE in that it explicitly
    declares datastream operators for the type.

    \sa Q_IMPLEMENT_USER_METATYPE_ENUM(), Q_IMPLEMENT_USER_METATYPE(), Q_DECLARE_METATYPE(), Q_REGISTER_USER_METATYPE()
*/

/*!
    \macro Q_IMPLEMENT_USER_METATYPE(TYPE)

    This macro implements the code necessary to register \a TYPE
    as a user-defined type within the Qt metatype system.

    This example implements the registration logic for the class \c{MyClass}:

    \code
        Q_IMPLEMENT_USER_METATYPE(MyClass)
    \endcode

    On most systems, this macro will arrange for registration to be
    performed at program startup.  On systems that don't support
    global constructors properly, it may be necessary to manually
    call Q_REGISTER_USER_METATYPE().

    \sa Q_DECLARE_USER_METATYPE(), Q_DECLARE_USER_METATYPE_ENUM(), Q_IMPLEMENT_USER_METATYPE_ENUM(), Q_REGISTER_USER_METATYPE()
*/

/*!
    \macro Q_IMPLEMENT_USER_METATYPE_TYPEDEF(TAG,TYPE)

    This macro implements the code necessary to register \a TYPE
    as a user-defined typedef alias within the Qt metatype system.
    The \a TAG should be the same as the tag used in the
    corresponding invocation of Q_DECLARE_USER_METATYPE_TYPEDEF.

    This example implements the registration logic for the typedef'ed
    types \c{Foo} and \c{Bar}:

    \code
        typedef Foo Bar;
        Q_IMPLEMENT_USER_METATYPE(Foo)
        Q_IMPLEMENT_USER_METATYPE_TYPEDEF(Bar, Bar)
    \endcode

    On most systems, this macro will arrange for registration to be
    performed at program startup.  On systems that don't support
    global constructors properly, it may be necessary to manually
    call Q_REGISTER_USER_METATYPE().

    \sa Q_DECLARE_USER_METATYPE(), Q_DECLARE_USER_METATYPE_ENUM(), Q_IMPLEMENT_USER_METATYPE_ENUM(), Q_REGISTER_USER_METATYPE()
*/

/*!
    \macro Q_IMPLEMENT_USER_METATYPE_ENUM(TYPE)

    This macro implements the code necessary to register \a TYPE
    as a user-defined type within the Qt metatype system.  \a TYPE
    is assumed to be an enumerated type.  Using this macro relieves
    the need to manually declare \c{operator<<} and \c{operator>>}
    for the enumerated type.  Non-enumerated types should use
    Q_IMPLEMENT_USER_METATYPE() instead.

    This example implements the registration logic for the type \c{MyEnum}:

    \code
        Q_IMPLEMENT_USER_METATYPE_ENUM(MyEnum)
    \endcode

    On most systems, this macro will arrange for registration to be
    performed at program startup.  On systems that don't support
    global constructors properly, it may be necessary to manually
    call Q_REGISTER_USER_METATYPE().

    \sa Q_DECLARE_USER_METATYPE_ENUM(), Q_DECLARE_USER_METATYPE(), Q_IMPLEMENT_USER_METATYPE(), Q_REGISTER_USER_METATYPE()
*/

/*!
    \macro Q_REGISTER_USER_METATYPE(TYPE)

    This macro can be called as a function to manually register \a TYPE
    as a user-defined type within the Qt metatype system.  It is only
    needed if the system does not support global constructors properly.

    \sa Q_DECLARE_USER_METATYPE(), Q_DECLARE_USER_METATYPE_ENUM(), Q_IMPLEMENT_USER_METATYPE(), Q_IMPLEMENT_USER_METATYPE_ENUM()
*/

Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(QUuid)

#if defined(QTOPIA_DBUS_IPC)
QDBusArgument &operator<<(QDBusArgument &a, const QUuid &v)
{
    a.beginStructure();
    a << v.toString();
    a.endStructure();
    return a;
}

const QDBusArgument &operator>>(const QDBusArgument &a, QUuid &v)
{
    QString uuid;
    a.beginStructure();
    a >> uuid;
    a.endStructure();

    v = QUuid(uuid);

    return a;
}
#endif
