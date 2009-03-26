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

\extension depends

The depends extension provides two interfaces for inter-project dependencies.

Projects can influence their dependants by creating depends objects.
\code
OBJECT.TYPE=DEPENDS
OBJECT.EVAL="FOO*=bar"
\endcode

You can use the following \c TYPE modifiers to affect how depends objects work.
\list type=dl
\o IMMEDIATE Only projects directly depending on this project will process this dependency.
\o PERSISTED This dependency will be written to the \l module.dep file so that it can be
   processed when this project does not exist.
\o SDK This PERSISTED dependency will be exported to the SDK. Most PERSISTED rules want
   to be exported to the SDK but any rules referencing paths in the build or source trees
   cannot be exported (as those paths will not exist when the SDK is deployed).
\o METOO This dependency will also be processed by the project it was declared in.
\endlist

Projects register their dependencies with the DEPENDS variable or with the MODULES variable.

\code
DEPENDS=/solution/path/to/other_project
MODULES=other_project
\endcode

The advantage of MODULES is that you do not need to know the exact location of the other project and
you can use the project even if it does not exist (which is the case when you depend on a project in
an SDK). If you depend on a module and it does not exist the project tree will be searched and if the
module is found your project will do an immediate DEPENDS-style load of the other project. This allows
greater parallelization when building. Note that extra dependencies will be inserted in this case to
ensure things aren't built out of order.

Note that some variables set by another project will be automatically imported into your project due
to the DEPENDS.AUTO_PERSIST variable. This is currently used by the \c qt and \c qtopia extensions
to let QT and QTOPIA values get inherited.

The \c .EVAL object is considered to be a list of statements that are evaluated by dependant projects.

\code
OBJECT.TYPE=DEPENDS
OBJECT.EVAL=\
    "FOO=foo bar" "BAR=""foo bar"""\
    "contains(FOO,foo):message(FOO)"\
    "contains(BAR,foo):message(BAR)"
\endcode

This equates to the following code:
\code
FOO=foo bar
BAR="foo bar"
contains(FOO,foo):message(FOO)
contains(BAR,foo):message(BAR)
\endcode

Note how the quotes were handled. Individual strings are interpreted as lines.
Embedded quotes become quotes. This makes the first test true and the second test false.

*/

/*!

\file module.dep
\brief Describes the module.dep file and its contents.

The module.dep file is a project file with all but the dependency information removed.

\sa {depends (Extension)}
*/

/*!

\qbuild_variable DEPENDS
\ingroup depends_extension

Specifies the projects that your project depends on.

Note that this supports the same :: syntax used by MODULES. In addition to the rules
listed in modules you can also use ::persisted which simulates module.dep files for
projects that are not under the current root.

\sa MODULES

*/

/*!

\qbuild_variable MODULE_NAME
\ingroup depends_extension

Specifies the name of the module. Note that this must currently be unset or set to the name of the directory
the project is in.

*/

/*!

\qbuild_variable MODULES
\ingroup depends_extension

Specifies the modules that your project depends on.

Note that you can extract part of a dependency using the following notation.

\code
MODULES*=foo::rule
\endcode

You must know what rules the project exports. It's the name of the object that is exported.
For example a rule called "rule" would be created by.

\code
rule.TYPE=DEPENDS PERSISTED
rule.EVAL="message(hi mom)"
\endcode

Note that any prefix does not count in the rule's name so the following has the same effect.

\code
SOME.PREFIX.rule.TYPE=DEPENDS PERSISTED
SOME.PREFIX.rule.EVAL="message(hi mom)"
\endcode

Common rules are:
\list type=dl
\o headers gives you the INCLUDEPATH set by a project
\o lib gives you the LIBS set by a project
\endlist

Note that the rule 'exists' is provided to check that a given project is enabled.
It returns true even if the other project cannot create a module.dep file.

\sa DEPENDS

*/

/*!

\qbuild_variable DEPENDS.AUTO_PERSIST
\ingroup depends_extension

This variable lists variables that are automatically promoted from dependencies. Note that
it must be set in the project exporting the dependencies, not the project importing them.

This is mostly used by extensions so that a project doing \c{QTOPIA*=phone} will also get the
\c{QTOPIA*=comm} that libqtopiaphone sets.

*/

