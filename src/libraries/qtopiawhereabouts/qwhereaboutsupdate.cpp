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

#include "qwhereaboutsupdate.h"

#include <QHash>
#include <QDebug>
#include <QRegExp>

/*!
    \class QWhereaboutsUpdate
    \inpublicgroup QtLocationModule
    \ingroup whereabouts
    \brief The QWhereaboutsUpdate class contains the global position and other related information received at a particular point in time.

    A QWhereaboutsUpdate object has, at a minimum, a coordinate value and
    a timestamp. It may also have course and speed measurements and
    estimates of the accuracy of the provided data.

    Use dataValidityFlags() to determine whether values have been set for
    a particular data type.

    \sa QWhereabouts, QWhereaboutsCoordinate
*/

/*!
    \enum QWhereaboutsUpdate::DataType
    Defines the types of data that may be included in an update.

    \value GroundSpeed The speed over ground.
    \value VerticalSpeed The vertical speed.
    \value Course The course/track made good (i.e. bearing to true north).
    \value HorizontalAccuracy The horizontal accuracy.
    \value VerticalAccuracy The vertical accuracy
    \value GroundSpeedAccuracy Accuracy of the speed over ground value.
    \value VerticalSpeedAccuracy Accuracy over the vertical speed value.
    \value CourseAccuracy Accuracy of the course/track made good value.
    \value UpdateTimeAccuracy Accuracy of the update time value.

    \sa dataValidityFlags()
*/

/*!
    \enum QWhereaboutsUpdate::PositionFixStatus
    Defines the available position fix statuses for a position update. A global position cannot be accurately determined until a position fix is acquired.

    \value FixStatusUnknown The position fix status cannot be determined from the position update.
    \value FixNotAcquired No position fix has been acquired at the time of the update.
    \value FixAcquired A position fix has been acquired at the time of the update.
*/

class QWhereaboutsUpdatePrivate
{
public:
    QWhereaboutsUpdate::DataTypes flags;
    QWhereaboutsCoordinate coord;
    QDateTime dateTime;
    QHash<int, qreal> values;
};

/*!
    Constructs a null update.
*/
QWhereaboutsUpdate::QWhereaboutsUpdate()
    : d(new QWhereaboutsUpdatePrivate)
{
    d->flags = 0;
}

/*!
    Constructs an update with the given \a coordinate and \a dateTime.
*/
QWhereaboutsUpdate::QWhereaboutsUpdate(const QWhereaboutsCoordinate &coordinate, const QDateTime &dateTime)
    : d(new QWhereaboutsUpdatePrivate)
{
    d->flags = 0;
    d->coord = coordinate;
    d->dateTime = dateTime;
}

/*!
    Constructs an update from the contents of \a other.
*/
QWhereaboutsUpdate::QWhereaboutsUpdate(const QWhereaboutsUpdate &other)
    : d(new QWhereaboutsUpdatePrivate)
{
    operator=(other);
}

/*!
    Destroys an update.
*/
QWhereaboutsUpdate::~QWhereaboutsUpdate()
{
    delete d;
}

/*!
    Assigns \a other to this update and returns a reference to this update.
*/
QWhereaboutsUpdate &QWhereaboutsUpdate::operator=(const QWhereaboutsUpdate &other)
{
    if (this == &other)
        return *this;

    d->flags = other.d->flags;
    d->coord = other.d->coord;
    d->dateTime = other.d->dateTime;
    d->values = other.d->values;

    return *this;
}

/*!
    Returns \c true if no attributes are set for this update.
*/
bool QWhereaboutsUpdate::isNull() const
{
    return (d->coord.isNull() && d->dateTime.isNull() && d->values.isEmpty());
}

/*!
    Returns a set of flags that indicate whether data has been set for
    a particular update attribute.
*/
QWhereaboutsUpdate::DataTypes QWhereaboutsUpdate::dataValidityFlags() const
{
    return d->flags;
}

