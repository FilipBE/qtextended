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

#include <QWhereaboutsUpdate>
#include <qtest.h>

#include <QMetaType>
#include <QDebug>

Q_DECLARE_METATYPE(QWhereaboutsUpdate)
Q_DECLARE_METATYPE(QWhereaboutsUpdate::PositionFixStatus)



class tst_QWhereaboutsUpdate : public QObject
{
    Q_OBJECT

private:

    #define Q_COMPARE_IF_VALID(x, y) if (x.isValid()) QCOMPARE(x, y); else QCOMPARE(x.isValid(), y.isValid());

    void compareUpdates(const QWhereaboutsUpdate &u1, const QWhereaboutsUpdate &u2)
    {
        QCOMPARE(u1.dataValidityFlags(), u2.dataValidityFlags());

        Q_COMPARE_IF_VALID(u1.updateDateTime(), u2.updateDateTime());
        Q_COMPARE_IF_VALID(u1.updateDate(), u2.updateDate());
        Q_COMPARE_IF_VALID(u1.updateTime(), u2.updateTime());

        QCOMPARE(QString::number(u1.groundSpeed()), QString::number(u2.groundSpeed()));
        QCOMPARE(u1.verticalSpeed(), u2.verticalSpeed());
        QCOMPARE(u1.updateTimeAccuracy(), u2.updateTimeAccuracy());
        QCOMPARE(u1.horizontalAccuracy(), u2.horizontalAccuracy());
        QCOMPARE(u1.groundSpeedAccuracy(), u2.groundSpeedAccuracy());
        QCOMPARE(u1.courseAccuracy(), u2.courseAccuracy());

        QCOMPARE(QString::number(u1.coordinate().latitude()),
                 QString::number(u2.coordinate().latitude()));
        QCOMPARE(QString::number(u1.coordinate().longitude()),
                 QString::number(u2.coordinate().longitude()));
        QCOMPARE(QString::number(u1.coordinate().altitude()),
                 QString::number(u2.coordinate().altitude()));

        QCOMPARE(u1.isNull(), u2.isNull());
    }

    // assumes sentence starts with '$' and ends with '*'
    QByteArray addChecksum(const QByteArray &sentence)
    {
        // XOR byte value of all characters between '$' and '*'
        int result = 0;
        for (int i=1; i<sentence.length()-1; i++)
            result ^= sentence[i];

        QString sum;
        sum.sprintf("%02x", result);
        return sentence + sum.toAscii();
    }

private slots:
    void initialState()
    {
        QWhereaboutsUpdate u;
        QVERIFY(u.isNull());
    }

    void setCoordinate()
    {
        QWhereaboutsCoordinate c(1, 1, 1);
        QWhereaboutsUpdate u;
        u.setCoordinate(c);

        QWhereaboutsCoordinate c2 = u.coordinate();
        QCOMPARE(QString::number(c.distanceTo(c2)), QString("0"));
        QCOMPARE(QString::number(c2.altitude()),
                 QString::number(c2.altitude()));
    }

    void fromNmea_data()
    {
        QByteArray gga_bad("$GPGGA,*");
        QByteArray gga_empty("$GPGGA,,,,,,,,,,,,,,*");
        QByteArray gga("$GPGGA,060613.626,2734.7964,S,15306.0124,E,1,03,2.9,-26.8,M,36.8,M,1,0000*");
        QWhereaboutsUpdate ggaUpdate;
        ggaUpdate.setCoordinate(QWhereaboutsCoordinate(-27.579940, 153.100207, -26.8));
        ggaUpdate.setUpdateTime(QTime(6, 6, 13, 626));

        QByteArray gll_bad("$GGGLL,*");
        QByteArray gll_empty("$GGGLL,,,,,,,,*");
        QByteArray gll("$GGGLL,2734.7964,S,15306.0124,E,220125.999,A,,*");
        QWhereaboutsUpdate gllUpdate;
        gllUpdate.setCoordinate(QWhereaboutsCoordinate(-27.579940, 153.100207));
        gllUpdate.setUpdateTime(QTime(22, 1, 25, 999));

        QByteArray rmc_bad("$GPRMC,*");
        QByteArray rmc_empty("$GPRMC,,,,,,,,,,,,*");
        QByteArray rmc("$GPRMC,220125.999,A,2734.7964,S,15306.0124,E,8.9,47.6,030408,11.2,W,A*");
        QWhereaboutsUpdate rmcUpdate;
        rmcUpdate.setCoordinate(QWhereaboutsCoordinate(-27.579940, 153.100207));
        rmcUpdate.setUpdateDateTime(QDateTime(QDate(2008, 4, 3), QTime(22, 1, 25, 999), Qt::UTC));
        rmcUpdate.setGroundSpeed(8.9 * 1.852 / 3.6);
        rmcUpdate.setCourse(47.6);

        QByteArray vtg_bad("$GPVTG,*");
        QByteArray vtg_empty("$GPVTG,,,,,,,,,*");
        QByteArray vtg("$GPVTG,158.7,T,169.9,M,33.2,N,61.5,K,A*");
        QWhereaboutsUpdate vtgUpdate;
        vtgUpdate.setCourse(158.7);
        vtgUpdate.setGroundSpeed(61.5 / 3.6);

        QByteArray zda_bad("$GPZDA,*");
        QByteArray zda_empty("$GPZDA,,,,,,,,,*");
        QByteArray zda("$GPZDA,052953.05,03,04,2008,00,00*");
        QWhereaboutsUpdate zdaUpdate;
        zdaUpdate.setUpdateDateTime(QDateTime(QDate(2008, 4, 3), QTime(5, 29, 53, 5), Qt::UTC));

        QTest::addColumn<QByteArray>("sentence");
        QTest::addColumn<QWhereaboutsUpdate>("update");

        QTest::newRow("GGA, bad") << addChecksum(gga_bad) << QWhereaboutsUpdate();
        QTest::newRow("GGA, empty") << addChecksum(gga_empty) << QWhereaboutsUpdate();
        QTest::newRow("GGA") << addChecksum(gga) << ggaUpdate;

        QTest::newRow("GLL, bad") << addChecksum(gll_bad) << QWhereaboutsUpdate();
        QTest::newRow("GLL, empty") << addChecksum(gll_empty) << QWhereaboutsUpdate();
        QTest::newRow("GLL") << addChecksum(gll) << gllUpdate;

        QTest::newRow("RMC, bad") << addChecksum(rmc_bad) << QWhereaboutsUpdate();
        QTest::newRow("RMC, empty") << addChecksum(rmc_empty) << QWhereaboutsUpdate();
        QTest::newRow("RMC") << addChecksum(rmc) << rmcUpdate;

        QTest::newRow("VTG, bad") << addChecksum(vtg_bad) << QWhereaboutsUpdate();
        QTest::newRow("VTG, empty") << addChecksum(vtg_empty) << QWhereaboutsUpdate();
        QTest::newRow("VTG") << addChecksum(vtg) << vtgUpdate;

        QTest::newRow("ZDA, bad") << addChecksum(zda_bad) << QWhereaboutsUpdate();
        QTest::newRow("ZDA, empty") << addChecksum(zda_empty) << QWhereaboutsUpdate();
        QTest::newRow("ZDA") << addChecksum(zda) << zdaUpdate;
    }