function depends_init()
{
###
    QMAKE.FINALIZE.depends.CALL = depends_finalize
    QMAKE.FINALIZE.depends.RUN_BEFORE_ME = templates
    MODULES=
    DEPENDS.AUTO_PERSIST=MODULES
###
    // These variables track dependencies.
    project.property("DEPENDS.MODULES").clear();
    project.property("DEPENDS.DEPENDS").clear();
}

function depends_finalize()
{
    // Populate MODULES.MATCHES
    if ( project.name.match(/depends\.pri/) )
        depends_get_modules_matches();

    if ( !project.config("depends") || project.absName.match(/\{file\}\/$/) )
        return;

    // This holds the contents to be written to the module.dep file. It's generated as we process our dependencies.
    project.property("MODULE_CONTENTS").clear();
    project.property("SDK_MODULE_CONTENTS").clear();

    depends_process_metoo();
    //project.info("calling depends_load_dependencies from depends_finalize");
    depends_load_dependencies();
    depends_create_module_dep();

###
    CONFIG+=runlast
    RUNLAST+="depends_hook_sub_rules()"
###

    var toprule = project.rule("print_depends");
    toprule.help = "Print out dependency information";
    var rule = project.rule();
    toprule.prerequisiteActions.append(rule.name);
    var mod = project.property("DEPENDS.MODULES").value();
    for ( var ii in mod ) {
        var path = project.property("DEPENDS.MODULES").property(mod[ii]).property("LOCATION").strValue();
        if ( path ) {
            rule.commands.append("#(eh)echo $$shellQuote(DEPENDS: "+project.absName+" depends on "+path+")");
            toprule.prerequisiteActions.append(path+"print_depends");
        }
    }

    var dep = project.property("DEPENDS.DEPENDS").value();
    for ( var ii in dep ) {
        var path = dep[ii]+"/";
        rule.commands.append("#(eh)echo $$shellQuote(DEPENDS: "+project.absName+" depends on "+path+")");
        toprule.prerequisiteActions.append(path+"print_depends");
    }
}

/*
  Process all objects with TYPE=DEPENDS METOO.
  Execute the .EVAL statements.
*/
function depends_process_metoo()
{
    echo("dep.proc", "Resolving METOO dependencies for project "+project.absName);
    var deps = project.find("TYPE", "DEPENDS");
    for ( var jj in deps ) {
        var obj = qbuild.object(deps[jj]);
        if ( obj.property("TYPE").contains("METOO") ) {
            // Run each METOO object on the project
            depends_runDependObject(obj, 0);
        }
    }
}

/*
  Load all pending dependencies (both MODULES and DEPENDS).
  This operates in a loop so that indirect dependencies are also processed.
  Returns the number of dependencies loaded.
*/
function depends_load_dependencies()
{
    if ( !project.config("depends") || project.absName.match(/\{file\}\/$/) )
        return 0;

    var ret = 0;
    var loaded = 0;
    do {
        loaded = depends_load_modules(project.property("MODULES"));
        loaded += depends_resolve_depends(project, 0);
        ret += loaded;
    } while ( loaded );
    return ret;
}

/*
  Load DEPENDS values into project \a proj.
  Returns the number of projects processed.
*/
function depends_resolve_depends(proj, module)
{
    var depends = proj.property("DEPENDS");
    if ( depends.isEmpty() )
        return 0;
    echo("dep.proc.misc", "Resolving dependencies for project "+proj.absName);

    // We need to decode the incoming rules because they might contain rules (eg. module::rule)
    var depends_pending = project.property("DEPENDS.DEPENDS");
    depends_decode_rules(depends_pending, depends);

    var processed = 0;
    var depends = depends_pending.value();
    for ( var ii in depends ) {
        var depend_name = depends[ii];
        if ( depends_pending.property("PROCESSED").contains(depend_name) ) {
            echo("dep.proc.misc", "Processing "+depend_name+" again");
            //continue;
        } else {
            echo("dep.proc", "Processing "+depend_name);
        }
        var ret = depends_process_dependency(proj, depend_name, depends_pending, module);
        if ( ret > 0 ) {
            echo("dep.proc", "Something happened in "+depend_name);
            processed++;
            depends_pending.property("PROCESSED").append(depend_name);
        }
    }
    return processed;
}

