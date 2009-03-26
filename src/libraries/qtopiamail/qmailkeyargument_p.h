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

#ifndef QMAILKEYARGUMENT_P_H
#define QMAILKEYARGUMENT_P_H

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

#include <QDataStream>
#include <QVariantList>

class QMailKeyValueList : public QVariantList
{
public:
    bool operator==(const QMailKeyValueList& other) const
    {
        if (count() != other.count())
            return false;

        if (isEmpty())
            return true;

        // We can't compare QVariantList directly, since QVariant can't compare metatypes correctly
        QByteArray serialization, otherSerialization;
        {
            QDataStream serializer(&serialization, QIODevice::WriteOnly);
            serialize(serializer);

            QDataStream otherSerializer(&otherSerialization, QIODevice::WriteOnly);
            other.serialize(otherSerializer);
        }
        return (serialization == otherSerialization);
    }

    template <typename Stream> void serialize(Stream &stream) const
    {
        stream << count();
        foreach (const QVariant& value, *this)
            stream << value;
    }

    template <typename Stream> void deserialize(Stream &stream)
    {
        clear();

        int v = 0;
        stream >> v;
        for (int i = 0; i < v; ++i) {
            QVariant value;
            stream >> value;
            append(value);
        }
    }
};


template<typename PropertyType, typename ComparatorType = QMailDataComparator::Comparator>
class QMailKeyArgument
{
public:
    typedef PropertyType Property;
    typedef ComparatorType Comparator;

    Property property;
    Comparator op;
    QMailKeyValueList valueList;

    QMailKeyArgument()
    {
    }

    QMailKeyArgument(Property p, Comparator c, const QVariant& v)
        : property(p),
          op(c)
    {
          valueList.append(v);
    }
    
    template<typename ListType>
    QMailKeyArgument(Property p, Comparator c, const ListType& l)
        : property(p),
          op(c)
    {
        foreach (typename ListType::const_reference v, l)
            valueList.append(v);
    }
    
    bool operator==(const QMailKeyArgument<PropertyType, ComparatorType>& other) const
    {
        return property == other.property &&
               op == other.op &&
               valueList == other.valueList;
    }

    template <typename Stream> void serialize(Stream &stream) const
    {
        stream << property;
        stream << op;
        stream << valueList;
    }

    template <typename Stream> void deserialize(Stream &stream)
    {
        int v = 0;

        stream >> v;
        property = static_cast<Property>(v);
        stream >> v;
        op = static_cast<Comparator>(v);

        stream >> valueList;
    }
};

#endif

