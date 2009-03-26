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

#ifndef QTIMEZONE_H
#define QTIMEZONE_H

#include <qstring.h>
#include <qdatetime.h>
#include <qdatastream.h>
#include <qtopiaglobal.h>
#include <qtopiaipcmarshal.h>
#include <time.h>

class TimeZonePrivate;

class QTOPIA_EXPORT QTimeZone
{
public:
    QTimeZone();
    explicit QTimeZone( const char *id );
    QTimeZone( const QTimeZone & );
    ~QTimeZone();

    bool isValid() const;

    void setId( const char *id );

    QTimeZone &operator=( const QTimeZone &);
    bool operator==( const QTimeZone &) const;
    bool operator!=( const QTimeZone &) const;

    /*! returns QDateTime in this timezone */
    QDateTime fromTime_t( time_t t ) const;
    uint toTime_t( const QDateTime &thisT ) const;

    /** /a dt is in this time zone */
    QDateTime toUtc( const QDateTime &thisT ) const;
    QDateTime fromUtc( const QDateTime &utc ) const;

    QDateTime toCurrent( const QDateTime &thisT ) const;
    QDateTime fromCurrent( const QDateTime &curT ) const;

    // convert the /a dt which is in the timezone /a tz to "this" timezone
    QDateTime convert( const QDateTime &dt, const QTimeZone &dtTz ) const;

    static QTimeZone current();
    static QTimeZone utc();
    static QDateTime utcDateTime();
    static QTimeZone findFromMinutesEast(const QDateTime& t, int mineast, bool isdst);

    /** in seconds */
    int latitude() const;
    /** in seconds */
    int longitude() const;

    QString name() const;

    QString dstAbbreviation() const;
    QString standardAbbreviation() const;

    QString description() const;
    QString area() const;
    QString city() const;
    QString countryCode();
    QString id() const;
    int distance( const QTimeZone &e ) const;

    static QStringList ids();

    static void setSystemTimeZone(const QString &newtz);
    static void setApplicationTimeZone(const QString &newtz);

    /* for debugging purposes */
    //void dump() const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    TimeZonePrivate *d;
};

Q_DECLARE_USER_METATYPE(QTimeZone)

#endif
