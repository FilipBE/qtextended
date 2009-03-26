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

\extension functions

\sa Functions

The functions extension contains miscelaneous functons that the rest of the build system uses.
Most of these functions can be called from both QBuild Script and JavaScript.
The usage demonstates this as follows.

Functions that are intended to be called from JavaScript are documented like this.
\code
func("arg");
\endcode

Functions that are intended to be called from QBuild Script are documented like this.
\code
###
func(arg)
###
\endcode

Most functions can be called from both environments but there are some things to be aware of.
QBuild Script variables are stored as lists. When passed to a JavaScript function, a QBuild
Script variable becomes an array. However, single values are passed to JavaScript as values.
JavaScript functions expecting items that are not strings or arrays will fail when called from
QBuild Script.

*/

function functions_init()
{
###
    CONFIG+=php
###
}

/*!

\function echo
\ingroup functions_extension

\usage

\code
echo("message");
echo("cat", "message");
\endcode

\description

If category is omitted the string is unconditionally printed.
Otherwise you need to pass -js_debug cat,cat to enable the
categories you want to see.

Note that \c _all means all categories and \c _top means only output
from the top project (not sub-projects). You can pass \c _list to see
what categories there are (the first message from each category will
be printed). Note that the list of printed categories is per-project
so you should generally combine \c _top with \c _list.

*/
function echo(category, string)
{
    if ( !string ) {
        project.message("_none: "+category);
        return;
    }
    if ( !project.isProperty("_echo") ) {
        project.property("_echo").setValue(String(qbuild.invoke("globalValue", "js_debug")).split(","));
    }
    var _echo = project.property("_echo");
    if ( !_echo.contains("_top") || project.absName == qbuild.root.node() ) {
        if ( _echo.contains("_list") ) {
            var list = project.property("_echo_list");
            if ( !list.contains(category) ) {
                list.append(category);
                project.message(category+": "+string);
            }
            return;
        }
        if ( _echo.contains(category) || _echo.contains("_all") )
            project.message(category+": "+string)
    }
}

/*!

\function requires
\ingroup functions_extension

\usage

\code
###
requires(statement)
###
\endcode

\description

The statement (which must be QBuild Script) is executed and if it returns
false, the project is disabled.

*/
function requires(condition, message)
{
    if ( !project.disabledReason() ) {
        var notmet = "not met";
        if (message)
            notmet += " - "+message;
### condition notmet
        <0> {
        } else {
            echo("disable", "disabling project because required condition(<0>) <1>.")
            # FIXME For now this is quiet but it indicates a mismatch between projects.pri
            # and current project so it should eventually be made loud (and the warnings
            # fixed by ensuring the same conditions apply in both cases)
            disable_project("Required condition (<0>) <1>.")
        }
###
    }
}

/*!

\function source_file
\ingroup functions_extension

\usage

\code
foo = source_file("foo.cpp");
foo = source_file("foo.cpp", obj);
\endcode

\description

Locate a source file using the property \l SOURCEPATH from the project.
If obj is passed then SOURCEPATH is used from it instead of from the project.

*/
function source_file(file, baseobj)
{
    var absFile = project.file(file);

    if (!absFile) {
        if (!baseobj)
            baseobj = project;
        var search_paths = Array().concat(baseobj.property("SOURCEPATH").value(), baseobj.property("VPATH").value());

        for (var ii = 0; search_paths && !absFile && ii < search_paths.length; ++ii)
        {
            var filename = search_paths[ii] + "/" + file;
            absFile = project.file(filename);
        } 
    }

    if (absFile)
        return absFile.filesystemPath();
    else
        return null;
}

