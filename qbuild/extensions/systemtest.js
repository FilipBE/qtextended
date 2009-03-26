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

\extension systemtest

The systemtest extension is used for Qt Extended system tests.

*/

function systemtest_init()
{
###
    CONFIG+=test
    QMAKE.FINALIZE.systemtest.CALL = systemtest_finalize
###
}

function systemtest_finalize()
{
    systemtest_external_rules();
}

function systemtest_stub_rule()
{
    var stubrule = project.rule("stub");
    stubrule.help = "Create a stub script to run this system test";

    var stubfile = project.property("SYSTEMTEST_STUB_FILE").strValue();
    var runner   = project.property("SYSTEMTEST_RUNNER").strValue();
    var source   = project.property("SYSTEMTEST_SOURCE").strValue();
    stubrule.inputFiles  = [ source, runner ];
    stubrule.outputFiles = [ stubfile ];
    stubrule.commands = [
        "#(ev)echo create " + stubfile,
        "#(Ev)echo '#!/bin/sh' > " + stubfile,
        "#(Ev)echo 'exec " + runner + " " + source + " \"$@\"' >> " + stubfile,
        "#(Ev)chmod 755 " + stubfile
    ];

    // Build the stub by default.
    project.rule("default").prerequisiteActions.append("stub");


    stubrule = project.rule("remove_target");
    stubrule.help = "Remove the stub script used to run this system test";
    stubrule.commands = [
        "#(e)echo rm " + stubfile,
        "#(e)$$MKSPEC.DEL_FILE " + stubfile
    ];
    project.rule("clean").prerequisiteActions.append("remove_target");
}

function systemtest_syntaxcheck_rule()
{
    var rule = project.rule("syntaxcheck");
    rule.prerequisiteActions.append("stub");
    rule.help = "Check the syntax of this system test without running it";

    var source = project.property("SYSTEMTEST_SOURCE").strValue();
    var stubfile = project.property("SYSTEMTEST_STUB_FILE").strValue();

    rule.inputFiles  = [ source ];
    rule.outputFiles = [ stubfile ];

    // Syntax verification depends on any other .js file which might be included
###
    CONFIG+=functions
    add_input_files_to_rule(syntaxcheck, $$path(/tests/shared,existing), "*.js")
    add_input_files_to_rule(syntaxcheck, $$path(/src/tools/qtuitestrunner,existing), "*.js")
###

    rule.commands = [
        "#(ev)echo syntaxcheck " + source,
        "#(Ev)" + stubfile + " -c -auto",   // -auto prevents qtuitestrunner from connecting to X
        "#(ev)touch " + stubfile
    ];

    // Check syntax by default.
    project.rule("default").prerequisiteActions.append("syntaxcheck");
}

function systemtest_test_rule()
{
    var testrule = project.rule("test");
    testrule.prerequisiteActions.append("stub");
    testrule.help = "Run this system test";

    testrule.commands =
        "export QPEHOME=$(mktemp -d /tmp/qtopia_maketest_XXXXXX); " +
        "cp -r $$path(/tests/shared/skel,existing)/* $QPEHOME; " +
        "$$path($$TARGET,generated) $ARGS; " +
        'rm -rf $QPEHOME';
}

function systemtest_external_rules()
{
###
    SYSTEMTEST_STUB_FILE = $$path(.,generated)/$$TARGET
    SYSTEMTEST_RUNNER    = $$path(QtopiaSdk:/bin/qtuitestrunner,generated)
    SYSTEMTEST_SOURCE    = $$source_file($$SOURCES)
###

    var source = project.property("SYSTEMTEST_SOURCE").strValue();
    // If the system test is an old-style C++ test, we don't support it
    if (source.indexOf(".cpp") != -1) return;
    // If the system test contains >1 source file, SYSTEMTEST_SOURCE will be empty.
    // We don't support this either.
    if (source == "") return;

    systemtest_stub_rule();
    systemtest_syntaxcheck_rule();
    systemtest_test_rule();
}

