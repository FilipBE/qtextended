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

\extension installs

The installs extension handles installing applications, libraries
and plugins as well as processing many of basic install hints.
For information about the available install hints, see \l{Install Hints}.

\section1 Variables

\table
\header \o Type \o Name \o Description
\row \o Input
     \o QTOPIA_IMAGE
     \o The location of the image to create.
\row \o Output
     \o INSTALLS_INSTALLPIC
     \o The location of the installpic script is stored in this variable.
\row \o Output
     \o INSTALLS_INSTALLHELP
     \o The location of the installhelp script is stored in this variable.
\endtable

*/

/*!
  \group installhints
  \title Install Hints
  \ingroup qbuild_script
  \brief Documents each of the values that can appear in a .hint.

  \generatelist related

  \sa {QBuild Script}, {installs Extension}
*/

/*!

\qbuild_variable PREIMAGE_RULES
\ingroup installs_extension
\brief Run rules before the "image" rule.

This causes rules to be run before the "image" rule. Note that it makes the "image" rule execute in serial.

*/

/*!

\qbuild_variable POSTIMAGE_RULES
\ingroup installs_extension
\brief Run rules after the "image" rule.

This causes rules to be run after the "image" rule. Note that it makes the "image" rule execute in serial.

*/

function installs_init()
{
###
    CONFIG*=runlast
    QMAKE.FINALIZE.installs [
        CALL = installs_finalize
        RUN_BEFORE_ME = cpp_compiler i18n
        RUN_AFTER_ME = rules
    ]
    RUNLAST+="installs_pre_post_run()"
###
}

/*!

\js_class installs
\ingroup installs_extension

\Methods

\js_member targets(name): list<SolutionFile>

\js_member fetchdata(obj, data): bool

Use it like this:

\code
    var obj = project.property("target");
    var data = {
        files: {
            value: null,
            type: "existingFiles"
        },
        path: {
            value: null,
            type: "imagePath"
        }
    };
    if ( !installs.fetchdata(obj, data) ) return;
\endcode

\a type can be one of:

\list type=dl
\o single Returns a single value
\o array Returns an array of values
\o existingFile Returns a single, existing file
\o existingFiles Returns multiple, existing files
\o imagePath Returns a path in the image
\endlist

\js_member isDirectory(file): bool

Pass in a filename (string) and returns true if it's a directory.

*/

