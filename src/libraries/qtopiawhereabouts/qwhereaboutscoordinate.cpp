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

#include "qwhereaboutscoordinate.h"

#include <math.h>

#include <qnumeric.h>
#include <QDateTime>
#include <QHash>
#include <QDebug>

static const double qwhereaboutscoordinate_EARTH_MEAN_RADIUS = 6371.0072;

inline static double qwhereaboutscoordinate_degToRad(double deg) { return deg * M_PI / 180; }
inline static double qwhereaboutscoordinate_radToDeg(double rad) { return rad * 180 / M_PI; }


class QWhereaboutsCoordinatePrivate
{
public:
    double lat;
    double lng;
    double alt;
};


/*!
    \class QWhereaboutsCoordinate
    \inpublicgroup QtLocationModule
    \ingroup whereabouts
    \brief The QWhereaboutsCoordinate class contains a geographical coordinate.

    A QWhereaboutsCoordinate has a latitude and longitude, and optionally, an
    altitude.

    Use type() to determine whether a coordinate is a 2D coordinate (has latitude and
    longitude only) or 3D coordinate (has latitude, longitude and altitude). Use
    distanceTo() and azimuthTo() to calculate the distance and bearing between
    coordinates.

    \sa QWhereabouts
*/

/*!
    \enum QWhereaboutsCoordinate::CoordinateFormat
    Defines the possible formatting options for toString().

    \value DecimalDegrees Returns a string representation of the coordinates in decimal degrees format.
    \value DecimalDegreesWithHemisphere Returns a string representation of the coordinates in decimal degrees format, using 'N', 'S', 'E' or 'W' to indicate the hemispheres of the coordinates.
    \value DegreesMinutes Returns a string representation of the coordinates in degrees-minutes format.
    \value DegreesMinutesWithHemisphere Returns a string representation of the coordinates in degrees-minutes format, using 'N', 'S', 'E' or 'W' to indicate the hemispheres of the coordinates.
    \value DegreesMinutesSeconds Returns a string representation of the coordinates in degrees-minutes-seconds format.
    \value DegreesMinutesSecondsWithHemisphere Returns a string representation of the coordinates in degrees-minutes-seconds format, using 'N', 'S', 'E' or 'W' to indicate the hemispheres of the coordinates.

    \sa toString()
*/

/*!
    \enum QWhereaboutsCoordinate::CoordinateType
    Defines the possible types for a coordinate.

    \value InvalidCoordinate The coordinate is neither a 2D nor 3D coordinate.
    \value Coordinate2D The coordinate has valid latitude and longitude values.
    \value Coordinate3D The coordinate has valid latitude, longitude and altitude values.
*/

/*!
    Constructs a coordinate. The coordinate will be invalid until
    setLatitude() and setLongitude() have been called.
*/
QWhereaboutsCoordinate::QWhereaboutsCoordinate()
    : d(new QWhereaboutsCoordinatePrivate)
{
    d->lat = qQNaN();
    d->lng = qQNaN();
    d->alt = qQNaN();
}

/*!
    Constructs a coordinate with the given \a latitude and \a longitude.
*/
QWhereaboutsCoordinate::QWhereaboutsCoordinate(double latitude, double longitude)
    : d(new QWhereaboutsCoordinatePrivate)
{
    d->lat = latitude;
    d->lng = longitude;
    d->alt = qQNaN();
}

/*!
    Constructs a coordinate with the given \a latitude, \a longitude
    and \a altitude.
*/
QWhereaboutsCoordinate::QWhereaboutsCoordinate(double latitude, double longitude, double altitude)
    : d(new QWhereaboutsCoordinatePrivate)
{
    d->lat = latitude;
    d->lng = longitude;
    d->alt = altitude;
}

/*!
    Constructs a coordinate from the contents of \a other.
*/
QWhereaboutsCoordinate::QWhereaboutsCoordinate(const QWhereaboutsCoordinate &other)
    : d(new QWhereaboutsCoordinatePrivate)
{
    operator=(other);
}

/*!
    Destroys the coordinate object.
*/
QWhereaboutsCoordinate::~QWhereaboutsCoordinate()
{
    delete d;
}

/*!
    Assigns \a other to this coordinate and returns a reference to this
    coordinate.
*/
QWhereaboutsCoordinate &QWhereaboutsCoordinate::operator=(const QWhereaboutsCoordinate &other)
{
    if (this == &other)
        return *this;

    d->lat = other.d->lat;
    d->lng = other.d->lng;
    d->alt = other.d->alt;

    return *this;
}

/*!
    Returns \c true if no attributes have been set for this coordinate.
*/
bool QWhereaboutsCoordinate::isNull() const
{
    return (qIsNaN(d->lat) && qIsNaN(d->lng) && qIsNaN(d->alt));
}

