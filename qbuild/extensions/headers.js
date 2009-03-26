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

\extension headers

The headers extension handles setting up the include directory.

For basic usage, no special actions are needed. If you need to install headers
but you cannot do TEMPLATE=lib then simply load the headers extension manually.

\code
CONFIG+=headers
\endcode

Public headers go in HEADERS. Private headers (not in the include directory)
go in PRIVATE_HEADERS. Private headers that you want in the include directory
go in SEMI_PRIVATE_HEADERS. Note that semi-private headers will be available
as \c{private/foo_p.h}.

\section1 Variables

\table
\header \o Type \o Name \o Description
\row \o Input
     \o TARGET
     \o Headers are installed to QtopiaSdk:/include/$$MODULE_NAME
\row \o Output
     \o HEADERS_SYNCQTOPIA
     \o The location of the syncqtopia script is stored in this variable.
\endtable

*/

/*!

\qbuild_variable HEADERS
\brief The HEADERS variable lists header files used by the project.
For libraries that are modules, these headers are installed into the SDK.

*/

/*!

\qbuild_variable NON_PUBLIC_HEADERS
\brief The NON_PUBLIC_HEADERS variable lists header files used by the project.
These headers are not installed into the SDK.

*/

/*!

\qbuild_variable PRIVATE_HEADERS
\brief The PRIVATE_HEADERS variable lists private header files used by the project.
These headers are not installed into the SDK.

*/

/*!

\qbuild_variable SEMI_PRIVATE_HEADERS
\brief The SEMI_PRIVATE_HEADERS variable lists private headers that other projects will use.
These headers are not installed into the SDK.

*/

/*!

\qbuild_variable PUBLIC_HEADERS_OVERRIDE
\brief The PUBLIC_HEADERS_OVERRIDE variable is used to allow \c{_p.h} files to be installed as public headers.

Ths is provided for third party projects which have files ending in \c{_p.h} that are public headers.

\code
HEADERS=foo_p.h
PUBLIC_HEADERS_OVERRIDE=foo_p.h
\endcode

*/

/*!

\qbuild_variable HEADERS_NAME
\brief The HEADERS_NAME variable allows a name other than MODULE_NAME to be used for the include directory.

You should not use this in most cases.

*/

function headers_init()
{
###
    CONFIG*=conditional_sources
    QMAKE.FINALIZE.headers.CALL = headers_finalize
    QMAKE.FINALIZE.headers.RUN_BEFORE_ME = conditional_sources
    QMAKE.FINALIZE.headers.RUN_AFTER_ME = rules depends
### 
} 

function headers_finalize()
{
    if ( !project.config("headers") )
        return;

    headers_conf();

    // The other extensions don't know about PRIVATE and SEMI_PRIVATE headers so merge them into HEADERS
###
    HEADERS*=$$PRIVATE_HEADERS $$SEMI_PRIVATE_HEADERS $$NON_PUBLIC_HEADERS
###
}

function headers_conf()
{
    var target;
    if ( project.isProperty("HEADERS_NAME") )
        target = project.property("HEADERS_NAME").strValue();
    else
        target = project.property("MODULE_NAME").strValue();

    // Can't continue if we have nowhere to install headers!
    if ( target == "" )
        return;

    echo("headers", "Using target "+target);

    // We need syncqtopia to create the headers
    var headers_syncqtopia = project.property("HEADERS_SYNCQTOPIA");
    var paths = solution.pathMappings("/extensions/syncqtopia");
    for (var ii = 0; ii < paths.length; ++ii) {
        var file = project.filesystemFile(paths[ii]);
        if ( file != null ) {
            headers_syncqtopia.setValue(file.path());
            break;
        }
    }
    if ( headers_syncqtopia.strValue() == "" ) {
        project.warning("Unable to locate syncqtopia script needed for Qt-style headers.");
        return;
    }

    // public headers go into the SDK
    if ( !project.property("HEADERS").isEmpty() ) {
        var include = project.buildPath("QtopiaSdk:/include");
        var path = include+"/"+target;
        var mpath = include+"/"+target+"/module";

        project.property("INCLUDEPATH").append(path);
        project.property("INCLUDEPATH").append(mpath);

### path mpath
        HEADERS.DEP.headers.headers.TYPE=DEPENDS PERSISTED SDK
        HEADERS.DEP.headers.headers.EVAL=\
            "INCLUDEPATH*=""<0>"""\
            "INCLUDEPATH*=""<1>"""
        # Build headers before using them!
        HEADERS.DEP.depend.headers.TYPE=DEPENDS
        HEADERS.DEP.depend.headers.EVAL=\
            "compiler_source_depends.TYPE=RULE"\
            "compiler_source_depends.prerequisiteActions+=""#(h)"$$project()"headers"""
###
        headers_makeHeaders(project.property("HEADERS").value(), include, target, project.property("CONFIG").contains("qt"));
        headers_makeModuleSymlink(mpath, target);
        headers_makeClean(path);
    }

    // non-public and semi-private headers go into the build tree
    if ( !project.property("NON_PUBLIC_HEADERS").isEmpty() || !project.property("SEMI_PRIVATE_HEADERS").isEmpty() ) {
        var include = project.buildPath("/include");
        var path = include+"/"+target;
        var mpath = include+"/"+target+"/module";

        project.property("INCLUDEPATH").unite(path);
        project.property("INCLUDEPATH").unite(mpath);

### path mpath
        HEADERS.DEP.buildheaders.headers.TYPE=DEPENDS PERSISTED
        HEADERS.DEP.buildheaders.headers.EVAL=\
            "INCLUDEPATH*=""<0>"""\
            "INCLUDEPATH*=""<1>"""
        # Build headers before using them!
        HEADERS.DEP.depend.headers.TYPE=DEPENDS
        HEADERS.DEP.depend.headers.EVAL=\
            "compiler_source_depends.TYPE=RULE"\
            "compiler_source_depends.prerequisiteActions+=""#(h)"$$project()"headers"""
###

        if ( !project.property("NON_PUBLIC_HEADERS").isEmpty() )
            headers_makeHeaders(project.property("NON_PUBLIC_HEADERS").value(), include, target, false);

        if ( !project.property("SEMI_PRIVATE_HEADERS").isEmpty() )
            headers_makeHeaders(project.property("SEMI_PRIVATE_HEADERS").value(), include, target+"/private", false);

        headers_makeModuleSymlink(mpath, target);
        headers_makeClean(path);
    }
}