var installs = {
    targets: function(name)
    {
        if (name instanceof Array) {
            var rv = new Array();
            for (var ii in name) {
                var tmp = installs.targets(name[ii]);
                if ( tmp.length )
                    rv = rv.concat(tmp);
            }
            if (rv.length)
                return rv;
            else
                return new Array();
        } 

        var files = project.files(name);
        if (files && files.length)
            return files;
        files = project.paths(name);
        if (files && files.length)
            return files;
        return new Array();
    },

    fetchdata: function(obj, data)
    {
        var ret = true;
        for ( propname in data ) {
            var property = data[propname];
            // Assign defaults (if they're missing)
            if ( typeof(property["optional"]) == "undefined" )
                property["optional"] = false;
            if ( typeof(property["type"]) == "undefined" )
                property["type"] = "single";
            // Check if the property exists
            var exists = obj.isProperty(propname);
            if ( !exists ) {
                if ( !property["optional"] ) {
                    installs_warning("Property "+obj.name+"."+propname+" does not exist!");
                    ret = false;
                }
                continue;
            }
            // Resolve the property
            var type = property["type"];
            var value = null;

            if ( type == "single" || type == "existingFile" ||
                 type == "imagePath" || type == "generatedPath" ) {
                // Pull out a single value
                value = obj.property(propname).strValue();
                // We don't accept values that are empty
                if ( value == "" ) {
                    installs_warning("Property "+obj.name+"."+propname+" (type "+type+") is empty!");
                    ret = false;
                    continue;
                }
            } else if ( type == "array" || type == "existingFiles" ) {
                // Pull out an array
                value = obj.property(propname).value();
                // We don't accept arrays with no elements
                if ( !value.length ) {
                    installs_warning("Property "+obj.name+"."+propname+" (type "+type+") is empty!");
                    ret = false;
                    continue;
                }
            } else {
                installs_warning("Unknown type "+type);
                ret = false;
                continue;
            }

            if ( type == "existingFile" || type == "existingFiles" ) {
                // Look for existing files
                if ( typeof(value) == "string" ) {
                    // Package the single value into an array (so they can use the same code below)
                    var tmp = new Array();
                    tmp.push(value);
                    value = tmp;
                }
                var found = new Array();
                var cont = false;
                for ( vi in value ) {
                    var v = value[vi];
                    var f = installs_get_files(v);
                    if ( f.length ) {
                        f = installs_filter(f);
                        for ( var ii in f ) {
                            echo("installs.files", "prop "+obj.name+": found file "+f[ii].filesystemPath());
                            found.push(f[ii].filesystemPath());
                        }
                    } else if ( type == "existingFiles" ) {
                        if ( obj.property("hint").contains("optional") ) {
                            //project.message("Property "+obj.name+"."+propname+" (type "+type+") has item "+v+" that cannot be located!");
                        } else {
                            installs_warning("Property "+obj.name+"."+propname+" (type "+type+") has item "+v+" that cannot be located!");
                            ret = false;
                            cont = true;
                            break;
                        }
                    }
                    if ( type == "existingFile" ) break;
                }
                if ( cont ) continue;
                // Only for type existingFile
                if ( !found.length ) {
                    if ( !obj.property("hint").contains("optional") )
                        installs_warning("Property "+obj.name+"."+propname+" (type "+type+") has file "+value[0]+" that cannot be located!");
                    ret = false;
                    continue;
                }
                if ( type == "existingFiles" )
                    value = found;
                else
                    value = found[0];
            }
            if ( type == "imagePath" ) {
                // Give us a path in the image
                value = project.property("QTOPIA_IMAGE").strValue()+(File(value).isRelative()?"/":"")+value;
            }
            if ( type == "generatedPath" ) {
                // If we've got an absolute path, leave it alone
                // If we've got a relative path, map it under the project's build directory
                if ( File(value).isRelative() )
                    value = project.buildPath(value);
            }

            property["value"] = value;
        }
        return ret;
    },

    dump: function(name, obj)
    {
        for ( prop in obj ) {
            var value = obj[prop];
            if ( value != null && typeof(value) == "function" )
                value = obj[prop]();

            project.message(name+"."+prop+" = "+value+" ("+typeof(value)+")");

            if ( typeof(value) == "object" && value != null ) {
                installs.dump(name+"."+prop, value);
            }
        }
    },

    isDirectory: function(file)
    {
        var f = project.filesystemPaths(file);
        return ( f.length && f[0].isDirectory() );
    }
}

function installs_finalize()
{
    if ( !project.config("installs") )
            return;

###
    QBUILD_INSTALL_APP_IMPL = \
        "#(e)$$MKSPEC.MKDIR $$[OTHER.0]" \
        "$$MKSPEC.INSTALL_PROGRAM $$[INPUT.0.ABS] $$[OTHER.1]"
    QBUILD_INSTALL_PLUGIN_IMPL = \
        "#(e)$$MKSPEC.MKDIR $$[OTHER.0]" \
        "$$MKSPEC.INSTALL_FILE $$shellQuote($$[INPUT.0.ABS]) $$[OTHER.1]"
    QBUILD_INSTALL_LIB_IMPL = \
        "#(e)$$MKSPEC.MKDIR $$[OTHER.0]" \
        "#(e)$$MKSPEC.DEL_FILE $$[OTHER.0]/lib$${COMPILER.TARGET}.*" \
        "$$MKSPEC.INSTALL_FILE $$[INPUT.0.ABS] $$[OTHER.1]"

    release:!isEmpty(MKSPEC.STRIP) {
        QBUILD_INSTALL_APP_IMPL += \
            "$$MKSPEC.STRIP $$[OTHER.1]"
        QBUILD_INSTALL_PLUGIN_IMPL += \
            "$$MKSPEC.STRIP $$MKSPEC.STRIPFLAGS_SHLIB $$[OTHER.1]"
        QBUILD_INSTALL_LIB_IMPL += \
            "$$MKSPEC.STRIP $$MKSPEC.STRIPFLAGS_SHLIB $$[OTHER.1]"
    }
###

    // FIXME this is here for porting only!
    // Set .hint=image for anything in INSTALLS without a hint
    var instobjects = project.property("INSTALLS").value();
    for ( var ii in instobjects ) {
        var obj = project.property(instobjects[ii]);
        if ( obj.property("hint").isEmpty() ) {
            obj.property("hint").setValue("image");
        }
        if ( obj.property("hint").contains("themecfg") ) {
            obj.property("hint").unite("image");
        }
        if ( obj.property("hint").contains("desktop") || obj.property("hint").contains("content") ) {
            obj.property("hint").unite("image");
        }
    }

    installs_hints();
}

