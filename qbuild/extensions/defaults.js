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

\extension defaults

*/

/*!

\qbuild_variable PRERUN_RULES
\ingroup defaults_extension
\brief Run rules before the "default" rule.

This causes rules to be run before the "default" rule. Note that it makes the "default" rule execute in serial.

*/

/*!

\qbuild_variable POSTRUN_RULES
\ingroup defaults_extension
\brief Run rules after the "default" rule.

This causes rules to be run after the "default" rule. Note that it makes the "default" rule execute in serial.

*/

function defaults_init()
{
###
    CONFIG*=runlast
    QMAKE.FINALIZE.defaults.CALL = defaults_finalize
    QMAKE.FINALIZE.defaults.RUN_BEFORE_ME = subdirs
    QMAKE.FINALIZE.defaults.RUN_AFTER_ME = templates cpp_compiler installs depends headers
    QT_DEPOT_PATH=$$path(/qtopiacore/qt,existing)
    RUNLAST+="defaults_pre_post_run()"
    # We need PIC enabled by default because virtually everything is built into dynamic libs or plugins
    CONFIG+=use_pic
    # Packages uses heuristics to decide if a default package should be created
    CONFIG+=packages
    # Make usually comes from the qbuild script (via the environment)
    MAKE=$$(MAKE)
    isEmpty(MAKE):MAKE=make
###
    if ( !project.absName.match(/3rdparty/) ) {
###
        CONFIG+=warn_on
###
    }
    project.rule("default").prerequisiteActions.append("check_enabled");
    project.rule("check_enabled");
}

function defaults_finalize()
{
    defaults_checkEnabled();
    defaults_hook_sub_rules();

###
    # FIXME enabled for non-depot projects
    #INCLUDEPATH+=$$path(/include,generated)
    embedded {
        MKSPEC.LFLAGS+=-L$$path(QtopiaSdk:/lib,generated)
    } else {
        MKSPEC.LFLAGS+=-L$$path(QtopiaSdk:/lib/host,generated)
    }
    equals(TEMPLATE,app) {
        !embedded {
            isEmpty(TARGETDIR):TARGETDIR=QtopiaSdk:/bin
            legacy_bin {
                DEFAULTS.symlink_bin.TYPE=RULE
                DEFAULTS.symlink_bin.commands=\
                    "#(E)$$MKSPEC.DEL_FILE $$path(/bin/$$TARGET,generated)"\
                    "$$MKSPEC.SYMBOLIC_LINK $$path($$TARGETDIR/$$TARGET,generated) $$path(/bin/$$TARGET,generated)"
                DEFAULTS.target_post.TYPE=RULE
                DEFAULTS.target_post.prerequisiteActions=symlink_bin
            }
        }
        embedded {
            target.path=/bin
            target.hint*=image
        }
    }
    equals(TEMPLATE,lib):!plugin {
        isEmpty(TARGETDIR) {
            embedded:TARGETDIR=QtopiaSdk:/lib
            else:TARGETDIR=QtopiaSdk:/lib/host
        }
        embedded:!staticlib {
            target.path=/lib
            target.hint*=image
        }
    }
    equals(TEMPLATE,lib):plugin {
        isEmpty(PLUGIN_TYPE):warning("You must specify a value for PLIGIN_TYPE")
        isEmpty(PLUGIN_FOR):warning("You must specify a value for PLUGIN_FOR")
        !isEmpty(PLUGIN_TYPE):!isEmpty(PLUGIN_FOR) {
            equals(PLUGIN_FOR,qt):plugins=qt_plugins
            equals(PLUGIN_FOR,qtopia):plugins=plugins
            embedded:!staticlib {
                target.path=/$$plugins/$$PLUGIN_TYPE
                target.hint*=image
            }
        }
    }
    DEFINES+=QTOPIA_TARGET=$$define_string($$TARGET)
    desktop:DEFINES+=QTOPIA_HOST
###
    // delay linking the target in the other project until this project has completed
    // this ensures a command like qbuild -node src/server actually causes the libraries
    // to write out their module.dep files.
###
    DEFAULTS.DEP.libs.TYPE=DEPENDS
    DEFAULTS.DEP.libs.EVAL=\
        "target_pre.TYPE=RULE"\
        "target_pre.prerequisiteActions+=""#(h)"$$project()"default"""
###
    // Optimize linker algorithm for memory use. Handy on memory-constrained systems.
###
    ld_optimize_memory:MKSPEC.LFLAGS+=-Wl,-no-keep-memory
###

    // set the define DEBUG when CONFIG+=debug
###
    debug:DEFINES+=DEBUG
###

    // QBuild test executes test cases (if any)
    {
        var sproj = project.sproject(project.absName+"tests/");
        if ( sproj.subprojects().length ) {
            var proj = sproj.project();
            if ( proj && proj.property("TEMPLATE") != "blank" ) {
                var rule = project.rule("test");
                rule.prerequisiteActions.append(project.absName+"tests/test");
            }
        }
    }

    // SXE tests (enable the app test, set target.hint=sxe)
###
    enable_sxe:qtopia:if(equals(TEMPLATE,app)|quicklaunch) {
        CONFIG+=sxe_test
        target.hint*=sxe
    }
###

    // If you use RESOURCES, you must also use singleexec_link linking
###
    enable_singleexec:qt:embedded:!isEmpty(RESOURCES):CONFIG+=singleexec_link
###
    setup_target_rules();
}