/*
  Load dependency \a depend_name into project \a proj.
  Returns the number of rules executed or -1 if the dependency could not be opened.
*/
function depends_process_dependency(proj, depend_name, ruledb, module)
{
    var sproj = project.sproject(depend_name);

    var ruledbindex = depends_ruledbindex(module?module:depend_name);
    var wanted_rules = ruledb.property(ruledbindex).property("WANTED");
    echo("dep.rules", "Using rules "+ruledb.name+"."+ruledbindex+" ("+wanted_rules.value().join(", ")+")");

    if ( !sproj.filename() ) {
        if ( module ) {
            echo("dep.misc", "sproj.filename() is null");
            return -1; // Continue searching
        } else
            project.warning("Dependency "+depend_name+" not found");
    } else if ( qbuild.root.isSubproject(sproj) ) {
        // Read in qbuild.pro from the dependency
        var depproj = sproj.project();
        if ( !module && depproj.disabledReason() ) {
            if ( !project.disabledReason() ) {
                var m = module;
                if ( !m )
                    m = depend_name;
                echo("disable", "disable project because dependency "+m+" was disabled");
                project.reset("depends");
                qbuild.invoke("disable_project", "Disabled because dependency '"+m+"' was disabled ("+depproj.disabledReason()+").");
            }
            return 0; // stop searching
        }
        if ( module && ( !depproj.isProperty("MODULE_NAME") || module != depproj.property("MODULE_NAME").strValue() || depproj.disabledReason() ) ) {
            echo("dep.misc", "Looking for module "+module+" and found "+(depproj.isProperty("MODULE_NAME")?"``"+depproj.property("MODULE_NAME").strValue()+"''":"no MODULE_NAME"));
            return -1; // continue searching
        }
        // Our check_enabled rule depends on their check_enabled rule
        project.rule("check_enabled").prerequisiteActions.append(depproj.absName+"check_enabled");
        var ret = depends_process_dep_project(proj, depend_name, ruledb, module, depproj);
        return ret;
    } else {
        if ( module ) {
            echo("dep.misc", "!qbuild.root.isSubproject(sproj)");
            return -1; // Continue searching
        } else {
            if ( wanted_rules.contains("persisted") ) {
                // Read in qbuild.pro from the dependency
                var depproj = sproj.project();
                // Our check_enabled rule depends on their check_enabled rule
                project.rule("check_enabled").prerequisiteActions.append(depproj.absName+"check_enabled");
                var ret = depends_process_dep_project(proj, depend_name, ruledb, module, depproj, 1);
                return ret;
            } else if ( wanted_rules.contains("exists") ) {
                /* FIXME - we want to check something here but we can't open the project or it'll
                           just die (it's out of the tree)
                // Read in qbuild.pro from the dependency
                var depproj = sproj.project();
                // Our check_enabled rule depends on their check_enabled rule
                project.rule("check_enabled").prerequisiteActions.append(depproj.absName+"check_enabled");
                */
                return 0;
            } else {
                project.warning("Dependency "+depend_name+" is outside the current project root ("+qbuild.root.node()+").");
            }
        }
    }
    return 0; // stop searching
}

