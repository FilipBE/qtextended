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

#ifndef QDBUSARGUMENT_H
#define QDBUSARGUMENT_H

#include <QtCore/qbytearray.h>
#include <QtCore/qglobal.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qvariant.h>
#include <QtDBus/qdbusextratypes.h>
#include <QtDBus/qdbusmacros.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(DBus)

class QDBusArgumentPrivate;
class QDBusDemarshaller;
class QDBusMarshaller;
class QDBUS_EXPORT QDBusArgument
{
public:
    QDBusArgument();
    QDBusArgument(const QDBusArgument &other);
    QDBusArgument &operator=(const QDBusArgument &other);
    ~QDBusArgument();

    // used for marshalling (Qt -> D-BUS)
    QDBusArgument &operator<<(uchar arg);
    QDBusArgument &operator<<(bool arg);
    QDBusArgument &operator<<(short arg);
    QDBusArgument &operator<<(ushort arg);
    QDBusArgument &operator<<(int arg);
    QDBusArgument &operator<<(uint arg);
    QDBusArgument &operator<<(qlonglong arg);
    QDBusArgument &operator<<(qulonglong arg);
    QDBusArgument &operator<<(double arg);
    QDBusArgument &operator<<(const QString &arg);
    QDBusArgument &operator<<(const QDBusVariant &arg);
    QDBusArgument &operator<<(const QDBusObjectPath &arg);
    QDBusArgument &operator<<(const QDBusSignature &arg);
    QDBusArgument &operator<<(const QStringList &arg);
    QDBusArgument &operator<<(const QByteArray &arg);

    void beginStructure();
    void endStructure();
    void beginArray(int elementMetaTypeId);
    void endArray();
    void beginMap(int keyMetaTypeId, int valueMetaTypeId);
    void endMap();
    void beginMapEntry();
    void endMapEntry();

    // used for de-marshalling (D-BUS -> Qt)
    QString currentSignature() const;

    const QDBusArgument &operator>>(uchar &arg) const;
    const QDBusArgument &operator>>(bool &arg) const;
    const QDBusArgument &operator>>(short &arg) const;
    const QDBusArgument &operator>>(ushort &arg) const;
    const QDBusArgument &operator>>(int &arg) const;
    const QDBusArgument &operator>>(uint &arg) const;
    const QDBusArgument &operator>>(qlonglong &arg) const;
    const QDBusArgument &operator>>(qulonglong &arg) const;
    const QDBusArgument &operator>>(double &arg) const;
    const QDBusArgument &operator>>(QString &arg) const;
    const QDBusArgument &operator>>(QDBusVariant &arg) const;
    const QDBusArgument &operator>>(QDBusObjectPath &arg) const;
    const QDBusArgument &operator>>(QDBusSignature &arg) const;
    const QDBusArgument &operator>>(QStringList &arg) const;
    const QDBusArgument &operator>>(QByteArray &arg) const;

    void beginStructure() const;
    void endStructure() const;
    void beginArray() const;
    void endArray() const;
    void beginMap() const;
    void endMap() const;
    void beginMapEntry() const;
    void endMapEntry() const;
    bool atEnd() const;

protected:
    QDBusArgument(QDBusArgumentPrivate *d);
    friend class QDBusArgumentPrivate;
    mutable QDBusArgumentPrivate *d;
};

template<typename T> inline T qdbus_cast(const QDBusArgument &arg
#ifndef Q_QDOC
, T * = 0
#endif
    )
{
    T item;
    arg >> item;
    return item;
}

template<typename T> inline T qdbus_cast(const QVariant &v
#ifndef Q_QDOC
, T * = 0
#endif
    )
{
    int id = v.userType();
    if (id == qMetaTypeId<QDBusArgument>())
        return qdbus_cast<T>(qvariant_cast<QDBusArgument>(v));
    else
        return qvariant_cast<T>(v);
}

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QVariant &v);

