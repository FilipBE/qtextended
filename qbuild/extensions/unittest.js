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

\extension unittest

The unittest extension is used for Qt Extended unit tests.
It makes the QtTest library and helper classes in /tests/shared available to
the project.

*/

function unittest_init()
{
###
    SOURCEPATH+=/tests
    QT+=testlib
    DEFINES*=QTOPIA_TEST
    CONFIG+=test

    QMAKE.FINALIZE.unittest.CALL = unittest_finalize
###

    // Set up the TEST_SRCDIR define to allow finding the "testdata" directory
    var srcdir = qbuild.invoke("path", project.sproject().nodePath(), "existing");
    project.property("TEST_SRCDIR").setValue(srcdir);

###
    DEFINES*=TEST_SRCDIR=$$define_string($$TEST_SRCDIR)
###
}

function unittest_finalize()
{
    unittest_external_rules();
}

function unittest_external_rules()
{
    var testrule = project.rule("test");
    testrule.help = "Run this unit test";

    // Unit test should be installed before running.
    testrule.prerequisiteActions.append("image");

    // Run the test.
    testrule.commands =
        "export QPEHOME=$(mktemp -d /tmp/qtopia_maketest_XXXXXX); " +
        "$$path(QtopiaSdk:/bin/runqtopia,generated) -exec '$$QTOPIA_IMAGE/bin/$$TARGET -qws $ARGS'; " +
        "ret=$?; " +
        "rm -rf $QPEHOME; " +
        "exit $ret";
}

