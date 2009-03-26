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

\extension singleexec_libs

*/
function singleexec_libs_init()
{
###
    QMAKE.FINALIZE.singleexec_libs.CALL=singleexec_libs_finalize
    QMAKE.FINALIZE.singleexec_libs.RUN_BEFORE_ME=depends
    QMAKE.FINALIZE.singleexec_libs.RUN_AFTER_ME=cpp_compiler
###
}

function singleexec_libs_finalize()
{
    if ( project.config("singleexec") && project.config("enable_singleexec") )
        return;

    // A special case for the server (or anyone else) linking to singleexec_link libs
    if ( project.isProperty("WLIBS") && !project.property("WLIBS").isEmpty() ) {
        var libs = project.property("LIBS").value();
        project.property("LIBS").setValue("-Wl,-whole-archive");
        project.property("LIBS").append(project.property("WLIBS").value());
        project.property("LIBS").append("-Wl,-no-whole-archive");
        project.property("LIBS").append(libs);
    }
}

