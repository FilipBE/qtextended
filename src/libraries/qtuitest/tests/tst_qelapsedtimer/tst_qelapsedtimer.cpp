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
#include <qelapsedtimer_p.h>

//TESTED_COMPONENT=QA: Testing Framework (18707)

class tst_QElapsedTimer : public QObject
{
    Q_OBJECT

private slots:
    void elapsed();
};

QTEST_MAIN(tst_QElapsedTimer)

/*
    \req QTOPIA-78

    \groups
*/
void tst_QElapsedTimer::elapsed()
{
    QElapsedTimer et;
    et.start();

    // Allow a lot of latency so the test doesn't break when the
    // machine is under extreme load.
    QVERIFY(et.elapsed() <= 1000);

    QTest::qWait(5000);

    QVERIFY(et.elapsed() >= 5000);
    QVERIFY(et.elapsed() <= 30000);
}

#include "tst_qelapsedtimer.moc"