function installs_hints()
{
    var objs = project.find("hint");

    for (var ii in objs) {
        var obj = qbuild.object(objs[ii]);
        var hint = obj.property("hint");
        var handled = false;
        if ( hint.contains("disabled") ) {
            handled = true;
            continue;
        }
        if ( hint.contains("image") || hint.contains("desktop") || hint.contains("content") || hint.contains("themecfg") ) {
            installs_hint_image(obj);
            handled = true;
        }
        if ( hint.contains("pics") ) {
            installs_hint_pics(obj);
            handled = true;
        }
        if ( hint.contains("help") ) {
            installs_hint_help(obj);
            handled = true;
        }
        if ( hint.contains("desktop") || hint.contains("content") ) {
            installs_hint_content(obj);
            handled = true;
        }
        if ( hint.contains("script") ) {
            installs_hint_script(obj);
            handled = true;
        }
        if ( hint.contains("dawg") ) {
            installs_hint_dawg(obj);
            handled = true;
        }
        if ( hint.contains("background") ) {
            if ( !project.property("QTOPIA_DISP_WIDTH").isEmpty() && !project.property("QTOPIA_DISP_HEIGHT").isEmpty())
                installs_hint_background(obj);
            else
                installs_hint_image(obj);
            handled = true;
        }
        if ( hint.contains("sxe") && project.config("enable_sxe") ) {
            installs_hint_sxe(obj);
            handled = true;
        }
    }
}

function installs_target(install, imagePath)
{
    var obj = project.property("target");

    var data = {
        path: {
            value: null,
            type: imagePath
        }
    };
    if ( !installs.fetchdata(obj, data) ) return;

    if ( project.property("TEMPLATE").contains("app") ) {
        project.rule("install_target").set = {
            inputFiles: project.property("COMPILER.EXECUTABLE").value(),
            other: project.expression("$$shellQuote("+data.path.value+") $$shellQuote("+data.path.value+"/$$basename($${COMPILER.EXECUTABLE}))"),
            commands: project.property("QBUILD_INSTALL_APP_IMPL").value()
        }
        install.prerequisiteActions.append("install_target");
        installs_process_depends(project.rule("install_target"), obj);
        project.property("pkg.default_targets").unite("install_target");
    } else if ( project.property("TEMPLATE").contains("lib") && project.config("plugin") ) {
        project.rule("install_target").set = {
            inputFiles: project.property("COMPILER.SHAREDLIB").value(),
            other: project.expression("$$shellQuote("+data.path.value+") $$shellQuote("+data.path.value+"/$$basename($${COMPILER.SHAREDLIB}))"),
            commands: project.property("QBUILD_INSTALL_PLUGIN_IMPL").value()
        }
        install.prerequisiteActions.append("install_target");
        installs_process_depends(project.rule("install_target"), obj);
        project.property("pkg.default_targets").unite("install_target");
    } else if ( project.property("TEMPLATE").contains("lib") && !project.config("staticlib") ) {
        project.rule("install_target").set = {
            inputFiles: project.property("COMPILER.SHAREDLIB").value(),
            other: project.expression("$$shellQuote("+data.path.value+") $$shellQuote("+data.path.value+"/$$basename($${COMPILER.SHAREDLIB_SONAME}))"),
            commands: project.property("QBUILD_INSTALL_LIB_IMPL").value()
        }
        install.prerequisiteActions.append("install_target");
        installs_process_depends(project.rule("install_target"), obj);
        project.property("pkg.default_targets").unite("install_target");
    }
}