function defaults_pre_post_run()
{
    var prerun = project.property("PRERUN_RULES").value();
    var postrun = project.property("POSTRUN_RULES").value();
    var rule = project.rule("default");
    rule.serial = true;

    if (prerun && prerun.length) {
        var prerun_rule = project.rule("default.prerun");
        prerun_rule.prerequisiteActions = prerun;
        prerun_rule.serial = true;

        prereq = rule.prerequisiteActions.value();
        rule.prerequisiteActions = "default.prerun";
        rule.prerequisiteActions.append(prereq);
    }

    if (postrun && postrun.length) {
        var postrun_rule = project.rule("default.postrun");
        postrun_rule.prerequisiteActions = postrun;
        postrun_rule.serial = true;

        rule.prerequisiteActions.append("default.postrun");
    }
}

function defaults_checkEnabled()
{
    if ( project.disabledReason() || project.absName.match(/\{file\}\/$/) )
        return; // Don't try to disable an already-disabled project

    if ( project.absName.match(/\/tests\//) )
        return; // Don't disable test cases

    var ok = 1;
    var projectName = project.absName;
    echo("checkEnabled", "defaults_checkEnabled "+projectName);

    // Check for things in /src
    if ( projectName.match(/\/src\/.+/) )
        ok = 0;
    if ( !ok ) {
        var projects = project.property("PROJECTS").value()
        for ( var ii in projects ) {
            var p = projects[ii];
            if ( projectName == "/src/"+p+"/" ) {
                if ( project.config("no_qbuild_pro") )
                    hard_disable_project("This project is listed in PROJECTS but has no qbuild.pro.");
                echo("checkEnabled", "Found "+p+" in PROJECTS");
                ok = 1;
                break;
            }
        }
    }
    if ( !ok && !project.config("no_qbuild_pro") ) {
        echo("checkEnabled", "Not in PROJECTS ");
        soft_disable_project("Disabling project because it is not listed in PROJECTS");
        return;
    }

    // Check for things in /examples
    if ( projectName.match(/\/examples\/.+/) )
        ok = 0;
    if ( !ok ) {
        var projects = project.property("PROJECTS").value()
        for ( var ii in projects ) {
            var p = projects[ii];
            if ( ".."+projectName == p+"/" ) {
                if ( project.config("no_qbuild_pro") )
                    hard_disable_project("This project is listed in PROJECTS but has no qbuild.pro.");
                ok = 1;
                break;
            }
        }
    }
    if ( !ok && project.config("build_examples") ) {
        ok = 1;
###
        CONFIG+=runlast
        RUNLAST+="defaults_disable_image_rule()"
###
    }
    if ( !ok && !project.config("no_qbuild_pro") ) {
        soft_disable_project("Disabling project because it is not listed in PROJECTS", "image");
        return;
    }

    // Check for things in /etc/themes
    if ( projectName.match(/\/etc\/themes\/.+/) )
        ok = 0;
    if ( !ok ) {
        var projects = project.property("THEMES").value()
        for ( var ii in projects ) {
            var p = projects[ii];
            if ( projectName == "/etc/themes/"+p+"/" ) {
                if ( project.config("no_qbuild_pro") )
                    hard_disable_project("This project is listed in THEMES but has no qbuild.pro.");
                ok = 1;
                break;
            }
        }
    }
    if ( !ok && !project.config("no_qbuild_pro") ) {
        soft_disable_project("Disabling project because it is not listed in THEMES");
        return;
    }
}

function defaults_hook_sub_rules()
{
    var rules = project.property("SUBDIRS_RULES").value();
    for ( var ii in rules ) {
        var rule = project.rule(rules[ii]);

        var rulename = rules[ii]+"_sub";
        project.rule(rulename).prerequisiteActions = rule.name;
    }
}

function defaults_disable_image_rule()
{
    project.rule("image").prerequisiteActions = "";
    project.rule("image").help = "";
}

