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

\extension rcc

The rcc extension handles interaction with rcc.

Rules:
    rccables
    clean_rccables
    force_clean_rccables

Inputs:
    TYPE == RCC_SOURCES
        Used as input for rcc

Outputs:    
    RCC.RCCDIR
    RCC.RCCABLES
    RCC.RCCRULES

*/

/*!

\qbuild_variable RESOURCES
\brief The RESOURCES variable selects .qrc files to embed into the executable.

*/

function rcc_init()
{
### 
    CONFIG*=qt conditional_sources
    QMAKE.FINALIZE.rcc.CALL = rcc_finalize
    QMAKE.FINALIZE.rcc.RUN_BEFORE_ME = conditional_sources
    QMAKE.FINALIZE.rcc.RUN_AFTER_ME = cpp_compiler rules qt
    TYPE*=RCC_SOURCES
###
}

function rcc_finalize()
{
    if (!project.config("qt") || !project.config("rcc"))
        return;

    qt_get_qtdir();
    // The base path is the location rcc generated output will go to.
    var rccdir = qbuild.invoke("path", ".rcc", "generated");
    project.property("RCC.RCCDIR").setValue(rccdir);

    rcc_rccables();
    rcc_external_rules();

    var rule = project.rule("ensure_rccdir");
    rule.outputFiles = rccdir;
    rule.commands = "#(e)$$MKSPEC.MKDIR $$[OUTPUT.0]";
}

function rcc_rccables()
{
###
    QBUILD_RCC_IMPL = "#(eh)echo rcc $$[INPUT.0]" \
                     "#(E)$$QTDIR/bin/rcc -name $$[OTHER.0] $$[INPUT.0] -o $$[OUTPUT.0]"
    QBUILD_RCC_DEP_IMPL = "#(e)$$QTDIR/bin/rcc -list $$[INPUT.0] >$$[OUTPUT.0]"
    QBUILD_RCC_ARGS_TEST = "$$QTDIR/bin/rcc"
###

    var rcc = project.property("RCC");
    var rcc_base_path = rcc.property("RCCDIR").strValue();

    create_args_rule({
        name: "rcc_args_test",
        file: rcc_base_path+"/rcc.args",
        contents: project.property("QBUILD_RCC_ARGS_TEST").strValue(),
        prereq: "#(oh)ensure_rccdir",
        depend_on_qt: 1
    });

    var rccables = rcc.property("RCCABLES");
    var rccrules = rcc.property("RCCRULES");

    var sources = project.find("TYPE", "RCC_SOURCES");

    for ( var ii in sources ) {
        var baseobj = qbuild.object(sources[ii]);
        rcc_rccables_for_obj(baseobj, rcc_base_path, rccables, rccrules, rcc);
    }
}

function rcc_rccables_for_obj(obj, path, rccables, rccrules, rcc)
{
    /*
       Each entry in the RESOURCES list will cause the generation of a rcc output file
       of the form rcc_<name>.cpp.
     */

    var resources = obj.property("RESOURCES").value();
    var rcc = project.property("RCC");
    var rccsources = rcc.property("SOURCES");

    rcc.property("TYPE").unite("CPP_SOURCES");
    rcc.property("TYPE").unite("GENERATED_SOURCES");
    rcc.property("TYPE").unite("NON_TRANSLATABLE");

    for ( var ii in resources ) {
        var base = File(resources[ii]).name();
        var name = path + "/rcc_" + base + ".cpp";
        var dependency_file = path + "/" + basename(resources[ii]) + ".d";

        var source = source_file(resources[ii]);
        if ( !source ) {
            project.warning("Missing rcc input file "+resources[ii]);
            continue;
        }

        // We need a list to use in the clean_rccables rule
        rccables.append(name);

        // This needs to be compiled
        rccsources.append(name);

        // The rule to create the .d file (which lists the dependencies on the .qrc file)
        var dependencyRule = project.rule(basename(dependency_file));
        dependencyRule.outputFiles = dependency_file;
        dependencyRule.inputFiles = source;
        dependencyRule.inputFiles.append("#(f)$$include_depends_rule($$[OUTPUT.0.ABS])");
        dependencyRule.commands = project.property("QBUILD_RCC_DEP_IMPL").value();
        dependencyRule.prerequisiteActions.append("#(oh)build_rcc_rule");
        dependencyRule.prerequisiteActions.append("#(oh)ensure_rccdir")

        // The actual rule
        var rule = project.rule(basename(name));
        rule.outputFiles = name;
        rule.inputFiles = source;
        rule.inputFiles.append("#(f)$$include_depends_rule("+dependency_file+")");
        rule.other = base;
        rule.commands = project.property("QBUILD_RCC_IMPL").value();
        rule.prerequisiteActions.append("rcc_args_test");
        rule.prerequisiteActions.append("#(h)"+dependencyRule.name);
        rule.prerequisiteActions.append("#(oh)build_rcc_rule");
        rule.prerequisiteActions.append("#(oh)ensure_rccdir");

        // We need a list to use in the rccables rule
        rccrules.append(rule.name);
    } 
}

function rcc_external_rules()
{
###
    QBUILD_RCC_CLEAN = "$$MKSPEC.DEL_FILE $${RCC.RCCABLES}"
    QBUILD_RCC_FORCE_CLEAN = "$$MKSPEC.DEL_FILE $${RCC.RCCDIR}/rcc_*.cpp"

    !isEmpty(RCC.RCCABLES) {
        RCC.RULES.rccables.TYPE = RULE
        RCC.RULES.rccables.help = "Generate all rcc files"
        for(r,RCC.RCCRULES) {
            RCC.RULES.rccables.prerequisiteActions += "#(h)"$$r
        }

        RCC.RULES.clean_rccables.TYPE = RULE
        RCC.RULES.clean_rccables.help = "Remove rcc generated files"
        RCC.RULES.clean_rccables.commands = $$QBUILD_RCC_CLEAN

        RCC.RULES.clean.TYPE = RULE
        RCC.RULES.clean.prerequisiteActions = clean_rccables
    }

    RCC.RULES.force_clean_rccables.TYPE = RULE
    RCC.RULES.force_clean_rccables.help = "Remove all rcc_*.h files"
    RCC.RULES.force_clean_rccables.commands = $$QBUILD_RCC_FORCE_CLEAN

    RCC.RULES.force_clean.TYPE = RULE
    RCC.RULES.force_clean.prerequisiteActions = force_clean_rccables
###
}