    void fromNmea()
    {
        QFETCH(QByteArray, sentence);
        QFETCH(QWhereaboutsUpdate, update);

        compareUpdates(QWhereaboutsUpdate::fromNmea(sentence), update);
    }

    void fromNmea_fixStatus_data()
    {
        QTest::addColumn<QByteArray>("sentence");
        QTest::addColumn<QWhereaboutsUpdate::PositionFixStatus>("fixStatus");

        QTest::newRow("GGA fix unknown") << addChecksum("$GPGGA,,,,,,,,,,,,,,*") << QWhereaboutsUpdate::FixStatusUnknown;
        QTest::newRow("GGA no fix") << addChecksum("$GPGGA,,,,,,0,,,,,,,,*") << QWhereaboutsUpdate::FixNotAcquired;
        QTest::newRow("GGA fix") << addChecksum("$GPGGA,,,,,,1,,,,,,,,*") << QWhereaboutsUpdate::FixAcquired;

        QTest::newRow("GGL fix unknown") << addChecksum("$GGGLL,,,,,,,,*") << QWhereaboutsUpdate::FixStatusUnknown;
        QTest::newRow("GGL no fix") << addChecksum("$GGGLL,,,,,,V,,*") << QWhereaboutsUpdate::FixNotAcquired;
        QTest::newRow("GGL fix") << addChecksum("$GGGLL,,,,,,A,,*") << QWhereaboutsUpdate::FixAcquired;

        QTest::newRow("RMC fix unknown") << addChecksum("$GPRMC,,,,,,,,,,,,*") << QWhereaboutsUpdate::FixStatusUnknown;
        QTest::newRow("RMC no fix") << addChecksum("$GPRMC,,V,,,,,,,,,,*") << QWhereaboutsUpdate::FixNotAcquired;
        QTest::newRow("RMC fix") << addChecksum("$GPRMC,,A,,,,,,,,,,*") << QWhereaboutsUpdate::FixAcquired;

        QTest::newRow("VTG") << addChecksum("$GPVTG,,,,,,,,,*") << QWhereaboutsUpdate::FixStatusUnknown;

        QTest::newRow("ZDA") << addChecksum("$GPZDA,,,,,,,,,*") << QWhereaboutsUpdate::FixStatusUnknown;
    }

    void fromNmea_fixStatus()
    {
        QFETCH(QByteArray, sentence);
        QFETCH(QWhereaboutsUpdate::PositionFixStatus, fixStatus);

        QWhereaboutsUpdate::PositionFixStatus actualStatus;
        QWhereaboutsUpdate::fromNmea(sentence, &actualStatus);
        QCOMPARE(actualStatus, fixStatus);
    }

    void fromNmea_time_data()
    {
        QTest::addColumn<QByteArray>("sentence");
        QTest::addColumn<QTime>("time");

        QTest::newRow("no msecs") << addChecksum("$GPGGA,060613,,,,,,,,,,,,,*")
                << QTime(6, 6, 13);
        QTest::newRow("no msecs, unnecessary dot separator") << addChecksum("$GPGGA,060613.,,,,,,,,,,,,,*")
                << QTime(6, 6, 13);
        QTest::newRow("1 msec digit") << addChecksum("$GPRMC,060613.6,,,,,,,,,,,*")
                << QTime(6, 6, 13, 6);
        QTest::newRow("2 msec digits") << addChecksum("$GGGLL,,,,,060613.62,,,*")
                << QTime(6, 6, 13, 62);
        QTest::newRow("3 msec digits") << addChecksum("$GPGGA,060613.626,,,,,,,,,,,,,*")
                << QTime(6, 6, 13, 626);
    }

    void fromNmea_time()
    {
        QFETCH(QByteArray, sentence);
        QFETCH(QTime, time);

        QWhereaboutsUpdate update = QWhereaboutsUpdate::fromNmea(sentence);
        QCOMPARE(update.updateTime(), time);
    }
};

QTEST_MAIN(tst_QWhereaboutsUpdate)
#include "tst_qwhereaboutsupdate.moc"