/*!
    Returns the type of this coordinate.
*/
QWhereaboutsCoordinate::CoordinateType QWhereaboutsCoordinate::type() const
{
    if ( (d->lat >= -90 && d->lat <= 90)
          && (d->lng >= -180 && d->lng <= 180) ) {
        if (qIsNaN(d->alt))
            return Coordinate2D;
        return Coordinate3D;
    }
    return InvalidCoordinate;
}


/*!
    Returns the latitude, in decimal degrees. A positive latitude indicates
    the Northern Hemisphere, and a negative latitude indicates the Southern
    Hemisphere.

    \sa setLatitude()
*/
double QWhereaboutsCoordinate::latitude() const
{
    return d->lat;
}

/*!
    Sets the latitude (in decimal degrees) to \a latitude. The value should
    be in the WGS84 datum.

    To be valid, the latitude must be between -90 to 90 inclusive.

    \sa latitude()
*/
void QWhereaboutsCoordinate::setLatitude(double latitude)
{
    d->lat = latitude;
}

/*!
    Returns the longitude, in decimal degrees. A positive longitude indicates
    the Eastern Hemisphere, and a negative longitude indicates the Western
    Hemisphere.

    \sa setLongitude()
*/
double QWhereaboutsCoordinate::longitude() const
{
    return d->lng;
}

/*!
    Sets the longitude (in decimal degrees) to \a longitude. The value should
    be in the WGS84 datum.

    To be valid, the longitude must be between -180 to 180 inclusive.

    \sa longitude()
*/
void QWhereaboutsCoordinate::setLongitude(double longitude)
{
    d->lng = longitude;
}

/*!
    Returns the altitude (meters above sea level).

    \sa setAltitude()
*/
double QWhereaboutsCoordinate::altitude() const
{
    return d->alt;
}

/*!
    Sets the altitude (meters above sea level) to \a altitude.

    \sa altitude()
*/
void QWhereaboutsCoordinate::setAltitude(double altitude)
{
    d->alt = altitude;
}

/*!
    Returns the distance (in meters) from this coordinate to the coordinate
    specified by \a other. Altitude is not used in the calculation.

    This calculation returns the great-circle distance between the two
    coordinates, without taking the aspects of the geoid into account.

    Returns 0 if the type of this coordinate or the type of \a other is
    QWhereaboutsCoordinate::InvalidCoordinate.
*/
qreal QWhereaboutsCoordinate::distanceTo(const QWhereaboutsCoordinate &other)
{
    if (type() == QWhereaboutsCoordinate::InvalidCoordinate
            || other.type() == QWhereaboutsCoordinate::InvalidCoordinate) {
        return 0;
    }

    // Haversine formula
    double dlat = qwhereaboutscoordinate_degToRad(other.d->lat - d->lat);
    double dlon = qwhereaboutscoordinate_degToRad(other.d->lng - d->lng);
    double y = sin(dlat/2) * sin(dlat/2)
            + cos(qwhereaboutscoordinate_degToRad(d->lat))
            * cos(qwhereaboutscoordinate_degToRad(other.d->lat))
            * sin(dlon/2) * sin(dlon/2);
    double x = 2 * atan2(sqrt(y), sqrt(1-y));
    return qreal(x * qwhereaboutscoordinate_EARTH_MEAN_RADIUS * 1000);
}

/*!
    Returns the azimuth (or bearing) in degrees from this coordinate to the
    coordinate specified by \a other. Altitude is not used in the calculation.

    The calculation does not take the aspects of the geoid into account.

    Returns 0 if the type of this coordinate or the type of \a other is
    QWhereaboutsCoordinate::InvalidCoordinate.
*/
qreal QWhereaboutsCoordinate::azimuthTo(const QWhereaboutsCoordinate &other)
{
    if (type() == QWhereaboutsCoordinate::InvalidCoordinate
            || other.type() == QWhereaboutsCoordinate::InvalidCoordinate) {
        return 0;
    }

    double dlon = qwhereaboutscoordinate_degToRad(other.d->lng - d->lng);
    double lat1Rad = qwhereaboutscoordinate_degToRad(d->lat);
    double lat2Rad = qwhereaboutscoordinate_degToRad(other.d->lat);

    double y = sin(dlon) * cos(lat2Rad);
    double x = cos(lat1Rad) * sin(lat2Rad) - sin(lat1Rad) * cos(lat2Rad) * cos(dlon);

    double whole;
    double fraction = modf(qwhereaboutscoordinate_radToDeg(atan2(y, x)), &whole);
    return qreal((int(whole + 360) % 360) + fraction);
}

