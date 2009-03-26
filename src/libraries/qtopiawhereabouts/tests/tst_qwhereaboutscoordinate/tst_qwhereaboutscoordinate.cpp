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

#include <QWhereaboutsCoordinate>
#include <qtest.h>

#include <QMetaType>
#include <QDebug>

Q_DECLARE_METATYPE(QWhereaboutsCoordinate)
Q_DECLARE_METATYPE(QWhereaboutsCoordinate::CoordinateFormat)

static const QWhereaboutsCoordinate BRISBANE(-27.46758, 153.027892);
static const QWhereaboutsCoordinate MELBOURNE(-37.814251, 144.963169);
static const QWhereaboutsCoordinate LONDON(51.500152, -0.126236);
static const QWhereaboutsCoordinate NEW_YORK(40.71453, -74.00713);
static const QWhereaboutsCoordinate NORTH_POLE(90, 0);
static const QWhereaboutsCoordinate SOUTH_POLE(-90, 0);

static const QChar DEGREES_SYMB(0x00B0);


class tst_QWhereaboutsCoordinate : public QObject
{
    Q_OBJECT

private slots:
    void type()
    {
        QWhereaboutsCoordinate c;
        QVERIFY(c.type() == QWhereaboutsCoordinate::InvalidCoordinate);

        c.setLatitude(1);
        QVERIFY(c.type() == QWhereaboutsCoordinate::InvalidCoordinate);

        c.setLongitude(1);
        QVERIFY(c.type() == QWhereaboutsCoordinate::Coordinate2D);

        c.setAltitude(1);
        QVERIFY(c.type() == QWhereaboutsCoordinate::Coordinate3D);
    }

    // TODO fix tests to test range (-90 -> 90 for lat, -180 -> 180 for long)
    void addDataValues()
    {
        QTest::addColumn<double>("value");
        QTest::newRow("negative") << -1.0;
        QTest::newRow("zero") << 0.0;
        QTest::newRow("positive") << 1.0;
    }

    void latitude_data() { addDataValues(); }
    void latitude()
    {
        QFETCH(double, value);
        QWhereaboutsCoordinate c;
        c.setLatitude(value);
        QCOMPARE(QString::number(c.latitude()), QString::number(value));

        QWhereaboutsCoordinate c2 = c;
        QCOMPARE(QString::number(c2.latitude()), QString::number(value));
    }

    void longitude_data() { addDataValues(); }
    void longitude()
    {
        QFETCH(double, value);
        QWhereaboutsCoordinate c;
        c.setLongitude(value);
        QCOMPARE(QString::number(c.longitude()), QString::number(value));

        QWhereaboutsCoordinate c2 = c;
        QCOMPARE(QString::number(c2.longitude()), QString::number(value));
    }

    void altitude_data() { addDataValues(); }
    void altitude()
    {
        QFETCH(double, value);
        QWhereaboutsCoordinate c;
        c.setAltitude(value);
        QCOMPARE(QString::number(c.altitude()), QString::number(value));

        QWhereaboutsCoordinate c2 = c;
        QCOMPARE(QString::number(c2.altitude()), QString::number(value));
    }

    void distanceTo_data()
    {
        QTest::addColumn<QWhereaboutsCoordinate>("c1");
        QTest::addColumn<QWhereaboutsCoordinate>("c2");
        QTest::addColumn<double>("distance");

        QTest::newRow("invalid coord 1")
                << QWhereaboutsCoordinate() << BRISBANE << 0.0;
        QTest::newRow("invalid coord 2")
                << BRISBANE << QWhereaboutsCoordinate() << 0.0;
        QTest::newRow("brisbane -> melbourne")
                << BRISBANE << MELBOURNE << 1374820.0;
        QTest::newRow("london -> new york")
                << LONDON << NEW_YORK << 5570540.0;
        QTest::newRow("north pole -> south pole")
                << NORTH_POLE << SOUTH_POLE << 20015100.0;
    }

    void distanceTo()
    {
        QFETCH(QWhereaboutsCoordinate, c1);
        QFETCH(QWhereaboutsCoordinate, c2);
        QFETCH(double, distance);

        QCOMPARE(QString::number(c1.distanceTo(c2)), QString::number(distance));
    }

    void azimuthTo_data()
    {
        QTest::addColumn<QWhereaboutsCoordinate>("c1");
        QTest::addColumn<QWhereaboutsCoordinate>("c2");
        QTest::addColumn<double>("azimuth");

        QTest::newRow("invalid coord 1")
                << QWhereaboutsCoordinate() << BRISBANE << 0.0;
        QTest::newRow("invalid coord 2")
                << BRISBANE << QWhereaboutsCoordinate() << 0.0;
        QTest::newRow("brisbane -> melbourne")
                << BRISBANE << MELBOURNE << 211.1717;
        QTest::newRow("london -> new york")
                << LONDON << NEW_YORK << 288.3389;
        QTest::newRow("north pole -> south pole")
                << NORTH_POLE << SOUTH_POLE << 180.0;
    }

