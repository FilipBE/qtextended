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

\extension moc

The moc extension handles interaction with moc.

Rules:
    mocables              (external)
    clean_mocables        (external)
    force_clean_mocables  (external)

Inputs:
    TYPE == MOC_SOURCES
        Used as input for moc

Outputs:
    MOC.MOCDIR
    MOC.MOCABLES
    MOC.MOCRULES

*/

/*!

\qbuild_variable MOC_COMPILE_EXCEPTIONS
\brief Support for a qmake feature.

If you use Q_PRIVATE_SLOT() you need to do \c{#include "moc_foo.cpp"} in \c{foo.cpp} but this breaks
QBuild because it can't "see" the include. To work around this you must specify \c{foo.h} as a moc
compile exception.

\code
HEADERS+=foo.h
SOURCES+=foo.cpp
MOC_COMPILE_EXCEPTIONS+=foo.h
\endcode

\sa MOC_IGNORE

*/

/*!

\qbuild_variable MOC_IGNORE
\brief tell moc to ignore files

This is handy for files that moc should not touch.

\code
HEADERS+=foo.h
MOC_IGNORE+=foo.h
\endcode

\sa MOC_COMPILE_EXCEPTIONS

*/

function moc_init()
{
###
    CONFIG*=qt conditional_sources
    QMAKE.FINALIZE.moc.CALL = moc_finalize
    QMAKE.FINALIZE.moc.RUN_BEFORE_ME = headers defaults conditional_sources
    QMAKE.FINALIZE.moc.RUN_AFTER_ME = rules cpp_compiler qt
    TYPE*=MOC_SOURCES
    MOC_COMPILE_EXCEPTIONS=
###
}

function moc_finalize()
{
    if (!project.config("qt") || !project.config("moc"))
        return;

    qt_get_qtdir();
    moc_paths();
    moc_mocables();
    moc_external_rules();
}

function moc_paths()
{
    var mocdir = qbuild.invoke("path", ".moc", "generated");
    project.property("MOC.MOCDIR").setValue(mocdir);

    project.property("INCLUDEPATH.GENERATED").append(mocdir);

    var rule = project.rule("ensure_mocdir");
    rule.outputFiles = mocdir;
    rule.commands = "#(e)$$MKSPEC.MKDIR $$[OUTPUT.0]";
}

function moc_mocables()
{
###
    QBUILD_MOC_IMPL = "#(eh)echo moc $$[INPUT.0]" \
                      "#(E)$$QTDIR/bin/moc -nw $${COMPILER.DEFINES} $${COMPILER.INCLUDEPATH} $$[INPUT.0] -o $$[OUTPUT.0]"

    QBUILD_MOC_DEPS_IMPL = "#(Et)$$MKSPEC.CXX -MM $${COMPILER.CXXFLAGS} $${COMPILER.DEFINES} $${COMPILER.INCLUDEPATH} -o $$[OUTPUT.1] -xc++ $$[INPUT.0]"

    QBUILD_MOC_ARGS_TEST = "$$QTDIR/bin/moc -nw $${COMPILER.DEFINES} $${COMPILER.INCLUDEPATH}"
###


    var moc = project.property("MOC");
    var moc_base_path = moc.property("MOCDIR").strValue();

    create_args_rule({
        name: "moc_args_test",
        file: moc_base_path+"/moc.args",
        contents: project.property("QBUILD_MOC_ARGS_TEST").strValue(),
        prereq: "#(oh)ensure_mocdir",
        depend_on_qt: 1
    });

    var mocables = moc.property("MOCABLES");
    var mocrules = moc.property("MOCRULES");

    var sources = project.find("TYPE", "MOC_SOURCES");

    for ( var ii in sources ) {
        var baseobj = qbuild.object(sources[ii]);
        moc_mocables_for_obj(baseobj, moc_base_path, mocables, mocrules, moc);
    }
}