/*
  Dependency \a depend_name (project \a depproj) is being processed into project \a proj.
  Returns the number of rules executed.
*/
function depends_process_dep_project(proj, depend_name, ruledb, module, depproj, persistedonly)
{
    var deps = depproj.find("TYPE", "DEPENDS");
    if ( !deps.length ) {
        if (module)
            return 0;
        project.warning("Dependency "+depend_name+" does not export any TYPE=DEPENDS objects");
    }
    var ruledbindex = depends_ruledbindex(module?module:depend_name);
    var wanted_rules = ruledb.property(ruledbindex).property("WANTED");
    var processed_rules = ruledb.property(ruledbindex).property("PROCESSED");
    echo("dep.rules", "Using rules "+ruledb.name+"."+ruledbindex+" ("+wanted_rules.value().join(", ")+")");
    echo("dep.rules", "WANTED "+wanted_rules.value().join(", "));
    echo("dep.rules", "PROCESSED "+processed_rules.value().join(", "));
    if ( wanted_rules.strValue() == "exists" ) {
        if ( processed_rules.contains("exists") )
            return 0;
        processed_rules.unite("exists");
        return 1;
    }
    // Our dependency has been soft-disabled! We will need to hook up some rules so that it gets built properly.
    if ( !depend_name.match(/.dep$/) && depproj.isProperty("SOFT_DISABLE_MESSAGE") )
        project.property("DEPENDS.SOFT_DISABLED_DEPENDENCIES").unite(depproj.absName);

    var rules = 0;
    var persisted = 0;
    echo("dep.rules.misc", "Found DEPENDS objects "+deps.join(", "));
    if ( ( wanted_rules.contains("all") || wanted_rules.contains("auto_persist") )
               && !processed_rules.contains("auto_persist") ) {
        processed_rules.unite("auto_persist");
        echo("dep.rules", "Checking for auto_persist'ed variables")
        if ( depproj.isProperty("DEPENDS.AUTO_PERSIST") ) {
            var auto_persist = depproj.property("DEPENDS.AUTO_PERSIST").value();
            for ( var ii in auto_persist ) {
                var variable = auto_persist[ii];
                echo("dep.rules", "variable "+variable);
                // Add any <variable> from the dependency
                if ( depproj.isProperty(variable) && depproj.property(variable).value().length ) {
                    persisted++;
                    echo("dep.rules", "Adding "+variable+" ("+depproj.property(variable).value().join(", ")+") from dependency");
                    project.property(variable).unite(depproj.property(variable).value());
                    rules++;
                }
            }
        }
        if ( !module && depproj.isProperty("MODULES") ) {
            // Recursively load dependencies
            depends_load_modules(depproj.property("MODULES"));
            depends_resolve_depends(depproj, module);
        }
    }

    for ( var jj in deps ) {
        var obj = qbuild.object(deps[jj]);
        echo("dep.rules.misc", "Found DEPENDS object "+deps[jj]);
        if ( obj.property("TYPE").contains("PERSISTED") ) {
            persisted++;
            if ( persistedonly && (proj == project || !obj.property("TYPE").contains("IMMEDIATE")) ) {
                if ( ( wanted_rules.contains("all") || wanted_rules.contains(obj.name) )
                     && !processed_rules.contains(obj.absName) ) {
                    processed_rules.unite(obj.absName);
                    echo("dep.rules", "Running depends object (persistedonly) "+obj.absName);
                    depends_runDependObject(obj, 0);
                    rules++;
                } else if ( !wanted_rules.contains(obj.name) ) {
                    echo("dep.rules", "Skipping object "+obj.name+" because it's not in the rules");
                }
            }
        }
        if ( persistedonly )
            continue;
        if ( proj != project && obj.property("TYPE").contains("IMMEDIATE") ) {
            echo("dep.rules", "... but it's immediate and we're not immediately depending on the project");
            continue;
        }
        if ( ( wanted_rules.contains("all") || wanted_rules.contains(obj.name) )
             && !processed_rules.contains(obj.absName) ) {
            processed_rules.unite(obj.absName);
            echo("dep.rules", "Running depends object "+obj.absName);
            depends_runDependObject(obj, 0);
            rules++;
        } else if ( !wanted_rules.contains(obj.name) ) {
            echo("dep.rules", "Skipping object "+obj.name+" because it's not in the rules");
        }
    }

    if ( module && !persisted )
        project.warning("Dependency "+depend_name+" cannot create a module.dep file");
    return rules;
}

