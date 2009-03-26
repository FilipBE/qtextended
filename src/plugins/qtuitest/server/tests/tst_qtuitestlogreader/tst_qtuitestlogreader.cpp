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

#include "qtuitestlogreader_p.h"

#include <QTest>
#include <QSignalSpy>
#include <shared/util.h>

class tst_QtUiTestLogReader : public QObject
{
Q_OBJECT
private slots:
    void error_notexist();
    void error_notexist_data();

    void error_nonzero();
    void error_nonzero_data();

    void error_nocommands();

    void log();
    void log_data();
};

QTEST_MAIN(tst_QtUiTestLogReader)

void tst_QtUiTestLogReader::error_notexist()
{
    QFETCH(QStringList, commands);

    QtUiTestLogReader reader;

    QSignalSpy spy(&reader, SIGNAL(error(QString)));
    reader.start(commands);
    QTRY_COMPARE(spy.count(), 1);
}

void tst_QtUiTestLogReader::error_notexist_data()
{
    QTest::addColumn<QStringList>("commands");

    QTest::newRow("simple")
        << (QStringList() << "/bin/qtuitest_logreader_command_notexist");

    QTest::newRow("multiple")
        << (QStringList() << "/tmp/foobarbaz_qtuitest_logreader_notexist" << "/bin/qtuitest_logreader_command_notexist");
}

void tst_QtUiTestLogReader::error_nonzero()
{
    QFETCH(QStringList, commands);

    QtUiTestLogReader reader;

    QSignalSpy spy(&reader, SIGNAL(error(QString)));
    reader.start(commands);
    QTRY_COMPARE(spy.count(), 1);

    QString error = spy.at(0).at(0).toString();
    QVERIFY2(error.contains(commands.at(0)), qPrintable(QString("Error is %1").arg(error)));
    QVERIFY2(error.contains("exit code"),    qPrintable(QString("Error is %1").arg(error)));
}

void tst_QtUiTestLogReader::error_nonzero_data()
{
    QTest::addColumn<QStringList>("commands");

    QTest::newRow("simple")
        << (QStringList() << "/bin/false");

    QTest::newRow("multiple")
        << (QStringList() << "/bin/false" << "[ 1 != 1 ]");

    QTest::newRow("with_output")
        << (QStringList() << "/bin/sh -c '{ echo foo; echo bar; /bin/false; }'" << "[ 1 != 1 ]");
}

void tst_QtUiTestLogReader::error_nocommands()
{
    QtUiTestLogReader reader;

    QSignalSpy spy(&reader, SIGNAL(error(QString)));
    reader.start(QStringList());
    QCOMPARE(spy.count(), 1);
}

void tst_QtUiTestLogReader::log()
{
    QFETCH(QStringList, commands);
    QFETCH(QStringList, logs);

    QtUiTestLogReader reader;

    QSignalSpy errorSpy   (&reader, SIGNAL(error(QString)));
    QSignalSpy finishedSpy(&reader, SIGNAL(finished()));
    QSignalSpy logSpy     (&reader, SIGNAL(log(QStringList)));
    reader.start(commands);
    QTRY_COMPARE(finishedSpy.count(), 1);

    QVERIFY2(errorSpy.count() == 0, qPrintable(QString("Error occurred: %1").arg((!errorSpy.count() ? QString() : errorSpy.at(0).at(0).toString()))));

    QStringList actualLogs;
    while (logSpy.count())
        actualLogs += logSpy.takeAt(0).at(0).toStringList();

    QCOMPARE(actualLogs.join("\n"), logs.join("\n"));
}

void tst_QtUiTestLogReader::log_data()
{
    QTest::addColumn<QStringList>("commands");
    QTest::addColumn<QStringList>("logs");

    QTest::newRow("empty single")
        << (QStringList() << "/bin/true")
        << (QStringList())
    ;

    QTest::newRow("empty multiple")
        << (QStringList() << "/bin/true" << "[ 1 = 1 ]")
        << (QStringList())
    ;

    QTest::newRow("mixed, first and last output")
        << (QStringList()
            << "/bin/sh -c \"{ echo foo; echo bar 1>&2; }\""
            << "/bin/true"
            << "echo baz")
        << (QStringList() << "foo" << "bar" << "baz")
    ;

    QTest::newRow("mixed, first and last silent")
        << (QStringList()
            << "/bin/true"
            << "/bin/sh -c \"{ echo baz; echo foo; }\""
            << "echo bar"
            << "[ 1 = 1 ]")
        << (QStringList() << "baz" << "foo" << "bar")
    ;

    QTest::newRow("mixed, last takes a while")
        << (QStringList()
            << "echo baz"
            << "/bin/true"
            << "/bin/sh -c \"{ echo bar; sleep 2; echo foo 1>&2; }\"")
        << (QStringList() << "baz" << "bar" << "foo")
    ;
}

#include "tst_qtuitestlogreader.moc"