/*!

\hint image

*/
function installs_hint_image(obj)
{
    if ( obj.name == "target" ) {
        installs_target(installs_getImage(), "imagePath");
        return;
    }
    
    var data = {
        files: {
            optional: true,
            value: null,
            type: "existingFiles"
        },
        path: {
            optional: true,
            value: null,
            type: "imagePath"
        },
        commands: {
            optional: true,
            value: Array(),
            type: "array"
        }
    };
    if ( obj.property("hint").contains("generated") )
        data.files.type = "array";
    if ( !installs.fetchdata(obj, data) ) return;

    var rule = project.rule("install_"+obj.name);

    if ( data.path.value ) {
        rule.other = data.path.value;
        rule.commands.append("#(e)$$MKSPEC.MKDIR $$[OTHER.0]");
    }

    if ( data.files.value ) {
        for ( var ii in data.files.value ) {
            var file = data.files.value[ii];
            rule.inputFiles.append(file);
            if ( installs.isDirectory(file) )
                rule.commands.append("$$MKSPEC.INSTALL_DIR $$path("+file+",existing) $$[OTHER.0]");
            else
                rule.commands.append("$$MKSPEC.INSTALL_FILE $$path("+file+",existing) $$[OTHER.0]");
        }
    }
    for ( var ii in data.commands.value ) {
        rule.commands.append(data.commands.value[ii]);
    }

    if ( !obj.property("hint").contains("desktop") ) {
        var install = installs_getImage();
        install.prerequisiteActions.append(rule.name);
    }
    installs_process_depends(rule, obj);
    project.property("pkg.default_targets").unite(rule.name);
}

function installs_getImage()
{
    var image = project.rule("image");
    image.help = "Install files into the image";
    image.prerequisiteActions.append("image_work");
    var ret = project.rule("image_work");
    return ret;
}

/*!

\hint pics

*/
function installs_hint_pics(obj)
{
    var data = {
        files: {
            value: null,
            type: "existingFiles"
        },
        path: {
            value: null,
            type: "imagePath"
        }
    };
    if ( !installs.fetchdata(obj, data) ) return;

    if ( project.property("INSTALLS_INSTALLPIC").strValue() == "" ) {
        var script = project.property("INSTALLS_INSTALLPIC");
        var file = qbuild.invoke("path", "QtopiaSdk:/src/build/bin/installpic", "generated");
        script.setValue(file);
        if ( script.strValue() == "" ) {
            installs_warning("Unable to locate installpic script needed to handle hint=pics.");
            return;
        }
    }

    var rule = project.rule("install_"+obj.name);

    rule.inputFiles = data.files.value;
    rule.other = data.path.value;
    rule.commands.append("#(e)$$MKSPEC.MKDIR $$[OTHER.0]");

    var cmd = "#(E)$$INSTALLS_INSTALLPIC "+
            "$$shellQuote($$LANGUAGES) "+
            "$$shellQuote($$IMAGE_EXTENSION_ORDER) "+
            "$$shellQuote($$QTOPIA_ICON_SIZE) "+
            "$$shellQuote($$path(.,project)) "+
            "$$[OTHER.0] "+
            "$$[INPUT.ABS]";
    rule.commands.append(cmd);

    // We need to ensure svgtopicture has been built before installpic is run!
    installs_buildBeforeRule(rule, "/src/tools/svgtopicture");
    // We need to ensure pngscale has been built before installpic is run!
    installs_buildBeforeRule(rule, "/src/tools/pngscale");

    var install = installs_getImage();
    install.prerequisiteActions.append(rule.name);
    installs_process_depends(rule, obj);
    project.property("pkg.default_targets").unite(rule.name);
}

function installs_pre_post_run()
{
    var prerun = project.property("PREIMAGE_RULES").value();
    var postrun = project.property("POSTIMAGE_RULES").value();
    var rule = project.rule("image");

    if (prerun && prerun.length) {
        var prerun_rule = project.rule("image.prerun");
        prerun_rule.prerequisiteActions = prerun;
        prerun_rule.serial = true;

        rule.serial = true;
        prereq = rule.prerequisiteActions.value();
        rule.prerequisiteActions = "image.prerun";
        rule.prerequisiteActions.append(prereq);
    }

    if (postrun && postrun.length) {
        var postrun_rule = project.rule("image.postrun");
        postrun_rule.prerequisiteActions = postrun;
        postrun_rule.serial = true;

        rule.serial = true;
        rule.prerequisiteActions.append("image.postrun");
    }
}