// QVariant types
#ifndef QDBUS_NO_SPECIALTYPES

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QDate &date);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QDate &date);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QTime &time);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QTime &time);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QDateTime &dt);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QDateTime &dt);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QRect &rect);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QRect &rect);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QRectF &rect);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QRectF &rect);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QSize &size);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QSize &size);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QSizeF &size);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QSizeF &size);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QPoint &pt);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QPoint &pt);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QPointF &pt);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QPointF &pt);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QLine &line);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QLine &line);

QDBUS_EXPORT const QDBusArgument &operator>>(const QDBusArgument &a, QLineF &line);
QDBUS_EXPORT QDBusArgument &operator<<(QDBusArgument &a, const QLineF &line);
#endif

template<template <typename> class Container, typename T>
inline QDBusArgument &operator<<(QDBusArgument &arg, const Container<T> &list)
{
    int id = qMetaTypeId<T>();
    arg.beginArray(id);
    typename Container<T>::const_iterator it = list.begin();
    typename Container<T>::const_iterator end = list.end();
    for ( ; it != end; ++it)
        arg << *it;
    arg.endArray();
    return arg;
}

template<template <typename> class Container, typename T>
inline const QDBusArgument &operator>>(const QDBusArgument &arg, Container<T> &list)
{
    arg.beginArray();
    list.clear();
    while (!arg.atEnd()) {
        T item;
        arg >> item;
        list.push_back(item);
    }

    arg.endArray();
    return arg;
}

// QList specializations
template<typename T>
inline QDBusArgument &operator<<(QDBusArgument &arg, const QList<T> &list)
{
    int id = qMetaTypeId<T>();
    arg.beginArray(id);
    typename QList<T>::ConstIterator it = list.constBegin();
    typename QList<T>::ConstIterator end = list.constEnd();
    for ( ; it != end; ++it)
        arg << *it;
    arg.endArray();
    return arg;
}

template<typename T>
inline const QDBusArgument &operator>>(const QDBusArgument &arg, QList<T> &list)
{
    arg.beginArray();
    list.clear();
    while (!arg.atEnd()) {
        T item;
        arg >> item;
        list.push_back(item);
    }
    arg.endArray();

    return arg;
}

inline QDBusArgument &operator<<(QDBusArgument &arg, const QVariantList &list)
{
    int id = qMetaTypeId<QDBusVariant>();
    arg.beginArray(id);
    QVariantList::ConstIterator it = list.constBegin();
    QVariantList::ConstIterator end = list.constEnd();
    for ( ; it != end; ++it)
        arg << QDBusVariant(*it);
    arg.endArray();
    return arg;
}

// QMap specializations
template<typename Key, typename T>
inline QDBusArgument &operator<<(QDBusArgument &arg, const QMap<Key, T> &map)
{
    int kid = qMetaTypeId<Key>();
    int vid = qMetaTypeId<T>();
    arg.beginMap(kid, vid);
    typename QMap<Key, T>::ConstIterator it = map.constBegin();
    typename QMap<Key, T>::ConstIterator end = map.constEnd();
    for ( ; it != end; ++it) {
        arg.beginMapEntry();
        arg << it.key() << it.value();
        arg.endMapEntry();
    }
    arg.endMap();
    return arg;
}

template<typename Key, typename T>
inline const QDBusArgument &operator>>(const QDBusArgument &arg, QMap<Key, T> &map)
{
    arg.beginMap();
    map.clear();
    while (!arg.atEnd()) {
        Key key;
        T value;
        arg.beginMapEntry();
        arg >> key >> value;
        map.insertMulti(key, value);
        arg.endMapEntry();
    }
    arg.endMap();
    return arg;
}

inline QDBusArgument &operator<<(QDBusArgument &arg, const QVariantMap &map)
{
    arg.beginMap(QVariant::String, qMetaTypeId<QDBusVariant>());
    QVariantMap::ConstIterator it = map.constBegin();
    QVariantMap::ConstIterator end = map.constEnd();
    for ( ; it != end; ++it) {
        arg.beginMapEntry();
        arg << it.key() << QDBusVariant(it.value());
        arg.endMapEntry();
    }
    arg.endMap();
    return arg;
}

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QDBusArgument)

QT_END_HEADER

#endif
