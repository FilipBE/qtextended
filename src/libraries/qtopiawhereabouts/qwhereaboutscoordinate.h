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
#ifndef QWHEREABOUTSCOORDINATE_H
#define QWHEREABOUTSCOORDINATE_H

#include <qtopiaglobal.h>
#include <QObject>

class QWhereaboutsCoordinatePrivate;

class QTOPIAWHEREABOUTS_EXPORT QWhereaboutsCoordinate
{
public:
    enum CoordinateFormat {
        DecimalDegrees,
        DecimalDegreesWithHemisphere,
        DegreesMinutes,
        DegreesMinutesWithHemisphere,
        DegreesMinutesSeconds,
        DegreesMinutesSecondsWithHemisphere
    };

    enum CoordinateType {
        InvalidCoordinate,
        Coordinate2D,
        Coordinate3D
    };

    QWhereaboutsCoordinate();
    QWhereaboutsCoordinate(double latitude, double longitude);
    QWhereaboutsCoordinate(double latitude, double longitude, double altitude);
    QWhereaboutsCoordinate(const QWhereaboutsCoordinate &other);
    ~QWhereaboutsCoordinate();

    QWhereaboutsCoordinate &operator=(const QWhereaboutsCoordinate &other);

    bool isNull() const;
    CoordinateType type() const;

    void setLatitude(double latitude);
    double latitude() const;

    void setLongitude(double longitude);
    double longitude() const;

    void setAltitude(double altitude);
    double altitude() const;

    qreal distanceTo(const QWhereaboutsCoordinate &other);
    qreal azimuthTo(const QWhereaboutsCoordinate &other);

    QString toString(CoordinateFormat format = DegreesMinutesSecondsWithHemisphere) const;

private:
    QWhereaboutsCoordinatePrivate *d;
};

#ifndef QT_NO_DEBUG_STREAM
QTOPIAWHEREABOUTS_EXPORT QDebug operator<<(QDebug, const QWhereaboutsCoordinate &);
#endif

#endif