/*!

\hint help

*/

function installs_hint_help(obj)
{
    var data = {
        source: {
            value: null,
            type: "existingFile"
        },
        files: {
            value: null,
            type: "array"
        }
    };
    if ( !installs.fetchdata(obj, data) ) return;

    // help.path is hard-coded
    data["path"] = {
        value: project.property("QTOPIA_IMAGE").strValue()+"/help"
    }

    if ( project.property("INSTALLS_INSTALLHELP").strValue() == "" ) {
        var script = project.property("INSTALLS_INSTALLHELP");
        var file = qbuild.invoke("path", "QtopiaSdk:/src/build/bin/installhelp", "generated");
        script.setValue(file);
        if ( script.strValue() == "" ) {
            installs_warning("Unable to locate installhelp script needed to handle hint=help.");
            return;
        }
    }

    var rule = project.rule("install_"+obj.name);

    rule.other.append(data.path.value);
    rule.other.append(data.source.value);
    rule.commands.append("#(e)$$MKSPEC.MKDIR $$[OTHER.0]");
    var cmd = "#(E)$$INSTALLS_INSTALLHELP "+
            "$$shellQuote($$TRANSLATIONS) "+
            "$$shellQuote($$path(.,project)) "+
            "$$shellQuote($$[OTHER.1.ABS]) "+
            "$$[OTHER.0] ";
    for ( var ii in data.files.value ) {
        var file = data.files.value[ii];
        cmd += "$$shellQuote("+file+") ";
    }
    rule.commands.append(cmd);

    var install = installs_getImage();
    install.prerequisiteActions.append(rule.name);
    installs_process_depends(rule, obj);
    project.property("pkg.default_targets").unite(rule.name);
}

/*!

\hint content

*/

/*!

\hint desktop

*/

/*!

\hint prep_db

*/

