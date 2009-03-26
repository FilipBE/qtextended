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

#ifndef QTAGMAP_P_H
#define QTAGMAP_P_H

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

#include <QString>
#include <QMap>

class QTagString
{
public:
    inline QTagString();
    inline QTagString(const QTagString& other);
    inline QTagString(const QString& str);
    inline QTagString(const QLatin1String& str);
    inline QTagString(const char *str);
    inline ~QTagString();

    inline QTagString& operator=(const QTagString& other);
    inline QTagString& operator=(const QString& str);
    inline QTagString& operator=(const QLatin1String& str);
    inline QTagString& operator=(const char *str);

    inline QString toString() const;

    inline bool operator==(const QTagString& other) const;
    inline bool operator!=(const QTagString& other) const;
    inline bool operator<(const QTagString& other) const;
    inline bool operator<=(const QTagString& other) const;
    inline bool operator>(const QTagString& other) const;
    inline bool operator>=(const QTagString& other) const;

private:
    // Invariant: if latin1 is not null, then qstr is null.
    // Invariant: if latin1 is null, then qstr is not null.
    const char *latin1;
    QString *qstr;
};

inline QTagString::QTagString()
{
    latin1 = "";
    qstr = 0;
}

inline QTagString::QTagString(const QTagString& other)
{
    latin1 = other.latin1;
    if (other.qstr)
        qstr = new QString(*other.qstr);
    else
        qstr = 0;
}

inline QTagString::QTagString(const QString& str)
{
    latin1 = 0;
    qstr = new QString(str);
}

inline QTagString::QTagString(const QLatin1String& str)
{
    latin1 = str.latin1();
    if (latin1)
        qstr = 0;
    else
        qstr = new QString();
}

inline QTagString::QTagString(const char *str)
{
    latin1 = str;
    if (str)
        qstr = 0;
    else
        qstr = new QString();
}

inline QTagString::~QTagString()
{
    if (qstr)
        delete qstr;
}

inline QTagString& QTagString::operator=(const QTagString& other)
{
    if (this != &other) {
        if (other.latin1) {
            latin1 = other.latin1;
            if (qstr) {
                delete qstr;
                qstr = 0;
            }
        } else {
            latin1 = 0;
            if (qstr)
                *qstr = *other.qstr;
            else
                qstr = new QString(*other.qstr);
        }
    }
    return *this;
}

inline QTagString& QTagString::operator=(const QString& str)
{
    latin1 = 0;
    if (qstr)
        *qstr = str;
    else
        qstr = new QString(str);
    return *this;
}

inline QTagString& QTagString::operator=(const QLatin1String& str)
{
    latin1 = str.latin1();
    if (latin1) {
        if (qstr) {
            delete qstr;
            qstr = 0;
        }
    } else {
        if (qstr)
            *qstr = QString();
        else
            qstr = new QString();
    }
    return *this;
}

inline QTagString& QTagString::operator=(const char *str)
{
    latin1 = str;
    if (str) {
        if (qstr) {
            delete qstr;
            qstr = 0;
        }
    } else {
        if (qstr)
            *qstr = QString();
        else
            qstr = new QString();
    }
    return *this;
}

inline QString QTagString::toString() const
{
    if (latin1)
        return QLatin1String(latin1);
    else
        return *qstr;
}

inline bool QTagString::operator==(const QTagString& other) const
{
    if (latin1 && other.latin1)
        return (strcmp(latin1, other.latin1) == 0);
    else if (latin1)
        return (QLatin1String(latin1) == *other.qstr);
    else if (other.latin1)
        return (*qstr == QLatin1String(other.latin1));
    else
        return (*qstr == *other.qstr);
}

inline bool QTagString::operator!=(const QTagString& other) const
{
    if (latin1 && other.latin1)
        return (strcmp(latin1, other.latin1) != 0);
    else if (latin1)
        return (QLatin1String(latin1) != *other.qstr);
    else if (other.latin1)
        return (*qstr != QLatin1String(other.latin1));
    else
        return (*qstr != *other.qstr);
}

inline bool QTagString::operator<(const QTagString& other) const
{
    if (latin1 && other.latin1)
        return (strcmp(latin1, other.latin1) < 0);
    else if (latin1)
        return (QLatin1String(latin1) < *other.qstr);
    else if (other.latin1)
        return (*qstr < QLatin1String(other.latin1));
    else
        return (*qstr < *other.qstr);
}

inline bool QTagString::operator<=(const QTagString& other) const
{
    if (latin1 && other.latin1)
        return (strcmp(latin1, other.latin1) <= 0);
    else if (latin1)
        return (QLatin1String(latin1) <= *other.qstr);
    else if (other.latin1)
        return (*qstr <= QLatin1String(other.latin1));
    else
        return (*qstr <= *other.qstr);
}

inline bool QTagString::operator>(const QTagString& other) const
{
    if (latin1 && other.latin1)
        return (strcmp(latin1, other.latin1) > 0);
    else if (latin1)
        return (QLatin1String(latin1) > *other.qstr);
    else if (other.latin1)
        return (*qstr > QLatin1String(other.latin1));
    else
        return (*qstr > *other.qstr);
}

inline bool QTagString::operator>=(const QTagString& other) const
{
    if (latin1 && other.latin1)
        return (strcmp(latin1, other.latin1) >= 0);
    else if (latin1)
        return (QLatin1String(latin1) >= *other.qstr);
    else if (other.latin1)
        return (*qstr >= QLatin1String(other.latin1));
    else
        return (*qstr >= *other.qstr);
}

template <class T>
class QTagMap : public QMap<QTagString, T>
{
public:
    inline QTagMap() : QMap<QTagString, T>() {}
    inline QTagMap(const QTagMap<T>& other) : QMap<QTagString, T>(other) {}
    inline ~QTagMap() {}

    QTagMap<T> &operator=(const QTagMap<T> &other);

};

template <class T>
Q_INLINE_TEMPLATE QTagMap<T> &QTagMap<T>::operator=(const QTagMap<T> &other)
{
    *((QMap<QTagString, T> *)this) = *((const QMap<QTagString, T> *)&other);
    return *this;
}

#endif
