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

\extension templates

The templates extension contains common build templates.

*/

/*!

\qbuild_variable TEMPLATE
\ingroup templates_extension

Specifies the template of the project. Valid templates are:
\list type=dl
\o blank This is a stub project (can still export dependencies)
\o app This project builds a application.
\o lib This project builds a library.
\endlist

The following values are provided for clarity when writing qbuild.pro files:
\list type=dl
\o plugin This project builds a plugin. Equivalent to \c{TEMPLATE=lib CONFIG+=plugin}.
\endlist

*/

function templates_init()
{
###
    QMAKE.FINALIZE.templates.CALL = templates_finalize
    QMAKE.FINALIZE.templates.RUN_AFTER_ME = cpp_compiler depends
    TEMPLATE=blank

    # Specifies the available templates
    TEMPLATE.app = templates_app
    TEMPLATE.lib = templates_lib
    TEMPLATE.blank = templates_blank
###
    project.property("TEMPLATE").watch("templates_change_template");
}

function templates_change_template()
{
    var template = project.property("TEMPLATE").strValue();
    if ( template == "app" || template == "lib" ) {
        // FIXME this doesn't work
        //project.property("CONFIG").unite("cpp_compiler");
###
        CONFIG*=cpp_compiler
###
    }
    if ( template == "lib" ) {
###
        CONFIG*=headers
###
    }
    if ( template == "plugin" ) {
        // Syntactic sugar. Really just TEMPLATE=lib, CONFIG+=plugin
###
        TEMPLATE=lib
        CONFIG+=plugin
###
    }
    if ( template == "subdirs" ) {
###
    TEMPLATE=blank
    CONFIG-=$$DEFAULT_CONFIG
###
    }
}

function templates_finalize()
{
    var template = project.property("TEMPLATE").strValue();
    var templates = project.property("TEMPLATE");

    if ( templates.isProperty(template) ) {
        //project.message("qbuild.invoke "+templates.property(template).strValue());
        qbuild.invoke(templates.property(template).strValue());
    } else {
        var avail_templates = templates.properties();
        project.warning("You must specify a valid TEMPLATE. You specified "+template+". Available templates are: "+
            templates.properties().join(", "));
    }
}

function templates_app()
{
###
    COMPILER.OUTPUT = exe
    isEmpty(TARGET):warning("You must specify a TARGET name to use the app template")
###
}

function templates_lib()
{
###
    staticlib:COMPILER.OUTPUT = staticlib
    else:COMPILER.OUTPUT = lib
    isEmpty(TARGET):warning("You must specify a TARGET name to use the lib template")

    singleexec_link {
        requires(embedded)
        TEMPLATES.DEP1.libs.TYPE=DEPENDS PERSISTED
        TEMPLATES.DEP1.libs.EVAL=\
            "WLIBS*=-l"$$TARGET
    } else {
        TEMPLATES.DEP1.libs.TYPE=DEPENDS PERSISTED SDK
        TEMPLATES.DEP1.libs.EVAL="LIBS=-l"$$TARGET" $$LIBS"
    }
    TEMPLATES.DEP2.libs.TYPE=DEPENDS
    TEMPLATES.DEP2.libs.EVAL=\
        "!staticlib {"\
            "target_pre.TYPE=RULE"\
            "target_pre.prerequisiteActions*="""$$project()"target"""\
        "} else {"\
            "requires(use_pic|staticlib|if(equals(TEMPLATE,app):!quicklaunch))"\
        "}"
###
}

function templates_blank()
{
    // Doesn't do anything
}