/*!
    Clears all attribute values for this update.
*/
void QWhereaboutsUpdate::clear()
{
    d->flags = 0;
    d->coord = QWhereaboutsCoordinate();
    d->dateTime = QDateTime();
    d->values.clear();
}

/*!
    Returns the coordinate for this update.

    \sa setCoordinate()
*/
QWhereaboutsCoordinate QWhereaboutsUpdate::coordinate() const
{
    return d->coord;
}

/*!
    Sets the coordinate for this update to \a coordinate.

    \sa coordinate()
*/
void QWhereaboutsUpdate::setCoordinate(const QWhereaboutsCoordinate &coordinate)
{
    d->coord = coordinate;
}

/*!
    Sets the date and time at which this update was received to \a dateTime.

    \sa updateDate()
*/
void QWhereaboutsUpdate::setUpdateDateTime(const QDateTime &dateTime) const
{
    d->dateTime = dateTime;
}

/*!
    Returns the date and time at which this update was received.

    Returns an invalid value if the updateDate() or updateTime() is not
    valid.

    \sa setUpdateDate()
*/
QDateTime QWhereaboutsUpdate::updateDateTime() const
{
    return d->dateTime;
}

/*!
    Sets the date on which this update was received to \a date.

    \sa updateDate()
*/
void QWhereaboutsUpdate::setUpdateDate(const QDate &date)
{
    d->dateTime.setDate(date);
}

/*!
    Returns the date on which this update was received.

    \sa setUpdateDate()
*/
QDate QWhereaboutsUpdate::updateDate() const
{
    return d->dateTime.date();
}

/*!
    Sets the time at which this update was received to \a time, which
    should be in UTC time.

    \sa updateTime()
*/
void QWhereaboutsUpdate::setUpdateTime(const QTime &time)
{
    d->dateTime.setTime(time);
}

/*!
    Returns the time at which this update was received.

    \sa setUpdateTime()
*/
QTime QWhereaboutsUpdate::updateTime() const
{
    return d->dateTime.time();
}

/*!
    Returns the speed over ground, in meters/sec.

    Use dataValidityFlags() to check whether this value has been set.

    \sa setGroundSpeed()
*/
qreal QWhereaboutsUpdate::groundSpeed() const
{
    return d->values.value(QWhereaboutsUpdate::GroundSpeed);
}

/*!
    Sets the speed over ground (in meters/sec) to \a speed.

    \sa groundSpeed()
*/
void QWhereaboutsUpdate::setGroundSpeed(qreal speed)
{
    d->flags |= QWhereaboutsUpdate::GroundSpeed;
    d->values[QWhereaboutsUpdate::GroundSpeed] = speed;
}

/*!
    Returns the vertical speed, in meters/sec.

    Use dataValidityFlags() to check whether this value has been set.

    \sa setVerticalSpeed()
*/
qreal QWhereaboutsUpdate::verticalSpeed() const
{
    return d->values.value(QWhereaboutsUpdate::VerticalSpeed);
}

/*!
    Sets the vertical speed (in meters/sec) to \a speed.

    \sa verticalSpeed()
*/
void QWhereaboutsUpdate::setVerticalSpeed(qreal speed)
{
    d->flags |= QWhereaboutsUpdate::VerticalSpeed;
    d->values[QWhereaboutsUpdate::VerticalSpeed] = speed;
}

/*!
    Returns the course (i.e. bearing to true north, in degrees).

    Use dataValidityFlags() to check whether this value has been set.

    \sa setCourse()
*/
qreal QWhereaboutsUpdate::course() const
{
    return d->values.value(QWhereaboutsUpdate::Course);
}

/*!
    Sets the course (i.e. bearing to true north, in degrees) to
    \a course.

    \sa course()
*/
void QWhereaboutsUpdate::setCourse(qreal course)
{
    d->flags |= QWhereaboutsUpdate::Course;
    d->values[QWhereaboutsUpdate::Course] = course;
}

