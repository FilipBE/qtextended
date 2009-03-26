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

\extension subdirs

*/

/*!

\qbuild_variable SUBDIRS
\ingroup subdirs_extension

*/

/*!

\qbuild_variable SUBDIRS_RULES
\ingroup subdirs_extension

*/

/*!

\qbuild_variable SUBDIRS_EXCLUDE
\ingroup subdirs_extension

*/

function subdirs_init()
{
###
    QMAKE.FINALIZE.subdirs.CALL = subdirs_finalize
    SUBDIRS=all
    SUBDIRS_RULES=default clean force_clean image prep_db remove_target relink headers print_depends lupdate test packages
    SUBDIRS_ORDERED=test
###
}

function subdirs_finalize()
{
    if ( !project.config("subdirs") )
        return;

###
    isEmpty(SUBDIRS_RULES) {
        SUBDIRS_RULES = default
    }
###

    var subdirs_list = project.property("SUBDIRS");
    var subdirs_rules = project.property("SUBDIRS_RULES").value();
    var subdirs_exclude = project.property("SUBDIRS_EXCLUDE");
    var subdirs_ordered = project.property("SUBDIRS_ORDERED");

    if ( subdirs_list.contains("all") ) {
        subdirs_list.setValue(project.sproject(project.absName).subprojects());
        subdirs_list.remove("tests");
    }

    if ( project.absName != "/" )
        subdirs_check_matching_projects(subdirs_list);

    subdirs_list = subdirs_list.value();
    if ( !subdirs_list.length )
        return;

    echo("subdirs", "SUBDIRS "+subdirs_list.join(", "));
    for ( var ii in subdirs_rules ) {
        var top_rule = project.rule(subdirs_rules[ii]);
        var do_sub_help = 1;
        if ( !top_rule.help ) {
            do_sub_help = 0;
            top_rule.help = "Process the "+top_rule.name+" rule in all subdirectories.";
        }
        var subdirs_rule = project.rule("sub_"+subdirs_rules[ii]);
        top_rule.prerequisiteActions.append(subdirs_rule.name);

        if ( subdirs_ordered.contains(subdirs_rules[ii]) )
            subdirs_rule.serial = true;

        for ( var jj in subdirs_list ) {
            var dir = subdirs_list[jj];
            if ( subdirs_exclude.contains(dir) )
                continue;
            // We process default_sub instead of default in subdirectories.
            // This is so that errors can be reported differently.
            var subrulename = top_rule.name+"_sub";
            subdirs_rule.prerequisiteActions.append("#(o)"+dir+"/"+subrulename);
            if ( do_sub_help )
                subdirs_rule.help = "Process the "+top_rule.name+" rule in all subdirectories.";
        }
    }
}

function subdirs_check_matching_projects(subdirs_list)
{
    var projects = project.property("PROJECTS").value();
    for ( var ii in projects ) {
        var proj = "/src/"+projects[ii];
        proj = proj.replace(/[^\/]*$/, "");
        if ( proj.indexOf(project.absName) == 0 ) {
            var deeper = proj.replace(project.absName, "").replace(/\/.*/, "");
            //project.message("deeper? "+deeper+" "+subdirs_list.contains(deeper)+" - "+proj);
            if ( subdirs_list.contains(deeper) )
                continue;
            proj = basename(projects[ii]);
            if ( !subdirs_list.contains(proj) ) {
                // For now it's a warning that you have to opt-in to see
                // Once the old build system is retired this will be a fatal error
                if ( qbuild.invoke("globalValue", "showDisabled").join("") == 1 )
                    project.info("Project "+projects[ii]+" is listed in PROJECTS but has no qbuild.pro!");
            }
        }
    }
}