function headers_makeHeaders(files, include, target, qt_style_headers)
{
    var path = include+"/"+target;
    var headers = headers_getrule();
    var public__p_headers = project.property("PUBLIC_HEADERS_OVERRIDE");

    for ( var ii in files ) {
        var header = basename(files[ii]);
        var outfile = path+"/"+header;
        var infile = source_file(files[ii]);

        if ( File(header).name().match(/_p$/) && !path.match(/\/private$/) && !public__p_headers.contains(header) ) {
            project.warning("Header "+header+" appears to be a private header. Please list it in SEMI_PRIVATE_HEADERS instead of HEADERS. If this file does need to be a public header, list it in PUBLIC_HEADERS_OVERRIDE.");
            continue;
        }

        if ( infile == null || infile == "" ) {
            if ( qbuild.invoke("rule_for_file", files[ii]) ) {
                // This is a generated file
                infile = files[ii];
            } else {
                project.warning("Missing header: " + files[ii]);
                continue;
            }
        }

        var generate_rule = project.rule();
        generate_rule.inputFiles = infile;
        generate_rule.outputFiles.append(outfile);
        generate_rule.other.append(path);
        generate_rule.commands.append("#(eh)echo $$shellQuote(Create header "+outfile+")");
        generate_rule.commands.append("#(en)$$MKSPEC.MKDIR $$shellQuote($$[OTHER.0])");
        if ( qt_style_headers ) {
            var cmd = "#(e)$$shellQuote($$HEADERS_SYNCQTOPIA)";
            if ( project.config("hide_symbols") ) {
                cmd += " -needs-export";
            }
            cmd += " $$shellQuote("+path+") $$[INPUT.0]";
            generate_rule.commands.append(cmd);
        }
        if ( path.match(/\/private$/) ) {
            generate_rule.commands.append("#(vE)echo '#include \""+infile+"\"' > \""+outfile+"\"");
        } else {
            generate_rule.commands.append("#(vE)install -m 0644 \""+infile+"\" \""+outfile+"\"");
        }
        headers.prerequisiteActions.append("#(h)"+generate_rule.name);
    }
}

function headers_getrule()
{
    var headers = project.rule("headers");
    headers.help = "Generate library headers";
    return headers;
}

function headers_makeModuleSymlink(mpath, target)
{
    var rule = project.rule();
    // Bail if the symlink already exists
    rule.commands.append("#(es)test ! -h $$shellQuote("+mpath+"/"+target+")");
    // Remove a pre-existing directory
    rule.commands.append("#(E)if test -d $$shellQuote("+mpath+"/"+target+"); then rm -r $$shellQuote("+mpath+"/"+target+"); fi");
    rule.commands.append("#(E)$$MKSPEC.MKDIR $$shellQuote("+mpath+")");
    rule.commands.append("#(E)ln -s .. $$shellQuote("+mpath+"/"+target+")");
    var headers = headers_getrule();
    headers.prerequisiteActions.append("#(h)"+rule.name);
}

function headers_makeClean(dir)
{
    var rule = project.rule("clean_headers");
    rule.commands.append("[ ! -d $$shellQuote("+dir+") ] || rm -r $$shellQuote("+dir+")");
    project.rule("clean").prerequisiteActions.append(rule.name);
}