    void azimuthTo()
    {
        QFETCH(QWhereaboutsCoordinate, c1);
        QFETCH(QWhereaboutsCoordinate, c2);
        QFETCH(double, azimuth);

        QCOMPARE(QString::number(c1.azimuthTo(c2)), QString::number(azimuth));
    }

    void degreesToString_data()
    {
        QTest::addColumn<QWhereaboutsCoordinate>("coord");
        QTest::addColumn<QWhereaboutsCoordinate::CoordinateFormat>("format");
        QTest::addColumn<QString>("string");

        QWhereaboutsCoordinate northEast(27.46758, 153.027892);
        QWhereaboutsCoordinate southEast(-27.46758, 153.027892);
        QWhereaboutsCoordinate northWest(27.46758, -153.027892);
        QWhereaboutsCoordinate southWest(-27.46758, -153.027892);

        QTest::newRow("NE, dd, no hemisphere")
                << northEast << QWhereaboutsCoordinate::DecimalDegrees
                << QString("27.46758%1, 153.02789%1").arg(DEGREES_SYMB);
        QTest::newRow("NE, dd, hemisphere")
                << northEast << QWhereaboutsCoordinate::DecimalDegreesWithHemisphere
                << QString("27.46758%1 N, 153.02789%1 E").arg(DEGREES_SYMB);
        QTest::newRow("NE, dm, no hemisphere")
                << northEast << QWhereaboutsCoordinate::DegreesMinutes
                << QString("27%1 28.055', 153%1 1.674'").arg(DEGREES_SYMB);
        QTest::newRow("NE, dm, hemisphere")
                << northEast << QWhereaboutsCoordinate::DegreesMinutesWithHemisphere
                << QString("27%1 28.055' N, 153%1 1.674' E").arg(DEGREES_SYMB);
        QTest::newRow("NE, dms, no hemisphere")
                << northEast << QWhereaboutsCoordinate::DegreesMinutesSeconds
                << QString("27%1 28' 3.3\", 153%1 1' 40.4\"").arg(DEGREES_SYMB);
        QTest::newRow("NE, dms, hemisphere")
                << northEast << QWhereaboutsCoordinate::DegreesMinutesSecondsWithHemisphere
                << QString("27%1 28' 3.3\" N, 153%1 1' 40.4\" E").arg(DEGREES_SYMB);

        QTest::newRow("SE, dd, no hemisphere")
                << southEast << QWhereaboutsCoordinate::DecimalDegrees
                << QString("-27.46758%1, 153.02789%1").arg(DEGREES_SYMB);
        QTest::newRow("SE, dd, hemisphere")
                << southEast << QWhereaboutsCoordinate::DecimalDegreesWithHemisphere
                << QString("27.46758%1 S, 153.02789%1 E").arg(DEGREES_SYMB);
        QTest::newRow("SE, dm, no hemisphere")
                << southEast << QWhereaboutsCoordinate::DegreesMinutes
                << QString("-27%1 28.055', 153%1 1.674'").arg(DEGREES_SYMB);
        QTest::newRow("SE, dm, hemisphere")
                << southEast << QWhereaboutsCoordinate::DegreesMinutesWithHemisphere
                << QString("27%1 28.055' S, 153%1 1.674' E").arg(DEGREES_SYMB);
        QTest::newRow("SE, dms, no hemisphere")
                << southEast << QWhereaboutsCoordinate::DegreesMinutesSeconds
                << QString("-27%1 28' 3.3\", 153%1 1' 40.4\"").arg(DEGREES_SYMB);
        QTest::newRow("SE, dms, hemisphere")
                << southEast << QWhereaboutsCoordinate::DegreesMinutesSecondsWithHemisphere
                << QString("27%1 28' 3.3\" S, 153%1 1' 40.4\" E").arg(DEGREES_SYMB);

        QTest::newRow("NE, dd, no hemisphere")
                << northEast << QWhereaboutsCoordinate::DecimalDegrees
                << QString("27.46758%1, 153.02789%1").arg(DEGREES_SYMB);
        QTest::newRow("NE, dd, hemisphere")
                << northEast << QWhereaboutsCoordinate::DecimalDegreesWithHemisphere
                << QString("27.46758%1 N, 153.02789%1 E").arg(DEGREES_SYMB);
        QTest::newRow("NE, dm, no hemisphere")
                << northEast << QWhereaboutsCoordinate::DegreesMinutes
                << QString("27%1 28.055', 153%1 1.674'").arg(DEGREES_SYMB);
        QTest::newRow("NE, dm, hemisphere")
                << northEast << QWhereaboutsCoordinate::DegreesMinutesWithHemisphere
                << QString("27%1 28.055' N, 153%1 1.674' E").arg(DEGREES_SYMB);
        QTest::newRow("NE, dms, no hemisphere")
                << northEast << QWhereaboutsCoordinate::DegreesMinutesSeconds
                << QString("27%1 28' 3.3\", 153%1 1' 40.4\"").arg(DEGREES_SYMB);
        QTest::newRow("NE, dms, hemisphere")
                << northEast << QWhereaboutsCoordinate::DegreesMinutesSecondsWithHemisphere
                << QString("27%1 28' 3.3\" N, 153%1 1' 40.4\" E").arg(DEGREES_SYMB);

        QTest::newRow("SE, dd, no hemisphere")
                << southEast << QWhereaboutsCoordinate::DecimalDegrees
                << QString("-27.46758%1, 153.02789%1").arg(DEGREES_SYMB);
        QTest::newRow("SE, dd, hemisphere")
                << southEast << QWhereaboutsCoordinate::DecimalDegreesWithHemisphere
                << QString("27.46758%1 S, 153.02789%1 E").arg(DEGREES_SYMB);
        QTest::newRow("SE, dm, no hemisphere")
                << southEast << QWhereaboutsCoordinate::DegreesMinutes
                << QString("-27%1 28.055', 153%1 1.674'").arg(DEGREES_SYMB);
        QTest::newRow("SE, dm, hemisphere")
                << southEast << QWhereaboutsCoordinate::DegreesMinutesWithHemisphere
                << QString("27%1 28.055' S, 153%1 1.674' E").arg(DEGREES_SYMB);
        QTest::newRow("SE, dms, no hemisphere")
                << southEast << QWhereaboutsCoordinate::DegreesMinutesSeconds
                << QString("-27%1 28' 3.3\", 153%1 1' 40.4\"").arg(DEGREES_SYMB);
        QTest::newRow("SE, dms, hemisphere")
                << southEast << QWhereaboutsCoordinate::DegreesMinutesSecondsWithHemisphere
                << QString("27%1 28' 3.3\" S, 153%1 1' 40.4\" E").arg(DEGREES_SYMB);

        QTest::newRow("NW, dd, no hemisphere")
                << northWest << QWhereaboutsCoordinate::DecimalDegrees
                << QString("27.46758%1, -153.02789%1").arg(DEGREES_SYMB);
        QTest::newRow("NW, dd, hemisphere")
                << northWest << QWhereaboutsCoordinate::DecimalDegreesWithHemisphere
                << QString("27.46758%1 N, 153.02789%1 W").arg(DEGREES_SYMB);
        QTest::newRow("NW, dm, no hemisphere")
                << northWest << QWhereaboutsCoordinate::DegreesMinutes
                << QString("27%1 28.055', -153%1 1.674'").arg(DEGREES_SYMB);
        QTest::newRow("NW, dm, hemisphere")
                << northWest << QWhereaboutsCoordinate::DegreesMinutesWithHemisphere
                << QString("27%1 28.055' N, 153%1 1.674' W").arg(DEGREES_SYMB);
        QTest::newRow("NW, dms, no hemisphere")
                << northWest << QWhereaboutsCoordinate::DegreesMinutesSeconds
                << QString("27%1 28' 3.3\", -153%1 1' 40.4\"").arg(DEGREES_SYMB);
        QTest::newRow("NW, dms, hemisphere")
                << northWest << QWhereaboutsCoordinate::DegreesMinutesSecondsWithHemisphere
                << QString("27%1 28' 3.3\" N, 153%1 1' 40.4\" W").arg(DEGREES_SYMB);

        QTest::newRow("SW, dd, no hemisphere")
                << southWest << QWhereaboutsCoordinate::DecimalDegrees
                << QString("-27.46758%1, -153.02789%1").arg(DEGREES_SYMB);
        QTest::newRow("SW, dd, hemisphere")
                << southWest << QWhereaboutsCoordinate::DecimalDegreesWithHemisphere
                << QString("27.46758%1 S, 153.02789%1 W").arg(DEGREES_SYMB);
        QTest::newRow("SW, dm, no hemisphere")
                << southWest << QWhereaboutsCoordinate::DegreesMinutes
                << QString("-27%1 28.055', -153%1 1.674'").arg(DEGREES_SYMB);
        QTest::newRow("SW, dm, hemisphere")
                << southWest << QWhereaboutsCoordinate::DegreesMinutesWithHemisphere
                << QString("27%1 28.055' S, 153%1 1.674' W").arg(DEGREES_SYMB);
        QTest::newRow("SW, dms, no hemisphere")
                << southWest << QWhereaboutsCoordinate::DegreesMinutesSeconds
                << QString("-27%1 28' 3.3\", -153%1 1' 40.4\"").arg(DEGREES_SYMB);
        QTest::newRow("SW, dms, hemisphere")
                << southWest << QWhereaboutsCoordinate::DegreesMinutesSecondsWithHemisphere
                << QString("27%1 28' 3.3\" S, 153%1 1' 40.4\" W").arg(DEGREES_SYMB);
    }

    void degreesToString()
    {
        QFETCH(QWhereaboutsCoordinate, coord);
        QFETCH(QWhereaboutsCoordinate::CoordinateFormat, format);
        QFETCH(QString, string);

        QCOMPARE(coord.toString(format), string);
    }

};

QTEST_MAIN(tst_QWhereaboutsCoordinate)
#include "tst_qwhereaboutscoordinate.moc"
