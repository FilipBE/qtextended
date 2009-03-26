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

#include <QTest>
#include <qbenchmark.h>
#include <shared/qtopiaunittest.h>
#include <QtopiaApplication>

#include "alternativequeries.h"
#include "currentqueries.h"
#include "filtercombinations.h"

#ifndef QBENCHMARK
#define QBENCHMARK
#endif

class tst_Performance : public QObject
{
    Q_OBJECT

public:
    tst_Performance();
    virtual ~tst_Performance();

    void testPerformance();
private slots:
};

QTEST_APP_MAIN( tst_Performance, QtopiaApplication )
#include "tst_performance.moc"

tst_Performance::tst_Performance()
{
}

tst_Performance::~tst_Performance()
{
}

void tst_Performance::testPerformance()
{
    FilterCombinations *data = new FilterCombinations;
    AlternativeQueries testAlt( QtopiaUnitTest::baseDataPath() + "/alternativeschema.sqlite", data);
    CurrentQueries testCurrent(data);

    data->resetTestCombination();
    while (data->prepareNextTestCombination()) {
        QBENCHMARK {
            testAlt.applyCurrentFilter();
        }
        QBENCHMARK {
            testCurrent.applyCurrentFilter();
        }
        int altC, currentC;
        QBENCHMARK {
            altC = testAlt.count();
        }
        QBENCHMARK {
            currentC = testAlt.count();
        }
        QCOMPARE(altC, currentC);
        int c = altC;

        // scroll forward
        QBENCHMARK {
            for (int i = 0; i < c; i++) {
                testAlt.retrieveRow(i);
            }
        }
        QBENCHMARK {
            for (int i = 0; i < c; i++) {
                testCurrent.retrieveRow(i);
            }
        }
        // scroll backwards
        QBENCHMARK {
            for (int i = c; i >= 0; --i) {
                testAlt.retrieveRow(i);
            }
        }
        QBENCHMARK {
            for (int i = c; i >= 0; --i) {
                testCurrent.retrieveRow(i);
            }
        }
        // ribbon jump.
        QBENCHMARK {
            char searchChar = 'a';
            for (int i = 0; i < 26; i++) {
                testAlt.findRowForLabel(QString(QChar(searchChar+i)));
            }
        }
        QBENCHMARK {
            char searchChar = 'a';
            for (int i = 0; i < 26; i++) {
                testCurrent.findRowForLabel(QString(QChar(searchChar+i)));
            }
        }
    }
}
