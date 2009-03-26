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

/*!

\extension benchmark

The benchmark extension is used for performance tests using the experimental
benchlib project.

The path to the benchlib sources may need to be specified using the
BENCHLIB_SOURCE environment variable.

This extension is considered transitional.  The benchmarking API should
eventually be made available to all unit tests.

*/

function benchmark_init()
{
###
    # Make the usual unit test stuff work.
    SOURCEPATH+=/tests /qtopiacore/qt/tests
    DEFINES*=QTOPIA_TEST
    CONFIG+=test

    # Find benchlib sources.
    BENCHLIB_SOURCE=$$(BENCHLIB_SOURCE)
    isEmpty(BENCHLIB_SOURCE) {
        BENCHLIB_SOURCE=$$(HOME)/depot/research/main/benchlib
    }
    requires( exists($$BENCHLIB_SOURCE/testlib.pri) )

    SOURCEPATH+=$$BENCHLIB_SOURCE/testlib-src
    INCLUDEPATH+=$$BENCHLIB_SOURCE/testlib-src

    # Repair the damage caused by testlib.pri
    OLDSOURCES=$$SOURCES
    OLDHEADERS=$$HEADERS
    include($$BENCHLIB_SOURCE/testlib.pri)
    SOURCES*=$$OLDSOURCES
    HEADERS*=$$OLDHEADERS
    DEFINES-=QT_NO_DATASTREAM

    # Make it possible to check if benchlib is being used.
    DEFINES*=USE_BENCHLIB

    QMAKE.FINALIZE.benchmark.CALL = benchmark_finalize
    QMAKE.FINALIZE.benchmark.RUN_BEFORE_ME = qt
###

    // Set up the TEST_SRCDIR define to allow finding the "testdata" directory
    var srcdir = qbuild.invoke("path", project.sproject().nodePath(), "existing");
    project.property("TEST_SRCDIR").setValue("\\\"" + srcdir + "\\\"");

###
    DEFINES*=TEST_SRCDIR=$$TEST_SRCDIR
###
}

function benchmark_finalize()
{
###
    # Make it possible to use Qt-style qtestlib headers.
    # Strictly speaking, this may not be correct if benchlib and qtestlib's API diverge,
    # but that shouldn't happen.
    INCLUDEPATH*=$$QTINC/QtTest
###

    benchmark_external_rules();
}

function benchmark_external_rules()
{
    var testrule = project.rule("test");
    testrule.help = "Run this benchmark";

    // Unit test should be installed before running.
    testrule.prerequisiteActions.append("image");

    // Run the test.
    testrule.commands =
        "export QPEHOME=$(mktemp -d /tmp/qtopia_maketest_XXXXXX); " +
        "$$path(QtopiaSdk:/bin/runqtopia,generated) -exec '$$QTOPIA_IMAGE/bin/$$TARGET -qws $ARGS'; " +
        'rm -rf $QPEHOME';
}