function moc_mocables_for_obj(obj, path, mocables, mocrules, moc)
{
    /*
       Each entry in the SOURCES list will cause the generation of a moc output
       file of the form <filename>.moc.  To simplify the generation (and 
       regeneration) of dependencies, this file is generated regardless of
       whether the source file in question contains #include "<filename>.moc".

       If the source file does include the generated moc output, for automated 
       dependency generation to work, it must have been created when the 
       dependency generation occurs.  As such, for each source file, the
       dependency generation stage will depend on the completion of the moc
       output.

       Output files are written to the directory returned by mocables_basepath()
      */
    var sources_list = obj.property("SOURCES").value();
    var is_generated = obj.property("TYPE").contains("GENERATED_SOURCES");

    var sources = obj.property("SOURCES");

    for ( var ii in sources_list ) {
        var base = File(sources_list[ii]).name();
        var name = path + "/" + base + ".moc";
        var source;

        if ( is_generated ) {
            source = sources_list[ii];
        } else {
            source = source_file(sources_list[ii]);
            if (!source) {
                project.warning("Missing moc input file " + sources_list[ii]);
                continue;
            }
        }

        // We need a list to use in the clean_mocables rule
        mocables.append(name);

        // Dependency generation for the source file depends on having created
        // this moc file
        sources.property("~" + ii + ".DEPENDS_DEPENDS").append(name);

        // The actual rule
        var rule = project.rule(basename(name));
        rule.category = "moc";
        rule.outputFiles = name;
        rule.inputFiles.append(source);
        rule.inputFiles.append("#(f)$$include_depends_rule("+cpp_compiler_dep_file(source)+")");
        rule.commands = project.property("QBUILD_MOC_IMPL").value();
        rule.prerequisiteActions.append("moc_args_test");
        rule.prerequisiteActions.append("#(oh)headers");
        rule.prerequisiteActions.append("#(oh)compiler_source_depends");
        rule.prerequisiteActions.append("#(oh)build_moc_rule");
        rule.prerequisiteActions.append("#(oh)ensure_mocdir");

        // We need a list to use in the mocables rule
        mocrules.append(rule.name);
    }

    /*
       Each entry in the HEADERS list will cause the generation of a moc output
       file of the form moc_<filename>.cpp.  To simplify the generation (and
       regeneration) of dependencies, this file is generated regardless of 
       whether the header file in question contains any Q_OBJECT macros.

       The generated moc files will be added as a compilable cpp file list to
       ensure that they are built appropriately.

       Output files are written to the directory returned by mocables_basepath()
     */
    var headers_list = obj.property("HEADERS").value();
    var mocsources = moc.property("SOURCES");
    var header_exceptions = obj.property("MOC_COMPILE_EXCEPTIONS");
    var moc_ignore = obj.property("MOC_IGNORE");

    moc.property("TYPE").unite("CPP_SOURCES");
    moc.property("TYPE").unite("GENERATED_SOURCES");
    moc.property("TYPE").unite("NON_TRANSLATABLE");

    var depends_depends = project.property("COMPILER.DEPENDS_DEPENDS");

    for (var ii in headers_list) {
        if (moc_ignore.contains(headers_list[ii]))
            continue;

        var base = File(headers_list[ii]).name();
        var name = path+"/moc_"+base+".cpp";

        var source = source_file(headers_list[ii]);
        if (!source) {
            project.warning("Missing moc input file " + headers_list[ii]);
            continue;
        }

        // We need a list to use in the clean_mocables rules
        mocables.append(name);

        if (!header_exceptions.contains(headers_list[ii])) {
            mocsources.append(name);
        } else {
            depends_depends.append(name);
        }

        // The actual rule
        var rule = project.rule(basename(name));
        rule.category = "moc";
        rule.outputFiles = name;
        rule.inputFiles = source;
        rule.inputFiles.append("#(f)$$include_depends_rule("+cpp_compiler_dep_file(source)+")");
        rule.prerequisiteActions.append("moc_args_test");
        rule.prerequisiteActions.append("#(oh)headers");
        rule.prerequisiteActions.append("#(oh)compiler_source_depends");
        rule.prerequisiteActions.append("#(oh)build_moc_rule");
        rule.prerequisiteActions.append("#(oh)ensure_mocdir");
        rule.commands = project.property("QBUILD_MOC_IMPL").value();
        rule.outputFiles.append(cpp_compiler_dep_file(source));
        rule.commands.append(project.property("QBUILD_MOC_DEPS_IMPL").value());
        rule.prerequisiteActions.append("#(oh)ensure_objdir");

        // We need a list to use in the clean_mocables rules
        mocrules.append(rule.name);
    }
}

function moc_external_rules()
{
    /*
       Three external targets are generated to make interacting with moc output 
       easier:
           + mocables: Generate all moc files
           + clean_mocables: Remove all moc files
           + force_clean_mocables: Remove all files in the moc output directory
                                   that match the moc_*.cpp or *.moc pattern
       Additionally, the clean_mocables target is added as a prerequisite for
       the clean rule, and the force_clean_mocables as a prerequisite for the
       force_clean rule.
     */

###
    QBUILD_MOC_CLEAN = "$$MKSPEC.DEL_FILE $${MOC.MOCABLES}"
    QBUILD_MOC_CLEAN += "$$MKSPEC.DEL_FILE $${MOC.MOCDIR}/moc.args"
    QBUILD_MOC_CLEAN_FORCE  = "$$MKSPEC.DEL_FILE $${MOC.MOCDIR}/*.moc"
    QBUILD_MOC_CLEAN_FORCE += "$$MKSPEC.DEL_FILE $${MOC.MOCDIR}/moc_*.cpp"
    QBUILD_MOC_CLEAN_FORCE += "$$MKSPEC.DEL_FILE $${MOC.MOCDIR}/moc.args"

    !isEmpty(MOC.MOCABLES) {
        MOC.RULES.mocables.TYPE = RULE
        MOC.RULES.mocables.help = "Generate all moc files"
        for(r,MOC.MOCRULES) {
            MOC.RULES.mocables.prerequisiteActions += "#(h)"$$r
        }
        MOC.RULES.mocables.prerequisiteActions += "#(o)headers"

        MOC.RULES.clean_mocables.TYPE = RULE
        MOC.RULES.clean_mocables.help = "Remove moc generated files"
        MOC.RULES.clean_mocables.commands = $$QBUILD_MOC_CLEAN

        MOC.RULES.clean.TYPE = RULE
        MOC.RULES.clean.prerequisiteActions = clean_mocables
    }

    MOC.RULES.force_clean_mocables.TYPE = RULE
    MOC.RULES.force_clean_mocables.help = "Remove all *.moc and moc_*.cpp files"
    MOC.RULES.force_clean_mocables.commands = $$QBUILD_MOC_CLEAN_FORCE
    
    MOC.RULES.force_clean.TYPE = RULE
    MOC.RULES.force_clean.prerequisiteActions = force_clean_mocables
###
}

