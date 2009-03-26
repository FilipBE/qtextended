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

function get_sourcepath(module)
{
    // This holds the module -> project mapping
    var modules_matches = depends_get_modules_matches();

    var found = 0;
    var sourcepath = project.property("SOURCEPATH");
    if ( modules_matches.isProperty(module) ) {
        var matches = modules_matches.property(module).value();
        var found = false;
        for ( var jj in matches ) {
            var match = matches[jj];

            // Don't process me (causes deadlock)
            if ( match == project.absName )
                continue;

            // Check that this is the right project...
            var sproj = project.sproject(match);
            if ( !sproj.filename() )
                continue;
            // Read in qbuild.pro from the dependency
            var depproj = sproj.project();
            if ( depproj.disabledReason() )
                continue;
            if ( !depproj.isProperty("MODULE_NAME") || module != depproj.property("MODULE_NAME").strValue() )
                continue;

            var paths;
            if ( depproj.isProperty("SOURCEPATH") )
                paths = depproj.property("SOURCEPATH").value();
            else
                paths = new Array();
            paths.push(match.replace(/\/$/, ""));
            for ( var ii in paths ) {
                var path = paths[ii];
                if ( new File(path).isRelative() )
                    path = match+path;
                sourcepath.unite(path);
            }

            if ( depproj.isProperty("VPATH") )
                paths = depproj.property("VPATH").value();
            else
                paths = new Array();
            paths.push(match.replace(/\/$/, ""));
            for ( var ii in paths ) {
                var path = paths[ii];
                if ( new File(path).isRelative() )
                    path = match+path;
                sourcepath.unite(path);
            }
            found = 1;
            break;
        }
    }
    if ( !found )
        warning("Module "+module+" has no matches!");
}