function installs_hint_content(obj)
{
    var data = {
        files: {
            value: null,
            type: "existingFiles"
        },
        categories: {
            value: new Array(),
            type: "array",
            optional: true
        },
        path: {
            value: null,
            type: "single"
        }
    };
    if ( !installs.fetchdata(obj, data) ) return;

    var content_installer = project.buildPath("QtopiaSdk:/bin/content_installer");

    var rule = project.rule("install_docapi_"+obj.name);

    rule.inputFiles = data.files.value;
    rule.other = data.path.value;
    rule.commands.append("#(e)$$MKSPEC.MKDIR $$QTOPIA_IMAGE");

    rule.commands.append("#(eh)echo content_installer $$[INPUT]");
    var cmd = "#(E)$$shellQuote("+content_installer+") "+
              "$$shellQuote($$QTOPIA_IMAGE/qtopia_db.sqlite) "+
              "$$shellQuote($$QTOPIA_PREFIX) "+
              "$$[OTHER.0] "+
              "$$shellQuote("+data.categories.value.join(" ")+") "+
              "$$[INPUT.ABS]";
    rule.commands.append(cmd);

    cmd = "#(e)"+
        "for qtopia in /tmp/qtopia-*; do "+
        "   if [ -f $$shellQuote($qtopia$$QTOPIA_IMAGE/qtopia_db.sqlite) ]; then "+
        "       for pid in $(fuser $$shellQuote($qtopia$$QTOPIA_IMAGE/qtopia_db.sqlite) 2>/dev/null); do "+
        "           qpe_binary=$(readlink /proc/$pid/exe 2>/dev/null | sed 's/ (deleted)$//g'); "+
        "           if [ $$shellQuote($qpe_binary) = $$shellQuote($$QTOPIA_IMAGE/bin/qpe) ]; then "+
        "               QWS_DISPLAY=QVFb:$(echo $$shellQuote($qtopia) | sed $$shellQuote(sz/tmp/qtopia-zz)) "+
        "                   $$shellQuote("+content_installer+") -qcop "+
        "                   $$shellQuote($qtopia$$QTOPIA_IMAGE/qtopia_db.sqlite) "+
        "                   $$shellQuote($$QTOPIA_PREFIX) "+
        "                   $$shellQuote($$[OTHER.0]) "+
        "                   $$shellQuote("+data.categories.value.join(" ")+") ";

    for ( var ii in data.files.value ) {
        var file = data.files.value[ii];
        cmd += "$$shellQuote($$path("+file+",existing)) ";
    }
    cmd += "            ; "+
        "           fi; "+
        "       done; "+
        "   fi; "+
        "done";
    rule.commands.append(cmd);

    if ( obj.property("hint").contains("prep_db") ) {
        project.rule("prep_db").prerequisiteActions.append(rule.name);

        // This is to help with dependencies. It ensures all of your dependencies have
        // completed their prep_db rules before yours fire (required if you want to
        // reference their categories in your .directory files).
        //
        // An example is that /src/libraries/qtopia adds a MainApplications category
        // that is used by some themes to decide what goes in the Launcher. By depending
        // on /src/libraries/qtopia your prep_db rule will depend on libqtopia's prep_db rule
        // so your .directory files will be able to reference the MainApplications category.
        //
        // Apps that don't want to link to libqtopia should use MODULES*=qtopia::prep_db

        rule.prerequisiteActions.append("prep_db_depends");
### project.name
        INSTALLS.prep_db.TYPE=DEPENDS
        INSTALLS.prep_db.EVAL=\
            "prep_db_depends.TYPE=RULE"\
            "prep_db_depends.prerequisiteActions+=<0>/prep_db"
###
    }

    // We need to ensure content_installer has been built before it is run!
    installs_buildBeforeRule(rule, "/src/tools/content_installer");

    var install = installs_getImage();
    install.prerequisiteActions.append(rule.name);
    installs_process_depends(rule, obj);
}

/*!

\hint script

*/
function installs_hint_script(obj)
{
    var data = {
        files: {
            optional: true,
            value: null,
            type: "existingFiles"
        },
        path: {
            optional: true,
            value: null,
            type: "imagePath"
        },
        commands: {
            optional: true,
            value: Array(),
            type: "array"
        }
    };
    if ( !installs.fetchdata(obj, data) ) return;

    if ( project.property("INSTALLS_SCRIPT_PREPROCESSOR").strValue() == "" ) {
        var script = project.property("INSTALLS_SCRIPT_PREPROCESSOR");
        var file = qbuild.invoke("path", "QtopiaSdk:/src/build/bin/script_preprocessor", "generated");
        script.setValue(file);
        if ( script.strValue() == "" ) {
            installs_warning("Unable to locate script_preprocessor script needed to handle hint=script.");
            return;
        }
    }

    var rule = project.rule("install_"+obj.name);

    rule.inputFiles = data.files.value;
    if ( data.path.value ) {
        rule.other = data.path.value;
        rule.commands.append("#(e)$$MKSPEC.MKDIR $$[OTHER.0]");
    }

    rule.commands.append("$$INSTALLS_SCRIPT_PREPROCESSOR $$[OTHER.0] $$[INPUT.ABS]");

    for ( var ii in data.commands.value ) {
        rule.commands.append(data.commands.value[ii]);
    }

    var install = installs_getImage();
    install.prerequisiteActions.append(rule.name);
    installs_process_depends(rule, obj);
    project.property("pkg.default_targets").unite(rule.name);
}

