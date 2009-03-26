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

#ifndef QWHEREABOUTSUPDATE_H
#define QWHEREABOUTSUPDATE_H

#include <qtopiaglobal.h>
#include <QWhereaboutsCoordinate>
#include <QDateTime>

class QWhereaboutsUpdatePrivate;
class QDataStream;
class QDebug;

class QTOPIAWHEREABOUTS_EXPORT QWhereaboutsUpdate
{
public:
    enum DataType {
        GroundSpeed = 0x2,
        VerticalSpeed = 0x4,
        Course = 0x8,
        HorizontalAccuracy = 0x10,
        VerticalAccuracy = 0x20,
        GroundSpeedAccuracy = 0x40,
        VerticalSpeedAccuracy = 0x80,
        CourseAccuracy = 0x100,
        UpdateTimeAccuracy = 0x200
    };
    Q_DECLARE_FLAGS(DataTypes, DataType)

    enum PositionFixStatus {
        FixStatusUnknown,
        FixNotAcquired,
        FixAcquired
    };

    QWhereaboutsUpdate();
    QWhereaboutsUpdate(const QWhereaboutsCoordinate &coordinate, const QDateTime &dateTime);
    QWhereaboutsUpdate(const QWhereaboutsUpdate &other);
    ~QWhereaboutsUpdate();

    QWhereaboutsUpdate &operator=(const QWhereaboutsUpdate &other);

    bool isNull() const;
    DataTypes dataValidityFlags() const;

    void clear();

    void setUpdateDateTime(const QDateTime &dateTime) const;
    QDateTime updateDateTime() const;

    void setUpdateDate(const QDate &date);
    QDate updateDate() const;

    void setUpdateTime(const QTime &time);
    QTime updateTime() const;

    void setCoordinate(const QWhereaboutsCoordinate &coordinate);
    QWhereaboutsCoordinate coordinate() const;

    qreal groundSpeed() const;
    void setGroundSpeed(qreal speed);

    qreal verticalSpeed() const;
    void setVerticalSpeed(qreal speed);

    qreal course() const;
    void setCourse(qreal course);

    qreal updateTimeAccuracy() const;
    void setUpdateTimeAccuracy(qreal accuracy);

    qreal horizontalAccuracy() const;
    void setHorizontalAccuracy(qreal accuracy);

    qreal verticalAccuracy() const;
    void setVerticalAccuracy(qreal accuracy);

    qreal groundSpeedAccuracy() const;
    void setGroundSpeedAccuracy(qreal accuracy);

    qreal verticalSpeedAccuracy() const;
    void setVerticalSpeedAccuracy(qreal accuracy);

    qreal courseAccuracy() const;
    void setCourseAccuracy(qreal accuracy);

    static QWhereaboutsUpdate fromNmea(const QByteArray &nmea, PositionFixStatus *fixStatus = 0);

private:
    QWhereaboutsUpdatePrivate *d;
};

#ifndef QT_NO_DEBUG_STREAM
QTOPIAWHEREABOUTS_EXPORT QDebug operator<<(QDebug, const QWhereaboutsUpdate &);
#endif

Q_DECLARE_OPERATORS_FOR_FLAGS(QWhereaboutsUpdate::DataTypes);

#endif