/*!
    Returns this coordinate as a string in the specified \a format.

    For example, if this coordinate latitude-longitude coordinates of
    (-27.46758\unicode{0xB0}, 153.027892\unicode{0xB0}), these are the
    strings that are returned depending on \a format:

    \table
    \header
        \o \a format value
        \o Returned string
    \row
        \o \l DecimalDegrees
        \o -27.46758\unicode{0xB0}, 153.02789\unicode{0xB0}
    \row
        \o \l DecimalDegreesWithHemisphere
        \o 27.46758\unicode{0xB0} S, 153.02789\unicode{0xB0} E
    \row
        \o \l DegreesMinutes
        \o -27\unicode{0xB0} 28.054', 153\unicode{0xB0} 1.673'
    \row
        \o \l DegreesMinutesWithHemisphere
        \o 27\unicode{0xB0} 28.054 S', 153\unicode{0xB0} 1.673' E
    \row
        \o \l DegreesMinutesSeconds
        \o -27\unicode{0xB0} 28' 3.2", 153\unicode{0xB0} 1' 40.4"
    \row
        \o \l DegreesMinutesSecondsWithHemisphere
        \o 27\unicode{0xB0} 28' 3.2" S, 153\unicode{0xB0} 1' 40.4" E
    \endtable
*/
QString QWhereaboutsCoordinate::toString(CoordinateFormat format) const
{
    if (type() == QWhereaboutsCoordinate::InvalidCoordinate)
        return QObject::tr("<Invalid coordinate>");

    QString latStr;
    QString longStr;

    double absLat = qAbs(d->lat);
    double absLng = qAbs(d->lng);
    QChar symbol(0x00B0);   // degrees symbol

    switch (format) {
        case DecimalDegrees:
        case DecimalDegreesWithHemisphere:
        {
            latStr = QString::number(absLat, 'f', 5) + symbol;
            longStr = QString::number(absLng, 'f', 5) + symbol;
            break;
        }
        case DegreesMinutes:
        case DegreesMinutesWithHemisphere:
        {
            double latMin = (absLat - int(absLat)) * 60;
            double lngMin = (absLng - int(absLng)) * 60;
            latStr = QString("%1%2 %3'")
                    .arg(QString::number(int(absLat)))
                    .arg(symbol)
                    .arg(QString::number(latMin, 'f', 3));
            longStr = QString("%1%2 %3'")
                    .arg(QString::number(int(absLng)))
                    .arg(symbol)
                    .arg(QString::number(lngMin, 'f', 3));
            break;
        }
        case DegreesMinutesSeconds:
        case DegreesMinutesSecondsWithHemisphere:
        {
            double latMin = (absLat - int(absLat)) * 60;
            double lngMin = (absLng - int(absLng)) * 60;
            double latSec = (latMin - int(latMin)) * 60;
            double lngSec = (lngMin - int(lngMin)) * 60;

            latStr = QString("%1%2 %3' %4\"")
                    .arg(QString::number(int(absLat)))
                    .arg(symbol)
                    .arg(QString::number(int(latMin)))
                    .arg(QString::number(latSec, 'f', 1));
            longStr = QString("%1%2 %3' %4\"")
                    .arg(QString::number(int(absLng)))
                    .arg(symbol)
                    .arg(QString::number(int(lngMin)))
                    .arg(QString::number(lngSec, 'f', 1));
            break;
        }
    }

    // now add the "-" to the start, or append the hemisphere char
    switch (format) {
        case DecimalDegrees:
        case DegreesMinutes:
        case DegreesMinutesSeconds:
        {
            if (d->lat < 0)
                latStr.insert(0, "-");
            if (d->lng < 0)
                longStr.insert(0, "-");
            break;
        }
        case DecimalDegreesWithHemisphere:
        case DegreesMinutesWithHemisphere:
        case DegreesMinutesSecondsWithHemisphere:
        {
            if (d->lat < 0)
                latStr.append(" S");
            else if (d->lat > 0)
                latStr.append(" N");
            if (d->lng < 0)
                longStr.append(" W");
            else if (d->lng > 0)
                longStr.append(" E");
            break;
        }
    }

    return QString("%1, %2").arg(latStr, longStr);
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QWhereaboutsCoordinate &coord)
{
    dbg.nospace() << "QWhereaboutsCoordinate(" << coord.latitude();
    dbg.nospace() << ", ";
    dbg.nospace() << coord.longitude();
    dbg.nospace() << ", ";
    if (coord.type() == QWhereaboutsCoordinate::Coordinate3D)
        dbg.nospace() << coord.altitude();
    else
        dbg.nospace() << '?';
    dbg.nospace() << ')';
    return dbg;
}
#endif