/*!

\function eval_escape
\ingroup functions_extension

\usage

\code
foo = eval_escape(commands);
\endcode

\description

Escape the commands by replacing \c{"} with \c{""}.

*/
function eval_escape(line)
{
    var ret = line;
    return ret.replace(/"/g, '""');
}

/*!

\function dump_property
\ingroup functions_extension

\usage

\code
dump_property(obj);
\endcode

\description

Dump an object (including all sub-properties) if the js_debug category "dump" is enabled.

*/
function dump_property(prop, recurse)
{
    echo("dump", "Dumping "+prop.absName);
    var _echo = project.property("_echo");
    if ( !_echo.contains("_top") || project.absName == qbuild.root.node() )
        if ( _echo.contains(category) || _echo.contains("_all") )
            force_dump_property(prop, recurse);
}

/*!

\function dump_property
\ingroup functions_extension

\usage

\code
force_dump_property(obj);
\endcode

\description

Dump an object (including all sub-properties).

*/
function force_dump_property(prop, recurse)
{
    var propname = prop.absName;
    // strip out the path and project name
    propname = propname.replace(/^.*\/[^\.]+\./, "");
    if ( !recurse )
        project.message("Dumping property "+propname);
    project.message("    "+propname+"="+prop.strValue());;
    var subs = prop.properties();
    for ( var ii in subs ) {
        var sub = subs[ii];
        force_dump_property(prop.property(sub), 1);
    }
}

/*!

\function device_overrides
\ingroup functions_extension

\usage

\code
###
foo=$$device_overrides(/etc/default/Trolltech/foo.conf)
###
\endcode

\description

Returns the path to the selected file based on the solution filesystem.
This primarily exists to keep old code working and is almost a no-op
(though it does return nothing for files that don't exist).

*/
function device_overrides(file)
{
    var f = project.file(file);
    if ( !f || !f.exists() )
        return "";
    return f.filesystemPath();
}

/*!

\function paths
\ingroup functions_extension

\usage

\code
###
foo=$$paths(.,project)
###
\endcode

\description

This is like $$path() but it returns multiple entries for ,project
(based on how your solution is stacked). It's designed for the server
which adds . to INCLUDEPATH (but needs to support stacked directories).

*/
function paths(path,qualifier)
{
    if ( qualifier == "project" ) {
        var list = project.paths(path);
        var ret = new Array;
        for ( var ii in list ) {
            if ( !list[ii].exists() )
                continue;
            var p = list[ii].filesystemPath();
            if ( p != project.buildPath(path) )
                ret.push(p);
        }
        return ret;
    } else if ( qualifier == "generated" ) {
        return project.buildPath(path);
    } else if ( qualifier == "existing" ) {
        var list = project.paths(path);
        for ( var ii in list ) {
            if ( list[ii].exists() ) {
                return list[ii].filesystemPath();
            }
        }
        return undef;
    }
}

// Add HOST_ or TARGET_ INCLUDEPATH/LIBS/DEFINES as appropriate
function add_extra_paths()
{
    var h = "HOST";
    if ( project.resetReason().contains("embedded") )
        h = "TARGET";
    project.property("INCLUDEPATH").append(project.property(h+"_INCLUDEPATH").value());
    project.property("LIBS").append(project.property(h+"_LIBS").value());
    project.property("DEFINES").append(project.property(h+"_DEFINES").value());
}

/*!

\function add_input_files_to_rule
\ingroup functions_extension

\usage

\code
###
add_input_files_to_rule(rule,$$path(/path,project),*.pro)
###
\endcode

\description

This searches in the specified path for files that match a glob and adds them to the rule's dependencies.
This is most useful for third party build system integration (since a potentially large number of
files changing mean you'll need to rebuild).

*/
function add_input_files_to_rule(rulename, path, glob)
{
    var rule = project.rule(rulename);
    if ( glob ) {
        var iter = new PathIterator(path);
        var ret = add_input_files_to_rule_recursive_bit(iter, glob);
        //project.message("got from ret "+ret.join(", "));
        rule.inputFiles.append(ret);
    } else {
        //project.info("Adding path "+path);
        rule.inputFiles.append(path);
    }
}

function add_input_files_to_rule_recursive_bit(iter, glob)
{
    var ret = new Array;
    var thisdir = iter.files(glob);
    for ( var i in thisdir ) {
        //project.message("Adding "+thisdir[i].filesystemPath());
        ret.push(thisdir[i].filesystemPath());
    }
    var subpaths = iter.paths("*");
    for ( var ii in subpaths ) {
        //project.message("subpath "+subpaths[ii]);
        var sub = add_input_files_to_rule_recursive_bit(iter.cd(subpaths[ii]), glob);
        if ( sub.length ) {
            //project.message("got from sub "+sub.join(", "));
            ret = ret.concat(sub);
        }
    }
    return ret;
}

/*!

\function run_if_not_found
\ingroup functions_extension

\usage

\code
###
rule.tests="$$run_if_not_found(foo.d)"
###
\endcode

\description

This tests for .d inputs and causes the rule to run if any of them are not present.
It does not cause the rule to run if any of the files have changed.

*/
function run_if_not_found(depfile)
{
    var deps = qbuild.invoke("include_depends_rule", depfile);
    if ( deps ) {
        for ( var ii in deps ) {
            var file = deps[ii].replace(/#\(os\)/, "");
            var sfile = project.file(file);
            if ( !sfile || !sfile.exists() ) {
                project.message("Missing "+file);
                return "1";
            }
        }
    }
    return "";
}

/*!

\function add_module
\ingroup functions_extension

\usage

\code
###
add_module(pim)
###
\endcode

\description

This is how you pull in a module in projects.pri

*/
function add_module(module)
{
### module
    exists(/src/module_<0>.pri):include(/src/module_<0>.pri)
    else:error(Missing module_<0>.pri)
###
}

/*!

\function soft_disable_project
\ingroup functions_extension

\usage

\code
soft_disable_project("message");
\endcode

\description

This does a soft disable of a project. The default_sub rule prints a warning
but the default rule works as normal.

*/
function soft_disable_project(message, flag)
{
### message flag
    CONFIG+=soft_disable
    SOFT_DISABLE_MESSAGE+="<0>"
    CONFIG+=runlast
    RUNLAST+="do_soft_disable(<1>)"
###
}

function do_soft_disable(flag)
{
    var message = project.property("SOFT_DISABLE_MESSAGE").value().join(" ");
    var rules = project.property("SUBDIRS_RULES").value();
    //if ( flag ) rules = flag.split(" ");
    for ( var ii in rules ) {
        var rule = project.rule(rules[ii]+"_sub");
        rule.prerequisiteActions = "";
        if ( qbuild.invoke("globalValue", "showDisabled").join("") == 1 )
            rule.commands.append("#(ve)echo \"Project "+project.name+" is normally disabled. ("+message+")\"");
    }
    if ( /* !flag && */ !project.config("no_qbuild_pro") ) {
        var rule = project.rule("check_enabled");
        var cmd = "#(ve)echo \"Project "+project.name+" is normally disabled. ("+message+")\"";
        if ( qbuild.invoke("globalValue", "no_load_soft_disabled") == 1 )
            cmd += ";exit 1";
        rule.commands.append(cmd);
    }
}

/*!

\function hard_disable_project
\ingroup functions_extension

\usage

\code
hard_disable_project("message");
\endcode

\description

This sets a flag and then calls \l disable_project to disable the project.
The flag will cause an error to occur if this project is used.

*/
function hard_disable_project(message)
{
    project.reset("disable_error");
    qbuild.invoke("disable_project", message);
}

/*!

\function create_args_rule
\ingroup functions_extension

\usage

\code
create_args_rule({
    name: "rulename",
    file: "foo.args",
    contents: "gcc -c $$DEFINES -o $$[OUTPUT.0] $$[INPUT.0]"
});

var rule = create_args_rule({
    name: "rulename",
    file: dir+"/foo.args",
    contents: "gcc -c $$DEFINES -o $$[OUTPUT.0] $$[INPUT.0]"
    prereq: "#(oh)ensure_dir"
});
\endcode

\description

This function assists in creating an "args test" rule so that changes to the
arguments of a utility will cause that utility to be run again.

The second example demonstrates the complete function, with the optional prereq
value and capturing the rule object.

*/
function create_args_rule(obj)
{
    var rule = project.rule(obj.name);
    rule.outputFiles = obj.file;
    // If there are any #(...) bits it'll bork the test so remove them now
    var output = obj.contents.replace(/#\([^\)]+\)/g, "");
    rule.tests = "$$not($$testFile(!$$[OUTPUT.0.ABS],"+output+"))";
    rule.commands = "#(e)$$writeFile(!$$[OUTPUT.0.ABS],"+output+")";
    var list = ["prerequisiteActions", "inputFiles"];
    if ( obj.prereq )
        rule.prerequisiteActions.append(obj.prereq);
    if ( obj.depend_on_qt ) {
        var qt = "/src/libraries/"+(project.config("embedded")?"qtopiacore":"qt");
        if ( ( !project.config("system_qt") || project.config("embedded") ) &&
             project.config("in_build_tree") && qbuild.root.isSubproject(project.sproject(qt)) )
        {
            rule.prerequisiteActions.append(qt+"/qt");
        }
    }
    return rule;
}

function setup_target_rules()
{
###
    setup_target_rules [
        default.TYPE=RULE
        default.prerequisiteActions*=target
        target.TYPE=RULE
        target.prerequisiteActions*=target_pre target_post
        target.serial=true
    ]
###
}

/*!

\function define_string
\ingroup functions_extension

\usage

\code
###
DEFINES+=FOO=$$define_string(foo.cpp)
###
\endcode

\description

This function correctly escapes a value so that it is seen by the compiler as a string.

*/
function define_string(val)
{
    if ( val instanceof Array )
        val = val.join(" ");
    var ret = "\"\"\\\"\""+val+"\\\"\"\"\"";
    return ret;
}

/*!

\function define_value
\ingroup functions_extension

\usage

\code
###
DEFINES+=FOO=$$define_value("(1<<2)")
###
\endcode

\description

This function correctly escapes a value so that it is not abused by the shell.
The value is passed in literally, not as a string. If you want the value to be
available as a string, use define_string().

*/
function define_value(val)
{
    if ( val instanceof Array )
        val = val.join(" ");
    var ret = "\"\"\""+val+"\"\"\"";
    return ret;
}

/*!

\function array_contains
\ingroup functions_extension

\usage

\code
if ( array_contains(list, "foo") ) {
    ...
}
\endcode

\description

This function provides an array.contains method that JavaScript lacks.

*/
function array_contains(list, val)
{
    for ( var ii in list ) {
        if ( list[ii] == val )
            return 1;
    }
    return 0;
}

