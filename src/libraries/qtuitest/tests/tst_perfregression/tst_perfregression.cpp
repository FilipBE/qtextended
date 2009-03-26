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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include <QTest>
#include <QObject>
#include <qbenchmark.h>
#include <stdlib.h>
#include <time.h>
#include <QFile>

//TESTED_COMPONENT=QA: Testing Framework (18707)

/*
    This test simulates a performance regression every once in a while.

    The results of this test can be used to verify that an automatic regression
    detection system works as intended.
*/

class tst_PerfRegression : public QObject
{
    Q_OBJECT

private slots:
    void bad();
    void bad_data();

    void initTestCase();
};

QTEST_MAIN(tst_PerfRegression)

typedef qreal(*RandomDistribution)(qreal,qreal);

inline qreal qrand_real()
{
    return qreal(qrand())/qreal(RAND_MAX);
}

qreal exponential(qreal mean,qreal)
{
    qreal lambda;

    // mean = 1 / lambda
    // stddev^2 = 1 / lambda^2

    // therefore:
    lambda = 1/mean;
    // ...and stddev is ignored, being dependent on mean.

    return -log(qrand_real())/lambda;
}

qreal constant(qreal mean, qreal)
{
    return mean;
}

qreal normal(qreal mean, qreal stddev)
{
    static int i = 0;
    static float y2 = 0.;
    if (i++ % 2) {
        return y2;
    }


    // Box-Muller transform: http://www.taygeta.com/random/gaussian.html
    float x1, x2, w, y1;

    do {
        x1 = 2.0 * qrand_real() - 1.0;
        x2 = 2.0 * qrand_real() - 1.0;
        w = x1 * x1 + x2 * x2;
    } while ( w >= 1.0 );

    w = sqrt( (-2.0 * log( w ) ) / w );
    y1 = x1 * w;
    y2 = x2 * w;


    // Currently, mean is 0 and stddev is 1.
    // Transform to desired mean/stddev.
    y1 *= stddev;
    y1 += mean;
    y2 *= stddev;
    y2 += mean;

    return y1;
}

/*
    \req QTOPIA-78

    \groups
*/
void tst_PerfRegression::bad()
{
    static qreal dummy = 1.0;
    static const int DEFAULT_ITERS = 1000000;

    QFETCH(int,   changeInterval);  // a regression occurs once every changeInterval changes
    QFETCH(int,   badChanges);      // the regression lasts for this many changes
    QFETCH(qreal, badness);         // the measured value increases by this factor

    QFETCH(void*, random);          // function describing how the values are randomly distributed
    QFETCH(qreal, mean);            // mean for `random', as a fraction of DEFAULT_ITERS
    QFETCH(qreal, stddev);          // standard deviation for `random', as a fraction of DEFAULT_ITERS

    int iters = DEFAULT_ITERS;
    iters += int(((RandomDistribution)(random))(mean*qreal(iters), stddev*qreal(iters)));

    bool ok;
    int change = qgetenv("QPE_CHANGENO").toInt(&ok);
    if (!ok) {
        QSKIP("QPE_CHANGENO environment variable must be set for this test", SkipAll);
    }

    if ((change % changeInterval) < badChanges) {
        iters = (int)(qreal(iters)*(1. + badness));
    }

    QBENCHMARK {
        for (int i = 0; i < iters; ++i) {
            dummy *= 1.5;
            dummy /= 1.5;
        }
    }
}

void tst_PerfRegression::bad_data()
{
    QTest::addColumn<int>("changeInterval");
    QTest::addColumn<int>("badChanges");
    QTest::addColumn<qreal>("badness");

    QTest::addColumn<void*>("random");
    QTest::addColumn<qreal>("mean");
    QTest::addColumn<qreal>("stddev");

    QTest::newRow("20 out of 200 changes, normal, badness 10%, mean 10%, stddev 10%")
        << 200
        << 20
        << 0.1
        << (void*)normal
        << 0.1
        << 0.1
    ;

    QTest::newRow("20 out of 200 changes, constant, badness 5%")
        << 200
        << 20
        << 0.05
        << (void*)constant
        << 0.
        << 0.
    ;

    QTest::newRow("5 out of 200 changes, constant, badness 20%")
        << 200
        << 5
        << 0.2
        << (void*)constant
        << 0.
        << 0.
    ;

    QTest::newRow("20 out of 200 changes, exponential, badness 5%, mean 5%")
        << 200
        << 20
        << 0.05
        << (void*)exponential
        << 0.05
        << 0.
    ;

    QTest::newRow("20 out of 200 changes, exponential, badness 10%, mean 10%")
        << 200
        << 20
        << 0.1
        << (void*)exponential
        << 0.1
        << 0.
    ;

    QTest::newRow("20 out of 200 changes, exponential, badness 10%, mean 30%")
        << 200
        << 20
        << 0.1
        << (void*)exponential
        << 0.3
        << 0.
    ;
}

void tst_PerfRegression::initTestCase()
{
    static const char randomfile[] = "/dev/urandom";

    unsigned int seed = 0;

    if (QFile::exists(randomfile)) {
        QFile file(randomfile);
        if (file.open(QIODevice::ReadOnly)) {
            if (sizeof(seed) != file.read(reinterpret_cast<char*>(&seed), sizeof(seed))) {
                seed = 0;
            }
        }
    }

    if (!seed) seed = time(0);
    qsrand(seed);
}


#include "tst_perfregression.moc"