/*!
    Returns the estimated accuracy of the updateTime() value (in seconds).

    Use dataValidityFlags() to check whether this value has been set.
*/
qreal QWhereaboutsUpdate::updateTimeAccuracy() const
{
    return d->values.value(QWhereaboutsUpdate::UpdateTimeAccuracy);
}

/*!
    Sets the estimated accuracy of the updateTime() value (in seconds)
    to \a accuracy.
*/
void QWhereaboutsUpdate::setUpdateTimeAccuracy(qreal accuracy)
{
    d->flags |= QWhereaboutsUpdate::UpdateTimeAccuracy;
    d->values[QWhereaboutsUpdate::UpdateTimeAccuracy] = accuracy;
}

/*!
    Returns the estimated horizontal accuracy (in meters).

    Use dataValidityFlags() to check whether this value has been set.
*/
qreal QWhereaboutsUpdate::horizontalAccuracy() const
{
    return d->values.value(QWhereaboutsUpdate::HorizontalAccuracy);
}

/*!
    Sets the estimated horizontal accuracy (in meters) to \a accuracy.
*/
void QWhereaboutsUpdate::setHorizontalAccuracy(qreal accuracy)
{
    d->flags |= QWhereaboutsUpdate::HorizontalAccuracy;
    d->values[QWhereaboutsUpdate::HorizontalAccuracy] = accuracy;
}

/*!
    Returns the estimated vertical accuracy (in meters).

    Use dataValidityFlags() to check whether this value has been set.
*/
qreal QWhereaboutsUpdate::verticalAccuracy() const
{
    return d->values.value(QWhereaboutsUpdate::VerticalAccuracy);
}

/*!
    Sets the estimated vertical accuracy (in meters) to \a accuracy.
*/
void QWhereaboutsUpdate::setVerticalAccuracy(qreal accuracy)
{
    d->flags |= QWhereaboutsUpdate::VerticalAccuracy;
    d->values[QWhereaboutsUpdate::VerticalAccuracy] = accuracy;
}

/*!
    Returns the estimated accuracy of the groundSpeed() value (in meters/sec).

    Use dataValidityFlags() to check whether this value has been set.
*/
qreal QWhereaboutsUpdate::groundSpeedAccuracy() const
{
    return d->values.value(QWhereaboutsUpdate::GroundSpeedAccuracy);
}

/*!
    Sets the estimated accuracy of the groundSpeed() value (in meters/sec)
    to \a accuracy.
*/
void QWhereaboutsUpdate::setGroundSpeedAccuracy(qreal accuracy)
{
    d->flags |= QWhereaboutsUpdate::GroundSpeedAccuracy;
    d->values[QWhereaboutsUpdate::GroundSpeedAccuracy] = accuracy;
}

/*!
    Returns the estimated accuracy of the verticalSpeed() value
    (in meters/sec).

    Use dataValidityFlags() to check whether this value has been set.
*/
qreal QWhereaboutsUpdate::verticalSpeedAccuracy() const
{
    return d->values.value(QWhereaboutsUpdate::VerticalSpeedAccuracy);
}

/*!
    Sets the estimated accuracy of the verticalSpeed() value (in meters/sec)
    to \a accuracy.
*/
void QWhereaboutsUpdate::setVerticalSpeedAccuracy(qreal accuracy)
{
    d->flags |= QWhereaboutsUpdate::VerticalSpeedAccuracy;
    d->values[QWhereaboutsUpdate::VerticalSpeedAccuracy] = accuracy;
}

/*!
    Returns the estimated accuracy of the course() value (in degrees).

    Use dataValidityFlags() to check whether this value has been set.
*/
qreal QWhereaboutsUpdate::courseAccuracy() const
{
    return d->values.value(QWhereaboutsUpdate::CourseAccuracy);
}