/*
  Execute dependency object \a src_object into the project.
  If \a printonly is 1 save the commands into MODULE_CONTENTS instead of executing them.
*/
function depends_runDependObject(src_object, printonly)
{
    var properties = src_object.properties();
    if ( !properties ) {
        project.warning("Dependency object "+src_object.absName+" has no properties!");
        return;
    }

    // This is used to set the contents of the module.dep file when printonly is set
    var module_contents = project.property("MODULE_CONTENTS");
    var sdk_module_contents = project.property("SDK_MODULE_CONTENTS");

    for ( var ii in properties ) {
        var property = properties[ii];
        var src_prop = src_object.property(properties[ii]);
        if ( property == "EVAL" ) {
            // evaluate the command
            echo("dep.rules.run", "Dependency object "+src_object.absName+" has property "+property+" (evaluated)");
            var snippet = src_prop.value().join("\n");
            echo("dep.rules.run", "eval: "+snippet);
            if ( printonly ) {
                var propname = src_object.absName;
                // strip out the path and the module name
                propname = propname.replace(/^.*\/[^\.]+\//, "");
                echo("dep.rules.run", "Saving snippet to MODULE_CONTENTS."+propname);
                module_contents.property(propname).append(snippet);
                module_contents.append(propname);
                if ( src_object.property("TYPE").contains("SDK") ) {
                    echo("dep.rules.run", "Saving snippet to SDK_MODULE_CONTENTS."+propname);
                    sdk_module_contents.property(propname).append(snippet);
                    sdk_module_contents.append(propname);
                }
            } else {
                project.run(snippet);
            }
        } else {
            // Do nothing
            echo("dep.rules.run", "Dependency object "+src_object.absName+" has property "+property+" (ignored)");
        }
    }
}

/*
  Load MODULES from \a modules.
  Returns the number of projects processed.
*/
function depends_load_modules(modules)
{
    if ( !project.config("depends") || project.absName.match(/\{file\}\/$/) )
        return 0;

    if ( modules.isEmpty() )
        return 0;

    echo("dep.proc.misc", "Resolving modules ("+modules.value().join(", ")+") for project "+project.absName);

    // We need to decode the incoming rules because they might contain rules (eg. module::rule)
    var modules_pending = project.property("DEPENDS.MODULES");
    depends_decode_rules(modules_pending, modules);

    // This holds the module -> project mapping
    var modules_matches = depends_get_modules_matches();

    var processed = 0;
    var modules = modules_pending.value();
    for ( var ii in modules ) {
        var module = modules[ii];
        if ( modules_pending.property("PROCESSED").contains(module) ) {
            echo("dep.proc.misc", "Processing "+module+" again");
            //continue;
        } else {
            echo("dep.proc", "Processing "+module);
        }
        echo("dep.proc.misc", "Searching for module "+module);
        if ( modules_matches.isProperty(module) ) {
            var matches = modules_matches.property(module).value();
            var found = false;
            for ( var jj in matches ) {
                var match = matches[jj];

                // Don't process me (causes deadlock)
                if ( match == project.absName )
                    continue;

                modules_pending.property(module).property("LOCATION").setValue(match);

                // Load the project directly, as it would be if it was listed in DEPENDS
                var ret = depends_process_dependency(project, match, modules_pending, module);
                if ( ret != -1 ) {
                    if ( ret > 0 ) {
                        echo("dep.proc", "Something happened in "+module);
                        processed++;
                    }
                    echo("dep.proc.misc", "Found MODULE "+module+" in "+match);
                    modules_pending.property("PROCESSED").append(module);
                    found = true;
                    break;
                }
            }
            if ( found ) continue;
        } else {
            echo("dep.proc.misc", "Module "+module+" has no matches!");
        }

        // Try <module>.dep file
        var module_dep = "/modules/"+module+".dep";
        echo("dep.proc.misc", "I couldn't find module "+module+" directly so I'm going to try "+module_dep);
        if ( project.file(module_dep) ) {
            modules_pending.property("PROCESSED").append(module);
            echo("dep.proc.misc", "Loading "+module_dep);
            var proj = project.sproject(module_dep).fileModeProject();
            if (proj.isProperty("SOURCE_PROJECT")) {
                var source_project = proj.property("SOURCE_PROJECT").strValue();
                project.rule("check_enabled").prerequisiteActions.append("#(o)"+source_project+"check_enabled");
                /*
                if ( qbuild.root.isSubproject(project.sproject(source_project)) ) {
                    var depproj = project.sproject(source_project).project();
                    if (depproj.disabledReason()) {
                        // We found a module.dep but the project itself has been disabled.
                        project.rule("check_enabled").prerequisiteActions.append(depproj.absName+"check_enabled");
                    }
                }
                */
            }
            var ret = depends_process_dep_project(project, module_dep, modules_pending, module, proj);
            if ( ret > 0 ) processed++;
        } else {
            var sdk_module_dep = "QtopiaSdk:/modules/"+module+".dep";
            echo("dep.proc.misc", "I couldn't find "+module_dep+" so I'm going to try "+sdk_module_dep);
            if ( project.file(sdk_module_dep) ) {
                modules_pending.property("PROCESSED").append(module);
                echo("dep.proc.misc", "Loading "+sdk_module_dep);
                var proj = project.sproject(sdk_module_dep).fileModeProject();
                var ret = depends_process_dep_project(project, sdk_module_dep, modules_pending, module, proj);
                if ( ret > 0 ) processed++;
            } else {
                var tried = (modules_matches.isProperty(module)?modules_matches.property(module).value().join(", ")+", ":"")+
                            module_dep+" and "+sdk_module_dep;
                if ( modules_matches.isProperty(module) && !modules_matches.property(module).isEmpty() ) {
                    if ( !project.disabledReason() ) {
                        // do a soft disable because there were matches (probably outside the root)
                        echo("disable", "disable project because dependency "+module+" cannot be located");
                        qbuild.invoke("disable_project", "Cannot locate module "+module+" (tried "+tried+")");
                    }
                } else {
                    if ( !project.disabledReason() ) {
                        echo("disabled", "disable project because dependency "+module+" cannot be located");
                        qbuild.invoke("disable_project", "Cannot locate module "+module+" (tried "+tried+")");
                    }
                }
                return 0;
            }
        }
    }
    return processed;
}

/*
  Create the module.dep file based on the contents of MODULE_CONTENTS.
*/
function depends_create_module_dep()
{
    var depends = project.find("TYPE", "DEPENDS");
    if ( !depends.length || project.property("MODULE_NAME").isEmpty() )
        return;
    echo("dep.module", "Creating module_dep rule for "+project.name);

    var module_name = basename(project.name.replace(/\/$/, ''));
    // MODULE_NAME can be set but it must be set to the name of the project
    var check_module_name = project.property("MODULE_NAME");
    if ( check_module_name.strValue() != module_name ) {
        project.message("You must set MODULE_NAME to the name of the project ("+module_name+") to create a module.dep file.");
        return;
    }
    echo("dep.module", "MODULE_NAME "+module_name);

    // Find out what dependencies are to be persisted?
    for ( var ii in depends ) {
        var dep = qbuild.object(depends[ii]);
        // Only save persisted objects
        if ( !dep.property("TYPE").contains("PERSISTED") )
            continue;
        depends_runDependObject(dep, 1);
    }

    var module_contents = depends_convert_module_contents("MODULE_CONTENTS");
    var sdk_module_contents = depends_convert_module_contents("SDK_MODULE_CONTENTS");

    // Embed our auto-persist dependencies too
    var auto_persist = project.property("DEPENDS.AUTO_PERSIST").value();
    module_contents.append("DEPENDS.AUTO_PERSIST="+auto_persist.join(" "));
    sdk_module_contents.append("DEPENDS.AUTO_PERSIST="+auto_persist.join(" "));
    for ( var ii in auto_persist ) {
        var variable = auto_persist[ii];
        var value = project.property(variable);
        if ( !value.isEmpty() ) {
            module_contents.append(variable+"*="+value.strValue());
            sdk_module_contents.append(variable+"*="+value.strValue());
        }
    }

    // You can't make a module.dep file with no contents!
    if ( module_contents.isEmpty() && sdk_module_contents.isEmpty() )
        return;

    // Save the source project (so we know if it has been disabled!)
    module_contents.append("SOURCE_PROJECT="+project.absName);

    // Combine the items into a string
    var tmp = module_contents.value().join("\n")+"\n";
    module_contents.clear();
    module_contents.setValue(tmp);
    echo("dep.module", "MODULE_CONTENTS:\n"+module_contents.strValue());

    tmp = sdk_module_contents.value().join("\n")+"\n";
    sdk_module_contents.clear();
    sdk_module_contents.setValue(tmp);
    echo("dep.module", "SDK_MODULE_CONTENTS:\n"+sdk_module_contents.strValue());

    var filename = project.buildFile("/modules/"+module_name+".dep").filesystemPath();
    project.rule("local_module_dep").set = {
        outputFiles: filename,
        other: File(filename).path(),
        tests: "$$not($$testFile(!$$[OUTPUT.0],$$MODULE_CONTENTS))",
        commands: [ "#(ne)$$MKSPEC.MKDIR $$[OTHER.0]",
                    "#(ne)$$writeFile(!$$[OUTPUT.0],$$MODULE_CONTENTS)" ]
    };

    var sdk_filename = project.buildFile("QtopiaSdk:/modules/"+module_name+".dep").filesystemPath();
    project.rule("sdk_module_dep").set = {
        outputFiles: sdk_filename,
        other: File(sdk_filename).path(),
        tests: "$$not($$testFile(!$$[OUTPUT.0],$$SDK_MODULE_CONTENTS))",
        commands: [ "#(ne)$$MKSPEC.MKDIR $$[OTHER.0]",
                    "#(ne)$$writeFile(!$$[OUTPUT.0],$$SDK_MODULE_CONTENTS)" ]
    };

    project.rule("module_dep").set = {
        help: "Write "+module_name+".dep module description",
        prerequisiteActions: [ "#(o)target_post", "local_module_dep", "sdk_module_dep" ],
        serial: true
    };
    project.rule("default").prerequisiteActions.append("module_dep");

    var rule = project.rule("clean_module");
    rule.commands.append("$$MKSPEC.DEL_FILE $$shellQuote("+filename+")");
    rule.commands.append("$$MKSPEC.DEL_FILE $$shellQuote("+sdk_filename+")");
    project.rule("clean").prerequisiteActions.append(rule.name);
}

/*
  Returns a property that maps module names to projects that may implement the module.
*/
function depends_get_modules_matches()
{
    var modules_matches;
    if ( project.name.match(/depends\.pri/) ) {
        modules_matches = project.property("MODULES.MATCHES");
        // Prep MODULES.MATCHES (with /src)
        var sproj = project.sproject("/src/");
        var sub = sproj.subprojects();
        depends_module_search("/src/", sub);
        // Prep MODULES.MATCHES (with /examples)
        sproj = project.sproject("/examples/");
        sub = sproj.subprojects();
        depends_module_search("/examples/", sub);
    } else {
        var root = project.sproject("/extensions/depends.pri").fileModeProject();
        modules_matches = root.property("MODULES.MATCHES");
    }
    return modules_matches;
}

/*
  Decode module/dependency rules from \a modules into \a depvar.
  This is what makes the ::rule syntax work.
*/
function depends_decode_rules(depvar, modules)
{
    depvar.setValue(""); // only pull in new ones
    var module_values = modules.value();
    for ( var ii in module_values ) {
        var module = module_values[ii];
        var module_name = String(module.match(/[^:]+/));
        echo("dep.misc", "module/depends name "+module_name);
        /*
        if ( depvar.property("PROCESSED").contains(module_name) ) {
            // We've already processed this item!
            echo("dep.misc", "We've already processed "+module_name);
            continue;
        }
        */
        depvar.unite(module_name);
        // Sanitise the name (for DEPENDS values, which may have illegal characters in them)
        var ruledbindex = depends_ruledbindex(module_name);
        if ( module.match(/::/) ) {
            var module_rule = module.replace(/^[^:]+::/, "");
            echo("dep.misc", "module/depends rule "+module_rule);
            depvar.property(ruledbindex).property("WANTED").unite(module_rule);
            if ( module_rule == "persisted" )
                depvar.property(ruledbindex).property("WANTED").unite("all");
        } else {
            depvar.property(ruledbindex).property("WANTED").unite("all");
        }
    }
}

/*
  Sanitize \a module_name so that it can be used as a property name.
*/
function depends_ruledbindex(module_name)
{
    var ruledbindex = String(module_name);
    ruledbindex = ruledbindex.replace(/[\/.]/g, "_");
    return ruledbindex;
}

function depends_module_search(path, projects)
{
    if ( !projects )
        return;

    var matches = project.property("MODULES.MATCHES");

    for ( var ii in projects ) {
        var module = projects[ii];
        // Ignore tests directories
        if ( module == "tests" ) continue;
        var match_p = path+module;
        var match = match_p+"/";
        echo("dep.search", "MODULES.MATCHES."+module+"+="+match);
        matches.property(module).append(match);

        // Recurse down the tree
        echo("dep.search", "Recurse down the tree ("+match_p+")");
        var sproj = project.sproject(match_p);
        var sub = sproj.subprojects();
        depends_module_search(match, sub);
    }
}

function depends_hook_sub_rules()
{
    var rules = project.property("SUBDIRS_RULES").value();
    var deps = project.property("DEPENDS.SOFT_DISABLED_DEPENDENCIES").value();
    for ( var ii in rules ) {
        var rule = project.rule(rules[ii]);
        for ( var jj in deps ) {
            rule.prerequisiteActions.append(deps[jj]+rule.name);
        }
    }
}

function depends_convert_module_contents(name)
{
    // Convert the MODULE_CONTENTS variable into a series of persisted dependencies
    // so that we can run over this file like a regular project.
    var module_contents = project.property(name);
    var names = module_contents.value();
    //echo("dep.module", "MODULE_CONTENTS has properties "+names.join(", "));
    module_contents.setValue("");
    for ( var ii in names ) {
        var name = names[ii];
        module_contents.append(name+".TYPE=DEPENDS PERSISTED");
        var line = module_contents.property(name).strValue();
        //echo("dep.module", "line "+line);
        module_contents.append(name+".EVAL+=\""+eval_escape(line)+"\"");
    }
    return module_contents;
}

