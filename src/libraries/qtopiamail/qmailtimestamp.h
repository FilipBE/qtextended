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
#ifndef QMAILTIMESTAMP_H
#define QMAILTIMESTAMP_H

#include <qtopiaglobal.h>

#include <QDateTime>
#include <QSharedDataPointer>
#include <QString>

class QMailTimeStampPrivate;

class QTOPIAMAIL_EXPORT QMailTimeStamp
{
public:
    static QMailTimeStamp currentDateTime();

    QMailTimeStamp();
    explicit QMailTimeStamp(const QString& timeText);
    explicit QMailTimeStamp(const QDateTime& dateTime);
    QMailTimeStamp(const QMailTimeStamp& other);
    ~QMailTimeStamp();

    QString toString() const;

    QDateTime toLocalTime() const;
    QDateTime toUTC() const;

    bool isNull() const;
    bool isValid() const;

    bool operator== (const QMailTimeStamp& other) const;
    bool operator!= (const QMailTimeStamp& other) const;

    bool operator< (const QMailTimeStamp& other) const;
    bool operator<= (const QMailTimeStamp& other) const;

    bool operator> (const QMailTimeStamp& other) const;
    bool operator>= (const QMailTimeStamp& other) const;

    const QMailTimeStamp& operator=(const QMailTimeStamp& other);

private:
    QSharedDataPointer<QMailTimeStampPrivate> d;
};

#endif