/*!
    Sets the estimated accuracy of the course() value (in degrees)
    to \a accuracy.
*/
void QWhereaboutsUpdate::setCourseAccuracy(qreal accuracy)
{
    d->flags |= QWhereaboutsUpdate::CourseAccuracy;
    d->values[QWhereaboutsUpdate::CourseAccuracy] = accuracy;
}

// converts e.g. 15306.0235 from GGA sentence to 153.100392
static double qwhereaboutsupdate_nmeaDegreesToDecimal(double nmeaDegrees)
{
    int degrees = int(nmeaDegrees/100);
    double minutes = nmeaDegrees - 100*degrees;
    return (degrees + minutes/60);
}

static void qwhereaboutsupdate_getUpdateCoordinate(QWhereaboutsCoordinate *coord, const QByteArray &latStr, const QByteArray &latHmsph, const QByteArray &lngStr, const QByteArray &lngHmsph, const QByteArray &altStr = QByteArray())
{
    bool hasLat = false;
    bool hasLong = false;
    double lat = qwhereaboutsupdate_nmeaDegreesToDecimal(latStr.toDouble(&hasLat));
    double lng = qwhereaboutsupdate_nmeaDegreesToDecimal(lngStr.toDouble(&hasLong));
    if (hasLat && hasLong) {
        if (latHmsph == "S")
            lat *= -1;
        if (lngHmsph == "W")
            lng *= -1;
        bool hasAlt = false;
        double altitude = altStr.toDouble(&hasAlt);
        if (hasAlt)
            *coord = QWhereaboutsCoordinate(lat, lng, altitude);
        else
            *coord = QWhereaboutsCoordinate(lat, lng);
    }
}

static QWhereaboutsUpdate::PositionFixStatus qwhereaboutsupdate_fixStatusFromValidityFlag(const QByteArray &flag)
{
    if (flag.size() > 0) {
        if (flag[0] == 'V')
            return QWhereaboutsUpdate::FixNotAcquired;
        if (flag[0] == 'A')
            return QWhereaboutsUpdate::FixAcquired;
    }
    return QWhereaboutsUpdate::FixStatusUnknown;
}

static void qwhereaboutsupdate_getTimeString(const QByteArray &bytes, QTime *time)
{
    int dotIndex = bytes.indexOf('.');
    if (dotIndex < 0) {
        *time = QTime::fromString(bytes, "hhmmss");
    } else {
        *time = QTime::fromString(bytes.mid(0, dotIndex), "hhmmss");
        bool hasMsecs = false;
        int msecs = bytes.mid(dotIndex+1).toUInt(&hasMsecs);
        if (hasMsecs)
            *time = time->addMSecs(msecs);
    }
}

static void qwhereaboutsupdate_readGgaSentence(const QByteArray &sentence, QWhereaboutsUpdate *update, QWhereaboutsUpdate::PositionFixStatus *fixStatus)
{
    QList<QByteArray> parts = sentence.split(',');

    if (parts.count() > 1) {
        QTime time;
        qwhereaboutsupdate_getTimeString(parts[1], &time);
        update->setUpdateTime(time);
    }

    if (parts.count() > 10) {
        QWhereaboutsCoordinate coord;
        qwhereaboutsupdate_getUpdateCoordinate(&coord, parts[2], parts[3], parts[4], parts[5], parts[9]);
        if (coord.type() != QWhereaboutsCoordinate::InvalidCoordinate)
            update->setCoordinate(coord);
    }

    if (fixStatus && parts.count() > 6) {
        bool fixOk = false;
        int fixQuality = parts[6].toInt(&fixOk);
        if (fixOk) {
            if (fixQuality == 0)
                *fixStatus = QWhereaboutsUpdate::FixNotAcquired;
            else
                *fixStatus = QWhereaboutsUpdate::FixAcquired;
        } else {
            *fixStatus = QWhereaboutsUpdate::FixStatusUnknown;
        }
    }
}

