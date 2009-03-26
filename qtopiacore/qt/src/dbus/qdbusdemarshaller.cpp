/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtDBus module of the Qt Toolkit.
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

#include "qdbusargument_p.h"
#include <stdlib.h>

QT_BEGIN_NAMESPACE

template <typename T>
static inline T qIterGet(DBusMessageIter *it)
{
    T t;
    q_dbus_message_iter_get_basic(it, &t);
    q_dbus_message_iter_next(it);
    return t;
}

QDBusDemarshaller::~QDBusDemarshaller()
{
}

inline QString QDBusDemarshaller::currentSignature()
{
    char *sig = q_dbus_message_iter_get_signature(&iterator);
    QString retval = QString::fromUtf8(sig);
    q_dbus_free(sig);

    return retval;
}

inline uchar QDBusDemarshaller::toByte()
{
    return qIterGet<uchar>(&iterator);
}

inline bool QDBusDemarshaller::toBool()
{
    return bool(qIterGet<dbus_bool_t>(&iterator));
}

inline ushort QDBusDemarshaller::toUShort()
{
    return qIterGet<dbus_uint16_t>(&iterator);
}

inline short QDBusDemarshaller::toShort()
{
    return qIterGet<dbus_int16_t>(&iterator);
}

inline int QDBusDemarshaller::toInt()
{
    return qIterGet<dbus_int32_t>(&iterator);
}

inline uint QDBusDemarshaller::toUInt()
{
    return qIterGet<dbus_uint32_t>(&iterator);
}

inline qlonglong QDBusDemarshaller::toLongLong()
{
    return qIterGet<qlonglong>(&iterator);
}

inline qulonglong QDBusDemarshaller::toULongLong()
{
    return qIterGet<qulonglong>(&iterator);
}

inline double QDBusDemarshaller::toDouble()
{
    return qIterGet<double>(&iterator);
}

inline QString QDBusDemarshaller::toString()
{
    return QString::fromUtf8(qIterGet<char *>(&iterator));
}

inline QDBusObjectPath QDBusDemarshaller::toObjectPath()
{
    return QDBusObjectPath(QString::fromUtf8(qIterGet<char *>(&iterator)));
}

inline QDBusSignature QDBusDemarshaller::toSignature()
{
    return QDBusSignature(QString::fromUtf8(qIterGet<char *>(&iterator)));
}

inline QDBusVariant QDBusDemarshaller::toVariant()
{
    QDBusDemarshaller sub;
    sub.message = q_dbus_message_ref(message);
    q_dbus_message_iter_recurse(&iterator, &sub.iterator);
    q_dbus_message_iter_next(&iterator);

    return QDBusVariant( sub.toVariantInternal() );
}

QVariant QDBusDemarshaller::toVariantInternal()
{
    switch (q_dbus_message_iter_get_arg_type(&iterator)) {
    case DBUS_TYPE_BYTE:
        return qVariantFromValue(toByte());
    case DBUS_TYPE_INT16:
	return qVariantFromValue(toShort());
    case DBUS_TYPE_UINT16:
	return qVariantFromValue(toUShort());
    case DBUS_TYPE_INT32:
        return toInt();
    case DBUS_TYPE_UINT32:
        return toUInt();
    case DBUS_TYPE_DOUBLE:
        return toDouble();
    case DBUS_TYPE_BOOLEAN:
        return toBool();
    case DBUS_TYPE_INT64:
        return toLongLong();
    case DBUS_TYPE_UINT64:
        return toULongLong();
    case DBUS_TYPE_STRING:
        return toString();
    case DBUS_TYPE_OBJECT_PATH:
        return qVariantFromValue(toObjectPath());
    case DBUS_TYPE_SIGNATURE:
        return qVariantFromValue(toSignature());
    case DBUS_TYPE_VARIANT:
        return qVariantFromValue(toVariant());

    case DBUS_TYPE_ARRAY:
        switch (q_dbus_message_iter_get_element_type(&iterator)) {
        case DBUS_TYPE_BYTE:
            // QByteArray
            return toByteArray();
        case DBUS_TYPE_STRING:
            return toStringList();
        case DBUS_TYPE_DICT_ENTRY:
            return qVariantFromValue(duplicate());

        default:
            return qVariantFromValue(duplicate());
        }

    case DBUS_TYPE_STRUCT:
        return qVariantFromValue(duplicate());

    default:
        qWarning("QDDBusDemarshaller: Found unknown D-DBUS type %d '%c'",
                 q_dbus_message_iter_get_arg_type(&iterator),
                 q_dbus_message_iter_get_arg_type(&iterator));
        return QVariant();
        break;
    };
}

QStringList QDBusDemarshaller::toStringList()
{
    QStringList list;

    QDBusDemarshaller sub;
    q_dbus_message_iter_recurse(&iterator, &sub.iterator);
    q_dbus_message_iter_next(&iterator);
    while (!sub.atEnd())
        list.append(sub.toString());

    return list;
}

QByteArray QDBusDemarshaller::toByteArray()
{
    DBusMessageIter sub;
    q_dbus_message_iter_recurse(&iterator, &sub);
    q_dbus_message_iter_next(&iterator);
    int len;
    char* data;
    q_dbus_message_iter_get_fixed_array(&sub,&data,&len);
    return QByteArray(data,len);
}

bool QDBusDemarshaller::atEnd()
{
    // dbus_message_iter_has_next is broken if the list has one single element
    return q_dbus_message_iter_get_arg_type(&iterator) == DBUS_TYPE_INVALID;
}

inline QDBusDemarshaller *QDBusDemarshaller::beginStructure()
{
    return beginCommon();
}

inline QDBusDemarshaller *QDBusDemarshaller::beginArray()
{
    return beginCommon();
}

inline QDBusDemarshaller *QDBusDemarshaller::beginMap()
{
    return beginCommon();
}

inline QDBusDemarshaller *QDBusDemarshaller::beginMapEntry()
{
    return beginCommon();
}

QDBusDemarshaller *QDBusDemarshaller::beginCommon()
{
    QDBusDemarshaller *d = new QDBusDemarshaller;
    d->parent = this;
    d->message = q_dbus_message_ref(message);

    // recurse
    q_dbus_message_iter_recurse(&iterator, &d->iterator);
    q_dbus_message_iter_next(&iterator);
    return d;
}

inline QDBusDemarshaller *QDBusDemarshaller::endStructure()
{
    return endCommon();
}

inline QDBusDemarshaller *QDBusDemarshaller::endArray()
{
    return endCommon();
}

inline QDBusDemarshaller *QDBusDemarshaller::endMap()
{
    return endCommon();
}

inline QDBusDemarshaller *QDBusDemarshaller::endMapEntry()
{
    return endCommon();
}

QDBusDemarshaller *QDBusDemarshaller::endCommon()
{
    QDBusDemarshaller *retval = parent;
    delete this;
    return retval;
}

QDBusArgument QDBusDemarshaller::duplicate()
{
    QDBusDemarshaller *d = new QDBusDemarshaller;
    d->iterator = iterator;
    d->message = q_dbus_message_ref(message);

    q_dbus_message_iter_next(&iterator);
    return QDBusArgumentPrivate::create(d);
}

QT_END_NAMESPACE
