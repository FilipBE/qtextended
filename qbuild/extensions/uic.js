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

\extension uic

The uic extension handles interaction with uic.

Rules:
    uicables
    clean_uicables
    force_clean_uicables

Inputs:
    TYPE == UIC_FORMS
        Used as input for uic

Outputs:    
    UIC.UICDIR
    UIC.UICABLES
    UIC.UICRULES

*/

/*!

\qbuild_variable FORMS
\brief The FORMS variable selects .ui files to build.

*/

function uic_init()
{
### 
    CONFIG*=qt conditional_sources
    QMAKE.FINALIZE.uic.CALL = uic_finalize
    QMAKE.FINALIZE.uic.RUN_BEFORE_ME = conditional_sources
    QMAKE.FINALIZE.uic.RUN_AFTER_ME = cpp_compiler rules qt headers moc
    TYPE*=UIC_FORMS
###
}

function uic_paths()
{
    // The base path is the location uic generated output will go to.
    var uicdir = qbuild.invoke("path", ".uic", "generated");
    project.property("UIC.UICDIR").setValue(uicdir);

    project.property("INCLUDEPATH.GENERATED").append(uicdir);

    var rule = project.rule("ensure_uicdir");
    rule.outputFiles = uicdir;
    rule.commands = "#(e)$$MKSPEC.MKDIR $$[OUTPUT.0]";
}

function uic_uicables_for_obj(obj, path, uicables, uicrules, uic)
{
    /*
       Each entry in the FORMS list will cause the generation of a uic output file
       of the form ui_<name>.h.

       SOURCE_DEPENDS_RULES is used to ensure that these headers are created early
       enough during the build process (eg. gcc -MM might need this header but we
       can't know that before we run it!)
     */

    var forms_list = obj.property("FORMS").value();
    var semi_private_headers = project.property("SEMI_PRIVATE_HEADERS");
    var moc_ignore = project.property("MOC_IGNORE");

    for (var ii in forms_list) {

        var base = File(forms_list[ii]).name();
        var name = path + "/ui_" + base + ".h";

        // We need a list to use in the clean_uicables rule
        uicables.append(name);

        if (project.config("semi_private_uic_headers")) {
            semi_private_headers.append(name);
            moc_ignore.append(name);
        }

        var source = source_file(forms_list[ii]);
        // The actual rule
        var rule = project.rule(basename(name));
        rule.outputFiles = name;
        rule.inputFiles = source;
        rule.inputFiles.append("#(f)$$include_depends_rule("+cpp_compiler_dep_file(name)+")");
        rule.commands = project.property("QBUILD_UIC_IMPL").value();
        rule.other = path;
        rule.prerequisiteActions.append("uic_args_test");
        rule.prerequisiteActions.append("#(oh)build_uic_rule");
        rule.prerequisiteActions.append("#(oh)ensure_uicdir");
        rule.outputFiles.append(cpp_compiler_dep_file(name));
        rule.commands.append(project.property("QBUILD_UIC_DEPS_IMPL").value());
        rule.prerequisiteActions.append("#(oh)ensure_objdir");

        // We need a list to use in the uicables rule
        uicrules.append(rule.name);
    } 
}

function uic_uicables()
{
###
    QBUILD_UIC_IMPL = "#(eh)echo uic $$[INPUT.0]" \
                      "#(e)$$MKSPEC.MKDIR $$[OTHER.0]"\
                      "#(E)$$QTDIR/bin/uic $$[INPUT.0] -o $$[OUTPUT.0]"

    QBUILD_UIC_DEPS_IMPL = "#(Et)$$MKSPEC.CXX -MM $${COMPILER.CXXFLAGS} $${COMPILER.DEFINES} $${COMPILER.INCLUDEPATH} -o $$[OUTPUT.1] -xc++ $$[OUTPUT.0]"

    QBUILD_UIC_ARGS_TEST = "$$QTDIR/bin/uic"
###

    var uic = project.property("UIC");
    var uic_base_path = project.property("UIC.UICDIR").strValue();

    create_args_rule({
        name: "uic_args_test",
        file: uic_base_path+"/uic.args",
        contents: project.property("QBUILD_UIC_ARGS_TEST").strValue(),
        prereq: "#(oh)ensure_uicdir",
        depend_on_qt: 1
    });

    var uicables = uic.property("UICABLES");
    var uicrules = uic.property("UICRULES");

    var sources = project.find("TYPE", "UIC_FORMS");

    for (var ii in sources) {
        var baseobj = qbuild.object(sources[ii]);
        uic_uicables_for_obj(baseobj, uic_base_path, uicables, uicrules, uic);
    }
}

function uic_finalize()
{
    if (!project.config("qt") || !project.config("uic"))
        return;

    qt_get_qtdir();
    uic_paths();
    uic_uicables();
    uic_external_rules();
}

function uic_external_rules()
{
###
    QBUILD_UIC_CLEAN = "$$MKSPEC.DEL_FILE $${UIC.UICABLES}"
    QBUILD_UIC_FORCE_CLEAN = "$$MKSPEC.DEL_FILE $${UIC.UICDIR}/ui_*.h"

    !isEmpty(UIC.UICABLES) {
        UIC.RULES.uicables.TYPE = RULE
        UIC.RULES.uicables.help = "Generate all uic files"
        for(r,UIC.UICRULES) {
            UIC.RULES.uicables.prerequisiteActions += "#(h)"$$r
        }

        UIC.RULES.clean_uicables.TYPE = RULE
        UIC.RULES.clean_uicables.help = "Remove uic generated files"
        UIC.RULES.clean_uicables.commands = $$QBUILD_UIC_CLEAN

        UIC.RULES.clean.TYPE = RULE
        UIC.RULES.clean.prerequisiteActions = clean_uicables

        SOURCE_DEPENDS_RULES += uicables
    }

    UIC.RULES.force_clean_uicables.TYPE = RULE
    UIC.RULES.force_clean_uicables.help = "Remove all uic_*.h files"
    UIC.RULES.force_clean_uicables.commands = $$QBUILD_UIC_FORCE_CLEAN

    UIC.RULES.force_clean.TYPE = RULE
    UIC.RULES.force_clean.prerequisiteActions = force_clean_uicables
###
}

