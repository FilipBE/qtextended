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

var skip_sdk = false;
qbuild.setValue("js_debug", 0);
qbuild.setValue("showDisabled", 0);
qbuild.setValue("no_load_soft_disabled", 0);

for(var ii in qbuild.options) {
    var option = qbuild.options[ii];

    if(option.name == "sdk_path") 
        solution.addSolution("QtopiaSdk", option.parameters[0]);
    else if(option.name == "spec")
        qbuild.mkspec = option.parameters[0];
    else if ( option.name == "js_debug" )
        qbuild.setValue("js_debug", option.parameters[0]);
    else if ( option.name == "disabled" )
        qbuild.setValue("showDisabled", 1);
    else if ( option.name == "module_testing" )
        qbuild.setValue("no_load_soft_disabled", 1);
}

if(!skip_sdk) {
    if(!solution.isSolution("QtopiaSdk")) {
        // Try environment
        var env = qbuild.env("QTOPIA_SDK_PATH");
        if(env) 
            solution.addSolution("QtopiaSdk", env);
    }

    if(!solution.isSolution("QtopiaSdk")) {
        solution.addSolution("QtopiaSdk", "../")
    }

    qbuild.addExtensions("QtopiaSdk:/extensions");
}

var mkspecenv = qbuild.env("QMAKESPECPATH");
if(mkspecenv)
    qbuild.addMkspecs("!" + mkspecenv);