static void qwhereaboutsupdate_readGllSentence(const QByteArray &sentence, QWhereaboutsUpdate *update, QWhereaboutsUpdate::PositionFixStatus *fixStatus)
{
    QList<QByteArray> parts = sentence.split(',');

    if (parts.count() > 5) {
        QTime time;
        qwhereaboutsupdate_getTimeString(parts[5], &time);
        update->setUpdateTime(time);
    }

    if (parts.count() > 4) {
        QWhereaboutsCoordinate coord;
        qwhereaboutsupdate_getUpdateCoordinate(&coord, parts[1], parts[2], parts[3], parts[4]);
        if (coord.type() != QWhereaboutsCoordinate::InvalidCoordinate)
            update->setCoordinate(coord);
    }

    if (fixStatus && parts.count() > 6)
        *fixStatus = qwhereaboutsupdate_fixStatusFromValidityFlag(parts[6]);
}

static void qwhereaboutsupdate_readRmcSentence(const QByteArray &sentence, QWhereaboutsUpdate *update, QWhereaboutsUpdate::PositionFixStatus *fixStatus)
{
    QList<QByteArray> parts = sentence.split(',');

    if (parts.count() > 9) {
        QTime time;
        qwhereaboutsupdate_getTimeString(parts[1], &time);
        update->setUpdateTime(time);
        if (time.isValid()) {
            QDate date = QDate::fromString(parts[9], "ddMMyy");
            date = date.addYears(100);     // otherwise starts from 1900
            update->setUpdateDateTime(QDateTime(date, time, Qt::UTC));
        }
    }

    if (parts.count() > 6) {
        QWhereaboutsCoordinate coord;
        qwhereaboutsupdate_getUpdateCoordinate(&coord, parts[3], parts[4], parts[5], parts[6]);
        if (coord.type() != QWhereaboutsCoordinate::InvalidCoordinate)
            update->setCoordinate(coord);
    }

    if (parts.count() > 7) {
        bool hasSpeed = false;
        float speed = parts[7].toFloat(&hasSpeed);
        if (hasSpeed)
            update->setGroundSpeed(speed * 1.852 / 3.6);
    }

    if (parts.count() > 8) {
        bool hasCourse = false;
        float course = parts[8].toFloat(&hasCourse);
        if (hasCourse)
            update->setCourse(course);
    }

    if (fixStatus && parts.count() > 2)
        *fixStatus = qwhereaboutsupdate_fixStatusFromValidityFlag(parts[2]);
}

static void qwhereaboutsupdate_readVtgSentence(const QByteArray &sentence, QWhereaboutsUpdate *update, QWhereaboutsUpdate::PositionFixStatus *fixStatus)
{
    QList<QByteArray> parts = sentence.split(',');

    bool ok = false;
    float value = 0.0;
    if (parts.count() > 1) {
        value = parts[1].toFloat(&ok);
        if (ok)
            update->setCourse(value);
    }
    if (parts.count() > 7) {
        value = parts[7].toFloat(&ok);
        if (ok)
            update->setGroundSpeed(value / 3.6);
    }

    if (fixStatus)
        *fixStatus = QWhereaboutsUpdate::FixStatusUnknown;
}

static void qwhereaboutsupdate_readZdaSentence(const QByteArray &sentence, QWhereaboutsUpdate *update, QWhereaboutsUpdate::PositionFixStatus *fixStatus)
{
    QList<QByteArray> parts = sentence.split(',');

    QTime time;
    if (parts.count() > 1)
        qwhereaboutsupdate_getTimeString(parts[1], &time);

    QDate date;
    if (parts.count() > 4) {
        int day = parts[2].toUInt();
        int month = parts[3].toUInt();
        int year = parts[4].toUInt();
        if (day > 0 && month > 0 && year > 0)
            date.setDate(year, month, day);
    }

    update->setUpdateDateTime(QDateTime(date, time, Qt::UTC));
    if (fixStatus)
        *fixStatus = QWhereaboutsUpdate::FixStatusUnknown;
}