function installs_process_depends(rule, obj)
{
    var data = {
        depends: {
            optional: true,
            value: Array(),
            type: "array"
        }
    };
    if ( !installs.fetchdata(obj, data) ) return;

    for ( var ii in data.depends.value ) {
        var deprulename = data.depends.value[ii];
        if ( deprulename.match(/^\//) ) {
            var rulename = basename(deprulename);
            var projname = dirname(deprulename);

            // Filter out any projects built in another location (ie. external to the build or source tree)
            if (!project.config("in_build_tree"))
                continue;

            var node = projname+"/";
            if ( qbuild.root.isSubproject(project.sproject(node)) ) {
                rule.prerequisiteActions.append(node+rulename);
                // An extra check so we bail if the required project has been disabled
                rule.prerequisiteActions.append(node+"check_enabled");
            }
        } else {
            rule.prerequisiteActions.append(deprulename);
        }
    }
}

/*!

\hint dawg

*/
function installs_hint_dawg(obj)
{
    var data = {
        files: {
            optional: true,
            value: null,
            type: "existingFiles"
        },
        path: {
            optional: true,
            value: null,
            type: "imagePath"
        },
        commands: {
            optional: true,
            value: Array(),
            type: "array"
        }
    };
    if ( !installs.fetchdata(obj, data) ) return;

    var qdawggen = project.buildPath("QtopiaSdk:/bin/qdawggen");
    // qdawggen needs the -e switch when installing for target systems that have a different endianness to the host
    if (project.config("embedded") && project.property("QTOPIA_HOST_ENDIAN").strValue() != project.property("QTOPIA_TARGET_ENDIAN").strValue())
        qdawggen+=" -e";


    var rule = project.rule("install_"+obj.name);

    if ( data.path.value ) {
        rule.other = data.path.value;
        rule.commands.append("#(e)$$MKSPEC.MKDIR $$[OTHER.0]");
    }

    if ( data.files.value ) {
        for ( var ii in data.files.value ) {
            var file = data.files.value[ii];
            rule.inputFiles.append(file);
            rule.commands.append(qdawggen+" $$[OTHER.0] $$path("+file+",existing)");
        }
    }
    for ( var ii in data.commands.value ) {
        rule.commands.append(data.commands.value[ii]);
    }

    // We need to ensure qdawggen has been built before it is run!
    installs_buildBeforeRule(rule, "/src/tools/qdawggen");

    var install = installs_getImage();
    install.prerequisiteActions.append(rule.name);
    installs_process_depends(rule, obj);
    project.property("pkg.default_targets").unite(rule.name);
}

/*!

\hint background

*/
function installs_hint_background(obj)
{
    var data = {
        files: {
            optional: true,
            value: null,
            type: "existingFiles"
        },
        path: {
            optional: true,
            value: null,
            type: "imagePath"
        },
        commands: {
            optional: true,
            value: Array(),
            type: "array"
        }
    };
    if ( !installs.fetchdata(obj, data) ) return;

    var pngscale = project.buildPath("QtopiaSdk:/bin/pngscale");

    var rule = project.rule("install_"+obj.name);

    if ( data.path.value ) {
        rule.other = data.path.value;
        rule.commands.append("#(e)$$MKSPEC.MKDIR $$[OTHER.0]");
    }

    if ( data.files.value ) {
        for ( var ii in data.files.value ) {
            var file = data.files.value[ii];
            rule.inputFiles.append(file);
            rule.commands.append(pngscale+" -width $$QTOPIA_DISP_WIDTH -height $$QTOPIA_DISP_HEIGHT $$path("+file+",existing) $$[OTHER.0]/"+basename(file));
        }
    }
    for ( var ii in data.commands.value ) {
        rule.commands.append(data.commands.value[ii]);
    }

    // We need to ensure pngscale has been built before it is run!
    installs_buildBeforeRule(rule, "/src/tools/pngscale");

    var install = installs_getImage();
    install.prerequisiteActions.append(rule.name);
    installs_process_depends(rule, obj);
    project.property("pkg.default_targets").unite(rule.name);
}

/*!

\hint sxe

*/
function installs_hint_sxe(obj)
{
    var data = {
        domain: {
            optional: true,
            value: "none",
            type: "single"
        },
        files: {
            optional: true,
            value: null,
            type: "existingFiles"
        },
        path: {
            optional: true,
            value: null,
            type: "imagePath"
        },
        commands: {
            optional: true,
            value: Array(),
            type: "array"
        }
    };
    if ( !installs.fetchdata(obj, data) ) return;

    var sxe_installer = project.buildPath("QtopiaSdk:/bin/sxe_installer");

    // You don't set target.files, it is set for you
    if ( obj.name == "target" ) {
        var target = project.property("COMPILER.TARGETFILE").strValue();
        data.files.value = new Array(target);
        // domain must be trusted so just force it
        data.domain.value = "trusted";
        if ( basename(target) == "qpe" )
            // the server has a special domain
            data.domain.value = "qpe";
    }

    var rule = project.rule("register_domain_"+obj.name);
    rule.prerequisiteActions.append("install_"+obj.name);

    if ( data.path.value ) {
        rule.other = data.path.value;
        rule.commands.append("#(e)$$MKSPEC.MKDIR $$[OTHER.0]");
    }

    if ( data.files.value ) {
        for ( var ii in data.files.value ) {
            var file = data.files.value[ii];
            rule.inputFiles.append(file);
            var destfile = data.path.value+"/"+basename(file);
            rule.commands.append("#(eh)echo sxe_installer "+destfile);
            rule.commands.append("#(E)"+sxe_installer+" $$QTOPIA_PREFIX $$QTOPIA_IMAGE "+destfile+" "+data.domain.value);
        }
    }
    for ( var ii in data.commands.value ) {
        rule.commands.append(data.commands.value[ii]);
    }

    // We need to ensure sxe_installer has been built before it is run!
    installs_buildBeforeRule(rule, "/src/tools/sxe_installer");

    var install = installs_getImage();
    install.prerequisiteActions.append(rule.name);
    installs_process_depends(rule, obj);
}

function installs_buildBeforeRule(rule, projname)
{
    // Filter out any projects built in another location (ie. external to the build or source tree)
    if (!project.config("in_build_tree"))
        return;
    var node = projname+"/";
    if ( qbuild.root.isSubproject(project.sproject(node)) ) {
        rule.prerequisiteActions.append(node+"target");
        // An extra check so we bail if the required project has been disabled
        rule.prerequisiteActions.append(node+"check_enabled");
    }
}

function installs_warning(message)
{
    if ( project.config("soft_disable") ) {
        // Soft-disabled projects with install "warnings" get converted to disabled projects.
        // Anything that was hoping to un-soft-disable this project by depending on it will
        // no longer be able to do so. The alternative is to go ahead and call project.warning
        // but that will stop the build (not something we want to do for a disabled project).
        qbuild.invoke("disable_project", message);
    } else {
        project.warning(message);
    }
}

function installs_device_compat_match(v)
{
    // For compatibility with pre-4.4.2 device profiles, first search an absolute
    // path when a relative path to etc is used.
    if ( v.match(/^etc\//) )
        return v;

    // A special case for server projects
    if ( project.sproject().node() == "/src/server/" ) {
        if ( v.match(/\/etc\//) )
            return v.replace(/^.*\/etc\//, "etc/");
    }

    return null;
}

function installs_filter(list)
{
    if ( list == null )
        return list;

    // Filter out items that have the same filename.
    // This is needed because project.files("foo") returns multiple values
    // if foo exists in multiple real locations mapped to the same solution path.
    // For example, a Greenphone, QVFb build finds 3 Storage.conf files.
    if ( list instanceof Array ) {
        var tmp = new Array;
        var files = new Array;
        for ( var ii in list ) {
            var item = list[ii];
            var file = item.name();
            if ( !array_contains(files, file) ) {
                tmp.push(item);
                files.push(file);
            }
        }
        return tmp;
    }

    return list;
}

function installs_existing_only(files)
{
    // Filter out non-existing items (QBuild wants us to know about things that don't exist!)
    var ret = new Array;
    if ( files != null ) {
        for ( ii in files ) {
            if ( files[ii].exists() ) {
                echo("installs.files", "found file "+files[ii].filesystemPath());
                ret.push(files[ii]);
            }
        }
    }
    return ret;
}

function installs_get_files(v)
{
    // Get files and directories that match v
    var v2 = installs_device_compat_match(v);
    if ( v2 ) {
        var ff = installs_existing_only(project.files("/"+v2));
        var fd = installs_existing_only(project.paths("/"+v2));
        var f = ff.concat(fd);
        if ( f.length != 0 )
            return f;
    }
    var ff = installs_existing_only(project.files(v));
    var fd = installs_existing_only(project.paths(v));
    var f = ff.concat(fd);
    return f;
}

