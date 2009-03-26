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

#ifndef QTOPIAIPCMARSHAL_H
#define QTOPIAIPCMARSHAL_H

#include <qtopiaglobal.h>

#include <qvariant.h>
#include <qmetatype.h>
#include <qdatastream.h>
#include <qatomic.h>
#include <quuid.h>

#if defined(QTOPIA_DBUS_IPC) && !defined(QTOPIA_HOST)
#include <qdbusmetatype.h>
#include <qdbusargument.h>
#endif

template <typename T>
struct QMetaTypeRegister
{
    static int registerType() { return 1; }
};

#ifdef Q_CC_GNU
# define _QATOMIC_ONCE() do {} while(0)
#else
# define _QATOMIC_ONCE()                \
    static QAtomicInt once;             \
    if ( once.fetchAndStoreOrdered(1) ) \
        return 1
#endif

#if defined(QTOPIA_DBUS_IPC) && !defined(QTOPIA_HOST)
#define Q_DECLARE_USER_METATYPE_NO_OPERATORS(TYPE) \
    Q_DECLARE_METATYPE(TYPE) \
    template<> \
    struct QMetaTypeRegister< TYPE > \
    { \
        static int registerType() \
        { \
            _QATOMIC_ONCE(); \
            int id = qMetaTypeId( reinterpret_cast<TYPE *>(0) ); \
            if ( id >= static_cast<int>(QMetaType::User) ) {\
                qRegisterMetaTypeStreamOperators< TYPE >( #TYPE ); \
                qDBusRegisterMetaType< TYPE >(); \
            } \
            return 1; \
        } \
        static int __init_variable__; \
    };
#else
#define Q_DECLARE_USER_METATYPE_NO_OPERATORS(TYPE) \
    Q_DECLARE_METATYPE(TYPE) \
    template<> \
    struct QMetaTypeRegister< TYPE > \
    { \
        static int registerType() \
        { \
            _QATOMIC_ONCE(); \
            int id = qMetaTypeId( reinterpret_cast<TYPE *>(0) ); \
            if ( id >= static_cast<int>(QMetaType::User) ) \
                qRegisterMetaTypeStreamOperators< TYPE >( #TYPE ); \
            return 1; \
        } \
        static int __init_variable__; \
    };
#endif

#if defined(QTOPIA_DBUS_IPC) && !defined(QTOPIA_HOST)
#define Q_DECLARE_USER_METATYPE(TYPE) \
    Q_DECLARE_USER_METATYPE_NO_OPERATORS(TYPE) \
    QTOPIABASE_EXPORT QDataStream &operator<<(QDataStream &stream, const TYPE &var); \
    QTOPIABASE_EXPORT QDataStream &operator>>( QDataStream &stream, TYPE &var ); \
    QTOPIABASE_EXPORT QDBusArgument &operator<<(QDBusArgument &stream, const TYPE &var); \
    QTOPIABASE_EXPORT const QDBusArgument &operator>>(const QDBusArgument &stream, TYPE &var );
#else
#define Q_DECLARE_USER_METATYPE(TYPE) \
    Q_DECLARE_USER_METATYPE_NO_OPERATORS(TYPE) \
    QTOPIABASE_EXPORT QDataStream &operator<<(QDataStream &stream, const TYPE &var); \
    QTOPIABASE_EXPORT QDataStream &operator>>( QDataStream &stream, TYPE &var );
#endif

#if defined(QTOPIA_DBUS_IPC) && !defined(QTOPIA_HOST)
#define Q_DECLARE_USER_METATYPE_TYPEDEF(TAG,TYPE)       \
    template <typename T> \
    struct QMetaTypeRegister##TAG \
    { \
        static int registerType() { return 1; } \
    }; \
    template<> struct QMetaTypeRegister##TAG< TYPE > { \
        static int registerType() { \
            _QATOMIC_ONCE(); \
            int id = qRegisterMetaType< TYPE >( #TYPE ); \
            qRegisterMetaTypeStreamOperators< TYPE >( #TYPE ); \
            void (*mf)(QDBusArgument &, const TYPE *) = qDBusMarshallHelper<TYPE>; \
            void (*df)(const QDBusArgument &, TYPE *) = qDBusDemarshallHelper<TYPE>;\
            QDBusMetaType::registerMarshallOperators(id, \
                reinterpret_cast<QDBusMetaType::MarshallFunction>(mf), \
                reinterpret_cast<QDBusMetaType::DemarshallFunction>(df)); \
            return 1; \
        } \
        static int __init_variable__; \
    };
#else
#define Q_DECLARE_USER_METATYPE_TYPEDEF(TAG,TYPE)       \
    template <typename T> \
    struct QMetaTypeRegister##TAG \
    { \
        static int registerType() { return 1; } \
    }; \
    template<> struct QMetaTypeRegister##TAG< TYPE > { \
        static int registerType() { \
            _QATOMIC_ONCE(); \
            qRegisterMetaType< TYPE >( #TYPE ); \
            qRegisterMetaTypeStreamOperators< TYPE >( #TYPE ); \
            return 1; \
        } \
        static int __init_variable__; \
    };
#endif

#define Q_DECLARE_USER_METATYPE_ENUM(TYPE)      \
    Q_DECLARE_USER_METATYPE(TYPE)

#define Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(TYPE) \
    int QMetaTypeRegister< TYPE >::__init_variable__ = \
        QMetaTypeRegister< TYPE >::registerType();

#if defined(QTOPIA_DBUS_IPC) && !defined(QTOPIA_HOST)
#define Q_IMPLEMENT_USER_METATYPE(TYPE) \
    QDataStream &operator<<(QDataStream &stream, const TYPE &var) \
    { \
        var.serialize(stream); \
        return stream; \
    } \
    QDataStream &operator>>( QDataStream &stream, TYPE &var ) \
    { \
        var.deserialize(stream); \
        return stream; \
    } \
    QDBusArgument &operator<<(QDBusArgument &stream, const TYPE &var) \
    { \
        stream.beginStructure(); \
        QByteArray array; \
        { \
            QDataStream bstream(&array, QIODevice::WriteOnly | QIODevice::Append); \
            var.serialize(bstream); \
        } \
        stream << array; \
        stream.endStructure(); \
        return stream; \
    } \
    const QDBusArgument &operator>>(const QDBusArgument &stream, TYPE &var ) \
    { \
        stream.beginStructure(); \
        QByteArray data; \
        stream >> data; \
        QDataStream bstream(data); \
        var.deserialize(bstream); \
        stream.endStructure(); \
        return stream; \
    } \
    Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(TYPE)
#else
#define Q_IMPLEMENT_USER_METATYPE(TYPE) \
    QDataStream &operator<<(QDataStream &stream, const TYPE &var) \
    { \
        var.serialize(stream); \
        return stream; \
    } \
    \
    QDataStream &operator>>( QDataStream &stream, TYPE &var ) \
    { \
        var.deserialize(stream); \
        return stream; \
    } \
    Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(TYPE)
#endif

#define Q_IMPLEMENT_USER_METATYPE_TYPEDEF(TAG,TYPE)     \
    int QMetaTypeRegister##TAG< TYPE >::__init_variable__ = \
        QMetaTypeRegister##TAG< TYPE >::registerType();

#if defined(QTOPIA_DBUS_IPC) && !defined(QTOPIA_HOST)
#define Q_IMPLEMENT_USER_METATYPE_ENUM(TYPE)    \
    QTOPIABASE_EXPORT QDataStream& operator<<( QDataStream& stream, const TYPE &v ) \
    { \
        stream << static_cast<qint32>(v); \
        return stream; \
    } \
    QTOPIABASE_EXPORT QDataStream& operator>>( QDataStream& stream, TYPE& v ) \
    { \
        qint32 _v; \
        stream >> _v; \
        v = static_cast<TYPE>(_v); \
        return stream; \
    } \
    QTOPIABASE_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const TYPE& v) \
    { \
        a.beginStructure(); \
        a << static_cast<qint32>(v); \
        a.endStructure(); \
        return a; \
    } \
    QTOPIABASE_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, TYPE &v) \
    { \
        a.beginStructure(); \
        qint32 _v; \
        a >> _v; \
        v = static_cast<TYPE>(_v); \
        a.endStructure(); \
        return a; \
    } \
    Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(TYPE)
#else
#define Q_IMPLEMENT_USER_METATYPE_ENUM(TYPE)    \
    QDataStream& operator<<( QDataStream& stream, const TYPE &v ) \
    { \
        stream << static_cast<qint32>(v); \
        return stream; \
    } \
    QDataStream& operator>>( QDataStream& stream, TYPE& v ) \
    { \
        qint32 _v; \
        stream >> _v; \
        v = static_cast<TYPE>(_v); \
        return stream; \
    } \
    Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(TYPE)
#endif

#define Q_REGISTER_USER_METATYPE(TYPE)  \
    QMetaTypeRegister< TYPE >::registerType()

#if defined(QTOPIA_DBUS_IPC) && !defined(QTOPIA_HOST)

QTOPIABASE_EXPORT const QDBusArgument& operator>>(const QDBusArgument& s, QUuid& u);
QTOPIABASE_EXPORT QDBusArgument& operator<<(QDBusArgument& s, const QUuid& u);

#endif

Q_DECLARE_USER_METATYPE_NO_OPERATORS(QUuid)

// Special variant class that can perform QDataStream operations
// without the QVariant header information.
class QtopiaIpcAdaptorVariant : public QVariant
{
    public:
        QtopiaIpcAdaptorVariant() : QVariant() {}
        explicit QtopiaIpcAdaptorVariant( const QVariant& value ) : QVariant( value ) {}

        void load( QDataStream& stream, int typeOrMetaType )
        {
            clear();
            create( typeOrMetaType, 0 );
            d.is_null = false;
            QMetaType::load
                ( stream, d.type, const_cast<void *>(constData()) );
        }
        void save( QDataStream& stream ) const
        {
            QMetaType::save( stream, d.type, constData() );
        }
};

#if defined(QTOPIA_DBUS_IPC) && !defined(QTOPIA_HOST)

#include <QPair>

template <class T1, class T2>
inline const QDBusArgument& operator>>(const QDBusArgument& s, QPair<T1, T2>& p)
{
    s.beginStructure();
    s >> p.first >> p.second;
    s.endStructure();
    return s;
}

template <class T1, class T2>
inline QDBusArgument& operator<<(QDBusArgument& s, const QPair<T1, T2>& p)
{
    s.beginStructure();
    s << p.first << p.second;
    s.endStructure();
    return s;
}

#endif

#endif