static bool qwhereaboutsupdate_isChecksumValid(const QByteArray &sentence)
{
    int asteriskIndex = sentence.indexOf('*');
    if (asteriskIndex < 0 || !sentence.startsWith('$'))
        return false;

    // XOR byte value of all characters between '$' and '*'
    int result = 0;
    for (int i=1; i<asteriskIndex; i++)
        result ^= sentence[i];

    QString checksum;
    checksum.sprintf("%02x", result);
    return (checksum == sentence.mid(asteriskIndex + 1).trimmed());
}

/*!
    Returns the parsed form of the NMEA data in \a nmea and sets \a fixStatus
    according to the parsed data. Returns a null update if \a nmea could not
    be parsed, or if it has an invalid checksum.

    This function is able to parse \c GGA, \c GLL, \c RMC, \c VTG and \c ZDA
    sentences.

    Some points to note:
    \list
    \o If \a nmea contains a date that has a year component in two-digit form,
    it is assumed that it refers to a year after the year 2000, and the
    updateDate() of the returned upate is set accordingly.
    \o The returned update may not have valid date and/or time values as some
    sentences do not have this information. \c GGA and \c GLL sentences do not
    have date information, and \c VTG sentences have neither date nor time
    information.
    \endlist
*/
QWhereaboutsUpdate QWhereaboutsUpdate::fromNmea(const QByteArray &nmea, PositionFixStatus *fixStatus)
{
    int posn = 0;
    QWhereaboutsUpdate update;
    while (posn < nmea.length()) {
        // Extract the next line from the NMEA data.
        int end = nmea.indexOf('\n', posn);
        if (end < 0)
            end = nmea.length();
        int len = end - posn;
        if (len > 0 && nmea[posn + len - 1] == '\r')
            --len;
        QByteArray line = nmea.mid(posn, len);
        posn = end + 1;

        // Determine how to parse the sentence.
        if (line.length() < 6 || line[0] != '$')
            continue;

        if (!qwhereaboutsupdate_isChecksumValid(nmea))
            return QWhereaboutsUpdate();

        if (line[3] == 'G' && line[4] == 'G' && line[5] == 'A') {
            // "$--GGA" sentence.
            qwhereaboutsupdate_readGgaSentence(line, &update, fixStatus);
        } else if (line[3] == 'G' && line[4] == 'L' && line[5] == 'L') {
            // "$--GLL" sentence.
            qwhereaboutsupdate_readGllSentence(line, &update, fixStatus);
        } else if (line[3] == 'R' && line[4] == 'M' && line[5] == 'C') {
            // "$--RMC" sentence.
            qwhereaboutsupdate_readRmcSentence(line, &update, fixStatus);
        } else if (line[3] == 'V' && line[4] == 'T' && line[5] == 'G') {
            // "$--VTG" sentence.
            qwhereaboutsupdate_readVtgSentence(line, &update, fixStatus);
        } else if (line[3] == 'Z' && line[4] == 'D' && line[5] == 'A') {
            // "$--ZDA" sentence.
            qwhereaboutsupdate_readZdaSentence(line, &update, fixStatus);
        }
    }
    return update;
}


#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QWhereaboutsUpdate &update)
{
    QWhereaboutsCoordinate coord = update.coordinate();
    dbg.nospace() << "QWhereaboutsUpdate("
            << update.updateDateTime().toString("yy/MM/dd hh:mm:ss").toLatin1().constData()
            << " ("
            << coord.latitude()
            << ", " << coord.longitude()
            << ", ";
    if (coord.type() == QWhereaboutsCoordinate::Coordinate3D)
        dbg.nospace() << coord.altitude();
    else
        dbg.nospace() << '?';
    dbg.nospace() << "))";
    return dbg.space();
}
#endif
